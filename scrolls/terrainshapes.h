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
	Blocktype gen_func(ivec3 pos, int scale);
	
	template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
	Blocktype gen_block(ostream& ofile, ivec3 globalpos, int scale);
	template <typename Shape>
	Blocktype gen_block(ostream& ofile, ivec3 globalpos, int scale);
	template <typename ... CurShapes>
	Blocktype gen_block(ostream& ofile, ivec3 globalpos, int scale, Blocktype mytype);
	
	virtual Block* generate_chunk(ivec3 pos);
	virtual int get_height(ivec3 pos);
};








template <typename ... Shapes>
float ShapeResolver<Shapes...>::get_max_value(vec3 pos) {
	float vals[sizeof...(Shapes)] = {Shapes::gen_value(seed, pos)...};
	return *std::max_element(vals, vals+sizeof...(Shapes));
}

template <typename ... Shapes>
float ShapeResolver<Shapes...>::get_max_deriv() {
	float vals[sizeof...(Shapes)] = {Shapes::max_deriv()...};
	return *std::max_element(vals, vals+sizeof...(Shapes));
}

template <typename ... Shapes>
template <typename Shape>
Blocktype ShapeResolver<Shapes...>::gen_func(ivec3 globalpos, int scale) {
	float val = Shape::gen_value(seed, vec3(globalpos) + float(scale)/2);
	float level_needed = float(scale-1)/2 * Shape::max_deriv() * 2.1f;
	if (val >= level_needed) {
		return Shape::block_val();
	} else if (val <= -level_needed) {
		return BLOCK_NULL;
	} else {
		return BLOCK_SPLIT;
	}
}


template <typename ... Shapes>
template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
Blocktype ShapeResolver<Shapes...>::gen_block(ostream& ofile, ivec3 globalpos, int scale) {
	Blocktype val = gen_func<FirstShape>(globalpos, scale);
	if (val == BLOCK_NULL) {
		return gen_block<SecondShape,OtherShapes...>(ofile, globalpos, scale);
	} else {
		return gen_block<FirstShape,SecondShape,OtherShapes...>(ofile, globalpos, scale, val);
	}
}

template <typename ... Shapes>
template <typename Shape>
Blocktype ShapeResolver<Shapes...>::gen_block(ostream& ofile, ivec3 globalpos, int scale) {
	Blocktype val = gen_func<Shape>(globalpos, scale);
	if (val == BLOCK_NULL) {
		val = 0;
	}
	return gen_block<Shape>(ofile, globalpos, scale, val);
}



template <typename ... Shapes>
template <typename ... CurShapes>
Blocktype ShapeResolver<Shapes...>::gen_block(ostream& ofile, ivec3 globalpos, int scale, Blocktype myval) {
  if (myval != BLOCK_SPLIT or scale == 1) {
		if (myval == BLOCK_NULL) myval = 0;
    FileFormat::write_variable(ofile, myval<<2);
    return myval;
  } else {
    stringstream ss;
    ss << blockformat::chunk;
    Blocktype val = gen_block<CurShapes...>(ss, globalpos, scale/csize);
    bool all_same = val != -1;
    for (int x = 0; x < csize; x ++) {
      for (int y = 0; y < csize; y ++) {
        for (int z = 0; z < csize; z ++) {
          if (x > 0 or y > 0 or z > 0) {
            Blocktype newval = gen_block<CurShapes...>(ss, globalpos + ivec3(x,y,z)*(scale/csize), scale/csize);
            all_same = all_same and newval == val;
          }
        }
      }
    }
    if (all_same) {
      FileFormat::write_variable(ofile, val<<2);
      return val;
    } else {
      ofile << ss.rdbuf();
      return -1;
    }
  }
}



template <typename ... Shapes>
Block* ShapeResolver<Shapes...>::generate_chunk(ivec3 pos) {
	stringstream ss;
	gen_block<Shapes...>(ss, pos * 64, 64);
	return new Block(ss);
}

template <typename ... Shapes>
int ShapeResolver<Shapes...>::get_height(ivec3 pos) {
	float val;
	while (std::abs(val = get_max_value(vec3(pos) + 0.5f)) > 2) {
		if (val > 0) {
			pos.y += std::max(1.0f, val / get_max_deriv() / 2);
		} else {
			pos.y += std::min(-1.0f, val / get_max_deriv() / 2);
		}
	}
	return pos.y;
}










template <typename Shape, BlockData& data>
struct SolidType : public Shape {
	static Blocktype block_val() {
		return data.id;
	}
};

template <typename Shape, int x, int y, int z>
struct Shift : public Shape {
	static float gen_value(int seed, vec3 pos) {
		return Shape::gen_value(seed, pos - vec3(x,y,z));
	}
};

template <typename Shape, int x, int y, int z>
struct Scale : public Shape {
	static float gen_value(int seed, vec3 pos) {
		return Shape::gen_value(seed, pos * vec3(x,y,z));
	}
	
	static float max_deriv() {
		return Shape::max_deriv() * std::max(x, std::max(y, z));
	}
};

template <typename ... Shapes>
struct Add {
	static float gen_value(int seed, vec3 pos) {
		return (Shapes::gen_value(seed, pos) + ...);
	}
	
	static float max_deriv() {
		return (Shapes::max_deriv() + ...);
	}
};

template <typename Shape1, int num, int denom = 1>
struct AddVal : public Shape1 {
	static constexpr float val = float(num) / denom;
	static float gen_value(int seed, vec3 pos) {
		return Shape1::gen_value(seed, pos) + val;
	}
};

template <typename Shape1, int num, int denom = 1>
struct MulVal : public Shape1 {
	static constexpr float val = float(num) / denom;
	static float gen_value(int seed, vec3 pos) {
		return Shape1::gen_value(seed, pos) * val;
	}
	
	static float max_deriv() {
		return Shape1::max_deriv() * std::abs(val);
	}
};






template <typename Shape, int num_falloff, int denom_falloff = 1>
struct HeightFalloff : public Shape {
	static float gen_value(int seed, vec3 pos) {
		return Shape::gen_value(seed, pos) - pos.y * num_falloff / denom_falloff;
	}
	
	static float max_deriv() {
		return Shape::max_deriv() + num_falloff / denom_falloff;
	}
};

template <int scale, int layer>
struct Perlin2d {
	static float gen_value(int seed, vec3 pos) {
		return perlin2d(vec2(pos.x, pos.z) / float(scale), seed, layer) * scale;
	}
	
	static float max_deriv() {
		return 1.5f;
	}
};

template <int max_scale, int divider, int layer>
struct FractalPerlin2d {
	static float gen_value(int seed, vec3 pos) {
		float val = 0;
	  int i = 0;
		int scale = max_scale;
	  while (scale > 1) {
	    val += perlin2d(vec2(pos.x, pos.z) / float(scale), seed, layer*100 + i) * scale;
	    scale /= divider;
	    i ++;
	  }
		val -= pos.y * max_deriv();
	  return val;
	}
	
	static float max_deriv() {
		return std::max(1.0, std::log(max_scale) / std::log(divider));
	}
};

template <int scale, int layer>
struct Perlin3d {
	static float gen_value(int seed, vec3 pos) {
		return perlin3d(pos / float(scale) + randvec3(seed, layer, 143, 211, 566), seed, layer) * scale * 2;
	}
	
	static float max_deriv() {
		return 1.5f;
	}
};

template <int max_scale, int divider, int layer>
struct FractalPerlin3d {
	static float gen_value(int seed, vec3 pos) {
		// return perlin3d(pos / float(max_scale), seed, layer) * max_scale;
		float val = 0;
	  int i = 0;
		int scale = max_scale;
	  while (scale > 1) {
	    val += perlin3d(pos / float(scale), seed, layer*100 + i) * scale;
	    scale /= divider;
	    i ++;
	  }
	  return val;
	}
	
	static float max_deriv() {
		return std::log(max_scale) / std::log(divider);
	}
};


template <int scale, int layer>
struct RidgePerlin3d : Perlin3d<scale, layer> {
	static float gen_value(int seed, vec3 pos) {
		return scale - std::abs(Perlin3d<scale, layer>::gen_value(seed, pos));
	}
};








template <int height>
struct FlatTerrain {
	static float gen_value(int seed, vec3 pos) {
		return height - pos.y;
	}
	
	static float max_deriv() {
		return 1;
	}
};

template <int height, int slope_numer, int slope_denom = 1>
struct SlopedTerrain {
	static float gen_value(int seed, vec3 pos) {
		return height + pos.x * slope_numer / slope_denom - pos.y;
	}
	
	static float max_deriv() {
		return std::max(std::abs((float)slope_numer/slope_denom), 1.0f);
	}
};






#endif
