# Copyright 2007 TeX Users Group.
# You may freely use, modify and/or distribute this file.
# 
# =========================================================================
# Win32.Mak - Win32 application master NMAKE definitions file for the
#     Microsoft Plaform SDK for Win32 and Win64 programming samples
#           Copyright 1991 - 1999 Microsoft Corporation
# -------------------------------------------------------------------------
# This files should be included at the top of all MAKEFILEs as follows:
#  !include <Win32.Mak>
# -------------------------------------------------------------------------
#
# Define APPVER = [ 4.0 | 5.0 ] prior to including win32.mak to get
#  build time checking for version dependencies and to mark the executable
#  with version information.
#
# Define TARGETOS = [ WIN95 | WINNT | BOTH ] prior to including win32.mak
#  to get some build time checking for platform dependencies.
#
# Define TARGETLANG = [ LANG_JAPANESE | LANG_CHINESE | LANG_KOREAN ] prior
#  to including win32.mak to getcompile & link flags for building
#  applications to run on Far-East Windows. (This is an optional parameter.
#  The system locale is the default.)
#
# Define _WIN32_IE = [ 0x0300 | 0x0400 | 0x0500 ] prior to including win32.mak to
#  get compile and link flags for building applications and components to
#  run on Internet Explorer. (This is an optional parameter.  IE 4.0 is
#  the default.)
#
# -------------------------------------------------------------------------
# NMAKE Options
#
# Use the table below to determine the additional options for NMAKE to
# generate various application debugging, profiling and performance tuning
# information.
#
# Application Information Type         Invoke NMAKE
# ----------------------------         ------------
# For No Debugging Info                nmake nodebug=1
# For Working Set Tuner Info           nmake tune=1
# For Call Attributed Profiling Info   nmake profile=1
#
# Note: The three options above are mutually exclusive (you may use only
#       one to compile/link the application).
#
# Note: creating the environment variables NODEBUG, TUNE, and PROFILE is an
#       alternate method to setting these options via the nmake command line.
#
# Additional NMAKE Options             Invoke NMAKE
# ----------------------------         ------------
# For No ANSI NULL Compliance          nmake no_ansi=1
# (ANSI NULL is defined as PVOID 0)
#
# =========================================================================
# Build Rules Quick Start
#
# To build one of the following types of executables, use the specified
# compiler and linker command-line options.
#
#  ---------------------------------------------------------------------------
#   To build:         |  Compiler Options      | Linker options (pick one
#                     |                        | line. con = console,
#                     |                        | gui = GUI, ole = GUI OLE)
#  ---------------------------------------------------------------------------
#  Single threaded    | cdebug cflags cvars    | ldebug guilflags guilibs
#  app with static    |                        | ldebug conlflags conlibs
#  CRT                |                        | ldebug guilflags olelibs
#  ---------------------------------------------------------------------------
#  Multithreaded app  | cdebug cflags cvarsmt  | ldebug guilflags guilibsmt
#  with static CRT    |                        | ldebug conlflags conlibsmt
#                     |                        | ldebug guilflags olelibsmt
#  ---------------------------------------------------------------------------
#  Single or multi-   | cdebug cflags cvarsdll | ldebug guilflags guilibsdll
#  threaded app with  |                        | ldebug conlflags conlibsdll
#  DLL version of CRT |                        | ldebug guilflags olelibsdll
#  (MSVCRT.DLL)       |                        |
#  ---------------------------------------------------------------------------
#  DLL with static    | cdebug cflags cvarsmt  | ldebug dlllflags guilibsmt
#  CRT*               |                        | ldebug dlllflags conlibsmt
#                     |                        | ldebug dlllflags olelibsmt
#  ---------------------------------------------------------------------------
#  DLL with DLL       | cdebug cflags cvarsdll | ldebug dlllflags guilibsdll
#  version of CRT     |                        | ldebug dlllflags conlibsdll
#  (MSVCRT.DLL)       |                        | ldebug dlllflags olelibsdll
#  ---------------------------------------------------------------------------
#
#  * Always make DLLs multithreaded because a DLL has no way to know whether
#    the calling application has multiple threads, and has no way to prevent
#    multithreaded apps from loading it.
#
# =========================================================================

!IFNDEF _WIN32_MAK_
_WIN32_MAK_ = 1

# -------------------------------------------------------------------------
# Get CPU Type - exit if CPU environment variable is not defined
# -------------------------------------------------------------------------

# Win95 does not define PROCESSOR_ARCHITECTURE - default to i386

!IF "$(PROCESSOR_ARCHITECTURE)" == ""
CPU=i386
PROCESSOR_ARCHITECTURE=x86
!endif

!IF !DEFINED(CPU) || "$(CPU)" == ""
CPU = $(PROCESSOR_ARCHITECTURE)
!ENDIF # CPU

# if PROCESSOR_ARCHITECTURE was x86 or X86 change CPU to i386

!IF ( "$(CPU)" == "X86" ) || ( "$(CPU)" == "x86" )
CPU = i386
!ENDIF # CPU == X86

!IF "$(CPU)" != "i386"
!IF "$(CPU)" != "ALPHA"
!IF "$(CPU)" != "ALPHA64"
!IF "$(CPU)" != "IA64"
!ERROR  Must specify CPU environment variable ( CPU=i386, CPU=ALPHA, CPU=IA64, CPU=ALPHA64)
!ENDIF
!ENDIF
!ENDIF
!ENDIF


# -------------------------------------------------------------------------
# Get Target Operating System - Default to WINNT
# -------------------------------------------------------------------------
!IFNDEF TARGETOS
TARGETOS = WINNT
!ENDIF

!IF "$(TARGETOS)" != "WINNT"
!IF "$(TARGETOS)" != "WIN95"
!IF "$(TARGETOS)" != "BOTH"
!ERROR Must specify TARGETOS environment variable (BOTH, WIN95, WINNT)
!ENDIF
!ENDIF
!ENDIF

# default to APPVER of 4.0

!IFNDEF APPVER
APPVER = 4.0
!ENDIF

!IF "$(APPVER)" != "5.0"
!IF "$(APPVER)" != "4.0"
!ERROR Must specify APPVER environment variable (4.0, 5.0)
!ENDIF
!ENDIF

!IF "$(APPVER)" =="5.0"
!IFNDEF _WIN32_IE
_WIN32_IE = 0x0500
!ENDIF # _WIN32_IE
!ENDIF # APPVER == 5.0

!IFNDEF _WIN32_IE
_WIN32_IE = 0x0400
!ENDIF

# -------------------------------------------------------------------------
# Check to see if Cole Porter is used
# -------------------------------------------------------------------------

!ifdef COLE_64
SUBSTITUTE_CC = Port64
!endif

!ifndef CC_NAME
!if "$(SUBSTITUTE_CC)" != ""
CC_NAME   = "$(SUBSTITUTE_CC)"
LINK_NAME = Cole & Rem no link when using Cole
LIB_NAME = Rem no lib when using Cole since the Cole .Objs are text
!else
CC_NAME   = cl
LINK_NAME = Link
LIB_NAME  = Lib
!endif
!endif

# -------------------------------------------------------------------------
# binary declarations common to all platforms
# -------------------------------------------------------------------------
cc     = $(CC_NAME)
rc     = Rc
link   = $(LINK_NAME)
implib = $(LIB_NAME)
hc     = Start /Wait Hcrtf -xn

# for compatibility with older-style makefiles
cvtobj = REM !!! CVTOBJ is no longer necessary - please remove !!!
cvtres = REM !!! CVTRES is no longer necessary - please remove !!!

# -------------------------------------------------------------------------
# Platform Dependent Compile Flags - must be specified after $(cc)
#
# Note: Debug switches are on by default for current release
#
# These switches allow for source level debugging with WinDebug for local
# and global variables.
#
# Both compilers now use the same front end - you must still define either
# _X86_ or _ALPHA_.  These have replaced the i386 and ALPHA definitions
# which are not ANSI compliant.
#
# Common compiler flags:
#   -c   - compile without linking
#   -W3  - Set warning level to level 3
#   -Zi  - generate debugging information
#   -Od  - disable all optimizations
#   -Ox  - use maximum optimizations
#   -Zd  - generate only public symbols and line numbers for debugging
#
# i386 specific compiler flags:
#   -Gz  - stdcall
#
# -------------------------------------------------------------------------

# declarations common to all compiler options
ccommon = -c -DCRTAPI1=_cdecl -DCRTAPI2=_cdecl -nologo

# for compatibility with old source code, map {try, except, leave, finally}
# to their proper names (i.e. prefaced by "__")
!IFDEF SEHMAP
ccommon = $(ccommon) -FIsehmap.h
!ENDIF

!IF "$(TARGETLANG)" == "LANG_JAPANESE"
ccommon = $(ccommon) -DJAPAN -DDBCS -DFE_IME
!ENDIF

!IF "$(TARGETLANG)" == "LANG_CHINESE"
ccommon = $(ccommon) -DDBCS -DFE_IME
!ENDIF

!IF "$(TARGETLANG)" == "LANG_KOREAN"
ccommon = $(ccommon) -DDBCS -DFE_IME
!ENDIF


!IF "$(CPU)" == "i386"
cflags = $(ccommon) -D_X86_=1  -DWIN32 -D_WIN32 -W3 -Z7
scall  = -Gz
lflags = -pdb:none
!ELSEIF "$(CPU)" == "ALPHA"
cflags = $(ccommon) -D_ALPHA_=1  -DWIN32 -D_WIN32 -W3 -Z7
scall  =
lflags = -pdb:none
!ELSEIF "$(CPU)" == "IA64"
cflags = $(ccommon) -DWIN64 -D_WIN64 -D_IA64_=1 -DWIN32 -D_WIN32
# Compiler setting for test building 64-bit
cflags = $(cflags) -Wp64 -Zs -W4
scall  =
link = @Echo NOTE not linking for $(CPU): link
!ELSEIF "$(CPU)" == "ALPHA64"
cflags = $(ccommon) -DWIN64 -DALPHA=1 -DWIN32
# Compiler setting for building Win64
cflags = $(cflags) -Wp64 -W4 -Ap64 -Zi
scall  =
lflags = -debugtype:cv,fixup -pdb:$(@R).Pdb
!ENDIF


!IF "$(APPVER)" == "4.0"
NMAKE_WINVER = 0x0400
!ELSEIF "$(APPVER)" == "5.0"
NMAKE_WINVER = 0x0500
!ENDIF


!IF "$(TARGETOS)" == "WINNT"
cflags = $(cflags) -D_WINNT -D_WIN32_WINNT=$(NMAKE_WINVER)
!ENDIF

!IF "$(TARGETOS)" == "WIN95"
cflags = $(cflags) -D_WIN95 -D_WIN32_WINDOWS=$(NMAKE_WINVER)
!ENDIF

# regardless of the TARGET OS, define compile time WINVER to match APPVER macro
cflags = $(cflags) -D_WIN32_IE=$(_WIN32_IE) -DWINVER=$(NMAKE_WINVER)


!IFDEF NODEBUG
cdebug = -Ox -DNDEBUG
!ELSE
!IFDEF PROFILE
cdebug = -Gh -Zd -Ox -DNDEBUG
!ELSE
!IFDEF TUNE
cdebug = -Gh -Zd -Ox -DNDEBUG
!ELSE
cdebug = -Z7 -Od
!ENDIF
!ENDIF
!ENDIF

# -------------------------------------------------------------------------
# Target Module & Subsystem Dependent Compile Defined Variables - must be
#   specified after $(cc)
#
# The following table indicates the various acceptable combinations of
# the C Run-Time libraries LIBC, LIBCMT, and MSVCRT respect to the creation
# of a EXE and/or DLL target object.  The appropriate compiler flag macros
# that should be used for each combination are also listed.
#
# Executable Type          C Runtime Lib        Compiler switch
# -------------------------------------------------------------
# Single threaded app      static CRT           CVARS       *
# Single-threaded app      DLL CRT              CVARSDLL
# Multi-threaded app       static CRT           CVARSMT     *
# Multi-threaded app       DLL CRT              CVARSDLL    *
#
# Single threaded DLL      static CRT           CVARS
# Single-threaded DLL      DLL CRT              CVARSDLL
# Multi-threaded DLL       static CRT           CVARSMT     *
# Multi-threaded DLL       DLL CRT              CVARSDLL    *
#
# * - Denotes the Recommended Configuration
#
# When building single-threaded applications you can link your executable
# with either LIBC, LIBCMT, or MSVCRT, although LIBC will provide the best
# performance.
#
# When building multi-threaded applications, either LIBCMT or MSVCRT can
# be used as the C-Runtime library, as both are multi-thread safe.
#
# Note: Any executable which accesses a DLL linked with MSVCRT.LIB must
#       also link with MSVCRT.LIB instead of LIBC.LIB or LIBCMT.LIB.
#       When using DLLs, it is recommended that all of the modules be
#       linked with MSVCRT.LIB.
#
# Note: The macros of the form xDLL are used when linking the object with
#       the DLL version of the C Run-Time (that is, MSVCRT.LIB).  They are
#       not used when the target object is itself a DLL.
#
# -------------------------------------------------------------------------

!IFDEF NO_ANSI
noansi = -DNULL=0
!ENDIF


# for Windows applications that use the C Run-Time libraries
!IFDEF NODEBUG
cvars      = $(noansi) -ML
cvarsmt    = $(noansi) -D_MT -MT
cvarsdll   = $(noansi) -D_MT -D_DLL -MD
!ELSE
cvars      = $(noansi) -MLd
cvarsmt    = $(noansi) -D_MT -MTd
cvarsdll   = $(noansi) -D_MT -D_DLL -MDd
!ENDIF


# for compatibility with older-style makefiles
cvarsmtdll = $(cvarsdll)

# for POSIX applications
psxvars    = -D_POSIX_

# resource compiler
rcflags = /r
rcvars = -DWIN32 -D_WIN32 -DWINVER=$(NMAKE_WINVER) $(noansi)

!IF "$(TARGETLANG)" == "LANG_JAPANESE"
rcflags = $(rcflags) /c932
rcvars = $(rcvars) -DJAPAN -DDBCS -DFE_IME
!ENDIF

!IF "$(TARGETLANG)" == "LANG_CHINESE"
rcvars = $(rcvars) -DDBCS -DFE_IME
!ENDIF

!IF "$(TARGETLANG)" == "LANG_KOREAN"
rcvars = $(rcvars) -DDBCS -DFE_IME
!ENDIF



# -------------------------------------------------------------------------
# Platform Dependent Link Flags - must be specified after $(link)
#
# Note: $(DLLENTRY) should be appended to each -entry: flag on the link
#       line.
#
# Note: When creating a DLL that uses C Run-Time functions it is
#       recommended to include the entry point function of the name DllMain
#       in the DLL's source code.  Also, the MAKEFILE should include the
#       -entry:_DllMainCRTStartup$(DLLENTRY) option for the creation of
#       this DLL.  (The C Run-Time entry point _DllMainCRTStartup in turn
#       calls the DLL defined DllMain entry point.)
#
# -------------------------------------------------------------------------

# declarations common to all linker options
lflags	= $(lflags) /INCREMENTAL:NO /NOLOGO

# declarations for use on Intel x86 systems
!IF "$(CPU)" == "i386"
DLLENTRY = @12
!ENDIF

# declarations for use on self hosted Digital Alpha AXP systems
!IF "$(CPU)" == "ALPHA"
DLLENTRY =
!ENDIF

# -------------------------------------------------------------------------
# Target Module Dependent Link Debug Flags - must be specified after $(link)
#
# These switches allow the inclusion of the necessary symbolic information
# for source level debugging with WinDebug, profiling and/or performance
# tuning.
#
# Note: Debug switches are on by default.
# -------------------------------------------------------------------------

!IFDEF NODEBUG
ldebug = /RELEASE
!ELSE
!IFDEF PROFILE
ldebug = -debug:mapped,partial -debugtype:coff
!ELSE
!IFDEF TUNE
ldebug = -debug:mapped,partial -debugtype:coff
!ELSE
ldebug = -debug:full -debugtype:cv,fixup
!ENDIF
!ENDIF
!ENDIF

# for compatibility with older-style makefiles
linkdebug = $(ldebug)

# -------------------------------------------------------------------------
# Subsystem Dependent Link Flags - must be specified after $(link)
#
# These switches allow for source level debugging with WinDebug for local
# and global variables.  They also provide the standard application type and
# entry point declarations.
#
# Note that on x86 screensavers have a WinMain entrypoint, but on RISC
# platforms it is main.  This is a Win95 compatibility issue.
#
# -------------------------------------------------------------------------

# Windows 98 needs subsystem version set to 4.10 for version 5.0 features.
!IF ("$(APPVER)" == "5.0") && (("$(TARGETOS)" == "BOTH") || ("$(TARGETOS)" == "WIN95"))
EXEVER = 4.10
!ELSE
EXEVER = $(APPVER)
!ENDIF


# ---------------------------------------------

# for Windows applications that use the C Run-Time libraries
conlflags = $(lflags) -subsystem:console,$(EXEVER)
guilflags = $(lflags) -subsystem:windows,$(EXEVER)
dlllflags = $(lflags) -entry:_DllMainCRTStartup$(DLLENTRY) -dll

# For screen savers
!IF "$(CPU)" == "i386"
savlflags = $(lflags) -subsystem:windows,$(EXEVER) -entry:WinMainCRTStartup
!ELSE
savlflags = $(lflags) -subsystem:windows,$(EXEVER) -entry:mainCRTStartup
!ENDIF

# for POSIX applications
psxlflags = $(lflags) -subsystem:posix -entry:__PosixProcessStartup

# for compatibility with older-style makefiles
conflags  = $(conlflags)
guiflags  = $(guilflags)
psxflags  = $(psxlflags)

# -------------------------------------------------------------------------
# C Run-Time Target Module Dependent Link Libraries
#
# Note: For POSIX applications, link with $(psxlibs).
# -------------------------------------------------------------------------

# for POSIX applications
psxlibs    = libcpsx.lib psxdll.lib psxrtl.lib oldnames.lib


# optional profiling and tuning libraries
!IFDEF PROFILE
optlibs =  cap.lib
!ELSE
!IFDEF TUNE
optlibs = wst.lib
!ELSE
optlibs =
!ENDIF
!ENDIF

# if building for basic Windows 95, use WinSock1, else use WinSock2
!IF "$(TARGETOS)" == "WIN95"
!IF "$(APPVER)" == "4.0"
winsocklibs = wsock32.lib
!ELSE
winsocklibs = ws2_32.lib mswsock.lib
!ENDIF
!ELSE
winsocklibs = ws2_32.lib mswsock.lib
!ENDIF


# basic subsystem specific libraries, less the C Run-Time
baselibs    = kernel32.lib $(optlibs) $(winsocklibs) advapi32.lib
winlibs     = $(baselibs) user32.lib gdi32.lib comdlg32.lib winspool.lib

# for Windows applications that use the C Run-Time libraries
conlibs     = $(baselibs)
guilibs     = $(winlibs)

# for OLE applications
olelibs     = ole32.lib uuid.lib oleaut32.lib $(guilibs)

#for backwards compatibility
conlibsmt   = $(conlibs)
conlibsdll  = $(conlibs)
guilibsmt   = $(guilibs)
guilibsdll  = $(guilibs)
olelibsmt   = $(olelibs)
olelibsdll  = $(olelibs)

# for backward compatibility
ole2libs    = $(olelibs)
ole2libsmt  = $(olelibsmt)
ole2libsdll = $(olelibsdll)

# Visual Basic
bc     = vb6
bc_exe = /Make
bc_dll = /Makedll

#ENDIF _WIN32_MAK_
!ENDIF

# /nologo /MLd /W4 /Gm /GX /Zi /Od /D "WIN32"/D "_DEBUG"/D "_WINDOWS" /YX /c
# /nologo /ML /W4 /GX /O2 /D "WIN32"/D "NDEBUG"/D "_WINDOWS" /YX /c
