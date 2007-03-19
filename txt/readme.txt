	Placed in the public domain in 2006 by the TeX Users Group.

==============================================
	Welcome to the Y&Y TeX System release 2.2!
==============================================

Installation is covered by the single page installation sheet
(which is also the file `cdinstal.txt' on the CD-ROM).

Basic information is in the Y&Y TeX System Manual.
See section 2.5 for a quick system usage tutorial.

Esoteric details are covered in the `Technical Addendum'.
Even deeper technical topics are addressed in `*.txt' files 
in the `doc-misc' subfolder of your `yandy' folder.  

Technical issues specific to particular system components 
are addressed in `readme.txt' files in the corresponding 
sub-folder (dviwindo, dvipsone, and yandytex).

Information on LaTeX 2e is available in HTML form
at c:\yandy\tex\latex2e\doc\html\index.html ---
accessible from `Start > Programs > YandY22'

For detailed list of what is available, check `overview.txt'

The previewer -- DVIWindo -- is the heart of the system. To 
launch it use `Start > Programs > YandY22 > DVIWindo22'
--- or double click the Y&Y icon on the desk top.

Windows 2000 Professional:

W2K has built in support for Type 1 fonts. Do *not* install
ATM.  Instead, use the "Font Installation for W2K" menu
item obtained by right clicking on the CD icon.

ATM:

In the ATM folder on the CD you will find ATM 4.0 for 
Windows 95/98, and ATM 4.0 for Windows NT 4.0. Install 
ATM *first*.  This will also automatically install the fonts that 
come with your TeX System, as well as the new `base 13' 
fonts (PostScript 3 versions of Times, Helvetica and Courier).

Tutorial:

To gain some familiarity with the system, you may want to
walk through the simple exercise in section 2.5 of the Y&Y
TeX System manual.

If you want to see how to get started with LaTeX 2e when 
using different font sets, look at 
	hello_LB.tex (for Lucida Bright), 
	hello_MT.tex (for MathTime), and
	hello_EM.tex (for European Modern).
in the `texinput' folder in your `yandy' folder. 
The corresponding .dvi files are also there. In DVIWindo,
open the one corresponding to the font set you have.

Installing over an earlier release of Y&Y TeX System:

The LaTeX 2e low-level folder structure has changed 
slightly.  The installation did not delete the folder that 
is now redundant, just in case you have added some files.
But you may now want to remove this folder in order 
to save disk space, and to avoid the possibility of having 
two copies of a style file, one possibly out of date.

So, delete c:\yandy\tex\latex2e (drag it to the recycle bin).

LaTeX contributed packages:

A collection of contributed packages from CTAN - both 
`supported' and `other' - may be found on the CD in the 
`ctan\latex\contrib' folder. This totals around 200 Mb, 
so  the installation leaves it on the CD.  To use any of 
these packages, copy to the appropriate folder under
c:\yandy\tex\latex2e\contrib\supported or ...contrib\other.

Hyphenation patterns:

Hyphenation patterns for many languages may be found in the
folder `ctan\language\hyphenation' on the CD.  You can use
these to create new formats with hyphenation patterns for
several languages.  In this case the `babel' package may be 
of interest (note that the Babel package is not installed
unless you specifically request it when installing LaTeX 2e).
Or, read `initex.txt' in the doc-misc folder and
check out the sample file `hyphen.smp.'

XY Pic fonts:

If you use the XY Pic drawing package then you can either
set it up to use TPIC \specials, or, alternatively, the XY
fonts.  We supply the latter in ATM compatible Adobe Type 1
format in the xy_fonts folder on the CD.  Install as you
would any other fonts -- `Start > Programs > Adobe > ATM'.

Other Type 1 fonts:

If you wish to use fonts other than the ones we supply,
create TFM files for them using `Fonts > WriteTFM...' in 
the DVIWindo previewer.  First check that the correct 
encoding is selected in `Fonts > Encoding'.  For convenient
use with LaTeX 2e, you may also need to make suitable `font
definition' (*.fd) and style (*.sty) files.  The easiest
way to do this is to look at what is in the PSNFSs folder
(\yandy\tex\latex2e\required\PSNFSS) and modify copies of 
files there. See Chapter 7 of `The LaTeX Companion' 
ISBN 0-201-54199-8

Documentation:

In c:\yandy\tex\latex2e\doc\html you will find documentation 
in HTML form that you can view with your browser.  More on 
the CD.

Shareware:

We include some handy shareware programs on the CD: 
	(1) UltraEdit32 (alternate editor)
	(2) JPEG2PS (convert .jpg images to .eps form)
	(3) WMF2EPS (convert .wmf graphics to .eps form)
	(4) PKWARE's PKZIP204G 
	(5) CourierX a heavier version of Courier (free!)
Remember that the idea of shareware is that you get to 
try these products for free, but are expected to pay the
registration fee if you do use them on a continuing basis.

PCL printer drivers:

Windows PCL printer drivers do not handle reencoded fonts
well. Many of these devices can be made to deal properly
with font reencoding, but in some cases it can be tedious 
to discover how.  Details may be found in `win95.txt'.  
An alternative is to use Windows ANSI encoding when dealing
with such a device.  Yet another alternative is to disable
use of f-ligatures using a Y&Y TeX command line (-2).

Last updated 2000 Jan 1

	(NOTE: this is the file `readme.txt' in `doc-misc')
