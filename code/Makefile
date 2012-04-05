include common.mk

.PHONY: clean

all :
	cd $(GAME_PATH) && make $(MAKE_ARG)
	cd $(TEST_PATH) && make clean && make test $(MAKE_ARG)

clean:
	cd $(GAME_PATH) && make clean
	cd $(TEST_PATH) && make clean
