#ifndef MATERIALS_PREDEF
#define MATERIALS_PREDEF

#include "classes.h"

class Material { public:
	string name;
	double toughness;
	double elastic;
	
	Material(istream& ifile);
	double collision_score(Material* other, double sharpness, double force);
};

class MaterialStorage { public:
	unordered_map<string,Material> materials;
	MaterialStorage();
};

MaterialStorage* matstorage;

#endif
