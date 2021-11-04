#include "player.h"
#include "graphics.h"

static int scroll_rel_val = 0;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	scroll_rel_val += yoffset;
}

GLControls::GLControls() {
	glfwSetScrollCallback(window, scroll_callback);
	
	KEY_SHIFT = GLFW_KEY_LEFT_SHIFT;
	KEY_CTRL = GLFW_KEY_LEFT_CONTROL;
	KEY_ALT = GLFW_KEY_LEFT_ALT;
	KEY_TAB = GLFW_KEY_TAB;
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

int GLControls::scroll_rel() {
	int val = scroll_rel_val;
	scroll_rel_val = 0;
	return val;
}

EXPORT_PLUGIN(GLControls);
