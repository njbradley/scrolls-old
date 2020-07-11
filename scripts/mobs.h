#ifndef MOBS
#define MOBS

#include "mobs-predef.h"

MobData::MobData(istream& ifile): box1(-1,-1,-1), box2(1,1,1), blockpos(-1,-1,-1) {
	string buff;
  ifile >> buff;
  if (buff != "mob") {
    cout << "ERR: this file is not a mob data file " << endl;
  } else {
    ifile >> name;
    getline(ifile, buff, '{');
    string varname;
    getline(ifile, buff, ':');
    getline(ifile, varname, ':');
    while (!ifile.eof() and varname != "") {
      if (varname == "box1") {
        ifile >> box1.x >> box1.y >> box1.z;
      } else if (varname == "box2") {
        ifile >> box2.x >> box2.y >> box2.z;
      } else if (varname == "blockpath") {
        ifile >> blockpath;
      } else if (varname == "blockpos") {
        ifile >> blockpos.x >> blockpos.y >> blockpos.z;
      } else if (varname == "limbs") {
        string linestr;
				getline(ifile, buff, '[');
				getline(ifile, linestr, ']');
				stringstream line(linestr);
				string limb;
				vec3 pos;
				line >> pos.x >> pos.y >> pos.z;
				line >> limb;
				while (!line.eof()) {
					limbs.emplace_back(pos,limb);
					line >> pos.x >> pos.y >> pos.z;
					line >> limb;
				}
      }
      getline(ifile, buff, ':');
      getline(ifile, varname, ':');
    }
	}
}

MobStorage::MobStorage() {
	vector<string> files;
	get_files_folder("resources/data/entities/mobs", &files);
	for (string path : files) {
		ifstream ifile("resources/data/entities/mobs/" + path);
		MobData* data = new MobData(ifile);
		mobdata[data->name] = data;
	}
	cout << "sucessfully loaded " << files.size() << " mob data files" << endl;
}


Mob::Mob(World* nworld, vec3 start_pos, string newname):
NamedEntity(nworld, start_pos, mobstorage->mobdata[newname]->box1, mobstorage->mobdata[newname]->box2, mobstorage->mobdata[newname]->blockpath,
	mobstorage->mobdata[newname]->blockpos, get_limbs(nworld, newname)), name(newname) {
	
}

vector<DisplayEntity*> Mob::get_limbs(World* world, string name) {
	vector<DisplayEntity*> list;
	for (pair<vec3,string> limb : mobstorage->mobdata[name]->limbs) {
		list.push_back(new Mob(world, limb.first, limb.second));
	}
	return list;
}

void Mob::on_timestep() {
	DisplayEntity::on_timestep();
	if (dead_falling) {
		if ( consts[4]) {
			ivec3 pos(position + blockpos);
			for (int x = 0; x < block.block->scale; x ++) {
				for (int y = 0; y < block.block->scale; y ++) {
					for (int z = 0; z < block.block->scale; z ++) {
						Pixel* pix = block.get_global(x, y, z, 1)->get_pix();
						if (pix->value != 0) {
							world->set(pos.x+x, pos.y+y, pos.z+z, pix->value, pix->direction);
						}
					}
				}
			}
			alive = false;
		}
	} else if (health <= 0) {
		fall_apart(this);
	}
}

void Mob::fall_apart(DisplayEntity* entity) {
	entity->dead_falling = true;
	int render_off = entity->vecs.num_verts/6;
	entity->render_index.second = render_off;
	for (DisplayEntity* limb : entity->limbs) {
		limb->vel = limb->position*3.0f + vec3(0,6,0);
		limb->position += entity->position + vec3(0,0.1f,0);
		limb->render_index = pair<int,int>(entity->render_index.first + render_off, limb->vecs.num_verts/6);
		render_off += limb->render_index.second;
		entity->world->summon(limb);
		limb->drop_ticks();
		fall_apart(limb);
	}
	entity->limbs.clear();
}


void Mob::to_file(ostream& ofile) {
	
}



Pig::Pig(World* nworld, vec3 start_pos): Mob(nworld, start_pos, "pig"), next_point(start_pos) {
	
}

void Pig::on_timestep() {
	Mob::on_timestep();
	if (dead_falling) {
		
	}
	if (!dead_falling) {
		if (int(position.x - next_point.x) == 0 and int(position.z - next_point.z) == 0) {
			if (rand()%100 == 0) {
				limbs[4]->angle.x = rand()%100/50.0 - 1;
			}
			if (rand()%200 == 0) {
				next_point = ivec3(position) + ivec3(rand()%10-5, 0, rand()%10-5);
			}
		} else {
			if (rand()%400 == 0) {
				next_point = ivec3(position) + ivec3(rand()%10-5, 0, rand()%10-5);
			}
			limbs[4]->angle.x = 0;
			vec3 dist =  vec3(next_point) - position;
		  double lon =  atan2(dist.z, dist.x);
			float timer = int(glm::length(dist)*50)%100 / 50.0 - 1;
			if (timer < 0) {
				timer *= -1;
			}
			dist /= glm::length(dist);
			angle.x = lon;
		  if (consts[4]) {
		    vel.x += 0.2 * dist.x;
		    vel.z += 0.2 * dist.z;
		  } else {
		    vel.x += 0.02 * dist.x;
		    vel.z += 0.02 * dist.z;
		  }
		  if (consts[4] and (consts[0] or consts[2] or consts[3] or consts[5])) {
		    vel.y = 20;
		  }
			int b = 1;
			for (int i = 0; i < 4; i ++) {
				limbs[i]->angle.y = (timer - 0.5) * b;
				b *= -1;
			}
		}
	}
}


// void Mob::tick() {
// 	if (target == nullptr) {
// 		find_target();
// 	}
// }
//
// void Mob::find_target() {
// 	target = world->player;
// }
//
// void Mob::find_next_point() {
// 	vec3 diff =
// }


#endif
