#ifndef GAME_PREDEF
#define GAME_PREDEF

#include "classes.h"
#include "plugins.h"

class Game { public:
	BASE_PLUGIN_HEAD(Game, ());
	
	bool playing = true;
	bool errors = false;
	stringstream debugstream;
	bool debug_visible = true;
	
	Plugin<World> world;
	
	virtual ~Game() {};
	
	virtual void setup_gameloop() = 0;
	virtual void gametick() = 0;
	
	void crash(long long err_code);
	void hard_crash(long long err_code);
};


extern Plugin<Game> game;



#endif
