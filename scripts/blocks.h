#ifndef BLOCKS_PREDEF
#define BLOCKS_PREDEF

#include "classes.h"

#include <cmath>
#include <thread>
#include <atomic>

#include "collider.h"
#include "rendervec.h"

#define csize 2

extern const uint8 lightmax;


// The main class of the octree. Has three states:
//
// undefined, this is after a block has been initialized from
//   the empty constructor. Most functions will not work in this
//   state, though some, like from_file, require it. In this state,
//   the block has no value or children. continues is false, but
//   pixel is nullptr.
//
// pixel type: This is where the block is an end of the tree, and it
//   has a value. continues is false, and pixel points to a pixel.
//
// chunk type: This is when the block is a branch block, whith 8 children
//   continues is true. Chunks can have different winding types, to better
//   suit smaller and different shaped objects. the types are:
//     winding = 0 - normal, 2 by 2 by 2.
//     winding = 0b10XYZXYZ - flat face, 1 by 4 by 2, the first XYZ marks which axis is
//                            4 long, the second marks the 2 long axis.
//     winding = 0b01000XYZ - long line, 1 by 1 by 8, the XYZ marks the 8 long axis

class Block: public Collider { public:
	ivec3 parentpos;
	ivec3 globalpos;
	bool render_flag = true;
	bool light_flag = true;
	int scale;
	Block* parent = nullptr;
	Container* world;
	bool continues;
	uint8 winding;
	union {
		Pixel* pixel;
		Block* children;
	};
	
	// Initializes to uninitialized state,
	// continues == false, pixel == nullptr
	Block();
	// Initializes pixel state with given new allocated
	// pixel
	Block(Pixel* pix);
	// Initializez to chunk state with given new[8] allocated
	// children array
	Block(Block* childs, uint8 nwinding = 0);
	// initializes from a file (must be top level)
	Block(istream& ifile);
	~Block();
	
	// sets the parent of this block (only call for toplevel blocks)
	void set_parent(Block* parent, Container* world, ivec3 ppos, int nscale);
	void set_child(ivec3 pos, Block* block);
	void set_pixel(Pixel* pix);
	// gets the block for chunk blocks
	// (no error checking for speed)
	Block* get(ivec3 pos);
	Block* get(int x, int y, int z);
	int indexof(ivec3 pos) const;
	int indexof(int x, int y, int z) const;
	ivec3 posof(int index) const;
	const Block* get(ivec3 pos) const;
	const Block* get(int x, int y, int z) const;
	// travels the tree to find the requested size and pos.
	// calls to world if it reaches the top
	Block* get_global(int x, int y, int z, int w);
	Block* get_global(ivec3 pos, int w);
	void set_global(ivec3 pos, int w, int val, int direc = -1, int joints[6] = nullptr);
	// turns undef or pixel type blocks to chunk type
	// divide leaves all children as undef type (use subdivide for
	// fully initialized children)
	void divide(uint8 nwinding = 0);
	void subdivide();
	void join();
	void changewinding(uint8 nwinding);
	// read/writes the block to a file recursively
	// reading should be done to undef type
	void to_file(ostream& ofile) const;
	void from_file(istream& ifile);
	// sets the render/light flags all up to top node
	void set_render_flag();
	void set_light_flag();
	void set_all_render_flags();
	
	void render(RenderVecs* vecs, RenderVecs* transvecs, uint8 faces, bool render_null);
	void lighting_update();
	
	BlockIter iter(ivec3 startpos = ivec3(0,0,0), ivec3 endpos = ivec3(csize-1,csize-1,csize-1), ivec3 (*iterfunc)(ivec3 pos, ivec3 start, ivec3 end) = nullptr);
	BlockIter iter_side(ivec3 dir);
	BlockIter iter_touching_side(ivec3 dir);
	BlockTouchIter iter_touching();
	ConstBlockIter const_iter(ivec3 startpos = ivec3(0,0,0), ivec3 endpos = ivec3(csize-1,csize-1,csize-1), ivec3 (*iterfunc)(ivec3 pos, ivec3 start, ivec3 end) = nullptr) const;
	ConstBlockIter const_iter_side(ivec3 dir) const;
	bool is_air(int, int, int, char otherval = -1) const;
	int get_sunlight(int dx, int dy, int dz) const;
	int get_blocklight(int dx, int dy, int dz) const;
	Block* raycast(vec3* pos, vec3 dir, double time);
	vec3 get_position() const;
	
	static void write_pix_val(ostream& ofile, char type, unsigned int val);
	static void read_pix_val(istream& ifile, char* type, unsigned int* val);
};

class Pixel { public:
	Block* parbl;
	int value;
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
	void render_face(MemVecs* vecs, GLfloat x, GLfloat y, GLfloat z, Block* blocks[6], int index, ivec3 dir, int uv_dir, int minscale, int mat, bool render_null);
	void rotate(int axis, int dir);
	void render(RenderVecs* vecs, RenderVecs* transvecs, uint8 faces, bool render_null);
	void set(int val, int direction = -1, int joints[6] = nullptr);
	void render_update();
	void tick();
	void random_tick();
	void erase_render();
	void calculate_sunlight(unordered_set<ivec3,ivec3_hash>& next_poses);
	void calculate_blocklight(unordered_set<ivec3,ivec3_hash>& next_poses);
	void reset_lightlevel();
	void lighting_update();
	bool is_air(int,int,int);
	BlockGroupIter iter_group();
	BlockGroupIter iter_group(bool (*func)(Pixel* base, Pixel* pix));
	BlockGroupIter iter_group(Collider* world, bool (*func)(Pixel* base, Pixel* pix) = nullptr);
	void to_file(ostream& ofile);
	
  static void rotate_uv(GLfloat* uvs, int rot);
  // takes in a glfloat array 12 long and
  // rotates the face rottimes clockwise
  static void rotate_from_origin(int* mats, int* dirs, int rotation);
  // takes in two arrays 6 long, the texture and rotation of
  // every face, and rotates everything to direction rotation
  // from origin
  static void rotate_to_origin(int* mats, int* dirs, int rotation);
};


class BlockContainer: public Container { public:
	Block* block;
	bool allocated;
	
	BlockContainer(Block* b);
	BlockContainer(int scale);
	~BlockContainer();
	vec3 get_position() const;
	Block* get_global(int x, int y, int z, int size);
  void set(ivec4 pos, char val, int direction, int joints[6] = nullptr);
	void block_update(ivec3 pos){}
};

class BlockIter { public:
  Block* base;
  ivec3 start_pos;
  ivec3 end_pos;
  ivec3 (*increment_func)(ivec3 pos, ivec3 startpos, ivec3 endpos);
  class iterator { public:
    BlockIter* parent;
    Pixel* pix;
    Pixel* operator*();
    iterator operator++();
    friend bool operator!=(const iterator& iter1, const iterator& iter2);
  };
  iterator begin();
  iterator end();
};

class BlockTouchIter { public:
  Block* base;
  Collider* world;
  BlockIter blockiter;
  class iterator { public:
    BlockTouchIter* parent;
    int dir_index;
    BlockIter::iterator iter;
    Pixel* operator*();
    iterator operator++();
    friend bool operator != (const iterator& iter1, const iterator& iter2);
  };
  iterator begin();
  iterator end();
	static constexpr ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
};

class BlockGroupIter { public:
  Pixel* base;
  Collider* world;
  bool (*includefunc)(Pixel* base, Pixel* pix);
  unordered_set<Pixel*> pixels;
  BlockGroupIter(Pixel* newbase, Collider* nworld, bool (*func)(Pixel* base, Pixel* pix));
  unordered_set<Pixel*>::iterator begin();
  unordered_set<Pixel*>::iterator end();
  void swap(unordered_set<Pixel*>& other);
	static constexpr ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
};
    

class ConstBlockIter { public:
  const Block* base;
  ivec3 start_pos;
  ivec3 end_pos;
  ivec3 (*increment_func)(ivec3 pos, ivec3 startpos, ivec3 endpos);
  class iterator { public:
    ConstBlockIter* parent;
    const Pixel* pix;
    const Pixel* operator*();
    iterator operator++();
    friend bool operator!=(const iterator& iter1, const iterator& iter2);
  };
  iterator begin();
  iterator end();
};
    
    
    


#endif
