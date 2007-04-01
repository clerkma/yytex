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

# -Gf saves about 7k bytes in EXE file

# For production (-Bd show compiler passes)
# ASSUMING WE WORK WITH MS COMPILER AND LINKER ONLY use -Gy and -Gf
# CFLAGS=-c -DDOS -DMSDOS -DPHARLAP -DTeX -DMYDEBUG -Ilib -Ox -Gy -Gf -W3
# CFLAGS=-c -DDOS -DMSDOS -DPHARLAP -DTeX -DMYDEBUG -Ilib -YX -Ox -Ge -Gy -Gf -W4
# Release version
CFLAGS=/c /Ge /Gy /Gf /Ox /W4 /DMSDOS /DTeX /DPHARLAP /DNDEBUG /Ilib /YX
# Debugging version
# CFLAGS=/c /Ge /Gy /Gf /Od /Zi /W4 /DMSDOS /DTeX /DPHARLAP /Ilib /YX 

# NOTE: -Ox => -Ogityb1 -Gs

# Link with MS LINK
# LFLAGS=@yandytex.nt /MAP lib\libdos.lib
# Release version
# LFLAGS=/MAP lib\libdos.lib
LFLAGS=/MAP
# debugging version
# LFLAGS=@yandytex.nt /MAP /DEBUG /PDB:yandytex.pdb lib\libdos.lib
# LFLAGS=/MAP /DEBUG /PDB:yandytex.pdb

# objs = cextra.obj citex.obj openinou.obj local.obj \
objs = 	texmf.obj itex.obj openinou.obj subroute.obj local.obj \
          tex0.obj tex1.obj tex2.obj tex3.obj tex4.obj \
          tex5.obj tex6.obj tex7.obj tex8.obj tex9.obj

# ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ###

# yandytex.exe: $(objs) lib/libdos.lib
# texmf.exe: $(objs) lib/libdos.lib
texmf.exe: $(objs) 
   	$(LINK) $(LFLAGS) texmf itex openinou subroute local \
	tex0 tex1 tex2 tex3 tex4 tex5 tex6 tex7 tex8 tex9
	copy texmf.exe yandytex.exe

# ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ###

# cextra.obj: texd.h ..\texmf.c
# cextra.obj: texmf.c texd.h
texmf.obj: texmf.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	$(CC) -focextra -DINITEX -DINI $(CFLAGS) -c ..\lib\texmf.c
#	$(CC) -Focextra -DINITEX -DINI $(CFLAGS) lib\texmf.c
	$(CC) /DINITEX /DINI $(CFLAGS) texmf.c
#	copy texmf.obj cextra.obj

# citex.obj: itex.c texd.h 
itex.obj: itex.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	$(CC) -focitex -DINITEX $(CFLAGS) -c itex.c
#	$(CC) -Focitex -DINITEX $(CFLAGS) itex.c
	$(CC) /DINITEX $(CFLAGS) itex.c
#	copy itex.obj citex.obj

# openinout.obj: texd.h ..\openinout.c 
openinou.obj: openinou.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 
	$(CC) $(CFLAGS) openinou.c

subroute.obj: subroute.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
	$(CC) $(CFLAGS) subroute.c	

local.obj: local.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 
	$(CC) /DINITEX $(CFLAGS) local.c

# NOTE: cannot compile tex5.c and tex6.c with full optimizations ...

tex0.obj: tex0.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 

tex1.obj: tex1.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 

tex2.obj: tex2.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 

tex3.obj: tex3.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 

tex4.obj: tex4.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 

# tex5.obj: tex5.c texd.h texmf.h coerce.h

tex5.obj: tex5.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 
#	$(CC) $(CFLAGSAFE) tex5.c

# tex6.obj: tex6.c texd.h texmf.h coerce.h

tex6.obj: tex6.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 
#	$(CC) $(CFLAGSAFE) tex6.c

tex7.obj: tex7.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 

tex8.obj: tex8.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 

tex9.obj: tex9.c \
	texd.h texmf.h texmfmem.h coerce.h lib.h config.h common.h
#	config.h c-auto.h \
#	c-std.h c-unistd.h c-memstr.h c-errno.h \
#	c-minmax.h c-limits.h c-pathmx.h c-fopen.h \
#	c-proto.h common.h getopt.h lib.h \
#	types.h ourpaths.h pascal.h \
#	c-ctype.h c-pathch.h 

.SUFFIXES:
.SUFFIXES: .obj .c

c..obj:
	$(CC) $(CFLAGS) $*.c

