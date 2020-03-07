#ifndef RENDERVEC_PREDEF
#define RENDERVEC_PREDEF
#include "classes.h"
#include <iostream>
#include <GL/glew.h>
#include <ctime>

class RenderVecs {
    public:
      vector<GLfloat> verts;
    	vector<GLfloat> uvs;
    	vector<GLfloat> light;
    	vector<GLint> mats;
      int num_verts = 0;
      
      void add_face(GLfloat* newverts, GLfloat* newuvs, GLfloat newlight, GLint minscale, GLint mat);
};


class GLVecs {
    public:
        GLuint vertexbuffer = 4545;
        GLuint uvbuffer;
        GLuint lightbuffer;
        GLuint matbuffer;
        vector<pair<int,int>> empty;
        int size_alloc;
        int num_verts = 0;
        
        void set_buffers(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size);
        pair<int,int> add(RenderVecs* newvecs);
        void del(pair<int,int> index);
        void clean();
        void edit( pair<int,int> index, RenderVecs* newvecs);
        void status(std::stringstream & message);
};
#endif
