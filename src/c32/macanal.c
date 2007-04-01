/* Copyright 1991, 1992 Y&Y, Inc.
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

/* Program for decoding Mac file headers */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* #include <conio.h> */
/* #include <assert.h> */
/* #include <errno.h> */

#define MACBINARYHEAD 128

int verboseflag = 1;		/* not used yet */
int traceflag = 0;			/* not used yet */
int detailflag = 0;
int tryzerofirst = 1;		/* Try zero offset first */
int showcolor = 1;			/* File Flags => labels */

int macbinflag=0;			/* Normal MacBinary file */
int binhexflag=0;			/* BinHeX derived file */

int resetnomac = 0;			/* command line non-zero if no MacBinary header */
int nomacbinary = 0;		/* per file non-zero if no MacBinary header */
long offset = 0;			/* offset into file */

int offsetflag=0;

int showresourcedataflag = 0;
int showresourcemapflag = 1;
int showeachresource = 1;

long filelength;

/* resource numbering 0 -- 127 systems resources */
/* resource numbering 128 to 32767 applications */

/* Resource Attributes: */

#define RESSYSHEAP 64		/* 1 => read into system heap, 0 => application */
#define RESPURGEABLE 32		/* 1 => purgeable, 0 => not purgeable */
#define RESLOCKED 16		/* 1 => locked, 0 => unlocked */
#define RESPROTECTED 8		/* 1 => protected, 0 => unprotected */
#define RESPRELOAD 4		/* 1 => preload, 0 => don't preload */
#define RESCHANGED 2		/* 1 => write to resource file, 0 => don't write */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright = "\
Copyright (C) 1990-1998  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

char *version="1.3.1";

FILE *errout=stdout;

#define COPYHASH 12415373

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

char *labels[] = {
/* "None", "Essential", "Hot", "In Progress", */
/* "Cool", "Personal", "Project 1", "Project 2" */
"None", "", "Project 2", "",
"Project 1", "", "Personal", "",
"Cool", "", "In Progress", "",
"Hot", "", "Essential", "", 
};

char *colors[] = {
/* "Gray", "Blue", "Red", "Magenta", */
/* "Green", "Yellow", "Light Green", "Lavender" */
"Gray", "", "Red", "",
"Light Green", "", "Yellow", "",
"Green", "", "Magenta", "",
"Red", "", "Blue", "", 
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* unsigned char buffer[MACBINARYHEAD]; */
unsigned char buffer[MACBINARYHEAD * 2];

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

long dataforkpos;

void setupresourcefork(FILE *input) {
	if (binhexflag == 0) {
		resourceforkstart = ((datalength + 511) / 512) * 512;
		resourceforkstart = datalength;			/* 98/Jun/28 */
/*		skip over data fork */
		if (offset == 0) printf("Resource Fork starts at %lu\n", 
								resourceforkstart);
		else printf("Resource Fork starts at %lu + %d\n", 
					resourceforkstart, offset);
/*		if (nomacbinary != 0) resourceforkpos = resourceforkstart; */
		if (nomacbinary != 0) resourceforkpos = resourceforkstart + offset;
		else resourceforkpos = resourceforkstart + MACBINARYHEAD;
	}
	if (resourceforkpos > filelength)
		printf("Inconsistent ResourceForkPos %lu > %lu\n",
			   resourceforkpos, filelength);
	printf("Seek to %lu\n", resourceforkpos);
	if (fseek(input, resourceforkpos, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return;
	}
	beginresource = ftell(input);
/*	read resource fork header */
	resourcedataoffset = ureadfour(input);
	resourcemapoffset  = ureadfour(input);
	resourcedatalength = ureadfour(input);
	resourcemaplength  = ureadfour(input);
	printf("ResDataOff %lu ResMapOff %lu ResDatLen %lu ResMapLen %lu\n",
		resourcedataoffset,	resourcemapoffset, 
			resourcedatalength, resourcemaplength);
}

void showresourcedata(FILE *input) {
	unsigned long reslength, resposition, totallength;
	int k;

	resposition = resourcedataoffset;
	if (fseek(input, resposition + beginresource, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return;
	}
	totallength = 0L;
	for(k = 1; k > 0; k++) {
		reslength = ureadfour(input);
		printf("K %d position %ld length %ld\n", k, resposition, reslength);
		resposition += (reslength + 4);
		totallength += (reslength + 4);
		if (totallength >= resourcedatalength) break;
		if (fseek(input, resposition + beginresource, 
					SEEK_SET) != 0) {
			fprintf(errout, "SEEK FAILED\n");
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
	char resourcetype[5];
	char str[MAXSTRING];
	long beginresmap, begintype, present, current;
	unsigned int k, l;

	resposition = resourcemapoffset;
	if (fseek(input, resposition + beginresource, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return;
	}
	beginresmap = ftell(input);
	skipbytes(input,  16 + 4 + 2);
	resourceattributes = ureadtwo(input);
	printf("Resource Attributes %d\n", resourceattributes);
	typelistoffset = ureadtwo(input);
	namelistoffset = ureadtwo(input);
	printf("TypeListOffset %u NameListOffset %u\n",
		typelistoffset,	namelistoffset);
/*	go to Type List */
/*	if (fseek(input, typelistoffset + resposition + MACBINARYHEAD,  */
	if (fseek(input, typelistoffset + beginresmap, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return;
	}
	begintype = ftell(input);
	numberoftypes = ureadtwo(input);
	printf("NumOfTypes %u\n", numberoftypes+1);
	if (numberoftypes > 1000) return;		/* 1993/May/1 */
	for (k = 0; k <= numberoftypes; k++) {		/* ??? */
		getfourbytestring(input, resourcetype);
		numberofthistype = ureadtwo(input);
		referenceoffset = ureadtwo(input);
/*		printf("ResType %s NumofThisType %u RefOffset %u\n",  */
		printf("ResType `%s' NumofThisType %u RefOffset %u\n", 
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
					printf(
/*	"\tResID %u ResName %s ResAttrib %u ResDataOff %lu ResDataLen %lu\n", */
		"\tResID %u ResName `%s' ResAttrib %u ResDataOff %lu ResDataLen %lu\n",
					resourceid, str, resattributes, 
						resdataoffset, resdatalength);
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

void showusage(char *s) {
	printf("%s [-r] <mac-file>\n", s);
	printf("\tr:  Resource fork only (i.e. no 128 byte MacBinary header)\n");
	printf("\n");
	printf("\t    MACANAL shows structure of MacIntosh file\n");
	printf("\t    Lists data fork and resource fork details\n");
	exit(1);
}

int checkstart(void) {		/* see whether could be MacBinary header */
	int k, n;
	if (buffer[0] != 0) {
		if (traceflag) printf("Byte 0 (%d) not zero\n", buffer[0]);
		return 0;			/* should be zero */
	}
	if ((n = buffer[1]) == 0) {
		if (traceflag) printf("Byte 1 is zero\n");
		return 0;		/* should be between 0 and 64 */
	}
	if ((n = buffer[1]) > 64) {
		if (traceflag) printf("Byte 1 (%d) > 64\n", buffer[1]);
		return 0;		/* should be between 0 and 64 */
	}
	for (k = 0; k < n; k++) {
		if (buffer[k+2] == 0) {
			if (traceflag) printf("Byte %d is zero (file name)\n", k);
			return 0;		/* should be part of file name */
		}
	}
	return -1;
}

/* Returns 0 if does NOT look like BinHex style header */

int checkbinhex(void) {		/* see whether could be MacBinary header */
	int k, n;
	int flags;			/* could check CRC also */
/*	long reslen, datlen, */
	long nlen;

	n = buffer[0];
	if (n == 0) {
		if (traceflag) printf("Byte 0 zero\n");
		return 0;			/* should be zero in MacBinary header */
	}
	if (n > 63) {
		if (traceflag) printf("Byte 0 %d > 63\n", n);
		return 0;			/* length of file name <= 63 */
	}
	for (k = 0; k < n; k++) {
		if (buffer[k+1] == 0) {
			if (traceflag) printf("Byte %d is zero (file name)\n", k);
			return 0;		/* should be part of file name */
		}
	}
	if (buffer[n+1] != 0) {
		if (traceflag) printf("Version %d not zero\n", buffer[n+1]);
		return 0;			/* should be zero */
	}
	offset = 0;
	if (verboseflag) printf("BinHex style file: ");
	for (k = 0; k < n; k++)	putc(buffer[1+k], stdout);
	putc('\n', stdout);
/*	ignore one byte version ? */
	if (verboseflag) printf("Type: ");
	for (k = 0; k < 4; k++)	putc(buffer[n+2+k], stdout);
	putc(' ', stdout);
	if (verboseflag) printf("Creator: ");
	for (k = 0; k < 4; k++)	putc(buffer[n+2+4+k], stdout);
	putc(' ', stdout);
	flags = (buffer[n+2+4+4+k] << 8) | buffer[n+2+4+4+1+k];
	if (verboseflag) printf("Flags: %04X ", flags);
	putc('\n', stdout);		
	datalength = 0;
	for (k = 0; k < 4; k++)
		datalength = (datalength << 8) | buffer[n+2+4+4+2+k];
	dataforkpos = n+22;
	if (verboseflag)
		printf("Length of Data Fork: %ld bytes, starts at %ld\n",
			   datalength, dataforkpos);
	resourcelength = 0;
	for (k = 0; k < 4; k++)
		resourcelength = (resourcelength << 8) | buffer[n+2+4+4+2+4+k];
	resourceforkpos = n+22+datalength+2;
	if (verboseflag)
		printf("Length of Rsrc Fork: %ld bytes, starts at %ld\n",
			   resourcelength, resourceforkpos);
	nlen = n+22+datalength+2+resourcelength+2;
	if (verboseflag) {
		printf("Apparently ends at %ld ", nlen);
		if (nlen != filelength)
			printf("rather than at %ld", filelength);
		putc('\n', stdout);
	}
	return -1;		/* looks like BinHex header maybe */
}

#define SPLITPOINT 52

int reasonable(int k) {				/* return non-zero if passes test */
	unsigned long dataoffset, mapoffset, datalength, maplength;
	dataoffset = uscanfour(k);
	mapoffset = uscanfour(k+4);
	datalength = uscanfour(k+8);
	maplength = uscanfour(k+12);
	if (traceflag != 0)
		printf("ResDataOff %lu ResMapOff %lu ResDatLen %lu ResMapLen %lu\n",
			dataoffset, mapoffset, datalength, maplength);		
	if (dataoffset != 256) return 0;
	if (mapoffset == 0 || mapoffset > 16777216) return 0;
	if (datalength == 0 || datalength > 16777216) return 0;
	if (maplength == 0 || maplength > 16777216) return 0;
	return -1;
}

/* look for 0 0 1 0 */

int guessstart(int nmax) {	/* try and locate resource fork start pattern */
	int k;

	if (tryzerofirst != 0) {
		if (buffer[0] == 0 && buffer[1] == 0 &&
			buffer[2] == 1 && buffer[3] == 0) {
			if (reasonable(0) != 0)
				return 0;
		}
	}
	for (k = SPLITPOINT; k < nmax; k++) {
		if (buffer[k] == 0 && buffer[k+1] == 0 &&
			buffer[k+2] == 1 &&	buffer[k+3] == 0) {
			if (reasonable(k) != 0)
				return k;
		}
	}
	for (k = 0; k < SPLITPOINT; k++) {
		if (buffer[k] == 0 && buffer[k+1] == 0 &&
			buffer[k+2] == 1 &&	buffer[k+3] == 0) {
			if (reasonable(k) != 0)
				return k;
		}
	}
	return -1;
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(errout, "HASHED %ld\n", hash);	/* (void) _getch();  */
	fflush(stderr);
	return hash;
}

int decodeflag (int c) {
	switch(c) { 
		case 'v': verboseflag = 1; break;
		case 't': traceflag = 1; break;
/*		case 'r': nomacbinary = 1; break; */
		case 'r': resetnomac = 1; break;
		case 'z': tryzerofirst = 0; break;
		case '?': detailflag = 1; break;
		case 'o': offsetflag = 1; return -1;
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
	return 0;
}

int commandline(int argc, char *argv[]) {	/* check for command flags */
	unsigned int i;
	int c, firstarg=1;
	char *t;

	while (firstarg < argc && argv[firstarg][0] == '-') { 
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) break;	/* takes argument */
		}
		if (offsetflag != 0) {
			if ((t = (strchr(argv[firstarg], '='))) != NULL) t++;
			else {
				firstarg++;
				t = argv[firstarg];
			}
/*			printf("rest %s", t); */
			sscanf(t, "%ld", &offset); 
			firstarg++; 
			offsetflag = 0;
			resetnomac++;			/* 1993/Jan/31 */
/*			nomacbinary++; */
		}
		else firstarg++;
	}
	return firstarg;
}

void processfile(FILE *input) {
/*	long filelength; */
	int allocblocks;
	unsigned char *s;
	unsigned int k, nfilename;
	int m;
	int filelabel;

	fseek(input, 0, SEEK_END);
	filelength = ftell(input);
	fseek(input, 0, SEEK_SET);			/* rewind */
	allocblocks = (int) (filelength / 512);
	printf("File length %ld = %d * 512 + %d\n",
		   filelength, allocblocks, (int) (filelength - allocblocks * 512));

	if (nomacbinary == 0) {
/*		first try and read MacBinary file header of 128 bytes */	
		s = buffer;
		for (k = 0; k < MACBINARYHEAD; k++) *s++ = (char) getc(input);

		macbinflag = checkstart();
		if (macbinflag == 0) {
			if (checkbinhex()) {
				binhexflag = 1;
				printf("Header appears to be BinHeX style, not MacBinary\n");
			}
		}
/*		if (macbinflag == 0) */
		if (macbinflag == 0 && binhexflag == 0) {
			fprintf(errout, 
					"WARNING:  There appears to be no MacBinary header\n");
			nomacbinary++;
/* modified 1993/Feb/5 to read in more right away */
/*			m = guessstart(MACBINARYHEAD); */
/*			if (m < 0) {	*/
			for (k = 0; k < MACBINARYHEAD; k++) *s++ = (char) getc(input);
			m = guessstart(MACBINARYHEAD * 2);
			fseek(input, MACBINARYHEAD, SEEK_SET);	/* go back */
/*			} */
			if (m >= 0) {
				fprintf(errout, 
						"WARNING:  Guessing that resource fork starts at %d\n",	m);
				offset = m;
				fseek(input, m, SEEK_SET);			/* rewind */				
			}
			else if (buffer[0] == 0 && buffer[1] == 0) { 
				fprintf(errout, 
						"WARNING:  Assuming this is the resource fork only\n");
				fseek(input, 0, SEEK_SET);			/* rewind */
			}
			else {
				fprintf(errout, 
						"ERROR: Sorry, don't understand this file at all\n");
/*				exit(73); */
				return;
			}
		}
	}
/*	if (nomacbinary == 0) { */
	if (nomacbinary == 0 && binhexflag == 0) {
		printf("FileName: ");
		nfilename = uscantwo(0);
		for (k = 0; k < nfilename; k++) putc(buffer[k+2], stdout);
		putc('\n', stdout);

		printf("File Type:    ");
		for (k = 0; k < 4; k++) putc(buffer[k+65], stdout);
		putc('\n', stdout);

		printf("File Creator: ");
		for (k = 0; k < 4; k++) putc(buffer[k+69], stdout);
		putc('\n', stdout);

		datalength = uscanfour(83);
		printf("Data Fork Size:     %lu\n", datalength);	

		resourcelength = uscanfour(87);
		printf("Resource Fork Size: %lu\n", resourcelength);	

		creationdate = uscanfour(91);
		printf("Creation Date:     ");
		showdate(creationdate);

		modificationdate = uscanfour(95);
		printf("Modification Date: ");
		showdate(modificationdate);

		if (buffer[73] != 0) printf("File Flags:     %d\n", buffer[73]);
		if (buffer[74] != 0) {
			filelabel = buffer[74];
			if (showcolor != 0 && filelabel >= 0 && filelabel < 16
				&& strcmp(labels[filelabel], "") != 0) {
				printf("File Label: %s (%s)\n",
					   labels[filelabel], colors[filelabel]);
			}
			else printf("File Label:     %d\n", filelabel);
		}
		if (buffer[81] != 0) printf("Protected Flag: %d\n", buffer[81]);

		for (k = 75; k < 81; k++) {
			if (buffer[k] != 0) {
/*			printf("Desktop Location %d %d %d %d %d %d\n",
				buffer[75], buffer[76], buffer[77], 
					buffer[78], buffer[79], buffer[80]); */
				printf("Desktop Location v: %d h: %d d: %d\n",
					   uscantwo(75), uscantwo(77), uscantwo(79));
				break;
			}
		}
	}

	if (offset > 0) fseek (input, offset, SEEK_SET);

	if (resourcelength > 0 || nomacbinary != 0) {
		if (showresourcedataflag != 0 || showresourcemapflag != 0)
			setupresourcefork(input);
		if (showresourcedataflag != 0) showresourcedata(input);
		if (showresourcemapflag != 0) showresourcemap(input);
	}
}

int main(int argc, char *argv[]) {
	char macfile[FILENAME_MAX];
	FILE *input;
	int firstarg = 1;
	int l;

	if (argc < 2) showusage (argv[0]);

	firstarg = commandline (argc, argv);

	printf("MACANAL program for analysing Macintosh file structure %s\n",
		  version);

	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	for (l = firstarg; l < argc; l++) {

		nomacbinary = resetnomac;
		binhexflag=0;

		strcpy(macfile, argv[l]);
		if((input = fopen(macfile, "rb")) == NULL) {
			extension(macfile, "mac");
			if((input = fopen(macfile, "rb")) == NULL) {
				extension(macfile, "bin");
				if((input = fopen(macfile, "rb")) == NULL) {
					perror(argv[1]); 
					continue;
				}
			}
		}
		printf("Processing %s\n", macfile);
		processfile(input);
		fclose(input);
		putc('\n', stdout);
	}
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

