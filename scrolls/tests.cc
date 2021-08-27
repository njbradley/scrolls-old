#include "tests.h"

DEFINE_PLUGIN(Test);

void Test::run_tests() {
	for (int i = 0; i < ntests; i ++) {
		test();
	}
	
	for (int i = 0; i < nrand; i ++) {
		random_test(i);
	}
	
	for (int i = 0; i < nfast; i ++) {
		fast_test();
	}
	
	for (int i = 0; i < nslow; i ++) {
		slow_test();
	}
}

void Test::print_summary(ostream& mainout, ostream& mainerr) {
	mainout << name << ": " << (result ? "true" : "false") << endl;
	mainout << out.rdbuf();
	if (!result) {
		mainerr << "Test " << name << " failed:" << endl;
	}
	mainerr << errout.rdbuf();
}
