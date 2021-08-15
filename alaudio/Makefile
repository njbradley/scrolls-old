TARGET := alaudio
SCROLLSDEPS := scrolls
ISDLL := TRUE
EXTRALIBS := -lopenal -lalut

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS)))
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

include ../build.mk
