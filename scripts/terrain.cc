#ifndef TERRAIN
#define TERRAIN

#include "terrain.h"
#include "blockdata.h"
#include "generative.h"
#include "world.h"
#include "cross-platform.h"

#include <math.h>


int poshash(int seed, ivec3 pos, int myseed) {
	return hash4(seed, pos.x, pos.y, pos.z, myseed);
}

double poshashfloat(int seed, ivec3 pos, int myseed) {
	return randfloat(seed, pos.x, pos.y, pos.z, myseed);
}



template <ivec3 offset, ivec3 scale>
class Rect : public TerrainShape { public:
	ShapeResult collide(ivec4 pos) {
		bool containing = true;
		for (int axis = 0; axis < 3; axis ++) {
			if (pos[axis] - offset[axis] > scale[axis]
				or offset[axis] - pos[axis] > pos.w) {
				return SHAPE_OUTSIDE;
			}
			if (offset[axis] > pos[axis] or pos[axis] + pos.w > offset[axis] + scale[axis]) {
				containing = false;
			}
		}
		return containing ? SHAPE_INSIDE : SHAPE_INTERSECT;
	}
};

template <typename ShapeType>
class Invert : public TerrainShape { public:
	ShapeResult collide(ivec4 pos) {
		ShapeResult result = ShapeType().collide(pos)
		if (result == SHAPE_OUTSIDE) {
			return SHAPE_INSIDE;
		}
		if (result == SHAPE_INSIDE) {
			return SHAPE_OUTSIDE;
		}
		return result;
	}
};



template <ivec3 scale, int heightscale, ivec3 offset = ivec3(0,0,0)>
class Perlin2D { public:
	ShapeResult collide(ivec4 pos) {
		double perlinval = scaled_perlin(





/*
constexpr double merge_threshold = 0.2;
const int chunksize = World::chunksize;




ivec3 TerrainObjectPlacer::get_nearest_3d(TerrainLoader* loader, ivec3 pos, ivec3 size, int radius, int position_offset) {
	ivec3 gap = radius - size;
	ivec3 rounded = pos/radius - ivec3(pos.x<0, pos.y<0, pos.z<0);
	int layer = size.x ^ size.y ^ size.z ^ radius;
	ivec3 offset = ivec3(hash4(loader->seed, rounded.x,rounded.y,rounded.z,layer+1),
		hash4(loader->seed, rounded.x,rounded.y,rounded.z,layer+2),
		hash4(loader->seed, rounded.x,rounded.y,rounded.z,layer+3)) % gap;
	ivec3 nearest = offset - pos%radius;
	ivec3 in_bounds = ivec3(nearest.x < size.x, nearest.y < 	size.y, nearest.z < size.z);
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




TerrainLoader::TerrainLoader(int newseed): seed(newseed) {
	
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

int TerrainLoader::gen_func(ivec4 pos) {
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
	if (pos.w == 1) {
		// return pos.y*2 < pos.x;
		char val = get_biome(ivec3(pos.x, pos.y, pos.z));
		//Biome<Shallow,Plains,TallTree,Tree<0>,BigTree>().gen_func(this, ivec3(pos.x, pos.y, pos.z));
		if (val == 0 and pos.y < 96) {
			return 7;
		}
		return val;
	}
	return -1;
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
			return blocks->names["bark"];
		} else if (pos.y > 4 + poshash(loader->seed, offset, 48935) % 4 and glm::length(vec2(pos.x-5,pos.z-5)) + (pos.y-6)/2 <
		3+poshashfloat(loader->seed, offset+pos, 2634928)) {
			if (poshash(loader->seed, offset, 244562) % 20 == 0) {
				return -1;
			}
			return blocks->names["leaves"];
		}
		return -1;
	}
	vec2 trunk_off(perlin(pos.y/3.0, offset.x ^ offset.z, loader->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, loader->seed, 2749245)*2);
	if (glm::length(vec2(pos.x-5, pos.z-5) - trunk_off) < 1.75 - (pos.y/10.0) and pos.y < 10) {
		return blocks->names["bark"];
	} else if (pos.y > 4 + poshash(loader->seed, offset, 48935) % 4 and glm::length(vec3(5,8,5)-vec3(pos)) <
	4 + poshashfloat(loader->seed, offset+pos, 37482)) {
		if (poshash(loader->seed, offset, 242362) % 20 == 0) {
			return -1;
		}
		return blocks->names["leaves"];
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
			return blocks->names["bark"];
		} else if (pos.y > start_leaves and center_dist + (pos.y-15)/3 < 8 + leaves_frill) {
			return blocks->names["leaves"];
		}
		return -1;
	}
	
	vec2 trunk_off(perlin(pos.y/3.0+4543, offset.x ^ offset.z, loader->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, loader->seed, 2749245)*2);
	if (glm::length(vec2(pos.x-10, pos.z-10) - trunk_off) < 4 - (pos.y/15.0)*2 and pos.y < 25) {
		return blocks->names["bark"];
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
			return blocks->names["leaves"];
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
		return blocks->names["leaves"];
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
			return blocks->names["rock"];
	} if (pos.y < height-1) {
			return blocks->names["dirt"];
	} if (pos.y < height) {
		if (pos.y < 96) {
			return blocks->names["gravel"];
		} else {
			return blocks->names["grass"];
		}
	}
	return 0;
}







/*
TerrainBase::TerrainBase(TerrainLoader* loader): parent(loader) {
	
}





TerrainBaseMerger::TerrainBaseMerger(TerrainBase* newfirst, TerrainBase* newsecond): TerrainBase(newfirst->parent), first(newfirst), second(newsecond) {
	
}

int TerrainBaseMerger::get_height(ivec2 pos) {
	if (second == nullptr) {
		return first->get_height(pos);
	}
	ivec3 pos3(pos.x, 0, pos.y);
	double wetness = parent->get_wetness(pos3);
	double temp = parent->get_temp(pos3);
	double elev = parent->get_elev(pos3);
	double bestscore = first->valid_score(wetness, temp, elev);
	double nextscore = second->valid_score(wetness, temp, elev);
	double diff = bestscore-nextscore;
	
	double bestweight = 0.5+(diff/merge_threshold)*0.5;
	double nextweight = 0.5-(diff/merge_threshold)*0.5;
	int bestheight = first->get_height(pos);
	int nextheight = second->get_height(pos);
	int height = bestheight * bestweight + nextheight * nextweight;
	return height;
}

char TerrainBaseMerger::gen_func(ivec3 pos) {
	if (second == nullptr) {
		return first->gen_func(pos);
	}
	double wetness = parent->get_wetness(pos);
	double temp = parent->get_temp(pos);
	double elev = parent->get_elev(pos);
	double bestscore = first->valid_score(wetness, temp, elev);
	double nextscore = second->valid_score(wetness, temp, elev);
	//cout << '-' << bestscore << ' ' << nextscore << endl;
	double diff = bestscore-nextscore;
	
	double bestweight = 0.5+(diff/merge_threshold)*0.5;
	double nextweight = 0.5-(diff/merge_threshold)*0.5;
	int bestheight = first->get_height(ivec2(pos.x, pos.z));
	int nextheight = second->get_height(ivec2(pos.x, pos.z));
	int height = bestheight * bestweight + nextheight * nextweight;
	if (randfloat(parent->seed, pos.x, pos.y, pos.z, 2432) < nextweight) {
		return second->gen_func(pos + ivec3(0, nextheight - height, 0));
	} else {
		return first->gen_func(pos + ivec3(0, bestheight - height, 0));
	}
}

double TerrainBaseMerger::valid_score(double wetness, double temp, double elev) {
	exit(1);
}

string TerrainBaseMerger::name() {
	if (second == nullptr) {
		return first->name();
	} else {
		return first->name() + '-' + second->name();
	}
}






TerrainObject::TerrainObject(TerrainLoader* loader, ivec3 nsize, int nradius, int new_unique_seed):
parent(loader), size(nsize), radius(nradius),
unique_seed(loader->seed ^ nsize.x ^ nsize.y ^ nsize.z ^ nradius ^ (new_unique_seed+1)), position_offset(new_unique_seed%radius) {
	
}

ivec3 TerrainObject::get_nearest(ivec3 pos) {
	return get_nearest_3d(pos);
}

ivec3 TerrainObject::get_nearest_3d(ivec3 pos) {
	ivec3 gap = radius - size;
	ivec3 rounded = pos/radius - ivec3(pos.x<0, pos.y<0, pos.z<0);
	int layer = unique_seed ^ priority();
	ivec3 offset = ivec3(hash4(parent->seed, rounded.x,rounded.y,rounded.z,layer+1),
		hash4(parent->seed, rounded.x,rounded.y,rounded.z,layer+2),
		hash4(parent->seed, rounded.x,rounded.y,rounded.z,layer+3)) % gap;
	ivec3 nearest = offset - pos%radius;
	return nearest;
}

ivec3 TerrainObject::get_nearest_2d(ivec3 pos3d) {
	ivec3 orig_pos = pos3d;
	pos3d += ivec3(position_offset, 0, position_offset);
	ivec2 pos (pos3d.x, pos3d.z);
	ivec2 size2d(size.x, size.z);
	ivec2 gap = radius - size2d;
	ivec2 rounded = pos/radius - ivec2(pos.x<0, pos.y<0);
	int layer = unique_seed ^ priority();
	ivec2 offset = ivec2(hash4(parent->seed, rounded.x,rounded.y,layer,1), hash4(parent->seed, rounded.x,rounded.y,layer,2)) % gap;
	ivec2 nearest = offset - (pos%radius + ivec2(pos.x<0, pos.y<0)*radius);
	int height = parent->get_height(ivec2(orig_pos.x, orig_pos.z) + nearest + size2d/2);
	ivec3 nearest3d = ivec3(nearest.x, height - pos3d.y, nearest.y);
	return nearest3d;
}







TerrainObjectMerger::TerrainObjectMerger(TerrainLoader* newparent): parent(newparent) {
	objs = vector<TerrainObject*> {new Tree(this,0), new Tree(this,10), new Lamp(this), new Cave(this), new BigTree(this), new TallTree(this)};
	vector<string> files;
	get_files_folder("resources/data/terrain", &files);
	for (string file : files) {
		objs.push_back(new FileTerrain(this, "resources/data/terrain/" + file));
	}
	cout << "loaded " << files.size() << " terrain objects from file" << endl;
}

TerrainObjectMerger::~TerrainObjectMerger() {
	for (TerrainObject* obj : objs) {
		delete obj;
	}
}

char TerrainObjectMerger::gen_func(ivec3 pos) {
	char val = -1;
	int priority = 0;
	for (TerrainObject* obj : objs) {
		ivec3 nearest = obj->get_nearest(pos);
		if (obj->is_valid(pos+nearest)) {
			if (obj->priority() > priority and nearest.x <= 0 and nearest.y <= 0 and nearest.z <= 0
				and nearest.x > -obj->size.x and nearest.y > -obj->size.y and nearest.z > -obj->size.z ) {
				char newval = obj->gen_func(pos + nearest, -nearest);
				if (newval != -1) {
					val = newval;
					priority = obj->priority();
				}
			}
		}
	}
	return val;
}


TerrainLoader::TerrainLoader(int nseed): objmerger(this), seed(nseed),
 	bases({new Mountains(this), new Plains(this)}) {
	
}

TerrainLoader::~TerrainLoader() {
	for (TerrainBase* base : bases) {
		delete base;
	}
}

TerrainBaseMerger TerrainLoader::get_base(ivec3 pos) {
	//return TerrainBaseMerger(bases[0], nullptr);
	double wetness = 0;//get_wetness(pos);
	double temp = 0;//get_temp(pos);
	double elev = 0;//get_elev(pos);
	TerrainBase* bestbase = nullptr;
	double bestscore = 0;
	TerrainBase* nextbase = nullptr;
	double nextscore = 0;
	for (TerrainBase* base : bases) {
		double score = base->valid_score(wetness, temp, elev);
		if (score > bestscore) {
			nextbase = bestbase;
			nextscore = bestscore;
			bestbase = base;
			bestscore = score;
		} else if (score > nextscore) {
			nextbase = base;
			nextscore = score;
		}
	}
	double diff = bestscore-nextscore;
	if (diff < merge_threshold and nextbase != nullptr) {
		//cout << bestscore << ' ' << nextscore << endl;
		return TerrainBaseMerger(bestbase, nextbase);
	}
	return TerrainBaseMerger(bestbase, nullptr);
}

string TerrainLoader::get_biome_name(ivec3 pos) {
	return get_base(pos).name();
}

double TerrainLoader::get_wetness(ivec3 pos) {
	return scaled_perlin(pos.x, pos.z, 1, 100, seed, 100);
}

double TerrainLoader::get_temp(ivec3 pos) {
	return scaled_perlin(pos.x, pos.z, 1, 100, seed, 101);
}

double TerrainLoader::get_elev(ivec3 pos) {
	return scaled_perlin(pos.x, pos.z, 1, 100, seed, 102);
}

int TerrainLoader::get_height(ivec2 pos) {
	return get_base(ivec3(pos.x, 0, pos.y)).get_height(pos);
}

char TerrainLoader::gen_func(ivec3 pos) {
	//get_height(ivec2(pos.x, pos.z));
	//return pos.y < 96;
	// double start = glfwGetTime();
	char objval = objmerger.gen_func(pos);
	// cout << glfwGetTime() - start << " objs " << endl;
	char val;
	if (objval != -1) {
		val = objval;
	} else {
		//cout << "start gen func" << endl;
		// start = glfwGetTime();
		val = get_base(pos).gen_func(pos);
		// cout << glfwGetTime() - start << " base " << endl;
		//cout << "end gen func" << endl;
	}
	if (val == 0 and pos.y < 96) {
		return blocks->names["water"];
	}
	// exit(1);
	return val;
}




Plains::Plains(TerrainLoader* loader): TerrainBase(loader) {
	
}

int Plains::get_height(ivec2 pos) {
	return int(get_heightmap(pos.x, pos.y, parent->seed))/2;
}

char Plains::gen_func(ivec3 pos) {
	int height = get_height(ivec2(pos.x, pos.z));
	if (pos.y < height-5) {
			return blocks->names["rock"];
	} if (pos.y < height-1) {
			return blocks->names["dirt"];
	} if (pos.y < height) {
		if (pos.y < 96) {
			return blocks->names["gravel"];
		} else {
			return blocks->names["grass"];
		}
	}
	return 0;
}

double Plains::valid_score(double wetness, double temp, double elev) {
	return 1-elev;
}

string Plains::name() {
	return "plains";
}







Flatworld::Flatworld(TerrainLoader* loader, char newblock, int newheight): TerrainBase(loader), block(newblock), height(newheight) {
	
}

int Flatworld::get_height(ivec2 pos) {
	return height;
}

char Flatworld::gen_func(ivec3 pos) {
	return (pos.y <= height) * block;
}

double Flatworld::valid_score(double wetness, double temp, double elev) {
	return 1;
}

string Flatworld::name() {
	return "flatworld";
}




Forest::Forest(TerrainLoader* loader): TerrainBase(loader) {
	
}

int Forest::get_height(ivec2 pos) {
	return int(get_heightmap(pos.x, pos.y, parent->seed))*2/3;
}

char Forest::gen_func(ivec3 pos) {
	int height = get_height(ivec2(pos.x, pos.z));
	if (pos.y < height-5) {
			return blocks->names["rock"];
	} if (pos.y < height-1) {
			return blocks->names["dirt"];
	} if (pos.y < height) {
		if (pos.y < 96) {
			return blocks->names["gravel"];
		} else {
			return blocks->names["grass"];
		}
	}
	return 0;
}

double Forest::valid_score(double wetness, double temp, double elev) {
	return 1-elev + wetness;
}

string Forest::name() {
	return "forest";
}





Mountains::Mountains(TerrainLoader* loader): TerrainBase(loader) {
	
}

int Mountains::get_height(ivec2 pos) {
	return int(get_heightmap(pos.x, pos.y, parent->seed))*3/4;
}

char Mountains::gen_func(ivec3 pos) {
	int height = get_height(ivec2(pos.x, pos.z));
	if (pos.y < height-5) {
			return blocks->names["shale"];
	} if (pos.y < height-2) {
			return blocks->names["rock"];
	} if (pos.y < height-1) {
			return blocks->names["dirt"];
	} if (pos.y < height) {
		if (pos.y < 96) {
			return blocks->names["gravel"];
		} else {
			return blocks->names["snow"];
		}
	}
	return 0;
}

double Mountains::valid_score(double wetness, double temp, double elev) {
	return elev;
}

string Mountains::name() {
	return "mountains";
}






BlanketForest::BlanketForest(TerrainLoader* loader): TerrainBase(loader) {
	
}

int BlanketForest::get_height(ivec2 pos) {
	return int(get_heightmap(pos.x, pos.y, parent->seed))/2;
}

char BlanketForest::gen_func(ivec3 pos) {
	int height = get_height(ivec2(pos.x, pos.z));
	if (pos.y < height-5) {
			return blocks->names["rock"];
	} if (pos.y < height-1) {
			return blocks->names["dirt"];
	} if (pos.y < height) {
		if (pos.y < 96) {
			return blocks->names["gravel"];
		} else {
			return blocks->names["grass"];
		}
	} else {
		
	}
	return 0;
}

double BlanketForest::valid_score(double wetness, double temp, double elev) {
	return 1-elev;
}

string BlanketForest::name() {
	return "blanketforest";
}







Tree::Tree(TerrainObjectMerger* merger, int uid): TerrainObject(merger->parent, ivec3(10,15,10), 20, uid) {
	
}

char Tree::gen_func(ivec3 offset, ivec3 pos) {
	if (parent->get_biome_name(offset) == "mountain" or poshash(offset, 24242) % 20 == 0) {
		vec2 trunk_off(perlin(pos.y/3.0, offset.x ^ offset.z, parent->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, parent->seed, 27554245)*2);
		if (glm::length(vec2(pos.x-5, pos.z-5) - trunk_off) < 1.75 - (pos.y/10.0) and pos.y < 10) {
			return blocks->names["bark"];
		} else if (pos.y > 4 + poshash(offset, 48935) % 4 and glm::length(vec2(pos.x-5,pos.z-5)) + (pos.y-6)/2 <
		3+poshashfloat(offset+pos, 2634928)) {
			if (poshash(offset, 244562) % 20 == 0) {
				return -1;
			}
			return blocks->names["leaves"];
		}
		return -1;
	}
	
	vec2 trunk_off(perlin(pos.y/3.0, offset.x ^ offset.z, parent->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, parent->seed, 2749245)*2);
	if (glm::length(vec2(pos.x-5, pos.z-5) - trunk_off) < 1.75 - (pos.y/10.0) and pos.y < 10) {
		return blocks->names["bark"];
	} else if (pos.y > 4 + poshash(offset, 48935) % 4 and glm::length(vec3(5,8,5)-vec3(pos)) <
	4 + poshashfloat(offset+pos, 37482)) {
		if (poshash(offset, 242362) % 20 == 0) {
			return -1;
		}
		return blocks->names["leaves"];
	}
	return -1;
}

ivec3 Tree::get_nearest(ivec3 pos) {
	return get_nearest_2d(pos);
}

int Tree::priority() {
	return 5;
}

bool Tree::is_valid(ivec3 pos) {
	return true;
}




TallTree::TallTree(TerrainObjectMerger* merger, int uid): TerrainObject(merger->parent, ivec3(20,40,20), 40, uid) {
	
}

char TallTree::gen_func(ivec3 offset, ivec3 pos) {
	if (parent->get_biome_name(offset) == "mountains" or poshash(offset,3487394) % 5 == 0) {
		double center_dist = glm::length(vec2(pos.x-10,pos.z-10));
		int start_leaves = 10 + poshash(offset, 45356) % 10;
		double leaves_frill = 2*poshashfloat(offset+pos, 2634928);
		vec2 trunk_off(perlin(pos.y/3.0+3543, offset.x ^ offset.z, parent->seed, 3453458)*2, perlin(pos.y/10.0, offset.x ^ offset.z, parent->seed, 2749245)*2);
		
		if (glm::length(vec2(pos.x-10, pos.z-10) - trunk_off) < 4 - pos.y/40.0 * 4) {
			return blocks->names["bark"];
		} else if (pos.y > start_leaves and center_dist + (pos.y-15)/3 < 8 + leaves_frill) {
			return blocks->names["leaves"];
		}
		return -1;
	}
	
	vec2 trunk_off(perlin(pos.y/3.0+4543, offset.x ^ offset.z, parent->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, parent->seed, 2749245)*2);
	if (glm::length(vec2(pos.x-10, pos.z-10) - trunk_off) < 4 - (pos.y/15.0)*2 and pos.y < 25) {
		return blocks->names["bark"];
	}
	double leaves_start = perlin(pos.x/3.0, pos.z/3.0, parent->seed, 49375^offset.x) * (10 + 15 * (poshash(offset,237842)%2));
	double len = glm::length(vec2(pos.x-10, pos.z-10))/2;
	len *= len;
	leaves_start += 10 + len;
	
	if (pos.y >= leaves_start) {
		double leaves_end = perlin(pos.x/3.0, pos.z/3.0, parent->seed, 445345^offset.z) * 3;
		double len = glm::length(vec2(pos.x-10, pos.z-10))/(2 + 2 * (poshash(offset,273345)%2));
		len *= len;
		leaves_end = 35 - leaves_end - len;
		if (pos.y <= leaves_end) {
			return blocks->names["leaves"];
		}
	}
	
	return -1;
}

ivec3 TallTree::get_nearest(ivec3 pos) {
	return get_nearest_2d(pos) - ivec3(0,5,0);
}

int TallTree::priority() {
	return 5;
}

bool TallTree::is_valid(ivec3 pos) {
	return true;
}






BigTree::BigTree(TerrainObjectMerger* merger, int uid): TerrainObject(merger->parent, ivec3(50,100,50), 1000, uid) {
	
}

char BigTree::gen_func(ivec3 offset, ivec3 pos) {
	
	pos.y -= (parent->get_height(ivec2(offset.x+pos.x, offset.z+pos.z)) - offset.y)*(1 - pos.y/200.0);
	
	int seed = hash4(parent->seed, offset.x, offset.y, offset.z, 0);
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
		return blocks->names["leaves"];
	}
	return -1;
}

ivec3 BigTree::get_nearest(ivec3 pos) {
	return get_nearest_2d(pos) - ivec3(0,20,0);
}

int BigTree::priority() {
	return 11;
}

bool BigTree::is_valid(ivec3 pos) {
	return true;
}





Cave::Cave(TerrainObjectMerger* merger, int uid): TerrainObject(merger->parent, ivec3(10,10,10), 40, uid) {
	
}

char Cave::gen_func(ivec3 offset, ivec3 pos) {
	return (glm::length(vec3(5,5,5)-vec3(pos)) < 4) - 1;
}

ivec3 Cave::get_nearest(ivec3 pos) {
	return get_nearest_2d(pos) - ivec3(0,5,0);
}

int Cave::priority() {
	return 1;
}

bool Cave::is_valid(ivec3 pos) {
	return true;
}








Lamp::Lamp(TerrainObjectMerger* merger, int uid): TerrainObject(merger->parent, ivec3(1,15,1), 40, uid) {
	
}

char Lamp::gen_func(ivec3 offset, ivec3 pos) {
	if (pos.y < 4) {
		return blocks->names["bark"];
	} else {
		int voronoi = voronoi2d(offset.x/600.0, offset.z/600.0, parent->seed, 1, 1/600.0);
		if (voronoi == 2) {
			if (pos.y == 4) {
				return blocks->names["torch"];
			} else {
				return -1;
			}
		} else {
			if (pos.y < 14) {
				return blocks->names["bark"];
			} else {
				return blocks->names["torch"];
			}
		}
	}
}

ivec3 Lamp::get_nearest(ivec3 pos) {
	int voronoi = voronoi2d(pos.x/600.0, pos.z/600.0, parent->seed, 1, 1/600.0);
	if ((voronoi > 1 and poshash(ivec3(pos.x, 0, pos.z), 49332) % 30 == 0) or voronoi > 2) {
		return ivec3(0,parent->get_height(ivec2(pos.x, pos.z)) - pos.y,0);
	}
	return ivec3(100,100,100);
}

int Lamp::priority() {
	return 20;
}

bool Lamp::is_valid(ivec3 pos) {
	return true;
}







FileTerrain::FileTerrain(TerrainObjectMerger* merger, string path, int uid): TerrainObject(merger->parent, ivec3(1,1,1), 1, uid) {
	ifstream ifile(path);
	//cout << "loading terrain object " << path << endl;
	int scale;
	bool ignore_air;
	ifile >> scale;
	size = ivec3(scale, scale, scale);
	ifile >> radius;
	ifile >> priority_level;
	ifile >> ignore_air;
	string buff;
	getline(ifile, buff, ':');
	stringstream biomestr(buff);
	while (biomestr >> buff) {
		biomes.emplace(buff);
	}
	data = new char[scale*scale*scale];
	ifile.read(data, scale*scale*scale);
	if (ignore_air) {
		for (int i = 0; i < scale*scale*scale; i ++) {
			if (data[i] == 0) {
				data[i] = -1;
			}
		}
	}
}

FileTerrain::~FileTerrain() {
	delete[] data;
}

char FileTerrain::gen_func(ivec3 offset, ivec3 pos) {
	return data[pos.x*size.x*size.x + pos.y*size.x + pos.z];
}

ivec3 FileTerrain::get_nearest(ivec3 pos) {
	return get_nearest_2d(pos) - ivec3(0,size.y/3,0);
}

int FileTerrain::priority() {
	return priority_level;
}

bool FileTerrain::is_valid(ivec3 pos) {
	return true;
	return biomes.count(parent->get_biome_name(pos)) > 0;
}
*/
#endif
