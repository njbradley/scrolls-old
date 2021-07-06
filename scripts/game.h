#ifndef GAME_PREDEF
#define GAME_PREDEF

#include "classes.h"
#include "world.h"
#include "graphics.h"
#include "multithreading.h"
#include "audio.h"

#include <iomanip>

class Settings { public:
	int max_fps;
	bool framerate_sync = true;
	int allocated_memory;
	bool fullscreen = false;
	float initialFoV = 110.0f;
	
	Settings();
	void load_settings();
};

extern World* world;
extern Settings* settings;
extern GraphicsContext* graphics;
extern AudioContext* audio;
extern ThreadManager* threadmanager;
extern PluginManager* pluginmanager;

class Game { public:
	bool playing = true;
	bool errors = false;
	DisplayEntity* debugentity = nullptr;
	Pixel* debugblock = nullptr;
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

class MainGame : public Game { public:
	double min_ms_per_frame;
	
	double lastTime;
	double currentTime;
	double lastFrameTime;
	double preswap_time;
	double afterswap_time;
	int nbFrames;
	int fps;
	bool reaching_max_fps;
	double slow_frame;
	int slow_tick;
	bool dologtime = false;
	
	MainGame();
	virtual ~MainGame();
	
	virtual void setup_gameloop();
	virtual void gametick();
	void print_debug();
	
	virtual void inven_menu();
	virtual void level_select_menu();
	virtual void new_world_menu();
	virtual void main_menu();
	
	virtual void set_display_env(vec3 new_clear_color, int new_view_dist);
	virtual void dump_buffers();
	virtual void dump_emptys();
	
};







#endif
