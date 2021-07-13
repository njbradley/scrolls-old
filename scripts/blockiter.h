#ifndef BLOCKITER_PREDEF
#define BLOCKITER_PREDEF

#include "classes.h"
#include "entity.h"


class BlockIter { public:
  Block* base;
  ivec3 start_pos;
  ivec3 end_pos;
  ivec3 (*increment_func)(ivec3 pos, ivec3 startpos, ivec3 endpos);
  class iterator { public:
    BlockIter* parent;
    Pixel* pix;
    Pixel* operator*();
    iterator operator++();
    friend bool operator!=(const iterator& iter1, const iterator& iter2);
  };
  iterator begin();
  iterator end();
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
  

class BlockTouchSideIter { public:
	Block* base;
	Collider* world;
	// union {
		BlockIter blockiter;
		FreeBlockIter freeiter;
	// };
	bool free;
	class iterator { public:
		BlockTouchSideIter* parent;
		// union {
			BlockIter::iterator blockiter;
			FreeBlockIter::iterator freeiter;
		// };
    Pixel* operator*();
    iterator operator++();
    friend bool operator != (const iterator& iter1, const iterator& iter2);
	};
	BlockTouchSideIter(Block* block, ivec3 dir);
	iterator begin();
	iterator end();
};


class BlockTouchIter { public:
  Block* base;
  Collider* world;
  BlockIter blockiter;
  class iterator { public:
    BlockTouchIter* parent;
    int dir_index;
    BlockIter::iterator iter;
    Pixel* operator*();
    iterator operator++();
    friend bool operator != (const iterator& iter1, const iterator& iter2);
  };
  iterator begin();
  iterator end();
	static constexpr ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
};

class BlockGroupIter { public:
  Pixel* base;
  Collider* world;
  bool (*includefunc)(Pixel* base, Pixel* pix);
  unordered_set<Pixel*> pixels;
  BlockGroupIter(Pixel* newbase, Collider* nworld, bool (*func)(Pixel* base, Pixel* pix));
  unordered_set<Pixel*>::iterator begin();
  unordered_set<Pixel*>::iterator end();
  void swap(unordered_set<Pixel*>& other);
	static constexpr ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
};
    

class ConstBlockIter { public:
  const Block* base;
  ivec3 start_pos;
  ivec3 end_pos;
  ivec3 (*increment_func)(ivec3 pos, ivec3 startpos, ivec3 endpos);
  class iterator { public:
    ConstBlockIter* parent;
    const Pixel* pix;
    const Pixel* operator*();
    iterator operator++();
    friend bool operator!=(const iterator& iter1, const iterator& iter2);
  };
  iterator begin();
  iterator end();
};

#endif
