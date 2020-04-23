#ifndef RENDERVEC_PREDEF
#define RENDERVEC_PREDEF
#include "classes.h"
#include <iostream>
#include <GL/glew.h>
#include <ctime>

class RenderVecs {/*
      this is a class that encapsulates all of the vectors needed for rendering a face,
      vertices, uv coords, the lightlevel, and the material. values are pushed onto the
      vectors for every face.
    */
    public:
      vector<GLfloat> verts;
    	vector<GLfloat> uvs;
    	vector<GLfloat> light;
    	vector<GLint> mats;
      int num_verts = 0;
      
      void add_face(GLfloat* newverts, GLfloat* newuvs, GLfloat newlight, GLint minscale, GLint mat);/*
        adds a face to the vectors, newverts and newuvs are arrays, 3 and 2 length respectivly
      */
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
