#ifndef CRAFTING_PREDEF
#define CRAFTING_PREDEF

#include "classes.h"

#include "items.h"

class Recipe {
public:
	ItemContainer input;
	ItemContainer output;
	int level;
	Recipe(istream&);
	bool display(ItemContainer* in, ItemContainer* out);
	bool craft(ItemContainer* in, ItemContainer* out);
};

class RecipeStorage {
public:
	vector<Recipe*> recipes;
	RecipeStorage();
	~RecipeStorage();
};

extern RecipeStorage* recipestorage;


#endif
