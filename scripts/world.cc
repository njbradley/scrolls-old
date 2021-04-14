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


TileLoop::iterator::iterator(TileLoop* newtileloop, int newiter): tileloop(newtileloop), index(newiter) {
  while (index < tileloop->world->tiles.size() and (tileloop->world->tiles[index] == nullptr or tileloop->world->tiles[index]->deleting)) {
    index ++;
  }
}

vector<Tile*>::iterator TileLoop::iterator::operator->() {
  return tileloop->world->tiles.begin() + index;
}

Tile* TileLoop::iterator::operator*() {
  return tileloop->world->tiles[index];
}

TileLoop::iterator TileLoop::iterator::operator++() {
  if (index < tileloop->world->tiles.size()) {
    index ++;
    while (index < tileloop->world->tiles.size() and (tileloop->world->tiles[index] == nullptr or tileloop->world->tiles[index]->deleting)) {
      index ++;
    }
  }
  return *this;
}

bool operator==(const TileLoop::iterator& iter1, const TileLoop::iterator& iter2) {
  return iter1.index == iter2.index and iter1.tileloop == iter2.tileloop;
}

bool operator!=(const TileLoop::iterator& iter1, const TileLoop::iterator& iter2) {
  return iter1.index != iter2.index or iter1.tileloop != iter2.tileloop;
}

TileLoop::TileLoop(World* nworld): world(nworld) {
  
}

TileLoop::iterator TileLoop::begin() {
  return iterator(this, 0);
}

TileLoop::iterator TileLoop::end() {
  return iterator(this, world->tiles.size());
}









World::World(string newname, int newseed): loader(seed), seed(newseed), name(newname), closing_world(false), sunlight(1),
commandprogram(this,&cout,&cout), tiles( ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * 2, nullptr) {
    setup_files();
    startup();
}

World::World(string oldname): loader(seed), name(oldname), closing_world(false), sunlight(1),
commandprogram(this,&cout,&cout), tiles( ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * 2, nullptr) {
    //unzip();
    string path = "saves/" + name + "/worlddata.txt";
    ifstream ifile(path);
    load_data_file(ifile);
    loader.seed = seed;
    startup();
}

World::World(string newname, istream& datafile): loader(seed), name(newname), closing_world(false), sunlight(1),
commandprogram(this,&cout,&cout), tiles( ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * ((view_dist-1)*2+1) * 2, nullptr) {
  load_data_file(datafile);
}

void World::set_buffers(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size) {
  vecs_dest.set_buffers(verts, uvs, light, mats, start_size);
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
  // dfile << "load ";
  if (!player->spectator) {
    
    bool loaded_chunks = false;
    
    int px = player->position.x/chunksize - (player->position.x<0);
    int py = player->position.y/chunksize - (player->position.y<0);
    int pz = player->position.z/chunksize - (player->position.z<0);
    //px = 0;
    //pz = 0;
    const int maxrange = view_dist;
    // ivec3 pos(px,py,pz);
    // if (!tiles.count(pos) and std::find(loading_chunks.begin(), loading_chunks.end(), pos) == loading_chunks.end()) {
    //   //load_chunk(ivec3(x,y,z));
    //   if (threadmanager->add_loading_job(pos)) {
    //     loading_chunks.push_back(pos);
    //   }
    // }
    //
    int max_chunks = ((view_dist)*2+1) * ((view_dist)*2+1) * ((view_dist)*2+1);
    int num_chunks = loading_chunks.size();
    for (Tile* tile : tiles) {
      num_chunks += (tile != nullptr);
    }
    
    if (!closing_world) {
      for (int range = 0; range < maxrange; range ++) {
        for (int y = py-range; y < py+range+1; y ++) {
        //for (int y = py+range; y >= py-range; y --) {
          for (int x = px-range; x < px+range+1; x ++) {
            for (int z = pz-range; z < pz+range+1; z ++) {
              if (x == px-range or x == px+range or y == py-range or y == py+range or z == pz-range or z == pz+range) {
                //cout << range << ' ' << x << ' ' << y << ' ' << z << endl;
                ivec3 pos(x,y,z);
                if (tileat(pos) == nullptr and num_chunks < max_chunks and std::find(loading_chunks.begin(), loading_chunks.end(), pos) == loading_chunks.end()) {
                  //load_chunk(ivec3(x,y,z));
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
    //game->crash(17658654574687);
    const double max_distance = std::sqrt((maxrange)*(maxrange)*2);
    //if (tiles_lock.try_lock_shared_for(std::chrono::seconds(1))) {
    TileLoop loop(this);
    for (Tile* tile : loop) {
      ivec3 pos = tile->pos;
      int range = maxrange;
      if ((pos.x < px-range or pos.x > px+range or pos.y < py-range or pos.y > py+range or pos.z < pz-range or pos.z > pz+range)
      and std::find(deleting_chunks.begin(), deleting_chunks.end(), pos) == deleting_chunks.end()) {
      // double distance = std::sqrt((px-pos.x)*(px-pos.x) + (py-pos.y)*(py-pos.y) + (pz-pos.z)*(pz-pos.z));
      // if ((distance > max_distance or closing_world) and std::find(deleting_chunks.begin(), deleting_chunks.end(), pos) == deleting_chunks.end()) {
        //del_chunk(kv.first, true);
        if (threadmanager->add_deleting_job(pos)) {
          deleting_chunks.push_back(pos);
        }
      }
    }
    //  tiles_lock.unlock();
    //}
    if (!loaded_chunks and threadmanager->load_queue.empty() and initial_generation) {
      cout << "Initial generation finished: " << glfwGetTime() - gen_start_time << 's' << endl;
      initial_generation = false;
    }
  }
  get_async_loaded_chunks();
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
  
  for (int i = 0; i < tiles.size(); i ++) {
    if (tiles[i] == nullptr) {
      // dfile << "load" << i << ' ';
      tiles[i] = tile;
      // dfile << "LOAD" << i << endl;
      break;
    }
  }
  
  for (int i = 0; i < loading_chunks.size(); i ++) {
    if (loading_chunks[i] == pos) {
      loading_chunks.erase(loading_chunks.begin()+i);
      break;
    }
  }
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
  
  const double min_sun = 0.1;
  
  sunlight = min_sun;
  if (daytime > 450 and daytime < 500) {
    sunlight = (daytime-450)/50.0f * (1-min_sun) + 0.05;
  } else if (daytime > 500 and daytime < 1500) {
    sunlight = 1;
  } else if (daytime > 1500 and daytime < 1550) {
    sunlight = -(daytime-1500)/50.0f * (1-min_sun) + 1;
  }
  //cout << daytime << ' ' << sunlight << endl;
  mobcount = 0;
  timestep_clock ++;
  int i = 0;
  TileLoop loop(this);
  
  for (Tile* tile : loop) {
      tile->timestep();
      mobcount += tile->entities.size() + tile->block_entities.size();
    i ++;
  }
  
    //cout << "total " << entities << endl;
  //  tiles_lock.unlock();
  //}
}

void World::tick() {
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
  TileLoop loop(this);
  for (Tile* tile : loop) {
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
      TileLoop loop(this);
      for (Tile* tile : loop) {
        changed = changed or (tile->chunk->render_flag and tile->fully_loaded);
        if (changed) {
          tile->render(&glvecs, &transparent_glvecs);
          break;
        }
      }
      if (!changed) {
        loop = TileLoop(this);
        for (Tile* tile : loop) {
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
    for (pair<int,int> render_index : dead_render_indexes) {
      glvecs.del(render_index);
      changed = true;
    }
    dead_render_indexes.clear();
    
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
  TileLoop loop(this);
  for (Tile* tile : loop) {
    tile->update_lighting();
  }
}

Tile* World::tileat(ivec3 pos) {
  for (Tile* tile : tiles) {
    if (tile != nullptr and tile->pos == pos and !tile->deleting) {
      return tile;
    }
  }
  return nullptr;
}

Tile* World::tileat_global(ivec3 pos) {
  ivec3 divpos = pos / chunksize;
  divpos -= ivec3(
    pos.x < 0 and -pos.x%chunksize != 0,
    pos.y < 0 and -pos.y%chunksize != 0,
    pos.z < 0 and -pos.z%chunksize != 0
  );
  return tileat(divpos);
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
  // if (tiles.find(pos) == tiles.end() or std::find(deleting_chunks.begin(), deleting_chunks.end(), pos) != deleting_chunks.end()) {
  //   return;
  // }
  // try {
  //   tiles.at(pos)->entities.push_back(entity);
  // } catch (std::out_of_range& ex) {
  //   cout << "world summon caught " << ex.what() << endl;
  // }
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
  tile->chunk->set_global(ivec3(lx, ly, lz), pos.w, val, direction, joints);
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

Block* World::raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time) {
    //cout << "world raycast" << *x << ' ' << *y << ' ' << *z << ' ' << dx << ' ' << dy << ' ' << dz << endl;
    Block* b = get_global((int)*x - (*x<0), (int)*y - (*y<0), (int)*z - (*z<0), 1);
    //cout << b << endl;
    if (b == nullptr) {
        return b;
    }
    return nullptr;//b->raycast(this, x, y, z, dx, dy, dz, time);
}

void World::save_chunk(ivec3 pos) {
  //cout << "saving chunk ..." << endl;
  if (saving) {
    tileat(pos)->save();
  }
  //cout << "done" << endl;
}

void World::del_chunk(ivec3 pos, bool remove_faces) {
    // Tile* tile = tiles[pos];
    // tiles.erase(pos);
    // tile->save();
    // tile->del(remove_faces);
    // delete tile;
    Tile* tile = tileat(pos);
    tile->deleting = true;
    for (int i = 0; i < tiles.size(); i ++) {
      if (tiles[i] != nullptr and tiles[i]->pos == pos) {
        // dfile << "del_chunk ";
        tiles[i] = nullptr;
        // dfile << "DEL_CHUNK " << endl;
        break;
      }
    }
    // if (saving) {
    //   tile->save();
    // }
    tile->del(remove_faces);
    delete tile;
    // save_chunk(pos);
    // tiles[pos]->del(remove_faces);
    // delete tiles[pos];
    // tiles.erase(pos);
}

void World::load_chunk(ivec3 pos) {
    //cout << "loading chunk ..." << endl;
    //cout << "creating tile " << this << endl;
    for (int i = 0; i < tiles.size(); i++) {
      if (tiles[i] == nullptr) {
        // dfile << "loadc ";
        tiles[i] = new Tile(pos, this);
        // dfile << "LOADC " << endl;
        break;
      }
    }
    //cout << "done" << endl;
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
    TileLoop loop(this);
      for (Tile* tile : loop) {
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
