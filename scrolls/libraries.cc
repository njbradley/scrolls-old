#include "libraries.h"
#include "plugins.h"
#include "cross-platform.h"
#include <filesystem>

namespace fs = std::filesystem;


#include <sys/types.h>
#include <sys/stat.h>


LibEntry::LibEntry(string dir, string file): dirname(dir), filename(file) {
	
}

bool LibEntry::matches(string path) const {
	return dirname + filename == path or filename == path;
}

LibEntry::operator string() const {
	return dirname + filename;
}


PathLib::PathLib(string newdirectory): directory(newdirectory) {
	load(directory);
}

void PathLib::load(string newdirectory) {
	directory = newdirectory;
	struct stat info;
	for (PluginLib* plugins : pluginloader.plugins) {
		string search_path = plugins->dirname + "/" + directory;
		if (stat(search_path.c_str(), &info) == 0) {
			int i = paths.size();
			for (fs::path path : fs::recursive_directory_iterator(search_path)) {
				if (fs::is_regular_file(path)) {
					paths.emplace_back(search_path + '/', fs::relative(path, search_path).generic_string());
				}
			}
			// get_files_folder(search_path, &paths);
			// for (; i < paths.size(); i ++) {
				// paths[i] = search_path + '/' + paths[i];
			// }
		}
	}
}

int PathLib::add(string path) {
	paths.emplace_back("", path);
	return paths.size() - 1;
}

int PathLib::getindex(string path) const {
	for (int i = 0; i < paths.size(); i ++) {
		if (paths[i].matches(path)) {
			return i;
		}
	}
	return -1;
}

LibEntry PathLib::getpath(int index) const {
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
