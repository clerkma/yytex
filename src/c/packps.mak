#
# makefile for PStrip program

packPS: packPS.o makefile
	cc -g -o packPS packPS.o

packPS.o:
	@echo "Compiling edition `awk '{print $$3+1; exit}' version.h` at `date` ..." >> History;tail -1 History

# increment current version number in version.h and compile...
#	@echo "#define EDIT `awk '{print $$3+1; exit}' version.h` /* `date` */" > make.tmp
#	@tail -1 version.h >> make.tmp
#	@mv make.tmp version.h

	cc -c packPS.c

manpage:
	cp man.page /usr/man/manl/packPS.l

all:	packPS

lint:
	lint -hvxa packPS.c

install:
	strip ./packPS
	cp ./packPS /usr/local/bin/packPS
