#ifndef MOBS
#define MOBS

Mob::Mob(World* nworld, vec3 start_pos, string name): NamedEntity(nworld, start_pos, name), target(nullptr), next_point(0,0,0) {
	
}

void Mob::tick() {
	if (target == nullptr) {
		find_target();
	}
}

void Mob::find_target() {
	target = world->player;
}

void Mob::find_next_point() {
	vec3 diff =
}


#endif
