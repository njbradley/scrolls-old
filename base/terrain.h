#ifndef BASE_TERRAIN_H
#define BASE_TERRAIN_H

#include "classes.h"
#include "plugins.h"

class TerrainLoader { public:
	PLUGIN_HEAD(TerrainLoader, (int seed));
	
	int seed;
	
	TerrainLoader(int seed);
	virtual ~TerrainLoader() {}
	
	virtual Block* generate_chunk(ivec3 pos);
	Blocktype gen_block(ostream& ofile, int gx, int gy, int gz, int scale);
	Blocktype gen_func(ivec3 pos);
};


#endif
