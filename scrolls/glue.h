#ifndef GLUE_PREDEF
#define GLUE_PREDEF

#include "classes.h"
#include <map>

class ConnectorData { public:
	string name;
	Material* material;
	int tex_index;
	ConnectorData(istream& ifile);
};

class ConnectorStorage { public:
	std::map<string,ConnectorData*> connectors;
	ConnectorStorage();
	~ConnectorStorage();
};

extern ConnectorStorage* connstorage;

#endif
