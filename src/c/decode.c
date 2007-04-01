/* Copyright 1990, 1991, 1992 Y&Y
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

/* Try and guess encoding of TFM, PFM, and PFB files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>		/* for dos_find... */
/* #include <malloc.h> */

/* #include <assert.h> */
/* #include <conio.h> */

#define MAXCHRS 256
#define MAXAFMS 512
#define MAXCHARNAME 32
/* #define FNAMELEN 128 */
#define FONTNAME_MAX 64 
#define MAXVECTOR 64
/* #define MAXPATHLEN 64 */
#define MAXLINE 256
/* #define MAXBUFFER 2048 */
#define MAXBUFFER 4096

/* We assume the good stuff occurs in the first 2048/4096 bytes ... */

/* char *programversion = "Encoding guessing program version 0.9"; */
/* char *programversion = "Encoding guessing program version 1.0"; */
/* char *programversion = "Encoding guessing program version 1.0.1"; */
/* char *programversion = "Encoding guessing program version 1.0.2"; */
/* char *programversion = "Encoding guessing program version 1.1"; */
char *programversion = "Encoding guessing program version 1.2";

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

struct ratio { /* structure for rational numbers */
	long numer;
	long denom;
};

/* #define COPYHASH 14448919 */

/* #define COPYHASH 15411551 */

#define COPYHASH 4380384

/* Copyright (C) 1993, Y&Y, Inc. All rights reserved."; */

/*static char *copyright = "\
Copyright (C) 1993-1994, Y&Y, Inc. All rights reserved."; */

static char *copyright = "\
Copyright (C) 1993-1995, Y&Y, Inc. All rights reserved.";

int wantcpyrght=1;		/* want copyright message in there */

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int checkflag = 0;		/* check whether windows is running */
int resetatm = 0;
int afmflag = 0;
int winflag = 0;
int showfaceflag = 0;		/* 1995/July/22 */
int vectorflag = 0;
int fontflag = 0;

int useafm = 0;			/* use specified AFM file */

int usewin = 0;			/* process ATM.INI file - modify face names */

int facex = 0;			/* in conjunction with usewin */
						/* non-zero => add `X', zero => remove 'X' */

int rationalize = 0;
int tweakflag = 0;

char *vector="";

char *vectorpath = "";	/*  path for vectors */

char programpath[FILENAME_MAX] = "";

char *afmfile="";

char *winfile="";

char *PFM_Dir="";			/* default c:\psfonts\pfm */

char *PFB_Dir="";			/* default c:\psfonts */

int	lf, lh, bc, ec, nw, nh, nd,	ni, nl, nk, ne, np;

unsigned long checksum;

unsigned long checkdefault = 0x59265920;	/* default signature */

double designsize;

char checkvector[8]="";		/* vector retrieved from checksum */

/* char codingscheme[40]; */		/* Xerox PARC header data */
char codingscheme[MAXVECTOR];		/* may be read from vector file */
char fontid[20];			/* Xerox PARC header data */

unsigned long faceword=0;	/* Xerox PARC header data */ /* NA */
int facebyte=0;				/* Xerox PARC header data */

int charstart, widthstart;
/* int  heightstart, depthstart, italicstart; */
/* int ligkernstart, kernstart, extenstart, paramstart; */

char codingvector[20]="";	/* which coding scheme in use */

/* char *defaultvec="textext"; */		/* default encoding vector */

char *defaultvec="";		/* default encoding vector */

char *encodingnames[][2] = {
{"tex text", "textext"},				/* cmr*, cmb*, cmsl*, cmss* */
/*		or			"texital"		for			cmti* */
{"tex text without f-ligatures", "textype"},	/* cmcsc10 & cmr5 */
{"tex typewriter text", "typewrit"},		/* cm*tt* */
/*		or			"typeital"		for			cmitt* */
{"tex extended ascii", "texascii"},			/* cmtex* */
{"tex math italic", "mathit"},				/* cmmi* */
{"tex math symbols", "mathsy"},				/* cmsy* */
{"tex math extension", "mathex"},			/* cmex10 */
/* {"ascii caps and digits", "textext"},*/	/* cminch */
{"tex text in adobe setting", "textext"},	/* pctex style ? */
{"adobe standardencoding", "standard"},
{"adobestandardencoding", "standard"},
{"adobe symbol encoding", "symbol"},
{"adobe dingbats encoding", "dingbats"},
{"microsoft windows ansi 3.0", "ansi"},
{"microsoft windows ansi 3.1", "ansinew"},
{"microsoft windows ansi", "ansi"},
{"microsoft windows new ansi", "ansinew"},
{"tex ansi windows 3.0", "texansi"},
{"tex ansi windows 3.1", "texannew"},
{"ventura publisher encoding", "ventura"},
{"tex 256 character encoding", "tex256"},
{"tex extended text -- latin", "tex256"},			/* 1993/July/20 */
{"macintosh", "mac"},
{"cyrillic", "cyrillic"},
{"dvips new", "neonnew"},							/* 1992/Dec/22 */
{"tex text + adobestandardencoding", "neonnew"},	/* 1992/Dec/22 */
{"ibm oem", "ibmoem"},								/* 1993/May/31 */
{"tex typewriter and windows ansi", "texnansi"},	/* 1994/dec/31 */
{"", ""}
 };	
  
int ansiflag = 1;		/* non-zero means ANSI encoding */
						/* otherwise use symbol/decorative flags */
int oemflag = 0;		/* non-zero means OEM encoding */
int symbolflag = 0;		/* non-zero means symbol encoding */
int decorative = 0;		/* NOT ANSI encoding - `native' encoding */
 
int remapflag=0;		/* on if font appears to be remapped */

char driver[FONTNAME_MAX], facename[FONTNAME_MAX], fontname[FONTNAME_MAX];

char line[MAXLINE];						/* 256 */

unsigned char buffer[MAXBUFFER];		/* 4k */

double afmwidths[MAXCHRS];				/* 2k */ /* from AFM */

double binwidths[MAXCHRS];				/* 2k */ /* from TFM or PFM */

/* char encoding[MAXCHRS][MAXCHARNAME]; */	/* 8k */

char *encoding[MAXCHRS];				/* < 8k */

/* use more efficient scheme for above ? see DVIPSONE */

struct charmetric {		
/*	char charname[MAXCHARNAME]; */
	char *charname;							/* 95/May/15 */
	double charwidth;
};	 /* structure for AFM data */

/* struct charmetric afmdata[MAXAFMS]; */	/* 40 * 512 = 20k */

struct charmetric afmdata[MAXAFMS]; 	/* 12 * 512 = 6144 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following is the tfm to afm conversion stuff */

int readint(int k) {	 /* read 16-bit count from buffer */
	int a, b, k2= k << 1, res;
	a = buffer[k2]; b = buffer[k2+1];
/*	assert (a >= 0); assert (b >= 0); */
	res =  (a << 8) | b;
	return res;
}

long readlong(int k) {		/* read 32-bit number from buffer */
	int a, b, c, d, k4 = k << 2;
	long res;
	a = buffer[k4]; b = buffer[k4+1]; c = buffer[k4+2];  d = buffer[k4+3]; 
/*	assert (a >= 0); assert (b >= 0); assert(c >= 0); assert(d >= 0); */
	res = ((long) a) << 24 | ((long) b) << 16 | ((long) c << 8) | d;
	return res;
}

#define DENOMLIM 3600
#define NUMERLIM 3600

/* compute good rational approximation to floating point number */
/* assumes number given is positive */ /* number theory at work! */

struct ratio rational(double x, long nlimit, long dlimit) {
	long p0=0, q0=1, p1=1, q1=0, p2, q2;
	double s=x, ds;
	struct ratio res;

/*	if (showratio != 0) 
		printf("Entering rational %g %ld %ld\n", x, nlimit, dlimit);  */
	
	if (x < 0.0) {		/* negative */
		res = rational(- x, nlimit, dlimit);
		res.numer = - res.numer;
		return res;
	}

	if (x == 0.0) {
			res.numer = 0; res.denom = 1;
			return res;		/* the answer is 0/1 */
	}
	for(;;) {
		p2 = p0 + (long) s * p1;
		q2 = q0 + (long) s * q1;
/*		if (showratio != 0) 
		  printf("%ld %ld %ld %ld %ld %ld %g\n", p0, q0, p1, q1, p2, q2, s); */
		if (p2 > nlimit || q2 > dlimit || p2 < 0 || q2 <= 0) {
			p2 = p1; q2 = q1; break;
		}
		if ((double) p2 / (double) q2 == x) break;
		ds = s - (double) ((long) s);
		if (ds == 0.0) break;
		p0 = p1; q0 = q1; p1 = p2; q1 = q2;	s = 1/ds;
/*		catch large s that will overflow when converted to long */
		if (s > 1000000000.0 || s < -1000000000.0) break; /* 1992/Dec/6 */
	}
	res.numer = p2; res.denom = q2;
/*	if (showratio != 0) 
	printf(" return with %ld/%ld = %lg\n", p2, q2, (double)p2/(double)q2);*/
	return res;		/* the answer is p2/q2 */
}

double unmaplong(long n) {		/* map from tfm scaled integer */
	double res;
	struct ratio rats;

	if (n == 0) return 0.0;
	else if (n < 0) return - unmaplong( - n);
	else {
		if (tweakflag != 0) n = n - 2;		/* ATTEMPT TO CORRECT */
/*		res = ((double) n) * 1000.0 / 1048576.0; */
		res = ((double) n) / 1048576.0; 
		if (rationalize != 0) {
			rats = rational(res, (long) DENOMLIM, (long) NUMERLIM);
			res = (double) rats.numer / (double) rats.denom;
		}
		res = res * 1000.0; 
/*		res = ((double) ((long) (res * 1000.0))) / 1000.0; */
		res = ((double) ((long) (res * 1000.0 + 0.5))) / 1000.0;
		return res;
	}
}

double unmapread(int k) {
	return unmaplong(readlong(k));
}

int readtfm(FILE *input) {
	int c;
	unsigned char *s = buffer; /* new ??? */
	
	while ((c = getc(input)) != EOF) {
		*s++ = (unsigned char) c;
		if (s >= buffer + MAXBUFFER-1) {
/*			fprintf(stderr, "TFM file too large (%d > %d)\n", 
				(s - buffer), MAXBUFFER); */
/*			exit(13); */
			return MAXBUFFER;
		}
	}
	return (s - buffer);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following is the PFM to AFM conversion stuff */

int readpfm(FILE *input) {  /* read the PFM file into the buffer */
	unsigned int k=0;
	unsigned char *s=buffer;	
	int c;

	while ((c = getc(input)) != EOF) {
		*s++ = (unsigned char) c;
		if (s >= buffer + MAXBUFFER) {
			return MAXBUFFER;
		}
	}
	return(s - buffer);
}

void lowercase(char *s, char *t) { 
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'A' && c <= 'Z') *s++ = (unsigned char) (c + 'a' - 'A');
		else *s++ = (unsigned char) c;
	}
	*s++ = (unsigned char) c;
}

void uppercase(char *s, char *t) { /* convert to upper case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

void readheader(void) { /* see whether Xerox PARC style header */
	int k, nlen;

	char *s;

	s = codingscheme; 
	nlen = buffer[24 + 8];		/* step over counts two header words */
	for (k = 0; k < nlen; k++)	*s++ =  buffer[24 + 8 + 1 +k];
	*s = '\0';
/*	if (strcmp(codingscheme, "UNSPECIFIED") == 0) 
		strcpy(codingscheme, "");	*/

	s = fontid; 
	nlen = buffer[24 + 8 + 40];	/* step over counts and header words */
	for (k = 0; k < nlen; k++)	*s++ = buffer[24 + 8 + 40 + 1 + k];
	*s = '\0';
	if (strcmp(fontid, "UNSPECIFIED") == 0) 
		strcpy(fontid, "");					/* 1993/Feb/9 */

	faceword = (unsigned long) readlong(6 + 2 + 10 + 5);	/* ??? */
/*	facebyte = buffer[24 + 8 + 40 + 20]; */ /* ??? */
/*  facebyte = faceword & 255; */	
}

/* Mac style code: 1 - bold, 2 - italic, 4 - underline, 8 - outline */
/* 16 - shadow, 32 - condensed, 64 - extended, 128 - reserved */

/* Encoding: 0 - none, 1 TeX text, 2 TeX type, 4 Lucida Math */

void showcode(unsigned int code) {
	if (code == 0) printf("None ");
	else if ((code & 3) == 1) printf("TeX text ");
	else if ((code & 3) == 2) printf("TeX type ");	
	else if ((code & 4) != 0) printf("Lucida Math ");	
	else printf("Other %d ", code);
}

void macfaceword(unsigned long face) {	/* analyze TeXtures style faceword */
	unsigned int qdcode, pscode;
	putc('\n', stdout);
	if ((face & 1) != 0) printf("BOLD ");
	if ((face & 2) != 0) printf("ITALIC ");
	qdcode = (unsigned int) (face >> 24) & 255;
	pscode = (unsigned int) (face >> 16) & 255;
	if (qdcode != 0 || pscode != 0) {
		printf("QD code: "); showcode(qdcode); 
		printf("PS code: "); showcode(pscode);
	}
	putc('\n', stdout);
}

double epsilon=0.99999;		/* max allowed error in character width */

int afmmatch;

int fixedwidth;

int checkremap(int bc, int ec) {
	int k;
	double allwidth;

	fixedwidth = 1;
	allwidth = binwidths[0];
	for (k = bc; k <= ec; k++) {
		if (binwidths[k] != allwidth) {
			fixedwidth = 0; break;
		}
	}

	if (bc > 0 || ec < 196) return 0;
	
	for (k = 0; k < 10; k++) {
		if (binwidths[k] != binwidths[k+161]) {
			remapflag = 0; return 0;
		}
	}
	for (k = 10; k < 32; k++) {
		if (binwidths[k] != binwidths[k+163]) {
			remapflag = 0; return 0;
		}
	}
	return 1;	/* yes, appears to be remapped */
}

int checkmatch (void) {			/* check if matches AFM widths */
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], "") == 0) continue;
/*		if (strcmp(encoding[k], "") != 0) */
/*		now compare binwidths[k] with afmwidths[k] */ 
		if (binwidths[k] < afmwidths[k] - epsilon ||
			binwidths[k] > afmwidths[k] + epsilon) {
			if (traceflag != 0) {
				printf("k %d %s charwidth %lg widths[k] %lg\n",
					k, encoding[k], binwidths[k], afmwidths[k]);
			}	
			return 0;			/* something did not match */
		}
	}
	return 1;					/* everything matched */
}

void sayremap(int remapflag, int fixedwidth) {
	if (fixedwidth != 0) printf("Fixed width font\n");
	if (remapflag != 0)
		printf("Using remap (0-9 => 161-170, 10-32 => 173-195, 127 => 196)\n");
}

/* Takes first six letters of encoding vector name and compresses */
/* into four bytes using base 40 coding */ /* 40^6 < s^32 */
/* Treats lower case and upper case the same, ignores non alphanumerics */
/* Except handles -, &, and _ */

int decodefourty(unsigned long checksum, char *codingvector) {
	int c;
	int k;
/*	char codingvector[6+1]; */

/*	if (checksum == checkdefault) { */
	if (checksum == 0) {
		strcpy(codingvector, "unknown");
		return 1;
	}
	else if ((checksum >> 8) == (checkdefault >> 8)) {	/* last byte random */
		strcpy (codingvector,  "native");		/* if not specified ... */
		return 1;
	}
	else {
		for (k = 0; k < 6; k++) {
/*			c = (int) (checksum % 36); */
			c = (int) (checksum % 40);
/*			checksum = checksum / 36; */
			checksum = checksum / 40;
			if (c <= 'z' - 'a' ) c = c + 'a';
			else if (c < 36) c = (c + '0') - ('z' - 'a') - 1;
			else if (c == 36) c = '-';
			else if (c == 37) c = '&';
			else if (c == 38) c = '_';
			else c = '.';				/* unknown */
			codingvector[5-k] = (char) c;
		}
		codingvector[6] = '\0';
	}
/*	printf("Reconstructed vector %s\n", codingvector); */
/*	return strdup(codingvector); */
	return 0;
}

int analyzetfm (FILE *input, char *filename) {
	int k;
	int windex;
	long charinfo;
	double charwidth;
/*	long lcharwidtha, lcharwidthb; */

	readtfm(input);

	if (ferror(input) != 0) {
		fprintf(stderr, "Error in input file ");
		perror(filename);	return -1;
	}

	if (traceflag != 0) 
		printf("Now decoding TFM information\n");


/* read header stuff words */

	lf = readint(0); lh = readint(1); bc = readint(2); ec = readint(3);
	nw = readint(4); nh = readint(5); nd = readint(6); ni = readint(7);
	nl = readint(8); nk = readint(9); ne = readint(10); np = readint(11);

	if (traceflag != 0) printf(
"lf %d lh %d bc %d ec %d nw %d nh %d nd %d ni %d nl %d nk %d ne %d np %d \n",
				lf, lh, bc, ec, nw, nh, nd, ni, nl, nk, ne, np);

	if (lf != 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np
		|| ne > 256) {
		fprintf(stderr, 
			"Inconsistent start of TFM file (lengths don't add up)\n");
		return -1;
	}
	charstart = 6 + lh;
	widthstart = charstart + (ec - bc + 1);
/*	heightstart = widthstart + nw; */
/*	depthstart = heightstart + nh; */
/*	italicstart = depthstart + nd; */
/*	ligkernstart = italicstart + ni; */
/*	kernstart = ligkernstart + nl; */
/*	extenstart = kernstart + nk; */
/* 	paramstart = extenstart + ne; */
/*	assert (paramstart + np == lf); */

	if (verboseflag != 0)
		printf("Starting char %d, ending char %d\n", bc, ec);

	checksum = readlong(6);						/* 95/Jan/7 */
	designsize = unmapread(6 + 1)/1000.0;  

	if (decodefourty(checksum, checkvector) == 0) {
		printf("Partial encoding vector from checksum: %s..\n", checkvector);
	}

/*	lowercase(fontname, fontname); */
/*	italicflag = isititalic(fontname);  */

	if (lh >= 16) { /* is it Xerox PARC style header ? */
		readheader();
/*		if (useafm == 0) {  */
		if (strncmp(codingscheme, "PostScript ", 11) == 0) {
			printf("TeXtures PS FontName %s\n         QD fontname %s ", 
				codingscheme+11, fontid);
			macfaceword(faceword);
		}
		else if (strncmp(codingscheme, "PS ", 3) == 0) {
			printf("TeXtures PS FontName %s\n         QD fontname %s ", 
				codingscheme+3, fontid);
			macfaceword(faceword);
		}
		else {
/*			printf("`%s' claims to use `%s' encoding\n", */
/*			printf("`%s' claims to use `%s'\n", filename, codingscheme); */
			printf("Claims to use `%s' coding scheme\n", codingscheme);
			lowercase(line, codingscheme);
			strcpy(codingvector, "");
			for (k = 0; k < 64; k++) { 
				if (strcmp(encodingnames[k][0], "") == 0) break;
				if (strcmp(encodingnames[k][1], "") == 0) break;
				if (strcmp(line, encodingnames[k][0]) == 0) {
					strcpy(codingvector, encodingnames[k][1]); break;
				}
			}
			if (strcmp(codingvector, "") == 0) {
				printf("Sorry, don't recognize coding scheme `%s'\n",
					codingscheme);
				strcpy(codingvector, defaultvec); /* default */
			}
		}
		if (strcmp(codingvector, "") != 0) 
/*			printf("`%s' claims to use `%s' encoding vector\n",
				filename, codingvector);  */
			printf("Claims to use `%s' encoding vector\n", codingvector); 
/*		} */
	} 
	else {
/*		if (useafm == 0) */
			printf("Sorry, not Xerox style header (only %d words)\n", lh);
	}

/*	if (useafm == 0) { */
	if (traceflag != 0) printf("TFM file has %d parameters\n", np);
	if (np == 6) {
		if (verboseflag != 0) {
			printf("WARNING: Only 6 Params (OK for math italic)\n");
			printf("WARNING: ExtraSpace parameter missing\n");
		}
	}
	else if (np < 7) {
		if (verboseflag != 0) 
			printf("WARNING: Fewer than 7 Params (%d)\n", np); 
	}
/*	readparams(); */
	if (np == 13){
		if (verboseflag != 0) 
			printf("WARNING: 13 Params (OK for math extension)\n");
	}
	else if (np == 22) {
		if (verboseflag != 0) 
			printf("WARNING: 22 Params (OK for math symbol)\n");
	}
	else if (np > 7)  {
		if (verboseflag != 0) 
			printf("WARNING: More than 7 Params (%d)\n", np);
	}

	if (traceflag != 0) printf("designsize = %lg\n", designsize);
	
	if (designsize < 5 || designsize > 17.5) {
		if (verboseflag != 0) 
			printf("WARNING: Unusual design size (%lg)\n", designsize);
	}

	if (verboseflag != 0 && traceflag != 0) printf("\n");
/*	} */

/*	now extract char widths */

	for (k = 0; k < MAXCHRS; k++) binwidths[k] = 0.0;

	for (k = bc; k <= ec; k++) {
		charinfo = readlong(charstart + k - bc);
/*		if (charinfo == 0) continue;  */
		windex = (int) (charinfo >> 24) & 255;
		if (windex >= 0) charwidth = unmapread(widthstart + windex);
		else charwidth = 0.0; 
		binwidths[k] = charwidth;
	}

/*  check whether TFM perhaps remapped --- shouldn't happen! */
	remapflag = checkremap (bc, ec);
	sayremap(remapflag, fixedwidth);
/*  see whether matches AFM file encoding */
	if (useafm != 0) {
		afmmatch = checkmatch ();
		if (afmmatch != 0)
			printf("Encoding appears to match that in AFM file\n");
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned int readone(unsigned char *s) {
	return (unsigned int) *s;
}

unsigned int readtwo(unsigned char *s) {
	return ((unsigned int) *s) | (((unsigned int) *(s+1)) << 8);
}

unsigned long readthree(unsigned char *s) {
	return *s | (((unsigned int) *(s+1)) << 8) 
		| ((unsigned long) *(s+2) << 16);
}

unsigned long readfour(unsigned char *s) {
	return *s | (((unsigned int) *(s+1)) << 8) | 
		((unsigned long) *(s+2) << 16) | ((unsigned long) *(s+3) << 24);
}

int sreadtwo(unsigned char *s) {
	return ((int) *s) | (((int) *(s+1)) << 8);
}

void removetrailing(char *code) {	/* remove trailing white space */
	char *s; 
	s = code + strlen(code); 
	while (*(--s) <= ' ') ;		/* skip back over white space */
	*(s+1) = '\0';
}

int	findcomment(unsigned int n) {	/* find encoding vector comment */
	unsigned int k;
	int i;
	unsigned char *s=buffer;

	for (k = 0; k < n-10; k++) {
/*		if (strncmp(s++, "Encoding:", 9) == 0) {  */
		if (strncmp((char *) s++, "Encoding:", 9) == 0) { 
			s--;
			if (traceflag != 0)	/* debugging only ? */
/*				printf("Found encoding vector comment: %s", s); */
				printf("%s", s);
			s += 10;
			strncpy(codingscheme, (char *)s, sizeof(codingscheme)-1); /* save it up */
			removetrailing(codingscheme);
			lowercase((char *) s, (char *)s);
			removetrailing((char *) s);
/*			printf("Checking: *%s*\n", s);	*/ /* debugging */
/*			for (i = 0; i < encodingcount; i++) { */
			for (i = 0; i < 64; i++) {
				if (strcmp(encodingnames[i][0], "") == 0) break;
				if (strcmp(encodingnames[i][1], "") == 0) break;
/*				if (strcmp(s, encodingnames[i][0]) == 0) { */
				if (strcmp((char *) s, encodingnames[i][0]) == 0) {
					strcpy(codingvector, encodingnames[i][1]);
					return 1;	/* success */
				}
			}
			fprintf(stderr, "Sorry, don't recognize coding scheme `%s'\n",
				codingscheme);
			return 0;
		}
	}
	return -1;		/* not found */
}

int analyzepfm (FILE *input, char *filename) {
	unsigned int pfmlen;

	unsigned int firstchar, lastchar;
	unsigned int defaultchar, breakchar, charset;
	unsigned int avwidth, maxwidth, code, pitch, family;
	unsigned int len;
	unsigned int widthinx, origininx, driverinx, faceinx, fontnameinx;
/*	unsigned in pairkerninx, trackkerninx, reserved, dftype; */
/*	unsigned int pointsize; */
/*	unsigned int vres, hres; */
	unsigned int extendinx;
	unsigned int points, extsize, metsize;
	int k;
/*	double charwidth; */
	int flag;
	unsigned char *s;
	int italic, bold, weight;

	if ((pfmlen = readpfm(input)) == 0) {
		return -1;
	}
	if (*buffer == 128 && *(buffer+1) == 1) {
		fprintf(stderr, 
			"This is a PFB file (outlines), not a PFM file (metrics)\n");
		return -1;
	}
	if (*buffer > 1 || *(buffer+1) > 3) {  /* should be 0, 1 (version) */
		fprintf(stderr, "Not a valid PFM file\n");
		return -1;
	}

/*	reset to defaults for new file - 1992/Dec/2 */
	ansiflag = 1;		/* non-zero means ANSI encoding */
	oemflag = 0;		/* non-zero means OEM encoding */
	symbolflag = 0;		/* non-zero means symbol encoding */
	decorative = 0;		/* NOT ANSI encoding - `native' encoding */

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

	if (showfaceflag) goto skipencoding;

	strcpy(codingscheme, "");
	strcpy(codingvector, "");
	flag = findcomment(pfmlen);

	if (flag < 0) {
/*		printf("Sorry, PFM file does not contain encoding comment\n"); */
		printf("PFM file does not contain encoding comment\n");
		if (ansiflag && !decorative)
			printf("Hence probably Windows ANSI if a plain text font\n");
		else 
			printf("Hence probably Font Specific unless its a text font\n");
	}
	else {
/*	check on consistency of reputed encoding and the flags */
		if (strcmp(codingscheme, "") != 0) 
/*			printf("`%s' claims to use `%s'\n", filename, codingscheme); */
			printf("Claims to use `%s' coding scheme\n", codingscheme);
		if (flag > 0) {
/*			printf("`%s' claims to use `%s' encoding vector\n",
				filename, codingvector); */
			printf("Claims to use `%s' encoding vector\n", codingvector);
		}
		if (strcmp(codingvector, "") != 0) {
			if (strcmp(codingvector, "ansi") == 0 ||
				strcmp(codingvector, "ansinew") == 0) {
				if (symbolflag != 0)
	printf("ERROR: Symbol flag set in PFM, yet ANSI encoding\n");
				if (decorative != 0) 
	printf("ERROR: Decorative flag set in PFM, yet ANSI encoding\n");
			}
			if (strcmp(codingvector, "ansi") != 0 &&
				strcmp(codingvector, "ansinew") != 0) {
				if (ansiflag != 0)
	printf("WARNING: Symbol flag not set in PFM, yet not ANSI encoding\n");
				if (decorative == 0) 
	printf("WARNING: Decorative flag not set in PFM, yet not ANSI encoding\n");
			}
		}
	}

skipencoding:

/*		start decoding PFMHEADER (PFM file header) now */		
/*		PFMHEADER fields are the same as Windows screen font file fields */
/*		documented in `MicroSoft Windows Device Driver Adaptation Guide' */

	len = readtwo(buffer +2);			/* dfSize */
	if (pfmlen != len) {
		if (verboseflag != 0) 
		printf("WARNING: Actual file length %u not equal to coded length %u\n",
			pfmlen, len);
	}
/*	get version (of PFM specification ?) */
/*	a = *buffer; b = *(buffer+1);	
	if (verboseflag != 0) printf("Version: %u.%u \n", b, a);
 	printf("Version %u.%u\n", b, a); */
/*	copyright message next - up to 60 characters - null delimited */
	if (*(buffer +6) != 0) {
/*		if (verboseflag != 0) printf("%s\n", buffer +6); */
		if (verboseflag != 0) printf("Copyright string: %s\n", buffer +6); 
	}
/*	dftype =readtwo(buffer +66); 
	if (dftype != (128 | 1))
		fprintf(stderr, "WARNING: dfType (%d) is not 128 | 1\n", 
			dftype); */
	points = readtwo(buffer +68); /* dfPoints */
	if (points != 10)
		fprintf(stderr, "WARNING: dfPoints (%d) is not 10\n", points);
/*	vres = readtwo(buffer +70); hres = readtwo(buffer +72);  */
/*	if (vres != 300 || hres != 300) 
		fprintf(stderr, "WARNING: dfVertRes %u, dfHorizRes %u not 300\n", 
			vres, hres); */
/* The PostScript driver assumes dfVertRes == 300 */

	charset = readone(buffer +85); /* dfCharSet */
	if (verboseflag != 0) {
		printf("CharSet: ");
		if (charset == 0) printf("ANSI - ");
		else if (charset == 1) printf("DEFAULT - ");
		else if (charset == 2) printf("Symbol - ");
		else if (charset == 128) printf("ShiftJis - "); 
		else if (charset == 129) printf("Hangeul - ");
		else if (charset == 136) printf("ChineseBig5 - ");
		else if (charset == 255) printf ("OEM - ");
		else printf ("UNKNOWN - ");
	}

	if (showfaceflag) goto skipcoding;

/*	THE FOLLOWING DOES NOT ALWAYS WORK CORRECTLY: */
	if (verboseflag != 0 || strcmp(codingscheme, "") == 0) {
		printf("EncodingScheme (based on flags): ");
	if (decorative != 0)
		printf("FontSpecific\n");
/*	else if (ansiflag != 0)
		printf("MS Windows ANSI\n");  */
/*	else if (symbolflag != 0)
		printf("Symbol %% maybe\n"); */
	else if (oemflag != 0)
		printf("IBM OEM\n"); 
/*	else if (standardflag != 0)
		printf("AdobeStandardEncoding\n");  */
/*	else printf("FontSpecific\n"); */
		else printf(" MS Windows ANSI\n");
	}

skipcoding:

/*	4-bits of family and 2-bits of Pitch information */
	code = *(buffer +90);	/* dfPitchAndFamily */
	pitch = code & 15; 
	family = code >> 4;
/*	printf("Code %u %u \n", family, pitch); */
	if (pitch != 1) 
		fprintf(stderr, "WARNING: pitch (%d) <> 1\n", code & 15);

/*	THE FOLLOWING DOES NOT ALWAYS WORK CORRECTLY: */
/*	the variable pitch bit seems to always be ON in PS fonts ... */
/*	if (pitch == 0) printf("IsFixedPitch true\n"); */
/*	else printf("IsFixedPitch false\n"); */
	avwidth = readtwo(buffer +91);	/* sometimes width of `x' */
	maxwidth = readtwo(buffer +93);
/*	if (avwidth == maxwidth) printf("IsFixedPitch true\n");
	else printf("IsFixedPitch false\n"); */
/*	maybe base instead on analysis of characters width table ? */		

/*	Font Family terms are defined in: */
/*	`MicroSoft Windows Device Driver Adaptation Guide' */
/*	`MicroSoft Windows SDK Tools' page 6-9 */
/*	code = code >> 4; */
	if (verboseflag != 0) {
		printf("Family: ");				/* ??? */
		if (family == 0) printf("Don't Care (Custom) \n");
		else if (family == 1) printf("Roman (Serif) \n");
		else if (family == 2) printf ("Swiss (Sans serif) \n");
		else if (family == 3) printf ("Modern (Fixed pitch) \n");
		else if (family == 4)  printf ("Script (Cursive) \n");
		else if (family == 5)  printf ("Decorative (NOT ANSI) \n");
		else printf("UNKNOWN code %d\n", family);
	}

/*	dfFirstChar and dfLastChar */
	firstchar = *(buffer +95); 
	lastchar = *(buffer +96);
	if (verboseflag != 0) printf("Starting char %u, ending char %u, ", 
		firstchar, lastchar);

/*	dfDefaultChar and dfBreakChar */
	defaultchar = *(buffer +97); 
	breakchar = *(buffer +98);
	defaultchar = defaultchar + firstchar;	/* often 32 */
	breakchar = breakchar + firstchar;		/* often 32 (or	0) */

	if (verboseflag != 0) printf("default char %u, break char %u\n", 
		defaultchar, breakchar);

/* dfWidthBytes -	buffer +99 */

	driverinx = readtwo(buffer +101); /* dfDevice Driver - usually 199 */
/*	face name next - up to 32 characters - null delimited */
	faceinx = readtwo(buffer +105);	  /* dfFace - usually 210 */

/* dfBitsPointer -	buffer +109 */
/* dfBitsOffset  -	buffer +113 */

/* we assume there is no width table between PFMHEADER and PFMEXTENSION */
/* since only variable width PCL fonts should have one */
/* PostScript fonts use the extent table for character widths */

/*	now start to decode PFMEXTENSION */

	s = buffer +117; 
	extsize = readtwo(s);		/* dfSizeFields - usually 30 */
	if (extsize != 30) 
		fprintf(stderr, "WARNING: PFM extension size (%d) <> 30\n", 
			extsize);
	if (traceflag != 0) printf("PFM extension size %u (30) - ", 
		extsize);

/*	byte offset in the file of EXTTEXTMETRIC structure */
	extendinx = readtwo(s +2);		/* dfExtMetricsOffset - usually 147 */
/*	byte offset in file of extent table - used for widths in PS fonts */
	widthinx = readtwo(s +6);		/* dfExtentTable - varies */

/*	byte offset in file of table of character origins */
	origininx = readtwo(s +10);		/* only non NULL for screen fonts */
	if (origininx != 0)
		fprintf(stderr, "WARNING: orginx (%d) <> 0\n", origininx);

/*	byte offset in file of optional pair-kern table */
/*	pairkerninx = readtwo(s +14);	*/	/* dfPairKernTable */
/*	size of table given by etmKernPairs in EXTTEXTMETRIC structure */
/*	if (pairkerninx > 0) {
		if (verboseflag != 0) 
			printf("pair kern table start %u\n", pairkerninx); 
		printf("Comment pair kern table start %u\n", 
			pairkerninx); 
	} */

/*	byte offset in file of optional track-kern table */ /* PairKernTable */
/*	trackkerninx = readtwo(s +18);		
	if (trackkerninx > 0) {
		fprintf(stderr, "WARNING: track kern table start %u\n", 
			trackkerninx);
	} */

/*	byte offset in file of driver-specific information */
/*	for PostScript drivers this is the PostScript fontname */
	fontnameinx = readtwo(s +22);  /* dfDriverInfo - varies */
/*	reserved = readtwo(s +26);
	if (reserved > 0) {
		fprintf(stderr, "WARNING: reserved dword non-zero %u\n", 
			reserved);
	} */

/*  now decode EXTTEXTMETRIC - extended metrics */

	s = buffer + extendinx; 
	metsize = readtwo(s);			/* etmSize - normally is 52 */
	if (metsize != 52 && metsize != 0)
		fprintf(stderr, "WARNING: Extended metrics size (%d) <> 52 or 0\n",
			metsize);
	if (traceflag != 0) 
		printf("Extended metrics size %u (52)\n", metsize);

/*	pointsize = readtwo(s +2); */			/* etmPointSize - ignored */

/*	italicnum = sreadtwo(s +22);

	if (italicnum != 0) {
		if (verboseflag != 0) printf("Italic Angle %lg\n", 
			((double) italicnum) / 10.0);
	}
	printf("ItalicAngle %lg\n", ((double) italicnum) / 10.0); */
		
/*	now for the embedded strings */
			
/*	s = buffer + driverinx; */ /* 199 */
/*	strcpy(driver, buffer + driverinx); */
	strcpy(driver, (char *) (buffer + driverinx));
/*	if (verboseflag != 0) printf("Printer driver type: %s \n", driver); */
		
/*	s = buffer + faceinx;  */
/*	strcpy(facename, buffer + faceinx); */
	strcpy(facename, (char *) (buffer + faceinx));
		
/*	s = buffer + fontnameinx; */
/*	strcpy(fontname, buffer + fontnameinx); */
	strcpy(fontname, (char *) (buffer + fontnameinx));

	italic = *(buffer +80); 
	weight = readtwo(buffer +83);	/* dfWeight */

/*	if (verboseflag != 0) { */
	if (verboseflag || showfaceflag) {
/*		printf("MS Face Name   `%s'	", facename); */
		if (showfaceflag) printf("`%s' ", facename);
		else printf("Face Name: `%s' ", facename);
		printf("\t");
		bold = (weight > 400);
		if (!bold && !italic)    printf("REGULAR   ");
		else if (bold && italic) printf("BOLDITALIC");
		else if (bold)           printf("BOLD      ");
		else if (italic)         printf("ITALIC    ");
/*		else {
			if (weight > 400) printf("BOLD");
			else printf("    ");  
			if (italic > 0) printf("ITALIC");
			else printf("      "); 
		} */
		printf("\t");
		if (showfaceflag) printf("`%s'\n", fontname);
		else printf("PS FontName: `%s'\n", fontname);
		if (verboseflag && showfaceflag) putc('\n', stdout);
	}

	if (showfaceflag) return 0;

/* now decode character widths */
	for (k = 0; k < MAXCHRS; k++) binwidths[k] = 0.0;
	for (k = (int) firstchar; k <= (int) lastchar; k++) {
		binwidths[k] = (double) sreadtwo (buffer + widthinx + (k - firstchar) * 2);
	}

/* now check whether remapped maybe */

	remapflag = checkremap (firstchar, lastchar);
	sayremap(remapflag, fixedwidth);

/*  see whether matches AFM file encoding */
	if (useafm != 0) {
		afmmatch = checkmatch ();
		if (afmmatch != 0)
			printf("Encoding appears to match that in AFM file\n");
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int compareencoding (FILE *input) {
	int chrs;
	char charname[MAXCHARNAME];
	char *s;
	int count=0;

	if (fgets(line, MAXLINE, input) == NULL) return 0;	/* EOF */
	if ((s = strstr(line, "Encoding: ")) != NULL) {
		s += 9;
		while (*s <= ' ') s++;
		strncpy (codingscheme, s, sizeof(codingscheme)-1);
		removetrailing(codingscheme);
	}
	else strcpy(codingscheme, "");

	rewind (input);

	for (;;) {
		if (fgets(line, MAXLINE, input) == NULL) break; /* EOF */
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (sscanf (line, "%d %s", &chrs, &charname) == 2) {
			if (chrs < 0 || chrs > 255) continue;
			if (strcmp(encoding[chrs], charname) != 0) {
				return -1;				/* mismatch */
			}
			else count++;				/* count matches */
		}
		else continue;
	}
	return count;							/* matches ! */
}

struct charmetric *lookupafm (char *);

/* returns -1 if mismatch, otherwise count of matched characters */

int comparewidths (FILE *input) {
	int chrs;
	char charname[MAXCHARNAME];
	char *s;
	int count=0;
	double afmwidth, binwidth;
	struct charmetric *ptr;

	if (fgets(line, MAXLINE, input) == NULL) return 0;
	if ((s = strstr(line, "Encoding: ")) != NULL) {
		s += 9;
		while (*s <= ' ') s++;
		strncpy (codingscheme, s, sizeof(codingscheme)-1); 
		removetrailing(codingscheme);
	}
	else strcpy(codingscheme, "");

	rewind (input);

	for (;;) {
		if (fgets(line, MAXLINE, input) == NULL) break; /* EOF */
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (sscanf (line, "%d %s", &chrs, &charname) == 2) {
			if (chrs < 0 || chrs > 255) continue;
			ptr =  lookupafm (charname);	/* look up charname in AFM */
			if (ptr == NULL) {
				if (traceflag != 0)
					printf("k %d %s not found\n", chrs, charname);
				return -1;				/* mismatch */
			}
			afmwidth = ptr->charwidth;		/* get corresp width from AFM */
			binwidth = binwidths[chrs];		/* get width from metric file */
/* deal with width = 0 represented as width = 1 (or more) */
			if (afmwidth == 0.0) binwidth = afmwidth;
			if (binwidth > afmwidth - epsilon &&
				binwidth < afmwidth + epsilon) {
				count++;				/* count matches */
			}
			else {
				if (traceflag != 0)
					printf("k %d %s afm %lg bin %lg\n",
						chrs, charname, afmwidth, binwidth);
				return -1;				/* mismatch */
			}
		}
		else continue;
	}
	return count;							/* matches ! */
}

int isremapped () {		/* check whether encoding is remapped */
	int k;
	for (k = 0; k < 10 ; k++) {
		if (strcmp(encoding[k], encoding[k+161]) != 0) return 0;
	}
	for (k = 10; k < 32 ; k++) {
		if (strcmp(encoding[k], encoding[k+163]) != 0) return 0;
	}
/*	also check 32 and 196 ? */
/*	also check 32 and 128 ? */
	return -1;							/* appears to be remapped */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int gettoken (char *buff, FILE *input) {
	int c, n=0;
	char *s=buff;

	for (;;) {
		c = getc(input);
		if (c == EOF) return EOF;
		if (n++ >= MAXLINE) return n;
		if (c <= ' ') {
			*s++ = '\0';
/*			if (traceflag != 0) printf("%s\n", buff); */
			return n;
		}
		else *s++ = (char) c;
	}
}

int nchrs;	/* number of characters read from AFM file */

/* int compare (struct charmetric *a1, struct charmetric *a2) { */
int compare (const void *a1, const void *a2) {
	struct charmetric *c1 = (struct charmetric *) a1;
	struct charmetric *c2 = (struct charmetric *) a2;
	return strcmp(c1->charname, c2->charname);
}

void sortafmdata (int nchrs) {
/*	qsort(afmdata, nchrs, sizeof(afmdata[0]), compare); */
	qsort((void *) afmdata,	nchrs, sizeof(afmdata[0]), compare);
}

struct charmetric *lookupafm (char *name) {
	struct charmetric *ptr;
	ptr = bsearch(name,	afmdata, nchrs,	sizeof(afmdata[0]),	compare);
	if (ptr == NULL) {
	if (traceflag != 0)	printf("charmetric %s not found\n", name);
		return NULL;
	}
	else {
		if (traceflag != 0)
		 printf("charmetric %s %s %lg\n", name, ptr->charname, ptr->charwidth);
		return ptr;
	}
}

void showafmwidths(FILE *output) {
	int k;
	for (k = 0; k < nchrs; k++)
		fprintf(output, "%d %s\t%lg\n",
			k, afmdata[k].charname, afmdata[k].charwidth);
}

int readafmfile (FILE *input) {		/* read info from AFM file */
	int k, chrs;
	double wx;
	char charname[MAXCHARNAME];
	double allwidth=0.0;
	
	if (traceflag != 0) printf("Reading AFM file\n");

	for (k = 0; k < MAXCHRS; k++) {
/*		strcpy(encoding[k], ""); */
		encoding[k] = "";
		afmwidths[k] = 0.0; 
	}
	for (k = 0; k < MAXAFMS; k++) {
/*		strcpy(afmdata[k].charname, ""); */	/* hmm... */
		afmdata[k].charname = "";			/* 95/May/12 */
		afmdata[k].charwidth = 0.0;
	}
	nchrs = 0;
	fixedwidth = 1;
	for (;;) {
		if (fgets(line, MAXLINE, input) == NULL) return -1;
		if (strncmp(line, "StartCharMetrics", 16) == 0) break;
	}
	for (;;) {
		if (fgets(line, MAXLINE, input) == NULL) break;
		if (strncmp(line, "EndCharMetrics", 16) == 0) break;
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (strncmp(line, "Comment", 7) == 0) continue;
		if (sscanf (line, "C %d ; WX %lg ; N %s", &chrs, &wx, &charname) == 3) {
			if (chrs >= 0 || chrs < MAXCHRS) {
/*				strcpy(encoding[chrs], charname); */
				encoding[chrs] = _strdup(charname);	/* 95/May/15 */
				afmwidths[nchrs] = wx; 
				if (allwidth == 0.0) allwidth = wx;
				else if (allwidth != wx) fixedwidth = 0;
			}
			if (nchrs < MAXAFMS) {
/*				strcpy(afmdata[nchrs].charname, charname); */
				afmdata[nchrs].charname = _strdup(charname);
				afmdata[nchrs].charwidth = wx;
			}
			nchrs++;
		}
	}
	if (traceflag != 0) showafmwidths(stdout);
	if (traceflag != 0)
		printf("Sorting AFM file data for %d characters\n", nchrs);
	sortafmdata (nchrs);		/* sort for easy lookup later */
	if (traceflag != 0) showafmwidths(stdout);
	return nchrs;				/* normal return */
}

int lookup (char *name) {	/* look up a character in the encoding */
	int k;
	for (k = 0; k < nchrs; k++) {
		if (strcmp(encoding[k], name) == 0) return k;
	}
	return -1;
}

int standardflag;			/* non-zero if StandardEncoding */

int scanpfbfile(FILE *input) {
	int chrs, count=0;

/*	search for /Encoding first */
	for (;;) {
		if (gettoken(line, input) == EOF) {
			fprintf(stderr, "Unexpected EOF\n");
			exit(1);
		}
		if (strcmp(line, "/Encoding") == 0) break;
	}

	standardflag = 0;
	if (useafm != 0) afmmatch = 1;

/*	read encoding vector from PFB file */
	for (;;) {
		if (gettoken(line, input) == EOF) {
			fprintf(stderr, "Unexpected EOF\n");
			exit(1);
		}
		if (strcmp(line, "StandardEncoding") == 0) {
			standardflag = -1;
			return 0;
		}
		if (strcmp(line, "readonly") == 0) break;
		if (strcmp(line, "dup") == 0) {
			(void) gettoken(line, input);
			if (sscanf(line, "%d", &chrs) < 1) {
				fprintf(stderr, "%s", line);
				continue;
			}
			if (chrs < 0 || chrs > 255) {
				fprintf(stderr, "Bad char number %d\n", chrs);
				continue;
			}
			(void) gettoken(line, input);

			if (*line == '/') {
				if (useafm != 0 && afmmatch != 0) { 
					if (strcmp(encoding[chrs], "") == 0 &&
						(chrs < 32 || chrs > 127)) {
/* ignore possible remapping `mismatches' */
					}
					else if (strcmp(encoding[chrs], line+1) != 0) {
						if (traceflag != 0)
							printf("k %d AFM `%s' PFB `%s'\n",
								chrs, encoding[chrs], line+1);
						afmmatch = 0;
					}
				}
/*				strcpy(encoding[chrs], line+1); */
				encoding[chrs] = _strdup(line+1);
				count++;
			}
			if (traceflag != 0) putc('.', stdout);
		}
	}	
	return count;
}

void showencoding (FILE *output) {
	int k;
	
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], "") == 0) continue;
		fprintf(output, "%d\t%s\n", k, encoding[k]);
	}
}

int analyzepfb (FILE *input, char *filename) {
	int k, count=0;

	if (useafm != 0) {						/* check if matches AFM */
		count = scanpfbfile(input);
		rewind (input);
	}
	if (useafm == 0 || afmmatch == 0) {		/* if no AFM or if not match */
/*		for (k = 0; k < MAXCHRS; k++) strcpy(encoding[k], ""); */
		for (k = 0; k < MAXCHRS; k++) encoding[k] = "";
		count = scanpfbfile(input);
	}
	if (standardflag != 0) {
		printf("Adobe Standard Encoding\n");
/*		if (verboseflag != 0) */
			printf("(which means ATM will reencode to Windows ANSI!)\n");
		return 0;
	}
	if (traceflag != 0) showencoding(stdout);
	remapflag = isremapped();
/* assuming fixedwidth set by readafmfile */
	sayremap (remapflag, fixedwidth);	
	if (useafm != 0) {
		if (afmmatch != 0)
			printf("Encoding appears to match that in AFM file\n");
		else 
			printf("Encoding does NOT match that found in AFM file\n");
	}
	return count;
}

#ifdef IGNORE
void extension(char *fname, char *str) {  /* supply exten if none present */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, str);
	}
	else strcpy(s+1, str);		/* give it default extension */
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

/* returns pointer to file name without path */

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
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
}

FILE *openvector (char *vectorname) {
	FILE *vector;
	char filename[FILENAME_MAX];

	extension (vectorname, "vec");
	if ((vector = fopen (vectorname, "r")) != NULL) return vector;
	strcpy (filename, programpath);
	strcat (filename, "\\");
	strcat (filename, stripname (vectorname));
	if ((vector = fopen (vectorname, "r")) != NULL) return vector;
	strcpy (filename, vectorpath);
	strcat (filename, "\\");
	strcat (filename, stripname (vectorname));
	if ((vector = fopen (vectorname, "r")) != NULL) return vector;
	fprintf (stderr, "ERROR: cannot find %s\n", vectorname);
	return NULL;
}

FILE *openafm (char *afmname) {
	FILE *afm;
	char filename[FILENAME_MAX];

	strcpy(filename, afmname);
/*	First try AFM file name as is */
	if ((afm = fopen (filename, "r")) != NULL) return afm;
/*	Then try with AFM extension */
	extension (filename, "afm");
	if ((afm = fopen (filename, "r")) != NULL) return afm;
/*	Then try with underscores */
	underscore(filename);
	if ((afm = fopen (filename, "r")) != NULL) return afm;
/*	Then try in program directory */
	strcpy (filename, programpath);
	strcat (filename, "\\");
	strcat (filename, stripname (afmname));
	if ((afm = fopen (afmname, "r")) != NULL) return afm;
/*	Then try with AFM extension */
	extension (filename, "afm");
	if ((afm = fopen (filename, "r")) != NULL) return afm;		
/*	Then try with underscores */
	underscore(filename);
	if ((afm = fopen (filename, "r")) != NULL) return afm;
	fprintf (stderr, "ERROR: cannot find %s\n", afmname);
	return NULL;
}

int guessvecpfb (int argc, char *argv[], int firstarg, int count) {
	FILE *vector;
	int m;
	int found = 0;
	char *s;
	char fn_vec[FILENAME_MAX];
	int match=0;

	for (m = firstarg; m < argc; m++) {
		strcpy(fn_vec, argv[m]);
		lowercase(fn_vec, fn_vec);
		if (strstr(fn_vec, ".vec") == NULL) {
/*			fprintf(stderr, "%d %s\n", m, fn_vec); */
				found = 1;
				continue;
			}
/*			strcpy(fn_vec, argv[m]); */
			if ((vector = openvector(fn_vec)) == NULL) {
/*					fprintf(stderr, "%d %s\n", m, fn_vec); */
				continue;
			}
/*			if (traceflag != 0) */
			if (verboseflag != 0)
				printf("Looking at vector file %s\n", fn_vec);
/*			now try and check encoding vector */
			match = compareencoding(vector);
/* check that match >= count ??? */
/* unless remapflag != 0 in which case match >= count - 36 */
			if (match > 0) {
				if (traceflag != 0) 
					printf ("Count %d Match %d\n", count, match);
				if (remapflag != 0) count = count - 36;
				strcpy(fn_vec, stripname(argv[m]));
				lowercase(fn_vec, fn_vec);
				if ((s = strrchr(fn_vec, '.')) != NULL) *s='\0';
/*				printf("Matches `%s' encoding vector\n", fn_vec); */
				if (strcmp(codingscheme, "") != 0) {
					printf("Matches `%s' coding scheme\n", codingscheme);
				}
				printf("Matches `%s' encoding vector\n", fn_vec);
				fclose (vector);
				found = 1;
				if (match < count)
					printf ("Incomplete match: Count %d Match %d\n",
						count, match);
				if (match >= count) break;
			}
			else {
/*				fprintf(stderr, "%d %s failed\n", m, fn_vec); */
			}
			fclose (vector);
	}
	if (found == 0)
		fprintf(stderr, "Sorry, do not recognize encoding vector\n");
	if (match < 0)
		fprintf(stderr, "Sorry, do not recognize encoding vector\n");
	return match;
}

int guessvecbin (int argc, char *argv[], int firstarg, int count) {
	FILE *vector;
	int m;
	int found = 0;
	int match = 0;
	char *s;
	char fn_vec[FILENAME_MAX];

	for (m = firstarg; m < argc; m++) {
		strcpy(fn_vec, argv[m]);
		lowercase(fn_vec, fn_vec);
		if (strstr(fn_vec, ".vec") == NULL) {
/*			fprintf(stderr, "%d %s\n", m, fn_vec); */
				found = 1;
				continue;
			}
/*			strcpy(fn_vec, argv[m]); */
			if ((vector = openvector(fn_vec)) == NULL) {
/*					fprintf(stderr, "%d %s\n", m, fn_vec); */
				continue;
			}
/*			if (traceflag != 0) */
			if (verboseflag != 0)
				printf("Looking at vector file %s\n", fn_vec);
/*			now try and check encoding vector */
			match = comparewidths(vector);
/* check that match >= count ??? */
/* unless remapflag != 0 in which case match >= count - 36 */
/*			if (traceflag != 0) showafmwidths(stdout);	 */
			if (match > 0) {
				if (traceflag != 0) 
					printf ("Count %d Match %d\n", count, match);
				if (remapflag != 0) count = count - 36;
				strcpy(fn_vec, stripname(argv[m]));
				lowercase(fn_vec, fn_vec);
				if ((s = strrchr(fn_vec, '.')) != NULL) *s='\0';
/*				printf("Matches `%s' encoding vector\n", fn_vec); */
				if (strcmp(codingscheme, "") != 0) {
					printf("Matches `%s' coding scheme\n", codingscheme);
				}
				printf("Matches `%s' encoding vector\n", fn_vec);
				fclose (vector);
				found = 1;
				if (match < count)
					printf ("Incomplete match: Count %d Match %d\n",
						count, match);
				if (match >= count) break;
			}
			else {
/*				fprintf(stderr, "%d %s failed\n", m, fn_vec); */
			}
			fclose (vector);
	}
	if (found == 0)
		fprintf(stderr, "Sorry, do not recognize encoding vector\n");
	if (match < 0)
		fprintf(stderr, "Sorry, do not recognize encoding vector\n");
	return match;
}


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int analyzepfa (FILE *input, char *filename) {
	return -1;
} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int analyzeafm (FILE *input, char *filename) {
	return -1;
} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* remove file name - keep only path - inserts '\0' to terminate */

void removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
}

/* \t\t<file-1> [<file-2> ... <file-n>]\n\ */

void showusage(char *s) {
	fprintf (stderr, 
		"Usage:\n%s [-{v}{x}{r}{Z}] [-a=<afm-file>] [-w=<win-dir>] \n\
\t<file-1> [<file-2> ... <file-n>]\n\
\t\t[<vector-1> <vector-2>...<vector-n>]\n\
", s);
	if (detailflag == 0) exit(1);
	putc('\n', stderr);
	fprintf(stderr, "\tv verbose\n");
	fprintf(stderr, "\tx check whether Windows is running (sets DOS errlevel)\n");
	fprintf(stderr, "\tr delete ATMFONTS.QLC (ATM's font metric cache)\n");
	putc('\n', stderr);
	fprintf(stderr, "\ta use specified AFM file for reference\n");
	fprintf(stderr, "\tZ show Face Name, Style and PS FontName in PFM files\n");
	fprintf(stderr, "\tw Windows Directory (where ATM.INI may be found)\n");
	putc('\n', stderr);
	fprintf(stderr, "\t  <file-i> may be PFB, PFA, PFM, or TFM file\n");
	fprintf(stderr, "\t  <afm-file> needed for analyzing PFB file\n");
	fprintf(stderr, "\t  <afm-file> also needed if vectors are specified\n");
	putc('\n', stderr);
	fprintf(stderr, "\t NOTE: More useful information available if vectors specified\n");
	putc('\n', stderr);
	fprintf(stderr, "\t Specifying `-w=...' causes Windows Face Name of specified fonts\n");
	fprintf(stderr, "\t to be extended with an `X' in ATM.INI (alternate use of DECODE)\n");
	fprintf(stderr, "\t Specifying `-W=...' removes `X' from Windows Face Names.\n");
	exit(1);
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

void stampit(FILE *outfile) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);
	fprintf(outfile, "%s (%s %s)\n", 
		programversion, date, compiletime);
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	
/*	(void) _getch();  */
	return hash;
}


int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0; 
		case 'v': verboseflag = 1; return 0;
		case 't': traceflag = 1; return 0; 
		case 'x': checkflag = 1; return 0;
		case 'r': resetatm = 1; return 0;
		case 'Z': showfaceflag = 1; return 0;
/*	following take arguments */
		case 'w': winflag = 1; facex = 1; return -1;
		case 'W': winflag = 1; facex = 0; return -1;		
		case 'F': fontflag = 1; return -1;
		case 'c': vectorflag = 1; return -1;
		case 'a': afmflag = 1; return -1;
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
	return 0;		/* ??? */
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (afmflag != 0) {
					afmfile = s; 
					afmflag = 0; useafm=1;
				} 
				else if (winflag != 0) {
					winfile = s; 
/*					printf("WIN FILE: `%s'\n", s); */ /* debug */
					winflag = 0; usewin=1;
				} 
				else if (vectorflag != 0) {
					vector = s; 
/*					printf("VECTOR: `%s'\n", s); */ /* debug */
					vectorflag = 0; 
				} 
				else if (fontflag != 0) {
					PFB_Dir = s; 
/*					printf("PSFONTS: `%s'\n", s); */ /* debug */
					fontflag = 0; 
				} 
/*				else if (extenflag != 0) {
					ext = s; firstarg++;
					extenflag = 0; 
				} */
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

FILE *findfile (char *filename) {
	FILE *input=NULL;

/*  first check whether extension specified */
	if (strrchr(filename, '.') != NULL)	{
		if ((input = fopen (filename, "rb")) != NULL) return input;
		underscore(filename);
		if ((input = fopen (filename, "rb")) != NULL) return input;
		return NULL;
	}
/*	extension was NOT specified */
	forceexten(filename, "tfm");
	if ((input = fopen (filename, "rb")) != NULL) return input;
	forceexten(filename, "pfm");
	if ((input = fopen (filename, "rb")) != NULL) return input;	
	forceexten(filename, "pfb");
	if ((input = fopen (filename, "rb")) != NULL) return input;
	forceexten(filename, "pfa");
	if ((input = fopen (filename, "rb")) != NULL) return input;
	forceexten(filename, "afm");
	if ((input = fopen (filename, "rb")) != NULL) return input;
	underscore(filename);
	forceexten(filename, "tfm");
	if ((input = fopen (filename, "rb")) != NULL) return input;
	forceexten(filename, "pfm");
	if ((input = fopen (filename, "rb")) != NULL) return input;	
	forceexten(filename, "pfb");
	if ((input = fopen (filename, "rb")) != NULL) return input;
	forceexten(filename, "pfa");
	if ((input = fopen (filename, "rb")) != NULL) return input;
	forceexten(filename, "afm");
	if ((input = fopen (filename, "rb")) != NULL) return input;
	return NULL;
}

/* Alternate use of DECODE to modify Face Name in ATM.INI */
/* Try and change Windows Face Name of all listed fonts */
/* facex != 0 => add    `X' */ /* (unless they already end in X) */
/* facex == 0 => remove `X' */ /* (unless they does not end in X) */

int modifyatmini(int argc, char *argv[], int firstarg) {
	int k, c, n;
	char atmfile[FILENAME_MAX], outfile[FILENAME_MAX];
	FILE *input, *output;
	char *s, *t, *u, *v;

	strcpy(atmfile, winfile);
/*	see if user specified full name or just directory */
	if (strstr(atmfile, "atm.ini") == NULL) {
		c = *(atmfile + strlen(atmfile) - 1);
		if (c != '\\' && c != '/') strcat (atmfile, "\\");
		strcat(atmfile, "atm.ini");
	}
/*	see whether the darn thing even exists */
	if ((input = fopen (atmfile, "r")) == NULL) {
		perror(atmfile); exit(3);
	}
	else fclose(input);
/*	try and place output in current directory */
	strcpy(outfile, stripname(atmfile));
/*	check whether it is the same file - avoid overwrite */
	if (strcmp(outfile, atmfile) == 0) {
		forceexten(atmfile, "bak");
		if (verboseflag != 0)
			printf("Renaming %s to %s\n", outfile, atmfile);
		remove (atmfile);				/* get rid of old `atm.bak' file */
		rename (outfile, atmfile);		/* rename to `atm.bak' */
	}
	if ((input = fopen (atmfile, "r")) == NULL) {
		perror(atmfile); exit(3);
	}	
	if ((output = fopen (outfile, "w")) == NULL) {
		perror(outfile); exit(3);
	}	
/*	Now finally do some work ! */

/*  scan up to [Fonts] section */
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) break;		/* EOF */
		fputs(line, output);
/*		if (strncmp(line, "[Fonts]", 7) == 0) break; */
/*		if (strncmp(line, "[fonts]", 7) == 0) break; */
		if (_strnicmp(line, "[fonts]", 7) == 0) break;	/* 1995/June/23 */
	}

/*	See which Windows Face Names need to be modified */
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) break;		/* EOF */
		if (*line == ';' || *line < ' ') { /* comments and blanks */
			fputs(line, output);
			continue;
		}
		if (*line == '[') {
			fputs(line, output);
			break;							/* end of [Fonts] section */
		}
/*Didot LH Roman,ITALIC=c:\psfonts\pfm\doi_____.pfm,c:\psfonts\doi_____.pfb */
		if ((s = strchr(line, '=')) == NULL) {
			fputs(line, output); continue;
		}
		if ((t = strstr(s, ".pfm")) == NULL) {
			fputs(line, output); continue;
		}
		if ((u = strrchr(t, '\\')) == NULL &&
			(u = strrchr(t, '/')) == NULL &&
			(u = strrchr(t, ':')) == NULL) {
			fputs(line, output); continue;
		}
/*		u now points to start of actual font file name (PFM) */
		u++;

/*		if (traceflag != 0) printf("Looking for %s", u); */

		for (k = firstarg; k < argc; k++) {
/*		see if file name matches (possibly minus underscores) */
			n = strlen(argv[k]);
			if (strncmp(u, argv[k], n) != 0) continue;
/*			c = *(u + strlen(u) -1); */
			c = *(u + n);
			if (c != ',' && c != '_' && c > ' ') continue; 
			if ((v = strchr(line, ',')) != NULL) {
				if (v < s) s = v;
			}
			if (traceflag != 0) fputs(line, stdout);
/*			s now points to one past last character of Face Name */
			if (facex != 0) {				/* try to add an `X' */
				if (*(s-1) != 'X') {		/* extended with X ? */
					memmove(s+1, s, strlen(s)+1);
					*s = 'X';				/* stick in an `X' */
				}
			}
			else {							/* try to remove an `X' */
				if (*(s-1) == 'X') {		/* extended with X ? */
					memmove(s-1, s, strlen(s)+1);
				}
			}
			if (verboseflag != 0) fputs(line, stdout);
/*			fputs(line, output); */
			break;
		}
		fputs(line, output);
	}

	for(;;) {		/* copy rest of file */
		if (fgets(line, MAXLINE, input) == NULL) break;		/* EOF */
		fputs(line, output);
	}
	if (ferror(output) != 0) {
		perror(atmfile); return -1;
	}
	fclose (output);
	fclose (input);
	return 0;
}

/* Note this environment variable is unique in being lower case */
/* Note that output can be directed to file to set WINDIR */
/* decode -vx > setwindow.bat */
/* call setwindow */
/* if exist %WINDIR%\win.ini echo we found windows directory ! */

/* NOTE: cannot use `putenv' to set environment variable */
/* since that only affect the environment of this process */
/* and the change is lost when we exit */

int inwindows (void) {	/* attempt check whether running in Windows */
	char *s;
	if ((s = getenv("windir")) != NULL) {
/*		if (verboseflag != 0) printf("windir=%s\n", s); */
		if (verboseflag != 0) printf("set windir=%s\n", s);	/* 93/Dec/29 */
		return 1;
	}
	else return 0;
}

/* get rid of return, linefeed and other white space at end of line */

void stripreturn (char *name) {
	char *s;
	s = name + strlen(name) - 1;
	while (*s <= ' ' && s > name) s--;
	*(s+1) = '\0';						/* terminate it */
}

int getridof (char *qlcfile) {
	char bakfile[FILENAME_MAX];
	FILE *input;

	if ((input = fopen(qlcfile, "rb")) == NULL) {/*	see whether file exists */
		if (verboseflag != 0) perror(qlcfile);
		return 0;			/* file not found */
	}
	else {					/*	if it does, try and rename it ATMFONTS.BAK */
		fclose(input);
		strcpy (bakfile, qlcfile);
		forceexten (bakfile, "bak");
		(void) remove (bakfile);	/* get rid of old one, if any */
		if (rename (qlcfile, bakfile) == 0) {
			if (verboseflag != 0)
				printf("Succesfully renamed %s %s\n",
					qlcfile, stripname(bakfile));
			return 1;		/* success, renamed it */
		}
		if (remove(qlcfile) == 0) {	/* try deleting the darn thing anyway */
			if (verboseflag != 0)
				printf("Succesfully deleted %s\n", qlcfile);
			return 1;
		}
		return 0;		/* failed to delete */
	}
}

/* scan ATM.INI for QLCDir line and delete ATMFONTS.QLC if found */
/* return +1 if found and deleted */
/* return  0 if found QLCDir, but ATMFONTS.QLC already deleted */
/* return -1 if can't find [Settings] or can't find QLCDir= */

int scanatmini(FILE *input, char *atmini) {
	char qlcfile[FILENAME_MAX];

	for (;;) {
		if (fgets(line, MAXLINE, input) == NULL) break; /* EOF */
		if (*line == ';') continue;
		if (*line == '[') {
/*			if (strncmp(line, "[Settings]", 10) == 0) { */
			if (_strnicmp(line, "[Settings]", 10) == 0) {
				for (;;) {
					if (fgets(line, MAXLINE, input) == NULL) break;
					if (*line == ';') continue;
					if (*line == '[') break;		/* end of [Settings] */
					if (strncmp(line, "QLCDir=", 7) == 0) {
						strcpy(qlcfile, line+7);
						stripreturn(qlcfile);
						strcat(qlcfile, "\\");
						strcat(qlcfile, "atmfonts.qlc");
						return (getridof (qlcfile));
					}
				}
				if (verboseflag != 0)
		printf("Did not find `QLCDir' in [Settings] section of %s\n", atmini);
				return -1;
			}
		}
	}
	if (verboseflag != 0)
		printf("Did not find `[Settings]' section in %s\n", atmini);
	return -1;
}

/***************************************************************************/

/* Extract next pathname from a searchpath - and write into pathname */
/* return NULL if there are no more pathnames, */
/* otherwise returns pointer to NEXT pathname in searchpath */
/* searchpath = pathname1;pathname2; ... ;pathnamen */

/* used for pfb search path and eps search path */
/* this version also allows space as separator */

char *nextpathname(char *pathname, char *searchpath) {
	int n;
	char *s;

	if (*searchpath == '\0') return NULL;	/* nothing left */
	else if (((s = strchr(searchpath, ';')) != NULL) ||
		     ((s = strchr(searchpath, ' ')) != NULL)) {
		n = (s - searchpath);
		if (n >= FILENAME_MAX) {
			fprintf(stderr, "Path too long %s\n", searchpath);
			return NULL;
		}
		strncpy(pathname, searchpath, (unsigned int) n);
		*(pathname + n) = '\0';				/* terminate it */
		return(s + 1);						/* next pathname or NULL */
	}
	else {
		n = (int) strlen(searchpath);
		if (n >= FILENAME_MAX) {
			fprintf(stderr, "Path too long %s\n", searchpath);
			return NULL;
		}
		strcpy(pathname, searchpath);
		return(searchpath + n);
	}
}

/***************************************************************************/

/* New common routine 1992/Nov/28 */
/* does not use backslash if beginning is blank or already ends on : \ or / */

void makefilename(char filepath[], char *fontname) {
	char *s;
	if (strcmp(filepath, "") != 0) {	/* 1992/Oct/30 */
		s = filepath + strlen(filepath) - 1;
		if (*s != ':' && *s != '\\' && *s != '/') strcat(filepath, "\\");
	}
	strcat(filepath, fontname);
/*	extension(filepath, ext); */
}

/* Try and find Windows directory */

char *windowsdir (char *windir) {
	char *path, *searchpath;
	FILE *atmfile;
	char atmini[FILENAME_MAX];

	path = getenv("PATH");
	searchpath = path;
	for (;;) {
		if ((searchpath = nextpathname(windir, searchpath)) == NULL) break;
		strcpy(atmini, windir);
		makefilename(atmini, "atm.ini");			/* 1993/Dec/24 */
/*	try and open ATM.INI file */
		if ((atmfile = fopen(atmini, "r")) != NULL) {
			fclose (atmfile);
			return windir;
		}
	}
	return NULL;	/* couldn't find atm.ini */
}

/* [Settings] */
/* QLCDir=c:\psfonts */

/* return +1 if found and deleted */
/* return  0 if found QLCDir, but ATMFONTS.QLC already deleted */
/* return -1 if can't find ATM.INI, or [Settings] or QLCDir= */

int findcache (char *dir) {
	char filename[FILENAME_MAX];
	char *s;
	FILE *input;
	int flag=0;

	strcpy(filename, dir);
	if (strchr(filename, '.') == NULL) {
		s = filename + strlen(filename) - 1;
		if (*s != '\\' && *s != '/') strcat(filename, "\\");
		strcat(filename, "atm.ini");
	}
	if ((input = fopen(filename, "r")) == NULL) {
		if (verboseflag != 0) perror(filename);
		return -1;								/* can't find ATM.INI */
	}
	if (traceflag != 0) printf("Trying %s\n", filename);
	flag = scanatmini(input, filename);
	fclose(input);
	return flag;
}

/* Try and find ATM.INI in Windows directory - and use it ATMFONTS.QLC */

/* Returns -1 if can't find (i) ATM.INI, (ii) [Settings], (iii) QLCDir= */
/* Returns 0 if ATMFONTS.QLC already deleted */
/* Returns 1 if ATMFONTS.QLC found and deleted */

int deleteatm(char *windir) {
	int flag;
	char *s;
	char windirnew[FILENAME_MAX];					/* 1993/Dec/30 */

	if (traceflag != 0 && strcmp(windir, "") != 0) 
		printf("Given directory %s\n", windir);
/*	See whether running in Windows */
	if ((s = getenv("windir")) != NULL) {
		fprintf(stderr,
	"WARNING: Deleting ATMFONTS.QLC from inside Windows has no effect\n");
		if ((flag = findcache(s)) >= 0) return flag;
	}
/*	Try directory specified on command line */
	if (strcmp(windir, "") != 0) {
		if ((flag = findcache(windir)) >= 0) return flag;
	}
/*	Try and find Windows directory along path */
	if (windowsdir(windirnew) != NULL) {				/* 1993/Dec/30 */
		if ((flag = findcache(windirnew)) >= 0) return flag;
	}
/*	Try standard place to keep Windows */
	if ((flag = findcache("c:\\windows")) >= 0) return flag;
/*	Try current directory */
	if ((flag = findcache("")) >= 0) return flag;	
	fprintf(stderr, "ERROR: Unable to find ATM.INI\n");
	return -1;		/* failed, unable to find ATM.INI anywhere! */
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

	if (verboseflag) {
		if (strcmp(windir, "") != 0)
			printf("Found Windows directory: %s\n", windir);
	}

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

/* grab setting from `atm.ini' 94/June/15 */

/*	if (useatmini)  setupatmini(); 	*/ /* need to do this before use */

int useatmini = 1;			/* use [Setup] section in `atm.ini' */
							/* reset if setup of atm.ini file fails */

char *atmininame = "atm.ini";		/* name of ini file we are looking for */

char *atmini = "";				/* full file name for atm.ini with path */

int setupatmini (void) {
	if (useatmini == 0) return 0;		/* already tried and failed */
	if (*atmini != '\0') return 1;		/* already tried and succeeded */
	atmini = setupinifile ("atm.ini", "[Setup]");  
	if (*atmini == '\0') useatmini = 0;	/* failed, don't try this again */
	else if (verboseflag) printf("Found ATM.INI: %s\n", atmini);
	return (*atmini != '\0');
}

char *grabenv (char *varname) {	/* get from [Environment] in dviwindo.ini */
	return grabenvvar (varname, dviwindo, "[Environment]", usedviwindo);
}

char *grabsetup (char *varname) {
	return grabenvvar (varname, atmini, "[Setup]", useatmini);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void freeencoding(void) {			/* 1995/May/8 */
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], "") != 0) {
			free(encoding[k]);
			encoding[k] = "";
		}
	}
}

void freeafmencoding(void) {			/* 1995/May/8 */
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(afmdata[k].charname, "") != 0) {
			free(afmdata[k].charname);
			afmdata[k].charname = "";
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int dofacespfm (char *fname) {
	char fn_in[FILENAME_MAX];
	char *s, *t;
	FILE *input;
	int count;

	strcpy(fn_in, fname);
	lowercase(fn_in, fn_in);
	input = findfile(fn_in);
	if (input == NULL) {			/* could not find file */
		perror(fname); 
		return 0;
	}
	s = strrchr(fn_in, '.') + 1;
/*	putc('\n', stdout); */
	if ((t = strrchr(fn_in, '\\')) != NULL) t++;
	else t = fn_in;
	printf("%s\t", t);
	count = analyzepfm(input, fn_in);
	fclose(input);
	return count;
}

void showface (int firstarg, int argc, char *argv[]) {
	int k, count=0;
	char infilename[FILENAME_MAX];
	unsigned int attrib = _A_NORMAL | _A_RDONLY;
	struct _find_t fileinfo;
	unsigned flag;

	putc('\n', stdout);
	printf("File Name\tFace Name\tStyle\t\tPS FontName\n\n");

	if (argc < firstarg+1) {
		strcpy(infilename, PFM_Dir);
		strcat(infilename, "\\");
		strcat(infilename, "*.pfm");
		flag = _dos_findfirst(infilename, attrib, &fileinfo);
		if (flag != 0) {			/* if that fails ... */
			strcpy(infilename, PFB_Dir);
			strcat(infilename, "\\");
			strcat(infilename, "pfm");
			PFM_Dir = _strdup(infilename);
			strcat(infilename, "\\");
			strcat(infilename, "*.pfm");
			flag = _dos_findfirst(infilename, attrib, &fileinfo);
		}
		if (flag != 0) {
			printf("No PFM files found in %s\n", infilename);
		}
		while (flag == 0) {
			strcpy(infilename, PFM_Dir);
			strcat(infilename, "\\");
			strcat(infilename, fileinfo.name);
			count += dofacespfm(infilename);
			flag = _dos_findnext(&fileinfo);
		}
	}
	if (argc > firstarg) {
		for (k = firstarg; k < argc; k++) {
			count += dofacespfm(argv[k]);
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	FILE *input, *afm;
	char fn_in[FILENAME_MAX], fn_afm[FILENAME_MAX];
/*	char line[MAXLINE]; */
	char *s, *t;
	int k, firstarg=1, match=0, flag, count;
	int ncount = 0;

	if (argc < 2) showusage(argv[0]);

	strncpy(programpath, argv[0], sizeof(programpath));
	removepath(programpath);
/*	printf("Default program path is %s\n", programpath); */
	
	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("VECPATH")) != NULL) vectorpath = s;
/*	else if (usedviwindo) { */
	if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("VECPATH")) != NULL) vectorpath = s;
	}

	firstarg = commandline(argc, argv, 1);

	if (checkcopyright(copyright) != 0) {
		exit(69);	/* check for tampering */
	}

	if (showfaceflag) {
		if (useatmini)  setupatmini(); 	 /* need to do this before use */
		if (strcmp(PFM_Dir, "") == 0) {
			s = grabsetup("PFM_Dir");
			if (s != NULL) {
				PFM_Dir = s;
				if (verboseflag) printf("Found PFM_Dir: %s\n", PFM_Dir);
			}
			else PFM_Dir = "c:\\psfonts\\pfm";
		}
		if (strcmp(PFB_Dir, "") == 0) {	
			s = grabsetup("PFB_Dir");
			if (s != NULL) {
				PFB_Dir = s;
				if (verboseflag) printf("Found PFB_Dir: %s\n", PFB_Dir);
			}
			else PFB_Dir = "c:\\psfonts";
		}
		showface (firstarg, argc, argv);
		return 0;
	}

	lowercase (vector, vector);						/* 93/Oct/4 */
	if (strcmp(vector, "ansinew") == 0) facex = 0;
	if (strcmp(vector, "ansi") == 0) facex = 0;

	if (resetatm != 0) {
		if (firstarg <= argc - 1) s = argv[firstarg];
		else s = "";
		if (deleteatm(s) < 0)
			exit(1); /* did not find ATM.INI, or [Settings] or QLCDir= */
		else return 0;	/* file deleted, or not found */
	}

	if (checkflag != 0) {		/* check whether running in Windows */
		if (inwindows() != 0) exit(1);
		else return 0;
	}

	if (firstarg > argc - 1) showusage(argv[0]);

	if (usewin != 0) {		   /* add `X' to Face Names in ATM.INI */
		modifyatmini(argc, argv, firstarg);
		return 0;
	}

	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	for (k = 0; k < MAXCHRS; k++) encoding[k] = ""; /* 95/May/15 */

	for (k = 0; k < MAXAFMS; k++) afmdata[k].charname = "";	/* 95/May/12 */

	ncount = 0;

	for (k=firstarg; k < argc; k++) {

		strcpy(fn_in, argv[k]);
		lowercase(fn_in, fn_in);
		if (strstr(fn_in, ".vec") != NULL) continue;	/* skip vector files */
		input = findfile(fn_in);
		if (input == NULL) {			/* could not find file */
			perror(argv[k]); 
			continue;
		}
		s = strrchr(fn_in, '.') + 1;
		if (!showfaceflag)	putc('\n', stdout);
/*		if (verboseflag != 0) */
		if (showfaceflag) {
			if ((t = strrchr(fn_in, '\\')) != NULL) t++;
			else t = fn_in;
			printf("%s\t", t);
		}
		else printf("Processing %s\n", fn_in);
		flag = 0;
		if (useafm != 0) {
			if (strcmp(afmfile, "") != 0) {
				strcpy(fn_afm, afmfile);
				afm = openafm(fn_afm);
				if (afm == NULL) {
/*					perror(afmfile); */
					useafm = 0;
				}
				else {
					if (traceflag != 0) printf("Reading %s\n", afmfile);
					if (readafmfile (afm) < 0) useafm = 0;
					fclose (afm);
				}
			}
			else useafm = 0;
		}
/*		if (strncmp(s, "tfm", 3) == 0) { */
		if (strncmp(s, "tf", 2) == 0) {			/* 1994/Feb/4 */
			count = analyzetfm(input, fn_in);
			if (useafm != 0) {
				guessvecbin (argc, argv, firstarg, count);
			}
		}
		else if (strncmp(s, "pfb", 3) == 0 ||
			     strncmp(s, "pfa", 3) == 0) {
		    if (useafm == 0) {
			printf("WARNING: may need AFM file to say much about PFB file\n");
				fixedwidth = 0;
			}
			count = analyzepfb(input, fn_in);
			if (count == 0) {	/* Was StandardEncoding */
				fclose(input);
				freeencoding();			/* 95/May/15 */
				ncount++;
				continue;
			}
			guessvecpfb (argc, argv, firstarg, count);
		}
/*		else if (strncmp(s, "pfm", 3) == 0) { */
		else if (strncmp(s, "pf", 2) == 0) {	/* 1994/Feb/2 */
			count = analyzepfm(input, fn_in);
			if (useafm != 0) {
				guessvecbin (argc, argv, firstarg, count);
			}
		}
		else fprintf(stderr, "Sorry, don't recognize file extension in `%s'\n",
			fn_in);
/*		if (flag != 0) fprintf(stderr, "Sorry, unable to process file `%s'\n",
			fn_in);	 */ /* unreachable */
		fclose(input);
		freeencoding();
		ncount++;
	}
	freeafmencoding();			/* 95/May/15 */
	if (verboseflag) putc('\n', stdout);
	if (verboseflag && ncount > 1)	printf("Processed %d files\n", ncount);
	return 0;
}

/* check on contradition between flags in PFM and announced encoding */

/* check on remapping in PFM ? */

/* check on encoding based on AFM and potential encoding vectors ? */

/* worry about problems with fixed width fonts */

/* may need to move some data out into malloc space */

/* can pass in encoding vector */
/* if it is ansinew (or ansi) then *remove* trailing `X' from face name */
/* if it is not ansinew then *add* trailing `X' to face name */

/* [Setup] */
/* PFM_Dir=c:\psfonts\pfm */
/* PFB_Dir=c:\psfonts */
