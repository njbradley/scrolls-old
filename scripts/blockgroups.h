#ifndef BLOCKGROUPS_PREDEF
#define BLOCKGROUPS_PREDEF

#include "classes.h"
#include "items.h"

#include <unordered_set>
using std::unordered_set;

class BlockGroup { public:
	int size = 0;
	World* world;
	bool consts[6] = {false, false, false, false, false, false};
	bool final = false;
	bool uniform_type = true;
	char blocktype;
	BlockGroup(World* nworld);
	BlockGroup(World* nworld, vector<Pixel*> pixels);
	virtual ~BlockGroup();
	virtual bool add(Pixel* pix);
	void spread(Pixel* pix, int depth = 10);
	virtual bool del(Pixel* pix);
	virtual bool can_take(Pixel* pix);
	virtual bool contains(Pixel* pix);
	static constexpr ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
};


#endif
