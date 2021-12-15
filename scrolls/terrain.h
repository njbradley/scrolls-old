#ifndef BASE_TERRAIN_H
#define BASE_TERRAIN_H

#include "classes.h"
#include "plugins.h"


class TerrainLoader { public:
	int seed;
	PluginNow<TerrainGenerator> terrain;
	
	TerrainLoader(int newseed);
	
	void set_seed(int newseed);
	
	int get_height(ivec2 pos);
	Block* generate_chunk(ivec3 pos);
};

class TerrainGenerator { public:
	BASE_PLUGIN_HEAD(TerrainGenerator, (int seed));
	int seed;
	
	TerrainGenerator(int seed);
	virtual ~TerrainGenerator() {}
	
	virtual Block* generate_chunk(ivec3 pos) = 0;
	virtual int get_height(ivec2 pos) = 0;
};


struct TerrainShape {
	virtual float gen_value(int seed, vec3 pos) = 0;
	virtual float max_deriv() = 0;
	virtual Blocktype block_val() = 0;
};


template <typename ... Shapes>
struct ShapeResolver : public TerrainGenerator {
	PLUGIN_HEAD(ShapeResolver);
	using TerrainGenerator::TerrainGenerator;
	
	float get_max_value(vec3 pos);
	float get_max_deriv();
	
	template <typename Shape>
	Blocktype gen_func(ivec3 pos, int scale);
	
	template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
	Blocktype gen_block(ostream& ofile, ivec3 globalpos, int scale);
	template <typename Shape>
	Blocktype gen_block(ostream& ofile, ivec3 globalpos, int scale);
	template <typename ... CurShapes>
	Blocktype gen_block(ostream& ofile, ivec3 globalpos, int scale, Blocktype mytype);
	
	virtual Block* generate_chunk(ivec3 pos);
	virtual int get_height(ivec2 pos);
};



#endif
