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

class Block { public:
	Block* parent;
	
	union {
		Pixel* pixel;
		Block* children[csize3];
	};
	FreeBlock* freechild = nullptr;
	
	bool continues;
	bool top_level;
	uint16 flags = 0b00000000;
	uint8 locked = 0;
	int refcount = 0;
	int freecount = 0;
	
	enum BlockFlags : uint16 {
		RENDER_FLAG = 0b00000001,
		LIGHT_FLAG = 0b00000010,
		CHANGE_PROP_FLAG = 0b00000100,
		UPDATE_FLAG = 0b00010000,
		CHANGE_FLAG = 0b00100000,
		PROPAGATING_FLAGS = 0b00001111,
	};
	
	// Initializes to undef
	Block();
	// Initializes to chunk state with given values
	Block(Block* childs[8]);
	// Initializes pixel state with given new allocated
	// pixel
	Block(Pixel* pix);
	
	~Block();
	
	bool ischunk() const;
	bool ispixel() const;
	bool isundef() const;
	
	void lock();
	void unlock();
	
	void incref();
	void decref();
	
	void on_change();
	void update();
	
	// sets or resets the passed flag
	// flags are specified in the enum above,
	// some propagate up, like render, and light flag
	// other flags, like the change flag are specific to the block
	// and are not propagated up
	// when reseting, no values are propagated, as it cannot be determined
	// if the flag was set by other blocks
	void set_flag(uint8 flag);
	void reset_flag(uint8 flag);
	// sets the flags on children recursively
	void set_all_flags(uint8 flag);
	void reset_all_flags(uint8 flag);
	
	// gets the block for chunk blocks
	Block* get(ivec3 pos);
	Block* get(int x, int y, int z);
	Block* get(int index);
	const Block* get(ivec3 pos) const;
	const Block* get(int x, int y, int z) const;
	const Block* get(int index) const;
	
	void set_parent(Block* nparent);
	
	void set_child(ivec3 pos, Block* block);
	void set_child(int index, Block* block);
	Block* swap_child(ivec3 pos, Block* block);
	Block* swap_child(int index, Block* block);
	
	void add_freechild(BlockView& view, FreeBlock* freeblock);
	void remove_freechild(FreeBlock* freeblock);
	
	void set_pixel(Pixel* pix);
	Pixel* swap_pixel(Pixel* pix);
	
	// turns pixel type blocks to chunk type
	// divide leaves all children as null
	void divide();
	void subdivide();
	// turns chunk type into pixel type (deletes all children)
	void join();
	// joins if all children are all the same and have nothing
	// keeping them small (freeblocks, edges)
	void try_join();
	
	// returns the index into the children array
	// for a position
	static int indexof(ivec3 pos);
	static int indexof(int x, int y, int z);
	// returns the position of a specific index into
	// the children array
	ivec3 posof(int index) const;
	// read/writes the block to a file recursively
	void to_file(ostream& ofile) const;
	void from_file(istream& ifile);
};




template <typename BlockT>
class BlockViewTempl { public:
	using BlockType = BlockT;
	using PixelT = typename BlockTypes<BlockT>::PixelT;
	using ColliderT = typename BlockTypes<BlockT>::ColliderT;
	using FreeBlockT = typename BlockTypes<BlockT>::FreeBlockT;
	using ContainerT = typename BlockTypes<BlockT>::ContainerT;
	
	BlockT* parent = nullptr;
	BlockT* curblock = nullptr;
	FreeBlockT* freecontainer = nullptr;
	ContainerT* world = nullptr;
	ivec3 globalpos;
	int scale = 0;
	bool in_freecontainer = false;
	
	BlockViewTempl();
	explicit BlockViewTempl(ContainerT* world);
	// BlockViewTempl(BlockT* block);
	BlockViewTempl(const BlockViewTempl<BlockT>& other);
	BlockViewTempl(BlockViewTempl<BlockT>&& other);
	~BlockViewTempl();
	
	BlockViewTempl& operator=(BlockViewTempl<BlockT> other);
	BlockViewTempl& operator=(BlockViewTempl<BlockT>&& other);
	
	bool operator==(BlockViewTempl<BlockT>& other) const;
	
	void swap(BlockViewTempl<BlockT>& other);
	
	ivec3 parentpos() const;
	bool isvalid() const;
	bool isrelative() const;
	bool isfreeblock() const;
	
	void invalidate();
	
	Hitbox local_hitbox() const;
	Hitbox local_freebox() const;
	Hitbox hitbox() const;
	Hitbox freebox() const;
	Movingbox movingbox() const;
	
	const Block* operator->() const;
	
	static void incref(BlockT* block);
	static void decref(BlockT* block);
	
	bool step_down(ivec3 pos);
	bool step_down(int x, int y, int z);
	bool step_down_free();
	bool step_up();
	bool step_up_free();
	bool step_side(ivec3 pos);
	bool step_side(int x, int y, int z);
	bool step_side_free();
	
	BlockViewTempl<BlockT> get_global(ivec3 pos, int w);
	bool moveto(ivec3 pos, int w);
};

class BlockView : public BlockViewTempl<Block> { public:
	
	using BlockViewTempl<Block>::BlockViewTempl;
	BlockView(const BlockViewTempl<Block>& other);
	
	Block* operator->();
	
	BlockView get_global_exact(ivec3 pos, int w);
	bool moveto_exact(ivec3 pos, int w);
	
	void set(Block* block);
	void set(Blocktype val, int direc = -1);
	
	void set_curblock(Block* block);
	Block* swap_curblock(Block* block);
	
	// divides this block and leaves children how the block
	// was (if curblock is null, children will be null,
	// if curblock was a pixel, children will be the same pixel)
	void subdivide();
	// joins if all children are all the same and have nothing
	// keeping them small (freeblocks, edges)
	void try_join();
	
	BlockIterable<PixelIterator<BlockView>> iter();
	BlockIterable<DirPixelIterator<BlockView>> iter_side(ivec3 dir);
	BlockSideIterable<BlockView> iter_touching_side(ivec3 dir);
};

class ConstBlockView : public BlockViewTempl<const Block> { public:
	using BlockViewTempl<const Block>::BlockViewTempl;
	ConstBlockView(const BlockViewTempl<const Block>& other);
	ConstBlockView(const BlockView& other);
	
	BlockIterable<PixelIterator<ConstBlockView>> iter();
	BlockIterable<DirPixelIterator<ConstBlockView>> iter_side(ivec3 dir);
	BlockSideIterable<ConstBlockView> iter_touching_side(ivec3 dir);
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
// that are not fixed inside a tile. This is done with the allfreeblocks pointer.


class FreeBlock : public Block { public:
	Movingbox box;
	Hitbox lastbox;
	Hitbox renderbox;
	float update_time = 0;
	
	BlockView highparent;
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
	void set_parent(const BlockView& nparent);
	void expand(ivec3 dir);
	bool set_box(Movingbox newbox, float curtime = -1);
	virtual void tick(float curtime, float deltatime);
	virtual void timestep(float curtime, float deltatime);
	virtual Entity* entity_cast();
		
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
	
	Pixel(int val, int direction = -1, int njoints[6] = nullptr);
	Pixel(Block* newblock, istream& ifile);
	~Pixel();
	void set_block(Block* nblock);
	void rotate(int axis, int dir);
	void render(BlockView& view, RenderVecs* vecs, RenderVecs* transvecs);
	void render_position(const BlockView& view);
	void set(BlockView& view, Blocktype val, int direction = -1, int joints[6] = nullptr);
	void render_update();
	void erase_render();
	void calculate_sunlight(BlockView& view, unordered_set<ivec3,ivec3_hash>* next_poses = nullptr);
	void calculate_blocklight(BlockView& view, unordered_set<ivec3,ivec3_hash>* next_poses = nullptr);
	void reset_lightlevel();
	void lighting_update(BlockView& view, bool spread = true);
	BlockData* data() const;
	
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
