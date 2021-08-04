#ifndef BLOCKDATA_PREDEF
#define BLOCKDATA_PREDEF

#include "classes.h"
#include "plugins.h"



class BlockData { public:
  PLUGIN_HEAD(BlockData, ());
  
  int id;
  int* idref = nullptr;
  Material* material;
  int texture[6];
  int default_direction = 0;
  bool rotation_enabled = false;
  string name;
  int lightlevel = 0;
  bool transparent = false;
  
  BlockData() {}
  void set_id(int newid);
  virtual ~BlockData() {}
};

class BlockStorage { public:
  PLUGIN_HEAD(BlockStorage, ());
  
  PluginList<BlockData> blocks;
  
  BlockData* operator[] (int index);
  BlockData* operator[] (string name);
  
  BlockStorage();
  virtual ~BlockStorage() {}
};

extern Plugin<BlockStorage> blockstorage;

#endif
