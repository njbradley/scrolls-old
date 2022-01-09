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
#include "settings.h"

#include <map>


DEFINE_AND_EXPORT_PLUGIN(World);


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

const Tile* TileMap::tileat(ivec3 pos) const {
  Bucket* bucket = buckets + (ivec3_hash()(pos) % num_buckets);
  std::lock_guard guard(bucket->lock);
  const Item* item = bucket->items;
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


float World::tick_deltatime = 1/20.0f;


World::World(string oldname): seed(std::hash<string>()(name)), terrainloader(seed), tileloader(this), name(oldname), physics(this, tick_deltatime),
tiles( ((settings->view_dist-1)*2+1) * ((settings->view_dist-1)*2+1) * ((settings->view_dist-1)*2+1) + 3) {
  
}

World::~World() {
  close_world();
}

string World::path(string filename) const {
  return SAVES_PATH + name + '/' + filename;
}


void World::parse_config_line(string buff, istream& ifile) {
  if (buff == "seed") {
    ifile >> seed;
    terrainloader.set_seed(seed);
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
  if (buff == "last_player_pos") {
    ifile >> last_player_pos.x >> last_player_pos.y >> last_player_pos.z;
  }
}

void World::load_config(istream& ifile) {
  string buff;
  getline(ifile, buff);
  getline(ifile,buff,':');
  while ( !ifile.eof() and buff != "end" ) {
    parse_config_line(buff, ifile);
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
  ofile << "last_player_pos:" << last_player_pos.x << ' ' << last_player_pos.y << ' ' << last_player_pos.z << endl;
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
  ifstream ifile(path("worlddata.txt"));
  if (ifile.good()) {
    load_config(ifile);
  } else {
    setup_files();
  }
  
  tileloader->begin_serving();
  tickrunner.init(this, tick_deltatime);
  
  {
    ifstream ifile(path("player.txt"));
    if (ifile.good()) {
      // player = new Player(this, ifile);
      // set_player_vars();
    } else {
      last_player_pos = ivec3(10,terrainloader.get_height(ivec2(10,10))+4,10);
      // spawn_player();
    }
  }
}

void World::spawn_player() {
  ivec3 spawnpos(15,terrainloader.get_height(ivec2(15,15))+8,15);
  int i = 0;
  // while (loader.gen_func(ivec4(spawnpos.x, spawnpos.y, spawnpos.z, 1)) != 0 and i < 100) {
  //   i ++;
  //   spawnpos = ivec3(spawnpos.x+1,loader.get_height(ivec2(10,10))+4,10);
  // }
  player = new Player();
  Block* spawnblock = get_global(spawnpos, 8);
  while (spawnblock->scale > 8) {
    spawnblock->subdivide();
    spawnblock = spawnblock->get_global(spawnpos, 8);
  }
  spawnblock->add_freechild(player);
  player->set_position(spawnpos);
  // player->spectator = true;
  set_player_vars();
}

void World::set_player_vars() {
  audio->set_listener(&player->renderbox.position, &player->box.velocity, &player->angle);
  graphics->set_camera(&player->renderbox.position, &player->angle);
  viewbox->playerpos = &player->renderbox.position;
  player->controller = controls;
  spectator.controller = nullptr;
}

void World::set_spectator_vars() {
  // audio->set_listener(&player->renderbox.position, &player->box.velocity, &player->angle);
  graphics->set_camera(&spectator.position, &spectator.angle);
  viewbox->playerpos = &player->renderbox.position;
  spectator.controller = controls;
  player->controller = nullptr;
}

void World::load_nearby_chunks() {
  static double start_time = getTime();
  if (player == nullptr or !player->spectator) {
    
    bool loaded_chunks = false;
    
    if (player != nullptr) {
      last_player_pos = player->box.position;
    }
    
    int px = last_player_pos.x/chunksize - (last_player_pos.x<0);
    int py = last_player_pos.y/chunksize - (last_player_pos.y<0);
    int pz = last_player_pos.z/chunksize - (last_player_pos.z<0);
    
    const int maxrange = settings->view_dist;
    
    int max_chunks = ((settings->view_dist)*2+1) * ((settings->view_dist)*2+1) * ((settings->view_dist)*2+1);
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

void World::timestep(float curtime, float deltatime) {
  
  debuglines->clear("timestep");
  // ivec3 chunk(SAFEDIV(player->box.position, (float)chunksize);
  // //player->position/float(chunksize) - vec3(player->position.x<0, player->position.y<0, player->position.z<0));
  // if (player != nullptr and (tileat(chunk) != nullptr or player->spectator)) {
  //   player->timestep();
  //   player->computeMatricesFromInputs();
  // }
  
  for (Tile* tile : tiles) {
    if (player == nullptr) {
      for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
        player = dynamic_cast<Player*>(free);
        if (player != nullptr) {
          cout << "got player from world" << endl;
          set_player_vars();
          break;
        }
      }
    }
    tile->timestep(curtime, deltatime);
  }
  
  if (player == nullptr and tileat(SAFEDIV(last_player_pos, chunksize)) != nullptr) {
    spawn_player();
  }
  
  if (controls->key_pressed('F')) {
    spectator.position = player->box.position;
    spectator.angle = player->angle;
    set_spectator_vars();
  } else if (controls->key_pressed('G')) {
    set_player_vars();
  }
  
  spectator.timestep(curtime, deltatime);
  
  daytime += deltatime;
  if (daytime > 2000) {
    daytime -= 2000;
  }
  if (daytime < 0) {
    daytime += 2000;
  }
  
}

void World::tick(float curtime, float deltatime) {
  static bool n_pressed = false;
  static bool paused = true;
  
  if (controls != nullptr) {
    if (controls->key_pressed('M')) {
      paused = false;
    }
    if (controls->key_pressed('B')) {
      paused = true;
    }
    
    bool cur_n_pressed = controls->key_pressed('N');
    if (paused and (!cur_n_pressed or n_pressed)) {
      n_pressed = cur_n_pressed;
      return;
    }
    n_pressed = cur_n_pressed;
  }
  
  debuglines->clear("tick");
  
  for (Tile* tile : tiles) {
    tile->tick(curtime, deltatime);
  }
  physics->tick(curtime, deltatime);
}

vec3 World::get_position() const {
  return vec3(0,0,0);
}

void World::block_update(ivec3 pos) {
  // block_updates.emplace(pos);
}

bool World::render() {
  //
  // for (Tile* tile : tiles) {
  //   if (!tile->lightflag) {
  //     tile->render(graphics->blockvecs, graphics->transvecs);
  //   }
  // }
  //
  // return false;
  
  static bool initial_started = false;
  static bool initial_done = false;
  static double startTime;
  static int num_times_idle = 0;
  
  bool changed = false;
  double start = getTime();
  Tile* playertile = nullptr;
  if (player != nullptr) {
    playertile = tileat_global(player->box.position);
    // ivec3 ppos(player->box.position);
    // ppos = ppos / chunksize - ivec3(ppos.x < 0, ppos.y < 0, ppos.z < 0);
    // Tile* playertile = tileat(ppos);
    // cout << playertile->pos << ' ';
  }
  if (playertile != nullptr and playertile->chunk->flags & Block::RENDER_FLAG and !playertile->lightflag) {
    // cout << "playertile " << endl;
    playertile->render(graphics->blockvecs, graphics->transvecs);
    changed = true;
  } else {
    for (Tile* tile : tiles) {
      changed = changed or (tile->chunk->flags & Block::RENDER_FLAG and tile->fully_loaded);
      if (changed) {
        tile->render(graphics->blockvecs, graphics->transvecs);
        // cout << tile->pos << " fully loaded " << endl;
        break;
      }
    }
    if (!changed) {
      for (Tile* tile : tiles) {
        if (!tile->lightflag) {
          changed = changed or (tile->chunk->flags & Block::RENDER_FLAG);
          tile->render(graphics->blockvecs, graphics->transvecs);
          // cout << tile->pos << "not loaded " << endl;
          if (changed) {
            break;
          }
        }
      }
    }
  }
  
  if (changed) {
    num_times_idle = 0;
  } else {
    num_times_idle ++;
  }
  
  if (changed and !initial_started) {
    initial_started = true;
    startTime = getTime();
  }
  if (initial_started and !changed and !initial_done and num_times_idle > 20) {
    initial_done = true;
    cout << "initial rendering done in " << getTime() - startTime << endl;
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

Block* World::get_global(ivec3 pos, int scale) {
  ivec3 tpos = SAFEDIV(pos, chunksize);
  Tile* tile = tileat(tpos);
  if (tile != nullptr) {
    Block* result = tile->chunk->get_global(pos, scale);
    return result;
  }
  return nullptr;
}

const Block* World::get_global(ivec3 pos, int scale) const {
  ivec3 tpos = SAFEDIV(pos, chunksize);
  const Tile* tile = tiles.tileat(tpos);
  if (tile != nullptr) {
    const Block* result = tile->chunk->get_global(pos, scale);
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
  Block* b = get_global(SAFEFLOOR3(*pos), 1);
  // Block* b = get_global((int)pos->x - (pos->x<0), (int)pos->y - (pos->y<0), (int)pos->z - (pos->z<0), 1);
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
  tickrunner.close();
  
  if (saving) {
    ofstream ofile(path("player.txt"));
    ofile << ',' << endl;
    // player->save_to_file(ofile);
    ofile.close();
  }
  
  if (saving) {
    cout << "all tiles saved sucessfully: " << tiles.size() << endl;
  } else {
    cout << "no tiles saved due to user command: " << tiles.size() << endl;
  }
  
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
