#ifndef BLOCKGROUPS
#define BLOCKGROUPS

#include "blockgroups.h"

#include "blocks.h"
#include "world.h"
#include "blockdata.h"
#include "menu.h"

BlockGroup::BlockGroup(World* nworld): world(nworld) {
	
}

BlockGroup::BlockGroup(World* nworld, vector<Pixel*> pixels): world(nworld), final(true) {
	for (Pixel* pix : pixels) {
		add(pix);
	}
}

BlockGroup::~BlockGroup() {
	
}

bool BlockGroup::add(Pixel* pix) {
	if (final) {
		return false;
	}
	if (pix->group != nullptr) {
		if (!pix->group->final and !final and pix->group->blocktype == blocktype) {
			if (pix->group->size > size) {
				return false;
			}
		}
	}
	if (size == 0 and uniform_type) {
		blocktype = pix->value;
	}
	if ((uniform_type and pix->value == blocktype) or !uniform_type) {
		if (pix->group != nullptr) {
			pix->group->del(pix);
		}
		pix->group = this;
		size ++;
		return true;
	}
	return false;
}

void BlockGroup::spread(Pixel* pix, int depth) {
	if (depth > 0 and add(pix)) {
		ivec3 pos;
		pix->global_position(&pos.x, &pos.y, &pos.z);
		int i = 0;
		for (ivec3 dir : dir_array) {
			ivec3 sidepos = dir*pix->scale + pos;
			Block* sideblock = world->get_global(sidepos.x, sidepos.y, sidepos.z, pix->scale);
			if (sideblock != nullptr) {
				for (Pixel* sidepix : sideblock->iter()) {
					spread(sidepix, depth-1);
					if (!contains(sidepix)) {
						if (sidepix->group != nullptr) {
							if (!final and !sidepix->group->final and sidepix->group->blocktype == blocktype) {
								cout << "would let take over " << endl;
								del(pix);
								sidepix->group->spread(pix);
								return;
							}
						}
						if (sidepix->value != 0) {
							consts[i] = true;
						}
					}
					if (!contains(pix)) {
						cout << "ive been taken over " << endl;
						return;
					}
				}
			}
			i ++;
		}
	}
}

bool BlockGroup::del(Pixel* pix) {
	if (contains(pix)) {
		pix->group = nullptr;
		size --;
		return true;
	}
	return false;
}

bool BlockGroup::contains(Pixel* pix) {
	return pix->group == this;
}

#endif
