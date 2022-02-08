#ifndef BLOCKITER
#define BLOCKITER

#include "blockiter.h"
#include "blocks.h"
#include "world.h"
#include "debug.h"
#include "player.h"

#define dout if (false) cout

template <typename BlockViewT>
BlockIterator<BlockViewT>::BlockIterator(BlockViewT& view): BlockViewT(view) {
  if (isvalid()) {
    max_scale = scale;
  } else {
    max_scale = 1;
  }
}

template <typename BlockViewT>
void BlockIterator<BlockViewT>::to_end() {
  parent = nullptr;
  curblock = nullptr;
  scale = -1;
}

template <typename BlockViewT>
ivec3 BlockIterator<BlockViewT>::increment_func(ivec3 pos) {
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

template <typename BlockViewT>
void BlockIterator<BlockViewT>::step_down() {
  dout << " step_down " << parent << ' ' << curblock << ' ' << scale << endl;
  if (curblock->continues) {
    dout << "  going down " << endl;
    step_down(startpos());
    get_safe();
  } else {
    dout << "  going to the side " << endl;
    step_side();
  }
}

template <typename BlockViewT>
void BlockIterator<BlockViewT>::step_side() {
  dout << " step_side " << parent << ' ' << curblock << ' ' << scale << ' ' << parentpos() << ' '<< globalpos << endl;
  if (parent == nullptr or scale == max_scale) {
    dout << "  finishing " << endl;
    finish();
  } else if (parentpos() == endpos()) {
    dout << "  going up " << endl;
    step_up();
    step_side();
  } else {
    dout << "  normal inc " << increment_func(parentpos()) << endl;
    step_side(increment_func(parentpos()));
    dout << "  now " << parent << ' ' << curblock << ' ' << scale << endl;
    get_safe();
  }
}

template <typename BlockViewT>
void BlockIterator<BlockViewT>::get_safe() {
  dout << " get_safe " << parent << ' ' << ' ' << curblock << ' ' << scale << endl;
  if (!valid_block()) {
    dout << "  invalid block " << endl;
    step_side();
  } else if (skip_block()) {
    dout << "  skip block " << endl;
    step_down();
  }
}


template <typename BlockViewT>
BlockIterator<BlockViewT> BlockIterator<BlockViewT>::operator++() {
  dout << "plus plus op " << endl;
  step_down();
  return *this;
}

template <typename BlockViewT>
BlockViewT& BlockIterator<BlockViewT>::operator*() {
  return *this;
}

template <typename BlockViewT>
bool BlockIterator<BlockViewT>::operator != (const BlockIterator<BlockViewT>& other) const {
  return curblock != other.curblock or (max_scale != other.max_scale and isvalid());
}





template <typename Iterator>
Iterator BlockIterable<Iterator>::begin() const {
  if (!iterator.isvalid()) return end();
  Iterator iter = iterator;
  iter.get_safe();
  return iter;
}

template <typename Iterator>
Iterator BlockIterable<Iterator>::end() const {
  Iterator iter = iterator;
  iter.to_end();
  return iter;
}

template class BlockIterator<BlockView>;
template class BlockIterator<ConstBlockView>;
template class BlockIterable<BlockIterator<BlockView>>;
template class BlockIterable<BlockIterator<ConstBlockView>>;



template <typename BlockT>
bool PixelIterator<BlockT>::skip_block() {
  return curblock->continues;
}


template class PixelIterator<BlockView>;
template class PixelIterator<ConstBlockView>;
template class BlockIterable<PixelIterator<BlockView>>;
template class BlockIterable<PixelIterator<ConstBlockView>>;



template class DirPixelIterator<BlockView>;
template class DirPixelIterator<ConstBlockView>;
template class BlockIterable<DirPixelIterator<BlockView>>;
template class BlockIterable<DirPixelIterator<ConstBlockView>>;




template <typename BlockViewT>
bool FreePixelIterator<BlockViewT>::valid_block() {
  return curblock != nullptr and box.collide(this->hitbox());
}


template class FreePixelIterator<BlockView>;
template class FreePixelIterator<ConstBlockView>;
template class BlockIterable<FreePixelIterator<BlockView>>;
template class BlockIterable<FreePixelIterator<ConstBlockView>>;


template <typename BlockViewT>
bool FreeBlockIterator<BlockViewT>::valid_block() {
  return curblock != nullptr and curblock->freecount != 0 and box.collide(this->freebox());
}

template <typename BlockViewT>
bool FreeBlockIterator<BlockViewT>::skip_block() {
  return curblock->freechild == nullptr;
}

template class FreeBlockIterator<BlockView>;
template class FreeBlockIterator<ConstBlockView>;
template class BlockIterable<FreeBlockIterator<BlockView>>;
template class BlockIterable<FreeBlockIterator<ConstBlockView>>;





template <typename Iterator>
MultiBaseIterable<Iterator>::ComboIterator::ComboIterator(Iterator* base_list, int num_bases):
Iterator(base_list[num_bases-1]), bases(base_list), base_index(num_bases-1) {
  
}


template <typename Iterator>
void MultiBaseIterable<Iterator>::ComboIterator::finish() {
  // cout << " finishing " << endl;
  if (base_index == 0) {
    Iterator::finish();
  } else {
    // cout << "  switching iters, at base_index " << base_index << endl;
    *((Iterator*)this) = bases[--base_index];
    // cout << "  new curblock " << this->curblock << endl;
    if (this->curblock != nullptr) {
      this->get_safe();
    } else {
      finish();
    }
    // cout << "  final new curblock " << this->curblock << endl;
  }
}


template <typename Iterator>
typename MultiBaseIterable<Iterator>::ComboIterator MultiBaseIterable<Iterator>::begin() {
  ComboIterator iter (&bases.front(), bases.size());
  if (iter.curblock != nullptr) {
    iter.get_safe();
  } else {
    iter.finish();
  }
  return iter;
}

template <typename Iterator>
typename MultiBaseIterable<Iterator>::ComboIterator MultiBaseIterable<Iterator>::end() {
  // cout << "  doing comboiter end " << endl;
  ComboIterator iter (&bases.front(), bases.size());
  // for (int i = 0; i < bases.size(); i ++) {
  //   iter.finish();
  // }
  // cout << "  done combo iter end" << endl;
  // Iterator* iterptr = &iter;
  iter.to_end();
  return iter;
}
  





template <typename Iterator>
HitboxIterable<Iterator>::HitboxIterable(ColliderT* world, Hitbox box) {
  if (world == nullptr) {
    BlockViewT view;
    this->add_base(view, box);
    return;
  }
  
  Hitbox boundingbox = box.boundingbox();
  
  float max_side = std::max(std::max(boundingbox.size().x, boundingbox.size().y), boundingbox.size().z);
  
  int scale = 1;
  while (scale < max_side) {
    scale *= 2;
  }
  
  vec3 points[8];
  boundingbox.points(points);
  // debuglines->render(box.boundingbox(), vec3(1,0,0), "freeiter");
  BlockViewT bases[8];
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
    BlockViewT newbase = world->get_global(pointpos, scale);
    bases[i-num_removed] = newbase;
    if (!newbase.isvalid()) {
      num_removed ++;
    } else {
      // debuglines->render(newbase->hitbox(), vec3(0,1,0), "freeiter");
      for (int j = 0; j < i-num_removed; j ++) {
        if (newbase == bases[j]) {
          num_removed ++;
          break;
        }
      }
    }
  }
  
  int num_bases = 8 - num_removed;
  
  for (int i = 0; i < num_bases; i ++) {
    this->add_base(bases[i], box);
  }
  
  if (num_bases == 0) {
    BlockViewT view;
    this->add_base(view, box);
  }
}




template <typename Iterator>
FreeboxIterable<Iterator>::FreeboxIterable(ColliderT* world, Hitbox box) {
  if (world == nullptr) {
    BlockViewT view;
    this->add_base(view, box);
    return;
  }
  
  ivec3 base_pos = SAFEFLOOR3(box.global_center());
  base_pos = SAFEDIV(base_pos, World::chunksize);
  
  for (int x = -1; x < 2; x ++) {
    for (int y = -1; y < 2; y ++) {
      for (int z = -1; z < 2; z ++) {
        BlockViewT newbase = world->get_global((base_pos + ivec3(x,y,z)) * World::chunksize, World::chunksize);
        if (newbase.isvalid()) {// and newbase->freecount != 0) {
          this->add_base(newbase, box);
        }
      }
    }
  }
  if (this->bases.size() == 0) {
    BlockViewT view;
    this->add_base(view, box);
  }
}

template <typename BlockViewT>
FullFreePixelIterable<BlockViewT>::FullFreePixelIterable(ColliderT* world, Hitbox box, const Block* ignore, bool ignore_world):
HitboxIterable<FreePixelIterator<BlockViewT>>(ignore_world ? nullptr : world, box) {
  double start = getTime();
  for (BlockViewT& view : FreeboxIterable<FreeBlockIterator<BlockViewT>>(world, box)) {
    for (FreeBlockT* free = view->freechild; free != nullptr; free = free->next) {
      // debuglines->render(free->box, vec3(0,0,1), "raycast");
      if (free != ignore and free->box.collide(box)) {
        BlockViewT view = free->highparent;
        view.step_down_free();
        while (view.curblock != free) {
          view.step_side_free();
        }
        this->add_base(view, box);
      }
    }
  }
}

template class MultiBaseIterable<FreePixelIterator<BlockView>>;
template class MultiBaseIterable<FreeBlockIterator<BlockView>>;

template class FreeboxIterable<FreePixelIterator<BlockView>>;
template class FreeboxIterable<FreeBlockIterator<BlockView>>;

template class HitboxIterable<FreePixelIterator<BlockView>>;
template class HitboxIterable<FreeBlockIterator<BlockView>>;

template class FullFreePixelIterable<BlockView>;
template class FullFreePixelIterable<ConstBlockView>;


template <typename BlockViewT>
BlockSideIterable<BlockViewT>::BlockSideIterable(BlockViewT& block, ivec3 dir):
blockiter(BlockViewT(block.get_global(block.globalpos + dir*block.scale, block.scale)), -dir),
freeiter(nullptr, Hitbox()) {
  if (!blockiter.iterator.isvalid() and block.freecontainer != nullptr) {
    freeiter = HitboxIterable<FreePixelIterator<BlockViewT>>(block.freecontainer->highparent.world, make_side_hitbox(block, dir));
    free = true;
  }
}

template <typename BlockViewT>
Hitbox BlockSideIterable<BlockViewT>::make_side_hitbox(BlockViewT& block, ivec3 dir) {
  Hitbox localbox = block.local_hitbox();
  localbox.position += vec3(dir) * float(block.scale);
  int dir_index = dir_to_index(dir);
  dir_index = (dir_index + 3) % 6; // flips dir
  if (dir_index < 3) {
    localbox.negbox[dir_index] += block.scale - 0.1f;
  } else {
    localbox.posbox[dir_index-3] -= block.scale - 0.1f;
  }
  
  typename BlockViewT::FreeBlockT* freecont = block.freecontainer;
  while (freecont != nullptr) {
    localbox = freecont->box.transform_out(localbox);
    freecont = freecont->highparent.freecontainer;
  }
  
  return localbox;
}



template <typename BlockViewT>
BlockViewT& BlockSideIterable<BlockViewT>::Iterator::operator*() {
  if (free) {
    return *freeiter;
  } else {
    return *blockiter;
  }
}

template <typename BlockViewT>
bool BlockSideIterable<BlockViewT>::Iterator::operator!=(const Iterator& other) {
  return free != other.free or (free and freeiter != other.freeiter) or (!free and blockiter != other.blockiter);
}

template <typename BlockViewT>
typename BlockSideIterable<BlockViewT>::Iterator BlockSideIterable<BlockViewT>::Iterator::operator++() {
  if (free) {
    ++ freeiter;
  } else {
    ++ blockiter;
  }
  return *this;
}

template <typename BlockViewT>
typename BlockSideIterable<BlockViewT>::Iterator BlockSideIterable<BlockViewT>::begin() {
  Iterator iter {blockiter.begin(), freeiter.begin(), free};
  return iter;
}

template <typename BlockViewT>
typename BlockSideIterable<BlockViewT>::Iterator BlockSideIterable<BlockViewT>::end() {
  Iterator iter {blockiter.end(), freeiter.end(), free};
  return iter;
}


template class BlockSideIterable<BlockView>;
template class BlockSideIterable<ConstBlockView>;





#endif
