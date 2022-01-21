#ifndef CLIENT_WORLD
#define CLIENT_WORLD

#include "classes.h"
#include "scrolls/world.h"

class ClientWorld : public World { public:
	PLUGIN_HEAD(ClientWorld);
	
	ClientWorld(string name);
	uint32 clientid;
	
  virtual void startup();
  virtual void spawn_player();
	
  virtual void load_nearby_chunks();
  virtual void timestep(float curtime, float deltatime);
	
  virtual void close_world();
	
	
	void add_block(Block* block, ivec3 globalpos, int scale);
};

#endif
