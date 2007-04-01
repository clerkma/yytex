/* Copyright 2007 TeX Users Group

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Can't use the structures defined in sfnt.h because of Big-endian order */

#define uint32 unsigned long 
#define uint16 unsigned short 
#define uint8 unsigned char

#define int32 long 
#define int16 short
#define int8 char

#ifdef IGNORED
#define FAR 

typedef struct {
	uint16 a;
	uint16 b;
} Fixed;	/* ??? */

typedef struct {
	uint16 a;
	uint16 b;
} FUnit;	/* ??? */

#include "sfnt.h"
#endif

int verboseflag=0;
int traceflag=0;
int showtablesflag=0;
int showcmapsflag=0;
int showpostflag=0;
int showencodingflag=0;	/* show cmap abbreviated to encoding vectors */
int overwriteflag=0;	/* rename file if conflict with output file name */
int strictflag=0;		/* do not process file if magic number wrong */
int shownotdef=0;

int writefileflag=1;
int fixbadflag=1;
int fixcheckflag=1;		/* fix bad checksums */
int ignorehead=1;		/* ignore bad checksum on `head' */

/* #define MAXSEG 256 */

/* unsigned int endCount[MAXSEG], startCount[MAXSEG], idRangeOffset[MAXSEG]; */
/* int idDelta[MAXSEG]; */

unsigned char *font=NULL;		/* memory for font file */

unsigned long nfilelen;

int changed;

unsigned int *endCount=NULL;
unsigned int *startCount=NULL;
unsigned int *idRangeOffset=NULL;
int *idDelta=NULL;

char *copyright="Copyright (C) 1998 Y&Y, Inc. http://www.YandY.com";

char *signature="bkph";

char infile[FILENAME_MAX], outfile[FILENAME_MAX], bakfile[FILENAME_MAX];

void freearrays (void) {
	if (endCount != NULL) free(endCount);
	endCount = NULL;
	if (startCount != NULL) free(startCount);
	startCount = NULL;
	if (idRangeOffset != NULL) free(idRangeOffset);
	idRangeOffset = NULL;
	if (idDelta != NULL) free(idDelta);
	idDelta = NULL;
}

void allocarrays (int nseg) {
	endCount = (unsigned int *) malloc (nseg * sizeof(unsigned int));
	startCount = (unsigned int *) malloc (nseg * sizeof(unsigned int));
	idRangeOffset = (unsigned int *) malloc (nseg * sizeof(unsigned int));
	idDelta = (int *) malloc (nseg * sizeof(unsigned int));
}

char **glyphnames=NULL;
char **extraglyphs=NULL;
unsigned int nextras;

char *MacGlyphs[]={		/* standard Mac glyph names */
	".notdef", "null", "CR", 
	"space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle", 
	"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", 
	"zero", "one", "two", "three", "four", "five", "six", "seven", 
	"eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question", 
	"at", "A", "B", "C", "D", "E", "F", "G", 
	"H", "I", "J", "K", "L", "M", "N", "O", 
	"P", "Q", "R", "S", "T", "U", "V", "W", 
	"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", 
	"grave", "a", "b", "c", "d", "e", "f", "g", 
	"h", "i", "j", "k", "l", "m", "n", "o", 
	"p", "q", "r", "s", "t", "u", "v", "w", 
	"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", /* "delete" */
	"Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis", "aacute", 
	"agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla", "eacute", "egrave", 
	"ecircumflex", "edieresis", "iacute", "igrave", "icircumflex", "idieresis", "ntilde", "oacute", 
	"ograve", "ocircumflex", "odieresis", "otilde", "uacute", "ugrave", "ucircumflex", "udieresis", 
	"dagger", "degree", "cent", "sterling", "section", "bullet", "paragraph", "germandbls", 
	"registered", "copyright", "trademark", "acute", "dieresis", "notequal", "AE", "Oslash", 
	"infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu", "partialdiff", "summation", 
	"product", "pi", "integral", "ordfeminine", "ordmasculine",
	"Omega", /* "Ohm" */
	"ae", "oslash", 
	"questiondown", "exclamdown", "logicalnot", "radical", "florin", "approxequal",
	"increment", /* "Delta" */
	"guillemotleft", 
	"guillemotright", "ellipsis", "nbspace", "Agrave", "Atilde", "Otilde", "OE", "oe", 
	"endash", "emdash", "quotedblleft", "quotedblright", "quoteleft", "quoteright", "divide", "lozenge", 
	"ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft", "guilsinglright", "fi", "fl", 
	"daggerdbl", "middot", /* "periodcentered" */
	"quotesinglbase", "quotedblbase", "perthousand", "Acircumflex", "Ecircumflex", "Aacute", 
	"Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex", 
	"apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde", 
	"overscore", /* "macron" */
	"breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron", 
/*	there are more `standard' Mac glyphs here to fill in up to 256+2 total */
	"Lslash", "lslash", "Scaron", "scaron", "Zcaron", "zcaron", "brokenbar",
	"Eth", "eth", "Yacute", "yacute", "Thorn", "thorn", "minus", "multiply",
	"onesuperior", "twosuperior", "threesuperior", "onehalf", "onequarter",
	"threequarters", "franc", "Gbreve", "gbreve", "Idot", "Scedilla", "scedilla",
	"Cacute", "cacute", "Ccaron", "ccaron", "dslash", ""
};

#ifdef IGNORED
/* compute checksum of a TT table */
uint32 checksum (uint32 *start, int nlen) {
	uint32 checksum=0;
	int k;
	for (k = 0; k < nlen; k++) checksum += start[k];
	return checksum;
}
#endif

uint16 uinttwo(unsigned char *s) {
	uint16 c, d;
	c = *s++;
	d = *s++;
	return (unsigned short) (c << 8 | d);
}

uint32 uintfour(unsigned char *s) {
	uint32 c, d, e, f;
	c = *s++;
	d = *s++;
	e = *s++;
	f = *s++;
	return (((c << 8 | d) << 8 | e) << 8 | f);
}

void uwritetwo(unsigned char *s, uint32 n) {
	uint32 c, d;
	d = n & 255;
	n = n >> 8;
	c = n & 255;
	*s++ = (unsigned char) c;
	*s++ = (unsigned char) d;	
}

void uwritefour(unsigned char *s, uint32 n) {
	uint32 c, d, e, f;
	f = n & 255;
	n = n >> 8;
	e = n & 255;
	n = n >> 8;
	d = n & 255;
	n = n >> 8;
	c = n & 255;
	*s++ = (unsigned char) c;
	*s++ = (unsigned char) d;
	*s++ = (unsigned char) e;
	*s++ = (unsigned char) f;
}

uint32 checktable(unsigned char *font, uint32 offset, uint32 length) {
	uint32 k;
	uint32 sum=0;
	unsigned char *s=font+offset;
	uint32 nwords = (length + 3) / 4;

	if (nwords * 4 != length) {
		if (traceflag)
			printf("%d not multiple of four\n", length);
	}
	for (k = 0; k < nwords; k++) {
		sum += uintfour(s);
		s += 4;
	}
	if (nwords * 4 != length) {
		uint32 wrd;
		s -= 4;
		wrd = uintfour(s);
		s += 4;
		if (traceflag) printf("Last one was %08X\n", wrd);
	}
	return sum;
}

uint32 version, numOffsets, searchRange, entrySelector, rangeShift;

void analyzechecksums(unsigned char *font, int start, int fixcheckflag) {
	uint32 checkSum, offset, length, sum;
	unsigned char tag[5];
	uint32 k;
	unsigned char *s=font+start;
	
	if (showtablesflag) printf("\n");
	for (k = 0; k < numOffsets; k++) {
		strncpy((char *) tag, (char *) s, 4);
		tag[4] = '\0';
		s += 4;
		checkSum = uintfour(s);
		s += 4;
		offset = uintfour(s);		
		s += 4;
		length = uintfour(s);		
		s += 4;
		if (showtablesflag)
			printf("%2d '%s'\t%08X offset %6d length %6d\n",
								k, tag, checkSum, offset, length);
		fflush(stdout);
		sum = checktable(font, offset, length);
		if (sum != checkSum) {
			if (fixcheckflag) {
				if (ignorehead && strcmp((char *) tag, "head") == 0)
					continue;
				if (verboseflag)
				printf("Bad checkSum for %s %08X versus %08X\n",
					   tag, sum, checkSum);
				uwritefour(s-12, sum);
				if (verboseflag)
				printf("Fixing checkSum for %s at %d\n", tag, s-16-font);
			}
		}
	}
}

unsigned char tag[5];
uint32 tableChecksum, tableOffset, tableLength;

unsigned long findtable(unsigned char *font, unsigned int start, char *table) {
	unsigned int k;
	unsigned char *s=font+start;
	
	for (k = 0; k < numOffsets; k++) {
		strncpy((char *) tag, (char *) s, 4);
		tag[4] = '\0';
		s += 4;
		tableChecksum = uintfour(s);
		s += 4;
		tableOffset = uintfour(s);
		s += 4;
		tableLength = uintfour(s);
		s += 4;
		if (strcmp((char *) tag, table) == 0) {
			if (verboseflag)
 printf("Found table entry for '%s' at %d offset %d length %d\n",
				   table, s-16-font, tableOffset, tableLength);
			return tableOffset;
		}
	}
	printf("Unable to find table '%s'\n", table);
	return 0;
}

int analyzecmaps(unsigned char *font, int start) {
	unsigned long nlen;
	unsigned int temp;
	uint32 k, i, j, ncmaps;
	uint32 platform, specific, cmapoffset;
	uint32 format, cmaplength, formatversion;
	unsigned int offset, cmapstart, indx;
	unsigned char *s;
	unsigned char *t;
	char *type="";
	unsigned int segCount, segSearch, entrySelect;
	unsigned short glyphIndex;
	unsigned int chr;
	unsigned short glyf;
	int glyphIDarray;

	if (verboseflag || showencodingflag) printf("\n");
	offset = findtable(font, start, "cmap");
	nlen = tableLength;
	if (offset == 0) return 0;
	s = font + offset;

	if (verboseflag)
		printf("%s table at %d\n", "cmap", offset);
	s = font + offset;
	cmapstart = s-font;
	ncmaps = uintfour(s);
	s += 4;
	if (verboseflag)
		printf("%u cmaps at %d\n", ncmaps, cmapstart);
	for (k = 0; k < ncmaps; k++) {
		platform = uinttwo(s);
		s += 2;
		specific = uinttwo(s);
		s += 2;
		cmapoffset = uintfour(s);
		s += 4;
		if (fixbadflag) {
			if ((platform == 3 && specific == 0))
				printf ("Font %s is already marked as non-text\n", infile);
			else if ((platform == 3 && specific == 1)) {
				printf ("Marking font %s as non-text\n",
					   infile);
				*(s-6+1) = 0;	/* change from 3.1 to 3.0 */
				changed++;
			}
		}
		if (platform == 1 && specific == 0) type =      "Macintosh";
		else if (platform == 3 && specific == 1) type = "Text     ";
		else if (platform == 3 && specific == 0) type = "Non-text ";
		if (showcmapsflag || showencodingflag) {
			printf("\n");
			printf("CMAP Type %d.%d (%s) ", platform, specific, type);
			if (showcmapsflag) printf("at %d", cmapoffset);
			printf("\n");
			if (verboseflag)
				printf("At byte %d\n", cmapstart+cmapoffset);
		}
/*		if (platform == 3 && specific == 1) { */
		{
			t = font + cmapstart + cmapoffset;
			format = uinttwo(t);
			t += 2;
			cmaplength = uinttwo(t);
			t += 2;
			formatversion = uinttwo(t);
			t += 2;
			if (showcmapsflag || showencodingflag)
				printf("Format %d Version %d CmapLength %d\n",
				   format, formatversion, cmaplength);
		}
		if (showcmapsflag) printf("At byte %d\n", t-font);
		if (verboseflag) printf("\n");
		if (format == 0) {
			if (cmaplength-6 != 256)
				printf("Unexpected cmaplength %d (not 256)\n", cmaplength);
/*			we expect cmaplength-6 = 256 */
			for (i = 0; i < cmaplength-6; i++) {
				indx = (unsigned char) *t++;
				if (showencodingflag) {
					if (strcmp(glyphnames[indx], ".notdef") != 0 ||
					   shownotdef)
						printf("%5d\t%s\n",  i, glyphnames[indx]);
				}
				else if (showcmapsflag) 
					printf("%5d %5d %s\n",  i, indx, glyphnames[indx]);
			}
		}
		if (format == 4) {
			segCount = uinttwo(t) / 2;
			t += 2;
			segSearch = uinttwo(t);
			t += 2;
			entrySelect = uinttwo(t);
			t += 2;
			if (showcmapsflag) 
				printf("segCount %d segSearch %d entrySelect %d\n",
				   segCount, segSearch, entrySelect);
			t +=2;				/* ??? */
/*			if (segCount > MAXSEG) {
				printf("Too many segments in cmap %d\n", segCount);
				exit(1);
			} */
			allocarrays(segCount);
			for (i = 0; i < segCount; i++) {
				endCount[i] = uinttwo(t);
/*				printf("end %04X ", endCount[i]); */
				if (fixbadflag) {
					if (endCount[i] == 0x2219) {
						*(t+1) = 0xFF;
						changed++;
					}
					if (endCount[i] != 0xFFFF && (*t != 0xF0)) {
						*t = 0xF0;
						changed++;
					}
				}
				t += 2;
			}
			t += 2;			/* step over reserved padding */
			for (i = 0; i < segCount; i++) {
				startCount[i] = uinttwo(t);
/*				printf("start %04X ", startCount[i]); */
				if (fixbadflag) {
					if (startCount[i] == 0x2219) {
						*(t+1) = 0xFF;
						changed++;
					}
					if (startCount[i] != 0xFFFF && (*t != 0xF0)) {
						*t = 0xF0;
						changed++;
					}
				}
				t += 2;
			}
			for (i = 0; i < segCount; i++) {
				temp = uinttwo(t);
				t += 2;
				if (temp < 32768) idDelta[i] = temp;
				else idDelta[i] = temp - 65536;
			}
			for (i = 0; i < segCount; i++) {
				idRangeOffset[i] = uinttwo(t);
				t += 2;
			}
			glyphIDarray = t - font;		/* where glyph ID array starts */
			if (showcmapsflag) {
				printf("glyphIDarray at %d\n", (t - font));
				printf("\n");
			}
			for (i = 0; i < segCount; i++) {
				if (showcmapsflag) 
				printf("%2d start %04X end %04X idDelta %5d idRangeOffset %5d ",
				 i+1, startCount[i], endCount[i], idDelta[i], idRangeOffset[i]);
				if (idRangeOffset[i] == 0) {
/*					glyphIndex = (unsigned short) idDelta[i]; */
					glyphIndex = 0;	/* N/A */
				}
				else {
					glyphIndex = (unsigned short)
								 (idRangeOffset[i] / 2 + i - segCount);
				}
				if (showcmapsflag) 
					printf ("glyphIndex %5d\n", glyphIndex);
			}
			if (showcmapsflag) printf("\n");
			for (i = 0; i < segCount; i++) {
				if (idRangeOffset[i] == 0) {
					for (chr = startCount[i]; chr <= endCount[i]; chr++) {
						glyf = (unsigned short) (chr +idDelta[i]);
						if (showencodingflag) {
							if (strcmp(glyphnames[glyf], ".notdef") != 0 ||
							   shownotdef)
							printf("%04X\t%5d\t%s\n",
								   chr, chr, glyphnames[glyf]);
						}
						else if (showcmapsflag) 
							printf("chr %04X (%5d)         glyf %4d %s\n",
								   chr, chr, glyf, glyphnames[glyf]);
						fflush(stdout);
					}
						
				}
				else {
					glyphIndex = (unsigned short)
								 (idRangeOffset[i] / 2 + i - segCount);
					for (chr = startCount[i]; chr <= endCount[i]; chr++) {
						j = glyphIndex+chr-startCount[i];
						glyf = uinttwo(font + glyphIDarray + 2 * j);
						if (showencodingflag) {
							if (strcmp(glyphnames[glyf], ".notdef") != 0)
							printf("%04X\t%5d\t%s\n", chr, chr, glyphnames[glyf]);
						}
						else if (showcmapsflag) 
						printf("chr %04X (%5d) inx %3d glyf %4d %s\n",
							   chr, chr, j, glyf, glyphnames[glyf]);
							   
					}
				}
			}
		}
	}
	return 0;
}

unsigned long buildextrastable(unsigned char *font, int start, int end){
	unsigned int k;
	unsigned int n;
	unsigned char *s;
	char *new;

	if (verboseflag) {
		printf("\n");
		printf("Buildextrastable from %ld to %ld\n", start, end);
	}
	k = 0;
	s = font + start;
	for (;;) {
		n = *s++;
		if (n == 0) break;
		if ( (s-font) > end) break;
		s += n;
		k++;
	}
	nextras = k;
	if (verboseflag)
		printf("There are %d extra glyph names\n", nextras);
	extraglyphs = (char **) malloc (nextras * sizeof(char *));
	k = 0;
	s = font + start;
	for (;;) {
		n = *s++;
		if (n == 0) break;
		if ((s-font) > end) break;
		new = malloc(n+1);
		strncpy(new, (char *) s, n);
		*(new+n) = '\0';
		extraglyphs[k] = new;
		if (verboseflag)
			printf("%3d\t%s\n", k, extraglyphs[k]);
		s += n;
		k++;
	}
	return k;
}

void freeextraglyphs (void) {
	unsigned int k;
	if (extraglyphs == NULL) return;
	for (k = 0; k < nextras; k++) {
		if (extraglyphs[k] == NULL) continue;
		free(extraglyphs[k]);
		extraglyphs[k] = NULL;				
	}
}

void freeglyphnames (void) {
	if (glyphnames == NULL) return;
	free(glyphnames);
	glyphnames = NULL;
}

void analyzepost(unsigned char *font, int start) {
	unsigned long nlen;
	unsigned long version, italicAngle;
	unsigned long underlinePosition, underlineThickness;
	int isFixedPitch, pad, bytesper;
	unsigned long minMemType42, maxMemType42, minMemType1, maxMemType1;
	unsigned int  numberGlyphs;
	unsigned int offset;
	unsigned int k, pnindex;
	unsigned char *s;
	char *glyphname;
	unsigned int nextra;
	
	if (verboseflag) printf("\n");
	offset = findtable(font, start, "post");
	nlen = tableLength;
	if (offset == 0) return;
	s = font + offset;

	if (showpostflag)
		printf("%s table at %d\n", "post", offset);
	version = uintfour(s);
	s += 4;
	if (showpostflag) 
		printf("POST version %d.%d\n",
		   (version >> 16), version & 65535);
/*	version 2.0 has uint16, while 2.5 has int8 indeces */
	if ((version >> 16) == 2 && (version & 65535) == 5)
		bytesper = 1;	/* Mac style cmap */
	else bytesper = 2;

	italicAngle = uintfour(s);	
	s += 4;
	if (showpostflag) 
	printf("italicAngle %d.%d\n", (italicAngle >> 16), italicAngle & 65535);
	underlinePosition = uintfour(s);
	s += 4;
	if (showpostflag) 
	printf("underlinePosition %d.%d\n", (underlinePosition >> 16), underlinePosition & 65535);
	underlineThickness = uintfour(s);
	s += 4;
	if (showpostflag) 
	printf("underlineThickness %d.%d\n", (underlineThickness >> 16), underlineThickness & 65535);
	isFixedPitch = uinttwo(s);
	s += 2;
	if (showpostflag) 
	printf("isFixedPitch %d\n", isFixedPitch);
	pad = uinttwo(s);
	s += 2;
	s -= 4;		/* ??? */
	minMemType42 = uintfour(s);
	s += 4;
	maxMemType42 = uintfour(s);
	s += 4;
	minMemType1  = uintfour(s);
	s += 4;
	maxMemType1  = uintfour(s);
	s += 4;
	numberGlyphs = uinttwo(s);
	s += 2;
	if (showpostflag) 
	printf("%d glyphs\n", numberGlyphs);
/*	nextra = buildextrastable(font, nlen, s + numberGlyphs * 2 - font); */
	nextra = buildextrastable(font,
					  (s - font) + numberGlyphs * bytesper, offset + nlen);
	fflush(stdout);
	if (showpostflag) printf("\n");
	glyphnames = (char **) malloc (numberGlyphs * sizeof(char *));
	for (k = 0; k < numberGlyphs; k++) {
		if (bytesper == 2) {
			pnindex = uinttwo(s);
			s += 2;
		}
		else pnindex = (unsigned int) *s++;
		if (pnindex < (256+2)) glyphname = MacGlyphs[pnindex];
		else if (pnindex - (256+2) < nextras)
			glyphname = extraglyphs[pnindex - (256+2)];
		else glyphname = "";
		if (showpostflag) 
			printf("%3d\t%5d\t%s\n", k, pnindex, glyphname);
		glyphnames[k] = glyphname;
		fflush(stdout);
	}
}

int analyzename(unsigned char *font, int start, int adjustflag) {
	unsigned long offset, nlen;
	unsigned int k, i, fmtselector, numnames, strngoffset;
	unsigned int platformID, specificID, languageID, nameID;
	unsigned int strlength, stroffset;
	unsigned char *s, *t;
	int c;

	if (verboseflag) printf("\n");
	offset = findtable(font, start, "name");
	nlen = tableLength;
	if (offset == 0) return -1;
	s = font + offset;
	fmtselector = uinttwo(s);
	s += 2;
	numnames = uinttwo(s);
	s += 2;
	strngoffset = uinttwo(s);
	s += 2;
	for (k = 0; k < numnames; k++) {
		platformID = uinttwo(s);
		s += 2;
		specificID = uinttwo(s);
		s += 2;
		languageID = uinttwo(s);
		s += 2;
		nameID = uinttwo(s);
		s += 2;
		strlength = uinttwo(s);
		s += 2;
		stroffset = uinttwo(s);
		s += 2;
/*	ID = 1.0 => Macintosh */
/*	ID = 3.1 => Windows text font */
/*	ID = 3.0 => Windows symbol font */
		if (platformID == 1 && specificID == 0) {
			if (verboseflag)
			printf("%3d Macintosh name %3d %3d %4d %3d\n",
				   k, languageID, nameID, stroffset, strlength);
		}
		else if (platformID == 3 && specificID == 0) {
			if (verboseflag)
			printf("%3d Windows SYMBOL name %3d %3d %4d %3d\n",
				   k, languageID, nameID, stroffset, strlength);
		}
		else if (platformID == 3 && specificID == 1) {
			if (verboseflag)
			printf("%3d Windows TEXT name %3d %3d %4d %3d\n",
				   k, languageID, nameID, stroffset, strlength);
			if (adjustflag) {
				*(s-10+1) = 0;	/* 3.1 => 3.0 */
				if (verboseflag)
				printf("Changing from 3.1 => 3.0\n");
				changed++;
			}				
			if (nameID == 5) {	/* version number */
				if (verboseflag) printf("Version: ");
				t = font + offset + strngoffset + stroffset;
				for (i = 0; i < strlength; i++) {
					c = *t++;
					if (c == 0) continue;
					if (verboseflag) putc(c, stdout);
				}
				if (verboseflag) putc('\n', stdout);
				if (adjustflag) {
					*(t-1) = (unsigned char) (*(t-1) + 1);	/* increment version */
					changed++;
				}
			}
		}
		else {
			if (verboseflag)
			printf("%3d Unknown name %3d %3d %4d %3d\n",
				   k, languageID, nameID, stroffset, strlength);
		}
	}

	return 0;		
}


int analyzehead(unsigned char *font, int start, int adjustflag) {
	unsigned long offset, nlen, version, revision, chk, chksumadjust, magic;
	unsigned char *s;

	if (verboseflag) printf("\n");
	offset = findtable(font, start, "head");
	nlen = tableLength;
	if (offset == 0) return -1;
	s = font + offset;
	version = uintfour(s);
	s += 4;
/*	if (adjustflag) {
		*(s-1) = (unsigned char) (*(s-1) + 1);
		changed++;
	} */
	revision = uintfour(s);
	s += 4;
	chksumadjust = uintfour(s);
	s += 4;
	magic = uintfour(s);
	s += 4;
	if (magic != 0x5F0F3CF5) {
		printf("Magic number is %08X, not %08X\n",
			   magic, 0x5F0F3CF5);
		return -1;
	}
	if (adjustflag) {
		*(s-8) = *(s-7) = *(s-6) = *(s-5) = 0;	/* reset to zero */
		chk = checktable(font, 0, nfilelen);
		if (chk != 0xB1B0AFBA) {
			if (verboseflag)
			printf("Need to change checksum adjust in head to %08X\n",
				  chksumadjust);
			changed++;
		}
		else if (verboseflag)
			printf("Checksum for font file OK\n");
		chksumadjust = 0xB1B0AFBA - chk;
		*(s-5) = (unsigned char) (chksumadjust & 255);
		chksumadjust = chksumadjust >> 8;
		*(s-6) = (unsigned char) (chksumadjust & 255);
		chksumadjust = chksumadjust >> 8;
		*(s-7) = (unsigned char) (chksumadjust & 255);
		chksumadjust = chksumadjust >> 8;
		*(s-8) = (unsigned char) (chksumadjust & 255);
	}
	return 0;		
}


int analyzeOS2(unsigned char *font, int start, int adjustflag) {
	unsigned long offset, nlen;
	unsigned int Version, xAvgCharWidth, usWeightClass, usWidthClass, fsType;
	unsigned int ySubscriptXSize, ySubscriptYSize, ySubscriptXOffset;
	unsigned int ySubscriptYOffset, ySuperScriptXSize, ySuperScriptYSize;
	unsigned int ySuperScriptXOffset, ySuperScriptYOffset, yStrikeOutSize;
	unsigned int yStrikeOutPosition, sFamilyClass;
	unsigned int achVenID, usSelection, usFirstChar, usLastChar;
	unsigned char *s, *t;
	int c, k, flag;

	if (verboseflag) printf("\n");
	offset = findtable(font, start, "OS/2");
	nlen = tableLength;
	if (offset == 0) return -1;
	s = font + offset;
/* OS/2 table */
	Version = uinttwo(s);
	s += 2;
	xAvgCharWidth = uinttwo(s); /* short */
	s += 2;
	usWeightClass = uinttwo(s);
	s += 2;
	usWidthClass = uinttwo(s);
	s += 2;
	fsType = uinttwo(s);		/* short */
	s += 2;
	ySubscriptXSize = uinttwo(s);
	s += 2;
	ySubscriptYSize = uinttwo(s);
	s += 2;
	ySubscriptXOffset = uinttwo(s);
	s += 2;
	ySubscriptYOffset = uinttwo(s);
	s += 2;
	ySuperScriptXSize = uinttwo(s);
	s += 2;
	ySuperScriptYSize = uinttwo(s);
	s += 2;
	ySuperScriptXOffset = uinttwo(s);
	s += 2;
	ySuperScriptYOffset = uinttwo(s);
	s += 2;
	yStrikeOutSize = uinttwo(s);
	s += 2;
	yStrikeOutPosition = uinttwo(s);
	s += 2;
	sFamilyClass = uinttwo(s);
	s += 2;
/*	Panose = inteight(s); */
	s += 10;		/* ignore this Panose nonsense */
/*	ulCharRange = uinttwo(s); */
	s += 4 * 4;
	achVenID = uintfour(s);		/* char[4] */
	s += 4;
	t = s-4;
	if (adjustflag) {
		if (verboseflag) printf("VendID ");
		flag = 0;
		for (k = 0; k < 4; k++) {
			c = *t++;
			if (verboseflag) putc(c, stdout);
			if (c != 0) flag++;
		}
		if (verboseflag) printf("\n");
		if (flag == 0) {
			strncpy((char *) (s-4), signature, 4); /* change vendor ID */
			printf("Replacing empty signature\n");
		}
		changed++;
	}
	usSelection = uinttwo(s);
	s += 2;
	usFirstChar = uinttwo(s);
	s += 2;
	if (adjustflag) {
		if (*(s-2) != 0xF0) {
			*(s-2) = 0xF0;
			*(s-1) = 0x00;		/* or 0x20 */
			changed++;
			if (verboseflag)
			printf("Changing startchar %d (%04X)\n",
				   usFirstChar, usFirstChar);
		}
	}
	usLastChar = uinttwo(s);
	s += 2;
	if (adjustflag) {
		if (*(s-2) != 0xF0) {
			*(s-2) = 0xF0;
			*(s-1) = 0xFF;		/* or ... */
			changed++;
			if (verboseflag)
			printf("Changing lastchar %d (%04X)\n",
				   usLastChar, usLastChar);
		}
	}
/* and more */
}

int analyzefont(unsigned char *font, size_t nlen) {
	unsigned char *s=font;
	uint32 tableOffset;

	changed=0;
	version = uintfour(s);
	s += 4;
	numOffsets = uinttwo(s);
	s += 2;
	searchRange = uinttwo(s);
	s += 2;
	entrySelector = uinttwo(s);
	s += 2;
	rangeShift = uinttwo(s);
	s += 2;
	tableOffset = s-font;
	if (verboseflag) {
		printf("\n");
		printf(" sfnt version:\t%d.%d\n", version / 65536, version & 65535);
		printf(" numTables\t%d\n", numOffsets);
		printf(" searchRange\t%d\n", searchRange);
		printf(" entrySelector\t%d\n", entrySelector);
		printf(" rangeShift\t%d\n", rangeShift);
	}
	if (traceflag) printf ("At byte %d\n", tableOffset);
	analyzechecksums(font, tableOffset, 0);
	analyzehead(font, tableOffset, 0);
	analyzepost(font, tableOffset);
	analyzecmaps(font, tableOffset);
	analyzename(font, tableOffset, fixbadflag);
	analyzeOS2(font, tableOffset, fixbadflag);
	fflush(stdout);
	if (fixbadflag) {
		if (changed) {
			if (verboseflag)
				printf("Fixing up checksums of tables\n");
			analyzechecksums(font, tableOffset, fixbadflag);
			analyzehead(font, tableOffset, fixbadflag);
		}
		else printf("Nothing changed, no need to fix checksums\n");
	}
	return 0;
}

int checkfont(FILE *input, size_t nlen, char *filename) {
	size_t nret;
	unsigned long chk;

	if (verboseflag) printf("Reading file of %ld bytes\n", nlen);
	fflush(stdout);
	nret = fread(font, 1, nlen, input);
	if (nret != nlen) {
		printf("Unable to read file %ld <> %ld\n", nret, nlen);
		return -1;
	}
	if (verboseflag) printf("Analyzing font %s\n", filename);
	chk = checktable(font, 0, nlen);
	if (chk != 0xB1B0AFBA) {
		printf("Overall font checksum %08X (not %08X\n",
			   chk, 0xB1B0AFBA);
			if (strictflag) return -1;
	}
	else if (verboseflag) printf("Overall checksum OK\n");
	analyzefont(font, nlen);
	return 0;
}

void extension(char *fname, char *str) { /* supply extension if none */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

void stripfilename(char *path) {	/* truncate full filename */
	char *s;
	if ((s = strrchr(path, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(path, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(path, ':')) != NULL) *(s+1) = '\0';
	else *path = '\0';
}

char *extractfilename(char *path) {	/* return pointer to filename part only */
	char *s;
	if ((s = strrchr(path, '\\')) != NULL) return s+1;
	else if ((s = strrchr(path, '/')) != NULL) return s+1;
	else if ((s = strrchr(path, ':')) != NULL) return s+1;
	else return path;
}

int commandline (int argc, char *argv[]) {
	int m=1;
	while (m < argc && argv[m][0] == '-') {
		if (strcmp(argv[m], "-v") == 0)
			verboseflag = ! verboseflag;
		else if (strcmp(argv[m], "-t") == 0)
			traceflag = ! traceflag;
		else if (strcmp(argv[m], "-s") == 0)
			showtablesflag = ! showtablesflag;
		else if (strcmp(argv[m], "-c") == 0)
			showcmapsflag = ! showcmapsflag;
		else if (strcmp(argv[m], "-p") == 0)
			showpostflag = ! showpostflag;
		else if (strcmp(argv[m], "-f") == 0)
			fixcheckflag = ! fixcheckflag;
		else if (strcmp(argv[m], "-b") == 0)
			fixbadflag = ! fixbadflag;
		else if (strcmp(argv[m], "-w") == 0)
			writefileflag = ! writefileflag;
		else if (strcmp(argv[m], "-S") == 0)
			strictflag = ! strictflag;
		else if (strcmp(argv[m], "-n") == 0)
			shownotdef = ! shownotdef;
		else if (strcmp(argv[m], "-o") == 0)
			overwriteflag = ! overwriteflag;
		else if (strcmp(argv[m], "-e") == 0) {
			showencodingflag = 1;
			writefileflag = fixbadflag = fixcheckflag = 0;
		}
		else if (strcmp(argv[m], "-V") == 0) {
			verboseflag = showtablesflag = showcmapsflag = showpostflag = 1;
		}
		m++;
	}
	return m;
}

void showusage(int argc, char *argv[]) {
	printf("%s will convert TrueType fonts that are incorrectly marked as\n",
		   argv[0]);
	printf("text fonts to non-text (symbol, pi, decorative) fonts\n");
	printf("Do not apply to Type 1 fonts\n");
	printf("Do not apply to real TEXT fonts in TrueType format\n");
	printf("\t-v be verbose\n");
	printf("\t-V be more verbose - show all details\n");
	printf("\t-e show cmaps as encoding vectors\n");
	printf("\t-w do not write output file\n");
}

int main (int argc, char *argv[]) {
	int c, d, m, firstarg, count, flag;
	FILE *input, *output;
	unsigned long nret;

	printf("FixFont 1.0 %s\n", copyright);
	if (argc < 2) {
		showusage (argc, argv);
		exit(1);
	}
	if (traceflag) printf("%d arguments\n", argc-1);
	fflush(stdout);
	firstarg = commandline (argc, argv);
	if (argc < firstarg+1) {
		showusage (argc, argv);
		exit(1);
	}
	count = 0;
	for (m = firstarg; m < argc; m++) {
		strcpy(infile, argv[m]);
		extension(infile, "ttf");
		if ((input = fopen(infile, "rb")) == NULL) {
			perror(infile);
/*			exit(1); */
			continue;
		}
		c = getc(input);			/* perform simple sanity check first */
		d = getc(input);
		if (c != 0 || d != 1) {
			printf("%s does not appear to be a TT font file\n",
				   infile);
			fclose(input);
			continue;
		}
		if (verboseflag) printf("Reading %s\n", infile);
		fflush(stdout);
		fseek(input, 0, SEEK_END);
		nfilelen = ftell(input);
		fseek(input, 0, SEEK_SET);
		if ((font = (unsigned char *) malloc(nfilelen)) == NULL) {
			printf("Unable to allocate %ld bytes for file\n",
				  nfilelen);
			exit(1);
		}
		flag = checkfont(input, nfilelen, infile);
		fclose(input);
		count++;
		if (verboseflag) printf("\n");
		freeglyphnames();
		freeextraglyphs();
		freearrays();
		if (writefileflag && flag == 0) {
			strcpy(outfile, extractfilename(infile));
			if ((output = fopen(outfile, "rb")) != NULL) {
				fclose(output);
				if (overwriteflag) {
					printf("%s already exists ", outfile);
					strcpy(bakfile, outfile);
					forceexten(bakfile, "bak");
					printf("- renaming %s to %s\n", outfile, bakfile);
					remove (bakfile);
					rename(outfile, bakfile);
				}
				else forceexten(outfile, "new");
				printf("Writing file %s\n", outfile);
			}
			else {
				if (verboseflag)
					printf("Writing file %s\n", outfile);
			}
			if ((output = fopen(outfile, "wb")) == NULL) {
				perror(outfile);
				exit(1);
			}
			nret = fwrite(font, 1, nfilelen, output);
			if (traceflag)
				printf("nfilelen %d nret %d font %d\n", nfilelen, nret, font);
			if (nret != nfilelen) {
				printf("Unable to write file %ld <> %ld\n", nret, nfilelen);
				perror("FOO");
/*				exit(1); */
			}
			fclose(output);
		}
		if (font != NULL) free(font);
		font = NULL;
	}
	if (verboseflag) printf("Processed %d file%s\n",
							count, (count == 1) ? "" : "s");
	return 0;
}
