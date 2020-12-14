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
	vector<GLuint> ui_textures;
	
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint lightbuffer;
	GLuint matbuffer;
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
	
	GraphicsContext(Settings* newsettings);
	void set_world_buffers(World* world, int mem);
	void init_glfw();
	void load_textures();
	void block_draw_call(Player* player, float sunlight, AsyncGLVecs* glvecs, AsyncGLVecs* transparent_glvecs);
	void make_ui_buffer(Player* player, string debugstream);
	void ui_draw_call(Player* player, std::stringstream* debugstream);
	void swap();
	~GraphicsContext();
};

#endif
