#ifndef SERVER_GRAPHICS
#define SERVER_GRAPHICS

#include "classes.h"
#include "scrolls/graphics.h"
#include "scrolls/libraries.h"

class NullGraphicsContext : public GraphicsContext { public:
	PLUGIN_HEAD(NullGraphicsContext);
	
	PathLib fakepaths;
	
	virtual void set_camera(vec3* pos, vec2* rot) {};
	
	virtual const PathLib* block_textures() const { return &fakepaths; }
	virtual const PathLib* trans_block_textures() const { return &fakepaths; }
	virtual const PathLib* ui_textures() const { return &fakepaths; }
	
	virtual void block_draw_call() {}
	virtual void ui_draw_call() {}
	virtual void swap() {}
};

#endif
