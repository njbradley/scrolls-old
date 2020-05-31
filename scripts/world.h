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
#include "blocks-predef.h"

#include "world-predef.h"
#include "tiles-predef.h"
#include "multithreading-predef.h"
#include <GL/glew.h>
#include <map>
using std::thread;
#include "terrain-predef.h"

#define csize 2



World::World(string newname, int newseed): loader(seed), seed(newseed), name(newname) {
    setup_files();
    startup();
}

World::World(string oldname): loader(seed), name(oldname) {
    unzip();
    load_data_file();
    loader.seed = seed;
    startup();
}

void World::unzip() {
  ifstream ifile("saves/world/worlddata.txt");
  if (ifile.good()) {
    return;
  }
  string command = "cd saves & unzip " + name + ".scrollsworld.zip > NUL";
  system(command.c_str());
}

void World::load_data_file() {
    string path = "saves/world/worlddata.txt";
    ifstream ifile(path);
    string buff;
    getline(ifile, buff);
    while ( !ifile.eof() ) {
        getline(ifile,buff,':');
        if (buff == "seed") {
            ifile >> seed;
        }
        if (buff == "view_dist") {
            ifile >> view_dist;
        }
        getline(ifile, buff);
    }
}

void World::save_data_file() {
    string path = "saves/world/worlddata.txt";
    ofstream ofile(path);
    ofile << "Scrolls data file of world '" + name + "'\n";
    ofile << "seed:" << seed << endl;
    ofile << "view_dist:" << view_dist << endl;
    ofile.close();
}

void World::setup_files() {
    create_dir("saves/world");
    ofstream ofile("saves/saves.txt", ios::app);
    ofile << ' ' << name;
}

void World::startup() {
    //load_image();
    //generative_setup_world(seed);
    ifstream ifile("saves/world/player.txt");
    if (ifile.good()) {
      player = new Player(ifile);
    } else {
      spawn_player();
    }
    lighting_flag = true;
    load_nearby_chunks();
}

void World::spawn_player() {
  player = new Player( vec3(10,loader.terrain->get_height(ivec2(10,10))+7,10), world);
  player->flying = false;
  player->autojump = true;
  player->health = 10;
}

void World::load_nearby_chunks() {
  if (!player->flying) {
    int px = player->position.x/chunksize - (player->position.x<0);
    int py = player->position.y/chunksize - (player->position.y<0);
    int pz = player->position.z/chunksize - (player->position.z<0);
    //px = 0;
    //pz = 0;
    const int maxrange = 2;
    // ivec3 pos(px,py,pz);
    // if (!tiles.count(pos) and std::find(loading_chunks.begin(), loading_chunks.end(), pos) == loading_chunks.end()) {
    //   //load_chunk(ivec3(x,y,z));
    //   if (threadmanager->add_loading_job(pos)) {
    //     loading_chunks.push_back(pos);
    //   }
    // }
    //
    for (int range = 0; range < maxrange; range ++) {
      for (int x = px-range; x < px+range+1; x ++) {
        for (int y = py-range; y < py+range+1; y ++) {
          for (int z = pz-range; z < pz+range+1; z ++) {
            if (x == px-range or x == px+range or y == py-range or y == py+range or z == pz-range or z == pz+range) {
              //cout << range << ' ' << x << ' ' << y << ' ' << z << endl;
              ivec3 pos(x,y,z);
              if (!tiles.count(pos) and std::find(loading_chunks.begin(), loading_chunks.end(), pos) == loading_chunks.end()) {
                //load_chunk(ivec3(x,y,z));
                if (threadmanager->add_loading_job(pos)) {
                  loading_chunks.push_back(pos);
                }
              }
            }
          }
        }
      }
    }
    //crash(17658654574687);
    const double max_distance = std::sqrt((maxrange/2+1)*(maxrange/2+1)*2);
    for (pair<ivec3,Tile*> kv : tiles) {
      double distance = std::sqrt((px-kv.first.x)*(px-kv.first.x) + (py-kv.first.y)*(py-kv.first.y) + (pz-kv.first.z)*(pz-kv.first.z));
      if (distance > max_distance and std::find(deleting_chunks.begin(), deleting_chunks.end(), kv.first) == deleting_chunks.end()) {
        //del_chunk(kv.first, true);
        if (threadmanager->add_deleting_job(kv.first)) {
          deleting_chunks.push_back(kv.first);
          cout << deleting_chunks.size() << "-------------------" << endl;
        }
        render_flag = true;
      }
    }
  }
  get_async_loaded_chunks();
}

void World::get_async_loaded_chunks() {
  vector<Tile*> loaded_chunks = threadmanager->get_loaded_tiles();
  for (Tile* tile : loaded_chunks) {
    ivec3 pos = tile->pos;
    tiles[pos] = tile;
    for (int i = 0; i < loading_chunks.size(); i ++) {
      if (loading_chunks[i] == pos) {
        loading_chunks.erase(loading_chunks.begin()+i);
        break;
      }
    }
  }
  for (int i = deleting_chunks.size()-1; i >= 0; i --) {
    if (tiles.find(deleting_chunks[i]) == tiles.end()) {
      deleting_chunks.erase(deleting_chunks.begin()+i);
    }
  }
}

void World::timestep() {
  for (pair<ivec3, Tile*> kvpair : tiles) {
    tiles[kvpair.first]->timestep();
  }
}

void World::tick() {
  //cout << "world tick" << endl;
  //if (writelock.try_lock_for(std::chrono::seconds(1))) {
    for (ivec3 pos : block_updates) {
      //cout << pos.x << endl;
      Block* block = get_global(pos.x, pos.y, pos.z, 1);
      if (block != nullptr and block->get_pix() != nullptr) {
        block->get_pix()->tick();
      }
    }
    block_updates.clear();
    for (int i = 0; i < 3; i ++) {
      std::map<ivec3,Tile*>::iterator item = tiles.begin();
      if (tiles.size() > 0) {
        //cout << 827349 << endl;
        std::advance( item, rand()%tiles.size() );
        Tile* tile = item->second;
        //cout << "start of tick for tile ";
        //print (tile->pos);
        if (std::find(deleting_chunks.begin(), deleting_chunks.end(), item->first) == deleting_chunks.end()) {
          if (tile->writelock.try_lock_for(std::chrono::seconds(1))) {
            Block* block = tile->chunk->get_global(rand()%chunksize, rand()%chunksize, rand()%chunksize, 1);
            if (block != nullptr and block->get_pix() != nullptr) {
              block->get_pix()->tick();
            }
            tile->writelock.unlock();
          }
        }
      }
    }
  //  writelock.unlock();
  //}
  //cout << "end world tick" << endl;
}

vec3 World::get_position() {
  return vec3(0,0,0);
}

void World::block_update(int x, int y, int z) {
  if (writelock.try_lock_for(std::chrono::seconds(1))) {
    block_updates.push_back(ivec3(x,y,z));
    writelock.unlock();
  }
}

void World::render() {
    if (lighting_flag) {
      //update_lighting();
      lighting_flag = false;
    }
    bool changed = false;
    //cout << "back in render" << endl;
    vector<ivec3> empty;
    for (pair<ivec3, Tile*> kvpair : tiles) {
        //double before = clock();
        //chunks[kvpair.first]->all([](Pixel* pix) {
        //    pix->lightlevel = -1;
        //});
        //double during = clock();
        //chunks[kvpair.first]->calculate_light_level();
        //double after = clock();
        //chunks[kvpair.first]->calculate_light_level();
        changed = changed or tiles[kvpair.first]->chunk->render_flag;
        if (tiles[kvpair.first]->chunk->render_flag) {
          cout << kvpair.first.x << ' ' << kvpair.first.y << ' ' << kvpair.first.z << endl;
        }
        if (tiles.find(kvpair.first) != tiles.end() and std::find(deleting_chunks.begin(), deleting_chunks.end(), kvpair.first) == deleting_chunks.end()) {
          kvpair.second->render(&glvecs);
          // if (tiles[kvpair.first] == nullptr) {
          //   empty.push_back(kvpair.first);
          // } else {
          //   tiles[kvpair.first]->render(&glvecs);
          // }
        }
        //cout << "rendered chunk " << kvpair.first.first << ' ' << kvpair.first.second << endl;
        //render_chunk_vectors(kvpair.first);
        //cout << during - before << ' ' << after - during << ' ' << clock()-after << endl;
    }
    for (ivec3 e : empty) {
      tiles.erase(e);
    }
    if (changed) {
			cout << "changed" << endl;
      world->glvecs.clean();
      if (glvecs.writelock.try_lock_for(std::chrono::seconds(1))) {
        glFinish();
        glvecs.writelock.unlock();
      }
    }
}

void World::update_lighting() {
  for (pair<ivec3,Tile*> kvpair : tiles) {
    kvpair.second->update_lighting();
  }
}

Block* World::get_global(int x, int y, int z, int scale) {
    //cout << "world::get global(" << x << ' ' << y << ' ' << z << ' ' << scale << endl;
    int px = x/chunksize;
    int py = y/chunksize;
    int pz = z/chunksize;
    if (x < 0 and -x%chunksize != 0) {
        px --;
    } if (y < 0 and -y%chunksize != 0) {
        py --;
    } if (z < 0 and -z%chunksize != 0) {
        pz --;
    }
    ivec3 pos(px, py, pz);
    //cout << "pair<" << pos.first << ' ' << pos.second << endl;
    if (tiles.find(pos) == tiles.end() or std::find(deleting_chunks.begin(), deleting_chunks.end(), pos) != deleting_chunks.end()) {
        //cout << "   returning null\n";
        return nullptr;
    }
    //cout << endl;
    int ox = ((x < 0) ? chunksize : 0) + x%chunksize;
    int oy = ((y < 0) ? chunksize : 0) + y%chunksize;
    int oz = ((z < 0) ? chunksize : 0) + z%chunksize;
    //cout << ox << ' ' << y << ' ' << oz << endl;
    return tiles[pos]->chunk->get_global(ox, oy, oz, scale);
}

void World::set(char val, int x, int y, int z, int direction) {
  int before = clock();
    Block* newblock = get_global(x,y,z, 1);
    int minscale = 1;
    if (newblock == nullptr) {
        return;
    }
    if (newblock->get() != 0) {
        minscale = blocks->blocks[newblock->get()]->minscale;
    }
    if (val != 0) {
        int place_block_minscale = blocks->blocks[val]->minscale;
        if (place_block_minscale > minscale) {
            minscale = place_block_minscale;
        }
    }
    while (newblock->scale > minscale) {
      cout << newblock->scale << endl;
      char val = newblock->get();
      if (newblock->parent == nullptr) {
        ivec3 pos(newblock->px, newblock->py, newblock->pz);
        Block* result = newblock->get_pix()->subdivide([=] (ivec3 pos) {return newblock->get_pix()->value;});
        cout << "ksdjflakjsd" << result->px << ' ' << result->py << ' ' << result->pz << endl;
        tiles[pos]->chunk = result;
      } else {
        newblock->get_pix()->subdivide([=] (ivec3 pos) {return newblock->get_pix()->value;});
      }
      delete newblock;
      newblock = get_global((int)x, (int)y, (int)z, 1);
    }
    while (newblock->scale < minscale) {
        Chunk* parent = newblock->parent;
        parent->del(true);
        Pixel* newpix = new Pixel(parent->px, parent->py, parent->pz, 0, parent->scale, parent->parent, newblock->get_pix()->tile);
        parent->parent->blocks[parent->px][parent->py][parent->pz] = newpix;
        delete parent;
        newblock = get_global((int)x, (int)y, (int)z, 1);
    }
    newblock->set(val);
    if (val != 0 and blocks->blocks[val]->rotation_enabled) {
      newblock->get_pix()->direction = direction;
    }
    cout << "done with setting! time: " << clock() - before << endl;
}

char World::get(int x, int y, int z) {
    return get_global(x, y, z, 1)->get();
}

Block* World::raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time) {
    //cout << "world raycast" << *x << ' ' << *y << ' ' << *z << ' ' << dx << ' ' << dy << ' ' << dz << endl;
    Block* b = get_global((int)*x - (*x<0), (int)*y - (*y<0), (int)*z - (*z<0), 1);
    //cout << b << endl;
    if (b == nullptr) {
        return b;
    }
    return b->raycast(this, x, y, z, dx, dy, dz, time);
}

void World::save_chunk(ivec3 pos) {
  //cout << "saving chunk ..." << endl;
  tiles[pos]->save();
  //cout << "done" << endl;
}

void World::del_chunk(ivec3 pos, bool remove_faces) {
    save_chunk(pos);
    tiles[pos]->del(remove_faces);
    delete tiles[pos];
    tiles.erase(pos);
}

void World::load_chunk(ivec3 pos) {
    //cout << "loading chunk ..." << endl;
    tiles[pos] = new Tile(pos, this);
    //cout << "done" << endl;
}

void World::zip() {
  string command = "cd saves & zip -r -m " + name + ".scrollsworld.zip world > NUL";
  //remove(("saves/" + name + ".scrollsworld.zip").c_str());
  system(command.c_str());
}

void World::close_world() {
    ofstream ofile("saves/world/player.txt");
    player->save_to_file(ofile);
    delete player;
    ofile.close();
    vector<ivec3> poses;
    for (pair<ivec3, Tile*> kvpair : tiles) {
        poses.push_back(kvpair.first);
    }
    for (ivec3 pos : poses) {
        del_chunk(pos, false);
    }
    save_data_file();
    zip();
    ofstream ofile2("saves/latest.txt");
    ofile2 << name;
}

#endif
