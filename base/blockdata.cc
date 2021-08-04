#ifndef BLOCKDATA
#define BLOCKDATA

#include "blockdata.h"



void BlockData::set_id(int newid) {
  id = newid;
  if (idref != nullptr) {
    *idref = newid;
  }
}

BlockStorage::BlockStorage() {
  for (int i = 0; i < blocks.size(); i ++) {
    blocks[i]->set_id(i+1);
  }
}

BlockData* BlockStorage::operator[] (int index) {
  return blocks[index];
}

BlockData* BlockStorage::operator[] (string name) {
  for (BlockData* data : blocks) {
    if (data->name == name) {
      return data;
    }
  }
  return nullptr;
}

Plugin<BlockStorage> blockstorage;

#endif
