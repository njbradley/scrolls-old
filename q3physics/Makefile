TARGET := q3physics
SCROLLSDEPS := scrolls
ISDLL := TRUE

Q3FLAGS := -std=c++11

Q3SOURCES := $(wildcard qu3e/src/*/*.cpp)
Q3HEADERS := $(wildcard qu3e/src/*/*.h)
Q3OBJECTS := $(patsubst qu3e/src/%.cpp,obj/%.o, $(Q3SOURCES))

SOURCES := $(wildcard *.cc)
HEADERS := $(wildcard *.h) $(wildcard $(patsubst %,../%/*.h, $(SCROLLSDEPS))) libqu3e.a
OBJECTS := $(patsubst %,obj/%, $(patsubst %.cc,%.o, $(SOURCES)))

EXTRALIBS := libqu3e.a
EXTRAFLAGS := -I qu3e/src/

include ../build.mk

libqu3e.a: $(Q3OBJECTS)
	ar rvs libqu3e.a $(Q3OBJECTS)

$(Q3OBJECTS): obj/%.o : qu3e/src/%.cpp $(Q3HEADERS)
	mkdir -p $(dir $@)
	g++ $(Q3FLAGS) -c $< -o $@
