#include "settings.h"
#include "scrolls/cross-platform.h"

DEFINE_PLUGIN(Settings);
EXPORT_PLUGIN(Settings);

double Settings::aspect_ratio() {
	return screen_dims.x / double(screen_dims.y);
}


Settings::Settings() {
	load_settings();
}

void Settings::load_settings() {
	ifstream ifile("settings.txt");
	if (ifile.good()) {
		string name;
		while (!ifile.eof()) {
			getline(ifile, name, ':');
			if (name == "fov") {
				ifile >> fov;
			} else if (name == "fullscreen") {
				ifile >> name;
				fullscreen = name == "on";
			} else if (name == "max_fps") {
				ifile >> max_fps;
			} else if (name == "framerate_sync") {
				string sync;
				ifile >> sync;
				framerate_sync = sync == "on";
			} else if (name == "view_dist") {
				ifile >> view_dist;
			} else if (name == "alloc_memory") {
				ifile >> allocated_memory;
			} else if (name == "dims") {
				ifile >> screen_dims.x >> screen_dims.y;
			} else if (name == "") {
				continue;
			} else {
				cout << "error reading settings file" << endl;
				cout << "no setting '" << name << "'" << endl;
				exit(1);
			}
			getline(ifile, name);
		}
	} else {
		ofstream ofile("settings.txt");
		ofile << "fov: 90" << endl;
		ofile << "fullscreen: true" << endl;
		ofile << "dims: 1600 900" << endl;
		ofile << "max_fps: 90" << endl;
		ofile << "framerate_sync: on" << endl;
		ofile << "alloc_memory: 70" << endl;
		ofile << "view_dist: 3" << endl;
		load_settings();
	}
}

Plugin<Settings> settings;
