#ifndef SCROLLS_FILEFORMAT
#define SCROLLS_FILEFORMAT

#include "classes.h"

class FileFormat { public:
	
	static void write_fixed(ostream& ofile, uint64 value);
	static void write_fixed(ostream& ofile, uint32 value);
	static void write_fixed(ostream& ofile, float value);
	static void write_variable(ostream& ofile, uint64 value);
	
	static void read_fixed(istream& ifile, uint64* value);
	static void read_fixed(istream& ifile, uint32* value);
	static void read_fixed(istream& ifile, float* value);
	static unsigned long int read_variable(istream& ifile);
};

#endif
