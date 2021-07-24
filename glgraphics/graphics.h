#ifndef GRAPHICS_PREDEF
#define GRAPHICS_PREDEF

#include "base/classes.h"
#include "base/graphics.h"

class GraphicsMainContext : public GraphicsContext { public:
	Settings* settings;
	//GLFWwindow* window;
	GLuint VertexArrayID;
	GLuint uiVertexArrayID;
	GLuint programID;
	GLuint uiProgram;
	
	int num_blocks;
	int num_uis;
	
	int last_num_ui_verts = 0;
	int num_ui_tris;
	int num_tris;
	
	vector<GLuint> block_textures;
	vector<GLuint> transparent_block_textures;
	GLuint breaking_textures;
	GLuint overlay_textures;
	GLuint edges_textures;
	vector<GLuint> ui_textures;
	
	GLuint vertexbuffer;
	GLuint databuffer;
	
	GLuint vertex_ui_buffer;
	GLuint uv_ui_buffer;
	GLuint mat_ui_buffer;
	
	GLuint pMatID;
	GLuint mvMatID;
	GLuint TextureID;
	GLuint uiTextureID;
	GLuint viewdistID;
	GLuint clearcolorID;
	GLuint sunlightID;
	GLuint suncolorID;
	GLuint breakingTexID;
	GLuint overlayTexID;
	GLuint edgesTexID;
	
	GLuint triquery;
	GLuint triquery_result;
	bool triquery_recieved = true;
	
	GraphicsMainContext(Settings* newsettings);
	virtual void set_world_buffers(World* world, int mem);
	virtual void init_glfw();
	virtual void load_textures();
	virtual void block_draw_call(Player* player, vec3 sunlight, AsyncGLVecs* glvecs, AsyncGLVecs* transparent_glvecs);
	virtual void make_ui_buffer(Player* player, string debugstream);
	virtual void ui_draw_call(Player* player, std::stringstream* debugstream);
	virtual void swap();
	virtual ~GraphicsMainContext();
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
