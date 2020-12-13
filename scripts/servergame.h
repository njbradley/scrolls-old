#ifndef SERVERGAME_PREDEF
#define SERVERGAME_PREDEF

#include "classes.h"
#include "multiplayer.h"

class ServerGame { public:
	ServerSocketManager socketmanager;
	std::thread iothread;
	bool playing = true;
	
	class IOthread { public:
		ServerGame* game;
		IOthread(ServerGame* game);
		void operator()();
	};
	
	ServerGame(int port);
	~ServerGame();
	
	void command(string command);
	
	void setup_gameloop();
	void gametick();
};

#endif
