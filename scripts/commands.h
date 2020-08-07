#ifndef COMMANDS
#define COMMANDS

#include "commands-predef.h"
#include "world-predef.h"
#include "blocks-predef.h"
#include "mobs-predef.h"

CommandFunc throwerror(string message) {
	return [=](World* world, ostream& console) {console << message << endl; };
}

CommandFunc printmessage(string message) {
	return [=](World* world, ostream& console) {console << message << endl; };
}

Command::Command(istream& ifile) {
	string id;
	//ifile >> id;
	char lett;
	while ((lett = ifile.get()) != ' ' and lett != '(' and lett != '=' and !ifile.eof()) id.push_back(lett);
	if (ifile.eof()) {
		func = throwerror("you must preform an assignment or function call");
	} else {
		string mode;
		if (lett != ' ') {
			mode.push_back(lett);
	  } else {
			mode.push_back(ifile.get());
		}
		if (ifile.peek() == '=') mode.push_back(ifile.get());
		
		if (id == "world.summon") {
			if (mode == "(") {
				string name;
				vec3 pos;
				ifile >> pos.x >> lett >> pos.y >> lett >> pos.z >> lett;
				while ((ifile >> lett) and lett != ')') name.push_back(lett);
				
				if (name == "skeleton") func = [=](World* world, ostream& console) {world->summon(new Skeleton(world, pos)); };
				else if (name == "pig") func = [=](World* world, ostream& console) {world->summon(new Pig(world, pos)); };
				else if (name == "ghost") func = [=](World* world, ostream& console) {world->summon(new Ghost(world, pos)); };
				else if (name == "ent") func = [=](World* world, ostream& console) {world->summon(new Ent(world, pos)); };
				else if (name == "glofly") func = [=](World* world, ostream& console) {world->summon(new Glofly(world, pos)); };
				else func = [=](World* world, ostream& console) {world->summon(new Mob(world, pos, name)); };
			} else {
				func = throwerror("summon is a function");
			}
		} else if (id == "world.setblock") {
			if (mode == "(") {
				ivec3 pos;
				string name;
				int direction;
				ifile >> pos.x >> lett >> pos.y >> lett >> pos.z >> lett;
				while ((ifile >> lett) and lett != ',') name.push_back(lett);
				ifile >> direction;
				char btype = blocks->names[name];
				func = [=](World* world, ostream& console) {world->set(pos.x, pos.y, pos.z, btype, direction); };
			} else {
				func = throwerror("setblock is a function");
			}
		} else if (id == "world.time") {
			if (mode == "(") {
				func = [](World* world, ostream& console) {console << world->daytime; };
			} else if (mode == "=") {
				double time;
				ifile >> time;
				func = [=](World* world, ostream& console) {world->daytime = time; };
			} else if (mode == "+=") {
				double time;
				ifile >> time;
				func = [=](World* world, ostream& console) {world->daytime += time; };
			} else {
				func = throwerror("world.time is not a function");
			}
		} else if (id == "me.pos") {
			if (mode == "(") {
				func = [](World* world, ostream& console) {print(world->player->position); };
			} else if (mode == "=") {
				vec3 pos;
				ifile >> pos.x >> lett >> pos.y >> lett >> pos.z;
				func = [=](World* world, ostream& console) {world->player->position = pos; };
			} else if (mode == "+=") {
				vec3 pos;
				ifile >> pos.x >> lett >> pos.y >> lett >> pos.z;
				func = [=](World* world, ostream& console) {world->player->position += pos; };
			} else {
				func = throwerror("pos is not a function");
			}
		} else {
			func = throwerror("that is not a valid command");
		}
	}
}
			


#endif
