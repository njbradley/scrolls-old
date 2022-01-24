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
		place_object_at(chunkpos, chunk, vec3(pos) + vec3(0.5f, 0, 0.5f), quat(1,0,0,0), 8);
	}
	
	void place_object_at(ivec3 chunkpos, Block* chunk, vec3 pos, quat rot, int len) {
		if (len <= 3) return;
		
		// vec3 up = rot * vec3(0,1,0);
		// vec3 rot_axis = glm::normalize(glm::cross(up, randvec3(seed, pos.x, pos.y, pos.z, 1)));
		vec3 dir_array[4] = {
			{1,0,0}, {-1,0,0}, {0,0,1}, {0,0,-1}
		};
		vec3 rot_axis = vec3(dir_array[hash4(seed, ivec3(pos), 8193)%4]);
		float rot_amount = (3.1415f/4) * (hash4(seed, ivec3(pos), 82374) % 2);
		rot = rot * glm::angleAxis(rot_amount, rot_axis);
		
		// rot = rot * glm::angleAxis(randfloat(seed, pos.x, pos.y, pos.z, 0) - 0.5f, glm::normalize(randvec3(seed, pos.x, pos.y, pos.z, 1)));
		FreeBlock* free = new FreeBlock(Hitbox(pos, vec3(-0.5f,-0.5,-0.5f), vec3(3.5f,3.5,3.5f), rot));
		free->fix();
		
		free->set_parent(nullptr, nullptr, ivec3(0,0,0), 1);
		
		for (int i = 0; i < len; i ++) {
			free->set_global(ivec3(0,i,0), 1, blocktypes::wood.id, 1);
			
			
			if (hash4(seed, ivec3(pos), 121+i)%(10) == 0) {
			// while (i < len and hash4(seed, ivec3(pos), 121 + i)%3 != 0) {
				place_object_at(chunkpos, chunk, free->box.transform_out(vec3(0.5f,i-0.5f,0.5f)), rot, len*3/4);
			}
			
			// if (hash4(seed, ivec3(pos), 3783+i)%5 == 0) {
			// 	free->set_global(ivec3(0,i,1), 1, blocktypes::leaves.id, 1);
			// }
			// if (hash4(seed, ivec3(pos), 3783+i)%5 == 0) {
			// 	free->set_global(ivec3(1,i,0), 1, blocktypes::leaves.id, 1);
			// }
		}
		
		// branch
		// if (
		
		
		
		// int i = 0;
		if (hash4(seed, ivec3(pos), 121)%len != 0) {
		// while (i < len and hash4(seed, ivec3(pos), 121 + i)%3 != 0) {
			place_object_at(chunkpos, chunk, free->box.transform_out(vec3(0.5f,len-0.5f,0.5f)), rot, len-1);
			// i ++;
			// if (false and hash4(seed, ivec3(pos), 156)%(len/2) == 0) {
			// 	vec3 newrot_axis = glm::normalize(glm::cross(up, randvec3(seed, pos.x, pos.y, pos.z, 1)));
			// 	quat newrot = rot * glm::angleAxis(randfloat(seed, int(pos.x), int(pos.y), int(pos.z), 333) + 0.5f, newrot_axis);
			// 	place_object_at(chunkpos, chunk, free->box.transform_out(vec3(0.5f,len-0.5f,0.5f)), newrot, len-1);
			// }
		}
		
		place_object(chunkpos, chunk, free);
	}
	
	
};

EXPORT_PLUGIN(BareTree);
		
