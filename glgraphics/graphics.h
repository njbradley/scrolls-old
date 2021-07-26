#ifndef GRAPHICS_PREDEF
#define GRAPHICS_PREDEF

#include "classes.h"
#include "base/graphics.h"
#include "base/libraries.h"

class GLGraphicsContext : public GraphicsContext { public:
	GLuint block_vertexid;
	GLuint ui_vertexid;
	GLuint block_program;
	GLuint ui_program;
	
	GLuint blocktex_id;
	GLuint transblocktex_id;
	GLuint breaking_textures;
	GLuint overlay_textures;
	GLuint edges_textures;
	vector<GLuint> uitex_id;
	
	GLuint vertexbuffer;
	GLuint databuffer;
	
	GLuint uibuffer;
	
	GLuint pMatID;
	GLuint mvMatID;
	GLuint blockTextureID;
	GLuint uiTextureID;
	GLuint viewdistID;
	GLuint clearcolorID;
	GLuint sunlightID;
	GLuint suncolorID;
	GLuint breakingTexID;
	GLuint overlayTexID;
	GLuint edgesTexID;
	
	PathLib blocktex;
	PathLib transblocktex;
	PathLib uitex;
	
	vec3* camera_pos;
	vec2* camera_rot;
	
	GLGraphicsContext();
	~GLGraphicsContext();
	
	virtual void set_camera(vec3* pos, vec2* rot);
	
	virtual const PathLib* block_textures() const;
	virtual const PathLib* trans_block_textures() const;
	virtual const PathLib* ui_textures() const;
	
	void init_graphics();
	void load_textures();
	
	virtual void block_draw_call();
	virtual void ui_draw_call();
	virtual void swap();
};


class GraphicsNullContext : public GraphicsContext { public:
	virtual void set_world_buffers(World* world, int mem) {}
	virtual void init_glfw() {}
	virtual void load_textures() {}
	virtual void block_draw_call(Player* player, vec3 sunlight, AsyncGLVecs* glvecs, AsyncGLVecs* transparent_glvecs) {}
	virtual void make_ui_buffer(Player* player, string debugstream) {}
	virtual void ui_draw_call(Player* player, std::stringstream* debugstream) {}
	virtual void swap() {}
	virtual ~GraphicsNullContext() {}
};

#endif
