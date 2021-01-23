#ifndef GLUE
#define GLUE

#include "glue.h"
#include "materials.h"
#include "cross-platform.h"


ConnectorData::ConnectorData(istream& ifile) {
	string buff;
  ifile >> buff;
  if (buff != "connector") {
    cout << "ERR: this file is not a connector data file " << endl;
  } else {
    string tex_str;
    ifile >> name;
    getline(ifile, buff, '{');
    string varname;
    getline(ifile, buff, ':');
    getline(ifile, varname, ':');
    while (!ifile.eof() and varname != "") {
      if (varname == "texture") {
        ifile >> tex_str;
      } else if (varname == "material") {
        string gluetype;
        ifile >> gluetype;
        material = matstorage->materials[gluetype];
      }
      getline(ifile, buff, ':');
      getline(ifile, varname, ':');
    }
    if (tex_str == "") {
      tex_str = name;
    }
    vector<string> paths;
    get_files_folder("resources/textures/blocks/edges", &paths);
    tex_index = 0;
    for (int i = 0; i < paths.size(); i ++) {
      stringstream ss(paths[i]);
      string filename;
      getline(ss, filename, '.');
      if (filename == tex_str) {
        tex_index = i;
      }
    }
	}
}

ConnectorStorage::ConnectorStorage() {
	vector<string> block_paths;
	get_files_folder("resources/data/connectors", &block_paths);
	for (string filename : block_paths) {
			ifstream ifile("resources/data/connectors/" + filename);
			ConnectorData* conn = new ConnectorData(ifile);
			connectors[conn->name] = conn;
	}
	cout << "loaded " << block_paths.size() << " connector files" << endl;
}

ConnectorStorage::~ConnectorStorage() {
	for (pair<string,ConnectorData*> kv : connectors) {
		delete kv.second;
	}
}

ConnectorStorage* connstorage;

#endif
