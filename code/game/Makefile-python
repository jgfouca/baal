include ../common.mk

.PHONY: clean check test

check:
	for pyfile in $$(/bin/ls -1 *.py); \
        do   echo "@@@@@@@@@@ Checking file $$pyfile @@@@@@@@@@"; \
        pylint --errors-only $$pyfile \
          && echo "\033[1;32mPASS - $$pyfile\033[0m" \
          || echo "\033[1;31mFAILED - $$pyfile\033[0m" ;\
        done

test:
	for pyfile in $$(/bin/ls -1 *.py); \
        do   echo "@@@@@@@@@@ Testing file $$pyfile @@@@@@@@@@"; \
        ./$$pyfile \
          && echo "\033[1;32mPASS - $$pyfile\033[0m" \
          || echo "\033[1;31mFAILED - $$pyfile\033[0m" ;\
        done

clean:
	rm -f *~ *.pyc *.pyo
