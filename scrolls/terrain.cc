#include "terrain.h"
#include "world.h"
#include "blockdata.h"
#include "blocks.h"
#include "fileformat.h"

DEFINE_PLUGIN(TerrainLoader);


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
  
  float val11 = interpolate(gridval3d(ipos, pos, seed, layer), gridval2d(ipos+ivec3(1,0,0), pos, seed, layer), localpos.x);
  float val12 = interpolate(gridval3d(ipos+ivec3(0,1,0), pos, seed, layer), gridval2d(ipos+ivec3(1,1,0), pos, seed, layer), localpos.x);
  float val1 = interpolate(val11, val12, localpos.y);
  
  float val21 = interpolate(gridval3d(ipos+ivec3(0,0,1), pos, seed, layer), gridval2d(ipos+ivec3(1,0,1), pos, seed, layer), localpos.x);
  float val22 = interpolate(gridval3d(ipos+ivec3(0,1,1), pos, seed, layer), gridval2d(ipos+ivec3(1,1,1), pos, seed, layer), localpos.x);
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

template <int max_scale, int divider, int seed, int layer>
struct Perlin2d {
	static float gen_value(ivec3 pos) {
		float val = 0;
	  int i = 0;
		int scale = max_scale;
	  while (scale > 1) {
	    val += perlin2d(pos / scale, seed, layer*100 + i) * scale;
	    scale /= divider;
	    i ++;
	  }
	  return val;
	}
	
	static float max_deriv() {
		return std::log(max_scale) / std::log(divider);
	}
};







TerrainLoader::TerrainLoader(int nseed): seed(nseed) {
	
}





Block* SimpleTerrainLoader::generate_chunk(ivec3 pos) {
	pos *= World::chunksize;
	stringstream ss;
	gen_block(ss, pos.x, pos.y, pos.z, World::chunksize);
	return new Block(ss);
}

Blocktype SimpleTerrainLoader::gen_block(ostream& ofile, int gx, int gy, int gz, int scale) {
  // Blocktype val = gen_func(ivec4(gx, gy, gz, scale));
  // cout << int(val) << ' ' << ivec3(gx,gy,gz) << ' ' << scale << endl;
  if (scale == 1) {// or val != -1) {
		Blocktype val = gen_func(ivec3(gx, gy, gz));
    FileFormat::write_variable(ofile, val<<2);
    return val;
  } else {
    stringstream ss;
    ss << char(0b11000000);
    Blocktype val = gen_block(ss, gx, gy, gz, scale/csize);
    bool all_same = val != -1;
    for (int x = 0; x < csize; x ++) {
      for (int y = 0; y < csize; y ++) {
        for (int z = 0; z < csize; z ++) {
          if (x > 0 or y > 0 or z > 0) {
            Blocktype newval = gen_block(ss, gx+x*(scale/csize), gy+y*(scale/csize), gz+z*(scale/csize), scale/csize);
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



EXPORT_PLUGIN(FlatTerrainLoader);

int FlatTerrainLoader::get_height(ivec2 pos) {
  return 31;
}

int FlatTerrainLoader::gen_func(ivec3 pos) {
  if (pos.y < 31) {
    return blocktypes::stone.id;
  }
  return 0;
}


EXPORT_PLUGIN(PerlinTerrainLoader);

int PerlinTerrainLoader::get_height(ivec2 pos) {
	// return SAFEFLOOR(perlin2d(vec2(pos)/64.0f, seed, 0) * 64);
	float height = fractal_perlin2d(vec2(pos), 64, 8, seed, 0);
	return SAFEFLOOR(height);
}

int PerlinTerrainLoader::gen_func(ivec3 pos) {
  if (pos.y < get_height(ivec2(pos.x, pos.z))) {
    return blocktypes::stone.id;
  }
  return 0;
}
