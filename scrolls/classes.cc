#ifndef CLASSES
#define CLASSES

#include "classes.h"



//////////////////////// GLOBAL VARIABLES ////////////////////////
GLFWwindow* window;
Menu* menu;


void wait() {
	//return;
	std::cout << "Press ENTER to continue...";
	std::cin.ignore( std::numeric_limits <std::streamsize> ::max(), '\n' );
}

void print(vec3 v);



#endif
