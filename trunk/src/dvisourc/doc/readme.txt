Copyright 2007 TeX Users Group.
You may freely use, modify and/or distribute this file.
===========================================================================
	Release Notes for DVIPSONE 1.2 (1995/May/1)	(file:readme.txt)
===========================================================================

Contents:
---------

 0.	Installation Problems - DOS clones.
 1.	Partial Font Downloading
 2.	True Page Independence
 3.	Optimizations / Clone Interpreter Bugs
 4.	Potential problems with some printer-resident `Synthetic Fonts'
 5.	Potential problems with `Font Wrappers'
 6.	Working with Textures files
 7.	Complete list of command line flags for DVIPSONE
 8.	Complete list of command line arguments for DVIPSONE
 9.	Font Naming Issues and Font Encoding
10.	Saving disk space
11.	Uninstalling DVIPSONE

-------> NOTE: please read `news.txt' for *really* late breaking news <------

0. INSTALLATION PROBLEMS --- DOS clones and the NUL device.
-----------------------------------------------------------

Some DOS clones deal incorrectly with batch file statements of the form

	if exists <path>\nul ...   and  if not exists <path>\nul ...  

which are used to test for existence of directories in the installation
batch file.  Comment out lines in `install.bat' containing this construction
if this is a problem on your system (You may then get some `Directory already
exists' messages, which you can ignore).

Some DOS clones also cannot handle redirection of output using the > character
and have trouble with the characters ` or ' in comments, so we avoid those...

(By `DOS clone' we mean 4DOS, NDOS, DRDOS, Compaq DOS, DOS in OS/2, etc).

1. PARTIAL FONT DOWNLOADING:
----------------------------

We didn't really know in the past just how much partial font downloading
contributes to reduction of PostScript file size.  We did know it played a
major role in keeping down printer virtual memory usage --- which is quite
important, since TeX loves to call upon many fonts.  In fact other DVI
processors often run out of printer VM when a DVI file calls for many fonts
(This is particularly likely with older typesetters and printers).

Recently we did some comparisons between DVIPSONE output and output
produced by drivers that download Type 1 fonts as a whole.  On short jobs
calling for 8 to 12 fonts (a page of math say), the output from other
drivers can be four to six times as large as that from DVIPSONE!
The ratio is smaller on long jobs that use only a few fonts --- or jobs
that depend mostly on printer-resident fonts.  

You can see for yourself just how impressive the savings are by
comparing the PostScript file normally produced by DVIPSONE with that
obtained when using `-P' on the command line --- which disables
partial font downloading.

The reduction in file size not only translates into less storage space,
but a large reduction in time to transmit the file to the output device,
as well as a large reduction in time to interpret the PostScript file.
(Transmission of large files over serial lines can take irritatingly
long, even when MODEX is used to up the baud rate above the DOS limit
of 19200 baud to say 57600 baud).

***	DVIPSONE is the only application --- of any kind ---
***	that uses partial font downloading of Adobe Type 1 fonts!

2. True Page Independence:
--------------------------

We have also learnt that page imposition software has an easy time with
PostScript files produced by DVIPSONE, because DVIPSONE enforces true
page independence, and because the output conforms to Document Structuring
Conventions version 3.0.  Pages can be arbitrarily permuted in the
PostScript  file and arbitrary pages can be removed --- the new PostScript
file will still work correctly.  The utility TWOUP exploits this
(TWOUP is a kind of `poor mans' page imposition software with
somewhat limited capabilities).

Note that some Page Imposition software cannot believe that a file
is truly page-independent!  So you are forced to specify a `filter' ---
in that case select the `Borland Sprint filter' --- it is a safe choice,
since output from Borland Sprint also requires no additional processing.

We also have made output from DVIPSONE compatible with commercial
color separation software, such as InSight System's `Publishers Prism'.

3. Optimizations / Clone Interpreter Bugs:
------------------------------------------

We now `program to the lowest common denominator'.  That is, the default is
to turn off many optimizations, simply because one clone PS interpreter or
another has a bug relating to that optimization.  Turning these optimizations
off reduces the chance of unexpected problems with clone interpreters, but
also curtails some of the benefits of the clever things that DVIPSONE can do.

*	If you have used any of these flags in the past (mostly undocumented
*	so far), please read this section and check section 8, since clone
*	interpreter bugs have forced us to change the default of several flags.

You may want to experiment by turning some of these optimizations back on
again using the following flags:

R	use `newline' only as line terminator (not `return' + `newline')
J	do not force the /Encoding array to have 256 elements
Z	strip out comments in EPS file (optimization, unsafe for AI EPS files)
*l	Use short octal sequences for small char codes (not for old ALW)
*o	Do not include optional DSC comments (such as %%BeginProlog) in PS file
N	do not force in base & accent of accented chars (if you don't use `em)
n	allow use of the DVIENCODE numeric encoding vector

These tricks may save you a bit in file size and VM usage on the printer.
It is probably best though not to use the extra optimization when maximum
portability of the PostScript file is paramount - given todays `clone' world!

Conversely, if you have problems with a particular clone you may want to turn
off some of the remaining `safe' optimization:

O	do not use \b \t \n \f \r to shorten strings (remove optimization).
L	keep white space at end of Subrs/CharStrings (remove optimization)
Y	do not strip comments when sending output directly to the printer.

A small number of poorly designed clone interpreters cannot even handle
partial font downloading (the usual symptom is corruption of the font cache
after printing for a while).  For these, turn off partial font downloading
using the `P' command line flag --- and get ready for PS files that are as
huge as those produced by other DVI-to-PS converters!

Some service bureaus insist on `stripping' fonts out of the PostScript file,
particularly when using page imposition software.  Such `stripped' fonts will
typically not work on their own, since they refer to encoding vectors defined
in the prolog.  Use the -*T command line flag to force DVIPSONE to write the
encoding vector out in full.  Keep in mind however that there may still be
problems due to partial font downloading (unless you use -P) if the page
imposition software is used to combine different PS jobs calling for the same
fonts.   In this case you are better off telling DVIPSONE that all fonts are
printer resident (-k flag) and having the service bureau get their own copy
of the fonts and downloading these ahead of time.

4. Potential problems with some printer resident `Synthetic Fonts':
-------------------------------------------------------------------

Synthetic fonts are fonts based on other fonts.  They use the CharStrings
from the base font, but have a different FontMatrix.  This technique is
sometimes used to make oblique, narrow, or expanded versions of font
(as opposed to having a separate design).

Certain PS interpreters (such as QMS PS 815 MR) cannot handle a down-loaded
`synthetic' fonts when the base font is one of the printer resident fonts.
For example, Helvetica-Oblique is a synthetic font that contains a copy of
Helvetica in its encrypted section.  This is wrapped in some code that reads
over and discards the embedded font if (a) a font of that name already
exists, and (b) if that font has the same UniqueID and (c) if that font
is a Type 1 font.

If all three tests succeed, then the new font borrows the CharStrings of
the base font to construct the oblique version.  If the font is printer
resident, however, then the CharString dictionary may not contain the
CharStrings themselves, but indeces into a string called CharOffsets,
which in turn contains offsets into a string called CharData.  This is
the format of a Type 5 font.  And unfortunately on some printers such
fonts are incorrectly labelled Type 1!

In this situation nothing will print when the synthetic font is selected.
This is not a problem due to DVIPSONE, but happens whenever a synthetic
font is downloaded that contains a base font that is printer resident.

The solution is to simply use the printer resident version of the
synthetic font and save the downloading.  Use *resident* in the font
substitution file (and *force*, otherwise DVIPSONE will not refer
to the substitution if it finds a PFB file first).


5. Potential problems with `Font Wrappers':
-------------------------------------------

Some fonts use a `wrapper' --- bits of code added near the beginning
and near the end of a font file.  The first part tests whether a Type
1 font with the same FontName and the same UniqueID already exists in
the printer.  If this is the case then the rest of the font is
wrapped in a save/restore pair.  This means it will be loaded and
then discarded.  DVIPSONE has to strip out this wrapper so it can use
partial font downloading.

There is no standard for the format of this wrapper, it can be arbitrary
PostScript codee, although the few fonts from Adobe that do use this old
technique all use the exact same sequence of PostScript commands.  We
recently came across some fonts from Digital Typeface Corporation that have
an unusual wrapper, which old versions of DVIPSONE cannot handle.  We revised
DVIPSONE so it can deal with this style of wrapper also.

(These Digital Typeface Corporation fonts are distributed with
LaserMaster printers and with some applications, such as MicroGrafx
Designer.  They were made using Fontographer, and hence are not
commercial quality fonts.  For example, there is no use made of `hint
replacement', which is required for optimal rendering of some
characters. If you have fonts with the same names made by Adobe,
make sure to use those instead of the DTC fonts.  Note that installing
Designer or LaserMaster will override your good Adobe fonts with DTC fonts
that have the same file names.  Reinstall your true Adobe fonts
after installing Designer, Corel Draw, Laser Master printers etc).


6. Working with Textures files:
-------------------------------

DVIPSONE can read Textures files directly.  There are some things to
watch out for in processing Textures files though.

One is to make sure you are using the correct encoding for fonts.
Textures EdMetrics can set up Textures metric files using a few
different encoding schemes, including the fonts `raw' or `native'
encoding (which usually is StandardEncoding) and `TeX text' encoding.
If you are uncertain about what encoding is in effect, use EdMetrics
to inspect the TeX Metrics file.  Note that what Textures calls `TeX text'
in NOT the `TeX text' encoding used in Computer Modern fonts.  

Starting with Textures 1.6 font default encoding of `PostScript' fonts
has been changed to a modification of `Mac standard roman encoding'.
Use the `texmac' encoding vector for these fonts.

DVIPSONE's default is to use the font's `raw' or `native' encoding,
which usually is StandardEncoding (unless the PFB file has been
explicitly reencoded using REENCODE).  Thus Textures files made
without reencoding will work directly.  If instead the `PostScript
fonts' in Textures are set up using `TeX text' reencoding, then set
up a substitution file specifying `*remap* Textures'. Do not specify
`textext', since what Textures considers `TeX text' encoding is
actually some derivative of StandardEncoding with the lower half
(0 - 127) replaced with more or less the true `TeX text' encoding.

Another problem is that TeX metric files for use with Textures are
sometimes set up to use the font's true PostScript FontNames (such as
`Times-Roman').  This is fine if the font is not reencoded, but
inappropriate if it is reencoded to say `TeX text'.  The reason is
that inserted EPS files may call for the same font and get the
reencoded version instead of the `real thing'.  A simple way to fix
this problem is to force DVIPSONE to add a prefix to all fontnames
using the new `F' command line flag.  For example, `-F=TeX-' will
change `Times-Roman' into `TeX-Times-Roman'.


7. Complete list of command line flags for DVIPSONE:
----------------------------------------------------

The list of command line flags and command line arguments produced in
response to `dvipsone -?' is limited to one screen full and so does not
cover some of the less frequently used flags and arguments.

Here is a complete list, including the more obscure items:

?	show detailed list of command line arguments and flags
v	be verbose
r	output pages in reverse order
g	do only odd pages
h	do only even pages --- in reverse order
q	suppress complaints about bad \special and missing fonts
I	ignore all \specials (useful if some of them crash the printer)
z	make output conform to EPS specification (leave out /jobname)
k	assume *all* fonts are printer resident (even if PFBs available)
K	assume *no* fonts are printer resident (must have PFB files then)
X	use Windows ANSI encoding for text fonts that use StandardEncoding.
U	convert CM, AMS, LaTeX, SliTeX font names to upper case (use with k)
j	allow verbatim PostScript code (and other dangerous things :=)

O	do not use \b \t \n \f \r to shorten strings (remove optimization).
L	keep white space at end of Subrs/CharStrings (remove optimization)
Y	do not strip comments when sending output directly to the printer.
P	suppress partial font downloading (generates voluminous output)

R	use `newline' only as line terminator (possibly unsafe optimization)
Z	strip out comments in EPS file (optimization, unsafe for AI EPS files)
J	allow /Encoding array to have less than 256 elements (optimization)
n	allow use of DVIENCODE encoding vector (unsafe optimization)
N	do not force in base & accent chars of accented chars (optimization)

W	do not send control D to printer at end of job
T	do not preserve current font across inserted verbatim PostScript
u	show page VM usage information (not very useful)
Q	do not strip font wrappers from wrapped fonts (NOT recommended)

M	show command line (debugging check on what is passed in call)
S	show font substitution table (useful for debugging substition file)
V	show complete font tables (useful for debugging font problems)

The following `extended' flags require a leading asterisk:

*c	Allow use of `colorimage' operator for color image TIFF file.
*b	Do not allow use of color in text and rules (all is B/W).
*T	Do not let fonts refer to encoding vectors defined in prolog.

*l	Use short octal sequences for small char codes (optimization - not ALW)
*o	Do not include optional DSC comments in PS file (optimization)

*p	Do not include inserted EPS figures in output.
*s	Copy inserted EPS files verbatim without interpretation of DSC

*f	Check that fonts called for are defined -- else give error page.
*i	Do not obey %%IncludeFont DSC comments in included EPS files.
*a	Do not add `TeX text' encoding accent chars in bottom of ANSI vector.
*n	Do not use short font numbers (use TeX's internal font numbers instead)
*m	Do not look for aliases in `texfonts.map' for missing fonts.
*S	Interpret EPS figure inclusion \specials from Scientific Word.
*R	Remap char codes 0 - 9 => 161 - 170, 10 - 32 => 172 - 195, 127 => 196
*M	reduce size of substitution table (saves memory).
*F	reduce size of font table (saves memory).
*E	Do not complain about encoding mismatch between TFM info and font.
*O	Print landscape instead of portrait orientation.

NOTE:	Command line flags toggle the corresponding variable.  You can
	also prefix the flag with `1' or `0' to force the flag `on' or `off'
	(useful	e.g.  when you can't remember what is in DVIPSONE.CMD).

If you have trouble with a clone PS interpreter, try removing some of the
remaining default optimizations (O, L, Y, *T, P), and do *not* turn on any of
the optional optimizations (J, R, N, Z, n, *l, *o).  Also try using *s if
you have problems with inserted EPS figures.  Please notify us if you can
identify which optimization a particular clone is unable to handle.  We'd
like to (i) complain to the source, (ii) provide a suitable work-around if
possible, (iii) warn other users about the problem.

Note that some command line flags (v, S, V, M) force DVIPSONE to provide
more feedback about the job --- which may be useful when debugging.

If you use verbatim PostScript you need to use `j' (and read `verbatim.txt').

If you use color, then you need to use the command line flags *c (and not *b).

8. Complete list of command line arguments for DVIPSONE:
--------------------------------------------------------

d	destination for output
b	beginning of page range (counter[0] - TeX page number)
e	end of page range (counter[0] - TeX page number)
	-b=... and -e=... may be used more than once.
B	beginning of page range (sequential dvi page count)
E	end of page range (sequential dvi page count)
	-B=... and -E=... may be used more than once.
m	magnification (if used twice: x and y magnification)
o	rotation about center of page (in degrees)
x	shift in x (in points)
y	shift in y (in points)
c	number of copies (uncollated)
C	number of copies (collated - slow printing)
l	paper size
s	font substitution file to use (declare printer resident, reencode etc)
w	header/prolog file to be inserted
F	prefix for FontName in output PS file (to avoid FontName conflict)
D	size of PostScript dictionary `dvidict' (default 256)

*h	specify frequency of halftone screen (default: use printer's default)

NOTE: You can control the screen angle as well by appending a colon followed
by the screen angle in degrees (default 45).  For example: -*h=100:60

*P	Insert fixed BoundingBox for paper type specified (e.g. -*P=letter).
	(As opposed to BoundingBox computed from page width, height & depth).
*D	Add extra DSC comment to head of PS file (e.g. -*D=%%DocumentColor:).
	If first character is not `%', then that char is an alias for space
	in the rest of the string (e.g. -*D=_%%DocumentColor:_Cyan,_Magenta).
	If the command line gets too long, use an indirect command file
	(using @filename) or `dvipsone.cmd'.
*A	Scale coordinate system used in PostScript output by given factor.
	(Useful for some faulty clone interpreters - such as Lino 50 & 
	Agfa Viper that have trouble with large scale factors in `makefont').

NOTE: In TeX, there may be several pages with the same count[0] `page number'.
With -b and -e, you can specify which `page group' you are referring to
by appending a colon -- and the number of the page group.
Each `page group' consists of pages with an ascending sequence of page
numbers.  A new group starts when a page number is not larger (in absolute
value) than the previous page number.  To select the page range 4 -- 7 in the
first `page group', for exampe, use -b=4:1 -e=7.

The following controls where DVIPSONE looks for things (typically it's more
convenient to set the environment variable shown in parentheses instead):

f	font search path					(PSFONTS)
i	EPS file search path					(PSPATH)

G	encoding vector path					(VECPATH)
H	font substitution file path				(SUBPATH)
A	preamble/header file path				(PREPATH)

a	AFM metric file search path (not normally needed)	(AFMPATH)
t	TFM metric file search path (not normally needed)	(TFMPATH)
p	PFM metric file search path (not normally needed)	(PFMPATH)

9. Font Naming Issues and Font Encoding:
----------------------------------------

If your source file uses Karl Berry names for printer resident fonts then
rename `texfonts.kb' in the TEXFONTS directory to `texfonts.map,' or use
-s=berry on the command line (Please consult the Technical Reference Manual
regarding use of font substitution files).

DVIPSONE can reencode text fonts on-the-fly.  You specify what encoding 
to use by means of the environment variable ENCODING.  DVIPSONE first
looks for environment variables in the [Environment] section of
the file dviwindo.ini.  Only if an environment variable is not found
there does DVIPSONE look for DOS  environment variables.

If you are using fonts with Windows ANSI encoding in DVIWindo, then please
instead add `Set TEXANSI=1' to your `autoexec.bat' file to ask DVIPSONE to
reencode plain text fonts to Windows ANSI encoding.

DVIPSONE can reencode fonts on-the-fly.  You can specify a different
encoding vector for each font that is to be reencoded using a font
substitution file.  Sample substitution files may be found on the
diskette in subdirectory `sub'.

If you have use the batch file `encode.bat' to set fonts up for DVIWindo,
then no additional action is needed for DVIPSONE.  The fonts are already
encoded as requried.

For encoding related issues, please consult `morass.txt'

10. Saving Disk Space:
---------------------

Many of the files on the DVIPSONE diskette are not often needed, and
the installation batch file does not copy these to the hard disk.
What is  copied takes up about 700k byte.

There are certainly files there that you can delete after reading,
such as many of the `*.txt' files --- that will save about 100k bytes.
You can also delete the sample files in the `afm' subdirectory, saving a
bit more than 100k bytes.  You save almost another 100k bytes by
deleting the font files in the `pfb' subdirectory, unless you want to
use the sample `logo' fonts.

Most of the executable files `*.exe', vector files `*.vec', and substitution
files `*.sub', on the other hand, may come in handy at some point or another.

The absolute miminum is probably `dvipsone.exe', all files with extension
`enc', `standard.sub', and `textext.vec'.


11.  Uninstalling DVIPSONE:
--------------------------

Should you wish to remove DVIPSONE from your computer, simply delete
all the files in the DVIPSONE directory and its SUB, VEC, PFB, AFM, TFM
subdirectories.  In DOS 6.0 you can do this using just one command:

	deltree /Y c:\dvipsone
