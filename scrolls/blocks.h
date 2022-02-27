#ifndef BLOCKS_PREDEF
#define BLOCKS_PREDEF

#include "classes.h"

#include <cmath>
#include <thread>
#include <atomic>

#include "collider.h"
#include "rendervecs.h"
#include "physics.h"

extern const uint8 lightmax;

// Blocks are the main structural class of the world, they create a three dimentional tree, where each node
// has 8 (or more, change csize) children. The scale of a block is the side length, so each child
// is half the scale of their parent.
//
// The tree structure means that large blocks of the same type (like air) can be consolodated to a larger
// block and save on memory usage
//
// There are two states that a block can be in:
//
// Pixel type (continues = false): a leaf node in the tree, meaning it is a block. has a pointer to a
// pixel object, and the children array is not used
//
// Chunk type (continues = true): an internal node that has 8 children. Has to have a scale larger than
// 1 and the pixel pointer isn't used

class Block: public Collider { public:
	ivec3 parentpos;
	ivec3 globalpos; // should be called localpos, but i really dont want to change everything
	int scale = 0;
	Block* parent = nullptr;
	Container* world;
	FreeBlock* freecontainer = nullptr;
	bool continues;
	uint8 locked = 0;
	FreeBlock* freechild = nullptr;
	int flags = 0;
	int freecount = 0;
	int min_scale = 0;
	Blocktype prevvalue = 0;
	union {
		Pixel* pixel;
		Block* children[csize3];
	};
	
	enum BlockFlags : int {
		RENDER_FLAG = 0x0001,
		LIGHT_FLAG = 0x0002,
		CHANGE_PROP_FLAG = 0x0004,
		
		GENERATION_FLAG = 0x0010,
		
		UPDATE_FLAG = 0x0100,
		CHANGE_FLAG = 0x0200,
		
		PROPAGATING_FLAGS = 0x00FF,
		GLOBAL_FLAGS = 0x000F,
	};
	
	// Initializes to undef
	Block();
	// Initializes to chunk state with given values
	Block(Block* childs[8]);
	// Initializes pixel state with given new allocated
	// pixel
	Block(Pixel* pix);
	// initializes from a file
	Block(istream& ifile);
	
	~Block();
	
	// locks and unlocks block (used for editing)
	// prevents setting render flags from going up,
	// and from render and lighting updates from going down
	// Important: not enough for true thread safety, only
	// minimal protection
	void lock();
	void unlock();
	
	void swap(Block* other);
	
	void update();
	void update_min_scale();
	void on_change();
	// sets the parent of this block (only call for toplevel blocks)
	void set_parent(Container* world, ivec3 gpos, int nscale);
	void set_parent(Block* parent, Container* world, FreeBlock* freecont, ivec3 ppos, int nscale);
	void set_parent(Block* parent, Container* world, FreeBlock* freecont, ivec3 ppos, ivec3 gpos, int nscale);
	// sets a child at a location
	// set_child deletes the old child, swap_child returns it
	void set_child(ivec3 pos, Block* block);
	void set_child(int index, Block* block);
	Block* swap_child(ivec3 pos, Block* block);
	Block* swap_child(int index, Block* block);
	
	void add_freechild(FreeBlock* freeblock);
	void remove_freechild(FreeBlock* freeblock);
	// sets the pixel of this block
	// set_pixel deletes the current pixel, swap_pixel
	// returns it
	void set_pixel(Pixel* pix);
	Pixel* swap_pixel(Pixel* pix);
	// gets the block for chunk blocks
	// (no error checking for speed)
	Block* get(ivec3 pos);
	Block* get(int x, int y, int z);
	Block* get(int index);
	const Block* get(ivec3 pos) const;
	const Block* get(int x, int y, int z) const;
	const Block* get(int index) const;
	// returns the index into the children array
	// for a position
	int indexof(ivec3 pos) const;
	int indexof(int x, int y, int z) const;
	// returns the position of a specific index into
	// the children array
	ivec3 posof(int index) const;
	// returns the floating point global pos
	vec3 fglobalpos() const;
	quat getrotation() const;
	// local
	Hitbox local_hitbox() const;
	Hitbox local_freebox() const;
	Hitbox hitbox() const;
	Hitbox freebox() const;
	Movingbox movingbox() const;
	
	int chunk_scale() const;
	// travels the tree to find the requested size and pos.
	// calls to world if it reaches the top
	Block* get_global(ivec3 pos, int w);
	const Block* get_global(ivec3 pos, int w) const;
	// travels the tree, and sets the block specified. does not travel between
	// freeblocks/baseblocks
	void set_global(ivec3 pos, int w, Blocktype val, int direc = -1, int joints[6] = nullptr);
	void set_global(ivec3 pos, int w, Block* newblock);
	// turns pixel type blocks to chunk type
	// divide leaves all children as null (use subdivide for
	// fully initialized children)
	void divide();
	void subdivide();
	// turns chunk type into pixel type (deletes all children)
	void join();
	// joins if all children are all the same and have nothing
	// keeping them small (freeblocks, edges)
	void try_join();
	// read/writes the block to a file recursively
	// reading should be done to undef type
	void to_file(ostream& ofile) const;
	void from_file(istream& ifile);
	// sets or resets the passed flag
	// flags are specified in the enum above,
	// some propagate up, like render, and light flag
	// other flags, like the change flag are specific to the block
	// and are not propagated up
	// when reseting, no values are propagated, as it cannot be determined
	// if the flag was set by other blocks
	void set_flag(int flag);
	void reset_flag(int flag);
	// sets the flags all up to top node
	void set_all_flags(int flag);
	void reset_all_flags(int flag);
	
	void tick(float curtime, float deltatime);
	void timestep(float curtime, float deltatime);
	
	void render(RenderVecs* vecs, RenderVecs* transvecs);
	void render_position();
	void lighting_update(bool spread = true);
	
	BlockIterable<PixelIterator<Block>> iter();
	BlockIterable<DirPixelIterator<Block>> iter_side(ivec3 dir);
	BlockSideIterable<Block> iter_touching_side(ivec3 dir);
	BlockSideIterable<const Block> iter_touching_side(ivec3 dir) const;
	// BlockTouchSideIter iter_touching_side(ivec3 dir);
	BlockIterable<PixelIterator<const Block>> const_iter() const;
	BlockIterable<DirPixelIterator<const Block>> const_iter_side(ivec3 dir) const;
	bool is_air(ivec3 dir, char otherval = -1) const;
	bool is_air(int,int,int, char otherval = -1) const;
	int get_sunlight(ivec3 dir) const;
	int get_blocklight(ivec3 dir) const;
	Block* raycast(vec3* pos, vec3 dir, double time);
	vec3 get_position() const;
	
	void to_log(ostream& out) const;
	
	friend ostream& operator<<(ostream& out, const Block& block);
};

// FreeBlocks are blocks not grid alligned
// They behave similar to normal blocks, they have a parent that is double their
// scale, and the parent and child are linked with pointers.
//
// A freeblock is a child
// when it falls in the bounding cube twice the scale of the parent block.
// This means that these bounding boxes (called freeboxes in the code) overlap,
// but this is so that a freeblock is never straddling an edge, there will always be
// a parent that fully contains any freeblock
//
// 																					         V-- this overlap of freeboxes between blocks next to each other
// (------)															(-------(---------)-------)     means that a child freeblock will always be fully
// ( |==| )	<-- freebox (extends 				(   |===(====|====)===|   )    contained in at least one parent block (cause even
// ( |==| )			scale/2 from all faces)	(   |   (    |    )   |   )    if a cube is rotated to create the largest bounding
// (--^---)															(   |   (    |    )   |   )    box, it is still less than the parents size, which
//    |																	(   |===(====|====)===|   )    is how big the overlap is)
// normal block													(-------(---------)-------)
//
// Freeblocks are stored in a block with the freeblock attribute, which points to a child.
// Multiple freeblocks that are in one parent are stored in a linked list, using the next pointer in freeblocks
//
// [Parent block] ->freechild [freeblock] ->next [other freeblock] ->next nullptr
//   (scale 4)									(scale 2)						(scale 2)
// these two freeblocks are both in parentblock, and have separate rotations/positions
//
// In addition, to speed up collisions, a second linked list is maintained through all freeblocks
// inside a tile. This is done with the allfreeblocks pointer.


class FreeBlock : public Block { public:
	Movingbox box;
	Hitbox lastbox;
	Hitbox renderbox;
	float update_time = 0;
	
	Block* highparent = nullptr;
	FreeBlock* next = nullptr;
	FreeBlock* allfreeblocks = nullptr;
	PhysicsBody* physicsbody = nullptr;
	
	bool fixed = false;
	bool paused = false;
	bool asleep = false;
	bool allow_sleep = true;
	bool allow_rotation = true;
	bool allow_raycast = true;
	
	FreeBlock();
	FreeBlock(Movingbox newbox);
	FreeBlock(istream& ifile);
	~FreeBlock();
	virtual void to_file(ostream& ofile) const;
	virtual void from_file(istream& ifile);
	void set_parent(Block* nparent);
	void set_parent(Block* nparent, Container* nworld, ivec3 ppos, int nscale);
	void expand(ivec3 dir);
	bool set_box(Movingbox newbox, float curtime = -1);
	virtual void tick(float curtime, float deltatime);
	virtual void timestep(float curtime, float deltatime);
	virtual Entity* entity_cast();
	
	void calculate_mass();
	
	void try_sleep();
	void force_sleep();
	void wake();
	
	void fix();
	void unfix();
};

class Pixel { public:
	Block* parbl;
	Blocktype value;
	uint8 direction;
	uint8 blocklight;
	uint8 sunlight;
	uint8 entitylight;
	uint8 lightsource = -1;
	uint8 joints[6] = {0,0,0,0,0,0};
	RenderIndex render_index = RenderIndex::npos;
	RenderVecs* lastvecs = nullptr;
	BlockGroup* group = nullptr;
	PhysicsBox* physicsbox = nullptr;
	int divides_left = 0;
	
	Pixel(int val, int direction = -1, int njoints[6] = nullptr);
	Pixel(Block* newblock, istream& ifile);
	~Pixel();
	void set_block(Block* nblock);
	void rotate(int axis, int dir);
	void render(RenderVecs* vecs, RenderVecs* transvecs);
	void render_position();
	void set(Blocktype val, int direction = -1, int joints[6] = nullptr);
	void render_update();
	void tick(float curtime, float deltatime);
	void random_tick();
	void erase_render();
	void calculate_sunlight(unordered_set<ivec3,ivec3_hash>* next_poses = nullptr);
	void calculate_blocklight(unordered_set<ivec3,ivec3_hash>* next_poses = nullptr);
	void reset_lightlevel();
	void lighting_update(bool spread = true);
	bool is_air(int,int,int);
	BlockData* data() const;
	// BlockGroupIter iter_group();
	// BlockGroupIter iter_group(bool (*func)(Pixel* base, Pixel* pix));
	// BlockGroupIter iter_group(Collider* world, bool (*func)(Pixel* base, Pixel* pix) = nullptr);
	void to_file(ostream& ofile);
	
  // takes in a glfloat array 12 long and
  // rotates the face rottimes clockwise
  static void rotate_from_origin(int* mats, int* dirs, int rotation);
  // takes in two arrays 6 long, the texture and rotation of
  // every face, and rotates everything to direction rotation
  // from origin
  static void rotate_to_origin(int* mats, int* dirs, int rotation);
};


namespace blockformat {
	const unsigned char chunk = 0b11000000;
	const unsigned char null = 0b11111111;
	const unsigned char free = 0b11110000;
	const unsigned char entity = 0b11100000;
	
	const unsigned char valuetype = 0b00;
	const unsigned char dirtype = 0b01;
	const unsigned char jointstype = 0b10;
}


#endif
