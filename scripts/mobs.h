#ifndef MOBS_PREDEF
#define MOBS_PREDEF

#include "classes.h"
#include "entity.h"

class MobData { public:
	string name;
	int health;
	vec3 box1;
	vec3 box2;
	string blockpath;
	vec3 blockpos;
	int emitted_light;
	vector<pair<vec3,string> > limbs;
	MobData(istream& ifile);
};

class MobStorage { public:
	map<string,MobData*> mobdata;
	MobStorage();
};

extern MobStorage* mobstorage;

class Mob: public NamedEntity { public:
	// Entity* target;
	string name;
	// ivec3 last_point;
	Mob(World* world, vec3 start_pos, string newname);
	Mob(World* world, istream& ifile);
	virtual void on_timestep(double deltatime);
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
	virtual void on_timestep(double deltatime);
};

class Biped: public Mob { public:
	ivec3 next_point;
	Biped(World* nworld, vec3 start_pos, string name);
	Biped(World* nworld, istream& ifile);
	virtual void on_timestep(double deltatime);
};

class Skeleton: public Biped { public:
	float attack_recharge;
	Skeleton(World* nworld, vec3 start_pos);
	Skeleton(World* nworld, istream& ifile);
	virtual void on_timestep(double deltatime);
};

class FlyingMob: public Mob { public:
	ivec3 next_point;
	Entity* target = nullptr;
	FlyingMob(World* nworld, vec3 start_pos, string newname);
	FlyingMob(World* nworld, istream& ifile);
	virtual void on_timestep(double deltatime);
	virtual void random_point();
};

class Ghost: public FlyingMob { public:
	float attack_recharge = 0;
	Ghost(World* nworld, vec3 start_pos);
	Ghost(World* nworld, istream& ifile);
	virtual void on_timestep(double deltatime);
};

class Glofly: public FlyingMob { public:
	Glofly(World* nworld, vec3 start_pos);
	Glofly(World* nworld, istream& ifile);
	virtual void on_timestep(double deltatime);
	virtual void random_point();
};

class Ent: public Mob { public:
	double grab_angle = 0;
	bool grabbed = false;
	double force = 0;
	Entity* target = nullptr;
	double attack_recharge = 0;
	Ent(World* nworld, vec3 start_pos);
	Ent(World* nworld, istream& ifile);
	virtual void on_timestep(double deltatime);
};

#endif
