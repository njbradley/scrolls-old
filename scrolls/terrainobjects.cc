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
					place_object(chunkpos, block->get(x,1,z), block->get(x,0,z));
				}
			}
		}
	}
	
	void place_object(ivec3 chunkpos, Block* above, Block* below) {
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
						place_object(chunkpos, above->get(x,1,z), above->get(x,0,z));
					}
					if (below->continues) {
						place_object(chunkpos, below->get(x,1,z), below->get(x,0,z));
					}
					if (above->continues and below->continues) {
						place_object(chunkpos, above->get(x,0,z), below->get(x,1,z));
					}
				}
			}
		} else {
			if (hash4(seed, above->globalpos, 0) % rarity == hash4(seed, below->globalpos, 0) % rarity) {
				FreeBlock* newobj = create_object(above->globalpos);
				Block* destblock = above;
				vec3 objpos = above->globalpos;
				
				while (destblock->scale < newobj->scale * csize) {
					destblock = destblock->parent;
				}
				while (destblock->scale > newobj->scale * csize) {
					destblock->subdivide();
					destblock = destblock->get(0,0,0);
				}
				
				Movingbox newbox = newobj->box;
				newbox.position = objpos;
				
				destblock->add_freechild(newobj);
				cout << newbox << endl;
				newobj->set_box(newbox);
				cout << newobj->highparent << ' ' << newobj->highparent->globalpos << endl;
				cout << newbox << endl;
			}
		}
	}
	
	virtual FreeBlock* create_object(ivec3 pos) = 0;
};




struct BareTree : SurfaceObject<1000> {
	PLUGIN_HEAD(BareTree);
	
	using SurfaceObject<1000>::SurfaceObject;
	
	virtual FreeBlock* create_object(ivec3 pos) {
		cout << "creating object at " << pos << endl;
		quat rot = glm::angleAxis(randfloat(seed, pos.x, pos.y, pos.z, 0), vec3(0,1,0));
		FreeBlock* free = new FreeBlock(Hitbox(vec3(0,0,0), vec3(-2,0,-2), vec3(2,4,2), rot));
		
		free->set_parent(nullptr, nullptr, ivec3(0,0,0), 4);
		
		free->set_global(ivec3(0,0,0), 1, blocktypes::wood.id, 1);
		free->set_global(ivec3(0,1,0), 1, blocktypes::wood.id, 1);
		free->set_global(ivec3(0,2,0), 1, blocktypes::wood.id, 1);
		free->set_global(ivec3(0,3,0), 1, blocktypes::wood.id, 1);
		
		return free;
	}
};

EXPORT_PLUGIN(BareTree);
		
