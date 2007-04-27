Y&Y TeX and its components are Copyright (c) 1993-96, 07 Y&Y, Inc.
Copyright 2007 TeX Users Group.
See the top-level README and individual files for more on copying
conditions.  Generally, the distribution is now free (as in freedom)
software.  (You may freely use, modify and/or distribute this file.)

Y&Y includes:
(*)     Complete TeX packages, including DVIWindo, DVIPSONE, ATM and font sets

(*)     DVIWindo -- unique in providing for `on the fly' font reencoding
(*)     DVIPSONE -- unique in partial font downloading of PS Type 1 fonts
(*)     Y&Y TeX -- unique in dynamic memory allocation on IBM PC

(1)     CM fonts -- all 75 CM fonts in fully hinted ATM Type 1 format
(2)     extra LaTeX + SliTeX fonts
(3)     AMS Font Set in Type 1 format (includes `extra' AMS fonts)


Repository
==========
The files here come directly from the Y&Y master machine, where they
were arranged in a similar structure.

bib          - a few example bib files.
bin/bat      - PC batch scripts, both internal and external.
bin/unixprog - third-party pfa/pfb conversion scripts.
fonts        - European Modern and other fonts as prepared by Y&Y.  
               (The proprietary Lucida and MathTime fonts are not included.)
ps           - PostScript test and utility files.
src          - the C sources for dviwindo, dvipsone, yandytex.
tex          - documentation source, Y&Y information sheets, etc.
txt          - plain text tips, flyers, etc.

For a listing of the Y&Y distribution as it was installed on one user's
machine, see the file YYFILES.txt.

No Windows binaries are included.  Since the binaries as I found them
all included, or may have included, proprietary libraries, I felt it
would be unwise to distribute them.

I myself do not use Windows and cannot do any development of the
software here.  I've taken the time to make these files available in the
hope that there will be interest in the TeX community to revive the Y&Y
distribution, and/or integrate the Y&Y programs, fonts, and other files
into other free software packages, and that others will come forward to
lead the project from this point.  I will be most pleased to turn over
administration of the project.

I hope that any new distribution based on Y&Y will follow the TeX
Directory Structure and use the current versions of TeX-related packages
and programs, e.g., from CTAN (http://www.ctan.org).


Provenance
==========
After Y&Y, Inc. was dissolved, its software assets were voluntarily
donated to TUG in fall 2005.  TUG extends many thanks to Blenda Horn for
this extremely generous donation.

Files previously copyrighted by Y&Y ("all rights reserved") are now
copyrighted by TeX Users Group (TUG).  Source code for the main programs
is released under the GNU GPL, miscellanous documentation, font, and
other files under an all-permissive license, so that they can be reused
wholly or partially in other projects.

Some older Y&Y copyright statements remain, in particular in the .pfb
files.  They should be disregarded, and replaced if anything is ever
actually done with the fonts.  (I simply did not wish to take the time
to unpack, edit, and repack all those files.)  Similarly, old product
announcements, bug fixes, and other documentation files were left in the
repository for historical purposes.

I started with a raw dump of the Y&Y machines.  I did my best to remove
Y&Y business correspondence and other irrelevant files, but with such a
large file set, it seems inevitable that some extraneous files remain,
and conversely that some necessary files were removed.  Please email
any problems, suggestions, questions.

I hope these files will be useful to the TeX community.

Karl Berry, for TUG  -  http://tug.org/yandy/  -  yandytex@tug.org

With, again, major thanks to Blenda Horn.  Thanks also to David Walden,
Christina Thiele, Robin Fairbairns, Mimi Burbank, Barbara Beeton, Jochen
Autschbach, and the members of the Y&Y mailing list.
