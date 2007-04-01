/* Copyright 1992 Y&Y, Inc.
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

/* Program for splitting TeXtures metric file into TFM files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* #define FNAMELEN 80 */
#define MACBINARYHEAD 128

int verboseflag = 0;
int detailflag = 0;
int traceflag = 0;
int textures = 1;
int nomacbinary = 0;			/* non-zero if no MacBinary header */
int showresourcedataflag = 0;
int showresourcemapflag = 1;
int showeachresource = 1;
int splitoffextra = 1;	/* split off extra header from TFM if needed */
int includeextra = 0;	/* include extra header in TFM even if splitoffextra != 0 */
int showfields = 1;		/* decode BSR cruft in header */
int avoidoverwrite = 0;	/* avoid overwriting TFM file with same name */

int nvirtual=0;

FILE *errout=stdout;	/* or stderr */

char *copyright = "\
Copyright (C) 1992-1998  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1992, 1993  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 13986445 */
/* #define COPYHASH 11737093 */
#define COPYHASH 3956063

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

/* resource numbering 0 -- 127 systems resources */
/* resource numbering 128 to 32767 applications */

/* Resource Attributes: */

#define RESSYSHEAP 64		/* 1 => read into system heap, 0 => application */
#define RESPURGEABLE 32		/* 1 => purgeable, 0 => not purgeable */
#define RESLOCKED 16		/* 1 => locked, 0 => unlocked */
#define RESPROTECTED 8		/* 1 => protected, 0 => unprotected */
#define RESPRELOAD 4		/* 1 => preload, 0 => don't preload */
#define RESCHANGED 2		/* 1 => write to resource file, 0 => don't write */

#ifdef _WIN32
#define MAXFONTS 768
#define MAXFONTNAME 48
#else
#define MAXFONTS 512
#define MAXFONTNAME 32		/* actual limit on the Mac ? */
#endif

char resourcenames[MAXFONTS][MAXFONTNAME];

int resids[MAXFONTS], resattrib[MAXFONTS];

unsigned long resdataoffsets[MAXFONTS], resdatalengths[MAXFONTS];

int resindex = 0;

char filename[64+1];

char filetype[4+1], filecreator[4+1];

unsigned char buffer[MACBINARYHEAD];

unsigned int uscantwo (int n) {
	return (buffer[n] << 8) | buffer[n+1];
} 

unsigned int ureadtwo (FILE *input) {
	return ((getc(input) << 8) | getc(input));
} 

unsigned long uscanfour (int n) {
	unsigned int c, d, e, f;
	c = buffer[n];		d = buffer[n+1];
	e = buffer[n+2];	f = buffer[n+3];
	return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
} 

unsigned long ureadthree (FILE *input) {
	unsigned int c, d, e;
	c = getc(input);	d = getc(input);
	e = getc(input);	
	return ((((unsigned long) c << 8) | d) << 8) | e;
} 

unsigned long ureadfour (FILE *input) {
	unsigned int c, d, e, f;
	c = getc(input);	d = getc(input);
	e = getc(input);	f = getc(input);
	return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
} 

unsigned long extractdays(unsigned long date) {
	return (date / 86400);
}

unsigned long extractseconds(unsigned long date) {
	return (date  - extractdays(date) * 86400);
}

void showseconds(long seconds) {
	int hh, mm, ss;
	hh = (int) (seconds / 3600);
	seconds = seconds - (long) hh * 3600;
	mm = (int) (seconds / 60);
	seconds = seconds - (long) mm * 60;
	ss = (int) seconds;
	hh = hh + 1;			/* weirdness */
	if (hh == 24) hh = 0;	/* weirdness */
	printf("%02d:%02d:%02d ", hh, mm, ss);
}

char *weekdays[7] = {"Fri", "Sat", "Sun", "Mon", "Tue", "Wed", "Thu"};

char *months[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int monthstart[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
int leapstart[12] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};

void showdays(long days) {
	long weeks;
	int weekday;
	int cycles, remain, dyear, year, dayof, month, day;

	weeks = days / 7;
	weekday = (int) (days - weeks * 7);
	printf("%s ", weekdays[weekday]);
	cycles = (int) (days / 1461);			/* groups of four years */
	remain = (int) (days - (long) cycles * 1461);
/*	dyear = remain / 365; */
/*	dayof = remain - dyear * 365; */
	if (remain >= 1096) {
		dyear = 3; dayof = remain - 1096;
	}
	else if (remain >= 731) {
		dyear = 2; dayof = remain - 731;
	}
	else if (remain >= 366) {
		dyear = 1; dayof = remain - 366;
	}
	else {
		dyear = 0; dayof = remain;
	}	
	year = 1904 + 4 * cycles + dyear;
	if (dyear != 0) {
		for (month = 0; month < 12; month++) {
			if (monthstart[month] > dayof) break;
		}
		month = month - 1;
		day = dayof - monthstart[month] + 1;
	}
	else {
		for (month = 0; month < 12; month++) {
			if (leapstart[month] > dayof) break;
		}
		month = month - 1;
		day = dayof - leapstart[month] + 1;
	}
	printf("%02d %s %02d ", year, months[month], day);

/*	printf("%d ", dayof); */
/*	printf("%ld ", days); */
}

unsigned long datalength=0, resourcelength=0;
unsigned long creationdate, modificationdate;
unsigned long resourcedataoffset=0, resourcemapoffset=0;
unsigned long resourcedatalength=0, resourcemaplength=0;

void skipbytes(FILE *input, unsigned int n) {
	unsigned int k;
	for (k = 0; k < n; k++) (void) getc(input);
}

/* show resources - assumes now at end of MacBinary header */

long resourceforkstart;
long resourceforkpos;
long beginresource;

void setupresourcefork(FILE *input) {
	resourceforkstart = ((datalength + 511) / 512) * 512;
/*	skip over data fork */
	if (traceflag != 0) 
		printf("Resource Fork starts at %lu\n", resourceforkstart);
	if (nomacbinary != 0) resourceforkpos = resourceforkstart;
	else resourceforkpos = resourceforkstart + MACBINARYHEAD;
	if (fseek(input, resourceforkpos, SEEK_SET) != 0) {
		fprintf(errout, "SEEK to %ld FAILED\n", resourceforkpos);
		return;
	}
	beginresource = ftell(input);
/*	read resource fork header */
	resourcedataoffset = ureadfour(input);
	resourcemapoffset  = ureadfour(input);
	resourcedatalength = ureadfour(input);
	resourcemaplength  = ureadfour(input);
	if (traceflag != 0) 
	printf("ResDataOff %lu ResMapOff %lu ResDatLen %lu ResMapLen %lu\n",
		resourcedataoffset,	resourcemapoffset, 
			resourcedatalength, resourcemaplength);
}

void showresourcedata(FILE *input) {
	unsigned long reslength, resposition, totallength;
	int k;

	resposition = resourcedataoffset;
	if (fseek(input, resposition + beginresource, SEEK_SET) != 0) {
		fprintf(errout, "SEEK to %ld FAILED\n", resposition + beginresource);
		return;
	}
	totallength = 0L;
	for(k = 1; k > 0; k++) {
		reslength = ureadfour(input);
		if (traceflag != 0) 
		printf("K %d position %ld length %ld\n", k, resposition, reslength);
		resposition += (reslength + 4);
		totallength += (reslength + 4);
		if (totallength >= resourcedatalength) break;
		if (fseek(input, resposition + beginresource, 
					SEEK_SET) != 0) {
			fprintf(errout, "SEEK to %ld FAILED\n", resposition + beginresource);
			return;
		}
	}
	
}

void getfourbytestring(FILE *input,  char *res) {
	char *s=res;
	*s++ = (char) getc(input); *s++ = (char) getc(input);
	*s++ = (char) getc(input); *s++ = (char) getc(input);
	*s++ = '\0';
}

void copyresname(FILE *input, char *buff) {
	int k, n;
	char *s=buff;
	n = getc(input);
	for (k = 0; k < n; k++) *s++ = (char) getc(input);
	*s++ = '\0';
}

#define MAXSTRING 256

void showresourcemap(FILE *input) {
	unsigned long resposition, resdatalength;
	unsigned int resourceattributes, typelistoffset, namelistoffset;
	unsigned int numberoftypes, numberofthistype, referenceoffset;
	unsigned int resourceid, nameoffset, resattributes;
	unsigned long resdataoffset;
	char resourcetype[4+1];
	char str[MAXSTRING];
	long beginresmap, begintype, present, current;
	unsigned int k, l;
	int wewantthese=0;

	resposition = resourcemapoffset;
	if (fseek(input, resposition + beginresource, SEEK_SET) != 0) {
		fprintf(errout, "SEEK to %ld FAILED\n", resposition + beginresource);
		return;
	}
	beginresmap = ftell(input);
	skipbytes(input,  16 + 4 + 2);
	resourceattributes = ureadtwo(input);
	if (traceflag != 0) 
	printf("Resource Attributes %d\n", resourceattributes);
	typelistoffset = ureadtwo(input);
	namelistoffset = ureadtwo(input);
	if (traceflag != 0) 
	printf("TypeListOffset %u NameListOffset %u\n",
		typelistoffset,	namelistoffset);
/*	go to Type List */
/*	if (fseek(input, typelistoffset + resposition + MACBINARYHEAD,  */
	if (fseek(input, typelistoffset + beginresmap, SEEK_SET) != 0) {
		fprintf(errout, "SEEK to %ld FAILED\n", typelistoffset + beginresmap);
		return;
	}
	begintype = ftell(input);
	numberoftypes = ureadtwo(input);
	if (traceflag != 0) 
	printf("NumOfTypes %u\n", numberoftypes+1);
	for (k = 0; k <= numberoftypes; k++) {		/* ??? */
		getfourbytestring(input, resourcetype);
		if (strcmp(resourcetype, "*TFM") != 0) {
			fprintf(errout,
				"ERROR: wrong Resource Type `%s' (expecting *TFM)\n",
					resourcetype);
			wewantthese = 0;
		}
		else {
			wewantthese = 1;
			resindex = 0;
		}
		numberofthistype = ureadtwo(input);
		if (wewantthese != 0 && numberofthistype > MAXFONTS)
			fprintf(errout, "ERROR: too many TFM fonts\n");
		referenceoffset = ureadtwo(input);
		if (traceflag != 0) 
		printf("ResType %s NumofThisType %u RefOffset %u\n", 
			resourcetype, numberofthistype+1, referenceoffset);
		if (showeachresource != 0) {
			present = ftell(input);
			fseek(input, begintype + referenceoffset, SEEK_SET);
			for (l = 0; l <= numberofthistype; l++) {	/* ??? */
				resourceid = ureadtwo(input);
				nameoffset = ureadtwo(input);
				resattributes = getc(input);
				if (resattributes == EOF) {
					fprintf(errout, "Unexpected EOF\n");
					return;
				}
				resdataoffset = ureadthree(input);
				(void) ureadfour(input);
				current = ftell(input);
				fseek (input, 
					resdataoffset + resourcedataoffset + beginresource, 
						SEEK_SET);
				resdatalength = ureadfour(input);
				if (nameoffset == 65535) {
/*					if (traceflag != 0) */
					printf(
		"\tResID %u ResAttrib %u ResDataOff %lu ResDataLen %lu\n",
					resourceid, resattributes, 
						resdataoffset, resdatalength);
				}
				else {
					fseek(input,
						nameoffset + namelistoffset + beginresmap,
							SEEK_SET);
					copyresname(input, str);
/* debugging hack .... */
					if (strcmp(str, "") == 0) sprintf(str, "%u", nameoffset);
					if (traceflag != 0)
					printf(
		"\tResID %u ResName %s ResAttrib %u ResDataOff %lu ResDataLen %lu\n",
					resourceid, str, resattributes, 
						resdataoffset, resdatalength);
/* save information for later retrieval */
					if (wewantthese != 0) {
						if (strlen(str) >= MAXFONTNAME)
							fprintf(errout, "Resource name %s too long\n", str);
						else strcpy (resourcenames[resindex], str);
						resids[resindex] = resourceid;
						resattrib[resindex] = resattributes;
						resdataoffsets[resindex] = resdataoffset;
						resdatalengths[resindex] = resdatalength;
						if (resindex++ >= MAXFONTS) {
							fprintf(errout, "ERROR: Too many resources\n");
							resindex--;
/*							exit(17); */
						}
					}
				}
				fseek(input, current, SEEK_SET);
			}
			fseek(input, present, SEEK_SET);
		}
	}
}

void showdate(unsigned long date) {
	showdays(extractdays(date));
	showseconds(extractseconds(date));	
	putc('\n', stdout);
}

void macanal(FILE *input) {
	long filelength;
	int allocblocks;
	unsigned int k, nfilename;
	unsigned char *s;

	fseek(input, 0, SEEK_END);
	filelength = ftell(input);
	fseek(input, 0, SEEK_SET);			/* rewind */
	allocblocks = (int) (filelength / 512);
	if (traceflag != 0) 
	printf("File length %ld = %d * 512 + %d\n",
		filelength, allocblocks, (int) (filelength - allocblocks * 512));

	if (nomacbinary == 0) {
/*	first try and read MacBinary file header of 128 bytes */	
		s = buffer;
		for (k = 0; k < MACBINARYHEAD; k++) *s++ = (char) getc(input);

		if (buffer[0] == 0 && buffer[1] == 0) {
			fprintf(errout, "There appears to be no MacBinary header\n");
			fprintf(errout, "Assuming this is the resource fork only\n");
			nomacbinary++;
			fseek(input, 0, SEEK_SET);			/* rewind */
		}
	}
	if (nomacbinary == 0) {
		nfilename = uscantwo(0);
/*		strncpy(filename, buffer + 2, nfilename); */
		strncpy(filename, (char *) (buffer + 2), nfilename);
		filetype[nfilename] = '\0';
		if (traceflag != 0) 
			printf("FileName: %s\n", filename);
/*		printf("FileName: ");
		for (k = 0; k < nfilename; k++) putc(buffer[k+2], stdout);
		putc('\n', stdout); */
	
/*		strncpy(filetype, buffer + 65, 4); */
		strncpy(filetype, (char *) (buffer + 65), 4);
		filetype[4] = '\0';
		if (traceflag != 0) printf("File Type:    %s\n", filetype);
		if (strcmp(filetype, "FFIL") != 0) {
			fprintf(errout, "WARNING: wrong file type `%s' (expecting FFIL)\n",
				filetype);
			textures = 0;
		}
/*		printf("File Type:    ");
		for (k = 0; k < 4; k++) putc(buffer[k+65], stdout);
		putc('\n', stdout); */
	
/*		strncpy(filecreator, buffer + 69, 4); */
		strncpy(filecreator, (char *) (buffer + 69), 4);
		filecreator[4] = '\0';
		if (traceflag != 0) printf("File Creator: %s\n", filecreator);
		if (strcmp(filecreator, "*TEX") != 0) {
			fprintf(errout, "WARNING: wrong file creator `%s' (expecting *TEX)\n",
				filecreator);	
			textures = 0;
		}
/*		printf("File Creator: "); 
		for (k = 0; k < 4; k++) putc(buffer[k+69], stdout);
		putc('\n', stdout); */
	
		datalength = uscanfour(83);
		if (traceflag != 0) 
		printf("Data Fork Size:     %lu\n", datalength);	
	
		resourcelength = uscanfour(87);
		if (traceflag != 0) 
		printf("Resource Fork Size: %lu\n", resourcelength);	

		creationdate = uscanfour(91);
		if (traceflag != 0) {
			printf("Creation Date:     ");
			showdate(creationdate);
		}
	
		modificationdate = uscanfour(95);
		if (traceflag != 0) {
			printf("Modification Date: ");
			showdate(modificationdate);
		}
	
		if (traceflag != 0)  {
		if (buffer[73] != 0) printf("File Flags:     %d\n", buffer[73]);
		if (buffer[81] != 0) printf("Protected Flag: %d\n", buffer[81]);
		}

	for (k = 75; k < 81; k++) {
		if (buffer[k] != 0) {
/*			printf("Desktop Location %d %d %d %d %d %d\n",
				buffer[75], buffer[76], buffer[77], 
					buffer[78], buffer[79], buffer[80]); */
		if (traceflag != 0) 
			printf("Desktop Location v: %d h: %d d: %d\n",
				uscantwo(75), uscantwo(77), uscantwo(79));
			break;
		}
	}
	}

	if (resourcelength > 0 || nomacbinary != 0) {
		if (showresourcedataflag != 0 || showresourcemapflag != 0)
			setupresourcefork(input);
		if (showresourcedataflag != 0) showresourcedata(input);
		if (showresourcemapflag != 0) showresourcemap(input);
	}
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

int	lf, lh, bc, ec, nw, nh, nd,	ni, nl, nk, ne, np;

/*	lf	 length of file in words */ 
/*	lh	 length of header data in words */
/*	bc	 smallest character code */
/*	ec	 largest character code */
/*	nw	 number of words in the width table */
/*	nh	 number of words in the height table */
/*	nd	 number of words in the depth table */
/*	ni	 number of words in the italic correction table */
/*	nl	 number of words in the lig/kern table */ 
/*	nk	 number of words in the kern table */ 
/*	ne	 number of words in the extensible char table */ 
/*	np	 number of font parameter words */

int readint (FILE *input) {	 /* read 16-bit count  */
	int a, b;
	a = getc(input);
	b = getc(input);
	return (a << 8) | b;
}

void writeint (FILE *output, int n) {	 /* write 16-bit count  */
	int a, b;
	a = (n >> 8) & 255;
	b = n & 255;
	putc (a, output); 	putc (b, output);
}

void copyfile (FILE *output, FILE *input, int length) {
	int m;
	for (m = 0; m < length; m++) putc(getc(input), output);
}

void skipfile (FILE *output, FILE *input, int length) {
	int m;
	for (m = 0; m < length; m++) getc(input);
}

int readhead (FILE *input) {
	lf = readint(input);
	lh = readint(input);
	bc = readint(input);
	ec = readint(input);
 	nw = readint(input);
	nh = readint(input);
	nd = readint(input);
	ni = readint(input);
 	nl = readint(input);
	nk = readint(input);
	ne = readint(input);
	np = readint(input); 
	if (lf != 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np
		|| ne > 256) {
		fprintf(errout, 
			"Inconsistent start of TFM file (lengths don't add up)\n");
		exit(19);
	}
	return (lf << 2);		/* length of TFM file */
}

void writehead (FILE *output, int lfx, int lhx) {
	writeint(output, lfx);
	writeint(output, lhx);
	writeint(output, bc);
	writeint(output, ec);
	writeint(output, nw);
	writeint(output, nh);
	writeint(output, nd);
	writeint(output, ni);
	writeint(output, nl);
	writeint(output, nk);
	writeint(output, ne);
	writeint(output, np); 
}

/* Returns length of extra header (possible VF or ENCOD) portion ---  if any */

int copytfm (FILE *output, FILE *input, int length) {	/* 1993/June/19 */
	int lhx, lengthx;
	if (splitoffextra == 0) {				/* just write out the whole TFM */
		copyfile (output, input, length);
		return 0;
	}
	lengthx = readhead(input);	/* lengthx stated in TFM ? */
	if (lh <= 16+2) {			/* easy case, no extra stuff in header */
		writehead(output, lf, lh);
		copyfile (output, input, length - 24);
		return 0;
	}
/*	get here if extra stuff in header (after XEROX header) & splitoffextra */ 
/*	always copy checksum, design size, and xerox header */
	lhx = lh;
/*	but only copy extra header part into output TFM if includeextra != 0 */
	if (lhx > 16+2 && includeextra == 0) lhx = 16+2;
	writehead(output, lf - (lh - lhx), lhx);
	copyfile (output, input, lhx << 2);
/*	then skip over extra stuff in header */
	if (lh > lhx) skipfile (output, input, (lh - lhx) << 2);
/*	then copy the rest of the TFM part of the file */
	copyfile (output, input, length - 24 - (lh << 2));
	return (lh  - (16+2)) << 2;		/* number of extra bytes in header */
}

/* Look at extra information in header of TFM file */
/* Possibly ENCOD resource name or packaged VF */

void copyextra(FILE *input, char *tfmname, int extralength) {
	FILE *output=NULL;
	char vfname[FILENAME_MAX];
	int c, k, n, nfields, ncruft=52;
/*	unsigned long bsrid; */
	unsigned int ida, idb;
	unsigned long i, m;
	unsigned long fields[16];
	int flag;

	if (showfields != 0) {
/*		skip over the usual twelve TFM length fields */
		skipfile(output, input, 24);
/*		skip over the XEROX header */
		skipfile(output, input, (16 + 2) << 2);
/*		for (k = 0; k < 4; k++) {
			c = getc(input); printf("%d ", c);
		} */
/*		bsrid = ureadfour(input); */
		ida = ureadtwo(input);			/* ??? */
		idb = ureadtwo(input);			/* ??? */
/*		if (verboseflag != 0) printf("Header ID is %lu ", bsrid); */
		if (verboseflag != 0) printf("Extra Header info (VF/nCOD): ");
		if (traceflag != 0 || ida != 57005 || idb != 64206) {
			printf("ID %u %u ", ida, idb);
		}
/*		read how many entries in table */
		nfields = (int) ureadfour(input);

		if (nfields > 16) n = 16;
		else n = nfields;

		for (k = 0; k < n; k++) fields[k] = ureadfour(input);

		flag = 0;
		if (traceflag != 0) flag++;
		if (nfields != 7) flag++;
/*		first three are expected to be non-zero */
/*		rest indicate something unusual - meaning 3 and 6 */
		for (k = 0; k < nfields; k++) {
			if (k < 3) continue;	/* QD name, PS name, nCOD */
			if (k == 4) continue;	/* PS font Modifier */
			if (k == 5) continue;	/* VF */
			if (fields[k] != 0) flag++;
		}
/*		for (k = 3; k < 4; k++) if (fields[k] != 0) flag++; */
/*		for (k = 6; k < 7; k++) if (fields[k] != 0) flag++; */

		if (flag != 0) {
			printf(" followed by %d fields: ", nfields);
			for (k = 0; k < n; k++) printf("%d ", fields[k]);
		}

		for (k = n; k < nfields; k++) (void) ureadfour(input);
		if (verboseflag != 0) putc('\n', stdout);
		for (k = 0; k < n; k++) {
			if ((m = fields[k]) == 0) continue;
			if (k == 0) {			/* QD Font Name */
				printf("QD: ");
				(void) getc(input);	/* Pascal string length */
				for (i = 0; i < m-1; i++) putc(getc(input), stdout);
				putc(' ', stdout);
			}
			else if (k == 1) {		/* PS Font Name */
				printf("PS: ");
				(void) getc(input);	/* Pascal string length */
				for (i = 0; i < m-1; i++) putc(getc(input), stdout);
				putc(' ', stdout);
			}
			else if (k == 2) {		/* nCOD Name */
				printf("nCOD: ");
				for (i = 0; i < m; i++) putc(getc(input), stdout);
				putc(' ', stdout);
			}
			else if (k == 4) {		/* QuickDraw modifier */
				printf("QD MOD: ");
				for (i = 0; i < m; i++) putc(getc(input), stdout);
				putc(' ', stdout);
			}
			else if (k == 5) {		/* VF - virtual font */
				strcpy(vfname, tfmname);
				forceexten(vfname, "vf");
				if ((output = fopen(vfname, "wb")) == NULL) {
					perror(vfname); exit(13);
				}
/*				if (verboseflag != 0) */
					printf("Creating VF file %s (%d bytes) ",
						vfname, extralength - ncruft);
				copyfile (output, input, (int) m);
				fclose(output);
				nvirtual++;						/* count `em */
			}
			else {		
				printf("Unknown field type %d ", k);
				for (i = 0; i < m; i++) {
					c = getc(input);
					if (c >= 32 && c < 127) putc(c, stdout);
				}			
			}
/*	always round up to nearest 32 bit word boundary */
			for (i = m; i < ((m + 3) / 4) * 4; i++) getc(input);
/*			putc('\n', stdout); */
		}
		putc('\n', stdout); 
		return;
	}
	if (extralength < 100) ncruft = 36;
	else ncruft = 52;
	if (extralength < 100) {
		printf("PS encoding requested for output to printer:\n");
/*	skip over 24 bytes of length codes in TFM up to header part of TFM */
		skipfile(output, input, 24);
/*	skip over checksum & design size & xerox header */
		skipfile(output, input, (16 + 2) << 2);
/*	skip over BSR mysterious stuff 4 + 48 bytes total ??? WHAT IS IT */
/*	4 bytes of `ID', 1 word of number of table entries */
/*  1 word per table entry lengths (in bytes) of table entries */
/*  actual data -- padded on right to bring to multiple of four */
		skipfile(output, input, ncruft);
/*	then copy VF part of file */
		for (k = 0; k < extralength - ncruft; k++) {
			c = getc(input);
			putc(c, stdout);
		}
		putc('\n', stdout);
			 return;
	}
	strcpy(vfname, tfmname);
	forceexten(vfname, "vf");
	if ((output = fopen(vfname, "wb")) == NULL) {
		perror(vfname); exit(13);
	}
	if (verboseflag != 0) printf("Creating VF file: %s (%d bytes)\n",
		vfname, extralength - ncruft);
	nvirtual++;						/* count `em */
/*	skip over 24 bytes of length codes in TFM up to header part of TFM */
	skipfile(output, input, 24);
/*	skip over checksum & design size & xerox header */
	skipfile(output, input, (16 + 2) << 2);
/*	skip over BSR mysterious stuff 4 + 48 bytes total ??? WHAT IS IT */
	skipfile (output, input, ncruft);
/*	then copy VF part of file */
	copyfile (output, input, extralength - ncruft);
	fclose(output);
}

unsigned long gototfm(FILE *input, int k) {	/* go to k-th TFM start in input file */
	long length;
/*	go to beginning of resource */
	fseek (input, resourceforkpos + 256 + resdataoffsets[k], SEEK_SET);
/*	skip over resource length word */
/*	getc(input); getc(input); getc(input); getc(input); */
	length = ureadfour(input);
	return length;
}

void splitanal(FILE *input) {
	int k, l, m, c;
	char tfmname[FILENAME_MAX];
	int npsname, nqdname;
	FILE *output;
	FILE *dummy;
	int length;
	int pscode, qdcode, reserve, style;
	int extralength;
	char *s;

	if (resindex == 0) return;
	if (verboseflag)
		printf("TFM Name\tPS Name\t\tQD Name\tEncoding       Style\n\n");
	for (k = 0; k < resindex; k++) {
#ifdef _WIN32
		strcpy(tfmname, resourcenames[k]);
#else
		if (strlen (resourcenames[k]) < 9) strcpy(tfmname, resourcenames[k]);
		else {
			printf("Truncating TeX font name `%s' to ", resourcenames[k]);
			strncpy(tfmname, resourcenames[k], 8);
			tfmname[8] = '\0';
			printf("`%s' for DOS file name\n", tfmname);
		}
#endif
/*		printf("%s ", tfmname); */
/*		for (m = strlen(tfmname); m < 8; m++) putc(' ', stdout); */
/*		printf("%s ", resourcenames[k]); */	/* 1993/July/19 */
/*		for (m = strlen(resourcenames[k]); m < 8; m++) putc(' ', stdout);*/
		strcat(tfmname, ".tfm");
		if ((dummy = fopen(tfmname, "rb")) != NULL) {	/* 1993/July/19 */
/*			fprintf(errout, "WARNING: attempt to overwrite `%s' ", tfmname); */
			fclose(dummy);
/*		limited attempt to come up with unique file name */
			if (avoidoverwrite != 0) {
			fprintf(errout, "WARNING: attempt to overwrite `%s' ", tfmname);
			for (l = 0; l < 99; l++) {
				if ((s = strchr(tfmname, '.')) != NULL) {
					s--;	c = *s;
					if (c >= '0' && c < '9') *s = (char) (c+1);
					else *s = '0';
					if (c == 9) {
						s--;	c = *s;
						if (c >= '0' && c < '9') *s = (char) (c+1);
						else *s = '0';
					}
					if ((dummy = fopen(tfmname, "rb")) != NULL) 
						fclose(dummy);
					else break;		/* found a name we can use */
				}
				else break;	/* should not happen */
			}
			fprintf(errout, " - using `%s' instead\n", tfmname);
			}
/*			else putc('\n', errout); */
		}
		if ((output = fopen(tfmname, "wb")) == NULL) {
			perror(tfmname); exit(13);
		}
/*		printf("%s ", resourcenames[k]); */	/* 1993/July/19 */
		printf("%s", resourcenames[k]);	
		for (m = strlen(resourcenames[k]); m < 8; m++) putc(' ', stdout);
		putc ('\t', stdout);
		(void) gototfm(input, k);
		for (m = 0; m < 24 + 8; m++) getc(input);	/* skip over head */
/*		extract `coding scheme' - PS name field from XEROX header */
		npsname = getc(input);				/* length of PS name */
		if (traceflag != 0) printf("(%d) ", npsname);
		if (npsname > 40) npsname = 40;		/* sanity check */
		for (m = 0; m < npsname; m++) putc(getc(input), stdout);
		for (m = npsname + 1; m < 40; m++) getc(input);
/*		putc (' ', stdout); */
		putc ('\t', stdout);
/*		extract `Font ID' - QD name field from XEROX header */
		nqdname = getc(input);				/* length of QD name */
		if (traceflag != 0) printf("(%d) ", nqdname);
		if (nqdname > 20) nqdname = 20;		/* sanity check */
		for (m = 0; m < nqdname; m++) putc(getc(input), stdout);
		for (m = nqdname + 1; m < 20; m++) getc(input);
/*		putc (' ', stdout); */
		putc ('\t', stdout);
/*		extract PS code, QS code, Style --- `face byte' from XEROX header */
		pscode = getc(input);
		qdcode = getc(input);
		reserve = getc(input);
		style = getc(input);
/*		printf("PS %d QD %d RS %d ST %d\n", 
			pscode, qdcode, reserve, style); */
		printf("PS %d QD %d RS %d %s%s%s\n",
			pscode, qdcode, reserve,
			   (style == 0) ? "Regular" : "",
			   (style & 1) ? "Bold" : "",
			   (style & 2) ? "Italic" : "");
		(void) gototfm(input, k);
		length = (int) resdatalengths[k];
/*		for (m = 0; m < length; m++) putc(getc(input), output); */
		extralength = copytfm (output, input, length);		/* 1993/June/19 */
		if (extralength > 0) {
			(void) gototfm(input, k);
			copyextra(input, tfmname, extralength);
		}
		if (ferror(output) != 0) {
			perror(tfmname); exit(17);
		}
		else fclose(output);
	}
	printf("Processed %d TFM files ", resindex);
	if (nvirtual > 0) printf(" - Created %d VF files", nvirtual);
	putc('\n', stdout);
}


void showusage(char *s) {
	printf("%s [-v] [-r] [-s] [-o] <textures-metrics>\n", s);
	printf("\tv:  verbose\n");
	printf("\tr:  resource fork only (i.e. no 128 byte MacBinary header)\n");
	printf("\ts:  don't split off extra header (VF & ENCOD) from TFM resources\n");
	printf("\to:  avoid overwriting existing TFM files\n");
	printf("\n");
	printf("\t    METtoTFM splits TeXtures metrics file into TFM files\n");
	exit(1);
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
	fprintf(errout, "HASHED %ld\n", hash);	/* (void) getch();  */
/*	fflush(errout); */
	return hash;
}

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0; 
		case 'v': verboseflag = 1; return 0;
		case 't': traceflag = 1; return 0; 
		case 'r': nomacbinary = 1; return 0;
		case 's': splitoffextra = 0; return 0;
		case 'd': showfields = 0; return 0;
		case 'i': includeextra = 0; return 0;
		case 'o': avoidoverwrite = (1 - avoidoverwrite); return 0;
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

/*	while (argv[firstarg][0] == '-') */ /* check for command flags */
	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
/*				if (vectorflag != 0) {
					strncpy(codingvector, s, 40); 
					vectorflag = 0; reencodeflag = 1; 
				} */
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

int main(int argc, char *argv[]) {
	char macfile[FILENAME_MAX];
	FILE *input;
	int firstarg = 1;

	firstarg = commandline(argc, argv, 1);

	if (argc < firstarg + 1) showusage(argv[0]);

#ifdef _WIN32
	printf("METtoTFM (32) Copyright (C) 1992-1998 Y&Y, Inc. http://www.YandY.com\n");
#else
	printf("METtoTFM Copyright (C) 1992-1998 Y&Y, Inc. http://www.YandY.com\n");
#endif

	if (checkcopyright(copyright) != 0) {
/*		fprintf(stderr, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	nvirtual = 0;

	strcpy(macfile, argv[firstarg]);
	if((input = fopen(macfile, "rb")) == NULL) {
		extension(macfile, "met");
		if((input = fopen(macfile, "rb")) == NULL) {
			perror(macfile); exit(3);
		}
	}

	macanal(input);
	splitanal(input);
	fclose(input);
	
	return 0;
}

/* MacBinary header format */

/*	0	 1 BYTE		Zero	*/
/*	1	64 BYTES	File Name -  Pascal string format */
/*	65	 4 BYTES 	File Type	 (no length) */
/*	69	 4 BYTES	File Creator (no length) */
/*	73	 1 BYTE		File Flags */
/*	74	 1 BYTE		Zero */
/*	75	 6 BYTES	DeskTop Location */
/*  81	 1 BYTE		Protected Flag */
/*	82	 1 BYTE		Zero */
/*	83	 LONG		Data Fork Length	*/
/*	87	 LONG		Resource Fork Length	*/
/*	91	 LONG		Creation Date	*/
/*	95	 LONG		Modification Date	*/


/* ICN# */
/* FREF */
/* BNDL */
/* vers */
/* TEXT */
/* CODE */
/* FONT */
/* NFNT */
/* FOND */
/* PICT */

/* Textures doesn't care about case of TeX font name */

/* Split out virtual fonts from TFMs where lh > 16 ? */

/* What is BSR mystery stuff 4 + 48 bytes total at start of VF ? */

/* QD code 0 => None */
/* QD code 1 => Roman => Macintosh */
/* QD code 2 => Typewriter => Macintosh */
/* QD code 3 => Roman => Mac8bit */
/* QD code 4 => Roman => Lucida */

/* PS code 0 => None */
/* PS code 1 => Roman => AdobeStandard */
/* PS code 2 => Typewriter => AdobeStandard  */
/* PS code 3 => Roman => MacintoshStandard */
/* PS code 4 => Roman => Lucida */

/* Style = italic * 2 + bold */
