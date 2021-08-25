TARGET := dwarfdebug
SCROLLSDEPS := scrolls
ISDLL := TRUE
EXTRALIBS := -ldwarfstack


SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS)))
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

ifeq ($(PLAT),MAC)
fake:
else
ifeq ($(PLAT),LINUX)
fake:
else
include ../build.mk
endif
endif
