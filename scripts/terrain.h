#ifndef TERRAIN
#define TERRAIN

#include "terrain-predef.h"
#include "blockdata-predef.h"
#include "generative.h"
#include <math.h>

const int chunksize = World::chunksize;


TerrainObject::TerrainObject(TerrainLoader* loader, ivec3 nsize, int nradius): parent(loader), size(nsize), radius(nradius) {
	
}

ivec3 TerrainObject::get_nearest(ivec3 pos) {
	return get_nearest_3d(pos);
}

ivec3 TerrainObject::get_nearest_3d(ivec3 pos) {
	ivec3 gap = radius - size;
	ivec3 rounded = pos/radius - ivec3(pos.x<0, pos.y<0, pos.z<0);
	int layer = priority();
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
	int layer = priority();
	ivec2 offset = ivec2(hash4(parent->seed, rounded.x,rounded.y,layer,1), hash4(parent->seed, rounded.x,rounded.y,layer,2)) % gap;
	ivec2 nearest = offset - (pos%radius + ivec2(pos.x<0, pos.y<0)*radius);
	return nearest;
}









TerrainObjectMerger::TerrainObjectMerger(TerrainLoader* newparent): parent(newparent) {
	objs = vector<TerrainObject*> {new Tree(this), new Cave(this), new BigTree(this)};
	vector<string> files;
	get_files_folder("resources/data/terrain", &files);
	for (string file : files) {
		objs.push_back(new FileTerrain(this, "resources/data/terrain/" + file));
	}
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
		if (obj->priority() > priority and nearest.x <= 0 and nearest.y <= 0 and nearest.z <= 0
			and nearest.x > -obj->size.x and nearest.y > -obj->size.y and nearest.z > -obj->size.z ) {
			char newval = obj->gen_func(pos + nearest, -nearest);
			if (newval != -1) {
				val = newval;
				priority = obj->priority();
			}
		}
	}
	return val;
}


TerrainLoader::TerrainLoader(int nseed): objmerger(this), seed(nseed) {
	terrain = new Land(this);
}

char TerrainLoader::gen_func(ivec3 pos) {
	char objval = objmerger.gen_func(pos);
	if (objval != -1) {
		return objval;
	} else {
		return terrain->gen_func(pos);
	}
}




Land::Land(TerrainLoader* loader): parent(loader) {
	
}

int Land::get_height(ivec2 pos) {
	return int(get_heightmap(pos.x, pos.y, parent->seed))/2;
}

char Land::gen_func(ivec3 pos) {
	int height = get_height(ivec2(pos.x, pos.z));
	if (pos.y < height-5) {
			return blocks->names["rock"];
	} if (pos.y < height-1) {
			return blocks->names["dirt"];
	} if (pos.y < height) {
			if (pos.y > 80) {
				return blocks->names["snow"];
			} else {
				return blocks->names["grass"];
			}
	}
	return 0;
}



Tree::Tree(TerrainObjectMerger* merger): TerrainObject(merger->parent, ivec3(10,10,10), 20) {
	
}

char Tree::gen_func(ivec3 offset, ivec3 pos) {
	if (pos.x == 5 and pos.z == 5 and pos.y < 7) {
		return blocks->names["bark"];
	} else if (pos.y > 4 and glm::length(vec3(5,5,5)-vec3(pos)) < 4) {
		return blocks->names["leaves"];
	}
	return -1;
}

ivec3 Tree::get_nearest(ivec3 pos) {
	ivec2 pos2d = get_nearest_2d(ivec2(pos.x, pos.z));
	return ivec3(pos2d.x, parent->terrain->get_height(pos2d+5+ivec2(pos.x, pos.z)) - pos.y, pos2d.y);
}

int Tree::priority() {
	return 5;
}




BigTree::BigTree(TerrainObjectMerger* merger): TerrainObject(merger->parent, ivec3(50,100,50), 100) {
	
}

char BigTree::gen_func(ivec3 offset, ivec3 pos) {
	
	pos.y -= (parent->terrain->get_height(ivec2(offset.x+pos.x, offset.z+pos.z)) - offset.y)*(1 - pos.y/200.0);
	
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
	return ivec3(pos2d.x, parent->terrain->get_height(pos2d+25+ivec2(pos.x, pos.z)) - pos.y - 20, pos2d.y);
}

int BigTree::priority() {
	return 11;
}





Cave::Cave(TerrainObjectMerger* merger): TerrainObject(merger->parent, ivec3(10,10,10), 20) {
	
}

char Cave::gen_func(ivec3 offset, ivec3 pos) {
	return (glm::length(vec3(5,5,5)-vec3(pos)) < 4) - 1;
}

ivec3 Cave::get_nearest(ivec3 pos) {
	ivec2 pos2d = get_nearest_2d(ivec2(pos.x, pos.z));
	return ivec3(pos2d.x, parent->terrain->get_height(pos2d+5+ivec2(pos.x, pos.z)) - pos.y - 5, pos2d.y);
}

int Cave::priority() {
	return 1;
}





FileTerrain::FileTerrain(TerrainObjectMerger* merger, string path): TerrainObject(merger->parent, ivec3(0,0,0), 0) {
	ifstream ifile(path);
	cout << "loading terrain object " << path << endl;
	int scale;
	bool ignore_air;
	ifile >> scale;
	size = ivec3(scale, scale, scale);
	ifile >> radius;
	ifile >> priority_level;
	ifile >> ignore_air;
	cout << scale << ' ' << radius << ' ' << priority_level << ' ' << ignore_air << endl;
	string buff;
	getline(ifile, buff, ':');
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
	return ivec3(pos2d.x, parent->terrain->get_height(pos2d+ivec2(size.x/2,size.z/2)+ivec2(pos.x, pos.z)) - pos.y - size.y/2, pos2d.y);
}

int FileTerrain::priority() {
	return priority_level;
}

#endif
