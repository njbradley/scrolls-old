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
	if (pix->group != nullptr and !can_take(pix)) {
		return false;
	}
	if (size == 0 and uniform_type) {
		blocktype = pix->value;
	}
	if ((uniform_type and pix->value == blocktype) or !uniform_type) {
		if (pix->group != nullptr) {
			pix->group->del(pix);
		}
		pix->group = this;
		size += pix->scale * pix->scale * pix->scale;
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
							if (sidepix->group->can_take(pix)) {
								cout << "blocktypes " << int(blocktype) << ' ' << int(sidepix->group->blocktype) << endl;
								cout << "would let take over " << this << '.' << size << ' '  << ' ' << pix << ' ' << ' ' << sidepix->group << '.' << sidepix->group->size << endl;
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
		size -= pix->scale * pix->scale * pix->scale;
		return true;
	}
	return false;
}

bool BlockGroup::can_take(Pixel* pix) {
	return !final and pix->group != nullptr and pix->group != this
	and !pix->group->final and pix->group->blocktype == blocktype
	and pix->group->size <= size;
}

bool BlockGroup::contains(Pixel* pix) {
	return pix->group == this;
}

#endif
