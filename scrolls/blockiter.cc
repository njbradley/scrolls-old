#ifndef BLOCKITER
#define BLOCKITER

#include "blockiter.h"
#include "blocks.h"
#include "world.h"
#include "debug.h"
#include "player.h"

template <typename BlockT>
BlockIterator<BlockT>::BlockIterator(BlockT* base): curblock(base) {
  if (base != nullptr) {
    max_scale = base->scale;
  } else {
    max_scale = 1;
  }
}

template <typename BlockT>
void BlockIterator<BlockT>::to_end() {
  curblock = nullptr;
}

template <typename BlockT>
ivec3 BlockIterator<BlockT>::increment_func(ivec3 pos) {
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
void BlockIterator<BlockT>::step_down(BlockT* parent, ivec3 curpos, BlockT* block) {
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
void BlockIterator<BlockT>::step_sideways(BlockT* parent, ivec3 curpos, BlockT* block) {
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
void BlockIterator<BlockT>::get_safe(BlockT* parent, ivec3 curpos, BlockT* block) {
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
BlockIterator<BlockT> BlockIterator<BlockT>::operator++() {
  if (curblock != nullptr) {
    step_down(curblock->parent, curblock->parentpos, curblock);
  }
  return *this;
}

template <typename BlockT>
BlockT* BlockIterator<BlockT>::operator*() {
  return curblock;
}

template <typename BlockT>
bool BlockIterator<BlockT>::operator != (const BlockIterator<BlockT>& other) const {
  return curblock != other.curblock or max_scale != other.max_scale;
}





template <typename Iterator>
Iterator BlockIterable<Iterator>::begin() const {
  if (iterator.curblock == nullptr) return end();
  Iterator iter = iterator;
  iter.get_safe(iter.curblock->parent, iter.curblock->parentpos, iter.curblock);
  return iter;
}

template <typename Iterator>
Iterator BlockIterable<Iterator>::end() const {
  Iterator iter = iterator;
  iter.curblock = nullptr;
  return iter;
}

template class BlockIterator<Block>;
template class BlockIterator<const Block>;
template class BlockIterable<BlockIterator<Block>>;
template class BlockIterable<BlockIterator<const Block>>;



template <typename BlockT>
bool PixelIterator<BlockT>::skip_block(BlockT* block) {
  return block->continues;
}

template <typename BlockT>
typename PixelIterator<BlockT>::PixelT* PixelIterator<BlockT>::operator*() {
  return this->curblock->pixel;
}


template class PixelIterator<Block>;
template class PixelIterator<const Block>;
template class BlockIterable<PixelIterator<Block>>;
template class BlockIterable<PixelIterator<const Block>>;



template class DirPixelIterator<Block>;
template class DirPixelIterator<const Block>;
template class BlockIterable<DirPixelIterator<Block>>;
template class BlockIterable<DirPixelIterator<const Block>>;




template <typename BlockT>
bool FreePixelIterator<BlockT>::valid_block(Block* block) { return box.collide(block->hitbox()); }


template class FreePixelIterator<Block>;
template class FreePixelIterator<const Block>;
template class BlockIterable<FreePixelIterator<Block>>;
template class BlockIterable<FreePixelIterator<const Block>>;


template <typename BlockT>
bool FreeBlockIterator<BlockT>::valid_block(Block* block) {
  return box.collide(block->freebox());
}

template <typename BlockT>
bool FreeBlockIterator<BlockT>::skip_block(Block* block) {
  return block->freechild == nullptr;
}

template <typename BlockT>
FreeBlock* FreeBlockIterator<BlockT>::operator*() {
  return this->curblock->freechild;
}


template class FreeBlockIterator<Block>;
template class FreeBlockIterator<const Block>;
template class BlockIterable<FreeBlockIterator<Block>>;
template class BlockIterable<FreeBlockIterator<const Block>>;





template <typename Iterator>
MultiBaseIterable<Iterator>::ComboIterator::ComboIterator(Iterator* base_list, int num_bases):
Iterator(base_list[num_bases-1]), bases(base_list), base_index(num_bases-1) {
  
}


template <typename Iterator>
void MultiBaseIterable<Iterator>::ComboIterator::finish() {
  if (base_index == 0) {
    Iterator::finish();
  } else {
    *((Iterator*)this) = bases[--base_index];
    if (this->curblock != nullptr) {
      this->get_safe(this->curblock->parent, this->curblock->parentpos, this->curblock);
    } else {
      finish();
    }
  }
}


template <typename Iterator>
typename MultiBaseIterable<Iterator>::ComboIterator MultiBaseIterable<Iterator>::begin() {
  ComboIterator iter (&bases.front(), bases.size());
  return iter;
}

template <typename Iterator>
typename MultiBaseIterable<Iterator>::ComboIterator MultiBaseIterable<Iterator>::end() {
  ComboIterator iter (&bases.front(), bases.size());
  Iterator* iterptr = &iter;
  iter.to_end();
  return iter;
}
  





template <typename Iterator>
HitboxIterable<Iterator>::HitboxIterable(Collider* world, Hitbox box) {
  // cout << "hitbox iteratb;e " << world << ' ' << box << endl;
  if (world == nullptr) {
    this->add_base(nullptr, box);
    return;
  }
  
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
    // cout << " points " << points[i] << ' ' << pointpos << endl;
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
    // cout << "  removed " << num_removed << ' ' << world->get_global(pointpos, 1) << endl;
	}
  // for (vec3 point : points) cout << point << ' '; cout << endl;
  
  Block* bases[8];
  int num_bases;
  
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
  
  
  for (int i = 0; i < num_bases; i ++) {
    this->add_base(bases[i], box);
    // cout << "base " << bases[i] << ' ' << bases[i]->globalpos << ' ' << bases[i]->scale << endl;
  }
  if (this->bases.size() == 0) {
    this->add_base(nullptr, box);
  }
}




template <typename Iterator>
FreeboxIterable<Iterator>::FreeboxIterable(Collider* world, Hitbox box) {
  if (world == nullptr) {
    this->add_base(nullptr, box);
    return;
  }
    
  ivec3 base_poses[8];
  int num_bases = 0;
  
  vec3 points[8];
  box.points(points);
  
  for (int i = 0; i < 8; i ++) {
    vec3 pos = points[i] / (float)World::chunksize;
    ivec3 new_pos = SAFEFLOOR3(pos);
    bool found = false;
    for (int j = 0; j < num_bases; j ++) {
      found = found or new_pos == base_poses[j];
    }
    if (!found) {
      base_poses[num_bases++] = new_pos;
    }
  }
  
  for (int i = 0; i < num_bases; i ++) {
    Block* block = world->get_global(base_poses[i] * World::chunksize, World::chunksize);
    if (block != nullptr) {
      this->add_base(block, box);
    }
  }
  if (this->bases.size() == 0) {
    this->add_base(nullptr, box);
  }
}


template class MultiBaseIterable<FreePixelIterator<Block>>;
template class MultiBaseIterable<FreeBlockIterator<Block>>;

template class FreeboxIterable<FreePixelIterator<Block>>;
template class FreeboxIterable<FreeBlockIterator<Block>>;

template class HitboxIterable<FreePixelIterator<Block>>;
template class HitboxIterable<FreeBlockIterator<Block>>;



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

BlockTouchSideIter::BlockTouchSideIter(Block* block, ivec3 dir): blockiter(nullptr, ivec3(0,0,0)), freeiter(nullptr, Hitbox()) {
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
      freeiter = HitboxIterable<FreePixelIterator<Block>>(block->world,
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
