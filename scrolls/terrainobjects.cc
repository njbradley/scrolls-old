#include "terrainobjects.h"

#include "blocks.h"
#include "blockdata.h"
#include "world.h"

template <int rarity>
struct SurfaceObject : public TerrainObject {
	using TerrainObject::TerrainObject;
	
	virtual void place_object(ivec3 chunkpos, Block* block) {
		if (block->continues) {
			for (int x = 0; x < csize; x ++) {
				for (int z = 0; z < csize; z ++) {
					find_surface(chunkpos, block, block->get(x,1,z), block->get(x,0,z));
				}
			}
		}
	}
	
	void find_surface(ivec3 chunkpos, Block* chunk, Block* above, Block* below) {
		if (!above->continues and above->pixel->value != 0) {
			return;
		}
		if (!below->continues and below->pixel->value == 0) {
			return;
		}
		
		if (above->continues or below->continues) {
			for (int x = 0; x < csize; x ++) {
				for (int z = 0; z < csize; z ++) {
					if (above->continues) {
						find_surface(chunkpos, chunk, above->get(x,1,z), above->get(x,0,z));
					}
					if (below->continues) {
						find_surface(chunkpos, chunk, below->get(x,1,z), below->get(x,0,z));
					}
					// if (above->continues and below->continues) {
					// 	place_object(chunkpos, above->get(x,0,z), below->get(x,1,z));
					// }
				}
			}
		} else {
			if (hash4(seed, above->globalpos, 0) % rarity == hash4(seed, below->globalpos, 0) % rarity) {
				place_object_at(chunkpos, chunk, above->globalpos);
			}
		}
	}
	
	void place_object(ivec3 chunkpos, Block* chunk, FreeBlock* newobj) {
		Block* destblock = chunk->get_global(SAFEFLOOR3(newobj->box.position), newobj->scale*csize);
		if (destblock == nullptr) return;
		
		while (destblock->scale > newobj->scale * csize) {
			destblock->subdivide();
			destblock = destblock->get(0,0,0);
		}
		
		destblock->add_freechild(newobj);
		if (!newobj->set_box(newobj->box)) {
			destblock->remove_freechild(newobj);
			delete newobj;
		}
	}
	
	// virtual FreeBlock* create_object(ivec3 pos) = 0;
	virtual void place_object_at(ivec3 chunkpos, Block* chunk, ivec3 pos) = 0;
};




struct BareTree : SurfaceObject<1000> {
	PLUGIN_HEAD(BareTree);
	
	using SurfaceObject<1000>::SurfaceObject;
	
	virtual void place_object_at(ivec3 chunkpos, Block* chunk, ivec3 pos) {
		vector<vec3> leaf_poses;
		place_object_at(chunkpos, chunk, vec3(pos) + vec3(0.5f, 0, 0.5f), quat(1,0,0,0), 8, &leaf_poses);
		
		Hitbox leaf_box = Hitbox::boundingbox(vec3(0,0,0), &leaf_poses[0]
	}
	
	void place_object_at(ivec3 chunkpos, Block* chunk, vec3 pos, quat rot, int len, vector<vec3>* leaf_poses) {
		if (len <= 3) {
			leaf_poses->push_back(pos);
			return;
		}
		
		rand_gen generator = init_generator(seed, chunkpos, pos, rot, len);
		
		vec3 dir_array[4] = {
			{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}
		};
		vec3 rot_axis = vec3(dir_array[generator()%4]);
		float rot_amount = (3.1415f/4) * (generator() % 2);
		rot = rot * glm::angleAxis(rot_amount, rot_axis);
		
		FreeBlock* free = new FreeBlock(Hitbox(pos, vec3(-0.5f,-0.5,-0.5f), vec3(3.5f,3.5,3.5f), rot));
		free->fix();
		
		free->set_parent(nullptr, nullptr, ivec3(0,0,0), 1);
		
		for (int i = 0; i < len; i ++) {
			free->set_global(ivec3(0,i,0), 1, blocktypes::wood.id, 1);
			
			if (generator()%(10) == 0) {
				place_object_at(chunkpos, chunk, free->box.transform_out(vec3(0.5f,i-0.5f,0.5f)), rot, len*3/4, leaf_poses);
			}
		}
		
		if (generator()%len != 0) {
			place_object_at(chunkpos, chunk, free->box.transform_out(vec3(0.5f,len-0.5f,0.5f)), rot, len-1, leaf_poses);
		}
		
		place_object(chunkpos, chunk, free);
	}
	
	
};

EXPORT_PLUGIN(BareTree);
		
