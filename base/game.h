#ifndef GAME_PREDEF
#define GAME_PREDEF

#include "classes.h"
#include "plugins.h"

class Game { public:
	PLUGIN_HEAD(Game, ());
	
	bool playing = true;
	bool errors = false;
	stringstream debugstream;
	bool debug_visible = true;
	
	virtual ~Game() {};
	
	virtual void setup_gameloop() = 0;
	virtual void gametick() = 0;
	
	virtual void inven_menu() = 0;
	virtual void level_select_menu() = 0;
	virtual void new_world_menu() = 0;
	virtual void main_menu() = 0;
	
	void crash(long long err_code);
	void hard_crash(long long err_code);
	
	virtual void set_display_env(vec3 new_clear_color, int new_view_dist) = 0;
	virtual void dump_buffers() = 0;
	virtual void dump_emptys() = 0;
};


extern Plugin<Game> game;



#endif
