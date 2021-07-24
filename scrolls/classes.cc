#ifndef CLASSES
#define CLASSES

#include "classes.h"



//////////////////////// GLOBAL VARIABLES ////////////////////////
GLFWwindow* window;
Menu* menu;
const ivec3 dir_array[6] =    {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};


void wait() {
	//return;
	std::cout << "Press ENTER to continue...";
	std::cin.ignore( std::numeric_limits <std::streamsize> ::max(), '\n' );
}

void print(vec3 v);


int dir_to_index(ivec3 dir) {
	for (int i = 0; i < 6; i ++) {
		if (dir_array[i] == dir) {
			return i;
		}
	}
	return -1;
}

#endif
