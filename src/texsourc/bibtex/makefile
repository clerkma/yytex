# Generated automatically from Makefile.in by configure.
# Makefile for BibTeX.

SHELL = /bin/sh
srcdir = .
VPATH = .

# This is so kpathsea will get remade automatically if you change
# something in it and recompile from the package directory.
kpathsea_parent = ../..
kpathsea_dir = $(kpathsea_parent)/kpathsea
kpathsea = $(kpathsea_dir)/kpathsea.a

# Routines used everywhere.
commondefines = $(srcdir)/../lib/common.defines $(srcdir)/../lib/texmf.defines
commonh = $(srcdir)/../lib/config.h $(kpathsea_dir)/paths.h
commono = ../lib/lib.a $(kpathsea)

# Routines used in TeX, Metafont, and BibTeX.
fileioc = ../lib/openinout.c
fileioo = openinout.o

program = bibtex

CC = gcc
CFLAGS = -g
CPPFLAGS = -DBibTeX -I$(srcdir)/../lib -I../..

CCLD = $(CC)
LDFLAGS = $(CFLAGS) $(XLDFLAGS)
LIBS =  $(extralibs)
libs = $(fileioo) $(commono) $(LIBS)

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = $(INSTALL)

LN = ln -s


.SUFFIXES:
.SUFFIXES: .o .c .p .ch
.p.c:
	$(SHELL) $(srcdir)/../bibtex/convert $*.p $*.c
.ch.p:
	../web/tangle $*.web c$*.ch
.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $*.c


default: all
all: $(program)


$(program): $(program).o $(commono) $(fileioo)
	$(CCLD) -o $(program) $(LDFLAGS) $(program).o $(libs)
$(program).o: $(program).c $(commonh)
$(program).c: $(program).p $(commondefines) convert
$(program).p: $(program).web cbibtex.ch

cbibtex.ch: bibtex.ch
	rm -f $@
	cp $(srcdir)/../bibtex/bibtex.ch cbibtex.ch

openinout.c: $(fileioc)
	rm -f openinout.c
	$(LN) $(fileioc) openinout.c


Makefile: Makefile.in ../config.status
	(cd ..; sh config.status)


c-sources: bibtex.c openinout.c
.PHONY: c-sources

install install-exec: all
	$(INSTALL_PROGRAM) $(program) $(bindir)/$(program)
install-data:


TAGS: *.c *.h
	etags -t *.c *.h


mostlyclean::
	rm -f *.o $(program) $(lib) $(programs)

clean:: mostlyclean
	rm -f *.dvi *.pool

distclean:: clean
	rm -f Makefile config.status c-auto.h

# Although we can remake configure and c-auto.h.in, we don't remove
# them, since many people may lack Autoconf.  Use configclean for that.
realclean:: distclean
	rm -f TAGS *.info*

extraclean::
	rm -f *.aux *.bak *.bbl *.blg *.dvi *.log *.orig *.pl *.rej
	rm -f *.i *.s *.tfm *.vf *.vpl *\#* *gf *pk *~
	rm -f CONTENTS.tex a.out core mfput.* patch* texput.*

configclean:
	rm -f configure c-auto.h.in c-auto.h


# Prevent GNU make 3 from overflowing arg limit on system V.
.NOEXPORT:

clean::
	rm -f *.p *.c *.h

distclean:: clean
	rm -f cbibtex.ch
