#ifndef BASE_LIBRARIES_H
#define BASE_LIBRARIES_H

#include "classes.h"

class PathLib {
	vector<string> paths;
public:
	string directory;
	PathLib(string directory);
	PathLib() {};
	
	void load(string directory);
	virtual int add(string path);
	
	int getindex(string path) const;
	string getpath(int index) const;
	
	typedef vector<string>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;
	size_t size() const;
	
	static string find_path(string path);
};


#endif
