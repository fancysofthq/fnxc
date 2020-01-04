CXX = clang++
CXXLTO = -flto=thin
CXXFLAGS = -g $(CXXLTO) -std=c++17 -I`llvm-config --includedir` -I./lib/lua
LIBS = `llvm-config --system-libs --ldflags support` \
	-lpthread -lstdc++fs \
	-L./lib/lua -llua -ldl

SRC_DIR = src
CACHE_DIR = cache
BIN_DIR = bin

_OBJECTS = \
	macro.o \
	token.o \
	lexer.o \
	sast.o \
	parser.o \
	compiler.o \
	shared.o \
	onyx.o

OBJECTS = $(patsubst %,$(CACHE_DIR)/%,$(_OBJECTS))

onyx: $(BIN_DIR)/onyx

$(BIN_DIR)/onyx: $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXLTO) $^ $(LIBS) $(LLVMFLAGS) -o $(BIN_DIR)/onyx

$(CACHE_DIR)/%.o: $(SRC_DIR)/%.cpp | $(CACHE_DIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(BIN_DIR):
	mkdir -p $@

$(CACHE_DIR):
	mkdir -p $@

clean:
	-rm -r $(BIN_DIR) $(CACHE_DIR)

.PHONY: clean
