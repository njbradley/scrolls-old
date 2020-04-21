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
#include <GL/glew.h>
#include <map>
using std::thread;
#include "terrain-predef.h"

#define csize 2



World::World(string newname, int newseed): seed(newseed), name(newname) {
    setup_files();
    startup();
}

World::World(string oldname): name(oldname) {
    unzip();
    load_data_file();
    startup();
}

void World::unzip() {
  string command = "cd saves & unzip " + name + ".scrollsworld.zip";
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
  player = new Player( vec3(0,64,0), world);
  player->flying = false;
  player->autojump = true;
  player->health = 10;
}

void World::load_nearby_chunks() {
  
  int px = player->position.x/chunksize - (player->position.x<0);
  int py = player->position.y/chunksize - (player->position.y<0);
  int pz = player->position.z/chunksize - (player->position.z<0);
  //px = 0;
  py = 1;
  //pz = 0;
  int range = 1;
  for (int x = px-range; x < px+range+1; x ++) {
    for (int y = py-range; y < py+range+1; y ++) {
      for (int z = pz-range; z < pz+range+1; z ++) {
        if (!tiles.count(ivec3(x,y,z))) {
          load_chunk(ivec3(x,y,z));
        }
      }
    }
  }
  for (pair<ivec3,Tile*> kv : tiles) {
    double distance = std::sqrt((px-kv.first.x)*(px-kv.first.x) + (py-kv.first.y)*(py-kv.first.y) + (pz-kv.first.z)*(pz-kv.first.z));
    if (distance > 6) {
      del_chunk(kv.first);
      render_flag = true;
    }
  }
}




void World::render() {
    if (lighting_flag) {
      //update_lighting();
      lighting_flag = false;
    }
    bool changed = false;
    //cout << "back in render" << endl;
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
        tiles[kvpair.first]->render(&glvecs);
        //cout << "rendered chunk " << kvpair.first.first << ' ' << kvpair.first.second << endl;
        //render_chunk_vectors(kvpair.first);
        //cout << during - before << ' ' << after - during << ' ' << clock()-after << endl;
    }
    if (changed) {
			cout << "changed" << endl;
      world->glvecs.clean();
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
    if (tiles.find(pos) == tiles.end()) {
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

void World::set(char val, int x, int y, int z) {
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
      char val = newblock->get();
      newblock->subdivide();
      delete newblock;
      newblock = get_global((int)x, (int)y, (int)z, 1);
    }
    while (newblock->scale < minscale) {
        Chunk* parent = newblock->parent;
        parent->del();
        Pixel* newpix = new Pixel(parent->px, parent->py, parent->pz, 0, parent->scale, parent->parent, newblock->get_pix()->tile);
        parent->parent->blocks[parent->px][parent->py][parent->pz] = newpix;
        delete parent;
        newblock = get_global((int)x, (int)y, (int)z, 1);
    }
    newblock->set(val);
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
    return b->raycast(x, y, z, dx, dy, dz, time);
}

void World::save_chunk(ivec3 pos) {
  tiles[pos]->save();
}

void World::del_chunk(ivec3 pos) {
    save_chunk(pos);
    tiles[pos]->del();
    delete tiles[pos];
    tiles.erase(pos);
}

void World::load_chunk(ivec3 pos) {
    tiles[pos] = new Tile(pos, this);
}

void World::zip() {
  string command = "cd saves & zip -r -m " + name + ".scrollsworld.zip world";
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
        del_chunk(pos);
    }
    save_data_file();
    zip();
    ofstream ofile2("saves/latest.txt");
    ofile2 << name;
}

#endif
