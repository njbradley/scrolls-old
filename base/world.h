#ifndef WORLD_PREDEF
#define WORLD_PREDEF

#include "classes.h"

#include <set>

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
  
  Tile* operator[] (ivec3 pos);
  
  size_t size() const;
  
  iterator begin();
  iterator end();
  
  void status(ostream& ofile);
};
  



class World: public Collider { public:
  PLUGIN_HEAD(World, (string name));
  
  static const int chunksize = 64;
  
  string name;
  int seed;
  int difficulty = 1;
  double worldtime = 0;
  bool generation = true;
  bool saving = true;
  
  TileMap tiles;
  Plugin<TerrainLoader> terrainloader;
  
  std::set<ivec3,ivec3_comparator> loading_chunks;
  std::set<ivec3,ivec3_comparator> deleting_chunks;
  
  World(string name);
  virtual ~World();
  
  string path(string file = "") const;
  
  void setup_files();
  void read_config(istream& ifile);
  void write_config(ostream& ofile);
  void startup();
  void spawn_player();
  void set_player_vars();
  
  vec3 get_position() const;
  bool render();
  void timestep();
  void tick();
  void drop_ticks();
  
  void block_update(ivec3);
  void update_lighting();
  
  void load_nearby_chunks();
  
  Tile* tileat(ivec3 pos);
  Tile* tileat_global(ivec3 pos);
  Block* get_global(int x, int y, int z, int scale);
  Block* get_global(ivec3 pos, int scale);
  
  void set_global(ivec3 pos, int w, Blocktype val, int direction, int joints[6] = nullptr);
  Blocktype get(int x, int y, int z);
  Block* raycast(vec3* pos, vec3 dir, double time);
  void del_chunk(ivec3 pos, bool remove_faces);
  void close_world();
  bool is_world_closed();
};


#endif
