#ifndef TEXTURE
#define TEXTURE

#include "classes.h"

GLuint loadBMP_custom(const char * imagepath, bool transparency);

GLuint loadBMP_image_folder(string dirpath, bool transparency);

GLuint loadBMP_array_folder(string dirpath, bool transparency = false, GLint colorformat = 0);

GLuint loadBMP_edge_combos(string dirpath, int* num_edges);

GLuint loadBMP_array_custom(const char * imagepath);

#endif
