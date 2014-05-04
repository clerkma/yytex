# Example of Makefile to build program using libavl shared library

.PHONY: all clean distclean

all: ../libavl/libavl.so example.x

# Be sure that libavl is compiled
../libavl/libavl.so:
	@(cd ../libavl/ && $(MAKE))

example.o: example.c
	gcc -o example.o -c example.c -I../libavl/

example.x: example.o
	gcc -o example.x -L../libavl/ example.o -lavl
	@(echo "Before run, add \"../libavl/\" to your LD_LIBRARY_PATH env var.")

clean:
	rm example.x

distclean: clean
	rm example.o

