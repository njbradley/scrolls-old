#ifndef SCROLLS_FILEFORMAT
#define SCROLLS_FILEFORMAT

#include "classes.h"

class FileFormat { public:
	
	static void write_fixed(ostream& ofile, unsigned long int value);
	static void write_fixed(ostream& ofile, unsigned int value);
	static void write_fixed(ostream& ofile, float value);
	static void write_variable(ostream& ofile, unsigned long int value);
	
	static void read_fixed(istream& ifile, unsigned long int* value);
	static void read_fixed(istream& ifile, unsigned int* value);
	static void read_fixed(istream& ifile, float* value);
	static unsigned long int read_variable(istream& ifile);
};

#endif
