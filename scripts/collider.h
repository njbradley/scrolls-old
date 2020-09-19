#ifndef COLLIDER_PREDEF
#define COLLIDER_PREDEF

#include "classes.h"

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

#endif
