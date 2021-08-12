#include "graphics.h"
#include "rendervecs.h"
#include "base/settings.h"
#include "shader.h"
#include "texture.h"
#include "base/ui.h"

GLGraphicsContext::GLGraphicsContext(): blocktex("textures/blocks"), transblocktex("textures/blocks/transparent"), uitex("textures/ui") {
	init_graphics();
	load_textures();
}

GLGraphicsContext::~GLGraphicsContext() {
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &databuffer);
	
	glDeleteBuffers(1, &uibuffer);
	
	glDeleteProgram(block_program);
	glDeleteProgram(ui_program);
	
	glDeleteTextures(1, &blocktex_id);
	glDeleteTextures(1, &transblocktex_id);
	glDeleteTextures(1, &uitex_id);
	glDeleteVertexArrays(1, &block_vertexid);
	glDeleteVertexArrays(1, &ui_vertexid);
}

void GLGraphicsContext::init_graphics() {
	
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
		settings->screen_dims = ivec2(return_struct->width, return_struct->height);
		//set_screen_dims(return_struct->width, return_struct->height);
		window = glfwCreateWindow( settings->screen_dims.x, settings->screen_dims.y, "Scrolls - An Adventure Game", glfwGetPrimaryMonitor(), nullptr);
	} else {
		// Open a window and create its OpenGL context
		window = glfwCreateWindow( settings->screen_dims.x, settings->screen_dims.y, "Scrolls - An Adventure Game", nullptr, nullptr);
	}
	glfwSetWindowPos(window, 100, 40);
	
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
  glfwSetCursorPos(window, settings->screen_dims.x/2, settings->screen_dims.y/2);

	// Dark blue background
	vec3 clearcolor = vec3(0.4f, 0.7f, 1.0f);
	glClearColor(clearcolor.x, clearcolor.y, clearcolor.z, 0.0f);
	
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	// glDisable(GL_CULL_FACE);
	glGenVertexArrays(1, &block_vertexid);
	
	glGenVertexArrays(1, &ui_vertexid);
	// Create and compile our GLSL program from the shaders
	block_program = LoadShadersGeo( RESOURCES_PATH "shaders/block.vs", RESOURCES_PATH "shaders/block.fs", RESOURCES_PATH "shaders/block.gs" );
	ui_program = LoadShaders( RESOURCES_PATH "shaders/ui.vs", RESOURCES_PATH "shaders/ui.fs" );
	
	
	pMatID = glGetUniformLocation(block_program, "Pmat");
	mvMatID = glGetUniformLocation(block_program, "MVmat");
	blockTextureID  = glGetUniformLocation(block_program, "myTextureSampler");
	uiTextureID  = glGetUniformLocation(ui_program, "myTextureSampler");
	viewdistID = glGetUniformLocation(block_program, "view_distance");
	clearcolorID = glGetUniformLocation(block_program, "clear_color");
	sunlightID = glGetUniformLocation(block_program, "sunlight");
	suncolorID = glGetUniformLocation(block_program, "suncolor");
	breakingTexID = glGetUniformLocation(block_program, "breakingTex");
	overlayTexID = glGetUniformLocation(block_program, "overlayTex");
	edgesTexID = glGetUniformLocation(block_program, "edgesTex");
	
	glBindVertexArray(ui_vertexid);
	glGenBuffers(1, &uibuffer);
	
	glBindVertexArray(block_vertexid);
	GLuint blockbuffs[2];
	glGenBuffers(2, blockbuffs);
	vertexbuffer = blockbuffs[0];
	databuffer = blockbuffs[1];
	
	int total_size = settings->allocated_memory * 1000000 / sizeof(RenderData);
	glvecsdest.set_buffers(vertexbuffer, databuffer, total_size);
	glvecs.set_destination(&glvecsdest);
	gltransvecs.set_destination_offset(&glvecsdest, total_size - total_size / 10);
}

void GLGraphicsContext::set_camera(vec3* pos, vec2* rot) {
	camera_pos = pos;
	camera_rot = rot;
}

const PathLib* GLGraphicsContext::block_textures() const {
	return &blocktex;
}

const PathLib* GLGraphicsContext::trans_block_textures() const {
	return &transblocktex;
}

const PathLib* GLGraphicsContext::ui_textures() const {
	return &uitex;
}


RenderVecs* GLGraphicsContext::blockvecs() {
	return &glvecs;
}

RenderVecs* GLGraphicsContext::transvecs() {
	return &gltransvecs;
}

UIVecs* GLGraphicsContext::uivecs() {
	return &gluivecs;
}

void GLGraphicsContext::load_textures() {
	
	blocktex_id = loadBMP_array_folder(block_textures(), false);
	transblocktex_id = loadBMP_array_folder(trans_block_textures(), true);
	
	ui_atlas.resize(ui_textures()->size());
	uitex_id = loadBMP_pack_folder(ui_textures(), &ui_atlas.front(), 1024, 1024);
	
	gluivecs.set_atlas(&ui_atlas.front());
}

void GLGraphicsContext::block_draw_call() {
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBindVertexArray(block_vertexid);
	
	// Use our shader
	glUseProgram(block_program);
	
	vec3 direction(
		cos(camera_rot->y) * sin(camera_rot->x),
		sin(camera_rot->y),
		cos(camera_rot->y) * cos(camera_rot->x)
	);
	
	vec3 right = glm::vec3(
		sin(camera_rot->x - 3.14f/2.0f),
		0,
		cos(camera_rot->x - 3.14f/2.0f)
	);
	vec3 forward = -glm::vec3(
		-cos(camera_rot->x - 3.14f/2.0f),
		0,
		sin(camera_rot->x - 3.14f/2.0f)
	);
	
	vec3 up = glm::cross( right, forward );
	
	
	// Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 projectionMatrix = glm::perspective(glm::radians(settings->fov), (float) settings->aspect_ratio(), 0.1f, 1000.0f);
	glm::mat4 viewMatrix = glm::lookAt(*camera_pos, *camera_pos + direction, up);
	
	glm::mat4 modelMatrix = glm::mat4(1.0);
	glm::mat4 P = projectionMatrix;
	glm::mat4 MV = viewMatrix * modelMatrix;
	
	glUniformMatrix4fv(pMatID, 1, GL_FALSE, &P[0][0]);
	glUniformMatrix4fv(mvMatID, 1, GL_FALSE, &MV[0][0]);
	
	ivec3 sunlightdir = vec3(MV * vec4(0, -1, 0, 0));
	float sunlight = 1;
	// Send our transformation to the currently bound shader,
	// in the "MVP" uniform
	
	vec3 clearcolor = viewbox->params.clear_color;
	glUniform3f(clearcolorID, clearcolor.x, clearcolor.y, clearcolor.z);
	glClearColor(clearcolor.x * sunlight, clearcolor.y * sunlight, clearcolor.z * sunlight, 0.0f);
	glUniform1i(viewdistID, viewbox->params.view_distance);
	
	
	glUniform3f(sunlightID, viewbox->params.sun_direction.x, viewbox->params.sun_direction.y, viewbox->params.sun_direction.z);
	glUniform3f(suncolorID, viewbox->params.sun_color.x, viewbox->params.sun_color.y, viewbox->params.sun_color.z);
	
	//// Vertex attribures : position, rotation, and scale
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RenderPosData), (void*) offsetof(RenderPosPart, pos));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(RenderPosData), (void*) offsetof(RenderPosPart, rot));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(RenderPosData), (void*) offsetof(RenderPosPart, scale));
	
	// Face data
	glBindBuffer(GL_ARRAY_BUFFER, databuffer);
	for (int i = 0; i < 6; i ++) {
		glEnableVertexAttribArray(3+i);
		glVertexAttribIPointer(3+i, 2, GL_UNSIGNED_INT, sizeof(RenderTypeData), (void*) (offsetof(RenderTypeData, faces) + i * sizeof(RenderFaceData)));
	}
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, blocktex_id);
	glUniform1i(blockTextureID, 0);
	
	glDrawArrays(GL_POINTS, glvecs.offset, glvecs.num_verts);
	
	/// Transparent blocks
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, transblocktex_id);
	glUniform1i(blockTextureID, 0);
	
	glDrawArrays(GL_POINTS, gltransvecs.offset, gltransvecs.num_verts);
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
}
	
void GLGraphicsContext::ui_draw_call() {
	glBindBuffer(GL_ARRAY_BUFFER, uibuffer);
	glBufferData(GL_ARRAY_BUFFER, gluivecs.num_verts * sizeof(GLfloat), gluivecs.data(), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uibuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, (void*) (sizeof(GLfloat)*1));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, (void*) (sizeof(GLfloat)*3));
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(GLfloat)*5, (void*) (0));
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, uitex_id);
	glUniform1i(uiTextureID, 0);
	
	glDrawArrays(GL_TRIANGLES, 0, gluivecs.num_verts); // 12*3 indices starting at 0 -> 12 triangles
	
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	
	gluivecs.clear();
}

void GLGraphicsContext::swap() {
	glfwSwapBuffers(window);
	glfwPollEvents();
}



EXPORT_PLUGIN(GLGraphicsContext);
