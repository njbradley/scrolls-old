#ifndef CRAFTING
#define CRAFTING

#include "crafting-predef.h"

Recipe::Recipe(istream& ifile): input(ifile), output(ifile) {
	ifile >> level;
}

bool Recipe::display(ItemContainer* in, ItemContainer* out) {
	for (ItemStack is : input.items) {
		if (!in->contains(is)) {
			return false;
		}
	}
	if (out == nullptr) {
		return true;
	}
	for (ItemStack is : output.items) {
		out->add(is);
	}
	return true;
}

bool Recipe::craft(ItemContainer* in, ItemContainer* out) {
	if (display(in, out)) {
		for (ItemStack is : input.items) {
			in->take(is);
		}
		return true;
	}
	return false;
}



RecipeStorage::RecipeStorage() {
		vector<string> recipe_paths;
		get_files_folder("resources/data/recipes", &recipe_paths);
		for (string filename : recipe_paths) {
				ifstream ifile("resources/data/recipes/" + filename);
				//cout << "loading recipe " << filename << " from file" << endl;
				recipes.push_back(new Recipe(ifile));
		}
		cout << "loaded " << recipe_paths.size() << " recipe files" << endl;
}

RecipeStorage::~RecipeStorage() {
	for (Recipe* recipe : recipes) {
		delete recipe;
	}
}


#endif
