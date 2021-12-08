#ifndef SCROLLS_FILEFORMAT
#define SCROLLS_FILEFORMAT

#include "classes.h"

/*
This class provides methods to read and write to binary files
*_fixed writes/reads for a specific type and size, which
is why the read_fixed methods require a pointer to be passed in,
so that the exact type and size is known.
*_variable reads/writes a variable length integer. This means it
will only take up the bytes it needs, for example 5 would only take 1
byte but 289 would take 2 bytes. this is useful
when most values are small.
*/

class FileFormat { public:
	
	static void write_fixed(ostream& ofile, uint64 value);
	static void write_fixed(ostream& ofile, uint32 value);
	static void write_fixed(ostream& ofile, float value);
	static void write_variable(ostream& ofile, uint64 value);
	
	static void read_fixed(istream& ifile, uint64* value);
	static void read_fixed(istream& ifile, uint32* value);
	static void read_fixed(istream& ifile, float* value);
	static uint64 read_variable(istream& ifile);
};

#endif
