#ifndef MATERIALS_PREDEF
#define MATERIALS_PREDEF

#include "classes.h"

class Material { public:
	string name;
	double toughness;
	double elastic;
	
	Material(istream& ifile);
	// calculates a score based on the outlined impact. The score will be
	// positive if the other material is stronger. if the score is the same
	// as the other toughness, the other material breaks
	double collision_score(Material* other, double sharpness, double force);
};

class MaterialStorage { public:
	unordered_map<string,Material*> materials;
	MaterialStorage();
	~MaterialStorage();
};

MaterialStorage* matstorage;

#endif
