#ifndef BASE_TERRAIN_H
#define BASE_TERRAIN_H

#include "classes.h"
#include "plugins.h"

const Blocktype BLOCK_NULL = -1;
const Blocktype BLOCK_SPLIT = -2;

int hash4(int seed, int a, int b, int c, int d);
int hash4(int seed, ivec3 pos, int d);
int hash4(int seed, ivec2 pos, int c, int d);

float randfloat(int seed, int a, int b, int c, int d);

vec3 randvec3(int seed, int a, int b, int c, int d);
vec2 randvec2(int seed, int a, int b, int c, int d);

float perlin3d(vec3 pos, int seed, int layer);
float perlin2d(vec2 pos, int seed, int layer);

float fractal_perlin2d(vec2 pos, float scale, float divider, int seed, int layer);
float fractal_perlin3d(vec3 pos, float scale, float divider, int seed, int layer);


class TerrainLoader { public:
	int seed;
	PluginNow<TerrainGenerator> terrain;
	PluginListNow<TerrainObject> objects;
	
	TerrainLoader(int newseed);
	
	void set_seed(int newseed);
	
	int get_height(ivec2 pos);
	int get_height(ivec3 pos);
	Block* generate_chunk(ivec3 pos);
};

class TerrainGenerator { public:
	BASE_PLUGIN_HEAD(TerrainGenerator, (int seed));
	int seed;
	
	TerrainGenerator(int seed);
	virtual ~TerrainGenerator() {}
	
	virtual Block* generate_chunk(ivec3 pos) = 0;
	virtual int get_height(ivec3 pos) = 0;
};

class TerrainObject { public:
	BASE_PLUGIN_HEAD(TerrainObject, (int seed));
	int seed;
	
	TerrainObject(int seed);
	virtual ~TerrainObject() {}
	
	virtual void place_object(ivec3 pos, Block* block) = 0;
};


#endif
