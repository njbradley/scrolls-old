#ifndef CONTROLLERS_PREDEF
#define CONTROLLERS_PREDEF

#include "classes.h"

class Controller { public:
	Controllable* mount;
	Controller(Controllable* newmount);
};

class KeyController : public Controller { public:
	static map<GLFWwindow*,Controller*> controllers;
	KeyController(Controllable* mount);
	void timestep(double deltatime);
};
	

class Controllable { public:
	virtual void force(ivec3 vel) = 0;
	virtual void rotate(ivec2 rot_offset) = 0;
	virtual void keydown(char key);
};

#endif
