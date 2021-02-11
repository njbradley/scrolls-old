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
#include "rendervec.h"
#include "terrain.h"
#include "collider.h"
#include "commands.h"
#include <future>
#include <thread>
#include <mutex>
//#include <shared_mutex>
#include <unordered_set>

using std::unordered_set;
using std::unordered_map;


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
  

extern int view_dist;

class World: public Collider {
    vector<ivec3> loading_chunks;
    vector<ivec3> deleting_chunks;
    unordered_set<ivec3,ivec3_hash> block_updates;
    unordered_set<ivec3,ivec3_hash> light_updates;
    vector<std::function<void(World*)>> aftertick_funcs;
    ThreadManager* threadmanager;
    //mutable std::shared_timed_mutex tiles_lock;
    char* tmparr;
    public:
        //unordered_map<ivec3, Tile*, ivec3_hash> tiles;
        vector<Tile*> tiles;
        std::mutex tilelock;
    		vector<pair<int,int> > dead_render_indexes;
        int seed;
        int difficulty = 1;
        bool generation = true;
        bool saving = true;
        string name;
        AsyncGLVecs glvecs;
        AsyncGLVecs transparent_glvecs;
        GLVecsDestination vecs_dest;
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
        bool initial_generation = true;
        double gen_start_time;
				
        static const int chunksize = 64;
        
        World(string newname, ThreadManager* manager, int newseed);
        World(string oldname, ThreadManager* manager);
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
        bool render();
        void timestep();
        void tick();
        void drop_ticks();
        void block_update(int,int,int);
        void aftertick(std::function<void(World*)> func);
        void light_update(int,int,int);
        void update_lighting();
        void load_nearby_chunks();
        void get_async_loaded_chunks();
        void add_tile(Tile* tile);
        Tile* tileat(ivec3 pos);
        Tile* tileat_global(ivec3 pos);
        Block* get_global(int x, int y, int z, int scale);
        void summon(DisplayEntity* entity);
        void set(int x, int y, int z, char val, int direction = 0, BlockExtra* extras = nullptr);
        void set(ivec4 pos, char val, int direction, int joints[6] = nullptr);
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
