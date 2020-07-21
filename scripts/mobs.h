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
			ivec3 pos(glm::round(position + blockpos));
			if (angle.y > 3.14/4) {
				block.block->rotate(2, 1);
			} else if (angle.y < -3.14/4) {
				block.block->rotate(2,-1);
			}
			
			int lat = round(angle.x/(3.14/2));
			lat = lat%4;
			for (int i = 0; i < lat; i ++) {
				block.block->rotate(1, 1); //ERR: somehow skeletons are getting instantiated with uninitialized block.blocks? block.block is 0xfeeefeeefeeefeee
																	 //weird, it cant be 0xfeeefeeefeeefeee from the start, must be partially deleted?
			}
			for (int i = 0; i > lat; i --) {
				block.block->rotate(1, -1);
			}
			
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
			cout <<  this << endl;
			alive = false;
		}
		return;
	} else if (health <= 0) {
		fall_apart(this);
	}
}

void Mob::fall_apart(DisplayEntity* entity) {
	entity->dead_falling = true;
	int render_off = entity->vecs.num_verts/6;
	entity->render_index.second = render_off;
	for (DisplayEntity* limb : entity->limbs) {
		// rotation
		float x = limb->position.x;
		float y = limb->position.y;
		float z = limb->position.z;
		limb->position.x = cos(entity->angle.y) * x - sin(entity->angle.y) * y;
		limb->position.y = sin(entity->angle.y) * x + cos(entity->angle.y) * y;
		x = limb->position.x;
		y = limb->position.y;
		z = limb->position.z;
		limb->position.x = cos(entity->angle.x) * x - sin(entity->angle.x) * z;
		limb->position.z = sin(entity->angle.x) * x + cos(entity->angle.x) * z;
		
		limb->vel = limb->position*3.0f + vec3(0,6,0) + entity->vel;
		limb->angle += entity->angle;
		limb->position += entity->position + vec3(0,0.1f,0);
		limb->render_index = pair<int,int>(entity->render_index.first + render_off, limb->vecs.num_verts/6);
		render_off += limb->render_index.second;
		entity->world->summon(limb);
		limb->drop_ticks();
		fall_apart(limb);
	}
	entity->limbs.clear();
	cout << entity << endl;
}

Mob::Mob(World* nworld, istream& ifile): NamedEntity(nworld, ifile) {
	ifile >> name;
	
	MobData* data = mobstorage->mobdata[name];
	box1 = data->box1;
	box2 = data->box2;
	blockpos = data->blockpos;
	
	ifstream blockifile(entitystorage->blocks[data->blockpath], ios::binary);
  int scale;
  blockifile >> scale;
  char buff;
  blockifile.read(&buff,1);
  block.block = Block::from_file(blockifile, 0, 0, 0, scale, nullptr, nullptr);
	
	int size;
	ifile >> size;
	for ( int i = 0; i < size; i ++) {
		limbs.push_back( Mob::from_file(nworld, ifile) );
	}
}


void Mob::to_file(ostream& ofile) {
	ofile << name << endl;
	DisplayEntity::to_file(ofile);
	ofile << name << endl;
	ofile << limbs.size() << endl;
	for (DisplayEntity* limb : limbs) {
		limb->to_file(ofile);
	}
}


Mob* Mob::from_file(World* nworld, istream& ifile) {
	string name;
	ifile >> name;
	if (name == "skeleton") return new Skeleton(nworld, ifile);
	if (name == "pig") return new Pig(nworld, ifile);
	
	return new Mob(nworld, ifile);
}



Pig::Pig(World* nworld, vec3 start_pos): Mob(nworld, start_pos, "pig"), next_point(start_pos) {
	
}

Pig::Pig(World* nworld, istream& ifile): Mob(nworld, ifile), next_point(position) {
	
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



Biped::Biped(World* nworld, vec3 start_pos, string name): Mob(nworld, start_pos, name), next_point(start_pos) {
	
}

Biped::Biped(World* nworld, istream& ifile): Mob(nworld, ifile) {
	
}

void Biped::on_timestep() {
	Mob::on_timestep();
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
			dist /= glm::length(vec2(dist.x, dist.z));
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
			for (int i = 0; i < 2; i ++) {
				limbs[i]->angle.y = (timer - 0.5) * b;
				b *= -1;
			}
		}
	}
}




Skeleton::Skeleton(World* nworld, vec3 start_pos): Biped(nworld, start_pos, "skeleton"), attack_recharge(0) {
	
}

Skeleton::Skeleton(World* nworld, istream& ifile): Biped(nworld, ifile), attack_recharge(0) {
	
}

void Skeleton::on_timestep() {
	Biped::on_timestep();
	vec3 dist_to_player = world->player->position - (position + box2 / 2.0f);
	if (glm::length(dist_to_player) < 50) {
		next_point = world->player->position;
	}
	if (attack_recharge > 0) {
		attack_recharge -= 0.01;
	}
	limbs[2]->angle.y = attack_recharge;
	limbs[3]->angle.y = attack_recharge;
	if (colliding(world->player) and attack_recharge <= 0) {
    world->player->health -= 1;
    world->player->vel += (dist_to_player + vec3(0,1,0)) * 5.0f;
    vel -= (dist_to_player + vec3(0,1,0)) * 3.0f;
		attack_recharge = 1;
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
