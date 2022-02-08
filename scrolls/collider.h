#ifndef COLLIDER_PREDEF
#define COLLIDER_PREDEF

#include "classes.h"

struct Collider {
  // virtual Block * get_global(int x,int y,int z,int scale) = 0;
	virtual BlockView get_global(ivec3 pos, int scale) = 0;// {return get_global(pos.x, pos.y, pos.z, scale);}
  virtual ConstBlockView get_global(ivec3 pos, int scale) const = 0;
  // this is a function shared by all classes that hold
  // chunks/pixels in them
  // it returns a block that is at the global position xyz
  // and at scale scale
  // to make sure a pixel is returned, use scale 1 and call
  // get_pix() on the result
  // important: nullptr is returned on out of range, so always check for null
};


class Container : public Collider { public:
  // FreeBlock* allfreeblocks = nullptr;
	// virtual void add_freeblock(FreeBlock* freeblock) = 0;
	// virtual void remove_freeblock(FreeBlock* freeblock) = 0;
	virtual Block* get_child(ivec3 pos) = 0;
	virtual const Block* get_child(ivec3 pos) const = 0;
	virtual void set_child(ivec3 pos, Block* child) = 0;
};


#endif
