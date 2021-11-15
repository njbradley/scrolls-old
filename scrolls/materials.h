#ifndef MATERIALS_PREDEF
#define MATERIALS_PREDEF

#include "classes.h"
#include "audio.h"

struct MaterialParams {
	string name;
	double toughness = 0;
	double elastic = 0;
	string breaksound = "null";
	string hitsound = "null";
	float density = 1;
	float restitution = 0.2;
	float friction = 0.4;
};

class Material : public MaterialParams { public:
	BASE_PLUGIN_HEAD(Material, ());
	
	Material(MaterialParams&& data): MaterialParams(data) {}
	// calculates a score based on the outlined impact. The score will be
	// positive if the other material is stronger. if the score is the same
	// as the other toughness, the other material breaks
	virtual double damage(Material* other, double sharpness, double force);
	virtual double dig_time(Material* other, double sharpness, double force);
	virtual double material_score(Material* other);
	virtual double collision_force(Material* other, double sharpness, double force);
};

extern Storage<Material> materialstorage;

namespace materials {
	extern Material dirt;
	extern Material stone;
	extern Material wood;
	extern Material leaves;
}

#endif
