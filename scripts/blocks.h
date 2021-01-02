#ifndef BLOCKS_PREDEF
#define BLOCKS_PREDEF

#include "classes.h"

#include <cmath>
#include <thread>

#include "collider.h"
#include "rendervec.h"

#define csize 2




extern const float lightmax;

/*
 *this is the file with classes for storing blocks. the main idea is blocks are stored in power of two containers. Block is the base virtual class,
 *and Pixel and Chunk derive from Block.
 *
 */

class Block: public Collider { public:
    
    int px; // parent coordinates, bool because only need to go up to 1
    int py;
    int pz;
    int scale; //scale of the block, number of units on one side length.
    Chunk * parent; //the parent of this block. if this block is the root block, then the parent should be set to nullptr.
    bool render_flag = true;
    bool light_flag = true;
    std::shared_mutex setlock;
    
    class write_lock { public:
      Block* block;
      write_lock(Block* newblock);
      ~write_lock();
    };
    class read_lock { public:
      Block* block;
      read_lock(Block* newblock);
      ~read_lock();
    };
    
    void set_render_flag();
    void set_light_flag();
    virtual bool continues() const = 0; // whether the block is a chunk or a pixel. a hacky way of telling, but nessicary because pixels can be at different depths
    virtual char get() const = 0;                 //get the value of the block. these two methods are exclusive for pixel/chunk, but same reaseon as above
    //virtual void set(char val, int direction = -1, BlockExtra* extras = nullptr) = 0;
    virtual void del(bool remove_faces) = 0;
    virtual Chunk* get_chunk() = 0;
    virtual Pixel* get_pix() = 0;
    virtual const Chunk* get_chunk() const = 0;
    virtual const Pixel* get_pix() const = 0;
    // returns a pixel pointer if the object is a pixel,
    // otherwise returns nullptr
    int get_sunlight(int dx, int dy, int dz) const;
    int get_blocklight(int dx, int dy, int dz) const;
    Block(int x, int y, int z, int newscale, Chunk* newparent);
    virtual ~Block();
    void world_to_local(int x, int y, int z, int* lx, int* ly, int* lz) const;
    void global_position(int*, int*, int*) const;
    virtual void render(RenderVecs* vecs, RenderVecs* transvecs, Collider* world, int x, int y, int z, int depth, bool faces[6], bool render_null, bool yield) = 0;
    virtual void rotate(int axis, int direction) = 0;
    bool is_air(int, int, int, char otherval = -1) const;
    virtual Block * get_global(int,int,int,int) = 0;
    virtual Block* set_global(ivec4 pos, char val, int direction = -1) = 0;
    virtual void set_all_render_flags() = 0;
    virtual void save_to_file(ostream&, bool yield = false) = 0;
    virtual void lighting_update() = 0;
    Block* raycast(Collider* world, double* x, double* y, double* z, double dx, double dy, double dz, double time);
    Block* get_world();
    BlockIter iter(ivec3 startpos = ivec3(0,0,0), ivec3 endpos = ivec3(csize-1,csize-1,csize-1), ivec3 (*iterfunc)(ivec3 pos, ivec3 start, ivec3 end) = nullptr);
    BlockIter iter_side(ivec3 dir);
    ConstBlockIter const_iter(ivec3 startpos = ivec3(0,0,0), ivec3 endpos = ivec3(csize-1,csize-1,csize-1), ivec3 (*iterfunc)(ivec3 pos, ivec3 start, ivec3 end) = nullptr) const;
    ConstBlockIter const_iter_side(ivec3 dir) const;
    vec3 get_position() const;
    
    static Block* from_file(istream& ifile, int px, int py, int pz, int scale, Chunk* parent, Tile* tile);
    static void write_pix_val(ostream& ofile, char type, unsigned int val);
    static void read_pix_val(istream& ifile, char* type, unsigned int* val);
};

class Pixel: public Block {
  static void rotate_uv(GLfloat* uvs, int rot);
  // takes in a glfloat array 12 long and
  // rotates the face rottimes clockwise
  static void rotate_from_origin(int* mats, int* dirs, int rotation);
  // takes in two arrays 6 long, the texture and rotation of
  // every face, and rotates everything to direction rotation
  // from origin
  static void rotate_to_origin(int* mats, int* dirs, int rotation);
  // same as above but rotates from a location to the origin
public:
    char value;
    char direction;
    RenderIndex render_index;
    bool render_transparent = false;
    int blocklight;
    int sunlight;
    int entitylight;
    int lightsource = -1;
    Tile* tile;
    BlockGroup* group = nullptr;
    //int num;
    //ItemContainer* container;
    
    Pixel(int x, int y, int z, char new_val, int nscale, Chunk* nparent, Tile* tile);
    virtual bool continues() const;
    virtual char get() const;
    // runs the function only on blocks touching the
    // side specified by the three ints, dx, dy, dz.
    virtual Pixel* get_pix();
    virtual Chunk* get_chunk();
    virtual const Pixel* get_pix() const;
    virtual const Chunk* get_chunk() const;
    void set(char val, int direction = -1, BlockExtra* extras = nullptr, bool update = true);
    // sets the value, direction, and BlockExtra of the pixel
    void render_update();
    virtual void set_all_render_flags();
    void tick();
    void random_tick();
    virtual void del(bool remove_faces);
    void calculate_sunlight(unordered_set<ivec3,ivec3_hash>& next_poses);
    void calculate_blocklight(unordered_set<ivec3,ivec3_hash>& next_poses);
    void reset_lightlevel();
    virtual void rotate(int axis, int dir);
    virtual void lighting_update();
    virtual Block* get_global(int x, int y, int z, int scale);
    virtual Block* set_global(ivec4 pos, char val, int direction = -1);
    void render_face(MemVecs* vecs, GLfloat x, GLfloat y, GLfloat z, Block* block, ivec3 dir, int uv_dir, int minscale, int mat, bool render_null);
    virtual void render(RenderVecs* vecs, RenderVecs* transvecs, Collider* world, int x, int y, int z, int depth, bool faces[6], bool render_null, bool yield);
    void render_smooth(RenderVecs* vecs, RenderVecs* transvecs, Collider* collider, vec3 view_normal);
    Chunk* subdivide();
    // splits the pixel into a chunk with 8 pixels of the
    // same value in it
    Chunk* subdivide(function<char(ivec3)> gen_func);
    // subdivides the pixel based on a function that returns
    // a char for each global position
    // the new chunk is returned and if possible it is placed
    // into the parent. if this pixel has no parent it is up
    // to the function caller to properly replace the pixel
    // with the chunk
    Chunk* resolve(bool yield = false);
    // defines the chunk based on the function, like subdivide
    // but doesn't force one division
    // if the pixel is divided a chunk will be returned and
    // the parent will be altered. if the pixel is not divided,
    // a nullptr is returned
    virtual void save_to_file(ostream& of, bool yield = false);
};

class Chunk: public Block { public:
    Block * blocks[csize][csize][csize];
    
    Chunk(int x, int y, int z, int nscale, Chunk* nparent);
    virtual bool continues() const;
    Block * get(int x, int y, int z) const;
    virtual Pixel* get_pix();
    virtual Chunk* get_chunk();
    virtual const Pixel* get_pix() const;
    virtual const Chunk* get_chunk() const;
    virtual char get() const;
    virtual void set_all_render_flags();
    virtual void lighting_update();
    virtual void render(RenderVecs* vecs, RenderVecs* transvecs, Collider* world, int x, int y, int z, int depth, bool faces[6], bool render_null, bool yield);
    virtual void del(bool remove_faces);
    virtual void rotate(int axis, int dir);
    virtual Block* get_global(int x, int y, int z, int nscale);
    virtual Block* set_global(ivec4 pos, char val, int direction = -1);
    virtual void save_to_file(ostream& of, bool yield = false);
};


class BlockContainer: public Collider { public:
	Block* block;
	
	BlockContainer(Block* b);
	vec3 get_position() const;
	Block* get_global(int x, int y, int z, int size);
  void set(ivec4 pos, char val, int direction);
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
