****************************************************************************
	Copyright (c) 1991 -- 1993, Y&Y, Inc. All rights reserved.
	Placed in the public domain in 2006 by the TeX Users Group.
****************************************************************************

======================================================================
The AMS font set in fully hinted, ATM compatible, Adobe Type 1 format:
======================================================================

The AMS font set in Adobe Type 1 format contains six Euler fonts
(each in three design sizes), two math symbol fonts (each in three
design sizes), a bold math italic fonts and a bold math symbol font
(each in two design sizes), a math extension font, five Cyrillic fonts, 
and one small caps font.

The Euler Font Project:
-----------------------

The Euler fonts were designed by Hermann Zapf and converted from his
drawings to METAFONT code by the Euler Project at Stanford, under the
direction of Charles Bigelow in his Digital Typography Project.
David Siegel, Carol Twombly, Scott Kim, Dan Mills, Lynn Ruggles were
amongst the main contributors (see `The Euler Project at Stanford' 
by David R. Siegel, Computer Science Department, Stanford, 1985).

The Cyrillic fonts were designed by Dr. Thomas Ridgeway at the
University of Washington Humanities and Arts Computing Center.

The American Mathematical Society designed the AMS math fonts.

Fonts Included:
---------------

Included are three design sizes of each of the regular and bold styles
of each of the `Regular', `Script', and `Fraktur' fonts:

	eurm10,	eurm7, eurm5,
	eurb10,	eurb7, eurb5,
	eufm10,	eufm7, eufm5,
	eufb10,	eufb5, eufb5,
	eusm10,	eusm7, eusm5,
	eusb10, eusb7, eusb5,

three design sizes of the two mathematical symbol fonts:

	msam10, msam7, msam5,	
	msbm10, msbm7, msbm5, 

the mathematical extension font:

	euex10

two design sizes of the bold math italic and the bold math symbol fonts:

	cmmib5, cmmib7,		
	cmbsy5, cmbsy7

as well as the five Cyrillic fonts:

	wncyr10, wncyb10, wncyi10, wncysc10, wncyss10

and one small caps font:

	cmcsc9

Absolutely Accurate Outlines:
-----------------------------

It may interest you to know that the six Euler fonts (each in three
design sizes for a total of eighteen) were converted directly from the
METAFONT source by Y&Y in order to ensure perfect accuracy.  Algorithms 
for doings this could be developed because the Euler fonts use METAFONT 
in a restricted way (except for a few changes made later by Donald E. Knuth)

The latest version (V2.1) of the METAFONT source available from the
American Mathematical Society was used in the conversion. 

The fonts are Adobe Type Manager (ATM) compatible and so can be used
with applications on the Macintosh and with MS Windows applications
on IBM PC compatibles.  They are also compatible with Display
PostScript on the NeXT.

File Format:
-----------

(*) For use on PC compatibles, the outline fonts themselves are provided in
    the compact PFB (Printer Font Binary) form, along with Windows font 
    metric files in PFM (Printer Font Metric) form.

(*) For use on Unix systems and NeXT computers, the outline fonts themselves
    are provided in PFA (Printer Font ASCII) form, along with Adobe font
    metric files in AFM form. 

(*) The Macintosh version of the font set has the outline fonts in Mac format
    and the metric information in the `screen fonts' --- actually in the FOND
    resource of the `screen fonts'.  There is also a file of TeX metric
    information in the form expected by Textures. 

For IBM PC compatibles, we include some simple utilities for converting
between PFA and PFB format, in case this should prove useful.

NOTE: The TeX font metric files are from the standard distribution.
If you are a TeX user, you most likely already have these TFM files.

Installation - IBM PC compatibles (DOS and MS Windows):
------------------------------------------------------

For use with Adobe Type Manager (ATM) on IBM PC Windows simply
install using the ATM control panel.  If you are not using ATM, 
install instead using the Windows PostScript driver.

For use with DVIWindo in MS Windows, install using ATM as above.

For use with DVIPSONE, install using ATM as above, or, if you are not 
using ATM, simply copy the PFB files to a directory where DVIPSONE 
expects outline fonts to reside (typically c:\psfonts).  Metric files 
are not needed in this case.  

With some other applications, you will need to determine where they keep
outline font files (PFB or PFA) and font metric files (PFM or AFM).

Installation - Unix/NeXT:
-------------------------

Note that DOS diskettes can be read on Sun workstations.  Do:

	man mtools

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
the line listing a particular font (see `psfonts.am').

DVIPS can be asked to `automatically download' the outline font for
the duration of the print hob.  In this case each line added to
`psfonts.map' should end with `<' followed by the file name of the
corresponding PFA (or PFB) file.  DVIPS will typically look for the
outline font files in PFA (or PFB) format in the directory
`/usr/lib/tex/ps'  (see `psfonts.amx' or `psfonts.amz').

Older versions of DVIPS can only handle the verbose PFA format.  
Newer versions (recommended) can also handle the more compact PFB format.
You can convert from PFB to PFA format using PFBtoPFA if needed.
See section at the end regarding problems with older versions of DVIPS.

For installation on the NeXT, read `nextfont.txt' and use the installation
script `nextinst.bat'.  The fonts can be used with any program on the
NeXT, not just TeX and DVIPS.  For use on the NeXT, the fonts are
provided in PFA format instead of PFB.

Rarely used Design Sizes:
-------------------------

NOTE: the bitmapped (PK font) version of the AMS font set contains additional
sizes of some of the fonts.  The Type 1 version of the font set includes
only the sizes called for by the AMS TeX macros.  The AMS TeX macros are set
up to take account of the fact that the Type 1 version of the font set does
not contain the rarely used sizes.  You may want to check that the line: 
	
	\def\PSAMSFonts{TT}%  Y&Y / Blue Sky Research PS AMS fonts: True

is uncommented in the files `\amstex\amsppt.sty',
`\amsfonts\doc\userdoc.cyr', and \amstex\doc\amsppt.doc'.

For AMS LaTeX, use the corresponding files `ams/amslatex/inputs/amsfonts.sty'
`ams/amslatex/inputs/fontdef.ams', and `ams/amslatex/fontsel/fontdef.max'
that are supplied here.

Note also that DVIPSONE comes with a `font substitution' file called
`amsfont.sub' that may prove useful.  It maps the rarely used point sizes
6pt, 8pt and 9pt onto the more common 5pt, 7pt, and 10pt.
Similarly, DVIWindo comes with a font name remapping file (called
`dviwindo.fn5') that can be renamed `dviwindo.fnt', and used to remap the
rarely used sizes.  But this shouldn't be necessary if \PSAMSFonts is 
defined to be TT rather than TF.

While there are significant differences in shape between corresponding glyphs
in fonts with very different design sizes, differences between fonts with
nearby design sizes tend to be small.  So, for example, a font designed for a
9pt size can be well approximated by a font with 10pt design size used at 9pt.

Remapping of character codes:
---------------------------

Since the character codes 0--31 and 127 cannot be generated from the keyboard
on the Macintosh, these characters have been remapped to the 161--196 range.
The range 0--9 can be accessed via codes 161--170, while 10--32 can be
accessed via 173--195, and 127 can be accessed via 196.

All of these codes can be generated by suitable key chords using the shift
and option keys on the Macintosh.  On IBM PC compatibles these characters can
be generated by holding down the `Alt' key, typing `0' and the numeric code
on the numeric key pad (with `Num Lock' on).  Of course, you don't need
to use these methods when the fonts are used with TeX.

Using the Euler Fonts in TeX for ordinary Text:
-----------------------------------------------

The Euler fonts were designed to provide symbols in mathematical
typesetting.  The TeX metric files (TFM) specify that the word space
cannot shrink or grow.  This means that one will get under- and
overfull boxes on ever line when attempting to typeset ordinary text
using one of the fonts. 

One solution to this problem is to modify the TFM files.  The easiest
way to do this is change the line 

`Comment Space 333 0 0'

in the corresponding AFM file and then to run Y&Y's AFMtoTFM. 
You may wish to start with an AFM file created from the
corresponding TFM file (since the TFM files in some cases specify
character height and depth that are not the actual character height
and depth).  Use Y&Y's TFMtoAFM for this purpose.

Another solution is to change the appropriate `font dimension' in the
TeX source code.  See the TeX book for details.  This may be
easier than modifying the TFM file itself.

Using the Euler Fonts in applications other than TeX:
-----------------------------------------------------

Fonts designed for use with TeX do not normally have a `space' character. 
Instead, the TFM file specifies the width of a normal word space, and
the amount by which it can stretch and shrink.  Typically there is
some other character in character code position 32 (for CM text fonts
this is `suppress', the stroke used to construct the Polish letters
`lslash' and `Lslash').

To make it possible to use the fonts with programs other than TeX, a
`space' has been inserted in position 32 in those fonts that do not
have a character there already, and in position 160 in fonts that do.
Methods for accessing this `space' character vary from system to
system. In MS Windows, for example, hold down the `Alt' key, then
type the digits 0, 1, 6, 0 on the numeric key pad (with `Num Lock' on).

This is somewhat inconvenient.  An alternative is to interchange
characters in position 32 and 160.  For the NeXT version of the
fonts, simply edit the encoding vector in the PFA file and make the
corresponding change in the AFM file (see `nextfont.txt').  A shell
script (called `spacify') is provided to make this more convenient.
On the IBM PC, use the utility `spacify.exe' to modify both PFB and PFM
files.  See `spacify.txt' for additional details.

AMS TeX and AMS LaTeX:
----------------------

The files for AMS TeX may be found in the `amsfonts' and `amstex'
subdirectory of the `ams' directory.  You can always obtain the latest
version of these files from the American Mathematical Society via anonymous
FTP on InterNet from `e-math.ams.org' in directories /ams/amsfonts and
/ams/amstex.  When you obtain the files this way, make sure to change the
setting of PSAMSFonts from false to true:

\def\PSAMSFonts{TT}%  Y&Y / Blue Sky Research PS AMS fonts: True
%%\def\PSAMSFonts{TF}% Y&Y / Blue Sky Research PS AMS fonts: False

in the files 

	ams/amsfonts/doc/userdoc.cyr
	ams/amstex/amsppt.sty
	ams/amstex/doc/amsppt.doc

We do not include the complete AMS LaTeX file set here, since it is quite
voluminous.  You can always obtain the latest version of these files from the
American Mathematical Society via anonymous FTP on InterNet from
`e-math.ams.org' in directories /ams/amslatex.  When you obtain the files
this way, make sure to replace the following files with the ones provided
here in the subdirectory `amslatex' of the directory `ams':

	ams/amslatex/inputs/amsfonts.sty
	ams/amslatex/inputs/fontdef.ams
	ams/amslatex/fontsel/fontdef.max

see the `readme.txt' file in that directory for additional details.

Note that the AMS LaTeX files on `e-math.ams.org' will be updated
when the new release of NFSS becomes available.


IBM PC utility programs supplied with the PC version of the fonts:
------------------------------------------------------------------

A number of utility programs are supplied with IBM PC versions of the fonts:

(*) PFBtoPFA converts from compact binary form to the printable ASCII format.

(*) PFAtoPFB converts from printable ASCII format to compact binary format.

PFBtoPFA and PFAtoPFB come in handy when a change is to be made to an outline
font file (such as changing the case of the name of a font, or changing the
encoding of a font).  A PFB file cannot normally be edited directly, since it
contains binary length fields.

(*) SPACIFY can be used to put a `space' character in character code
    position 32.  This is useful for use of these fonts with applications
    other than TeX.  Please read `spacify.txt' for additional details.

(*) REENCODE permanently changes the encoding vector in a PFB or PFA file.

The encoding vector to be used is specified on the command line.  The
encoding vector is stored in a file, each line of which contains a number
followed by white space and the PostScript name of the corresponding
character.  For example: `65 A'.  This provides a more efficient way of
reencoding a font than directly editing the encoding vector in the PFA file.
Sample vector files are included (extension VEC).  Copy the desired encoding
vector files to where REENCODE can find them - for example the directory from
which it is invoked.  Sample encoding vectors may be found on the diskette.

Some applications (such as DVIPSONE) can reencode a font on the fly, and so
don't normally require permanent reencoding of a font.  However, the Windows
PostScript driver (and Adobe Type Manager) do not provide for reencoding,
so REENCODE can come in handy when a font is used with a Windows application.

(*) DOWNLOAD can be used to permanently download a font to the printer, 
    to reset the printers virtual memory, or to print out a list of 
    printer-resident fonts.  

Note that some of the PostScript commands used by DOWNLOAD are not
standardized -- they work on the Apple Laser Writer and its relatives, but
may not work on some `clone' printers.  It is therefore preferably to use
specific utility programs supplied with the printer instead of DOWNLOAD.

(*) SERIAL is a program for sending files over a serial link to a PostScript
    printer. It need only be used if your printer does not support hardware
    handshaking.  SERIAL has the ability to write information returned over
    the serial link to a log file.  This can be useful for debugging.

(*) MODEX is a program for setting the baud rate on a serial line to speeds
    higher than those possible using the DOS MODE command.  Use as you would
    the DOS MODE command - 38400 and 57600 can be selected as baud rates.

Make sure to switch your printer's baud rate first (use sccbatch or
equivalent PS command - see your printer manual).

All utility programs from Y&Y give a brief usage summary when invoked without
arguments.  To see more detail, use the command line flag `-?'.  So to see
what arguments DOWNLOAD takes, for example, invoke it as follows:

	download -?

(*) EHANDLER.PS is an error handler that can be send to the PostScript
    printer for debugging purposes.

`Automatic' versus `Manual' downloading:
---------------------------------------

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

Fonts can also be downloaded `manually' by sending them to the printer after
adding the line

	serverdict begin 0 exitserver

immediately following the last of the initial comment lines in the PFA file
(Comment lines start with a `%').  If you have PFB files, first convert them
to PFA format, insert the above line, then convert back to PFB format. (In
the above it has been assumed that the printer's password is still the
factory default, namely 0). 

Sometimes `manual' downloading is used because the time to transmit a
PostScript file to the printer is an issue.  File transmission time can be
reduced by using a parallel instead of a serial printer connection.  
Or, if a serial line must be used, the baud rate can be increased.  
Most printers can handle 38,400 or 57,600 baud quite well (provided
hand-shaking is set up properly).  (The utility MODEX, supplied with
DVIPSONE, can be used to circumvent the DOS limit of 19,200 baud.  
When using Windows 3.0, one can use TurboComm from BioEngineering Research
laboratory to go beyond the built-in limit of 19,200 baud).

Applications that use `partial font downloading' (presently only DVIPSONE)
produce PostScript files that are much smaller than those produced by
applications that simply unpack the PFB file into PFA format, making 
`automatic' downloading much more attractive.

PostScript `Clone' Interpreter Bugs:
------------------------------------

These fonts contain numerous work-arounds for known bugs in clone
interpreters, such as the NewGen and LaserMaster printers, as well
as font downloaders on image setters.  Contact us for modified fonts 
that work on first generation NewGen printers.

NeWSPRint Bugs:
---------------

Like most PostScript `clones', older versions of NeWSPRint (release 2.0) 
have many bugs in Type 1 font interpretation.  The fonts included here 
contain work-arounds for most of the known bugs.  On problem could not be
solved without impacting the quality of the fonts.  To deal with this
problem, use `euexnews.pfa' instead of `euex10.pfa' --- after renaming 
it `euex10.pfa'.  Make sure to get the latest version of NeWSPrint 
(presently release 2.1) --- it is somewhat better. 

PostScript Interpreter Test Programs:
------------------------------------

In the PS subdirectory you will find nine PostScript test programs, and the
error handler (ehandler.ps).  These test files may come in useful should
there be a problem printing PostScript files using these fonts.  The test
programs check for various known bugs in `clone' interpreters, particularly 
as regards Type 1 font interpretation.  There is a short readme.txt file
in the same directory that describes the tests in more detail.

Problems with ATM for Windows:
------------------------------

One of the bugs in the current version of ATM (2.5 and before) for Windows is
that it cannot handle repeated encodings.  A character that appears more than
once in the encoding vector can only be accessed through the first mentioned
encoding.  So if, for example, the character `suppress' appears in character
code positions 32, 128, and 195, and the `dup 195 /space def' appears first
in the encoding vector, then that character can only be accessed using `Alt 0
1 9 5', not by typing the `space' key, or using `Alt 0 1 2 8'.  What is
worse, ATM goes into `invisible ink mode' if you try to access the character
using one of these alternatives --- characters in the same string after this
point will not be shown on screen.

Similarly characters in code positions 0 - 31 should be accessed through
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

====>	Get the	latest version of DVIPS!	<========

Seriously, it will be well worth your while.  DVIPS is constantly evolving
and improving, as well as adapting new features found in other DVI drivers. 
Here is some advice should you be FORCED to use an older version:

(1)	Older versions of DVIPS can only handle Type 1 fonts in PFA format.
	They cannot unpack the more compact PFB format.  To deal with this,
	you will have to run the PFB files through the utility PFBtoPFA.

(2)	Older versions of DVIPS have a problem with fonts that use repeated
	encodings, because DVIPS modifies the metrics of `PostScript' fonts
	based on information in the TFM files.  The Lucida New Math fonts use
	repeated encodings because (a) TeX expects certain characters to be
	in the control character range (0 - 31) and at the same time (b) many
	applications cannot access characters in the control character range.

DVIPS redefines the metrics of a font by referring to the TFM file in a way
that causes the second appearance of a character in the encoding to end up
with zero width.  This means that ligatures and Greek characters will
overprint, since the higher numbered encodings are listed first 
(in order to deal with a `mis-feature' of ATM for Windows).  
There are several work-arounds for this problem.  

(a)	One is to prevent DVIPS from redefining the font's metrics.  
	In the file `texps.pro' change

		/Metrics currentdict put
	to
		/Bogus currentdict put

(b)	Another way is to change the order in which the characters are
	accessed, thereby assuring that the one with the smaller character
	code  (the one called for by TeX) gets the non-zero metric
	information.   Replace

		0 1 255 { 2 copy get...
	with
		0 1 255 { 255 exch sub 2 copy get...

(c)	A third option is to remove the repeated encodings from the fonts
	themselves. Delete the lines starting with 

		dup 161 ... put
	through 
		dup 196 ... put

	after the line 

		/Encoding 256 array

This change can be made directly on a PFA file, but not on a PFB file, since
the latter contains binary segment length codes which will be invalidated by
any editing. If you are using PFB files, convert them first to PFA format
using PFBtoPFA, edit the PFA file and then convert back to PFB form using
PFAtoPFB.   Of course, this is assuming that you will never want to access
characters in the `control character' range (0 - 31 and 127) using the
alternate character codes. 

Using the AMS fonts with GhostScript:
-------------------------------------

Please add the contents of the file fontmap.add to the end of the 
GhostScript FontMap file in order to use the AMS fonts with GhostScript.
