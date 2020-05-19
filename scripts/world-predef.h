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
#include <map>
#include "rendervec-predef.h"
#include "entity-predef.h"
#include "terrain-predef.h"
#include "blocks-predef.h"
#include <future>
#include <thread>
#include <mutex>

#define csize 2

struct ivec3_comparator {
  bool operator() (const ivec3& lhs, const ivec3& rhs) const
  {
      return lhs.x < rhs.x || lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
  }
};

class World: public Collider {
    vector<ivec3> loading_chunks;
    vector<ivec3> deleting_chunks;
    vector<ivec3> block_updates;
    std::timed_mutex writelock;
    //vector< pair<ivec3, future<Block*> > > loading_chunks;
    //vector< pair<ivec3, future<bool> > > deleting_chunks;
    char* tmparr;
    public:
        map<ivec3, Tile*, ivec3_comparator> tiles;
        int seed;
        string name;
        int view_dist = 3;
        float sun = 0.0f;
        GLVecs glvecs;
        TerrainLoader loader;
        bool lighting_flag;
        Player* player;
				
        static const int chunksize = 64;
        
        World(string newname, int newseed);
        World(string oldname);
        void unzip();
        void load_data_file();
        void save_data_file();
        void setup_files();
        void startup();
        void spawn_player();
        void render();
        void timestep();
        void tick();
        void block_update(int,int,int);
        void update_lighting();
        void load_nearby_chunks();
        void get_async_loaded_chunks();
        Block* get_global(int x, int y, int z, int scale);
        void set(char val, int x, int y, int z);
        char get(int x, int y, int z);
        Block* raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time);
        void save_chunk(ivec3 pos);
        void del_chunk(ivec3 pos, bool remove_faces);
        void load_chunk(ivec3 pos);
        void close_world();
        void zip();
};

#endif
