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
    virtual bool continues() = 0; // whether the block is a chunk or a pixel. a hacky way of telling, but nessicary because pixels can be at different depths
    virtual Block * get(int, int, int) = 0; //get a block at coords
    virtual char get() = 0;                 //get the value of the block. these two methods are exclusive for pixel/chunk, but same reaseon as above
    virtual void set(char) = 0;
    virtual Chunk* subdivide() = 0;
    virtual void del(bool remove_faces) = 0;
    virtual void calculate_lightlevel() = 0;
    virtual Pixel* get_pix()  = 0;
    virtual float get_lightlevel(int,int,int) = 0;
    virtual void all(function<void(Pixel*)>) = 0;
    virtual void all_side(function<void(Pixel*)>,int,int,int) = 0;
    Block(int x, int y, int z, int newscale, Chunk* newparent);
    void world_to_local(int x, int y, int z, int* lx, int* ly, int* lz);
    void global_position(int*, int*, int*);
    virtual void render(RenderVecs*, int, int, int) = 0;
    virtual bool is_air(int, int, int) = 0;
    virtual Block * get_global(int,int,int,int) = 0;
    virtual void render_update() = 0;
    virtual void save_to_file(ofstream&) = 0;
    Block* raycast(Collider* world, double* x, double* y, double* z, double dx, double dy, double dz, double time);
    Block* get_world();
    vec3 get_position();
    void update_chunk();
    
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
    BlockExtras* extras = nullptr;
    Tile* tile;
    //ItemContainer* container;
    
    Pixel(int x, int y, int z, char new_val, int nscale, Chunk* nparent, Tile* tile);
    Pixel(int x, int y, int z, char new_val, int nscale, Chunk* nparent, Tile* tile, BlockExtras* new_extra);
    void generate_extras();
    bool continues();
    char get();
    void all(function<void(Pixel*)> func);
    void all_side(function<void(Pixel*)>,int,int,int);
    // runs the function only on blocks touching the
    // side specified by the three ints, dx, dy, dz.
    Pixel* get_pix();
    float get_lightlevel(int dx, int dy, int dz);
    Block * get(int x, int y, int z);
    void set(char val);
    void render_update();
    void tick();
    void del(bool remove_faces);
    void calculate_lightlevel();
    void reset_lightlevel();
    void lighting_update();
    bool is_air(int dx, int dy, int dz);
    Block* get_global(int x, int y, int z, int scale);
    void render(RenderVecs*, int, int,int);
    Chunk* subdivide();
    Chunk* resolve();
    void save_to_file(ofstream& of);
};

class Chunk: public Block { public:
    Block * blocks[csize][csize][csize];
    
    Chunk(int x, int y, int z, int nscale, Chunk* nparent);
    bool continues();
    Block * get(int x, int y, int z);
    Pixel* get_pix();
    char get();
    void set(char c);
    Chunk* subdivide();
    void calculate_lightlevel();
    void all(function<void(Pixel*)> func);
    void all_side(function<void(Pixel*)>,int,int,int);
    float get_lightlevel(int dx, int dy, int dz);
    void render(RenderVecs* vecs, int gx, int gy, int gz);
    void del(bool remove_faces);
    void render_update();
    Block* get_global(int x, int y, int z, int nscale);
    bool is_air(int dx, int dy, int dz);
    void save_to_file(ofstream& of);
};


#endif
