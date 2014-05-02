# Copyright 2007 TeX Users Group.
# Copyright 2014 Clerk Ma.
#
# You may freely use, modify and/or distribute this file.

# Makefile for Y&YTeX

# TO SET UP DEBUGGING INFO:
# (1) add -Zi to CFLAGS for CL
# (2) add /MAP /DEBUG /PDB:yandytex.pdb for LINK

# NOTE: full optimization causes some problems especially in TEX5.C & TEX6.C
# NOTE: assuming no aliasing causes some problems especially in TEXMF.C

CC = cl
LINK = link
RC = rc

# -Gf saves about 7k bytes in EXE file

# For production (-Bd show compiler passes)
# ASSUMING WE WORK WITH MS COMPILER AND LINKER ONLY use -Gy and -Gf
# CFLAGS=-c -DDOS -DMSDOS -DPHARLAP -DTeX -DMYDEBUG -Ilib -Ox -Gy -Gf -W3
# CFLAGS=-c -DDOS -DMSDOS -DPHARLAP -DTeX -DMYDEBUG -Ilib -YX -Ox -Ge -Gy -Gf -W4
# Release version
# CFLAGS=/c /Ge /Gy /Gf /Ox /W4 /DMSDOS /DTeX /DPHARLAP /DNDEBUG /Ilib /YX
CFLAGS=/nologo /c /MT /GF /Gy /Ox /W4 /DMSDOS /DTeX /DPHARLAP /DNDEBUG \
	/I"kpathsea" /I"libharu/include" /I"libharu/win32/include" 
# Debugging version
# CFLAGS=/c /Ge /Gy /Gf /Od /Zi /W4 /DMSDOS /DTeX /DPHARLAP /Ilib /YX

# NOTE: -Ox => -Ogityb1 -Gs

# Link with MS LINK
# LFLAGS=@yandytex.nt /MAP lib\libdos.lib
# Release version
# LFLAGS=/MAP lib\libdos.lib
LFLAGS=/MAP
# /MACHINE:X86
# debugging version
# LFLAGS=@yandytex.nt /MAP /DEBUG /PDB:yandytex.pdb lib\libdos.lib
# LFLAGS=/MAP /DEBUG /PDB:yandytex.pdb

objs = texmf.obj itex.obj openinou.obj subroute.obj local.obj \
	tex0.obj tex1.obj tex2.obj tex3.obj tex4.obj \
	tex5.obj tex6.obj tex7.obj tex8.obj tex9.obj \
	yandy_pool.obj yandytex.res md5.obj md5file.obj

# ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ###

texmf.exe: $(objs) 
	$(LINK) $(LFLAGS) texmf itex openinou subroute local \
	tex0 tex1 tex2 tex3 tex4 tex5 tex6 tex7 tex8 tex9 \
	yandy_pool yandytex.res md5 md5file kpathsea\kpathsea.lib \
	libharu\libhpdf.lib libpng\libpng.lib zlib\zlib.lib 
	copy texmf.exe yandytex.exe
	del ..\yandy\bin\texmf.exe
	copy texmf.exe ..\yandy\bin\texmf.exe
# ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ### ###
md5.obj: libmd5\md5.c
	$(CC) -Ilibmd5 $(CFLAGS) libmd5\md5.c

md5file.obj: md5file.c
	$(CC) -Ilibmd5 $(CFLAGS) md5file.c

yandytex.res: yandytex.rc
	rc yandytex.rc

texmf.obj: texmf.c \
	texd.h texmf.h texmfmem.h coerce.h
	$(CC) /DINITEX /DINI $(CFLAGS) texmf.c

itex.obj: itex.c \
	texd.h texmf.h texmfmem.h coerce.h
	$(CC) /DINITEX $(CFLAGS) itex.c

openinou.obj: openinou.c \
	texd.h texmf.h texmfmem.h coerce.h
	$(CC) $(CFLAGS) openinou.c

subroute.obj: subroute.c \
	texd.h texmf.h texmfmem.h coerce.h
	$(CC) $(CFLAGS) subroute.c	

local.obj: local.c \
	texd.h texmf.h texmfmem.h coerce.h
	$(CC) /DINITEX $(CFLAGS) local.c

# NOTE: cannot compile tex5.c and tex6.c with full optimizations ...

tex0.obj: tex0.c \
	texd.h texmf.h texmfmem.h coerce.h

tex1.obj: tex1.c \
	texd.h texmf.h texmfmem.h coerce.h

tex2.obj: tex2.c \
	texd.h texmf.h texmfmem.h coerce.h

tex3.obj: tex3.c \
	texd.h texmf.h texmfmem.h coerce.h

tex4.obj: tex4.c \
	texd.h texmf.h texmfmem.h coerce.h

tex5.obj: tex5.c \
	texd.h texmf.h texmfmem.h coerce.h

tex6.obj: tex6.c \
	texd.h texmf.h texmfmem.h coerce.h

tex7.obj: tex7.c \
	texd.h texmf.h texmfmem.h coerce.h

tex8.obj: tex8.c \
	texd.h texmf.h texmfmem.h coerce.h

tex9.obj: tex9.c \
	texd.h texmf.h texmfmem.h coerce.h

yandy_pool.obj: yandy_pool.c \
	texd.h texmf.h texmfmem.h coerce.h

.SUFFIXES:
.SUFFIXES: .obj .c

c..obj:
	$(CC) $(CFLAGS) $*.c

