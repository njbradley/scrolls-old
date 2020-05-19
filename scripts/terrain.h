#ifndef TERRAIN
#define TERRAIN

#include "terrain-predef.h"
#include "blockdata-predef.h"
#include "generative.h"

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
	objs = vector<TerrainObject*> {new Tree(this), new Cave(this)};
}

char TerrainObjectMerger::gen_func(ivec3 pos) {
	char val = -1;
	int priority = 0;
	for (TerrainObject* obj : objs) {
		ivec3 nearest = obj->get_nearest(pos);
		if (obj->priority() > priority and nearest.x <= 0 and nearest.y <= 0 and nearest.z <= 0
			and nearest.x > -obj->size.x and nearest.y > -obj->size.y and nearest.z > -obj->size.z ) {
			char newval = obj->gen_func(-nearest);
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
	return int(get_heightmap(pos.x, pos.y, parent->seed));
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

char Tree::gen_func(ivec3 pos) {
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




Cave::Cave(TerrainObjectMerger* merger): TerrainObject(merger->parent, ivec3(10,10,10), 20) {
	
}

char Cave::gen_func(ivec3 pos) {
	return (glm::length(vec3(5,5,5)-vec3(pos)) < 4) - 1;
}

ivec3 Cave::get_nearest(ivec3 pos) {
	ivec2 pos2d = get_nearest_2d(ivec2(pos.x, pos.z));
	return ivec3(pos2d.x, parent->terrain->get_height(pos2d+5+ivec2(pos.x, pos.z)) - pos.y - 5, pos2d.y);
}

int Cave::priority() {
	return 1;
}

#endif
