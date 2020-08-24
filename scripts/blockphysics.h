#ifndef BLOCKPHYSICS
#define BLOCKPHYSICS

#include "blockphysics-predef.h"

BlockGroup::BlockGroup(World* nworld, ivec3 starting_pos): world(nworld), position(starting_pos), size(0,0,0), update_flag(false), block(nullptr) {
	find_group();
}

BlockGroup::BlockGroup(World* nworld, unordered_set<ivec3,ivec3_hash>& poses, Block* newblock):
world(nworld), position(0,0,0), size(0,0,0), update_flag(false), block(newblock) {
	block_poses.swap(poses);
	if (block_poses.size() > 0) {
		bool init = false;
		ivec3 max;
		ivec3 min;
		for (ivec3 pos : block_poses) {
			if (!init) {
				min = pos;
				max = pos;
				init = true;
			} else {
				if (pos.x < min.x) {
					min.x = pos.x;
				}
				if (pos.y < min.y) {
					min.y = pos.y;
				}
				if (pos.z < min.z) {
					min.z = pos.z;
				}
				if (pos.x > max.x) {
					max.x = pos.x;
				}
				if (pos.y > max.y) {
					max.y = pos.y;
				}
				if (pos.z > max.z) {
					max.z = pos.z;
				}
			}
		}
		position = min;
		size = max;
	}
}

BlockGroup::~BlockGroup() {
	// cout << "deleting " << groupname << endl;
	// for (ivec3 pos : block_poses) {
	// 	cout << pos.x << ' ' << pos.y << ' ' << pos.z << ", ";
	// } cout << endl;
	if (block != nullptr) {
		block->del(false);
		delete block;
		block = nullptr;
	}
	remove_pix_pointers();
}

void BlockGroup::find_group() {
	
	Block* starting_block = world->get_global(position.x, position.y, position.z, 1);
	if (starting_block == nullptr or starting_block->get() == 0) {
		return;
	}
	groupname = blocks->blocks[starting_block->get()]->clumpy_group;
	starting_block->get_pix()->physicsgroup = this;
	unordered_set<ivec3,ivec3_hash> lastblocks;
	unordered_set<ivec3,ivec3_hash> newblocks;
	lastblocks.emplace(position);
	const ivec3 dir_array[6] = {{1,0,0}, {0,1,0}, {0,0,1}, {-1,0,0}, {0,-1,0}, {0,0,-1}};
	bool under_size = true;
	while (lastblocks.size() > 0 and under_size) {
		//loops until no new blocks or it is over size
		under_size = block_poses.size() < max_size;
		
		for (ivec3 pos : lastblocks) {
			Pixel* pix = world->get_global(pos.x, pos.y, pos.z, 1)->get_pix();
			BlockData* data = blocks->blocks[pix->value];
			for (int i = 0; i < 6; i ++) {
				ivec3 off = dir_array[i];
				if (block_poses.find(pos+off) == block_poses.end()) {
					Block* block = world->get_global(pos.x+off.x, pos.y+off.y, pos.z+off.z, 1);
					if (block != nullptr) {
						if (block->get() != 0) {
							Pixel* newpix = block->get_pix();
							BlockData* newdata = blocks->blocks[newpix->value];
							bool same = newdata->clumpy_group == data->clumpy_group or newdata->clumpy_group == "all" or data->clumpy_group == "all";
							if (same and under_size) {
								newblocks.emplace(pos+off);
								newpix->physicsgroup = this;
							} else {
								consts[i] = true;
								if (persistant() and newpix->physicsgroup != nullptr and newpix->physicsgroup->persistant()) {
									//cout << newpix->physicsgroup << endl;
									neighbors.emplace(newpix->physicsgroup);
								}
							}
						}
					} else {
						consts[i] = true;
					}
				}
			}
		}
		for (ivec3 pos : lastblocks) {
			block_poses.emplace(pos);
		}
		lastblocks.clear();
		lastblocks.swap(newblocks);
		
		// cout << "newsize " << lastblocks.size() << endl;
		// for (int i = 0; i < 6; i ++) {
		// 	cout << consts[i] << ' ';
		// } cout << endl;
	}
	// if (persistant() and group_group() != "null") {
	// 	for (BlockGroup* group : neighbors) {
	// 		cout << "neighbor -------" << group << endl;
	// 		if (group->group_group() == group_group()) {
	// 			for (ivec3 pos : group->block_poses) {
	// 				cout << "--";
	// 				print(pos);
	// 			}
	// 			for (ivec3 pos : group->block_poses) {
	// 				cout << "--";
	// 				print(pos);
	// 				for (int i = 0; i < 6; i ++ ) {
	// 					if (!consts[i]) {
	// 						ivec3 off = pos + dir_array[i];
	// 						print (off);
	// 						if (block_poses.find(off) == block_poses.end() and group->block_poses.find(off) == group->block_poses.end()) {
	// 							Block* b = world->get_global(off.x, off.y, off.z, 1);
	// 							if (b != nullptr and b->get() != 0) {
	// 								consts[i] = true;
	// 								cout << "setting consts" << endl;
	// 							}
	// 						}
	// 					}
	// 				}
	// 			}
	// 		}
	// 	}
	// }
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

bool BlockGroup::persistant() {
	return false;
}

void BlockGroup::update() {
	//cout << this << " blockgroup " << position.x << ' ' << position.y << ' ' << position.z << " update" << endl;
	vector<ivec3> dead_blocks;
	for (ivec3 pos : block_poses) {
		Block* b = world->get_global(pos.x, pos.y, pos.z, 1);
		if (b != nullptr) {
			Pixel* pix = b->get_pix();
			if (pix->physicsgroup != this) {
				dead_blocks.push_back(pos);
			}
		}
	}
	for (ivec3 pos : dead_blocks) {
		block_poses.erase(pos);
	}
	bool found_pos = false;
	ivec3 newpos;
	for (ivec3 pos : block_poses) {
		Block* b = world->get_global(pos.x, pos.y, pos.z, 1);
		if (b != nullptr and b->get() != 0 and blocks->blocks[b->get()]->clumpy_group == groupname) {
			found_pos = true;
			newpos = pos;
			break;
		}
	}
	remove_pix_pointers();
	block_poses.clear();
	if (found_pos) {
		//cout << this << " group found new pos " << newpos.x << ' ' << newpos.y << ' ' << newpos.z << " instead of ";
		//print (position);
		position = newpos;
		size = ivec3(0,0,0);
		find_group();
		on_update();
	}
}

void BlockGroup::on_update() {
	//cout << "on update" << endl;
}

void BlockGroup::add_item(ItemStack stack) {
	
}

void BlockGroup::copy_to_block() {
	int scale = 1;
	// print (size);
	// print (position);
	while (scale < size.x or scale < size.y or scale < size.z) {
		scale *= 2;
	}
	//cout << scale << endl;
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
		pix->set(orig->value, orig->direction, orig->extras, false);
		block = pix;
	}
}
	
void BlockGroup::remove_pix_pointers() {
	for (ivec3 pos : block_poses) {
		Block* b = world->get_global(pos.x, pos.y, pos.z,1);
		if (b != nullptr) {
			Pixel* pix = b->get_pix();
		// if (pix->physicsgroup != nullptr and pix->physicsgroup != this) {
		// 	cout << "153removing " << pos.x << ' ' << pos.y << ' ' << pos.z << " from  " << pix->physicsgroup << endl;
		// 	pix->physicsgroup->block_poses.erase(pos);
		// }
			pix->physicsgroup = nullptr;
		}
	}
}

void BlockGroup::set_pix_pointers() {
	if (block == nullptr) {
		for (ivec3 pos : block_poses) {
			Block* b = world->get_global(pos.x, pos.y, pos.z,1);
			if (b != nullptr) {
				// if (b->get_pix()->physicsgroup != nullptr and b->get_pix()->physicsgroup != this) {
				// 	b->get_pix()->physicsgroup->block_poses.erase(pos);
				// }
				b->get_pix()->physicsgroup = this;
			}
		}
	}
}

void BlockGroup::erase_from_world() {
	remove_pix_pointers();
	for (ivec3 pos : block_poses) {
		//print (pos);
		world->set(pos.x, pos.y, pos.z, 0);
	}
}

void BlockGroup::copy_to_world(ivec3 newpos) {
	for (ivec3 pos : block_poses) {
		ivec3 offset = pos - position;
		Pixel* pix = block->get_global(offset.x, offset.y, offset.z,1)->get_pix(); // ERR: offset.z is uninitialized, something to do with new entity deleting system?
		world->set(newpos.x+offset.x, newpos.y+offset.y, newpos.z+offset.z, pix->value, pix->direction, pix->extras);
	}
}

void BlockGroup::custom_textures(int mats[6], int dirs[6], ivec3 pos) {
	
}

AxleInterface* BlockGroup::cast_axle() {
	return nullptr;
}

void BlockGroup::tick() {
	if (update_flag) {
		update();
		update_flag = false;
	}
}

bool BlockGroup::rcaction(Player* player, Item* item) {
	return false;
}

string BlockGroup::group_group() {
	return "null";
}

void BlockGroup::link() {
	
}

BlockGroup* BlockGroup::make_group(char val, World* world, ivec3 pos) {
	BlockData* data = blocks->blocks[val];
	string name = "undef";
	if (data != nullptr) {
		name = data->clumpy_group;
	}
	if (name == "axle-group") return new AxleGroup(world, pos);
	if (name == "grindstone-group") return new GrindstoneGroup(world, pos);
	if (name == "crafting-group") return new CraftingGroup(world, pos);
	if (name == "chest-group") return new ChestGroup(world, pos);
	if (name == "backpack-group") return new BackpackGroup(world, pos);
	if (name == "ghost-group") return new DissolveGroup(world, pos);
	if (name == "anvil-group") return new AnvilGroup(world, pos);
	if (name == "glass-group") return new MatchSidesGroup(world, pos);
	return new BlockGroup(world, pos);
}

BlockGroup* BlockGroup::from_file(World* world, istream& ifile) {
	string name;
	int pos = ifile.tellg();
	ifile >> name;
	//cout << name << endl;
	ifile.seekg(pos);
	if (name == "axle-group") return new AxleGroup(world, ifile);
	if (name == "grindstone-group") return new GrindstoneGroup(world, ifile);
	if (name == "crafting-group") return new CraftingGroup(world, ifile);
	if (name == "chest-group") return new ChestGroup(world, ifile);
	if (name == "backpack-group") return new BackpackGroup(world, ifile);
	if (name == "ghost-group") return new DissolveGroup(world, ifile);
	if (name == "anvil-group") return new AnvilGroup(world, ifile);
	if (name == "glass-group") return new MatchSidesGroup(world, ifile);
	return new BlockGroup(world, ifile);
}

bool BlockGroup::is_persistant(char val) {
	BlockData* data = blocks->blocks[val];
	string name = "undef";
	if (data != nullptr) {
		name = data->clumpy_group;
	}
	if (name == "axle-group") return true;
	if (name == "grindstone-group") return true;
	if (name == "crafting-group") return true;
	if (name == "chest-group") return true;
	if (name == "backpack-group") return true;
	if (name == "ghost-group") return true;
	if (name == "anvil-group") return true;
	if (name == "glass-group") return true;
	return false;
}

BlockGroup::BlockGroup(World* nworld, istream& ifile): world(nworld), update_flag(false) {
	ifile >> groupname;
	ifile >> position.x >> position.y >> position.z;
	ifile >> size.x >> size.y >> size.z;
	int size;
	ifile >> size;
	for (int i = 0; i < size; i ++) {
		ivec3 pos;
		ifile >> pos.x >> pos.y >> pos.z;
		block_poses.emplace(pos);
	}
	string isblock;
	ifile >> isblock;
	if (isblock == "block") {
		int scale;
		ifile >> scale;
		string buff;
		getline(ifile, buff, ':');
		block = Block::from_file(ifile, 0, 0, 0, scale, nullptr, nullptr);
	} else {
		block = nullptr;
	}
}

void BlockGroup::to_file(ostream& ofile) {
	ofile << groupname << endl;
	ofile << position.x << ' ' << position.y << ' ' << position.z << endl;
	ofile << size.x << ' ' << size.y << ' ' << size.z << endl;
	ofile << block_poses.size() << endl;
	for (ivec3 pos : block_poses) {
		ofile << pos.x << ' ' << pos.y << ' ' << pos.z << endl;
	}
	if (block != nullptr) {
		ofile << "block " << block->scale << endl << ':';
		block->save_to_file(ofile);
	} else {
		ofile << "null" << endl;
	}
}




AxleInterface::AxleInterface(World* nworld, ivec3 starting_pos): BlockGroup(nworld, starting_pos) {
	
}

AxleInterface::AxleInterface(World* nworld, istream& ifile): BlockGroup(nworld, ifile) {
	
}

AxleInterface* AxleInterface::cast_axle() {
	return this;
}

bool AxleInterface::persistant() {
	return true;
}

string AxleInterface::group_group() {
	return "axle-group-group";
}





AxleGroup::AxleGroup(World* nworld, ivec3 starting_pos): AxleInterface(nworld, starting_pos) {
	//find_group();
	is_valid = calc_is_valid();
	port1 = nullptr;
	port2 = nullptr;
	if (is_valid) {
		find_neighbors();
	}
}

void AxleGroup::on_update() {
	cout << "on acxle update " << endl;
	is_valid = calc_is_valid();
	cout << "is valid " << is_valid << endl;
	print (start);
	print (position);
	print (size);
	print (end);
	port1 = nullptr;
	port2 = nullptr;
	if (is_valid) {
		find_neighbors();
	}
	cout << port1 << ' ' << port2 << endl;
}

void AxleGroup::find_neighbors() {
	cout << "find neigh" << endl;
	ivec3 neg(start);
	neg[axis]--;
	Block* start_block = world->get_global(neg.x, neg.y, neg.z, 1);
	if (start_block != nullptr) {
		BlockGroup* group = start_block->get_pix()->physicsgroup;
		if (group != nullptr) {
			port1 = group->cast_axle();
		}
	}
	
	ivec3 pos(end);
	pos[axis]++;
	Block* end = world->get_global(pos.x, pos.y, pos.z, 1);
	if (end != nullptr) {
		BlockGroup* group = end->get_pix()->physicsgroup;
		if (group != nullptr) {
			port1 = group->cast_axle();
		}
	}
}

void AxleGroup::rotate(AxleInterface* sender, double amount, double force) {
	if (is_valid) {
		if (sender != port1 and port1 != nullptr) {
			port1->rotate(this, amount, force);
		}
		if (sender != port2 and port2 != nullptr) {
			port2->rotate(this, amount, force);
		}
	}
}

bool AxleGroup::calc_is_valid() {
	int dir = -1;
	for (ivec3 pos : block_poses) {
		int newdir = world->get_global(pos.x, pos.y, pos.z,1)->get_pix()->direction;
		if (newdir > 2) {
			newdir -= 3;
		}
		if (dir == -1) {
			dir = newdir;
		}
		if (dir != newdir) {
			return false;
		}
	}
	if (dir != -1) {
		axis = dir;
		for (int i = 0; i < 3; i ++) {
			if (i != axis and size[i] != 1) {
				return false;
			}
		}
		start = position;
		end = position + size - 1;
		return true;
	}
	return false;
}

bool AxleGroup::rcaction(Player* player, Item* item) {
	rotate(this, 0.25, 0.1);
	return true;
}

void AxleGroup::link() {
	cout << "linking ";
	cout << "port1 " << port1 << " port2 " << port2 << endl;
	print(position);
	print(port1pos);
	print(port2pos);
	cout << endl;
	for (BlockGroup* group : world->physicsgroups) {
		print (group->position);
		if (port1 == (AxleInterface*) 1 and group->position == port1pos) {
			cout << "match1 " << group << ' ' << group->cast_axle() << endl;
			port1 = group->cast_axle();
		}
		if (port2 == (AxleInterface*) 1 and group->position == port2pos) {
			cout << "match2 " << group << ' ' << group->cast_axle() << endl;
			port2 = group->cast_axle();
		}
	}
	cout << "port1 " << port1 << " port2 " << port2 << endl;
}

AxleGroup::AxleGroup(World* nworld, istream& ifile): AxleInterface(nworld, ifile), is_valid(true) {
	string buff;
	ifile >> buff;
	cout << buff << endl;
	if (buff == "pointer") {
		ifile >> port1pos.x >> port1pos.y >> port1pos.z;
		cout << "1 " << endl;
		print (port1pos);
		port1 = (AxleInterface*)1;
	} else {
		port1 = nullptr;
		cout << "null1" << endl;
	}
	ifile >> buff;
	cout << buff << endl;
	if (buff == "pointer") {
		ifile >> port2pos.x >> port2pos.y >> port2pos.z;
		cout << "2 ";
		print (port2pos);
		port2 = (AxleInterface*)1;
	} else {
		port2 = nullptr;
	}
}

void AxleGroup::to_file(ostream& ofile) {
	BlockGroup::to_file(ofile);
	cout << port1 << ' ' << port2 << endl;
	if (port1 == nullptr) {
		ofile << "null" << endl;
	} else {
		ofile << "pointer" << endl;
		ivec3 pos = port1->position;
		ofile << pos.x << ' ' << pos.y << ' ' << pos.z << endl;
	}
	if (port2 == nullptr) {
		ofile << "null" << endl;
	} else {
		ofile << "pointer" << endl;
		ivec3 pos = port2->position;
		ofile << pos.x << ' ' << pos.y << ' ' << pos.z << endl;
	}
}











GrindstoneGroup::GrindstoneGroup(World* nworld, ivec3 starting_pos): AxleInterface(nworld, starting_pos) {
	//find_group();
}

bool GrindstoneGroup::rcaction(Player* player, Item* item) {
	if (!item->isnull and item->data->sharpenable) {
		item->sharpen(speed + 0.5, force + 0.5);
		return true;
	}
	// if (menu == nullptr) {
	// 	menu = new InventoryMenu("grindstone", &input, [&] () {
	// 		delete menu;
	// 		menu = nullptr;
	// 	});
	// }
	return false;
}

void GrindstoneGroup::rotate(AxleInterface* sender, double amount, double newforce) {
	cout << "recieved rotation" << endl;
	speed = amount;
	force = newforce;
	// Item* item = input.get(0);
	// if (!item->isnull) {
	// 	progress += amount;
	// 	if (progress >= 1) {
	// 		item->sharpness += 1;
	// 		item->weight -= 0.1;
	// 		progress = 0;
	// 	}
	// }
}

void GrindstoneGroup::custom_textures(int mats[6], int dirs[6], ivec3 pos) {
	
}

GrindstoneGroup::GrindstoneGroup(World* nworld, istream& ifile): AxleInterface(nworld, ifile) {
	
}

void GrindstoneGroup::to_file(ostream& ofile) {
	BlockGroup::to_file(ofile);
}




MatchSidesGroup::MatchSidesGroup(World* nworld, ivec3 starting_pos): BlockGroup(nworld, starting_pos) {
	//find_group();
}

MatchSidesGroup::MatchSidesGroup(World* nworld, istream& ifile): BlockGroup(nworld, ifile) {
	
}

void MatchSidesGroup::custom_textures(int mats[6], int dirs[6], ivec3 position) {
	// order: all_sides, u shape, top + bottom sides, L shape, only bottom, no sides
	
	
	int first_open = -1, first_closed = -1;
	
	for (int axis = 0; axis < 3; axis ++) {
		const ivec2 all_dirs[] = {{-1,0}, {0,1}, {1,0}, {0,-1}};
		bool sides[4];
		int i = 0;
		for (ivec2 dir : all_dirs) {
			ivec3 dir3(0,0,0);
			if (axis == 0) {
				dir3 = ivec3(0, dir.y, dir.x);
			} else if (axis == 1) {
				dir3 = ivec3(dir.y, 0, -dir.x);
			} else if (axis == 2) {
				dir3 = ivec3(-dir.x, dir.y, 0);
			}
			sides[i] = block_poses.count(position+dir3) > 0;
			i++;
		}
		
		bool side_mats[6][4] = {
			{false, false, false, false},
			{false, false, false, true},
			{false, true, false, true},
			{false, true, true, false},
			{true, true, true, false},
			{true, true, true, true}
		};
		
		for (int pos_neg = 0; pos_neg < 2; pos_neg ++) {
			
			int offset = 0;
			bool match = false;
			int mat = -2;
			bool newsides[4];
			for (int i = 0; i < 4; i ++) {
				newsides[i] = sides[i];
			}
			
			while (!match and offset < 4) {
				
				
				for (int j = 0; j < 6; j ++) {
					bool all_same = true;
					for (int k = 0; k < 4; k ++) {
						if (side_mats[j][k] != newsides[k]) {
							all_same = false;
						}
					}
					if (all_same) {
						match = true;
						mat = j;
						break;
					}
				}
				if (!match) {
					int tmp = newsides[3];
					newsides[3] = newsides[2];
					newsides[2] = newsides[1];
					newsides[1] = newsides[0];
					newsides[0] = tmp;
					offset ++;
				}
			}
			
			mats[axis + 3*pos_neg] += mat;
			dirs[axis + 3*pos_neg] = offset;
			
			//cout << offset << ' ' << pos_neg << ' ' << axis << ' ' << sides[0] << sides[1] << sides[2] << sides[3] << ' ';
			bool tmp = sides[0];
			sides[0] = sides[2];
			sides[2] = tmp;
			//cout  << sides[0] << sides[1] << sides[2] << sides[3] << endl;
		}
	}
}

bool MatchSidesGroup::persistant() {
	return true;
}

void MatchSidesGroup::on_update() {
	for (ivec3 pos : block_poses) {
		Block* b = world->get_global(pos.x, pos.y, pos.z, 1);
		if (b != nullptr) {
			b->get_pix()->set_render_flag();
		}
	}
}



ChestGroup::ChestGroup(World* nworld, ivec3 starting_pos): BlockGroup(nworld, starting_pos), inven(block_poses.size()) {
	//find_group();
}

ChestGroup::ChestGroup(World* nworld, istream& ifile): BlockGroup(nworld, ifile), inven(ifile) {
	
}

bool ChestGroup::rcaction(Player* player, Item* item) {
	menu = new InventoryMenu("chest", &inven, [&] () {
		delete menu;
		menu = nullptr;
	});
	return true;
}

void ChestGroup::on_update() {
	update_inven_size();
}

void ChestGroup::add_item(ItemStack stack) {
	inven.add(stack);
}

void ChestGroup::update_inven_size() {
	int newsize = block_poses.size();
	if (newsize > 10) {
		newsize = 10;
	}
	if (newsize != inven.size) {
		ItemContainer newinven(newsize);
		for (int i = 0; i < newsize and i < inven.size; i ++) {
			newinven.items[i] = inven.items[i];
		}
		inven = newinven;
	}
}

bool ChestGroup::persistant() {
	return true;
}

void ChestGroup::to_file(ostream& ofile) {
	BlockGroup::to_file(ofile);
	inven.save_to_file(ofile);
}



BackpackGroup::BackpackGroup(World* world, ivec3 spos): ChestGroup(world, spos) {
	
}

BackpackGroup::BackpackGroup(World* world, istream& ifile): ChestGroup(world, ifile) {
	
}

bool BackpackGroup::rcaction(Player* player, Item* item) {
	menu = new InventoryMenu("chest", &inven, [&] () {
		int num_items = 0;
		for (ItemStack stack : inven.items) {
			if (!stack.item.isnull) {
				num_items ++;
			}
		}
		if (num_items == 0) {
			erase_from_world();
			block_poses.clear();
		}
		delete menu;
		menu = nullptr;
	});
	return true;
}




CraftingGroup::CraftingGroup(World* nworld, ivec3 starting_pos): BlockGroup(nworld, starting_pos) {
	
}

CraftingGroup::CraftingGroup(World* nworld, istream& ifile): BlockGroup(nworld, ifile) {
	
}

bool CraftingGroup::rcaction(Player* player, Item* item) {
	int level = 1;
	bool found = false;
	for (int i = 0; i < 3; i ++) {
		if (size[i] != 1) {
			if (size[i] == level) {
				found = true;
				break;
			}
			if (size[i] != level) {
				level = size[i];
			}
		}
	}
	if ((found and block_poses.size() == level*level) or size == ivec3(1,1,1)) {
		menu = new CraftingMenu(level, [&] () {
			delete menu;
			menu = nullptr;
		});
	}
	return true;
}

bool CraftingGroup::persistant() {
	return true;
}





AnvilGroup::AnvilGroup(World* nworld, ivec3 starting_pos): BlockGroup(nworld, starting_pos) {
	
}

AnvilGroup::AnvilGroup(World* nworld, istream& ifile): BlockGroup(nworld, ifile) {
	
}

bool AnvilGroup::rcaction(Player* player, Item* item) {
	menu = new ToolMenu([&] () {
		delete menu;
		menu = nullptr;
	});
	return true;
}

bool AnvilGroup::persistant() {
	return true;
}





DissolveGroup::DissolveGroup(World* world, ivec3 starting_pos): BlockGroup(world, starting_pos) {
	
}

DissolveGroup::DissolveGroup(World* world, istream& ifile): BlockGroup(world, ifile) {
	
}

bool DissolveGroup::persistant() {
	return true;
}

void DissolveGroup::tick() {
	ivec3 pos;
	int index = rand()%block_poses.size();
	int i = 0;
	for (ivec3 p : block_poses) {
		if (index == i) {
			pos = p;
			break;
		}
		i++;
	}
	world->set(pos.x, pos.y, pos.z, 0);
	block_poses.erase(pos);
}

#endif
