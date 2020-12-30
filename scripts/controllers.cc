#ifndef CONTROLLERS
#define CONTROLLERS

Controller::Controller(Controllable* newmount): mount(newmount) {
	
}

char key = 0;

void key_callback(GLFWwindow* window, int newkey, int scancode, int action, int mods) {
	key = newkey;
	KeyControllers::controllers[window]->mount->keydown(newkey);
}

KeyController::KeyController(Controllable* mount, GLFWwindow* window): Controller(mount) {
  glfwSetKeyCallback(window, key_callback);
	controllers[window] = this;
}

void KeyController::timestep(double deltatime) {
	
}

#endif
