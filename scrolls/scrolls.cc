
#include "plugins.h"

class ScrollsPlugin : public Plugin { public:
	
	ScrollsPlugin(): Plugin("base") {
		cout << "plugin created" << endl;
	}
	
	virtual void init() {
		cout << "plugin init" << endl;
	}
	
};

extern "C" Plugin* load_plugin() {
	return new ScrollsPlugin();
}
