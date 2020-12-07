#ifndef RENDERVEC_PREDEF
#define RENDERVEC_PREDEF

#include "classes.h"
#include <ctime>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <list>


class RenderIndex { public:
  int start;
  int size;
  RenderIndex(int newindex, int newsize);
  RenderIndex(pair<int,int> pairindex);
  RenderIndex();
  
  static int to_vert_index(int num);
  static int to_uv_index(int num);
  static int to_light_index(int num);
  static int to_mat_index(int num);
  
  int vert_start() const;
  int uv_start() const;
  int light_start() const;
  int mat_start() const;
  int vert_size() const;
  int uv_size() const;
  int light_size() const;
  int mat_size() const;
  int vert_end() const;
  int uv_end() const;
  int light_end() const;
  int mat_end() const;
  int end() const;
  
  bool isnull() const;
  static const RenderIndex npos;
  
  bool can_add(RenderIndex other) const;
  RenderIndex add(RenderIndex other);
  bool can_subtract(RenderIndex other) const;
  RenderIndex subtract(RenderIndex);
};

class RenderVecs {
  // the base class that holds all of the render vectors
public:
  virtual RenderIndex add(MemVecs* newvecs) = 0;
  virtual void del(RenderIndex) = 0;
  virtual void edit(RenderIndex, MemVecs*) = 0;
};

class MemVecs: public RenderVecs {/*
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
      
      void add_face(GLfloat* newverts, GLfloat* newuvs, GLfloat sunlight, GLfloat blocklight, GLint minscale, GLint mat);/*
        adds a face to the vectors, newverts and newuvs are arrays, 3*6 and 2*6 length respectivly
      */
      RenderIndex add(MemVecs* newvecs);
      void del(RenderIndex);
      void edit(RenderIndex index, MemVecs* newvecs);
      void clear();
};

/*
class GLVecs: public RenderVecs {
    public:
        GLuint vertexbuffer;
        GLuint uvbuffer;
        GLuint lightbuffer;
        GLuint matbuffer;
        vector<pair<int,int>> empty;
        int size_alloc;
        int offset = 0;
        int num_verts = 0;
        std::timed_mutex writelock;
        bool clean_flag = false;
        
        void set_buffers(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size);
        void set_buffers_prealloc(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size, int newoffset);
        RenderIndex add(MemVecs* newvecs);
        void del(pair<int,int> index);
        void clean();
        void edit( pair<int,int> index, MemVecs* newvecs);
        void status(std::stringstream & message);
};*/

class AsyncGLVecs: public RenderVecs { public:
  class Change { public:
    RenderIndex index;
    MemVecs vecs;
    Change(RenderIndex newindex);
    Change(RenderIndex newindex, MemVecs vecs);
  };
  class Empty { public:
    RenderIndex index;
    std::list<Change>::iterator change;
  };
  
  GLuint vertexbuffer;
  GLuint uvbuffer;
  GLuint lightbuffer;
  GLuint matbuffer;
  
  int size_alloc;
  int offset = 0;
  int num_verts = 0;
  
  std::mutex addlock;
  std::mutex emptyaddlock;
  std::shared_mutex synclock;
  
  std::list<Change> changes;
  std::list<Empty> emptys;
  
  void set_buffers(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size);
  void set_buffers_prealloc(GLuint verts, GLuint uvs, GLuint light, GLuint mats, int start_size, int newoffset);
  RenderIndex add(MemVecs* newvecs);
  void del(RenderIndex index);
  void sync();
  void edit(RenderIndex index, MemVecs* newvecs);
  void status(std::ostream& ofile);
};


#endif
