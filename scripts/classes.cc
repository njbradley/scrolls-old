#ifndef CLASSES
#define CLASSES

#include "classes.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>
#include <unordered_map>
#include <unordered_set>
using std::unordered_map;
using glm::vec3;
using glm::ivec3;
using glm::vec2;
using glm::ivec2;
using std::cout;
using std::endl;
using std::string;
using std::function;
using std::unordered_set;
using std::stringstream;
using std::ifstream;
using std::ofstream;
using std::istream;
using std::ostream;



//////////////////////// GLOBAL VARIABLES ////////////////////////
Game* game;
GLFWwindow* window;
Menu* menu;
const ivec3 dir_array[6] =    {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};


void wait() {
	//return;
	std::cout << "Press ENTER to continue...";
	std::cin.ignore( std::numeric_limits <std::streamsize> ::max(), '\n' );
}

void print(vec3 v);

bool ivec3_comparator::operator() (const ivec3& lhs, const ivec3& rhs) const
{
    return lhs.x < rhs.x || lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
}

bool vec3_comparator::operator() (const vec3& lhs, const vec3& rhs) const
{
    return lhs.x < rhs.x || lhs.x == rhs.x && (lhs.y < rhs.y || lhs.y == rhs.y && lhs.z < rhs.z);
}


size_t ivec3_hash::operator() (const ivec3& pos) const {
  std::hash<int> ihash;
	return ihash(pos.x) ^ ihash(pos.y) ^ ihash(pos.z);
}

size_t ivec4_hash::operator() (const ivec4& pos) const {
  std::hash<int> ihash;
	return ihash(pos.x) ^ ihash(pos.y) ^ ihash(pos.z) ^ ihash(pos.w);
}

//ofstream dfile("debug.txt");
int dloc = -69;

ostream& operator<<(ostream& out, ivec3 pos) {
	out << pos.x << ',' << pos.y << ',' << pos.z;
	return out;
}

ostream& operator<<(ostream& out, vec3 pos) {
	out << pos.x << ',' << pos.y << ',' << pos.z;
	return out;
}

#endif
