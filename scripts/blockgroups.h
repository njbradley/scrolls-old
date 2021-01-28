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
	GroupConnection(BlockGroup* newgroup, ConnectorData* newdata);
	GroupConnection(int index, ConnectorData* mat);
};

class BlockGroup { public:
	int size = 0;
	World* world;
	int consts[6] = {false, false, false, false, false, false};
	int id;
	bool final = false;
	bool uniform_type = true;
	Item item;
	char blocktype;
	vector<GroupConnection> connections;
	BlockGroup* taking_over = nullptr;
	bool recalculate_flag = false;
	
	BlockGroup(World* nworld);
	BlockGroup(World* nworld, vector<Pixel*> pixels);
	BlockGroup(World* nworld, istream& ifile);
	virtual ~BlockGroup();
	void link(vector<BlockGroup*>* allgroups);
	void merge_copy(BlockGroup* other);
	virtual bool add(Pixel* pix);
	void spread(Pixel* pix, int depth = 10);
	virtual bool del(Pixel* pix);
	virtual bool can_take(Pixel* pix);
	virtual bool contains(Pixel* pix);
	virtual void to_file(ostream& ofile, vector<BlockGroup*>* allgroups);
	void recalculate_all(Pixel* pix);
	bool constrained(int index, vector<BlockGroup*>* past_groups = nullptr);
	void connect_to(GroupConnection connection);
	GroupConnection* get_connection(BlockGroup* othergroup);
	void del_connection(BlockGroup* othergroup);
	static constexpr ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
};


#endif
