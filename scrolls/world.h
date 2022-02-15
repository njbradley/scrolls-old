#ifndef WORLD_PREDEF
#define WORLD_PREDEF

#include "classes.h"
#include "plugins.h"
#include "collider.h"
#include "player.h"
#include "terrain.h"

#include <set>
#include <mutex>
#include <atomic>




class World: public Container { public:
  BASE_PLUGIN_HEAD(World, (string name));
  
  static float tick_deltatime;
  static int chunksize;
  
  string name;
  int seed;
  int difficulty = 1;
  bool generation = true;
  bool saving = true;
  ivec3 last_player_pos = ivec3(0,0,0);
  
  double daytime = 600;
  int max_scale = 256;
  Block* mainblock = nullptr;
  TerrainLoader terrainloader;
  PluginNow<TileLoader> tileloader;
  PluginNow<PhysicsEngine> physics;
  Plugin<TickRunner> tickrunner;
  
  Player* player = nullptr;
  Spectator spectator;
  
  std::set<ivec3,ivec3_comparator> loading_chunks;
  std::set<ivec3,ivec3_comparator> deleting_chunks;
  bool world_closing = false;
  bool initial_generation = true;
  
  World(string name);
  virtual ~World();
  
  string path(string file = "") const;
  
  virtual void setup_files();
  virtual void load_config(istream& ifile);
  virtual void parse_config_line(string buff, istream& ifile);
  virtual void save_config(ostream& ofile);
  virtual void startup();
  virtual void spawn_player();
  virtual void set_player_vars();
  virtual void set_spectator_vars();
  
  virtual bool render();
  virtual void timestep(float curtime, float deltatime);
  virtual void tick(float curtime, float deltatime);
  void drop_ticks();
  
  void divide_terrain(Block* block, int max_divides);
  void divide_level(Block* block, int level);
  void join_level(Block* block, int level);
  virtual void load_nearby_chunks();
  
  void set_root(Block* nblock);
  
  Block* get_global(ivec3 pos, int scale);
  const Block* get_global(ivec3 pos, int scale) const;
  
  void set_global(ivec3 pos, int w, Blocktype val, int direction, int joints[6] = nullptr);
  Blocktype get(ivec3 pos);
  void set(ivec3 pos, Blocktype val, int direction = 0);
  Block* raycast(vec3* pos, vec3 dir, double time);
  virtual void close_world();
  virtual bool is_world_closed();
  
  virtual void add_freeblock(FreeBlock* free);
  virtual void remove_freeblock(FreeBlock* free);
};


#endif
