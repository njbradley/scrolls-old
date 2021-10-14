#ifndef WORLD_PREDEF
#define WORLD_PREDEF

#include "classes.h"
#include "plugins.h"
#include "collider.h"

#include <set>
#include <mutex>
#include <atomic>

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
  bool generation = true;
  bool saving = true;
  
  double daytime = 500;
  
  TileMap tiles;
  PluginNow<TerrainLoader> terrainloader;
  PluginNow<TileLoader> tileloader;
  PluginNow<PhysicsEngine> physics;
  
  Player* player;
  
  std::set<ivec3,ivec3_comparator> loading_chunks;
  std::set<ivec3,ivec3_comparator> deleting_chunks;
  bool world_closing = false;
  
  World(string name);
  virtual ~World();
  
  string path(string file = "") const;
  
  virtual void setup_files();
  virtual void load_config(istream& ifile);
  virtual void save_config(ostream& ofile);
  virtual void startup();
  virtual void spawn_player();
  virtual void set_player_vars();
  
  vec3 get_position() const;
  bool render();
  void timestep();
  void tick();
  void drop_ticks();
  
  void block_update(ivec3);
  void update_lighting();
  
  virtual void load_nearby_chunks();
  
  Tile* tileat(ivec3 pos);
  Tile* tileat_global(ivec3 pos);
  Block* get_global(int x, int y, int z, int scale);
  Block* get_global(ivec3 pos, int scale);
  
  void set_global(ivec3 pos, int w, Blocktype val, int direction, int joints[6] = nullptr);
  Blocktype get(ivec3 pos);
  void set(ivec3 pos, Blocktype val, int direction = 0);
  Block* raycast(vec3* pos, vec3 dir, double time);
  void add_tile(Tile* tile);
  void del_tile(ivec3 pos, bool remove_faces);
  void close_world();
  bool is_world_closed();
};


#endif
