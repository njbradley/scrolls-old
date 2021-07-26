#include "libraries.h"
#include "plugins.h"
#include "cross-platform.h"

#include <sys/types.h>
#include <sys/stat.h>

PathLib::PathLib(string directory) {
	load(directory);
}

void PathLib::load(string directory) {
	struct stat info;
	for (PluginLib* plugins : pluginloader.plugins) {
		string search_path = plugins->dirname + "/" + directory;
		if (stat(search_path.c_str(), &info) == 0) {
			int i = paths.size();
			get_files_folder(search_path, &paths);
			for (; i < paths.size(); i ++) {
				paths[i] = search_path + '/' + paths[i];
			}
		}
	}
}

int PathLib::add(string path) {
	paths.push_back(path);
	return paths.size() - 1;
}

int PathLib::getindex(string path) const {
	for (int i = 0; i < paths.size(); i ++) {
		if (path == paths[i]) {
			return i;
		}
	}
	return -1;
}

string PathLib::getpath(int index) const {
	return paths[index];
}


PathLib::const_iterator PathLib::begin() const {
	return paths.begin();
}

PathLib::const_iterator PathLib::end() const {
	return paths.end();
}

size_t PathLib::size() const {
	return paths.size();
}






string PathLib::find_path(string path) {
	struct stat info;
	for (PluginLib* plugins : pluginloader.plugins) {
		string search_path = plugins->dirname + "/" + path;
		if (stat(search_path.c_str(), &info) == 0) {
			return search_path;
		}
	}
	cout << "WARN: couldn't find path " << path << endl;
	return "NUL";
}
