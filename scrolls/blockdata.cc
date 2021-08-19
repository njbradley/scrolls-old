#ifndef BLOCKDATA
#define BLOCKDATA

#include "blockdata.h"
#include "graphics.h"
#include "libraries.h"
#include "materials.h"

DEFINE_PLUGIN(BlockData);
DEFINE_PLUGIN(BlockStorage);

BlockData::BlockData(BlockDataParams&& data): BlockDataParams(data) {
  
}


void BlockData::set_id(Blocktype newid) {
  id = newid;
  
  int last = 0;
  for (int i = 0; i < 6; i ++) {
    if (texture[i] == 0) {
      if (texture_names[i] != "") {
        if (transparent) {
          last = graphics->trans_block_textures()->getindex(texture_names[i]);
        } else {
          last = graphics->block_textures()->getindex(texture_names[i]);
        }
      }
      texture[i] = last;
    }
  }
}

void BlockStorage::init() {
  Storage<BlockData>::init();
  for (int i = 0; i < size(); i ++) {
    pointers[i]->set_id(i+1);
  }
}

BlockData* BlockStorage::operator[](int index) {
  return pointers[index-1];
}

BlockStorage blockstorage;

namespace blocktypes {
  BlockData dirt ({
    .name = "dirt",
    .material = &materials::dirt,
    .texture_names = {"dirt.bmp"},
  });
  
  BlockData grass ({
    .name = "grass",
    .material = &materials::dirt,
    .texture_names = {"grass.bmp"},
  });
  
  BlockData bark ({
    .name = "wood",
    .material = &materials::dirt,
    .texture_names = {"bark.bmp"},
  });
  
  BlockData wood ({
    .name = "wood",
    .material = &materials::dirt,
    .texture_names = {"wood.bmp"},
  });
  
  BlockData leaves ({
    .name = "leaves",
    .material = &materials::leaves,
    .texture_names = {"leaves.bmp"},
  });
  
  BlockData water ({
    .name = "water",
    .material = &materials::leaves,
    .texture_names = {"water.bmp"},
    .transparent = true,
  });
  
  EXPORT_PLUGIN_SINGLETON(dirt);
  EXPORT_PLUGIN_SINGLETON(grass);
  EXPORT_PLUGIN_SINGLETON(wood);
  EXPORT_PLUGIN_SINGLETON(leaves);
}


#endif
