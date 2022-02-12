#include "world.h"

#include "scrolls/debug.h"
#include "scrolls/tiles.h"
#include "multiplayer/multiplayer.h"
#include "scrolls/graphics.h"
#include "clientgame.h"

struct BlockExecutor : ClientExecutor<BlockPacket> {
	PLUGIN_HEAD(BlockExecutor);
	virtual void run(BlockPacket* pack) {
		cout << "recieved " << pack->globalpos << ' ' << pack->scale << endl;
		game->world.as<ClientWorld>()->add_block(pack->block, pack->globalpos, pack->scale);
	}
};
EXPORT_PLUGIN(BlockExecutor);




ClientWorld::ClientWorld(string name): World(name) {
	
}

void ClientWorld::startup() {
	cout << "begin serving " << endl;
	tileloader->begin_serving();
	
	spectator.position = vec3(0,0,0);
	spectator.angle = vec2(0,0);
	graphics->set_camera(&spectator.position, &spectator.angle);
  viewbox->playerpos = &spectator.position;
  spectator.controller = controls;
}

void ClientWorld::spawn_player() {
	
}

void ClientWorld::load_nearby_chunks() {
	
}

void ClientWorld::timestep(float curtime, float deltatime) {
  
  debuglines->clear("timestep");
  
  // for (Tile* tile : tiles) {
  //   if (player == nullptr) {
  //     for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
  //       player = dynamic_cast<Player*>(free);
	// 			cout << player << " player " << endl;
  //       if (player != nullptr and player->clientid == clientid) {
  //         cout << "got player from world" << endl;
  //         set_player_vars();
  //         break;
  //       }
  //     }
  //   }
  //   tile->timestep(curtime, deltatime);
  // }
	//
  // if (controls->key_pressed('F')) {
  //   spectator.position = player->box.position;
  //   spectator.angle = player->angle;
  //   set_spectator_vars();
  // } else if (controls->key_pressed('G')) {
  //   set_player_vars();
  // }
	//
  // spectator.timestep(curtime, deltatime);
	//
  // daytime += deltatime;
  // if (daytime > 2000) {
  //   daytime -= 2000;
  // }
  // if (daytime < 0) {
  //   daytime += 2000;
  // }
  
}

void ClientWorld::close_world() {
	tileloader->end_serving();
}





void ClientWorld::add_block(Block* block, ivec3 globalpos, int scale) {
	// Tile* tile = tileat_global(globalpos);
	//
	// if (scale == chunksize) {
	// 	if (tile == nullptr) {
	// 		tile = new Tile(SAFEDIV(globalpos, chunksize), this, block);
	// 		add_tile(tile);
	// 	} else {
	// 		cout << "overwriting " << globalpos << ' ' << scale << endl;
	// 		tile->set_chunk(block);
	// 	}
	// } else {
	// 	if (tile == nullptr) {
	// 		tile = new Tile(SAFEDIV(globalpos, chunksize), this, new Block());
	// 	}
	// 	tile->chunk->set_global(globalpos, chunksize, block);
	// }
}
	
	


EXPORT_PLUGIN(ClientWorld);
