CXX := g++
CXXFLAGS := -std=c++17 -DGLEW_STATIC -I ../ -fPIC
LIBS :=
LDFLAGS :=
DLLFLAGS :=
ifeq ($(BUILD),RELEASE)
OPT := -O3
else
OPT := -g -DSCROLLS_DEBUG
endif


ifeq ($(PLAT),MAC)
LIBS := -ldl
EXESUFFIX :=exe
DLLSUFFIX :=.so
LDFLAGS := -rdynamic
DLLFLAGS := -shared
else
ifeq ($(PLAT),LINUX)
LIBS := -pthread -lboost_system -ldl
EXESUFFIX :=exe
DLLSUFFIX :=.so
LDFLAGS := -rdynamic
DLLFLAGS := -shared
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
TARGETFILE := $(TARGET)$(DLLSUFFIX)
else
TARGETFILE := ../$(TARGET)$(EXESUFFIX)
endif



LIBS := $(LIBS) $(EXTRALIBS)
CXXFLAGS := $(CXXFLAGS) $(EXTRAFLAGS)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OPT) $(OBJECTS) -o $(TARGETFILE) $(LDFLAGS) $(LIBS)

$(OBJECTS): obj/%.o : %.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(OPT) -c $< -o $@

.PHONY: clean setup

setup:
	mkdir obj

clean:
	rm -rf obj/*
	rm -f $(TARGET)
