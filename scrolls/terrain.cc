#include "terrain.h"
#include "world.h"
#include "blockdata.h"
#include "blocks.h"
#include "fileformat.h"





TerrainLoader::TerrainLoader(int nseed): seed(nseed), terrain(seed), objects(seed) {
	
}

void TerrainLoader::set_seed(int newseed) {
	seed = newseed;
	terrain->seed = seed;
	for (TerrainObject* obj : objects) {
		obj->seed = seed;
	}
}

int TerrainLoader::get_height(ivec2 pos) {
	return terrain->get_height(ivec3(pos.x, 0, pos.y));
}

int TerrainLoader::get_height(ivec3 pos) {
	return terrain->get_height(pos);
}

Block* TerrainLoader::generate_chunk(ivec3 pos) {
	Block* block = terrain->generate_chunk(pos);
	for (TerrainObject* obj : objects) {
		obj->place_object(pos, block);
	}
	return block;
}



DEFINE_PLUGIN(TerrainGenerator);

TerrainGenerator::TerrainGenerator(int newseed): seed(newseed) {
	
}

DEFINE_PLUGIN(TerrainObject);

TerrainObject::TerrainObject(int newseed): seed(newseed) {
	
}
