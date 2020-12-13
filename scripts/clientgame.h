#ifndef CLIENTGAME_PREDEF
#define CLIENTGAME_PREDEF

#include "classes.h"
#include "multiplayer.h"

class ClientGame { public:
	ClientSocketManager socketmanager;
	bool playing = true;
	
	ClientGame(string ip, int port, string username);
	~ClientGame();
	
	void setup_gameloop();
	void gametick();
};

#endif
