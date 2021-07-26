#ifndef TEXTURE
#define TEXTURE

#include "classes.h"

GLuint loadBMP_custom(const char * imagepath, bool transparency);

GLuint loadBMP_image_folder(const PathLib* img_paths, bool transparency);

GLuint loadBMP_array_folder(const PathLib* img_paths, bool transparency = false, GLint colorformat = 0);

GLuint loadBMP_array_custom(const char * imagepath);

#endif
