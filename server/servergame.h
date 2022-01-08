#ifndef SERVERGAME_PREDEF
#define SERVERGAME_PREDEF

#include "classes.h"
#include "multiplayer/multiplayer.h"
#include "scrolls/game.h"

class ServerGame : public Game { public:
	PLUGIN_HEAD(ServerGame);
	ServerSocketManager socketmanager;
	std::thread iothread;
	
	class IOthread { public:
		ServerGame* game;
		IOthread(ServerGame* game);
		void operator()();
	};
	
	ServerGame();
	~ServerGame();
	
	void command(string command);
	
	virtual void load_world();
	virtual void setup_gameloop();
	virtual void gametick();
};

#endif
