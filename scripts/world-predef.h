#ifndef WORLD_PREDEF
#define WORLD_PREDEF

#include "classes.h"
#include <iostream>
#include <functional>
#include <cmath>
#include <fstream>
#include <sstream>
#include <thread>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <map>
#include "rendervec-predef.h"
#include <future>
using namespace glm;

#define csize 2


class World {
    map<pair<int,int>, Block*> chunks;
    vector< pair<pair<int,int>, future<Block*> > > loading_chunks;
    vector< pair<pair<int,int>, future<bool> > > deleting_chunks;
    char* tmparr;
    public:
        int seed;
        string name;
        int view_dist = 3;
        float sun = 0.0f;
        static const int chunksize = 64;
        function<void(int,int,int,Pixel*)> iter_gen_func;
        GLVecs glvecs;
				
        World(string newname, int newseed);
        World(string oldname);
        void load_data_file();
        void save_data_file();
        void setup_files();
        void startup();
        void render_chunks();
        char gen_func(int x, int y, int z);
        void iter_gen(int gx, int gy, int gz, Pixel* pix);
        Block* generate(pair<int,int> pos);
        void render();
        void load_nearby_chunks(Player*);
        Block* get_global(int x, int y, int z, int scale);
        void set(char val, int x, int y, int z);
        char get(int x, int y, int z);
        Block* raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time);
        void save_chunk(pair<int,int> pos);
        void del_chunk(pair<int,int> pos);
        Block* parse_file(ifstream* ifile, int px, int py, int pz, int scale, Chunk* parent);
        Block* load_chunk(pair<int,int> pos);
        void close_world();
};

#endif
