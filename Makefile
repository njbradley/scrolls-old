
PLUGINS := $(patsubst %/Makefile, %, $(wildcard */Makefile))

$(PLUGINS):
	$(MAKE) -C $@

.PHONY: $(PLUGINS)
