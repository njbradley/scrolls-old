#ifndef BLOCKITER_PREDEF
#define BLOCKITER_PREDEF

#include "classes.h"
#include "physics.h"


#define BLOCKITER_BEGIN_END \
  iterator end() { iterator iter (this); iter.curblock = nullptr; return iter; } \
  iterator begin() { if (this->base == nullptr) return end(); iterator iter (this); iter.get_safe(this->base->parent, this->base->parentpos, this->base); return iter; }

// Base class for almost all block iterator classes
// iterates over all blocks below the given base, including the base block.
// it has customizable functions, increment_func and valid_block for easier
// specialization


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



template <typename BlockT>
class BlockIterator { public:
  using BlockType = BlockT;
  using PixelT = typename BlockTypes<BlockT>::PixelT;
  using FreeBlockT = typename BlockTypes<BlockT>::FreeBlockT;
  using ColliderT = typename BlockTypes<BlockT>::ColliderT;
  
  BlockT* curblock;
  int max_scale;
  
  BlockIterator(BlockT* base);
  
  virtual void to_end();
  
  virtual ivec3 startpos() { return ivec3(0,0,0); }
  virtual ivec3 endpos() { return ivec3(1,1,1); }
  
  virtual ivec3 increment_func(ivec3 pos);
  void step_down(BlockT* parent, ivec3 curpos, BlockT* block);
  void step_sideways(BlockT* parent, ivec3 curpos, BlockT* block);
  void get_safe(BlockT* parent, ivec3 curpos, BlockT* block);
  virtual bool valid_block(BlockT* block) { return block != nullptr; }
  virtual bool skip_block(BlockT* block) { return false; }
  virtual void finish() { curblock = nullptr; }
  
  BlockIterator<BlockT> operator++();
  BlockT* operator*();
  bool operator != (const BlockIterator<BlockT>& other) const;
};


template <typename Iterator>
class BlockIterable { public:
  Iterator iterator;
  
  template <typename ... Args>
  BlockIterable(Args ... args): iterator(args...) {}
  
  Iterator begin() const;
  Iterator end() const;
};



template <typename BlockT>
class PixelIterator : public BlockIterator<BlockT> { public:
  using BlockIterator<BlockT>::BlockIterator;
  
  virtual bool skip_block(BlockT* block);
  typename BlockIterator<BlockT>::PixelT* operator*();
};


template <typename BlockT>
class DirPixelIterator : public PixelIterator<BlockT> { public:
  ivec3 dir;
  
  DirPixelIterator(BlockT* base, ivec3 newdir): PixelIterator<BlockT>(base), dir(newdir) {}
  
  virtual ivec3 startpos() { return (dir+1)/2 * (csize-1); }
  virtual ivec3 endpos() { return (1-(1-dir)/2) * (csize-1); }
};


template <typename BlockT>
class FreePixelIterator : public PixelIterator<BlockT> { public:
  Hitbox box;
  
  FreePixelIterator(BlockT* base, Hitbox nbox): PixelIterator<BlockT>(base), box(nbox) {}
  
  virtual bool valid_block(BlockT* block);
};

template <typename BlockT>
class FreeBlockIterator: public BlockIterator<BlockT> { public:
  Hitbox box;
  
  FreeBlockIterator(BlockT* base, Hitbox nbox): BlockIterator<BlockT>(base), box(nbox) {}
  
  virtual bool valid_block(BlockT* block);
  virtual bool skip_block(BlockT* block);
  typename BlockTypes<BlockT>::FreeBlockT* operator*();
};







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


template <typename Iterator>
class HitboxIterable : public MultiBaseIterable<Iterator> { public:
  using BlockT = typename Iterator::BlockType;
  using ColliderT = typename Iterator::ColliderT;
  HitboxIterable(ColliderT* world, Hitbox box);
};

template <typename Iterator>
class FreeboxIterable : public MultiBaseIterable<Iterator> { public:
  using BlockT = typename Iterator::BlockType;
  using ColliderT = typename Iterator::ColliderT;
  FreeboxIterable(ColliderT* world, Hitbox box);
};


template <typename BlockT>
class FullFreePixelIterable : public HitboxIterable<FreePixelIterator<BlockT>> { public:
  using ColliderT = typename BlockTypes<BlockT>::ColliderT;
  FullFreePixelIterable(ColliderT* world, Hitbox box, const Block* ignore = nullptr, bool ignore_world = false);
};


template <typename BlockT>
class BlockSideIterable { public:
  using PixelT = typename BlockTypes<BlockT>::PixelT;
  
  BlockIterable<DirPixelIterator<BlockT>> blockiter;
  HitboxIterable<FreePixelIterator<BlockT>> freeiter;
  // FullFreePixelIterable<BlockT> freeiter;
  bool free = false;
  
  BlockSideIterable(BlockT* block, ivec3 dir);
  static Hitbox make_side_hitbox(BlockT* block, ivec3 dir);
  
  class Iterator { public:
    DirPixelIterator<BlockT> blockiter;
    FreePixelIterator<BlockT> freeiter;
    // typename FullFreePixelIterable<BlockT>::ComboIterator freeiter;
    bool free;
    
    PixelT* operator*();
    Iterator operator++();
    bool operator!=(const Iterator& other);
  };
  
  Iterator begin();
  Iterator end();
};


class FreeBlockIter { public:
	Block* bases[8];
  int num_bases;
  Hitbox box;
	ivec3 start_pos = ivec3(0,0,0);
	ivec3 end_pos = ivec3(csize,csize,csize)-1;
	ivec3 (*increment_func)(ivec3 pos, ivec3 startpos, ivec3 endpos);
  class iterator { public:
    FreeBlockIter* parent;
		Block* base;
		int baseindex = 0;
    Pixel* pix;
    Pixel* operator*();
    iterator operator++();
		void increment(Block* block);
		void get_to_pix(Block* block);
    friend bool operator!=(const iterator& iter1, const iterator& iter2);
  };
  FreeBlockIter(Collider* world, Hitbox box);
	FreeBlockIter() {};
  iterator begin();
  iterator end();
};

class FreeBlockFreeIter { public:
  FreeBlock* freebases[8];
  int num_freebases;
  class iterator { public:
    FreeBlock** bases;
    int baseindex;
    FreeBlock* free;
    FreeBlock* operator*();
    iterator operator++();
    friend bool operator!=(const iterator& iter1, const iterator& iter2);
  };
  FreeBlockFreeIter(Collider* world, Hitbox box);
  Hitbox grow_box(Hitbox box);
  iterator begin();
  iterator end();
};

class BlockTouchSideIter { public:
	Block* base;
	Collider* world;
	// union {
		BlockIterable<DirPixelIterator<Block>> blockiter;
		HitboxIterable<FreePixelIterator<Block>> freeiter;
	// };
	bool free;
	class iterator { public:
		BlockTouchSideIter* parent;
		// union {
			DirPixelIterator<Block> blockiter;
			FreePixelIterator<Block> freeiter;
		// };
    
    iterator(BlockTouchSideIter* newparent): parent(newparent), blockiter(nullptr, ivec3(0,0,0)), freeiter(nullptr, Hitbox()) {}
    
    Pixel* operator*();
    iterator operator++();
    friend bool operator != (const iterator& iter1, const iterator& iter2);
	};
	BlockTouchSideIter(Block* block, ivec3 dir);
	iterator begin();
	iterator end();
};


#endif
