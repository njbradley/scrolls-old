#ifndef MATERIALS
#define MATERIALS

#include "materials-predef.h"
#include "cross-platform.h"

Material::Material(istream& ifile): toughness(0), elastic(0) {
	string buff;
	ifile >> buff;
	if (buff == "material") {
		ifile >> name;
		string varname;
		getline(ifile, buff, ':');
		getline(ifile, varname, ':');
		while (!ifile.eof() and varname != "") {
			if (varname == "toughness") {
				ifile >> toughness;
			} else if (varname == "elastic") {
				ifile >> elastic;
			}
			getline(ifile, buff, ':');
			getline(ifile, varname, ':');
		}
	} else {
		cout << "ERR: this is not a material file" << endl;
	}
}

double Material::collision_score(Material* other, double sharpness, double force) {
	return std::min(toughness - force, elastic - sharpness);
}




MaterialStorage::MaterialStorage() {
	vector<string> paths;
	get_files_folder("resources/data/materials", &paths);
	for (string path : paths) {
		ifstream ifile("resources/data/materials/" + path);
		Material mat(ifile);
		materials.emplace(mat.name, mat);
	}
	cout << "sucessfully loaded in " << paths.size() << " material files" << endl;
}

#endif
