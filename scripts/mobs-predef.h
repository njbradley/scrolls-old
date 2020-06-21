#ifndef MOBS_PREDEF
#define MOBS_PREDEF

#include "classes.h"
#include "entity-predef.h"

class Mob: public NamedEntity { public:
	Entity* target;
	ivec3 next_point;
	ivec3 last_point;
	
	Mob(vec3 start_pos, string name);
	virtual void tick();
	virtual void find_target();
	virtual void find_next_point();
};

#endif
