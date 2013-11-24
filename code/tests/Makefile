include ../common.mk

# The hacks below are necessary because gtest must be installed with a native
# c++11 compiler, like clang-4
TEST_LINK_FLAGS := -L /Users/jgfouca/manually-installed-packges/gtest-1.7.0/install -lgtest $(GAME_LIB_FLAGS) $(GAMELDFLAGS) -lpthread
TEST_COMPILE_FLAGS = -I /Users/jgfouca/manually-installed-packges/gtest-1.7.0/include $(GAMECXXFLAGS) $(GAME_INC_FLAGS)
ALL_TEST_FLAGS := $(TEST_COMPILE_FLAGS) $(TEST_LINK_FLAGS)

.PHONY: test
.PHONY: clean
.PHONY: lib
.PHONY: valgrind

EXES = $(patsubst %.$(MAIN_FILE_EXT), $(BIN_DIR)/%.$(EXE_FILE_EXT),$(wildcard *.$(MAIN_FILE_EXT)))

OBJECTS = $(patsubst %.$(SOURCE_FILE_EXT), $(BIN_DIR)/%.$(OBJ_FILE_EXT),$(wildcard *.$(SOURCE_FILE_EXT)))

all : lib $(BIN_DIR) $(EXES)

# We need to implement the dependency on the game library in
# this way. It's the only way to guarantee we get the most
# up-to-date game code even if the library file already exists
# (it may itself by out-of-date).
lib:
	cd $(GAME_PATH) && make $(MAKE_ARG)

$(BIN_DIR):
	-mkdir $(BIN_DIR)

$(EXES): $(BIN_DIR)/%.$(EXE_FILE_EXT): %.$(MAIN_FILE_EXT) $(OBJECTS)
	make $(BIN_DIR)
	$(CXX) $(TEST_COMPILE_FLAGS) $< -o $@ ${OBJECTS} $(TEST_LINK_FLAGS)


$(OBJECTS) : $(BIN_DIR)/%.$(OBJ_FILE_EXT): %.$(SOURCE_FILE_EXT) $(GAME_PATH)/*.$(HEADER_FILE_EXT)
	$(CXX) $(TEST_COMPILE_FLAGS) -o $@ -c $<

clean:
	rm -f $(BIN_DIR)/*.$(ALL_EXE_FILE_EXT) $(BIN_DIR)/*.$(ALL_OBJ_FILE_EXT)
	-@rmdir $(BIN_DIR) 2> /dev/null

test: $(EXES)
	for exe in $(EXES); do \
          ./$$exe || exit 1; \
        done

valgrind: $(EXES)
	for exe in $(EXES); do \
          valgrind --tool=memcheck --leak-check=full ./$$exe || exit 1; \
        done
