#ifndef TILES_PREDEF
#define TILES_PREDEF

#include "classes.h"

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

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
		std::mutex changelock;
		bool deleting;
		bool lightflag = true;
		bool fully_loaded = false;
		bool done_reading = false;
		Tile(ivec3 position, World* world, istream& ifile);
		Tile(ivec3 position, World* world);
		~Tile();
		void timestep();
		void drop_ticks();
		void render(RenderVecs* vecs, RenderVecs* transvecs);
		void save();
		void update_lighting();
		void lighting_update();
		void generate_chunk(ivec3 pos);
		char gen_block(ostream& ofile, int gx, int gy, int gz, int scale);
		void fix_side_groups(std::unordered_map<int,BlockGroup*>& sidegroups);
		void get_side_groups(std::unordered_map<int,BlockGroup*>& sidegroups, Tile* tile, ivec3 dir);
};

#endif
