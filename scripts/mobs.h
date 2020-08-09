#ifndef MOBS
#define MOBS

#include "mobs-predef.h"

MobData::MobData(istream& ifile): box1(-1,-1,-1), box2(1,1,1), blockpos(-1,-1,-1), health(5), emitted_light(0) {
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
      } else if (varname == "health") {
				ifile >> health;
      } else if (varname == "emitted_light") {
				ifile >> emitted_light;
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
		MobData* data = mobstorage->mobdata[newname];
		health = data->health;
		max_health = data->health;
		emitted_light = data->emitted_light;
	// cout << "mob summoned init " << this << " pos ";
	// print (position);
}

vector<DisplayEntity*> Mob::get_limbs(World* world, string name) {
	vector<DisplayEntity*> list;
	for (pair<vec3,string> limb : mobstorage->mobdata[name]->limbs) {
		list.push_back(new Mob(world, limb.first, limb.second));
	}
	return list;
}

void Mob::on_timestep(double deltatime) {
	DisplayEntity::on_timestep(deltatime);
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
			//cout <<  this << endl;
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
	//cout << entity << endl;
}

Mob::Mob(World* nworld, istream& ifile): NamedEntity(nworld, ifile) {
	ifile >> name;
	
	// cout << "mob from file " << this << " pos ";
	// print (position);
	
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


void Mob::to_file(ostream& ofile) { //ERR: mobs are being saved with 'nan' as their positions, maybe they are being deleted at the same time?
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
	if (name == "ent") return new Ent(nworld, ifile);
	if (name == "ghost") return new Ghost(nworld, ifile);
	if (name == "glofly") return new Glofly(nworld, ifile);
	
	return new Mob(nworld, ifile);
}



Pig::Pig(World* nworld, vec3 start_pos): Mob(nworld, start_pos, "pig"), next_point(start_pos) {
	
}

Pig::Pig(World* nworld, istream& ifile): Mob(nworld, ifile), next_point(position) {
	
}

void Pig::on_timestep(double deltatime) {
	Mob::on_timestep(deltatime);
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
			if (glm::length(dist) > 0.1) {
				dist /= glm::length(dist);
			}
			angle.x = lon;
		  if (consts[4]) {
		    vel.x += 5 * dist.x * deltatime;
		    vel.z += 5 * dist.z * deltatime;
		  } else {
		    vel.x += 0.5 * dist.x * deltatime;
		    vel.z += 0.5 * dist.z * deltatime;
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

void Biped::on_timestep(double deltatime) {
	Mob::on_timestep(deltatime);
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
			if (glm::length(vec2(dist.x, dist.z)) > 0.1) {
				dist /= glm::length(vec2(dist.x, dist.z));
			}
			angle.x = lon;
		  if (consts[4]) {
				vel.x += 5 * dist.x * deltatime;
		    vel.z += 5 * dist.z * deltatime;
		  } else {
		    vel.x += 0.5 * dist.x * deltatime;
		    vel.z += 0.5 * dist.z * deltatime;
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

void Skeleton::on_timestep(double deltatime) {
	Biped::on_timestep(deltatime);
	vec3 dist_to_player = world->player->position - (position + box2 / 2.0f);
	if (glm::length(dist_to_player) < 50) {
		next_point = world->player->position;
	}
	if (attack_recharge > 0) {
		attack_recharge -= deltatime;
	}
	limbs[2]->angle.y = attack_recharge;
	limbs[3]->angle.y = attack_recharge;
	if (colliding(world->player) and attack_recharge <= 0) {
    world->player->health -= 1;
    world->player->vel += (dist_to_player + vec3(0,1,0)) * 2.0f;
    vel -= (dist_to_player + vec3(0,1,0)) * 2.0f;
		attack_recharge = 1;
  }
}





FlyingMob::FlyingMob(World* nworld, vec3 start_pos, string newname): Mob(nworld, start_pos, newname), next_point(start_pos) {
	flying = true;
}

FlyingMob::FlyingMob(World* nworld, istream& ifile): Mob(nworld, ifile), next_point(position) {
	flying = true;
}

void FlyingMob::random_point() {
	next_point = ivec3(position) + ivec3(rand()%10-5, rand()%10-5, rand()%10-5);
}

void FlyingMob::on_timestep(double deltatime) {
	Mob::on_timestep(deltatime);
	// if (glm::length(world->player->position - vec3(position)) < 20) {
	// 	target = world->player;
	// } else if (glm::length(target->position - vec3(position)) > 30) {
	// 	target = nullptr;
	// }
	if (target != nullptr) {
		next_point = target->position;
	} else if (glm::length(position-vec3(next_point)) < 1 or glm::length(position-vec3(next_point)) > 50 or rand()%400 == 0) {
		random_point();
	}
	vec3 to_target = vec3(next_point) - position;
	if (glm::length(to_target) > 0.05) {
		to_target /= glm::length(to_target);
	}
	angle.x = atan2(to_target.z, to_target.x);
	angle.y = atan2(to_target.y, glm::length(vec2(to_target.x, to_target.z)));
	vel += to_target * 25.0f * float(deltatime);
	vel *= 0.95;
}





Ghost::Ghost(World* nworld, vec3 start_pos): FlyingMob(nworld, start_pos, "ghost") {
	
}

Ghost::Ghost(World* nworld, istream& ifile): FlyingMob(nworld, ifile) {
	
}

void Ghost::on_timestep(double deltatime) {
	FlyingMob::on_timestep(deltatime);
}






Glofly::Glofly(World* nworld, vec3 start_pos): FlyingMob(nworld, start_pos, "glofly") {
	
}

Glofly::Glofly(World* nworld, istream& ifile): FlyingMob(nworld, ifile) {
	
}

void Glofly::on_timestep(double deltatime) {
	FlyingMob::on_timestep(deltatime);
	
}

void Glofly::random_point() {
	next_point = ivec3(position) + ivec3(rand()%10-5, rand()%10-5, rand()%10-5);
	int i = 0;
	while (i < 20 and world->get(next_point.x, next_point.y - i - 10, next_point.z) == 0) {
		i ++;
	}
	next_point.y -= i;
	i = 0;
	while (i < 20 and world->get(next_point.x, next_point.y + i, next_point.z) != 0) {
		i ++;
	}
	next_point.y += i;
}










Ent::Ent(World* nworld, vec3 start_pos): Mob(nworld, start_pos, "ent") {
	
}

Ent::Ent(World* nworld, istream& ifile): Mob(nworld, ifile) {
	
}

void Ent::on_timestep(double deltatime) {
	Mob::on_timestep(deltatime);
	if (target == nullptr) {
		if (grab_angle > 0) {
			grab_angle -= 3.14/2 * 0.5 * deltatime;
		} else if (glm::length(world->player->position - position) < 10) {
			target = world->player;
			vec3 to_target = target->position - position;
			double lon =  atan2(to_target.z, to_target.x);
			angle.x = lon;
		}
	} else {
		if (grab_angle < 3.14*3/4) {
			if (!grabbed) {
				grab_angle += 3.14*3/4 * 2 * deltatime;
			}
		} else {
			vec3 to_target = target->position - position;
			double lon =  atan2(to_target.z, to_target.x);
			if (glm::length(target->position - position) < 12 and std::abs(angle.x - lon) < 3.14/6) {
				grabbed = true;
				force = 20;
				target->damage(0);
			} else {
				target = nullptr;
			}
		}
		
		if (grabbed) {
			vec3 tmp(-1 * sin(-grab_angle * 0.5), cos(-grab_angle * 0.5), 0);
			vec3 posoff(0,0,0);
			
			posoff.x = tmp.x * cos(angle.x) - tmp.z * sin(angle.x);
			posoff.y = tmp.y;
			posoff.z = tmp.x * sin(angle.x) + tmp.z * cos(angle.x);
			
			target->position = position + posoff * 9.0f;
			target->vel.y = 0;
			vec3 targetvel = target->vel;
			target->vel = vec3(0,0,0);
			if (attack_recharge <= 0) {
				target->damage(0);
				if (target->health <= 0) {
					target = nullptr;
					grabbed = false;
				}
				attack_recharge = 1;
			}
			if (grab_angle > 0) {
				grab_angle -= 3.14/2 * 0.5 * deltatime;
			} else {
				grab_angle = 0;
			}
			force -= glm::length(targetvel);
			if (force < 0) {
				target->vel = targetvel * 200.0f + vec3(0,10,0);
				grabbed = false;
				target = nullptr;
			}
		}
	}
	double lat = -grab_angle * 0.5;
	angle.y = lat*2/3;
	limbs[0]->angle.y = lat;
	for (DisplayEntity* limb : limbs[0]->limbs) {
		limb->angle.y = lat/2;
	}
	if (attack_recharge > 0) {
		attack_recharge -= deltatime;
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
