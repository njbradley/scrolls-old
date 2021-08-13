TARGET := multithreading
SCROLLSDEPS := base
ISDLL := TRUE

ifeq ($(PLAT),LINUX)
EXTRALIBS := -pthread
endif

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS)))
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

include ../build.mk
