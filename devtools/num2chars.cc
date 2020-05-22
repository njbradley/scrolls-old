#include <iostream>
#include <fstream>
#include <string>

using std::string;
using std::cout;
using std::endl;

void help() {
	cout << "usage: num2char in-file.txt out-file.txt" << endl;
	cout << "this command goes through a text file, and turns every #<num>" << endl;
	cout << "into the char equivalent, ie #65 > A" << endl;
}

void convert(std::istream & inp, std::ostream & out) {
	string buff;
	getline(inp,buff,'#');
	while (!inp.eof()) {
		out << buff;
		int num;
		inp >> num;
		out << char(num);
		getline(inp,buff,'#');
	}
}
		
		

int main(int numargs, const char** args) {
	if (numargs != 3) {
		cout << "wrong number of params" << endl;
		help();
		return 1;
	} else {
		std::ifstream ifile(args[1]);
		std::ofstream ofile(args[2]);
		if (!ifile.good()) {
			cout << "file " << args[1] << " doesnt exist" << endl;
			return 1;
		}
		if (!ofile.good()) {
			cout << "file " << args[2] << " doesnt exist" << endl;
			return 1;
		}
		convert(ifile, ofile);
	}
	return 0;
}
