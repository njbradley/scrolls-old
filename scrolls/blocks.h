#ifndef BLOCKS_PREDEF
#define BLOCKS_PREDEF

#include "classes.h"

#include <cmath>
#include <thread>
#include <atomic>

#include "collider.h"
#include "rendervecs.h"
#include "entity.h"

extern const uint8 lightmax;

#define RENDER_FLAG 0b1
#define LIGHT_FLAG 0b10

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
	uint8 flags = 0b00000000;
	int scale;
	Block* parent = nullptr;
	Container* world;
	FreeBlock* freecontainer = nullptr;
	bool continues;
	bool locked = false;
	FreeBlock* freechild = nullptr;
	union {
		Pixel* pixel;
		Block* children[csize3];
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
	
	Block(Block&& other);
	
	~Block();
	
	// locks and unlocks block (used for editing)
	// prevents setting render flags from going up,
	// and from render and lighting updates from going down
	// Important: not enough for true thread safety, only
	// minimal protection
	void lock();
	void unlock();
	// sets the parent of this block (only call for toplevel blocks)
	void set_parent(Container* world, ivec3 ppos, int nscale);
	void set_parent(Block* parent, Container* world, FreeBlock* freecont, ivec3 ppos, int nscale);
	// sets a child at a location
	// set_child deletes the old child, swap_child returns it
	void set_child(ivec3 pos, Block* block);
	Block* swap_child(ivec3 pos, Block* block);
	
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
	const Block* get(ivec3 pos) const;
	const Block* get(int x, int y, int z) const;
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
	Hitbox local_hitbox();
	Hitbox local_freebox();
	Hitbox hitbox();
	Hitbox freebox();
	// travels the tree to find the requested size and pos.
	// calls to world if it reaches the top
	Block* get_global(int x, int y, int z, int w);
	Block* get_global(ivec3 pos, int w);
	Block* get_global(vec3 pos, int w, vec3 dir); // WIP
	// travels the tree, but does not travel between freeblocks/baseblocks
	Block* get_local(ivec3 pos, int w);
	Block* get_local(vec3 pos, int w, vec3 dir); // WIP
	Block* get_touching(ivec3 pos, int w, ivec3 dir); //bad
	// travels the tree, and sets the block specified. does not travel between
	// freeblocks/baseblocks
	void set_global(ivec3 pos, int w, Blocktype val, int direc = -1, int joints[6] = nullptr);
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
	// sets the render/light flags all up to top node
	void set_flag(uint8 flag);
	// void set_render_flag();
	// void set_light_flag();
	void set_all_flags(uint8 flag);
	
	void tick();
	void timestep(float deltatime);
	
	void render(RenderVecs* vecs, RenderVecs* transvecs, uint8 faces, bool render_null);
	void lighting_update(bool spread = true);
	
	BlockIter iter(ivec3 startpos = ivec3(0,0,0), ivec3 endpos = ivec3(csize-1,csize-1,csize-1), ivec3 (*iterfunc)(ivec3 pos, ivec3 start, ivec3 end) = nullptr);
	BlockIter iter_side(ivec3 dir);
	BlockTouchSideIter iter_touching_side(ivec3 dir);
	BlockTouchIter iter_touching();
	ConstBlockIter const_iter(ivec3 startpos = ivec3(0,0,0), ivec3 endpos = ivec3(csize-1,csize-1,csize-1), ivec3 (*iterfunc)(ivec3 pos, ivec3 start, ivec3 end) = nullptr) const;
	ConstBlockIter const_iter_side(ivec3 dir) const;
	bool is_air(ivec3 dir, char otherval = -1);
	bool is_air(int,int,int, char otherval = -1) const;
	int get_sunlight(ivec3 dir);
	int get_blocklight(ivec3 dir);
	Block* raycast(vec3* pos, vec3 dir, double time);
	vec3 get_position() const;
	bool collide(Hitbox newbox, Hitbox* boxhit, float deltatime, FreeBlock* ignore);
	bool collide(Hitbox newbox, Block* block, Hitbox* boxhit, float deltatime, FreeBlock* ignore);
	bool collide_free(Hitbox newbox, Block* block, Hitbox* boxhit, float deltatime, FreeBlock* ignore);
	
	vec3 force(vec3 amount);
	
	static void write_pix_val(ostream& ofile, char type, unsigned int val);
	static void read_pix_val(istream& ifile, char* type, unsigned int* val);
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
// Freeblocks can't be inside freeblocks (no nesting), so instead the freeblock attr on freeblocks
// is used to make a linked list of all freeblocks in the normal parent block.
//
// [Parent block] -> [freeblock] -> [other freeblock] -> nullptr
//   (scale 4)					(scale 2)				(scale 1)
// these two freeblocks are both in parentblock, and have separate rotations/positions
//
// In addition, to speed up collisions, a second linked list is maintained through all freeblocks
// inside a tile. This is done with the allfreeblocks pointer.


class FreeBlock : public Block { public:
	Hitbox box;
	bool fixed = false;
	Block* highparent = nullptr;
	FreeBlock* allfreeblocks = nullptr;
	
	vec3 velocity;
	vec3 angularvel;
	
	FreeBlock(Hitbox newbox);
	void set_parent(Block* nparent);
	void set_parent(Block* nparent, Container* nworld, ivec3 ppos, int nscale);
	void expand(ivec3 dir);
	void set_box(Hitbox newbox);
	void tick();
	void timestep(float deltatime);
	vec3 force(vec3 pos, vec3 dir);
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
	RenderVecs* lastvecs;
	BlockGroup* group = nullptr;
	
	Pixel(int val, int direction = -1, int njoints[6] = nullptr);
	Pixel(Block* newblock, istream& ifile);
	~Pixel();
	void set_block(Block* nblock);
	void rotate(int axis, int dir);
	void render(RenderVecs* vecs, RenderVecs* transvecs, uint8 faces, bool render_null);
	void set(Blocktype val, int direction = -1, int joints[6] = nullptr);
	void render_update();
	void tick();
	void random_tick();
	void erase_render();
	void calculate_sunlight(unordered_set<ivec3,ivec3_hash>* next_poses = nullptr);
	void calculate_blocklight(unordered_set<ivec3,ivec3_hash>* next_poses = nullptr);
	void reset_lightlevel();
	void lighting_update(bool spread = true);
	bool is_air(int,int,int);
	BlockGroupIter iter_group();
	BlockGroupIter iter_group(bool (*func)(Pixel* base, Pixel* pix));
	BlockGroupIter iter_group(Collider* world, bool (*func)(Pixel* base, Pixel* pix) = nullptr);
	void to_file(ostream& ofile);
	
  // takes in a glfloat array 12 long and
  // rotates the face rottimes clockwise
  static void rotate_from_origin(int* mats, int* dirs, int rotation);
  // takes in two arrays 6 long, the texture and rotation of
  // every face, and rotates everything to direction rotation
  // from origin
  static void rotate_to_origin(int* mats, int* dirs, int rotation);
};

    
    
    


#endif
