#ifndef SERVERGAME_PREDEF
#define SERVERGAME_PREDEF

#include "classes.h"
#include "multiplayer.h"
#include "game.h"

class ServerGame : public Game { public:
	ServerSocketManager socketmanager;
	std::thread iothread;
	
	class IOthread { public:
		ServerGame* game;
		IOthread(ServerGame* game);
		void operator()();
	};
	
	ServerGame(int port);
	~ServerGame();
	
	void command(string command);
	
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
