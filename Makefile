
PLUGINS := $(patsubst %/Makefile, %, $(wildcard */Makefile))

all: $(PLUGINS)

$(PLUGINS):
	$(MAKE) -C $@

.PHONY: $(PLUGINS)
