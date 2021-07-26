CXX := g++
CXXFLAGS := -std=c++17 -DGLEW_STATIC -I ../
LIBS := -lglew32s -lmingw32 -lglfw3 -lopengl32 -luser32 -lgdi32 -lshell32
LDFLAGS :=
DLLFLAGS :=
ifeq ($(BUILD),RELEASE)
OPT := -O3
else
OPT := -g -DSCROLLS_DEBUG
endif


ifeq ($(PLAT),MAC)
LIBS := -ldl
DLLSUFFIX :=.so
LDFLAGS := -rdynamic
DLLFLAGS := -shared -fPIC
else
ifeq ($(PLAT),LINUX)
LIBS := -pthread -lboost_system -ldl
DLLSUFFIX :=.so
LDFLAGS := -rdynamic
DLLFLAGS := -shared -fPIC
else
LIBS := -lmingw32 -luser32 -lgdi32 -lshell32 -lboost_system-mt -lWs2_32 -ldbghelp $(patsubst %,-l%, $(SCROLLSDEPS)) $(patsubst %,-L../%/, $(SCROLLSDEPS))
DLLFLAGS := -shared -Wl,--out-implib,lib$(TARGET).a
LDFLAGS := -Wl,--out-implib,lib$(TARGET).a,--export-all-symbols
EXESUFFIX :=.exe
DLLSUFFIX :=.dll
endif
endif

ifeq ($(ISDLL), TRUE)
LDFLAGS := $(DLLFLAGS)
EXESUFFIX := $(DLLSUFFIX)
endif
