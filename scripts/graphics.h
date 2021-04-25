#ifndef GRAPHICS_PREDEF
#define GRAPHICS_PREDEF

#include "classes.h"

class GraphicsContext { public:
	Settings* settings;
	//GLFWwindow* window;
	GLuint VertexArrayID;
	GLuint uiVertexArrayID;
	GLuint programID;
	GLuint uiProgram;
	
	vec3 clearcolor;
	int view_distance;
	
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
	
	GLuint MatrixID;
	GLuint TextureID;
	GLuint uiTextureID = 8969;
	GLuint viewdistID;
	GLuint clearcolorID;
	GLuint player_positionID;
	GLuint sunlightID;
	GLuint breakingTexID;
	GLuint overlayTexID;
	GLuint edgesTexID;
	
	GLuint triquery;
	GLuint triquery_result;
	bool triquery_recieved = true;
	
	virtual void set_world_buffers(World* world, int mem) = 0;
	virtual void init_glfw() = 0;
	virtual void load_textures() = 0;
	virtual void block_draw_call(Player* player, float sunlight, AsyncGLVecs* glvecs, AsyncGLVecs* transparent_glvecs) = 0;
	virtual void make_ui_buffer(Player* player, string debugstream) = 0;
	virtual void ui_draw_call(Player* player, std::stringstream* debugstream) = 0;
	virtual void swap() = 0;
	virtual ~GraphicsContext() {}
};

class GraphicsMainContext : public GraphicsContext { public:
	GraphicsMainContext(Settings* newsettings);
	virtual void set_world_buffers(World* world, int mem);
	virtual void init_glfw();
	virtual void load_textures();
	virtual void block_draw_call(Player* player, float sunlight, AsyncGLVecs* glvecs, AsyncGLVecs* transparent_glvecs);
	virtual void make_ui_buffer(Player* player, string debugstream);
	virtual void ui_draw_call(Player* player, std::stringstream* debugstream);
	virtual void swap();
	virtual ~GraphicsMainContext();
};

class GraphicsNullContext : public GraphicsContext { public:
	virtual void set_world_buffers(World* world, int mem) {}
	virtual void init_glfw() {}
	virtual void load_textures() {}
	virtual void block_draw_call(Player* player, float sunlight, AsyncGLVecs* glvecs, AsyncGLVecs* transparent_glvecs) {}
	virtual void make_ui_buffer(Player* player, string debugstream) {}
	virtual void ui_draw_call(Player* player, std::stringstream* debugstream) {}
	virtual void swap() {}
	virtual ~GraphicsNullContext() {}
};

#endif
