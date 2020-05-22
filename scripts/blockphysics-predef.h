#ifndef BLOCKPHYSICS_PREDEF
#define BLOCKPHYSICS_PREDEF

#include "classes.h"

struct BlockGroup {
	ivec3 pos;
	ivec3 size;
	Block* block;
	vector<ivec3> blocks;
	BlockGroup(ivec3 starting_pos);
	void find_group();
	void copy_to_block();
	void erase_from_world();
	void copy_to_world(ivec3 newpos);
private:
	void find_block(ivec3 newpos);
}


#endif
