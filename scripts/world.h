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
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
using namespace glm;
using std::thread;
#include "generative.h"

#define csize 2



World::World(string newname, int newseed): seed(newseed), name(newname) {
    setup_files();
    startup();
}

World::World(string oldname): name(oldname) {
    load_data_file();
    startup();
}

void World::load_data_file() {
    string path = "saves/" + name + "/worlddata.txt";
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
    string path = "saves/" + name + "/worlddata.txt";
    ofstream ofile(path);
    ofile << "Scrolls data file of world '" + name + "'\n";
    ofile << "seed:" << seed << endl;
    ofile << "view_dist:" << view_dist << endl;
    ofile.close();
}

void World::setup_files() {
    create_dir("saves/" + name);
    ofstream ofile("saves/saves.txt", ios::app);
    ofile << ' ' << name;
}

void World::startup() {
    //load_image();
    generative_setup_world(seed);
    ifstream ifile("saves/" + name + "/player.txt");
    if (ifile.good()) {
      player = new Player(ifile);
    } else {
      player = new Player( vec3(10,52,10), world);
    	player->flying = true;
    	player->autojump = true;
    	player->health = 10;
    }
    iter_gen_func = [&] (int x, int y, int z, Pixel* p) {
        iter_gen(x, y, z, p);
    };
    lighting_flag = true;
    load_nearby_chunks();
}

void World::load_nearby_chunks() {
  
  int px = player->position.x/chunksize - (player->position.x<0);
  int pz = player->position.z/chunksize - (player->position.z<0);
  int range = 0;
  for (int x = px-range; x < px+range+1; x ++) {
    for (int y = pz-range; y < pz+range+1; y ++) {
      if (!chunks.count(pair<int,int>(x,y))) {
        chunks[pair<int,int>(x,y)] = load_chunk(pair<int,int>(x,y));
        render_flag = true;
      }
    }
  }
  for (pair<pair<int,int>,Block*> kv : chunks) {
    double distance = std::sqrt((px-kv.first.first)*(px-kv.first.first) + (pz-kv.first.second)*(pz-kv.first.second));
    if (distance > 4) {
      threads_running.push_back(true);
      //threads.push_back(new std::thread([&] (pair<int,int> pos, ) {
        
      //}, kv.first, kv.second));
      del_chunk(kv.first);
      render_flag = true;
    }
  }
}

char World::gen_func(int x, int y, int z) {
    return generative_function(x, y, z);
}

void World::iter_gen(int gx, int gy, int gz, Pixel* pix) {
    //gx = gx%chunksize + (gx<0)*chunksize;
    //gy = gy%chunksize + (gy<0)*chunksize;
    //gz = gz%chunksize + (gz<0)*chunksize;
    char value = gen_func(gx,gy,gz);///tmparr[gx*chunksize*chunksize + gy*chunksize + gz];
    bool cont = true;
    for (int x = gx; x < gx+pix->scale and cont; x ++) {
        for (int y = gy; y < gy+pix->scale and cont; y ++) {
            for (int z = gz; z < gz+pix->scale and cont; z ++) {
                char newval = gen_func(x,y,z);//tmparr[x*chunksize*chunksize + y*chunksize + z];
                if (newval != value) {
                    cont = false;
                }
                value = newval;
            }
        }
    }
    if (cont) {
        pix->value = value;
    } else {
        pix->subdivide(iter_gen_func);
        delete pix;
    }
}

Block* World::generate(pair<int,int> pos) {
    cout << chunksize << ' ' << pos.first << ',' << pos.second << endl;
    Pixel* pix = new Pixel(pos.first,0,pos.second,0,chunksize,nullptr);
    Block* result = pix->subdivide(iter_gen_func);
    delete pix;
    return result;
}

void World::render() {
    if (lighting_flag) {
      //update_lighting();
      lighting_flag = false;
    }
    cout << "back in render" << endl;
    for (pair<pair<int,int>, Block*> kvpair : chunks) {
        //double before = clock();
        //chunks[kvpair.first]->all([](Pixel* pix) {
        //    pix->lightlevel = -1;
        //});
        //double during = clock();
        //chunks[kvpair.first]->calculate_light_level();
        //double after = clock();
        //chunks[kvpair.first]->calculate_light_level();
        chunks[kvpair.first]->render(&glvecs, 0, 0, 0);
        cout << "rendered chunk " << kvpair.first.first << ' ' << kvpair.first.second << endl;
        //render_chunk_vectors(kvpair.first);
        //cout << during - before << ' ' << after - during << ' ' << clock()-after << endl;
    }
}

void World::update_lighting() {
  for (pair<pair<int,int>,Block*> kvpair : chunks) {
    pair<int,int> pos = kvpair.first;
    Block* block = kvpair.second;
    block->all([=] (Pixel* pix) {
      pix->reset_lightlevel();
      //cout << pix->lightlevel << endl;
    });
    for (int x = pos.first*chunksize; x < pos.first*chunksize+chunksize; x ++) {
      for (int z = pos.second*chunksize; z < pos.second*chunksize+chunksize; z ++) {
        int y = chunksize-1;
        while (y >= 0 and get(x,y,z) == 0) {
          get_global(x,y,z,1)->get_pix()->lightlevel = 1;
          y--;
        }
      }
    }
  }
  for (pair<pair<int,int>,Block*> kvpair : chunks) {
    kvpair.second->calculate_lightlevel();
    cout << "calculated light of chunk " << kvpair.first.first << ' ' << kvpair.first.second << endl;
  }
}

Block* World::get_global(int x, int y, int z, int scale) {
    //cout << "world::get global(" << x << ' ' << y << ' ' << z << ' ' << scale << endl;
    int px = x/chunksize;
    int pz = z/chunksize;
    if (x < 0 and -x%chunksize != 0) {
        px --;
    } if (z < 0 and -z%chunksize != 0) {
        pz --;
    }
    pair<int,int> pos(px, pz);
    //cout << "pair<" << pos.first << ' ' << pos.second << endl;
    if (chunks.find(pos) == chunks.end() or y >= chunksize or y < 0) {
        //cout << "   returning null\n";
        return nullptr;
    }
    //cout << endl;
    int ox = ((x < 0) ? chunksize : 0) + x%chunksize;
    int oz = ((z < 0) ? chunksize : 0) + z%chunksize;
    //cout << ox << ' ' << y << ' ' << oz << endl;
    return chunks[pos]->get_global(ox, y, oz, scale);
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
      newblock->subdivide([=] (int x, int y, int z, Pixel* pix) {
      	pix->value = val;
      });
      delete newblock;
      newblock = get_global((int)x, (int)y, (int)z, 1);
    }
    while (newblock->scale < minscale) {
        Chunk* parent = newblock->parent;
        parent->del();
        Pixel* newpix = new Pixel(parent->px, parent->py, parent->pz, 0, parent->scale, parent->parent);
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

void World::save_chunk(pair<int,int> pos) {
  save_chunk(pos, chunks[pos]);
}

void World::save_chunk(pair<int,int> pos, Block* block) {
  stringstream path;
  path << "saves/" << name << "/chunk" << pos.first << "x" << pos.second << "y.dat";
  cout << "saving '" << path.str() << "' to file\n";
  ofstream of(path.str(), ios::binary);
  of << "scrolls chunk file:";
  block->save_to_file(of);
}

void World::del_chunk(pair<int,int> pos) {
    save_chunk(pos);
    chunks[pos]->del();
    delete chunks[pos];
    chunks.erase(pos);
}



Block* World::parse_file(istream& ifile, int px, int py, int pz, int scale, Chunk* parent) {
    char c;
    ifile.read(&c,1);
    if (c == '{') {
        Chunk* chunk = new Chunk(px, py, pz, scale, parent);
        for (int x = 0; x < csize; x ++) {
            for (int y = 0; y < csize; y ++) {
                for (int z = 0; z < csize; z ++) {
                    chunk->blocks[x][y][z] = parse_file(ifile, x, y, z, scale/csize, chunk);
                }
            }
        }
        ifile.read(&c,1);
        if (c != '}') {
            cout << "error, mismatched {} in save file" << endl;
        }
        return (Block*) chunk;
    } else {
        if (c == '~') {
          c = ifile.get();
          BlockExtras* extra = new BlockExtras(ifile);
          Pixel* p = new Pixel(px, py, pz, c, scale, parent, extra);
          return (Block*) p;
        } else {
          Pixel* p = new Pixel(px, py, pz, c, scale, parent);
          return (Block*) p;
        }
    }
}

Block* World::load_chunk(pair<int,int> pos) {
    stringstream path;
    path << "saves/" << name << "/chunk" << pos.first << "x" << pos.second << "y.dat";
    ifstream ifile(path.str(), ios::binary);
    if (ifile.good()) {
        cout << "loading '" << path.str() << "' from file\n";
        string header;
        getline(ifile, header, ':');
        
        //chunks[pos] = parse_file(&ifile, pos.first, 0, pos.second, chunksize, nullptr);
        return parse_file(ifile, pos.first, 0, pos.second, chunksize, nullptr);
    } else {
        cout << "generating '" << path.str() << "'\n";
        generate_chunk(pos);
        return load_chunk(pos);
        //return generate(pos);
    }
    //render_chunk_vectors(pos);
}

void World::generate_chunk(pair<int,int> pos) {
  generative_setup_chunk(pos.first, pos.second);
  stringstream path;
  path << "saves/" << name << "/chunk" << pos.first << "x" << pos.second << "y.dat";
  cout << "generating '" << path.str() << "' to file\n";
  ofstream of(path.str(), ios::binary);
  of << "scrolls chunk file:";
  char val = gen_block(of, 0, 0, 0, chunksize);
  //cout << (int)val << int(char(0xff)) << endl;
}

char World::gen_block(ostream& ofile, int gx, int gy, int gz, int scale) {
  if (scale == 1) {
    char val = gen_func(gx, gy, gz);
    ofile << val;
    return val;
  } else {
    stringstream ss;
    ss << '{';
    char val = gen_block(ss, gx, gy, gz, scale/2);
    bool all_same = val != -1;
    for (int x = 0; x < csize; x ++) {
      for (int y = 0; y < csize; y ++) {
        for (int z = 0; z < csize; z ++) {
          if (x > 0 or y > 0 or z > 0) {
            char newval = gen_block(ss, gx+x*(scale/2), gy+y*(scale/2), gz+z*(scale/2), scale/2);
            all_same = all_same and newval == val;
          }
        }
      }
    }
    if (all_same) {
      ofile << val;
      return -1;
    } else {
      ss << '}';
      ofile << ss.rdbuf();
      return 0xff;
    }
  }
}
/*
int World::gen_get_chunk_size(int gx, int gy, int gz) {
  char val = gen_func(gx, gy, gz);
  bool all_same = true;
  int scale = 1
  int scale_max;
  
  int max = 1;
  int tmp = gx;
  while (tmp%2 == 0) {
    tmp /= 2;
    max ++;
  }
  scale_max = max;
  
  int max = 1;
  int tmp = gy;
  while (tmp%2 == 0) {
    tmp /= 2;
    max ++;
  }
  if (max < scale_max) scale_max = max;
  
  int max = 1;
  int tmp = gz;
  while (tmp%2 == 0) {
    tmp /= 2;
    max ++;
  }
  if (max < scale_max) scale_max = max;
  
  while (all_same and scale < max_scale) {
    all_same = true;
    for (int x = 0; x < csize and all_same; x ++) {
      for (int y = 0; y < csize and all_same; y ++) {
        for (int z = 0; z < csize and all_same; z ++) {
          if (x > 0 or y > 0 or z > 0) {
            for (int ox = 0; ox < scale and all_same; ox ++) {
              for (int oy = 0; oy < scale and all_same; oy ++) {
                for (int oz = 0; oz < scale and all_same; oz ++) {
                  if (gen_func(gx+x+ox, gy+y+oy, gz+z+oz) != val) {
                    all_same = false;
                  }
                }
              }
            }
          }
        }
      }
    }
    if (all_same) {
      scale *= 2;
    }
  }
  return scale;
}
*/
void World::close_world() {
    ofstream ofile("saves/" + name + "/player.txt");
    player->save_to_file(ofile);
    delete player;
    vector<pair<int,int> > poses;
    for (pair<pair<int,int>, Block*> kvpair : chunks) {
        poses.push_back(kvpair.first);
    }
    for (pair<int,int> pos : poses) {
        del_chunk(pos);
    }
    save_data_file();
}

#endif
