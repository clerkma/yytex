# Copyright 2007 TeX Users Group
# Copyright 2014, 2015, 2016 Clerk Ma
# You may freely use, modify and/or distribute this file.

# Makefile for DVIPSONE

# TO SET UP DEBUGGING INFO:
# (1) add -Zi to CFLAGS for CL
# (2) add /MAP /DEBUG /PDB:dvipsone.pdb for LINK

CC = cl

LINK = link

# Need not define MSDOS, since compiler already takes care of that...

# -Gf saves about 7k bytes in EXE file
# For production (-Bd show compiler passes)
# ASSUMING WE WORK WITH MS COMPILER AND LINKER ONLY use -Gy and -Gf
# CFLAGS=-c  -Ox -Gy -Gf -W3
# -c compile only, no link
# -F 6000 change stack allocation ? passed to LINK ?
# -YX auto precompiled header
# -Ox == /Ogityb1 /Gs
# -Ge stack checking (remove later for speed) -Gs maybe?
# -Gy separate functions for linker
# -Gf enable string pooling
# -Zd line number information for debugging ?
# -Zi generate debugging information 98/March/5
# Release version
CFLAGS=/nologo /c /Ox /GF /Gy /W4
# Debugging version
# CFLAGS=/c /YX /Ge /Gy /Gf /W4 /Od /Zi

# NOTE: -Ox => -Ogityb1 -Gs

# /MAP create map file
# Release version
LFLAGS=/nologo /MAP /RELEASE
# Debugging version
# LFLAGS=/MAP /DEBUG /PDB:dvipsone.pdb

# objs = cextra.obj citex.obj openinou.obj local.obj 
objs = dvipsone.obj  dviextra.obj \
	 dvianal.obj dvispeci.obj dvitiff.obj dvipslog.obj

# ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ###

# dvipsone.exe: $(objs) lib/libdos.lib
dvipsone.exe: $(objs) 
#   	$(LINK) $(LFLAGS) dvipsone dvipslog dviextra dvianal dvispeci dvitiff
	$(LINK) $(LFLAGS) $(objs)
	copy dvipsone.exe ..\yandy\bin\dvipsone.exe


# ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ###

dvipsone.obj: dvipsone.c dvipsone.h
	$(CC) $(CFLAGS) dvipsone.c

dvipslog.obj: dvipslog.c dvipsone.h
	$(CC) $(CFLAGS) dvipslog.c

dviextra.obj: dviextra.c dvipsone.h
	$(CC) $(CFLAGS) dviextra.c

dvianal.obj: dvianal.c dvipsone.h
	$(CC) $(CFLAGS) dvianal.c

dvispeci.obj: dvispeci.c dvipsone.h
	$(CC) $(CFLAGS) dvispeci.c

dvitiff.obj: dvitiff.c dvipsone.h
	$(CC) $(CFLAGS) dvitiff.c

.SUFFIXES:
.SUFFIXES: .obj .c

c..obj:
	$(CC) $(CFLAGS) $*.c

