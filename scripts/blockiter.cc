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











FreeBlockIter::FreeBlockIter(Collider* world, vec3 pos, vec3 dx, vec3 dy, vec3 dz, vec3 ndims):
position(pos), dirx(dx), diry(dy), dirz(dz), dims(ndims) {
  // cout << "new freiter " << pos << ' ' << dx << ' ' << dy << ' ' << dz << ' ' << dims << endl;
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
  
	dx *= dims.x;
	dy *= dims.y;
	dz *= dims.z;
	vec3 points[8] {
		pos,
		pos+dz,
		pos+dy,
		pos+dy+dz,
		pos+dx,
		pos+dx+dz,
		pos+dx+dy,
		pos+dx+dy+dz
	};
	
	int num_removed = 0;
	for (int i = 0; i < 8; i ++) {
		if (world->get_global(ivec3(points[i]), 1) == nullptr) {
			num_removed++;
		} else if (num_removed != 0) {
			points[i-num_removed] = points[i];
		}
	}
	
	int num_points = 8 - num_removed;
	
	if (num_points > 0) {
		int maxscale = 1;
		int progress = 1;
		// Block* last = world->get_global(ivec3(points[0]), maxscale);
		Block* last;
		// aqui aumento la tama~no hasta el bloque tiene todos los puntos
		do {
			last = world->get_global(ivec3(points[progress-1]), maxscale);
			Block* block;
			while ((block = world->get_global(ivec3(points[progress]), maxscale)) == last and progress < num_points) {
				last = block;
				progress ++;
			}
			maxscale *= csize;
		} while (last->parent != nullptr and progress < num_points);
		
		maxscale /= csize;
		
		bases.push_back(world->get_global(ivec3(points[0]), maxscale));
		// cout << "bases " << bases.back() << ' ' << bases.back()->globalpos << ' ' << bases.back()->scale << endl;
		if (maxscale == World::chunksize) {
			for (int i = 1; i < 8; i ++) {
				Block* block = world->get_global(ivec3(points[i]), maxscale);
				if (block != bases.back()) {
					bases.push_back(block);
					// cout << "bases " << bases.back() << ' ' << bases.back()->globalpos << ' ' << bases.back()->scale << endl;
				}
			}
		}
	}
}


Pixel* FreeBlockIter::iterator::operator*() {
  // cout << "-- Used " << pix->parbl->globalpos << ' ' << pix->parbl->scale << endl;
  return pix;
}

bool FreeBlockIter::iterator::point_in_cube(vec3 pos, vec3 center, float scale) {
  return  center.x - scale < pos.x and pos.x < center.x + scale
      and center.y - scale < pos.y and pos.y < center.y + scale
      and center.z - scale < pos.z and pos.z < center.z + scale;
}

bool FreeBlockIter::iterator::cube_in_cube(Block* block) {
  float scale = block->scale / 2.0f;
  vec3 center = vec3(block->globalpos) + scale;
	vec3 localpoint (
		glm::dot(center - parent->position, parent->dirx),
		glm::dot(center - parent->position, parent->diry),
		glm::dot(center - parent->position, parent->dirz)
	);
	localpoint = glm::max(glm::min(localpoint, parent->dims), vec3(0,0,0));
	vec3 closepoint = parent->position
		+ localpoint.x * parent->dirx
		+ localpoint.y * parent->diry
		+ localpoint.z * parent->dirz;
	// cout << center << ' ' << parent->position << ' ' << localpoint << ' ' << closepoint << ' ' << parent->dims << endl;
  return point_in_cube(closepoint, center, scale);
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
    if (baseindex >= parent->bases.size()) {
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
    if (!cube_in_cube(block)) {
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
  if (bases.size() == 0) {
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
      if (dir_to_index(dir) < 3) {
        position += block->scale;
        dx = dir_array[(dir_to_index(dir)+1) % 3 + 3];
        dy = dir_array[(dir_to_index(dir)+2) % 3 + 3];
      } else {
        dx = dir_array[(dir_to_index(dir)+1) % 3];
        dy = dir_array[(dir_to_index(dir)+2) % 3];
      }
      
      free = true;
      freeiter = FreeBlockIter(block->world,
        block->freecontainer->box.transform_out(position),
        block->freecontainer->box.transform_out_dir(dx),
        block->freecontainer->box.transform_out_dir(dy),
        block->freecontainer->box.transform_out_dir(norm),
        vec3(block->scale, block->scale, 0.1f)
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
