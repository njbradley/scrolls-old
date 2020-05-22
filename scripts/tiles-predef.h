#ifndef TILES_PREDEF
#define TILES_PREDEF

#include "classes.h"
#include <vector>
#include "terrain-predef.h"
#include <mutex>


class Tile {
	public:
		ivec3 pos;
		int height;
		Block* chunk;
		int chunksize;
		World* world;
		std::timed_mutex writelock;
		bool deleting;
		vector<DisplayEntity*> entities;
		Tile(ivec3 position, World* world);
		void timestep();
		void render(GLVecs*);
		void save();
		void update_lighting();
		Block* generate(ivec3 pos);
		void del(bool remove_faces);
};

#endif
