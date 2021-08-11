TARGET := dwarfdebug
SCROLLSDEPS := base
ISDLL := TRUE
EXTRALIBS := -ldwarfstack

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS)))
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

include ../build.mk
