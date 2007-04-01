# Copyright 2007 TeX Users Group.
# You may freely use, modify and/or distribute this file.

# Makefile for Y&YTeX

# TO SET UP DEBUGGING INFO:
# (1) add -Zi to CFLAGS for CL
# (2) add /MAP /DEBUG /PDB:yandytex.pdb for LINK

# NOTE: full optimization causes some problems especially in TEX5.C & TEX6.C
# NOTE: assuming no aliasing causes some problems especially in TEXMF.C

CC = cl

LINK = link

# Need not define MSDOS, since compiler already takes care of that...

# -Gf saves about 7k bytes in EXE file

# Turn off optimizations to try and catch bugs 94 March 20
# CFLAGS=-c -DDOS -DMSDOS -DPHARLAP -DTeX -DMYDEBUG -Ilib -Od -Gy -W3
# For production (-Bd show compiler passes)
# ASSUMING WE WORK WITH MS COMPILER AND LINKER ONLY use -Gy and -Gf
# Release Version
CFLAGS=-c -DDOS -DMSDOS -DPHARLAP -DTeX -DMYDEBUG -Ilib -Ox -Gy -Gf -W3
# -Zi generate debugging information 98/March/5
# CFLAGS=-c -DDOS -DMSDOS -DPHARLAP -DTeX -DMYDEBUG -Ilib -Ox -Gy -Gf -W4 -Zi

# NOTE: -Ox => -Ogityb1 -Gs

# Link with MS LINK
# Release version
LFLAGS=@yandytex.nt /MAP lib\libdos.lib
# LFLAGS=/MAP lib\libdos.lib
# debugging version
# LFLAGS=@yandytex.nt /MAP /DEBUG /PDB:yandytex.pdb lib\libdos.lib

# NEED TO REMOVE @nofloat FOR PRODUCTION IN CASE PEOPLE DON'T HAVE COPROCESSOR!

# objs = cextra.obj citex.obj openinou.obj local.obj \
objs = 	texmf.obj tex0.obj tex1.obj tex2.obj tex3.obj tex4.obj \
	  tex5.obj tex6.obj tex7.obj tex8.obj tex9.obj \
	      itex.obj openinou.obj local.obj
#	      itex.obj openinou.obj local.obj texmagic.obj

# NOTE: have to use PHARLAP linker for this

# ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ###

# yandytex.exe: $(objs) lib/libdos.lib
texmf.exe: $(objs) lib/libdos.lib
   	$(LINK) $(LFLAGS) texmf tex0 tex1 tex2 tex3 tex4 tex5 tex6 tex7 \
	tex8 tex9 itex openinou local 
	copy texmf.exe yandytex.exe


# ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ###

# cextra.obj: texd.h ..\lib\texmf.c
# cextra.obj: lib\texmf.c lib\texd.h
texmf.obj: lib\texmf.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h lib\c-ctype.h lib\c-pathch.h 
#	$(CC) -focextra -DINITEX -DINI $(CFLAGS) -c ..\lib\texmf.c
#	$(CC) -Focextra -DINITEX -DINI $(CFLAGS) lib\texmf.c
	$(CC) -DINITEX -DINI $(CFLAGS) lib\texmf.c
#	copy texmf.obj cextra.obj

# citex.obj: itex.c lib\texd.h 
itex.obj: itex.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 
#	$(CC) -focitex -DINITEX $(CFLAGS) -c itex.c
#	$(CC) -Focitex -DINITEX $(CFLAGS) itex.c
	$(CC) -DINITEX $(CFLAGS) itex.c
#	copy itex.obj citex.obj

# openinout.obj: texd.h ..\lib\openinout.c 
openinou.obj: lib\openinou.c \
	lib\config.h lib\c-auto.h lib\c-std.h lib\c-unistd.h \
	lib\c-memstr.h lib\c-errno.h lib\c-minmax.h lib\c-limits.h \
	lib\c-pathmx.h lib\c-fopen.h lib\c-proto.h lib\common.h \
	lib\getopt.h lib\lib.h lib\types.h lib\ourpaths.h \
	lib\pascal.h lib\c-namemx.h lib\c-pathch.h lib\texd.h \
	lib\texmf.h lib\texmfmem.h lib\coerce.h 
#	$(CC) $(CFLAGS) -c ..\lib\openinout.c
	$(CC) $(CFLAGS) lib\openinou.c

local.obj: local.c \
	lib\getopt.h lib\texd.h lib\texmf.h lib\config.h \
	lib\c-auto.h lib\c-std.h lib\c-unistd.h lib\c-memstr.h \
	lib\c-errno.h lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h \
	lib\c-fopen.h lib\c-proto.h lib\common.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h lib\coerce.h 
	$(CC) -DINITEX $(CFLAGS) local.c

# NOTE: cannot compile tex5.c and tex6.c with full optimizations ...

tex0.obj: tex0.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 

tex1.obj: tex1.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 

tex2.obj: tex2.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 

tex3.obj: tex3.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 

tex4.obj: tex4.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 

# tex5.obj: tex5.c lib\texd.h lib\texmf.h lib\coerce.h

tex5.obj: tex5.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 
#	$(CC) $(CFLAGSAFE) tex5.c

# tex6.obj: tex6.c lib\texd.h lib\texmf.h lib\coerce.h

tex6.obj: tex6.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 
#	$(CC) $(CFLAGSAFE) tex6.c

tex7.obj: tex7.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 

tex8.obj: tex8.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 

tex9.obj: tex9.c \
	lib\texd.h lib\texmf.h lib\config.h lib\c-auto.h \
	lib\c-std.h lib\c-unistd.h lib\c-memstr.h lib\c-errno.h \
	lib\c-minmax.h lib\c-limits.h lib\c-pathmx.h lib\c-fopen.h \
	lib\c-proto.h lib\common.h lib\getopt.h lib\lib.h \
	lib\types.h lib\ourpaths.h lib\pascal.h lib\texmfmem.h \
	lib\coerce.h 

# texmagic.obj: texmagic.c 

.SUFFIXES:
.SUFFIXES: .obj .c

c..obj:
	$(CC) $(CFLAGS) $*.c

