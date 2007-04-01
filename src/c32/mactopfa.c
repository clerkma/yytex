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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* #include <conio.h> */

#include <time.h>
/* #include <sys\types.h> */
#include <sys\stat.h>
#include <sys\utime.h>		/* for _utimbuf etc. */

/* #define FNAMELEN 80 */
#define MACBINARYHEAD 128
#define MAXRESOURCE 32767	/* could be longer ? */

int verboseflag = 0;			/* verbose */
int traceflag = 0;				/* very verbose */
int detailflag=0;
int tryzerofirst = 1;		/* Try zero offset first */
int showcolor=1;			/* show label/color of file */

int civilized=1;
int showaccessflag=0;
int showmodifyflag=0;

int convertreturn=1;	/* \r => \n */

int columns=78;			/* columns max in one line */

int resetnomac = 0;		/* command line - non-zero if no MacBinary header */
int nomacbinary = 0;	/* per file non-zero if no MacBinary header */
long offset = 0;		/* offset into file */

/* int noheader=0; */
int underflag=0;
int decompressflag=1;

int clmnflag=0;
/* int pathflag=0; */
int offsetflag=0;

unsigned long filelength;		/* global file name */

int dopostsequence=1;	/* do POST resources in sequence */

int showresourcedataflag = 0;
int showresourcemapflag = 0;	/* 1 */
int showeachresource = 0;		/* 1 */

/* resource numbering 0 -- 127 systems resources */
/* resource numbering 128 to 32767 applications */

/* Resource Attributes: */

#define RESSYSHEAP 64		/* 1 => read into system heap, 0 => application */
#define RESPURGEABLE 32		/* 1 => purgeable, 0 => not purgeable */
#define RESLOCKED 16		/* 1 => locked, 0 => unlocked */
#define RESPROTECTED 8		/* 1 => protected, 0 => unprotected */
#define RESPRELOAD 4		/* 1 => preload, 0 => don't preload */
#define RESCHANGED 2		/* 1 => write to resource file, 0 => don't write */

/* char *path=""; */

char *copyright = "\
Copyright (C) 1990-1998  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1990, 1991  Y&Y. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 2670755 */
/* #define COPYHASH 5769561 */
/* #define COPYHASH 11117870 */
#define COPYHASH 12415373

FILE *errout=stdout;

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
/*	return ((getc(input) << 8) | getc(input)); */
	unsigned int c, d;
	c = getc(input);
	d = getc(input);	
	if (c < 0 || d < 0) fprintf(errout, "EOF ");
	return ((c << 8) | d);
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
	if (c < 0 || d < 0 || e < 0) fprintf(errout, "EOF ");
	return ((((unsigned long) c << 8) | d) << 8) | e;
} 

unsigned long ureadfour (FILE *input) {
	unsigned int c, d, e, f;
	c = getc(input);	d = getc(input);
	e = getc(input);	f = getc(input);
	if (c < 0 || d < 0 || e < 0 || f < 0) fprintf(errout, "EOF ");
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

/*********************************************************************************/

/* char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
}; */

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char newyear[6];

	strcpy (newyear, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = newyear[k];
	date[4] = ' ';
	return;
}

int year, month, day, hour, minute, second;

int defyear, defmonth, defday;

struct tm *tm;

struct tm newtimemt;

time_t newtime;

struct _stat statbuf;

struct _utimbuf timebuf;	/* set up before domodification */

int getinfo(char *filename, int verboseflag) {
	char *s;
	int result;

	if (traceflag)
		printf("Getting date and time for file %s\n", filename);
	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(errout, "ERROR: Unable to obtain info on %s\n", filename);
		return -1;
	}
	s = ctime(&statbuf.st_ctime);
	if (traceflag) printf("CTIME (of ctime %0X): %s", statbuf.st_ctime, s);
	if (s == NULL) exit(1);
	if (civilized) lcivilize(s);
	if (verboseflag && ! showaccessflag && ! showmodifyflag)
		printf("%s created:       %s", filename, s);
	s = ctime(&statbuf.st_atime);
	if (traceflag != 0) printf("CTIME (of atime %0X): %s", statbuf.st_atime, s);
	if (s == NULL) exit(1);
	if (civilized) lcivilize(s);
	if (verboseflag && (showaccessflag || ! showmodifyflag))
		printf("%s last accessed: %s", filename, s);
	s = ctime(&statbuf.st_mtime);
	if (traceflag) printf("CTIME (of mtime %0X): %s", statbuf.st_mtime, s);
	if (s == NULL) exit(1);
	if (civilized) lcivilize(s);
	if (verboseflag && (showmodifyflag || ! showaccessflag))
		printf("%s last modified: %s", filename, s);
/*	strcpy (filetime, s); */
	return 0;
}

/* this date nonsense: should really use `timediffer' and reference */

unsigned long yearsoff = 86400 * 1461;		/* offset 1904 - 1900 */
/* unsigned long yearsoff = 86400 * 24107;	*/	/* offset 1970 - 1904 */
/* unsigned long hoursoff = 3600 * 8; */	/* GMT - EZT ??? */
unsigned long hoursoff = 3600 * 9;			/* GMT - EZT ??? */

/*	creationdate = modtime - yearsoff - hoursoff */

int makenewtime (void) {
	int flag = 0;

/*	Make sure we have valid data more or less */		
	if (year < 1900) year = 1900;
	else if (year > 2099) year = 2099;
	if (month < 0) month = 0;
	else if (month > 11) month = 11;
	if (day < 1) day = 1;
	else if (day > 31) day = 31;
	if (hour > 23) hour = 23;
	else if (hour < 0) hour = 0;
	if (minute > 59) minute = 59;
	else if (minute < 0) minute = 0;
	if (second > 59) second = 59;
	else if (second < 0) second = 0;
	if (verboseflag != 0) {
		printf("%4d %s %02d ", year, months[month], day);
		printf("%02d:%02d:%02d ", hour, minute, second);
		putc('\n', stdout);
	}
	newtimemt.tm_sec = second;
	newtimemt.tm_min = minute;
	newtimemt.tm_hour = hour;
	newtimemt.tm_mday = day;
	newtimemt.tm_mon = month;
	newtimemt.tm_year = year - 1900;
	newtimemt.tm_wday = 0; 
	newtimemt.tm_yday = 0;
	newtimemt.tm_isdst = 0;

	newtime = mktime(&newtimemt);
	if (newtime == -1) {
		fprintf(errout, "Cannot make time!\n");
		return -1;
	}
	return flag;
}

/* we want to extract the creationdate and modificationdate */
/* from MacBinary header and use that */

int domodification (char *filename) {
	if (verboseflag != 0)
		printf("Will now try and modify time of %s\n", filename);
	if (_utime(filename, &timebuf) != 0) {
		fprintf(errout, "Unable to modify date/time of %s\n", filename);
		perror(filename);
		return -1;
	}
/*	if (getinfo(filename, traceflag) < 0) return -1; */
	if (getinfo(filename, verboseflag) < 0) return -1;
	return 0;
}

/*********************************************************************************/

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

char *specialchar (int c) {
	if (c == 167) return "(beta)";
	else if (c == 168) return "(R)";
	else if (c == 169) return "(C)";
	else if (c == 170) return "(TM)";
	else if (c == 208) return "--";
	else if (c == 209) return "---";
	else return NULL;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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
unsigned long resourceforkpos;
long beginresource;

/* returns non-zero if there is a problem */

int setupresourcefork(FILE *input) {
	resourceforkstart = ((datalength + 511) / 512) * 512;
/*	skip over data fork */
	if (verboseflag != 0) {
		if (offset == 0) printf("Resource Fork starts at %lu\n", 
			resourceforkstart);
		else printf("Resource Fork starts at %lu + %d\n", 
			resourceforkstart, offset);
	}
/*	if (nomacbinary != 0) resourceforkpos = resourceforkstart; */
	if (nomacbinary != 0) resourceforkpos = resourceforkstart + offset;
	else resourceforkpos = resourceforkstart + MACBINARYHEAD;
	if (resourceforkpos > filelength) {
		fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
				resourceforkpos, filelength);
		return -1;
	}
	if (fseek(input, resourceforkpos, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return -1;
	}
	beginresource = ftell(input);
	if (feof(input)) {
		fprintf(errout, "EOF!\n");
		return -1;
	}
/*	read resource fork header */
	resourcedataoffset = ureadfour(input);
	resourcemapoffset  = ureadfour(input);
	resourcedatalength = ureadfour(input);
	resourcemaplength  = ureadfour(input);
	if (traceflag != 0)
		printf("ResDataOff %lu ResMapOff %lu ResDatLen %lu ResMapLen %lu\n",
			resourcedataoffset,	resourcemapoffset, 
				resourcedatalength, resourcemaplength);
/*	do some sanity checks */
	if (resourcedataoffset > (unsigned long) filelength) {
		fprintf(errout, "Ridiculous ResDataOff %lu\n", resourcedataoffset);
		return -1;
	}
	if (resourcemapoffset > (unsigned long) filelength) {
		fprintf(errout, "Ridiculous ResMapOff %lu\n", resourcemapoffset);
		return -1;
	}
	return 0;
}

int showresourcedata(FILE *input) {
	unsigned long reslength, resposition, totallength;
	int k;

	resposition = resourcedataoffset;
	if (resposition + beginresource > filelength) {
		fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
				resposition + beginresource, filelength);
		return -1;
	}
	if (fseek(input, resposition + beginresource, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return -1;
	}
	totallength = 0L;
	for(k = 1; k > 0; k++) {
		reslength = ureadfour(input);
		printf("K %d position %ld length %ld\n", k, resposition, reslength);
		resposition += (reslength + 4);
		totallength += (reslength + 4);
		if (totallength >= resourcedatalength) break;
		if (resposition + beginresource > filelength) {
			fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
					resposition + beginresource, filelength);
			return -1;
		}
		if (fseek(input, resposition + beginresource, SEEK_SET) != 0) {
			fprintf(errout, "SEEK FAILED\n");
			return -1;
		}
	}
	return 0;
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

/* char str[MAXSTRING]; */		/* 1992/Nov/8 */

int showresourcemap(FILE *input) {
	unsigned long resposition, resdatalength;
	unsigned int resourceattributes, typelistoffset, namelistoffset;
	unsigned int numberoftypes, numberofthistype, referenceoffset;
	unsigned int resourceid, nameoffset, resattributes;
	unsigned long resdataoffset;
	char str[MAXSTRING];  /* 1992/Nov/8 */
	char resourcetype[4+1];
	long beginresmap, begintype, present, current;
	unsigned int k, l;

	resposition = resourcemapoffset;
	if (resposition + beginresource > filelength) {
		fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
				resposition + beginresource, filelength);
		return -1;
	}
	if (fseek(input, resposition + beginresource, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return -1;
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
	if (typelistoffset + beginresmap > filelength) {
		fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
				typelistoffset + beginresmap, filelength);
		return -1;
	}
	if (fseek(input, typelistoffset + beginresmap, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return -1;
	}
	begintype = ftell(input);
	numberoftypes = ureadtwo(input);
	printf("NumOfTypes %u\n", numberoftypes+1);
	if (numberoftypes+1 == 0) {
		fprintf(errout, "Premature end of file\n");
		return -1;
	}
	for (k = 0; k <= numberoftypes; k++) {		/* ??? */
		getfourbytestring(input, resourcetype);
/*		numberofthistype = ureadtwo(input); */
		numberofthistype = ureadtwo(input)+1;
		referenceoffset = ureadtwo(input);
		printf("ResType %s NumofThisType %u RefOffset %u\n", 
/*			resourcetype, numberofthistype+1, referenceoffset); */
			resourcetype, numberofthistype, referenceoffset);
		if (showeachresource != 0) {
			present = ftell(input);
			if (begintype + referenceoffset > filelength) {
				fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
						begintype + referenceoffset, filelength);
				return -1;
			}
			fseek(input, begintype + referenceoffset, SEEK_SET);
/*			for (l = 0; l <= numberofthistype; l++) {	*/
			for (l = 0; l < numberofthistype; l++) {	
				resourceid = ureadtwo(input);
				nameoffset = ureadtwo(input);
				resattributes = getc(input);
				if (resattributes == EOF) {
					fprintf(errout, "Unexpected EOF\n");
					return -1;
				}
				resdataoffset = ureadthree(input);
				(void) ureadfour(input);
				current = ftell(input);
				if (resdataoffset + resourcedataoffset + beginresource > filelength) {
					fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
							resdataoffset + resourcedataoffset + beginresource, filelength);
					return -1;
				}
				fseek (input, resdataoffset + resourcedataoffset + beginresource, 
						SEEK_SET);
				resdatalength = ureadfour(input);
				if (nameoffset == 65535) {
					printf(
		"\tResID %u ResAttrib %u ResDataOff %lu ResDataLen %lu\n",
					resourceid, resattributes, 
						resdataoffset, resdatalength);
				}
				else {
					if (nameoffset + namelistoffset + beginresmap > filelength) {
						fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
								nameoffset + namelistoffset + beginresmap, filelength);
						return -1;
					}
					fseek(input,
						  nameoffset + namelistoffset + beginresmap,
							SEEK_SET);
					copyresname(input, str);
/* debugging hack .... */
					if (strcmp(str, "") == 0) sprintf(str, "%u", nameoffset);
					printf(
		"\tResID %u ResName %s ResAttrib %u ResDataOff %lu ResDataLen %lu\n",
					resourceid, str, resattributes, 
						resdataoffset, resdatalength);
				}
				fseek(input, current, SEEK_SET);
			}
			fseek(input, present, SEEK_SET);
		}
	}
	return 0;
}

void writelength(FILE *output, long n) { /* write length code in PC form */
	int c, k;
	for (k = 0; k < 4; k++) {
		c = (int) (n & 255);
		n = n >> 8;
		putc(c, output);
	}
}

int last=1;				/* type of last resource */
int clm=0;
int lastpost;
int postcount;

/* Process one POST resource */

int processpost(FILE *output, FILE *input, 
			unsigned long resdatalength, int resnum) {
	int c, d;
	unsigned long n, j;
	char *s;
	
	postcount++;

	if (resnum != lastpost+1)
		fprintf(errout, "ERROR: POST resource numbers not sequential\n");
	lastpost = resnum;

	if (traceflag != 0) 
		printf("Processing POST resource %d length %lu\n", 
			resnum, resdatalength);
	c = getc(input);		/* check out the record type */
	d = getc(input);
	if (c > 5 || d != 0) { 	/* probably not a POST resource ! */
		fprintf(errout, "WARNING: ignoring bad POST resource %d\n", resnum);
		return -1;
	}
	n = resdatalength - 2;	/* number of bytes left */

	if (c == 0) {			/* 0 ==> comment */
		if (verboseflag != 0) 
			printf("POST %d: Comment of length %ld: ", resnum, n);
		if (n > MAXRESOURCE) {
			fprintf(errout, "\nWARNING: Comment suspiciously long\n"); 
/*			_getch(); */
		}
		for (j = 0; j < n; j++) {
			c = getc(input);
			if (c == EOF) {
				fprintf(stderr, "ERROR: Unexpected EOF - reading comment\n");
				exit(4);
			}
			putc(c, stdout);	/* copy comment to console */
		}
		putc('\n', stdout);
	}	/* end of 0 ==> Comment case */

	else if (c == 1) {	/* 1 ==> ASCII text */
		if (verboseflag != 0)
			printf("POST %d: ASCII  section of length %ld\n", resnum, n);
		if (decompressflag == 0) {		/* PFB ASCII section header */
			putc(128, output);
			putc(1, output);
			writelength(output, n);
		}				
		else if (clm > 0 && last == 2) {  /* was last a binary section ? */
			putc('\n', output);	clm = 0;  /* 92/Nov/8 ??? */
		}
		last = 1;			/* remember this was an ASCII section */
		for (j = 0; j < n; j++) {
			c = getc(input);
			if (c == EOF) {
				fprintf(stderr, "ERROR: Unexpected EOF - reading ASCII text\n");
				exit(4);
			}
			if (c == '\r' && convertreturn != 0) c = '\n';
			if (c == '\n' || c == '\r') clm = 0;
			else clm++;
/* not sure	the following hair is really called for ... */
/* but, probably better not to include weird control and meta characters */
/*			putc(c, output); */
			if (c < 32 && c != '\n' && c != '\r' 
				&& c != '\t' && c != '\f') { /* complain control */
				c += 64;
				fprintf(errout, "C-%c ", c);
/*				fprintf(output, "C-%c", c); */
			}
/*			else if (c == 169) {	
				fprintf(output, "(C)");
				if (verboseflag != 0) printf("(C)\n");
			} */
/*			else if (c == 167) {	
				fprintf(output, "(beta)");
				if (verboseflag != 0) printf("(beta)\n");
			} */
			else if ((s = specialchar(c)) != NULL) {	/* 1994/July/18 */
				fputs(s, output);
				if (verboseflag) fputs(s, stdout);
			}
			else if (c >= 128) {		/* complain meta characters */
				c -= 128;
				if (c < 32) {
					c += 64;
					fprintf(errout, "M-C-%c ", c);
/*					fprintf(output, "M-C-%c", c); */
				}
				else {
					fprintf(errout, "M-%c ", c);
/*					fprintf(output, "M-%c", c); */
				}
			}
			else putc(c, output);  
		}
/*		putc('\n', output);	clm = 0; */
	}/* end of 1 ==> ASCII data case */

	else if (c == 2) { /* 2 ==> binary data */
		if (verboseflag != 0)
			printf("POST %d: Binary section of length %ld\n", resnum, n);
/*			printf("LAST was %d CLM is %d\n", last, clm); *//* debugging */
		if (decompressflag == 0) {	/* PFB binary section code */
			putc(128, output);
			putc(2, output);
			writelength(output, n);
		}
		else if (clm > 0 && last == 1) { /* last an ASCII section ? */
			putc('\n', output);	clm = 0;  /* 92/Nov/8 ??? */
		}
		last = 2;
		for (j = 0; j < n; j++) {
			c = getc(input);
			if (c == EOF) {
				fprintf(stderr, "ERROR: Unexpected EOF - reading binary\n");
				exit(4);
			}
			if (decompressflag == 0) putc(c, output);
			else {
				d = c & 15;
				c = c >> 4;
				if (c < 10) c = c + '0';
				else c = c - 10 + 'A';
				if (d < 10) d = d + '0';
				else d = d - 10 + 'A';		
				putc(c, output);
				putc(d, output);				
				if ((clm = clm + 2) >= columns) {
					putc('\n', output);
					clm = 0;
				}
			}
		}		
	}	/* end of 2 ==> binary data case */

	else if (c == 3) { /* 3 ==> EOF marker */
		if (verboseflag != 0) printf("POST %d: EOF marker", resnum);
		putc(4, output);
		return 0;		/* ??? */
	}	/* end of 3 ==> EOF marker case */

	else if (c == 4) { /* 4 ==> font program in data fork */
		if (verboseflag != 0) 
			printf("POST %d: Font in data fork => supposedly not possible",
				resnum);
		return -1;		/* ??? */
	}	/* end of 4 ==> font data in data fork case */

	else if (c == 5) { /* 5 ==> end of font program */
		if (decompressflag == 0) {
			putc(128, output);
			putc(3, output);
		}
		if (verboseflag != 0)
			printf("POST %d: End of font program\n", resnum);
		return 0;
	}
	return 0;
}

void processvers(FILE *output, FILE *input, 
			unsigned long resdatalength, int resnum) {
/*	unsigned long k; */
	int c; 
	int i, n;
	char *s;

	if (traceflag != 0) 
		printf("Processing vers resource %d length %lu\n", 
			resnum, resdatalength);			
	if (verboseflag == 0) return;
/*	for (k = 0; k < resdatalength; k++) {
		c = getc(input); putc(c, stdout);
	} */
	printf("Version resource %d: ", resnum);
	for (i = 0; i < 6; i++) getc(input);	/* skip over cruft */
	n = getc(input);
	for (i = 0; i < n; i++) putc(getc(input), stdout);
	printf(" - ");
	n = getc(input);
	for (i = 0; i < n; i++) {
		c = getc(input);
/*		if (c == 169) printf("(C)"); */
/*		if (c == 167) printf("(beta)"); */
		if ((s = specialchar(c)) != NULL) {
			fputs(s, stdout);
		}
		else putc(c, stdout);
	}
	putc('\n', stdout);
}

/* Try and find a particular POST resource */

int gotopost(FILE *input, unsigned int post, int n, unsigned long poststart) {
	int l;
	int resattributes;
	unsigned int resourceid, nameoffset;
	unsigned long resdataoffset;
	
	if (verboseflag != 0) printf("Sequence jump %d => %d\n", lastpost, post);
	if (poststart > filelength) {
		fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
				poststart, filelength);		
		return -1;
	}
	if (fseek(input, poststart, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return -1;
	}
	for (l = 0; l < n; l++) {
		resourceid = ureadtwo(input);
/*		printf("resourceid %u post %u\n", resourceid, post); */
		if (resourceid == post) return 0;
		nameoffset = ureadtwo(input);
		resattributes = getc(input);
		resdataoffset = ureadthree(input);
		(void) ureadfour(input);
	}
	fprintf(errout, "ERROR: POST resource %d not found\n", post);
	fseek(input, poststart, SEEK_SET);
	return -1;
}

/* returns non-zero if there is a problem */

int readpostresources(FILE *output, FILE *input) {
	unsigned long resposition, resdatalength;
	unsigned int resourceattributes, typelistoffset, namelistoffset;
	unsigned int numberoftypes, numberofthistype, referenceoffset;
	unsigned int resourceid, nameoffset, resattributes;
	unsigned long resdataoffset;
	char resourcetype[4+1];
/*	char str[MAXSTRING]; */
	long beginresmap, begintype, present, current;
	unsigned int k, l;

	resposition = resourcemapoffset;
	if (resposition + beginresource > filelength) {
		fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
				resposition + beginresource, filelength);
		return -1;
	}
	if (fseek(input, resposition + beginresource, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return -1;
	}
/*	printf("MILE STONE 2"); */
	beginresmap = ftell(input);
	skipbytes(input,  16 + 4 + 2);
	resourceattributes = ureadtwo(input);
	if (traceflag != 0) printf("Resource Attributes %d\n", resourceattributes);
	typelistoffset = ureadtwo(input);
	if (typelistoffset == 65535) {
		fprintf(errout, "Ridiculous TypeListOffset %u\n", typelistoffset);
		return -1;
	}
	namelistoffset = ureadtwo(input);
	if (namelistoffset == 65535) {
		fprintf(errout, "Ridiculous NameListOffset %u\n", namelistoffset);
		return -1;
	}
	if (traceflag != 0) printf("TypeListOffset %u NameListOffset %u\n",
		typelistoffset,	namelistoffset);
/*	go to Type List */
/*	if (fseek(input, typelistoffset + resposition + MACBINARYHEAD,  */
	if (typelistoffset + beginresmap > filelength) {
		fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
				typelistoffset + beginresmap, filelength);
		return -1;
	}
	if (fseek(input, typelistoffset + beginresmap, SEEK_SET) != 0) {
		fprintf(errout, "SEEK FAILED\n");
		return -1;
	}
	begintype = ftell(input);
	numberoftypes = ureadtwo(input);
	if (traceflag != 0) printf("NumOfTypes %u\n", numberoftypes+1);
	if (numberoftypes+1 == 0) {
		fprintf(errout, "Premature end of file\n");
		return -1;
	}

/*	printf("MILE STONE 3"); */

/*	step through the different types */	
	for (k = 0; k <= numberoftypes; k++) {		/* ??? */
		getfourbytestring(input, resourcetype);
/*		numberofthistype = ureadtwo(input); */
		numberofthistype = ureadtwo(input)+1;
		referenceoffset = ureadtwo(input);
		if (traceflag != 0) 
			printf("ResType %s NumofThisType %u RefOffset %u\n", 
/*				resourcetype, numberofthistype+1, referenceoffset); */
				resourcetype, numberofthistype, referenceoffset);

/*	look for POST resources */
		if (strcmp(resourcetype, "POST") == 0) {
/*			printf("MILE STONE 4"); */
			present = ftell(input);
			if (begintype + referenceoffset > filelength) {
				fprintf(errout, "SEEKING TO %lu PAST END %lu\n",
						begintype + referenceoffset, filelength);
				return -1;
			}
			fseek(input, begintype + referenceoffset, SEEK_SET);
/*	step through the POST resources */
			last = 1;
			clm = 0;			/* column zero */
			lastpost=500;
/*			for (l = 0; l <= numberofthistype; l++) {	*/
			for (l = 0; l < numberofthistype; l++) {	/* ??? */
/*					printf("MILE STONE 5"); */
				resourceid = ureadtwo(input);
				if (dopostsequence != 0 && resourceid != 501 + l) {
					if (gotopost(input, 501 + l, 
						numberofthistype, begintype + referenceoffset) != 0)
							continue;
/*					resourceid = ureadtwo(input); */
					resourceid = 501 + l;
				}
				nameoffset = ureadtwo(input);
				resattributes = getc(input);
				if (resattributes == EOF) {
					fprintf(errout, "Unexpected EOF\n");
					return -1;
				}
				resdataoffset = ureadthree(input);
				(void) ureadfour(input);
				current = ftell(input);
/* go to the resource */
				fseek (input, 
					resdataoffset + resourcedataoffset + beginresource, 
						SEEK_SET);
				resdatalength = ureadfour(input);
/* ignore the name of the resource (assume nameoffset == 65535) */
				if (nameoffset != 65535)
					fprintf(errout, "ERROR: POST resource is named\n");
/* go and process the POST resource */
				if (processpost(output, input, resdatalength, resourceid)
					!= 0) {
/* some kind of error in processing POST */
				}
/* go back to listing of resources of this type */				
				fseek(input, current, SEEK_SET);

				if (ferror(output) != 0) {			/* 1992/Dec/11 */
/*					fprintf(errout, "Output error"); */
					perror("Output error");
					exit(2);
				}
			}
/* go back to listing of resource types */
			fseek(input, present, SEEK_SET);
		}

/* Look for version number resources */

		if (strcmp(resourcetype, "vers") == 0) {
			present = ftell(input);
			fseek(input, begintype + referenceoffset, SEEK_SET);
/*	step through the vers resources */
/*			for (l = 0; l <= numberofthistype; l++) {	*/
			for (l = 0; l < numberofthistype; l++) {	/* ??? */
				resourceid = ureadtwo(input);
				nameoffset = ureadtwo(input);
				resattributes = getc(input);
				if (resattributes == EOF) {
					fprintf(errout, "Unexpected EOF\n");
					return -1;
				}
				resdataoffset = ureadthree(input);
				(void) ureadfour(input);
				current = ftell(input);
/* go to the resource */
				fseek (input, 
					resdataoffset + resourcedataoffset + beginresource, 
						SEEK_SET);
				resdatalength = ureadfour(input);
/* ignore the name of the resource (assume nameoffset == 65535) */
				if (nameoffset != 65535)
					fprintf(errout, "ERROR: vers resource is named\n");
/* go and process the POST resource */
				processvers(output, input, resdatalength, resourceid);
/* go back to listing of resources of this type */				
				fseek(input, current, SEEK_SET);
			}
/* go back to listing of resource types */
			fseek(input, present, SEEK_SET);
		}

	}
	return 0;			/* normal exit */
}

void showdate(unsigned long date) {
	showdays(extractdays(date));
	showseconds(extractseconds(date));	
	putc('\n', stdout);
}

void showusage(char *s) {
	printf("%s [-{v}{r}] <mac-file> ...\n", s);
	printf("\tv  Verbose mode\n");
	printf("\tr  Resource fork only (i.e. no 128 byte MacBinary header)\n");
/*	printf("\n"); */
	exit(1);
}

int checkstart(void) {		/* see whether could be MacBinary header */
	int k, n;
	if (buffer[0] != 0) return 0;			/* should be zero */
	if ((n = buffer[1]) == 0) return 0;		/* should be between 0 and 64 */
	if ((n = buffer[1]) > 64) return 0;		/* should be between 0 and 64 */
	for (k = 0; k < n; k++) {
		if (buffer[k+2] == 0) return 0;		/* should be part of file name */
	}
	return -1;
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

void forceunder(char *name) {
	int i, n=0;
	char *s=name;
	while(*s != '\0') {
		n++; s++;
	}
	for (i=n; i < 8; i++) *s++ = '_';
	*s = '\0';
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(errout, "HASHED %ld\n", hash);	/* (void) _getch();  */
	fflush(errout);
	return hash;
}

void decodeflag (int c) {
	switch(c) { 
		case 'v': verboseflag = 1; break;
		case 't': traceflag = 1; break;
		case 'b': decompressflag = 0; break;
		case 'u': underflag = 1; break;
/*		case 'p': pathflag = 1; break; */
		case 'c': clmnflag = 1; break;
/*		case 'r': nomacbinary = 1; break; */
		case 'r': resetnomac = 1; break;
/*		case 'n': noheader = 1; break; */
		case 'o': offsetflag = 1; break;
		case '?': detailflag = 1; break;
		default: {
				fprintf(errout, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
}

int commandline (int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
/*	while (argv[firstarg][0] == '-') */ /* check for command flags */
	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else decodeflag(c);
		}
		firstarg++;
/*		if (pathflag != 0) {
			path = argv[firstarg]; firstarg++; pathflag = 0;
		} */
		if (clmnflag != 0) {
			sscanf(argv[firstarg], "%d", &columns); firstarg++; clmnflag = 0;
		}		
		if (offsetflag != 0) {
			sscanf(argv[firstarg], "%ld", &offset); firstarg++; offsetflag = 0;
			nomacbinary++;
		}
	}
	return firstarg;
}

int main(int argc, char *argv[]) {
	char macfile[FILENAME_MAX], outfile[FILENAME_MAX];
	FILE *input, *output;
/*	long filelength; */
	int allocblocks;
	unsigned int k, nfilename;
	int firstarg = 1;
	int m, l;
/*	unsigned char i; */
	unsigned char *s; 
	char *t;
	int filelabel;


	if (firstarg + 1 > argc) showusage (argv[0]);

	firstarg = commandline(argc, argv, firstarg);

	if (firstarg >= argc) showusage(argv[0]); /* left out extension ? */

/*	if (traceflag) printf("After command line\n"); */

	if (traceflag != 0) {
		showresourcemapflag++;
		showeachresource++;
	}

/*	need to check copyright hashing ... */

#ifdef _WIN32
	printf("MACtoPFA (32) font conversion program version 1.4.1\n");
#else
	printf("MACtoPFA font conversion program version 1.3\n");	
#endif
	
	if (checkcopyright(copyright) != 0) {
/*		fprintf(errout, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	for (l = firstarg; l < argc; l++) {

		nomacbinary = resetnomac;		/* non-zero if no MacBinary header */
		offset = 0;						/* ??? */
		resourcelength = 0;				/* ??? */

		if (verboseflag) printf("Opening %s ", argv[l]);
		strcpy(macfile, argv[l]);
		if((input = fopen(macfile, "rb")) == NULL) {
			extension(macfile, "mac");
			if((input = fopen(macfile, "rb")) == NULL) {
				perror(macfile); 
				continue;
			}
		}
		if (verboseflag != 0) printf("Processing %s\n", macfile);
		(void) getinfo (macfile, verboseflag);

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

/*			if (buffer[0] == 0 && buffer[1] == 0) */
			if (checkstart() == 0) {
				fprintf(errout, 
						"WARNING:  There appears to be no MacBinary header\n");
				nomacbinary++;
/* modified 1993/Feb/5 to read in more right away */
/*				m = guessstart(MACBINARYHEAD); */
/*				if (m < 0) 	*/
				for (k = 0; k < MACBINARYHEAD; k++) *s++ = (char) getc(input);
				m = guessstart(MACBINARYHEAD * 2);
				fseek(input, MACBINARYHEAD, SEEK_SET);	/* go back */
/*			 */
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
/*					exit(73); */
					fclose(input);
					continue;
				}
			}
		}	 /* end of if nomacbinary == 0 */

/*		if (nomacbinary == 0 && traceflag != 0)  */
		if (nomacbinary == 0) {
			if (traceflag != 0) {
				printf("FileName: ");
				nfilename = uscantwo(0);
				for (k = 0; k < nfilename; k++) 	putc(buffer[k+2], stdout);
				putc('\n', stdout);
			}
	
			if (traceflag != 0) {
				printf("File Type:    ");
				for (k = 0; k < 4; k++) 	putc(buffer[k+65], stdout);
				putc('\n', stdout);
			}
	
			if (traceflag != 0) {
				printf("File Creator: ");
				for (k = 0; k < 4; k++) 	putc(buffer[k+69], stdout);
				putc('\n', stdout);
			}
	
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
	
			if (traceflag != 0) {
			if (buffer[73] != 0) printf("File Flags:     %d\n", buffer[73]);
/*			if (buffer[74] != 0) printf("File Label:     %d\n", buffer[74]); */
			filelabel = buffer[74];
			if (showcolor != 0 && filelabel >= 0 && filelabel < 16
				&& strcmp(labels[filelabel], "") != 0) {
				printf("File Label: %s (%s)\n",
					labels[filelabel], colors[filelabel]);
			}
			else printf("File Label:     %d\n", filelabel);
			if (buffer[81] != 0) 	printf("Protected Flag: %d\n", buffer[81]);
			}

			if (traceflag != 0) {
			for (k = 75; k < 81; k++) {
				if (buffer[k] != 0) {
/*					printf("Desktop Location %d %d %d %d %d %d\n",
						buffer[75], buffer[76], buffer[77], 
							buffer[78], buffer[79], buffer[80]); */
					printf("Desktop Location v: %d h: %d d: %d\n",
						uscantwo(75), uscantwo(77), uscantwo(79));
					break;
				}
			}
			}	
		} /* end of if nomacbinary == 0 */

		if (offset > 0) fseek (input, offset, SEEK_SET);

		if (resourcelength > 0 || nomacbinary != 0) {
/*		if (showresourcedataflag != 0 || showresourcemapflag != 0) */
			if (setupresourcefork(input) != 0) {
				fclose(input);
				continue;
			}
			if (showresourcedataflag != 0) showresourcedata(input);
			if (showresourcemapflag != 0) showresourcemap(input);
		}

		if ((t=strrchr(macfile, '\\')) != NULL) t++;
		else if ((t=strrchr(macfile, '/')) != NULL) t++;
		else if ((t=strrchr(macfile, ':')) != NULL) t++;
		else t = macfile;
		strcpy(outfile, t);  	/* copy input file name minus path */

		if (underflag != 0) forceunder(outfile);

		if (decompressflag != 0) forceexten(outfile, "pfa");
		else forceexten(outfile, "pfb");

		if (traceflag != 0) printf("Output goes to %s\n", outfile);

		if (decompressflag != 0) {
			if ((output = fopen(outfile, "w")) == NULL) {
				putc('\n', errout); 
				perror(outfile);
				exit(2);
			}
		}
		else {
			if ((output = fopen(outfile, "wb")) == NULL) { 
				putc('\n', errout); 
				perror(outfile);
				exit(2);
			}
		}
		if (verboseflag) printf("Opened %s ", outfile);

/*		printf("MILE STONE 1"); */

		postcount = 0;

		if (readpostresources(output, input) != 0) {
/*			hit an error ... */
			fprintf(errout, "Failed to read POST resources\n");
		}

		if (postcount == 0) 
			fprintf(errout, 
				"ERROR: found no POST resources - probably not a font file\n");

		fclose(input);
		if (ferror(output) != 0) {
			fprintf(errout, "Output error");
			perror(outfile);
			exit(2);
		}
		else fclose(output);

/*		for now just copy properties of source file rather than from */
/*		MacBinary header */
/*		makenewtime(); */
		if (traceflag) printf("creation %lu modification %lu\n",
								creationdate, modificationdate);
/*		need to setup up actime and modtime */
/*		timebuf.actime = creationdate; */
		timebuf.actime = statbuf.st_atime; /* mtime */
/*		timebuf.modtime = modificationdate; */
		timebuf.modtime = statbuf.st_mtime;
/*		need to setup timebuf before we get here */
		(void) domodification(outfile);

/*		printf("firstarg %d, l %d, argc %d\n", firstarg, l, argc); */
	} /* end of for loop over files */
	
	if ((argc - firstarg) > 1) printf("Processed %d Mac files\n", 
		(argc - firstarg));							/* 1993/Nov/20 */

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

/* convertreturn command line option ??? */
