#include "world.h"
#include "scrolls/debug.h"
#include "scrolls/tiles.h"

ClientWorld::ClientWorld(string name): World(name) {
	
}

void ClientWorld::startup() {
	
}

void ClientWorld::spawn_player() {
	
}

void ClientWorld::load_nearby_chunks() {
	
}

void ClientWorld::timestep(float curtime, float deltatime) {
  
  debuglines->clear("timestep");
  
  for (Tile* tile : tiles) {
    if (player == nullptr) {
      for (FreeBlock* free = tile->allfreeblocks; free != nullptr; free = free->allfreeblocks) {
        player = dynamic_cast<Player*>(free);
        if (player != nullptr and player->clientid == clientid) {
          cout << "got player from world" << endl;
          set_player_vars();
          break;
        }
      }
    }
    tile->timestep(curtime, deltatime);
  }
  
  if (controls->key_pressed('F')) {
    spectator.position = player->box.position;
    spectator.angle = player->angle;
    set_spectator_vars();
  } else if (controls->key_pressed('G')) {
    set_player_vars();
  }
  
  spectator.timestep(curtime, deltatime);
  
  daytime += deltatime;
  if (daytime > 2000) {
    daytime -= 2000;
  }
  if (daytime < 0) {
    daytime += 2000;
  }
  
}

void ClientWorld::close_world() {
	
}



EXPORT_PLUGIN(ClientWorld);
