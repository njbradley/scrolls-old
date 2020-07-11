#ifndef BLOCKS_PREDEF
#define BLOCKS_PREDEF

#include "classes.h"
#include <iostream>
#include <functional>
#include <cmath>
#include <fstream>
#include <sstream>
#include <thread>

#include <GL/glew.h>
#include <map>
#include "world-predef.h"
#include "collider-predef.h"

#define csize 2


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
    void set_render_flag();
    virtual bool continues() const = 0; // whether the block is a chunk or a pixel. a hacky way of telling, but nessicary because pixels can be at different depths
    virtual char get() const = 0;                 //get the value of the block. these two methods are exclusive for pixel/chunk, but same reaseon as above
    //virtual void set(char val, int direction = -1, BlockExtra* extras = nullptr) = 0;
    virtual void del(bool remove_faces) = 0;
    virtual void calculate_lightlevel() = 0;
    virtual Pixel* get_pix() = 0;
    // returns a pixel pointer if the object is a pixel,
    // otherwise returns nullptr
    virtual float get_lightlevel(int,int,int) = 0;
    virtual void all(function<void(Pixel*)>) = 0;
    virtual void all_side(function<void(Pixel*)>,int,int,int) = 0;
    Block(int x, int y, int z, int newscale, Chunk* newparent);
    void world_to_local(int x, int y, int z, int* lx, int* ly, int* lz) const;
    void global_position(int*, int*, int*) const;
    virtual void render(RenderVecs*, Collider*, int, int, int) = 0;
    virtual void rotate(int axis, int direction) = 0;
    virtual bool is_air(int, int, int) const = 0;
    virtual Block * get_global(int,int,int,int) = 0;
    virtual void render_update() = 0;
    virtual void save_to_file(ostream&) = 0;
    Block* raycast(Collider* world, double* x, double* y, double* z, double dx, double dy, double dz, double time);
    Block* get_world();
    vec3 get_position() const;
    
    static Block* from_file(istream& ifile, int px, int py, int pz, int scale, Chunk* parent, Tile* tile);
};

class Pixel: public Block {
  void rotate_uv(GLfloat* uvs, int rot);
  // takes in a glfloat array 12 long and
  // rotates the face rottimes clockwise
  void rotate_from_origin(int* mats, int* dirs, int rotation);
  // takes in two arrays 6 long, the texture and rotation of
  // every face, and rotates everything to direction rotation
  // from origin
  void rotate_to_origin(int* mats, int* dirs, int rotation);
  // same as above but rotates from a location to the origin
public:
    char value;
    char direction;
    pair<int,int> render_index;
    float lightlevel = 1;
    BlockExtra* extras = nullptr;
    Tile* tile;
    BlockGroup* physicsgroup;
    //int num;
    //ItemContainer* container;
    
    Pixel(int x, int y, int z, char new_val, int nscale, Chunk* nparent, Tile* tile);
    Pixel(int x, int y, int z, char new_val, int nscale, Chunk* nparent, Tile* tile, BlockExtra* new_extra);
    void generate_extras();
    bool continues() const;
    char get() const;
    void all(function<void(Pixel*)> func);
    void all_side(function<void(Pixel*)>,int,int,int);
    // runs the function only on blocks touching the
    // side specified by the three ints, dx, dy, dz.
    Pixel* get_pix();
    float get_lightlevel(int dx, int dy, int dz);
    void set(char val, int direction = -1, BlockExtra* extras = nullptr, bool update = true);
    // sets the value, direction, and BlockExtra of the pixel
    void render_update();
    void tick();
    void random_tick();
    void del(bool remove_faces);
    void calculate_lightlevel();
    void reset_lightlevel();
    void rotate(int axis, int dir);
    void lighting_update();
    bool is_air(int dx, int dy, int dz) const;
    Block* get_global(int x, int y, int z, int scale);
    void render(RenderVecs*, Collider*, int, int,int);
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
    Chunk* resolve(function<char(ivec3)> gen_func);
    // defines the chunk based on the function, like subdivide
    // but doesn't force one division
    // if the pixel is divided a chunk will be returned and
    // the parent will be altered. if the pixel is not divided,
    // a nullptr is returned
    void save_to_file(ostream& of);
};

class Chunk: public Block { public:
    Block * blocks[csize][csize][csize];
    
    Chunk(int x, int y, int z, int nscale, Chunk* nparent);
    bool continues() const;
    Block * get(int x, int y, int z) const;
    Pixel* get_pix();
    char get() const;
    //void set(char val, int direction, BlockExtra* extras);
    void calculate_lightlevel();
    void all(function<void(Pixel*)> func);
    void all_side(function<void(Pixel*)>,int,int,int);
    float get_lightlevel(int dx, int dy, int dz);
    void render(RenderVecs* vecs, Collider*, int gx, int gy, int gz);
    void del(bool remove_faces);
    void render_update();
    void rotate(int axis, int dir);
    Block* get_global(int x, int y, int z, int nscale);
    bool is_air(int dx, int dy, int dz) const;
    void save_to_file(ostream& of);
};


class BlockContainer: public Collider { public:
	Block* block;
	
	BlockContainer(Block* b);
	vec3 get_position() const;
	Block* get_global(int x, int y, int z, int size);
};

#endif
