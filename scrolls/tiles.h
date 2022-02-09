#ifndef TILES_PREDEF
#define TILES_PREDEF

#include "classes.h"
#include "collider.h"
#include "plugins.h"

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
		
		Tile(ivec3 position, World* world);
		Tile(ivec3 position, World* world, Block* nchunk);
		~Tile();
		void tick(float curtime, float deltatime);
		void timestep(float curtime, float deltatime);
		void drop_ticks();
		void render(RenderVecs* vecs, RenderVecs* transvecs);
		void save();
		void update_lighting();
		void lighting_update();
		void set_chunk(Block* nchunk);
		
		Block* get_global(ivec3 pos, int scale);
		const Block* get_global(ivec3 pos, int scale) const;
		vec3 get_position() const;
		void block_update(ivec3 pos);
		void set_global(ivec3 pos, int w, Blocktype val, int direc, int joints[6] = nullptr);
};

class TileLoader { public:
	BASE_PLUGIN_HEAD(TileLoader, (World*));
	
	bool do_lighting_update = true;
	
	virtual void begin_serving() = 0;
	
	virtual void request_load_tile(ivec3 pos) = 0;
	virtual void request_del_tile(ivec3 pos) = 0;
	
	virtual void end_serving() = 0;
};

#endif
