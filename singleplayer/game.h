#ifndef SINGLE_GAME_H
#define SINGLE_GAME_H

#include "scrolls/classes.h"
#include "scrolls/plugins.h"
#include "scrolls/game.h"

class SingleGame : public Game { public:
	
	SingleGame();
	~SingleGame();
	
	virtual void setup_gameloop();
	virtual void gametick();
};


#endif
