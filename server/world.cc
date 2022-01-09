#include "world.h"

#include "scrolls/settings.h"
#include "scrolls/tiles.h"
#include "multiplayer/multiplayer.h"

ServerWorld::ServerWorld(string name): World(name) {
	
}

void ServerWorld::startup() {
	World::startup();
	tileloader->do_lighting_update = false;
	
	spawn_pos = ivec3(15,terrainloader.get_height(ivec2(15,15))+8,15);
}

void ServerWorld::parse_config_line(string buff, istream& ifile) {
	World::parse_config_line(buff, ifile);
	if (buff.substr(0,16) == "playerdata") {
		PlayerData data;
		ifile >> data.username >> data.last_pos.x >> data.last_pos.y >> data.last_pos.z;
		unjoined_players.push_back(data);
	}
}

void ServerWorld::player_join(string username, uint32 id) {
	for (int i = 0; i < unjoined_players.size(); i ++) {
		if (unjoined_players[i].username == username) {
			players[id] = unjoined_players[i];
			unjoined_players.erase(unjoined_players.begin() + i);
			return;
		}
	}
	players[id] = PlayerData{.last_pos = spawn_pos, .username = username};
}



void ServerWorld::load_nearby_chunks() {
  bool loaded_chunks = false;
	
	const int maxrange = settings->view_dist;
	
	int max_chunks = ((settings->view_dist)*2+1) * ((settings->view_dist)*2+1) * ((settings->view_dist)*2+1);
	int num_chunks = tiles.size();
	
  for (const pair<uint32,PlayerData>& kvpair : players) {
		uint32 id = kvpair.first;
		const PlayerData& data = kvpair.second;
    
    int px = data.last_pos.x/chunksize - (data.last_pos.x<0);
    int py = data.last_pos.y/chunksize - (data.last_pos.y<0);
    int pz = data.last_pos.z/chunksize - (data.last_pos.z<0);
    
    for (int range = 0; range < maxrange; range ++) {
      // for (int y = py-range; y < py+range+1; y ++) {
      for (int y = py+range; y >= py-range; y --) {
        for (int x = px-range; x < px+range+1; x ++) {
          for (int z = pz-range; z < pz+range+1; z ++) {
            if (x == px-range or x == px+range or y == py-range or y == py+range or z == pz-range or z == pz+range) {
              ivec3 pos(x,y,z);
              if (tileat(pos) == nullptr) {
								if (num_chunks < max_chunks and loading_chunks.count(pos) == 0) {
	                loaded_chunks = true;
	                num_chunks ++;
	                tileloader->request_load_tile(pos);
	                loading_chunks.insert(pos);
								}
              } else {
								socketmanager->send_tile(id, tileat(pos));
							}
            }
          }
        }
      }
    }
	}
  
  const double max_distance = std::sqrt((maxrange)*(maxrange)*2);
  
  for (Tile* tile : tiles) {
    ivec3 pos = tile->pos;
    int range = maxrange;
		
		bool should_delete = true;
		
    for (const pair<uint32,PlayerData>& kvpair : players) {
			uint32 id = kvpair.first;
			const PlayerData& data = kvpair.second;
	    
	    int px = data.last_pos.x/chunksize - (data.last_pos.x<0);
	    int py = data.last_pos.y/chunksize - (data.last_pos.y<0);
	    int pz = data.last_pos.z/chunksize - (data.last_pos.z<0);
			
      if (!(pos.x < px-range or pos.x > px+range or pos.y < py-range or pos.y > py+range or pos.z < pz-range or pos.z > pz+range)) {
				should_delete = false;
				break;
      }
		}
		
		if (should_delete and deleting_chunks.count(pos) == 0) {
			tileloader->request_del_tile(pos);
			deleting_chunks.insert(pos);
		}
  }
}


bool ServerWorld::render() {
	return false;
}

EXPORT_PLUGIN(ServerWorld);
