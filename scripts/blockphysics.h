#ifndef BLOCKPHYSICS_PREDEF
#define BLOCKPHYSICS_PREDEF

#include "classes.h"
#include "items.h"

#include <unordered_set>
using std::unordered_set;

class BlockGroup { public:
	bool consts[6] = {0,0,0,0,0,0};
	World* world;
	char blockval;
	BlockGroup(World* nworld);
	virtual bool contains(ivec3 block) = 0;
	virtual int getsize() = 0;
	virtual bool add(ivec3 block) = 0;
	virtual bool del(ivec3 block) = 0;
	void spread(unordered_set<ivec4,ivec4_hash>& last_poses);
	bool spread_to_fill(ivec3 startpos);
	virtual void debug(stringstream& debugstream) = 0;
};

class UnlinkedGroup: public BlockGroup { public:
	int size = 0;
	ivec3 position;
	ivec3 bounds;
	UnlinkedGroup(World* nworld);
	virtual bool contains(ivec3 block);
	virtual int getsize();
	virtual bool add(ivec3 block);
	virtual bool del(ivec3 block);
	virtual void debug(stringstream& debugstream);
};

#endif
