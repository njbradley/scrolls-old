#ifndef GL_RENDERVECS_H
#define GL_RENDERVECS_H

#include "classes.h"
#include "base/rendervecs.h"

class AsyncGLVecs: public RenderVecs { public:
  GLVecsDestination* destination;
  
  std::mutex addlock;
  std::mutex emptyaddlock;
  std::shared_mutex synclock;
  
  unordered_set<int> emptys;
  unordered_map<int,RenderData> changes;
  
  void set_destination(GLVecsDestination* dest);
  void set_destination_offset(GLVecsDestination* dest, int newoffset);
  RenderIndex add(RenderData data);
  void del(RenderIndex);
  void edit(RenderIndex index, RenderData);
  void sync();
  void status(std::ostream& ofile);
};

class GLVecsDestination { public:
  GLuint vertexbuffer;
  GLuint databuffer;
  
  int size_alloc;
  int offset = 0;
  
  void set_buffers(GLuint verts, GLuint datas, int start_size);
  void write(RenderIndex index, RenderData data);
};


class GLUIVecs : public UIVecs { public:
  vector<GLfloat> datavec;
  
  virtual void add(UIRect rect);
  virtual void clear();
  
  virtual void* data();
};

#endif
