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
	cout << name << " t" << toughness << " e" << elastic << ' ';
	cout << other->name << " t" << other->toughness << " e" << other->elastic << endl;
	
	double toughness_diff = toughness - other->toughness;
	double elastic_diff = elastic - sharpness;
	
	if (elastic_diff < 0) {
		elastic_diff /= 4;
	}
	
	double score = toughness_diff + elastic_diff;
	score *= force;
	
	cout << "score " << score << endl;
	
	return score;
	
	// double score = 0;
	// const double elastic_buff = 3;
	// if (other->elastic > elastic) {
	// 	score += std::min((other->elastic - elastic) * elastic_buff, other->toughness);
	// } else {
	// 	score -= std::min((elastic - other->elastic) * elastic_buff, toughness);
	// }
	// cout << "elastic score " << score << endl;
	// score += (other->toughness - toughness) * 1;
	// cout << "material score " << score << endl;
	// const double sharpness_buff = 1;
	// if (elastic > 0) {
	// 	score *= force * std::min(sharpness / elastic, sharpness_buff);
	// } else {
	// 	score *= force * sharpness_buff;
	// }
	// cout << "final score " << score << endl;
	// return score;
}




MaterialStorage::MaterialStorage() {
	vector<string> paths;
	get_files_folder("resources/data/materials", &paths);
	for (string path : paths) {
		ifstream ifile("resources/data/materials/" + path);
		Material* mat = new Material(ifile);
		materials.emplace(mat->name, mat);
	}
	cout << "sucessfully loaded in " << paths.size() << " material files" << endl;
}

MaterialStorage::~MaterialStorage() {
	for (pair<string,Material*> mat : materials) {
		delete mat.second;
	}
}

#endif
