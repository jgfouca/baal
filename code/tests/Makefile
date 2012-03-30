include ../common.mk

ALL_TEST_FLAGS := $(GAMECXXFLAGS) $(GAME_INC_FLAGS) $(GAME_LIB_FLAGS) $(GAMELDFLAGS) -lgtest -lpthread

.PHONY: test
.PHONY: clean
.PHONY: lib

EXES = $(patsubst %.$(MAIN_FILE_EXT), $(BIN_DIR)/%.$(EXE_FILE_EXT),$(wildcard *.$(MAIN_FILE_EXT)))

all : lib $(BIN_DIR) $(EXES)

# We need to implement the dependency on the game library in
# this way. It's the only way to guarantee we get the most
# up-to-date game code even if the library file already exists
# (it may itself by out-of-date).
lib:
	cd $(GAME_PATH) && make $(MAKE_ARG)

$(BIN_DIR):
	-mkdir $(BIN_DIR)

$(EXES): $(BIN_DIR)/%.$(EXE_FILE_EXT): %.$(MAIN_FILE_EXT) *.$(SOURCE_FILE_EXT)
	$(CXX) $< $(UTEST_PREFIX)*.$(SOURCE_FILE_EXT) -o $@ $(ALL_TEST_FLAGS)

clean:
	rm -f $(BIN_DIR)/*.$(ALL_EXE_FILE_EXT)
	-@rmdir $(BIN_DIR) 2> /dev/null

test: $(EXES)
	for exe in $(EXES); do \
          ./$$exe || exit 1; \
        done
