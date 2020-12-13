#ifndef CLIENTGAME_PREDEF
#define CLIENTGAME_PREDEF

#include "classes.h"
#include "multiplayer.h"
#include "game.h"
#include "graphics.h"
#include "audio.h"

class ClientGame { public:
	ClientSocketManager socketmanager;
	bool playing = true;
	
	GraphicsContext graphics;
	AudioContext audio;
	Settings settings;
	
	Player* player;
	
	ClientGame(string ip, int port, string username);
	~ClientGame();
	
	void setup_gameloop();
	void gametick();
};

#endif
