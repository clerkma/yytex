/* Copyright 1995, 1996 Y&Y, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXBUFFER 1024		/* up to 8192 columns --- 27.3" at 300dpi */

#define MAXLINE 512			/* PS input line */

#define MAXFILENAME 256		/* more than enough on FAT file system */

/* Scan PS file for monochrome images and */
/* extract in TIFF PackBits compressed format. 1996/Jan/20 */
/* Should work for PS files produced by HPTAG system as well as DVIPSONE. */
/* Output files are named sequentially. */
/* Unless a log file produced by DVIPSONE with Can't find messages is used */
/* On screen output shows which page each file comes from. */
/* This information is also deposited in ARTIST tag in TIFF file */

unsigned char row[MAXBUFFER];

char line[MAXLINE];

int verboseflag=1;
int dotsflag=0;
int slowdotsflag=1;
int traceflag=0;
int debugflag=0;

int detailflag=0;

int err;

int pageno;

long longrun, longnonrun;

int nbytes, ncolumns, nrows;

char pagename[128]="";

int maxerror=256;			/* bomb out if more than this many errors */

/***************************************************************************/

unsigned int leastout=1;	/* non-zero => PC style, zero => Mac style */

unsigned int version=42;	/* TIFF version */

int binaryflag=1;			/* convert to monochrome if set */

int stretchflag=0;			/* stretch greylevels using mingrey & maxgrey */

int flipflag=0;				/* orientation */

int whimpflag=0;			/* try and accomodate Old Quark on Mac */

/* int photointer=1; */		/* Normal Gray */
int photointer=0;			/* Inverted Gray */

int resolution=1;			/* include xresolution and yresolution fields */

unsigned int ifdcount;		/* how many items in IFD */

unsigned int tagcount;		/* haw many actually written */

unsigned long ifdposition;	/* position of IFD */

unsigned long xrespos, yrespos, stripoffsets;

char comment[64]="";		/* comment for TIFF file */

int iwidth, iheight;		/* ncolumns, nrows */
unsigned int mingrey=0, maxgrey=1;
int bitspersampleout=1;		/* restricted to bilevel images */

int numscale = 300;			/* XRES = YRES = numscale / denscale */
int denscale = 1;			/* XRES = YRES = resscale / denscale */

/***************************************************************************/

/* new code to read dry run log file to extract names of files */

/* #define MAXIMAGES 1024 */

#define MAXIMAGES 2048

char *logfile="";

int logfileflag=0;

int useimagenames=0;

int imageindex;

char *imagenames[MAXIMAGES];	/* names of images called for */

int imagecount0[MAXIMAGES];		/* pages on which images occur - long ? */

int imagecount1[MAXIMAGES];		/* pages on which images occur - long ? */

int imagecount2[MAXIMAGES];		/* pages on which images occur - long ? */

int doubleflag=0;				/* if log file has repeat complaints */

void readpagerec (FILE *input) {
	char *s=line;
	int c;
	while ((c = getc(input)) != EOF) {
		if (c == ']') break;
		*s++ = (char) c;
		if ((s - line) >= MAXLINE-1) break;
	}
	*s++ = '\0';
}

/* [11 1 3 Ignoring revset Can't find `serial.hpg' Can't find `serial.hpg']  */

void processpagerec (char *line) {
	int n, a, b, c, m;
	char *s, *t, *u;
	int firstflag=1;

/*	n = sscanf(line, "%d %d %d%n", &a, &b, &c, &m); */
	n = sscanf(line, "%d%n%d%n%d%n", &a, &m, &b, &m, &c, &m);

	if (n == 0) printf("No page number info in record: %s\n", line);
/*	a is counter[0]	 b is counter[1] c is counter[2] ... */
	s = line+m;
	while (strstr(s, "Can't find") != NULL) {
		s = s + strlen("Can't find");
		if ((t = strchr(s, '`')) != NULL) {		/* isolate file name */
			t++;
			if ((u = strchr(t, '\'')) != NULL) {
/* file name is now from t to u-1 */				
				if (firstflag) {
/* or compare u with imagenames[imageindex] ? */
					*u = '\0';
					imagenames[imageindex] = _strdup(t);
					*u = '\'';
					if (n > 0) imagecount0[imageindex] = a;
					else imagecount0[imageindex] = 0;
					if (n > 1) imagecount1[imageindex] = b;
					else imagecount1[imageindex] = 0;
					if (n > 2) imagecount2[imageindex] = c;
					else imagecount2[imageindex] = 0;
					imageindex++;
					if (imageindex >= MAXIMAGES) {
						fprintf(stderr, "Too many images\n");
						return;
					}
				}
				s = u+1;
				if (doubleflag) firstflag = !firstflag;
			}
			else printf("Missing quoteright: %s\n", line);
		}
		else printf("Missing quoteleft: %s\n", line);
	}
}

void showimagetable (void) {
	int k;
/*	printf("Found references to %d images:\n", imageindex); */
	for (k = 0; k < imageindex; k++) {
		printf("%d\t", k);
		printf("%d ", imagecount0[k]);
		if (imagecount1[k] >= 0) printf("%d ", imagecount1[k]);
		if (imagecount2[k] >= 0) printf("%d ", imagecount2[k]);
		printf("\t%s\n", imagenames[k]);
	}
}

int checkduplicates (void) {
	int k;
	if ((imageindex % 2) != 0) return 0;	/* total not divisible by 2 */
	for (k = 0; k < imageindex; k += 2) {
		if (strcmp(imagenames[k+k], imagenames[k+k+1]) != 0) return 0;
		if (imagecount0[k+k] != imagecount0[k+k+1]) return 0;
		if (imagecount1[k+k] != imagecount1[k+k+1]) return 0;
		if (imagecount2[k+k] != imagecount2[k+k+1]) return 0;
	}
	return 1;								/* they all came in pairs */
}

void removeduplicates (void) {
	int k;
	for (k = 1; k < imageindex/2; k++) {
		if (imagenames[k] != NULL) free(imagenames[k]);
		imagenames[k] = imagenames[k+k];
		imagenames[k+k] = NULL;
		imagecount0[k] = imagecount0[k+k];
		imagecount0[k+k] = -1;
		imagecount1[k] = imagecount1[k+k];
		imagecount1[k+k] = -1;
		imagecount2[k] = imagecount2[k+k];
		imagecount2[k+k] = -1;
	}
	imageindex = imageindex / 2;
}

void readlogfile (FILE *input) {						/* 1995/Mar/2 */
	int c;
/*	for (k = 0; k < MAXIMAGES; k++) imagenames[k] = NULL; */
	imageindex=0;
	while ((c = getc(input)) != EOF) {
		if (c == '[') {
			readpagerec(input);
/*			if (traceflag) printf("LINE: %s\n", line); */
			processpagerec(line);
		}
	}
	if (checkduplicates()) {
		printf (
	"WARNING: All images mentioned twice, maybe use -d command line flag\n");
		doubleflag=1;
		removeduplicates();
	}
	if (verboseflag) printf("Found references to %d images:\n", imageindex);
	if (traceflag) showimagetable();
	useimagenames = 1;				/* indicate image names available */
}

/***************************************************************************/

#define TYPE_BYTE 1
#define TYPE_ASCII 2
#define TYPE_SHORT 3
#define TYPE_LONG 4
#define TYPE_RATIO 5

#define NEWSUBFILETYPE 254
#define SUBFILETYPE 255
#define IMAGEWIDTH 256
#define IMAGELENGTH 257
#define BITSPERSAMPLE 258
#define COMPRESSION 259
#define PHOTOMETRICINTERPRETATION 262
#define FILLORDER 266
#define DOCUMENTNAME 269
#define STRIPOFFSETS 273
#define ORIENTATION 274
#define SAMPLESPERPIXEL 277
#define ROWSPERSTRIP 278 
#define STRIPBYTECOUNTS 279
#define MINSAMPLEVALUE 280
#define MAXSAMPLEVALUE 281
#define XRESOLUTION 282
#define YRESOLUTION 283
#define PLANARCONFIGURATION 284
#define ARTIST 315

#define LZW 5
#define PACK_BITS 32773

/***************************************************************************/

void uwritetwo(unsigned long offset, FILE *output) {
	if (leastout != 0) {
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;
	}
	else {
		putc((int) ((offset >> 8) & 255), output); offset = offset << 8;
		putc((int) ((offset >> 8) & 255), output); offset = offset << 8;
	}
}

void uwritefour(unsigned long offset, FILE *output) {
	if (leastout != 0) {
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;
		putc((int) (offset & 255), output); offset = offset >> 8;	
	}
	else {
		putc((int) ((offset >> 24) & 255), output); offset = offset << 8;
		putc((int) ((offset >> 24) & 255), output); offset = offset << 8;
		putc((int) ((offset >> 24) & 255), output); offset = offset << 8;
		putc((int) ((offset >> 24) & 255), output); offset = offset << 8;	
	}
}

void writetagfield(unsigned int tag, unsigned int type, 
	unsigned long length, unsigned long offset, FILE *output) {

	uwritetwo(tag, output);
	uwritetwo(type, output);
	uwritefour(length, output);
	if (length ==1) {
		if (type == TYPE_LONG) uwritefour(offset, output);
		else if (type == TYPE_SHORT) {
			uwritetwo(offset, output);
			uwritetwo(0, output);
		}
		else if (type == TYPE_BYTE) {
			putc((int) offset, output);
			putc(0, output); putc(0, output); putc(0, output);
		}
		else  uwritefour(offset, output);
	}
	else uwritefour(offset, output);
	tagcount++;
}

/***************************************************************************/

void writetiffheader(FILE *output) {	/*  create header for TIFF file */
	char *s;
	int c;
	
	if (leastout != 0) {
		putc('I', output); 	putc('I', output);		/* leastout = 1 */
	}
	else {
		putc('M', output); 	putc('M', output);		/* leastout = 0 */
	}
	uwritetwo(version, output);		/* version number */
	if (whimpflag != 0) ifdposition = 2 + 2 + 4; 	/* right after header */
	else ifdposition = 2 + 2 + 4 + strlen(comment) + 1;
	uwritefour(ifdposition, output);
	if (whimpflag == 0) {
		s = comment;
		while((c = *s++) != '\0') putc(c, output);
		putc('\0', output);
	}

	ifdcount = 13;							/* number of tag fields */
/*	if (whimpflag != 0) ifdcount += 4 - 6; */	/* extra for whimpy format */
	if (whimpflag != 0) ifdcount -= 6 - 4; 	/* extra for whimpy format */
	if (resolution != 0) {
		ifdcount += 2;		/* extra for XRES & YRES tag fields */
		xrespos = ifdposition + 2 + ifdcount * 12 + 4;
		yrespos = ifdposition + 2 + ifdcount * 12 + 4 + 8;
	}
	stripoffsets =	ifdposition + 2 + ifdcount * 12 + 4;
	if (resolution != 0) stripoffsets += 2 * 8;	 /* space for two RATIOs */

	uwritetwo(ifdcount, output);
	tagcount = 0;
/*	writetagfield(tag, type, length, offset, output); */
	if (whimpflag != 0)
		writetagfield(SUBFILETYPE, TYPE_SHORT, 1, 1, output);		/* REQ */
	else 
		writetagfield(NEWSUBFILETYPE, TYPE_SHORT, 1, 0, output);	/* REQ */
	writetagfield(IMAGEWIDTH, TYPE_SHORT, 1, iwidth, output);		/* REQ */
	writetagfield(IMAGELENGTH, TYPE_SHORT, 1, iheight, output);		/* REQ */
	writetagfield(BITSPERSAMPLE, TYPE_SHORT, 1, bitspersampleout, output);
	if (whimpflag == 0)
		writetagfield(COMPRESSION, TYPE_SHORT, 1, PACK_BITS, output);
	writetagfield(PHOTOMETRICINTERPRETATION, TYPE_SHORT, 1, photointer, output);
	if (whimpflag != 0)
		writetagfield(FILLORDER, TYPE_SHORT, 1, 1, output);
	writetagfield(STRIPOFFSETS, TYPE_LONG, 1, stripoffsets, output); /* REQ */
	if (flipflag != 0) writetagfield(ORIENTATION, TYPE_SHORT, 1, 4, output);
	else writetagfield(ORIENTATION, TYPE_SHORT, 1, 1, output);
	writetagfield(SAMPLESPERPIXEL, TYPE_SHORT, 1, 1, output); 
	if (whimpflag == 0)
		writetagfield(ROWSPERSTRIP, TYPE_SHORT, 1, iheight, output);
	if (whimpflag != 0)
		writetagfield(STRIPBYTECOUNTS, TYPE_LONG, 1, 
			(long) iheight * ((iwidth * bitspersampleout + 7) / 8), output);
	if (binaryflag != 0) {				/* ??? */
			writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, 0, output);
			writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, 1, output);
	}
	else if (stretchflag != 0 && bitspersampleout == 4) {
			writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, 0, output);
			writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, 15, output);
	}
	else if (stretchflag != 0 && bitspersampleout == 8) {
			writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, 0, output);
			writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, 255, output);
	}
	else if (whimpflag == 0) {
		writetagfield(MINSAMPLEVALUE, TYPE_SHORT, 1, mingrey, output);
		writetagfield(MAXSAMPLEVALUE, TYPE_SHORT, 1, maxgrey, output);
	}
/*	XRESOLUTION & YRESOLUTION */
	if (resolution != 0) {
		writetagfield(XRESOLUTION, TYPE_RATIO, 1, xrespos, output);	
		writetagfield(YRESOLUTION, TYPE_RATIO, 1, yrespos, output);	
	}
	if (whimpflag != 0)
		writetagfield(PLANARCONFIGURATION, TYPE_SHORT, 1, 1, output);		
	if (whimpflag == 0)
		writetagfield(ARTIST, TYPE_ASCII, strlen(comment), 8, output);
	uwritefour(0, output);		/* indicate end of IFD fields */

	if (tagcount != ifdcount)
		printf("ERROR: IFDCOUNT (%u) <> TAGCOUNT (%u)\n",
			ifdcount, tagcount);

	if (resolution != 0) {
		uwritefour(numscale, output); uwritefour(denscale, output);
		uwritefour(numscale, output); uwritefour(denscale, output);
	}

}

/***************************************************************************/

int getnexthex(FILE *input) {
	int c;

	c = getc(input);
	for (;;) {
		if (debugflag) printf("%c (%d) ", c, c); 
		if (c == EOF) return EOF;
		if (c >= '0' && c <= '9') return (c - '0');
		if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
		if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
		if (c > ' ') {
			printf("%d? ", c);
			if (err++ > maxerror && maxerror > 0) exit(1);
		}
		c = getc(input);
	}
}

int readnextrow(FILE *input, int n) {
	int k, c, d;
	int err=0;

	if (n > MAXBUFFER) {
		printf("%d > %d\n", n, MAXBUFFER);
		exit(1);
	}

	for (k = 0; k < n; k++) {
		c = getnexthex(input);
		if (c == EOF) return EOF;
		d = getnexthex(input);
		if (d == EOF) return EOF;
		row[k] = (unsigned char) ((c << 4) | d);
		if (debugflag) printf("%d %02X - ", k, row[k]); 
	}
	return 0;
}

int nextrun (int i, int n) {			/* find next run */
	unsigned int c;

	for (;;) {
/*		if (i+2 >= n) return (n-1); */	/* too close to end of array */
		if (i+2 >= n) return (n);		/* too close to end of array */
		c = row[i];
		if (row[i+1] != c) {			/* definitely not a run */
			i++; continue;
		}
		if (row[i+2] != c) {			/* too short a run */
			i = i + 2 ; continue;
		}
		if (debugflag) printf("next run at %d\n", i); 
		return i;						/* found run length 3 or more */
	}
}

int runlength (int i, int n) {
	unsigned int c;
	int  k;

	if (i == n) return 0;				/* end of row */
	if (i == n-1) return 1;				/* end of row */
	c = row[i];
	for (k = i+1; k < n; k++) {
		if (row[k] != c) break;
	}
	if (debugflag) printf("run length %d at %d\n", k - i, i); 
	return (k - i);
}

void outputrun (FILE *output, int i, int m) {
	int c=row[i];
	while (m > 128) {
		putc(-(128-1), output);
		putc(c, output);
		if (debugflag) printf("Output run %d of %d\n", 128, c);
		m = m - 128;
		longrun++;
	}
	putc(-(m-1), output);
	putc(c, output);
	if (debugflag) printf("Output run %d of %d\n", m, c);
}

void outputnonrun (FILE *output, int k, int m) {
	int i;

	while (m > 128) {
		putc((128-1), output);
		for (i = k; i < k + m; i++) {
			putc(row[i], output);
		}
		if (debugflag) printf("Output non run %d at %d\n", 128, k);
		m = m - 128;
		k = k + 128;
		longnonrun++;
	}
	putc((m-1), output);
	for (i = k; i < k + m; i++) {
		putc(row[i], output);
	} 
	if (debugflag) printf("Output non run %d at %d\n", m, k);
}

void compressrow (FILE *output, int n) {
	int k=0;							/* current position */
	int i, m;

	if (n > MAXBUFFER) {
		printf("%d > %d\n", n, MAXBUFFER);
		exit(1);
	}

	for (;;) {
		i = nextrun(k, n);					/* find next run */
		m = i - k;
		if (m > 0) {					/* non-run stuff to put out? */
			outputnonrun(output, k, m);
			k = i + m;
		}
		if (i == n) break;			/* hit end of row */
/*		if (i == n-1) break; */			/* hit end of row */
		m = runlength(i, n);				/* how long a run */
		if (m < 3) {
		printf("Unexpected short run length %d at %d in row of length %d\n",
			m, i, n);
		}
		outputrun(output, i, m);
		k = i + m;
		if (k == n) break;			/* hit end of row */
/*		if (k == n-1) break; */			/* hit end of row */
	}
}

/* scan up to what appears to be a monochrome image */
/* keep track of page numbers on the way there */

int scantoimage(FILE *input) {
	int n;
	char *s;

	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == 12) strcpy(line, line+1);
		if (strncmp(line, "%%Page:", 7) == 0) {
			sscanf(line+2, "Page: %s %d", pagename, &pageno);
		}
/*		search for key line that introduces monochrome image */
/*		deal with both HPTAG PS style and DVIPSONE PS style */
		if (strncmp(line, "/Tiffrow", 8) == 0 ||
			strncmp(line, "/picstr", 7) == 0) {
			s = line;
			while (*s > ' ') s++;				/* step to first white space */
			while (*s <= ' ' && *s != '\0') s++;	/* step over white space */
			n = sscanf (s, "%d string def", &nbytes);
			if (n < 1) {
				printf("ERROR: expecting `/... <nbytes> string def' not: %s",
					   line);
				if (err++ > maxerror && maxerror > 0) exit(1);
			}
/* search for line with <ncolumns> <nrows> true ... */
			for (;;) {
				if (fgets(line, sizeof(line), input) == NULL) {
					printf("ERROR: premature EOF in image header\n");
					return EOF;
				}
				if (strstr(line, " true") != NULL) break;
				if (strstr(line, " false") != NULL) break;
			}
			n = sscanf(line, "%d %d true", &ncolumns, &nrows);
			if (n < 2) {
				printf("ERROR: expecting `<columns> <rows> true ...' not:", 
					   line);
				if (err++ > maxerror && maxerror > 0) exit(1);
			}
/* now search for line with `imagemask' */
			for (;;) {
				if (fgets(line, sizeof(line), input) == NULL) {
					printf("ERROR: premature EOF in image header\n");
					return EOF;
				}
				if (strstr(line, "imagemask") != NULL) break;
			}
/* quick sanity check */
			if ((ncolumns + 7) / 8 != nbytes) {
				printf("ERROR: Inconsistent ncolumns %d  nbytes %d (not %d)\n",
					ncolumns, nbytes, (ncolumns + 7) / 8);
				if (err++ > maxerror && maxerror > 0) exit(1);
			}
			return 0;			/* now positioned at start of image */
		}
	}
	return EOF;
}

int compressimage(FILE *input, FILE *output) {
	int k;
	for (k = 0; k < nrows; k++) {
/*		if (traceflag) printf("Reading row %d\n", k); */
		if (traceflag) printf("[%d ", k);
		if (readnextrow (input, nbytes) == EOF) return EOF;
/*		if (traceflag) printf("Compressing row %d\n", k); */
		if (traceflag) printf("%d] ", k);
		compressrow (output, nbytes);
		if (slowdotsflag) if (k % 100 == 0) putc('.', stdout);
		else if (dotsflag) putc('.', stdout);
	}
	if (slowdotsflag || dotsflag) putc('\n', stdout);
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

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void writeheader (FILE *output, char *outfile) {
	iheight = nrows;
	iwidth = ncolumns;
	bitspersampleout = 1;
	strcpy(comment, outfile);			/* put file name in Artist */
	sprintf(comment, "Page %s %d. File %s", pagename, pageno, outfile);
	writetiffheader(output);
}

void writetrailer (FILE *output) {

}

/**************************************************************************/

void showusage (char *name) {
	printf ("Usage:\n");
/*	printf ("%s <HPTAG generated PS file>\n", argv[0]); */
	printf ("%s [-v] [-t] [-e] [-d] [-l=<logfile>] <PS file>\n", name);
	if (detailflag == 0) exit(1);
	printf ("\n");
	printf ("\tv verbose mode\n");
	printf ("\tt trace mode (more verbosity)\n");
	printf ("\te do not exit after %d errors\n", maxerror);
	printf ("\td expect each image file to be mentioned twice, not once\n");
	printf ("\tl optional logfile (from DVIPSONE) listing image file names\n");
	printf ("\n");
	printf ("\tThe input PostScript file should be one made by the HPTAG system\n");
	printf ("\tThe output image files will be written into the current directory\n");
	printf ("\tIf no log file specified, image files will be numbered sequentially\n");
	printf ("\tImage files are TIFF images, but use original image file names\n");
	exit(1);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int decodeflag (int c) {
/*	printf ("FLAG: %c%n", c); */
	switch(c) {
		case '?': detailflag = 1; return 0;
		case 'v': verboseflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case 'p': dotsflag = 1; return 0;
		case 'D': debugflag = 1; return 0;
		case 'e': maxerror = 0; return 0;
		case 'd': doubleflag = 1; return 0;
/*		following take arguments */
		case 'l': logfileflag = 1; break;
		default: {
			 fprintf(stderr, "Invalid command line flag '%c'", c);
			 exit(1);
				 }
	}
	return -1;		/* need argument */
}

/* Flags and Arguments start with `-' */
/* Also allow use of `/' for convenience */
/* Normal use of `=' for command line arguments */
/* Also allow use of `:' for convenience */
/* Archaic: use space to separate - only for backward compatability */

int decodearg(char *command, char *next, int firstarg) {
	char *s;
	char *sarg=command;
	int c;
	
	if (*sarg == '-' || *sarg == '/') sarg++;	/* step over `-' or `/' */
	while ((c = *sarg++) != '\0') {				/* until end of string */
		if (decodeflag(c) != 0) {				/* flag requires argument ? */
/*			if ((s = strchr(sarg, '=')) == NULL) { */
			if (*sarg != '=' && *sarg != ':') {	/* arg in same string ? */
				if (next != NULL) {
					firstarg++; s = next;	/* when `=' or `:' is NOT used */
				}
				else {
					fprintf(stderr, "Don't understand: %s\n", command);
					return firstarg;
				}
			}
/*			else s++;	*/		/* when `=' IS used */
			else s = sarg+1;	/* when `=' or `:' IS used */

/* now analyze the various flags that could have gotten set */
			if (logfileflag != 0) {
				logfile = s;
				logfileflag = 0;
			}
			break;	/* default - no flag set */
		}
	}
	return firstarg;
}

/*	check command line flags and command line arguments */

int commandline (int argc, char *argv[], int firstarg) {
	int c;
	
	if (argc < firstarg+1) showusage(argv[0]);
	c = argv[firstarg][0];
	while (c == '-' || c == '/') {
		firstarg = decodearg(argv[firstarg], argv[firstarg+1], firstarg+1);
		if (firstarg >= argc) break;			/* safety valve */
		c = argv[firstarg][0];
	}
/*	if (argc < firstarg+1) showusage(argv[0]); */
	return firstarg;
}

#ifdef IGNORED
  while (*argv[firstarg] == '-') {
		if (strncmp(argv[firstarg], "-t", 2) == 0) traceflag++;
		else if (strncmp(argv[firstarg], "-q", 2) == 0) verboseflag=0;
		else if (strncmp(argv[firstarg], "-v", 2) == 0) verboseflag=1;
		else if (strncmp(argv[firstarg], "-p", 2) == 0) dotsflag++;
		else if (strncmp(argv[firstarg], "-d", 2) == 0) debugflag++;
		else if (strncmp(argv[firstarg], "-e", 2) == 0) maxerror = 0;
		else if (strncmp(argv[firstarg], "-l", 2) == 0) {
			if ((s = strchr(argv[firstarg], '=')) != NULL) logfile=s+1;
			else if (firstarg+1 < argc && *argv[firstarg+1] != '-') {
				logfile = argv[firstarg+1]; firstarg++;
			}
			else {
				printf("Invalid command line flag: %s\n", argv[firstarg]);
				showusage(argv[0]);
			}
		}
		else {
			printf("Invalid command line flag: %s\n", argv[firstarg]);
			showusage(argv[0]);
		}
		firstarg++;
  }
#endif

/**************************************************************************/

int main(int argc, char *argv[]) {
	FILE *input;
	FILE *output;
	char infile[MAXFILENAME];
	char outfile[MAXFILENAME];
	char ext[16];
	int firstarg=1;
	int k, flag;
	int kk=0, mismatch=0;
	int m=1;
/*	char *s; */

/*	if (argc < firstarg+1) {
		printf("Missing argument: need to specify input PS file\n");
		showusage(argv[0]);
	} */

/*	if (strncmp(argv[firstarg], "-?", 2) == 0) {
		detailflag = 1;
		showusage(argv[0]);
	} */

	if (argc < firstarg+1) showusage(argv[0]); 
	firstarg = commandline(argc, argv, firstarg); /* check for command flags */
	if (detailflag) showusage(argv[0]); 
/*	if (argc < firstarg+1) showusage(argv[0]); */

	for (k = 0; k < MAXIMAGES; k++) imagenames[k] = NULL;
	for (k = 0; k < MAXIMAGES; k++) {
		imagecount0[k] = imagecount1[k] = imagecount2[k] = -1;
	}

	if (*logfile != '\0') {						/* 1995/Mar/2 */
		strcpy(infile, logfile);
		extension(infile, "log");
		if ((input = fopen(infile, "r")) == NULL) {
			perror(infile);
			exit(1);
		}
		if (verboseflag) printf ("Reading logfile %s\n", infile);
		readlogfile(input);
		fclose(input);
	}

	if (argc < firstarg+1) {
		printf("Missing argument: need to specify input PS file\n");
		showusage(argv[0]);
	}

	putc('\n', stdout);
	printf("EXTRATIF version 1.0  Y&Y, Inc.  (978) 371-3286\n");
	putc('\n', stdout);

	strcpy(infile, argv[firstarg]);
	extension(infile, "ps");
	if ((input = fopen(infile, "r")) == NULL) {
		perror(infile);
		exit(1);
	}

	pageno=0;

	kk = 0;								/* index into imagenames[] */
	mismatch = 0;

	for (;;) {
		err=0;							/* reset for each image  ? */
		longrun=0;
		longnonrun=0;
		if (scantoimage(input) == EOF) break;
		if (verboseflag)
	printf("Found image size %d %d (%d bytes per row) on page %s %d\n",
				   ncolumns, nrows, nbytes, pagename, pageno);
		flag = 0;
		if (useimagenames) {
			if (kk >= imageindex) {
				fprintf(stderr,
	"MISMATCH? PS file has more images than indicated in log file (%d)\n",
						imageindex);
				flag++;
				kk--;
			}
/* compare imagecount0[kk] with pageno ? */
			if (imagecount0[kk] != pageno) {
	printf("MISMATCH?  Count0 %d not equal PS file DSC pageno %d\n",
					   imagecount0[kk], pageno);
				flag++;
			}
/* compare pagename with imagecount1[kk]-imagecount2[kk] ? */
			sprintf(line, "%d-%d", imagecount1[kk], imagecount2[kk]);
			if (strcmp(line, pagename) != 0) {
	printf("MISMATCH?  Count1-Count2 %s not equal PS file DSC pagelabel %s\n",
					   line, pagename);
				flag++;
			}
			if (flag) mismatch++;
			strcpy(outfile, imagenames[kk]);
			kk++;
/*			if (kk >= imageindex) {
				fprintf(stderr, "PS file has more images than log file\n");
				mismatch++;
				kk--;
			} */
		}
		else {
			strcpy(outfile, stripname(infile));
			sprintf(ext, "%03d", m);
			forceexten(outfile, ext);
		}
		if ((output = fopen(outfile, "wb")) == NULL) {
			perror(outfile);
			exit(1);
		}
		if (verboseflag)
			printf("Processing %s ==> %s Page %s %d\n",
				   infile, outfile, pagename, pageno);
		writeheader(output, outfile);
		if (compressimage(input, output) == EOF) break;
		if (verboseflag) {
			printf("Compressed an image %s ", outfile);
			if (traceflag) 
				printf(", %ld long runs, %ld long non runs\n",
				   longrun, longnonrun);	
			putc('\n', stdout);
		}
		writetrailer(output);
		fclose (output);
		if (err > 0) printf("%d ERRORS encountered\n", err);
		m++;
	}
	fclose (input);
	if (verboseflag)
		printf("Reached end of input file %s, %d image%s extracted\n",
			   infile, m-1, (m > 2) ? "s" : "");
	if (mismatch > 0) {
		printf("WARNING: Found %d apparent mismatch%s in page counters\n",
			   mismatch, (mismatch == 1) ? "" : "es");
		printf("         May have resulted in incorrect file names\n");
	}
	if (imageindex != 0 && kk != imageindex)
		printf("WARNING: Mismatch in image count: expected %d processed %d\n",
			   imageindex, kk);

	for (k = 0; k < MAXIMAGES; k++) {
		if (imagenames[k] != NULL) free(imagenames[k]);
	}
	return 0;
}

/* pattern of monochrome image call by DVIPSONE:

save
currentscreen pop dviscreen
/picstr 231 string def
currentpoint translate 29175399 36575847 scale
white
0 0 moveto 1 0 lineto 1 1 lineto 0 1 lineto closepath fill
black
1848 2317 true
[1848 0 0 2317 0 0]
{currentfile picstr readhexstring pop} bind
imagemask
......................HEX DATA.................
restore

*/

/* pattern of monochrome image call in HPTAG produced PS file:

TE
TB 354 2839 translate 1848 1 scale
/Tiffrow 231 string def
1848 2317 true [1848 0 0 -1 0 0]
{ currentfile Tiffrow readhexstring pop } bind imagemask
......................HEX DATA.................
TE
TB

 */

