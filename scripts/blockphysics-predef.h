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
	BlockGroup(World* world, unordered_set<ivec3,ivec3_hash>& poses, Block* newblock);
	~BlockGroup();
	virtual void find_group();
	void update();
	void copy_to_block();
	void remove_pix_pointers();
	void set_pix_pointers();
	void erase_from_world();
	void copy_to_world(ivec3 newpos);
	virtual void custom_textures(int mats[6], int dirs[6], ivec3 pos);
	virtual AxleInterface* cast_axle();
	virtual void add_item(ItemStack stack);
	virtual PipeInterface* cast_pipe();
	virtual void tick();
	virtual bool rcaction(Player* player, Item* item);
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


enum FluidType {
	Fluid_none = 0,
	Fluid_water,
	Fluid_num_fluids,
};

class AxleInterface { public:
	// AxleInterface(World* world, ivec3 starting_pos);
	// AxleInterface(World* world, istream& ifile);
	virtual void rotate(AxleInterface* sender, double amount, double force) = 0;
	virtual AxleInterface* cast_axle();
	string group_group();
	bool persistant();
};

class PipePacket { public:
	Item item;
	FluidType fluid;
	double amount;
};

class PipeInterface { public:
	virtual bool flow(PipeInterface* sender, PipePacket packet) = 0;
	virtual PipeInterface* cast_pipe();
};

class AxleGroup: public BlockGroup, public AxleInterface { public:
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
	bool rcaction(Player* player, Item* item);
	void link();
	void to_file(ostream& ofile);
private:
	bool calc_is_valid();
};

class GrindstoneGroup: public BlockGroup, public AxleInterface { public:
	double speed = 0;
	double force = 0;
	GrindstoneGroup(World* world, ivec3 spos);
	GrindstoneGroup(World* world, istream& ifile);
	bool rcaction(Player* player, Item* item);
	void rotate(AxleInterface* sender, double amount, double force);
	void custom_textures(int mats[6], int dirs[6], ivec3 pos);
	void to_file(ostream& ofile);
};

class MatchSidesGroup: public BlockGroup { public:
	MatchSidesGroup(World* world, ivec3 starting_pos);
	MatchSidesGroup(World* world, istream& ifile);
	void custom_textures(int mats[6], int dirs[6], ivec3 pos);
	bool persistant();
	void on_update();
};

// class WheelGroup: public AxleInterface { public:
// 	vector<AxleInterface*> ports;
// 	bool rcaction(Player* player, Item* item);

class ChestGroup: public BlockGroup { public:
	ItemContainer inven;
	ChestGroup(World* world, ivec3 spos);
	ChestGroup(World* world, istream& ifile);
	virtual bool rcaction(Player* player, Item* item);
	void add_item(ItemStack stack);
	void on_update();
	void update_inven_size();
	bool persistant();
	void to_file(ostream& ofile);
};

class BackpackGroup: public ChestGroup { public:
	BackpackGroup(World* world, ivec3 spos);
	BackpackGroup(World* world, istream& ifile);
	bool rcaction(Player* player, Item* item);
};

class CraftingGroup: public BlockGroup { public:
	CraftingGroup(World* world, ivec3 starting_pos);
	CraftingGroup(World* world, istream& ifile);
	bool rcaction(Player* player, Item* item);
	bool persistant();
};

class AnvilGroup: public BlockGroup { public:
	AnvilGroup(World* world, ivec3 starting_pos);
	AnvilGroup(World* world, istream& ifile);
	bool rcaction(Player* player, Item* item);
	bool persistant();
};

class DissolveGroup: public BlockGroup { public:
	DissolveGroup(World* world, ivec3 starting_pos);
	DissolveGroup(World* world, istream& ifile);
	virtual bool persistant();
	virtual void tick();
};


// class FluidLevel { public:
// 	unordered_set<ivec3> block_poses;
// 	int ylevel;
// 	int fill;
// 	FluidGroup* parent;
// 	FluidLevel(FluidGroup* newparent);
// 	void on_update();
// 	void flow();
// };

class FluidGroup: public BlockGroup, public PipeInterface { public:
	int viscosity = 5;
	int tickclock = 0;
	double fill_level;
	FluidGroup(World* nworld, ivec3 starting_pos);
	FluidGroup(World* nworld, istream& ifile);
	virtual void find_group();
	void add_block(ivec3 pos);
	void del_block(ivec3 pos);
	bool flow(PipeInterface* sender, PipePacket packet);
	bool flow_block(ivec3 pos);
	virtual bool persistant();
	virtual void tick();
};

#endif
