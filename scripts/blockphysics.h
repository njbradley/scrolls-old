#ifndef BLOCKPHYSICS
#define BLOCKPHYSICS

#include "blockphysics-predef.h"

BlockGroup::BlockGroup(World* nworld, ivec3 starting_pos, int max): world(nworld), position(starting_pos), max_size(max), size(0,0,0) {
	find_group();
}

void BlockGroup::find_group() {
	//cout << "-----------------" << endl;
	//find_block(position);
	Block* starting_block = world->get_global(position.x, position.y, position.z, 1);
	if (starting_block == nullptr or starting_block->get() == 0) {
		return;
	}
	starting_block->get_pix()->physicsgroup = this;
	unordered_set<ivec3,ivec3_hash> lastblocks;
	unordered_set<ivec3,ivec3_hash> newblocks;
	lastblocks.emplace(position);
	const ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	bool under_size = true;
	while (lastblocks.size() > 0 and under_size) {
		under_size = block_poses.size() < max_size;
		//cout << "start" << lastblocks.size() << endl;
		for (ivec3 pos : lastblocks) {
			//cout << "pos";
			//print(pos);
			Pixel* pix = world->get_global(pos.x, pos.y, pos.z, 1)->get_pix();
			BlockData* data = blocks->blocks[pix->value];
			for (int i = 0; i < 6; i ++) {
				ivec3 off = dir_array[i];
				//cout << "pos+off:";
				//print (pos+off);
				if (block_poses.find(pos+off) == block_poses.end()) {
					//cout << "not dup" << endl;
					Block* block = world->get_global(pos.x+off.x, pos.y+off.y, pos.z+off.z, 1);
					//cout << "val " << int(newpix->value) << endl;
					if (block != nullptr) {
						if (block->get() != 0) {
							Pixel* newpix = block->get_pix();
							BlockData* newdata = blocks->blocks[newpix->value];
							bool same = newdata->clumpy_group == data->clumpy_group or newdata->clumpy_group == "all" or data->clumpy_group == "all";
							if (same and under_size) {
								newblocks.emplace(pos+off);
								//print (pos+off);
								newpix->physicsgroup = this;
								//cout << "placing!" << endl;
							} else {
								consts[i] = true;
							}
						}
					} else {
						consts[i] = true;
					}
				}
			}
		}
		//cout << "done one loop" << endl;
		for (ivec3 pos : lastblocks) {
			block_poses.emplace(pos);
		}
		lastblocks.clear();
		lastblocks.swap(newblocks);
		
		cout << "newsize " << lastblocks.size() << endl;
		for (int i = 0; i < 6; i ++) {
			cout << consts[i] << ' ';
		} cout << endl;
	}
	for (ivec3 pos : block_poses) {
		for (int axis = 0; axis < 3; axis ++) {
			if (pos[axis] < position[axis]) {
				size[axis] += position[axis] - pos[axis];
				position[axis] = pos[axis];
			}
			if (pos[axis] >= position[axis]+size[axis]) {
				size[axis] = pos[axis] - position[axis] + 1;
			}
		}
	}
	//exit(1);
}

void BlockGroup::copy_to_block() {
	int scale = 1;
	print (size);
	print (position);
	while (scale < size.x or scale < size.y or scale < size.z) {
		scale *= 2;
	}
	cout << scale << endl;
	Pixel* pix = new Pixel(0, 0, 0, 0, scale, nullptr, nullptr);
	if (scale > 1) {
		block = pix->subdivide();
		for (ivec3 pos : block_poses) {
			Pixel* orig = world->get_global(pos.x, pos.y, pos.z, 1)->get_pix();
			ivec3 local = pos - position;
			Pixel* newpix = block->get_global(local.x, local.y, local.z, 1)->get_pix();
			while (newpix->scale > 1) {
				newpix->subdivide();
				newpix = block->get_global(local.x, local.y, local.z, 1)->get_pix();
			}
			newpix->set(orig->value, orig->direction, orig->extras, false);
		}
	} else {
		Pixel* orig = world->get_global(position.x, position.y, position.z, 1)->get_pix();
		pix->set(orig->value, orig->direction, orig->extras);
		block = pix;
	}
			
	
	// Chunk* result = pix->resolve([&](ivec3 npos) {
	// 	// char val = arr.get(npos.x, npos.y, npos.z);
	// 	// cout << int(val) << ' ';
	// 	// print (npos);
	// 	// return val;
	// 	char val;
	// 	ivec3 pos = npos + position;
	// 	if (block_poses.find(pos) != block_poses.end()) {
	// 		val =  world->get_global(pos.x, pos.y, pos.z, 1)->get();
	// 	} else {
	// 		val = 0;
	// 	}
	// 	return val;
	// });
	// cout << "done" << endl;
	// if (result == nullptr) {
	// 	cout << "pix" << endl;
	// 	block = pix;
	// } else {
	// 	cout << "chunk" << endl;
	// 	block = result;
	// }
}
	
void BlockGroup::remove_pix_pointers() {
	for (ivec3 pos : block_poses) {
		world->get_global(pos.x, pos.y, pos.z,1)->get_pix()->physicsgroup = nullptr;
	}
}

void BlockGroup::erase_from_world() {
	for (ivec3 pos : block_poses) {
		world->set(pos.x, pos.y, pos.z, 0);
	}
	remove_pix_pointers();
}

void BlockGroup::copy_to_world(ivec3 newpos) {
	for (ivec3 pos : block_poses) {
		ivec3 offset = pos - position;
		Pixel* pix = block->get_global(offset.x, offset.y, offset.z,1)->get_pix();
		world->set(newpos.x+offset.x, newpos.y+offset.y, newpos.z+offset.z, pix->value, pix->direction, pix->extras);
	}
}

void BlockGroup::find_block(ivec3 newpos) {
	Pixel* pix = world->get_global(newpos.x, newpos.y, newpos.z, 1)->get_pix();
	if (pix->value == 0) {
		//cout << "err" << endl;
		return;
	}
	block_poses.emplace(newpos);
	for (int axis = 0; axis < 3; axis ++) {
		if (newpos[axis] < position[axis]) {
			size[axis] += position[axis] - newpos[axis];
			position[axis] = newpos[axis];
		}
		if (newpos[axis] >= position[axis]+size[axis]) {
			size[axis] = newpos[axis] - position[axis] + 1;
		}
	}
	//print(newpos);
	//cout << int(pix->value) << ' ';
	BlockData* data = blocks->blocks[pix->value];
	//cout << data->clumpy_group << ' ' << data->clumpyness << endl;
	const ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	for (int i = 0; i < 6; i ++) {
		ivec3 pos = dir_array[i] + newpos;
		if (std::find(block_poses.begin(), block_poses.end(), pos) == block_poses.end()) {
			Block* sideblock = world->get_global(pos.x, pos.y, pos.z, 1);
			if (sideblock != nullptr) {
				Pixel* sidepix = sideblock->get_pix();
				BlockData* sidedata = blocks->blocks[sidepix->value];
				if (sidepix->value != 0) {
					if (data->clumpy_group == sidedata->clumpy_group and float(rand())/RAND_MAX < data->clumpyness and block_poses.size() < max_size) {
						find_block(pos);
					} else {
						consts[i] = true;
					}
				}
			} else {
				consts[i] = true;
			}
		}
	}
}
					

#endif
