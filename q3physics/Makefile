TARGET := q3physics
SCROLLSDEPS := scrolls
ISDLL := TRUE

Q3FLAGS := -std=c++11 -g -fPIC

Q3SOURCES := $(wildcard qu3e/*/*.cpp)
Q3HEADERS := $(wildcard qu3e/*/*.h)
Q3OBJECTS := $(patsubst qu3e/%.cpp,obj/%.o, $(Q3SOURCES))

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS))) libqu3e.a
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

EXTRALIBS := libqu3e.a
EXTRAFLAGS := -I qu3e/

include ../build.mk

libqu3e.a: $(Q3OBJECTS)
	ar rvs libqu3e.a $(Q3OBJECTS)

$(Q3OBJECTS): obj/%.o : qu3e/%.cpp $(Q3HEADERS)
#	mkdir -p $(dir $@)
	g++ $(Q3FLAGS) -c $< -o $@
