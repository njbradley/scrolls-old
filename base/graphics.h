#ifndef BASE_GRAPHICS_H
#define BASE_GRAPHICS_H

#include "classes.h"
#include "plugins.h"

class GraphicsContext { public:
	PLUGIN_HEAD(GraphicsContext, ());
	
	Plugin<RenderVecs> blockvecs;
	Plugin<RenderVecs> transvecs;
	Plugin<UIVecs> uivecs;
	
	virtual ~GraphicsContext() {}
	
	virtual void set_camera(vec3* pos, vec2* rot) = 0;
	
	virtual const PathLib* block_textures() const = 0;
	virtual const PathLib* trans_block_textures() const = 0;
	virtual const PathLib* ui_textures() const = 0;
	
	virtual void block_draw_call() = 0;
	virtual void ui_draw_call() = 0;
	virtual void swap() = 0;
};


extern Plugin<GraphicsContext> graphics;

#endif
