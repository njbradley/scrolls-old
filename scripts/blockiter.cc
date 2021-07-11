#ifndef BLOCKITER
#define BLOCKITER

#include "blockiter.h"
#include "blocks.h"
#include "world.h"





Pixel* BlockIter::iterator::operator*() {
  return pix;
}


BlockIter::iterator BlockIter::iterator::operator++() {
  Block* block = pix->parbl;
  while (block->parent != parent->base->parent and block->parentpos == parent->end_pos) {
    block = block->parent;
  }
  if (block->parent == parent->base->parent) {
    pix = nullptr;
    return *this;
  }
  ivec3 pos = block->parentpos;
  pos = parent->increment_func(pos, parent->start_pos, parent->end_pos);
  block = block->parent->get(pos);
  while (block->continues) {
    block = block->get(parent->start_pos);
  }
  pix = block->pixel;
  return *this;
}

BlockIter::iterator BlockIter::begin() {
  if (base == nullptr) {
    return end();
  }
  Block* block = base;
  while (block->continues) {
    block = block->get(start_pos);
  }
  return {this, block->pixel};
}

BlockIter::iterator BlockIter::end() {
  return {this, nullptr};
}

bool operator!=(const BlockIter::iterator& iter1, const BlockIter::iterator& iter2) {
  return iter1.parent != iter2.parent or iter1.pix != iter2.pix;
}






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
  
  vec3 points[8];
  box.points(points);
  
  Block* blocks[8];
  
  int num_removed = 0;
	for (int i = 0; i < 8; i ++) {
		if ((blocks[i-num_removed] = world->get_global(ivec3(points[i]), 1)) == nullptr) {
			num_removed++;
		}
	}
  
  num_bases = 8 - num_removed;
	
  std::copy(blocks, blocks+8, bases);
  
  int scale = 1;
  while (num_bases > 1 and blocks[0] != nullptr) {
    scale *= 2;
    num_removed = 0;
    for (int i = 0; i < num_bases; i ++) {
      if (blocks[i]->scale < scale) {
        blocks[i-num_removed] = blocks[i]->parent;
        
        for (int j = 0; j < i; j ++) {
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
  get_to_pix(block);
}

void FreeBlockIter::iterator::get_to_pix(Block* block) {
  // cout << "get_to_pix ( " << block << ' ' << block->globalpos << ' ' << block->scale << endl;
  while (pix == nullptr) {
    if (!parent->box.collide(block->hitbox(), 0)) {
      // cout << " not in face " << endl;
      increment(block);
    } else if (block->continues) {
      // cout << " inner " << endl;
      block = block->get(parent->start_pos);
    } else {
      // cout << " setting " << endl;
      pix = block->pixel;
    }
  }
}

FreeBlockIter::iterator FreeBlockIter::begin() {
  if (num_bases != 0) {
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




CollidingZoneIter::CollidingZoneIter(Block* nblock) {
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
  
  base = nblock;
  blockscale = nblock->scale;
  blockcenter = nblock->globalpos + blockscale/2;
  while (base->parent != nullptr) {
    base = base->parent;
  }
}

Block* CollidingZoneIter::iterator::operator*() {
  return block;
}

bool CollidingZoneIter::iterator::colliding(Block* other) {
  ivec3 diff = glm::abs(other->globalpos + other->scale/2 - parent->blockcenter);
  int thresh = parent->blockscale + other->scale;
  bool result = (diff.x < thresh and diff.y < thresh and diff.z < thresh);
  return true;
}

CollidingZoneIter::iterator CollidingZoneIter::iterator::operator++() {
  increment();
  return *this;
}

void CollidingZoneIter::iterator::increment_base() {
  while (block == nullptr) {
    baseoffset.z ++;
    if (baseoffset.z > 1) {
      baseoffset.z = -1;
      baseoffset.y ++;
      if (baseoffset.y > 1) {
        baseoffset.y = -1;
        baseoffset.x ++;
        if (baseoffset.x > 1) {
          baseoffset = vec3(2,2,2);
          return;
        }
      }
    }
    
    block = parent->base->get_global(parent->base->globalpos + baseoffset * parent->base->scale, parent->base->scale);
  }
}

void CollidingZoneIter::iterator::increment_up() {
  while (block->parentpos == end_pos and block->parent != nullptr) {
    block = block->parent;
  }
  if (block->parent == nullptr) {
    // increment base offset
    block = nullptr;
    increment_base();
  } else {
    // increment normal pos
    block = block->parent->get(parent->increment_func(block->parentpos, start_pos, end_pos));
  }
}

void CollidingZoneIter::iterator::increment() {
  if (block->continues) {
    block = block->get(start_pos);
  } else {
    increment_up();
  }
  get_to_val();
}

void CollidingZoneIter::iterator::get_to_val() {
  while (block != nullptr and !colliding(block)) {
    increment_up();
  }
}

CollidingZoneIter::iterator CollidingZoneIter::begin() {
  iterator iter {this, base->get_global(base->globalpos + ivec3(-1,-1,-1) * base->scale, base->scale), ivec3(-1,-1,-1)};
  iter.increment_base();
  iter.get_to_val();
  return iter;
}

CollidingZoneIter::iterator CollidingZoneIter::end() {
  return {this, nullptr, ivec3(2,2,2)};
}

bool operator!=(const CollidingZoneIter::iterator& iter1, const CollidingZoneIter::iterator& iter2) {
  return iter1.block != iter2.block or iter1.parent != iter2.parent;
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
      int dir_index = dir_to_index(dir);
      if (dir_index < 3) {
        localbox.negbox[dir_index] += block->scale - 0.1f;
      } else {
        localbox.posbox[dir_index-3] -= block->scale - 0.1f;
      }
      
      free = true;
      freeiter = FreeBlockIter(block->world,
        block->freecontainer->box.transform_out(localbox)
      );
    } else {
      free = false;
      blockiter = BlockIter{.base = nullptr};
    }
  } else {
    free = false;
    blockiter = side->iter_side(-dir);
  }
}

BlockTouchSideIter::iterator BlockTouchSideIter::begin() {
  iterator iter;
  iter.parent = this;
  if (free) {
    iter.freeiter = freeiter.begin();
  } else {
    iter.blockiter = blockiter.begin();
  }
  return iter;
}

BlockTouchSideIter::iterator BlockTouchSideIter::end() {
  iterator iter;
  iter.parent = this;
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









Pixel* BlockTouchIter::iterator::operator*() {
  return *iter;
}

BlockTouchIter::iterator BlockTouchIter::iterator::operator++() {
  ++iter;
  while (!(iter != parent->blockiter.end())) {
    dir_index ++;
    if (dir_index >= 6) {
      dir_index = -1;
      return *this;
    }
    ivec3 pos = parent->base->globalpos;
    int scale = parent->base->scale;
    ivec3 dir = dir_array[dir_index];
    Block* block = parent->base->get_global(pos+dir*scale, scale);
    if (block != nullptr) {
      parent->blockiter = block->iter_side(-dir);
      iter = parent->blockiter.begin();
    }
  }
  return *this;
}

BlockTouchIter::iterator BlockTouchIter::begin() {
  int dir_index = -1;
  Block* block;
  ivec3 dir;
  ivec3 pos = base->globalpos;
  int scale = base->scale;
  do {
    dir_index ++;
    if (dir_index >= 6) {
      return end();
    }
    dir = dir_array[dir_index];
    block = base->get_global(pos+dir*scale, scale);
  } while (block == nullptr);
  blockiter = block->iter_side(-dir);
  return {this, dir_index, blockiter.begin()};
}

BlockTouchIter::iterator BlockTouchIter::end() {
  BlockTouchIter::iterator iter;
  iter.parent = this;
  iter.dir_index = -1;
  return iter;
}

bool operator!=(const BlockTouchIter::iterator& iter1, const BlockTouchIter::iterator& iter2) {
  if (iter1.parent == iter2.parent and iter1.dir_index == -1 and iter2.dir_index == -1) {
    return false;
  }
  return iter1.parent != iter2.parent or iter1.iter != iter2.iter;
}



BlockGroupIter::BlockGroupIter(Pixel* newbase, Collider* nworld, bool (*func)(Pixel* base, Pixel* pix)):
base(newbase), world(nworld), includefunc(func) {
  unordered_set<Pixel*> new_pixels;
  unordered_set<Pixel*> last_pixels;
  
  last_pixels.emplace(base);
  
  while (last_pixels.size() > 0) {
    for (Pixel* pix : last_pixels) {
      pixels.emplace(pix);
    }
    
    for (Pixel* pix : last_pixels) {
      for (Pixel* sidepix : pix->parbl->iter_touching()) {
        if (includefunc(base, sidepix) and pixels.count(sidepix) == 0 and new_pixels.count(sidepix) == 0) {
          new_pixels.emplace(sidepix);
        }
      }
    }
    
    last_pixels.swap(new_pixels);
    new_pixels.clear();
  }
}



unordered_set<Pixel*>::iterator BlockGroupIter::begin() {
  return pixels.begin();
}

unordered_set<Pixel*>::iterator BlockGroupIter::end() {
  return pixels.end();
}

void BlockGroupIter::swap(unordered_set<Pixel*>& other) {
  pixels.swap(other);
}



const Pixel* ConstBlockIter::iterator::operator*() {
  return pix;
}

ConstBlockIter::iterator ConstBlockIter::iterator::operator++() {
  const Block* block = pix->parbl;
  while (block->parent != parent->base->parent and block->parentpos == parent->end_pos) {
    block = block->parent;
  }
  if (block->parent == parent->base->parent) {
    pix = nullptr;
    return *this;
  }
  ivec3 pos = block->parentpos;
  pos = parent->increment_func(pos, parent->start_pos, parent->end_pos);
  block = block->parent->get(pos);
  while (block->continues) {
    block = block->get(parent->start_pos);
  }
  pix = block->pixel;
  return *this;
}

ConstBlockIter::iterator ConstBlockIter::begin() {
  const Block* block = base;
  while (block->continues) {
    block = block->get(start_pos);
  }
  // cout << "start p " << block->globalpos << ' ' << block->parentpos << endl;
  return {this, block->pixel};
}

ConstBlockIter::iterator ConstBlockIter::end() {
  return {this, nullptr};
}

bool operator!=(const ConstBlockIter::iterator& iter1, const ConstBlockIter::iterator& iter2) {
  return iter1.parent != iter2.parent or iter1.pix != iter2.pix;
}

#endif
