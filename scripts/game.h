#ifndef GAME_PREDEF
#define GAME_PREDEF

#include "classes.h"
#include "world.h"
#include "graphics.h"
#include "multithreading.h"
#include "audio.h"

#include <iomanip>

class Game { public:
	World* world;
	
	int max_fps;
	double min_ms_per_frame;
	bool framerate_sync = true;
	int allocated_memory;
	bool debug_visible = true;
	bool fullscreen = false;
	bool playing = true;
	bool errors = false;
	float initialFoV = 110.0f;
	
	GraphicsContext graphics;
	AudioContext audio;
	ThreadManager threadmanager;
	
	DisplayEntity* debugentity = nullptr;
	Pixel* debugblock = nullptr;
	
	stringstream debugstream;
	
	double lastTime;
	double currentTime;
	double lastFrameTime;
	int nbFrames;
	int fps;
	bool reaching_max_fps;
	double slow_frame;
	int slow_tick;
	bool dologtime = false;
	
	Game();
	~Game();
	
	void setup_gameloop();
	void gametick();
	void print_debug();
	
	void inven_menu();
	void level_select_menu();
	void new_world_menu();
	void main_menu();
	
	void crash(long long err_code);
	void hard_crash(long long err_code);
	
	void set_display_env(vec3 new_clear_color, int new_view_dist);
	void dump_buffers();
	void dump_emptys();
	
	void load_settings();
};







#endif
