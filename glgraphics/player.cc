#include "player.h"
#include "graphics.h"

GLControls::GLControls() {
	
}


bool GLControls::key_pressed(int keycode) {
	return glfwGetKey(window, keycode) == GLFW_PRESS;
}

bool GLControls::mouse_button(int button) {
	return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

ivec2 GLControls::mouse_pos() {
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	return ivec2(x,y);
}

void GLControls::mouse_pos(ivec2 newpos) {
	glfwSetCursorPos(window, newpos.x, newpos.y);
}

EXPORT_PLUGIN(GLControls);
