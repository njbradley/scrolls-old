
CXX := g++
CXXFLAGS := -std=c++17 -DGLEW_STATIC -I scripts/
LIBS := -lglew32s -lmingw32 -lglfw3 -lopengl32 -luser32 -lgdi32 -lshell32
LDFLAGS :=
DLLFLAGS :=
ifeq ($(BUILD),RELEASE)
OPT := -O3
else
OPT := -g -DSCROLLS_DEBUG
endif


ifeq ($(PLAT),MAC)
LIBS := -lglfw -framework CoreVideo -framework OpenGL -framework IOKit -lGLEW -lopenal -lalut -ldl
DLLSUFFIX :=.so
LDFLAGS := -rdynamic
DLLFLAGS := -fPIC
else
ifeq ($(PLAT),LINUX)
LIBS := -lGLEW -lGL -pthread -lglfw -lopenal -lalut -lboost_system -ldl
DLLSUFFIX :=.so
LDFLAGS := -rdynamic
DLLFLAGS := -fPIC
else
LIBS := -lglew32 -lopenal -lalut -lmingw32 -lglfw3 -lopengl32 -luser32 -lgdi32 -lshell32 -lboost_system-mt -lWs2_32 -ldwarfstack -ldbghelp
DLLFLAGS := -lmain -L build/
LDFLAGS := -Wl,--out-implib,build/libmain.a,--export-all-symbols
EXESUFFIX :=.exe
DLLSUFFIX :=.dll
endif
endif

TARGETS := main gentest blocktest commandtest block_editor server client entitytest
MAINS := $(addprefix build/, $(addsuffix .o, $(TARGETS)))
SCRIPTNAMES := audio blockdata blockgroups blocks blockiter classes clientgame commands crafting cross-platform entity game generative glue graphics items materials menu multithreading multiplayer player plugins rendervec servergame shader terrain text texture tiles ui world
HEADERS := blockdata.h blockgroups.h blocks.h blockiter.h classes.h clientgame.h collider.h commands.h crafting.h cross-platform.h entity.h generative.h glue.h items.h materials.h menu.h multithreading.h multiplayer.h plugins.h rendervec.h servergame.h shader.h terrain.h text.h texture.h tiles.h ui.h world.h graphics.h game.h audio.h
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

$(MAINS): build/%.o : scripts/%.cc $(DEPS)
	$(CXX) -c -o $@ $< $(OPT) $(CXXFLAGS)

# $(OBJ): %.o : %.cc $(DEPS)
# 	$(CXX) -c -o build/$@ scripts/$< $(OPT) $(CXXFLAGS)

$(TARGETS): % : build/%.o $(OBJ)
	$(CXX) -o bin/$@$(EXESUFFIX) $^ $(OPT) $(CXXFLAGS) $(LIBS) $(LDFLAGS)

scrolls: scrolls/scrolls.cc $(DEPS)
	$(CXX) -shared -o resources/plugins/scrolls$(DLLSUFFIX) scrolls/scrolls.cc $(DLLFLAGS) $(CXXFLAGS) $(LIBS)
