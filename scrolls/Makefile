TARGET := scrolls
SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h)
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

SCROLLSDEPS := base
ISDLL := TRUE

include ../config.mk

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
