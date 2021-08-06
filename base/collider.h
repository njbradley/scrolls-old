#ifndef COLLIDER_PREDEF
#define COLLIDER_PREDEF

#include "classes.h"

struct Collider {
  virtual Block * get_global(int x,int y,int z,int scale) = 0;
	virtual Block* get_global(ivec3 pos, int scale) {return get_global(pos.x, pos.y, pos.z, scale);}
  // this is a function shared by all classes that hold
  // chunks/pixels in them
  // it returns a block that is at the global position xyz
  // and at scale scale
  // to make sure a pixel is returned, use scale 1 and call
  // get_pix() on the result
  // important: nullptr is returned on out of range, so always check for null
  virtual vec3 get_position() const = 0;
};


class Container : public Collider { public:
	virtual void block_update(ivec3 pos) = 0;
	virtual void set_global(ivec3 pos, int w, Blocktype val, int direc, int joints[6] = nullptr) = 0;
	virtual void add_freeblock(FreeBlock* freeblock) = 0;
	virtual void remove_freeblock(FreeBlock* freeblock) = 0;
};


#endif
