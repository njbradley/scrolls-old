#ifndef BASE_GRAPHICS_H
#define BASE_GRAPHICS_H

#include "classes.h"
#include "plugins.h"

struct ViewParams {
	double view_distance;
	vec3 clear_color;
	vec3 sun_color;
	vec3 sun_direction;
};

class ViewBox { public:
  PLUGIN_HEAD(ViewBox, ());
  
	ViewParams params;
	
	ViewBox();
	virtual ~ViewBox() {}
	
	void timestep(double deltatime, double worldtime);
};

class GraphicsContext { public:
	PLUGIN_HEAD(GraphicsContext, ());
	
	Plugin<ViewBox> viewbox;
	
	virtual ~GraphicsContext() {}
	
	virtual void set_camera(vec3* pos, vec2* rot) = 0;
	
	virtual const PathLib* block_textures() const = 0;
	virtual const PathLib* trans_block_textures() const = 0;
	virtual const PathLib* ui_textures() const = 0;
	
	virtual RenderVecs* blockvecs() = 0;
	virtual RenderVecs* transvecs() = 0;
	virtual UIVecs* uivecs() = 0;
	
	virtual void block_draw_call() = 0;
	virtual void ui_draw_call() = 0;
	virtual void swap() = 0;
};

extern Plugin<GraphicsContext> graphics;

#endif
