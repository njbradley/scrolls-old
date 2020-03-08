// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <chrono>
#include <thread>
using std::stringstream;

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "scripts/shader.h"
#include "scripts/texture.h"
#include "scripts/player.h"
#include "scripts/entity.h"
#include "scripts/menu.h"
#include "scripts/world.h"
#include "scripts/blockdata.h"
#include "scripts/blocks.h"
#include "scripts/items.h"
#include "scripts/crafting.h"

#include "scripts/cross-platform.h"


#include <future>

int num_blocks;
int num_uis;

int last_num_verts = 0;
int last_num_ui_verts = 0;

const int max_fps = 200;
const double min_ms_per_frame = 1000.0/max_fps;

bool debug_visible = true;



void make_ui_buffer(Player* player, string debugstream, GLuint vertexbuffer, GLuint uvbuffer, GLuint matbuffer, int * num_tris) {
	RenderVecs vecs;
	if (last_num_ui_verts != 0) {
		vecs.verts.reserve(last_num_ui_verts*3);
		vecs.uvs.reserve(last_num_ui_verts*2);
		vecs.mats.reserve(last_num_ui_verts);
	}
	render_debug(&vecs, debugstream);
	player->render_ui(&vecs);
	if (menu != nullptr) {
		menu->render(window, world, player, &vecs);
	}
	int num_verts = vecs.num_verts;
	last_num_ui_verts = num_verts;
	
	*num_tris = num_verts/3;
	
	GLfloat i = 0.0f;
	GLint j = 0;
	//cout << sizeof(i) << ' ' << sizeof(j) << endl;
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, num_verts*sizeof(i)*3, &vecs.verts.front(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, num_verts*sizeof(i)*2, &vecs.uvs.front(), GL_STATIC_DRAW);
	
	glBindBuffer(GL_ARRAY_BUFFER, matbuffer);
	glBufferData(GL_ARRAY_BUFFER, num_verts*sizeof(j), &vecs.mats.front(), GL_STATIC_DRAW);
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}


int main( void )
{
	
	bool fullscreen = false;
	cout.precision(10);
	
	itemstorage = new ItemStorage();
	blocks = new BlockStorage();
	recipestorage = new RecipeStorage();
	
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}
	
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	
	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Scrolls - An Adventure Game", fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	
	
	//launch_threads(window);
	
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
  glfwMakeContextCurrent(window);
		
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	
	int maxsize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxsize);
	cout << "maxsize: " << maxsize << endl;
	
	glEnable              ( GL_DEBUG_OUTPUT );
	glDebugMessageCallback( MessageCallback, 0 );
	
	
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
  // Hide the mouse and enable unlimited mouvement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
  // Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.3f, 0.0f);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	
	GLuint uiVertexArrayID;
	glGenVertexArrays(1, &uiVertexArrayID);
	
	
	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "resources/shaders/block.vs", "resources/shaders/block.fs" );
	GLuint uiProgram = LoadShaders( "resources/shaders/ui.vs", "resources/shaders/ui.fs" );
	
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	
	
	////// get mats from folders
	vector<string> blocks;
	vector<string> uis;
	get_files_folder("resources/blocks", &blocks);
	get_files_folder("resources/ui", &uis);
	num_blocks = blocks.size();
	num_uis = uis.size();
	
	
	GLuint block_textures[num_blocks];
	GLuint ui_textures[num_uis];
	// Load the texture
	
	
	
	for( int i = 0; i < num_blocks; i ++) {
		string block = "resources/blocks/" + blocks[i];
		const char* data = block.c_str();
		block_textures[i] = loadBMP_array_custom(data);
	}
	
	for( int i = 0; i < num_uis; i ++) {
		string ui = "resources/ui/" + uis[i];
		ui_names[uis[i]] = i;
		const char* data = ui.c_str();
		ui_textures[i] = loadBMP_custom(data, true);
	}
	// Load the texture
	//GLuint Texture = loadBMP_custom("scrolls/resources/blocks/dirt.bmp");
	//GLuint Texture = loadDDS("uvtemplate.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
	GLuint uiTextureID  = glGetUniformLocation(uiProgram, "myTextureSampler");
	
	
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint lightbuffer;
	GLuint matbuffer;
	
	GLuint vertex_ui_buffer;
	GLuint uv_ui_buffer;
	GLuint mat_ui_buffer;
	
	int num_tris;
	int num_ui_tris;
	
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
	
	
	//glGenBuffers(1, &vertexbuffer);
	//glGenBuffers(1, &uvbuffer);
	//glGenBuffers(1, &lightbuffer);
	//glGenBuffers(1, &matbuffer);
	
	//string level = menu(window, vertexbuffer, uvbuffer, matbuffer, uiVertexArrayID, uiTextureID, ui_textures, num_uis, uiProgram, "Select World:", worlds );
	//if (level == "") {
	//	return 0;
	//}
	
	
	world = new World("new-world");
	world->glvecs.set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, 600000*6);
	
	Player* player = world->player;
	
	
	
	double lastTime = glfwGetTime();
	double currentTime = lastTime;
	double lastFrameTime = lastTime;
	int nbFrames = 0;
	render_flag = true;
	int fps;
	bool reaching_max_fps = true;
	double slow_frame = 0;
	
	
	
	//make_vertex_buffer(vertexbuffer, uvbuffer, lightbuffer, &num_tris);
	do {
		
		
		
		
		if (menu == nullptr) {
			player->timestep(world);
			player->computeMatricesFromInputs(world);
		}
		
		
		if (debug_visible) {
			debugstream.str("");
			
			debugstream << "fps: " << fps << endl;
			debugstream << "x: " << player->position.x << "\ny: " << player->position.y << "\nz: " << player->position.z << endl;
			debugstream << "dx: " << player->vel.x << "\ndy: " << player->vel.y << "\ndz: " << player->vel.z << endl;
			debugstream << "consts: ";
			for (bool b : player->consts) {
	            debugstream << b << ' ';
	        }
	        debugstream << endl;
			world->glvecs.status(debugstream);
			////////////////////////// error handling:
			debugstream << "-----opengl errors-----" << endl;
			GLenum err;
			while((err = glGetError()) != GL_NO_ERROR) {
				debugstream << "err: " << std::hex << err << std::dec << endl;
			}
		}
		
		world->load_nearby_chunks();
		if (render_flag) {
			//cout << "rendering!!!!" << endl;
			//auto fut =  std::async([&] () {world->render();});//();
			world->render();
			world->glvecs.clean();
			num_tris = world->glvecs.num_verts/3;
			//make_vertex_buffer(vertexbuffer, uvbuffer, lightbuffer, matbuffer, &num_tris, render_flag);
			render_flag = false;
		}
		
		
		// Measure speed
		lastFrameTime = currentTime;
		currentTime = glfwGetTime();
		double ms = (currentTime-lastFrameTime)*1000;
		if (debug_visible) {
			debugstream << "-----time-----" << endl;
			debugstream << "ms: " << ms << endl;
			debugstream << "ms(goal):" << min_ms_per_frame << endl;
			debugstream << "reaching max fps: " << reaching_max_fps << " (" << slow_frame << ")" << endl;
		}
		
		if (ms > min_ms_per_frame) {
			reaching_max_fps = false;
		}
		if (ms > slow_frame) {
			slow_frame = ms;
		}
		
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
		    // printf and reset timer
		    //printf("%i fps \n", nbFrames);
			fps = nbFrames;
			nbFrames = 0;
		    lastTime += 1.0;
			reaching_max_fps = true;
			slow_frame = 0;
		}
		
		////////////// sleep to max fps
		double sleep_needed = min_ms_per_frame-ms;
		if (sleep_needed > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(min_ms_per_frame-ms)));
		}
		
		
		currentTime = glfwGetTime();
		
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//-----------------------------------------------------FIRST DRAW CALL-------------------------------------------------------------------------------------------------------------------------//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		glBindVertexArray(VertexArrayID);
		
		
		// Use our shader
		glUseProgram(programID);
		
		// Compute the MVP matrix from keyboard and mouse input
		glm::mat4 ProjectionMatrix = player->getProjectionMatrix();
		glm::mat4 ViewMatrix = player->getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		
		int ids[num_blocks];
		for (int i = 0; i < num_blocks; i ++) {
			ids[i] = i;
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D_ARRAY, block_textures[i]);
		}
		
		
		// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		//glUniform1i(TextureID, 0);
		glUniform1iv(TextureID, num_blocks, ids);
		
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
			1,                                // size : 1
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
		
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, num_tris*3); // 12*3 indices starting at 0 -> 12 triangles
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//-----------------------------------------------------SECOND DRAW CALL------------------------------------------------------------------------------------------------------------------------//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		make_ui_buffer(player, debugstream.str(), vertex_ui_buffer, uv_ui_buffer, mat_ui_buffer, &num_ui_tris);
		
		//allow trancaperancy
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		
		glBindVertexArray(uiVertexArrayID);
		//*
		glUseProgram(uiProgram);
		
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
		
		
		// desable trancparency
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		//*/
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		if (glfwGetKey(window, GLFW_KEY_M ) == GLFW_PRESS) {
			
			
			worlds.clear();
			ifstream ifile("saves/saves.txt");
			string name;
			while (ifile >> name) {
				worlds.push_back(name);
			}
			
			debug_visible = false;
			
			menu = new SelectMenu("Level Select:", worlds, [&] (string result) {
				world->close_world();
				delete world;
				world = new World(result);
				player = world->player;
				world->glvecs.set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, 600000*6);
				render_flag = true;
				delete menu;
				menu = nullptr;
				//debug_visible = true;
			});
			
			
		} else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			menu = new InventoryMenu("hi", nullptr, [&] () {
				cout << "done! " << endl;
				delete menu;
				menu = nullptr;
			});
		}
		
	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );
			 
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);
	
	world->close_world();
	
	
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	
	
}
