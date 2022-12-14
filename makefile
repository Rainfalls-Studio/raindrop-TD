# project
VERSION = 0.1.0

# compiler
CXX = g++
STD_VERSION = c++17
LIBSFLAGS = -lmingw32 -lSDL2main -lSDL2 -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lshell32 -lsetupapi -lversion -luuid -lyaml-cpp -lEFX-Util -lOpenAL32 -lsndfile -lvulkan-1 -lglad
CFLAGS =
DEFINES = -D VERSION='"$(VERSION)"'
INCLUDE = -I include/raindrop/ -I include/libs/

# directories
BIN = out
SRC = src
OBJ = .obj
LIB = libs
DLL = raindrop

# source files
SRCS = $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/**/*.cpp) $(wildcard $(SRC)/**/**/*.cpp) $(wildcard $(SRC)/**/**/**/*.cpp)
OBJS := $(patsubst %.cpp, $(OBJ)/%.o, $(notdir $(SRCS)))

all: $(DLL)

release: CFLAGS = -Wall -O2 -D NDEBUG
release: clean
release: all

dbg: CFLAGS = -g3
dbg: all

dbgRebuild: CFLAGS = -g3
dbgRebuild: rebuild

rebuild: clean
rebuild: $(DLL)

pch:
	$(CXX) -std=$(STD_VERSION) -c include/raindrop/pch.hpp -o include/raindrop/pch.pch $(INCLUDE)

clean:
	@del $(OBJ)\*.o

$(DLL) : $(OBJS)
	$(CXX) -std=$(STD_VERSION) -shared $(OBJ)/*.o $(INCLUDE) -L $(LIB) -o $(BIN)\$(DLL).dll $(CFLAGS) $(DEFINES) $(LIBSFLAGS) -Wl,--out-implib,$(BIN)\$(DLL).lib

$(OBJ)/%.o : $(SRC)/%.cpp
	$(CXX) -fPIC -std=$(STD_VERSION) -shared -o $@ -c $< $(INCLUDE) -L $(LIB) $(DEFINES) $(CFLAGS)

$(OBJ)/%.o : $(SRC)/*/%.cpp
	$(CXX) -fPIC -std=$(STD_VERSION) -shared -o $@ -c $< $(INCLUDE) -L $(LIB) $(DEFINES) $(CFLAGS)

$(OBJ)/%.o : $(SRC)/*/*/%.cpp
	$(CXX) -fPIC -std=$(STD_VERSION) -shared -o $@ -c $< $(INCLUDE) -L $(LIB) $(DEFINES) $(CFLAGS)
	
$(OBJ)/%.o : $(SRC)/*/*/*/%.cpp
	$(CXX) -fPIC -std=$(STD_VERSION) -shared -o $@ -c $< $(INCLUDE) -L $(LIB) $(DEFINES) $(CFLAGS)

info:
	@echo -----------------------------------------------------
	@echo info :                
	@echo 	compiler                     : $(CXX)
	@echo 	compiler standart version    : $(STD_VERSION)
	@echo 	flags                        : $(CFLAGS)
	@echo 	defines                      : $(DEFINES)
	@echo 	lib name                     : $(DLL)
	@echo 	libs directory               : $(LIB)
	@echo 	binary directory             : $(BIN)
	@echo 	source code directory        : $(SRC)
	@echo 	compiled object directory    : $(OBJ)
	@echo 	include directory            : $(INCLUDE)
	@echo -----------------------------------------------------
