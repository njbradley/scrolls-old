#ifndef TERRAIN_PREDEF
#define TERRAIN_PREDEF

#include "classes.h"

const int chunksize = 256;

class TerrainObject {
public:
	int seed;
	ChunkLoader* parent;
	TerrainObject(ChunkLoader* chunk, int nseed): seed(nseed), parent(chunk) { }
	virtual char gen_func(int x, int y, int z) = 0;
	virtual int priority() = 0;
};

class TerrainBase: public TerrainObject {
public:
	TerrainBase(ChunkLoader* chunk, int nseed): TerrainObject(chunk, nseed) { }
	virtual int get_height(int,int) = 0;
};

class Island: public TerrainBase {
public:
	int width;
	int height;
	int xpos;
	int ypos;
	int height_array[chunksize][chunksize];
	int base_array[chunksize][chunksize];
	Island(ChunkLoader* chunk, int nseed, int x, int y, int nwidth, int nheight);
	pair<int,int> generate_bounds(int,int);
	int get_height(int,int);
	void generate_arrays();
	char gen_func(int x, int y, int z);
	int priority();
};

class Trees: public TerrainObject {
public:
	string type;
	int num_trees = 16;
	vector<pair<int,int> > positions;
	int (*tree_data)[20][20][20];
	Trees(ChunkLoader* chunk, int nseed, string ntype);
	void generate_tree(int);
	char gen_func(int x, int y, int z);
	int priority();
};

class Path: public TerrainObject {
public:
	vector<pair<int,int> > path_points;
	int height;
	Path(ChunkLoader* chunk, int nseed);
	char gen_func(int x, int y, int z);
	int priority();
};

class ChunkLoader {
public:
	vector<TerrainObject*> objs;
	vector<TerrainBase*> terrain;
	int xpos;
	int ypos;
	int seed;
	ChunkLoader(int, int, int);
	char gen_func(int x, int y, int z);
	int get_height(int, int);
};

#endif
