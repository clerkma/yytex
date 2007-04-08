****************************************************************************
Copyright (c) 1991 -- 1992, Y&Y, Inc.
Copyright 2007 TeX Users Group.
You may freely use, modify and/or distribute this file.
****************************************************************************

===========================================================================
BSR Computer Modern font set in ATM compatible Adobe Type 1 format (IBM PC)
===========================================================================

Most of the information about this font set may be found in the
printed manual that comes with the IBM PC version of the fonts.  
Some of the main points are summarized here. 
Some additional trouble-shooting information may be found at the end.

Using the CM fonts in Adobe Type 1 format with TeX:
---------------------------------------------------

The CM fonts in Adobe Type 1 format have exactly the same metrics as
corresponding bitmapped CM fonts, hence the same TFM files are used by TeX.
TeX itself concerns itself only with metric information --- which is
contained entirely in the TFM file. The same DVI file is used for CM fonts
in Adobe Type 1 format as for corresponding bitmapped CM fonts in PK format.

The DVI processor (previewer or printer driver), however, has to be able to
deal with fonts in Adobe Type 1 format (referred to as `PostScript' fonts by
some and `ATM fonts' by others).  How this works is a function of which DVI
processor is being used.  Please read the manual for your DVI processor.  

In the case of DVIPSONE, simply copy the outline font files in PFB
format to the directory `c:\psfonts' (or any of the directories on
the path pointed to by the environment variable PSFONTS, or mentioned
on the command line following `-f=').  DVIPSONE gets all metric
information from the outline font itself, so the AFM, PFM, and TFM 
files are *not* needed.

For DVIWindo, simply install the fonts using the ATM control panel.
ATM will copy the PFB and PFM files to the hard disk and make them
available to all applications.

For use with DVIPS, fonts in Type 1 format have to be listed in the
file `psfonts.map' (typically found in `/usr/lib/tex/ps'). If the
fonts have PostScript FontNames different from the TFM file name,
than both TFM file name and PostScript FontName must be mentioned on
the line listing a particular font (see `psfonts.cm').

DVIPS can be asked to `automatically download' the outline font for
the duration of the print job.  In this case each line added to
`psfonts.map' should end with `<' followed by the file name of the
corresponding PFA (or PFB) file.  DVIPS will typically look for the
outline font files in PFA (or PFB) format in the directory
`/usr/lib/tex/ps'  (see `psfonts.cmx' or `psfonts.cmz').

Older versions of DVIPS can only handle the verbose PFA format.  
Newer versions (recommended) can also handle the more compact PFB format.
You can convert from PFB to PFA format using PFBtoPFA, if needed.

IBM PC utility programs supplied with the fonts:
------------------------------------------------

As a convenience, a few IBM PC compatible utility programs (extension
`.exe') are supplied with the fonts.  Note that these are not required
for normal use of the fonts.  Utilities for converting outline fonts
between PFA (Printer Font ASCII) and PFB (Printer Font Binary) format
are included, as is a utility for reencoding PFB and PFA files.

Most of the utilities are described in the manual that comes with the
IBM PC compatible version of the font set.  To see what command line
flags and command line arguments any one of the utilities takes, just
invoke it with `-?'  on the command line.

Using CM fonts with applications other than TeX:
------------------------------------------------

For use of these fonts with applications other than TeX, you may be
forced to exploit the utility called SPACIFY:

SPACIFY can be used to put a `space' character in character code position 32
of the text fonts.  This is useful for use of these fonts with applications
other than TeX.  Please read `spacify.txt' for additional details.  
The file `spacify.txt' also indicates how to put a `space' character in
character code position 32 by editing the PFA and AFM version of a font. 

Problems with ATM for Windows:
------------------------------

One of the bugs in ATM 2.5 for Windows is that it cannot handle repeated
encodings.  A character that appears more than once in the encoding vector
can only be accessed through the first mentioned encoding.  So if, for
example, the character `suppress' appears in character code positions 32,
128, and 195, and the `dup 195 /space def' appears first in the encoding
vector, then that character can only be accessed using `Alt 0 1 9 5', not by
typing the `space' key, or using `Alt 0 1 2 8' (Remember to have `Num Lock'
on when using this method).  What is worse, ATM 2.5 goes into `invisible ink
mode' if you try to access the character using one of these alternatives ---
characters in the same output string following the `offending' character will
not be shown on screen.

Similarly, characters in code positions 0 - 31 should be accessed through
their alternate codes (161 - 170 or 172 - 194) in non-TeX applications.

IMPORTANT NOTE:	These problems have been fixed in ATM 2.6

We also recommend that you use the utility `spacify' to put a `space' 
character in code position 32.  See above.

ATM installs fonts by loading PFB (outline font) and PFM (metric info) files.
If there are no PFM files, ATM can create them using AFM and INF files
(which are plain ASCII human readable files).  But ATM's MAKEPFM routine
cannot handle non-integer character widths in the AFM file, and the CM
fonts have non-integer character widths.  So make sure to always provide PFB
and PFM files rather than let ATM try and make up PFM files from AFM files.

Problems with Adobe Font Downloaders:
-------------------------------------

If your DVI printer driver uses `partial font downloading' of Type 1
fonts, then there will not be a need to download fonts to a printer.

If you do need to download fonts, you should be able to use the utility
DOWNLOAD supplied with the fonts to download fonts to VM or HD on your
printer.  If for some reason you need to resort to Adobe's downloaders,
then please note that some of the Adobe font downloaders will not
recognize fonts with names that are not extended to a full 8 characters
with underscores.  You can use these Adobe downloaders if you first rename
the PFB and PFM files so they all have the required trailing underscores.

This works for 68 of the 75 CM fonts.  For seven fonts, however, the
font file name is more than 6 characters, which some Adobe downloaders 
also do not like.  In this case, just make up some arbitrary short 
name with trailing underscores and use that name for both the PFB and 
PFM  files (this will not affect the PostScript FontName).

The Adobe Font Foundry used for making bitmapped fonts has similar problems.
It also requires AFM and INF files in addition to PFB and PFM files.
The AFM and INF files are not normally copied to hard disk by ATM.
