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
class BlockIter { public:
  BlockT* base;
  
  class iterator { public:
    BlockT* curblock;
    int max_scale;
    
    iterator(BlockIter<BlockT>* parent);
    
    virtual ivec3 startpos() { return ivec3(0,0,0); }
    virtual ivec3 endpos() { return ivec3(1,1,1); }
    
    virtual ivec3 increment_func(ivec3 pos);
    void step_down(BlockT* parent, ivec3 curpos, BlockT* block);
    void step_sideways(BlockT* parent, ivec3 curpos, BlockT* block);
    void get_safe(BlockT* parent, ivec3 curpos, BlockT* block);
    virtual bool valid_block(BlockT* block) { return block != nullptr; }
    virtual bool skip_block(BlockT* block) { return false; }
    virtual void finish() { curblock = nullptr; }
    
    iterator operator++();
    BlockT* operator*();
    bool operator != (const iterator& other) const;
  };
  
  BlockIter(BlockT* newbase = nullptr): base(newbase) {}
  
  BLOCKITER_BEGIN_END;
};

// template <typename BlockT>
// bool operator==(const typename BlockIter<BlockT>::iterator& iter1, const typename BlockIter<BlockT>::iterator& iter2);
// template <typename BlockT>
// bool operator!=(const typename BlockIter<BlockT>::iterator& iter1, const typename BlockIter<BlockT>::iterator& iter2) { return !(iter1 == iter2); }



template <typename BlockT>
struct GetPixelT {
  
};

template <>
struct GetPixelT<Block> {
  using PixelT = Pixel;
};

template <>
struct GetPixelT<const Block> {
  using PixelT = const Pixel;
};



template <typename BlockT>
class PixelIter : public BlockIter<BlockT> { public:
  using PixelT = typename GetPixelT<BlockT>::PixelT;
  
  class iterator : public BlockIter<BlockT>::iterator { public:
    using BlockIter<BlockT>::iterator::iterator;
    
    virtual bool skip_block(BlockT* block);
    PixelT* operator*();
  };
  
  using BlockIter<BlockT>::BlockIter;
  
  BLOCKITER_BEGIN_END;
};


template <typename BlockT>
class DirPixelIter : public PixelIter<BlockT> { public:
  ivec3 dir;
  
  class iterator : public PixelIter<BlockT>::iterator { public:
    ivec3 dir;
    
    iterator(DirPixelIter<BlockT>* parent): PixelIter<BlockT>::iterator(parent), dir(parent->dir) {}
    
    virtual ivec3 startpos() { return (dir+1)/2 * (csize-1); }
    virtual ivec3 endpos() { return (1-(1-dir)/2) * (csize-1); }
  };
  
  DirPixelIter(BlockT* base, ivec3 newdir): PixelIter<BlockT>(base), dir(newdir) {}
  DirPixelIter() {}
  
  BLOCKITER_BEGIN_END;
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
		DirPixelIter<Block> blockiter;
		FreeBlockIter freeiter;
	// };
	bool free;
	class iterator { public:
		BlockTouchSideIter* parent;
		// union {
			DirPixelIter<Block>::iterator blockiter;
			FreeBlockIter::iterator freeiter;
		// };
    
    iterator(BlockTouchSideIter* newparent): parent(newparent), blockiter(&parent->blockiter) {}
    
    Pixel* operator*();
    iterator operator++();
    friend bool operator != (const iterator& iter1, const iterator& iter2);
	};
	BlockTouchSideIter(Block* block, ivec3 dir);
	iterator begin();
	iterator end();
};


#endif
