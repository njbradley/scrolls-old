TARGET := multiplayer
SCROLLSDEPS := scrolls
ISDLL := TRUE

ifeq ($(PLAT),MAC)
EXTRALIBS := -lboost_system
else
ifeq ($(PLAT),LINUX)
EXTRALIBS := -lboost_system
else
EXTRALIBS := -lboost_system-mt -lWs2_32
endif
endif

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS)))
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

include ../build.mk
