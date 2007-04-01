/* Copyright 1990, 1991, 1992 Y&Y, Inc.
   Copyright 2007 TeX Users Group

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

/************************************************************************
 * Program to convert open code Type 3 fonts into canonical style coding 
 * will work for `rustic' format, as well as Projective Solutions formats 
 * Multiple files can be specified on the command line 
 *
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <time.h>
#include <math.h>

#pragma warning(disable:4032)	// different type when promoted
#include <conio.h>
#pragma warning(default:4032)	// different type when promoted

/* following needed so can copy date and time from input file */

#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>

/* #define FNAMELEN 80 */
#define FONTNAMELEN 80
#define MAXLINE 256
#define MAXCHRS 256			/* was: 128 */
#define COLUMNS 78
#define INFINITY 8191		/* 4095 */

/* #define DEBUGFLUSH */		/* flush output buffers */

#define UNIQUEIDSTART 5000761
/* Adobe UniqueID coordinator:  Terry O'Donnell 415-962-3836 */

FILE *errout=stdout;

long uniqueid;

static char inline[MAXLINE];
static char outline[MAXLINE];

/* static int widths[MAXCHRS]; */
static double fwidths[MAXCHRS];	/* 95/Dec/10 */

static long charstart[MAXCHRS]; /* starting place in file */

int fontchrs=MAXCHRS;		/* actual size needed for encoding vector */

double bezierq = 0.5541;		/* magic approximation for circle quadrant */

int texfont = 0;

char *ext="hex";	/* extension for output file */
char *ver= "";		/* version, if specified */

static char fontname[FONTNAMELEN];
static char commonfont[FONTNAMELEN];

int ndict;

int verboseflag=0;		/* lots of output if non-zero */
int traceflag=0;		/* lots of output if non-zero */
int debugflag=0;
int quietflag=0;
int bellflag=0;			/* make noise ... */
int showoffsets=0;		/* show offsets and scaling */
int complainflag=1;		/* complain if character not found */
int complainopen=0;		/* complain if subpath coordinates don't close */
int rusticflag=0;		/* non-zero means `rustic' format */
						/* now hard-wired to be zero */
int correctscale=0;		/* correct for BSR scale adjustement */
int adobescaling=1;		/* 0 => input in 4000 dpi units */
						/* 1 => input in Adobe standard units */
						/* 2 => input in Adobe units scaled by scalef */
int orderflag=1;		/* order character definitions in output */
int uppercaseflag=0;	/* make font name upper case */
int adjustknots=1;		/* adjust knots to avoid coincidence after round */ 
int wantuniqueid=0;		/* insert UniqueID if requested */
int remapflag=0;		/* insert remapping of character codes at end */
int wantdownload=0;		/* non-zero to insert downloading code */
int copydate=1;			/* copy input file date to output */
int checknear=0;		/* check side slip */

int closelastpath=1;	/* close last path if left open by mistake */
						/* PS interpreters work without this, but neater */
int allowmovetoin=1;	/* close path when moveto inside path */
						/* may be better than converting to lineto */

int pctexbug=0;			/* on for PC-TeX bug */

int fixonly=0;			/* expect only integers in input */

int fatal=0;			/* fatal error flag */

/* int putzerolast=0; *//* to deal with bug in ATM - put zeroth entry last */

double ptsize;			/* design size of font */
double scalef;			/* scale factor with repect to 4000 dpi ? */
double dpi=4000.0;		/* assumed metafont dpi used */
double skew=0.0;		/* global skew factor for oblique font */ /* 1992/Aug/25 */
						/* applied at output stage */

int scaleup;			/* non-zero if output scale larger than input */

int closed;				/* non-zero when path is closed */

int xll, yll, xur, yur;
int xmin, ymin, xmax, ymax;

char *path = "";	/* path for output files */

int metaflag=0;		/* next argument is meta font dpi used */
int xscaleflag=0;	/* next argument is global scale in x to be used */
int yscaleflag=0;	/* next argument is global scale in y to be used */
int skewflag=0;		/* next argument is amount off skew to impose */
int extenflag=0;	/* next argument is file extension for output */
int versionflag=0;	/* next argument is version of font */
int pathflag=0;		/* next argument is path for output files */
int charsflag=0;	/* next argument is size of encoding vector */

int nonamechar=0;	/* how many characters without names */

int chrs;			/* current character looking for */
int fchrs;			/* character number read from file */
/* int widthf; */
double fwidthf;
int clm;			/* current column count */
int numhex;			/* number of hex digits put out in string */
int nchrs;
long fstart;		/* beginning of character codes */
int xold, yold;		/* last knot (in original coordinates) */
int xlast, ylast;	/* last knot of subpath (in output coordinates) */
int xstart, ystart;	/* start of subpath (in output coordinates) */

int xprev, yprev;	/* last point coordinates as in source 1995/Nov/29 */

/* int xoff, yoff; */	/* offset for current path */

double xoff, yoff;		/* offset for current path */

double xscaleinit=1.0;		/* global x scale - 1993/Jan/1 */
double yscaleinit=1.0;		/* global y scale - 1993/Jan/1 */
double slantinit=0.0;		/* global slant info */

double slantcomp=0.0;		/* slant compensation in anisotropic scaling */

double slant=0.0;			/* slant for particular character 1996/Sep/2 */

double xscale=1.0;			/* scale for current path */
double yscale=1.0;			/* scale for current path */

int transformflag=0;		/* needs coordinate transform */

int xcenter=0, ycenter=0;	/* center for scaling operation */

#define ABS(x) (((x) < 0) ? (-x) : (x))
/* #define MAX(x, y) (((x) < (y)) ? (y) : (x)) */
/* #define MIN(x, y) (((x) < (y)) ? (x) : (y)) */

/* int firstmove;  */

static char header1[]="\
%!PS-Adobe-2.0\n";  

static char header15[]="\
% The following code loads to disk.\n\
% To load to permanent VM instead, remove the following 7 lines\n\
% and the '%' character from the 'exitserver' line.\n";

/* % /fd (fonts/cmr10) (w) file def\n\ */

static char header2[]="\
% /buff 128 string def\n\
% {currentfile buff readstring\n\
%\t{fd exch writestring}\n\
%\t{dup length 0 gt {fd exch writestring}{pop} ifelse fd closefile exit}\n\
%\tifelse\n\
%\t} bind loop\n\
\n\
% serverdict begin 0 exitserver % for VM installation\n\n";
 
static char front3[] = "\
/m {moveto} bind def\n\
/c {curveto} bind def\n\
/l {lineto} bind def\n\
/h {closepath} bind def\n\
/unhex{dup length 1 bitshift string 0 3 -1 roll\n\
\t{3 copy -4 bitshift (0123456789 -chlm)  exch get put exch 1 add exch\n\
\t15 and  (0123456789 -chlm)  exch get 3 copy put pop 1 add} bind\n\
\tforall pop} bind def\n\
/BuildChar{exch begin CharDefs exch get dup null eq {pop}\n\
\t{save exch unhex cvx dup exec 0 flattenpath pathbbox\n\
\tsetcachedevice newpath exec fill pop restore}\n\
\tifelse end} bind def\n\
end\n\
definefont\n\
\n\
/CharDefs get\n\n";

/*
/hexcode (0123456789 -chlm) def\n\
/unhex{hexcode exch dup length 2 mul string 0 3 -1 roll\n\
\t{3 copy 16 idiv 6 index exch get put exch 1 add exch\n\
\t15 and 3 index exch get 3 copy put pop 1 add} bind\n\
\tforall pop exch pop} bind def\n\ */

static char remapping[] = "\n\
0 1 9 {2 copy 2 copy get exch 161 add exch put pop} for\n\
10 1 32 {2 copy 2 copy get exch 163 add exch put pop} for\n\
dup dup 32 get 128 exch put\n\
dup dup 127 get 196 exch put\n";

/* The seventy-five ComputerModern font names */

/* unsigned char *cmfonts[] = { */
char *cmfonts[] = {
	"b10", "bsy10", "bx5", "bx6", "bx7", "bx8", "bx9", "bx10", "bx12",
	"bxsl10", "bxti10", "csc10", "dunh10", "ex10", "ff10", "fi10",
	"fib8", "inch", "itt10", "mi5", "mi6", "mi7", "mi8", "mi9",
	"mi10", "mi12", "mib10", "r5", "r6", "r7", "r8", "r9", "r10",
	"r12", "r17", "sl8", "sl9", "sl10", "sl12", "sltt10", "ss8",
	"ss9", "ss10", "ss12", "ss17", "ssbx10", "ssdc10", "ssi8",
	"ssi9", "ssi10", "ssi12", "ssi17", "ssq8", "ssqi8", "sy5", "sy6",
	"sy7", "sy8", "sy9", "sy10", "tcsc10", "tex8", "tex9", "tex10",
	"ti7", "ti8", "ti9", "ti10", "ti12", "tt8", "tt9", "tt10",
	"tt12", "u10", "vtt10"
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

struct _stat statbuf;	/* struct stat statbuf; */

struct _utimbuf timebuf;	/* struct utimbuf timebuf; */

int getinfo(char *filename, int verboseflag) {
	char *s;
	int result;

	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(errout, "Unable to obtain info on %s\n", filename);
		return -1;
	}
	s = ctime(&statbuf.st_atime);		/* ltime */
	lcivilize(s);								
	if (verboseflag != 0) printf("%s last modified: %s", filename, s);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void giveup(int code) { 	/* graceful exit with meaningful error message */
/*	fprintf(stderr, " while %s", task);  */
/*	if (chrs >= 0) fprintf(stderr, " for character %d ", chrs); 
	else fprintf(stderr, " ");  */
/* 	fclose(fp_out);	remove(fn_out); */
	exit(code);
}

void nputc(char c, FILE *fp_out) {
	if (c == '\n') {
		putc(c, fp_out); clm = 0;
	}
	else {
		if (clm >= COLUMNS) {
			putc('\n', fp_out);	clm = 0;
		}
		putc(c, fp_out); clm++;	numhex++;
	}
}

int round (double x) {
	if (x < 0) return (- round(-x));
	else return (int) (x + 0.5);
}

void writetext(char *s, FILE *fp_out) {
	while(*s != '\0') nputc(*s++, fp_out);
}

void writenum(int num, FILE *fp_out) {
	if (num < 0) {
		nputc('B', fp_out);
		num = - num;
	}
	sprintf(outline, "%d", num);
	writetext(outline, fp_out);
}

void writenuma(int num, FILE *fp_out) {
	writenum(num, fp_out); nputc('A', fp_out);
}

void writecoord(int x, int y, FILE *fp_out) {
	int xs;
	if (skew == 0.0) {
		writenum(x, fp_out); nputc('A', fp_out);
		writenum(y, fp_out); nputc('A', fp_out);
	}
	else {	 /* 1992/Aug/25 */
		xs = x + round (y * skew);
		writenum(xs, fp_out); nputc('A', fp_out);
		writenum(y, fp_out); nputc('A', fp_out);
	}
}

int getline (FILE *input, char *line) { /* read a line up to newline */
	int c, k = 0;
	char *s = line;

	while ((c = getc(input)) != EOF && c != '\n' && c != '\r') {
		*s++ = (char) c; k++;
		if (k >= MAXLINE) {
			*s = '\0';
			fprintf(errout, "Line too long: %s ...", line);
			giveup(7);
		}
	}
	if (c == '\r' || c == '\n') {
		*s++ = '\n'; k++;
	}
	*s = '\0';
	if (c == EOF && k == 0) {
		if (rusticflag != 0) {
			fprintf(errout, "Unexpected EOF");
			giveup(4);
		}
	}
/*	else return k; */
	return k;
}

/* int round (double x) {
	if (x < 0.0) return -round(-x);
	else return (int) (x + 0.5);
} */

void readmetrics(FILE *fp_in) {
/*	int width; */
	double fwidth;
	int k;

	(void) getline(fp_in, inline);
/*	for (k=0; k < fontchrs; k++) widths[k] = 0; */
	for (k=0; k < fontchrs; k++) fwidths[k] = 0.0;
	while(strstr(inline, "end def") == NULL) {
		if (strstr(inline, "/.notdef") == NULL) {
/*			if (sscanf(inline, " /char%d %d def", &fchrs, &width) != 2) { */
			if (sscanf(inline, " /char%d %lg def", &fchrs, &fwidth) != 2) {
				fprintf(errout, "Not character width info: %s\n", inline);
				giveup(3);
			}
			if (correctscale != 0)	/* new */
/*				width = round(((double) width) * 72.27 / 72.0);  */
				fwidth = fwidth * 72.27 / 72.0; 
/*			if (fchrs >= 0 && fchrs < fontchrs) widths[fchrs] = width; */
			if (fchrs >= 0 && fchrs < fontchrs) fwidths[fchrs] = fwidth;
			else {
				fprintf(errout, "Character number %d out of range\n", fchrs);
				giveup(7);
			}
		}
		(void) getline(fp_in, inline);
	}
}

void skipbbox(FILE *fp_in) {
	(void) getline(fp_in, inline);
	while (strstr(inline, "end def") == NULL) {
		(void) getline(fp_in, inline);
	}
}

void skipencoding(FILE *fp_in) {
	(void) getline(fp_in, inline);
	while (strstr(inline, "] def") == NULL) {
		(void) getline(fp_in, inline);
	}
}

/* int scalenum(int x) { */
double fscalenum (double x) {
/*	if (adobescaling != 0)  return x; */
	if (adobescaling == 0) {
		if (correctscale == 0)
/*			return round((double) x * 72.27 * 1000.0 /  */
			return (x * 72.27 * 1000.0 / (dpi * ptsize * scalef));
/*		else return round((double) x * 72.0 * 1000.0 /  */
		else return (x * 72.0 * 1000.0 / (dpi * ptsize * scalef));
	}
	else if (adobescaling == 1)  return x;
/*	else if (adobescaling == 2)  return round((double)x / scalef); */
	else if (adobescaling == 2)  return (x / scalef);
	else {
		printf("Bad value for adobescaling %d\n", adobescaling);
		return x;
	}
}

int scalenum (int x) {
	return round(fscalenum((double) x));
}

/* int unscalenum(int x) { */
double funscalenum(double x) {
/*	if (adobescaling != 0)  return x;  */
	if (adobescaling == 0) {
		if (correctscale == 0)
/*			return round((double) x *  (dpi * ptsize * scalef) /  */
			return (x *  (dpi * ptsize * scalef) / (72.27 * 1000.0));
/*		else return round((double) x * (dpi * ptsize * scalef) /  */
		else return (x * (dpi * ptsize * scalef) / (72.0 * 1000.0));
	}
	else if (adobescaling == 1)  return x; 
/*	else if (adobescaling == 2)  return round((double)x * scalef);  */
	else if (adobescaling == 2)  return (x * scalef); 
	else {
		printf("Bad value for adobescaling %d\n", adobescaling);
		return x;
	}
}

int unscalenum (int x) {
	return round(funscalenum((double) x));
}

void lowercase(char *s, char *t) { /* convert to lower case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'A' && c <= 'Z') *s++ = (char) (c + 'a' - 'A');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

void uppercase(char *s, char *t) { /* convert to upper case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

/* long makeuniqueid(unsigned char *name) { */ /* make unique ID */
long makeuniqueid(char *name) { /* make unique ID */
	long res=-1;		/* default if can't be found */
	char fontnamel[FONTNAMELEN]; /* lower case version */
	int i, n;

	lowercase(fontnamel, name);		/* in case it is upper case */
	if (strncmp(fontnamel, "cm", 2) != 0)
		fprintf(stdout, "Not cm font %s (no UniqueID)\n", fontnamel);
	else {
		n = sizeof(cmfonts)/sizeof(cmfonts[0]);
/*		printf(" **** %d fonts in list ***\n", n); */
		for (i = 0; i < n; i++)	{
			if (strcmp(fontnamel + 2, cmfonts[i]) == 0) {
				res = UNIQUEIDSTART + i; break;
			}
		}
		if (res < 0)
			fprintf(stdout, "Unknown cm font %s\n", fontnamel);
		if (verboseflag != 0) {
			if (res > 0 ) printf("UniqueID is %ld\n", res);
			else printf("UniqueID unknown\n");
		}
	}
	return res;
}

void copyinhead(FILE *fp_out, char *name) {
/*	int ndict; */
	char fontnamec[FONTNAMELEN]; /* upper case version */

	if (uppercaseflag != 0) uppercase(fontnamec, name);
	else strcpy(fontnamec, name);
	if (verboseflag != 0) printf("Fontname used in output: %s\n", fontnamec); 
	fprintf(fp_out, "%s", header1);		/* copy in header */
	if (wantdownload != 0) {
		fprintf(fp_out, "%s", header15);	/* copy in header */
		fprintf(fp_out, "%% /fd (fonts/%s) (w) file def\n", fontnamec);
		fprintf(fp_out, "%s", header2);		/* copy in header */
	}
	else putc('\n', fp_out);
	ndict = 15;
	if (wantuniqueid != 0 || uniqueid != -1) ndict++;
	fprintf(fp_out, "/%s %d dict dup begin\n", fontnamec, ndict);
	if (traceflag != 0)  {
		printf("/%s %d dict dup begin\n", fontnamec, ndict); 
/*		(void) _getch(); */
	}
}

void showchartable(void) {	/* for debugging only */ /* not used */
	int chr;
	printf("START CHARTABLE\n");
	for (chr = 0; chr < MAXCHRS; chr++) {
		if (charstart[chr] > 0) printf("%d %ld\n", chr, charstart[chr]);
	}
	printf("END CHARTABLE\n");
}

void makechartable(FILE *fp_in) {
	int chr, k, c;
/*	int width; */
	double fwidth;
	long place;
	char *s; 

/*	printf("Making Char Table\n"); */
	for (chr = 0; chr < MAXCHRS; chr++) charstart[chr] = -1L;
	while (getline(fp_in, inline) > 0) {
		if (*inline == ']') {
			place = ftell(fp_in);
/* following added to allow way to end file prematurely using `] end' */
/* note: can't have % between `]' and `end' that in effect comments out */
			if (strstr(inline, "] end") != NULL) {
				if (verboseflag != 0) printf("WARNING: early EOF\n");
				break;
			}
/* following added to catch case where first line is glued on after ] */
/*			if (*(inline+1) >= ' ') fprintf(errout, "\nERROR: %s", inline); */
			s = inline + 1;
			while ((c = *s++) > 0) {
				if (c == '%') break;	/* ignore comments ... */
				if (c > ' ') {
					fprintf(errout, "\nERROR: %s", inline);
					fatal++;
					break;
				}
			}
			k = getline(fp_in, inline);
			if (k == 0) break;		/* EOF */
/*			while (*inline == '%' || *inline == '\n')  { */
			while (*inline == '%' || *inline == '\n' || *inline == ']')  {
				k = getline(fp_in, inline);
				if (k == 0) break;
			}
/*			if (k == 0) break;  */
			if (k <= 0) break; /* EOF */
/*			if (sscanf(inline, "%d %d", &chr, &width) < 2) { */
			if (sscanf(inline, "%d %lg", &chr, &fwidth) < 2) {
				fprintf(errout, 
					"Character code or width bad: %s (%d) ", inline, k); 
				if (getline(fp_in, inline) == 0) {
					fprintf(errout, "EOF!\n");
					break;
				}
				else {
					fprintf(errout, "Next line: %s", inline);
					giveup(11);
				}
			}
/*			check whether empty character 97/July/13 */
/*			happens when auto converting character with SEAC call */
/*			assumes no comment lines in between ... */
			c = getc(fp_in);
			ungetc(c, fp_in);
			if (c == ']') continue;
/*			if ((s = strstr(inline, "offset")) != NULL) {} */
			if (charstart[chr] != -1L) {
				fprintf(errout, "\nERROR: duplicate char %d: %s\n", chr, inline);
/*	using the last version of this character in that case */
				fatal++;
			}
			charstart[chr] = place;
		}
	}
/*	showchartable(); */	/* debugging */
	rewind(fp_in);
/*	printf("Made Char Table\n"); */
}

void checkforfontname(char *inline) {
	if (sscanf(inline, "%%%% FontName: %s", commonfont) == 1) {
		if (verboseflag != 0)
			printf("Font Name from file: %s\n", commonfont);
		strcpy(fontname, commonfont);
	}
	else if (sscanf(inline, "%%%% FontName %s", commonfont) == 1) {
		if (verboseflag != 0)
			printf("Font Name from file: %s\n", commonfont);
		strcpy(fontname, commonfont);
	}
}

void doheader(FILE *fp_in, FILE * fp_out) {
	int c, n, ndict, nevect;
	char *s;

	c = getc(fp_in); ungetc(c, fp_in);
/*	rusticflag = (c == '/') ? 1 : 0; */
	if (c == '/') rusticflag = 1;	else rusticflag = 0;
/*	printf("RUSTICFLAG %d\n", rusticflag); */
	if (orderflag != 0 && rusticflag == 0) makechartable(fp_in);

	if (rusticflag != 0) {
		(void) getline(fp_in, inline);
		if (strstr(inline, "dict") == NULL) {
			fprintf(errout, "First line is not font dict: %s\n", inline);
			giveup(2);
		}
/*		sscanf(inline, "/%s %d ", &fontname, &ndict); */
		sscanf(inline, "/%s %d ", fontname, &ndict);
		if (wantuniqueid != 0) 	uniqueid = makeuniqueid(fontname);
/*		if (uppercaseflag != 0) uppercase(fontname, fontname); */
		copyinhead(fp_out, fontname);
		xmin = INFINITY; ymin = INFINITY;
		xmax = -INFINITY; ymax = -INFINITY;
		(void) getline(fp_in, inline);
		while (strstr(inline, "/CharacterDefs") == NULL) {
			if (strstr(inline, "/Metrics") != NULL) 
				readmetrics(fp_in);
			else if (strstr(inline, "/BBox") != NULL)
				skipbbox(fp_in);
			else if (strstr(inline, "/Encoding") != NULL)
				skipencoding(fp_in);
			else if (strstr(inline, "/FontMatrix") != NULL ||
				strstr(inline, "/FontBBox") != NULL ||
					strstr(inline, "/FontType") != NULL) {
					fprintf(fp_out, "%s", inline);
					if (traceflag != 0) {
						printf("Special: %s", inline);
/*						(void) _getch(); */
					}
			}
			(void) getline(fp_in, inline);
			if (traceflag != 0) {
				printf("Header: %s", inline);
/*				(void) _getch(); */
			}
		}
	}
	else { /* Projective Solutions Format */
		if (wantuniqueid != 0) uniqueid = makeuniqueid(fontname);
/*		copyinhead(fp_out, fontname); */
		if (getline(fp_in, inline) == 0) {
			fprintf(errout, "Unexpected EOF");
			giveup(1);			
		}
		while (*inline == '%' || *inline == '\n') { 
			(void) getline(fp_in, inline);
			checkforfontname(inline);
		}
		if (sscanf(inline, "%lg %lg", &ptsize, &scalef) != 2) {
			fprintf (errout, 
				"First line not design size and scale factor: %s",
					inline);
			giveup(7);
		}
/*		if (scalef == 2.048 && pctexbug != 0) scalef = 2.005; */
		transformflag = 0;
/*		allow for GLOBAL scaling (and slanting) */
		if ((s = strstr(inline, "scale")) != NULL) {	/* 1993/Jan/1 */
			while (*s != ' ' && *s != '\0') s++;
			if (*s != '\0') s++;
/*			n = sscanf(s, "scale %lg %lg", &xscaleinit, &yscaleinit); */
			n = sscanf(s, "%lg %lg %lg", &xscaleinit, &yscaleinit, &slantinit);
			if (n < 1) {
/*				fprintf(errout, "Don't understand scale: %s", s); */
				fprintf(errout, "ERROR: Don't understand scale %s %s",
						s, inline);
/*				giveup(7); */
			}
			if (n < 2) yscaleinit = xscaleinit;
		}
		if (ptsize == 0) {
			printf("WARNING: point size is zero\n");
			ptsize = 10.0;
		}
		if (scalef != 1.0) 
			printf("WARNING: scale factor not unity (%lg)\n", scalef);
		scaleup = 0;
		if (correctscale == 0 && 
			((72.27 * 1000.0) / (dpi * ptsize * scalef) > 1.0)) scaleup = 1;
		if (correctscale != 0 && 
			((72.0 * 1000.0) / (dpi * ptsize * scalef) > 1.0)) scaleup = 1;
		if (adobescaling == 0 ) {
			if (scaleup != 0) fprintf(errout, "WARNING: scaling up! ");
			if (verboseflag != 0 || scaleup != 0) {
				if (correctscale == 0) 	fprintf(errout, "Scaling by %lg\n",
					((72.27 * 1000.0) / (dpi * ptsize * scalef)));
				if (correctscale != 0) 	fprintf(errout, "Scaling by %lg\n",
					((72.0 * 1000.0) / (dpi * ptsize * scalef)));			
			}
		}
		if (verboseflag != 0)
			printf("Font %s - design size %g - scale factor %g\n",
				fontname, ptsize, scalef);
		if (getline(fp_in, inline) == 0) {
			fprintf(errout, "Unexpected EOF");
			giveup(1);
		}
		while (*inline == '%' || *inline == '\n') {
			(void) getline(fp_in, inline);
		}
		/* note interchange of yll and xur - not anymore ! */
		if (sscanf(inline, "%d %d %d %d", &xll, &yll, &xur, &yur) != 4) {
			fprintf (errout, "Second line not FontBBox: %s", inline);
			giveup(7);
		}
		if (xll == xur || yll == yur) {	/* emergency defaults */
			fprintf(errout, "Bad FontBBox: %s", inline);
			xll = -50; yll = -250; xur = 1000; yur = 1000;
		}
		xll = scalenum(xll);
		yll = scalenum(yll);
		xur = scalenum(xur);
		yur = scalenum(yur);
		if (verboseflag != 0)
			printf("Given  FontBBox %d %d %d %d\n", xll, yll, xur, yur);
		xmin = INFINITY; ymin = INFINITY;
		xmax = -INFINITY; ymax = -INFINITY;	
/*		fprintf(fp_out, "/FontType 3 def\n");
		if (scalef == 1.0)
			fprintf(fp_out, "/FontMatrix [0.001 0 0 0.001 0 0] def\n");
		else
			fprintf(fp_out, "/FontMatrix [%lg 0 0 %lg 0 0] def\n",
				0.001/scalef, 0.001/scalef);
		fprintf(fp_out, "/FontBBox [%d %d %d %d] def\n", 
			xll, yll, xur, yur); */
		c = getc(fp_in); ungetc(c, fp_in);
		while (c == '%' ||  c == '\n') {
			(void) getline(fp_in, inline);
			checkforfontname(inline);
			c = getc(fp_in); ungetc(c, fp_in);
		}
		copyinhead(fp_out, fontname);			/* new place */
		fprintf(fp_out, "/FontType 3 def\n");
		if (strcmp(ver, "") != 0) /* NEW: stick in version number */
			fprintf(fp_out, "/version (%s) def\n", ver);
		if (scalef == 1.0) /*  || adobescaling != 0 */
			fprintf(fp_out, "/FontMatrix [0.001 0 0 0.001 0 0] def\n");
		else {
			fprintf(fp_out, "/FontMatrix [%lg 0 0 %lg 0 0] def\n",
				0.001/scalef, 0.001/scalef);
			if (verboseflag != 0)
				printf("/FontMatrix [%lg 0 0 %lg 0 0] def\n",
					0.001/scalef, 0.001/scalef);
		}
/*		fprintf(fp_out, "/FontBBox {%d %d %d %d} def\n", 
			xll, yll, xur, yur); */ /* ??? */
		fprintf(fp_out, "/FontBBox [%d %d %d %d] def\n", 
			xll, yll, xur, yur); 
/*		while (c == '%' || c == '\n') { 
			(void) getline(fp_in, inline);
			c = getc(fp_in); ungetc(c, fp_in);
		} */
	} /* end of Projective solutions stuff */
	if (wantuniqueid != 0 && uniqueid != -1) 
		fprintf(fp_out, "/UniqueID %ld def\n", uniqueid);
	if (remapflag != 0) nevect = 197;
	else nevect = 128;
	if (nevect < fontchrs) nevect = fontchrs;
	/* copy in front matter */
	fprintf(fp_out, "/CharDefs %d array def\n", nevect);
	if (wantuniqueid == 0) { /* say what ? */
		fprintf(fp_out, 
			"/Encoding 0 1 %d {1 string dup 0 4 -1 roll put cvn} for\n", 
				nevect-1);
	}
	else {
		fprintf(fp_out, 
"/Encoding 0 systemdict {pop exch 1 add dup %d eq {pop exit} if} forall\n",
			nevect);
	}
	fprintf(fp_out, "\t%d array astore def\n", nevect);

	fprintf(fp_out, "%s", front3);		/* copy in front matter */
	/*	fprintf(fp_out, "/CharDefs get\n"); */
}

void checklimits(int x, int y) {
	if (x > xmax) xmax = x;	if (x < xmin) xmin = x;
	if (y > ymax) ymax = y;	if (y < ymin) ymin = y;
}

void closepath(FILE *fp_out) {
	if (xlast == INFINITY && ylast == INFINITY) {
		return;	/* empty character - pointless closepath */
	}
	if (xlast != xstart || ylast != ystart) {	/* new check */
		if (complainopen != 0 || traceflag != 0) 
			fprintf(errout, "Subpath not closed in char %d (%d %d) (%d %d)\n",
					chrs, xstart, ystart, xlast, ylast);
/*		writenuma(xstart, fp_out); writenuma(ystart, fp_out); */
		writecoord(xstart, ystart, fp_out);
		writetext("EA", fp_out);	/* insert a lineto */
		closed = 0;
	}
	if (closed > 0) {  
		fprintf(errout, 
			"ERROR: Closing a path in char %d that is not open\n", chrs);
		fatal++;
	}
	else nputc('D', fp_out); 
/*	writetext("DA", fp_out); */
	closed++;
}

/* int round (double x) {
	if (x > 0.0) return (int) (x + 0.5);
	else if (x < 0.0) return (int) (x - 0.5);
	else return 0;
} */

int xinscale (int x) {	/* input scaling on x */
	if (debugflag) printf("xscale %d ", x);
	if (xcenter == 0) return round (xscale * x); 
	else return round (xscale * (x - xcenter) + xcenter);
}

int yinscale (int y) {	/* input scaling on y */
	if (debugflag) printf("yscale %d ", y);
	if (ycenter == 0) return round (yscale * y); 
	else return round (yscale * (y - ycenter) + ycenter);
}

int xtransform(int x, int y) {
	if (debugflag) printf("xtrans %d %d %lg %lg\n", x, y, slant, slantcomp);
/*	if (slant == 0.0) return xinscale(x); */	 /* revised 97/Aug/2 */
	if (slant == 0.0) {
		if (slantcomp == 0.0) return xinscale(x);
		else {
/*			printf("x %d y %d xd %d yd %d\n", x, y, xinscale(x), yinscale(y)); */
			return xinscale(x) + round(slantcomp * (yinscale(y) - y));
		}
	}
	else return xinscale(x) + round(yinscale(y) * slant);
}

int ytransform(int x, int y) {
	if (debugflag) printf("ytrans %d %d %lg %lg\n", x, y, slant, slantcomp); 
	return yinscale(y);
}

void moveto(FILE *fp_out) {
	double fx, fy;
	int x, y, xt;
	if (closed == 0) {
		fprintf(errout, "Subpath in char %d not closed\n", chrs);
		fatal++;		/* ??? */
	}
/*	if (sscanf(inline, " %d %d moveto", &x, &y) != 2) { */
	if (fixonly) {
		if (sscanf(inline, " %d %d m", &x, &y) != 2) {
			fprintf(errout, "ERROR: Invalid moveto: %s", inline);
			giveup(12);
		}
	}
	else {
		if (sscanf(inline, " %lg %lg m", &fx, &fy) != 2) { 
			fprintf(errout, "ERROR: Invalid moveto: %s", inline);
			giveup(12);
		}
		x = round(fx); y = round(fy);
	}
	xprev = x; yprev = y;				/* 1995/Nov/30 */
	if (rusticflag != 0 && correctscale != 0) { /* new */
		x = round(((double) x) * 72.27 / 72.0); 
		y = round(((double) y) * 72.27 / 72.0); 
	}
	x = (int) (x + xoff);	y = (int) (y + yoff);
/*	if (xscale != 1.0 || yscale != 1.0) { */
	if (transformflag) {
/*		if (slant == 0.0) { */
		if (slant == 0.0 && slantcomp == 0.0) {
/*			x = round (scale * x); y = round (scale * y); */
			x = xinscale (x); y = yinscale (y);
		}
		else {
			xt = xtransform (x,y);
			y = ytransform (x,y);
			x = xt;
		}
	}
/*	if (firstmove != 0) { */
		nputc('A', fp_out);
/*		firstmove = 0;	} */
	if (rusticflag == 0) {
		xold = x;
		yold = y;
		x = scalenum(x);
		y = scalenum(y);
	}
	xstart = x; ystart = y; /* save for closepath */
	xlast = x;  ylast = y;	/* save for closepath */
	checklimits(x, y);
/*	writenuma(x, fp_out); writenuma(y, fp_out); */
	writecoord(x, y, fp_out);
	writetext("FA", fp_out);	/* moveto */
	closed = 0;
}

void lineto(FILE *fp_out) {
	double fx, fy;
	int x, y, xt;
/*	if (sscanf(inline, " %d %d lineto", &x, &y) != 2) { */
	if (fixonly) {
		if (sscanf(inline, " %d %d l", &x, &y) != 2) { 
			fprintf(errout, "ERROR: Invalid lineto: %s", inline);
			giveup(12);
		}
	}
	else {
		if (sscanf(inline, " %lg %lg l", &fx, &fy) != 2) { 
			fprintf(errout, "ERROR: Invalid lineto: %s", inline);
			giveup(12);
		}
		x = round(fx); y = round(fy);
	}
	xprev = x; yprev = y;				/* 1995/Nov/30 */
	if (rusticflag != 0 && correctscale != 0) { /* new */
		x = round(((double) x) * 72.27 / 72.0); 
		y = round(((double) y) * 72.27 / 72.0); 
	}		
	x = (int) (x + xoff);	y = (int) (y + yoff);
/*	if (xscale != 1.0 || yscale != 1.0) { */
	if (transformflag) {
/*		if (slant == 0.0) { */
		if (slant == 0.0 && slantcomp == 0.0) {
/*			x = round (scale * x); y = round (scale * y); */
			x = xinscale (x); y = yinscale (y);
		}
		else {
			xt = xtransform (x,y);
			y = ytransform (x,y);
			x = xt;
		}
	}
	if (rusticflag == 0) {
		if (checknear) {
			if (x == xold + 1 || x == xold - 1 ||
				y == yold + 1 || y == yold - 1)
			fprintf(errout, 
				"\nIn char %d, sideslip LINETO at (%d %d) (%d, %d) ",
					fchrs, xold, yold, x, y);
		}
		if (x == xold && y == yold) {
			if (quietflag == 0) {
			fprintf(errout, 
				"\nIn char %d, coincident knots in LINETO at (%d, %d)\n",
					fchrs, x, y);
			fprintf(errout, "A == B");
			if (bellflag) fprintf(errout, "\a");
			}
/*			(void) _getch(); */
			return; /* forget it ! */
		}
/* also test coincidence of end points after scaling */
		if (scalenum(xold) == scalenum(x) && scalenum(yold) == scalenum(y)) {
			if (traceflag != 0)  /* verboseflag */
				printf(
			"\nIn char %d, coincident knots in lineto at (%d, %d) (%d %d)\n",
						fchrs, xold, yold, x, y);
			return; /* forget it ! */
		}
		xold = x;
		yold = y;
		x = scalenum(x);
		y = scalenum(y);
	}
/* avoid writing worthless linetos if possible */
	if (x != xlast || y != ylast) {
/*		writenuma(x, fp_out); writenuma(y, fp_out); */
		writecoord(x, y, fp_out);
		writetext("EA", fp_out);	/* lineto */
		checklimits(x, y);
		xlast = x; ylast = y;	/* save for closepath */
		closed = 0;
	}
}

void oval (FILE *fp_out) {
	double fxa, fya, fxb, fyb; /* fxc, fyc; */
	int xa, ya, xb, yb, xc, yc, xt;
	if (fixonly) {
		if (sscanf(inline, " %d %d %d %d o",
			&xa, &ya, &xb, &yb) != 4) { 
			fprintf(errout, "Invalid oval: %s", inline);
			giveup(12);
		}
	}
	else {
		if (sscanf(inline, " %lg %lg %lg %lg o",
			&fxa, &fya, &fxb, &fyb) != 4) { 
			fprintf(errout, "Invalid oval: %s", inline);
			giveup(12);
		}
		xa = round(fxa); ya = round (fya);
		xb = round(fxb); yb = round (fyb);
	}
	if (rusticflag != 0 && correctscale != 0) { /* new */
		xa = round(((double) xa) * 72.27 / 72.0); 
		ya = round(((double) ya) * 72.27 / 72.0); 
		xb = round(((double) xb) * 72.27 / 72.0); 
		yb = round(((double) yb) * 72.27 / 72.0); 
	}		
	xa = (int) (xa + xoff); ya = (int) (ya + yoff);
	xb = (int) (xb + xoff); yb = (int) (yb + yoff);
/*	if (xscale != 1.0 || yscale != 1.0)  */
/*		x = round (scale * x); y = round (scale * y); */
/*	if (slant == 0.0) { */
	if (slant == 0.0 && slantcomp == 0.0) {
		xa = xinscale (xa); ya = yinscale (ya);
		xb = xinscale (xb); yb = yinscale (yb);
	}
	else {
		xt = xtransform(xa, ya);
		ya = ytransform(xa, ya);
		xa = xt;
		xt = xtransform(xb, yb);
		yb = ytransform(xb, yb);
		xb = xt;
	}

	if (rusticflag == 0) {
		xold = xb;
		yold = yb;
		xa = scalenum(xa);
		ya = scalenum(ya);
		xb = scalenum(xb);
		yb = scalenum(yb);
	}
	xc = xb; yc = yb;
	xb = (int) (xa * bezierq + xb * (1.0 - bezierq));
	yb = (int) (ya * bezierq + yb * (1.0 - bezierq));
	xa = (int) (xa * bezierq + xlast * (1.0 - bezierq));
	ya = (int) (ya * bezierq + ylast * (1.0 - bezierq));
	writecoord(xa, ya, fp_out);
	writecoord(xb, yb, fp_out);
	writecoord(xc, yc, fp_out);
	writetext("CA", fp_out);	/* curveto */
	checklimits(xa, ya);
	checklimits(xb, yb);
	checklimits(xc, yc);
	xlast = xc; ylast = yc;		/* save for closepath */
	closed = 0;
}

void dooval (FILE *fp_out, int xa, int ya, int xb, int yb, int xc, int yc) {
	int x1, y1, x2, y2;
	if (xa == xb) x1 = xa;
	else x1 = (int) (xb * bezierq + xa * (1.0 - bezierq));
	if (ya == yb) y1 = ya;
	else y1 = (int) (yb * bezierq + ya * (1.0 - bezierq));
	if (xb == xc) x2 = xc;
	else x2 = (int) (xb * bezierq + xc * (1.0 - bezierq));
	if (yb == yc) y2 = yc;
	else y2 = (int) (yb * bezierq + yc * (1.0 - bezierq));
/*	we are at (xa, ya) */
	writecoord(x1, y1, fp_out);
	writecoord(x2, y2, fp_out);
	writecoord(xc, yc, fp_out);
	writetext("CA", fp_out);	/* curveto */
	checklimits(xa, ya);
	checklimits(xb, yb);
	checklimits(xc, yc);
	xlast = xc; ylast = yc;		/* save for closepath */
	closed = 0;
}

void circle (FILE *fp_out) {				/* 1994/Mar/11 */
	double fxll, fyll, fxur, fyur;
	int xll, yll, xur, yur, xtt;
	int xcen, ycen;
/*	double fxa, fya, fxb, fyb;  */
/*	int xa, ya, xb, yb, xc, yc; */
	if (fixonly) {
		if (sscanf(inline, " %d %d %d %d x",
			&xll, &yll, &xur, &yur) != 4) { 
			fprintf(errout, "Invalid circle: %s", inline);
			giveup(12);
		}
	}
	else {
		if (sscanf(inline, " %lg %lg %lg %lg x",
			&fxll, &fyll, &fxur, &fyur) != 4) { 
			fprintf(errout, "Invalid circle: %s", inline);
			giveup(12);
		}
		xll = round(fxll); yll = round (fyll);
		xur = round(fxur); yur = round (fyur);
	}
	if (rusticflag != 0 && correctscale != 0) { /* new */
		xll = round(((double) xll) * 72.27 / 72.0); 
		yll = round(((double) yll) * 72.27 / 72.0); 
		xur = round(((double) xur) * 72.27 / 72.0); 
		yur = round(((double) yur) * 72.27 / 72.0); 
	}		
	xll = (int) (xll + xoff); yll = (int) (yll + yoff);
	xur = (int) (xur + xoff); yur = (int) (yur + yoff);
/*	if (xscale != 1.0 || yscale != 1.0) { */
	if (transformflag) {
/*		if (slant == 0.0) { */
		if (slant == 0.0 && slantcomp == 0.0) {
/*			x = round (scale * x); y = round (scale * y); */
			xll = xinscale (xll); yll = yinscale (yll);
			xur = xinscale (xur); yur = yinscale (yur);
		}
		else {	/* ??? */
			xtt = xtransform (xll, yll);
			yll = ytransform (xll, yll);
			xll = xtt;
			xtt = xtransform (xur, yur);
			yur = ytransform (xur, yur);
			xur = xtt;
		}
	}
	if (rusticflag == 0) {
		xold = xur;
		yold = yur;
		xll = scalenum(xll);
		yll = scalenum(yll);
		xur = scalenum(xur);
		yur = scalenum(yur);
	}
	xcen = (xll + xur) / 2;  ycen = (yll + yur) / 2;
/*	xc = xur; yc = yur; */
	xstart = xll; ystart = ycen; /* save for closepath */
	nputc('A', fp_out);
	writecoord(xll, ycen, fp_out);
	writetext("FA", fp_out);	/* moveto */	
	if (xll < xur && yll < yur) {
		dooval(fp_out, xll, ycen, xll, yll, xcen, yll);
		dooval(fp_out, xcen, yll, xur, yll, xur, ycen);
		dooval(fp_out, xur, ycen, xur, yur, xcen, yur);
		dooval(fp_out, xcen, yur, xll, yur, xll, ycen);
	}
	else {	/* reversed contour 1996/June/15 interchange yll and yur */
		dooval(fp_out, xll, ycen, xll, yur, xcen, yur);
		dooval(fp_out, xcen, yur, xur, yur, xur, ycen);
		dooval(fp_out, xur, ycen, xur, yll, xcen, yll);
		dooval(fp_out, xcen, yll, xll, yll, xll, ycen);
	}
	checklimits(xll, yll);
	checklimits(xur, yur);
	xlast = xll; ylast = ycen;		/* save for closepath */
	closed = 0;
/*	current = 0; */			/* no current point */
	closepath(fp_out); 
}

/*	Expand quadrant code into curveto 1995/Nov/30 */
/*	More generally put in circular arc or elliptical arc */
/*	We mostly use right angle corner case --- 90 degree turn angle */

/*  More generally k = (4/3) cos theta/2 / (1 + cos theta/2) */
/*	Where theta is the angle we turn through */
/*	cos theta/2 = sqrt ((1/2) (1 + cos theta)) */
/*	sin theta/2 = sqrt ((1/2) (1 - cos theta)) */
/*	and cos theta can be found using dot-product of supplied vectors */

int expandquadrant(void) {
	double fxc, fyc, fxe, fye;
	int xs, ys, xc, yc, xe, ye;
	int xa, ya, xb, yb;
	double dotprod, magones, magtwos, costheta, costhetahalf, k;

	if (fixonly) {
/*		if (sscanf(inline, "%d %d %d %d %d %d q",
			&xs, &ys, &xc, &yc, &xe, &ye) != 6)  */
		if (sscanf(inline, "%d %d %d %d q",
				   &xc, &yc, &xe, &ye) != 4) { 
			fprintf(errout, "Invalid quadrant: %s", inline);
			return -1;
		}
	}
	else {
/*		if (sscanf(inline, "%lg %lg %lg %lg %lg %lg x",
			&fxs, &fys, &fxc, &fyc, &fxe, &fye) != 6)  */
		if (sscanf(inline, "%lg %lg %lg %lg x",
			&fxc, &fyc, &fxe, &fye) != 4) { 
			fprintf(errout, "Invalid quadrant: %s", inline);
			return -1;
		}
/*		xs = round(fxs); ys = round (fys); */
		xc = round(fxc); yc = round (fyc);
		xe = round(fxe); ye = round (fye);
	}
	xs = xprev; ys = yprev;
/*  Find cosine of the angle between */
/*	(xc - xs, yc - ys) and  (xe - xc, ye - yc) */
	dotprod = (double) (xc - xs) * (double) (xe - xc) +
		   (double) (yc - ys) * (double) (ye - yc);
	magones = (double) (xc - xs) * (double) (xc - xs)
			  + (double) (yc - ys) * (double) (yc - ys);
	magtwos = (double) (xe - xc) * (double) (xe - xc)
			  + (double) (ye - yc) * (double) (ye - yc);
	if (dotprod == 0.0) costheta = 0.0;
	else if (magones != 0.0 && magtwos != 0.0)
		costheta = dotprod / sqrt (magones * magtwos);
	else costheta = 1.0;
	if (costheta == 0) {
/*		costhetahalf = 1.0 / sqrt(2.0); */
		k = 0.5541;			/* best value for 90 degree turn */
	}
	else {
		costhetahalf = sqrt((1.0 + costheta) / 2.0);
		k = (4.0 / 3.0) * costhetahalf / (1 + costhetahalf);
		if (k > 1.0) k = 1.0;
		else if (k < 0.0) k = 0.0;
	}
	xa = xs + round (k * (xc - xs));
	ya = ys + round (k * (yc - ys));
	xb = xe + round (k * (xc - xe));
	yb = ye + round (k * (yc - ye));
/*	We are assuming that current point is (xs, ys) */
	sprintf(inline, "%d %d %d %d %d %d c\n",
		   xa, ya, xb, yb, xe, ye);
	if (traceflag) printf("CONVERTED TO: %s", inline);
	return 0;
}

/* adjust zb in direction (zc - za), so round(zb) != round(za) */
int adjustround(int za, int zb, int zc) { 
	int ia, ib, ic, zbd;

	if (adjustknots == 0) return zb;
/*	if (za == zb) {
		if (verboseflag != 0)  
			printf("A == B in char %d (%d %d %d) ", fchrs, za, zb, zc);
		return zb; 
	} */
	ia = round(scalenum(za));
	ib = round(scalenum(zb));
	ic = round(scalenum(zc));
/*	assert(ia == ib); */
	if (ia != ib) fprintf(errout, "ERROR: ia != ib (%d != %d)\n", ia, ib);
	if (ib == ic) {
		if (traceflag != 0)		/* verboseflag */
			printf("In char %d IB == IC (%d %d %d) -> (%d %d %d) FAIL ", 
				fchrs, za, zb, zc, ia, ib, ic);
		return zb; /* for now */
	}
/*	assert(ib != ic); */
	if (zc > zb) ib++;
	else if (zc < zb) ib--;
	zbd = unscalenum(ib);
/*	assert(scalenum(zbd) == ib); */
	if (scalenum(zbd) != ib)
		fprintf(errout, "scalenum(zbd) != ib (%d != %d)\n",
			scalenum(zbd), ib);
/* 	assert(zbd != zb); */
	if (zbd != zb && adobescaling != 2)
		fprintf(errout, "NOTE: zbd != zb (%d != %d) in char %d\n", zbd, zb, chrs);
	if (traceflag != 0)	/* verboseflag */
		printf("\nIn char %d: (%d %d %d) => (%d %d %d) ",
			fchrs, za, zb, zc, za, zbd, zc);
/*	if (za == zb) if (verboseflag != 0)	printf("A == B"); */
	return zbd;
}

/* adjust zb in direction (zc - za), so zb != za */
/* this treats things differently for case scaleup != 0 */
/* in this case make adjustment AFTER scaling */
int adjustscaled(int za, int zb, int zc, int flag, int ub) { 
/*	int ia, ib, ic; */
	int zbd;

	if (adjustknots == 0) return zb;
/*	assert(za == zb); */
	if (za != zb) fprintf(errout, "za != zb (%d != %d)\n", za, zb);
	if (zc == zb) {
		if (verboseflag != 0)
			printf("In char %d ZB == ZC (%d %d %d) FAIL ", 
				fchrs, za, zb, zc);
			return zb; /* for now */
	}
/*	assert(zc != zb); */
	if (zc > za) zbd = zb+1;
	else zbd = zb-1;
	if (zbd == zc) {
		fprintf(stdout, "\nIn char %d (%s) Can't adjust: ", fchrs, fontname);
		if (flag != 0) fprintf(stdout, "(%d %d) ", ub, zb);
		else fprintf(stdout, "(%d %d) ", zb, ub);
		fprintf(stdout, "(%d %d %d) ", za, zb, zc); /* errout ? */
/*		if (bellflag) fprintf(errout, "\a");  (void) _getch(); */
		zbd = zb;
	}
/*	assert(zbd != zc); */
	if (traceflag != 0)		/* verboseflag */
		printf("\nIn char %d: (%d %d %d) => (%d %d %d) ",
			fchrs, za, zb, zc, za, zbd, zc);
/*	if (za == zb) if (verboseflag != 0)	printf("A == B"); */
	return zbd;
} 

void curvetosub(FILE *fp_out, 
		int xa, int ya, int xb, int yb, int xc, int yc) {
	int xas, yas, xbs, ybs, xcs, ycs, xolds, yolds;
	int dx, dy, oxa, oya;
	if (rusticflag == 0) {
		if (checknear) {
			if (xa == xold + 1 || xa == xold - 1 ||
				ya == yold + 1 || ya == yold - 1 ||
/*				xb == xa + 1 || xb == xa - 1 || */
/*				yb == ya + 1 || yb == ya - 1 || */
				xc == xb + 1 || xc == xb - 1 ||
				yc == yb + 1 || yc == yb - 1) {
			fprintf(errout, 
				"\nIn char %d, sideslip knots: (%d, %d) (%d %d) (%d %d) (%d %d)",
					fchrs, xold, yold, xa, ya, xb,yb, xc, yc);
			}	
		}
		if ((xa == xold && ya == yold) ||
			(xb == xa && yb == ya)  ||
			(xc == xb && yc == yb)) {
			if (traceflag != 0) /* verboseflag */
				printf(
"\nIn char %d coincident knots: (%d, %d) (%d %d) (%d %d) (%d %d)",
					fchrs, xold, yold, xa, ya, xb,yb, xc, yc);
		}
		if (scaleup == 0) {
			if (scalenum(xold) == scalenum(xa) && 
				scalenum(yold) == scalenum(ya)) {
				dx = xb - xold; dy = yb - yold;
				if (ABS(dx) > ABS(dy)) 	xa = adjustround(xold, xa, xb);
				else ya = adjustround(yold, ya, yb);
			}
			if (scalenum(xb) == scalenum(xa) && 
				scalenum(yb) == scalenum(ya)) {
				oxa = xa;	oya = ya;
				dx = xb - xold; dy = yb - yold;
				if (ABS(dx) > ABS(dy)) xa = adjustround(xb, xa, xold);
				else ya = adjustround(yb, ya, yold);
				dx = xc - oxa; dy = yc - oya;
				if (ABS(dx) > ABS(dy)) xb = adjustround(oxa, xb, xc); 
				else yb = adjustround(oya, yb, yc); 
			}
			if (scalenum(xc) == scalenum(xb) && 
				scalenum(yc) == scalenum(yb)) {
				dx = xc - xa, dy = yc - ya;
				if (ABS(dx) > ABS(dy))  xb = adjustround(xc, xb, xa);
				else yb = adjustround(yc, yb, ya);
			}
			xold = xc; yold = yc;
			xas = scalenum(xa), yas = scalenum(ya); 
			xbs = scalenum(xb), ybs = scalenum(yb); 
			xcs = scalenum(xc), ycs = scalenum(yc); 
		}
		else {	/* that is, when scalup != 0 */
			xas = scalenum(xa), yas = scalenum(ya); 
			xbs = scalenum(xb), ybs = scalenum(yb); 
			xcs = scalenum(xc), ycs = scalenum(yc); 
			xolds = scalenum(xold); yolds = scalenum(yold);
			if (xolds == xas && yolds == yas) {
				dx = xbs - xolds; dy = ybs - yolds;
				if (ABS(dx) > ABS(dy)) 	
					xas = adjustscaled(xolds, xas, xbs, 0, yas);
				else yas = adjustscaled(yolds, yas, ybs, 1, xas);
			}
			if (xbs == xas && ybs == yas) {
				oxa = xas;	oya = yas;
				dx = xbs - xolds; dy = ybs - yolds;
				if (ABS(dx) > ABS(dy)) 
					xas = adjustscaled(xbs, xas, xolds, 0, yas);
				else yas = adjustscaled(ybs, yas, yolds, 1, xas);
				dx = xcs - oxa; dy = ycs - oya;
				if (ABS(dx) > ABS(dy)) 
					xbs = adjustscaled(oxa, xbs, xcs, 0, ybs);
				else ybs = adjustscaled(oya, ybs, ycs, 1, xbs);
			}
			if (xcs == xbs && ycs == ybs) {
				dx = xcs - xas, dy = ycs - yas;
				if (ABS(dx) > ABS(dy))  
					xbs = adjustscaled(xcs, xbs, xas, 0, ybs);
				else ybs = adjustscaled(ycs, ybs, yas, 1, xbs);
			}
			xold = xc; yold = yc;
		}
	}
	else { /* `rustic' format is already scaled */
		xas = xa; yas = ya; xbs = xb; ybs = yb; xcs = xc; ycs = yc;
	}
/* avoid writing worthless curvetos if possible */
	if (xlast != xas || ylast != yas ||
		xas != xbs || yas != ybs || xbs != xcs || ybs != ycs) {
/*		writenuma(xas, fp_out); writenuma(yas, fp_out); */
		writecoord(xas, yas, fp_out);
/*		writenuma(xbs, fp_out); writenuma(ybs, fp_out); */
		writecoord(xbs, ybs, fp_out);
/*		writenuma(xcs, fp_out); writenuma(ycs, fp_out); */
		writecoord(xcs, ycs, fp_out);
		writetext("CA", fp_out);	/* curveto */
		checklimits(xas, yas);
		checklimits(xbs, ybs);
		checklimits(xcs, ycs);
		xlast = xcs; ylast = ycs;	/* save for closepath */
		closed = 0;
	}
}

void curveto(FILE *fp_out) {
	int xa, ya, xb, yb, xc, yc, xt;
	double fxa, fya, fxb, fyb, fxc, fyc;
	if (fixonly) {
		if (sscanf(inline, " %d %d %d %d %d %d c",  
			&xa, &ya, &xb, &yb, &xc, &yc) != 6) {
			fprintf(errout, "Invalid curveto: %s", inline);
			giveup(12);
		}
	}
	else {
		if (sscanf(inline, " %lg %lg %lg %lg %lg %lg c",  
			&fxa, &fya, &fxb, &fyb, &fxc, &fyc) != 6) {
			fprintf(errout, "Invalid curveto: %s", inline);
			giveup(12);
		}
		xa = round (fxa); ya = round (fya);
		xb = round (fxb); yb = round (fyb);
		xc = round (fxc); yc = round (fyc);
	}
	xprev = xc; yprev = yc;						/* 1995/Nov/19 */
	if (rusticflag != 0 && correctscale != 0) { /* new */
		xa = round(((double) xa) * 72.27 / 72.0); 
		ya = round(((double) ya) * 72.27 / 72.0); 
		xb = round(((double) xb) * 72.27 / 72.0); 
		yb = round(((double) yb) * 72.27 / 72.0); 
		xc = round(((double) xc) * 72.27 / 72.0); 
		yc = round(((double) yc) * 72.27 / 72.0); 		
	}
	xa = (int) (xa + xoff); ya = (int) (ya + yoff);	 
	xb = (int) (xb + xoff); yb = (int) (yb + yoff);		
	xc = (int) (xc + xoff); yc = (int) (yc + yoff);		
/*	if (xscale != 1.0 || yscale != 1.0) { */
	if (transformflag) {
/*		if (slant == 0.0) { */
		if (slant == 0.0 && slantcomp == 0.0) {
			xa = xinscale (xa); ya = yinscale (ya);
			xb = xinscale (xb); yb = yinscale (yb);
			xc = xinscale (xc); yc = yinscale (yc);
		}
		else {
			xt = xtransform (xa, ya);
			ya = ytransform (xa, ya);
			xa = xt;
			xt = xtransform (xb, yb);
			yb = ytransform (xb, yb);
			xb = xt;
			xt = xtransform (xc, yc);
			yc = ytransform (xc, yc);
			xc = xt;
		}
	}
	curvetosub(fp_out, xa, ya, xb, yb, xc, yc);
}

void subrtrick(FILE *fp_out, int n) {
	int k;
/*  this is the trick code we use to indicate call subr */
/*	curvetosub(fp_out, nsubr, nsubr, nsubr, nsubr, nsubr, nsubr); */
	for (k = 0; k < 6; k++)	writenuma(n, fp_out);
	writetext("CA", fp_out);
}

void callsubr(FILE *fp_out) { /* new, trick for hint replacement */
	int nsubr;
	if (sscanf(inline, " %d s", &nsubr) != 1) {
		fprintf(errout, "\nERROR: Invalid callsubr: %s", inline);
		giveup(12);
	}
	subrtrick(fp_out, nsubr);
}

/* For a curveto that is spread over three lines */
void longcurveto(FILE *fp_in, FILE *fp_out) {
	int xa, ya, xb, yb, xc, yc, xt;
	if (sscanf(inline, " %d %d", &xa, &ya) != 2) {
		fprintf(errout, "\nERROR: Invalid potential long curveto: %s", inline);
		if (getline(fp_in, inline) == 0)
			fprintf(errout, "EOF!\n");
		else fprintf(errout, "Next line: %s", inline);
		giveup(12);
	}
	if (rusticflag != 0 && correctscale != 0) { /* new */
		xa = round(((double) xa) * 72.27 / 72.0); 
		ya = round(((double) ya) * 72.27 / 72.0); 
	}
	xa = (int) (xa + xoff); ya = (int) (ya + yoff);	
/*	if (xscale != 1.0 || yscale != 1.0) { */
	if (transformflag) {
/*		if (slant == 0.0) { */
		if (slant == 0.0 && slantcomp == 0.0) {
			xa = xinscale (xa); ya = yinscale (ya);
		}
		else {
			xt = xtransform (xa, ya);
			ya = ytransform (xa, ya);
			xa = xt;
		}
	}
	(void) getline(fp_in, inline);
	if (sscanf(inline, " %d %d", &xb, &yb) != 2) {
		fprintf(errout, "ERROR: Invalid potential long curveto: %s", inline);
		if (getline(fp_in, inline) == 0)
			fprintf(errout, "EOF!\n");
		else fprintf(errout, "Next line: %s", inline);
		giveup(12);
	}
	if (rusticflag != 0 && correctscale != 0) { /* new */
		xb = round(((double) xb) * 72.27 / 72.0); 
		yb = round(((double) yb) * 72.27 / 72.0); 
	}
	xb = (int) (xb + xoff); yb = (int) (yb + yoff);		
/*	if (xscale != 1.0 || yscale != 1.0) { */
	if (transformflag) {
/*		if (slant == 0.0) { */
		if (slant == 0.0 && slantcomp == 0.0) {
			xb = xinscale (xb); yb = yinscale (yb);
		}
		else {
			xt = xtransform (xb, yb);
			yb = ytransform (xb, yb);
			xb = xt;
		}
	}
	(void) getline(fp_in, inline);
	if (sscanf(inline, " %d %d c", &xc, &yc) != 2) {
		fprintf(errout, "ERROR: Invalid potential long curveto: %s", inline);
		if (getline(fp_in, inline) == 0)
			fprintf(errout, "EOF!\n");
		else fprintf(errout, "Next line: %s", inline);
		giveup(12);
	}
	xc = (int) (xc + xoff); yc = (int) (yc + yoff);		
/*	if (xscale != 1.0 || yscale != 1.0) { */
	if (transformflag) {
/*		if (slant == 0.0) { */
		if (slant == 0.0 && slantcomp == 0.0) {
			xc = xinscale (xc); yc = yinscale (yc);
		}
		else {
			xt = xtransform (xc, yc);
			yc = ytransform (xc, yc);
			xc = xt;
		}
	}
	if (rusticflag != 0 && correctscale != 0) { /* new */
		xc = round(((double) xc) * 72.27 / 72.0); 
		yc = round(((double) yc) * 72.27 / 72.0); 
	}
	curvetosub(fp_out, xa, ya, xb, yb, xc, yc);
}

/* void startchar(int chrs, int width, FILE *fp_out) { */
void startchar(int chrs, double fwidth, FILE *fp_out) {
	int width;
/*	if (traceflag != 0) printf("Start %d width %d\n", chrs, width); */
	if (traceflag != 0) printf("Start %d width %lg\n", chrs, fwidth);
	writetext("dup ", fp_out);
	writenum(chrs, fp_out);
	writetext("<", fp_out);
	numhex=0;
	width = round (fwidth);					/* 95/Dec/10 */
/*	writenuma(width, fp_out); */
	writenum(width, fp_out); 
/*	firstmove = 1; */
	xlast = INFINITY; ylast = INFINITY;	/* new */
	closed = 1;
}

void finishchar(FILE *fp_out) {
	if (xlast == INFINITY || ylast == INFINITY) { /* empty ? */
		if (complainopen != 0 || traceflag != 0) 
			fprintf(errout, "Empty character %d\n", chrs);
/*		nputc('D', fp_out);		*/
		writetext("AD", fp_out);		/* fake closepath */
	}
	if (closed == 0) {
		fprintf(errout, "\nLast path in char %d not closed (%s)\n", chrs, fontname);
		if (closelastpath) {
/*			current = 0; */
			closepath(fp_out);	/* fake closepath ? */ /* 94/Oct/15 */
		}
	}
	if (traceflag != 0) printf("Finish\n");
	if (numhex % 2 != 0) nputc('A', fp_out);	/* check on even hex chars ? */
	nputc('>', fp_out);
	if (clm > COLUMNS-3) nputc('\n', fp_out);
	writetext("put\n", fp_out);
	if (ferror(fp_out) != 0) {
		perror("Output error");
		giveup(2);
	}
}

void flushchar(FILE *fp_in) {
	if (rusticflag != 0) 
		while (strstr(inline, "} def") == NULL) 
			(void) getline(fp_in, inline);
	else 
		while (strchr(inline, ']') == NULL)
			if (getline(fp_in, inline) == 0) break; /* EOF */
}

void dochar(FILE *fp_in, FILE *fp_out, int fchrs) {
/*	int width; */
	double fwidth;
	int current=0;		/* no current point */ /* NEW */
	char *s, *sc;
	int n;

/* xscale = 1.0; yscale = 1.0;  */
	xoff = 0.0; yoff = 0.0;

/*	if (xoff != 0 || yoff != 0) 
		printf("Offset (%d %d) in character %d\n", xoff, yoff,
		fchrs); */

/*	width = widths[fchrs]; */
	fwidth = fwidths[fchrs];

	if (rusticflag != 0) {
		if (strstr(inline, "} def") != NULL)	return; /* empty character */ 
		(void) getline(fp_in, inline);
		if (strstr(inline, "width") != NULL) {
/*			if (sscanf(inline, " %d width", &width) < 1) { */
			if (sscanf(inline, " %lg width", &fwidth) < 1) {
				fprintf(errout,
					"ERROR: Can't understand width specification:	%s", 
						inline);
				fatal++;		/* ??? */
			}
			(void) getline(fp_in, inline);
		}
		if (strstr(inline, "} def") != NULL) return; /* empty escape */
	}
/*	startchar(fchrs, width, fp_out); */
	startchar(fchrs, fwidth, fp_out);

	while((rusticflag != 0 && strstr(inline, "} def") == NULL) ||
		  (rusticflag == 0 && 
			  (strchr(inline, ']') == NULL && strlen(inline) > 0))) {
		if (*inline == '%' || *inline == '\n') {
/*			printf("%s", inline); (void) _getch();  */
			if (getline(fp_in, inline) == 0) {
				fprintf(errout, "Unexpected EOF");
				giveup(1);
			}
			continue; 
		}			
		sc = strchr(inline, '%');			/* comment position if any */
		if (sc == NULL) sc = inline + strlen(inline);
		if (current != 0 && (strstr(inline, "moveto") != NULL || 
/*				strchr(inline, 'm') != NULL)) {*/
/*				strstr(inline, " m") != NULL)) { */
				 ((s = strstr(inline, " m")) != NULL && s < sc))) { /* 97/Jul/1 */
			fprintf(errout, "\nMoveto in path in char %d\n", 
					fchrs);
			if (allowmovetoin) {
				current = 0;		/* 1994/Oct/15 */
				closepath(fp_out);	/* 1994/Oct/15 */
			}
			else {
				if ((s = strstr(inline, "moveto")) != NULL)
					strcpy(s, "lineto");
/*				else if ((s = strchr(inline, 'm')) != NULL) */
				else if (((s = strchr(inline, 'm')) != NULL) && s < sc)	/* 97/Jul/1 */
					*s = 'l'; 
				fatal++;
			}
		}
/*		Easy method to give quadrant of circle or ellipse */
/*		if (strstr(inline, " q") != NULL) expandquadrant(); */
		if ((s = strstr(inline, " q")) != NULL && s < sc) {
			expandquadrant();
			sc = inline + strlen(inline);
		}
/*		Expand it into curveto format 95/Nov/29 */
		if (strstr(inline, "closepath") != NULL ||	
			strstr(inline, "cp") != NULL ||
/*			strchr(inline, 'h') != NULL) {  */
			((s = strchr(inline, 'h')) != NULL && s < sc)) {  /* 97/Jul/1 */
/*			strstr(inline, " h") != NULL) { */
			current = 0;			/* no current point */
			closepath(fp_out); 
		}
		else if (strstr(inline, "moveto") != NULL ||
/*			strchr(inline, 'm') != NULL) { */
/*			strstr(inline, " m") != NULL) { */
				 ((s = strstr(inline, " m")) != NULL && s < sc)) {	/* 97/Jul/1 */
			if (current != 0) {
/*				if (allowmoveto) closepath(fp_out); */ /* ??? */
				fprintf(errout, "\nERROR: Moveto in path in char %d\n", 
					fchrs); /* ??? */
				fatal++;
			}
			current++;				/* have a current point */
/*********************** support offset and slant ****************************/
			if ((s = strstr(inline, "offset")) != NULL) {
/*				scale = 1.0;	*/
/*				if (sscanf(s, "offset %d %d %lg", &xoff, &yoff, &scale) < 2) { */
				xoff = 0.0; yoff = 0.0;					/* 1992/Sep/12 */
/*				if (sscanf(s, "offset %d %d", &xoff, &yoff) < 2) {  */
				if (sscanf(s, "offset %lg %lg", &xoff, &yoff) < 2) { 
					fprintf(errout,
						"ERROR: Don't understand offset %s %s", s, inline);
					fatal++;	/* ??? */
				}
				if (verboseflag && showoffsets) {
/*					if (xscale != 1.0 || yscale != 1.0)  */
					if (xscale != xscaleinit || yscale != yscaleinit) 
/*						printf("Offset %d %d scale %lg %lg in char %d\n",  */
						printf("Offset %lg %lg scale %lg %lg in char %d\n", 
							xoff, yoff, xscale, yscale, fchrs);
					else if (xoff != 0.0 || yoff != 0.0)
/*						printf("Offset %d %d in char %d\n",  */
						printf("Offset %lg %lg in char %d\n", 
							xoff, yoff, fchrs);
				}
			}
			else if ((s = strstr(inline, "slant")) != NULL) {
				slant = 0.0;
				if (sscanf(s, "slant %lg", &slant) < 1) { 
					fprintf(errout,
							"ERROR: Don't understand slant %s %s", s, inline);
					fatal++;	/* ??? */
				}
				else transformflag++;
/*				if (verboseflag && showoffsets) printf("SLANT: %lg\n", slant); */
				if (verboseflag) printf("SLANT: %lg\n", slant);
			}
/******************* experiment to reset scale in middle of char *************/
			if ((s = strstr(inline, "scale")) != NULL) {
				while (*s != ' ' && *s != '\0') s++;
				if (*s != '\0') s++;
				n = sscanf(s, "%lg %lg %d %d %lg", 
						   &xscale, &yscale, &xcenter, &ycenter, &slantcomp);
				if (n < 1)
					fprintf(errout, "ERROR: Don't understand scale %s %s",
							s, inline);
				if (n < 2) yscale = xscale;
/*					widthf = (int) (widthf * xscale + 0.5); */	/* ??? */
/*				printf("%s", inline); */	/* debugging */
				xscale = xscale * xscaleinit;		/* 1993/Jan/1 */
				yscale = yscale * yscaleinit;		/* 1993/Jan/1 */
				transformflag = 0;
				if (xscale != 1.0 || yscale != 1.0 || slant != 0.0)
					transformflag++;
			} 
/******************* experiment to reset scale in middle of char *************/
			moveto(fp_out); 
		}
		else if (strstr(inline, "lineto") != NULL ||
/*			strchr(inline, 'l') != NULL) { */
/*			strstr(inline, " l") != NULL) { */
				 ((s = strstr(inline, " l")) != NULL && s < sc)) { /* 97/Jul 1 */
			if (current == 0) {
				fprintf(errout, "\nERROR: No current point in char %d\n", 
					fchrs);
				current++;
				fatal++;
			}
			lineto(fp_out); 
		}
		else if (strstr(inline, "oval") != NULL ||
/*			strchr(inline, 'o') != NULL) { */
/*			strstr(inline, " o") != NULL) { */
				 ((s = strstr(inline, " o")) != NULL && s < sc)) { /* 97/Jul 1 */
			if (current == 0) {
				fprintf(errout, "\nERROR: No current point in char %d\n", 
					fchrs);
				current++;
				fatal++;
			}
			oval(fp_out); 
		}
		else if (strstr(inline, "circle") != NULL ||
/*			strchr(inline, 'o') != NULL) { */
/*			strstr(inline, " x") != NULL) { */
				 ((s = strstr(inline, " x")) != NULL && s < sc)) { /* 97/Jul 1 */
			if (current != 0) {
/*				if (allowmoveto) closepath(fp_out); */ /* ??? */
				fprintf(errout, "\nERROR: Moveto in path in char %d\n", 
					fchrs); /* ??? */
				fatal++;
			}
			circle (fp_out); 
			current = 0;			/* no current point */
		}
		else if (strstr(inline, "curveto") != NULL ||
/*			strchr(inline, 'c') != NULL) { */
/*			strstr(inline, " c") != NULL) { */
				 ((s = strstr(inline, " c")) != NULL && s < sc)) { /* 97/Jul 1 */
			if (current == 0) {
				fprintf(errout, "\nERROR: No current point in char %d\n", 
					fchrs);
				current++;
				fatal++;
			}
			curveto(fp_out); 
		}
		else if (strstr(inline, "subr") != NULL ||
/*			strchr(inline, 's') != NULL) *//* new, for hint replacement */
/*				 strstr(inline, " s") != NULL) */
				 ((s = strstr(inline, " s")) != NULL && s < sc)) 
			callsubr(fp_out); 
		else if (strstr(inline, "dot") != NULL ||
/*				 strchr(inline, 'd') != NULL) */ /* new, for dot section */
				 ((s = strchr(inline, 'd')) != NULL && s < sc))  /* 97/Jul 1 */
			subrtrick(fp_out, 0); 
		else {
			if (current == 0) {
				fprintf(errout, "\nERROR: No current point in char %d\n", 
					fchrs);
				current++;
				fatal++;
			}
			longcurveto(fp_in, fp_out);	
		}

		if (getline(fp_in, inline) == 0) break; /* EOF */
		while (*inline == '%' || *inline == '\n') 
			(void) getline(fp_in, inline);
	}
	finishchar(fp_out);
	nchrs++;
	if (ferror(fp_out) != 0) {	/* in case disk full */
		perror("Output error");
		giveup(2);
	}
}

void unsortedchars(FILE *fp_in, FILE *fp_out) {
	char *s; 
	int k, n;

	clm=0;
	if (getline(fp_in, inline) == 0) {
		fprintf(errout, "Unexpected EOF");
		giveup(1);
	}
	while (*inline == '%' || *inline == '\n') 
		(void) getline(fp_in, inline);
	if (traceflag!= 0) {
		printf("Entering character: %s", inline);
/*		(void) _getch(); */
	}
	if (rusticflag != 0) {
		while (strstr(inline, "end def") == NULL) {
			if (strstr(inline, "/.notdef") == NULL) {
				if (sscanf(inline, " /char%d {", &fchrs) != 1) {
					fprintf(errout, "Bad character definition: %s\n", inline);
					giveup(13);
				}
				if (verboseflag != 0) {
					printf(".");
					if ((nchrs + 1) % 64 == 0 && nchrs != 0) 
						putc('\n', stdout);
				}
				dochar(fp_in, fp_out, fchrs); 
/*				dochar(fp_in, fp_out, fchrs, 0, 0); */
			}
			(void) getline(fp_in, inline); /* ? */
			while (*inline == '%' || *inline == '\n') 
				(void) getline(fp_in, inline);
			if (traceflag != 0) {
				printf("Between: %s", inline);
/*				 (void) _getch(); */
			}
		}
	}
	else {						/* Projective solutions format */
/*		(void) getline(fp_in, inline);  */
		while (*inline == '%' || *inline == '\n') {
			if (getline(fp_in, inline) == 0) break; /* EOF ? */
		}
		while(strchr(inline, ']') != NULL) {
			if (verboseflag != 0) {
				printf(".");
				if ((nchrs + 1) % 64 == 0 && nchrs != 0) 
					putc('\n', stdout);
			}
			k = getline(fp_in, inline);
			if (k == 0) break; /* EOF */
/*			while (*inline == '%' || *inline == '\n') { */
			while (*inline == '%' || *inline == '\n' || *inline == ']')  {
				k = getline(fp_in, inline);
			}
			if (k == 0) break; /* EOF */
/*			if (sscanf(inline, "%d %d", &fchrs, &widthf) != 2) { */
			if (sscanf(inline, "%d %lg", &fchrs, &fwidthf) != 2) {
				fprintf(errout, 
					"Character code or width bad: %s (%d) ", inline, k); 
				if (getline(fp_in, inline) == 0) {
					fprintf(errout, "EOF!\n");
					break;
				}
				else {
					fprintf(errout, "Next line: %s", inline);
					giveup(11);
				}
			}

			xscale = 1.0; yscale = 1.0; 		/* new version */
			slant = 0.0;						/* 1996/Sep/2 */
			xcenter = ycenter = 0;
			slantcomp = 0.0;
			if ((s = strstr(inline, "scale")) != NULL) {
				while (*s != ' ' && *s != '\0') s++;
				if (*s != '\0') s++;
/*				n = sscanf(s, "scale %lg %lg %d %d",
				&xscale, &yscale, &xcenter, &ycenter); */
				n = sscanf(s, "%lg %lg %d %d %lg",
						   &xscale, &yscale, &xcenter, &ycenter, &slantcomp);
/*				if (slantcomp != 0.0) printf("SLANTCOMP %lg\n", slantcomp); */
				if (n < 1) {
					fprintf(errout, "ERROR: Don't understand scale %s %s",
							s, inline);
					fatal++;	/* ??? */
				}
				if (n < 2) yscale = xscale;
/*				widthf = (int) (widthf * xscale + 0.5);	*/		/* ??? */
			} 
			else if ((s = strstr(inline, "slant")) != NULL) {
				n = sscanf(s, "slant %lg", &slant);
				if (n < 1) {
					fprintf(errout, "ERROR: Don't understand %s %s",
							s, inline);
					fatal++;	/* ??? */
				}
			} 
			else s = inline;
			xscale = xscale * xscaleinit;		/* 1993/Jan/1 */
			yscale = yscale * yscaleinit;		/* 1993/Jan/1 */
			transformflag = 0;
			if (xscale != 1.0 || yscale != 1.0 || slant != 0.0)
				transformflag++;
/*			if (xscale != 1.0) widthf = (int) (widthf * xscale + 0.5);	 */
			if (xscale != 1.0) fwidthf = fwidthf * xscale;	
			if ((s = strchr(s, '%')) != NULL) {
				fputs(s, fp_out);			/* copy comment (charname) across */
				printf("COMMENT: %s", s);
			}
			else {
				nonamechar++;
				fprintf(fp_out, "%% a%d\n", fchrs);	/* 98/Apr/25 */
				if (verboseflag)
					fprintf(errout, "NO CHAR NAME: %s", inline);
			}
/*			else fputs(inline, stdout); */	/* debugging */
			/* how to retain accuracy in width ? */
/*			widths[fchrs] = scalenum(widthf); */
			fwidths[fchrs] = fscalenum (fwidthf);
			if (getline(fp_in, inline) == 0) {
				fprintf(errout, "Unexpected EOF");
				giveup(1);
			}
			while (*inline == '%' || *inline == '\n') 
				k = getline(fp_in, inline);
			if (k == 0) break; /* ??? */
			dochar(fp_in, fp_out, fchrs); 
		}
/*		if (strlen(inline) > 0) {
			fprintf(errout, "Character data does not start with ']': %s", 
				inline);
			giveup(17);
		} */
		if (nonamechar > 0)
			fprintf(errout, "\nWARNING: %d characters without names\n",
					nonamechar);
	}
}

int sortedcharssub(FILE *fp_in, FILE *fp_out, int chrs) {
	int firstpass=1, nxtchrs, k, n;
	char *s, *t;

/*	printf("Trying to find char %d\n", chrs); */
	firstpass=1; nxtchrs = fontchrs;
	for(;;) { /* catch the end: */
		if ((rusticflag != 0) && strstr(inline, "end def") != NULL ||
			(rusticflag == 0) && strlen(inline) == 0) { /* EOF ? */
			if (firstpass != 0) {
				firstpass = 0;
				fseek(fp_in, fstart, SEEK_SET);
				if (verboseflag != 0) printf("<=");
				(void) getline(fp_in, inline);
				while (*inline == '%' || *inline == '\n') 
					(void) getline(fp_in, inline); 
/*				printf("First line %s", inline); (void) _getch(); */
			}
			else if (complainflag != 0 || verboseflag != 0) {
				fprintf(errout, "ERROR: Unable to find character %d\n", chrs); 
				fatal++;
				fseek(fp_in, fstart, SEEK_SET);
				chrs = nxtchrs - 1;					/* ??? */
				break;
			}
		}
/* not end of file: */
		if (rusticflag != 0) { /* deal with rustic format */
			if (strstr(inline, "/.notdef") != NULL) {
/*			printf("Found: %s", inline); */
				(void) getline(fp_in, inline); /* ? */
				continue;
			}
			if (sscanf(inline, " /char%d {", &fchrs) != 1) {
				fprintf(errout, "Bad character definition: %s\n", inline);
				giveup(13);
			}
			else if (fchrs == chrs) {
/*			printf("Found character %d\n", chrs); */
				if (verboseflag != 0) {
					printf(".");
					if ((chrs + 1) % 64 == 0 && chrs != 0) 
						putc('\n', stdout);
				}
/*				if (traceflag != 0) printf("%s", inline);	 */
				dochar(fp_in, fp_out, fchrs); 
/*				dochar(fp_in, fp_out, fchrs, 0, 0); */
				(void) getline(fp_in, inline); /* ? */
				break;
			}
			else flushchar(fp_in);	/* ignore this character */
			(void) getline(fp_in, inline); /* ? */
/*			while (*inline == '%' || *inline == '\n')  */
			while (*inline == '%' || *inline == '\n' || *inline == ']') 
				(void) getline(fp_in, inline); 
/*		printf("Between: %s", inline); (void) _getch(); */
		}	
		else { /* Projective Solutions format */
			while (charstart[chrs] < 0 && chrs < MAXCHRS) chrs++; 	
			if (chrs >= MAXCHRS) break;					/* NEW */
			fseek(fp_in, charstart[chrs], SEEK_SET);	/* NEW */
			k = getline(fp_in, inline);
 			if (k == 0) {
				fprintf(errout, "Unexpected EOF");
				giveup(1);
			}
/*			while (*inline == '%' || *inline == '\n') { */
			while (*inline == '%' || *inline == '\n' || *inline == ']')  {
				k = getline(fp_in, inline);
			}
			if (k == 0) {
				fprintf(errout, "Unexpected EOF");
				giveup(1);
			}
/*			if (sscanf(inline, "%d %d", &fchrs, &widthf) != 2) { */
			if (sscanf(inline, "%d %lg", &fchrs, &fwidthf) != 2) {
				fprintf(errout, 
					"Character code or width bad: %s (%d) ", inline, k); 
				if (getline(fp_in, inline) == 0) {
					fprintf(errout, "EOF!\n");
					break;
				}
				else {
					fprintf(errout, "Next line: %s", inline);
					giveup(11);
				}
			}
/*			printf("%d ", fchrs); */
			if (fchrs == chrs) {
/*			printf("Found character %d\n", chrs); */
				if (verboseflag != 0) {
					printf(".");
					if ((chrs + 1) % 64 == 0 && chrs != 0) printf("\n");
				}

				xscale = 1.0; yscale = 1.0;	/* new version */
				xcenter = ycenter = 0;
				slant = 0.0;
				slantcomp = 0.0;
				if ((s = strstr(inline, "scale")) != NULL) {
					while (*s != ' ' && *s != '\0') s++;
					if (*s != '\0') s++;
					n = sscanf(s, "%lg %lg %d %d %lg", 
							   &xscale, &yscale, &xcenter, &ycenter, &slantcomp);
/*					if (slantcomp != 0.0) printf("SLANTCOMP %lg\n", slantcomp); */
/*					n = sscanf(s, "scale %lg %lg %d %d", 
							   &xscale, &yscale, &xcenter, &ycenter); */
					if (n < 1)
						fprintf(errout, "ERROR: Don't understand scale %s %s",
								s,  inline);
					if (n < 2) yscale = xscale;
/*					widthf = (int) (widthf * xscale + 0.5); */	/* ??? */
				} 
				else if ((s = strstr(inline, "slant")) != NULL) {
					n = sscanf(s, "slant %lg", &slant);
				} 
				else s = inline;
				xscale = xscale * xscaleinit;		/* 1993/Jan/1 */
				yscale = yscale * yscaleinit;		/* 1993/Jan/1 */
				transformflag = 0;
				if (xscale != 1.0 || yscale != 1.0 || slant != 0.0)
					transformflag++;
/*				if (xscale != 1.0) widthf = (int) (widthf * xscale + 0.5);	 */
				if (xscale != 1.0) fwidthf = fwidthf * xscale;

/*				if ((s = strchr(s, '%')) != NULL) { */
				if ((s = strchr(inline, '%')) != NULL) {	/* ??? 97/Aug/10 */
					if ((t = strchr(s+1, '%')) != NULL) {
						*t-- = '\0';
						while (t > inline && *t <= ' ') *t-- = '\0';
						strcat(s, "\n");
					}
					fputs(s, fp_out);	/* copy comment across (charname) */
					if (traceflag) printf("COMMENT: %s", s); 
				}
				else {
					nonamechar++;
					fprintf(fp_out, "%% a%d\n", fchrs);	/* 98/Apr/25 */
					if (verboseflag)
						fprintf(errout, "NO CHAR NAME: %s", inline);
				}
/*				else fputs(inline, stdout);	 */		/* debugging */
				/* how to retain accuray in width ? */
/*				widths[fchrs] = fscalenum(widthf); */
				fwidths[fchrs] = fscalenum(fwidthf);
				if (getline(fp_in, inline) == 0) {
					fprintf(errout, "Unexpected EOF");
					giveup(1);
				}
				while (*inline == '%' || *inline == '\n') (void) getline(fp_in, inline);
				dochar(fp_in, fp_out, fchrs);
/*				if (getline(fp_in, inline) == 0) {
					fprintf(errout, "Unexpected EOF"); giveup(1);
				} */
				while (*inline == '%' || *inline == '\n') 
					(void) getline(fp_in, inline);
				break;
			}
			else {
				flushchar(fp_in);	/* ignore this character */
				if (fchrs > chrs && fchrs < nxtchrs) nxtchrs = fchrs;
			}
/*			(void) getline(fp_in, inline); */
		}
	}				
	return chrs;
}

void sortedchars(FILE *fp_in, FILE *fp_out) {
/*	int firstpass, nxtchrs, k; */
/*	char *s;  */

	clm=0;
	nonamechar=0;
	if (getline(fp_in, inline) == 0) {
		fprintf(errout, "Unexpected EOF");
		giveup(1);
	}
	while (*inline == '%' || *inline == '\n') {
		(void) getline(fp_in, inline);
	}
	if (traceflag!= 0) {
		printf("Entering character: %s", inline);
/*		(void) _getch(); */
	}
/*	if (putzerolast != 0) {
		for (chrs=1; chrs < fontchrs; chrs++) 
			chrs = sortedcharssub(fp_in, fp_out, chrs);
		chrs = sortedcharssub(fp_in, fp_out, 0); */
/*		need to rearrange hints for such a font also ... */
	for (chrs=0; chrs < fontchrs; chrs++) {
			chrs = sortedcharssub(fp_in, fp_out, chrs);
	}
	if (nonamechar > 0)
		fprintf(errout, "\nWARNING: %d characters without names\n",
				nonamechar);
}

void doallchars(FILE *fp_in, FILE *fp_out) {
	nchrs = 0;
	if (orderflag == 0) unsortedchars(fp_in, fp_out);
	else sortedchars(fp_in, fp_out);
	if (verboseflag != 0) putc('\n', stdout);
	if (nchrs < fontchrs)
		fprintf(stdout, "%d characters converted\n", nchrs); /* \n ? */
}

void dotrailer(FILE *fp_out) {
	if (traceflag != 0) {
		printf("Reached trailer\n");
/*		(void) _getch(); */
	}
	if (remapflag != 0) fprintf(fp_out, "%s", remapping);
	fprintf(fp_out, "\npop\n");
	xmin = scalenum(xmin);
	ymin = scalenum(ymin); /* ? */
	xmax = scalenum(xmax);
	ymax = scalenum(ymax); /* ? */
/*	fprintf(fp_out, "\n%% /FontBBox {%d %d %d %d} def ", 
			xmin, ymin, xmax, ymax); */ /* ??? */
	fprintf(fp_out, "\n%% /FontBBox [%d %d %d %d] def ", 
			xmin, ymin, xmax, ymax);
	if (xmin != xll || xmax != xur || ymin != yll || ymax != yur)
		fprintf(fp_out, "%% ACTUAL (ignoring composites)");
	putc('\n', fp_out);
/* possibly add remapping of character codes here ? OK */
}

void dofile(FILE *fp_in, FILE * fp_out) {
	doheader(fp_in, fp_out);
	fstart = ftell(fp_in);
	doallchars(fp_in, fp_out);
	dotrailer(fp_out);
/*	if (verboseflag != 0) printf("\n"); */
/*	if (verboseflag != 0) */
	if (xmin == INFINITY || xmax == -INFINITY ||
		ymin == INFINITY || ymax == -INFINITY )
		printf("WARNING: No characters seen!\n");
	else if (xmin != xll || xmax != xur || ymin != yll || ymax != yur)
		printf("WARNING: Actual FontBBox %d %d %d %d  (%s)\n", 
			xmin, ymin, xmax, ymax, fontname);
}

void extension(char *fname, char *txt) { /* supply extension if none */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, txt);
	}
}

void forceexten(char *fname, char *txt) { /* change extension if present */
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, txt);
	}
	else strcpy(s+1, txt);		/* give it default extension */
}

void decodeflag (int c) {
/*	printf ("FLAG: %c%n", c); */
	switch(c) {
		case 'v': verboseflag = 1; break;
		case '?': verboseflag = 1; break;
		case 't': traceflag = 1; break;
/*		case 't': rusticflag = 1; break; */
		case 'a': adjustknots = 0; break;
		case 'k': texfont = 1; break;
		case 'q': wantuniqueid = 1; break;
		case 'u': uppercaseflag = 1; break;
/*		case 's': correctscale = 1; break; */
		case 'b': correctscale = 1; break;
/*		case 'z': putzerolast = 1; break;	*/
		case 'o': orderflag = 0; break;
		case 'r': remapflag = 1; break;
		case 'n': adobescaling = 0; break;  
		case 'N': adobescaling = 2; break;   /* 1994/July/10 */
		case 'P': pctexbug =1; break;		 /* 1994/July/10 */
		case 'c': charsflag = 1; break;
		case 'p': pathflag =1; break;
		case 'm': metaflag = 1; break;
		case 'x': xscaleflag = 1; break; 
		case 'y': yscaleflag = 1; break; 
		case 's': skewflag = 1; break;
		case 'e': extenflag = 1; break;
		case 'w': complainopen = 1; break;
		case 'z': checknear = 1; break;
		case 'Q': quietflag = 1; break;
/*		case 'x': versionflag = 1; break; */
		default: {
				fprintf(errout, "Invalid command line flag '%c'", c);
				giveup(7);
		}
	}
}

/* \t\t[-m <dpi>] [-c <encoding-size>] [-x <version>] \n\ */

void showusage(char *s) {
	fprintf (errout, "Correct usage is:\n\n");
    fprintf (errout, 
		"%s [-{v}{k}{u}{q}{r}{a}{o}{n}{b}{x}{y}]\n\
\t\t[-s <skew>] [-m <dpi>] [-x <version>] \n\
\t\t<file-1> <file-2> ...\n", s); 
	fprintf (errout, "\
\tv: verbose mode\n\
\tk: TeX font file (also implies u, q, r)\n\
\t\tu: make fontname in output file upper case\n\
\t\tq: insert UniqueID\n\
\t\tr: insert remapping of character codes at the end\n\
\ta: do not adjust knots to avoid coincidence after rounding\n\
\to: do not order CharDefs in output\n\
\tn: input not in Adobe standard coordinates\n\
\tb: undo BSR scaling (not usually needed)\n\
");
/* \tz: put CharString for character zero at end\n\ */
	fprintf (errout,
	"\tm: next argument is MetaFont DPI used (default %lg)\n", dpi);
	fprintf (errout,
	"\ts: next argument is skewing factor (default %lg)\n", skew);
/*	fprintf (errout,
"\tc: next argument is size of encoding vector (default %d)\n",	fontchrs); */
/*	fprintf (errout, "\tx: next argument is version of font\n"); */
	fprintf (errout, "\tx: global scale in x\n");
	fprintf (errout, "\ty: global scale in y\n");
	fprintf (errout, "\tw: complain if sub-paths are not closed\n");
	fprintf (errout, "\tz: complain if side-slip off vertical/horizontal\n");
/*	fprintf (errout,
	"\te: next argument is extension to use on output file (default '%s')\n", 
		ext); */
/*	fprintf (errout,
	"\tp: next argument is path to use on output file");
	if (strcmp(path,"") != 0) fprintf(errout, " (default is '%s')", path);
	fprintf(errout, "\n"); */
	exit(1);
}

/* \t\t[-e <ext>] [-p path] <file-1> <file-2> ...\n", s); */

/* \tn: rescale input (input is scaled for TeX font matrix)\n\ */
/* \tt: trace mode\n\ */
/* \tt: `rustic' format (not Projective Solutions format)\n */
/* correct scale (use 72 instead of 72.27) */

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = stripname(filename);
/*	if ((s = strrchr(filename, '\\')) != NULL) s++;
	else if ((s = strrchr(filename, '/')) != NULL) s++;
	else if ((s = strrchr(filename, ':')) != NULL) s++;
	else s = filename; */
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
/*	if (traceflag != 0) printf("n = %d m = %d ", n, m); */
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
/*	if (traceflag != 0) printf("Now trying file name: %s\n", s); */
/*	return 0; */
}

int main(int argc, char *argv[]) {        /* main program */
/* Command line usage: */
    FILE *fp_in, *fp_out;
    char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX]; 
	char *s;
	unsigned i;
	int c, m, firstarg=1;

/*	if (setvbuf(stdout, NULL, _IOLBF, 512))	
		printf("Unable to line buffer %s\n", "stdout"); */
/*	if (setvbuf(errout, NULL, _IOLBF, 512))	
		printf("Unable to line buffer %s\n", "errout"); */
/*	setvbuf(errout, NULL, _IONBF, 0); */		/* 97/Sep/13 */

	if (argc < 2) showusage(argv[0]);

/*	while (argv[firstarg][0] == '-') */ /* check for command flags */
	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else decodeflag(c);
		}
		firstarg++;
		if (charsflag != 0) {
			sscanf(argv[firstarg], "%d", &fontchrs); firstarg++; charsflag = 0;
		}
		if (metaflag != 0) {
			sscanf(argv[firstarg], "%lg", &dpi); firstarg++; metaflag = 0;
		}		
		if (xscaleflag != 0) {
			sscanf(argv[firstarg], "%lg", &xscaleinit); firstarg++; xscaleflag = 0;
		}		
		if (yscaleflag != 0) {
			sscanf(argv[firstarg], "%lg", &yscaleinit); firstarg++; yscaleflag = 0;
		}		
		if (skewflag != 0) {
			sscanf(argv[firstarg], "%lg", &skew); firstarg++; skewflag = 0;
		}		
		if (extenflag != 0) {
			ext = argv[firstarg]; firstarg++; extenflag = 0;
		}
		if (versionflag != 0) {
			ver = argv[firstarg]; firstarg++; versionflag = 0;
		}
		if (pathflag != 0) {
			path = argv[firstarg]; firstarg++; pathflag = 0;
		}
	}

	if (texfont != 0) {
		remapflag = 1; wantuniqueid = 1; uppercaseflag = 1; 
/*		adobescaling = 0;			*/
	}

	printf("CONVERT font conversion program version 1.3\n");
/*	if (verboseflag != 0) putc('\n', stdout); */

	for (m = firstarg; m < argc; m++) { /* do each file in command line */

		fatal = 0;

		strcpy(fn_in, argv[m]);
		extension(fn_in, "raw");
/* use filename to guess fontname - in rustic format this is later ignored */
/*		if (rusticflag == 0) { */ 
			if ((s=strrchr(fn_in, '\\')) != NULL) s++;
			else if ((s=strrchr(fn_in, ':')) != NULL) s++;
			else s = fn_in;
			strcpy(fontname, s);
			if ((s = strchr(fontname, '.')) == NULL) {
				fprintf(errout, "Don't understand fontname: %s", fontname);
				giveup(13);
			}
			*s = '\0';
			if (uppercaseflag == 0) lowercase(fontname, fontname);
/*		} */

		extension(fn_in, "raw");
		if ((fp_in = fopen(fn_in, "rb")) == NULL) {
			forceexten(fn_in, "out");
			if ((fp_in = fopen(fn_in, "rb")) == NULL) {
				underscore(fn_in);
				if ((fp_in = fopen(fn_in, "rb")) == NULL) {
					fprintf(errout, "\n");
					perror(fn_in);
					giveup(2);
				}
			}
		}

		if (strcmp(path, "") != 0) {
			strcpy(fn_out, path);
			strcat(fn_out, "\\");
			if ((s=strrchr(fn_in, '\\')) != NULL) s++;
			else if ((s=strrchr(fn_in, ':')) != NULL) s++;
			else s = fn_in;
			strcat(fn_out, s);  	/* copy input file name minus path */
		}
		else {
			if ((s=strrchr(fn_in, '\\')) != NULL) s++;
			else if ((s=strrchr(fn_in, ':')) != NULL) s++;
			else s = fn_in;
			strcpy(fn_out, s);  	/* copy input file name minus path */
		}
		forceexten(fn_out, ext);
		if (verboseflag != 0) putc('\n', stdout);
		if (verboseflag != 0) printf("Output file: %s\n", fn_out);

/*		extension(fn_in, "raw");
		if ((fp_in = fopen(fn_in, "rb")) == NULL) {
			forceexten(fn_in, "out");
			if ((fp_in = fopen(fn_in, "rb")) == NULL) {
				underscore(fn_in);
				if ((fp_in = fopen(fn_in, "rb")) == NULL) {
					fprintf(errout, "\n");
					perror(fn_in);
					giveup(2);
				}
			}
		} */

		if ((fp_out = fopen(fn_out, "w")) == NULL) { /* "wb" ? */
			fprintf(errout, "\n");
			perror(fn_out);
			giveup(2);
		}

#ifdef DEBUGFLUSH
		setbuf(fp_out, NULL); /* serious stuff */
#endif

		printf("Processing font file %s\n", fn_in);

		dofile(fp_in, fp_out);

		fclose(fp_in);
		if (ferror(fp_out) != 0) {
			fprintf(errout, "Output error");
			perror(fn_out);
			giveup(2);
		}
		else fclose(fp_out);

		if (copydate == 0) continue;
		if (getinfo(fn_in, traceflag) < 0) {
			exit(1);
		}
//		timebuf.actime = statbuf.st_atime;
		timebuf.actime = statbuf.st_atime;
//		timebuf.modtime = statbuf.st_atime;
		timebuf.modtime = statbuf.st_mtime;
		if (_utime(fn_out, &timebuf) != 0) {
			fprintf(errout, "Unable to modify date/time\n");
			perror(fn_out);
/*			exit(3); */
		}

/*		see if it worked */
		if (getinfo(fn_out, traceflag) < 0) exit(1);
			
		if (fatal != 0) break;
	}
	if (fatal != 0) exit(1);
	if (argc > firstarg + 1) 
		printf("Processed %d font files\n", argc - firstarg);
	return 0;
}

/* produces character outputs ordered or - */ 

/* produces character outputs in same order as they appear in input */ 

/* uses ftell and fseek to do them in order */

/* allow scaling on input OK */

/* provide for both `rustic' as well as Projective Solutions format */

/* accumulate bounding box information - track xmin xmax ymin ymax OK */

/* add character remapping at the end - OK */

/* note interchange of yll and xur in Projective Solutions format OK */

/* permit change of MAXCHRS - make variable ? */
/* permit comments and blank lines in input */
/* allow suppress UniqueID on weird fonts OK */
/* allow suppress scaling on some fonts OK */

/* suppress coincident knot messages ? */

/* add initial pass through file to find where characters are */
/* if orderflag is on and rusticflag is off */

/* don't give up so easy when doing them in order and not finding one */

/* presently only allow offset in Projective Solutions format */

/* offsets used to be given with character code and width */
/* offsets are now given with each "m" or "moveto" */

/* on PC, files should have CR and NL to make sure ftell and fseek work */

/* note interchange of yll and xur - problem with Proj Sol outlines ! */

/* rustic format assumes input already in Adobe standard coordinates */

/* rustic format also appears to be based on wrong scale (72 pt/inch) */

/* take care of rescaling here if needed - not in FONTONE & SHOWCHAR */

/* right now adobescaling is always 1 - is this a bug ???? */

/* this should complain if subpath is not closed ? */

/* should not copy across worthless lineto's and curveto's */

/* WATCH OUT for bug/feature of ftell and fseek: */
/* if file contains C-J instead of C-R C-J sequences: */
/* in ftell it computes position as if C-J's where C-R and C-J */
/* in fseek it does not */ 
/* fix by opening with "rb" instead of "r" */

/* If a file is bad, it should flush that file not the whole job... */

/* Presently does not complain if same character code appears twice! */

/* typical usage   convert -vks -x 00.95 c:\cmbkph\*.mas   */

/* typical usage   convert -vkn -x 00.95 c:\bsr\*.gf    */

/* typical usage   convert -vk -x 1.0 c:\bsr\*.out    */

/* putzerolast != 0 => put character zero at end to deal with ATM bug NOT */

/* scaling is after offset */

/* notice that a path /character is empty and flush it ??? */

/* For PC TeX TrueType use N and P on command line */

/* 45 500 % hyphen % scale 0.5 0.4 0 0 0.2 */
/* chr wx % name % scale xscale yscale xcenter ycenter slant */
