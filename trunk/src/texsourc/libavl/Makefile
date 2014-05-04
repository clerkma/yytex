.PHONY: all doc test help

all:
	@(cd libavl && $(MAKE) $@)

doc:
	@(doxygen Doxyfile)

test:
	@(cd libavl && $(MAKE) test)

help:
	@(echo "Usage:")
	@(echo "")
	@(echo "make all       build libavl library.")
	@(echo "make doc       build in-source documentation for libavl.")
	@(echo "make test      build tests for libavl.")
	@(echo "make help      show this help.")
	@(echo "")
	@(echo "For more information and more make function, run")
	@(echo "  $$ cd libavl && make help")
