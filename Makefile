
PLUGINS := $(patsubst %/Makefile, %, $(wildcard */Makefile))

all: $(PLUGINS)

$(PLUGINS):
	$(MAKE) -C $@

scrolls:
multithreading: scrolls
singleplayer: scrolls
alaudio: scrolls
glgraphics: scrolls
dwarfdebug: scrolls

.PHONY: $(PLUGINS)
