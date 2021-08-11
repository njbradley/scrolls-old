#ifndef BLOCKDATA_PREDEF
#define BLOCKDATA_PREDEF

#include "classes.h"
#include "plugins.h"



class BlockData { public:
  PLUGIN_HEAD(BlockData, ());
  
  Blocktype id;
  Blocktype* idref = nullptr;
  Material* material;
  int texture[6];
  int default_direction = 0;
  bool rotation_enabled = false;
  string name;
  int lightlevel = 0;
  bool transparent = false;
  
  BlockData() {}
  void set_id(Blocktype newid);
  virtual ~BlockData() {}
};

class BlockStorage : public Storage<BlockData> { public:
  PLUGIN_HEAD(BlockStorage, ());
  
  BlockStorage();
  virtual ~BlockStorage() {}
};

extern Plugin<BlockStorage> blockstorage;

extern Blocktype Block_DIRT;
extern Blocktype Block_GRASS;
extern Blocktype Block_BARK;
extern Blocktype Block_WOOD;
extern Blocktype Block_LEAVES;
extern Blocktype Block_WATER;

#endif
