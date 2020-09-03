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
#include <unordered_map>
#include "rendervec-predef.h"
#include "entity-predef.h"
#include "terrain-predef.h"
#include "blocks-predef.h"
#include "collider-predef.h"
#include "commands-predef.h"
#include <future>
#include <thread>
#include <mutex>
//#include <shared_mutex>
#include <unordered_set>

using std::unordered_set;
using std::unordered_map;

#define csize 2

class TileLoop { public:
  World* world;
  class iterator { public:
    TileLoop* tileloop;
    int index;
    iterator(TileLoop* newtileloop, int newindex);
    vector<Tile*>::iterator operator->();
    Tile* operator*();
    iterator operator++();
    friend bool operator==(const iterator& iter1, const iterator& iter2);
    friend bool operator!=(const iterator& iter1, const iterator& iter2);
  };
  TileLoop(World* nworld);
  iterator begin();
  iterator end();
};
  

int view_dist = 3;

class World: public Collider {
    vector<ivec3> loading_chunks;
    vector<ivec3> deleting_chunks;
    unordered_set<ivec3,ivec3_hash> block_updates;
    unordered_set<ivec3,ivec3_hash> light_updates;
    //mutable std::shared_timed_mutex tiles_lock;
    char* tmparr;
    public:
        //unordered_map<ivec3, Tile*, ivec3_hash> tiles;
        vector<Tile*> tiles;
        std::mutex tilelock;
        unordered_set<BlockGroup*> physicsgroups;
    		vector<pair<int,int> > dead_render_indexes;
        int seed;
        int difficulty = 1;
        bool generation = true;
        bool saving = true;
        string name;
        GLVecs glvecs;
        GLVecs transparent_glvecs;
        Program commandprogram;
        TerrainLoader loader;
        bool lighting_flag;
        Player* player;
        bool closing_world;
        int timestep_clock = 0;
        int mobcount;
        float sunlight;
        double daytime = 500;
        double last_time;
				
        static const int chunksize = 64;
        
        World(string newname, int newseed);
        World(string oldname);
        void set_buffers(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size);
        vec3 get_position() const;
        void unzip();
        void load_groups();
        void save_groups();
        void load_data_file();
        void save_data_file();
        void setup_files();
        void startup();
        void spawn_player();
        void set_player_vars();
        void render();
        void timestep();
        void tick();
        void drop_ticks();
        void block_update(int,int,int);
        void light_update(int,int,int);
        void update_lighting();
        void load_nearby_chunks();
        void get_async_loaded_chunks();
        Tile* tileat(ivec3 pos);
        Block* get_global(int x, int y, int z, int scale);
        void summon(DisplayEntity* entity);
        void set(int x, int y, int z, char val, int direction = 0, BlockExtra* extras = nullptr);
        char get(int x, int y, int z);
        Block* raycast(double* x, double* y, double* z, double dx, double dy, double dz, double time);
        void save_chunk(ivec3 pos);
        void del_chunk(ivec3 pos, bool remove_faces);
        void load_chunk(ivec3 pos);
        void close_world();
        bool is_world_closed();
        void zip();
};

#endif
