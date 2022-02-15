#ifndef BLOCKITER_PREDEF
#define BLOCKITER_PREDEF

#include "classes.h"
#include "physics.h"

template <typename BlockT>
struct BlockTypes {
  
};

template <>
struct BlockTypes<Block> {
  using BlockT = Block;
  using PixelT = Pixel;
  using FreeBlockT = FreeBlock;
  using ColliderT = Collider;
};

template <>
struct BlockTypes<const Block> {
  using BlockT = const Block;
  using PixelT = const Pixel;
  using FreeBlockT = const FreeBlock;
  using ColliderT = const Collider;
};

// Base class for almost all block iterator classes
// iterates over all blocks below the given base, including the base block.
// it has customizable functions, increment_func and valid_block for easier
// specialization

template <typename BlockT>
class BlockIterator { public:
  using BlockType = BlockT;
  using PixelT = typename BlockTypes<BlockT>::PixelT;
  using FreeBlockT = typename BlockTypes<BlockT>::FreeBlockT;
  using ColliderT = typename BlockTypes<BlockT>::ColliderT;
  
  BlockT* curblock;
  int max_scale;
  
  BlockIterator(BlockT* base);
  
  // turns the iterator into an end iterator
  virtual void to_end();
  
  // the start and end point for iterating inside a single block
  virtual ivec3 startpos() { return ivec3(0,0,0); }
  virtual ivec3 endpos() { return ivec3(1,1,1); }
  // increments the position inside a single block, from the startpos to the endpos
  virtual ivec3 increment_func(ivec3 pos);
  
  // moves one step farther down in the block tree. goes to the block
  // at startpos in block. if the block doesnt continue, calls step_sideways.
  // calls get_safe at end
  void step_down(BlockT* parent, ivec3 curpos, BlockT* block);
  // increments curpos and gets the new block from parent. calls get_safe at end
  void step_sideways(BlockT* parent, ivec3 curpos, BlockT* block);
  // gets the iterator to a safe block. if the block is not valid, calls step_sideways,
  // and if the block should be skipped, calls step_down.
  void get_safe(BlockT* parent, ivec3 curpos, BlockT* block);
  
  // whether a block is valid, if a block is not valid the iterator will skip the block
  // and all of its children
  virtual bool valid_block(BlockT* block) { return block != nullptr; }
  // whether a block should be skipped, only skips the single block, the iterator will
  // still go to its children
  virtual bool skip_block(BlockT* block) { return false; }
  // called when the iterator has reached the end
  virtual void finish() { curblock = nullptr; }
  
  BlockIterator<BlockT> operator++();
  BlockT* operator*();
  bool operator != (const BlockIterator<BlockT>& other) const;
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
template <typename BlockT>
class PixelIterator : public BlockIterator<BlockT> { public:
  using BlockIterator<BlockT>::BlockIterator;
  
  virtual bool skip_block(BlockT* block);
  typename BlockIterator<BlockT>::PixelT* operator*();
};

// iterates over only the pixels touching the specified side
template <typename BlockT>
class DirPixelIterator : public PixelIterator<BlockT> { public:
  ivec3 dir;
  
  DirPixelIterator(BlockT* base, ivec3 newdir): PixelIterator<BlockT>(base), dir(newdir) {}
  
  virtual ivec3 startpos() { return (dir+1)/2 * (csize-1); }
  virtual ivec3 endpos() { return (1-(1-dir)/2) * (csize-1); }
};

// only iterates over all pixels colliding with a specified hitbox
template <typename BlockT>
class FreePixelIterator : public PixelIterator<BlockT> { public:
  Hitbox box;
  
  FreePixelIterator(BlockT* base, Hitbox nbox): PixelIterator<BlockT>(base), box(nbox) {}
  
  virtual bool valid_block(BlockT* block);
};

// iterates over all freeblocks in a block
template <typename BlockT>
class FreeBlockIterator: public BlockIterator<BlockT> { public:
  Hitbox box;
  
  FreeBlockIterator(BlockT* base, Hitbox nbox): BlockIterator<BlockT>(base), box(nbox) {}
  
  virtual bool valid_block(BlockT* block);
  virtual bool skip_block(BlockT* block);
  typename BlockTypes<BlockT>::FreeBlockT* operator*();
};

template <typename BlockT>
class ChunkIterator: public BlockIterator<BlockT> { public:
  using BlockIterator<BlockT>::BlockIterator;
  
  virtual bool valid_block(BlockT* block);
  virtual bool skip_block(BlockT* block);
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
  using ColliderT = typename Iterator::ColliderT;
  HitboxIterable(ColliderT* world, Hitbox box);
};

// iterable that finds all bases needed to find all freeblocks touching a given hitbox
template <typename Iterator>
class FreeboxIterable : public MultiBaseIterable<Iterator> { public:
  using BlockT = typename Iterator::BlockType;
  using ColliderT = typename Iterator::ColliderT;
  FreeboxIterable(ColliderT* world, Hitbox box);
};

// iterates over all pixels touching a hitbox, including freeblocks
template <typename BlockT>
class FullFreePixelIterable : public HitboxIterable<FreePixelIterator<BlockT>> { public:
  using ColliderT = typename BlockTypes<BlockT>::ColliderT;
  FullFreePixelIterable(ColliderT* world, Hitbox box, const Block* ignore = nullptr, bool ignore_world = false);
};

// iterates over all pixels that touch the side of a block
// if the given block is a freeblock and it is on the edge, it will
// use a freeiter to look in the base world
template <typename BlockT>
class BlockSideIterable { public:
  using PixelT = typename BlockTypes<BlockT>::PixelT;
  
  BlockIterable<DirPixelIterator<BlockT>> blockiter;
  HitboxIterable<FreePixelIterator<BlockT>> freeiter;
  bool free = false;
  
  BlockSideIterable(BlockT* block, ivec3 dir);
  static Hitbox make_side_hitbox(BlockT* block, ivec3 dir);
  
  class Iterator { public:
    DirPixelIterator<BlockT> blockiter;
    typename HitboxIterable<FreePixelIterator<BlockT>>::ComboIterator freeiter;
    bool free;
    
    PixelT* operator*();
    Iterator operator++();
    bool operator!=(const Iterator& other);
  };
  
  Iterator begin();
  Iterator end();
};


#endif
