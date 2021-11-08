#ifndef BLOCKDATA_PREDEF
#define BLOCKDATA_PREDEF

#include "classes.h"
#include "plugins.h"

struct BlockDataParams {
  string name;
  Material* material;
  int texture[6] = {0,0,0,0,0,0};
  string texture_names[6];
  int default_direction = 0;
  bool rotation_enabled = false;
  int lightlevel = 0;
  bool transparent = false;
  float density = 1;
};
  

class BlockData : public BlockDataParams { public:
  BASE_PLUGIN_HEAD(BlockData, ());
  
  Blocktype id;
  
  BlockData(BlockDataParams&& data);
  void set_id(Blocktype newid);
  virtual ~BlockData() {}
};

class BlockStorage : public Storage<BlockData> { public:
  void init();
  BlockData* operator[](int index);
  virtual ~BlockStorage() {}
};

extern BlockStorage blockstorage;

namespace blocktypes {
  extern BlockData dirt;
  extern BlockData grass;
  extern BlockData bark;
  extern BlockData wood;
  extern BlockData leaves;
  extern BlockData water;
}

#endif
