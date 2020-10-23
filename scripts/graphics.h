#ifndef GRAPHICS_PREDEF
#define GRAPHICS_PREDEF

#include "classes.h"

class GraphicsContext { public:
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
	
	GraphicsContext();
	set_world_buffers(World* world, int mem);
	init_glfw();
	load_textures();
	block_draw_call(World* world);
	void make_ui_buffer(Player* player, string debugstream);
	ui_draw_call(World* world, std::stringstream* debugstream);
	~GraphicsContext();
};

#endif
