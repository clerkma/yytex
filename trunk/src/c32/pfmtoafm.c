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

/* Program for decoding PFM files and making partial AFM files from them */
/* sorts kern table so first character is most significant */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>			/* for isupper() */
#include <malloc.h>
#include <conio.h>

#ifdef _WIN32
#define __far
#define _frealloc realloc
#define _fmalloc malloc
#define _ffree free
#endif

/* #define MAXKERNS 512 */		/* NOT the real limit */
#define MAXKERNS 1024		/* NOT the real limit */
#define MAXCHRS 256
#define CHARNAME_MAX 32
#define MAXLINE 512

#define MAXCOMMENT 128

#define ITALICFUZZ 30

int relativeflag = 1;	/* non-zero means default and break wrt firstchar */
						/* probably right, some fonts use this convention */

int ansiflag = 1;		/* non-zero means ANSI encoding */
						/* otherwise use symbol/decorative flags */
int oemflag = 0;		/* non-zero means OEM encoding */
int symbolflag = 0;		/* non-zero means symbol encoding */
int decorative = 0;		/* NOT ANSI encoding - `native' encoding */
 
int texflag = 0;		/* non-zero means TeX font */

int verboseflag = 0;
int traceflag = 0;
int debugflag = 0;
int showencoding = 0;

int ignorezerokerns = 1;	/* non-zero => don't output zero size kerns */

int sortflag = 1;			/* sort kern table before output */

int detailflag = 0;
int positionflag = 0;
int vectorflag = 0;

int fontfiddler=0;			/* 1993/Oct/28 */

int reencodeflag = 0;

int initreencodeflag = 0;

long position=-1;

/* FILE *errout=stderr; */
FILE *errout=stdout;

/* char *programversion = "PFMtoAFM conversion utility version 0.9"; */
/* char *programversion = "PFMtoAFM conversion utility version 1.0"; */
/* char *programversion = "PFMtoAFM conversion utility version 1.1"; */
/* char *programversion = "PFMtoAFM conversion utility version 1.2"; */
/* char *programversion = "PFMtoAFM conversion utility version 1.3"; */
/* char *programversion = "PFMtoAFM conversion utility version 1.3.1"; */
/* char *programversion = "PFMtoAFM conversion utility version 1.3.2"; */
/* char *programversion = "PFMtoAFM conversion utility version 1.3.3"; */
char *programversion =
/* "PFMtoAFM conversion utility version 1.3.4"; */
/* "PFMtoAFM conversion utility version 1.3.5";	/* 96/Mar/3 */
/* "PFMtoAFM conversion utility version 1.4"; */	/* 97/Nov/6 */
/* "PFMtoAFM (32) conversion utility version 1.4.2"; */	/* 98/Aug/15 */
#ifdef _WIN32
"PFMtoAFM (32) conversion utility version 1.4.3";	/* 98/Sep/10 */
#else
"PFMtoAFM conversion utility version 1.4.1";	/* 97/Nov/16 */
#endif

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

int wantcpyrght=1;		/* want copyright message in there */

/* #define COPYHASH 2244854 */
/* #define COPYHASH 5236673 */
/* #define COPYHASH 15004919 */
/* #define COPYHASH 5793828 */
/* #define COPYHASH 11425582 */
#define COPYHASH 5606175

char *copyright = "\
Copyright (C) 1990--1998, Y&Y. All rights reserved. (978) 371-3286\
";

char *commandvector="";				/* vector specified on command line */

char vector[32]="";				/* which coding scheme in use */

char *vectorpath = "";			/* was: c:\\dvipsone path for vectors */

char *defaultvector="ansinew";

char programpath[FILENAME_MAX] = "c:\\yandy\\util";

unsigned char __far *buffer=NULL;

/* char encoding[MAXCHRS][CHARNAME_MAX]; */

char *encoding[MAXCHRS];		/* 1997/Nov/16 */

char line[MAXLINE];

/* New, to try and figure out encoding vector from file */

char *encodingnames[][2] = {
{"TeX text", "textext"},					/* cmr*, cmb*, cmsl*, cmss* */
/*		or			"texital"		for			cmti* */
{"TeX text without f-ligatures", "textype"},	/* cmcsc10 & cmr5 */
{"TeX typewriter text", "typewrit"},		/* cm*tt* */
/*		or			"typeital"		for			cmitt* */
{"TeX extended ASCII", "texascii"},			/* cmtex* */
{"TeX math italic", "mathit"},				/* cmmi* */
{"TeX math symbols", "mathsy"},				/* cmsy* */
{"TeX math extension", "mathex"},			/* cmex10 */
{"ASCII caps and digits", "textext"},		/* cminch */
{"Text in Adobe Setting", "textext"},		/* pctex style ? */
{"Adobe StandardEncoding", "standard"},
{"AdobeStandardEncoding", "standard"},
{"Adobe Symbol Encoding", "symbol"},
{"Adobe Dingbats Encoding", "dingbats"},
{"MicroSoft Windows ANSI 3.0", "ansi"},
{"MicroSoft Windows ANSI 3.1", "ansinew"},
/* {"tex ansi windows 3.0", "texansi"}, */
/* {"TeX ANSI windows 3.1", "texannew"}, */
{"Ventura Publisher Encoding", "ventura"},
{"TeX 256 character Encoding", "tex256"},
{"Extended TeX Font Encoding - Latin", "tex256"},
{"Extended TeX Font Encoding - Latin", "cork"},
{"TeX text companion symbols 1---TS1", "ts1"},
{"Macintosh", "mac"},
/* {"cyrillic", "cyrillic"}, */
{"TEX TEXT + ADOBESTANDARDENCODING", "neonnew"},	/* 1992/Dec/22 */
/* {"ansi", "tex&ansi"}, */								/* 1994/Jun/20 */
{"TeX typewriter and Windows ANSI", "texnansi"},	/* 1994/dec/31 */
{"FontSpecific", "numeric"},						/* 1995/Apr/8 */
{"", ""}
 };	
  
/* encoding vectors read in from file now rather than kept in program */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef IGNORE
void extension(char *fname, char *ext) {  /* supply extension if none */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}
#endif

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

int getline(FILE *input, char *buf, int nmax) {	/* reading encoding */
	int c, k=0;
	char *s=buf;

	while ((c = getc(input)) != '\n' && c != EOF) {
		*s++ = (char) c;
		k++;
		if (k >= nmax) {
			*s = '\0';
			fprintf(errout, "Line too long: %s\n", buf);
			exit (13);
		}
	}
	if (c != EOF) {
		*s++ = (char) c;
		k++;
	}
	*s = '\0';
	if (showencoding != 0) printf("%s", buf);
	return k;
}

int getrealline(FILE *input, char *buf, int nmax) {
	int k;
	k = getline(input, buf, nmax);
	while ((*buf == '%' || *buf == '\n') && k > 0)
		k = getline(input, buf, nmax);		
	return k;
}

FILE *openencoding(char *name) {
	FILE *input=NULL;
	char fn_in[FILENAME_MAX];
	char *s, *t;

/*	printf("READVECTORFILE %s\n", name); */

/*	First try current directory */
/*	First try name as given, possibly with path */
	strcpy(fn_in, name);
	extension(fn_in, "vec");
	if (traceflag != 0) printf("Trying %s\n", fn_in);
	input = fopen(fn_in, "r");

/*  Next try along path specified by environmental variable VECPATH */
	if (input == NULL) {
/*		don't bother to do this if name is qualified ... */
		if (strchr(name, '\\') == NULL &&
			strchr(name, '/') == NULL &&
			strchr(name, ':') == NULL) {
			s = vectorpath;
			while (*s != '\0') {
				if ((t = strchr(s, ';')) != NULL) *t = '\0';	/* flush ; */
				strcpy(fn_in, s);
				if (strcmp(fn_in, "") != 0) {
					s = fn_in + strlen(fn_in) - 1;
					if (*s != '\\' && *s != '/') strcat(fn_in, "\\");
				}
				strcat(fn_in, name);
				extension(fn_in, "vec");
				if (traceflag != 0) printf("Trying %s\n", fn_in);
				input = fopen(fn_in, "r");
				if (input != NULL) break;		/* success */
				if (t == NULL) break;			/* failed */
				else s = t+1;					/* continue after ; */
			}
		}
	}

	if (input == NULL) {	/*	then try in directory of procedure itself */
		strcpy(fn_in, programpath);
		strcat(fn_in, "\\");
		strcat(fn_in, name);
		extension(fn_in, "vec");
		if (traceflag != 0) printf("Trying %s\n", fn_in);
		input = fopen(fn_in, "r");
	}
	if (input == NULL) {	/*	OK, time to punt ! */
		fprintf(stderr, "ERROR: Can't open encoding vector\n");
		perror(name);
		return NULL;
	}

	if (verboseflag != 0) printf("Using encoding vector file %s\n", fn_in);

	return input;
}

#ifdef IGNORED
FILE *openencoding(char *name) {
	char fn_vec[FILENAME_MAX];
	FILE *fp_vec=NULL;
	
/*	first try current directory */ /* added 1993/Aug/31 */
	strcpy(fn_vec, name);
	extension(fn_vec, "vec");
	if (traceflag != 0) printf("Trying %s\n", fn_in);

	if ((fp_vec = fopen(fn_vec, "r")) == NULL) {
/*		try name as given if path name included */
		if (strchr(name, ':') != NULL ||
			strchr(name, '\\') != NULL ||
			strchr(name, '/') != NULL) {
			strcpy(fn_vec, name);
			extension(fn_vec, "vec");
			printf("%s\n", fn_vec);
			if ((fp_vec = fopen(fn_vec, "r")) == NULL) {
				fprintf(errout, "ERROR: Can't open encoding vector\n");
				perror(fn_vec); 
				return NULL;
			}
			else return fp_vec;
		}
	}
	else return fp_vec;

/*	see if vector path specified use it - otherwise try current directory */
	if (strcmp(vectorpath, "") != 0) {
		strcpy(fn_vec, vectorpath);
		strcat(fn_vec, "\\");
	}
	else strcpy(fn_vec, "");

	strcat(fn_vec, name);
	extension(fn_vec, "vec");

	printf("%s\n", fn_vec);
	if ((fp_vec = fopen(fn_vec, "r")) == NULL) {
/*		if that fails try program path */
		strcpy(fn_vec, programpath); 
		strcat(fn_vec, "\\");
		strcat(fn_vec, name);
/*		strcat(fn_vec, ".vec"); */
		extension(fn_vec, "vec");
		printf("%s\n", fn_vec);
		if ((fp_vec = fopen(fn_vec, "r")) == NULL) {
			fprintf(errout, "ERROR: Can't open encoding vector\n");
			perror(fn_vec); 
/*			if (strcmp(name, "textext") != 0) setromanencoding();	
			else if (strcmp(name, "standard") != 0) setstandardencoding();
			else if (strcmp(name, "ansi") != 0) setansiencoding(); */
			return NULL;
		}
	}
/*	if (verboseflag != 0) */	/* 1993/Aug/23 */
		printf("Using encoding vector %s\n", fn_vec);
	return fp_vec;
}
#endif

void setnumericencoding(void) {
	int k;
	for(k = 0; k < MAXCHRS; k++) {
/*		sprintf(encoding[k], "a%d", k); */
		sprintf(line, "a%d", k);
		encoding[k] = _strdup(line);		/* 97/Nov/16 */
	}
/*	(void) readencoding("numeric");  */
}

void cleanencoding (void) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {			/* clean out */
		if (strcmp(encoding[k], "") != 0) {
			free(encoding[k]);				/* 97/Nov/16 */
			encoding[k] = "";
		}
	}
}

int readencoding(char *name) {
	char charcode[CHARNAME_MAX];
	FILE *fp_vec=NULL;
	int k, n;			/* n is not accessed */

	if (strcmp(name, "numeric") == 0) {
		setnumericencoding();
		return 0;
	}

	fp_vec = openencoding(name);

	if (fp_vec == NULL) return -1;

	cleanencoding();

 	n=0; 
	while (getrealline(fp_vec, line, MAXLINE) > 0) {
		if (*line == '%' || *line == ';') continue;	/* ignore comment */
/*		if (sscanf(line, "%d %s", &k, &charcode) < 2) { */
		if (sscanf(line, "%d %s", &k, charcode) < 2) {
			fprintf(errout, "WARNING: don't understand encoding line: %s", 
				line);
		} 
		else if (k >= 0 && k < 256) {
/*			strcpy(encoding[k], charcode); */
			encoding[k] = _strdup(charcode);	/* 97/Nov/16 */
			n++;
		}
/*		printf("%d %s", n, line); */
	}

	if (ferror(fp_vec) != 0) 
		fprintf(errout, "WARNING: Error in encoding vector file read\n");
	else fclose(fp_vec);
	return 0;		/* ??? */
}

void setromanencoding(void) {
	(void) readencoding("textext"); /* "default" ? */
}

void setansiencoding(void) {
 	(void) readencoding(defaultvector); 
}

void setstandardencoding(void) {
	(void) readencoding("standard");
}

void setoemencoding(void) {
	(void) readencoding("ibmoem");
}

void setsymbolencoding(void) {
	(void) readencoding("symbol");
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* unsigned int readone(unsigned char *s) {
	return (unsigned int) *s;
} */

unsigned int readone(unsigned char __far *s) {
	return (unsigned int) *s;
}

/* unsigned int readtwo(unsigned char *s) {
	return ((unsigned int) *s) | (((unsigned int) *(s+1)) << 8);
} */

unsigned int readtwo(unsigned char __far *s) {
	return ((unsigned int) *s) | (((unsigned int) *(s+1)) << 8);
}

/* unsigned long readthree(unsigned char *s) {
	return *s | (((unsigned int) *(s+1)) << 8) 
		| ((unsigned long) *(s+2) << 16);
} */

unsigned long readthree(unsigned char __far *s) {
	return *s | (((unsigned int) *(s+1)) << 8) 
		| ((unsigned long) *(s+2) << 16);
}

/* unsigned long readfour(unsigned char *s) {
	return *s | (((unsigned int) *(s+1)) << 8) | 
		((unsigned long) *(s+2) << 16) | ((unsigned long) *(s+3) << 24);
} */

unsigned long readfour(unsigned char __far *s) {
	return *s | (((unsigned int) *(s+1)) << 8) | 
		((unsigned long) *(s+2) << 16) | ((unsigned long) *(s+3) << 24);
}

/* int sreadtwo(unsigned char *s) {
	return ((int) *s) | (((int) *(s+1)) << 8);
} */

#ifdef _WIN32
short sreadtwo(unsigned char __far *s) {
	return ((short) *s) | (((short) *(s+1)) << 8);
}
#else
int sreadtwo(unsigned char __far *s) {
	return ((int) *s) | (((int) *(s+1)) << 8);
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* return pointer to file name - minus path - returns pointer to filename */

char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = stripname(filename);
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
#ifdef DEBUGGING
	if (traceflag != 0) printf("n = %d m = %d ", n, m);
#endif
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
#ifdef DEBUGGING
	if (traceflag != 0) printf("Now trying file name: %s\n", s);
#endif
	return 0;
}

/* int readpfm(char *pfm) {*/ /* read the PFM file into the buffer */
/* int readpfm(char *fn_in) { */ /* read the PFM file into the buffer */
int readpfm(char fn_in[]) {  /* read the PFM file into the buffer */
	FILE *fp_in;
	unsigned int k=0;
/*	char *s=buffer;	 */
/*	unsigned char *s=buffer; */
/*	unsigned char *s; */
	unsigned char __far *s;
	int c;
	long length;

/*	crude way to guess whether this is a TeX file */
	if(strncmp(fn_in, "CM", 2) == 0 ||
		strncmp(fn_in, "cm", 2) == 0) texflag = 1;
	else texflag = 0;
	extension(fn_in, "pfm");
	if ((fp_in = fopen(fn_in, "rb")) == NULL) {
		underscore(fn_in);
		if((fp_in = fopen(fn_in, "rb")) == NULL) {
			perror(fn_in);	/* exit(3); */
			return 0;
		}
	}
	
	fseek(fp_in, 0, SEEK_END);
	length = ftell(fp_in);					/* get file length */
	fseek(fp_in, 0, SEEK_SET);				/* rewind to start */

	if (debugflag) printf("%s has length %ld\n", fn_in, length);

	if (length > 65535) {
		fprintf(errout,
		"ERROR: Supposed PFM file seems ridiculously large (%ld > 65535)\n",
				length);
		exit(1);
	}
	if (buffer != NULL) {
		fprintf(errout, "ERROR: memory allocation\n");
		_ffree(buffer);
	}
	buffer = (unsigned char __far *) _fmalloc((unsigned int) length);
	if (buffer == NULL) {
		fprintf(errout,
			"ERROR: Unable to allocate memory for PFM file (%ld)\n", length);
		exit(1);
	}
	s = buffer;

	while ((c = getc(fp_in)) != EOF) {
/*		*s++ = (char) c; */
		*s++ = (unsigned char) c;
/*		if (k++ >= BUFFERLENGTH) { */
		if (s > buffer + length) {		/* 1993/Nov/17 */
/*			fprintf(errout, 
				"WARNING: PFM file too large (> %d)\n", length); */
			fprintf(errout,
				"IMPOSSIBLE ERROR: PFM file overrun (> %d) char %d\n", 
					(s - buffer), c);
			exit(1);
		}
	}
	fclose (fp_in);
	return (s - buffer);		/* should be == length */
}

void printchar(int c) {
	if (c >= ' ' && c < 127) printf("%c ", c);
	else if (c >= 128) {
		printf("M-"); printchar(c - 128);
	}
	else if (c < 32) {
		printf("C-"); printchar(c + 64);
	}
	else if (c == 127) {
		printf("rubout");
	}
}

void showusage(char *s) {
	fprintf (errout, "\
%s [-{v}{z}{s}] [-c=<vector>] <pfm-file-1> <pfm-file-2>...\n", s);
	if (detailflag == 0) exit(1);
	fprintf (errout, "\
\tv: verbose mode (extra info in PFM shown on screen)\n\
\tz: do not ignore zero size kerns\n\
\ts: do not re-sort kern pairs\n\
\tc: use specified encoding vector (default `%s')\n\
\t   (unless encoding vector specified in comment in PFM file)\n\
", defaultvector);
/*	fprintf (errout, "\n\
\t   NOTE: information in AFM file may be incomplete\n\
); */
	exit(1);
}

/* \tt: tracing mode\n\ */
/* \t[-n=<position>] */
/* \tn: compare bytes in files at specified position\n\ */

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0; 
		case 'v': verboseflag = 1; return 0; 
		case 't': traceflag = 1; return 0;
		case 'd': debugflag = 1; return 0; 
		case 's': sortflag = 0; return 0;
		case 'z': ignorezerokerns = 0; return 0;
/*		case 'E': errout = stdout; return 0; */
		case 'E': errout = stderr; return 0;	/* 97/Sep/23 */
/* rest take arguments */
		case 'c': vectorflag = 1; return -1;  
		case 'n': positionflag = 1; return -1; 
		default: {
			fprintf(errout, "Invalid command line flag '%c'", c);
			exit(7);
		}
	}
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(errout, "HASHED %ld\n", hash);	
/*	fflush(errout); */
	(void) _getch(); 
	return hash;
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (positionflag != 0) {
/*					sscanf(s, "%d", &position);  */
					sscanf(s, "%ld", &position); 
					positionflag = 0; 
				}
				else if (vectorflag != 0) {
/*					strcpy(vector, s); */
					commandvector = s; 
					vectorflag = 0; reencodeflag = 1; 
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

/* remove file name - keep only path - inserts '\0' to terminate */
void removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
}

	int italicnum, superscriptpos, kern, width, PixWidth; 
	unsigned int italic, underline, strikeout;
	unsigned int firstchar, lastchar, weight;
	unsigned int defaultchar, breakchar, charset;
	unsigned int avwidth, MaxWidth, code, pitch, family;
	unsigned int pfmlen, len, Height, Ascent, Descent;
	unsigned int strikeoutpos, strikeoutsize;
	unsigned int widthinx, origininx, driverinx, faceinx, fontnameinx;
	unsigned int reserved;
	unsigned int pairkerninx, trackkerninx, extendinx;
	unsigned int underlinethickness, underlineposition;
	unsigned int superscriptsize, subscriptpos, subscriptsize;
	unsigned int doubleupperpos, doubleuppersize, doublelowerpos, doublelowersize;
	unsigned int Ascender, XHeight, CapHeight, Descender;
	unsigned int pointsize, orientation, maxscale, minscale;
	unsigned int masterHeight, masterunits, dftype;
	unsigned int kernpairs, kerntracks, vres, hres;
	unsigned int InternalLeading, ExternalLeading;	/* NOT USED FOR ANYTHING */
	unsigned int points, extsize, metsize;

int comparative(int firstarg, int argc, char *argv[]) {
	int m, c;
	unsigned char __far *s;
	char fn_in[FILENAME_MAX];

	for (m = firstarg; m < argc; m++) {
		printf("%s \t", argv[m]);
		strcpy(fn_in, argv[m]); 
/*		if (readpfm(argv[m]) > 0) { */
		if (readpfm(fn_in) > 0) {			/* 1992/Nov/22 */
			s = buffer + position;			/* k */
			c = *s;
			printchar(c);
			printf(" \t%02u (%02X)\t%u \t%lu \t%lu\n",
				readone(s), readone(s), readtwo(s), readthree(s), readfour(s));
		}
		else printf("\n");					/* bad PFM file */
#ifndef _WIN32
		if ((n = _fheapchk ()) != _HEAPOK) {		/* 1994/Feb/18 */
			fprintf(errout, "WARNING: Far heap corrupted (%d)\n", n);
			exit(1);
		}
		else {
			if (buffer != NULL) _ffree(buffer);
			buffer = NULL;
		}
#else
		if (buffer != NULL) _ffree(buffer);
		buffer = NULL;
#endif
	}
	printf("Processed %d PFM files\n", argc - 1);
	return 0;
}

/* void writewidthtable (FILE *fp_out, char *s) { */

/* void writewidthtable (FILE *fp_out, unsigned char *s) { */
void writewidthtable (FILE *fp_out, unsigned char __far *s) {
	unsigned int k, kk, count=0, badcount = 0;
	int xll, yll, xur, yur;
	int defwidth; 
	char *charname="";
/*	char str[2]; */

/*	fprintf(fp_out, "StartCharMetrics %u\n", lastchar - firstchar +1); */
	for (k = firstchar; k <= lastchar; k++) {
/*		charname = encoding[k];
		if (strcmp(charname, "") != 0) count++; */
		if (strcmp(encoding[k], "") != 0) count++;	/* 97/Nov/16 */
	}
	if (count > 0)
		fprintf(fp_out, "StartCharMetrics %u\n", count);
	kk = 2 * (defaultchar - firstchar);			/* usually 149 ANSI bullet ... */
	defwidth = sreadtwo(s + kk);		/* default character width */

	for (k = firstchar; k <= lastchar; k++) {
		kk = 2 * (k - firstchar);
/*		width = readtwo(s + kk); */
		width = sreadtwo(s + kk); 
		if (width >= 0 && width <= 2) {
			if (traceflag != 0) printf("C %u ; WX %u ; ", k, width);
			width = 0;			/* complement to afmtopfm ??? */
		}
/*		if (traceflag != 0) printf("C %u ; WX %u ; ", k, width);
		fprintf(fp_out, "C %u ; WX %u ; ", k, width);
		if (((k + 1 - firstchar) % 4) == 0) {
		if (traceflag != 0) {printf("\n"); getch();}
		else if (traceflag != 0) printf("\t");
		fprintf(fp_out, "\t"); 
		fprintf(fp_out, "\n"); 
		} */
/*		assuming Tex encoding unless full encoding vector given */
/*		if (lastchar < 256) charname = defaultencoding[k]; */
/*		else charname = standardencoding[k]; */
/*		Naey, use encoding specified after all */ /* ??? */
/*		if (lastchar < 256) charname = encoding[k];
		else fprintf(errout, "WARNING: lastchar > 256\n"); */ /* what ? */
/*		charname = encoding[k]; */
/*		if (strcmp(charname, "") != 0) { */
		if (strcmp(encoding[k], "") != 0) {			/* 97/Nov/16 */
/*			fprintf(fp_out, "C %u ; WX %d ; N %s ; ", k, width, charname); */
			fprintf(fp_out, "C %u ; WX %d ; N %s ; ", k, width, encoding[k]);
/*		Add crude character BBox information */ /* 1992/Jan/22 */
/*		fprintf(fp_out, "B %d %d %d %d\n", 0, -Descent, width, Ascent); */
/*		fprintf(fp_out, "B %d %d %d %d\n", 0, -Descender, width, Ascender); */
/*		fprintf(fp_out, "B %d %d %u %u\n", 0, -Descender, width, Ascender); */
			xll = ITALICFUZZ; 
			xur = width - ITALICFUZZ;
			if (xur < 0) xur = 0;
			if (xur < xll) 	xll = 0;
/*			yll = -Descender;  */
			yll = - (int) Descender; 
			yur = Ascender;
			if (k >= 'A' && k <= 'Z') {		/* upper case */
				yll = 0; yur = CapHeight;
				if (k == 'Q') yll = - (int) (CapHeight/5);
				if (k == 'J') yll = - (int) (CapHeight/5);		/* ? */
			}
			else if (k >= '0' && k <= '9') { /* digit */
				yll = 0; yur = CapHeight;
			}
			else if (k >= 'a' && k <= 'z') { /* lower case */
/*				str[0] = (char) k; str[1] = '\0'; */
				yll = 0; yur = XHeight;
/*				if (strpbrk(str, "bdfhklt") != NULL) yur = Ascender; */
				if (strchr("bdfhklt", k) != NULL) yur = Ascender; /* 93/Jun/12 */
/*				if (strpbrk(str, "fgjpqy") != NULL) yll = - (int) Descender; */
				if (strchr("fgjpqy", k) != NULL) yll = - (int) Descender; /* 93/Jun/12 */
			}
/*			fprintf(fp_out, "B %d %d %u %u\n", xll, yll, xur, yur);  */
			fprintf(fp_out, "B %d %d %u %u ;\n", xll, yll, xur, yur); 
			/* above fixed 95/Aug/14 */
		}
		else if (width != 0) {
			if (width != defwidth) {
/*				if (traceflag) */
				if (verboseflag)
			printf("Unencoded char %d has width %d not default %d (%d)\n",
						   k, width, defwidth, defaultchar);
				badcount++;				/* 1995/Jan/22 */
			}
		}
	}
	if (count > 0)	fprintf(fp_out, "EndCharMetrics\n");
	if (badcount > 0) {
		fprintf(errout,
		"WARNING: There appear to be %u chars that are not in the encoding\n",
				badcount);
		fprintf(errout, "         Perhaps the wrong encoding vector ");
		if (strcmp(vector, "") == 0) fprintf(errout, "(%s) ", vector);
		fprintf(errout, "is being used?\n");
/*		fprintf(errout,
"         Perhaps the wrong encoding vector (%s) is being used?\n",
				vector); */
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef _WIN32
struct kernpair { 
	unsigned short kppair; short kpamount;
} KERNPAIR;
#else
struct kernpair { 
	unsigned int kppair; int kpamount;
} KERNPAIR;
#endif

/* Big problem here! Need to sort kern pairs in far space ??? */
/* Either transfer to near space or get far version of qsort */

/* int _cdecl compare(const struct kernpair *ka, const struct kernpair *kb) { */
/* int compare(const void *vka, const void *vkb) { 
	const struct kernpair *ka;
	const struct kernpair *kb;
	unsigned int botha, bothb;
	unsigned int aone, atwo, bone, btwo;

	ka = (const struct kernpair *) vka;
	kb = (const struct kernpair *) vkb;
	botha = ka->kppair; bothb = kb->kppair;
	aone = botha & 255; atwo = botha >> 8;
	bone = bothb & 255; btwo = bothb >> 8;
	if (aone < bone) return -1;
	else if (aone > bone) return +1;
	else if (atwo < btwo) return -1;
	else if (atwo > btwo) return +1;	
	else return 0;
} */


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* struct kernpair {	
	int a; int b; float kern;
}; */

int swapcount;			/* debugging */

void swap(char __far *v, int nsize, int i , int j) {
	int c;
	char __far *vi;
	char __far *vj;
	int k;

	swapcount++;
	vi = v + i * nsize; vj = v + j * nsize;
	for (k = 0; k < nsize; k++) {
		c = *vi; *vi = *vj; *vj = (char) c;
		vi++; vj++;
	}
}

void fqsortsub(void __far *v, int nsize, int left, int right, 
/*		int (*compare) (void __far *, void __far *)) { */
/*		int (*compare) (void __far *, int, int, int)) { */
		int (*compare) (const void __far *, int, int, int)) {	/* 92/Nov/24 */
	int i, last;

	if (left >= right) return;
	swap(v, nsize, left, (left + right) /2 );
	last = left;
	for (i = left+1; i <= right; i++)
/*		if ((*compare)((void __far *) ((char __far *) v + i * nsize), 
			(void __far *) ((char __far *) v + left * nsize)) < 0)
			swap(v, nsize, ++last, i); */
		if ((*compare)(v, nsize, i, left) < 0)
			swap(v, nsize, ++last, i);
	swap(v, nsize, left, last);
	fqsortsub(v, nsize, left, last-1, compare);
	fqsortsub(v, nsize, last+1, right, compare);		
}

void fqsort(void __far *v, int nitems, int nsize,
/*		int (*compare) (const void __far *, const void __far *)) {*/
		int (*compare) (const void __far *, int, int, int)) {
	swapcount=0;
	fqsortsub(v, 
		nsize, 
			0, 
				nitems-1, 
					compare);
	if (traceflag != 0)			/* debugging */
		printf("Did %d swaps in presort of kern pairs\n", swapcount);	
}

int comparekernpair(const void __far *arr, int nsize, int i, int j) {
	const struct kernpair __far *kern1;
	const struct kernpair __far *kern2;
	int kern1a, kern1b, kern2a, kern2b;

/*	kern1 = (const struct kernpair __far *) point1;*/
	kern1 = (const struct kernpair __far *) ((char __far *) arr + i * nsize);
/*	kern2 = (const struct kernpair __far *) point2;*/
	kern2 = (const struct kernpair __far *) ((char __far *) arr + j * nsize);

	kern1b= kern1->kppair >> 8; kern1a = kern1->kppair & 255;
	kern2b= kern2->kppair >> 8; kern2a = kern2->kppair & 255;

/*	if (kern1->a < kern2->a) return -1;  */
	if (kern1a < kern2a) return -1; 
/*	else if (kern1->a > kern2->a) return 1; */
	else if (kern1a > kern2a) return 1;
/*	if (kern1->b < kern2->b) return -1; */
	if (kern1b < kern2b) return -1; 
/*	else if (kern1->b > kern2->b) return 1; */
	else if (kern1b > kern2b) return 1;
/*	fprintf(errout, "ERROR: Repeated kern pair %s %s %g (%d %d)\n", */
	fprintf(errout, "ERROR: Repeated kern pair %s %s %d (%d %d)\n", 
/*		encoding[kern1->a], encoding[kern1->b], */  /* 1993/May/3 */
		encoding[kern1a], encoding[kern1b],			/* 1993/May/3 */
/*			kern1->kern, kern1->a, kern1->b); */
			kern1->kpamount, kern1a, kern1b);
	return 0;
}

/* void sortkernpairs(struct kernpair __far *rawkerns, int n) { */
void sortkerns(unsigned char __far *rawkerns, int n) {
	fqsort(rawkerns, 
		n, 
			sizeof(struct kernpair),
				comparekernpair);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* void writekerntable(FILE *fp_out, char *s) { */
/* void writekerntable(FILE *fp_out, unsigned char *s) { */
void writekerntable(FILE *fp_out, unsigned char __far *s) {
	unsigned int k;
	unsigned char chara, charb;
	int badkern = 0, nonzerocount = 0;
	char namea[CHARNAME_MAX], nameb[CHARNAME_MAX];

	if (pairkerninx != 0) {
		kernpairs = readtwo(s);		/* hopefully the same as before ! */
		if (debugflag) printf("Kern pairs %u\n", kernpairs);
/*		if (kernpairs > 4096) { */
		if (kernpairs > 10000) {
			fprintf(errout, "ERROR: Ridiculously many kern pairs (%d)\n",
				kernpairs);
			return;
		}
/*		if (kernpairs > MAXKERNS) {	
			fprintf(errout, "NOTE: Many kern pairs (%d > %d)\n", 
				kernpairs, MAXKERNS); 
		} */ /* not a real limit */
		if (sortflag)						/* 1993/Nov/9 */
			sortkerns(s+2, kernpairs);		/* new, sort on first char */
		fprintf(fp_out, "StartKernData\n");
		if (ignorezerokerns != 0) {
			for(k = 0; k < kernpairs; k++) {
				chara = *(s + 2 + 4 * k);
				charb = *(s + 3 + 4 * k);
				kern = sreadtwo(s + 4 + 4 * k);		/* ? */ 
				if (kern != 0) nonzerocount++;
			}
			fprintf(fp_out, "StartKernPairs %u\n", nonzerocount);
		}
		else fprintf(fp_out, "StartKernPairs %u\n", kernpairs);
		for(k = 0; k < kernpairs; k++) {
			chara = *(s + 2 + 4 * k);
			charb = *(s + 3 + 4 * k);
			kern = sreadtwo(s + 4 + 4 * k);		/* ? */ 
/*			if (traceflag != 0) printf("KPX %c %c %d\n", chara, charb, kern);
				fprintf(fp_out, "KPX %c %c %d\n", chara, charb,	kern); */

			if (ignorezerokerns == 0 || kern != 0) {
				strcpy(namea, encoding[chara]);			/* ??? */
				strcpy(nameb, encoding[charb]);			/* ??? */
				if (strcmp(namea, "") != 0 && strcmp(nameb, "") != 0) 
					fprintf(fp_out, "KPX %s %s %d\n", namea, nameb, kern);
				else {
					badkern++;
					fprintf(fp_out, "KPX %u %u %d\n", chara, charb, kern);
					fprintf(errout, 
						"WARNING: KPX %u %u %d\n", chara, charb, kern);
				}
			}
		}
		fprintf(fp_out, "EndKernPairs\n");
		fprintf(fp_out, "EndKernData\n");
		if (badkern > 0) 
			fprintf(errout, 
				"WARNING: May need to specify correct encoding vector\n");
	}
}

/* convert to lower case letters */
/* void lowercase(unsigned char *s, unsigned char *t) {  */
void lowercase(char *s, char *t) { 
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'A' && c <= 'Z') *s++ = (unsigned char) (c + 'a' - 'A');
		else *s++ = (unsigned char) c;
	}
	*s++ = (unsigned char) c;
}

int lstrncmp(char __far *s, char *name, int len) {	/* 1993/Nov/8 */
	char *t = name;
	while (len-- > 0) if (*s++ != *t++) return 1;		/* no match */
	return 0;											/* match */
}

void lstrcpy(char __far *s, char __far *t) {				/* 1993/Nov/8 */
	int c;
	while ((c = *t++) != 0) *s++ = (char) c;
	*s = '\0';
}

/* convert to upper case letters 
void uppercase(char *s, char *t) { 
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
} */

void removetrailing(char *code) { 
/* void removetrailing(unsigned char *code) {*//* remove trailing space */
/*	unsigned char *s; */
	char *s; 
/*	s = code + strlen(code); */
	s = code + strlen((char *) code);
	while (*(--s) <= ' ') ;		/* skip back over white space */
	*(s+1) = '\0';
}

int	findcomment(unsigned int n) {	/* find encoding vector comment */
	unsigned int k;
	int i;
/*	char *s=buffer; */
/*	unsigned char *s=buffer; */
	unsigned char __far *s=buffer;
	char comment[MAXCOMMENT]="";

	for (k = 0; k < n-10; k++) {
/*		if (strncmp(s++, "Encoding:", 9) == 0) {  */
/*		if (strncmp((char *) s++, "Encoding:", 9) == 0) {  */
		if (lstrncmp((char __far *) s++, "Encoding:", 9) == 0) { 
			s--;
			lstrcpy(comment, (char __far *) s+10);	/* 1993/Nov/8 */
			if (verboseflag != 0)				/* debugging only ? */
/*				printf("Found encoding vector comment: %s", s); */
/*				printf("%s", s); */
				printf("Encoding: %s", comment);	/* 1993/Nov/8 */
/*			s += 10; */
/*			lowercase(s, s); */
			lowercase(comment, comment);		/* 1993/Nov/8 */
/*			removetrailing(s); */
			removetrailing(comment); 			/* 1993/Nov/8 */
/*			printf("Checking: *%s*\n", s);	*/ /* debugging */
/*			for (i = 0; i < encodingcount; i++) { */
			for (i = 0; i < 64; i++) {
				if (strcmp(encodingnames[i][0], "") == 0) break;
				if (strcmp(encodingnames[i][1], "") == 0) break;
/*				if (strcmp(s, encodingnames[i][0]) == 0) { */
/*				if (strcmp((char *) s, encodingnames[i][0]) == 0) { */
/*				if (strcmp(comment, encodingnames[i][0]) == 0) { */
				if (_stricmp(comment, encodingnames[i][0]) == 0) {
					strcpy(vector, encodingnames[i][1]);
					return -1;
				}
			}
/*			fprintf(errout, "\nWARNING: Don't recognize %s\n", s-10); */
			fprintf(errout,
				"\nWARNING: Don't recognize Encoding: %s\n", comment);
			return 0;
		}
	}
	return 0;		/* not found */
}

/* Sep 27 1990 => 1990 Sep 27 */

void scivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 7);
	for (k = 5; k >= 0; k--) date[k+5] = date[k];
/*	date[11] = '\0'; */
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy (date, compiledate);
	scivilize(date);
	fprintf(output, "%s (%s %s)\n", 
		programversion, date, compiletime);
}

/* From Henry McGilton at Trilithon */ /* 1993/March/9 */

char *MakeFamilyNameFromFont(char *FontName) {
    static char FamilyName[FILENAME_MAX];
	unsigned int  FontIndex, NameIndex;
    int  SentSpace = 1;

    for (FontIndex = 0, NameIndex = 0;
			FontIndex < strlen(FontName);  FontIndex ++) {
	if (FontName[FontIndex] == '-') {
	    break;
	}
	if (isupper(FontName[FontIndex])) {
	    if (SentSpace == 0) {
		FamilyName[NameIndex++] = ' ';
		SentSpace = 1;
	    }
	} else {
	    SentSpace = 0;
	}
	FamilyName[NameIndex++] = FontName[FontIndex];
    }
    FamilyName[NameIndex] = '\0';
    return FamilyName;
}

char *MakeFullNameFromFont(char *FontName) {
    static char FullName[FILENAME_MAX];
	unsigned int  FontIndex, NameIndex;
    int  SentSpace = 1;

    for (FontIndex = 0, NameIndex = 0;
			FontIndex < strlen(FontName);  FontIndex ++) {
	if (FontName[FontIndex] == '-') {
	    FullName[NameIndex++] = ' ';
	    SentSpace = 1;
	    continue;
	} else if (isupper(FontName[FontIndex])) {
	    if (SentSpace == 0) {
		FullName[NameIndex++] = ' ';
		SentSpace = 1;
	    }
	} else {
	    SentSpace = 0;
	}
	FullName[NameIndex++] = FontName[FontIndex];
    }
    FullName[NameIndex] = '\0';
    return FullName;
} 


/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindo = "";			/* full file name for dviwindo.ini with path */

/* set up full file name for ini file and check for specified section */
/* e.g. dviwindo = setupinifile ("dviwindo.ini", "[Environment]") */

char *setupinifile (char *ininame, char *section) {	
	char fullfilename[FILENAME_MAX];
	FILE *input;
	char *windir;
	char line[MAXLINE];
	int m;

/*	Easy to find Windows directory if Windows runs */
/*	Or if user kindly set WINDIR environment variable */
/*	Or if running in Windows NT */	
	if ((windir = getenv("windir")) != NULL ||	/* 1994/Jan/22 */
		(windir = getenv("WINDIR")) != NULL ||
		(windir = getenv("winbootdir")) != NULL ||
		(windir = getenv("SystemRoot")) != NULL ||
		(windir = getenv("SYSTEMROOT")) != NULL) { /* 1995/Jun/23 */
		strcpy(fullfilename, windir);
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);
	}
	else _searchenv (ininame, "PATH", fullfilename);

	if (strcmp(fullfilename, "") == 0) {		/* ugh, try standard place */
		strcpy(fullfilename, "c:\\windows");
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);		
	}

/*	if (*fullfilename != '\0') { */
/*	check whether ini file actually has required section */
		if ((input = fopen(fullfilename, "r")) != NULL) {
			m = strlen(section);
			while (fgets (line, sizeof(line), input) != NULL) {
				if (*line == ';') continue;
/*				if (strncmp(line, section, m) == 0) { */
				if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
					fclose(input);
					return _strdup(fullfilename);
				}
			}					
			fclose(input);
		}
/*	} */
	return "";							/* failed, for one reason or another */
}

int setupdviwindo (void) {
	if (usedviwindo == 0) return 0;		/* already tried and failed */
	if (*dviwindo != '\0') return 1;	/* already tried and succeeded */
/*	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");  */
	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");
	if (*dviwindo == '\0') usedviwindo = 0; /* failed, don't try this again */
	return (*dviwindo != '\0');
}

char *grabenvvar (char *varname, char *inifile, char *section, int useini) {
	FILE *input;
	char line[MAXLINE];
	char *s;
	int m, n;

	if (useini == 0 || *inifile == '\0')
		return getenv(varname);	/* get from environment */
	if ((input = fopen(inifile, "r")) != NULL) {
		m = strlen(section);
/* search for [...] section */
		while (fgets (line, sizeof(line), input) != NULL) {
			if (*line == ';') continue;
/*			if (strncmp(line, section, m) == 0) { */
			if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
/* search for varname=... */
				n = strlen(varname);
				while (fgets (line, sizeof(line), input) != NULL) {
					if (*line == ';') continue;
					if (*line == '[') break;
/*					if (*line == '\n') break; */	/* ??? */
					if (*line <= ' ') continue;		/* 95/June/23 */
/*					if (strncmp(line, varname, n) == 0 && */
					if (_strnicmp(line, varname, n) == 0 &&
						*(line+n) == '=') {	/* found it ? */
							fclose (input);
							/* flush trailing white space */
							s = line + strlen(line) - 1;
							while (*s <= ' ' && s > line) *s-- = '\0';
							if (traceflag)  /* DEBUGGING ONLY */
								printf("%s=%s\n", varname, line+n+1);
							return _strdup(line+n+1);
					}							
				}	/* end of while fgets */
			}	/* end of search for [Environment] section */
		}	/* end of while fgets */
		fclose (input);
	}	/* end of if fopen */
/*	useini = 0; */				/* so won't try this again ! need & then */
	return getenv(varname);	/* failed, so try and get from environment */
}							/* this will return NULL if not found anywhere */

char *grabenv (char *varname) {	/* get from [Environment] in dviwindo.ini */
	return grabenvvar (varname, dviwindo, "[Environment]", usedviwindo);
}

#ifdef NEEDATMINI
/* grab setting from `atm.ini' 94/June/15 */

/*	if (useatmini)  setupatmini(); 	*/ /* need to do this before use */

int useatmini = 1;			/* use [Setup] section in `atm.ini' */
							/* reset if setup of atm.ini file fails */

char *atmininame = "atm.ini";		/* name of ini file we are looking for */

char *atmsection = "[Setup]";		/* ATM.INI section */

char *atmini = "";				/* full file name for atm.ini with path */

int setupatmini (void) {
	if (useatmini == 0) return 0;		/* already tried and failed */
	if (*atmini != '\0') return 1;		/* already tried and succeeded */
/*	atmini = setupinifile ("atm.ini", "[Setup]");  */
	atmini = setupinifile (atmininame, atmsection);
	if (*atmini == '\0') useatmini = 0;	/* failed, don't try this again */
	return (*atmini != '\0');
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Avoid stack problems by moving this off the stack */

char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
char driver[FILENAME_MAX], facename[FILENAME_MAX], fontname[FILENAME_MAX];

int doonefile(char *filename) {
	FILE *fp_out;
	int a, b;
	unsigned int k;
	int xll, yll, xur, yur, FontHeight;
	unsigned char __far *sf; 
	char *t;
	unsigned char __far *s;
	int kk, defwidth;

/*	reset to defaults for new file - 1992/Dec/2 */
	ansiflag = 1;		/* non-zero means ANSI encoding */
	oemflag = 0;		/* non-zero means OEM encoding */
	symbolflag = 0;		/* non-zero means symbol encoding */
	decorative = 0;		/* NOT ANSI encoding - `native' encoding */
	texflag = 0;		/* non-zero means TeX font */

	buffer = NULL;

	strcpy(fn_in, filename); 
/*	read PFM file into buffer */
	if (verboseflag != 0) putc('\n', stdout);	/* 1992/June/12 */
	printf("Processing font metric file: %s\n", fn_in);
/*	if (verboseflag != 0) printf("PFM file %s\n", filename); */
	if ((pfmlen = readpfm(fn_in)) == 0) return -1;
	if (*buffer == 128 && *(buffer+1) == 1) {
		fprintf(errout, 
			"This is a PFB file (outlines), not a PFM file (metrics)\n");
/*		continue; */
		return -1;
	}
/*	if (*buffer > 1 || *(buffer+1) > 3) { */
	if (*buffer <= 1 && *(buffer+1) <= 3) { /* should be 0, 1 (version) */
		fontfiddler=0;
	}
	else if (*buffer == 'X' && *(buffer+1) == 'X') {
		printf("FontFiddler Template File\n");
		fontfiddler=1;
	}
	else {
		fprintf(errout, "ERROR: Not a valid PFM file\n");
		return -1;
	}
/*	write output in current directory */
	if ((t=strrchr(fn_in, '\\')) != NULL) t++;
	else if ((t=strrchr(fn_in, '/')) != NULL) t++;
	else if ((t=strrchr(fn_in, ':')) != NULL) t++;
	else t = fn_in;
	strcpy(fn_out, t);  

	forceexten(fn_out, "afm");
	if ((fp_out=fopen(fn_out, "w")) == NULL) {
		perror(fn_out);	exit(3);
	}

/*	crude way to determine whether ansi encoding is used */
	charset = readone(buffer +85);
	if (charset == 0) ansiflag = 1;
	else ansiflag = 0;
	if (charset == 2) symbolflag = 1;
	else symbolflag = 0;
	if (charset == 255) oemflag = 1;
	else oemflag = 0;
		
	code = *(buffer +90);	/* dfPitchAndFamily */
	pitch = code & 15; 
	family = code >> 4;
		
	if (family == 5) decorative = 1;
	else decorative = 0;

	if (reencodeflag == 0) {	/* ignore comment in file if -c used */
		if (findcomment(pfmlen) != 0) {
			reencodeflag = 1;
/*			printf("Will use encoding vector %s\n", vector); */
			(void) readencoding(vector);
		}
	}

	if (reencodeflag == 0) { /*	don't mess up encoding if specified ... */
/*	major changes in following 1992/Feb/29 */
/*	PSCRIPT.DRV rule is: use ANSI unless family == 5 */			
		if (decorative != 0) {
			fprintf(errout, 
"WARNING: font does not use ANSI encoding - specify encoding vector\n");
			setnumericencoding();
		}
/*		else if (texflag != 0) setromanencoding(); */
/*		else if (ansiflag != 0) setansiencoding(); */
/*		else if (symbolflag != 0) setsymbolencoding(); */
		else if (oemflag != 0) setoemencoding();
		else setansiencoding();
	}

	fprintf(fp_out, "StartFontMetrics 2.0\n");
	fprintf(fp_out, 
			"Comment partial AFM file information from PFM file %s\n",
			filename);
	
/*	start decoding PFMHEADER (PFM file header) now */		
/*	PFMHEADER fields are the same as Windows screen fonts file fields */
/*	documented in `MicroSoft Windows Device Driver Adaptation Guide' */

	len = readtwo(buffer +2);			/* dfSize */
	if (debugflag) printf("%s length field %u\n", fn_in, len);
/*	if (pfmlen != len) { */
	if (pfmlen != len && fontfiddler == 0) {
		if (pfmlen > len)
			fprintf(errout,
					"WARNING: Actual length %u > coded length %u\n", 
					pfmlen, len);
		else {
			fprintf(errout, "ERROR: Actual length %u < coded length %u\n", 
					pfmlen, len);
			buffer = (unsigned char __far *) _frealloc(buffer, (unsigned int) len);
			sf = buffer + pfmlen;	/* zero out extension */
			for (k = pfmlen; k < len; k++) *sf++ = 0;
		}
	}
/*	get version (of PFM specification ?) */
	a = *buffer; b = *(buffer+1);	/* dfVersion */
	if (a == 'X' && b == 'X') {		/* FontFiddler Template */
		fprintf(fp_out, "Comment Font Fiddler Template\n");
	}
	else {
		if (verboseflag != 0) printf("Version: %u.%u \n", b, a);
		fprintf(fp_out, "Version %u.%u\n", b, a);		/* ??? */
	}
/*	copyright message next - up to 60 characters - null delimited */
/* 	if (strcmp(buffer +6, "") != 0) { */
	if (*(buffer +6) != 0) {
		lstrcpy(line, (char __far *) buffer+6);			/* 1993/Nov/9 */
/*		if (verboseflag != 0) printf("Notice: %s\n", buffer +6);  */
		if (verboseflag != 0) printf("Notice: %s\n", line); 
/*		fprintf(fp_out, "Notice %s\n", buffer +6);  */ 
/*		fprintf(fp_out, "Comment %s\n", buffer +6);  */
		fprintf(fp_out, "Comment %s\n", line);  
	}
	dftype =readtwo(buffer +66); /* dfType - should be 128 | 1 */
/*	if (dftype != (128 | 1)) */
	if (dftype != (128 | 1) && fontfiddler == 0)
		fprintf(errout, "WARNING: dfType (%d) is not 128 | 1\n", 
			dftype);
	points = readtwo(buffer +68); /* dfPoints */
/*	if (points != 10) */
	if (points != 10 && fontfiddler == 0)
		fprintf(errout, "WARNING: dfPoints (%d) is not 10\n", points);
	vres = readtwo(buffer +70); hres = readtwo(buffer +72); 
/*	if (vres != 300 || hres != 300) */ /* dfVertRes dfHorizRes */
	if ((vres != 300 || hres != 300) && fontfiddler == 0)
		fprintf(errout, "WARNING: dfVertRes %u, dfHorizRes %u not 300\n", 
			vres, hres); 
/*	The PostScript driver assumes dfVertRes == 300 */
	InternalLeading = readtwo(buffer +76);  /* dfInternalLeading - usually 0 */
	ExternalLeading = readtwo(buffer +78); /* dfExternalLeading - sometimes 0 */
	if (verboseflag != 0)
		printf("InternalLeading %u, ExternalLeading %u - ",  /* \n */
			   InternalLeading, ExternalLeading);

	Ascent = readtwo(buffer +74);		/* dfAscent */
	PixWidth = sreadtwo(buffer +86);	/* dfPixWidth */
	if (PixWidth != 0)
		fprintf(errout, "WARNING: PixWidth (%d) is not zero\n", width);
	Height = readtwo(buffer +88);	/* dfPiXHeight */
	Descent = Height - Ascent;		/* an experiment ??? */
	if (verboseflag != 0) 
		printf("Height %u, PixWidth %d, Ascent %u\n", 
			Height, PixWidth, Ascent);

	italic = *(buffer +80); 
	underline = *(buffer +81); 
	strikeout =	*(buffer +82);
	if (underline > 0) 
		if (verboseflag != 0) printf("Underlined font (%u)\n", underline);
	if (strikeout > 0) 
		if (verboseflag != 0) printf("Strikeout font (%u)\n", strikeout);
	if (italic > 0) {
/* 		if (verboseflag != 0) printf("Italic font (%u)\n", italic); */
		if (verboseflag != 0) printf("Style: Italic (%u) ", italic);
	}
	else
		if (verboseflag != 0) printf("Style: Regular (%u) ", italic);		
	weight = readtwo(buffer +83);	/* dfWeight */
	if (weight == 400) {
/*		if (verboseflag != 0) printf("Normal weight (%u) - ", weight); */
		if (verboseflag != 0) printf("Weight: Medium (%u)\n", weight);
		fprintf(fp_out, "Weight Medium\n"); /* Roman, Regular, Book */
	}
		else if (weight > 400) {
/*		if (verboseflag != 0) printf("Bold (%u) - ", weight); */
			if (verboseflag != 0) printf("Weight: Bold (%u)\n", weight);
		fprintf(fp_out, "Weight Bold\n");
	}
	else if (weight < 400) {
/*		if (verboseflag != 0) printf ("Light (%u) - ", weight); */
		if (verboseflag != 0) printf ("Weight: Light (%u) - ", weight);
		fprintf(fp_out, "Weight Light\n");
	}

	charset = readone(buffer +85); /* dfCharSet */
	if (verboseflag != 0) {
		printf("CharSet: ");
		if (charset == 0) printf("ANSI - ");
		else if (charset == 1) printf("DEFAULT - ");
		else if (charset == 2) printf("Symbol - ");
		else if (charset == 128) printf("ShiftJis - ");
		else if (charset == 129) printf("Hangeul - ");
		else if (charset == 136) printf("ChineseBig5 - ");
		else if (charset == 180) printf("Arabic - ");
		else if (charset == 238) printf("Central European - ");
		else if (charset == 255) printf ("OEM - ");
		else printf ("UNKNOWN - ");
	}

/*	THE FOLLOWING DOES NOT ALWAYS WORK CORRECTLY: */
	if (reencodeflag == 0) {
		if (decorative != 0)
			fprintf(fp_out, "EncodingScheme FontSpecific\n");
/*		else if (ansiflag != 0)
			fprintf(fp_out, "EncodingScheme MS Windows ANSI\n");  */
/*		else if (symbolflag != 0)
			fprintf(fp_out,	"EncodingScheme Symbol %% maybe\n"); */
		else if (oemflag != 0)
			fprintf(fp_out, "EncodingScheme IBM OEM\n"); 
/*		else if (standardflag != 0)
			fprintf(fp_out, "EncodingScheme AdobeStandardEncoding\n");  */
/*		else fprintf(fp_out, "EncodingScheme FontSpecific\n");  */
		else fprintf(fp_out, "EncodingScheme MS Windows ANSI\n"); 
	}
	else fprintf(fp_out, "EncodingScheme FontSpecific %% %s\n", vector); 
/*		4-bits of family and 2-bits of Pitch information */
	code = *(buffer +90);	/* dfPitchAndFamily */
	pitch = code & 15; 
	family = code >> 4;
/*		printf("Code %u %u \n", family, pitch); */
/*		if (pitch != 1)  */
	if (pitch != 1 && fontfiddler == 0) 
		fprintf(errout, "WARNING: pitch (%d) <> 1\n", code & 15);
/* Following probably only applies to PCL fonts, for PS fonts, always 1 */
/* DEFAULT_PITCH 0, FIXED_PITCH	1, VARIABLE_PITCH 2 */

/*	THE FOLLOWING DOES NOT ALWAYS WORK CORRECTLY: */
/*	the variable pitch bit seems to always be ON in PS fonts ... */
/*	if (pitch == 0) fprintf(fp_out, "IsFixedPitch true\n"); */
/*	else fprintf(fp_out, "IsFixedPitch false\n"); */
	avwidth = readtwo(buffer +91);	/* sometimes width of `x' */
	MaxWidth = readtwo(buffer +93);
	if (avwidth != 0 || MaxWidth != 0) {
		if (avwidth == MaxWidth) fprintf(fp_out, "IsFixedPitch true\n");
		else fprintf(fp_out, "IsFixedPitch false\n");
/*		maybe base instead on analysis of characters width table ? */
	}

/*	Font Family terms are defined in: */
/*	`MicroSoft Windows Device Driver Adaptation Guide' */
/*	`MicroSoft Windows SDK Tools' page 6-9 */
	if (verboseflag != 0) printf(" Family: ");				/* ??? */
/*	code = code >> 4; */
	if (verboseflag != 0) {
		if (family == 0) printf("Don't Care (Custom) \n");
		else if (family == 1) printf("Roman (Serif) \n");
		else if (family == 2) printf ("Swiss (Sans serif) \n");
/*		else if (family == 3) printf ("Modern (Fixed pitch) \n"); */
		else if (family == 3) printf ("Modern (Fixed stroke width) \n");
		else if (family == 4)  printf ("Script (Cursive) \n");
		else if (family == 5)  printf ("Decorative (NOT ANSI) \n");
		else printf("UNKNOWN code %d\n", family);
	}

/*	dfAvgWidth & dfMaxWidth */
	avwidth = readtwo(buffer +91);	/* sometimes width of `x' */
	MaxWidth = readtwo(buffer +93);
	if (verboseflag != 0) {
		printf("Average width %u - Max width %u - ", avwidth, MaxWidth);
		firstchar = *(buffer +95); 
		defaultchar = *(buffer +97); 
		if (relativeflag) defaultchar += firstchar;
		sf = buffer +117; 
		widthinx = readtwo(sf +6);		/* dfExtentTable - varies */
		s = buffer + widthinx;
		kk = 2 * (defaultchar - firstchar);			/* usually 149 ANSI bullet ... */
		defwidth = sreadtwo(s + kk);		/* default character width */
		printf("Default width %d (%d %s)\n",
			   defwidth, defaultchar, encoding[defaultchar]);
	}

/*	dfFirstChar and dfLastChar */
	firstchar = *(buffer +95); 
	lastchar = *(buffer +96);
	if (verboseflag != 0) printf("Starting char %u, ending char %u, ", 
		firstchar, lastchar);

/*	dfDefaultChar and dfBreakChar */
	defaultchar = *(buffer +97); 
	breakchar = *(buffer +98);
	if (relativeflag != 0) {
		defaultchar += firstchar;	/* often 32 */
		breakchar += firstchar;		/* often 32 (or 0) */
	}
	if (verboseflag != 0) printf("default char %u, break char %u\n", 
	defaultchar, breakchar);
/*	dfWidthBytes -	buffer +99 */

	driverinx = readtwo(buffer +101); /* dfDevice Driver - usually 199 */
/*	face name next - up to 32 characters - null delimited */
	faceinx = readtwo(buffer +105);	  /* dfFace - usually 210 */
/*	dfBitsPointer -	buffer +109 */
/*	dfBitsOffset  -	buffer +113 */
	if (debugflag) printf("Driver at %u, Windows Face Name at %u\n",
						  driverinx, faceinx);

/*	we assume there is no width table between PFMHEADER and PFMEXTENSION */
/*	since only variable width PCL fonts should have one */
/*	PostScript fonts use the extent table for character widths */

/*	now start to decode PFMEXTENSION */

	sf = buffer +117; 
	extsize = readtwo(sf);		/* dfSizeFields - usually 30 */
/*	if (extsize != 30)  */
	if (extsize != 30 && fontfiddler == 0) 
		fprintf(errout, "WARNING: PFM extension size (%d) <> 30\n", 
			extsize);
	if (traceflag != 0) printf("PFM extension size %u (30) - ", 
		extsize);
/*	byte offset in the file of EXTTEXTMETRIC structure */
	extendinx = readtwo(sf +2);		/* dfExtMetricsOffset - usually 147 */
/*	byte offset in file of extent table - used for widths in PS fonts */
	widthinx = readtwo(sf +6);		/* dfExtentTable - varies */
/*	byte offset in file of table of character origins */
	origininx = readtwo(sf +10);	/* only non NULL for screen fonts */
	if (origininx != 0)
		fprintf(errout, "WARNING: orginx (%d) <> 0\n", origininx);
/*	byte offset in file of optional pair-kern table */
	pairkerninx = readtwo(sf +14);		/* dfPairKernTable */
/*	size of table given by etmKernPairs in EXTTEXTMETRIC structure */
/*	if (pairkerninx > 0) {
		if (verboseflag != 0) 
			printf("pair kern table start %u\n", pairkerninx); 
		fprintf(fp_out, "Comment pair kern table start %u\n", 
			pairkerninx); 
	} */

/*	byte offset in file of optional track-kern table */
	trackkerninx = readtwo(sf +18);		/* PairKernTable */
	if (trackkerninx > 0) {
		fprintf(errout, "WARNING: track kern table start %u\n", 
			trackkerninx);
	}

/*	byte offset in file of driver-specific information */
/*	for PostScript drivers this is the PostScript fontname */
	fontnameinx = readtwo(sf +22);  /* dfDriverInfo - varies */

	reserved = readtwo(sf +26);  /* dfReserved - should be null */ 
	if (reserved > 0) {
		fprintf(errout, "WARNING: reserved dword non-zero %u\n", 
			reserved);
	}

/*  now decode EXTTEXTMETRIC - extended metrics */

/*	s = buffer + extendinx;  */
	sf = buffer + extendinx; 
	metsize = readtwo(sf);			/* etmSize - normally is 52 */
	if (metsize != 52 && metsize != 0)
		fprintf(errout, "WARNING: Extended metrics size (%d) <> 52 or 0\n",
			metsize);
	if (traceflag != 0) 
		printf("Extended metrics size %u (52)\n", metsize);

	pointsize = readtwo(sf +2);			/* etmPointSize - ignored */
	orientation = readtwo(sf +4);		/* etmOrientation */
	if (verboseflag != 0) {
		if (orientation == 0) printf("Portrait & Landscape - ");
		else if (orientation == 1) printf("Portrait - ");
		else if (orientation == 2) printf("Landscape - ");
		else printf("UNKNOWN orientation %u - ", orientation);
	}

	maxscale = readtwo(sf +10);	/* etmMaxScale */
	minscale = readtwo(sf +8);	/* etmMinScale */
	if (verboseflag != 0) 
		printf("Nominal PointSize %lg, MaxScale %u, MinScale %u\n",
			(double) pointsize / 20.0, maxscale, minscale);

/* point size is in `twips' - twentieth of a point - usually 240 */

	masterHeight = readtwo(sf +6);	/* etmMasterHeight */
	masterunits = readtwo(sf +12);	/* etmMasterUnits */
/* The PostScript driver assumes etmMasterHeight == 300 */
	if (masterHeight != 300 && masterHeight != 1000)
		fprintf(errout, "WARNING: MasterHeight (%d) <> 300 or 1000\n",
			masterHeight);
/* The PostScript driver assumes etmMasterUnits == 1000 */
	if (masterunits != 1000)
		fprintf(errout, "WARNING: MasterUnits (%d) <> 1000\n",
			masterunits);
	if (verboseflag != 0) 
		printf("MasterHeight = %u, MasterUnits = %u\n",
			masterHeight, masterunits);
	CapHeight = readtwo(sf +14);		/* etmCapHeight - typically of `H' */
	XHeight = readtwo(sf +16);		/* etmXHeight - typically of `x' */
	Ascender = readtwo(sf +18);		/* etmLowerCaseAscender - of `d' */
	Descender = readtwo(sf +20);	/* etmLowerCaseDescender - of `p' */
	if (verboseflag != 0) 
		printf("CapHeight %u, XHeight %u, lc ascend %u, lc descend (-) %u\n",
				CapHeight, XHeight, Ascender, Descender);
	if (CapHeight != 0)
		fprintf(fp_out, "CapHeight %d\n", CapHeight);
	if (XHeight != 0)
		fprintf(fp_out, "XHeight %d\n", XHeight);
	if (Ascender != 0)
		fprintf(fp_out, "Ascender %d\n", Ascender);	
	if (Descender != 0)
		fprintf(fp_out, "Descender %d\n", - (int) Descender);	

/*	italicnum =  *(sf +22) | *(sf +23) << 8;  */
	italicnum = sreadtwo(sf +22);	/* etmSlant (CW in tenth of degree) */

	if (italic && (italicnum == 0)) {
		fprintf(errout, "ERROR: ItalicFlag SET, but ItalicAngle = 0\n");
		fprintf(errout, "       You may want to edit ItalicAngle in AFM file\n");
	}
	if (!italic && (italicnum != 0)) {
		fprintf(errout, "ERROR: ItalicFlag NOT set, but ItalicAngle not equal to 0\n");
		fprintf(errout, "       You may want to edit ItalicAngle in AFM file\n");
	}

	if (italicnum != 0) {
		if (verboseflag != 0) printf("Italic Angle %lg\n", 
			((double) italicnum) / 10.0);
	}
	fprintf(fp_out, "ItalicAngle %lg\n", ((double) italicnum) / 10.0);
/*	supposed to be in degrees CCW from vertical */
	
/*	now for effects metrics */

	underlineposition = readtwo(sf +32);		/* etmUnderlineOffset */
	underlinethickness = readtwo(sf +34);	/* etmUnderlineWidth */
	if (verboseflag != 0) 
		printf("UnderlinePosition (-) %u - UnderlineThickness %u\n", 
			underlineposition, underlinethickness);
	if (underlineposition != 0) 
	fprintf(fp_out, "UnderlinePosition %d\n", - (int) underlineposition);
	if (underlinethickness != 0) 
	fprintf(fp_out, "UnderlineThickness %d\n", underlinethickness);

	strikeoutpos = readtwo(sf +44);			/* etmStrikeOutOffset */
	strikeoutsize = readtwo(sf +46);			/* etmStrikeOutWidth */
	if (verboseflag != 0) 
		printf("StrikeoutPosition %u - StrikeoutSize %u\n", 
			strikeoutpos, strikeoutsize);
	doubleupperpos = readtwo(sf +36);   /* etmDoubleUpperUnderlineOffset */
	doubleuppersize = readtwo(sf +40);  /* etmDoubleUpperUnderlineWidth */
	if (verboseflag != 0)
		printf("DoubleupperPosition (-) %u - DoubleupperSize %u\n", 
			doubleupperpos, doubleuppersize);
	
	doublelowerpos = readtwo(sf +38);   /* etmDoubleLowerUnderlineOffset */
	doublelowersize = readtwo(sf +42);  /* etmDoubleLowerUnderlineWidth */
	if (verboseflag != 0) 
		printf("DoublelowerPosition (-) %u - DoublelowerSize %u\n", 
			doublelowerpos, doublelowersize);
	
/*	superscriptpos =  *(sf +24) | *(sf +25) << 8;  */
	superscriptpos = sreadtwo(sf +24);	/* etmSuperScript (< 0) */
	superscriptsize = readtwo(sf +28);	/* etmSuperScriptSize */
	if (verboseflag != 0) 
		printf("SuperscriptPosition (-) %d - SuperscriptSize %u\n", 
			superscriptpos, superscriptsize);
	subscriptpos = sreadtwo(sf +26);		/* etmSubScript (> 0) */
	subscriptsize = readtwo(sf +30);		/* etmSubScriptSize */
	if (verboseflag != 0) 
		printf("SubscriptPosition (-) %u - SubscriptSize %u\n", 
			   subscriptpos, subscriptsize);
		
	kernpairs = readtwo(sf +48);			/* etmKernPairs */
	if (verboseflag != 0) printf("Kern pairs %u - ", kernpairs);
	else if (debugflag) printf("Kern pairs %u\n", kernpairs);

	kerntracks = readtwo(sf +50);			/* etmKernTracks */
	if (kerntracks != 0)
		fprintf(errout, "WARNING: track kerning (%d)\n", kerntracks);
	if (verboseflag != 0) printf("Kern tracks %u\n", kerntracks);
	
/*	now for the embedded strings */
			
/*	s = buffer + driverinx; */ /* 199 */
/*	strcpy(driver, buffer + driverinx); */
/*	strcpy(driver, (char *) (buffer + driverinx)); */
	lstrcpy(driver, (char __far *) (buffer + driverinx));
	if (verboseflag != 0) printf("Printer driver type: %s \n", driver);
		
/*	s = buffer + faceinx;  */
/*	strcpy(facename, buffer + faceinx); */
/*	strcpy(facename, (char *) (buffer + faceinx)); */
	lstrcpy(facename, (char __far *) (buffer + faceinx));
	if (verboseflag != 0) 
		printf("Face (MS Windows Name): %s \n", facename);
		
/*	s = buffer + fontnameinx; */
/*	strcpy(fontname, buffer + fontnameinx); */
/*	strcpy(fontname, (char *) (buffer + fontnameinx)); */
	lstrcpy(fontname, (char __far *) (buffer + fontnameinx));
	if (verboseflag != 0) 
		printf("DriverInfo (PostScript FontName): %s\n", fontname);
/* no \n */
/*	fprintf(fp_out, "Comment MS-WindowsName %s\n", facename); */
	fprintf(fp_out, "Comment MSMenuName %s\n", facename);	/* 92/Dec/8 */
	fprintf(fp_out, "FontName %s \n", fontname);

/*	THE FOLLOWING DOES NOT ALWAYS WORK CORRECTLY: */

	fprintf(fp_out, "FamilyName %s \n", MakeFamilyNameFromFont(fontname)); 
	fprintf(fp_out, "FullName %s \n", MakeFullNameFromFont(fontname));
/*	now for the character width table */

/*	fprintf(fp_out, "FontBBox %d %d %d %d\n",
		0, - (int) Descender, MaxWidth, Ascender); */

/* Following according to information from Terry O'Donnell 1993/June/12 */
	xll = 0; xur = MaxWidth;
	if (InternalLeading > 0) FontHeight = InternalLeading + 1000;
	else FontHeight = Height;
	yll = Ascent - InternalLeading - 1000;
	yur = yll + FontHeight;
	fprintf(fp_out, "FontBBox %d %d %d %d\n", xll, yll, xur, yur);

/*	s = buffer + widthinx; */
	fprintf(fp_out, 
"Comment Character Bounding Boxes are merely approximations (use PFAtoAFM)\n");
	if (debugflag) printf("Width table %u - %u (char %d - char %d)\n",
						  widthinx,
						  widthinx + (lastchar - firstchar + 1) * 2,
						  firstchar, lastchar);
	writewidthtable(fp_out, buffer + widthinx);

/*	now for the kerning table */

	if (debugflag) printf("Kern table %u - %u (%u pairs)\n", pairkerninx,
						  pairkerninx + 2 + kernpairs * sizeof(KERNPAIR),
						 kernpairs);
	writekerntable(fp_out, buffer + pairkerninx);
	
	fprintf(fp_out, "EndFontMetrics\n");
	if (debugflag) printf("PFM file length %u\n", pfmlen);

	if (ferror(fp_out) != 0) {
		perror(fn_out);	
		exit(3);
	}
	else fclose(fp_out);
	if (buffer != NULL) _ffree(buffer);
	buffer = NULL;
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	char *s, *t;
	int firstarg, m;

	firstarg = commandline(argc, argv, 1);

	if (firstarg > argc - 1) showusage(argv[0]);

/*	if (argc < firstarg + 1) return 0; */	/* no arguments */

/*  if -n arg used - compare bytes at specified position in multiple files */

	for (m = 0; m < MAXCHRS; m++) encoding[m] = "";	/* initialize */

	strncpy(programpath, argv[0], sizeof(programpath));
	removepath(programpath);
/*	printf("Default program path is %s\n", programpath); */
/*	if programpath exists, use as default for vec */
/*	if (strcmp(programpath, "") != 0) vectorpath = programpath; */
	
	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((t = getenv("VECPATH")) != NULL) vectorpath = t; 
	else if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((t = grabenv("VECPATH")) != NULL) vectorpath = t;
	}

	if (reencodeflag != 0) {
/*		(void) readencoding(vector); */
		(void) readencoding(commandvector);
/*		if (readencoding(vector) != 0) {
			fprintf(errout, "WARNING: can't find encoding vector (%s)\n",
				vector); getch();
		} */
	}

/*	scivilize(compiledate);	 */
	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);
	putc('\n', stdout);

	if (position >= 0) {
 /* comparison of bytes at same position in multiple files */
		(void) comparative(firstarg, argc, argv);
		return 0;
	}

/*	NOT comparison of bytes at same position in multiple files */
	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASHED %lu", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	initreencodeflag = reencodeflag;			/* remember 95/Dec/31 */

	for (m = firstarg; m < argc; m++) {		
		reencodeflag = initreencodeflag;	/* reset 95/Dec/31 */
		strcpy(vector, commandvector);		/* command line spec if any */
		doonefile(argv[m]);
	}
	cleanencoding();
	if (buffer != NULL) _ffree(buffer);
	if (argc - firstarg > 1)
		printf("Processed %d font metric files\n", argc - firstarg);
	return 0;
}

/* does nothing about track kerning - since no application uses this */

/* this progam also provides a way of probing a binary file */
/* give it a number using command line argument -n */
/* it will show file names */
/* byte n as number, bytes n & (n +1) as number, bytes n & (n+1) & (n + 2) */
/* given multiple file arguments to see comparison of the n-th byte of each */
/* useful for comparative anatomy of things like PFM files ... */

/*	see also TEXTMETRIC (section 7-59, SDK Reference 2) ? */
/*  see also GetDeviceCaps and GetTextMetrics ? */

/* MISSING from PFM format: full listing of character encoding */
/* Also font has been reencoded - usually to ANSI (MS Windows) */
/* Should translate to AdobeStandardEncoding if possible ? */

/* MISSING from PFM format: ligatures */
/* MISSING from PFM format: character bounding boxes */
/* MISSING from PFM format: unencoded character information */
/* MISSING from PFM format: composite character information */

/* Also, all kern pairs are sorted on second letter instead of first */

/* FamilyName ? */		/* Version ? */		/* EncodingScheme ? */	
/* FullName ? */		/* FontBBox ? */

/* SET VECPATH=C:\DVIPSONE	before use */

/* Font Families */
/* FF_DONTCARE	(0<<4) */	/* Don't care or don't know. */
/* FF_ROMAN		(1<<4) */	/* Variable stroke width, serifed. */
							/* Times Roman, Century Schoolbook, etc. */
/* FF_SWISS		(2<<4) */	/* Variable stroke width, sans-serifed. */
							/* Helvetica, Swiss, etc. */
/* FF_MODERN   (3<<4) */	/* Constant stroke width, serifed or sans-serifed. */
							/* Pica, Elite, Courier, etc. */
/* FF_SCRIPT	(4<<4) */	/* Cursive, etc. */
/* FF_DECORATIVE (5<<4)  */ /* Old English, etc. */

/* Font Weights */
/* FW_DONTCARE	    0 */
/* FW_THIN			100 */
/* FW_EXTRALIGHT	200 */
/* FW_LIGHT			300 */
/* FW_NORMAL	    400 */
/* FW_MEDIUM	    500 */
/* FW_SEMIBOLD	    600 */
/* FW_BOLD			700 */
/* FW_EXTRABOLD	    800 */
/* FW_HEAVY			900 */

/* ANSI_CHARSET		0 */
/* DEFAULT_CHARSET  1 */
/* SYMBOL_CHARSET	2 */
/* SHIFTJIS_CHARSET	128 */
/* HANGEUL_CHARSET  129 */
/* CHINESEBIG5_CHARSET 136 */
/* ARABIC_CHARSET	180 */
/* CE_CHARSET ???	238 */
/* OEM_CHARSET		255 */

/* split up main - it is much too long */

/* if (width == 1) width = 0; */		/* complement to afmtopfm ??? */

/* EM = 1000 */		/* InternalLeading non-negative */
/* FontHeight = FontBBox.yur - FontBBox.yll;
/* InternalLeading = max(0, FontHeight - EM) */
/* PFM Ascent = EM + FontBBox.yll + InternalLeading */

/* So PFM Ascent = FontBBox.yur		if  FontHeight >= EM */
/* So PFM Ascent = FontBBox.yll + EM	if  FontHeight <  EM */

/* allocation/freeing once per file, not once per job */
