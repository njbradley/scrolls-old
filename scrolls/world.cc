#ifndef WORLD
#define WORLD

#include "cross-platform.h"
#include "rendervecs.h"
#include "blockdata.h"
#include "blocks.h"
#include "world.h"
#include "tiles.h"
#include "entity.h"
#include "game.h"
#include "terrain.h"
#include "player.h"
#include "graphics.h"
#include "audio.h"
#include "debug.h"

#include <map>


DEFINE_PLUGIN(World);
EXPORT_PLUGIN(World);

int view_dist = 3;

Tile* TileMap::iterator::operator*() {
  return item->tile;
}

TileMap::iterator TileMap::iterator::operator++() {
  item = item->next;
  move_to_next();
  return *this;
}

void TileMap::iterator::move_to_next() {
  while (item == nullptr and bucket != endbucket) {
    item = bucket->items;
    bucket ++;
  }
}

bool operator!=(const TileMap::iterator& iter1, const TileMap::iterator& iter2) {
  return iter1.item != iter2.item or iter1.bucket != iter2.bucket;
}


TileMap::TileMap(int max_items): num_buckets(max_items * 3/2), num_items(0) {
  buckets = new Bucket[num_buckets];
}

TileMap::~TileMap() {
  for (int i = 0; i < num_buckets; i ++) {
    std::lock_guard guard(buckets[i].lock);
    Item* item = buckets[i].items;
    while (item != nullptr) {
      Item* last = item;
      item = item->next;
      delete last;
    }
  }
  delete[] buckets;
}


void TileMap::add_tile(Tile* tile) {
  Bucket* bucket = buckets + (ivec3_hash()(tile->pos) % num_buckets);
  std::lock_guard guard(bucket->lock);
  Item* item = bucket->items;
  num_items ++;
  bucket->items = new Item {tile, bucket->items};
}

Tile* TileMap::tileat(ivec3 pos) {
  Bucket* bucket = buckets + (ivec3_hash()(pos) % num_buckets);
  std::lock_guard guard(bucket->lock);
  Item* item = bucket->items;
  while (item != nullptr) {
    if (item->tile->pos == pos) {
      return item->tile;
    }
    item = item->next;
  }
  return nullptr;
}
  
Tile* TileMap::del_tile(ivec3 pos) {
  Bucket* bucket = buckets + (ivec3_hash()(pos) % num_buckets);
  std::lock_guard guard(bucket->lock);
  Item** last = &bucket->items;
  while (*last != nullptr) {
    if ((*last)->tile->pos == pos) {
      Item* delitem = *last;
      *last = (*last)->next;
      Tile* tile = delitem->tile;
      delete delitem;
      num_items --;
      return tile;
    }
    last = &(*last)->next;
  }
  cout << "bad" << endl;
  return nullptr;
}

Tile* TileMap::operator[] (ivec3 pos) {
  return tileat(pos);
}

size_t TileMap::size() const {
  return num_items;
}

TileMap::iterator TileMap::begin() {
  iterator iter {buckets, buckets + num_buckets, nullptr};
  iter.move_to_next();
  return iter;
}

TileMap::iterator TileMap::end() {
  iterator iter {buckets + num_buckets, buckets + num_buckets, nullptr};
  return iter;
}

void TileMap::status(ostream& ofile) {
  int maxlen = 0;
  ofile << "Buckets: ";
  for (int i = 0; i < num_buckets; i ++) {
    std::lock_guard guard(buckets[i].lock);
    Item* item = buckets[i].items;
    int len = 0;
    while (item != nullptr) {
      len ++;
      item = item->next;
    }
    if (len > maxlen) {
      maxlen = len;
    }
    ofile << len;
    /*ofile << '[' << i << ']';
    std::lock_guard guard(buckets[i].lock);
    Item* item = buckets[i].items;
    while (item != nullptr) {
      ofile << ' ' << item->tile->pos << '(' << ivec3_hash()(item->tile->pos) << "):" << item->tile;
      item = item->next;
    }
    ofile << endl;*/
  }
  ofile << endl;
  ofile << "Bucket count: " << num_buckets << endl;
  ofile << "Item count: " << num_items << endl;
  ofile << "Average length: " << num_items / double(num_buckets) << endl;
  ofile << "Max bucket length: " << maxlen << endl;
}


World::World(string oldname): terrainloader(seed), tileloader(this), name(oldname),
tiles( ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * ((view_dist-1)*2+1) - 1) {
  ifstream ifile(path("worlddata.txt"));
  if (ifile.good()) {
    load_config(ifile);
  } else {
    setup_files();
  }
  startup();
  tileloader->begin_serving();
}

World::~World() {
  close_world();
}

string World::path(string filename) const {
  return SAVES_PATH + name + '/' + filename;
}


void World::load_config(istream& ifile) {
  string buff;
  getline(ifile, buff);
  getline(ifile,buff,':');
  while ( !ifile.eof() and buff != "end" ) {
    if (buff == "seed") {
      ifile >> seed;
    }
    if (buff == "daytime") {
      ifile >> daytime;
    }
    if (buff == "difficulty") {
      ifile >> difficulty;
    }
    if (buff == "generation") {
      string gen;
      ifile >> gen;
      generation = gen == "on";
    }
    if (buff == "saving") {
      string sav;
      ifile >> sav;
      saving = sav == "on";
    }
    getline(ifile, buff);
    getline(ifile,buff,':');
  }
}

void World::save_config(ostream& ofile) {
  ofile << "Scrolls data file of world '" + name + "'\n";
  ofile << "seed:" << seed << endl;
  ofile << "difficulty:" << difficulty << endl;
  ofile << "daytime:" << daytime << endl;
  ofile << "generation:" << (generation ? "on" : "off") << endl;
  ofile << "saving:" << (saving ? "on" : "off") << endl;
  ofile << "end:" << endl;
}

void World::setup_files() {
    create_dir(path());
    create_dir(path("chunks"));
    create_dir(path("groups"));
    ofstream ofile(SAVES_PATH "saves.txt", std::ios::app);
    ofile << ' ' << name;
}

void World::startup() {
  ifstream ifile(path("player.txt"));
  if (ifile.good()) {
    player = new Player(this, ifile);
    set_player_vars();
  } else {
    spawn_player();
  }
}

void World::spawn_player() {
  ivec3 spawnpos(10,terrainloader->get_height(ivec2(10,10))+4,10);
  int i = 0;
  // while (loader.gen_func(ivec4(spawnpos.x, spawnpos.y, spawnpos.z, 1)) != 0 and i < 100) {
  //   i ++;
  //   spawnpos = ivec3(spawnpos.x+1,loader.get_height(ivec2(10,10))+4,10);
  // }
  player = new Player(this, vec3(spawnpos));
  // player->spectator = true;
  player->autojump = true;
  player->health = 10;
  player->max_health = 10;
  set_player_vars();
}

void World::set_player_vars() {
  audio->set_listener(&player->position, &player->vel, &player->angle);
  graphics->set_camera(&player->position, &player->angle);
  viewbox->playerpos = &player->position;
}

void World::load_nearby_chunks() {
  static double start_time = getTime();
  if (!player->spectator) {
    
    bool loaded_chunks = false;
    
    int px = player->position.x/chunksize - (player->position.x<0);
    int py = player->position.y/chunksize - (player->position.y<0);
    int pz = player->position.z/chunksize - (player->position.z<0);
    
    const int maxrange = view_dist;
    
    int max_chunks = ((view_dist)*2+1) * ((view_dist)*2+1) * ((view_dist)*2+1);
    int num_chunks = tiles.size();
    
    if (!world_closing) {
      for (int range = 0; range < maxrange; range ++) {
        // for (int y = py-range; y < py+range+1; y ++) {
        for (int y = py+range; y >= py-range; y --) {
          for (int x = px-range; x < px+range+1; x ++) {
            for (int z = pz-range; z < pz+range+1; z ++) {
              if (x == px-range or x == px+range or y == py-range or y == py+range or z == pz-range or z == pz+range) {
                ivec3 pos(x,y,z);
                if (tileat(pos) == nullptr and num_chunks < max_chunks and loading_chunks.count(pos) == 0) {
                  loaded_chunks = true;
                  num_chunks ++;
                  tileloader->request_load_tile(pos);
                  loading_chunks.insert(pos);
                }
              }
            }
          }
        }
      }
    }
    
    const double max_distance = std::sqrt((maxrange)*(maxrange)*2);
    
    for (Tile* tile : tiles) {
      ivec3 pos = tile->pos;
      int range = maxrange;
      if (((pos.x < px-range or pos.x > px+range or pos.y < py-range or pos.y > py+range or pos.z < pz-range or pos.z > pz+range)
      or world_closing) and deleting_chunks.count(pos) == 0) {
        tileloader->request_del_tile(pos);
        deleting_chunks.insert(pos);
      }
    }
    
    static bool initial_generation = true;
    if (loading_chunks.size() == 0 and initial_generation) {
      cout << "Initial generation finished: " << getTime() - start_time << 's' << endl;
      initial_generation = false;
    }
  }
}

void World::add_tile(Tile* tile) {
  logger->log(4) << "Adding tile " << tile << ' ' << tile->pos << endl;
  tiles.add_tile(tile);
  loading_chunks.erase(tile->pos);
}

void World::timestep() {
  static double last_time = getTime();
  double now = getTime();
  double dt = now - last_time;
  last_time = now;
  
  ivec3 chunk(player->position/float(chunksize) - vec3(player->position.x<0, player->position.y<0, player->position.z<0));
  if (tileat(chunk) != nullptr or player->spectator) {
    player->timestep();
    player->computeMatricesFromInputs();
  }
  
  static double total_time = 0;
  static int num_times = 0;
  double start = getTime();
  for (Tile* tile : tiles) {
    tile->timestep(dt);
  }
  total_time += getTime() - start;
  num_times ++;
  
  // cout << total_time / num_times << ' ' << num_times << endl;
  
  daytime += dt;
  if (daytime > 2000) {
    daytime -= 2000;
  }
  if (daytime < 0) {
    daytime += 2000;
  }
  
}

void World::tick() {
  
}

void World::drop_ticks() {
  player->drop_ticks();
  for (Tile* tile : tiles) {
    tile->drop_ticks();
  }
}

vec3 World::get_position() const {
  return vec3(0,0,0);
}

void World::block_update(ivec3 pos) {
  // block_updates.emplace(pos);
}

bool World::render() {
  bool changed = false;
  double start = getTime();
  ivec3 ppos(player->position);
  ppos = ppos / chunksize - ivec3(ppos.x < 0, ppos.y < 0, ppos.z < 0);
  Tile* playertile = tileat(ppos);
  
  
  if (playertile != nullptr and playertile->chunk->flags & RENDER_FLAG and !playertile->lightflag) {
    playertile->render(graphics->blockvecs(), graphics->transvecs());
    changed = true;
  } else {
    for (Tile* tile : tiles) {
      changed = changed or (tile->chunk->flags & RENDER_FLAG and tile->fully_loaded);
      if (changed) {
        tile->render(graphics->blockvecs(), graphics->transvecs());
        break;
      }
    }
    if (!changed) {
      for (Tile* tile : tiles) {
        if (!tile->lightflag) {
          changed = changed or (tile->chunk->flags & RENDER_FLAG);
          tile->render(graphics->blockvecs(), graphics->transvecs());
          if (changed) {
            break;
          }
        }
      }
    }
  }
  
  return changed;
}

void World::update_lighting() {
  for (Tile* tile : tiles) {
    tile->update_lighting();
  }
}

Tile* World::tileat(ivec3 pos) {
  return tiles.tileat(pos);
}

Tile* World::tileat_global(ivec3 pos) {
  return tileat(SAFEDIV(pos, chunksize));
}

Block* World::get_global(int x, int y, int z, int scale) {
  return get_global(ivec3(x,y,z), scale);
}

Block* World::get_global(ivec3 pos, int scale) {
  ivec3 tpos = SAFEDIV(pos, chunksize);
  Tile* tile = tileat(tpos);
  if (tile != nullptr) {
    Block* result = tile->chunk->get_global(pos, scale);
    return result;
  }
  return nullptr;
}

void World::set_global(ivec3 pos, int w, Blocktype val, int direction, int joints[6]) {
  Tile* tile = tileat_global(pos);
  if (tile != nullptr) {
    tile->chunk->set_global(pos, w, val, direction, joints);
  }
}

Blocktype World::get(ivec3 pos) {
  Block* block = get_global(pos, 1);
  if (block == nullptr) {
    return -1;
  }
  return block->pixel->value;
}

void World::set(ivec3 pos, Blocktype val, int direction) {
  set_global(pos, 1, val, direction);
}

Block* World::raycast(vec3* pos, vec3 dir, double time) {
  Block* b = get_global((int)pos->x - (pos->x<0), (int)pos->y - (pos->y<0), (int)pos->z - (pos->z<0), 1);
  if (b == nullptr) {
      return b;
  }
  return b->raycast(pos, dir, time);
}

void World::del_tile(ivec3 pos, bool remove_faces) {
  logger->log(4) << "del tile " << pos << endl;
  Tile* tile = tiles.del_tile(pos);
  
  if (tile != nullptr) {
    tile->deleting = true;
    if (saving) {
      tile->save();
    }
    delete tile;
    
    deleting_chunks.erase(pos);
  }
}

bool World::is_world_closed() {
  return world_closing and tiles.size() == 0 and deleting_chunks.size() == 0;
}

void World::close_world() {
  tileloader->end_serving();
  
  ofstream ofile(path("player.txt"));
  player->save_to_file(ofile);
  delete player;
  ofile.close();
  
  cout << "all tiles saved sucessfully: " << tiles.size() << endl;
  
  for (Tile* tile : tiles) {
    tile->deleting = true;
    if (saving) {
      tile->save();
    }
    delete tile;
  }
  
  ofstream datafile(path("worlddata.txt"));
  save_config(datafile);
  ofstream ofile2(SAVES_PATH "latest.txt");
  ofile2 << name;
}


#endif
