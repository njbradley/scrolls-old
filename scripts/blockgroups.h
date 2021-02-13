#ifndef BLOCKGROUPS_PREDEF
#define BLOCKGROUPS_PREDEF

#include "classes.h"
#include "items.h"

#include <unordered_set>
using std::unordered_set;

class GroupConnection { public:
	BlockGroup* group;
	int globalindex;
	ConnectorData* data;
	int damage = 0;
	GroupConnection(BlockGroup* newgroup, ConnectorData* newdata);
	GroupConnection(int index, ConnectorData* mat);
};

class BlockGroup { public:
	int size = 0;
	World* world;
	int consts[6] = {false, false, false, false, false, false};
	int id;
	bool isolated = true;
	
	BlockGroup(World* nworld);
	BlockGroup(World* nworld, vector<Pixel*> pixels);
	BlockGroup(World* nworld, istream& ifile);
	virtual ~BlockGroup();
	virtual bool add(Pixel* pix, Pixel* sourcepix, ivec3 dir);
	void spread(Pixel* pix, Pixel* source, ivec3 dir, int depth = 10);
	virtual bool del(Pixel* pix);
	virtual bool contains(Pixel* pix);
	virtual void to_file(ostream& ofile, vector<BlockGroup*>* allgroups);
	static constexpr ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	static const int max_size = 5000;
};


#endif
