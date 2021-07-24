#ifndef CLIENTGAME_PREDEF
#define CLIENTGAME_PREDEF

#include "classes.h"
#include "multiplayer.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"

class ClientGame : public Game { public:
	ClientSocketManager socketmanager;
	
	Player* player;
	
	ClientGame(string ip, int port, string username);
	virtual ~ClientGame();
	
	virtual void setup_gameloop();
	virtual void gametick();
	
	virtual void inven_menu() {}
	virtual void level_select_menu() {}
	virtual void new_world_menu() {}
	virtual void main_menu() {}
	
	virtual void set_display_env(vec3 new_clear_color, int new_view_dist) {}
	virtual void dump_buffers() {}
	virtual void dump_emptys() {}
};

#endif
