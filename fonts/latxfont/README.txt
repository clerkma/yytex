****************************************************************************
Copyright (c) 1991, 1993, 1997 Y&Y, Inc. 
Copyright 2007 TeX Users Group.
You may freely use, modify and/or distribute this file.
****************************************************************************

==========================================================================
LaTeX + SliTeX font set in fully hinted ATM compatible Adobe Type 1 format:
==========================================================================

This LaTeX+ SliTeX font diskette contains:

	(*) Adobe Type 1 outline font files (PFB or PFA extension);
	(*) font metric files (AFM and TFM extension);
	(*) utility programs for IBM PC compatibles (EXE extension);

for:	(*) LaTeX line, circle, symbol fonts (5 design sizes) & bold symbol,

		line10, linew10,
		lcirclel, lcirclew,
		lasy5, lasy6, lasy7, lasy8, lasy9, lasy10,  &  lasyb10

	(*) SliTeX sans serif fonts:

		lcmss8, lcmssb8, lcmssi8

	(*) SliTeX `invisible' fonts:

		ilcmss8, ilcmssb8, ilcmssi8
		icmtt8, icmmi8, icmmsy8, icmex10, ilasy8,

	(*) logo fonts (used for the words METAFONT and METAPOST):

		logo8, logo9, logo10,
		logosl8, logosl9, logosl10,
		logod10, logobf10

(Note that this includes the new logod10 font, as well as new sizes of the
slanted LOGO font.  Also, the LOGO fonts now include all letters needed
for the word METAPOST in addition to the letters for the word METAFONT).

The IBM PC compatible versions of the fonts also include `Printer Font Metric'
(PFM) files for use with MS Windows.

The fonts are all fully hinted (both character-level and font-level), and so
will render well even on low-resolution (on screen) and medium-resolution
(laser printer) devices.  The fonts are completely compliant with the Type 1 
specification and, in addition, are ATM compatible (a tighter constraint).  
They also include work arounds for numerous bugs in `clone' interpreters.

The SliTeX fonts include the `invisible' fonts, although these will not 
often be used, since the they do not contain any actual character outlines.
To save space you may want to load only the 14 visible LaTeX + SliTeX fonts, 
and NOT load the 8 `invisible' fonts (names starting with `i').  
The same goes for the 5 `logo' fonts, which only contain the letters 
`M', `E', `T', `A', `F', `O', `N', `P' and `S.'

It may interest you to know that the LaTeX fonts and the LOGO fonts
were converted directly from the METAFONT source by Y&Y to ensure
perfect accuracy.  This was possible because the LaTeX fonts and LOGO
fonts use METAFONT in a rather restricted way.

Outline font file format:
-------------------------

The actual outline fonts themselves are supplied in  

	(*) Printer Font Binary (PFB) form; or 
	(*) Printer Font ASCII (PFA) form; or
	(*) Macintosh outline font format.

PFB files are typically used on IBM PC compatibles, and are what the Windows
PostScript driver and Adobe Type Manager (ATM) expect to see.  PFB file are
compact, but need to be `unpacked' into the more verbose hexadecimal PFA
format before being sent to a printer.  The PS printer driver does this.

On Unix and NeXT systems, it has been customary to use the PFA format, since
outline fonts in this form can be sent directly to the printer.  PFA files
can be included in a PostScript document that calls for a particular font.
Some applications (such as DVIPSONE) can handle fonts in either format.

For the NeXT version of the font set, see `nextfont.txt' for installation
instructions and use the script `nextinst.bat'.

Font Metric Files (AFM, TFM, PFM, and Mac screen font):
------------------------------------------------------

Adobe Type 1 outline fonts are normally supplied with human-readable 
metric files.  From these, compact binary application-specific metric files 
(such as TFM and PFM files) can be constructed.  

The IBM PC compatible versions of the fonts include `Printer Font Metric'
(PFM) files for use with Windows.  (These can also be constructed from the
AFM files using the utility AFMtoPFM that comes with DVIWindo).

The TeX font metric files for these fonts are also included --- in the TFM
subdirectory --- but these are just from the standard TeX distribution and
most TeX users already have these.

Installation - IBM PC compatibles (DOS and MS Windows):
------------------------------------------------------

For use with Adobe Type Manager (ATM) on IBM PC Windows simply install using
the ATM control panel.  When not using ATM, install instead using the Windows
PostScript driver. For use with DVIWindo in MS Windows, install using ATM.

For use with DVIPSONE, install using ATM as above, or, when not using ATM,
simply copy the PFB files to a directory where DVIPSONE expects outline fonts
to reside (typically c:\psfonts).  Metric files are not needed in this case.

With some other applications, you will need to determine where they keep
outline font files (PFB or PFA) and font metric files (PFM or AFM).

Installation - Unix/NeXT:
-------------------------

DOS diskettes can be read on Sun workstations.  Do:  `man mtools'
to see what commands are available for manipulating diskettes.
You can for example do `mdir a:' to see what files are on a diskette
and `mcopy a:*.*' to copy all the files in the top-level directory
on the diskette to your current directory.

If you can't find `mtools' on your system, try the InterNet.
For example, look in /src/unixdos/mtools on `world.std.com'.

(Note MTOOLS interprets DOS file names as all uppercase.  This is OK
for the font files themselves, since the fonts have all upper case names
anyway.  But you may find it convenient to rename some other files).

For use with DVIPS, fonts in Type 1 format have to be listed in the
file `psfonts.map' (typically found in `/usr/lib/tex/ps'). If the
fonts have PostScript FontNames different from the TFM file name,
than both TFM file name and PostScript FontName must be mentioned on
the line listing a particular font (see `psfonts.lt').

DVIPS can be asked to `automatically download' the outline font for
the duration of the print hob.  In this case each line added to
`psfonts.map' should end with `<' followed by the file name of the
corresponding PFA (or PFB) file.  DVIPS will typically look for the
outline font files in PFA (or PFB) format in the directory
`/usr/lib/tex/ps'  (see `psfonts.ltx' or `psfonts.ltz').

For installation on the NeXT, read `nextfont.txt' and use the installation
script `nextinst.bat'.  The fonts can be used with any program on the
NeXT, not just TeX and DVIPS.  For use on the NeXT, the fonts are provided
in PFA format instead of PFB.

Remapping of character codes:
----------------------------

Since the character codes 0--31 and 127 cannot be generated from the keyboard
on the Macintosh, these characters have been remapped to the 161--196 range.
The range 0--9 can be accessed via codes 161--170, while 10--32 can be
accessed via 173--195, and 127 can be accessed via 196.

All of these codes can be generated by suitable key chords using the shift
and option keys on the Macintosh.  On IBM PC compatibles these characters can
be generated by holding down the `Alt' key, typing `0' and the numeric code
on the numeric key pad.  Of course, you don't need to use these methods when
the fonts are used with TeX.

IBM PC utility programs supplied with the PC fonts:
--------------------------------------------------

PFBtoPFA converts from the compact binary form to the printable ASCII format.

PFAtoPFB converts from printable ASCII format to the compact binary format.

PFBtoPFA and PFAtoPFB come in handy when a change is to be made to an outline
font file.  A PFB file cannot normally be edited directly, since it contains
binary length fields.

SPACIFY can be used to put a `space' character in character code
position 32.  This is useful for use of these fonts with applications
other than TeX.  Please read `spacify.txt' for additional details.

REENCODE permanently changes the encoding vector in a PFB or PFA file.
The encoding vector is stored in a file that can be specified on the
command line.  Each line of the file simply contains a number followed by a
space or tab and the PostScript name of the corresponding character.
For example:	`65	A'.  This provides a more efficient way of reencoding
a font than directly editing the encoding vector in the PFA file.

Some applications (such as DVIPSONE) can reencode a font on the fly, and so
don't require permanent reencoding of a font.  However, the Windows
PostScript driver (and Adobe Type Manager) do not provide for reencoding,
so REENCODE can come in handy when a font is used with a Windows application.

DOWNLOAD can be used to permanently download a font to the printer, to
reset the printers virtual memory, or to print out a list of printer resident
fonts.  Note that some of the PostScript commands used by DOWNLOAD are not
standardized -- they work on the Apple Laser Writer and its relatives, but
may not work on some `clone' printers.  It is therefore preferably to use
specialized utility programs supplied with the printer instead of DOWNLOAD.

SERIAL can be used to send PostScript files to a printer connected over
a serial link that does not support hardware hand-shaking  (The DOS
COPY command can be used for printers connected over serial links that
support hardward hand-shaking).  Serial is also handy when the information 
sent back over the serial line from the printer is of interest.

All utility programs from Y&Y give a brief usage summary when invoked without
arguments.  To see more detail, use the command line flag `-?'.  So to see
what arguments DOWNLOAD takes, invoke it as follows:

	download -?

`Automatic' versus `Manual' downloading:
----------------------------------------

`Automatic' (i.e. temporary) downloading is normally the best approach 
in order to avoid filling up printer memory and to prevent print job
interdependence.  In this case the font is sent down to the printer
as part of the PostScript job being printed.  The font is removed from
printer memory at the end of the print job.

`Manual' (i.e. permanent) downloading can sometimes be useful when a printer
is used primarily with a fixed set of fonts (and these all fit into printer's
virtual memory).  Font downloaders can be used to permanently download
outline fonts.  A `manually' downloaded font remains resident until the
printer power is turned off (or the PostScript `quit' command is executed). 

The IBM PC utility DOWNLOAD supplied with the fonts can be used to download a
font `manually'/`permanently' if needed.

Fonts can also be downloaded `manually' by sending them to the printer after
adding the line

	serverdict begin 0 exitserver

immediately following the last of the initial comment lines in the PFA file
(Comment lines start with a `%').  If you have PFB files, first convert them
to PFA format using the PFBtoPFA utility, insert the above line, then convert
back to PFB format using the PFAtoPFB utility. (In the above it has been
assumed that the printer's password is still the factory default, namely 0).

Sometimes `manual' downloading is used because the time to transmit a
PostScript file to the printer is an issue.  File transmission time can be
reduced by using a parallel instead of a serial printer connection.  Or, if a
serial line must be used, the baud rate can be increased.  Most printers can
handle 38,400 or 57,600 baud quite well (provided hand-shaking is set up
properly).  (The utility MODEX, supplied with DVIPSONE, can be used to
circumvent the DOS limit of 19,200 baud.  Windows 3.1 also allows one to
set a port to a higher baud rate for use from within Windows.

Applications that use `partial font downloading' (such as DVIPSONE)
produce PostScript files that are much shorter than those produced by
applications that simply unpack the PFB file into PFA format, making 
`manual' downloading of much less interest.

PostScript `Clone' Interpreters Bugs:
----------------------------------

Most clone PostScript interpreters have some `misfeature' or other,
particularly when it comes to Type 1 font interpretation.  Such interpreters
may work prefectly well on plain vanilla Adobe text fonts, yet fail on more
complex fonts that fall perfectly well within in the Type 1 specification and
are even ATM compatible (a tighter constraint).

PostScript Interpreter Test Programs:
------------------------------------

In the PS subdirectory you will find nine PostScript test programs, and the
error handler (ehandler.ps).  These test files may come in useful should
there be a problem printing PostScript files using these fonts.  The test
programs check for various known bugs in `clone' interpreters, particularly 
as regards Type 1 font interpretation.  There is a short `readme.txt' file
in the same directory that describes the tests in more detail.

Problems with ATM for Windows:
-----------------------------

One of the bugs in older versions of ATM for Windows is that they cannot
handle repeated encodings.  Make sure to use a recent version (ATM 3.02).

Characters in code positions 0 - 31 should be accessed through
their alternate codes (161 - 170 or 172 - 194) in non-TeX applications.

We also recommend that you use the utility `spacify' to put a `space' 
character in code position 32.  This is not neccessary in the case of the
Macintosh version of the fonts since the `coordinated' fonts already
have a space character in character code position 32.

Problems with Adobe Font Downloaders and Adobe Font Foundry:
------------------------------------------------------------

We do not recommend Adobe Font Downloaders, since they are slow and not very
user friendly --- use the DOWNLOAD utility instead if possible.  If you are
forced to use a downloader from Adobe, be aware that some of them will
*only* recognize fonts that have file names that have been padded out to a
full eight characters using the underscore character.  These downloaders also
will not recognize fonts that have more than six real characters in the file
name (i.e. names with fewer than two trailing underscore characters).
The file names of both PFB and PFM files must have this form.

We do not recommend making bitmapped versions of these fonts, but if you
do use Adobe Font Foundry (supplied with any Adobe font set), then please
note that the foundry requires not only PFB and PFM files, but also AFM
and INF files.  The default directories for these files are c:\psfonts,
c:\psfonts\pfm, c:\psfonts\afm, c:\psfonts\fontinfo.  ATM does *not* copy
the AFM and INF files from the font distribution diskettes (since it doesn't
need them).  You'll have to copy them to the appropriate directories.
Also note that the font foundry program also does not recognize fonts
with names that have not been padded out with underscore characters...

Problems with old versions of DVIPS:
------------------------------------

If you have an older version of DVIPS, the best advice is:  
Get the	latest version of DVIPS (at least 5.74 say).

Seriously, it will be well worth your while.  DVIPS is constantly evolving
and improving, as well as adapting new features found in other DVI drivers. 

Using the extra LaTeX + SliTeX fonts with Ghostscript:
------------------------------------------------------

Please add the contents of the file fontmap.add to the end of the 
Ghostscript FontMap file in order to use the extra LaTeX + SliTEX fonts with
Ghostscript. 
