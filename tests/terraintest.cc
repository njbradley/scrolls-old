#include "scrolls/tests.h"

#include "scrolls/blocks.h"
#include "scrolls/terrain.h"

struct TerrainTest : Test {
	PLUGIN_HEAD(TerrainTest);
	
	TerrainTest() {
		name = "terraintest";
	}
	
	void speed_tests() {
		TerrainLoader loader (12345);
		
		double start = getTime();
		Block* chunk = loader.generate_chunk(ivec3(0,loader.get_height(ivec2(0,0)),0));
		double time = getTime() - start;
		out << "chunk time " << time << endl;
		
	}
	
	void test() {
		speed_tests();
	}
};

EXPORT_PLUGIN(TerrainTest);
