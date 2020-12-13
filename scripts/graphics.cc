#ifndef GRAPHICS
#define GRAPHICS

#include "graphics.h"

#include "world.h"
#include "entity.h"
#include "blocks.h"
#include "ui.h"
#include "text.h"
#include "shader.h"
#include "texture.h"
#include "menu.h"
#include "cross-platform.h"
#include "game.h"


GraphicsContext::GraphicsContext(Settings* newsettings): settings(newsettings) {
	init_glfw();
	load_textures();
}

void GraphicsContext::set_world_buffers(World* world, int allocated_memory) {
	world->set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, allocated_memory);
}

void GraphicsContext::init_glfw() {
	
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return;
	}
	
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	if (settings->fullscreen) {
		const GLFWvidmode* return_struct = glfwGetVideoMode( glfwGetPrimaryMonitor() );
		set_screen_dims(return_struct->width, return_struct->height);
		window = glfwCreateWindow( screen_x, screen_y, "Scrolls - An Adventure Game", glfwGetPrimaryMonitor(), nullptr);
	} else {
		// Open a window and create its OpenGL context
		window = glfwCreateWindow( screen_x, screen_y, "Scrolls - An Adventure Game", nullptr, nullptr);
	}
	glfwSetWindowPos(window, 100, 40);
	
	//launch_threads(window);
	
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return;
	}
  glfwMakeContextCurrent(window);
	
	
	//wait();
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return;
	}
	
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
  // Hide the mouse and enable unlimited mouvement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
  // Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(window, screen_x/2, screen_y/2);

	// Dark blue background
	clearcolor = vec3(0.4f, 0.7f, 1.0f);
	glClearColor(clearcolor.x, clearcolor.y, clearcolor.z, 0.0f);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	
	glGenVertexArrays(1, &VertexArrayID);
	
	glGenVertexArrays(1, &uiVertexArrayID);
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "resources/shaders/block.vs", "resources/shaders/block.fs" );
	uiProgram = LoadShaders( "resources/shaders/ui.vs", "resources/shaders/ui.fs" );
	
	
	MatrixID = glGetUniformLocation(programID, "MVP");
	TextureID  = glGetUniformLocation(programID, "myTextureSampler");
	uiTextureID  = glGetUniformLocation(uiProgram, "myTextureSampler");
	viewdistID = glGetUniformLocation(programID, "view_distance");
	clearcolorID = glGetUniformLocation(programID, "clear_color");
	player_positionID = glGetUniformLocation(programID, "player_position");
	sunlightID = glGetUniformLocation(programID, "sunlight");
	
	glBindVertexArray(uiVertexArrayID);
	
	GLuint uibuffs[3];
	glGenBuffers(3, uibuffs);
	vertex_ui_buffer = uibuffs[0];
	uv_ui_buffer = uibuffs[1];
	mat_ui_buffer = uibuffs[2];
	//glGenBuffers(1, &mat_ui_buffer);
	//glGenBuffers(1, &mat_ui_buffer);
	//glGenBuffers(1, &mat_ui_buffer);
	
	
	
	glBindVertexArray(VertexArrayID);
	
	GLuint blockbuffs[4];
	glGenBuffers(4, blockbuffs);
	vertexbuffer = blockbuffs[0];
	uvbuffer = blockbuffs[1];
	lightbuffer = blockbuffs[2];
	matbuffer = blockbuffs[3];
	
}

void GraphicsContext::load_textures() {
	////// get mats from folders
	//vector<string> block_tex;
	vector<string> uis;
	//get_files_folder("resources/blocks", &block_tex);
	get_files_folder("resources/textures/ui", &uis);
	ifstream num_blocks_file("resources/textures/blocks/num_blocks.txt");
	num_blocks_file >> num_blocks;
	
	
	num_uis = uis.size() + 1;
	
	block_textures.resize(num_blocks);
	transparent_block_textures.resize(num_blocks);
	ui_textures.resize(num_uis);
	
	// Load the texture
	
	int size = 1;
	for (int i = 0; i < num_blocks; i ++) {
		string block = "resources/textures/blocks/" + std::to_string(size);
		block_textures[i] = loadBMP_array_folder(block);
	}
	
	for (int i = 0; i < num_blocks; i ++) {
		string block = "resources/textures/blocks/transparent/" + std::to_string(size);
		transparent_block_textures[i] = loadBMP_array_folder(block, true);
	}
	
	for( int i = 0; i < uis.size(); i ++) {
		string ui = "resources/textures/ui/" + uis[i];
		ui_names[uis[i]] = i;
		const char* data = ui.c_str();
		ui_textures[i] = loadBMP_custom(data, true);
	}
	
	ui_textures[num_uis-1] = loadBMP_image_folder("resources/textures/items", true);
	ui_names["items.bmp"] = num_uis-1;
}



void GraphicsContext::block_draw_call(World* world) {
	
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//-----------------------------------------------------FIRST DRAW CALL-------------------------------------------------------------------------------------------------------------------------//
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	glBindVertexArray(VertexArrayID);
	
	// Use our shader
	glUseProgram(programID);
	
	// Compute the MVP matrix from keyboard and mouse input
	glm::mat4 ProjectionMatrix = world->player->getProjectionMatrix();
	glm::mat4 ViewMatrix = world->player->getViewMatrix();
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
	
	// Send our transformation to the currently bound shader,
	// in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniform3f(clearcolorID, clearcolor.x * world->sunlight, clearcolor.y * world->sunlight, clearcolor.z * world->sunlight);
	glClearColor(clearcolor.x * world->sunlight, clearcolor.y * world->sunlight, clearcolor.z * world->sunlight, 0.0f);
	glUniform1i(viewdistID, view_distance);
	glUniform3f(player_positionID, world->player->position.x, world->player->position.y, world->player->position.z);
	glUniform1f(sunlightID, world->sunlight);
	
	
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	
	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, lightbuffer);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 2, but must match the layout in the shader.
		2,                                // size : 1
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
	glVertexAttribIPointer(
		3,                                // attribute. No particular reason for 2, but must match the layout in the shader.
		2,                                // size : 1
		GL_INT,                         // type
														 // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	//cout << num_tris << endl;
	// Draw the triangle !
	
	int ids[num_blocks];
	for (int i = 0; i < num_blocks; i ++) {
		ids[i] = i;
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D_ARRAY, block_textures[i]);
	}
	glUniform1iv(TextureID, num_blocks, ids);
	
	glDrawArrays(GL_TRIANGLES, 0, world->glvecs.num_verts); // 12*3 indices starting at 0 -> 12 triangles
	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_CULL_FACE);
	
	for (int i = 0; i < num_blocks; i ++) {
		ids[i] = i;
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D_ARRAY, transparent_block_textures[i]);
	}
	
	glUniform1iv(TextureID, num_blocks, ids);
	
	
	glDrawArrays(GL_TRIANGLES, world->transparent_glvecs.offset, world->transparent_glvecs.num_verts);
	
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}

void GraphicsContext::make_ui_buffer(Player* player, string debugstream) {
	MemVecs vecs;
	if (last_num_ui_verts != 0) {
		vecs.verts.reserve(last_num_ui_verts*3);
		vecs.uvs.reserve(last_num_ui_verts*2);
		vecs.mats.reserve(last_num_ui_verts);
	}
	//if (game->debug_visible) {
		render_debug(&vecs, debugstream);
	//} else {
	//	render_debug(&vecs, "");
	//}
	player->render_ui(&vecs);
	if (menu != nullptr) {
		menu->render(window, game->world, player, &vecs);
	}
	int num_verts = vecs.num_verts;
	last_num_ui_verts = num_verts;
	
	num_ui_tris = num_verts/3;
	
	GLfloat i = 0.0f;
	GLint j = 0;
	//cout << sizeof(i) << ' ' << sizeof(j) << endl;
	glBindBuffer(GL_ARRAY_BUFFER, vertex_ui_buffer);
	glBufferData(GL_ARRAY_BUFFER, num_verts*sizeof(i)*3, &vecs.verts.front(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, uv_ui_buffer);
	glBufferData(GL_ARRAY_BUFFER, num_verts*sizeof(i)*2, &vecs.uvs.front(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, mat_ui_buffer);
	glBufferData(GL_ARRAY_BUFFER, num_verts*sizeof(j), &vecs.mats.front(), GL_STATIC_DRAW);
}

void GraphicsContext::ui_draw_call(World* world, std::stringstream* debugstream) {
	
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	glBindVertexArray(uiVertexArrayID);
	//*
	glUseProgram(uiProgram);
	
	make_ui_buffer(world->player, debugstream->str());
	
	int uv_ids[num_uis];
	
	for (int i = 0; i < num_uis; i ++) {
		uv_ids[i] = i;
		glActiveTexture(GL_TEXTURE0+i);
		glBindTexture(GL_TEXTURE_2D, ui_textures[i]);
	}
	
	glUniform1iv(uiTextureID, num_uis, uv_ids);
	
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_ui_buffer);
	glVertexAttribPointer(
		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);
	
	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uv_ui_buffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, mat_ui_buffer);
	glVertexAttribIPointer(
		2,                                // attribute. No particular reason for 2, but must match the layout in the shader.
		1,                                // size : 1
		GL_INT,                         // type
														 // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, num_ui_tris*3); // 12*3 indices starting at 0 -> 12 triangles
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	
	
	// disable transparency
	//*/
	// Swap buffers
}

void GraphicsContext::swap() {
	glfwSwapBuffers(window);
	glfwPollEvents();
}

GraphicsContext::~GraphicsContext() {
	
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &lightbuffer);
	glDeleteBuffers(1, &matbuffer);
	
	glDeleteBuffers(1, &vertex_ui_buffer);
	glDeleteBuffers(1, &uv_ui_buffer);
	glDeleteBuffers(1, &mat_ui_buffer);
	
	glDeleteProgram(programID);
	glDeleteProgram(uiProgram);
	
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteVertexArrays(1, &uiVertexArrayID);
}


#endif
