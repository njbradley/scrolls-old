#ifndef SERVER_WORLD
#define SERVER_WORLD

#include "classes.h"
#include "scrolls/world.h"

struct PlayerData {
	Player* player = nullptr;
	ivec3 last_pos;
	string username;
};

class ServerWorld : public World { public:
	PLUGIN_HEAD(ServerWorld);
	
	std::map<uint32,PlayerData> players;
	vector<PlayerData> unjoined_players;
	
	ivec3 spawn_pos;
	
	ServerSocketManager* socketmanager;
	
	ServerWorld(string name);
	
  virtual void startup();
  virtual void parse_config_line(string buff, istream& ifile);
	
	void player_join(string username, uint32 id);
	void player_leave(uint32 id);
	virtual bool render();
	
  virtual void load_nearby_chunks();
	virtual void timestep(float curtime, float deltatime);
	
	void send_tile(uint32 id, Tile* tile);
	void update_block(uint32 id, Block* block);
};

#endif
