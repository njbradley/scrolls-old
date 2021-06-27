#ifndef WORLD
#define WORLD

#include <iostream>
#include <functional>
#include <cmath>
#include <fstream>
#include <sstream>
#include <thread>
#include "cross-platform.h"
#include "rendervec.h"
#include "blockdata.h"
#include "blocks.h"
#include "world.h"
#include "tiles.h"
#include "multithreading.h"
#include "entity.h"
#include "blockgroups.h"
#include "commands.h"
#include "game.h"

#include <algorithm>
#include <GL/glew.h>
#include <map>
using std::thread;
#include "terrain.h"





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

int TileMap::size() {
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


World::World(string newname, int newseed): loader(seed), seed(newseed), name(newname),
commandprogram(this,&cout,&cout), tiles( ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * ((view_dist-1)*2+1) - 1) {
    setup_files();
    startup();
}

World::World(string oldname): loader(seed), name(oldname),
commandprogram(this,&cout,&cout), tiles( ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * ((view_dist-1)*2+1) - 1) {
    string path = "saves/" + name + "/worlddata.txt";
    ifstream ifile(path);
    load_data_file(ifile);
    loader.seed = seed;
    startup();
}

World::World(string newname, istream& datafile): loader(seed), name(newname),
commandprogram(this,&cout,&cout), tiles( ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * ((view_dist-1)*2+1) - 1) {
  load_data_file(datafile);
}

void World::set_buffers(GLuint verts, GLuint datas, int start_size) {
  vecs_dest.set_buffers(verts, datas, start_size);
  glvecs.set_destination(&vecs_dest);
  transparent_glvecs.set_destination_offset(&vecs_dest, start_size - start_size * 0.1);
}

void World::load_data_file(istream& ifile) {
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

void World::save_data_file(ostream& ofile) {
    ofile << "Scrolls data file of world '" + name + "'\n";
    ofile << "seed:" << seed << endl;
    ofile << "difficulty:" << difficulty << endl;
    ofile << "daytime:" << daytime << endl;
    ofile << "generation:" << (generation ? "on" : "off") << endl;
    ofile << "saving:" << (saving ? "on" : "off") << endl;
    ofile << "end:" << endl;
}

void World::setup_files() {
    create_dir("saves/" + name);
    create_dir("saves/" + name + "/chunks");
    create_dir("saves/" + name + "/groups");
    ofstream ofile("saves/saves.txt", std::ios::app);
    ofile << ' ' << name;
}

void World::startup() {
    last_time = glfwGetTime();
    ifstream ifile("saves/" + name + "/player.txt");
    if (ifile.good()) {
      player = new Player(this, ifile);
      set_player_vars();
    } else {
      spawn_player();
    }
    lighting_flag = true;
    cout << "loading chunks from file" << endl;
    gen_start_time = glfwGetTime();
    
    // sun setup
    
    vector<string> files;
    get_files_folder("resources/textures/blocks/1", &files);
    for (int i = 0; i < files.size(); i ++) {
      if (files[i] == "sun.bmp") {
        suntexture = i;
      }
      if (files[i] == "moon.bmp") {
        moontexture = i;
      }
    }
}

void World::spawn_player() {
  ivec3 spawnpos(10,loader.get_height(ivec2(10,10))+4,10);
  int i = 0;
  while (loader.gen_func(ivec4(spawnpos.x, spawnpos.y, spawnpos.z, 1)) != 0 and i < 100) {
    i ++;
    spawnpos = ivec3(spawnpos.x+1,loader.get_height(ivec2(10,10))+4,10);
  }
  player = new Player(this, vec3(spawnpos));
  player->spectator = true;
  player->autojump = true;
  player->health = 10;
  player->max_health = 10;
  set_player_vars();
}

void World::set_player_vars() {
  CommandVar<vec3> posvar(&player->position);
  CommandVar<vec3> velvar(&player->vel);
  commandprogram.vec3vars["me.pos"] = posvar;
  commandprogram.vec3vars["me.vel"] = velvar;
  commandprogram.doublevars["me.health"] = CommandVar<double>(&player->health);
  
  if (game != nullptr) {
    audio->listener = player;
  }
}

void World::load_nearby_chunks() {
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
        for (int y = py-range; y < py+range+1; y ++) {
        //for (int y = py+range; y >= py-range; y --) {
          for (int x = px-range; x < px+range+1; x ++) {
            for (int z = pz-range; z < pz+range+1; z ++) {
              if (x == px-range or x == px+range or y == py-range or y == py+range or z == pz-range or z == pz+range) {
                ivec3 pos(x,y,z);
                if (tileat(pos) == nullptr and num_chunks < max_chunks and std::find(loading_chunks.begin(), loading_chunks.end(), pos) == loading_chunks.end()) {
                  loaded_chunks = true;
                  num_chunks ++;
                  if (threadmanager->add_loading_job(pos)) {
                    loading_chunks.push_back(pos);
                  }
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
      or world_closing) and std::find(deleting_chunks.begin(), deleting_chunks.end(), pos) == deleting_chunks.end()) {
        if (threadmanager->add_deleting_job(pos)) {
          deleting_chunks.push_back(pos);
        }
      }
    }
    
    if (!loaded_chunks and threadmanager->load_queue.empty() and initial_generation) {
      cout << "Initial generation finished: " << glfwGetTime() - gen_start_time << 's' << endl;
      initial_generation = false;
    }
  }
}

void World::add_tile(Tile* tile) {
  tiles.add_tile(tile);
  
  loadinglock.lock();
  for (int i = 0; i < loading_chunks.size(); i ++) {
    if (loading_chunks[i] == tile->pos) {
      loading_chunks.erase(loading_chunks.begin()+i);
      break;
    }
  }
  loadinglock.unlock();
}

void World::timestep() {
  double now = glfwGetTime();
  double dt = now - last_time;
  last_time = now;
  
  ivec3 chunk(player->position/float(chunksize) - vec3(player->position.x<0, player->position.y<0, player->position.z<0));
  if (tileat(chunk) != nullptr or player->spectator) {
    player->timestep();
  }
  
  daytime += dt;
  if (daytime > 2000) {
    daytime -= 2000;
  }
  if (daytime < 0) {
    daytime += 2000;
  }
  
  const double min_sun = 0.5;
  
  const int sunrise = 450;
  const int sunset = 1550;
  const int riseduration = 100;
  
  float sunlevel;
  sunlevel = min_sun;
  if (daytime > sunrise and daytime < sunrise + riseduration) {
    sunlevel = float(daytime-sunrise)/riseduration * (1-min_sun) + min_sun;
  } else if (daytime >= sunrise + riseduration and daytime <= sunset - riseduration) {
    sunlevel = 1;
  } else if (daytime > sunset - riseduration and daytime < sunset) {
    sunlevel = -float(daytime-sunset+riseduration)/riseduration * (1-min_sun) + 1;
  }
  
  vec3 sundirection = vec3(sin(daytime*3.14/1000), -cos(daytime*3.14/1000), 0);
  
  RenderData data;
  data.pos.loc.pos = player->position + sundirection * 1000.0f;
  data.pos.loc.rot = quat(cos(daytime*3.14/1000/2), 0, 0, sin(daytime*3.14/1000/2));
  data.pos.loc.scale = 64;
  
  for (int i = 0; i < 6; i ++) {
    data.type.faces[i] = {(uint)suntexture, 0, 0, 20, 20};
  }
  
  if (sunindex.isnull()) {
    sunindex = glvecs.add(data);
  } else {
    glvecs.edit(sunindex, data);
  }
  
  data.pos.loc.pos = player->position - sundirection * 1000.0f;
  data.pos.loc.rot = quat(cos(daytime*3.14/1000/2 + 3.14), 0, 0, sin(daytime*3.14/1000/2 + 3.14));
  data.pos.loc.scale = 64;
  
  for (int i = 0; i < 6; i ++) {
    data.type.faces[i] = {(uint)moontexture, 0, 0, 20, 20};
  }
  
  if (moonindex.isnull()) {
    moonindex = glvecs.add(data);
  } else {
    glvecs.edit(moonindex, data);
  }
  
  sunlight = sundirection;
  
  float eveningTime = (daytime < 1000 ? daytime - sunrise : sunset - daytime) / riseduration;
  vec3 skycolor;
  if (eveningTime > 1) {
    skycolor = vec3(0.4f, 0.7f, 1.0f);
    suncolor = vec3(1,1,1);
  } else if (eveningTime < 0) {
    skycolor = vec3(0, 0, 0);
    suncolor = vec3(0.6,0.8,1);
  } else {
    float lateEvening = std::min(eveningTime, 0.2f) * 5;
    skycolor = vec3(1 - eveningTime*0.6f, 0.2f + eveningTime*0.5f, eveningTime) * lateEvening;
    suncolor = vec3(1, 0.2f + eveningTime*0.8f, eveningTime) * lateEvening + vec3(0.6, 0.8, 1) * (1-lateEvening);
  }
  
  suncolor *= sunlevel;
  graphics->clearcolor = skycolor * sunlevel;
  
  mobcount = 0;
  timestep_clock ++;
  int i = 0;
  
  for (Tile* tile : tiles) {
      tile->timestep();
      mobcount += tile->entities.size() + tile->block_entities.size();
    i ++;
  }
  
}

void World::tick() { return;
  unordered_set<ivec3,ivec3_hash> local_updates;
  local_updates.swap(block_updates);
  for (ivec3 pos : local_updates) {
    
    tileat_global(pos)->changelock.lock();
    Block* block = get_global(pos.x, pos.y, pos.z, 1);
    if (block != nullptr and !block->continues) {
      block->pixel->tick();
    }
    tileat_global(pos)->changelock.unlock();
  }
  
  for (int i = 0; i < aftertick_funcs.size(); i ++) {
    aftertick_funcs[i](this);
  }
  aftertick_funcs.clear();
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

void World::block_update(int x, int y, int z) {
  block_updates.emplace(ivec3(x,y,z));
}
void World::block_update(ivec3 pos) {
  block_updates.emplace(pos);
}

void World::aftertick(std::function<void(World*)> func) {
  aftertick_funcs.push_back(func);
}

void World::light_update(int x, int y, int z) {
  light_updates.emplace(ivec3(x,y,z));
}

bool World::render() {
    if (lighting_flag) {
      lighting_flag = false;
    }
    bool changed = false;
    double start = glfwGetTime();
    ivec3 ppos(player->position);
    ppos = ppos / chunksize - ivec3(ppos.x < 0, ppos.y < 0, ppos.z < 0);
    Tile* playertile = tileat(ppos);
    
    player->render(&glvecs);
    
    if (playertile != nullptr and playertile->chunk->render_flag and !playertile->lightflag) {
      playertile->render(&glvecs, &transparent_glvecs);
      changed = true;
    } else {
      for (Tile* tile : tiles) {
        changed = changed or (tile->chunk->render_flag and tile->fully_loaded);
        if (changed) {
          tile->render(&glvecs, &transparent_glvecs);
          break;
        }
      }
      if (!changed) {
        for (Tile* tile : tiles) {
          if (!tile->lightflag) {
            changed = changed or (tile->chunk->render_flag);
            tile->render(&glvecs, &transparent_glvecs);
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
  ivec3 pos (x,y,z);
  ivec3 tpos = SAFEDIV(pos, chunksize);
  Tile* tile = tileat(tpos);
  if (tile != nullptr) {
    Block* result = tile->chunk->get_global(pos, scale);
    return result;
  }
  return nullptr;
}

void World::summon(DisplayEntity* entity) {
  ivec3 pos(entity->position);
  pos = pos / chunksize - ivec3(
    int(pos.x<0 and -pos.x%chunksize!=0),
    int(pos.y<0 and -pos.y%chunksize!=0),
    int(pos.z<0 and -pos.z%chunksize!=0)
  );
  Tile* tile = tileat(pos);
  if (tile != nullptr) {
    tile->entities.push_back(entity);
  }
}

void World::set(ivec4 pos, char val, int direction, int joints[6]) {
  int px = pos.x/chunksize;
  int py = pos.y/chunksize;
  int pz = pos.z/chunksize;
  if (pos.x < 0 and -pos.x%chunksize != 0) {
      px --;
  } if (pos.y < 0 and -pos.y%chunksize != 0) {
      py --;
  } if (pos.z < 0 and -pos.z%chunksize != 0) {
      pz --;
  }
  Tile* tile = tileat(ivec3(px, py, pz));
  
  int lx = pos.x - px*chunksize;
  int ly = pos.y - py*chunksize;
  int lz = pos.z - pz*chunksize;
  
  tile->chunk->set_global(ivec3(pos.x, pos.y, pos.z), pos.w, val, direction, joints);
}

void World::set_global(ivec3 pos, int w, int val, int direction, int joints[6]) {
  ivec3 chunkpos = SAFEDIV(pos, chunksize);
  tileat(chunkpos)->chunk->set_global(pos, w, val, direction, joints);
}


void World::set(int x, int y, int z, char val, int direction, BlockExtra* extras) {
  set(ivec4(x,y,z,1), val, direction);
}

char World::get(int x, int y, int z) {
    Block* block = get_global(x, y, z, 1);
    if (block == nullptr) {
      return -1;
    }
    return block->pixel->value;
}

Block* World::raycast(vec3* pos, vec3 dir, double time) {
  Block* b = get_global((int)pos->x - (pos->x<0), (int)pos->y - (pos->y<0), (int)pos->z - (pos->z<0), 1);
  if (b == nullptr) {
      return b;
  }
  return b->raycast(pos, dir, time);
}

void World::save_chunk(ivec3 pos) {
  if (saving) {
    tileat(pos)->save();
  }
}

void World::del_chunk(ivec3 pos, bool remove_faces) {
  Tile* tile = tiles.del_tile(pos);
  
  if (tile != nullptr) {
    tile->deleting = true;
    delete tile;
    
    deletinglock.lock();
    for (int i = 0; i < deleting_chunks.size(); i ++) {
      if (deleting_chunks[i] == pos) {
        deleting_chunks.erase(deleting_chunks.begin()+i);
        break;
      }
    }
    deletinglock.unlock();
  }
}

bool World::is_world_closed() {
  return world_closing and tiles.size() == 0 and deleting_chunks.size() == 0;
}

void World::close_world() {
    ofstream ofile("saves/" + name + "/player.txt");
    player->save_to_file(ofile);
    delete player;
    ofile.close();
    vector<ivec3> poses;
    
    // for (Tile* tile : tiles) {
    //     poses.push_back(tile->pos);
    // }
    //
    // glvecs.ignore = true;
    // transparent_glvecs.ignore = true;
    // for (ivec3 pos : poses) {
    //     tileat(pos)->save();
    // }
    // for (ivec3 pos : poses) {
    //     del_chunk(pos, false);
    // }
    cout << "all tiles saved sucessfully: " << tiles.size() << endl;
    string path = "saves/" + name + "/worlddata.txt";
    ofstream datafile(path);
    save_data_file(datafile);
    ofstream ofile2("saves/latest.txt");
    ofile2 << name;
}

#endif
