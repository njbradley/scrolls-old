#ifndef GL_SHADER_PREDEF
#define GL_SHADER_PREDEF

#include "classes.h"

GLuint LoadShadersGeo(const char * vertex_file_path,const char * fragment_file_path, const char* geometry_path);
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

#endif
