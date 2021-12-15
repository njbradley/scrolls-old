#include "terrain.h"
#include "world.h"
#include "blockdata.h"
#include "blocks.h"
#include "fileformat.h"



int hash4(int seed, int a, int b, int c, int d) {
	int sum = seed + 13680553*a + 47563643*b + 84148333*c + 80618477*d;
	sum = (sum ^ (sum >> 13)) * 750490907;
  return sum ^ (sum >> 16);
}

float randfloat(int seed, int a, int b, int c, int d) {
  return (float)(hash4(seed,a,b,c,d)%1000000) / 1000000;
}

vec3 randvec3(int seed, int a, int b, int c, int d) {
  return vec3(randfloat(seed, a, b, c, d*3), randfloat(seed, a, b, c, d*3+1), randfloat(seed, a, b, c, d*3+2));
}

vec2 randvec2(int seed, int a, int b, int c, int d) {
  return vec2(randfloat(seed, a, b, c, d*2), randfloat(seed, a, b, c, d*2+1));
}

double lerp(double a0, double a1, double w) {
    return (1.0f - w)*a0 + w*a1;
}

float interpolate(float a0, float a1, float w) {
  //return (a1 - a0) * w + a0;
  // Use this cubic interpolation [[Smoothstep]] instead, for a smooth appearance:
  return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
  
  // Use [[Smootherstep]] for an even smoother result with a second derivative equal to zero on boundaries:
  //return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}



float gridval3d(ivec3 ipos, vec3 pos, int seed, int layer) {
  vec3 pvec = glm::normalize(randvec3(seed, ipos.x, ipos.y, ipos.z, layer));
  return glm::dot(pos - vec3(ipos), pvec);
}

float gridval2d(ivec2 ipos, vec2 pos, int seed, int layer) {
  vec2 pvec = glm::normalize(randvec2(seed, ipos.x, ipos.y, 0, layer));
  return glm::dot(pos - vec2(ipos), pvec);
}


float perlin3d(vec3 pos, int seed, int layer) {
  ivec3 ipos = SAFEFLOOR3(pos);
  vec3 localpos = pos - vec3(ipos);
  
  float val11 = interpolate(gridval3d(ipos+ivec3(0,0,0), pos, seed, layer), gridval3d(ipos+ivec3(1,0,0), pos, seed, layer), localpos.x);
  float val12 = interpolate(gridval3d(ipos+ivec3(0,1,0), pos, seed, layer), gridval3d(ipos+ivec3(1,1,0), pos, seed, layer), localpos.x);
  float val1 = interpolate(val11, val12, localpos.y);
  
  float val21 = interpolate(gridval3d(ipos+ivec3(0,0,1), pos, seed, layer), gridval3d(ipos+ivec3(1,0,1), pos, seed, layer), localpos.x);
  float val22 = interpolate(gridval3d(ipos+ivec3(0,1,1), pos, seed, layer), gridval3d(ipos+ivec3(1,1,1), pos, seed, layer), localpos.x);
  float val2 = interpolate(val21, val22, localpos.y);
  
  return interpolate(val1, val2, localpos.z);
}
  
float perlin2d(vec2 pos, int seed, int layer) {
  ivec2 ipos = ivec2(SAFEFLOOR(pos.x), SAFEFLOOR(pos.y));
  vec2 localpos = pos - vec2(ipos);
  
  float val1 = interpolate(gridval2d(ipos, pos, seed, layer), gridval2d(ipos+ivec2(1,0), pos, seed, layer), localpos.x);
  float val2 = interpolate(gridval2d(ipos+ivec2(0,1), pos, seed, layer), gridval2d(ipos+ivec2(1,1), pos, seed, layer), localpos.x);
  return interpolate(val1, val2, localpos.y);
}

float fractal_perlin2d(vec2 pos, float scale, float divider, int seed, int layer) {
  float val = 0;
  int i = 0;
  while (scale > 1) {
    val += perlin2d(pos / scale, seed, layer*100 + i) * scale;
    scale /= divider;
    i ++;
  }
  return val;
}

float fractal_perlin3d(vec3 pos, float scale, float divider, int seed, int layer) {
  float val = 0;
  int i = 0;
  while (scale > 1) {
    val += perlin3d(pos / scale, seed, layer*100 + i) * scale;
    scale /= divider;
    i ++;
  }
  return val;
}

const Blocktype BLOCK_NULL = -1;
const Blocktype BLOCK_SPLIT = -2;





TerrainLoader::TerrainLoader(int nseed): seed(nseed), terrain(seed) {
	
}

void TerrainLoader::set_seed(int newseed) {
	seed = newseed;
	terrain->seed = seed;
}

int TerrainLoader::get_height(ivec2 pos) {
	return terrain->get_height(pos);
}

Block* TerrainLoader::generate_chunk(ivec3 pos) {
	Block* block = terrain->generate_chunk(pos);
	return block;
}



DEFINE_PLUGIN(TerrainGenerator);

TerrainGenerator::TerrainGenerator(int newseed): seed(newseed) {
	
}




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
	gen_block<Shapes...>(ss, pos * World::chunksize, World::chunksize);
	return new Block(ss);
}

template <typename ... Shapes>
int ShapeResolver<Shapes...>::get_height(ivec2 pos2d) {
	ivec3 pos (pos2d.x, 0, pos2d.y);
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
	SolidType<Shift<AddVal<LandLevel, -2>, 0,2,0>, blocktypes::grass>
> TestWorld;
EXPORT_PLUGIN_TEMPLATE(TestWorld);
