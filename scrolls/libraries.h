#ifndef BASE_LIBRARIES_H
#define BASE_LIBRARIES_H

#include "classes.h"

class LibEntry { public:
	string dirname;
	string filename;
	
	LibEntry(string dir, string file);
	bool matches(string path) const;
	operator string() const;
};

class PathLib {
	vector<LibEntry> paths;
public:
	PathLib(string directory);
	PathLib() {};
	
	void load(string directory);
	virtual int add(string path);
	
	int getindex(string path) const;
	LibEntry getpath(int index) const;
	
	typedef vector<LibEntry>::const_iterator const_iterator;
	const_iterator begin() const;
	const_iterator end() const;
	size_t size() const;
	
	static string find_path(string path);
};


#endif
