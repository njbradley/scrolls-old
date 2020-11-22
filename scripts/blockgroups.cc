#ifndef BLOCKGROUPS
#define BLOCKGROUPS

#include "blockphysics.h"

#include "blocks.h"
#include "world.h"
#include "blockdata.h"
#include "menu.h"

using glm::ivec4;


BlockGroup::BlockGroup(World* nworld): world(nworld) {
	ivec4 i4(1,2,3,4);
	cout << "hi hi " << i4.x << ' ' << i4.y << ' ' << i4.z << ' ' << i4.w << endl;
}

void BlockGroup::spread(unordered_set<ivec4,ivec4_hash>& last_poses) {
	const ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	unordered_set<ivec4,ivec4_hash> new_poses;
	// for (ivec4 pos : last_poses) {
	// 	poses.emplace(pos);
	// }
	for (ivec4 pos : last_poses) {
		for (ivec3 dir : dir_array) {
			ivec4 offpos(pos.x + dir.x*pos.w, pos.y + dir.y*pos.w, pos.z + dir.z*pos.w, pos.w);
			Block* block = world->get_global(offpos.x, offpos.y, offpos.z, offpos.w);
			if (block != nullptr) {
				for (Pixel* pix : block->iter_side(-dir)) {
					ivec4 pixpos;
					pix->global_position(&pixpos.x, &pixpos.y, &pixpos.z);
					pixpos.w = pix->scale;
					if (!contains(pixpos) and new_poses.count(pixpos) == 0 and add(ivec3(pixpos))) {
						new_poses.emplace(pixpos);
					}
				}
			}
		}
	}
	cout << new_poses.size() << endl;
	last_poses.swap(new_poses);
}
	

bool BlockGroup::spread_to_fill(ivec3 startpos) {
	//cout << 19 << endl;
	if (!add(startpos)) {
		return false;
	}
	
	int start = clock();
	const ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	print(startpos);
	Block* startblock = world->get_global(startpos.x, startpos.y, startpos.z, 1);
	if (startblock == nullptr) {
		return false;
	}
	unordered_set<ivec4,ivec4_hash> last_poses;
	last_poses.emplace(ivec4(startpos.x, startpos.y, startpos.z, startblock->scale));
	
	while (last_poses.size() > 0) {
		spread(last_poses);
	}
	cout << "time: " << clock() - start << endl;
	return true;
}






UnlinkedGroup::UnlinkedGroup(World* nworld): BlockGroup(nworld) {
	
};

bool UnlinkedGroup::contains(ivec3 pos) {
	Block* block = world->get_global(pos.x, pos.y, pos.z, 1);
	return block != nullptr and block->get_pix()->group == this;
}

int UnlinkedGroup::getsize() {
	return size;
}

bool UnlinkedGroup::add(ivec3 pos) {
	//cout << "add ";
	Block* block = world->get_global(pos.x, pos.y, pos.z, 1);
	if (block != nullptr) {
		Pixel* pix = block->get_pix();
		if (size == 0) {
			blockval = pix->value;
			position = pos;
			bounds = ivec3(1,1,1);
			size = 1;
			pix->group = this;
			//cout << "first block " << endl;
			return true;
		} else if (pix->group != this and pix->value == blockval) {
			//cout << int(blockval) << ' ' << int(pix->value) << ' ' << pix << endl;
			if (pix->group != nullptr) {
				//cout << this << ' ' << pix->group << ' ';
				//print(pos);
				pix->group->del(pos);
			}
			pix->group = this;
			for (int i = 0; i < 3; i ++) {
				if (pos[i] < position[i]) {
					int diff = position[i] - pos[i];
					position[i] = pos[i];
					bounds[i] += diff;
					//cout << "expanded in the negative " << i << " dir" << endl;
				} else if (pos[i]+pix->scale-1 >= position[i]+bounds[i]) {
					//cout << bounds[i] << " to ";
					bounds[i] = pos[i] + pix->scale - position[i];
					//cout << bounds[i] << " expanded in the positive " << i << " dir " << endl;
				}
			}
			size ++;
			//cout << "added (not first block) " << endl;
			return true;
		}
	}
	return false;
}

bool UnlinkedGroup::del(ivec3 pos) {
	const ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	Block* block = world->get_global(pos.x, pos.y, pos.z, 1);
	if (block != nullptr) {
		Pixel* pix = block->get_pix();
		if (pix->group == this) {
			pix->group = nullptr;
			size --;
			if (size > 0) {
				vector<ivec4> side_pix;
				for (ivec3 dir : dir_array) {
					ivec3 sidepos = pos + dir * block->scale;
					Block* sideblock = world->get_global(sidepos.x, sidepos.y, sidepos.z, block->scale);
					if (sideblock != nullptr) {
						for (Pixel* pix : sideblock->iter_side(-dir)) {
							if (pix->group == this) {
								ivec4 gpos (0,0,0,pix->scale);
								pix->global_position(&gpos.x, &gpos.y, &gpos.z);
								side_pix.push_back(gpos);
							}
						}
					}
				}
				
				bool delete_arm = false;
				
				if (side_pix.size() > 1) {
					//cout << "side pix > 1" << endl;
					int num_goals = side_pix.size();
					vector<unordered_set<ivec4,ivec4_hash>> poses_array;
					vector<unordered_set<ivec4,ivec4_hash>> last_poses_array;
					vector<ivec4> goals;
					
					for (int i = 0; i < num_goals; i ++) {
						poses_array.emplace_back();
						last_poses_array.emplace_back();
						last_poses_array[i].emplace(side_pix[i]);
						poses_array[i].emplace(ivec4(pos, block->scale));
						goals.push_back(side_pix[i]);
					}
					
					//cout << "num arms " << num_goals << endl;
					
					while (poses_array.size() > 1) {
						for (int i = num_goals-1; i >= 0; i --) {
							//cout << "loop index " << i << endl;
							for (ivec4 newpos : last_poses_array[i]) {
								poses_array[i].emplace(newpos);
								//cout << ' ' << newpos.x << ' ' << newpos.y << ' ' << newpos.z << ' ' << newpos.w << endl;
							}
							
							unordered_set<ivec4,ivec4_hash> new_poses;
							
							for (ivec4 lastpos : last_poses_array[i]) {
								for (ivec3 dir : dir_array) {
									ivec4 offpos(lastpos.x + dir.x*lastpos.w, lastpos.y + dir.y*lastpos.w, lastpos.z + dir.z*lastpos.w, lastpos.w);
									Block* block = world->get_global(offpos.x, offpos.y, offpos.z, offpos.w);
									if (block != nullptr) {
										for (Pixel* pix : block->iter_side(-dir)) {
											ivec4 pixpos;
											pix->global_position(&pixpos.x, &pixpos.y, &pixpos.z);
											pixpos.w = pix->scale;
											if (poses_array[i].count(pixpos) == 0 and new_poses.count(pixpos) == 0 and contains(pixpos)) {
												new_poses.emplace(pixpos);
											}
										}
									}
								}
							}
							
							last_poses_array[i].swap(new_poses);
							
							if (poses_array.size() > 1) {
								if (last_poses_array[i].size() == 0) {
									//cout << "deleting arm " << i << endl;
									delete_arm = true;
									for (ivec4 posdel : last_poses_array[i]) {
										//cout << posdel.x << ' ' << posdel.y << ' ' << posdel.z << endl;
										world->get_global(posdel.x, posdel.y, posdel.z, posdel.w)->get_pix()->group = nullptr;
										size --;
									}
									for (ivec4 posdel : poses_array[i]) {
										//cout << posdel.x << ' ' << posdel.y << ' ' << posdel.z << endl;
										world->get_global(posdel.x, posdel.y, posdel.z, posdel.w)->get_pix()->group = nullptr;
										size --;
									}
									size ++; /// because the orig block is in poses array, so size is decremented too much
									last_poses_array.erase(last_poses_array.begin() + i);
									poses_array.erase(poses_array.begin() + i);
									goals.erase(goals.begin() + i);
									num_goals --;
								} else {
									for (int j = 0; j < num_goals; j ++) {
										if (j != i) {
											if (last_poses_array[i].count(goals[j]) > 0) {
												// cout << "merging arm " << i << " with " << j << endl;
												// cout << last_poses_array[j].size() << ' ' << last_poses_array[i].size() << " to ";
												last_poses_array[j].insert(last_poses_array[i].begin(), last_poses_array[i].end());
												// cout << last_poses_array[j].size() << endl;
												// cout << poses_array[j].size() << ' ' << poses_array[i].size() << " to ";
												poses_array[j].insert(poses_array[i].begin(), poses_array[i].end());
												// cout << poses_array[j].size() << endl;
												last_poses_array.erase(last_poses_array.begin() + i);
												poses_array.erase(poses_array.begin() + i);
												goals.erase(goals.begin() + i);
												num_goals --;
												break;
											}
										}
									}
								}
							}
						}
					}
				}
				for (int i = 0; i < 3; i ++) {
					int j = (i == 2) ? 0 : i + 1;
					int k = (i == 0) ? 2 : i - 1;
					if (delete_arm or pos[i] == position[i]) {
						bool empty = true;
						while (empty) {
							for (int a = 0; a < bounds[j] and empty; a ++) {
								for (int b = 0; b < bounds[k] and empty; b ++) {
									ivec3 cpos(0,0,0);
									cpos[j] = a;
									cpos[k] = b;
									empty = empty and world->get_global(cpos.x+position.x, cpos.y+position.y, cpos.z+position.z, 1)->get_pix()->group != this;
								}
							}
							if (empty) {
								position[i]++;
								bounds[i]--;
							}
						}
					} else if (delete_arm or pos[i] == position[i]+bounds[i]-1) {
						bool empty = true;
						while (empty) {
							for (int a = 0; a < bounds[j] and empty; a ++) {
								for (int b = 0; b < bounds[k] and empty; b ++) {
									ivec3 cpos = bounds-1;
									cpos[j] = a;
									cpos[k] = b;
									empty = empty and world->get_global(cpos.x+position.x, cpos.y+position.y, cpos.z+position.z, 1)->get_pix()->group != this;
								}
							}
							if (empty) {
								bounds[i]--;
							}
						}
					}
				}
			} else {
				cout << "delete group " << this << endl;
				delete this;
			}
			return true;
		}
	}
	return false;
}

//void UnlinkedGroup::spread_to_fill(ivec3 startpos) {
	

void UnlinkedGroup::debug(stringstream& debugstream) {
	debugstream << position.x << ' ' << position.y << ' ' << position.z << endl
							<< bounds.x << ' ' << bounds.y << ' ' << bounds.z << endl
							<< size << endl;
}

#endif
