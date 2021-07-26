TARGET := glgraphics
SCROLLSDEPS := base
ISDLL := TRUE

ifeq ($(PLAT),MAC)
EXTRALIBS := -lglfw -framework CoreVideo -framework OpenGL -framework IOKit -lGLEW
else
ifeq ($(PLAT),LINUX)
EXTRALIBS := -lGLEW -lGL -lglfw
else
EXTRALIBS := -lglew32 -lglfw3 -lopengl32
endif
endif

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
