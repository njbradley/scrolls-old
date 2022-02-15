#ifndef SCROLLS_TSHAPES
#define SCROLLS_TSHAPES

#include "classes.h"
#include "terrain.h"

#include "blockdata.h"
#include "blocks.h"
#include "fileformat.h"
#include "world.h"




// These are the methods that terrain shapes must implement
//
// struct TerrainShape {
//	returns a float value, below zero represents inside
// 	the shape, above zero represents outside
// 	static float gen_value(int seed, vec3 pos);
//
//	max_deriv is the max amount that the gen_value function can change over
// 	one unit in the x, y, or z directions. The idea is that the shape resolver
//	can limit the number of calls, because it can estimate how far away the surface
//	of the shape is and skip some calls to the gen_value func
// 	static float max_deriv();
//
//	The block type of the shape
// 	static Blocktype block_val();
// };


template <typename ... Shapes>
struct ShapeResolver : public TerrainGenerator {
	PLUGIN_HEAD(ShapeResolver);
	using TerrainGenerator::TerrainGenerator;
	
	float get_max_value(vec3 pos);
	float get_max_deriv();
	
	template <typename Shape>
	Blocktype gen_func(ivec3 pos, int scale, bool* split);
	
	virtual Blocktype gen_block(ivec3 globalpos, int scale, bool* split);
	
	template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
	Blocktype gen_block(ivec3 globalpos, int scale, bool* split);
	template <typename Shape>
	Blocktype gen_block(ivec3 globalpos, int scale, bool* split);
	
	virtual int get_height(ivec3 pos);
};













#endif
