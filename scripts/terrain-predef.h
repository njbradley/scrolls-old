#ifndef TERRAIN_PREDEF
#define TERRAIN_PREDEF

#include "classes.h"

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
	Island(ChunkLoader* chunk, int nseed, int x, int y, int nwidth, int nheight): TerrainBase(chunk, nseed), xpos(x), ypos(y), width(nwidth), height(nheight) { }
	pair<int,int> get_bounds(int,int);
	int get_height(int,int);
	char gen_func(int x, int y, int z);
	int priority();
};

class Trees: public TerrainObject {
public:
	string type;
	vector<pair<int,int> > trees;
	Trees(ChunkLoader* chunk, int nseed, string ntype);
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
