#include "terrainshapes.h"








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
Blocktype ShapeResolver<Shapes...>::gen_func(ivec3 globalpos, int scale, bool* split) {
	float val = Shape::gen_value(seed, vec3(globalpos) + float(scale)/2);
	float level_needed = float(scale-1)/2 * Shape::max_deriv() * 2.1f;
	if (std::abs(val) < level_needed) {
		*split = true;
	}
	if (val >= 0) {
		return Shape::block_val();
	} else {
		return BLOCK_NULL;
	}
}



template <typename ... Shapes>
Blocktype ShapeResolver<Shapes...>::gen_block(ivec3 globalpos, int scale, int* num_split) {
	*num_split = false;
	bool split = false;
	Blocktype val = gen_block<Shapes...>(globalpos, scale, &split);
	if (split) {
		(*num_split) ++;
		int newscale = scale/2;
		int i = 0;
		// return val;
		while (newscale >= 1 and i < 1) {
			split = false;
			for (int x = 0; x < scale; x += newscale) {
				for (int y = 0; y < scale; y += newscale) {
					for (int z = 0; z < scale; z += newscale) {
						Blocktype newval = gen_block<Shapes...>(globalpos, scale, &split);
						if (newval != val) {
							return val;
						}
					}
				}
			}
			(*num_split) ++;
			newscale /= 2;
			i ++;
			if (!split) {
				return val;
			}
		}
	}
	
	return val;
}



template <typename ... Shapes>
template <typename FirstShape, typename SecondShape, typename ... OtherShapes>
Blocktype ShapeResolver<Shapes...>::gen_block(ivec3 globalpos, int scale, bool* split) {
	Blocktype val = gen_func<FirstShape>(globalpos, scale, split);
	if (val == BLOCK_NULL) {
		return gen_block<SecondShape,OtherShapes...>(globalpos, scale, split);
	} else {
		return val;
	}
}

template <typename ... Shapes>
template <typename Shape>
Blocktype ShapeResolver<Shapes...>::gen_block(ivec3 globalpos, int scale, bool* split) {
	Blocktype val = gen_func<Shape>(globalpos, scale, split);
	if (val == BLOCK_NULL) {
		val = 0;
	}
	return val;
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


































typedef ShapeResolver<
	SolidType<FlatTerrain<32>, blocktypes::stone>
> FlatWorld;
EXPORT_PLUGIN_TEMPLATE(FlatWorld);

typedef ShapeResolver<
	SolidType<Shift<FractalPerlin2d<64,8,0>, 0,0,0>, blocktypes::stone>,
	SolidType<Shift<FractalPerlin2d<64,8,0>, 0,5,0>, blocktypes::dirt>,
	SolidType<Shift<FractalPerlin2d<64,8,0>, 0,6,0>, blocktypes::grass>
> Simple2D;
EXPORT_PLUGIN_TEMPLATE(Simple2D);

typedef ShapeResolver<
	SolidType<FractalPerlin3d<64,8,0>, blocktypes::stone>
> Simple3D;
EXPORT_PLUGIN_TEMPLATE(Simple3D);


// typedef HeightFalloff<RidgePerlin3d<64,0>, 1> LandLevel;
typedef HeightFalloff<Add<Perlin3d<64,0>, Perlin3d<9,1>, RidgePerlin3d<32,4>, Perlin2d<512,9>>, 1> LandLevel;


typedef ShapeResolver<
	SolidType<LandLevel, blocktypes::stone>,
	SolidType<Shift<AddVal<LandLevel, -3>, 0,2,0>, blocktypes::dirt>,
	SolidType<Shift<AddVal<LandLevel, -5>, 0,5,0>, blocktypes::grass>
> TestWorld;
EXPORT_PLUGIN_TEMPLATE(TestWorld);
