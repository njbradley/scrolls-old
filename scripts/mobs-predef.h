#ifndef MOBS_PREDEF
#define MOBS_PREDEF

#include "classes.h"
#include "entity-predef.h"

class MobData { public:
	string name;
	int health;
	vec3 box1;
	vec3 box2;
	string blockpath;
	vec3 blockpos;
	vector<pair<vec3,string> > limbs;
	MobData(istream& ifile);
};

class MobStorage { public:
	map<string,MobData*> mobdata;
	MobStorage();
};

MobStorage* mobstorage;

class Mob: public NamedEntity { public:
	// Entity* target;
	string name;
	// ivec3 last_point;
	Mob(World* world, vec3 start_pos, string newname);
	Mob(World* world, istream& ifile);
	virtual void on_timestep();
	virtual void to_file(ostream& ofile);
	static void fall_apart(DisplayEntity* entity);
	static Mob* from_file(World* nworld, istream& ifile);
	// virtual void tick();
	// virtual void find_target();
	// virtual void find_next_point();
private:
	vector<DisplayEntity*> get_limbs(World* world, string name);
};

class Pig: public Mob { public:
	ivec3 next_point;
	Pig(World* world, vec3 start_pos);
	Pig(World* nworld, istream& ifile);
	virtual void on_timestep();
};

class Biped: public Mob { public:
	ivec3 next_point;
	Biped(World* nworld, vec3 start_pos, string name);
	Biped(World* nworld, istream& ifile);
	virtual void on_timestep();
};

class Skeleton: public Biped { public:
	float attack_recharge;
	Skeleton(World* nworld, vec3 start_pos);
	Skeleton(World* nworld, istream& ifile);
	virtual void on_timestep();
};

#endif
