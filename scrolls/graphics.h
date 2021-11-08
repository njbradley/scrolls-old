#ifndef BASE_GRAPHICS_H
#define BASE_GRAPHICS_H

#include "classes.h"
#include "plugins.h"
#include "rendervecs.h"


struct ViewParams {
	double view_distance;
	vec3 clear_color;
	vec3 sun_color;
	vec3 sun_direction;
};

class ViewBox { public:
  BASE_PLUGIN_HEAD(ViewBox, ());
  
	ViewParams params;
	bool in_water = false;
	bool in_block = false;
	vec3* playerpos = nullptr;
	
	ViewBox();
	virtual ~ViewBox() {}
	void timestep(double deltatime, double worldtime);
	
private:
	int suntexture;
	int moontexture;
	RenderIndex sunindex;
	RenderIndex moonindex;
};

extern Plugin<ViewBox> viewbox;

class GraphicsContext { public:
	BASE_PLUGIN_HEAD(GraphicsContext, ());
	
	virtual ~GraphicsContext() {}
	
	virtual void set_camera(vec3* pos, vec2* rot) = 0;
	
	virtual const PathLib* block_textures() const = 0;
	virtual const PathLib* trans_block_textures() const = 0;
	virtual const PathLib* ui_textures() const = 0;
	
	PluginNow<RenderVecs> blockvecs;
	PluginNow<RenderVecs> transvecs;
	PluginNow<UIVecs> uivecs;
	
	virtual void block_draw_call() = 0;
	virtual void ui_draw_call() = 0;
	virtual void swap() = 0;
};

extern Plugin<GraphicsContext> graphics;

#endif
