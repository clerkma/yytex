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

/* Program for packing TFM files into TeXtures metric file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>

/* #define FNAMELEN 80 */
/* #define MACBINARYHEAD 128 */
/* #define MAXRESOURCES 1024 */
#define MAXRESOURCES 128
/* #define MAXRESNAME 9 */
#define MAXRESNAME 20

/* #define VERSION "1.1"	*/		/* version of this program */
#define VERSION "1.2"				/* version of this program */
#define MODDATE "1997 MARCH 15"		/* date of last modification */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright = "\
Copyright (C) 1992-1998  Y&Y, Inc (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1992-1997  Y&Y. All rights reserved. (508) 371-3286\ */

/* #define COPYHASH 13986445 */
/* #define COPYHASH 15364508 */
/* #define COPYHASH 9294052 */
#define COPYHASH 15177281

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;

int lowercase = 0;		/* force resource name to lower case */
int useqdname = 0;		/* if no XEROX header */
int usemacenc = 0;		/* Use new Textures 1.6 or later MacintoshStandard */

int fixposition = 1;		/* fixed desktop position */
int tryunderscore = 1;

int tfmindex;			/* current tfm file working on */

/* struct stat filestat; */
struct _stat filestat;

unsigned long yearsoff = 86400 * 1461;		/* offset 1904 - 1900 */
/* unsigned long yearsoff = 86400 * 24107; */		/* offset 1970 - 1904 */
/* unsigned long hoursoff = 3600 * 8; */	/* GMT - EZT ??? */
unsigned long hoursoff = 3600 * 9;				/* GMT - EZT ??? */

char *macfile="textures.met";	/* file name of output file */

char tfmfile[FILENAME_MAX];	/* file name of input TFM file */

char vffile[FILENAME_MAX];	/* file name of input VF file if any */

char MacName[FILENAME_MAX];	/* MacBinary header name */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

void removeexten(char *fname) {  /* remove extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) != NULL) {
		if ((t = strrchr(fname, '\\')) == NULL || s > t) *s = '\0';
	}
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

void removeunder(char *filename) { /* remove Adobe style underscores */
	char *s;
	s = filename + strlen(filename) - 1;
	while (*s == '_') s--;
	*(s + 1) = '\0';
}

void forceupper(char *s, char *t) { /* convert to upper case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}


void forcelower(char *s, char *t) { /* convert to lower case letters */
	int c;
	while ((c = *t++) != '\0') {
		if (c >= 'A' && c <= 'Z') *s++ = (char) (c + 'a' - 'A');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *filetype = "FFIL";	
char *filecreator = "*TEX";	

char *tfmres = "*TFM";					/* resource type */

unsigned char fileflags = 0;		/* was: 32 | 1 */

unsigned char protectedflags = 0;

int desktopv=-1;		/* vertical position of icon on desktop */
int desktoph=-1;		/* horizontal position of icon on desktop */
int desktopd=0;			/* directory of icon on desktop */

unsigned long dataforklength = 0;		/* no data fork, so zero */
unsigned long resourceforklength;		/* determined later */

unsigned long creationdate;				/* seconds since 1904 Jan 1 */
unsigned long modificationdate;			/* seconds since 1904 Jan 1 */

unsigned int resourcefileattributes = 0;		/* resource file attributes */

unsigned long ResDatOff=256;	/* offset to resource data */
unsigned long ResMapOff;		/* offset to resource map - later */
unsigned long ResDatLen;		/* length of resource data - later */
unsigned long ResMapLen;		/* length of resource map - later */

/* most of this is filled in only after first pass - written on second */

unsigned int TypeOffset;			/* offset to start of type list */
unsigned int NameOffset;			/* offset to start of name list */

unsigned long resourcepointer;			/* pointer into resource data */

unsigned long resourcemapstart;			/* pointer to start of resource map */
unsigned long referenceliststart;		/* pointer to start of references */
unsigned long typeliststart;			/* pointer to start of types */
unsigned long nameliststart;			/* pointer to start of names */

unsigned int nextresource;					/* pointer to next available */

unsigned long resourcedata[MAXRESOURCES];	/* pointer - resource data item */
											/* relative begin resource data */

char resourcename[MAXRESOURCES][MAXRESNAME];	/* names of resources */

unsigned long nameoffset[MAXRESOURCES];		/* where ResName will be */

unsigned int numberoftfms;					/* count of *TFM resources */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void uwritetwo(FILE *output, unsigned int n) {
	unsigned char c, d;
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
	putc(c, output); putc(d, output);
}

void uwritethree(FILE *output, unsigned long n) {
	unsigned char c, d, e;
	e = (unsigned char) (n & 255);	n = n >> 8;
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
	putc(c, output); putc(d, output);
	putc(e, output);
}

void uwritefour(FILE *output, unsigned long n) {
/*	int c, d, e, f; */
	unsigned char c, d, e, f;
	f = (unsigned char) (n & 255);	n = n >> 8;
	e = (unsigned char) (n & 255);	n = n >> 8;
	d = (unsigned char) (n & 255);	n = n >> 8;
	c = (unsigned char) (n & 255);
	putc(c, output); putc(d, output);
	putc(e, output); putc(f, output);
}

unsigned long ureadfour(FILE *input) {	/* LS => MS */
	unsigned int c, d, e, f;
	c = getc(input); d = getc(input);
	e = getc(input); f = getc(input);
	return (((((((unsigned long) f) << 8) | ((unsigned long) e) << 8) |
		d) << 8) | c);
}

void writestring(FILE *output, char *str, unsigned int n) {
	unsigned int k;
	char *s = str;
	for (k = 0; k < n; k++) putc(*s++, output);
}

void writepascalstring(FILE *output, char *str) {
	unsigned int k, n = strlen(str);
	char *s = str;
	putc(n, output);
	for (k = 0; k < n; k++) putc(*s++, output);	
}

void writezeros(FILE *output, int n) {
	int k;
	for (k = 0; k < n; k++) putc(0, output);
}

void writeresourceheader(FILE *output) {
	uwritefour(output, ResDatOff);		/* offset to resource data */
	uwritefour(output, ResMapOff);		/* offset to resource map - later */
	uwritefour(output, ResDatLen);		/* length of resource data - later */
	uwritefour(output, ResMapLen);		/* length of resource map - later */
}

void writeuserdefined(FILE *output) {
	writezeros(output, 128);
}
	
void setupdateandtime(char *name) {
	time_t modtime;
/*	if (stat(name, &filestat) != 0) { */
	if (_stat(name, &filestat) != 0) {
		fprintf(stderr, "WARNING: Can't get file modification date\n");
		return;				/* exit(9); */
	}
	modtime = filestat.st_atime;
	if (verboseflag != 0) 
		printf("File modification date: %s", ctime(&modtime));
/*	creationdate = modtime + yearsoff - hoursoff; */
	creationdate = modtime - yearsoff - hoursoff;
/*	modificationdate = modtime + yearsoff - hoursoff; */
	modificationdate = modtime - yearsoff - hoursoff;
}

/* ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  ***  */

/* PROBLEMS WHEN SWITCHING TO 32 BIT */

int sreadtwo(FILE *input) {
	int c, d;
	c = getc(input);
	d = getc(input);
	return ((c << 8) | d);
}

void swritetwo(FILE *output, int len) {
	int c, d;
	c = (len >> 8) & 255;
	d = len & 255;
	putc(c, output); putc(d, output);
}

#define XEROXHEAD 18			/* 2 + 10 + 5 + 1 */

int lf, lh, bc, ec, nw, nd, nh, ni, nl, nk, ne, np, lfdash, lhold;

int checktfmhead (FILE *input, int pass) {
	lf = sreadtwo(input);	lh = sreadtwo(input);
	bc = sreadtwo(input);	ec = sreadtwo(input);
	nw = sreadtwo(input);	nh = sreadtwo(input);
	nd = sreadtwo(input);	ni = sreadtwo(input);
	nl = sreadtwo(input);	nk = sreadtwo(input);
	ne = sreadtwo(input);	np = sreadtwo(input);
	lfdash = 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np;
	if (lf != lfdash) {
		if (pass == 1)
		fprintf(stderr, 
	"ERROR: %s inconsistent header (%d <> %d) - perhaps not a TFM file?\n",
			tfmfile, lf, lfdash);
	}
	if (lh != XEROXHEAD) {
		if (pass == 1) {
			if (lh < 2) 
				fprintf(stderr,
					"ERROR: %s missing required header words\n", tfmfile);
			fprintf(stderr,
"WARNING: %s missing XEROX style header (%d <> %d) --- will make space\n",
					tfmfile, lh, XEROXHEAD);
			if (lh > XEROXHEAD)	
				fprintf(stderr, "ERROR: %s header too long\n", tfmfile);
/*			useqdname = 0; */	/* can't rely on that being there anymore! */
		}
		return lh;
	}
	return 0;
}

void writehead (FILE *output) {
	swritetwo(output, lf);
	swritetwo(output, lh);
	swritetwo(output, bc);
	swritetwo(output, ec);
	swritetwo(output, nw);
	swritetwo(output, nh);
	swritetwo(output, nd);
	swritetwo(output, ni);
	swritetwo(output, nl);
	swritetwo(output, nk);
	swritetwo(output, ne);
	swritetwo(output, np);
}

char *ENCOD="";

char *MacENCOD="MacintoshStandard";

/* write out string padded with zeros to 32 bit boundary */
/* return number of bytes written */

int outstring (FILE *output, char *name) {
	int i, n, no;
	char *s=name;

	n = strlen(name);
	if (n == 0) return 0;
	no = ((n + 3) / 4) * 4; 
	for (i = 0; i < n; i++) putc(*s++, output);
	for (i = n; i < no; i++) putc(0, output);
	return no;
}

int trimPS(char *PSshort, char *Modifier, char *PSname) {
	int c;
	char *s;

/*	strip initial `PostScript ' or `PS ' */
	if (strncmp(PSname+1, "PostScript ", 11) == 0)
		strcpy(PSshort+1, PSname+1 + 11);
	else if (strncmp(PSname+1, "PS ", 3) == 0) 
		strcpy(PSshort+1, PSname+1 + 3);
	else {
/*		printf("PSname was `%s'\n", PSname); */
		printf("PSname was `%s' (%d)\n", PSname+1, *PSname);
		strcpy(PSshort, PSname);
	}
/*	see whether there is a modifier following the PS name */
	if ((s = strchr(PSshort+1, ',')) != NULL) {
		*s = '\0';					/* isolate PS FontName */
		strcpy(Modifier, s+1);		
		s = Modifier + strlen(Modifier) - 1;
		if (*s == ',') *s = '\0';	/* strip final comma if any */
	}
	else *Modifier = '\0';			/* no PS font modifier */
/*	it's a Pascal string, so needs a valid length byte */
	c = strlen(PSshort+1);
	*PSshort = (char) c; 
	return (c+1);			/* total length */
}

/*		it's a Pascal string, so needs a valid length byte */

/* First pass (0) establish lengths of things, second pass (1) actually copy */

/* The extra BSR stuff is a bity messy, having been added later on... */

void copytfmfile(FILE *output, FILE *input, int pass,
			long vflength, FILE *vfinput) {
	unsigned long k;
/*	int i, no; */
	int n, m, flag, c;
	char *s, *sdot;
	unsigned long tfmlength;
	long l;
	int vfwords=0;				/* words in VF file */
	int checkcount;				/* FLUSH, once it is debugged */
	char QDname[20], PSname[40], PSshort[40], Modifier[40];
	int nextra=0;				/* number of extra words for BSR header */
	long current;
	int nQDname, nPSname, nPSshort, nModifier, nENCOD;	/* field lengths */

	flag = checktfmhead(input, pass);
	fseek(input, 0L, SEEK_END);
	tfmlength = ftell(input);			/* file length */
	if (lf * 4 != (int) tfmlength && pass == 1)
		fprintf(stderr, 
			"WARNING: %s length (%d) does not agree with header info (%d)\n",
				tfmfile, (int) tfmlength, lf * 4);
	fseek(input, 0L, SEEK_SET);			/* rewind */

	if (pass == 0) {
		if (lh != XEROXHEAD) {			/* adjust for xerox header */
			tfmlength += (XEROXHEAD - lh) * 4;
			lf += (XEROXHEAD - lh);
			lh += (XEROXHEAD - lh);	/* i.e. force lh == XEROXHEAD */
		}
	}
/*	Add extra header info if VF  -or-  nCOD */
/*	if (vflength > 0) {			/* 1993/June/19 */
	if (vflength > 0 || usemacenc != 0) {			/* 1993/July/19 */
		if (lh != XEROXHEAD) {	/* can't happen ... */
			vflength = 0;
			fprintf(stderr,
				"ERROR: cannot add VF file to TFM with non-standard header\n");
		}
		vfwords = (int) ((vflength + 3) >> 2);		/* how many `words' */
/*		read TFM file to see how many words needed for 	PSname, QDname etc */
/*		copy PS FontName, QD FontName, `FaceByte' */

		current = ftell(input);
		fseek(input, 32L, SEEK_SET);
		s = PSname;			/*			read PS name field */
		for (k = 0; k < 40; k++) *s++ = (char) getc(input);
		s = QDname;			/*			read QD name field */
		for (k = 0; k < 20; k++) *s++ = (char) getc(input);
		fseek(input, current, SEEK_SET);

/*		Trim off `PS ' or `PostScript ' if found in PS name */
		trimPS(PSshort, Modifier, PSname);

/*		PSname & QDname are Pascal Strings - extra byte already counted */
/*		Modifier, ENCOD are not pascal strings - no extra byte counted */
		nPSname = strlen(PSname);
		nPSshort = strlen(PSshort);
		nModifier = strlen(Modifier);
		nQDname = strlen(QDname);
		nENCOD = strlen(ENCOD);
/* Extra words needed, for BSR `ID' word, field count (7) word, 7 fields, */
/* plus space for QDname, PSname, ENCOD, Modifier and VF */
		nextra = 1 + 1 + 7;	/* `ID' word, field count, field lengths */
		nextra += (nQDname + 3) >> 2;
/*		nextra += (nPSname + 3) >> 2; */
		nextra += (nPSshort + 3) >> 2;
		nextra += (nModifier + 3) >> 2;
		nextra += (nENCOD + 3) >> 2;
		lh += nextra + vfwords;			/* increase length of TFM header */
		lf += nextra + vfwords;			/* increase length of TFM `file' */
	}

	if (pass > 0) {		/* second pass, just copy the TFM file */
		if (lh == XEROXHEAD) {		/* trivial case */
			uwritefour(output, tfmlength);
			for (k = 0; k < tfmlength; k++) putc(getc(input), output);
		}
/*		else if (vflength > 0) { */			/* 1993/June/19 */
		else if (vflength > 0 || usemacenc != 0) {	/* 1993/July/19 */
			uwritefour(output, tfmlength + ((nextra + vfwords) << 2));
			checkcount=0;
/*			This assumes the TFM file has proper XEROX header */
/*			copy twelve 16-bit length fields */
			for (k = 0; k < 4; k++) (void) getc(input); 
/*			write modified lf and lh */
			swritetwo(output, lf);
			swritetwo(output, lh);
/*			copy ten 16-bit length fields */
			for (k = 4; k < 24; k++) putc(getc(input), output); 
			checkcount += 24;
/*			now we are in the header -> copy checksum, design size */
			for (k = 0; k < (2 << 2); k++) putc(getc(input), output);
			checkcount += 8;

/*			copy PS FontName, QD FontName, `FaceByte' */
/*			Modify this so the two fields are equal ? */
			s = PSname;			/*			read PS name field */
			for (k = 0; k < 40; k++) *s++ = (char) getc(input);
			s = QDname;			/*			read QD name field */
			for (k = 0; k < 20; k++) *s++ = (char) getc(input);
			s = QDname;			/* write QD name in place of PS name */
			for (k = 0; k < 20; k++) putc(*s++, output);
/*			pad out PS name field with (40 - 20) zeros */
			for (k = 0; k < 20; k++) putc(0, output); 
/*			Copy QD name (again) */
/*			for (k = 0; k < 20; k++) putc(getc(input), output);  */
			s = QDname;			/* write QD name */
			for (k = 0; k < 20; k++) putc(*s++, output);
/*			Face Byte */	/* If new MacintoshEncoding - force code to 3 */
			if (usemacenc != 0) {
				c = getc(input); putc(3, output);	/* PS code */
				c = getc(input); putc(3, output);	/* QD code */
				c = getc(input); putc(0, output);
				c = getc(input); putc(c, output);	/* style */
			}
			else for (k = 0; k < 4; k++) putc(getc(input), output); 
			checkcount += 64;

/*			Trim off `PS ' or `PostScript ' if found in PS name */
			trimPS(PSshort, Modifier, PSname);

/*			Should one also force two names to be the same here ??? NO */
			nPSname = strlen(PSname);
			nPSshort = strlen(PSshort);
			nModifier = strlen(Modifier);
			nQDname = strlen(QDname);
			nENCOD = strlen(ENCOD);
/*			add BSR mysterious ID bytes for Textures ???  */
			putc(222, output);
			putc(173, output);
			putc(250, output);
			putc(206, output);
			uwritefour(output, 7);			/* number of fields */
			uwritefour(output, nQDname);	/* length of QD name field */
/*			uwritefour(output, nPSname); */
			uwritefour(output, nPSshort);	/* length of PS name field */
			uwritefour(output, nENCOD);		/* length of ENCOD field */
			uwritefour(output, 0);			/* ??? */
			uwritefour(output, nModifier);	/* length of Modifier field */
			uwritefour(output, vflength);	/* length of VF file */
			uwritefour(output, 0);			/* ??? */
			checkcount += 36;

			checkcount += outstring(output, QDname);	/* QD name */

/*			checkcount += outstring(output, PSname); */
			checkcount += outstring(output, PSshort);	/* PS name */

			checkcount += outstring(output, ENCOD);		/* nCOD */

			checkcount += outstring(output, Modifier);	/* PS font modifier */

/*			now copy VF file */		
			for (l = 0; l < vflength; l++)	putc(getc(vfinput), output);
			checkcount += (int) vflength;
/*			pad out to multiple of 4 bytes */
			if (vflength < (vfwords  << 2)) {
				for (l = vflength; l < (vfwords << 2); l++)
					putc(248, output);
				checkcount += (vfwords << 2) - (int) vflength;
			}
/*			copy rest of TFM file now */
			for (k = 24 + 8 + 64; k < tfmlength; k++)
				putc(getc(input), output);
			checkcount += (int) tfmlength - (24 + 8 + 64);
/*			if (checkcount !=  (int) tfmlength + (vfwords << 2) + (48+4))
				fprintf(stderr, "ERROR: mismatch in lengths %d %d\n",
					checkcount, (int) tfmlength + (vfwords << 2) + (48+4)); */
			if (checkcount !=  (int) tfmlength + ((vfwords + nextra) << 2))
				fprintf(stderr, "ERROR: mismatch in lengths %d %d\n",
					checkcount, (int) tfmlength + ((vfwords + nextra) << 2));
		}
		else {				/* in case not XEROX header */
			(void) checktfmhead(input, -1);		/* skip over bad header */
			tfmlength += (XEROXHEAD - lh) * 4;
			lf += XEROXHEAD - lh;
			lhold = lh;
			lh += XEROXHEAD - lh;	/* i.e. lh = XEROXHEAD */
			uwritefour(output, tfmlength);
			writehead(output);				/* write good header */
/*			copy the existing header */
			for (m = 0; m < 4 * lhold; m++) putc(getc(input), output);
/*			pad with zeros to XEROX header length */
			for (m = 4 * lhold; m < XEROXHEAD * 4; m++) putc(0, output);
/*			copy rest of TFM file */
			for (m = 24 + XEROXHEAD * 4; m < (int) tfmlength; m++) 
				putc(getc(input), output);			
		}
	}				/* end of if (pass == 1) */
/*	following done in first pass */
	else if (useqdname != 0 && flag == 0) {		/* don't use unless XEROX */
		for (k = 0; k < 72; k++) (void) getc(input);
		n =  getc(input);
		if (n > 0 && n < 20) {
			s = resourcename[tfmindex+1];	/* + 1 ??? */
			for (k = 0; k < (unsigned long) n; k++) 
				*s++ = (char) getc(input);
			*s = '\0';
			for (k = 72+n+1; k < tfmlength; k++) (void) getc(input);
		}
		else for (k = 72+1; k < tfmlength; k++) (void) getc(input);
	}
	else for (k = 0; k < tfmlength; k++) (void) getc(input); /* skip over */

	resourcepointer += tfmlength + 4;
/*	if (vflength > 0) */
	if (vflength > 0 || usemacenc != 0)		/* 1993/July/19 */
/*		resourcepointer += (vfwords << 2) + (48+4); */ /* ??? */
		resourcepointer += ((vfwords + nextra) << 2);
	resourcedata[nextresource] = resourcepointer;

/*	allow for alternate source of ResName - QuickDraw name */

	if (useqdname == 0 || flag != 0) {		/* use if not XEROX header */
		s = stripname(tfmfile);
		sdot = strchr(s, '.');
		if (sdot != NULL) n = sdot - s;	else n = 8;
		strncpy(resourcename[nextresource], s, n);
		resourcename[nextresource][n] = '\0';
		if (lowercase != 0) 
			forcelower(resourcename[nextresource], resourcename[nextresource]);
	}
	nextresource++;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void addresources(FILE *output, int pass) {	/* resources after font data */
/* 	unsigned int n, m; */

/*	if (wantmessage != 0) {		
		n = strlen(message);
		if (pass > 0) 
			uwritefour(output, (unsigned long) (n + 1));
		resourcepointer += 4;
		if (pass > 0) 
			writepascalstring(output, message);
		resourcepointer += n + 1;
		resourcedata[nextresource++] = resourcepointer;
	} */
/*	fill in data now available */
	ResDatLen = resourcepointer;	/* resource data length */
	ResMapOff = 256 + resourcepointer;	/* resource data map start */
}

unsigned int referoffset;				/* pointer into reference list */

void writetype(FILE *output, char *name, unsigned int n, int pass) {
	if (pass > 0) {
		writestring(output, name, 4);
		uwritetwo(output, n - 1);
		uwritetwo(output, referoffset);		/* offset to type list */
	}
	referoffset += 12 * n;
	resourcepointer += 4 + 2 + 2;
}

/* following is a total kludge - too lazy to fix */

void writereferitem(FILE *output, unsigned int resid, unsigned int attribute, 
		unsigned long resptr, char *resname, int pass) {
	if (pass > 0) {
		uwritetwo(output, resid);		/* resource ID */
		if (strcmp(resname, "") == 0) {
/*			uwritetwo(output, -1);  */
			uwritetwo(output, 
				(unsigned int) (nameoffset[tfmindex+1] - nameliststart));
		}
		else { 
/* offset by one because first item is *TFM ??? */
			nameoffset[tfmindex+1] = 
				nameoffset[tfmindex] + strlen(resname) + 1; 
			uwritetwo(output, 
/*				(unsigned int) (nameoffset[tfmindex] - nameliststart)); */
			(unsigned int) (nameoffset[tfmindex+1] - nameliststart));
/*			nameoffset[tfmindex+1] = 
				nameoffset[tfmindex] + strlen(resname) + 1; */
		} 
		putc(attribute, output);		/* Resource Attributes */
		uwritethree(output, resptr);	/* pointer to resource data */
		uwritefour(output, 0L);			/* RESERVED */
	}
/*	nameoffset[tfmindex+1] = nameoffset[tfmindex] + strlen(resname) + 1; */
	resourcepointer += 12;
}

void writenamelist(FILE *output, int pass) {
	unsigned int k, n;
/*	scrindex += nameoffset;	*/	/* how much space we actually used earlier */
/*	resourcepointer += nameoffset; */
/*	for (k = 0; k < numberoftfms; k++) { */
	for (k = 1; k <= numberoftfms; k++) {
		nameoffset[k] = resourcepointer;
/* offset by one because first resource name is *TFM ... ??? */
		if (pass > 0) writepascalstring(output, resourcename[k]);
		n = strlen(resourcename[k]);
		resourcepointer += n + 1;
	}
}

/* NOTE: items following must be in same order as below ... */

void writetypelist(FILE *output, int pass) {
	int ntypes = 1;					/* number of different resource types */

	referoffset = 2 + ntypes * 8;
	if (pass > 0)
		uwritetwo(output, ntypes - 1);		/* number of types - 1 */
	resourcepointer += 2;
/*  start with *TFM resources */
	writetype(output, tfmres, numberoftfms, pass);
/*	if (wantmessage != 0) writetype(output, mstring, 1, pass); */
}

/* NOTE: items following must be in same order as above ... */

void writereferencellist(FILE *output, int pass) {
	int resourceid=501;		/* for *TFM resources */ /* random ??? */
	unsigned int k=0;

/*	while (k < numberoftfms) { 	*/	/* *TFM */
	for (k = 0; k < numberoftfms; k++) {
		tfmindex = k;	
		writereferitem(output, resourceid++, 32, resourcedata[k],  /* ??? */
			resourcename[k], pass);	/* k + 1 */
/*		k++; */
	}
}

#define DESKINCREMENT 64 /* #define DESKINCREMENT 50 */
#define DESKTOPWIDTH 350

void updatedesktop(void) {	/* pick a new desktop location */
	if (desktoph > DESKTOPWIDTH) {
		desktoph = 0; desktopv += DESKINCREMENT;
	}
	else desktoph += DESKINCREMENT;
}

/* Write MacBinary header - maybe don't bother to write when pass == 0 ? */

void writemacbinary(FILE *output) {
	unsigned int n;
/*	int k; */

/*	if (pass == 0) return;  */

	putc(0, output);						/* ZERO 1 */
/*	Actual Mac filename needs to be the same as that in header... */
/*	n = strlen(macfilename); */
	n = strlen(MacName);
/*	writepascalstring(output, macfilename); */
	writepascalstring(output, MacName);
	writezeros(output, 64 - (n + 1));

	writestring(output, filetype, 4);
	writestring(output, filecreator, 4);
	putc(fileflags, output);
	putc(0, output);						/* ZERO 2 */
/*	for (k = 0; k < 6; k++) putc(desktoplocation[k], output); */
	uwritetwo(output, desktopv);	/* vertical position */
	uwritetwo(output, desktoph);	/* horizontal position */
	uwritetwo(output, desktopd);	/* directory */
	if (fixposition == 0) updatedesktop();
	putc(protectedflags, output);
	putc(0, output);						/* ZERO 3 */
	uwritefour(output, dataforklength);		/* always zero */
	uwritefour(output, resourceforklength);	/* need to fix second pass */
	uwritefour(output, creationdate);
	uwritefour(output, modificationdate);
	writezeros(output, 128 - 99);
}

int tfmtometbegin(FILE *output, int pass) {
/*	unsigned long m; */

/*	first comes the MacBinary header of 128 bytes */
	if (pass > 0)
		writemacbinary(output);
	
/*	data fork --- which is empty */
	
/*  resource fork --- which where all the action is */
	
/*	first comes the resource header */
	
	if (pass > 0) {
		writeresourceheader(output);	/* resource header */
		writezeros(output, 128 - 16);	/* fill to 128 bytes with zeros */
	}

/*  second comes the user defined data - 128 bytes of zeros */
	
	if (pass > 0)
		writeuserdefined(output);		/* user-defined area - application */

	ResDatOff = 128 + 128;				/* resource header + user defined */
	
/*  third comes the TFM resource data of the outline font itself */	
	return 0;
}


int tfmtometend(FILE *output, int pass) {
	unsigned long m; 
	int k;

	numberoftfms = nextresource - 1;	/* number of *TFM resources */
	if (nextresource >= MAXRESOURCES) {
		fprintf(stderr, 
			"ERROR: Too many (%d) resources for table\n", nextresource);
		return -1;
	}

/*	finish off resource data  with other resources - icons and such */

	addresources(output, pass);
	if (nextresource >= MAXRESOURCES) {
		fprintf(stderr, 
			"ERROR: Too many (%d) resources for table\n", nextresource);
		return -1;
	}

/*	fourth in the resource fork comes the resource map */

	resourcemapstart = resourcepointer;	/* remember this place */

	if (pass > 0) {
		writeresourceheader(output);	/* repeat resource header 16 bytes */
		writezeros(output, 4);			/* reserved - handle to next res map */
		writezeros(output, 2);			/* reserved - file reference number */
		uwritetwo(output, resourcefileattributes);
	}
	TypeOffset = (unsigned int) (typeliststart - resourcemapstart);
	if (pass > 0)
		uwritetwo(output, TypeOffset);		/* offset to type list */
	NameOffset = (unsigned int) (nameliststart - resourcemapstart);
	if (pass > 0)
		uwritetwo(output, NameOffset);		/* offset to type list */
	resourcepointer += 16 + 4 + 2 + 2 + 2 + 2;
	
	typeliststart = resourcepointer;
/*	now write the resource Type List */
	writetypelist(output, pass);
	referenceliststart = resourcepointer;	/* remember this place */

	if (traceflag != 0) {
		for (k = 1; k <= (int) numberoftfms; k++) {
			printf("%d: (%ld) %s ", k, nameoffset[k], resourcename[k]); 
		}
		printf("\n");
	}
/*  now write the reference list */
	writereferencellist(output, pass);
	nameliststart = resourcepointer;		/* remember this place */
	
	writenamelist(output, pass);			/* ??? */

	ResMapLen = resourcepointer -  resourcemapstart;

	resourceforklength = resourcepointer + 128 + 128;

	m = dataforklength + resourceforklength;
	if (pass > 0) {
		while(m++ % 512 != 0) putc(0, output);
	}
	return 0;
}

void showusage(char *name) {
	printf("Useage: %s [-{v}{l}{m}] [-o=<output>]\n", name);
	printf("\t\t<tfm-file-1>, <tfm-file-2>, ...<tfm-file-n>\n");
	if (detailflag == 0) exit(0);
	printf("\tv  verbose mode\n");
	printf("\tl  force TeXtures font name to lower case\n");
/*	printf("\tq  use QuickDraw font name for TeXtures font name\n"); */
/*	printf("\t   (default is to use file name instead)\n"); */
	printf("\tm  Use MacintoshStandard nCOD (Textures 1.6 or later)\n");
	printf("\to  Use specified output file name\n");
	printf("\n");
	printf("\tDefault output file name is `%s'\n", macfile);
	printf("\tOutput is in MacBinary format & appears in current directory.\n");
	printf("\tVF files will be included if found in same directory as TFM.\n");
/*	printf("\t   TeXtures font name is TFM file name\n"); */
	exit(0);
}

int horizflag=0, vertiflag=0, depthflag=0, outputflag=0;

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0;
		case 'v': verboseflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case 'l': lowercase = 1; return 0;
		case 'q': useqdname = 1; return 0;
		case 'm': usemacenc = 1; return 0;
		case 'z': desktoph = 0; desktopv = 0; fixposition = 0; return 0;
/*		the rest need arguments */
		case 'o': outputflag = 1; break;
		case 'x': horizflag = 1; fixposition = 1; break;
		case 'y': vertiflag = 1; fixposition = 1; break;
		case 'd': depthflag = 1; fixposition = 1; break;
		default: {
			fprintf(stderr, "WARNING: Invalid command line flag '%c'\n", c);
				exit(7);
		}
	}
	return -1;		/* need argument */
}

int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command line flags */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break; 
			else if (decodeflag(c) != 0) { /* flag requires argument ? */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (horizflag != 0) {
					if (sscanf(s, "%d", &desktoph) < 1) {
						fprintf(stderr, "Don't understand position: %s\n",
							s);
					}
					horizflag = 0;
				}
				else if (vertiflag != 0) {
					if (sscanf(s, "%d", &desktopv) < 1) {
						fprintf(stderr, "Don't understand position: %s\n",
							s);
					}
					vertiflag = 0;
				}
				else if (depthflag != 0) {
					if (sscanf(s, "%d", &desktopd) < 1) {
						fprintf(stderr, "Don't understand position: %s\n",
							s);
					}
					depthflag = 0;
				}
				else if (outputflag != 0) {
					macfile = s;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

unsigned long hashstring(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	return hash;
}

unsigned long checkcopyright(char *s) {
	unsigned long hash;
	hash = hashstring(s);
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(stderr, "COPYHASH %ld\n", hash);	/* (void) getch();  */
	return hash;
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

unsigned int makerandom(unsigned int bottom, unsigned int top) {
	unsigned int trial;
	for(;;) {
		trial = rand() + rand();
		if (trial > bottom && trial < top) return trial;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	FILE *input, *output, *vfinput=NULL;
/*	unsigned long hash; */
	int m, firstarg = 1;
/*	char *s; */
/*	int vffileflag; */
	int vfexists;			/* non-zero if VF found */
	unsigned long vflength;	

	if (argc < 2) showusage(argv[0]);
	
/*	if (strchr(argv[1], '?') != NULL) showusage(argv[0]); */

	firstarg = commandline(argc, argv, 1);

	if (firstarg >= argc) showusage(argv[0]);

	printf("TFMtoMET font metric conversion program version %s\n", VERSION);

	if (usemacenc != 0) ENCOD = MacENCOD;	/* MacintoshStandard */

	if (fixposition != 0 && argc > firstarg &&
		desktoph != -1 && desktopv != -1) {
		fprintf(stderr, 
			"WARNING: Attempt to fix positon of multiple icons\n");
		fixposition = 0;
	}

	if (checkcopyright(copyright) != 0) {
/*		fprintf(stderr, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	if((output = fopen(macfile, "wb")) == NULL) {
		perror(macfile); exit(3);
	}
	strcpy(MacName, macfile);	/* actual Mac file name should be the same */

	resourcepointer = 0;
	nextresource = 0;
	resourcedata[nextresource++] = resourcepointer;
	nameoffset[0] = 0;

/*	reset resource name in case useqdname set and header not XEROX */
	for (m = 0; m < MAXRESOURCES; m++) strcpy(resourcename[m], "");

	(void) tfmtometbegin(output, 0);		/* first pass */

	for (m = firstarg; m < argc; m++) {		/* first pass */
		tfmindex = m - firstarg;
		strcpy(tfmfile, argv[m]);
		extension(tfmfile, "tfm");
		if((input = fopen(tfmfile, "rb")) == NULL) {
			if (tryunderscore != 0) underscore(tfmfile);
			if((input = fopen(tfmfile, "rb")) == NULL) {			
				strcpy(tfmfile, argv[m]);
				extension(tfmfile, "tfm");
				perror(tfmfile); exit(3);
			}
		}
		
		if (m == firstarg) setupdateandtime(tfmfile);

		printf("Processing %s\n", tfmfile);
		
		vfexists = 0;
		vflength = 0;
		strcpy (vffile, tfmfile);
		forceexten(vffile, "vf");
		if ((vfinput = fopen(vffile, "rb")) == NULL) {
			forceexten(vffile, "vfx");
			vfinput = fopen(vffile, "rb");
		}
		if (vfinput != NULL) {
			vfexists = 1;
			fseek(vfinput, 0L, SEEK_END);
			vflength = ftell(vfinput);
/*			fseek(vfinput, 0L, SEEK_SET); */
			fclose(vfinput); 
			vfinput=NULL;
			if (verboseflag != 0)
				printf("VF file %s found (%d bytes)\n", vffile, vflength);
			if (vflength %4 != 0)
				fprintf(stderr, "WARNING: VF file length not multiple of 4\n");
		}

		dataforklength = 0;						/* assumes zero throughout */
		
/*		(void) tfmtomet(output, input, 0); */		/* first pass */

		copytfmfile(output, input, 0, vflength, vfinput);	/* insert resource data */

		fclose(input);
/*		if (verboseflag != 0) putc('\n', stdout); */
	}

	(void) tfmtometend(output, 0);		/* first pass */

	if (ftell(output) > 0) {
		fprintf(stderr, "WARNING: output produced in first pass\n");
		rewind(output);
	}
		
	resourcepointer = 0;
	nextresource = 0;
	resourcedata[nextresource++] = resourcepointer;
	nameoffset[0] = 0;

	(void) tfmtometbegin(output, 1);		/* second pass */

	for (m = firstarg; m < argc; m++) {  	/* second pass */
		tfmindex = m - firstarg;
		strcpy(tfmfile, argv[m]);
		extension(tfmfile, "tfm");
		if((input = fopen(tfmfile, "rb")) == NULL) {
			if (tryunderscore != 0) underscore(tfmfile);
			if((input = fopen(tfmfile, "rb")) == NULL) {			
				strcpy(tfmfile, argv[m]);
				extension(tfmfile, "tfm");
				perror(tfmfile); exit(3);
			}
		}

		vfexists = 0;
		vflength = 0;
		strcpy (vffile, tfmfile);
		forceexten(vffile, "vf");
		if ((vfinput = fopen(vffile, "rb")) == NULL) {
			forceexten(vffile, "vfx");
			vfinput = fopen(vffile, "rb");
		}
		if (vfinput != NULL) {
			vfexists = 1;
			fseek(vfinput, 0L, SEEK_END);
			vflength = ftell(vfinput);
			fseek(vfinput, 0L, SEEK_SET);
/*			if (verboseflag != 0)
				printf("VF file %s found (%d bytes)\n", vffile, vflength);*/
/*			fclose(vfinput);  */
/*			vfinput=NULL; */
		}
	
/*		printf("Processing %s\n", tfmfile); */
		
		dataforklength = 0;						/* assumes zero throughout */
		
/*		(void) tfmtomet(output, input, 1); */		/* second pass */

		copytfmfile(output, input, 1, vflength, vfinput);	/* insert resource data */
		
		fclose(input);
		if (vfinput != NULL) {
			fclose(vfinput); 
			vfinput=NULL;
		}
/*		if (verboseflag != 0) putc('\n', stdout); */
	}		/* end of for (m = ... loop */

	(void) tfmtometend(output, 1);		/* second pass */

	if (ferror(output) != 0) {
		perror(macfile); exit(11);
	}
	fclose(output);

	if (argc - firstarg > 1) 
		printf("Processed %d TFM files\n", argc - firstarg);
	return 0;
}
	
/* MacBinary header: */
	
/*	0	 1 BYTE		Zero	*/
/*	1	64 BYTES	File Name -  Pascal string format */
/*	65	 4 BYTES 	File Type	 (no length) */
/*	69	 4 BYTES	File Creator (no length) */
/*	73	 1 BYTE		File Flags */		/* 32 + 1 for font ? */
/*	74	 1 BYTE		Zero */
/*	75	 6 BYTES	DeskTop Location */
/*  81	 1 BYTE		Protected Flag */
/*	82	 1 BYTE		Zero */
/*	83	 LONG		Data Fork Length	*/
/*	87	 LONG		Resource Fork Length	*/
/*	91	 LONG		Creation Date	*/
/*	95	 LONG		Modification Date	*/
	
/* Desktop holds a list that links each file to its creator */
/* Hence desktop icon is associated with the creator code */

/* See `Inside MacIntosh' Vol VI, page 9-6 for more information */

/* Hold down Option and Command keys as you start to rebuild desktop */

/* Default creator code is first four characters of icon file name, or ASPF */

/* TeXTures doesn't care about case of TeX font name */

/* added code for hidden VF file 1993/June/29 */

/* TFM file: 6 words (12 length codes), 2 words checksum+designsize */
/* 10 words (encodingscheme) PS name, 5 words (Font ID) QD name */
/* 1 word `face byte' */ /* then either TFM, or BSR extra: */
/* 1 word ID (222, 173, 250, 206), 1 word field count (7) */
/* 7 word field sizes  (for QD, PS, ENCOD, ?, ?, VF, ?) */
/* QD name, PS name, ENCOD, . . ., VF */  /* then rest of TFM */
