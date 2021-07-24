#ifndef CLASSES_H
#define CLASSES_H

#include "base/classes.h"

void wait();
void print(vec3 v);

int dir_to_index(ivec3 dir);

//ofstream dfile("debug.txt");
extern int dloc;

////////////////// CLASSES ///////////////////////////


//////////////////////// GLOBAL VARIABLES ////////////////////////
extern GLFWwindow* window;
extern Menu* menu;


extern const ivec3 dir_array[6];

#endif
