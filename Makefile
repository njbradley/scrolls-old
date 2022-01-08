
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
multiplayer: scrolls
server: scrolls multiplayer
client: scrolls multiplayer singleplayer

.PHONY: $(PLUGINS)
