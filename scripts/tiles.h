#ifndef TILES_PREDEF
#define TILES_PREDEF

#include "classes.h"

#include <vector>
#include <mutex>
#include <shared_mutex>

class Tile {
	public:
		ivec3 pos;
		int height;
		Block* chunk;
		int chunksize;
		World* world;
		int render_depth = 1;
		bool render_faces[6] = {true,true,true,true,true,true};
		bool optimized_render = false;
		std::shared_timed_mutex deletelock;
		bool deleting;
		bool lightflag = true;
		bool fully_loaded = false;
		vector<DisplayEntity*> entities;
		vector<FallingBlockEntity*> block_entities;
		Tile(ivec3 position, World* world);
		void timestep();
		void drop_ticks();
		void render(GLVecs* vecs, GLVecs* transvecs);
		void save();
		void update_lighting();
		void lighting_update();
		Block* generate(ivec3 pos);
		void del(bool remove_faces);
		void generate_chunk(ivec3 pos);
		char gen_block(ostream& ofile, int gx, int gy, int gz, int scale);
};

#endif
