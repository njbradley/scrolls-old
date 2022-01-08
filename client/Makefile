TARGET := client
SCROLLSDEPS := scrolls multiplayer singleplayer
ISDLL := TRUE
EXTRALIBS :=

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS)))
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

include ../build.mk
