#ifndef BLOCKPHYSICS_PREDEF
#define BLOCKPHYSICS_PREDEF

#include "classes.h"
#include <unordered_set>
using std::unordered_set;

struct BlockGroup {
	ivec3 position;
	// negative corner of the box surrounding the blocks
	ivec3 size;
	// bounding box of all blocks
	Block* block;
	unordered_set<ivec3,ivec3_hash> block_poses;
	unordered_set<ivec3,ivec3_hash> supported_blocks;
	// list of all blocks in the group
	bool consts[6] = {false, false, false, false, false, false};
	World* world;
	int max_size;
	BlockGroup(World* world, ivec3 starting_pos, int max);
	void find_group();
	void copy_to_block();
	void remove_pix_pointers();
	void erase_from_world();
	void copy_to_world(ivec3 newpos);
private:
	void find_block(ivec3 newpos);
};


#endif
