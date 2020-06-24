// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <chrono>
#include <thread>
using std::stringstream;

//#include <nvwa/debug_new.h>

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
#include "scripts/tiles.h"
#include "scripts/blocks.h"
#include "scripts/items.h"
#include "scripts/crafting.h"
#include "scripts/terrain.h"
#include "scripts/blockphysics.h"
#include "scripts/multithreading.h"


#include "scripts/cross-platform.h"


#include <future>

int num_blocks;
int num_uis;

int last_num_verts = 0;
int last_num_ui_verts = 0;

int max_fps;
double min_ms_per_frame;

GLuint vertexbuffer;
GLuint uvbuffer;
GLuint lightbuffer;
GLuint matbuffer;
GLuint vertex_ui_buffer;
GLuint uv_ui_buffer;
GLuint mat_ui_buffer;


int allocated_memory = 2000000*6;

bool debug_visible = true;
bool fullscreen = false;

bool playing = true;
bool errors = false;

void wait() {
	//return;
	std::cout << "Press ENTER to continue...";
	std::cin.ignore( std::numeric_limits <std::streamsize> ::max(), '\n' );
}

void dump_buffers() {
	float* data = (float*)glMapNamedBuffer( world->glvecs.vertexbuffer, GL_READ_ONLY);
	int len = allocated_memory*3;
	ofstream ofile("dump_buffers.txt");
	for (int i = 0; i < len; i ++) {
		if (data[i] > 100000 or i%(len/100) == 0 or i < 1000) {
			ofile << i/6/3 << ' ' << data[i] << endl;
		}
	}
	ofile << "numverts" << world->glvecs.num_verts/6 << endl;
	//char* dat = (char*)data;
	//ofstream ofile("datadump.dat");
	//ofile.write(dat, len*sizeof(float));
	glUnmapNamedBuffer( world->glvecs.vertexbuffer );
	
	int* matdata = (int*)glMapNamedBuffer( world->glvecs.matbuffer, GL_READ_ONLY);
	len = allocated_memory*2;
	ofstream mofile("dump_buffers_mat.txt");
	for (int i = 0; i < len; i ++) {
		if (matdata[i] > 100000 or i%(len/100) == 0 or i < 1000) {
			mofile << i << ' ' << matdata[i] << endl;
		}
	}
	mofile << "numverts" << world->glvecs.num_verts/6 << endl;
	//char* dat = (char*)data;
	//ofstream ofile("datadump.dat");
	//ofile.write(dat, len*sizeof(float));
	glUnmapNamedBuffer( world->glvecs.matbuffer );
	
	data = (float*)glMapNamedBuffer( world->glvecs.lightbuffer, GL_READ_ONLY);
	len = allocated_memory;
	ofstream fofile("dump_buffers_light.txt");
	for (int i = 0; i < len; i ++) {
		if (data[i] > 100000 or i%(len/100) == 0 or i < 1000) {
			fofile << i/6 << ' ' << data[i] << endl;
		}
	}
	fofile << "numverts" << world->glvecs.num_verts/6 << endl;
	//char* dat = (char*)data;
	//ofstream ofile("datadump.dat");
	//ofile.write(dat, len*sizeof(float));
	glUnmapNamedBuffer( world->glvecs.lightbuffer );
	
}

void dump_emptys() {
	ofstream ofile("dump_emptys.txt");
	for (pair<int,int> index : world->glvecs.empty) {
		ofile << index.first << ' ' << index.second << endl;
	}
}


void crash(long long err_code) {
	cout << "fatal error encountered! shutting down cleanly: err code: " << err_code << endl;
	dump_buffers();
	dump_emptys();
	errors = true;
	playing = false;
}

void hard_crash(long long err_code) {
	cout << "fatal error encountered! unable to close cleanly: err code: " << err_code << endl;
	dump_buffers();
	dump_emptys();
	errors = true;
	wait();
	exit(1);
}

void load_settings() {
	ifstream ifile("saves/settings.txt");
	if (ifile.good()) {
		string name;
		while (!ifile.eof()) {
			getline(ifile, name, ':');
			cout << name << ';' << endl;
			if (name == "fov") {
				ifile >> initialFoV;
			} else if (name == "fullscreen") {
				ifile >> name;
				fullscreen = name == "true";
			} else if (name == "max_fps") {
				ifile >> max_fps;
			} else if (name == "dims") {
				int x,y;
				ifile >> x >> y;
				set_screen_dims(x, y);
			} else if (name == "") {
				continue;
			} else {
				cout << "error reading settings file" << endl;
				cout << "no setting '" << name << "'" << endl;
				exit(1);
			}
			getline(ifile, name);
		}
	} else {
		create_dir("saves");
		ofstream ofile("saves/settings.txt");
		ofile << "fov: 110" << endl;
		ofile << "fullscreen: false" << endl;
		ofile << "dims: 1600 900" << endl;
		ofile << "max_fps: 60" << endl;
		load_settings();
	}
}

void make_ui_buffer(Player* player, string debugstream, GLuint vertexbuffer, GLuint uvbuffer, GLuint matbuffer, int * num_tris) {
	MemVecs vecs;
	if (last_num_ui_verts != 0) {
		vecs.verts.reserve(last_num_ui_verts*3);
		vecs.uvs.reserve(last_num_ui_verts*2);
		vecs.mats.reserve(last_num_ui_verts);
	}
	if (debug_visible) {
		render_debug(&vecs, debugstream);
	} else {
		render_debug(&vecs, "");
	}
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


void inven_menu() {
	menu = new InventoryMenu("hi", nullptr, [&] () {
		cout << "done! " << endl;
		delete menu;
		menu = nullptr;
	});
}

void level_select_menu() {
	worlds.clear();
	ifstream ifile("saves/saves.txt");
	string name;
	while (ifile >> name) {
		worlds.push_back(name);
	}
	
	menu = new SelectMenu("Level Select:", worlds, [&] (string result) {
		if (result != "") {
			world->close_world();
			delete world;
			world = new World(result);
			world->glvecs.set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, allocated_memory);
			render_flag = true;
			//debug_visible = true;
		}
		delete menu;
		menu = nullptr;
	});
}

void new_world_menu() {
	menu = new TextInputMenu("Enter your new name:", true, [&] (string result) {
		if (result != "") {
			world->close_world();
			delete world;
			world = new World(result, time(NULL));
			world->glvecs.set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, allocated_memory);
			render_flag = true;
		}
		delete menu;
		menu = nullptr;
	});
}

void main_menu() {
	vector<string> options = {"Level Select", "New World"};
	menu = new SelectMenu("Scrolls: an adventure game", options, [&] (string result) {
		if (result == "Level Select") {
			delete menu;
			level_select_menu();
		} else if (result == "New World") {
			delete menu;
			new_world_menu();
		} else {
			delete menu;
			menu = nullptr;
		}
	});
}


int main( void )
{
	
	//system("rmdir saves /s /q");
	load_settings();
	min_ms_per_frame = 1000.0/max_fps;
	
	cout.precision(10);
	
	itemstorage = new ItemStorage();
	blocks = new BlockStorage();
	
	recipestorage = new RecipeStorage();
	entitystorage = new EntityStorage();
	
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
	
	if (fullscreen) {
		const GLFWvidmode* return_struct = glfwGetVideoMode( glfwGetPrimaryMonitor() );
		set_screen_dims(return_struct->width, return_struct->height);
		window = glfwCreateWindow( screen_x, screen_y, "Scrolls - An Adventure Game", glfwGetPrimaryMonitor(), nullptr);
	} else {
		// Open a window and create its OpenGL context
		window = glfwCreateWindow( screen_x, screen_y, "Scrolls - An Adventure Game", fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
	}
	glfwSetWindowPos(window, 100, 40);
	
	threadmanager = new ThreadManager(window);
	
	//launch_threads(window);
	
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
  glfwMakeContextCurrent(window);
	
	
	//wait();
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
	
	
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
  // Hide the mouse and enable unlimited mouvement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	
  // Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(window, screen_x/2, screen_y/2);

	// Dark blue background
	vec3 clearcolor(0.4f, 0.7f, 1.0f);
	glClearColor(clearcolor.x, clearcolor.y, clearcolor.z, 0.0f);
	
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
	//vector<string> block_tex;
	vector<string> uis;
	//get_files_folder("resources/blocks", &block_tex);
	get_files_folder("resources/ui", &uis);
	ifstream num_blocks_file("resources/blocks/num_blocks.txt");
	num_blocks_file >> num_blocks;
	
	
	num_uis = uis.size() + 1;
	
	
	GLuint block_textures[num_blocks];
	GLuint ui_textures[num_uis];
	// Load the texture
	
	cout << "numblocks " << num_blocks << endl;
	int size = 1;
	for (int i = 0; i < num_blocks; i ++) {
		string block = "resources/blocks/" + std::to_string(size);
		cout << block << endl;
		block_textures[i] = loadBMP_array_folder(block);
		cout << block_textures[i] << endl;
		
		GLenum err;
		while((err = glGetError()) != GL_NO_ERROR) {
			cout << "err: " << std::hex << err << std::dec << endl;
		}
	}
	
	for( int i = 0; i < uis.size(); i ++) {
		string ui = "resources/ui/" + uis[i];
		ui_names[uis[i]] = i;
		const char* data = ui.c_str();
		ui_textures[i] = loadBMP_custom(data, true);
	}
	
	ui_textures[num_uis-1] = loadBMP_image_folder("resources/items", true);
	ui_names["items.bmp"] = num_uis-1;
	// Load the texture
	//GLuint Texture = loadBMP_custom("scrolls/resources/blocks/dirt.bmp");
	//GLuint Texture = loadDDS("uvtemplate.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
	GLuint uiTextureID  = glGetUniformLocation(uiProgram, "myTextureSampler");
	GLuint viewdistID = glGetUniformLocation(programID, "view_distance");
	GLuint clearcolorID = glGetUniformLocation(programID, "clear_color");
	GLuint player_positionID = glGetUniformLocation(programID, "player_position");
	
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
	
	
	//wait();
	ifstream ifile("saves/latest.txt");
	if (!ifile.good()) {
		create_dir("saves");
		ofstream ofile("saves/saves.txt");
		ofile << "";
		world = new World("Starting-World", 12345);
		world->glvecs.set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, allocated_memory);
	} else {
		string latest;
		ifile >> latest;
		if (latest != "") {
			world = new World(latest);
			world->glvecs.set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, allocated_memory);
		} else {
			ifstream ifile2("saves/saves.txt");
			ifile2 >> latest;
			if (latest != "") {
				world = new World(latest);
				world->glvecs.set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, allocated_memory);
			} else {
				world = new World("Starting-World", 12345);
				world->glvecs.set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, allocated_memory);
			}
		}
	}
	
	//Entity* test = new NamedEntity(vec3(0.5, 50, 0), "test");
	
	
	double lastTime = glfwGetTime();
	double currentTime = lastTime;
	double lastFrameTime = lastTime;
	int nbFrames = 0;
	int fps;
	render_flag = true;
	bool reaching_max_fps = true;
	double slow_frame = 0;
	int view_distance = 100;
	
	
	threadmanager->rendering = true;
	//make_vertex_buffer(vertexbuffer, uvbuffer, lightbuffer, &num_tris);
	while (playing) {
		
		
		if (menu == nullptr) {
			world->timestep();
			//test->timestep(world);
			world->player->computeMatricesFromInputs();
		}
		
		
		if (debug_visible) {
			debugstream.str("");
			debugstream << std::fixed;
			debugstream.precision(3);
			
			debugstream << "fps: " << fps << endl;
			debugstream << "x: " << world->player->position.x << "\ny: " << world->player->position.y << "\nz: " << world->player->position.z << endl;
			debugstream << "dx: " << world->player->vel.x << "\ndy: " << world->player->vel.y << "\ndz: " << world->player->vel.z << endl;
			debugstream << "chunk: "
				<< int((world->player->position.x/world->chunksize) - (world->player->position.x<0)) << ' '
				<< int((world->player->position.y/world->chunksize) - (world->player->position.y<0)) << ' '
				<< int((world->player->position.z/world->chunksize) - (world->player->position.z<0)) << endl;
			debugstream << "loaded chunks: " << world->tiles.size() << endl;
			debugstream << "consts: ";
			for (bool b : world->player->consts) {
	            debugstream << b << ' ';
	        }
	        debugstream << endl;
			world->glvecs.status(debugstream);
			debugstream << "------threads-------" << endl;
			debugstream << "load ";
			for (int i = 0; i < threadmanager->num_threads; i ++) {
				//debugstream << "load" << i;
				if (threadmanager->loading[i] != nullptr) {
					ivec3 pos = threadmanager->loading[i]->pos;
					debugstream << pos.x << ',' << pos.y << ',' << pos.z << " ";
				}
			}
			debugstream << endl << "del ";
			for (int i = 0; i < threadmanager->num_threads; i ++) {
				//debugstream << "del " << i;
				if (threadmanager->deleting[i] != nullptr) {
					ivec3 pos = *threadmanager->deleting[i];
					debugstream << pos.x << ',' << pos.y << ',' << pos.z << " ";
				}
			}
			debugstream << endl;
			////////////////////////// error handling:
			debugstream << "-----opengl errors-----" << endl;
			GLenum err;
			while((err = glGetError()) != GL_NO_ERROR) {
				debugstream << "err: " << std::hex << err << std::dec << endl;
			}
			
		}
		
		if (true) {
			//cout << "rendering!!!!" << endl;
			//auto fut =  std::async([&] () {world->render();});//();
			//world->render();
			num_tris = threadmanager->render_num_verts/3;//world->glvecs.num_verts/3;
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
			/// block and entity debug
			debugstream << "-----block-entity-tracking-----" << endl;
			if (debugentity != nullptr) {
				debugstream << "tracking entity " << debugentity << endl;
				debugstream << "x:" << debugentity->position.x << " y:" << debugentity->position.y << " z:" << debugentity->position.z << endl;
				debugstream << "dx:" << debugentity->vel.x << " dy:" << debugentity->vel.y << " dz:" << debugentity->vel.z << endl;
				debugstream << "consts: ";
				for (bool b : debugentity->consts ) {
					debugstream << b << ' ';
				}
				debugstream << endl;
			}
			if (debugblock != nullptr) {
				int gx, gy, gz;
				debugblock->global_position(&gx, &gy, &gz);
				string name = "undef";
				BlockData* data = blocks->blocks[debugblock->get()];
				if (data != nullptr) {
					name = data->name;
				}
				
				debugstream << "tracking block " << debugblock << " at " << gx << "x " << gy << "y " << gz << "z " << endl;
				debugstream << " type:" << name << " char:" << int(debugblock->value) << " scale:" << debugblock->scale << endl;
				debugstream << " direction:" << int(debugblock->direction) << " render_index:" << debugblock->render_index.first << ',' << debugblock->render_index.second << endl;
				debugstream << " parent coords:" << debugblock->px << ',' << debugblock->py << ',' << debugblock->pz << endl;
				debugstream << " physics_group:" << debugblock->physicsgroup << endl;
				if (debugblock->physicsgroup != nullptr) {
					//debugblock->physicsgroup->to_file(debugstream);
				}
			}
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
		glm::mat4 ProjectionMatrix = world->player->getProjectionMatrix();
		glm::mat4 ViewMatrix = world->player->getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
		
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniform3f(clearcolorID, clearcolor.x, clearcolor.y, clearcolor.z);
		glUniform1i(viewdistID, view_distance);
		glUniform3f(player_positionID, world->player->position.x, world->player->position.y, world->player->position.z);
		
		
		int ids[num_blocks];
		for (int i = 0; i < num_blocks; i ++) {
			ids[i] = i;
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_2D_ARRAY, block_textures[i]);
		}
		
		if (!world->glvecs.writelock.try_lock_for(std::chrono::seconds(1))) {
			exit(1);
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
		//cout << num_tris << endl;
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, num_tris*3); // 12*3 indices starting at 0 -> 12 triangles
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		
		world->glvecs.writelock.unlock();
		
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//-----------------------------------------------------SECOND DRAW CALL------------------------------------------------------------------------------------------------------------------------//
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		make_ui_buffer(world->player, debugstream.str(), vertex_ui_buffer, uv_ui_buffer, mat_ui_buffer, &num_ui_tris);
		
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
		
		
		
		if (menu == nullptr) {
			if (glfwGetKey(window, GLFW_KEY_M ) == GLFW_PRESS) {
				main_menu();
			} else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
				inven_menu();
			} else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
				world->glvecs.clean();
			} else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
				world->player->flying = true;
			} else if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
				world->player->flying = false;
			}
		} else {
			if (glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
				menu->close(world);
				delete menu;
				menu = nullptr;
			}
		}
		if (glfwGetKey(window, GLFW_KEY_O ) == GLFW_PRESS) {
			debug_visible = true;
		} else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			debug_visible = false;
		} else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			cout << "njbradley is king" << endl;
		} else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
			hard_crash(1);
		} else if (glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS and glfwGetKey(window, 	GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) {
			playing = false;
		} else if (glfwWindowShouldClose(window)) {
			playing = false;
		}
		
	}
	
	threadmanager->close();
	delete threadmanager;
	
	// Cleanup VBO and shader
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
	
	if (world != nullptr) {
		cout << "deleting world" << endl;
		world->close_world();
		delete world;
	}
	
	delete itemstorage;
	delete blocks;
	delete recipestorage;
	delete entitystorage;
	
	
	//wait();
	
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	
	if (errors) {
		wait();
	} else {
		cout << "completed without errors" << endl;
	}
	//wait();
}


/*
All other scripts included into this script are considered part of the
game and are under this same license

Copyright 2020 Nicholas Bradley. All rights reserved.

I grant the right to download and modify the game for your own personal use.
Please do not distribute or sell original or modified copies of this game without express permission.

This software is provided 'as is' with no expressed or implied warranties.
*/
