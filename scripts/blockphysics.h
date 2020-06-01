#ifndef BLOCKPHYSICS
#define BLOCKPHYSICS

#include "blockphysics-predef.h"

BlockGroup::BlockGroup(World* nworld, ivec3 starting_pos, int max): world(nworld), position(starting_pos), max_size(max), size(0,0,0) {
	find_group();
}

void BlockGroup::find_group() {
	//cout << "-----------------" << endl;
	find_block(position);
}

void BlockGroup::copy_to_block() {
	int scale = 1;
	print (size);
	print (position);
	while (scale < size.x or scale < size.y or scale < size.z) {
		scale *= 2;
	}
	cout << scale << endl;
	char val = world->get_global(position.x, position.y, position.z, 1)->get();
	Pixel* pix = new Pixel(0, 0, 0, val, scale, nullptr, nullptr);
	// char data[scale*scale*scale];
	// for (int i = 0; i < scale*scale*scale; i ++) {
	// 	data[i] = 0;
	// }
	// CharArray arr(data, scale, scale, scale);
	// for (ivec3 pos : block_poses) {
	// 	ivec3 lpos = pos - position;
	// 	arr.set(world->get_global(pos.x, pos.y, pos.z, 1)->get(), lpos.x, lpos.y, lpos.z);
	// }
	// cout << "array start" << endl;
	// for (int x = 0; x < scale; x ++) {
	// 	for (int y = 0; y < scale; y ++) {
	// 		for (int z = 0; z < scale; z ++) {
	// 			cout << int(arr.get(x,y,z)) << ' ';
	// 		}
	// 		cout << endl;
	// 	}
	// 	cout << endl << endl;
	// }
	// cout << "array end " << endl;
	cout << int(val) << endl;
	print(position);
	Chunk* result = pix->resolve([&](ivec3 npos) {
		// char val = arr.get(npos.x, npos.y, npos.z);
		// cout << int(val) << ' ';
		// print (npos);
		// return val;
		char val;
		ivec3 pos = npos + position;
		if (block_poses.find(pos) != block_poses.end()) {
			val =  world->get_global(pos.x, pos.y, pos.z, 1)->get();
		} else {
			val = 0;
		}
		return val;
	});
	cout << "done" << endl;
	if (result == nullptr) {
		cout << "pix" << endl;
		block = pix;
	} else {
		cout << "chunk" << endl;
		block = result;
	}
}
	


void BlockGroup::erase_from_world() {
	for (ivec3 pos : block_poses) {
		world->set(0,pos.x, pos.y, pos.z);
	}
}

void BlockGroup::copy_to_world(ivec3 newpos) {
	for (ivec3 pos : block_poses) {
		ivec3 offset = pos - position;
		world->set(block->get_global(offset.x, offset.y, offset.z,1)->get(), newpos.x+offset.x, newpos.y+offset.y, newpos.z+offset.z);
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
