TARGET := multithreading
SCROLLSDEPS := scrolls
ISDLL := TRUE

ifeq ($(PLAT),LINUX)
EXTRALIBS := -pthread -lrt
endif

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS)))
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

include ../build.mk
