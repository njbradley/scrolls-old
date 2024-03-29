#ifndef MATERIALS
#define MATERIALS

#include "materials.h"
#include "cross-platform.h"

DEFINE_PLUGIN(Material);

Storage<Material> materialstorage;


double Material::material_score(Material* other) {
	double score = (toughness - other->toughness);
	return score;
}

double Material::damage(Material* other, double sharpness, double force) {
	// cout << "damage calc" << endl;
	double damage_taken = force;
	double elastic_buff = (elastic - sharpness);
	if (elastic_buff > 0) {
		damage_taken -= elastic_buff;
	}
	if (damage_taken < 0) {
		damage_taken = 0;
	}
	// cout << "damage " << damage_taken << endl;
	double score = material_score(other);
	// cout << "score " << score << endl;
	double multiplier = 1/(1+exp(0.1*score));
	// cout << "multiplier " << multiplier << endl;
	return damage_taken * multiplier;
}

double Material::dig_time(Material* other, double sharpness, double force) {
	// cout << name << " t" << toughness << " e" << elastic << ' ';
	// cout << other->name << " t" << other->toughness << " e" << other->elastic << endl;
	// cout << 's' << sharpness << " f" << force << endl;
	
	
	double score = material_score(other);
	// cout << "score " << score << endl;
	double multiplier = 1/(1+exp(0.1*score));
	// cout << "multiplier " << multiplier << endl;
	
	multiplier = (1-multiplier) * 2;
	
	double tough_diff = toughness - force;
	double elastic_diff = elastic - sharpness;
	
	if (elastic_diff < 0) elastic_diff = -1 + std::pow(0.5, -elastic_diff);
	if (tough_diff < 0) tough_diff = -1 + std::pow(0.5,-tough_diff);
	
	// cout << elastic_diff << ' ' << tough_diff << endl;
	
	double total = elastic_diff + tough_diff;
	
	double time = std::pow(1.5,total);
	// cout << "time " << time << endl;
	return time;
	
	// double def_elastic_percent = elastic / (elastic + toughness);
	// double attack_sharp_percent = sharpness / (sharpness + force);
	// cout << def_elastic_percent << ' ' << attack_sharp_percent << endl;
	// double matching_score = 1 - std::abs(def_elastic_percent - attack_sharp_percent);
	// cout << matching_score << endl;
	// double damage_taken = damage(other, sharpness, force) / toughness;
	// cout << damage_taken << endl;
	// double time = 1 / (damage_taken * matching_score);
	// cout << "time " << time << endl;
	// return time;
}

double Material::collision_force(Material* other, double sharpness, double force) {
	//cout << "mat coll force " << elastic << ' ' << other->elastic << ' ' << sharpness << endl;
	//cout << force << endl;
	double absorbed_force = std::max(elastic + other->elastic - sharpness, 0.0);
	force -= absorbed_force;
	//cout << force << endl;
	return force;
}




namespace materials {
	Material dirt ({
		.name = "dirt",
		.hitsound = "dirt-hit",
		.density = 4,
		.restitution = 0.0,
	});
	
	Material stone ({
		.name = "stone",
		.density = 22,
		.restitution = 0.0,
	});
	
	Material wood ({
		.name = "wood",
		.density = 2,
		.restitution = 0.2,
	});
	
	Material leaves ({
		.name = "leaves",
		.hitsound = "sand-hit",
		.density = 0.2,
		.restitution = 0.3,
	});
	
	EXPORT_PLUGIN_SINGLETON(&dirt);
	EXPORT_PLUGIN_SINGLETON(&stone);
	EXPORT_PLUGIN_SINGLETON(&wood);
	EXPORT_PLUGIN_SINGLETON(&leaves);
}






#endif
