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

include ../build.mk
