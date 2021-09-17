#ifndef TESTS_H
#define TESTS_H

#include "scrolls/classes.h"
#include "scrolls/plugins.h"

struct TestParams {
};

class Test : public TestParams { public:
	PLUGIN_HEAD(Test, ());
	string name;
	int ntests = 1;
	int nrand = 0;
	int nfast = 0;
	int nslow = 0;
	bool result = true;
	
	std::stringstream out;
	std::stringstream errout;
	
	virtual void init() { }
	
	virtual void test() { }
	virtual void random_test(int seed) { }
	virtual void fast_test() { }
	virtual void slow_test() { }
	
	void run_tests();
	void print_summary(ostream& mainout, ostream& mainerr);
};

#define TASSERT(X) if (!(X)) { \
	errout << "	ERR: ASSERT failed: " << __PRETTY_FUNCTION__ << ": " << __FILE__ << " at line " << __LINE__ \
	<< endl << "		'" #X "' was false" << endl; \
	result = false; \
}

#define TASSERT_NOT(X) if (X) { \
	errout << "	ERR: ASSERT_NOT failed: " << __PRETTY_FUNCTION__ << ": " << __FILE__ << " at line " << __LINE__ \
	<< endl << "		'" #X "' was true" << endl; \
	result = false; \
}

#define TASSERT_EQ(VAL,X) { auto x = X; if (x != VAL) { \
	errout << "	ERR: ASSERT_EQ failed: " << __PRETTY_FUNCTION__ << ": " __FILE__ << " at line " << __LINE__ \
	<< endl << "		'" #X "' != '" << VAL << "', was '" << x << "'" << endl; \
	result = false; \
}}


#endif
