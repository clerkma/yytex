# Make file for DVIWindo

# TO SET UP DEBUGGING INFO:
# (1) add -Zi to CFLAGS for CL
# (2) add /MAP /DEBUG /PDB:dviwindo.pdb for LINK

# define TARGETOS = [WIN95 | WINNT | BOTH] - defaults to BOTH
# define APPVER = [3.50 | 3.51 | 4.0] - defaults to 4.0

# An experiment for NT 3.51 97/Mar/27

# APPVER=3.51

# nmake nodebug=1
# nmake tune=1
# nmake profile=1

# Alternatively define the environment variables NODEBUG, TUNE, PROFILE
# Default of winntmak.bat is to call with "nodebug=1"
# Needed cause debug switches are on by default for WinDebug source level debug

# can also define CPU=i386, CPU=MIPS, CPU=ALPHA, CPU=PPC - defaults to i386

# nmake macros for building Windows 32-bit apps - see there for flags

!include <win32.mak>

MODULE=dviwindo

# all: dviwindo.exe

ALL: $(MODULE).exe

# object modules to build

OBJECTS = dviwindo.obj winpslog.obj winanal.obj winprint.obj \
	 winfonts.obj winsearc.obj winspeci.obj \
	 wintiff.obj

ETC = dviwindo.res dviwindo.def makefile

# RESOURCES = dviwindo.rc dviwindo.ico \
RESOURCES = dviwindo.rc dviwindo.dlg dviwindo.ico \
	dviblank.cur dvihand.cur dvisciss.cur dvizoom.cur

# Gs disable stack checking calls
# Gf enable string pooling
# Ow assume cross-function aliasing
# YX automatic .PCH
# Fm name map file ?
# Zp pack structures
# Ze enable extensions (default)
# CFLAGSw = -c -YX -Fm -W4 -AM -G2 -GA -Gs -Gf -Ow -Zpe -DSTRICT
# CFLAGSw = -c -YX -Gs -Gf -Ow -W3 -DNT -DSTRICT
# CFLAGSw = -c -YX -Gs -Gf -Ow -W3 -DWIN32 -DSTRICT
# CFLAGSW = -c -YX -Gs -Gf -Ow -W3 -DWIN32 -DSTRICT

# /NOD		NODEFAULTLIB
# /AL:16	align
# /MAP		make mapfile
# LFLAGS =  /map/line/align:16/NOD
# LFLAGS =  /map/li/al:16/NOD
# LFLAGS =  /map/li/al:16/NOD
# LFLAGS =  /map

# Use MicroSoft LINK

# LINK = link

# Following is new attempt  93/Oct/4 (added in dviwindo.res, removed ::

# dviwindo.exe: dviwindo.obj winpslog.obj winanal.obj winprint.obj winfonts.obj winsearc.obj winspeci.obj dviwindo.res dviwindo.def
#    LINK $(LFLAGS) dviwindo winpslog winanal winprint winfonts winsearc winspeci dviwindo.res
#    rc -v dviwindo.res

# dviwindo.res: dviwindo.rc dviwindo.dlg dviwindo.h dvihand.cur dvizoom.cur dvisciss.cur dviwindo.ico 
#   rc -r dviwindo.rc

# $(MODULE).res: $(RESOURCES) dviwindo.rc 
$(MODULE).res: $(RESOURCES) dviwindo.h
# $(MODULE).rbj: $(RESOURCES) dviwindo.h
#	rc -r -fo $(MODULE).res $(MODULE).rc
	rc -r -DWIN32 -fo $(MODULE).res $(MODULE).rc
# need this dviwindo.rbj thing
	cvtres -$(CPU) $(MODULE).res -o $(MODULE).rbj
#	removed following to prevent relinking even when up to date!
#	del $(MODULE).res

$(MODULE).rbj: $(MODULE).res dviwindo.h
	cvtres -$(CPU) $(MODULE).res -o $(MODULE).rbj

# compile the sources

# CFLAGS = -c -YX -Gs -Gf -Ow -W3 -DWIN32 -DSTRICT

$(OBJECTS): $*.C dviwindo.h winbased.h winhead.h winextra.h
#	production version
	$(cc) -YX $(cflags) $(cdebug) $(cvars) -W4 $*.C
#	debugging version
#	$(cc) -YX $(cflags) $(cdebug) $(cvars) -W4 -Od -Zi $*.C

# $(MODULE).exe: $(OBJECTS) dviwindo.res dviwindo.def
$(MODULE).exe: $(OBJECTS) dviwindo.rbj dviwindo.def
#	production version
	$(link) /map $(guiflags) $(linkdebug) -out:$(MODULE).exe \
	$(OBJECTS) dviwindo.rbj $(guilibs) shell32.lib atm.lib
#	debugging version
#	$(link) /MAP /DEBUG /PDB:dviwindo.pdb $(guiflags) $(linkdebug) -out:$(MODULE).exe \
#	$(OBJECTS) dviwindo.rbj $(guilibs) shell32.lib atm.lib

# default compiler flags -c -YX -W3 -nologo -Z7 -Od -DWIN32 -D_WIN32 
# -D_X86_=1 -DWINVER=0x0400 
# -Z7 C7 style CodeView information
# -Od - disable all optimizations (default)
# -nologo suppress copyright message
# -YX[file] automatic .PCH

# -Zi - generate debugging information
# -Zd - generate only public symbols and line number for debugging
# -Ox - use maximum optimizations
# -Gz - stdcall (i386 specific flags)

# default link flags
# /NODEFAULTLIB /INCREMENTAL:NO /PDB:NONE /RELEASE /NOLOGO -align:0x1000 
# -subsystem:windows,4.0 -entry:WinMainCRTStartup 
# -debug:full -debugtype:cv
# libc.lib oldnames.lib 
# kernel32.lib advapi32.lib user32.lib gdi32.lib comdlg32.lib winspool.lib
# shell32.lib 

