TARGET := tests
SCROLLSDEPS := scrolls
ISDLL := TRUE
EXTRALIBS :=

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS)))
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

include ../build.mk
