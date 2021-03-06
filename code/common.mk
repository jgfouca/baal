#### Constants ####

# Compiler constants
CXX := clang++ # Use colorgcc/g++ if clang not installed

# File binary type substrings
OPT_BUILD_TAG  := opt
DBG_BUILD_TAG  := dbg
PROF_BUILD_TAG := prof

# Flag constants
ALWAYS_FLAGS   := -Wall -std=c++11 -stdlib=libc++ -I /opt/local/include
CXX_OPT_FLAGS  := -O3 -DNDEBUG $(ALWAYS_FLAGS)
CXX_DBG_FLAGS  := -g $(ALWAYS_FLAGS) -D_GLIBCXX_DEBUG
CXX_PROF_FLAGS := -O2 -DNDEBUG -g $(ALWAYS_FLAGS)

# Flag variables, default to debug. Note, build-type selection can be
# done with:
# % make BUILD=(opt|prof|dbg)
ifeq ($(BUILD), opt)
CXXFLAGS := $(CXX_OPT_FLAGS)
FILE_TAG := $(OPT_BUILD_TAG)
MAKE_ARG := BUILD=opt
else ifeq ($(BUILD), prof)
CXXFLAGS := $(CXX_PROF_FLAGS)
FILE_TAG := $(PROF_BUILD_TAG)
MAKE_ARG := BUILD=prof
else
CXXFLAGS := $(CXX_DBG_FLAGS)
FILE_TAG := $(DBG_BUILD_TAG)
MAKE_ARG :=
endif

# File extension/prefix constants
MAIN_FILE_EXT    := C
SOURCE_FILE_EXT  := cpp
HEADER_FILE_EXT  := hpp
ALL_LIB_FILE_EXT := a
ALL_EXE_FILE_EXT := out
ALL_OBJ_FILE_EXT := o
EXE_FILE_EXT     := $(FILE_TAG).$(ALL_EXE_FILE_EXT)
OBJ_FILE_EXT     := $(FILE_TAG).$(ALL_OBJ_FILE_EXT)
LIB_FILE_EXT     := $(FILE_TAG).$(ALL_LIB_FILE_EXT)
UTEST_PREFIX     := UnitTest

# Directory/filename constants
ROOT           := $(dir $(lastword $(MAKEFILE_LIST)))
BIN_DIR        := bin
GAME           := game
TEST_DIR       := tests
GAME_PATH      := $(ROOT)$(GAME)
TEST_PATH      := $(ROOT)$(TEST_DIR)
GAME_LIB_FILE  := lib$(GAME).$(LIB_FILE_EXT)
GAME_LIB_PATH  := $(GAME_PATH)/$(BIN_DIR)/$(GAME_LIB_FILE)
GAME_LIB_FLAGS := -L$(GAME_PATH)/$(BIN_DIR) -l$(GAME).$(FILE_TAG)
GAME_INC_FLAGS := -I$(GAME_PATH)

# TPL constants
BOOSTCXXFLAGS := # -I ~/manually-installed-packages/boost_1_48_0
BOOSTLDFLAGS := -L /opt/local/lib -lboost_thread-mt

XMLCXXFLAGS := -I /usr/include/libxml2
XMLLDFLAGS := -lxml2

READLINECXXFLAGS :=
READLINELDFLAGS := -lreadline

GAMECXXFLAGS := $(CXXFLAGS) $(BOOSTCXXFLAGS) $(XMLCXXFLAGS) $(READLINECXXFLAGS)
GAMELDFLAGS  := $(GAME_LIB_FLAGS) $(BOOSTLDFLAGS) $(XMLLDFLAGS) $(READLINELDFLAGS)
