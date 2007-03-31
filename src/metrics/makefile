# This is the makefile for sidebear, composit, subfont, mergepfa, renamech
# Copyright 2007 TeX Users Group.
# You may freely use, modify and/or distribute this file.

SIDEOBJECTS = sidebear.obj encrypt.obj decrypt.obj

ADDCOBJECTS = composit.obj encrypt.obj decrypt.obj

SUBOBJECTS = subfont.obj encrypt.obj decrypt.obj

MERGEOBJECTS = mergepfa.obj encrypt.obj decrypt.obj

RENAMEOBJECTS = renamech.obj encrypt.obj decrypt.obj

TRANSFRMOBJECTS = transfrm.obj encrypt.obj decrypt.obj

SEACOBJECTS = safeseac.obj encrypt.obj decrypt.obj

PFATOAFMOBJECTS = pfatoafm.obj encrypt.obj decrypt.obj

# CC = c:\c600\bin\cl
CC = cl
# CC = c:\windev\bin\cl
# CC = c:\msvc\bin\cl

# LINK = c:\c600\binb\link
LINK = link
# LINK = c:\windev\bin\link
# LINK = c:\msvc\bin\link

# CFLAGS = -c -AS -W4 -Od -Za -Lr -F 4000 
# CFLAGS = -c -AS -W4 -Ox -Za -Lr -F 4000 
# CFLAGS = -c -AS -W4 -Za -Lr -F 4000 
# CFLAGS = -c -AS -W4 -Oegcil -Za -Lr -F 4000 
# CFLAGS = -c -AS -W4 -Os -Za -Lr -F 4000 
# CFLAGS = /c /AM /W4 /Ox /Gr /Za /Lr /F 4000
# CFLAGS = /c /AM /W4 /Ox /Gr /Za /Lr
# CFLAGS = /c /AM /W4 /Lr

# release version
# CFLAGS = /c /AM /W4 /Ox /Gr /Lr
# CFLAGS = /c /AM /W4 /Lr /Gs
CFLAGS = /c /Gf /Ge /W4 /Zd
# debugging version:
# CFLAGS = /c /AM /W4 /Lr /Od /Or /Zi

# LFLAGS = /noe /noi /F /PACKC /E /ST:4000

# release version
# LFLAGS = /noe/noi/E/F/PACKC
LFLAGS = /DEBUG /MAP
# debugging version
# LFLAGS = /noe/noi/E/CO/map

# 	$(LINK) $(LFLAGS) $(OBJECTS) c:\c600\lib\setargv.obj;

all: sidebear.exe composit.exe subfont.exe mergepfa.exe renamech.exe transfrm.exe safeseac.exe pfatoafm.exe

sidebear.exe: $(SIDEOBJECTS) 
 	$(LINK) $(LFLAGS) $(SIDEOBJECTS) setargv.obj
#	mapsym sidebear.map

composit.exe: $(ADDCOBJECTS) 
 	$(LINK) $(LFLAGS) $(ADDCOBJECTS) setargv.obj
#	mapsym composit.map

subfont.exe: $(SUBOBJECTS)
 	$(LINK) $(LFLAGS) $(SUBOBJECTS) setargv.obj
#	mapsym subfont.map

mergepfa.exe: $(MERGEOBJECTS)
 	$(LINK) $(LFLAGS) $(MERGEOBJECTS) setargv.obj
#	mapsym mergepfa.map

renamech.exe: $(RENAMEOBJECTS)
 	$(LINK) $(LFLAGS) $(RENAMEOBJECTS) setargv.obj
#	mapsym renamech.map

transfrm.exe: $(TRANSFRMOBJECTS)
 	$(LINK) $(LFLAGS) $(TRANSFRMOBJECTS) setargv.obj
#	mapsym transfrm.map

safeseac.exe: $(SEACOBJECTS)
 	$(LINK) $(LFLAGS) $(SEACOBJECTS) setargv.obj
#	mapsym safeseac.map

pfatoafm.exe: $(PFATOAFMOBJECTS)
 	$(LINK) $(LFLAGS) $(PFATOAFMOBJECTS) setargv.obj
#	mapsym pfatoafm.map

sidebear.obj: sidebear.c metrics.h
	$(CC) $(CFLAGS) sidebear.c

composit.obj: composit.c metrics.h
	$(CC) $(CFLAGS) composit.c

subfont.obj: subfont.c metrics.h
	$(CC) $(CFLAGS) subfont.c

mergepfa.obj: mergepfa.c metrics.h
	$(CC) $(CFLAGS) mergepfa.c

renamech.obj: renamech.c metrics.h
	$(CC) $(CFLAGS) renamech.c

transfrm.obj: transfrm.c metrics.h
	$(CC) $(CFLAGS) transfrm.c

safeseac.obj: safeseac.c metrics.h
	$(CC) $(CFLAGS) safeseac.c

pfatoafm.obj: pfatoafm.c metrics.h
	$(CC) $(CFLAGS) pfatoafm.c

encrypt.obj: encrypt.c metrics.h
	$(CC) $(CFLAGS) encrypt.c

decrypt.obj: decrypt.c metrics.h
	$(CC) $(CFLAGS) decrypt.c

# CFLAGS = /c /AM /W4 /Ox /Gr /Lr
# remove the /Gr and use /Ze and remove _cdecl from dvipsone.c for ANSII
