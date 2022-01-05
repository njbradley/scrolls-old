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
  if (block->continues) {
    get_safe(block, startpos(), block->get(startpos()));
  } else {
    step_sideways(parent, curpos, block);
  }
}

template <typename BlockT>
void BlockIterator<BlockT>::step_sideways(BlockT* parent, ivec3 curpos, BlockT* block) {
  if (parent == nullptr or parent->scale > max_scale) {
    finish();
  } else if (curpos == endpos()) {
    step_sideways(parent->parent, parent->parentpos, parent);
  } else {
    curpos = increment_func(curpos);
    get_safe(parent, curpos, parent->get(curpos));
  }
}

template <typename BlockT>
void BlockIterator<BlockT>::get_safe(BlockT* parent, ivec3 curpos, BlockT* block) {
  if (!valid_block(block)) {
    step_sideways(parent, curpos, block);
  } else if (skip_block(block)) {
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
  return curblock != other.curblock or (max_scale != other.max_scale and curblock != nullptr);
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
typename BlockIterator<BlockT>::PixelT* PixelIterator<BlockT>::operator*() {
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
bool FreePixelIterator<BlockT>::valid_block(BlockT* block) {
  return block != nullptr and box.collide(block->hitbox());
}


template class FreePixelIterator<Block>;
template class FreePixelIterator<const Block>;
template class BlockIterable<FreePixelIterator<Block>>;
template class BlockIterable<FreePixelIterator<const Block>>;


template <typename BlockT>
bool FreeBlockIterator<BlockT>::valid_block(BlockT* block) {
  return block != nullptr and block->freecount != 0 and box.collide(block->freebox());
}

template <typename BlockT>
bool FreeBlockIterator<BlockT>::skip_block(BlockT* block) {
  return block->freechild == nullptr;
}

template <typename BlockT>
typename BlockTypes<BlockT>::FreeBlockT* FreeBlockIterator<BlockT>::operator*() {
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
  if (iter.curblock != nullptr) {
    iter.get_safe(iter.curblock->parent, iter.curblock->parentpos, iter.curblock);
  } else {
    iter.finish();
  }
  return iter;
}

template <typename Iterator>
typename MultiBaseIterable<Iterator>::ComboIterator MultiBaseIterable<Iterator>::end() {
  ComboIterator iter (&bases.front(), bases.size());
  for (int i = 0; i < bases.size(); i ++) {
    iter.finish();
  }
  // Iterator* iterptr = &iter;
  // iter.to_end();
  return iter;
}
  





template <typename Iterator>
HitboxIterable<Iterator>::HitboxIterable(ColliderT* world, Hitbox box) {
  if (world == nullptr) {
    this->add_base(nullptr, box);
    return;
  }
  
  vec3 points[8];
  box.points(points);
  
  BlockT* blocks[8];
  
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
  
  BlockT* bases[8];
  int num_bases;
  
  num_bases = 8 - num_removed;
  std::copy(blocks, blocks+8, bases);
  
  int scale = 1;
  while (num_bases > 1 and blocks[0] != nullptr) {
    scale *= 2;
    num_removed = 0;
    for (int i = 0; i < num_bases; i ++) {
      if (blocks[i]->scale < scale) {
        blocks[i-num_removed] = blocks[i]->parent;
        for (int j = 0; j < i-num_removed; j ++) {
          if (blocks[j] == blocks[i-num_removed]) {
            num_removed ++;
            break;
          }
        }
      } else {
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
  }
  if (this->bases.size() == 0) {
    this->add_base(nullptr, box);
  }
}




template <typename Iterator>
FreeboxIterable<Iterator>::FreeboxIterable(ColliderT* world, Hitbox box) {
  if (world == nullptr) {
    this->add_base(nullptr, box);
    return;
  }
  
  ivec3 base_pos = SAFEFLOOR3(box.global_center());
  base_pos = SAFEDIV(base_pos, World::chunksize);
  
  for (int x = -1; x < 2; x ++) {
    for (int y = -1; y < 2; y ++) {
      for (int z = -1; z < 2; z ++) {
        BlockT* newbase = world->get_global((base_pos + ivec3(x,y,z)) * World::chunksize, World::chunksize);
        if (newbase != nullptr and newbase->freecount != 0) {
          this->add_base(newbase, box);
        }
      }
    }
  }
  if (this->bases.size() == 0) {
    this->add_base(nullptr, box);
  }
}

template <typename BlockT>
FullFreePixelIterable<BlockT>::FullFreePixelIterable(ColliderT* world, Hitbox box, const Block* ignore, bool ignore_world):
HitboxIterable<FreePixelIterator<BlockT>>(ignore_world ? nullptr : world, box) {
  double start = getTime();
  for (typename BlockTypes<BlockT>::FreeBlockT* freeblock : FreeboxIterable<FreeBlockIterator<BlockT>>(world, box)) {
    if (freeblock != ignore and freeblock->box.collide(box)) {
      this->add_base(freeblock, box);
    }
  }
}

template class MultiBaseIterable<FreePixelIterator<Block>>;
template class MultiBaseIterable<FreeBlockIterator<Block>>;

template class FreeboxIterable<FreePixelIterator<Block>>;
template class FreeboxIterable<FreeBlockIterator<Block>>;

template class HitboxIterable<FreePixelIterator<Block>>;
template class HitboxIterable<FreeBlockIterator<Block>>;

template class FullFreePixelIterable<Block>;
template class FullFreePixelIterable<const Block>;


template <typename BlockT>
BlockSideIterable<BlockT>::BlockSideIterable(BlockT* block, ivec3 dir):
blockiter(block->get_global(block->globalpos + dir*block->scale, block->scale), -dir),
freeiter(nullptr, Hitbox()) {
  if (blockiter.iterator.curblock == nullptr and block->freecontainer != nullptr) {
    freeiter = HitboxIterable<FreePixelIterator<BlockT>>(block->freecontainer->highparent, make_side_hitbox(block, dir));
    free = true;
  }
}

template <typename BlockT>
Hitbox BlockSideIterable<BlockT>::make_side_hitbox(BlockT* block, ivec3 dir) {
  Hitbox localbox = block->local_hitbox();
  localbox.position += vec3(dir) * float(block->scale);
  int dir_index = dir_to_index(dir);
  dir_index = (dir_index + 3) % 6; // flips dir
  if (dir_index < 3) {
    localbox.negbox[dir_index] += block->scale - 0.1f;
  } else {
    localbox.posbox[dir_index-3] -= block->scale - 0.1f;
  }
  
  typename BlockTypes<BlockT>::FreeBlockT* freecont = block->freecontainer;
  while (freecont != nullptr) {
    localbox = freecont->box.transform_out(localbox);
    freecont = freecont->highparent->freecontainer;
  }
  
  return localbox;
}
  


template <typename BlockT>
typename BlockSideIterable<BlockT>::PixelT* BlockSideIterable<BlockT>::Iterator::operator*() {
  if (free) {
    return *freeiter;
  } else {
    return *blockiter;
  }
}

template <typename BlockT>
bool BlockSideIterable<BlockT>::Iterator::operator!=(const Iterator& other) {
  return free != other.free or (free and freeiter != other.freeiter) or (!free and blockiter != other.blockiter);
}

template <typename BlockT>
typename BlockSideIterable<BlockT>::Iterator BlockSideIterable<BlockT>::Iterator::operator++() {
  if (free) {
    ++ freeiter;
  } else {
    ++ blockiter;
  }
  return *this;
}

template <typename BlockT>
typename BlockSideIterable<BlockT>::Iterator BlockSideIterable<BlockT>::begin() {
  Iterator iter {blockiter.begin(), freeiter.begin(), free};
  return iter;
}

template <typename BlockT>
typename BlockSideIterable<BlockT>::Iterator BlockSideIterable<BlockT>::end() {
  Iterator iter {blockiter.end(), freeiter.end(), free};
  return iter;
}


template class BlockSideIterable<Block>;
template class BlockSideIterable<const Block>;






#endif
