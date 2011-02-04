include ../common.mk

ALL_TEST_FLAGS := $(CXXFLAGS) $(GAME_INC_FLAGS) $(GAME_LIB_FLAGS) -lgtest -lpthread

.PHONY: run_tests
.PHONY: clean

EXES = $(patsubst %.$(MAIN_FILE_EXT), $(BIN_DIR)/%.$(EXE_FILE_EXT),$(wildcard *.$(MAIN_FILE_EXT)))

all : $(BIN_DIR) $(EXES)

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(GAME_LIB_PATH):
	cd $(GAME_PATH) && make $(MAKE_ARG) -j

$(EXES): $(BIN_DIR)/%.$(EXE_FILE_EXT): %.$(MAIN_FILE_EXT) $(GAME_LIB_PATH)
	$(CXX) $< $(UTEST_PREFIX)*.$(SOURCE_FILE_EXT) -o $@ $(ALL_TEST_FLAGS)

clean:
	rm -f $(BIN_DIR)/*.$(ALL_EXE_FILE_EXT)

run_tests: $(EXES)
	for exe in $(EXES); do \
          ./$$exe || exit 1; \
        done
