#ifndef BASE_GRAPHICS_H
#define BASE_GRAPHICS_H

#include "classes.h"
#include "plugins.h"

class GraphicsContext { public:
	PLUGIN_HEAD(GraphicsContext, (Settings*));
	vec3 clearcolor;
	int view_distance;
	
	virtual void set_world_buffers(World* world, int mem) = 0;
	virtual void init_glfw() = 0;
	virtual void load_textures() = 0;
	virtual void block_draw_call(Player* player, vec3 sunlight, AsyncGLVecs* glvecs, AsyncGLVecs* transparent_glvecs) = 0;
	virtual void make_ui_buffer(Player* player, string debugstream) = 0;
	virtual void ui_draw_call(Player* player, std::stringstream* debugstream) = 0;
	virtual void swap() = 0;
	virtual ~GraphicsContext() {}
};
/*
class GraphicsContext { public:
	PLUGIN_HEAD(GraphicsContext, ());
	
	virtual ~GraphicsContext() {}
	virtual RenderVecs* blockvecs();
	virtual RenderVecs* transvecs();
	virtual MemVecs* uivecs();
	
};
*/
#endif
