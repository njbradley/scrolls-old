#ifndef WORLD
#define WORLD

#include <iostream>
#include <functional>
#include <cmath>
#include <fstream>
#include <sstream>
#include <thread>
#include "win.h"
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
    iter_gen_func = [&] (int x, int y, int z, Pixel* p) {
        iter_gen(x, y, z, p);
    };
    for (int x = -1; x < 2; x ++) {
        for (int y = -1; y < 2; y ++) {
            load_chunk(pair<int,int>(x,y));
        }
    }
}

void World::render_chunks() {
    
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
 
void World::generate(pair<int,int> pos) {
    Pixel* pix = new Pixel(pos.first,0,pos.second,0,chunksize,nullptr);/*
    tmparr = new char[chunksize*chunksize*chunksize];
    cout << 349 << endl;
    for (int x = 0; x < chunksize; x ++) {
        for (int y = 0; y < chunksize; y ++) {
            for (int z = 0; z < chunksize; z ++) {
                tmparr[x*chunksize*chunksize + y*chunksize + z] = gen_func(pos.first*chunksize+x, y, pos.second*chunksize+z);
            }
        }
    }
    cout << "half" << endl;*/
    chunks[pos] = pix->subdivide(iter_gen_func);
    delete pix;
}

void World::render() {
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
        //render_chunk_vectors(kvpair.first);
        //cout << during - before << ' ' << after - during << ' ' << clock()-after << endl;
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
    stringstream path;
    path << "saves/" << name << "/chunk" << pos.first << "x" << pos.second << "y.dat";
    cout << "saving '" << path.str() << "' to file\n";
    ofstream of(path.str(), ios::binary);
    of << "scrolls chunk file:";
    chunks[pos]->save_to_file(&of);
    
}

void World::del_chunk(pair<int,int> pos) {
    save_chunk(pos);
    chunks[pos]->del();
    delete chunks[pos];
    chunks.erase(pos);
}

Block* World::parse_file(ifstream* ifile, int px, int py, int pz, int scale, Chunk* parent) {
    char c;
    ifile->read(&c,1);
    if (c == '{') {
        Chunk* chunk = new Chunk(px, py, pz, scale, parent);
        for (int x = 0; x < csize; x ++) {
            for (int y = 0; y < csize; y ++) {
                for (int z = 0; z < csize; z ++) {
                    chunk->blocks[x][y][z] = parse_file(ifile, x, y, z, scale/csize, chunk);
                }
            }
        }
        ifile->read(&c,1);
        if (c != '}') {
            cout << "error, mismatched {} in save file" << endl;
        }
        return (Block*) chunk;
    } else {
        Pixel* p = new Pixel(px, py, pz, c, scale, parent);
        return (Block*) p;
    }
}

void World::load_chunk(pair<int,int> pos) {
    stringstream path;
    path << "saves/" << name << "/chunk" << pos.first << "x" << pos.second << "y.dat";
    ifstream ifile(path.str(), ios::binary);
    if (ifile.good()) {
        cout << "loading '" << path.str() << "' from file\n";
        string header;
        getline(ifile, header, ':');
        
        chunks[pos] = parse_file(&ifile, pos.first, 0, pos.second, chunksize, nullptr);
    } else {
        cout << "generating '" << path.str() << "'\n";
        
        generate(pos);
    }
    //render_chunk_vectors(pos);
}

void World::close_world() {
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
