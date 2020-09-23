#ifndef TERRAIN
#define TERRAIN

#include "terrain.h"
#include "blockdata.h"
#include "generative.h"
#include "world.h"
#include "cross-platform.h"

#include <math.h>


constexpr double merge_threshold = 0.2;
const int chunksize = World::chunksize;

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
parent(loader), size(nsize), radius(nradius), unique_seed(loader->seed ^ nsize.x ^ nsize.y ^ nsize.z ^ nradius ^ (new_unique_seed+1)) {
	
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

ivec2 TerrainObject::get_nearest_2d(ivec2 pos) {
	ivec2 size2d(size.x, size.z);
	ivec2 gap = radius - size2d;
	ivec2 rounded = pos/radius - ivec2(pos.x<0, pos.y<0);
	int layer = unique_seed ^ priority();
	ivec2 offset = ivec2(hash4(parent->seed, rounded.x,rounded.y,layer,1), hash4(parent->seed, rounded.x,rounded.y,layer,2)) % gap;
	ivec2 nearest = offset - (pos%radius + ivec2(pos.x<0, pos.y<0)*radius);
	return nearest;
}









TerrainObjectMerger::TerrainObjectMerger(TerrainLoader* newparent): parent(newparent) {
	objs = vector<TerrainObject*> {new Tree(this), new Cave(this), new BigTree(this), new TallTree(this)};
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
 	bases({new Flatworld(this,char(2),96)}) {//new Mountains(this), new Plains(this)}) {
	
}

TerrainLoader::~TerrainLoader() {
	for (TerrainBase* base : bases) {
		delete base;
	}
}

TerrainBaseMerger TerrainLoader::get_base(ivec3 pos) {
	double wetness = get_wetness(pos);
	double temp = get_temp(pos);
	double elev = get_elev(pos);
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
	char objval = objmerger.gen_func(pos);
	char val;
	if (objval != -1) {
		val = objval;
	} else {
		//cout << "start gen func" << endl;
		val = get_base(pos).gen_func(pos);
		//cout << "end gen func" << endl;
	}
	if (val == 0 and pos.y < 96) {
		return blocks->names["water"];
	}
	
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



Tree::Tree(TerrainObjectMerger* merger, int uid): TerrainObject(merger->parent, ivec3(10,15,10), 15, uid) {
	
}

char Tree::gen_func(ivec3 offset, ivec3 pos) {
	if (parent->get_biome_name(offset) == "mountain" or hash4(parent->seed, offset.x, offset.y, offset.z, 24242) % 20 == 0) {
		vec2 trunk_off(perlin(pos.y/3.0, offset.x ^ offset.z, parent->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, parent->seed, 27554245)*2);
		if (glm::length(vec2(pos.x-5, pos.z-5) - trunk_off) < 1.75 - (pos.y/10.0) and pos.y < 10) {
			return blocks->names["bark"];
		} else if (pos.y > 4 + hash4(parent->seed, offset.x, offset.y, 0, 48935) % 4 and glm::length(vec2(pos.x-5,pos.z-5)) + (pos.y-6)/2 <
		3+randfloat(parent->seed, offset.x+pos.x, offset.y+pos.y, offset.z+pos.z, 2634928)) {
			if (hash4(parent->seed, offset.x, offset.y, offset.z, 244562) % 20 == 0) {
				return -1;
			}
			return blocks->names["leaves"];
		}
		return -1;
	}
	
	vec2 trunk_off(perlin(pos.y/3.0, offset.x ^ offset.z, parent->seed, 3453458)*2, perlin(pos.y/3.0, offset.x ^ offset.z, parent->seed, 2749245)*2);
	if (glm::length(vec2(pos.x-5, pos.z-5) - trunk_off) < 1.75 - (pos.y/10.0) and pos.y < 10) {
		return blocks->names["bark"];
	} else if (pos.y > 4 + hash4(parent->seed, offset.x, offset.y, 0, 48935) % 4 and glm::length(vec3(5,8,5)-vec3(pos)) <
	4 + randfloat(parent->seed, offset.x+pos.x, offset.y+pos.y, offset.z+pos.z, 37482)) {
		if (hash4(parent->seed, offset.x, offset.y, offset.z, 242362) % 20 == 0) {
			return -1;
		}
		return blocks->names["leaves"];
	}
	return -1;
}

ivec3 Tree::get_nearest(ivec3 pos) {
	ivec2 pos2d = get_nearest_2d(ivec2(pos.x, pos.z));
	return ivec3(pos2d.x, parent->get_height(pos2d+5+ivec2(pos.x, pos.z)) - pos.y, pos2d.y);
}

int Tree::priority() {
	return 5;
}

bool Tree::is_valid(ivec3 pos) {
	return true;
}




TallTree::TallTree(TerrainObjectMerger* merger, int uid): TerrainObject(merger->parent, ivec3(20,40,20), 90, uid) {
	
}

char TallTree::gen_func(ivec3 offset, ivec3 pos) {
	double center_dist = glm::length(vec2(pos.x-10,pos.z-10));
	int start_leaves = 10 + hash4(parent->seed, offset.x, offset.y, offset.z, 45356) % 10;
	double leaves_frill = 2*randfloat(parent->seed, offset.x+pos.x, offset.y+pos.y, offset.z+pos.z, 2634928);
	
	if (center_dist < 4 - pos.y/40.0 * 4) {
		return blocks->names["bark"];
	} else if (pos.y > start_leaves and center_dist + (pos.y-start_leaves)/3 < 8 + leaves_frill) {
		return blocks->names["leaves"];
	}
	return -1;
}

ivec3 TallTree::get_nearest(ivec3 pos) {
	ivec2 pos2d = get_nearest_2d(ivec2(pos.x, pos.z));
	return ivec3(pos2d.x, parent->get_height(pos2d+5+ivec2(pos.x, pos.z)) - pos.y, pos2d.y);
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
	ivec2 pos2d = get_nearest_2d(ivec2(pos.x, pos.z));
	return ivec3(pos2d.x, parent->get_height(pos2d+25+ivec2(pos.x, pos.z)) - pos.y - 20, pos2d.y);
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
	ivec2 pos2d = get_nearest_2d(ivec2(pos.x, pos.z));
	return ivec3(pos2d.x, parent->get_height(pos2d+5+ivec2(pos.x, pos.z)) - pos.y - 5, pos2d.y);
}

int Cave::priority() {
	return 1;
}

bool Cave::is_valid(ivec3 pos) {
	return true;
}





FileTerrain::FileTerrain(TerrainObjectMerger* merger, string path, int uid): TerrainObject(merger->parent, ivec3(0,0,0), 0, uid) {
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
	ivec2 pos2d = get_nearest_2d(ivec2(pos.x, pos.z));
	return ivec3(pos2d.x, parent->get_height(pos2d+ivec2(size.x/2,size.z/2)+ivec2(pos.x, pos.z)) - pos.y - size.y/2, pos2d.y);
}

int FileTerrain::priority() {
	return priority_level;
}

bool FileTerrain::is_valid(ivec3 pos) {
	return biomes.count(parent->get_biome_name(pos)) > 0;
}

#endif
