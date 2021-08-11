#ifndef MATERIALS_PREDEF
#define MATERIALS_PREDEF

#include "classes.h"
#include "audio.h"

class Material { public:
	PLUGIN_HEAD(Material, ());
	string name;
	double toughness;
	double elastic;
	string breaksound;
	string hitsound;
	float density;
	
	// calculates a score based on the outlined impact. The score will be
	// positive if the other material is stronger. if the score is the same
	// as the other toughness, the other material breaks
	virtual double damage(Material* other, double sharpness, double force);
	virtual double dig_time(Material* other, double sharpness, double force);
	virtual double material_score(Material* other);
	virtual double collision_force(Material* other, double sharpness, double force);
};

class MaterialStorage : public Storage<Material> { public:
	PLUGIN_HEAD(MaterialStorage, ());
	MaterialStorage() {}
	virtual ~MaterialStorage() {}
};

extern Plugin<MaterialStorage> materialstorage;

#endif
