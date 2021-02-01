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


BlockGroup::BlockGroup(World* nworld): world(nworld), item(nullptr) {
	id = rand();
}

BlockGroup::BlockGroup(World* nworld, vector<Pixel*> pixels): world(nworld), item(nullptr), uniform_type(false) {
	
	id = rand();
	for (Pixel* pix : pixels) {
		add(pix);
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
						if (sidepix->scale == pix->scale) {
							sidepix->joints[(i+3)%6] = 7;
						}
						pix->joints[i] = 7;
						pix->render_update();
						sidepix->render_update();
					}
				} else {
					for (Pixel* sidepix : block->iter_side(-dir)) {
						if (sidepix->value != 0 and std::find(pixels.begin(), pixels.end(), sidepix) == pixels.end()) {
							int num_touching = std::min(pix->scale, sidepix->scale);
							consts[i] += num_touching * num_touching;
							
							sidepix->joints[(i+3)%6] = 7;
							pix->render_update();
							sidepix->render_update();
						}
					}
				}
			}
			i ++;
		}
	}
	
}

BlockGroup::BlockGroup(World* nworld, istream& ifile): world(nworld), item(ifile) {
	int intblocktype, numconnect;
	ifile >> id >> numconnect;
	for (int i = 0; i < numconnect; i ++) {
		int index;
		string matname;
		ifile >> index >> matname;
		connections.push_back(GroupConnection(index, connstorage->connectors[matname]));
	}
	ifile >> size >> final >> uniform_type >> intblocktype;
	blocktype = intblocktype;
	for (int i = 0; i < 6; i ++) {
		ifile >> consts[i];
	}
}

void BlockGroup::to_file(ostream& ofile, vector<BlockGroup*>* allgroups) {
	item.to_file(ofile);
	ofile << id << ' ' << connections.size() << ' ';
	for (int i = 0; i < connections.size(); i ++) {
		ofile << connections[i].group->id << ' ' << connections[i].data->name << ' ';
	}
	ofile << endl;
	ofile << size << ' ' << final << ' ' << uniform_type << ' ' << int(blocktype) << ' ';
	for (int val : consts) {
		ofile << val << ' ';
	}
	ofile << endl;
}

BlockGroup::~BlockGroup() {
	while (connections.size() > 0) {
		del_connection(connections.back().group);
	}
}

void BlockGroup::link(vector<BlockGroup*>* allgroups) {
	for (int i = 0; i < connections.size(); i ++) {
		for (BlockGroup* group : *allgroups) {
			if (group->id == connections[i].globalindex) {
				connections[i].group = group;
			}
		}
	}
}

void BlockGroup::merge_copy(BlockGroup* other) {
	for (int i = 0; i < connections.size(); i ++) {
		for (int j = 0; j < other->connections.size(); j ++) {
			if (connections[i].globalindex == other->connections[j].globalindex) {
				if (connections[i].group == nullptr) {
					connections[i].group = other->connections[j].group;
				} else if (other->connections[j].group == nullptr) {
					other->connections[j].group = connections[i].group;
				}
			}
		}
	}
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

void BlockGroup::spread(Pixel* pix, int depth, int max_size, float randlevel) {
	if (depth > 0 and max_size != 0 and add(pix)) {
		ivec3 pos;
		pix->global_position(&pos.x, &pos.y, &pos.z);
		int i = 0;
		for (ivec3 dir : dir_array) {
			ivec3 sidepos = dir*pix->scale + pos;
			Block* sideblock = world->get_global(sidepos.x, sidepos.y, sidepos.z, pix->scale);
			if (sideblock != nullptr and (randlevel == 1 or rand()%1000 / 1000.0f < randlevel)) {
				for (Pixel* sidepix : sideblock->iter_side(-dir)) {
					if (pix->joints[i] < 7) {
						spread(sidepix, depth-1, max_size - 1, randlevel);
					}
					if (!contains(sidepix)) {
						if (sidepix->value != 0) {
							if (pix->joints[i] < 7) {
								isolated = false;
							}
							int num_touching = std::min(pix->scale, sidepix->scale);
							consts[i] += num_touching * num_touching;
						}
					}
					if (!contains(pix)) {
						return;
					}
				}
			} else {
				consts[i] += pix->scale * pix->scale;
			}
			i ++;
		}
	}
}

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

bool BlockGroup::can_take(Pixel* pix) {
	// way that merges groups the smart way
	// return !final and pix->group != nullptr and pix->group != this
	// and !pix->group->final and pix->group->blocktype == blocktype
	// and pix->group->size <= size;
	
	// way that works with deleting
	return !final and pix->group != nullptr and pix->group != this
	and !pix->group->final and pix->group->blocktype == blocktype;
}

bool BlockGroup::contains(Pixel* pix) {
	return pix->group == this;
}

void BlockGroup::recalculate_all(Pixel* start_pix) {
	for (int i = 0; i < 6; i ++) {
		consts[i] = 0;
	}
	
  unordered_set<Pixel*> pixels;
  unordered_set<Pixel*> last_pixels;
  unordered_set<Pixel*> new_pixels;
  
  last_pixels.emplace(start_pix);
  
  while (last_pixels.size() > 0) {
    for (Pixel* pix : last_pixels) {
      pixels.emplace(pix);
    }
    for (Pixel* pix : last_pixels) {
      ivec3 pos;
      pix->global_position(&pos.x, &pos.y, &pos.z);
      
			int i = 0;
      for (ivec3 dir : dir_array) {
        ivec3 sidepos = pos + pix->scale * dir;
        Block* block = world->get_global(sidepos.x, sidepos.y, sidepos.z, pix->scale);
        if (block != nullptr) {
          for (Pixel* sidepix : block->iter_side(-dir)) {
            if (pixels.count(sidepix) == 0) {
							if (contains(sidepix)) {
              	new_pixels.emplace(sidepix);
            	} else if (sidepix->value != 0) {
								consts[i] ++;
							}
						}
          }
        }
				i ++;
      }
    }
    last_pixels.swap(new_pixels);
    new_pixels.clear();
  }
}
  

bool BlockGroup::constrained(int index, vector<BlockGroup*>* past_groups) {
	if (consts[index]) {
		return true;
	}
	if (past_groups == nullptr) {
		vector<BlockGroup*> groups {this};
		for (int i = 0; i < connections.size(); i ++) {
			if (connections[i].group->constrained(index, &groups)) {
				return true;
			}
		}
		return false;
	} else {
		past_groups->push_back(this);
		for (int i = 0; i < connections.size(); i ++) {
			if (std::find(past_groups->begin(), past_groups->end(), connections[i].group) == past_groups->end()
			and connections[i].group->constrained(index)) {
				return true;
			}
		}
		return false;
	}
}

void BlockGroup::connect_to(GroupConnection connection) {
	bool replaced = false;
	for (int i = 0; i < connections.size(); i ++) {
		if (connections[i].group == connection.group) {
			connections[i] = connection;
			replaced = true;
			break;
		}
	}
	if (!replaced) {
		connections.push_back(connection);
	}
	if (connection.group == nullptr) {
		cout << "Warning group null connect " << endl;
	}
	GroupConnection otherconnection = connection;
	otherconnection.group = this;
	replaced = false;
	for (int i = 0; i < connection.group->connections.size(); i ++) {
		if (connection.group->connections[i].group == this) {
			connection.group->connections[i] = otherconnection;
			replaced = true;
			break;
		}
	}
	if (!replaced) {
		connection.group->connections.push_back(otherconnection);
	}
}

GroupConnection* BlockGroup::get_connection(BlockGroup* othergroup) {
	for (int i = 0; i < connections.size(); i ++) {
		if (connections[i].group == othergroup) {
			return &connections[i];
		}
	}
	return nullptr;
}

void BlockGroup::del_connection(BlockGroup* othergroup) {
	for (int i = 0; i < connections.size(); i ++) {
		if (connections[i].group == othergroup) {
			connections.erase(connections.begin() + i);
		}
	}
	
	for (int j = 0; j < othergroup->connections.size(); j ++) {
		if (othergroup->connections[j].group == this) {
			othergroup->connections.erase(othergroup->connections.begin() + j);
		}
	}
}

void BlockGroup::del_connection(int index) {
	BlockGroup* othergroup = connections[index].group;
	connections.erase(connections.begin() + index);
	
	for (int j = 0; j < othergroup->connections.size(); j ++) {
		if (othergroup->connections[j].group == this) {
			othergroup->connections.erase(othergroup->connections.begin() + j);
		}
	}
}

#endif
