#ifndef BLOCKITER_PREDEF
#define BLOCKITER_PREDEF

#include "classes.h"
#include "physics.h"
#include "blocks.h"

// Base class for almost all block iterator classes
// iterates over all blocks below the given base, including the base block.
// it has customizable functions, increment_func and valid_block for easier
// specialization

template <typename BlockViewT>
class BlockIterator : public BlockViewT { public:

// template <typename BlockT>
// class BlockIterator { public:
//   using BlockType = BlockT;
//   using PixelT = typename BlockTypes<BlockT>::PixelT;
//   using FreeBlockT = typename BlockTypes<BlockT>::FreeBlockT;
//   using ColliderT = typename BlockTypes<BlockT>::ColliderT;
//
//   BlockT* curblock;
  
  using BlockType = typename BlockViewT::BlockType;
  using BlockViewType = BlockViewT;
  using PixelT = typename BlockTypes<BlockType>::PixelT;
  using FreeBlockT = typename BlockTypes<BlockType>::FreeBlockT;
  using ColliderT = typename BlockTypes<BlockType>::ColliderT;
  
  using BlockViewT::curblock;
  using BlockViewT::parent;
  using BlockViewT::scale;
  using BlockViewT::globalpos;
  using BlockViewT::hitbox;
  using BlockViewT::freebox;
  
  int max_scale;
  
  BlockIterator(BlockViewT& base);
  
  using BlockViewT::isvalid;
  using BlockViewT::parentpos;
  using BlockViewT::step_up;
  
  // turns the iterator into an end iterator
  virtual void to_end();
  
  // the start and end point for iterating inside a single block
  virtual ivec3 startpos() { return ivec3(0,0,0); }
  virtual ivec3 endpos() { return ivec3(1,1,1); }
  // increments the position inside a single block, from the startpos to the endpos
  virtual ivec3 increment_func(ivec3 pos);
  
  // moves one step farther down in the block tree. goes to the block
  // at startpos in block. if the block doesnt continue, calls step_side.
  // calls get_safe at end
  void step_down();
  using BlockViewT::step_down;
  // increments curpos and gets the new block from parent. calls get_safe at end
  void step_side();
  using BlockViewT::step_side;
  // gets the iterator to a safe block. if the block is not valid, calls step_side,
  // and if the block should be skipped, calls step_down.
  void get_safe();
  
  // whether a block is valid, if a block is not valid the iterator will skip the block
  // and all of its children
  virtual bool valid_block() { return curblock != nullptr; }
  // whether a block should be skipped, only skips the single block, the iterator will
  // still go to its children
  virtual bool skip_block() { return false; }
  // called when the iterator has reached the end
  virtual void finish() { curblock = nullptr; }
  
  BlockIterator<BlockViewT> operator++();
  BlockViewT& operator*();
  bool operator != (const BlockIterator<BlockViewT>& other) const;
};

// A container for an iterator, forwards all constructor arguments to the
// iterator constructor.
template <typename Iterator>
class BlockIterable { public:
  Iterator iterator;
  
  template <typename ... Args>
  BlockIterable(Args ... args): iterator(args...) {}
  
  // the beginning iterator is created using the arguments passed
  // to the constructor, and then get_safe is called
  Iterator begin() const;
  // the end iterator is created using the same arguments, then
  // to_end is called to make it an end iterator
  Iterator end() const;
};


// iterates over all pixels in a block
template <typename BlockViewT>
class PixelIterator : public BlockIterator<BlockViewT> { public:
  using BlockIterator<BlockViewT>::curblock;
  using BlockIterator<BlockViewT>::BlockIterator;
  virtual bool skip_block();
};

// iterates over only the pixels touching the specified side
template <typename BlockViewT>
class DirPixelIterator : public PixelIterator<BlockViewT> { public:
  ivec3 dir;
  
  DirPixelIterator(BlockViewT& base, ivec3 newdir): PixelIterator<BlockViewT>(base), dir(newdir) {}
  
  virtual ivec3 startpos() { return (dir+1)/2 * (csize-1); }
  virtual ivec3 endpos() { return (1-(1-dir)/2) * (csize-1); }
};

// only iterates over all pixels colliding with a specified hitbox
template <typename BlockViewT>
class FreePixelIterator : public PixelIterator<BlockViewT> { public:
  Hitbox box;
  using BlockIterator<BlockViewT>::curblock;
  
  FreePixelIterator(BlockViewT& base, Hitbox nbox): PixelIterator<BlockViewT>(base), box(nbox) {}
  
  virtual bool valid_block();
};

// iterates over all freeblocks in a block
template <typename BlockViewT>
class FreeBlockIterator: public BlockIterator<BlockViewT> { public:
  Hitbox box;
  using BlockIterator<BlockViewT>::curblock;
  
  FreeBlockIterator(BlockViewT& base, Hitbox nbox): BlockIterator<BlockViewT>(base), box(nbox) {}
  
  virtual bool valid_block();
  virtual bool skip_block();
};






// This is a base class for iterables that have multiple base blocks
// calling add_base with parameters to construct Iterator adds a new
// base
// they are all chained together and iterated through one after another
template <typename Iterator>
class MultiBaseIterable { public:
  vector<Iterator> bases;
  
  template <typename ... Args>
  void add_base(Args ... args) { bases.emplace_back(args...); }
  
  class ComboIterator : public Iterator { public:
    Iterator* bases;
    int base_index;
    
    ComboIterator(Iterator* base_list, int num_bases);
    
    virtual void finish();
  };
  
  ComboIterator begin();
  ComboIterator end();
};

// iterable that finds all bases needed to contain a given hitbox
template <typename Iterator>
class HitboxIterable : public MultiBaseIterable<Iterator> { public:
  using BlockT = typename Iterator::BlockType;
  using BlockViewT = typename Iterator::BlockViewType;
  using ColliderT = typename Iterator::ColliderT;
  HitboxIterable(ColliderT* world, Hitbox box);
};

// iterable that finds all bases needed to find all freeblocks touching a given hitbox
template <typename Iterator>
class FreeboxIterable : public MultiBaseIterable<Iterator> { public:
  using BlockT = typename Iterator::BlockType;
  using BlockViewT = typename Iterator::BlockViewType;
  using ColliderT = typename Iterator::ColliderT;
  FreeboxIterable(ColliderT* world, Hitbox box);
};

// iterates over all pixels touching a hitbox, including freeblocks
template <typename BlockViewT>
class FullFreePixelIterable : public HitboxIterable<FreePixelIterator<BlockViewT>> { public:
  using ColliderT = typename BlockTypes<typename BlockViewT::BlockType>::ColliderT;
  using FreeBlockT = typename BlockViewT::FreeBlockT;
  FullFreePixelIterable(ColliderT* world, Hitbox box, const Block* ignore = nullptr, bool ignore_world = false);
};

// iterates over all pixels that touch the side of a block
// if the given block is a freeblock and it is on the edge, it will
// use a freeiter to look in the base world
template <typename BlockViewT>
class BlockSideIterable { public:
  using PixelT = typename BlockViewT::PixelT;
  
  BlockIterable<DirPixelIterator<BlockViewT>> blockiter;
  HitboxIterable<FreePixelIterator<BlockViewT>> freeiter;
  bool free = false;
  
  BlockSideIterable(BlockViewT& base, ivec3 dir);
  static Hitbox make_side_hitbox(BlockViewT& view, ivec3 dir);
  
  class Iterator { public:
    DirPixelIterator<BlockViewT> blockiter;
    typename HitboxIterable<FreePixelIterator<BlockViewT>>::ComboIterator freeiter;
    bool free;
    
    BlockViewT& operator*();
    Iterator operator++();
    bool operator!=(const Iterator& other);
  };
  
  Iterator begin();
  Iterator end();
};


#endif
