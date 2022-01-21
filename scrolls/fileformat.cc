#include "fileformat.h"

void FileFormat::write_fixed(ostream& ofile, uint64 value) {
	for (int i = 0; i < 8; i ++) {
		ofile.put(value % 256);
		value /= 256;
	}
}

void FileFormat::write_fixed(ostream& ofile, uint32 value) {
	for (int i = 0; i < 4; i ++) {
		ofile.put(value % 256);
		value /= 256;
	}
}

void FileFormat::write_fixed(ostream& ofile, float value) {
	write_fixed(ofile, *((uint32*) &value));
}

void FileFormat::write_fixed(ostream& ofile, ivec3 value) {
	write_fixed(ofile, (uint32) value.x);
	write_fixed(ofile, (uint32) value.y);
	write_fixed(ofile, (uint32) value.z);
}

void FileFormat::write_fixed(ostream& ofile, vec3 value) {
	write_fixed(ofile, value.x);
	write_fixed(ofile, value.y);
	write_fixed(ofile, value.z);
}

void FileFormat::write_variable(ostream& ofile, uint64 value) {
	do {
		ofile.put(((value%128) << 1) | (value/128 > 0));
		value /= 128;
	} while (value != 0);
}



void FileFormat::read_fixed(istream& ifile, uint64* value) {
	*value = 0;
	uint64 multiplier = 1;
	for (int i = 0; i < 8; i ++) {
		*value += ifile.get() * multiplier;
		multiplier *= 256;
	}
}

void FileFormat::read_fixed(istream& ifile, uint32* value) {
	*value = 0;
	uint32 multiplier = 1;
	for (int i = 0; i < 4; i ++) {
		*value += ifile.get() * multiplier;
		multiplier *= 256;
	}
}

void FileFormat::read_fixed(istream& ifile, float* value) {
	read_fixed(ifile, (unsigned int*) value);
}

void FileFormat::read_fixed(istream& ifile, ivec3* value) {
	read_fixed(ifile, (uint32*) &value->x);
	read_fixed(ifile, (uint32*) &value->y);
	read_fixed(ifile, (uint32*) &value->z);
}

void FileFormat::read_fixed(istream& ifile, vec3* value) {
	read_fixed(ifile, &value->x);
	read_fixed(ifile, &value->y);
	read_fixed(ifile, &value->z);
}

uint64 FileFormat::read_variable(istream& ifile) {
	uint64 value = 0;
	uint64 multiplier = 1;
	unsigned char data;
	do {
		data = ifile.get();
		value += (data >> 1) * multiplier;
		multiplier *= 128;
	} while ((data & 0b00000001) and !ifile.eof());
	return value;
}




void FileFormat::write_string(ostream& ofile, string str) {
	write_variable(ofile, str.length());
	ofile.write(str.c_str(), str.length());
}

string FileFormat::read_string(istream& ifile) {
	int len = read_variable(ifile);
	char strbuf[len];
	ifile.read(strbuf, len);
	return string(strbuf);
}
