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
#include <unordered_set>

using std::unordered_set;

#define csize 2



class World: public Collider {
    vector<ivec3> loading_chunks;
    vector<ivec3> deleting_chunks;
    unordered_set<ivec3,ivec3_hash> block_updates;
    std::timed_mutex writelock;
    char* tmparr;
    public:
        map<ivec3, Tile*, ivec3_comparator> tiles;
        //unordered_set<BlockGroup*> physicsgroups;
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
        vec3 get_position();
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
        void set(int x, int y, int z, char val, int direction = 0, BlockExtras* extras = nullptr);
        char get(int x, int y, int z);
        Block* raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time);
        void save_chunk(ivec3 pos);
        void del_chunk(ivec3 pos, bool remove_faces);
        void load_chunk(ivec3 pos);
        void close_world();
        void zip();
};

#endif
