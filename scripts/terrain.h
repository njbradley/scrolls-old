#ifndef TERRAIN
#define TERRAIN

#include "terrain-predef.h"
#include "blockdata-predef.h"
#include "generative.h"


int Island::get_height(int x, int y) {
	pair<int,int> bounds = get_bounds(x, y);
	if (bounds.first > bounds.second) {
		return bounds.first;
	} else {
		return -1;
	}
}

pair<int,int> Island::get_bounds(int x, int y) {
  int ix = x-128;
  int iy = y-128;
  int top = get_heightmap(x, y)-(ix*ix+iy*iy)*0.004+height;
  int bottom = (scaled_perlin(x,y,32,80) + scaled_perlin(x,y,16,20) + scaled_perlin(x,y,4,5));// + (ix*ix+iy*iy)*0.01;
  bottom = bottom * bottom/20;
  bottom = -bottom + (ix*ix+iy*iy)*0.01 + 120;
  return pair<int,int>(top, bottom);
}

char Island::gen_func(int x, int y, int z) {
	set_seed(seed);
	pair<int,int> island = get_bounds(x, z);
	double height = island.first;
	double base = island.second;
	
	if (y > base and y < height-5) {
			return blocks->names["rock"];
	} if (y < height-1 and y > base) {
			return blocks->names["dirt"];
	} if (y < height and y > base) {
			return blocks->names["grass"];
	}
	return -1;
}

int Island::priority() {
	return 1;
}






Trees::Trees(ChunkLoader* chunk, int nseed, string ntype): TerrainObject(chunk,nseed), type(ntype) {
	for (int i = 0; i < 20; i ++) {
		set_seed(seed+i*2);
		int x = randfloat(parent->xpos, parent->ypos) * 255;
		set_seed(seed+i*2+1);
		int y = randfloat(parent->xpos, parent->ypos) * 255;
		if (parent->get_height(x, y) != -1) {
			trees.push_back(pair<int,int>(x,y));
		}
	}
}

char Trees::gen_func(int x, int y, int z) {
	for (pair<int,int> pos : trees) {
		int height = parent->get_height(pos.first, pos.second);
		if (x == pos.first and y > height and y < height + 10 and z == pos.second) {
			return blocks->names["wood"];
		}
	}
	return -1;
}

int Trees::priority() {
	return 2;
}



ChunkLoader::ChunkLoader(int nseed, int x, int y): seed(nseed), xpos(x), ypos(y) {
	terrain.push_back(new Island(this, seed, 0, 0, 100, 100));
	//objs.push_back(new Trees(this, seed, "oak"));
}

int ChunkLoader::get_height(int x, int y) {
	int height = -1;
	for (TerrainBase* base : terrain) {
		if (base->get_height(x,y)) {
			height = base->get_height(x,y);
		}
	}
	return height;
}

char ChunkLoader::gen_func(int x, int y, int z) {
	char val = 0;
	int priority = 0;
	for (TerrainBase* base : terrain) {
		char newval = base->gen_func(x,y,z);
		if (newval != -1 and base->priority() > priority) {
			val = newval;
			priority = base->priority();
		}
	}
	for (TerrainObject* obj : objs) {
		char newval = obj->gen_func(x,y,z);
		if (newval != -1 and obj->priority() > priority) {
			val = newval;
			priority = obj->priority();
		}
	}
	return val;
}


#endif
