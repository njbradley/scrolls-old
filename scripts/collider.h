#ifndef COLLIDER_PREDEF
#define COLLIDER_PREDEF

#include "classes.h"

class Axes {
	vec3 offset = vec3(0,0,0);
	quat rotation = quat(0,0,0,1);
	
	Axes(vec3 newoff = vec3(0,0,0), quat newrot = quat(0,0,0,1));
	vec3 transform(vec3 pos) const;
	ivec3 transformi(ivec3 pos) const;
	vec3 reverse(vec3 pos) const;
	ivec3 reversei(ivec3 pos) const;
};

struct Collider {
  virtual Block * get_global(int x,int y,int z,int scale) = 0;
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
};


#endif
