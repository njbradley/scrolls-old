TARGET := dwarfdebug
SCROLLSDEPS := base
ISDLL := TRUE
EXTRALIBS := -ldwarfstack

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS)))
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

include ../config.mk

LIBS := $(LIBS) $(EXTRALIBS)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OPT) $(OBJECTS) -o $(TARGET)$(EXESUFFIX) $(LDFLAGS) $(LIBS)

$(OBJECTS): obj/%.o : %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(OPT) -c $< -o $@

.PHONY: clean setup

setup:
	mkdir obj

clean:
	rm -rf obj/*
	rm -f $(TARGET)
