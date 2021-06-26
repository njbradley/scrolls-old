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


Tile* TileLoop::iterator::operator*() {
  return iter->second;
}

TileLoop::iterator TileLoop::iterator::operator++() {
  ++iter;
  return *this;
}

bool operator!=(const TileLoop::iterator& iter1, const TileLoop::iterator& iter2) {
  return iter1.iter != iter2.iter;
}

TileLoop::TileLoop(World* nworld): world(nworld) {
  world->tilelock.lock();
  tiles = world->tiles;
  world->tilelock.unlock();
}

TileLoop::~TileLoop() {
  
}

TileLoop::iterator TileLoop::begin() {
  return TileLoop::iterator {tiles.begin()};
}

TileLoop::iterator TileLoop::end() {
  return TileLoop::iterator {tiles.end()};
}









World::World(string newname, int newseed): loader(seed), seed(newseed), name(newname), closing_world(false),
commandprogram(this,&cout,&cout) {
    setup_files();
    startup();
}

World::World(string oldname): loader(seed), name(oldname), closing_world(false),
commandprogram(this,&cout,&cout) {
    //unzip();
    string path = "saves/" + name + "/worlddata.txt";
    ifstream ifile(path);
    load_data_file(ifile);
    loader.seed = seed;
    startup();
}

World::World(string newname, istream& datafile): loader(seed), name(newname), closing_world(false),
commandprogram(this,&cout,&cout) {
  load_data_file(datafile);
}

void World::set_buffers(GLuint verts, GLuint datas, int start_size) {
  vecs_dest.set_buffers(verts, datas, start_size);
  glvecs.set_destination(&vecs_dest);
  transparent_glvecs.set_destination_offset(&vecs_dest, start_size - start_size * 0.1);
  // transparent_glvecs.set_buffers_prealloc(verts, uvs, light, mats, start_size*0.1, start_size - start_size*0.25);
  // glvecs.size_alloc = start_size - start_size*0.25;
}

void World::unzip() {
  ifstream ifile("saves/" + name + "/worlddata.txt");
  if (ifile.good()) {
    return;
  }
  string command = "cd saves & unzip " + name + ".scrollsworld.zip > NUL";
  system(command.c_str());
  cout << "uncompressed files from zip" << endl;
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
    //load_image();
    //generative_setup_world(seed);
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
    
    //load_nearby_chunks();
    load_groups();
}

void World::load_groups() {
  // vector<string> group_paths;
  // get_files_folder("saves/" + name + "/groups", &group_paths);
  // for (string path : group_paths) {
  //   string fullpath = "saves/" + name + "/groups/" + path;
  //   //cout << "loading group from " << fullpath << endl;
  //   ifstream ifile(fullpath);
  //   BlockGroup* group = BlockGroup::from_file(this, ifile);
  //   ifile.close();
  //   //remove(fullpath.c_str());
  //   group->set_pix_pointers();
  //   physicsgroups.emplace(group);
  // }
  // cout << "loaded " << group_paths.size() << " groups from file" << endl;
  // for (BlockGroup* group : physicsgroups) {
  //   group->link();
  // }
  // cout << "linked groups sucessfully" << endl;
}

void World::save_groups() {
  // vector<string> group_paths;
  // get_files_folder("saves/" + name + "/groups", &group_paths);
  // for (string path : group_paths) {
  //   string fullpath = "saves/" + name + "/groups/" + path;
  //   remove(fullpath.c_str());
  // }
  // for (BlockGroup* group : physicsgroups) {
  //   ivec3 pos = group->position;
  //   std::stringstream path;
  //   path << "saves/" + name + "/groups/" << pos.x << 'x' << pos.y << 'y' << pos.z << "z.txt";
  //   ofstream ofile(path.str());
  //   //cout << "saving group to " << path.str() << endl;
  //   group->to_file(ofile);
  //   group->block_poses.clear();
  //   delete group;
  // }
  // cout << "saved " << physicsgroups.size() << " groups to file" << endl;
  // physicsgroups.clear();
}

void World::spawn_player() {
  ivec3 spawnpos(10,loader.get_height(ivec2(10,10))+4,10);
  int i = 0;
  while (loader.gen_func(ivec4(spawnpos.x, spawnpos.y, spawnpos.z, 1)) != 0 and i < 100) {
    i ++;
    spawnpos = ivec3(spawnpos.x+1,loader.get_height(ivec2(10,10))+4,10);
  }
  // ivec3 spawnpos(10, 40, 32);
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
    
    if (!closing_world) {
      for (int range = 0; range < maxrange; range ++) {
        for (int y = py-range; y < py+range+1; y ++) {
        //for (int y = py+range; y >= py-range; y --) {
          for (int x = px-range; x < px+range+1; x ++) {
            for (int z = pz-range; z < pz+range+1; z ++) {
              if (x == px-range or x == px+range or y == py-range or y == py+range or z == pz-range or z == pz+range) {
                ivec3 pos(x,y,z);
                if (tileat(pos) == nullptr and num_chunks < max_chunks) {
                  //load_chunk(ivec3(x,y,z));
                  loaded_chunks = true;
                  num_chunks ++;
                  threadmanager->add_loading_job(pos);
                }
              }
            }
          }
        }
      }
    }
    //game->crash(17658654574687);
    const double max_distance = std::sqrt((maxrange)*(maxrange)*2);
    //if (tiles_lock.try_lock_shared_for(std::chrono::seconds(1))) {
    for (Tile* tile : TileLoop(this)) {
      ivec3 pos = tile->pos;
      int range = maxrange;
      if ((pos.x < px-range or pos.x > px+range or pos.y < py-range or pos.y > py+range or pos.z < pz-range or pos.z > pz+range)) {
      // double distance = std::sqrt((px-pos.x)*(px-pos.x) + (py-pos.y)*(py-pos.y) + (pz-pos.z)*(pz-pos.z));
      // if ((distance > max_distance or closing_world) and std::find(deleting_chunks.begin(), deleting_chunks.end(), pos) == deleting_chunks.end()) {
        //del_chunk(kv.first, true);
        threadmanager->add_deleting_job(pos);
      }
    }
    //  tiles_lock.unlock();
    //}
    if (!loaded_chunks and threadmanager->load_queue.empty() and initial_generation) {
      cout << "Initial generation finished: " << glfwGetTime() - gen_start_time << 's' << endl;
      initial_generation = false;
    }
  }
  //get_async_loaded_chunks();
  // dfile << "LOAD " << endl;
}

void World::get_async_loaded_chunks() {
  vector<Tile*> loaded_chunks = threadmanager->get_loaded_tiles();
  //if (tiles_lock.try_lock_for(std::chrono::seconds(1))) {
    for (Tile* tile : loaded_chunks) {
      add_tile(tile);
    }
    for (int i = deleting_chunks.size()-1; i >= 0; i --) {
      if (tileat(deleting_chunks[i]) == nullptr) {
        deleting_chunks.erase(deleting_chunks.begin()+i);
      }
    }
    //tiles_lock.unlock();
  //}
}

void World::add_tile(Tile* tile) {
  ivec3 pos = tile->pos;
  tilelock.lock();
  tiles[pos] = tile;
  tilelock.unlock();
}

void World::timestep() {
  double now = glfwGetTime();
  double dt = now - last_time;
  last_time = now;
  
  ivec3 chunk(player->position/float(chunksize) - vec3(player->position.x<0, player->position.y<0, player->position.z<0));
  if (tileat(chunk) != nullptr or player->spectator) {
    player->timestep();
  }
  //if (tiles_lock.try_lock_shared_for(std::chrono::seconds(1))) {
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
  //cout << daytime << ' ' << sunlight << endl;
  mobcount = 0;
  timestep_clock ++;
  int i = 0;
  
  for (Tile* tile : TileLoop(this)) {
      tile->timestep();
      mobcount += tile->entities.size() + tile->block_entities.size();
    i ++;
  }
  
    //cout << "total " << entities << endl;
  //  tiles_lock.unlock();
  //}
}

void World::tick() { return;
  unordered_set<ivec3,ivec3_hash> local_updates;
  local_updates.swap(block_updates);
  for (ivec3 pos : local_updates) {
    //cout << pos.x << endl;
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
  
  /*
  //cout << "world tick" << endl;
  //if (writelock.try_lock_for(std::chrono::seconds(1))) {
  
    //world->load_nearby_chunks();
    
    for (ivec3 pos : block_updates) {
      Pixel* pix = get_global(pos.x, pos.y, pos.z, 1)->get_pix();
      BlockGroup* group = pix->physicsgroup;
      if (group != nullptr) {
        if (group->persistant()) {
          group->update_flag = true;
        } else {
          delete group;
        }
      }
    }
    
    vector<BlockGroup*> dead_groups;
    for (BlockGroup* group : physicsgroups) {
      if (group->block_poses.size() > 0) {
        group->tick();
      } else {
        dead_groups.push_back(group);
      }
    }
    for (BlockGroup* group : dead_groups) {
      delete group;
      physicsgroups.erase(group);
      cout << "erased " << group << endl;
    }
    
    for (ivec3 pos : block_updates) {
      //cout << pos.x << endl;
      Block* block = get_global(pos.x, pos.y, pos.z, 1);
      if (block != nullptr and block->get_pix() != nullptr) {
        block->get_pix()->tick();
      }
    }
    block_updates.clear();
    
    // for (BlockGroup* group : physicsgroups) {
    //   group->remove_pix_pointers();
    //   delete group;
    // }
    //physicsgroups.clear();
    
    for (int i = 0; i < 5; i ++) {
      
      if (tiles.size() > 0) {
        //cout << 827349 << endl;
        Tile* tile = tiles[rand()%tiles.size()];
        //cout << "start of tick for tile ";
        //print (tile->pos);
        if (tile != nullptr) {
          if (std::find(deleting_chunks.begin(), deleting_chunks.end(), tile->pos) == deleting_chunks.end()) {
            //if (tile->writelock.try_lock_for(std::chrono::seconds(1))) {
              Block* block = tile->chunk->get_global(rand()%chunksize, rand()%chunksize, rand()%chunksize, 1); //ERR: tile is 337? maybe change to del_chunk
              if (block != nullptr and block->get_pix() != nullptr) {
                block->get_pix()->random_tick();
              }
              //tile->writelock.unlock();
            //}
          }
        }
      }
    }
  //  writelock.unlock();
  //}
  //cout << "end world tick" << endl;*/
}

void World::drop_ticks() {
  player->drop_ticks();
  for (Tile* tile : TileLoop(this)) {
    tile->drop_ticks();
  }
}

vec3 World::get_position() const {
  return vec3(0,0,0);
}

void World::block_update(int x, int y, int z) {
  if (x == 0 and y == 0 and z == 0) {
    //cout << "big prob at 296, block update called on 0,0,0, likely a bug, but it is possible if the block at 0,0,0 was actually updated" << endl;
  }
    //cout << "Recording Block Update at the global world position " << x << ' ' << y << ' ' << z << endl;
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
      //update_lighting();
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
      for (Tile* tile : TileLoop(this)) {
        changed = changed or (tile->chunk->render_flag and tile->fully_loaded);
        if (changed) {
          tile->render(&glvecs, &transparent_glvecs);
          break;
        }
      }
      if (!changed) {
        for (Tile* tile : TileLoop(this)) {
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
    
    // if (glvecs.clean_flag or transparent_glvecs.clean_flag or changed) {
    //   glvecs.clean();
    //   //cout << "changed" << endl;
    //   transparent_glvecs.clean();
    //   //if (glvecs.writelock.try_lock_for(std::chrono::seconds(1))) {
    //     //int before = clock();
    //     glFinish();
    //     //cout << "glfinish time " << clock() - before << endl;
    //     //glvecs.writelock.unlock();
    //   //}
    //   return true;
    // }
    //return false;
    return changed;
}

void World::update_lighting() {
  for (Tile* tile : TileLoop(this)) {
    tile->update_lighting();
  }
}

Tile* World::tileat(ivec3 pos) {
  tilelock.lock();
  unordered_map<ivec3, Tile*, ivec3_hash>::iterator iter = tiles.find(pos);
  if (iter != tiles.end()) {
    tilelock.unlock();
    return iter->second;
  }
  tilelock.unlock();
  return nullptr;
}

Tile* World::tileat_global(ivec3 pos) {
  return tileat(SAFEDIV(pos, chunksize));
}

Block* World::get_global(int x, int y, int z, int scale) {
    //cout << "world::get global(" << x << ' ' << y << ' ' << z << ' ' << scale << endl;
    //// dfile << "gg ";
    ivec3 pos (x,y,z);
    ivec3 tpos = SAFEDIV(pos, chunksize);
    Tile* tile = tileat(tpos);
    if (tile != nullptr) {
      Block* result = tile->chunk->get_global(pos, scale);
      //// dfile << "GG " << endl;
      return result;
    }
    //// dfile << "GG2 " << endl;
    // if (foundkv->second != nullptr and !foundkv->second->deleting) {
    //   return foundkv->second->chunk->get_global(ox, oy, oz, scale);
    // }
    return nullptr;
    //cout << ox << ' ' << y << ' ' << oz << endl;
    // try {
    //   Tile* tile = tiles.at(pos);
    //   if (tile != nullptr and !tile->deleting) {
    //     return tile->chunk->get_global(ox, oy, oz, scale);
    //   } else {
    //     return nullptr;
    //   }
    // } catch (std::out_of_range& ex) {
    //   cout << "world get_global caught " << ex.what() <<  " with pos " << pos.x << ' ' << pos.y << ' ' << pos.z << endl;
    //   return nullptr;
    // }
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
  //cout << "tile " << ivec3(px, py, pz) << endl;
  int lx = pos.x - px*chunksize;
  int ly = pos.y - py*chunksize;
  int lz = pos.z - pz*chunksize;
  cout << tile << ' ' << tile->chunk << endl;
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
    //cout << "world raycast" << *x << ' ' << *y << ' ' << *z << ' ' << dx << ' ' << dy << ' ' << dz << endl;
    Block* b = get_global((int)pos->x - (pos->x<0), (int)pos->y - (pos->y<0), (int)pos->z - (pos->z<0), 1);
    //cout << b << endl;
    if (b == nullptr) {
        return b;
    }
    return b->raycast(pos, dir, time);
}

void World::save_chunk(ivec3 pos) {
  //cout << "saving chunk ..." << endl;
  if (saving) {
    tileat(pos)->save();
  }
  //cout << "done" << endl;
}

void World::del_chunk(ivec3 pos, bool remove_faces) {
  tilelock.lock();
  unordered_map<ivec3, Tile*, ivec3_hash>::iterator iter = tiles.find(pos);
  if (iter != tiles.end()) {
    tilelock.unlock();
    return;
  }
  
  iter->second->deleting = true;
  delete iter->second;
  tilelock.unlock();
}

void World::zip() {
  //return;
  string command = "cd saves & zip -r -m " + name + ".scrollsworld.zip world > NUL";
  //remove(("saves/" + name + ".scrollsworld.zip").c_str());
  system(command.c_str());
  cout << "compressed world into zip file" << endl;
}

bool World::is_world_closed() {
  return closing_world and tiles.size() == 0 and deleting_chunks.size() == 0;
}

void World::close_world() {
    ofstream ofile("saves/" + name + "/player.txt");
    player->save_to_file(ofile);
    delete player;
    ofile.close();
    vector<ivec3> poses;
    //if (tiles_lock.try_lock_shared_for(std::chrono::seconds(1))) {
      for (Tile* tile : TileLoop(this)) {
          poses.push_back(tile->pos);
      }
    //  tiles_lock.unlock();
    //} else {
    //  hard_game->crash(923400304);
    //}
    glvecs.ignore = true;
    transparent_glvecs.ignore = true;
    for (ivec3 pos : poses) {
        tileat(pos)->save();
    }
    for (ivec3 pos : poses) {
        del_chunk(pos, false);
    }
    cout << "all tiles saved sucessfully" << endl;
    save_groups();
    string path = "saves/" + name + "/worlddata.txt";
    ofstream datafile(path);
    save_data_file(datafile);
    //zip();
    ofstream ofile2("saves/latest.txt");
    ofile2 << name;
}

#endif
