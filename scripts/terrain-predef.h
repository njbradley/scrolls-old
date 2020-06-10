#ifndef TERRAIN_PREDEF
#define TERRAIN_PREDEF

#include "classes.h"


class TerrainBase {
public:
	virtual int get_height(ivec2) = 0;
	virtual char gen_func(ivec3) = 0;
};

class TerrainObject {
public:
	ivec3 size;
	int radius;
	TerrainLoader* parent;
	TerrainObject(TerrainLoader* loader, ivec3 nsize, int nradius);
	virtual char gen_func(ivec3,ivec3) = 0;
	virtual ivec3 get_nearest(ivec3);
	virtual int priority() = 0;
	ivec3 get_nearest_3d(ivec3);
	ivec2 get_nearest_2d(ivec2);
};

class TerrainObjectMerger {
public:
	vector<TerrainObject*> objs;
	TerrainLoader* parent;
	TerrainObjectMerger(TerrainLoader* newparent);
	char gen_func(ivec3);
};

class TerrainLoader {
public:
	int seed;
	TerrainObjectMerger objmerger;
	TerrainBase* terrain;
	TerrainLoader(int nseed);
	char gen_func(ivec3);
};
	
	

class Land: public TerrainBase {
public:
	TerrainLoader* parent;
	Land(TerrainLoader*);
	int get_height(ivec2);
	char gen_func(ivec3);
};

class Tree: public TerrainObject {
public:
	Tree(TerrainObjectMerger*);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
};

class BigTree: public TerrainObject {
public:
	BigTree(TerrainObjectMerger*);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
};

class Cave: public TerrainObject {
public:
	Cave(TerrainObjectMerger*);
	char gen_func(ivec3,ivec3);
	ivec3 get_nearest(ivec3);
	int priority();
};

#endif
