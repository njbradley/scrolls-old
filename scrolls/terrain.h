#ifndef BASE_TERRAIN_H
#define BASE_TERRAIN_H

#include "classes.h"
#include "plugins.h"


class TerrainLoader { public:
	BASE_PLUGIN_HEAD(TerrainLoader, (int seed));
	int seed;
	
	TerrainLoader(int newseed);
	virtual ~TerrainLoader() {}
	
	virtual int get_height(ivec2 pos) = 0;
	virtual Block* generate_chunk(ivec3 pos) = 0;
};


struct TerrainShape {
	virtual float gen_value(int seed, vec3 pos) = 0;
	virtual float max_deriv() = 0;
	virtual Blocktype block_val() = 0;
};


template <typename ... Shapes>
struct TerrainResolver {
	int seed;
	
	template <typename Shape>
	Blocktype gen_func(ivec3 pos, int scale);
	
	template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
	Blocktype gen_block(ostream& ofile, ivec3 globalpos, int scale);
	template <typename Shape>
	Blocktype gen_block(ostream& ofile, ivec3 globalpos, int scale);
	template <typename ... CurShapes>
	Blocktype gen_block(ostream& ofile, ivec3 globalpos, int scale, Blocktype mytype);
	
	Block* generate_chunk(ivec3 pos);
};







template <typename Terrain, typename Objects>
class TwoPassTerrainLoader : public TerrainLoader { public:
	PLUGIN_HEAD(TwoPassTerrainLoader<Terrain,Objects>);
	using TerrainLoader::TerrainLoader;
	
	virtual int get_height(ivec2 pos);
	virtual Block* generate_chunk(ivec3 pos);
};


#endif
