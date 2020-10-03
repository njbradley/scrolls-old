
CXX := g++
CXXFLAGS := -std=c++14 -DGLEW_STATIC
LIBS := -lglew32s -lmingw32 -lglfw3 -lopengl32 -luser32 -lgdi32 -lshell32
LDFLAGS :=
OPT := -O3


ifeq ($(PLAT),MAC)
LIBS := -lglfw -framework CoreVideo -framework OpenGL -framework IOKit -lGLEW
else
ifeq ($(PLAT),LINUX)
LIBS := -lGLEW -lGL -pthread -lglfw
else
LIBS := -lglew32s -lmingw32 -lglfw3 -lopengl32 -luser32 -lgdi32 -lshell32
EXESUFFIX :=.exe
endif
endif

TARGETS := main
MAINS := $(addprefix build/, $(addsuffix .o, $(TARGETS)))
SCRIPTNAMES := blockdata blockphysics blocks classes commands crafting cross-platform entity generative items materials menu mobs multithreading player rendervec shader terrain text texture tiles ui world $(TARGETS)
HEADERS := blockdata.h blockphysics.h blocks.h classes.h collider.h commands.h crafting.h cross-platform.h entity.h generative.h items.h materials.h menu.h mobs.h multithreading.h rendervec.h shader.h terrain.h text.h texture.h tiles.h ui.h world.h
#OBJ := blockdata.o blockphysics.o blocks.o classes.o commands.o crafting.o cross-platform.o entity.o generative.o items.o materials.o menu.o mobs.o multithreading.o player.o rendervec.o shader.o terrain.o text.o texture.o tiles.o ui.o world.o $(MAINS)
OBJ := $(addprefix build/, $(addsuffix .o, $(SCRIPTNAMES)))
#SRC := $(addprefix scripts/, $(addsuffix .cc, $(SCRIPTNAMES)))
DEPS := $(addprefix scripts/, $(HEADERS))

.PHONY: all clean

all: $(TARGETS)

clean:
	rm build/*

$(OBJ): build/%.o : scripts/%.cc $(DEPS)
	$(CXX) -c -o $@ $< $(OPT) $(CXXFLAGS)

# $(OBJ): %.o : %.cc $(DEPS)
# 	$(CXX) -c -o build/$@ scripts/$< $(OPT) $(CXXFLAGS)

$(TARGETS): $(OBJ)
	$(CXX) -o $@$(EXESUFFIX) $^ $(OPT) $(CXXFLAGS) $(LIBS) $(LDFLAGS)