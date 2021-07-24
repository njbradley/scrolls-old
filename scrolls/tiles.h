#ifndef TILES_PREDEF
#define TILES_PREDEF

#include "classes.h"
#include "collider.h"

#include <vector>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

class Tile : public Container {
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
		FreeBlock* allfreeblocks = nullptr;
		
		Tile(ivec3 position, World* world, istream& ifile);
		Tile(ivec3 position, World* world);
		~Tile();
		void timestep(float deltatime);
		void drop_ticks();
		void render(RenderVecs* vecs, RenderVecs* transvecs);
		void save();
		void update_lighting();
		void lighting_update();
		void generate_chunk(ivec3 pos);
		char gen_block(ostream& ofile, int gx, int gy, int gz, int scale);
		void fix_side_groups(std::unordered_map<int,BlockGroup*>& sidegroups);
		void get_side_groups(std::unordered_map<int,BlockGroup*>& sidegroups, Tile* tile, ivec3 dir);
		
		Block* get_global(int x,int y,int z,int scale);
		vec3 get_position() const;
		void block_update(ivec3 pos);
		void set_global(ivec3 pos, int w, int val, int direc, int joints[6] = nullptr);
		void add_freeblock(FreeBlock* freeblock);
		void remove_freeblock(FreeBlock* freeblock);
};

#endif