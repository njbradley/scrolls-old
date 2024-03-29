#ifndef BLOCKITER
#define BLOCKITER

#include "blockiter.h"
#include "blocks.h"
#include "world.h"
#include "debug.h"
#include "player.h"




Pixel* BlockIter::iterator::operator*() {
  return pix;
}

Block* BlockIter::iterator::increment(Block* block) {
  while (block->parent != parent->base->parent and block->parentpos == parent->end_pos) {
    block = block->parent;
  }
  if (block->parent == parent->base->parent) {
    return nullptr;
  }
  ivec3 pos = block->parentpos;
  do {
    pos = parent->increment_func(pos, parent->start_pos, parent->end_pos);
  } while (pos != parent->end_pos and block->parent->get(pos) == nullptr);
  if (block->parent->get(pos) == nullptr) {
    block = increment(block->parent);
  } else {
    block = block->parent->get(pos);
  }
  return block;
}

void BlockIter::iterator::move_down(Block* block) {
  while (block != nullptr and block->continues) {
    ivec3 pos = parent->start_pos;
    
    while (pos != parent->end_pos and block->get(pos) == nullptr) {
      pos = parent->increment_func(pos, parent->start_pos, parent->end_pos);
    }
    if (block->get(pos) == nullptr) {
      block = increment(block);
    } else {
      block = block->get(pos);
    }
  }
  if (block == nullptr) {
    pix = nullptr;
  } else {
    pix = block->pixel;
  }
}

BlockIter::iterator BlockIter::iterator::operator++() {
  move_down(increment(pix->parbl));
  return *this;
}

BlockIter::iterator BlockIter::begin() {
  if (base == nullptr) {
    return end();
  }
  iterator iter {this, nullptr};
  iter.move_down(base);
  return iter;
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
