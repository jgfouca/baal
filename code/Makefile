include common.mk

.PHONY: clean

all :
	cd $(GAME_PATH) && make $(MAKE_ARG) -j
	cd $(TEST_PATH) && make $(MAKE_ARG) -j

clean:
	cd $(GAME_PATH) && make clean
	cd $(TEST_PATH) && make clean
