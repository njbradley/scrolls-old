#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include "classes.h"

GLuint loadBMP_custom(const char * imagepath, bool transparency);

GLuint loadBMP_image_folder(const PathLib* img_paths, bool transparency);

GLuint loadBMP_array_folder(const PathLib* img_paths, bool transparency = false, GLint colorformat = 0);

GLuint loadBMP_array_custom(const char * imagepath);

GLuint loadBMP_pack_folder(const PathLib* img_paths, UIRect* ui_atlas, int max_widths, int max_height);

#endif
