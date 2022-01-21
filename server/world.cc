#include "world.h"

#include "scrolls/settings.h"
#include "scrolls/tiles.h"
#include "multiplayer/multiplayer.h"

#include "servergame.h"

struct JoinExecutorPlayer : ServerExecutor<JoinPacket> {
	PLUGIN_HEAD(JoinExecutorPlayer);
	virtual void run(JoinPacket* pack) {
		cout << " running player_join " << endl;
		game->world.as<ServerWorld>()->player_join(pack->username, pack->clientid);
	}
};
EXPORT_PLUGIN(JoinExecutorPlayer);

struct LeaveExecutorPlayer : ServerExecutor<LeavePacket> {
	PLUGIN_HEAD(LeaveExecutorPlayer);
	virtual void run(LeavePacket* pack) {
		game->world.as<ServerWorld>()->player_leave(pack->clientid);
	}
};
EXPORT_PLUGIN(LeaveExecutorPlayer);





ServerWorld::ServerWorld(string name): World(name) {
	
	PluginListNow<ServerExecutor<JoinPacket>> tmplist;
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
	cout << "player join " << endl;
	for (int i = 0; i < unjoined_players.size(); i ++) {
		if (unjoined_players[i].username == username) {
			players[id] = unjoined_players[i];
			unjoined_players.erase(unjoined_players.begin() + i);
			return;
		}
	}
	players[id] = PlayerData{.last_pos = spawn_pos, .username = username};
}

void ServerWorld::player_leave(uint32 id) {
	cout << "player leave " << id << endl;
	unjoined_players.push_back(players[id]);
	players.erase(id);
}


void ServerWorld::load_nearby_chunks() {
  bool loaded_chunks = false;
	const int maxrange = settings->view_dist;
	
	int max_chunks = ((settings->view_dist)*2+1) * ((settings->view_dist)*2+1) * ((settings->view_dist)*2+1);
	int num_chunks = tiles.size();
	
  for (const pair<uint32,PlayerData>& kvpair : players) {
		uint32 id = kvpair.first;
		const PlayerData& data = kvpair.second;
    
		if (data.player != nullptr) {
			players[kvpair.first].last_pos = SAFEFLOOR3(data.player->box.position);
		}
		
		// cout << kvpair.second.last_pos << " player pos " << endl;
		
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
								send_tile(id, tileat(pos));
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

void ServerWorld::timestep(float curtime, float deltatime) {
  
	for (const pair<uint32,PlayerData>& kvpair : players) {
		if (kvpair.second.player == nullptr) {
			Tile* tile = tileat_global(kvpair.second.last_pos);
			if (tile != nullptr) {
				Player* player = nullptr;
	      for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
					player = dynamic_cast<Player*>(free);
	        if (player != nullptr and player->username == kvpair.second.username) {
	          cout << "got player from world" << endl;
						break;
	        } else {
						player = nullptr;
					}
	      }
				if (player == nullptr) {
					
					cout << "spawnedd player " << endl;
					
					ivec3 spawnpos = kvpair.second.last_pos;
				  player = new Player();
					player->username = kvpair.second.username;
				  Block* spawnblock = get_global(spawnpos, 8);
				  while (spawnblock->scale > 8) {
				    spawnblock->subdivide();
				    spawnblock = spawnblock->get_global(spawnpos, 8);
				  }
				  spawnblock->add_freechild(player);
				  player->set_position(spawnpos);
					/*
					cout << "spawning player " << kvpair.second.last_pos << ' ' << *(void**)tile->chunk << endl;
					Block* block = get_global(kvpair.second.last_pos, 4);
					cout << block << ' ' << *(void**)block << ' ' << block->globalpos << ' ' << block->scale << endl;
					while (block->scale > 4) {
					 	cout << block << ' ' << *(void**)block << ' ' << block->globalpos << ' ' << block->scale << endl;
						// block->subdivide();
					 	cout << block << ' ' << *(void**)block << ' ' << block->globalpos << ' ' << block->scale << endl;
						Block* block = block->get_global(kvpair.second.last_pos, 4);
						cout << block << ' ' << *(void**)block << ' ' << block->globalpos << ' ' << block->scale << " - " << endl;
					}
					
					player = new Player();
					player->username = kvpair.second.username;
					player->clientid = kvpair.first;
					block->add_freechild(player);
					player->set_position(kvpair.second.last_pos);*/
				}
				players[kvpair.first].player = player;
				player->clientid = kvpair.first;
	    }
		}
	}
	
	for (Tile* tile : tiles) {
    tile->timestep(curtime, deltatime);
  }
  
  daytime += deltatime;
  if (daytime > 2000) {
    daytime -= 2000;
  }
  if (daytime < 0) {
    daytime += 2000;
  }
  
}

bool ServerWorld::render() {
	return false;
}









void ServerWorld::send_tile(uint32 id, Tile* tile) {
	update_block(id, tile->chunk);
}

void ServerWorld::update_block(uint32 id, Block* block) {
	if (block != nullptr) {
		if (block->flags & Block::CHANGE_PROP_FLAG) {
			block->reset_flag(Block::CHANGE_PROP_FLAG);
			if (((block->flags & Block::CHANGE_FLAG) and block->scale <= 8) or !block->continues) {
				block->reset_all_flags(Block::CHANGE_FLAG | Block::CHANGE_PROP_FLAG);
				BlockPacket blockpack(block);
				socketmanager->send(&blockpack, id);
			} else {
				for (int i = 0; i < csize3; i ++) {
					update_block(id, block->children[i]);
				}
			}
		}
	}
}






EXPORT_PLUGIN(ServerWorld);
