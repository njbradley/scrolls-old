#ifndef BLOCKPHYSICS_PREDEF
#define BLOCKPHYSICS_PREDEF

#include "classes.h"
#include <unordered_set>
using std::unordered_set;

class BlockGroup { public:
	ivec3 position;
	// negative corner of the box surrounding the blocks
	ivec3 size;
	// bounding box of all blocks
	Block* block;
	string groupname;
	bool update_flag;
	unordered_set<ivec3,ivec3_hash> block_poses;
	unordered_set<ivec3,ivec3_hash> supported_blocks;
	unordered_set<BlockGroup*> neighbors;
	// list of all blocks in the group
	bool consts[6] = {false, false, false, false, false, false};
	World* world;
	BlockGroup(World* world, ivec3 starting_pos);
	BlockGroup(World* world, istream& ifile);
	~BlockGroup();
	void find_group();
	void update();
	void copy_to_block();
	void remove_pix_pointers();
	void set_pix_pointers();
	void erase_from_world();
	void copy_to_world(ivec3 newpos);
	virtual AxleInterface* cast_axle();
	virtual void add_item(ItemStack stack);
	//virtual PipeInterface* cast_pipe();
	virtual void tick();
	virtual bool rcaction();
	virtual bool persistant();
	virtual void on_update();
	virtual void link();
	virtual string group_group();
	virtual void to_file(ostream& ofile);
	static BlockGroup* make_group(char val, World* world, ivec3 pos);
	static BlockGroup* from_file(World* world, istream& ifile);
	static bool is_persistant(char val);
	const static int max_size = 500;
};

class AxleInterface: public BlockGroup { public:
	AxleInterface(World* world, ivec3 starting_pos);
	AxleInterface(World* world, istream& ifile);
	virtual void rotate(AxleInterface* sender, double amount, double force) = 0;
	virtual AxleInterface* cast_axle();
	string group_group();
	bool persistant();
};

class AxleGroup: public AxleInterface { public:
	AxleInterface* port1;
	AxleInterface* port2;
	ivec3 port1pos;
	ivec3 port2pos;
	ivec3 start;
	ivec3 end;
	int axis;
	bool is_valid;
	AxleGroup(World* world, ivec3 starting_pos);
	AxleGroup(World* world, istream& ifile);
	void on_update();
	void find_neighbors();
	void rotate(AxleInterface* sender, double amount, double force);
	bool rcaction();
	void link();
	void to_file(ostream& ofile);
private:
	bool calc_is_valid();
};

class GrindstoneGroup: public AxleInterface { public:
	ItemContainer input;
	double progress;
	GrindstoneGroup(World* world, ivec3 spos);
	GrindstoneGroup(World* world, istream& ifile);
	bool rcaction();
	void rotate(AxleInterface* sender, double amount, double force);
	void to_file(ostream& ofile);
};

// class WheelGroup: public AxleInterface { public:
// 	vector<AxleInterface*> ports;
// 	bool rcaction();

class ChestGroup: public BlockGroup { public:
	ItemContainer inven;
	ChestGroup(World* world, ivec3 spos);
	ChestGroup(World* world, istream& ifile);
	virtual bool rcaction();
	void add_item(ItemStack stack);
	void on_update();
	void update_inven_size();
	bool persistant();
	void to_file(ostream& ofile);
};

class BackpackGroup: public ChestGroup { public:
	BackpackGroup(World* world, ivec3 spos);
	BackpackGroup(World* world, istream& ifile);
	bool rcaction();
};

class CraftingGroup: public BlockGroup { public:
	CraftingGroup(World* world, ivec3 starting_pos);
	CraftingGroup(World* world, istream& ifile);
	bool rcaction();
	bool persistant();
};

class PipePacket { public:
	Item item;
	bool water;
	PipePacket(Item item, bool water);
};



#endif
