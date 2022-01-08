#ifndef CLIENTGAME_PREDEF
#define CLIENTGAME_PREDEF

#include "classes.h"
#include "multiplayer/multiplayer.h"
#include "singleplayer/game.h"

class ClientGame : public SingleGame { public:
	PLUGIN_HEAD(ClientGame);
	ClientSocketManager socketmanager;
	
	ClientGame();
	virtual ~ClientGame();
	
	virtual void load_world();
	virtual void setup_gameloop();
	virtual void gametick();
};

#endif
