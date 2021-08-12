#ifndef SINGLE_GAME_H
#define SINGLE_GAME_H

#include "base/classes.h"
#include "base/plugins.h"
#include "base/game.h"

class SingleGame : public Game { public:
	
	Plugin<World> world;
	
	SingleGame();
	~SingleGame();
	
	virtual void setup_gameloop();
	virtual void gametick();
};


#endif
