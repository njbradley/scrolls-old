#ifndef BLOCKDATA
#define BLOCKDATA

#include "blockdata.h"



void BlockData::set_id(Blocktype newid) {
  id = newid;
  if (idref != nullptr) {
    *idref = newid;
  }
}

BlockStorage::BlockStorage() {
  for (int i = 0; i < library.size(); i ++) {
    library[i]->set_id(i+1);
  }
}


Plugin<BlockStorage> blockstorage;

Blocktype Block_DIRT;
Blocktype Block_GRASS;
Blocktype Block_BARK;
Blocktype Block_WOOD;
Blocktype Block_LEAVES;
Blocktype Block_WATER;


#endif
