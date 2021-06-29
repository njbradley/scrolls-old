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

#include "shader.h"
#include "texture.h"
#include "entity.h"
#include "menu.h"
#include "world.h"
#include "blockdata.h"
#include "tiles.h"
#include "blocks.h"
#include "items.h"
#include "crafting.h"
#include "terrain.h"
#include "blockgroups.h"
#include "multithreading.h"

#include "cross-platform.h"
#include "commands.h"

#include "classes.h"

int num_blocks;
int num_uis;

int last_num_verts = 0;
int last_num_ui_verts = 0;

int max_fps = 120;
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

bool rotation_enabled;




BlockContainer* worldblock;

class SimplePlayer {
	
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
	vec3 pointing;
	int block_sel;
	public:
		vec3 position;
	  vec3 vel;
	  vec2 angle;
		bool autojump = false;
		
		SimplePlayer(vec3 pos): position(pos), angle(0,0), block_sel(1) {
			glfwSetMouseButtonCallback(window, mouse_button_call);
			glfwSetScrollCallback(window, scroll_callback);
			speed = 4;
		}
			
		mat4 getViewMatrix(){
			return ViewMatrix;
		}
		mat4 getProjectionMatrix(){
			return ProjectionMatrix;
		}
		
		Block* raycast(double* x, double* y, double* z) {
			vec3 start(*x, *y, *z);
			double tiny = 0.00001;
			int scale = worldblock->block->scale;
			//cout << entity << endl;
			//print(start);
			float dt = 0;
			bool ray_valid = true;
			for (int axis = 0; axis < 3 and ray_valid; axis ++) {
				float time;
				if (start[axis] < 0) {
					time = start[axis] * -1 / pointing[axis];
				} else if (start[axis] >= scale) {
					time = (start[axis] - scale) * -1 / pointing[axis] + 0.001f;
				} else {
					continue;
				}
				//cout << time << ' ';
				if ( time < 0 ) {
					ray_valid = false;
					continue;
				} else if (time > dt) {
					dt = time;
				}
			}// cout << endl;
			if (!ray_valid or glm::length(pointing*dt) > 8) {
				return nullptr;
			}
			start += pointing*dt;
			//print (start);
			double ex = start.x;
			double ey = start.y;
			double ez = start.z;
			if (ex < 0 or ex >= scale or ey < 0 or ey >= scale or ez < 0 or ez >= scale) {
				return nullptr;
			}
			//cout << "made it" << endl;
			Block* start_block = worldblock->block->get_global(int(ex) - (ex<0), int(ey) - (ey<0), int(ez) - (ez<0), 1);
			//cout << start_block << endl;
			Block* newtarget = start_block->raycast(worldblock, &ex, &ey, &ez, pointing.x + tiny, pointing.y + tiny, pointing.z + tiny, 10);
			*x = ex;
			*y = ey;
			*z = ez;
			return newtarget;
		}
		
		void right_mouse() {
			//cout << "riht" << endl;
			vec3 v = position;
			double tiny = 0.0000001;
			double x = position.x;
			double y = position.y;
			double z = position.z;
			// Block* start = worldblock->get_global(int(x), int(y), int(z), 1);
			// Block* target = nullptr;
			// if (start != nullptr) {
			// 	target = start->raycast(worldblock, &x, &y, &z, pointing.x + tiny, pointing.y + tiny, pointing.z + tiny, 10);
			// }
			Block* target = raycast(&x, &y, &z);
			if (target == nullptr or target->get() == 0) {
				return;
			}
			int ox = (int)x - (x<0);
			int oy = (int)y - (y<0);
			int oz = (int)z - (z<0);
			x -= pointing.x*0.001;
			y -= pointing.y*0.001;
			z -= pointing.z*0.001;
			int dx = (int)x - (x<0) - ox;
			int dy = (int)y - (y<0) - oy;
			int dz = (int)z - (z<0) - oz;
			Block* newblock = worldblock->get_global((int)x - (x<0), (int)y - (y<0), (int)z - (z<0), 1);
			if (newblock == nullptr) {
				return;
			}
			while (newblock->scale != 1) {
				char val = newblock->get();
				newblock->get_pix()->subdivide();
				delete newblock;
				newblock = worldblock->get_global((int)x, (int)y, (int)z, 1);
			}
			//cout << "set" << endl;
			newblock->get_pix()->value = block_sel;
			newblock->get_pix()->set_render_flag();
			if (!rotation_enabled or !blocks->blocks[block_sel]->rotation_enabled) {
				newblock->get_pix()->direction = blocks->blocks[block_sel]->default_direction;
			}
			const ivec3 dirs[6] =    {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
			for (int i = 0; i < 6; i ++) {
				ivec3 offset = dirs[i];
				Block* other = worldblock->get_global((int)x + offset.x, int(y) + offset.y, int(z)+offset.z, 1);
				if (other != nullptr) {
					other->get_pix()->set_render_flag();
				}
				if (rotation_enabled and blocks->blocks[block_sel]->rotation_enabled and offset == ivec3(dx,dy,dz)) {
					newblock->get_pix()->direction = i;
				}
			}
			//newblock->get_pix()->value = block_sel;
			//worldblock->mark_render_update(pair<int,int>((int)position.x/worldblock->chunksize - (position.x<0), (int)position.z/worldblock->chunksize - (position.z<0)));
		}
		
		void left_mouse() {
			//cout << "lefftt" << endl;
			vec3 v = position;
			double tiny = 0.0000001;
			double x = position.x;
			double y = position.y;
			double z = position.z;
			Block* target = raycast(&x, &y, &z);
			if (target == nullptr or target->get() == 0) {
				return;
			}
			
			while (target->scale != 1) {
				char val = target->get();
				target->get_pix()->subdivide();
				delete target;
				target = worldblock->get_global((int)x, (int)y, (int)z, 1);
			}
			//cout << "set" << endl;
			target->get_pix()->value = 0;
			target->get_pix()->set_render_flag();
			const ivec3 dirs[7] {{-1,0,0}, {0,-1,0}, {0,0,-1}, {1,0,0}, {0,1,0}, {0,0,1}};
			for (ivec3 offset : dirs) {
				Block* other = worldblock->get_global((int)x + offset.x, int(y) + offset.y, int(z)+offset.z, 1);
				if (other != nullptr) {
					other->get_pix()->set_render_flag();
				}
			}
			//worldblock->mark_render_update(pair<int,int>((int)position.x/worldblock->chunksize - (position.x<0), (int)position.z/worldblock->chunksize - (position.z<0)));
		}
		
		
		void mouse_button() {
			//cout << "hello" << endl;
			if (button == GLFW_MOUSE_BUTTON_LEFT) {
				
			} else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				
			}
		}
		
		void computeMatricesFromInputs(){
		
			// glfwGetTime is called only once, the first time this function is called
			static double lastTime = glfwGetTime();
		
			// Compute time difference between current and last frame
			double currentTime = glfwGetTime();
			float deltaTime = float(currentTime - lastTime);
		
			// Get mouse position
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
		
			// Reset mouse position for next frame
			glfwSetCursorPos(window, screen_x/2, screen_y/2);
		
			// Compute new orientation
			angle += vec2(mouseSpeed * float(screen_x/2 - xpos ),  mouseSpeed * float( screen_y/2 - ypos ));
			//cout << horizontalAngle << ' ' << verticalAngle << endl;
			if (angle.y > 1.55f) {
				angle.y = 1.55f;
			}
			if (angle.y < -1.55) {
				angle.y = -1.55f;
			}
			if (angle.x > 6.28f) {
				angle.x = 0;
			}
			if (angle.x < 0) {
				angle.x = 6.28;
			}
			
			// Direction : Spherical coordinates to Cartesian coordinates conversion
			glm::vec3 direction(
				cos(angle.y) * sin(angle.x),
				sin(angle.y),
				cos(angle.y) * cos(angle.x)
			);
			pointing = direction;
			// Right vector
			glm::vec3 right = glm::vec3(
				sin(angle.x - 3.14f/2.0f),
				0,
				cos(angle.x - 3.14f/2.0f)
			);
			vec3 forward = -glm::vec3(
				-cos(angle.x - 3.14f/2.0f),
				0,
				sin(angle.x - 3.14f/2.0f)
			);
			
			
			// Up vector
			glm::vec3 up = glm::cross( right, forward );
			
			
			
			// Move forward
			if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
				position += forward * deltaTime * speed;
			}
			// Move backward
			if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
				position -= forward * deltaTime * speed;
			}
			// Strafe right
			if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
				position += right * deltaTime * speed;
			}
			// Strafe left
			if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
				position -= right * deltaTime * speed;
			}
			if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS){
				position += up * deltaTime * speed;
			}
			if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS){
				position -= up * deltaTime * speed;
			}
			
			if (mouse) {
				if (button == 0) {
					left_mouse();
				} else if (button == 1) {
					right_mouse();
				}
				mouse = false;
			}
			
			if (scroll != 0) {
				block_sel -= scroll;
				if (block_sel < 1) {
					block_sel = 1;
				} if (block_sel > blocks->num_blocks) {
					block_sel = blocks->num_blocks;
				}
				scroll = 0;
			}
			
			float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.
			
			
			// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
			ProjectionMatrix = glm::perspective(glm::radians(FoV), float(screen_x)/screen_y, 0.1f, 1000.0f);
			// Camera matrix
			ViewMatrix       = glm::lookAt(
										position,           // Camera is here
										position+direction, // and looks here : at the same position, plus "direction"
										up                  // Head is up (set to 0,-1,0 to look upside-down)
								   );
		
			// For the next frame, the "last time" will be "now"
			lastTime = currentTime;
		}
		
		void render_ui(MemVecs * uivecs) {
			///hearts
			draw_text(uivecs, blocks->blocks[block_sel]->name, -0.8f,-0.8f, 1);
			float scale = 0.1f;
			int i;
		}
};

SimplePlayer* player;


Block* new_block(int size) {
	Pixel pix(0,0,0,0,size,nullptr,nullptr);
	return pix.resolve([=] (ivec3 pos) {
		if ((pos.x == size-1 or pos.x == 0 or pos.z == size-1 or pos.z == 0 or
			((pos.x == size/2 or pos.x == size/2-1) and (pos.z == size/2 or pos.z == size/2-1))) and pos.y == 0) {
			return char(1);
		} else {
			return char(0);
		}
	});
}

Block* from_block_file(string path) {
	ifstream ifile(path, ios::binary);
	if (!ifile.good()) {
		cout << "File " << path << " does not exist" << endl;
		exit(1);
	}
	int size;
  ifile >> size;
  char buff;
  ifile.read(&buff,1);
  return Block::from_file(ifile, 0, 0, 0, size, nullptr, nullptr);
}

void to_block_file(string path) {
	ofstream ofile(path, ios::binary);
	ofile << worldblock->block->scale << ' ';
	worldblock->block->save_to_file(ofile);
}

Block* from_char_file(string path) {
	ifstream ifile(path, ios::binary);
	int size;
	ifile >> size;
	char data[size*size*size];
	string buff;
	getline(ifile, buff, ':');
	ifile.read(data, size*size*size);
	Pixel pix(0,0,0,0,size,nullptr,nullptr);
	return pix.resolve([&] (ivec3 pos) {
		return data[pos.x*size*size + pos.y*size + pos.z];
	});
}

void to_char_file(string path) {
	ofstream ofile(path, ios::binary);
	int size = worldblock->block->scale;
	char data[size*size*size];
	for (int x = 0; x < size; x ++) {
		for (int y = 0; y < size; y ++) {
			for (int z = 0; z < size; z ++) {
				data[x*size*size + y*size + z] = worldblock->get_global(x, y, z, 1)->get();
			}
		}
	}
	ofile << size << " :";
	ofile.write(data, size*size*size);
}



void make_ui_buffer(SimplePlayer* player, string debugstr, GLuint vertexbuffer, GLuint uvbuffer, GLuint matbuffer, int * num_tris) {
	MemVecs vecs;
	if (last_num_ui_verts != 0) {
		vecs.verts.reserve(last_num_ui_verts*3);
		vecs.uvs.reserve(last_num_ui_verts*2);
		vecs.mats.reserve(last_num_ui_verts);
	}
	render_debug(&vecs, debugstr);
	player->render_ui(&vecs);
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

// void mouse_button_call(GLFWwindow* window, int nbutton, int action, int mods) {
// 	if (action == GLFW_PRESS) {
// 		mouse = true;
// 		button = nbutton;
// 	}
// }


int main( int numargs, const char** args)
{
	//system("rmdir saves /s /q");
	min_ms_per_frame = 1000.0/max_fps;
	
	cout.precision(10);
	blocks = new BlockStorage();
	
	
	
	
	if (numargs < 4) {
		cout << "not enough arguments" << endl;
		cout << "usage: block_editor new|edit filepath [size]" << endl;
		return 1;
	}
	string worldpath = args[3];
	string type = args[2];
	string savepath = worldpath;
	string savetype = type;
	if (numargs == 6 and string(args[1]) != "new") {
		savepath = args[5];
		savetype = args[4];
	}
	if (string(args[1]) == "new") {
		stringstream ss(args[4]);
		int size;
		ss >> size;
		worldblock = new BlockContainer(new_block(size));
	} else if (string(args[1]) == "edit") {
		if (type == "block") {
			worldblock = new BlockContainer(from_block_file(worldpath));
		} else if (type == "char") {
			worldblock = new BlockContainer(from_char_file(worldpath));
		} else {
			cout << "invalid type" << endl;
			return 1;
		}
	} else {
		cout << "bad first arg" << endl;
		cout << "usage: block_editor new|edit filepath [size]" << endl;
		return 1;
	}
	
	rotation_enabled = savetype == "block";
	
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
	get_files_folder("resources/textures/ui", &uis);
	ifstream num_blocks_file("resources/textures/blocks/num_blocks.txt");
	num_blocks_file >> num_blocks;
	
	
	num_uis = uis.size() + 1;
	
	
	GLuint block_textures[num_blocks];
	GLuint ui_textures[num_uis];
	// Load the texture
	
	int size = 1;
	for (int i = 0; i < num_blocks; i ++) {
		string block = "resources/textures/blocks/" + std::to_string(size);
		block_textures[i] = loadBMP_array_folder(block);
		vector<string> files;
		get_files_folder(block, &files);
		for (string s : files) {
			cout << s << endl;
		}
	}
	
	for( int i = 0; i < uis.size(); i ++) {
		string ui = "resources/textures/ui/" + uis[i];
		ui_names[uis[i]] = i;
		const char* data = ui.c_str();
		ui_textures[i] = loadBMP_custom(data, true);
	}
	
	ui_textures[num_uis-1] = loadBMP_image_folder("resources/textures/items", true);
	ui_names["items.bmp"] = num_uis-1;
	
	// Load the texture
	//GLuint Texture = loadBMP_custom("scrolls/resources/blocks/dirt.bmp");
	//GLuint Texture = loadDDS("uvtemplate.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");
	GLuint uiTextureID  = glGetUniformLocation(uiProgram, "myTextureSampler");
	GLuint viewdistID = glGetUniformLocation(programID, "view_distance");
	GLuint clearcolorID = glGetUniformLocation(programID, "clear_color");
	GLuint sunlightID = glGetUniformLocation(programID, "sunlight");
	
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
	GLVecs glvecs;
	glvecs.set_buffers(vertexbuffer, uvbuffer, lightbuffer, matbuffer, 1000000);
	
	player = new SimplePlayer(vec3(0,0,0));
	
	//glGenBuffers(1, &vertexbuffer);
	//glGenBuffers(1, &uvbuffer);
	//glGenBuffers(1, &lightbuffer);
	//glGenBuffers(1, &matbuffer);
	
	//string level = menu(window, vertexbuffer, uvbuffer, matbuffer, uiVertexArrayID, uiTextureID, ui_textures, num_uis, uiProgram, "Select World:", worlds );
	//if (level == "") {
	//	return 0;
	//}
	
	
				
		
	
	
	
	
	
	double lastTime = glfwGetTime();
	double currentTime = lastTime;
	double lastFrameTime = lastTime;
	int nbFrames = 0;
	int fps;
	render_flag = true;
	bool reaching_max_fps = true;
	double slow_frame = 0;
	int view_distance = 800;
	bool saving = true;
	
	
	//make_vertex_buffer(vertexbuffer, uvbuffer, lightbuffer, &num_tris);
	while (playing) {
		
		
		
			//player->timestep();
			player->computeMatricesFromInputs();
		
		
		if (debug_visible) {
			debugstream.str("");
			debugstream << std::fixed;
			debugstream.precision(3);
			
			debugstream << "fps: " << fps << endl;
			debugstream << "x: " << player->position.x << "\ny: " << player->position.y << "\nz: " << player->position.z << endl;
			
			glvecs.status(debugstream);
			
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
			bool changed = worldblock->block->render_flag;
			bool[6] faces = {true, true, true, true, true, true};
			worldblock->block->render(&glvecs, &transparent_glvecs, worldblock, 0,0,0, 1, faces, true, false);
			
		}
		
		
		// Measure speed
		lastFrameTime = currentTime;
		currentTime = glfwGetTime();
		double ms = (currentTime-lastFrameTime)*1000;
		
		
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
		
		graphics.block_draw_call(world);
		
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
		
		
		
			if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
				glvecs.clean();
			}
		if (glfwGetKey(window, GLFW_KEY_O ) == GLFW_PRESS) {
			debug_visible = true;
		} else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
			debug_visible = false;
		} else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
			cout << "njbradley is king" << endl;
		} else if (glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS and glfwGetKey(window, 	GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) {
			playing = false;
		} else if (glfwGetKey(window, GLFW_KEY_X ) == GLFW_PRESS and glfwGetKey(window, 	GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) {
			playing = false;
			saving = false;
		} else if (glfwWindowShouldClose(window)) {
			playing = false;
		}
		
	}
	
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &TextureID);
	glDeleteVertexArrays(1, &VertexArrayID);
	
	
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	
	if (saving) {
		cout << "saving to " << savepath << endl;
		if (savetype == "block") {
			to_block_file(savepath);
		} else if (savetype == "char") {
			to_char_file(savepath);
		} else {
			cout << "invalid type" << endl;
			return 1;
		}
	} else {
		cout << "not saving changes!" << endl;
	}
	
	cout << "completed without errors" << endl;
}
