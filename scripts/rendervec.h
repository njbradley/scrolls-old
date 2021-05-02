#ifndef RENDERVEC_PREDEF
#define RENDERVEC_PREDEF

#include "classes.h"
#include <ctime>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <list>

#pragma pack(push, 0)

struct RenderPosPart {
  vec3 pos;
  vec4 rot;
  float scale;
};

struct RenderPosData {
  union {
    RenderPosPart loc;
    GLfloat data[8];
  };
};

struct RenderFaceData {
  uint tex;
  uint8 flags;
  uint8 rot;
  uint8 blocklight;
  uint8 sunlight;
};

struct RenderTypeData {
  union {
    struct {
      RenderFaceData faces[6];
    };
    GLuint data[12];
  };
};
  

struct RenderData {
  RenderPosData pos;
  RenderTypeData type;
};

#pragma pack(pop)

class RenderIndex { public:
  int index;
  
  RenderIndex(int newindex);
  RenderIndex();
  
  static int to_vert_index(int num);
  static int to_data_index(int num);
  static const int vert_size = 8;
  static const int data_size = 12;
  
  int vert_index() const;
  int data_index() const;
  
  bool isnull() const;
  static const RenderIndex npos;
};

class RenderVecs {
  // the base class that holds all of the render vectors
public:
  virtual RenderIndex add(RenderData data) = 0;
  virtual void del(RenderIndex) = 0;
  virtual void edit(RenderIndex, RenderData data) = 0;
};

/*
class MemVecs: public RenderVecs {/*
      this is a class that encapsulates all of the vectors needed for rendering a face,
      vertices, uv coords, the lightlevel, and the material. values are pushed onto the
      vectors for every face.
    //
    public:
      vector<RenderData> alldata;
      int num_verts = 0;
      
      RenderIndex add(RenderData data);
      void del(RenderIndex);
      void edit(RenderIndex index, RenderData);
      void clear();
};*/

class MemVecs {/*
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
      /*
      void add_face(GLfloat* newverts, GLfloat* newuvs, GLfloat sunlight,
      GLfloat blocklight, GLint minscale, GLint breaking, GLint overlay, GLint edges[4], GLint mat);/*
        adds a face to the vectors, newverts and newuvs are arrays, 3*6 and 2*6 length respectivly
      void add(MemVecs* newvecs);
      void del(RenderIndex);
      void edit(RenderIndex index, MemVecs* newvecs);
      void clear();*/
};

class AsyncGLVecs: public RenderVecs { public:
  GLVecsDestination* destination;
  
  int num_verts = 0;
  int offset = 0;
  bool ignore = false;
  
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
  


#endif
