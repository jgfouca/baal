include ../../common.mk

.PHONY: clean test

OBJECTS = $(patsubst %.$(SOURCE_FILE_EXT), $(BIN_DIR)/%.$(OBJ_FILE_EXT),$(wildcard *.$(SOURCE_FILE_EXT)))

EXES = $(patsubst %.$(MAIN_FILE_EXT), $(BIN_DIR)/%.$(EXE_FILE_EXT),$(wildcard *.$(MAIN_FILE_EXT)))

all : $(BIN_DIR) $(EXES)

$(BIN_DIR):
	-mkdir $(BIN_DIR)

$(EXES): $(BIN_DIR)/%.$(EXE_FILE_EXT) : %.$(MAIN_FILE_EXT) $(GAME_LIB_PATH)
	g++ $(CXXFLAGS) $(LDFLAGS) $< -o $@ ${OBJECTS} $(GAMECXXFLAGS) $(GAMELDFLAGS)

$(OBJECTS) : $(BIN_DIR)/%.$(OBJ_FILE_EXT): %.$(SOURCE_FILE_EXT) *.$(HEADER_FILE_EXT)
	$(CXX) $(GAMECXXFLAGS) -o $@ -c $<

$(GAME_LIB_PATH) : $(OBJECTS)
	ar -rsc $@ $(OBJECTS)

test: $(GAME_LIB_PATH)
	cd $(TEST_PATH) && make $(MAKE_ARG) test

clean:
	rm -f $(BIN_DIR)/*.$(ALL_EXE_FILE_EXT) $(BIN_DIR)/*.$(ALL_OBJ_FILE_EXT) $(BIN_DIR)/*.$(ALL_LIB_FILE_EXT)
	-@rmdir $(BIN_DIR) 2> /dev/null
