#ifndef BLOCKITER
#define BLOCKITER

#include "blockiter.h"
#include "blocks.h"
#include "world.h"
#include "debug.h"
#include "player.h"

template <typename BlockT>
BlockIter<BlockT>::iterator::iterator(BlockIter<BlockT>* parent): curblock(parent->base) {
  if (curblock != nullptr) {
    max_scale = curblock->scale;
  } else {
    max_scale = 1;
  }
}

template <typename BlockT>
ivec3 BlockIter<BlockT>::iterator::increment_func(ivec3 pos) {
  pos.z++;
  if (pos.z > endpos().z) {
    pos.z = startpos().z;
    pos.y ++;
    if (pos.y > endpos().y) {
      pos.y = startpos().y;
      pos.x ++;
    }
  }
  return pos;
}

template <typename BlockT>
void BlockIter<BlockT>::iterator::step_down(BlockT* parent, ivec3 curpos, BlockT* block) {
  // cout << "  Step down " << parent << ' ' << curpos << ' ' << block << endl;
  if (block->continues) {
    // cout << "   going down " << endl;
    get_safe(block, startpos(), block->get(startpos()));
  } else {
    // cout << "   going sideways " << endl;
    step_sideways(parent, curpos, block);
  }
}

template <typename BlockT>
void BlockIter<BlockT>::iterator::step_sideways(BlockT* parent, ivec3 curpos, BlockT* block) {
  // cout << "  Step sideways " << parent << ' ' << curpos << ' ' << block << endl;
  if (parent == nullptr or parent->scale > max_scale) {
    // cout << "   DONE " << endl;
    finish();
  } else if (curpos == endpos()) {
    // cout << "   Going up " << endl;
    step_sideways(parent->parent, parent->parentpos, parent);
  } else {
    // cout << "   normal increment " << endl;
    curpos = increment_func(curpos);
    get_safe(parent, curpos, parent->get(curpos));
  }
}

template <typename BlockT>
void BlockIter<BlockT>::iterator::get_safe(BlockT* parent, ivec3 curpos, BlockT* block) {
  // cout << "  get safe " << parent << ' ' << curpos << ' ' << block << endl;
  if (!valid_block(block)) {
    // cout << "   invalid block " << endl;
    step_sideways(parent, curpos, block);
  } else if (skip_block(block)) {
    // cout << "   skipping block " << block << ' ' << block->continues << ' ' << block->globalpos << ' ' << skip_block(block) << endl;
    skip_block(block);
    step_down(parent, curpos, block);
  } else {
    curblock = block;
  }
}


template <typename BlockT>
typename BlockIter<BlockT>::iterator BlockIter<BlockT>::iterator::operator++() {
  if (curblock != nullptr) {
    step_down(curblock->parent, curblock->parentpos, curblock);
  }
  return *this;
}

template <typename BlockT>
BlockT* BlockIter<BlockT>::iterator::operator*() {
  return curblock;
}

template <typename BlockT>
bool BlockIter<BlockT>::iterator::operator != (const typename BlockIter<BlockT>::iterator& other) const {
  return curblock != other.curblock or max_scale != other.max_scale;
}



template class BlockIter<Block>;
template class BlockIter<const Block>;





template <typename BlockT>
bool PixelIter<BlockT>::iterator::skip_block(BlockT* block) {
  return block->continues;
}

template <typename BlockT>
typename PixelIter<BlockT>::PixelT* PixelIter<BlockT>::iterator::operator*() {
  return this->curblock->pixel;
}



template class PixelIter<Block>;
template class PixelIter<const Block>;




template class DirPixelIter<Block>;
template class DirPixelIter<const Block>;









FreeBlockIter::FreeBlockIter(Collider* world, Hitbox newbox): box(newbox) {
  increment_func = [] (ivec3 pos, ivec3 startpos, ivec3 endpos) {
    pos.z++;
    if (pos.z > endpos.z) {
      pos.z = startpos.z;
      pos.y ++;
      if (pos.y > endpos.y) {
        pos.y = startpos.y;
        pos.x ++;
      }
    }
    return pos;
  };
  
  // cout << endl << endl << "Freeblockiter " << newbox << endl;
  // debuglines->clear();
  // debuglines->render(newbox);
  
  vec3 points[8];
  box.points(points);
  
  Block* blocks[8];
  
  int num_removed = 0;
	for (int i = 0; i < 8; i ++) {
    ivec3 pointpos = SAFEFLOOR3(points[i]);
    if (i/4 != 0 and points[i].x == pointpos.x) {
      pointpos.x --;
    }
    if (i/2%2 != 0 and points[i].y == pointpos.y) {
      pointpos.y --;
    }
    if (i%2 != 0 and points[i].z == pointpos.z) {
      pointpos.z --;
    }
		if ((blocks[i-num_removed] = world->get_global(pointpos, 1)) == nullptr) {
			num_removed++;
		} else {
      for (int j = 0; j < i-num_removed; j ++) {
        if (blocks[j] == blocks[i-num_removed]) {
          num_removed++;
          break;
        }
      }
    }
	}
  // for (vec3 point : points) cout << point << ' '; cout << endl;
  
  num_bases = 8 - num_removed;
  std::copy(blocks, blocks+8, bases);
  
  // for (Block* block : blocks) cout << block << ' '; cout << endl;
  int scale = 1;
  while (num_bases > 1 and blocks[0] != nullptr) {
    // cout << endl << "scale " << scale << " num_bases " << num_bases << " blocks ";
    // for (Block* block : blocks) cout << block << ' '; cout << endl;
    scale *= 2;
    num_removed = 0;
    for (int i = 0; i < num_bases; i ++) {
      // cout << blocks[i] << ' ' << blocks[i]->globalpos << ' ' << blocks[i]->scale << ' ' << num_removed << endl;
      if (blocks[i]->scale < scale) {
        // cout << i << "  replacing with " << blocks[i]->parent << endl;
        blocks[i-num_removed] = blocks[i]->parent;
        for (int j = 0; j < i-num_removed; j ++) {
          if (blocks[j] == blocks[i-num_removed]) {
            // cout << j << "  found duplicate " << blocks[j] << endl;
            num_removed ++;
            break;
          }
        }
      } else {
        // cout << "copying " << endl;
        blocks[i-num_removed] = blocks[i];
      }
    }
    
    if (num_removed > 0 and blocks[0] != nullptr) {
      num_bases -= num_removed;
      std::copy(blocks, blocks+num_bases, bases);
    }
  }
  
  // cout << " END " << num_bases << ' ' << bases[0]->hitbox().contains(box) << endl << endl;
}

Pixel* FreeBlockIter::iterator::operator*() {
  // cout << "-- Used " << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
  return pix;
}

FreeBlockIter::iterator FreeBlockIter::iterator::operator++() {
  Block* block = pix->parbl;
  pix = nullptr;
  increment(block);
  return *this;
}

void FreeBlockIter::iterator::increment(Block* block) {
  vec3 oldpos = vec3(block->globalpos) + block->scale/2.0f;
  while (block->parent != base->parent and block->parentpos == parent->end_pos) {
    block = block->parent;
  }
  if (block->parent == base->parent) {
    baseindex ++;
    if (baseindex >= parent->num_bases) {
      pix = (Pixel*)0x1; // horrible, lo cambiare pronto
      // cout << " Termino " << endl;
      return;
    } else {
      base = parent->bases[baseindex];
      get_to_pix(base);
      return;
    }
  }
  ivec3 pos = parent->increment_func(block->parentpos, parent->start_pos, parent->end_pos);
  block = block->parent->get(pos);
  // debuglines->render(oldpos, vec3(block->globalpos) + block->scale/2.0f, vec3(1,1,0));
  get_to_pix(block);
}

void FreeBlockIter::iterator::get_to_pix(Block* block) {
  // cout << "get_to_pix ( " << block << ' ' << block->globalpos << ' ' << block->scale << endl;
  while (pix == nullptr) {
    vec3 oldpos = vec3(block->globalpos) + block->scale/2.0f;
    if (!parent->box.collide(block->hitbox())) {
      // cout << " not in face " << endl;
      // debuglines->render(block->hitbox(), vec3(1,0.5f,0.5f));
      increment(block);
    } else if (block->continues) {
      // cout << " inner " << endl;
      // debuglines->render(block->hitbox(), vec3(0.5f,1,0.5f));
      block = block->get(parent->start_pos);
    } else {
      // debuglines->render(block->hitbox(), vec3(0.5f,0.5f,1));
      // debuglines->render(block->hitbox().global_midpoint(), parent->box.global_midpoint(), vec3(0,1,1));
      // cout << " setting " << endl;
      pix = block->pixel;
      // cout << "end " << pix << " PIX " << block << " Block" << endl;
      // increment(block);
    }
    // debuglines->render(oldpos, vec3(block->globalpos) + block->scale/2.0f, vec3(1,0,1));
  }
}

FreeBlockIter::iterator FreeBlockIter::begin() {
  if (num_bases == 0) {
    return end();
  }
  iterator iter {this, bases[0], 0, nullptr};
  iter.get_to_pix(bases[0]);
  return iter;
}

FreeBlockIter::iterator FreeBlockIter::end() {
  return {this, nullptr, 0, (Pixel*)0x1};
}

bool operator!=(const FreeBlockIter::iterator& iter1, const FreeBlockIter::iterator& iter2) {
  return iter1.parent != iter2.parent or iter1.pix != iter2.pix;
}








FreeBlock* FreeBlockFreeIter::iterator::operator*() {
  return free;
}

FreeBlockFreeIter::iterator FreeBlockFreeIter::iterator::operator++() {
  if (free == nullptr) {
    baseindex ++;
    if (baseindex < 8) {
      free = bases[baseindex];
    } else {
      free = nullptr;
    }
  } else {
    free = free->allfreeblocks;
  }
  return *this;
}

FreeBlockFreeIter::iterator FreeBlockFreeIter::begin() {
  return iterator {freebases, 0, freebases[0]};
}

FreeBlockFreeIter::iterator FreeBlockFreeIter::end() {
  return iterator {freebases, num_freebases, nullptr};
}

bool operator!=(const FreeBlockFreeIter::iterator& iter1, const FreeBlockFreeIter::iterator& iter2) {
  return iter1.bases != iter2.bases or iter1.baseindex != iter2.baseindex or iter1.free != iter2.free;
}

FreeBlockFreeIter::FreeBlockFreeIter(Collider* world, Hitbox box) {
  
  box.negbox -= World::chunksize/4;
  box.posbox += World::chunksize/4;
  
  vec3 points[8];
  box.points(points);
  
  num_freebases = 0;
  for (int i = 0; i < 8; i ++) {
    Block* block = world->get_global(SAFEFLOOR3(points[i]), 1);
    if (block != nullptr) {
      FreeBlock* newfree = block->world->allfreeblocks;
      if (newfree != nullptr) {
        freebases[num_freebases++] = newfree;
        for (int j = 0; j < num_freebases-1; j ++) {
          if (freebases[j] == newfree) {
            num_freebases --;
            break;
          }
        }
      }
    }
  }
  
  freebases[num_freebases] = nullptr;
}










Pixel* BlockTouchSideIter::iterator::operator*() {
  if (parent->free) {
    return *freeiter;
  } else {
    return *blockiter;
  }
}

BlockTouchSideIter::iterator BlockTouchSideIter::iterator::operator++() {
  if (parent->free) {
    ++freeiter;
  } else {
    ++blockiter;
  }
  return *this;
}

BlockTouchSideIter::BlockTouchSideIter(Block* block, ivec3 dir) {
  Block* side = block->get_local(block->globalpos + dir * block->scale, block->scale);
  if (side == nullptr) {
    if (block->freecontainer != nullptr) {
			// cout << "touchsideiter " << block << ' ' << block->globalpos << ' ' << block->scale << ' ' << dir << endl;
      vec3 norm = dir;
      vec3 dx;
      vec3 dy;
      vec3 position = block->globalpos;
      Hitbox localbox = block->local_hitbox();
      localbox.position += vec3(dir) * float(block->scale);
      // debuglines->render(block->freecontainer->box.transform_out(localbox));
      int dir_index = dir_to_index(dir);
      dir_index = (dir_index + 3) % 6; // flips dir
      if (dir_index < 3) {
        localbox.negbox[dir_index] += block->scale - 0.1f;
      } else {
        localbox.posbox[dir_index-3] -= block->scale - 0.1f;
      }
      
      free = true;
      Hitbox newbox = block->freecontainer->box.transform_out(localbox);
      freeiter = FreeBlockIter(block->world,
        newbox
      );
    } else {
      free = false;
    }
  } else {
    free = false;
    blockiter = side->iter_side(-dir);
  }
}

BlockTouchSideIter::iterator BlockTouchSideIter::begin() {
  iterator iter (this);
  if (free) {
    iter.freeiter = freeiter.begin();
  } else {
    iter.blockiter = blockiter.begin();
  }
  return iter;
}

BlockTouchSideIter::iterator BlockTouchSideIter::end() {
  iterator iter (this);
  if (free) {
    iter.freeiter = freeiter.end();
  } else {
    iter.blockiter = blockiter.end();
  }
  return iter;
}

bool operator!=(const BlockTouchSideIter::iterator& iter1, const BlockTouchSideIter::iterator& iter2) {
  if (iter1.parent != iter2.parent) {
    return false;
  }
  if (iter1.parent->free) {
    return iter1.freeiter != iter2.freeiter;
  } else {
    return iter1.blockiter != iter2.blockiter;
  }
}




#endif
