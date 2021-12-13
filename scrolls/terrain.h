#ifndef BASE_TERRAIN_H
#define BASE_TERRAIN_H

#include "classes.h"
#include "plugins.h"


struct TerrainShape {
	virtual float gen_value(ivec3 pos) = 0;
	virtual float max_deriv() = 0;
	virtual Blocktype block_val() = 0;
};





class TerrainLoader { public:
	BASE_PLUGIN_HEAD(TerrainLoader, (int seed));
	int seed;
	
	TerrainLoader(int newseed);
	virtual ~TerrainLoader() {}
	
	virtual int get_height(ivec2 pos) = 0;
	virtual Block* generate_chunk(ivec3 pos) = 0;
};

class SimpleTerrainLoader : public TerrainLoader { public:
	using TerrainLoader::TerrainLoader;
	
	virtual int get_height(ivec2 pos) = 0;
	virtual int gen_func(ivec3 pos) = 0;
	
	virtual Block* generate_chunk(ivec3 pos);
	Blocktype gen_block(ostream& ofile, int gx, int gy, int gz, int scale);
};

class FlatTerrainLoader : public SimpleTerrainLoader { public:
	PLUGIN_HEAD(FlatTerrainLoader);
	using SimpleTerrainLoader::SimpleTerrainLoader;
	
	virtual int get_height(ivec2 pos);
	virtual int gen_func(ivec3 pos);
};

class PerlinTerrainLoader : public SimpleTerrainLoader { public:
	PLUGIN_HEAD(PerlinTerrainLoader);
	using SimpleTerrainLoader::SimpleTerrainLoader;
	
	virtual int get_height(ivec2 pos);
	virtual int gen_func(ivec3 pos);
};

#endif
