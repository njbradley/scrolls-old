#include "terrain.h"
#include "world.h"
#include "blockdata.h"
#include "blocks.h"
#include "fileformat.h"

DEFINE_PLUGIN(TerrainLoader);
EXPORT_PLUGIN(TerrainLoader);



double lerp(double a0, double a1, double w) {
    return (1.0f - w)*a0 + w*a1;
}

// int cash(int x, int y){
//     int h = seed + x*374761393 + y*668265263; //all constants are prime
//     h = (h^(h >> 13))*1274126177;
//     return h^(h >> 16);
// }

int hash4(int seed, int a, int b, int c, int d) {
	int sum = seed + 13680553*a + 47563643*b + 84148333*c + 80618477*d;
	sum = (sum ^ (sum >> 13)) * 750490907;
  return sum ^ (sum >> 16);
}

double randfloat(int seed, int x, int y, int a, int b) {
    return (double)(hash4(seed,x,y,a,b)%1000000) / 1000000;
}

// Computes the dot product of the distance and gradient vectors.
double dotGridGradient(int ix, int iy, double x, double y, int seed, int layer) {

    // Precomputed (or otherwise) gradient vectors at each grid node
    //extern float Gradient[IYMAX][IXMAX][2];
    double pvecx = randfloat(seed, ix, iy, layer, 0)*2-1;
    double pvecy = randfloat(seed, ix, iy, layer, 1)*2-1;
    
    //cout << pvecx << '-' << pvecy << endl;
    double dist = sqrt( pvecx*pvecx + pvecy*pvecy );
    pvecx /= dist;
    pvecy /= dist;
    //cout << pvecx << '.' << pvecy << endl;
    // Compute the distance vector
    double dx = x - (double)ix;
    double dy = y - (double)iy;

    // Compute the dot-product
    return (dx*pvecx + dy*pvecy);
}

// Compute Perlin noise at coordinates x, y
double perlin(double x, double y, int seed, int layer) {
    
    int x0 = x;
    int y0 = y;
    
    if (x < 0 and x != x0) x0 --;
    if (y < 0 and y != y0) y0 --;
    
    // Determine grid cell coordinates
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    
    // Determine interpolation weights
    // Could also use higher order polynomial/s-curve here
    double sx = x - (double)x0;
    double sy = y - (double)y0;
    
    // Interpolate between grid point gradients
    double n0, n1, ix0, ix1, value;
    
    n0 = dotGridGradient(x0, y0, x, y, seed, layer);
    n1 = dotGridGradient(x1, y0, x, y, seed, layer);
    ix0 = lerp(n0, n1, sx);
    
    //cout << n0 << ' ' << n1 << endl;
    
    n0 = dotGridGradient(x0, y1, x, y, seed, layer);
    n1 = dotGridGradient(x1, y1, x, y, seed, layer);
    ix1 = lerp(n0, n1, sx);
    
    value = lerp(ix0, ix1, sy);
    //cout << value << endl;
    return value;
}

double scaled_perlin(double x, double y, double height, double width, int seed, int layer) {
    return perlin((double)x/(width), (double)y/(width), seed, layer) *height + height/2;
}


double get_heightmap(int x, int y, int seed) {
    return (scaled_perlin(x,y,16*2,20*2,seed,0) + scaled_perlin(x,y,8*2,10*2,seed,1))*scaled_perlin(x,y,2*2,40*2,seed,2)
     + scaled_perlin(x,y,4*2,5*2,seed,3) + scaled_perlin(x,y,500,500,seed,4);
}

int voronoi2d(double x, double y, int seed, int layer, double threshold) {
  
  int ix = x;
  int iy = y;
  
  if (x < 0 and x != ix) ix --;
  if (y < 0 and y != iy) iy --;
  
  double closest;
  bool first = true;
  int num_close = 0;
  int last_hash = 0;
  
  for (int offx = -1; offx < 2; offx ++) {
    for (int offy = -1; offy < 2; offy ++) {
      double randx = ix + offx + randfloat(seed, ix+offx, iy+offy, layer, 0)*0.8;
      double randy = iy + offy + randfloat(seed, ix+offx, iy+offy, layer, 1)*0.8;
      double dist = std::sqrt((randx-x) * (randx-x) + (randy-y) * (randy-y));
      //dist += perlin(atan2(randy-y, randx-x) * 10, 0, seed, 0) * threshold*2;
      int new_hash = hash4(seed, int(randx*1000), int(randy*1000), last_hash, 0);
      if (true) {
        if (dist <= threshold) {
          return -1;
        }
        if (first or dist < closest - threshold) {
          closest = dist;
          num_close = 1;
          first = false;
          last_hash = new_hash;
        } else if (std::abs(dist-closest) <= threshold) {
          num_close ++;
        }
      }
    }
  }
  if (last_hash % 5 == 0) {
    return 1;
  }
  return num_close;
}

int voronoi3d(double x, double y, double z, int seed, int layer, double threshold) {
  
  int ix = x;
  int iy = y;
  int iz = z;
  
  if (x < 0 and x != ix) ix --;
  if (y < 0 and y != iy) iy --;
  if (z < 0 and z != iz) iz --;
  
  double closest;
  bool first = true;
  int num_close = 0;
  
  for (int offx = -1; offx < 2; offx ++) {
    for (int offy = -1; offy < 2; offy ++) {
      for (int offz = -1; offz < 2; offz ++) {
        double randx = ix + offx + randfloat(seed, ix+offx, iy+offy, iz+offz, layer+0);
        double randy = iy + offy + randfloat(seed, ix+offx, iy+offy, iz+offz, layer+1);
        double randz = iz + offz + randfloat(seed, ix+offx, iy+offy, iz+offz, layer+2);
        double dist = std::sqrt((randx-x) * (randx-x) + (randy-y) * (randy-y) + (randz-z) * (randz-z));
        if (first or dist < closest - threshold) {
          closest = dist;
          num_close = 1;
          first = false;
        } else if (std::abs(dist-closest) <= threshold) {
          num_close ++;
        }
      }
    }
  }
  return num_close;
}







TerrainLoader::TerrainLoader(int nseed): seed(nseed) {
	
}

Block* TerrainLoader::generate_chunk(ivec3 pos) {
	pos *= World::chunksize;
	stringstream ss;
	gen_block(ss, pos.x, pos.y, pos.z, World::chunksize);
	return new Block(ss);
}

Blocktype TerrainLoader::gen_block(ostream& ofile, int gx, int gy, int gz, int scale) {
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


#include <math.h>


constexpr double merge_threshold = 0.2;
const int chunksize = World::chunksize;



int poshash(int seed, ivec3 pos, int myseed) {
	return hash4(seed, pos.x, pos.y, pos.z, myseed);
}

double poshashfloat(int seed, ivec3 pos, int myseed) {
	return randfloat(seed, pos.x, pos.y, pos.z, myseed);
}




ivec3 TerrainObjectPlacer::get_nearest_3d(TerrainLoader* loader, ivec3 pos, ivec3 size, int radius, int position_offset) {
	ivec3 gap = radius - size;
	ivec3 rounded = pos/radius - ivec3(pos.x<0, pos.y<0, pos.z<0);
	int layer = size.x ^ size.y ^ size.z ^ radius;
	ivec3 offset = ivec3(hash4(loader->seed, rounded.x,rounded.y,rounded.z,layer+1),
		hash4(loader->seed, rounded.x,rounded.y,rounded.z,layer+2),
		hash4(loader->seed, rounded.x,rounded.y,rounded.z,layer+3)) % gap;
	ivec3 nearest = offset - pos%radius;
	ivec3 in_bounds = ivec3(nearest.x < size.x, nearest.y < size.y, nearest.z < size.z);
	nearest = nearest * in_bounds + (1 - in_bounds);
	return nearest;
}

ivec3 TerrainObjectPlacer::get_nearest_2d(TerrainLoader* loader, ivec3 pos3d, ivec3 size, int radius, int position_offset) {
	ivec3 orig_pos = pos3d;
	pos3d += ivec3(position_offset, 0, position_offset);
	ivec2 pos (pos3d.x, pos3d.z);
	ivec2 size2d(size.x, size.z);
	ivec2 gap = radius - size2d;
	
	ivec2 rounded = pos/radius - ivec2(pos.x<0, pos.y<0);
	int layer = size.x ^ size.y ^ size.z ^ radius;//unique_seed ^ priority();
	ivec2 offset = ivec2(hash4(loader->seed, rounded.x,rounded.y,layer,1), hash4(loader->seed, rounded.x,rounded.y,layer,2)) % gap;
	ivec2 nearest = offset - (pos%radius + ivec2(pos.x<0, pos.y<0)*radius);
	int height = loader->get_height(ivec2(orig_pos.x, orig_pos.z) + nearest + size2d/2);
	ivec3 nearest3d = ivec3(nearest.x, height - pos3d.y, nearest.y);
	
	if (nearest3d.x <= -size.x) nearest3d.x = 100000;
	if (nearest3d.y <= -size.y) nearest3d.y = 100000;
	if (nearest3d.z <= -size.z) nearest3d.z = 100000;
	
	return nearest3d;
}




template <typename Elev1, typename Elev2, double (*ratiofunc)(TerrainLoader* loader, ivec2 pos)>
int HeightMapMerger<Elev1,Elev2,ratiofunc>::get_height(TerrainLoader* loader, ivec2 pos) {
	double ratio = ratiofunc(loader, pos);
	return Elev1().get_height(loader, pos) * ratio + Elev2().get_height(loader, pos) * (1-ratio);
}





template <typename ... Objs>
template <typename Firstobj, typename Secondobj, typename ... Otherobjs>
char TerrainObjectMerger<Objs...>::gen_func(TerrainLoader* loader, ivec3 pos) {
	Firstobj obj;
	ivec3 nearest = obj.get_nearest(loader, pos);
	char val = -1;
	if (nearest.x <= 0 and nearest.y <= 0 and nearest.z <= 0) {
		val = obj.gen_func(loader, pos+nearest, -nearest);
	}
	if (val == -1) {
		return gen_func<Secondobj,Otherobjs...>(loader, pos);
	} else {
		return val;
	}
}

template <typename ... Objs>
template <typename Lastobj>
char TerrainObjectMerger<Objs...>::gen_func(TerrainLoader* loader, ivec3 pos) {
	Lastobj obj;
	ivec3 nearest = obj.get_nearest(loader, pos);
	char val = -1;
	if (nearest.x <= 0 and nearest.y <= 0 and nearest.z <= 0) {
		val = obj.gen_func(loader, pos+nearest, -nearest);
	}
	return val;
}

template <typename ... Objs>
char TerrainObjectMerger<Objs...>::gen_func(TerrainLoader* loader, ivec3 pos) {
	return gen_func<Objs...>(loader, pos);
}







template <typename Elev, typename Land, typename ... Objs>
int Biome<Elev,Land,Objs...>::get_height(TerrainLoader* loader, ivec2 pos) {
	Elev elev;
	return elev.get_height(loader, pos);
}

template <typename Elev, typename Land, typename ... Objs>
char Biome<Elev,Land,Objs...>::gen_func(TerrainLoader* loader, ivec3 pos) {
	char objval = merger.gen_func(loader, pos);
	if (objval != -1) {
		return objval;
	}
	Land land;
	char val = land.gen_func(loader, pos);
	
	return val;
}




ClimateParams::ClimateParams(TerrainLoader* loader, ivec3 pos):
wetness(scaled_perlin(pos.x, pos.z, 1, 100, loader->seed, 100)),
temp(scaled_perlin(pos.x, pos.z, 1, 100, loader->seed, 101)) {
	
}





char TerrainLoader::get_biome(ivec3 pos) {
	return get_biome(pos, ClimateParams(this, pos));
}

double ratiofunc(TerrainLoader* loader, ivec2 pos) {
	ClimateParams params(loader, ivec3(pos.x,0,pos.y));
	return params.temp / 2 - 0.2;
}
	

char TerrainLoader::get_biome(ivec3 pos, ClimateParams params) {
	if (params.temp > 0.6) {
		return get_biome<Shallow>(pos, params);
	} else if (params.temp > 0.4) {
		return get_biome<HeightMapMerger<Shallow,Steep,ratiofunc>>(pos, params);
	} else {
		return get_biome<Steep>(pos, params);
	}
}

template <typename Elev>
char TerrainLoader::get_biome(ivec3 pos, ClimateParams params) {
	return get_biome<Elev,Plains>(pos, params);
}

template <typename Elev, typename Land>
char TerrainLoader::get_biome(ivec3 pos, ClimateParams params) {
	if (params.wetness > 0.5) {
		return Biome<Elev,Land,TallTree,Tree<0>,BigTree>().gen_func(this, pos);
	} else {
		return Biome<Elev,Land,TallTree,Tree<0>,BigTree>().gen_func(this, pos);
	}
}



int TerrainLoader::get_height(ivec2 pos) {
	return Shallow().get_height(this,pos);
	//return Biome<Shallow,Plains,BigTree,TallTree,Tree<0> >().get_height(this, pos);
	/*
	BiomeBase* biome = get_biome(ivec3(pos.x, 0, pos.y));
	int height = biome->get_height(this,pos);
	delete biome;
	return height;*/
}

int TerrainLoader::gen_func(ivec3 pos) {
	//return get_height(ivec2(pos.x, pos.z)) < pos.y;
	// if (pos.w == 1) {
	// 	return Shallow().get_height(this, ivec2(pos.x, pos.z)) > pos.y;//Biome<Shallow,Plains,TallTree,Tree<0>,BigTree>().gen_func(this, pos);
	// }
	// bool below = Shallow().get_height(this, ivec2(pos.x, pos.z)) > pos.y;
	// for (int x = 0; x < pos.w; x ++) {
	// 	for (int z = 0; z < pos.w; z ++) {
	// 		if (below) {
	// 			if (Shallow().get_height(this, ivec2(pos.x+x, pos.z+z)) <= pos.y+pos.w-1) {
	// 				return -1;
	// 			}
	// 		} else {
	// 			if (Shallow().get_height(this, ivec2(pos.x+x, pos.z+z)) > pos.y) {
	// 				return -1;
	// 			}
	// 		}
	// 	}
	// }
	// return below;
	// if (pos.w == 1) {
		// return pos.y*2 < pos.x;
		char val = get_biome(ivec3(pos.x, pos.y, pos.z));
		//Biome<Shallow,Plains,TallTree,Tree<0>,BigTree>().gen_func(this, ivec3(pos.x, pos.y, pos.z));
		if (val == 0 and pos.y < 96) {
			return blocktypes::water.id;
		}
		return val;
	// }
	// return -1;
	/*
	BiomeBase* biome = get_biome(pos);
	char val = biome->gen_func(this,pos);
	delete biome;
	return val;*/
}



template <int sx, int sy, int sz, int radius, int pos_offset>
ivec3 Placer2D<sx,sy,sz,radius,pos_offset>::get_nearest(TerrainLoader* loader, ivec3 pos) {
	return get_nearest_2d(loader, pos, ivec3(sx,sy,sz), radius, pos_offset);
}




template <typename Placer, typename ... Objs>
ivec3 RareObjects<Placer,Objs...>::get_nearest(TerrainLoader* loader, ivec3 pos) {
	return Placer().get_nearest(loader, pos);
}

template <typename Placer, typename ... Objs>
char RareObjects<Placer,Objs...>::gen_func(TerrainLoader* loader, ivec3 off, ivec3 pos) {
	int index = poshash(loader->seed, off, 1290) % sizeof...(Objs);
	return gen_func<Objs...>(loader, off, pos, index);
}

template <typename Placer, typename ... Objs>
template <typename FirstObj, typename SecondObj, typename ... OtherObjs>
char RareObjects<Placer,Objs...>::gen_func(TerrainLoader* loader, ivec3 off, ivec3 pos, int index) {
	if (index <= 0) {
		return FirstObj().gen_func(loader, off, pos);
	} else {
		return gen_func<SecondObj,OtherObjs...>(loader, off, pos, index-1);
	}
}

template <typename Placer, typename ... Objs>
template <typename LastObj>
char RareObjects<Placer,Objs...>::gen_func(TerrainLoader* loader, ivec3 off, ivec3 pos, int index) {
	return LastObj().gen_func(loader, off, pos);
}

template <int off> ivec3 Tree<off>::get_nearest(TerrainLoader* loader, ivec3 pos) {
	return get_nearest_2d(loader, pos, ivec3(10,15,10), 20, off);
}

template <int off> char Tree<off>::gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos) {
	if (false or poshash(loader->seed, offset, 24242) % 20 == 0) {
		vec2 trunk_off(perlin(pos.y/3.0, offset.x ^ offset.z, loader->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, loader->seed, 27554245)*2);
		if (glm::length(vec2(pos.x-5, pos.z-5) - trunk_off) < 1.75 - (pos.y/10.0) and pos.y < 10) {
			return blocktypes::bark.id;
		} else if (pos.y > 4 + poshash(loader->seed, offset, 48935) % 4 and glm::length(vec2(pos.x-5,pos.z-5)) + (pos.y-6)/2 <
		3+poshashfloat(loader->seed, offset+pos, 2634928)) {
			if (poshash(loader->seed, offset, 244562) % 20 == 0) {
				return -1;
			}
			return blocktypes::bark.id;
		}
		return -1;
	}
	vec2 trunk_off(perlin(pos.y/3.0, offset.x ^ offset.z, loader->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, loader->seed, 2749245)*2);
	if (glm::length(vec2(pos.x-5, pos.z-5) - trunk_off) < 1.75 - (pos.y/10.0) and pos.y < 10) {
		return blocktypes::bark.id;
	} else if (pos.y > 4 + poshash(loader->seed, offset, 48935) % 4 and glm::length(vec3(5,8,5)-vec3(pos)) <
	4 + poshashfloat(loader->seed, offset+pos, 37482)) {
		if (poshash(loader->seed, offset, 242362) % 20 == 0) {
			return -1;
		}
		return blocktypes::leaves.id;
	}
	return -1;
}


char TallTree::gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos) {
	if (false or poshash(loader->seed, offset,3487394) % 5 == 0) {
		double center_dist = glm::length(vec2(pos.x-10,pos.z-10));
		int start_leaves = 10 + poshash(loader->seed, offset, 45356) % 10;
		double leaves_frill = 2*poshashfloat(loader->seed, offset+pos, 2634928);
		vec2 trunk_off(perlin(pos.y/3.0+3543, offset.x ^ offset.z, loader->seed, 3453458)*2, perlin(pos.y/10.0, offset.x ^ offset.z, loader->seed, 2749245)*2);
		
		if (glm::length(vec2(pos.x-10, pos.z-10) - trunk_off) < 4 - pos.y/40.0 * 4) {
			return blocktypes::bark.id;
		} else if (pos.y > start_leaves and center_dist + (pos.y-15)/3 < 8 + leaves_frill) {
			return blocktypes::leaves.id;
		}
		return -1;
	}
	
	vec2 trunk_off(perlin(pos.y/3.0+4543, offset.x ^ offset.z, loader->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, loader->seed, 2749245)*2);
	if (glm::length(vec2(pos.x-10, pos.z-10) - trunk_off) < 4 - (pos.y/15.0)*2 and pos.y < 25) {
		return blocktypes::bark.id;
	}
	double leaves_start = perlin(pos.x/3.0, pos.z/3.0, loader->seed, 49375^offset.x) * (10 + 15 * (poshash(loader->seed, offset,237842)%2));
	double len = glm::length(vec2(pos.x-10, pos.z-10))/2;
	len *= len;
	leaves_start += 10 + len;
	
	if (pos.y >= leaves_start) {
		double leaves_end = perlin(pos.x/3.0, pos.z/3.0, loader->seed, 445345^offset.z) * 3;
		double len = glm::length(vec2(pos.x-10, pos.z-10))/(2 + 2 * (poshash(loader->seed, offset,273345)%2));
		len *= len;
		leaves_end = 35 - leaves_end - len;
		if (pos.y <= leaves_end) {
			return blocktypes::leaves.id;
		}
	}
	
	return -1;
}

ivec3 TallTree::get_nearest(TerrainLoader* loader, ivec3 pos) {
	return get_nearest_2d(loader, pos, ivec3(20,40,20), 50, 0) - ivec3(0,5,0);
}






char BigTree::gen_func(TerrainLoader* loader, ivec3 offset, ivec3 pos) {
	
	pos.y -= (loader->get_height(ivec2(offset.x+pos.x, offset.z+pos.z)) - offset.y)*(1 - pos.y/200.0);
	
	int seed = hash4(loader->seed, offset.x, offset.y, offset.z, 0);
	double angle = atan2(pos.x-25, pos.z-25)*30;
	double dist = sqrt((pos.x-25)*(pos.x-25) + (pos.z-25)*(pos.z-25));
	//double dist = std::abs(pos.x-25) + std::abs(pos.z-25);
	double perlin = scaled_perlin(angle+30, pos.y/10.0+30, 10, 5, seed, 1)-5;
	if (pos.y > 22) {
		perlin += (pos.y-25)/5;
		double dist3d = sqrt((pos.x-25)*(pos.x-25) + (pos.y-40)*(pos.y-40) + (pos.z-25)*(pos.z-25));
		if (dist3d > 25) {
			return -1;
		}
	}
	double multiplier = (pos.y/25.0 - 0.5);
	multiplier = 50*multiplier*multiplier*multiplier*multiplier;
	double height = perlin*multiplier;
	height += 5 + 3*multiplier;
	if (dist < height and dist > height-(perlin*3)-5) {
		return 1;
	} else if (dist <= height-(perlin*3)-5 and pos.y > 25) {
		return blocktypes::leaves.id;
	}
	return -1;
}

ivec3 BigTree::get_nearest(TerrainLoader* loader, ivec3 pos) {
	return get_nearest_2d(loader, pos, ivec3(50,100,50), 100) - ivec3(0,20,0);
}





int Flat::get_height(TerrainLoader* loader, ivec2 pos) {
	return 50;
}

int Shallow::get_height(TerrainLoader* loader, ivec2 pos) {
	return int(get_heightmap(pos.x, pos.y, 238479))/2;
}

int Steep::get_height(TerrainLoader* loader, ivec2 pos) {
	return int(get_heightmap(pos.x, pos.y, 238479))*2/3;
}




char Plains::gen_func(TerrainLoader* loader, ivec3 pos) {
	int height = loader->get_height(ivec2(pos.x, pos.z));
	if (pos.y < height-5) {
			return blocktypes::grass.id;//blocs->nmes["rock"];
	} if (pos.y < height-1) {
			return blocktypes::dirt.id;//blcks->naes["dirt"];
	} if (pos.y < height) {
		if (pos.y < 96) {
			return blocktypes::dirt.id;//blocs->naes["gravel"];
		} else {
			return blocktypes::grass.id;//blcks->nmes["grass"];
		}
	}
	return 0;
}
