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
#include <atomic>
//#include <shared_mutex>
#include <unordered_set>

using std::unordered_set;
using std::unordered_map;
  

class TileMap {
  
  struct Item {
    Tile* tile;
    Item* next;
  };
  
  struct Bucket {
    std::mutex lock;
    Item* items = nullptr;
  };
  
  Bucket* buckets;
  const int num_buckets;
  std::atomic<int> num_items;
  
public:
  
  class iterator {
  public:
    Bucket* bucket;
    Bucket* endbucket;
    Item* item;
    Tile* operator*();
    iterator operator++();
    void move_to_next();
    friend bool operator!=(const iterator& iter1, const iterator& iter2);
  };
  
  TileMap(int max_items);
  ~TileMap();
  
  void add_tile(Tile* tile);
  Tile* tileat(ivec3 pos);
  Tile* del_tile(ivec3 pos);
  
  int size();
  
  iterator begin();
  iterator end();
  
  void status(ostream& ofile);
};



extern int view_dist;

class World: public Container {
    vector<ivec3> loading_chunks;
    vector<ivec3> deleting_chunks;
    unordered_set<ivec3,ivec3_hash> block_updates;
    unordered_set<ivec3,ivec3_hash> light_updates;
    vector<std::function<void(World*)>> aftertick_funcs;
    char* tmparr;
    public:
        TileMap tiles;
        std::mutex loadinglock;
        std::mutex deletinglock;
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
        bool world_closing = false;
        int timestep_clock = 0;
        int mobcount;
        vec3 sunlight;
        vec3 suncolor;
        RenderIndex sunindex;
        int suntexture;
        RenderIndex moonindex;
        int moontexture;
        double daytime = 500;
        double last_time;
        bool initial_generation = true;
        double gen_start_time;
				
        static const int chunksize = 64;
        
        World(string newname, int newseed);
        World(string oldname);
        World(string newname, istream& datafile);
        void set_buffers(GLuint verts, GLuint datas, int start_size);
        vec3 get_position() const;
        void load_data_file(istream& ifile);
        void save_data_file(ostream& ofile);
        void setup_files();
        void startup();
        void spawn_player();
        void set_player_vars();
        bool render();
        void timestep();
        void tick();
        void drop_ticks();
        void block_update(int,int,int);
        void block_update(ivec3);
        void aftertick(std::function<void(World*)> func);
        void light_update(int,int,int);
        void update_lighting();
        void load_nearby_chunks();
        void add_tile(Tile* tile);
        Tile* tileat(ivec3 pos);
        Tile* tileat_global(ivec3 pos);
        Block* get_global(int x, int y, int z, int scale);
        void summon(DisplayEntity* entity);
        void set(int x, int y, int z, char val, int direction = 0, BlockExtra* extras = nullptr);
        void set(ivec4 pos, char val, int direction, int joints[6] = nullptr);
        void set_global(ivec3 pos, int w, int val, int direction, int joints[6] = nullptr);
        char get(int x, int y, int z);
        Block* raycast(vec3* pos, vec3 dir, double time);
        void save_chunk(ivec3 pos);
        void del_chunk(ivec3 pos, bool remove_faces);
        void close_world();
        bool is_world_closed();
};

#endif
