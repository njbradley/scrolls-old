#include "fileformat.h"

void FileFormat::write_fixed(ostream& ofile, unsigned long int value) {
	for (int i = 0; i < 8; i ++) {
		ofile.put(value % 256);
		value /= 256;
	}
}

void FileFormat::write_fixed(ostream& ofile, unsigned int value) {
	for (int i = 0; i < 4; i ++) {
		ofile.put(value % 256);
		value /= 256;
	}
}

void FileFormat::write_fixed(ostream& ofile, float value) {
	write_fixed(ofile, *((unsigned int*) &value));
}

void FileFormat::write_variable(ostream& ofile, unsigned long int value) {
	do {
		ofile.put(((value%128) << 1) | (value/128 > 0));
		value /= 128;
	} while (value != 0);
}



void FileFormat::read_fixed(istream& ifile, unsigned long int* value) {
	*value = 0;
	unsigned long int multiplier = 1;
	for (int i = 0; i < 8; i ++) {
		*value += ifile.get() * multiplier;
		multiplier *= 256;
	}
}

void FileFormat::read_fixed(istream& ifile, unsigned int* value) {
	*value = 0;
	unsigned int multiplier = 1;
	for (int i = 0; i < 4; i ++) {
		*value += ifile.get() * multiplier;
		multiplier *= 256;
	}
}

void FileFormat::read_fixed(istream& ifile, float* value) {
	read_fixed(ifile, (unsigned int*) value);
}

unsigned long int FileFormat::read_variable(istream& ifile) {
	unsigned long int value = 0;
	unsigned long int multiplier = 1;
	unsigned char data;
	do {
		unsigned char data = ifile.get();
		value += (data >> 1) * multiplier;
		multiplier *= 128;
	} while (data & 0b00000001);
	return value;
}
