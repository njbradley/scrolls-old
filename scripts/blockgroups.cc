#ifndef BLOCKGROUPS
#define BLOCKGROUPS

#include "blockgroups.h"

#include "blocks.h"
#include "world.h"
#include "blockdata.h"
#include "menu.h"
#include "materials.h"
#include "glue.h"

GroupConnection::GroupConnection(BlockGroup* newgroup, ConnectorData* mat): group(newgroup), data(mat) {
	
}

GroupConnection::GroupConnection(int index, ConnectorData* mat): globalindex(index), data(mat) {
	
}


BlockGroup::BlockGroup(World* nworld): world(nworld) {
	id = rand();
}

BlockGroup::BlockGroup(World* nworld, vector<Pixel*> pixels): world(nworld) {
	
	id = rand();
	for (Pixel* pix : pixels) {
		//add(pix, nullptr, ivec3(0,0,0));
		ivec3 pos;
		pix->global_position(&pos.x, &pos.y, &pos.z);
		int i = 0;
		for (ivec3 dir : dir_array) {
			ivec3 sidepos = pos + dir * pix->scale;
			Block* block = world->get_global(sidepos.x, sidepos.y, sidepos.z, pix->scale);
			if (block != nullptr) {
				if (block->get_pix() != nullptr) {
					Pixel* sidepix = block->get_pix();
					if (sidepix->value != 0 and std::find(pixels.begin(), pixels.end(), sidepix) == pixels.end()) {
						consts[i] += pix->scale * pix->scale;
						
						if (blocks->clumpy(sidepix->value, pix->value)) {
							if (sidepix->scale == pix->scale) {
								sidepix->joints[(i+3)%6] = 7;
							}
							pix->joints[i] = 7;
							pix->render_update();
							sidepix->render_update();
						}
					}
				} else {
					for (Pixel* sidepix : block->iter_side(-dir)) {
						if (sidepix->value != 0 and std::find(pixels.begin(), pixels.end(), sidepix) == pixels.end()) {
							int num_touching = std::min(pix->scale, sidepix->scale);
							consts[i] += num_touching * num_touching;
							
							if (blocks->clumpy(sidepix->value, pix->value)) {
								sidepix->joints[(i+3)%6] = 7;
								pix->render_update();
								sidepix->render_update();
							}
						}
					}
				}
			}
			i ++;
		}
	}
	
}

BlockGroup::BlockGroup(World* nworld, istream& ifile): world(nworld) {
	ifile >> id;
	ifile >> size >> isolated;
	for (int i = 0; i < 6; i ++) {
		ifile >> consts[i];
	}
}

void BlockGroup::to_file(ostream& ofile, vector<BlockGroup*>* allgroups) {
	ofile << size << ' ' << isolated << ' ';
	for (int val : consts) {
		ofile << val << ' ';
	}
	ofile << endl;
}

BlockGroup::~BlockGroup() {
	
}

bool BlockGroup::add(Pixel* pix, Pixel* source, ivec3 dir) {
	if (pix->value == 0 or contains(pix)) {
		return false;
	}
	
	int index = dir_to_index(dir);
	
	bool do_add = false;
	
	if (source != nullptr) {
		if (source->joints[index] < 7) {
			if (pix->value == source->value or blocks->clumpy(pix->value, source->value)) {
				do_add = true;
			}
		} else if (source->joints[index] > 7) {
			do_add = true;
		}
	} else {
		do_add = true;
	}
	
	if (do_add) {
		if (pix->group != nullptr) {
			pix->group->del(pix);
		}
		pix->group = this;
		size += pix->scale * pix->scale * pix->scale;
		//cout << pix << ' ' << int(pix->value) << endl;
		return true;
	} else {
		//cout << "possible set const, " << (pix->value == 0) << (source == nullptr) << endl;
		if (pix->value != 0 and source != nullptr) {
			int num_touching = std::min(pix->scale, source->scale);
			consts[index] += num_touching * num_touching;
			//cout << "setting cönsts in ädd "<< endl;
		}
		return false;
	}
}

void BlockGroup::spread(Pixel* start_pix, Pixel* source, ivec3 pastdir, int depth) {
  unordered_set<Pixel*> last_pixels;
  unordered_set<Pixel*> new_pixels;
  
	if (!add(start_pix, nullptr, ivec3(0,0,0))) {
		return;
	}
	
  last_pixels.emplace(start_pix);
  
	bool over_limit = false;
	
  while (last_pixels.size() > 0 and !over_limit) {
		over_limit = size > max_size;
    for (Pixel* pix : last_pixels) {
			int i = 0;
      for (ivec3 dir : dir_array) {
				BlockIter iter = pix->iter_touching_side(dir, world);
				
				if (iter.begin() != iter.end()) {
					for (Pixel* sidepix : iter) {
						if (over_limit) {
							if (!contains(sidepix) and sidepix->value != 0) {
								isolated = false;
								int num_touching = std::min(pix->scale, sidepix->scale);
								consts[i] += num_touching * num_touching;
							}
						} else if (!contains(sidepix) and add(sidepix, pix, dir)) {
							new_pixels.emplace(sidepix);
						}
					}
				} else {
					consts[i] += pix->scale * pix->scale;
				}
				i++;
      }
    }
    
    last_pixels.swap(new_pixels);
    new_pixels.clear();
  }
}
// 	if (depth > 0 and add(pix, source, pastdir)) {
// 		ivec3 pos;
// 		pix->global_position(&pos.x, &pos.y, &pos.z);
// 		int i = 0;
// 		for (ivec3 dir : dir_array) {
// 			BlockIter iter = pix->iter_touching_side(dir, world);
// 			if (iter.begin() != iter.end()) {
// 				for (Pixel* sidepix : iter) {
// 					spread(sidepix, pix, dir, depth-1);
// 				}
// 			} else {
// 				consts[i] += pix->scale * pix->scale;
// 				//cout << "setting cönsts in sprëad "<< endl;
// 			}
// 			i ++;
// 		}
// 	} else if (depth <= 0) {
// 		if (pix->value != 0) {
// 			isolated = false;
// 			consts[dir_to_index(pastdir)] += pix->scale * pix->scale;
// 		}
// 	}
// }

bool BlockGroup::del(Pixel* pix) {
	if (contains(pix)) {
		pix->group = nullptr;
		size -= pix->scale * pix->scale * pix->scale;
		if (size == 0) {
			delete this;
		}
		return true;
	}
	return false;
}

bool BlockGroup::contains(Pixel* pix) {
	return pix->group == this;
}

#endif
