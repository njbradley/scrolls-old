#ifndef BASE_SETTINGS_H
#define BASE_SETTINGS_H

#include "classes.h"
#include "plugins.h"

class Settings { public:
	BASE_PLUGIN_HEAD(Settings, ());
	
	int max_fps;
	bool framerate_sync = true;
	int allocated_memory;
	bool fullscreen = false;
	float fov = 110.0f;
	int view_dist;
	ivec2 screen_dims;
	
	double aspect_ratio();
	
	Settings();
	virtual void load_settings();
};

extern Plugin<Settings> settings;

#endif
