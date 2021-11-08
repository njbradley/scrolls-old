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
	double density = 0.5;
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
	extern Material leaves;
	extern Material fist;
}

#endif
