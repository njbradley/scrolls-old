#ifndef COMMANDS
#define COMMANDS

#include "commands-predef.h"
#include "world-predef.h"
#include "blocks-predef.h"
#include "mobs-predef.h"

CommandFunc throwerror(string message) {
	return [=](World* world, ostream& console) {console << message << endl; };
}

Command::Command(istream& ifile) {
	string id;
	ifile >> id;
	
	if (id == "summon") {
		string name;
		vec3 pos;
		ifile >> pos.x >> pos.y >> pos.z >> name;
		if (name == "skeleton") func = [=](World* world, ostream& console) {world->summon(new Skeleton(world, pos)); };
		else if (name == "pig") func = [=](World* world, ostream& console) {world->summon(new Pig(world, pos)); };
		else func = throwerror("that is not an entity name");
	} else if (id == "setblock") {
		ivec3 pos;
		string name;
		int direction;
		ifile >> pos.x >> pos.y >> pos.z >> name >> direction;
		char btype = blocks->names[name];
		func = [=](World* world, ostream& console) {world->set(pos.x, pos.y, pos.z, btype, direction); };
	} else if (id == "time") {
		double time;
		ifile >> time;
		func = [=](World* world, ostream& console) {world->daytime = time; };
	} else {
		func = throwerror("that is not a valid command");
	}
}
			


#endif
