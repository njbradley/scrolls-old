#ifndef TILES_PREDEF
#define TILES_PREDEF

#include "classes.h"
#include <vector>
#include "terrain-predef.h"


class Tile {
	public:
		ivec3 pos;
		int height;
		Block* chunk;
		int chunksize;
		World* world;
		Tile(ivec3 position, World* world);
		void render(GLVecs*);
		void save();
		void update_lighting();
		Block* generate(ivec3 pos);
		void del(bool remove_faces);
};

#endif
