#ifndef RENDERVEC_PREDEF
#define RENDERVEC_PREDEF

#include "classes.h"
#include "plugins.h"
#include <ctime>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <list>

#pragma pack(push, 0)

struct RenderPosPart {
  vec3 pos;
  quat rot;
  float scale;
};

struct RenderPosData {
  union {
    RenderPosPart loc;
    float data[8];
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
    uint data[12];
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

class RenderVecs { public:
  BASE_PLUGIN_HEAD(RenderVecs, ());
  // the base class that holds all of the render vectors
  int num_verts = 0;
  int offset = 0;
  bool ignore = false;
  
  virtual RenderIndex add(RenderData data) = 0;
  virtual void del(RenderIndex) = 0;
  virtual void edit(RenderIndex, RenderData data) = 0;
};
  
class UIVecs { public:
  BASE_PLUGIN_HEAD(UIVecs, ());
  int num_verts = 0;
	
	virtual void add(UIRect rect) = 0;
	virtual void add(UIObj* obj);
	virtual void clear() = 0;
};

#endif
