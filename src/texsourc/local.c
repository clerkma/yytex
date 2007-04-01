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

#ifdef _WINDOWS
#define NOCOMM
#define NOSOUND
#define NODRIVERS
#define STRICT
#pragma warning(disable:4115)	// kill rpcasync.h complaint
#include <windows.h>
#endif

#ifdef _WINDOWS
// We must define MYLIBAPI as __declspec(dllexport) before including
// texwin.h, then texwin.h will see that we have already
// defined MYLIBAPI and will not (re)define it as __declspec(dllimport)
#define MYLIBAPI __declspec(dllexport)
// #include "texwin.h"
#endif

#include "texwin.h"

#pragma warning(disable:4131)	// old style declarator
#pragma warning(disable:4135)	// conversion between different integral types 
#pragma warning(disable:4127)	// conditional expression is constant

#include <setjmp.h>

#pragma hdrstop

#define EXTERN extern

#include "texd.h"

/* Most Y&Y changes are localized here -- init() */

/* Note int main (int ac, char *av[]) is in texmf.c */
/* and that calls main_program = texbody in itex.c => initialize */
/* which in turn calls init here in local.c */
/* which then calls initcommands here in local.c */ 

#define USEOUREALLOC			/* 96/Jan/20 */

#define USEMEMSET				/* 98/Jan/23 */

/* #define PREALLOCHOLE */		/* make hole in heap at bottom debugging */

/* #define CHECKALIGNMENT */	/* reactivated 95/Jan/7 */

/* #define HEAPSHOW */			/* debugging 96/Jan/20 */

/* #ifdef TRYANDOPEN */			/* debugging only */

/* #define SHOWHEAPERROR */		/* debugging 96/Jan/10 */

#define ALLOWDEMO	  			/* demo model - limited lifetime - DANGER ! */

#ifndef _WINDOWS
/* #define HEAPWALK */			/* debugging 96/Oct/22 */
#endif

#ifdef USEOUREALLOC
#define REALLOC ourrealloc
#else
#define REALLOC realloc
#endif

// #include <stdio.h>
#include <time.h>						// needed for clock_t etc.
/* #include <string.h> */				/* for islower ? */
// #include <process.h>
#include <malloc.h>						/* _msize, _expand, HEAPOK et.c */
// #include <share.h>					/* SH_DENYNO */
#include <direct.h>						/* for _getcwd() */

#pragma warning(disable:4032)	// different type when promoted
#ifndef _WINDOWS
#include <conio.h>						/* for _getch() */
#endif
#pragma warning(default:4032)	// different type when promoted`

// #include "getopt.h"

// #include "texd.h"

/* Argument handling, etc.  */ /* from common.h - setup `main' in texmf.c */
/* extern int gargc; */
/* extern char **gargv; */

/* In the new way, DVISETUP modifies the EXE file directly */
/* newmagic is the old hexmagic string with 8 preliminary bytes added */
/* four are the author signature, four are the serial number * REEXEC */

long serialnumber=0;
/* char newmagic[65]= */
char newmagic[97]=				/* coordinate with texsetup.c */
"bkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkphbkph";
#define hexmagic (newmagic+8)	/* start of encrypted owner etc */

#define CRYPT_MUL 52845u	/* pseudo-random number generator multiplier */
#define CRYPT_ADD 22719u	/* pseudo-random number generator addend */

#define REEXEC 55665			/* seed constant for eexec encryption */
/* #define RCHARSTRING 4330 */ 	/* seed constant for charstring encryption */

int wantcopyrght=1;

/* char *version = "Y&YTeX v0.9.9 compiled " __DATE__ " " __TIME__; */

/* char *version = "Y&YTeX v0.9.9 compiled " __DATE__ " " __TIME__
".  Copyright (C) 1993 Y&Y, Inc."; */

/* char *version = "Y&YTeX v1.0.2 compiled " __DATE__ " " __TIME__
".  Copyright (C) 1993 Y&Y, Inc."; */

/* appears in reverse order in EXE file */

char *compiletime =  __TIME__ ;
char *compiledate =  __DATE__ ;
char *www = "http://www.YandY.com";					/* not used ? */
char *phone = "(978) 371-3286";						/* not used ? */
char *rights = "All Rights Reserved.";				/* not used ? */
char *copyright="Copyright (C) 1993--2000 Y&Y, Inc."; 
/* char *yandyversion = "2.0.0"; */			/* 96/Sep/10 */
/* char *yandyversion = "2.0.1"; */			/* 96/Oct/12 */
/* char *yandyversion = "2.0.1 (32)"; */	/* 96/Oct/28 */
/* char *yandyversion = "2.0.2 (32)"; */	/* 96/Nov/16 */
/* Fix in long literal \special \ETC TeX bug due to string space near full: */
/* char *yandyversion = "2.0.3 (32)"; */	/* 97/Mar/7 */
/* Introduced encoding specific TEXFONTS env variable: */
/* char *yandyversion = "2.0.4 (32)"; */	/* 97/Apr/2 */
/* char *yandyversion = "2.0.5 (32)"; */	/* 97/May/25 */
/* char *yandyversion = "2.0.6 (32)"; */	/* 97/June/7 */
/* char *yandyversion = "2.0.7 (32)"; */	/* 97/July/31 */
/* char *yandyversion = "2.0.8 (32)"; */	/* 97/Oct/27 */
/* char *yandyversion = "2.0.9 (32)"; */	/* fonts used in log 97/Dec/24 */
/* char *yandyversion = "2.0.10 (32)"; */	/* fix triesize 98/Jan/5 */
/* char *yandyversion = "2.0.11 (32)"; */	/* fix foo.bar.tex 98/Feb/7 */
/* char *yandyversion = "2.0.12 (32)"; */	/* fix foo.bar.tex 98/Feb/7 */
/* char *yandyversion = "2.0.13 (32)"; */	/* -9 98/Apr/4 */
/* char *yandyversion = "2.0.14 (32)"; */	/* 98/May/28 */
/* char *yandyversion = "2.1"; */			/* 98/Jun/10 */
/* char *yandyversion = "2.1.1"; */			/* 98/Sep/12 */
/* char *yandyversion = "2.1.2"; */			/* 98/Dec/8 */
/* char *yandyversion = "2.1.3"; */			/* 99/Jan/5 f-lig suppression */
/* char *yandyversion = "2.1.4"; */			/* 99/Jan/7 allocate save stack */
/* char *yandyversion = "2.1.5"; */			/* 99/Jan/22 allocate buffer & stacks */
/* char *yandyversion = "2.1.6"; */			/* 99/Oct/23 getting ready for DLL */
/* char *yandyversion = "2.2.0"; */			/* 99/Dec/26 */
/* char *yandyversion = "2.2.1"; */			/* 00/Feb/22 */
/* char *yandyversion = "2.2.2"; */			/* 00/Apr/08 */
char *yandyversion = "2.2.3";				/* 00/Jun/18 */

char *application = "Y&Y TeX";				/* 96/Jan/17 */

/* char *texversion = "This is TeX, C Version 3.141"; *//* change with upgrade */
char *texversion = "This is TeX, Version 3.14159";	/* change with upgrade */

/* #define COPYHASH 1890382 */
/* #define COPYHASH 13862905 */
/* #define COPYHASH 10558802 */
/* #define COPYHASH 7254699 */
/* #define COPYHASH 3950596 */
/* #define COPYHASH 646493 */
#define COPYHASH 12905299

clock_t starttime, maintime, finishtime;

char *dvidirectory="";	/* user specified directory for dvi file */
char *logdirectory="";	/* user specified directory for log file */
char *auxdirectory="";	/* user specified directory for aux file */

char *texpath="";		/* path to executable - used if env vars not set */

// #define MAXLINE 256

char logline[MAXLINE];	// used also in tex9.c

int memspecflag=0;		/* non-zero if `-m=...' was used */ 
int formatspec=0;		/* non-zero if a format specified on command line */

int closedalready=0;	// make sure we don't try this more than once

booleane reorderargflag = true;	/* put command line flags/arguments first */

#ifdef ALLOWDEMO
booleane bDemoFlag=0;				/* non-zero => DEMO version */
									/* as determined by user string DEMO */
time_t dtime=0;						/* seconds since customized, now global */
#define oneday (86400)				/* one day in seconds */
#define onemonth (86400 * 31)		/* one month in seconds */
#endif

/* Mapping from Windows ANSI to DOS code page 850 96/Jan/20 */
/* Used in tex0.c with wintodos[c-128] */

unsigned char wintodos[128] = {
  0,   0,   0, 159,   0,   0,   0,   0, 
 94,   0,   0,   0,   0,   0,   0,   0, 
  0,  96,  39,   0,   0,   7,   0,   0,
126,   0,   0,   0,   0,   0,   0,   0, 
 32, 173, 189, 156, 207, 190,  221, 21, 
  0, 184, 166, 174, 170,  45, 169,   0, 
248, 241, 253, 252,   0, 230,  20, 250, 
  0, 251, 167, 175, 172, 171, 243, 168, 
183, 181, 182, 199, 142, 143, 146, 128, 
212, 144, 210, 211, 222, 214, 215, 216, 
209, 165, 227, 224, 226, 229, 153, 158, 
157, 235, 233, 234, 154, 237, 232, 225, 
133, 160, 131, 198, 132, 134, 145, 135, 
138, 130, 136, 137, 141, 161, 140, 139, 
208, 164, 149, 162, 147, 228, 148, 246,
155, 151, 163, 150, 129, 236, 231, 152
};	

void showusage (char *program) {
	char *s=logline;
	sprintf (s, "\n\
%s [-?ivnwdrzpK] [-m=ini_mem] [-e=hyph_size] [-h=trie_size]\n\
\t[-x=xchr_file] [-k=key_file] [-o=dvi_dir] [-l=log_dir] [-a=aux_dir]\n\
\t\t[&format_file] [tex_file]\n\
", program);
	s += strlen(s);
	sprintf (s, "\
    -?    show this usage summary\n\
    -i    start up as iniTeX (create format file)\n\
    -v    be verbose (show implementation version number)\n\
    -n    do not allow `non ASCII' characters in input files (complain instead)\n\
    -w    do not show `non ASCII' characters in hexadecimal (show as is)\n\
    -d    do not allow DOS style file names - i.e. do not convert \\ to /\n\
    -r    do not allow Mac style termination - i.e. do not convert \\r to \\n\n\
    -p    allow use of \\patterns after loading format (iniTeX only)\n\
    -K    disable all extensions to basic TeX\n\
    -m    initial main memory size in kilo words (iniTeX only)\n\
    -e    hyphenation exception dictionary size (iniTeX only)\n\
    -h    hyphenation pattern trie size (iniTeX only)\n\
    -x    use `non ASCII' character mapping (xchr[]) defined in file\n\
    -k    use `key replacement' defined in file\n\
    -o    write DVI file in specified directory (default current directory)\n\
    -l    write LOG file in specified directory (default current directory)\n\
    -a    write AUX file in specified directory (default current directory)\n\
");
	strcat(s, "\n");
	showline(s, 1);
#ifndef _WINDOWS
	uexit(1);			// has this been setup yet ???
#endif
}

/* -z    do not discard control-Z at end of input file (treat as character)\n\ */

/* -c    prepend current directory (.) to TFM directory list\n\ */
/* -b    check that files with read access are not actually directories\n\ */

/* \t-d\tallow DOS style file names - i.e. convert \\ to / \n\ */
/* \t\t(applies to file name and format file name, if present)\n\ */
/* \t-r\tallow Mac style line termination - i.e. convert \\r to \\n \n\ */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* stuff for encrypting and decrypting */

unsigned char decryptbyte (unsigned char cipher, unsigned short *crypter) {
	unsigned char plain;
/*	plain = (cipher ^  (unsigned char) (*crypter >> 8)); */
	plain = (unsigned char) ((cipher ^  (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return plain;
} 

/*
unsigned char encryptbyte (unsigned char plain, unsigned short *crypter) {
	unsigned char cipher;
	cipher = (plain ^ (unsigned char) (*crypter >> 8));
	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD;
	return cipher;
} */

/* Sep 27 1990 => 1990 Sep 27 */

void scivilize (char *date) {
	int k;
	char year[6];

/*	if (strlen(date + 7) > 4) {
		sprintf(logline, date);
		showline(logline, 1);
		return;
	} */
	strcpy (year, date + 7);
	for (k = 5; k >= 0; k--) date[k+5] = date[k];
/*	date[11] = '\0'; */
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

/*	if (strlen(date + 20) > 4) {
		sprintf(logline, date);
		showline(logline, 1);
		return;
	} */
	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
/*	date[20] = '\n'; */
/*	date[21] = '\0'; */
	date[20] = '\0'; 
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

// void stampit (FILE *outfile)
// now writes result into given buffer
void stampit (char *s) {
	char date[11 + 1];

	strcpy(date, compiledate);
	scivilize(date);	 
	sprintf(s, "%s %s ", application, yandyversion);
	s += strlen(s);
#ifdef ALLOWDEMO
	if (bDemoFlag) {
		strcat(s, "DEMO ");
		s += strlen(s);
	}
#endif
	sprintf(s, "%s %s SN %d",
		date, compiletime, serialnumber/REEXEC);
	s += strlen(s);
//	if (wantcopyrght) {								/* 96/Nov/3 */
//		strcat(s, "\n");
//		s += strlen(s);
//		sprintf(s, "%s  %s  %s", copyright, phone, www); /* 98/Feb/14 */
//	}
//	no \n at end 
}

void stampcopy (char *s) {
	if (wantcopyrght) {								/* 96/Nov/3 */
		sprintf(s, "%s  %s  %s", copyright, phone, www); /* 98/Feb/14 */
//		sprintf(s, "%s  %s", copyright, www);		/* 99/Oct/25 */
	}
}

// put owner information in buffer

int showowner (char *buff, char *sour, int nlen) {
	unsigned short cryptma = REEXEC; /* int */
	unsigned char e;
	int i;
	char *s = sour, *t = buff;

	if (serialnumber == 0) {		/* not customized ... */
		showline("Invalid serial number\n", 1);
		*buff = '\0';
		return -1;
	}
//	if (serialnumber == 0) exit(0);		/* ... */

	for (i = 0; i < 4; i++)				/* prime the pump - discard key */
		e = decryptbyte((unsigned char) *s++, &cryptma);
	for (i = 4; i < nlen; i++) {
		e = decryptbyte((unsigned char) *s++, &cryptma);
		*t++ = (char) e;				/* unsigned char => char ? */
		if (e == 0) break;				/* null byte termination */
	}
	*t = '\0';					/* terminate it */

/*	This eventually won't be used anymore \xyz */
	t = buff;					/* postprocess for accented characters */
	i = 0;
	while ((e = *t) != '\0') {
		if (e == '\\') {			/* look for \xyz string */
/*			winerror(t); */
			i = 0; s = t; t++;
			while ((e = *t) >= '0' && e <= '9') {
				i = (i * 10) + (e - '0'); 
				t++;
			}
			if (i != 0) *s++ = (char) i;
			strcpy(s, t);
			t = s;
			if (e == 0) break;
			continue;				/* 92/Jan/3 */
		}
		t++;
	}
	return 0;
}

// int showownerout(FILE *output) {
int showownerout (char *s) {
	char *t;

	*s = '\0';							/* in case we skip out */
	if (serialnumber == 0) {
		showline("Invalid serial number\n", 1);
		return -1;
	}
//	stampit(output);						/* 93/Sep/28 */
//	stampit(s);
//	strcat(s, "\n");
//	s += strlen(s);
	t = s;				// remember start of this part
#ifdef ALLOCATEBUFFER
	showowner(s, hexmagic, currentbufsize);
#else
	showowner(s, hexmagic, sizeof(buffer));
#endif
	while ((t = strchr(t+1, '@')) != NULL) *t = ' ';
	t = (char *) (buffer + strlen((char *) buffer) - 1);
	if (*t < ' ') *t = '\0';	/* flush trailing newline */
	return 0;
}

/* check whether encrypted owner has been tampered with */
/* also check whether DEMO version */

int checkowner(char *hex, char *buffer, int nlen) { 
	unsigned short cryptma = REEXEC; /* int */
	unsigned char e=0;
	int i, k;
	char *s=hex;
	char *t=buffer;		/* should be enough space there */

/*	check first whether it is pre-customized version of executable */
	i = 0;						/* first check on pre-release version */
	for (k = 4; k < 32; k++) {	/* assumes pattern wavelength 4 */
		if (*(s+k) != *(s+k-4)) { 
			i = 1;
			break;
		}
	}
	if (i == 0) {				/* uncustomized */
		showline("SORRY: NOT CUSTOMIZED!\n", 1);
		if (getenv("CARLISLE") == NULL &&
			getenv("CONCORD") == NULL &&
			getenv("CAMBRIDGE") == NULL &&
			getenv("CONWAY") == NULL) return -1;
		return 0;
	}

/*	modified 97/May/23 to allow DOS 850 accented characters, */
/*	but also now disallows control characters, and checks signature */
	for (i = 0; i < 4; i++) {
		e = decryptbyte((unsigned char) *s++, &cryptma);
/*		if (e < 32 || e > 126) {  */
		if (e < 'a' || e > 'z') {		/* should be all lower case */
			return -1;					/* tampered with signature */
		}
	}
	for (i = 4; i < nlen; i++) {
		e = decryptbyte((unsigned char) *s++, &cryptma);
		*t++ = e;								/* assemble line */
		if (e == 0) break;
		if (e < 32) {
			if (e != 10 && e != 13 && e != 9) break;	/* tampered with ! */
		}
		else if (e > 127) {
            if (e == 156 || e == 158 || e == 159) break;
            if (e > 165 && e < 181) break;
            if (e > 183 && e < 198) break;
            if (e > 199 && e < 208) break;
            if (e > 216 && e < 222) break;
            if (e == 223 || e == 230) break;
            if (e > 237) break;
/*			could instead reject if dos850topdf[c-128] == 0 */
		}
	}
	if (e != 0) *t++ = '\0';
	if (e != 0) return -1;
	if (strchr(buffer, '@') == NULL) return -1;
#ifdef ALLOWDEMO
/*	if (strstr(buffer, "DEMO") != NULL) bDemoFlag = 1; */
/*	else bDemoFlag = 0; */
#endif
	if ((s = strstr(buffer, "DEMO")) != NULL) {	/* 98/May/20 */
		if (sscanf(s+4, "%d", &bDemoFlag) == 0)	
			bDemoFlag = 1;
	}
	return 0;	/* seems ok ! */
}

unsigned long checkcopyright (char *s) {
	int c;
	unsigned long hashed=0;

/*	Step over the `Y&YTeX v... compiled  ... ...' part that is variable */
/*	if ((s = strstr(s, "compiled")) == NULL) return 1; */
/*	if ((s = strchr(s, '.')) == NULL) return 1; */

	while ((c = *s++) != '\0')
		hashed = (hashed * 53 + c) & 16777215;	/* 2^24 - 1 */
	if (hashed == COPYHASH) return 0; /*  change if copyright changed */
	showchar('\n');
	showline("EXE FILE CORRUPTED ", 1);
	if (traceflag) {
		sprintf(logline, "%d", hashed);
		showline(logline, 0);
	}
	showchar('\n');
	return hashed;
}

/* stuff for DEMO version */

#ifdef ALLOWDEMO
char *months="JanFebMarAprMayJunJulAugSepOctNovDec";	/* 94/June/8 */

int monthnumber(char *smonth) {
	int k;
	char *s=months;
	for (k = 0; k < 12; k++) {
		if (strncmp(smonth, s, 3) == 0) return k;
		s += 3;
	}
	return 0;			/* Say what? */
}

void stripcolon(char *s) {
	while ((s = strchr(s+1, ':')) != NULL) *s = ' ';
}

/* Owner is of form "Berthold K.P. Horn@100@1998 May 23 07:43:48\n" */

time_t checkdemo(char *owner) {		/* now returns seconds since customized */
	time_t ltime, otime;			/* for date and time */
	time_t dtime;					/* seconds since customized */
	struct tm loctim;
	int year, month, day;
	int hour, min, sec;
	char buffer[64];
	char *s;

/*	first calculate compilation time */		/* not used anymore */
/*	sscanf(compiledate, "%s %d %d", buffer, &day, &year); */
	s = owner;							/* use customization time instead */
	if (*s < ' ') return 0;				/* uncustomized */
/*	check that there are two occurences of @ - and step to date part */
	if ((s = strchr(s+1, '@')) == NULL) return -1;
	if ((s = strchr(s+1, '@')) == NULL) return -1;
	stripcolon(s+1);
	if (sscanf(s+1, "%d %s %d %d %d %d",
			  &year, buffer, &day, &hour, &min, &sec) < 6) {
		return -1;
	}
	if (year > 1900) year = year - 1900;
	month = monthnumber(buffer);
	loctim.tm_year = year;
	loctim.tm_mon = month; 
	loctim.tm_mday = day;
/*	stripcolon(compiletime); */	/* extra fancy precision */
/*	sscanf(compiletime, "%d %d %d", 
		&loctim.tm_hour, &loctim.tm_min, &loctim.tm_sec); */
	loctim.tm_hour = hour;
	loctim.tm_min = min;
	loctim.tm_sec = sec;
	loctim.tm_isdst = -1;	/* daylight saving time flag - info not avail */
	otime = mktime(&loctim);
/*	Note: mktime returns -1 for invalid input data */
/*	This might be off by one hour (3600 sec) because of daylight savings */
	
	(void) time(&ltime);		/* get seconds since 1970 */
/*	Note: time() returns -1 if it can't get the time */
	dtime = ltime - otime;		/* time difference in sec so far */
/*	debugging, remove later */
/*	sprintf(logline, "dtime %ld = ltime %ld - otime %ld (%lg months)\n",
		   dtime, ltime, otime, (double) dtime / (double) onemonth); */
	if (dtime > onemonth * 12)
		for (;;);				/* SERIOUSLY EXPIRED ! */
	if (dtime < - oneday) {
		exit(7);				/* bogus date ... KILL! */
	}

/*	if (dtime > onemonth) {	*/
	if (dtime > onemonth * bDemoFlag) {		/* 98/May/20 */
		showline("Please contact Y&Y, Inc. for non-DEMO version\n", 1);
#ifdef _WINDOWS
//		need to do something here ?
#else
		putc(7, stdout);		// Ctrol-G
		fflush(stdout);			/* 97/Nov/28 */
		(void) _getch();		/* 97/May/24 */
#endif
	}
/*	if (dtime > onemonth * 3) { */
	if (dtime > onemonth * (bDemoFlag+2)) {
		showline("Sorry, but this DEMO version has expired\n", 1);
		exit(7);				/* EXPIRED ... KILL! */
	}
	return dtime;
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXCHRS 256
#define NOTDEF 127

void readxchrsub (FILE *input) {
	char buffer[PATH_MAX];
	int k, from, to, count = 0;
	char *s;

#ifdef USEMEMSET
	memset (xchr, NOTDEF, MAXCHRS);						/* mark unused */
#else
	for (k = 0; k < MAXCHRS; k++) xchr [ k ] = -1;	*/	/* mark unused */
#endif
#ifdef USEMEMSET
	memset (xord, NOTDEF, MAXCHRS);						/* mark unused */
#else
	for (k = 0; k < MAXCHRS; k++) xord [ k ] = -1;	*/	/* mark unused */
#endif

#ifdef ALLOCATEBUFFER
	while (fgets(buffer, currentbufsize, input) != NULL) 
#else
	while (fgets(buffer, sizeof(buffer), input) != NULL)
#endif
	{
		if (*buffer == '%' || *buffer == ';' || *buffer == '\n') continue;
/*		if (sscanf (buffer, "%d %d", &from, &to) < 2)
			sprintf(logline, "Do not understand: %s", buffer); */
		from = (int) strtol (buffer, &s, 0);
		to = (int) strtol (s, NULL, 0);
/*		what if line is bad ? do we just get from = 0 and to = 0 ? */
		if (from >= 0 && from < MAXCHRS && to >= 0 && to < MAXCHRS) {
			if (xchr [ from ] == (unsigned char) NOTDEF)
				xchr [ from ] = (unsigned char) to;
			else {
				sprintf(logline, "NOTE: %s collision: %d => %d, %d\n",
						"xchr", from, xchr [ from ], to);
				showline(logline, 0);
			}
			if (xord [ to ] == NOTDEF)
				xord [ to ] = (unsigned char) from;
			else {
				sprintf(logline, "NOTE: %s collision: %d => %d, %d\n",
						"xord", to, xord [ to ], from);
				showline(logline, 0);
			}
			count++;
		}
	}
/*	now fill in the gaps */	/* not clear this is a good idea ... */
	for (k = 0; k < MAXCHRS; k++) {
		if (xchr [ k ] == NOTDEF) {		/* if it has not been filled */
			if (xord [ k ] == NOTDEF) {	/* see whether used already */
				xchr [ k ] = (unsigned char) k;	/* no, so make identity */
				xord [ k ] = (unsigned char) k;	/* no, so make identity */
			}
		}
	}
	xchr [ NOTDEF ] = NOTDEF;					/* fixed point of mapping */
	if (traceflag) {
		sprintf(logline, "Read %d xchr[] pairs:\n", count);
		showline(logline, 0);
		for (k = 0; k < MAXCHRS; k++) {
			if (xchr [ k ] != NOTDEF) {
				sprintf(logline, "%d => %d\n", k, xchr [ k ]);
				showline(logline, 0);
			}
		}
	}
}

char *replacement[MAXCHRS];			/* pointers to replacement strings */

void readreplsub (FILE *input) {
	int k, n, m, chrs;
	char buffer[PATH_MAX];
	char charname[128];
	int charnum[10];
	char *s, *t;
	
#ifdef USEMEMSET
	memset(replacement, 0, MAXCHRS * sizeof(replacement[ 0 ]));
#else
	for (k = 0; k < MAXCHRS; k++) replacement[k] = NULL; 
#endif

	while (fgets(buffer, PATH_MAX, input) != NULL) {
		if (*buffer == '%' || *buffer == ';' || *buffer == '\n') continue;
		if ((m = sscanf (buffer, "%d%n %s", &chrs, &n, &charname)) == 0)
			continue; 
		else if (m == 2) {
			if (*charname == '"') {		/* deal with quoted string "..." */
				s = buffer + n;
				t = charname;
				while (*s != '"' && *s != '\0') s++;	/* step up to " */
				if (*s++ == '\0') continue;				/* sanity check */
				while (*s != '\0') {	
					if (*s == '"') {
						s++;						/* is it "" perhaps ? */
						if (*s != '"') break;		/* no, end of string */
					}
					*t++ = *s++;					/* copy over */
				}
				*t = '\0';							/* and terminate */
			}
			if (chrs >= 0 && chrs < MAXCHRS)
				replacement[chrs] = xstrdup(charname);
		}
/*		presently the following can never get triggered */
/*		which is good, because it is perhaps not right ... */
		else if ((m = sscanf (buffer, "%d %d %d %d %d %d %d %d %d %d %d",
			&chrs, charnum, charnum+1, charnum+2, charnum+3, charnum+4,
				charnum+5, charnum+6, charnum+7, charnum+8, charnum+9)) > 1) {
/*			for (k = 0; k < n-1; k++) charname[k] = (char) charnum; */
			for (k = 0; k < n-1; k++) charname[k] = (char) charnum[k];
			charname[m] = '\0';
			if (chrs >= 0 && chrs < MAXCHRS)
				replacement[chrs] = xstrdup(charname);			
		}
		else {
			sprintf(logline, "ERROR: don't understand %s", buffer);
			showline(logline, 1);
		}
	}
	if (traceflag) {									/* debugging output */
		showline("Key replacement table\n", 0);
		for (k = 0; k < MAXCHRS; k++) {
			if (replacement[k] != NULL) {
				sprintf(logline, "%d\t%s\n", k, replacement[k]);
				showline(logline, 0);
			}
		}
	}
}

/* Following used both to read xchr[] file and key replacement file */
/* the flag is 0 for -x=... and the flag is 1 for -k=... */

int readxchrfile (char *filename, int flag, char *argv[]) {
	FILE *input;
	char infile[PATH_MAX];
	char *s;

	if (filename == NULL) return -1;
	if (traceflag) {
		sprintf(logline, "Reading xchr/repl %s\n", filename);
		showline(logline, 0);
	}

/*	first try using file as specified */
	strcpy(infile, filename);
	if (traceflag) {
		sprintf(logline, "Trying %s\n", infile);
		showline(logline, 0);
	}
	if (shareflag == 0) input = fopen (infile, "r");
	else input = _fsopen (infile, "r", shareflag);		/* 94/July/12 */
	if (input == NULL) {
		if (strrchr(infile, '.') == NULL) {
			if (flag == 0) strcat(infile, ".map");
			else strcat(infile, ".key");
			if (traceflag) {
				sprintf(logline, "Trying %s\n", infile);
				showline(logline, 0);
			}
			if (shareflag == 0) input = fopen (infile, "r");
			else input = _fsopen (infile, "r", shareflag);	/* 94/July/12 */
		}
	}
	if (input == NULL) {
/*		strcpy (infile, gargv[0]); */		/* try TeX program path */
		strcpy (infile, argv[0]);			/* try TeX program path */
		if ((s = strrchr (infile, '\\')) != NULL) *(s+1) = '\0';
		else if ((s = strrchr (infile, '/')) != NULL) *(s+1) = '\0';
		else if ((s = strrchr (infile, ':')) != NULL) *(s+1) = '\0';
		strcat (infile, filename);
		if (traceflag) {
			sprintf(logline, "Trying %s\n", infile);
			showline(logline, 0);
		}
		if (shareflag == 0) input = fopen (infile, "r");
		else input = _fsopen (infile, "r", shareflag);		/* 94/July/12 */
		if (input == NULL) {
			if (strchr(infile, '.') == NULL) {
				if (flag == 0) strcat(infile, ".map");
				else strcat(infile, ".key");
				if (traceflag) {
					sprintf(logline, "Trying %s\n", infile);
					showline(logline, 0);
				}
				if (shareflag == 0) input = fopen (infile, "r");
				else input = _fsopen (infile, "r", shareflag); /* 94/July/12 */
			}
		}
	}
	if (input == NULL) {					/* 97/July/31 */
/*		strcpy (infile, gargv[0]); */		/* try TeX program path\keyboard */
		strcpy (infile, argv[0]);			/* try TeX program path */
		if ((s = strrchr (infile, '\\')) != NULL) *(s+1) = '\0';
		else if ((s = strrchr (infile, '/')) != NULL) *(s+1) = '\0';
		else if ((s = strrchr (infile, ':')) != NULL) *(s+1) = '\0';
		strcat (infile, "keyboard\\");
		strcat (infile, filename);
		if (traceflag) {
			sprintf(logline, "Trying %s\n", infile);
			showline(logline, 0);
		}
		if (shareflag == 0) input = fopen (infile, "r");
		else input = _fsopen (infile, "r", shareflag);
		if (input == NULL) {
			if (strchr(infile, '.') == NULL) {
				if (flag == 0) strcat(infile, ".map");
				else strcat(infile, ".key");
				if (traceflag) {
					sprintf(logline, "Trying %s\n", infile);
					showline(logline, 0);
				}
				if (shareflag == 0) input = fopen (infile, "r");
				else input = _fsopen (infile, "r", shareflag);
			}
		}
	}
/*	Note: can't look in TeX source file dir, since that is not known yet */
	if (input == NULL) {
		sprintf(logline, "ERROR: Sorry, cannot find %s file %s",
				flag ? " xchr[]" : "key mapping", filename);
		showline(logline, 1);
		perrormod (filename);
		return 0;					// failed
	}

	if (flag == 0) readxchrsub (input);
	else readreplsub (input);

	(void) fclose (input);
	return 1;
}

/* need to also set `keyreplace' here based on command line */
/* need to also allocate `buffercopy' here and free at end */
/* need to call `readreplace' in appropriate place */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following may be useful if link without floating point emulation */

#ifdef DEBUG
void testfloating (void) {
/*	double x = 1.0; */
/*	double dx = DBL_EPSILON; */
	double dx = 1.0;
	double dxold = 0.0;
	int k = 0;
/*	while (x + dx != 1.0) { */
	while (1.0 + dx != 1.0) {
		dxold = dx;
		dx = dx / 2.0;
		k++;
	}
	sprintf(logline, "Floating test: dx = %lg (k = %d)\n", dxold, k - 1);
	showline(logline, 0);
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *debugfile;			/* NULL or name of file to try and open */

#ifdef SHOWHEAPERROR
char *heapstrings[] = {
"", "Empty", "OK", "Bad Begin", "Bad Node", "End", "Bad Pointer"
};
#endif

/* Attempt to get at problem with eqtb ... temporarily abandoned */

#ifdef CHECKEQTB
void checkeqtb (char *act) {
	int k, count=0;
	memoryword *eqtb = zeqtb;
/*	for (k = 10280 + hash_extra; k < 10280 + eqtb_extra; k++) { */
	for (k = hash_size + 780 + hash_extra; k < hash_size + 780 + eqtb_extra; k++) {
		if (eqtb [ k ] .cint != 0) {
			if (count == 0) {
				showchar('\n');
				showline("EQTB ", 0);
			}
			sprintf(logline, "%d ", k);
			showline(logline, 0);
			if (count++ > 256) break;
		}
	}
	if (count != 0) showchar('\n');
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXSPLITS 3

/* ad hoc default minimum growth in memory realloc is 62% */
/* golden ratio (1 + \sqrt{5}) / 2 = 1.618033989... */
int percentgrow=62;		/* default minimum growth in memory realloc is 62% */

int totalallocated=0;	/* total memory allocated so far */

int inimaxaddress=0;	/* maximum address when starting */
int maxaddress=0;		/* maximum address seen in allocated memory */

/* see texd.h */

// DON'T USE THIS in DLL VERSION

#ifndef _WINDOWS
#ifdef HEAPWALK
unsigned int heapthreshold=0; 	/* smallest size block interested in ... */

unsigned int heapdump (FILE *output, int verbose) {
	unsigned int total=0;
	struct _heapinfo hinfo;
	int heapstatus;
	int end_block=0;
	int n;

	if (verbose) fprintf(output, "HEAP DUMP:\n");

/*	if ((n = _heapchk ()) != _HEAPOK) {	*/
	n = _HEAPOK;
#ifdef SHOWHEAPERROR
	n = _heapchk ();
#endif
	if (n != _HEAPOK) {	
		fprintf(stderr, "WARNING: Heap corrupted (%d)\n", n);
#ifdef SHOWHEAPERROR
		fprintf(stderr, "HEAP %s (%s)\n", heapstrings[-n], "heapdump");
#endif
	}
	hinfo._pentry = NULL;
	while ((heapstatus = _heapwalk(&hinfo)) == _HEAPOK) {
		if (end_block > 0 && (int) hinfo._pentry > end_block + 1024) {
//			if (verbose) printf("GAP of %d bytes!\n", (int)	hinfo._pentry - end_block);
		}
		end_block = (int) hinfo._pentry + hinfo._size;
		if (hinfo._useflag == _USEDENTRY) total += hinfo._size;
		if (hinfo._size >= heapthreshold && verbose)
		fprintf(output, "%6s block at %p (%7d) of size %6X (%7d) => (%7d)\n",
			(hinfo._useflag == _USEDENTRY ? "USED" : "...."),
				hinfo._pentry, hinfo._pentry, hinfo._size, hinfo._size,
					end_block);
	}
	switch (heapstatus) {
		case _HEAPEMPTY:
			if (verbose) fprintf(output, "OK - empty heap\n");
			break;
		case _HEAPEND:
			if (verbose) fprintf(output, "OK - end of heap (%u bytes used)\n", total);
			break;
		case _HEAPBADPTR:
			fprintf(output, "ERROR - %s\n", "bad pointer to heap");
			break;
		case _HEAPBADBEGIN:
			fprintf(output, "ERROR - %s\n", "bad start of heap");
			break;
		case _HEAPBADNODE:
			fprintf(output, "ERROR - %s\n", "bad node in heap");
			break;	
	}
	return total;
}
#endif
#endif

void showmaximums (FILE *output) {
#ifdef HEAPWALK
	unsigned heaptotal=0;						/* no longer used */
	heaptotal = heapdump(stdout, 0);			/* 94/Apr/3 */
#endif
	sprintf(logline,
		"Max allocated %d --- max address %d\n", 
			totalallocated, maxaddress); 
//	if (output != NULL) fputs(logline, output); // log file
//	else if (flag == 0) showline(logline, 0);	// informative
//	else if (flag == 1) showline(logline, 1);	// error
	if (output == stderr) showline(logline, 1);
	else if (output == stdout) showline(logline, 0);
	else fputs(logline, output);
}

/* our own version of realloc --- avoid supposed MicroSoft version bug */
/* also tries _expand first, which can avoid address growth ... */

#ifdef USEOUREALLOC 
void *ourrealloc (void *old, size_t new_size) {
	void *new;
	size_t old_size, overlap;

/*	round up to nearest multiple of four bytes *//* avoid unlikely alignment */
	if ((new_size % 4) != 0) new_size = ((new_size / 4) + 1) * 4;

	if (old == NULL) return malloc (new_size);	/* no old block - use malloc */

	old_size = _msize (old);
	if (old_size >= new_size && old_size < new_size + 4) return old;
/*	_heapmin(); */	/* release unused heap space to the system - no op ? */
#ifdef HEAPSHOW
	if (traceflag) {
		showline("BEFORE REALLOC: \n", 0);
#ifdef HEAPWALK
		(void) heapdump(stdout, 1);  		/* debugging 96/Jan/18 */
#endif
	}
#endif
	new = _expand (old, new_size);			/* first try and expand in place */
	if (new != NULL) {
		if (traceflag) {
			sprintf(logline, "EXPANDED! %d (%d) == %d (%d)\n",
				new, new_size, old, old_size);
			showline(logline, 0);
		}
		return new;
	}
/*  *********************************************************************** */
/*	do this if you want to call the real realloc next -  */
	new = realloc (old, new_size);
#ifdef HEAPSHOW
	if (traceflag) {
		showline("AFTER REALLOC: \n", 0);
#ifdef HEAPWALK
		(void) heapdump(stdout, 1);  		/* debugging 96/Jan/18 */
#endif
	}
#endif
	if (new != NULL) return new;
/*	we are screwed typically if we ever drop through here - no more space */
/*  *********************************************************************** */
	new = malloc (new_size);					/* otherwise find new space */
	if (new == NULL) return new;				/* if unable to allocate */
	if (old_size < new_size) overlap = old_size;
	else overlap = new_size;
	memcpy (new, old, overlap);					/* copy old data to new area */
	free(old);									/* free the old area */
	return new;
}
#endif

void memoryerror (char *s, int n) {
	if (logopened) {
		fprintf(logfile,
			"\n! Unable to allocate %d bytes for %s\n", n, s);
		showmaximums(logfile);
#ifdef HEAPWALK
		if (heapflag) (void) heapdump(logfile, 1);
#endif
	}
	sprintf(logline, "\n! Unable to allocate %d bytes for %s\n", n, s);
	showline(logline, 1);
	showmaximums(stderr);
#ifdef HEAPWALK
	if (heapflag) (void) heapdump(stderr, 1);
#endif
/*	exit (1); */			/* 94/Jan/22 */
/*	return to let TeX do its thing (such as complain about runaway) */	
/*	don't set abortflag here */
}

void tracememory (char *s, int n) {
	sprintf(logline, "Allocating %d bytes for %s\n", n, s);
	showline(logline, 0);
}

void updatestatistics (int address, int size, int oldsize) {
	if (address + size > maxaddress) maxaddress = address + size;
	totalallocated =  totalallocated + size - oldsize;
}

void probememory (void) {
	char *s;
	s = (char *) malloc (4);				/* get current top address */
	free(s);
	updatestatistics ((int) s, 0, 0);		/* show where we are */
}

void probeshow (void) {
	probememory();
	showmaximums(stdout);
#ifdef HEAPWALK
	if (heapflag) (void) heapdump(stdout, 1);
#endif
}

size_t roundup (size_t n) {
	if ((n % 4) == 0) return n;
	else return ((n / 4) + 1) * 4;
}

#ifdef ALLOCATETRIES
/* using allocating hyphenation trie slows things down maybe 1% */
/* but saves typically (270k - 55k) = 215k of memory */

/* NOTE: it's safe to allocate based on the triemax read from fmt file */
/* since hyphenation trie cannot be extended (after iniTeX) */
/* for iniTeX, however, we need to allocate the full triesize ahead of time */

/* NOTE: we don't ever reallocate these */
/* returns -1 if it fails */

int allocatetries (int triemax) {
	int n, nl, no, nc;
/*	if (triemax > triesize) {
		sprintf(logline, "ERROR: invalid trie size (%d > %d)\n",
			triemax, triesize);
			showline(logline, 1);
		exit (1);
	} */ /* ??? removed 1993/dec/17 */
	if (triemax > 1000000) triemax = 1000000;	/* some sort of sanity limit */
/*	important + 1 because original was halfword trietrl[triesize + 1] etc. */
	nl = (triemax + 1) * sizeof(halfword);		/* trietrl[triesize + 1] */
	no = (triemax + 1) * sizeof(halfword);		/* trietro[triesize + 1] */
	nc = (triemax + 1) * sizeof(quarterword);	/* trietrc[triesize + 1] */
	n = nl + no + nc;
	if (traceflag) tracememory("hyphen trie", n);
	trietrl = (halfword *) malloc (roundup(nl));
	trietro = (halfword *) malloc (roundup(no));
	trietrc = (quarterword *) malloc (roundup(nc));
	if (trietrl == NULL || trietro == NULL || trietrc == NULL) {
		memoryerror("hyphen trie", n);
		return -1;
//		exit (1);							/* serious error */
	}
	if (traceflag) {
		sprintf(logline, "Addresses trietrl %d trietro %d trietrc %d\n", 
				trietrl, trietro, trietrc);
		showline(logline, 0);
	}
	updatestatistics ((int) trietrl, nl, 0);
	updatestatistics ((int) trietro, no, 0);
	updatestatistics ((int) trietrc, nc, 0);
/*	sprintf(logline, "triesize %d triemax %d\n", triesize, triemax); */ /* debug */
	triesize = triemax;						/* BUG FIX 98/Jan/5 */
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return 0;								// success
}
#endif

#ifdef ALLOCATEHYPHEN
booleane prime (int);				/* test function later in this file */

int currentprime=0;					/* remember in case reallocated later */

/* we don't return an address here, since TWO memory regions allocated */
/* plus, we don't really reallocate, we FLUSH the old information totally */
/* returns -1 if it fails */

int reallochyphen (int hyphen_prime) {
	int n, nw, nl;
	if (!prime(hyphen_prime)) {
		sprintf(logline, "ERROR: non-prime hyphen exception number (%d)\n",
			hyphen_prime); 
		showline(logline, 1);
//		exit (1);
		return -1;
	}
/*	need not/cannot preserve old contents when hyphen prime is changed */
/*	if (hyphlist != NULL) free(hyphlist); */
/*	if (hyphword != NULL) free(hyphword); */
/*	important + 1 since strnumber hyphword[hyphen_prime + 1]  in original etc. */
	nw = (hyphen_prime + 1) * sizeof(strnumber);
	nl = (hyphen_prime + 1) * sizeof(halfword);
	n = nw + nl;
	if (traceflag) tracememory("hyphen exception", n);
/*	initially hyphword will be NULL so this acts like malloc */
/*	hyphword = (strnumber *) malloc (nw); */
	hyphword = (strnumber *) REALLOC (hyphword, nw);	/* 94/Mar/24 */
/*	initially hyphlist will be NULL so this acts like malloc */
/*	hyphlist = (halfword *) malloc (nl); */
	hyphlist = (halfword *) REALLOC (hyphlist, nl);		/* 94/Mar/24 */
	if (hyphword == NULL || hyphlist == NULL) {
		memoryerror("hyphen exception", n);
//		exit (1);							/* serious error */
		return -1;
	}
	if (traceflag) {
		sprintf(logline, "Addresses hyphword %d hyphlist %d\n", 
			   hyphword, hyphlist);
		showline(logline, 0);
	}
/*	cannot preserve old contents when hyphen prime is changed */
#ifdef USEMEMSET
	memset(hyphword, 0, (hyphen_prime + 1) * sizeof (hyphword [ 0 ]));
#else
	for (k = 0; k <= hyphen_prime; k++) hyphword [ k ] = 0; 
#endif
#ifdef USEMEMSET
	memset(hyphlist, 0, (hyphen_prime + 1) * sizeof (hyphlist [ 0 ]));
#else
	for (k = 0; k <= hyphen_prime; k++) hyphlist [ k ] = 0; 
#endif
	hyphcount = 0 ; 	/* or use resethyphen() in itex.c */
	if (currentprime != 0) {
		updatestatistics ((int) hyphword, nw, 
			(currentprime + 1) * sizeof(strnumber));
		updatestatistics ((int) hyphlist, nl, 
			(currentprime + 1) * sizeof(halfword));
	}
	else {
		updatestatistics ((int) hyphword, nw, 0);
		updatestatistics ((int) hyphlist, nl, 0);
	}
	currentprime = hyphen_prime;
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return 0;								// success
}
#endif

int currentmemsize=0;		/* current total words in main mem allocated -1 */

/* this gets called from itex.c when it figures out what memtop is */
/* or gets called from here when in ini_TeX mode */ /* and nowhere else */
/* initial allocation only, may get expanded later */
/* NOTE: we DON't use ALLOCATEHIGH & ALLOCATELOW anymore */
/* returns NULL if it fails */

#ifdef ALLOCATEMAIN		
/* void allocatemainmemory (int size) { */ /* initial main memory alloc - memtop */
memoryword *allocatemainmemory (int size) {  /* initial main memory alloc - memtop */
	int n;
	
/*	Using -i *and* pre-loading format */ /* in this case get called twice */
/*	Get rid of initial blank memory again */ 		/* or use realloc ... */
/*	Could we avoid this by detecting presence of & before allocating ? */
/*	Also, if its already large enough, maybe we can avoid this ? */
/*  don't bother if currentmemsize == memmax - memstart ? */
	if (mainmemory != NULL) {
/*		free(mainmemory); */
/*		mainmemory = NULL; */
		if (traceflag) showline("Reallocating initial memory allocation\n", 1);
/*		if (memspecflag)
	showline("Cannot change initial main memory size when format is	read\n", 1);*/
	} 

	memtop = membot + size;
#ifdef ALLOCATEHIGH					/* NOT USED ANYMORE */
	if (memextrahigh != 0 && !is_initex) memmax = memtop + memextrahigh;	
#endif
	memmax = memtop;
#ifdef ALLOCATELOW					/* NOT USED ANYMORE */
	if (memextralow != 0 && !is_initex)
		memstart = membot - memextralow;	/* increase main memory */
#endif
	memstart = 0;			/* bottom of memory allocated by system */
/*	memmin = memstart; */	/* bottom of area made available to TeX */
	memmin = 0; 			/* bottom of area made available to TeX */
	n = (memmax - memstart + 1) * sizeof (memoryword); /* 256k * 8 = 2000 k */
	if (traceflag) tracememory("main memory", n);
/*	mainmemory = (memoryword *) malloc (n); */	/* 94/March/24 */
/*	normally mainmemory == NULL here so acts like malloc ... */
	mainmemory = (memoryword *)	REALLOC (mainmemory, n);
	if (mainmemory == NULL)	{
		memoryerror("initial main memory", n);
//		exit (1);							/* serious error */
		return NULL;
	}
	if (traceflag) {
		sprintf(logline, "Address main memory == %d\n", mainmemory);
		showline(logline, 0);
	}
	zzzaa = mainmemory;
	if (memstart != 0 && !is_initex) zzzaa = mainmemory - memstart; 
	if (traceflag) {
		sprintf(logline, "Offset address main memory == %d\n", zzzaa);
		showline(logline, 0);
	}
	updatestatistics ((int) mainmemory, n,
		(currentmemsize + 1) * sizeof (memoryword));
/*	currentmemsize = (memmax - memstart + 1); */
	currentmemsize = memmax - memstart;		/* total number of words - 1 */
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return zzzaa;							/* same as zmem, mem 94/Jan/24 */
}
#endif	/* end of ALLOCATEMAIN */

#ifdef ALLOCATEMAIN
/* int firstallocation = 1; */

/* increase main memory allocation at low end and high end */
/* called only from tex0.c *//* called with one of losize or hisize == 0 */
/* returns NULL if it fails */

memoryword *reallocmain (int losize, int hisize) {	
	int k, minsize;
	int newsize=0;				/* to quieten compiler */
	int n=0;					/* to quieten compiler */
	memoryword *newmemory=NULL; /* to quieten compiler */

/*	if (losize == 0 && hisize > 0) runawayflag = 1;		  
	else runawayflag = 0; */ /* 94/Jan/22 */

	if (traceflag) {
		sprintf(logline, "WARNING: Entering reallocmain lo %d hi %d\n",
			losize, hisize);
		showline(logline, 0);
	}
	if (is_initex) {
		showline( "ERROR: Cannot extent main memory in iniTeX\n", 1);
		if (! knuthflag) 
			showline("Please use `-m=...' on command line\n", 0);
//		exit (1);
//		abortflag++;	// ???
		return NULL;
	}
	if (traceflag) {
		sprintf(logline, "Old Address %s == %d\n", "main memory", mainmemory);
		showline(logline, 0);
	}
	if (currentmemsize + 1 == maxmemsize) {/* if we REALLY run up to limit ! */
		memoryerror("main memory", (maxmemsize + 1) * sizeof(memoryword));
//		exit (1);							/* serious error */
//		abortflag++;	// ???
		return NULL;
	}
/*  first allocation should expand *both* lo and hi */
	if (hisize == 0 && memend == memmax) hisize = losize;
	if (losize == 0 && memstart == memmin) losize = hisize;
/*	try and prevent excessive frequent reallocations */
/*	while avoiding over allocation by too much */
	minsize = currentmemsize / 100 * percentgrow;
	if (losize + hisize < minsize) {
		if (losize > 0 && hisize > 0) {
			losize = minsize / 2;
			hisize = minsize / 2;
		}
		else if (losize > 0) losize = minsize;
		else if (hisize > 0) hisize = minsize;
	}
	if (losize > 0 && losize < memtop / 2) losize = memtop / 2;
	if (hisize > 0 && hisize < memtop / 2) hisize = memtop / 2;

	for (k = 0; k < MAXSPLITS; k++) {
		newsize = currentmemsize + losize + hisize;
		if (newsize >= maxmemsize) {		/* bump against limit - ha ha ha */
			while (newsize >= maxmemsize) {
				losize = losize / 2; hisize = hisize / 2;
				newsize = currentmemsize + losize + hisize;
			}
		}
		n = (newsize + 1) * sizeof (memoryword);
		if (traceflag) tracememory("main memory", n);
		newmemory = (memoryword *) REALLOC (mainmemory, n);
		if (newmemory != NULL) break;	/* did we get it ? */
		if (currentmemsize == 0) break;	/* in case we ever use for initial */
		losize = losize / 2; hisize = hisize / 2;
	}

	if (newmemory == NULL) {
		memoryerror("main memory", n);
		return zzzaa;						/* try and continue with TeX !!! */
	}
	if (traceflag) {
		sprintf(logline, "New Address %s == %d\n", "main memory", newmemory);
		showline(logline, 0);
	}
	if (losize > 0) {
/*	shift everything upward to make space for new low area */
		if (traceflag) {
			sprintf(logline, "memmove %d %d %d \n", newmemory + losize,
					newmemory, (currentmemsize + 1) * sizeof(memoryword));
			showline(logline, 0);
		}
		memmove (newmemory + losize, newmemory,	
/*			currentmemsize * sizeof(memoryword));	 */
			(currentmemsize + 1) * sizeof(memoryword));
/*	could reduce words moved by (memmax - memend) */
	}
	mainmemory = newmemory;				/* remember for free later */
	if (losize > 0) memstart = memstart - losize;	/* update lower limit */
	if (hisize > 0) memmax = memmax + hisize;		/* update upper limit */
	updatestatistics ((int) mainmemory, n,
		(currentmemsize + 1) * sizeof (memoryword));
	currentmemsize = newsize;
	if (currentmemsize != memmax - memstart) {
		showline("ERROR: Impossible Memory Error\n", 1);
	}
	if (memstart != 0) zzzaa = mainmemory - memstart; /* ??? sign ??? */
	else zzzaa = mainmemory;
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return zzzaa;
}
#endif

#ifdef ALLOCATEFONT
/* fontmemsize = 10000L ==> fontinfo array 100k * 8 = 800 kilobytes */

int currentfontmemsize=0;

/* fmemoryword can be either halfword or memoryword */

fmemoryword *reallocfontinfo (int size) {	/* number of memorywords */
	fmemoryword *newfontinfo=NULL;
	int k, minsize;
	int newsize=0;				/* to quieten compiler */
	int n=0;					/* to quieten compiler */

	if (traceflag) {
		sprintf(logline, "Old Address %s == %d\n",  "fontinfo", fontinfo);
		showline(logline, 0);
	}
/*	during initial allocation, fontinfo == NULL  - realloc acts like malloc */
/*	during initial allocation currentfontmemsize == 0 */
	if (currentfontmemsize == fontmemsize) { /* if we REALLY run up to limit */
/*		memoryerror("font", (fontmemsize + 1) * sizeof(memoryword)); */
/*		exit (1); */
		return fontinfo;		/* pass it back to TeX 99/Fabe/4 */
	}
/*	try and prevent excessive frequent reallocations */
/*	while avoiding over allocation by too much */
/*	minsize = currentfontmemsize / 2; */
	minsize = currentfontmemsize / 100 * percentgrow;
	if (size < minsize) size = minsize;
	if (size < initialfontmemsize) size = initialfontmemsize;

	for (k=0; k < MAXSPLITS; k++) {
		newsize = currentfontmemsize + size;
		if (newsize > fontmemsize) newsize = fontmemsize; /* bump against limit */
/*		important + 1 since fmemoryword fontinfo[fontmemsize + 1]  original */
		n = (newsize + 1) * sizeof (fmemoryword);
		if (traceflag) tracememory("fontinfo", n);
		newfontinfo = (fmemoryword *) REALLOC (fontinfo, n);
		if (newfontinfo != NULL) break;		/* did we get it ? */
		if (currentfontmemsize == 0) break;	/* initial allocation must work */
		size = size / 2;
	}

	if (newfontinfo == NULL) {
		memoryerror("font", n);
		return fontinfo;				/* try and continue !!! */
	}
	fontinfo = newfontinfo;
	if (traceflag) {
		sprintf(logline, "New Address %s == %d\n", "fontinfo", fontinfo);
		showline(logline, 0);
	}
	updatestatistics ((int) fontinfo, n, currentfontmemsize * sizeof(fmemoryword));
	currentfontmemsize = newsize;
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return fontinfo;
}
#endif

#ifdef ALLOCATESTRING
int currentpoolsize=0;

packedASCIIcode *reallocstrpool (int size) {
	int k, minsize;
	int newsize=0;
	int n=0;
	packedASCIIcode *newstrpool=NULL;

	if (traceflag) {
		sprintf(logline, "Old Address %s == %d\n", "string pool", strpool);
		showline(logline, 0);
	}
	if (currentpoolsize == poolsize) {
/*		memoryerror ("string pool", (poolsize + 1) * sizeof(packedASCIIcode)); */
/*		exit (1); */
		return strpool;		/* pass it back to TeX 99/Fabe/4 */
	}
/*	minsize =  currentpoolsize / 2; */
	minsize =  currentpoolsize / 100 * percentgrow;
	if (size < minsize) size = minsize;
	if (size < initialpoolsize) size = initialpoolsize;

	for (k = 0; k < MAXSPLITS; k++) {
		newsize = currentpoolsize + size;
		if (newsize > poolsize) newsize = poolsize;
/* important + 1 since  packedASCIIcode strpool[poolsize + 1] ; in original */
		n = (newsize + 1) * sizeof (packedASCIIcode);
		if (traceflag) tracememory("strpool", n);
		newstrpool = (packedASCIIcode *) REALLOC (strpool, n); /* 95/Sep/24 */
		if (newstrpool != NULL) break;		/* did we get it ? */
		if (currentpoolsize == 0) break;	/* initial allocation must work */
		size = size / 2;					/* else can retry smaller */
	}

	if (newstrpool == NULL) {
		memoryerror("string pool", n);
		return strpool;						/* try and continue !!! */
	}
	strpool = newstrpool;
	updatestatistics ((int) strpool, n, currentpoolsize);
	currentpoolsize = newsize;
	if (traceflag) {
		sprintf(logline, "New Address %s == %d\n", "string pool", strpool);
		showline(logline, 0);
	}
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return strpool;
}
#endif

#ifdef ALLOCATESTRING
int currentmaxstrings=0;

poolpointer *reallocstrstart (int size) {
	int k, minsize;
	int n=0;
	int newsize=0;
	poolpointer *newstrstart=NULL;

	if (traceflag) {
		sprintf(logline, "Old Address %s == %d\n", "string start", strstart);
		showline(logline, 0);
	}
	if (currentmaxstrings == maxstrings) {
/*		memoryerror ("string pointer", (maxstrings + 1) * sizeof(poolpointer)); */
/*		exit (1); */
		return strstart;		/* pass it back to TeX 99/Fabe/4 */
	}
/*	minsize = currentmaxstrings / 2; */
	minsize = currentmaxstrings / 100 * percentgrow;
	if (size < minsize) size = minsize;
	if (size < initialmaxstrings) size = initialmaxstrings;

	for (k = 0; k < MAXSPLITS; k++) {
		newsize = currentmaxstrings + size;
		if (newsize > maxstrings) newsize = maxstrings;
/*		important + 1 since strstart[maxstring + 1] originally */
		n = (newsize + 1) * sizeof (poolpointer); 
		if (traceflag) tracememory("strstart", n);
		newstrstart = (poolpointer *) REALLOC (strstart, n);
		if (newstrstart != NULL) break;		/* did we get it ? */
		if (currentmaxstrings == 0) break;	/* initial allocation must work */
		size = size / 2;					/* otherwise can try smaller */
	}

	if (newstrstart == NULL) {
		memoryerror("string pointer", n);
		return strstart;					/* try and continue */
	}
	strstart = newstrstart;
	updatestatistics((int) strstart, n, currentmaxstrings * sizeof (poolpointer));
	currentmaxstrings = newsize;
	if (traceflag) {
		sprintf(logline, "New Address %s == %d\n", "string start", strstart);
		showline(logline, 0);
	}
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return strstart;
}
#endif

#ifdef ALLOCATEINI

/* returns -1 if it fails */

int allocateini (int size) {		/* size == triesize */
	int n, nl, no, nc, nr, nh, nt;
		nh = nr = nl = (size + 1) *  sizeof(triepointer);
		no = (size + 1) *  sizeof(trieopcode);
		nc = (size + 1) *  sizeof(packedASCIIcode);
/*		nt = (size + 1) *  sizeof(booleane); */
		nt = (size + 1) *  sizeof(char);
		n = nl + no + nc + nr + nh + nt;
/*		n = (size + 1) * (sizeof(packedASCIIcode) + sizeof(trieopcode) +
			3 *  sizeof(triepointer) + sizeof (char)); */
		if (traceflag) tracememory ("iniTeX hyphen trie", n);
		triel = (triepointer *) malloc (roundup(nl));
		trieo = (trieopcode *) malloc (roundup(no));
		triec = (packedASCIIcode *) malloc (roundup(nc));
		trier = (triepointer *) malloc (roundup(nr));
		triehash = (triepointer *) malloc (roundup(nh));
/*		trietaken = (booleane *) malloc (nt); */
		trietaken = (char *) malloc (roundup(nt));
		if (triec == NULL || trieo == NULL || triel == NULL || trier == NULL ||
			triehash == NULL || trietaken == NULL) {
			memoryerror("iniTeX hyphen trie", n);
//			exit (1);						/* serious error */			
			return -1;
		}
		if (traceflag) {
			sprintf(logline, "Addresses triel %d trieo %d triec %d\n", 
					triel, trieo, triec);
			showline(logline, 0);
			sprintf(logline, "Addresses trier %d triehash %d trietaken %d\n", 
					trier, triehash, trietaken);
			showline(logline, 0);
		}
		updatestatistics ((int) triel, nl, 0);
		updatestatistics ((int) trieo, no, 0);
		updatestatistics ((int) triec, nc, 0);
		updatestatistics ((int) trier, nr, 0);
		updatestatistics ((int) triehash, nh, 0);
		updatestatistics ((int) trietaken, nt, 0);
/*		triesize = size; */	/* ??? */
		if (traceflag)  probeshow ();			/* 94/Mar/25 */
		return 0;								// success
}
#endif

#ifdef ALLOCATESAVESTACK
int currentsavesize=0;

memoryword *reallocsavestack (int size) {
	int k, minsize;
	int n=0, newsize=0;
	memoryword *newsavestack=NULL;

	if (traceflag) {
		sprintf(logline, "Old Address %s == %d\n", "save stack", savestack);
		showline(logline, 0);
	}
	if (currentsavesize == savesize) {	/* arbitrary limit */
/*		memoryerror ("save stack", (savesize + 1) * sizeof(memoryword)); */
/*		exit (1); */
		return savestack;				/* let TeX handle the error */
	}
	minsize =  currentsavesize / 100 * percentgrow;
	if (size < minsize) size = minsize;
	if (size < initialsavesize) size = initialsavesize;

	for (k = 0; k < MAXSPLITS; k++) {
		newsize = currentsavesize + size;
		if (newsize > savesize) newsize = savesize;
		n = (newsize + 1) * sizeof (memoryword); /* savestack[savesize + 1] */
		if (traceflag) tracememory("savestack", n);
		newsavestack = (memoryword *) REALLOC (savestack, n);
		if (newsavestack != NULL) break;		/* did we get it ? */
		if (currentsavesize == 0) break;	/* initial allocation must work */
		size = size / 2;					/* else can retry smaller */
	}

	if (newsavestack == NULL) {
		memoryerror("save stack", n);
		return savestack;						/* try and continue !!! */
	}
	savestack = newsavestack;
	updatestatistics ((int) savestack, n, currentsavesize);
	currentsavesize = newsize;
	if (traceflag) {
		sprintf(logline, "Current%s %d\n", "savesize", currentsavesize);
		showline(logline, 0);
		sprintf(logline, "New Address %s == %d\n", "save stack", savestack);
		showline(logline, 0);
	}
	if (traceflag) probeshow ();			/* 94/Mar/25 */
	return savestack;
}
#endif

#ifdef ALLOCATEINPUTSTACK
int currentstacksize=0;				/* input stack size */

instaterecord *reallocinputstack (int size) {
	int k, minsize;
	int n=0, newsize=0;
	instaterecord *newinputstack=NULL;

	if (traceflag) {
		sprintf(logline, "Old Address %s == %d\n",  "input stack", inputstack);
		showline(logline, 0);
	}
	if (currentstacksize == stacksize) {	/* arbitrary limit */
/*		memoryerror ("input stack", (stacksize + 1) * sizeof(instaterecord)); */
/* 		exit (1); */
		return inputstack;
	}
	minsize =  currentstacksize / 100 * percentgrow;
	if (size < minsize) size = minsize;
	if (size < initialstacksize) size = initialstacksize;

	for (k = 0; k < MAXSPLITS; k++) {
		newsize = currentstacksize + size;
		if (newsize > stacksize) newsize = stacksize;
		n = (newsize + 1) * sizeof (instaterecord); /* inputstack[stacksize + 1] */
		if (traceflag) tracememory("inputstack", n);
		newinputstack = (instaterecord *) REALLOC (inputstack, n);
		if (newinputstack != NULL) break;		/* did we get it ? */
		if (currentstacksize == 0) break;	/* initial allocation must work */
		size = size / 2;					/* else can retry smaller */
	}

	if (newinputstack == NULL) {
		memoryerror("input stack", n);
		return inputstack;						/* try and continue !!! */
	}
	inputstack = newinputstack;
	updatestatistics ((int) inputstack, n, currentstacksize);
	currentstacksize = newsize;
	if (traceflag) {
		sprintf(logline, "Current%s %d\n", "stacksize", currentstacksize);
		showline(logline, 0);
		sprintf(logline, "New Address %s == %d\n", "input stack", inputstack);
		showline(logline, 0);
	}
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return inputstack;
}
#endif

#ifdef ALLOCATENESTSTACK
int currentnestsize=0;				/* current nest size */

liststaterecord *reallocneststack (int size) {
	int k, minsize;
	int n=0, newsize=0;
	liststaterecord *newnest=NULL;

	if (traceflag) {
		sprintf(logline, "Old Address %s == %d\n",  "nest stack", nest);
		showline(logline, 0);
	}
	if (currentnestsize == nestsize) {	/* arbitrary limit */
/*		memoryerror ("nest stack", (nestsize + 1) * sizeof(liststaterecord)); */
/*		exit (1); */
		return nest;				/* let TeX handle the error */
	}
	minsize =  currentnestsize / 100 * percentgrow;
	if (size < minsize) size = minsize;
	if (size < initialnestsize) size = initialnestsize;

	for (k = 0; k < MAXSPLITS; k++) {
		newsize = currentnestsize + size;
		if (newsize > nestsize) newsize = nestsize;
		n = (newsize + 1) * sizeof (liststaterecord); /* nest[nestsize + 1] */
		if (traceflag) tracememory("nest stack", n);
		newnest = (liststaterecord *) REALLOC (nest, n);
		if (newnest != NULL) break;		/* did we get it ? */
		if (currentnestsize == 0) break;	/* initial allocation must work */
		size = size / 2;					/* else can retry smaller */
	}

	if (newnest == NULL) {
		memoryerror("nest stack", n);
		return nest;						/* try and continue !!! */
	}
	nest = newnest;
	updatestatistics ((int) nest, n, currentnestsize);
	currentnestsize = newsize;
	if (traceflag) {
		sprintf(logline, "Current%s %d\n", "nestsize", currentnestsize);
		showline(logline, 0);
		sprintf(logline, "New Address %s == %d\n", "nest stack", nest);
		showline(logline, 0);
	}
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return nest;
}
#endif

#ifdef ALLOCATEPARAMSTACK
int currentparamsize=0;				/* current param size */

halfword *reallocparamstack (int size) {
	int k, minsize;
	int n=0, newsize=0;
	halfword *newparam=NULL;

	if (traceflag) {
		sprintf(logline, "Old Address %s == %d\n",  "param stack", paramstack);
		showline(logline, 0);
	}
	if (currentparamsize == paramsize) {	/* arbitrary limit */
/*		memoryerror ("param stack", (paramsize + 1) * sizeof(halfword)); */
/*		exit (1); */
		return paramstack;				/* let TeX handle the error */
	}
	minsize =  currentparamsize / 100 * percentgrow;
	if (size < minsize) size = minsize;
	if (size < initialparamsize) size = initialparamsize;

	for (k = 0; k < MAXSPLITS; k++) {
		newsize = currentparamsize + size;
		if (newsize > paramsize) newsize = paramsize;
		n = (newsize + 1) * sizeof (halfword); /* paramstack[paramsize + 1] */
		if (traceflag) tracememory("param stack", n);
		newparam = (halfword *) REALLOC (paramstack, n); 
		if (newparam != NULL) break;		/* did we get it ? */
		if (currentparamsize == 0) break;	/* initial allocation must work */
		size = size / 2;					/* else can retry smaller */
	}

	if (newparam == NULL) {
		memoryerror("param stack", n);
		return paramstack;						/* try and continue !!! */
	}
	paramstack = newparam;
	updatestatistics ((int) paramstack, n, currentparamsize);
	currentparamsize = newsize;
	if (traceflag) {
		sprintf(logline, "Current%s %d\n", "paramsize", currentparamsize);
		showline(logline, 0);
		sprintf(logline, "New Address %s == %d\n", "param stack", paramstack);
		showline(logline, 0);
	}
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return paramstack;
}
#endif

#ifdef ALLOCATEBUFFER
int currentbufsize=0;

ASCIIcode *reallocbuffer (int size) {
	int k, minsize;
	int n=0, newsize=0;
	ASCIIcode *newbuffer=NULL;

	if (traceflag) {
		sprintf(logline, "Old Address %s == %d\n", "buffer", buffer);
		showline(logline, 0);
	}
	if (currentbufsize == bufsize) {	/* arbitrary limit */
/*		memoryerror ("buffer", bufsize); */
/*		exit (1); */
		return buffer;		/* pass it back to TeX 99/Fabe/4 */
	}
	minsize =  currentbufsize / 100 * percentgrow;
	if (size < minsize) size = minsize;
	if (size < initialbufsize) size = initialbufsize;

	for (k = 0; k < MAXSPLITS; k++) {
		newsize = currentbufsize + size;
		if (newsize > bufsize) newsize = bufsize;
		n = (newsize + 1) * sizeof(ASCIIcode);	/* buffer[bufsize + 1] */
		if (traceflag) tracememory("buffer", n);
		newbuffer = (ASCIIcode *) REALLOC (buffer, n);
		if (newbuffer != NULL) break;		/* did we get it ? */
		if (currentbufsize == 0) break;		/* initial allocation must work */
		size = size / 2;					/* else can retry smaller */
	}

	if (newbuffer == NULL) {
		memoryerror("buffer", n);
		return buffer;						/* try and continue !!! */
	}
	buffer = newbuffer;
	updatestatistics ((int) buffer, n, currentbufsize);
#ifdef USEMEMSET
	memset(buffer + currentbufsize, 0, newsize - currentbufsize);
#else
	for (k = currentbufsize; k < newsize; k++) buffer [ k ] = 0 ;
#endif
	currentbufsize = newsize;
	if (traceflag) {
		sprintf(logline, "Current%s %d\n", "buffer", currentbufsize);
		showline(logline, 0);
		sprintf(logline, "New Address %s == %d\n", "buffer", buffer);
		showline(logline, 0);
	}
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return buffer;
}
#endif

/* we used to allocate this one only to reduce the size of the PE file */
/* not used anymore - NO advantage */

#ifdef ALLOCATEDVIBUF
eightbits *allocatedvibuf (int size) {
	eightbits *dvibuf;
	int n;
	
	n = (size + 1) * sizeof(eightbits);
	if (traceflag) tracememory("dvibuf", n);
	dvibuf = (eightbits *) malloc (roundup(n));
	if (dvibuf == NULL) {
		memoryerror("dvibuf", n);
//		exit (1);						/* serious error */
		return NULL;
	}
	if (traceflag) {
		sprintf(logline, "Address dvibuf %d\n", dvibuf);
		showline(logline, 0);
	}
	updatestatistics ((int) dvibuf, n, 0);
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return dvibuf;
}
#endif

/* we used to allocate this one only to reduce the size of the PE file */
/* it can be done without loss in performance, since register eqtb = zeqtb */
#ifdef ALLOCATEZEQTB
memoryword *allocatezeqtb (int k) {
	memoryword *zeqtb;
	int n;

	n = k * sizeof (memoryword); 	/* 13507 * 8 = 108 kilobytes */
	if (traceflag) 	tracememory("eqtb", n);
	zeqtb = (memoryword *) malloc (roundup(n));
	if (zeqtb == NULL)	{
		memoryerror("eqtb", n);
//		exit (1);						/* serious error */		
		return NULL;
	}
	if (traceflag) {
		sprintf(logline, "Address zeqtb %d\n", zeqtb);
		showline(logline, 0);
	}
	updatestatistics ((int) zeqtb, n, 0);
	if (traceflag)  probeshow ();			/* 94/Mar/25 */
	return zeqtb;
}
#endif

/* here is the main memory allocation routine -- calls the above */
/* returns -1 if it fails */

int allocatememory (void) {	/* allocate rather than static 93/Nov/26 */
/*	int n;  */
#ifdef PREALLOCHOLE
	char *holeadr = malloc (300000);	/* testing - preallocate 95/Jan/20 */
#endif

#ifdef ALLOCATEHASH
#error ERROR: Not ready for ALLOCATEHASH...
#endif

/* probably not worth while/not a good idea allocating following */
/* they are all rather small, and typically don't need expansion */
/* WE ASSUME THIS DOESN'T HAPPEN, SO WON'T BOTHER WITH UPDATESTATISTICS */
#ifdef ALLOCATEHASH
/*	n = 9767 * sizeof (twohalves); 	*//* 60 kilo bytes */		
/*	n = (hash_size + 267) * sizeof (twohalves); */	/* 60 kilo bytes */
/*	n = (9767 + eqtb_extra) * sizeof (twohalves); */
#ifdef SHORTHASH
	n = (hash_size + 267 + eqtb_extra) * sizeof (htwohalves); 	/* 95/Feb/19 */
#else
	n = (hash_size + 267 + eqtb_extra) * sizeof (twohalves); 	/* 95/Feb/19 */
#endif
	if (traceflag) 	tracememory("hash table", n);
#ifdef SHORTHASH
	zzzae = (htwohalves *) malloc (roundup(n));
#else
	zzzae = (twohalves *) malloc (roundup(n));
#endif
/*	zzzae = (twohalves *) malloc ((hash_size + 267) * sizeof (twohalves)); */
	if (zzzae == NULL) 	{
		memoryerror("hash table", n);
//		exit (1);						/* serious error */
		return -1;						/* serious error */
	}

	n = (inputsize + 1) * sizeof(memoryword);
	if (traceflag) 	tracememory("inputstack", n);
/*	inputstack = (memoryword *) malloc ((inputsize + 1) * sizeof (memoryword)); */
	inputstack = (memoryword *) malloc (roundup(n));
	if (inputstack == NULL) 	{
		memoryerror("inputstack", n);
//		exit (1);						/* serious error */
		return -1;						/* serious error */
	}
#endif

/* no real reason to allocate dvibuf - no need to ever grow it */
#ifdef ALLOCATEDVIBUF
/*	zdvibuf = NULL; */
	zdvibuf = allocatedvibuf (dvibufsize);
	if (zdvibuf == NULL) return -1;
#endif

#ifdef ALLOCATEZEQTB
/*	zeqtb = NULL; */
#ifdef INCREASEFONTS
/*	zeqtb = allocatezeqtb (13507 + eqtb_extra); */	/* 94/Mar/29 */
	zeqtb = allocatezeqtb (hash_size + 4007 + eqtb_extra);	/* 94/Mar/29 */
#else
/*	zeqtb = allocatezeqtb (13507); */
	zeqtb = allocatezeqtb (hash_size + 4007); 
#endif
#endif

#ifdef ALLOCATEINPUTSTACK
	inputstack = NULL;				/* new 1999/Jan/21 */
	currentstacksize = 0;
	inputstack = reallocinputstack (initialstacksize);	/* + 1 */
#endif

#ifdef ALLOCATENESTSTACK
	nest = NULL;					/* new 1999/Jan/21 */
	currentnestsize = 0;
	nest = reallocneststack (initialnestsize);	/* + 1 */
#endif

#ifdef ALLOCATEPARAMSTACK
	paramstack = NULL;					/* new 1999/Jan/21 */
	currentparamsize = 0;
	paramstack = reallocparamstack (initialparamsize); /* + 1 */
#endif

#ifdef ALLOCATESAVESTACK
	savestack = NULL;				/* new 1999/Jan/7 */
	currentsavesize = 0;
	savestack = reallocsavestack (initialsavesize);
#endif

#ifdef IGNORED
	buffer = NULL;				/* need to do earlier */
	currentbufsize = 0;
	buffer = reallocbuffer (initialbufsize);
#endif

#ifdef ALLOCATESTRING
	strpool = NULL;
	currentpoolsize = 0;
	strstart = NULL;
	currentmaxstrings = 0;
/*	need to create space because iniTeX writes in before reading pool file */
/*	for a start, puts in strings for 256 characters */
/*	maybe taylor allocations to actual pool file 1300 strings 27000 bytes ? */
	if (is_initex) {
		if (traceflag) showline("ini TeX pool and string allocation\n", 0);
		strpool = reallocstrpool (initialpoolsize); 
		strstart = reallocstrstart (initialmaxstrings);
	}
#endif

/* the following can save a lot of the usual 800k fixed allocation */
#ifdef ALLOCATEFONT
	fontinfo = NULL;
	currentfontmemsize = 0;
/*	if not iniTeX, then do initial allocation on fmt file read in itex.c */
/*	if ini-TeX we need to do it here - no format file read later */
	if (is_initex) fontinfo = reallocfontinfo (initialfontmemsize);
#endif
		
#ifdef ALLOCATEMAIN
	mainmemory = NULL;
	zzzaa = NULL;
	memmin = membot;				/* just to avoid complaints in texbody */
	memtop = meminitex;
	memmax = memtop;
/*	allocate main memory here if this is iniTeX */
/*	otherwise wait for format undumping in itex.c ... */	
	if (is_initex) {
/*		avoid this if format specified on command line ??? */
/*		allocatemainmemory(meminitex); */		/* made variable ! */
		mem = allocatemainmemory(meminitex);	/* made variable ! */
		if (mem == NULL)
//			exit (1);
			return -1;						/* serious error */
	}
#endif

/* now for the hyphenation exception stuff */
#ifdef ALLOCATEHYPHEN
	hyphword = NULL;
	hyphlist = NULL;
/*	this will be overridden later by what is in format file */
	hyphen_prime = defaulthyphenprime;
/*  non ini-TeX use assumes format will be read and that specifies size */
	if (is_initex) {
		if (newhyphenprime) hyphen_prime = newhyphenprime;
		if (reallochyphen (hyphen_prime))	/* allocate just in case no format */
			return -1;
	}
#endif

/*	now for memory for the part of the hyphenation stuff that always needed */
/*	if iniTeX, need to allocate pre-determined fixed amount - triesize */
/*	if iniTeX not selected, allocate only enough later - undump in itex.c ! */
#ifdef ALLOCATETRIES
	if (is_initex) {
		if (allocatetries (triesize)) return -1;
	}
#endif

/*	now for memory for hyphenation stuff needed only when running iniTeX */
#ifdef ALLOCATEINI
	if (is_initex) {
		if (allocateini(triesize)) return -1;
	}
	else {
		triel = trier = trieo = triehash = NULL; /* (triesize + 1) * integer */
		triec = NULL;				/* (triesize + 1) * char */
		trietaken = NULL;			/* (triesize + 1) * booleane */
	}
#endif
#ifdef PREALLOCHOLE
	free(holeadr);					/* create the hole */
#endif
	return 0;						// success
}

/* returns non-zero if error - done to test integrity of stack mostly */

int freememory (void) {			/* free in reverse order 93/Nov/26 */
	int n;
	unsigned heaptotal=0;
/*	unsigned total; */

	if (traceflag) showline("freememory ", 0);

#ifdef CHECKEQTB
	if (debugflag) checkeqtb("freememory");
#endif
	if (verboseflag || traceflag) showmaximums(stdout); 
#ifdef HEAPWALK
	if (heapflag) (void) heapdump(stdout, 1);
#endif
	if (traceflag) {
#ifdef HEAPWALK
		heaptotal = (void) heapdump(stdout, 0);
#endif
		sprintf(logline, "Heap total: %u bytes --- max address %u\n", 
				heaptotal, maxaddress);
		showline(logline, 0);
	}
	if (traceflag) {
		sprintf(logline, "Main Memory: variable node %d (%d - %d) one word %d (%d - %d)\n",
			lomemmax - memmin, memmin, lomemmax, memend	 - himemmin, himemmin, memend);
		showline(logline, 0);
	}
/*  following only needed to check consistency of heap ... useful debugging */
	if (traceflag) showline("Freeing memory again\n", 0);

/*	if (traceflag)
		showline(logline, "Zero Glue Reference Count %d\n", mem [ 0 ] .hh .v.RH); */

/*	the following checks the heap integrity */

/*	if ((n = _heapchk ()) != _HEAPOK) { */			/* 94/Feb/18 */
	n = _HEAPOK;
#ifdef SHOWHEAPERROR
	n = _heapchk();
	if (n != _HEAPOK) {			/* 94/Feb/18 */
		sprintf(logline, "WARNING: Heap corrupted (%d)\n", n);
		showline(logline, 1);
		sprintf(logline, "HEAP %s (%s)\n", heapstrings[-n], "freememory");
		showline(logline, 0);
		return n;		/* non-zero and negative */ /* unreachable ??? */
	}
#endif
/*	only free memory if safe ... additional check */
#ifdef ALLOCATEINI
	if (is_initex) {
		if (trietaken != NULL) free(trietaken);
		if (triehash != NULL) free(triehash);
		if (trier != NULL) free(trier);
		if (triec != NULL) free(triec);
		if (trieo != NULL) free(trieo);
		if (triel != NULL) free(triel);
		trietaken = NULL;
		triehash = triel = trier = NULL;
		triec = NULL;
		trieo = NULL;
	}
#endif	
#ifdef ALLOCATETRIES
	if (trietrc != NULL) free (trietrc);
	if (trietro != NULL) free (trietro);
	if (trietrl != NULL) free (trietrl);
	trietrc = NULL;
	trietro = trietrl = NULL;
#endif
#ifdef ALLOCATEHYPHEN
	if (hyphlist != NULL) free(hyphlist);
	if (hyphword != NULL) free(hyphword);
	hyphlist = NULL;
	hyphword = NULL;
#endif
#ifdef ALLOCATEMAIN
/*	if (zzzaa != NULL) free(zzzaa); */	/* NO: zzzaa may be offset ! */
	if (mainmemory != NULL) free(mainmemory);
	mainmemory = NULL;
#endif
#ifdef ALLOCATEFONT
	if (fontinfo != NULL) free(fontinfo);
	fontinfo = NULL;
#endif
#ifdef ALLOCATESTRING
	if (strstart != NULL) free(strstart);
	if (strpool != NULL) free(strpool);
	strstart = NULL;
	strpool = NULL;
#endif

#ifdef ALLOCATEHASH
	if (zzzae != NULL) free(zzzae);
	zzzae = NULL;
#endif

#ifdef ALLOCATEDVIBUF
	if (zdvibuf != NULL) free(zdvibuf);
	zdvibuf = NULL;
#endif
#ifdef ALLOCATEZEQTB
	if (zeqtb != NULL) free(zeqtb);
	zeqtb = NULL;
#endif

#ifdef ALLOCATEPARAMSTACK
	if (paramstack != NULL) free(paramstack);
	paramstack = NULL;
#endif
#ifdef ALLOCATENESTSTACK
	if (nest != NULL) free(nest);
	nest = NULL;
#endif
#ifdef ALLOCATEINPUTSTACK
	if (inputstack != NULL) free(inputstack);
	inputstack = NULL;
#endif
#ifdef ALLOCATESAVESTACK
	if (savestack != NULL) free(savestack);
	savestack = NULL;
#endif
/*	if (buffercopy != NULL) free (buffercopy); */	/* 94/Jun/27 */
	if (formatfile != NULL) free(formatfile);		/* 96/Jan/16 */
	if (stringfile != NULL) free(stringfile);		/* 96/Jan/16 */
	if (sourcedirect != NULL) free(sourcedirect);	/* 98/Sep/29 */
	formatfile = stringfile = sourcedirect = NULL;
	if (dvifilename != NULL) free(dvifilename);
	if (logfilename != NULL) free(logfilename);
	logfilename = dvifilename = NULL;				/* 00/Jun/18 */
	return 0;
}

booleane prime (int x) {
	int k;
	int sum = 1;		/* 1 + 3 + 5 + k = (k + 1) * (k + 1) / 4 */
	if (x % 2 == 0) return false;
	for (k = 3; k < x; k = k + 2) {
		if (x % k == 0) return false;
/*		if (k * k > x) return true; */
		if (sum * 4 > x) return true;
		sum += k;
	}
	return true;
}

int quitflag=0;
booleane show_use=false;
booleane floating=false;

void complainarg (int c, char *s) {
	sprintf(logline, "ERROR: Do not understand `%c' argument value `%s'\n", c, s);
	showline(logline, 1);
	show_use = 1;						// 2000 June 21
}

/* following is list of allowed command line flags and args */

/* char *allowedargs="+vitrdcyzpsqnwbfXABCDFGKLMNOQRSTYWZ?g=m=u=e=o=a=x=k=h=l=u=E=H="; */

/* only  01234567.9 still left to take ... maybe recycle u */

char *allowedargs="+bcdfijnpqrstvwyzABCDFGIJKLMNOPQRSTVWXYZ23456789?a=e=g=h=k=l=m=o=u=x=E=H=P=U=";

/* char takeargs="gmueoazhluEH"; */	/* subset that takes args! needed here */

void reorderargs (int ac, char **av) {			/* put in 1993/Dec/28 */
	int n, m;
	char *s, *t;
//	char takeargs[128];		/* large enough for all command line arg chars */
	char takeargs[256];		/* large enough for all command line arg chars */

/*	assumes arg pointers av[] are writeable */
/*	for (n = 1; n < ac; n++) sprintf(logline, "%s ", av[n]); */

	if (ac < 3) {	/* need more than one arg to reorder anything 94/Feb/25 */
/*		showline("No arguments?\n", 0); */	/* debugging */
		return;							/* no args ! */
	}

	s = allowedargs;
	t = takeargs;		/* list of those that take args */
	while (*s != '\0' && *(s+1) != '\0') {
		if (*(s+1) == '=') *t++ = *s++;		/* copy over --- without the = */
		s++;
	}
	*t = '\0';
	if (traceflag) {
		showline(takeargs, 0);
		showchar('\n');
	}
	
	n = 1;
	for (;;) {							/* scan to end of command line args */
		if (*av[n] != '-') break;
/*		does it take an argument ? and is this argument next ? */
		if (n+1 < ac &&
			*(av[n]+2) == '\0' &&
/*				strchr("gmuhleoxE", *(av[n]+1)) != NULL) */
				strchr(takeargs, *(av[n]+1)) != NULL)
					n += 2; /* step over it */
		else n++;
		if (n == ac) break;
	}

	for (;;) {							/* look for more command line args */
		if (n == ac) break;
		m = n;
/*		while (*av[m] != '-' && m < ac) m++; */	/* first command */
		while (m < ac && *av[m] != '-') m++;	/* first command */
		if (m == ac) break;
/* does it take an argument ? and is this argument next ? */
/* check first whether the `-x' is isolated, or arg follows directly */
/* then check whether this is one of those that takes an argument */
		if (m+1 < ac &&
			*(av[m]+2) == '\0' &&
				strchr(takeargs, *(av[m]+1)) != NULL) {
			s = av[m];			/*  move command down before non-command */
			t = av[m+1];
			for (; m > n; m--)	av[m+1] = av[m-1];
			av[n] = s;
			av[n+1] = t;
			n += 2;				/* step over moved args */
		}
		else {
			s = av[m];			/*  move command down before non-command */
			for (; m > n; m--)	av[m] = av[m-1];
			av[n] = s;
			n++;				/* step over moved args */
		}
	}
}

int testalign (int address, int size, char *name) {
	int n;
	if (size > 4) n = address % 4;
	else n = address % size;
	if (n != 0)	{
		sprintf(logline, "OFFSET %d (ELEMENT %d) in %s\n", n, size, name);
		showline(logline, 0);
	}
	return n;
}

/* activate detailed checking of alignment when traceflag is set */

void checkfixedalign (int flag) {
	if (testalign ((int) &memtop, 4, "FIXED ALIGNMENT")) {
		showline("PLEASE RECOMPILE ME!\n", 1);
	}
#ifdef CHECKALIGNMENT
	if (!flag) return;
	testalign ((int) &memtop, 4, "memtop");
	testalign ((int) &memmax, 4, "memmax");
	testalign ((int) &memmin, 4, "memmin");
	testalign ((int) &bad, 4, "bad");
	testalign ((int) &triesize, 4, "triesize");
	testalign ((int) &xord, sizeof(xord[0]), "xord"); /* no op */
	testalign ((int) &xchr, sizeof(xchr[0]), "xchr"); /* no op */
	testalign ((int) &namelength, 4, "namelength");
	testalign ((int) &first, 4, "first");
	testalign ((int) &last, 4, "last");
	testalign ((int) &maxbufstack, 4, "maxbufstack");
	testalign ((int) &poolptr, 4, "poolptr");
	testalign ((int) &strptr, 4, "strptr");
	testalign ((int) &initpoolptr, 4, "initpoolptr");
	testalign ((int) &initstrptr, 4, "initstrptr");
	testalign ((int) &logfile, 4, "logfile");
	testalign ((int) &tally, 4, "tally");
	testalign ((int) &termoffset, 4, "termoffset");
	testalign ((int) &fileoffset, 4, "fileoffset");
	testalign ((int) &trickcount, 4, "trickcount");
	testalign ((int) &firstcount, 4, "firstcount");
	testalign ((int) &deletionsallowed, 4, "deletionsallowed");
	testalign ((int) &setboxallowed, 4, "setboxallowed");
	testalign ((int) &helpline, sizeof(helpline[0]), "helpline");
	testalign ((int) &useerrhelp, 4, "useerrhelp");
	testalign ((int) &interrupt, 4, "interrupt");
	testalign ((int) &OKtointerrupt, 4, "OKtointerrupt");
	testalign ((int) &aritherror, 4, "aritherror");
	testalign ((int) &texremainder, 4, "texremainder");
	testalign ((int) &tempptr, 4, "tempptr");
	testalign ((int) &lomemmax, 4, "lomemmax");
	testalign ((int) &himemmin, 4, "himemmin");
	testalign ((int) &varused, 4, "varused");
	testalign ((int) &dynused, 4, "dynused");
	testalign ((int) &avail, 4, "avail");
	testalign ((int) &memend, 4, "memend");
	testalign ((int) &memstart, 4, "memstart");
	testalign ((int) &rover, 4, "rover");
	testalign ((int) &fontinshortdisplay, 4, "fontinshortdisplay");
	testalign ((int) &depththreshold, 4, "depththreshold");
	testalign ((int) &breadthmax, 4, "breadthmax");
	testalign ((int) &nest, sizeof(nest[0]), "nest");

#ifdef ALLOCZEQTB
	testalign ((int) &zeqtb, sizeof(zeqtb[0]), "zeqtb");  /* not any more ? */
#endif
/*	testalign ((int) &xeqlevel, sizeof(xeqlevel[0]), "xeqlevel"); */
	testalign ((int) &zzzad, sizeof(zzzad[0]), "zzzad");
/*	testalign ((int) &hash, sizeof(hash[0]), "hash"); */
	testalign ((int) &zzzae, sizeof(zzzae[0]), "zzzae");

	testalign ((int) &savestack, sizeof(savestack[0]), "savestack");
	testalign ((int) &inputstack, sizeof(inputstack[0]), "inputstack");
	testalign ((int) &inputfile, sizeof(inputfile[0]), "inputfile");
	testalign ((int) &linestack, sizeof(linestack[0]), "linestack");
	testalign ((int) &paramstack, sizeof(paramstack[0]), "paramstack");
	testalign ((int) &curmark, sizeof(curmark[0]), "curmark");
	testalign ((int) &pstack, sizeof(pstack[0]), "pstack");
	testalign ((int) &readfile, sizeof(readfile[0]), "readfile");

	testalign ((int) &fontcheck, sizeof(fontcheck[0]), "fontcheck");
	testalign ((int) &fontsize, sizeof(fontsize[0]), "fontsize");
	testalign ((int) &fontdsize, sizeof(fontdsize[0]), "fontdsize");
	testalign ((int) &fontparams, sizeof(fontparams[0]), "fontparams");
	testalign ((int) &fontname, sizeof(fontname[0]), "fontname");
	testalign ((int) &fontarea, sizeof(fontarea[0]), "fontarea");
	testalign ((int) &fontbc, sizeof(fontbc[0]), "fontbc");
	testalign ((int) &fontec, sizeof(fontec[0]), "fontec");
	testalign ((int) &fontglue, sizeof(fontglue[0]), "fontglue");
	testalign ((int) &fontused, sizeof(fontused[0]), "fontused");
	testalign ((int) &hyphenchar, sizeof(hyphenchar[0]), "hyphenchar");
	testalign ((int) &skewchar, sizeof(skewchar[0]), "skewchar");
	testalign ((int) &bcharlabel, sizeof(bcharlabel[0]), "bcharlabel");
	testalign ((int) &fontbchar, sizeof(fontbchar[0]), "fontbchar");
	testalign ((int) &fontfalsebchar, sizeof(fontfalsebchar[0]), "fontfalsebchar");
	testalign ((int) &charbase, sizeof(charbase[0]), "charbase");
	testalign ((int) &widthbase, sizeof(widthbase[0]), "widthbase");
	testalign ((int) &heightbase, sizeof(heightbase[0]), "heightbase");
	testalign ((int) &depthbase, sizeof(depthbase[0]), "depthbase");
	testalign ((int) &italicbase, sizeof(italicbase[0]), "italicbase");
	testalign ((int) &ligkernbase, sizeof(ligkernbase[0]), "ligkernbase");
	testalign ((int) &kernbase, sizeof(kernbase[0]), "kernbase");
	testalign ((int) &extenbase, sizeof(extenbase[0]), "extenbase");
	testalign ((int) &parambase, sizeof(parambase[0]), "parambase");

#ifdef ALLOCATEDVIBUF
	testalign ((int) &zdvibuf, sizeof(zdvibuf[0]), "zdvibuf"); /* no op */
#endif
	testalign ((int) &totalstretch, sizeof(totalstretch[0]), "totalstretch");
	testalign ((int) &totalshrink, sizeof(totalshrink[0]), "totalshrink");
	testalign ((int) &activewidth, sizeof(activewidth[0]), "activewidth");
	testalign ((int) &curactivewidth, sizeof(curactivewidth[0]), "curactivewidth");
	testalign ((int) &background, sizeof(background[0]), "background");
	testalign ((int) &breakwidth, sizeof(breakwidth[0]), "breakwidth");
	testalign ((int) &minimaldemerits, sizeof(minimaldemerits[0]), "minimaldemerits");
	testalign ((int) &bestplace, sizeof(bestplace[0]), "bestplace");
	testalign ((int) &bestplline, sizeof(bestplline[0]), "bestplline");
	testalign ((int) &hc, sizeof(hc[0]), "hc");
	testalign ((int) &hu, sizeof(hu[0]), "hu");
	testalign ((int) &hyf, sizeof(hyf[0]), "hyf");
/*	testalign ((int) &x, sizeof(x[0]), "x"); */

	testalign ((int) &hyfdistance, sizeof(hyfdistance[0]), "hyfdistance");
	testalign ((int) &hyfnum, sizeof(hyfnum[0]), "hyfnum");
	testalign ((int) &hyfnext, sizeof(hyfnext[0]), "hyfnext");
	testalign ((int) &opstart, sizeof(opstart[0]), "opstart");

/*	testalign ((int) &trieophash, sizeof(trieophash[0]), "trieophash"); */
	testalign ((int) &zzzaf, sizeof(zzzaf[0]), "zzzaf");
	testalign ((int) &trieused, sizeof(trieused[0]), "trieused");
/*	testalign ((int) &trieoplang, sizeof(trieoplang[0]), "trieoplang");*/
	testalign ((int) &trieopval, sizeof(trieopval[0]), "trieopval");

	testalign ((int) &triemin, sizeof(triemin[0]), "triemin");
	testalign ((int) &pagesofar, sizeof(pagesofar[0]), "pagesofar");
	testalign ((int) &writefile, sizeof(writefile[0]), "writefile");
	testalign ((int) &writeopen, sizeof(writeopen[0]), "writeopen");
#endif
}

void checkallocalign (int flag) {
	if (testalign ((int) zeqtb, sizeof(zeqtb[0]), "ALLOCATED ALIGNMENT"))
		showline("PLEASE RECOMPILE ME!\n", 1);
#ifdef CHECKALIGNMENT
	if (!flag) return;
#ifndef ALLOCZEQTB
	testalign ((int) zeqtb, sizeof(zeqtb[0]), "zeqtb"); 
#endif
#ifndef ALLOCATEDVIBUF
	testalign ((int) &zdvibuf, sizeof(zdvibuf[0]), "zdvibuf");	/* no op */
#endif
	testalign ((int) strpool, sizeof(strpool[0]), "strpool");	/* no op */
	testalign ((int) strstart, sizeof(strstart[0]), "strstart");
	testalign ((int) zmem, sizeof(zmem[0]), "main memory");
	testalign ((int) fontinfo, sizeof(fontinfo[0]), "font memory");
	testalign ((int) trietrl, sizeof(trietrl[0]), "trietrl");
	testalign ((int) trietro, sizeof(trietro[0]), "trietro");
	testalign ((int) trietrc, sizeof(trietrc[0]), "trietrc");
	testalign ((int) hyphword, sizeof(hyphword[0]), "hyphword");
	testalign ((int) hyphlist, sizeof(hyphlist[0]), "hyphlist");
/*	testalign ((int) triec, sizeof(triec[0]), "triec"); *//* no op */
	testalign ((int) trieo, sizeof(trieo[0]), "trieo");
	testalign ((int) triel, sizeof(triel[0]), "triel");
	testalign ((int) trier, sizeof(trier[0]), "trier");
	testalign ((int) triehash, sizeof(triehash[0]), "triehash");
	testalign ((int) trietaken, sizeof(trietaken[0]), "trietaken");
#endif
}

#ifdef HEAPSHOW
void showaddresses (void) {					/* 96/Jan/20 */
	int c;
	int d;
	sprintf(logline, "STACK %d %d (grows %s) ", &c, &d, (&d > &c) ? "upward" : "downward");
	showline(logline, 0);
	sprintf(logline, "eqtb %d hash %d ", zeqtb, zzzae);
	showline(logline, 0);
	sprintf(logline, "newmagic %d\n", newmagic);
	showline(logline, 0);
	sprintf(logline, "dvibuf %d xchr %d xord %d nest %d\n",
			zdvibuf, xchr, xord, nest);
	showline(logline, 0);
	sprintf(logline, "savestack %d inputstack %d linestack %d paramstack %d\n",
		   savestack, inputstack, linestack, paramstack);
	showline(logline, 0);
	sprintf(logline, "fontcheck %d fontsize %d fontdsize %d fontparams %d fontname %d\n",
		   fontcheck, fontsize, fontdsize, fontparams, fontname);
	showline(logline, 0);
	sprintf(logline, "main %d fontinfo %d strpool %d strstart %d hyphword %d hyphlist %d\n",
			zmem, fontinfo, strpool, strstart, hyphword, hyphlist);
	showline(logline, 0);
}
#endif

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env var' from `dviwindo.ini' - or from DOS environment 94/May/19 */
/* controlled by USEDVIWINDOINI environment variable 94/June/19 */

booleane usedviwindo = true;  /* use [Environment] section in `dviwindo.ini' */

booleane backwardflag = false;	/* don't cripple all advanced features */

booleane shortenfilename = false;  /* don't shorten file names to 8+3 for DOS */

char *inifilename = "dviwindo.ini";		/* name of ini file we look for */

char *dviwindo = "";			/* full file name for dviwindo.ini with path */

char *envsection = "[Environment]";		/* Env var section in `dviwindo.ini' */

char *wndsection = "[Window]";			/* Window section in `dviwindo.ini' */

char *workdirect = "WorkingDirectory";	/* key in [Window] section */

booleane usesourcedirectory = true;		/* use source file directory as local */
										/* when WorkingDirectory is set */

booleane workingdirectory = false;		/* if working directory set in ini */

/* set up full file name for dviwindo.ini and check for [Environment] */

booleane setupdviwindo (void) {	/* set up full file name for dviwindo.ini */
	char dviwindoini[PATH_MAX];
	char line[PATH_MAX];
	FILE *input;
	char *windir;
	int em = strlen(envsection);
	int wm = strlen(wndsection);
	int dm = strlen(workdirect);
	int wndflag = 0;
	int envflag = 0;

/*	Easy to find Windows directory if Windows runs */
/*	Or if user kindly set WINDIR environment variable */
/*	Or if running in Windows NT */	
	if ((windir = getenv("windir")) != NULL ||		/* 94/Jan/22 */
		(windir = getenv("WINDIR")) != NULL ||
		(windir = getenv("winbootdir")) != NULL ||	/* 95/Aug/14 */
		(windir = getenv("SystemRoot")) != NULL ||	/* 95/Jun/23 */
		(windir = getenv("SYSTEMROOT")) != NULL) {	/* 95/Jun/23 */
		strcpy(dviwindoini, windir);
		strcat(dviwindoini, "\\");
		strcat(dviwindoini, inifilename);
/*		sprintf(logline, "Using WINDIR %s\n", dviwindoini); */
	}
	else {
		_searchenv (inifilename, "PATH", dviwindoini);
/*		sprintf(logline, "Using SEARCHENV %s\n", dviwindoini); */
	}

	wndflag = envflag = 0;
/*	workingdirectory = false; */
	if (*dviwindoini != '\0') {
		dviwindo = xstrdup(dviwindoini);		/* avoid PATH_MAX string */
/*		check whether dviwindo.ini actually has [Environment] section */
		if (shareflag == 0) input = fopen(dviwindo, "r");
		else input = _fsopen(dviwindo, "r", shareflag);
		if (input != NULL) {
			while (fgets (line, sizeof(line), input) != NULL) {
				if (*line == ';') continue;
				if (*line == '\n') continue;
				if (*line == '[') {
					if (wndflag && envflag) break;	/* escape early */
				}
				if (_strnicmp(line, wndsection, wm) == 0) {
					if (traceflag) {
						sprintf(logline, "Found %s", line);  /* DEBUGGING */
						showline(logline, 0);
					}
					wndflag++;
				}
				else if (_strnicmp(line, envsection, em) == 0) {
					if (traceflag) {
						sprintf(logline, "Found %s", line);  /* DEBUGGING */
						showline(logline, 0);
					}
/*					fclose(input); */
/*					return true; */
					envflag++;
				}
				else if (wndflag && _strnicmp(line, workdirect, dm) == 0) {
					if (traceflag) {
						sprintf(logline, "Found %s", line);  /* DEBUGGING */
						showline(logline, 0);
					}
					workingdirectory = true;
				}
			}
			if (envflag) {
				(void) fclose(input); 
				return true;
			}
			if (traceflag)
				showline("Failed to find [Environment]", 1); /* DEBUGGING */
			(void) fclose(input);
		}
		else if (traceflag) perrormod(dviwindo);	/* DEBUGGING */
		strcpy(dviwindo, "");	/* failed, for one reason or another */
	}
	return false;
}

/* cache to prevent allocating twice in a row */

char *lastname=NULL, *lastvalue=NULL;

/* get value of env var - try first in dviwindo.ini then DOS env */
/* returns allocated string -- these strings are not freed again */
/* is it safe to do that now ? 98/Jan/31 */

char *grabenv (char *varname) {
	char line[PATH_MAX];
	FILE *input;
	char *s;
	int m, n;
/*	int m = strlen(envsection); */
/*	int n = strlen(varname); */

	if (varname == NULL) return NULL;		/* sanity check */
	if (*varname == '\0') return NULL;		/* sanity check */
/*	speedup to avoid double lookup when called from setpaths in ourpaths.c */
/*	if (lastname != NULL && strcmp(lastname, varname) == 0) { */
	if (lastname != NULL && _strcmpi(lastname, varname) == 0) {
		if (traceflag) {
			sprintf(logline, "Cache hit: %s=%s\n", lastname, lastvalue);
			showline(logline, 0);
		}
/*		return lastvalue; */				/* save some time here */
		return xstrdup(lastvalue);
/*		duplicate so can free safely 98/Jan/31 */
	}

/*	hmm, following was not xstrdup(...) */ /* not cached */
	if (usedviwindo == 0 || *dviwindo == '\0') {
/*		return getenv(varname); */
		s = getenv(varname);
		if (s == NULL) return NULL;
		else return xstrdup(s);				/* make safe 98/Jan/31 */
	}

	if (shareflag == 0) input = fopen(dviwindo, "r");
	else input = _fsopen(dviwindo, "r", shareflag);

	if (input != NULL) {
		m = strlen(envsection);
/*		search for [Environment] section */	/* should be case insensitive */
		while (fgets (line, sizeof(line), input) != NULL) {
			if (*line == ';') continue;
			if (*line == '\n') continue;
			if (_strnicmp(line, envsection, m) == 0) {	/* 98/Jan/31 */
/*				search for varname=... */	/* should be case sensitive ? */
				n = strlen(varname);
				while (fgets (line, sizeof(line), input) != NULL) {
					if (*line == ';') continue;
					if (*line == '[') break;
/*					if (*line == '\n') break; */	/* ??? */
					if (*line <= ' ') continue;		/* 95/June/23 */
/*					if (strncmp(line, varname, n) == 0 && */
					if (_strnicmp(line, varname, n) == 0 &&
						*(line+n) == '=') {	/* found it ? */
							(void) fclose (input);
/*							flush trailing white space */
							s = line + strlen(line) - 1;
							while (*s <= ' ' && s > line) *s-- = '\0';
							if (traceflag) { /* DEBUGGING ONLY */
								sprintf(logline, "%s=%s\n", varname, line+n+1);
								showline(logline, 0);
							}
							s = line+n+1;
							if (lastname != NULL) free(lastname);
							lastname = xstrdup (varname);
							if (lastvalue != NULL) free(lastvalue);
							lastvalue = xstrdup(s);
							return xstrdup(s);		/* 98/Jan/31 */
					}		/* end of matching varname */
				}			/* end of while fgets */
/*				break; */	/* ? not found in designated section */    
			}				/* end of search for [Environment] section */
		}
		(void) fclose (input);
	}						/* end of if fopen */
	s = getenv(varname);		/* failed, so try and get from environment */
/*	if (s != NULL) return s;  */
	if (s != NULL) {
/*		sdup = xstrdup(s); */		/* just to be safe --- 1995/Jan/31 */
		if (lastname != NULL) free(lastname);
		lastname = xstrdup (varname);
		if (lastvalue != NULL) free(lastvalue);
		lastvalue = xstrdup(s);		/* remember in case asked again ... */
/*		return sdup; */
		return xstrdup(s);		/* 98/Jan/31 */
	}
	else return NULL;		/* return NULL if not found anywhere */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void flushtrailingslash (char *directory ) {
	char *s;
/*	flush trailing \ or / in directory, if any 1993/Dec/12 */
	if (strcmp(directory, "") != 0) {
		s = directory + strlen(directory) - 1;
		if (*s == '\\' || *s == '/') *s = '\0';
	}
}

void knuthify (void) {
/*		showcurrent = false;	*/		/* show ultimate limits */
/*		reorderargflag = false; */		/* don't reorder command line */
/*		deslash = false; */				/* don't unixify file names */
/*		returnflag = false; */			/* don't allow just ^^M termination */
/*		trimeof = false; */				/* don't trim ^^Z Ctrl-Z at end of file */
		restricttoascii = false;		/* don't complain non ASCII */
		allowpatterns = false;			/* don't allow pattern redefinition */
		showinhex = true;				/* show character code in hex */
		showindos = false;				/* redundant with previous */
		shownumeric = false;			/* don't show character code decimal */
		showmissing = false;			/* don't show missing characters */
		civilizeflag = false;			/* don't reorder date fields */
		cstyleflag = false;				/* don't add file name to error msg */
		showfmtflag = false;			/* don't show format file in log */
		showtfmflag = false;			/* don't show metric file in log */
/*		fontmax = 255; */				/* revert to TeX 82 limit */
/*		if you want this, change in tex.h definition of fontmax to `int' */
/*		and add define FONTMAX 511, and in local.c add fontmax = FONTMAX; */
		tabstep = 0;
		showlinebreakstats = false;		/* do not show line break stats */
		showfontsused = false;
		defaultrule = 26214;			/* revert to default rule thickness */
		pseudotilde = false;
		pseudospace = false;
		showtexinputflag = false;
		truncatelonglines = false;
		allowquotednames = false;
		showcsnames = false;
		fontdimenzero = false;			/* 98/Oct/5 */
		ignorefrozen = false;			/* 98/Oct/5 */
		suppressfligs = false;			/* 99/Jan/5 */
		fullfilenameflag = false;		// 00 Jun 18
		savestringsflag = false;		// 00 Aug 15
		knuthflag = true;				/* so other code can know about this */
}	/* end of knuthify */

/* following have already been used up */

/* abcdefghijklmnopqrstuvwxyz */

/* ABCDEFGHIJKLMNOPQRSTUVWXYZ */

/* ........ */

int nohandler=0;		/* experiment to avoid Ctrl-C interrupt handler */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following made global so analyzeflag can be made separate procedure */

// char *xchrfile="";					/* save space use xstrdup */
char *xchrfile=NULL;					/* save space use xstrdup */
// char *replfile="";					/* save space use xstrdup */
char *replfile=NULL;					/* save space use xstrdup */

/* abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ */

/* analyze command line flag or argument */
/* c is the flag letter itself, while optarg is start of arg if any */

/* when making changes, revise allowedargs */


int analyzeflag (int c, char *optarg) {
		switch (c)  {
          case 'v':  want_version = true;
		             verboseflag = true;
					 break;
          case 'i':  is_initex = true;
					 break;
		  case 'Q':  interaction = 0;			/* quiet mode */
					 break;
		  case 'R':  interaction = 1;			/* run mode */
					 break;
		  case 'S':  interaction = 2;			/* scroll mode */
					 break;
		  case 'T':  interaction = 3;			/* tex mode */
					 break;
		  case 'K':  backwardflag = true;		/* 94/Jun/15 */
					 knuthify ();				/* revert to `standard' Knuth TeX */
					 break;
		  case 'L':  cstyleflag = true;			/* C style error msg 94/Mar/21 */
					 break;
		  case 'Z':  showtfmflag = true;		/* show TFM in log file 94/Jun/21 */
					 break;
		  case 'c':  currenttfm = false;		/* not look current dir for TFM */
					 break;
		  case 'C':  currentflag = false;		/* not look current dir for files */
					 break;
		  case 'M':  showmissing = false;		/* do not show missing 94/June/10 */
					 break;
		  case 'd':  deslash = false;			/* flipped 93/Nov/18 */
/*					 pseudotilde = 0; */		/* new 95/Sep/26 */
					 break;
          case 'p':  allowpatterns = true;		/* 93/Nov/26 */
/*			         resetexceptions = true; */	/* 93/Dec/23 */
					 break;
//		  case 'w':	 showinhex = false;			/* 94/Jan/26 */
		  case 'w':	 showinhex = true;			/* flipped 00/Jun/18 */
					 break;
		  case 'j':	 showindos = true;			/* 96/Jan/26 */
					 break;
		  case 'n':	 restricttoascii = true;	/* 0 - 127 1994/Jan/21 */
					 break;
		  case '6':  workingdirectory = true;	/* use source dir 98/Sep/29 */
					 break;
		  case '7':  usesourcedirectory = false;	/* use working dir 98/Sep/29 */
					 break;
		  case 'f':	 showfontsused = false;		/* 97/Dec/24 */
					 break;
		  case '8':  shortenfilename = true;	/* 95/Feb/20 */
					 break;
		  case '9':  showcsnames = true;		/* 98/Mar/31 */
					 break;
		  case '4':	 ignorefrozen = true;		/* 98/Oct/5 */
					 break;
		  case '5':	 fontdimenzero = false;		/* 98/Oct/5 */
					 break;
		  case 'F':	 showtexinputflag = false;		/* 98/Jan/28 */
					 break;
//		  case 'X':  truncatelonglines = false;		/* 98/Feb/2 */
//					 break;
		  case 'W':	 usedviwindo = false;			/* 94/May/19 */
					 break;
		  case 'J':	 showlinebreakstats = false;	/* 96/Feb/8 */
					 break;
		  case 'O':  showfmtflag = false;		/* 94/Jun/21 */
					 break;
		  case 'I':	 formatspecific = false;	/* 95/Jan/7 */
					 break;
		  case '3':	 encodingspecific = false;	/* 98/Oct/5 */
					 break;
		  case '2':	 suppressfligs = true;		/* 99/Jan/5 f-lig */
					 break;
/* following are pretty obscure */
/*		  case 'y':  cachefileflag = false; */	/* 96/Nov/16 */
/*					 break; */
/*		  case 'r':  returnflag = false; */		/* flipped 93/Nov/18 */
/*					 break; */
/*		  case 'z':  trimeof = false; */		/* 93/Nov/24 */
/*					 break; */
		  case 'z': fullfilenameflag = false;	// 00 Jun 18
					break;
		  case 'X': savestringsflag = false;	// 00 Aug 15
					break;
/* following are unannounced options */ /* some may be recycled ... */
		  case 't':  traceflag = true;
					 break;
		  case 'q':  quitflag++;					/* 93/Dec/16 */
					 break;
/*	The following are really obscure and should not be advertized */
		  case 's':  showcurrent = false;		/* tex8 93/Dec/14 */
					 break;
		  case 'N':  shownumeric = false;		/* 93/Dec/21 */
					 break;
          case 'A':  civilizeflag = false;		/* 93/Dec/16 */
					 break; 
		  case 'B':	 opentraceflag = true;		/* openinou 1994/Jan/8 */
					 break;
		  case 'Y':  reorderargflag = false;	/* local */
					 break;
	      case 'b':  testdiraccess = false;		/* 94/Feb/10 */
					 break;
		  case 'D':	 dirmethod = false;			/* 94/Feb/10 */
					 break;
		  case 'G':  filemethod = false;		/* 94/Feb/13 */
					 break;
//		  case 'V':  shareflag = _SH_DENYNO; 	/* 0x40 - deny none mode */
//					 break; 
/*        case 'X':  nohandler++;	break; */
/*		  case 'f':	 waitflush = false;	break; */
/*		  case 'F':  floating = true;	break; */
/* *********** following command line options take arguments **************  */
		  case 'm':  if (optarg == 0) meminitex = memtop;
					 else meminitex = atoi(optarg) * 1024;	/* 93 Dec/1 */
					 if (meminitex == 0) complainarg(c, optarg);
					 memspecflag = 1;
			break;
#ifdef VARIABLETRIESIZE
		  case 'h':  if (optarg == 0) triesize = defaulttriesize;
					 else triesize = atoi(optarg);			/* 93 Dec/1 */
					 if (triesize == 0) complainarg(c, optarg);
			break;
#endif
#ifdef ALLOCATEHYPHEN
          case 'e':  if (optarg == 0) newhyphenprime = hyphen_prime * 2;
					 else newhyphenprime = atoi(optarg);	/* 93/Nov/26 */
					 if (newhyphenprime == 0) complainarg(c, optarg);
            break;
#endif
#ifdef ALLOCATEDVIBUF
		  case 'u':  if (optarg == 0) dvibufsize = defaultdvibufsize;
					 else dvibufsize = atoi(optarg);		/* 94/Mar/24 */
					 if (dvibufsize == 0) complainarg(c, optarg);
			break;
#endif
          case 'g':  if (optarg == 0) percentgrow = 62;
					 else percentgrow = atoi(optarg);	/* 93/Dec/11 */
					 if (percentgrow == 0) complainarg(c, optarg);
            break;
		  case 'U':  if (optarg == 0) pseudotilde = 0;
					 else pseudotilde = atoi(optarg);	/* 95/Sep/26 */
					 if (pseudotilde > 255) pseudotilde = 255;
					 else if (pseudotilde < 128) pseudotilde = 128;
			break;
/*          case 'H':  if (optarg == 0) heapthreshold = 1024;
					 else heapthreshold = atoi(optarg);	
					 if (heapthreshold == 0) complainarg(c, optarg);
					 else heapflag = 1;
            break; */
          case 'H':  if (optarg == 0) tabstep = 8;
					 else tabstep = atoi(optarg);	/* 94/July/3 */
					 if (tabstep == 0) complainarg(c, optarg);
            break;
          case 'x':  if (optarg == 0) xchrfile=xstrdup("xchr.map");
				     else xchrfile = xstrdup(optarg);
					 if (xchrfile == NULL || *xchrfile == '\0')
						 complainarg(c, optarg);
            break;
          case 'k':  if (optarg == 0) replfile =xstrdup("repl.key");
				     else replfile = xstrdup(optarg);
					 if (replfile == NULL || *replfile == '\0')
						 complainarg(c, optarg);
            break;
/* more obscure stuff - ppssibly recycle */
		  case 'P':  if (optarg == 0) defaultrule = 26214;	/* 95/Oct/9 */
					 else defaultrule = atoi(optarg);		/* 95/Oct/9 */
					 if (defaultrule == 0) complainarg(c, optarg);
					 break;
          case 'E':  if (optarg != 0) putenv(optarg);
					 else complainarg(c, optarg);
            break;
          case 'o':  if (optarg == 0) dvidirectory = "";
				     else dvidirectory = xstrdup(optarg); 
					 if (strcmp(dvidirectory, "") == 0)
						 complainarg(c, optarg);
            break;
          case 'l':  if (optarg == 0) logdirectory = "";
				     else logdirectory = xstrdup(optarg);
					 if (strcmp(logdirectory, "") == 0)
						 complainarg(c, optarg);
            break;
          case 'a':  if (optarg == 0) auxdirectory = "";
				     else auxdirectory = xstrdup(optarg);
					 if (strcmp(auxdirectory, "") == 0)
						 complainarg(c, optarg);
            break;
          case '?':
/*          default :  usage(av[0]); */
          default :  show_use = true;
					 return -1;				// failed to recognize
            break;
		}
		return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* char *yytexcmd="Y&YTEX.CMD"; */	/* name of command line file */

char *yytexcmd="YANDYTEX.CMD";		/* name of command line file */

/* Try and read default command file - YANDYTEX.CMD */
/* in current directory and then in directory of YANDYTEX */
/* (does not make sense in TeX file directory) */
/* since we don't yet know where that is ! */
/* can't conveniently include this in output file either - not open yet */

/* used both for yytex.cmd and @ indirect command line files */
/* can this be reentered ? */

void extension (char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

/* remove file name - keep only path - inserts '\0' to terminate */

void stripname (char *pathname) {
	char *s;
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
}

/* char commandfile[PATH_MAX]; */		/* keep around so can open later */

char *programpath="";					/* pathname of program */
										/* redundant with texpath ? */

/* The following does not deslashify arguments ? Do we need to ? */

int readcommands (char *filename) {
	char commandfile[PATH_MAX]; 
	FILE *command;
	char line[PATH_MAX];
	char *linedup;			/* need to copy line to preserve args */
	char *s;
/*  char *sn; */
	char *optarg;
	int c;

/*	Try first in current directory (or use full name as specified) */
	strcpy(commandfile, filename);
	extension(commandfile, "cmd");
	if (shareflag == 0) command = fopen(commandfile, "r");
	else command = _fsopen(commandfile, "r", shareflag);
	if (command == NULL) {
/*		If that fails, try in YANDYTeX program directory */
		strcpy(commandfile, programpath);
/*		don't need fancy footwork, since programpath non-empty */
		strcat(commandfile, "\\");
		strcat(commandfile, filename);
		extension(commandfile, "cmd");
		if (shareflag == 0) command = fopen(commandfile, "r");
		else command = _fsopen(commandfile, "r", shareflag);
		if (command == NULL) {
/*			perrormod(commandfile); */			/* debugging only */
/*			strcpy(commandfile, ""); */		/* indicate failed */
			return 0;				/* no command line file YYTEX.CMD */
		}
	}

/*	allow for multiple lines --- ignore args that don't start with `-' */
	while (fgets(line, PATH_MAX, command) != NULL) {
/*		sprintf(logline, "From %s:\t%s", commandfile, line); */
/*		skip over comment lines and blank lines */
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (strchr(line, '\n') == NULL) strcat(line, "\n");
/*		sfplogline, rintf("From %s:\t%s", commandfile, line); */
		linedup = xstrdup (line);					/* 93/Nov/15 */
		if (linedup == NULL) {
			showline("ERROR: out of memory\n", 1);		/* readcommands */
//			exit(1);
			return -1;		// failure
		}
		s = strtok(linedup, " \t\n\r"); 			/* 93/Nov/15 */
		while (s != NULL) {
			if (*s == '-' || *s == '/') {
				c = *(s+1);
				optarg = s+2;
/*				if (*optarg = '=') optarg++; */
				if (*optarg == '=') optarg++;
				if (analyzeflag(c, optarg) < 0) return -1;	// failure ???
			}
/*			else break; */							/* ignore non-flag items */
			s = strtok(NULL, " \t\n\r");			/* go to next token */
		}
/*		If you want to see command lines in file - put -v in the file */
/*		if (verboseflag != 0) sprintf(logline, "From %s:\t%s", commandfile, line); */
	}
	(void) fclose(command);		/* no longer needed */
	return 1;				// success
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* try and read commands on command line */

int readcommandline (int ac, char **av) { 
	int c;
	char *optargnew;	/* equal to optarg, unless that starts with `=' */
						/* in which case it is optarg+1 to step over the `=' */
						/* if optarg = 0, then optargnew = 0 also */

//	showline("readcommandline\n", 0);
	if (ac < 2) return 0; 		/* no args to analyze ? 94/Apr/10 */

/*	while ((c = getopt(ac, av, "+vitrdczp?m:h:x:E:")) != EOF) {  */
/*  NOTE: keep `Y' in there for `do not reorder arguments ! */
/*  WARNING: if adding flags, change also `allowedargs' and  `takeargs' !!!! */
	while ((c =	getopt(ac, av, allowedargs)) != EOF) {
		if (optarg != 0 && *optarg == '=') optargnew = optarg+1;
		else optargnew = optarg;
		analyzeflag (c, optargnew);
	}
	if (show_use || quitflag == 3) {
//		showversion (stdout);
		stampit(logline);
		strcat(logline, "\n");
		showline(logline, 0);
		stampcopy(logline);
		strcat(logline, "\n");
		showline(logline, 0);
		if (show_use) showusage(av[0]);
		else if (quitflag == 3) {
			showownerout (logline);
			strcat(logline, "\n");
			showline(logline, 0);
		}
//		exit (0);
		return -1;				// failure
	}	

#ifdef DEBUG
	if (floating) testfloating();		/* debugging */
#endif

	if (replfile != NULL && *replfile != '\0') {	/* read user defined replacement */
		if (readxchrfile(replfile, 1, av)) {
			if (traceflag) showline("KEY REPLACE ON\n", 0);
			keyreplace = true;
		}
	} 
/*	keyreplace used in texmf.c (input_line) */

	if (xchrfile != NULL && *xchrfile != '\0') {	/* read user defined xchr[] */
		if (readxchrfile(xchrfile, 0, av)) {
			if (traceflag) showline("NON ASCII ON\n", 0);
			nonascii = true;
		}
	} 
/*	nonascii used in texmf.c (topenin & input_line & calledit) */
/*	see also xchr [] & xord [] use in tex3.c and itex.c */
	return 0;
}

#ifdef IGNORED
void uppercase (char *s) {
	int c;
	while ((c = *s) != '\0') {
/*		if (islower(c)) *s = toupper (*s); */
		*s = toupper (*s);
		s++;
	}
}
#endif

int initcommands (int ac, char **av) {

/*  NOTE: some defaults changed 1993/Nov/18 */
/*	want_version = show_use = switchflag = returnflag = false;
	is_initex = traceflag = deslash = nonascii = false; */
	is_initex = allowpatterns = resetexceptions = false;
	nonascii = keyreplace = false;
	want_version = false;
	opentraceflag = traceflag = verboseflag = heapflag = false;
	restricttoascii = false;
//	showinhex = true;		/* default is to show as hex code ^^ */
	showinhex = false;		/* default is not to show as hex code ^^ 00/Jun/18 */
	showindos = false;		/* default is not to translate to DOS 850 */ 
	returnflag = trimeof = true;	// hard wired now
	deslash = true;
	pseudotilde = 254;		/* default '~' replace 95/Sep/26 filledbox DOS 850 */
	pseudospace = 255;		/* default ' ' replace 97/June/5 nbspace DOS 850 */
	defaultrule = 26214;	/* default rule variable 95/Oct/9 */
	showcurrent = civilizeflag = shownumeric = showmissing = true;
	currentflag = true;
	currenttfm = true;		/* search for TFMs in current dir as well */
	testdiraccess = true;	/* test if readable item is perhaps a sub-dir */
	dirmethod = true;		/* in dir_p: _findfirst instead of use fopen (nul) */
	filemethod = true;		/* use file_p (_findfirst) not readable (access) */
/*	waitflush = true; */	/* flushed 97/Dec/24 */
	cstyleflag = false;		/* use c-style error output */
	showfmtflag = true;		/* show format file in log */
	showtfmflag = false;	/* don't show metric file in log */
	shortenfilename = false; /* don't shorten file names to 8+3 */
	showtexinputflag = true;	/* show TEXINPUTS and TEXFONTS */
	truncatelonglines = true;	/* truncate long lines */
	tabstep = 0;			/* do not replace tabs with spaces */
	formatspecific = true;	/* do format specific TEXINPUTS 95/Jan/7 */
	encodingspecific = true;	/* do encoding specific TEXFONTS 98/Jan/31 */
	showlinebreakstats = true;	/* show line break statistics 96/Feb/8 */
	showfontsused = true;	/* show fonts used in LOG file 97/Dec/24 */
	allowquotednames = true;	/* allow quoted names with spaces 98/Mar/15 */
	showcsnames = false;	/* don't show csnames on start 98/Mar/31 */
	knuthflag = false;		/* allow extensions to TeX */
	cachefileflag = true;	/* default is to cache full file names 96/Nov/16 */
	fullfilenameflag = true;	/* new default 2000 June 18 */
	savestringsflag = true;	// 2000 Aug 15
	errout = stdout;		/* as opposed to stderr say --- used ??? */
	abortflag = 0;			// not yet hooked up ???
	errlevel = 0;			// not yet hooked up ???

	newhyphenprime = 0;
#ifdef VARIABLETRIESIZE
/*	triesize = defaulttriesize; */
	triesize = 0;
#endif
	memextrahigh = 0; memextralow = 0; meminitex = 0;
#ifdef ALLOCATEDVIBUF
	dvibufsize = 0;
#endif
/*	shareflag = _SH_DENYNO; */				/* 0x40 - deny none mode */
/*	shareflag = _SH_COMPAT; */				/* 0x00 - compatability mode */
	shareflag = 0;							/* revert to fopen for now */

/*	strncpy(programpath, argv[0], PATH_MAX); */	/* 94/July/12 */
	programpath = xstrdup(av[0]);				/* extract path executable */
	stripname(programpath);					/* strip off yandytex.exe */

/*	formatname = "PLAIN"; */	/* format name if specified on command line */
	formatname = "plain";	/* format name if specified on command line */

	encodingname = "";

	if (readcommands(yytexcmd) < 0)		/* read yandytex.cmd 1994/July/12 */
		return -1;						// in case of error

	if (readcommandline(ac, av) < 0) 	/* move out to subr 94/Apr/10 */
		return -1;						// in case of error

	if (optind == 0) optind = ac;		/* no arg case paranoia 94/Apr/10 */

/*	Print version *after* banner ? */	/* does this get in log file ? */
	if (want_version) {
//		showversion (stdout);
//		showversion (logline);
		stampit(logline);
		strcat(logline, "\n");
		showline(logline, 0);
		stampcopy(logline);
		strcat(logline, "\n");
		showline(logline, 0);
	}
/*	if (show_use) showusage(av[0]);		*/	/* show usage and quit */

/*	if we aren't including current directory in any directory lists */
/*	then makes no sense to avoid them separately for TFM files ... */
/*	(that is, the ./ is already omitted from the dir list in that case */
	if (!currentflag && !currenttfm) currenttfm = true;	/* 94/Jan/24 */
	return 0;								// success
}

/* E sets environment variable ? */

void initialmemory (void) {		/* set initial memory allocations */
	  if (memextrahigh < 0) memextrahigh = 0;
	  if (memextralow < 0) memextralow = 0;
	  if (meminitex < 0) meminitex = 0;
	  if (is_initex) {
 #if defined(ALLOCATEHIGH) || defined(ALLOCATELOW)
		  if (memextrahigh != 0 || memextralow != 0) {
			  showline("ERROR: Cannot extend main memory in iniTeX\n", 1);
			  memextrahigh = 0;	  memextralow = 0;
		  }
 #endif
	  }
	  else {
		  if (meminitex != 0) {
			  showline("ERROR: Can only set initial main memory size in iniTeX\n", 1);
			  meminitex = 0;
		  }
		  if (triesize != 0) {
			  showline("ERROR: Need only set hyphenation trie size in iniTeX\n", 1);
/*			  triesize = 0; */
		  }
	  }
	  if (meminitex == 0) meminitex = defaultmemtop;
	  if (triesize == 0) triesize = defaulttriesize;
/*	  Just in case user mistakenly specified words instead of kilo words */
	  if (memextrahigh > 10000L * 1024L) memextrahigh = memextrahigh / 1024;
	  if (memextralow > 10000L * 1024L) memextralow = memextralow / 1024;
	  if (meminitex > 10000L * 1024L) meminitex = meminitex / 1024;
#ifdef ALLOCATEHIGH					/* not used anymore */
	  if (memextrahigh > 2048L * 1024L) { /* extend SW area by 16 mega byte! */
		  showline(
		  "WARNING: There may be no benefit to asking for so much memory\n", 0); 
		  memextrahigh = 2048 * 1024; /* limit to SW to 4 x VLR */
	  }
#endif
#ifdef ALLOCATELOW					/* not used anymore */
	  if (memextralow > 2048L * 1024L) { /* extend VL area by 16 mega byte! */
		  showline(
		  "WARNING: There may be no benefit to asking for so much memory\n", 0); 
		  memextralow = 2048 * 1024; /* limit VLR to 4 x SW */
	  }
#endif
	  if (meminitex > 2048L * 1024L) { /* extend main memory by 16 mega byte! */
		  showline(
		  "WARNING: There may be no benefit to asking for so much memory\n", 0); 
/*		  meminitex = 2048 * 1024; */
	  }
 #ifdef ALLOCATEDVIBUF
	  if (dvibufsize == 0) dvibufsize = defaultdvibufsize;
	  /* if less than 1024 assume user specified kilo-bytes, not bytes */
	  if (dvibufsize < 1024) dvibufsize = dvibufsize * 1024;
	  if (dvibufsize % 8 != 0)				/* check multiple of eight */
			 dvibufsize = (dvibufsize / 8 + 1) * 8;
 #endif
	  if (newhyphenprime < 0) newhyphenprime = 0;
	  if (newhyphenprime > 0) {
		  if (! is_initex) 
			  showline("ERROR: Can only set hyphen prime in iniTeX\n", 1);
		  else {
			  if (newhyphenprime % 2 == 0) newhyphenprime++;
			  while (!prime(newhyphenprime)) newhyphenprime = newhyphenprime+2;
			  if (traceflag) {
				  sprintf(logline, "Using %d as hyphen prime\n", newhyphenprime);
				  showline(logline, 0);
			  }
		  }
	  }

	  if (percentgrow > 100) percentgrow = percentgrow - 100;
	  if (percentgrow > 100) percentgrow = 100;		/* upper limit - double */
	  if (percentgrow < 10) percentgrow = 10;		/* lower limit - 10% */
}

/**********************************************************************/

void perrormod (char *s) {
	sprintf(logline, "`%s': %s\n", s, strerror(errno));
	showline(logline, 1);
}

void pause (void) {
#ifndef _WINDOWS
	fflush(stdout);			/* ??? */
	fflush(stderr);			/* ??? */
	(void) _getch();		/* ??? */
#endif
}

void checkpause (int flag) {						/* 95/Oct/28 */
	char *s;
	int debugpause=0;
/*	don't stop if in Q (quiet) or R (run) mode */
/*	stop only in S (scroll) and T (TeX) mode */
	if (interaction >= 0 && interaction < 2) flag = 0;		/* 98/Jun/30 */
	s = grabenv("DEBUGPAUSE");
	if (s != NULL) sscanf(s, "%d", &debugpause);
	if (flag < 0) return;
	if (debugpause) {
//		if ((debugpause > 1) || flag) {
		if (debugpause || flag > 0) {
			showline("\n", 0);
#ifndef _WINDOWS
			showline("Press any key to continue . . .\n", 0);
			pause();
#endif
		}
	}
}

void checkenter (int argc, char *argv[]) {			/* 95/Oct/28 */
	int m;
	char current[FILENAME_MAX];
	if (grabenv ("DEBUGPAUSE") != NULL) {
		(void) _getcwd(current, sizeof(current));
		sprintf(logline, "Current directory: `%s'\n", current);
		showline(logline, 0);
		for (m = 0; m < argc; m++) {
//			sprintf(logline, "%2d:\t`%s'\n", m, argv[m]); 
			sprintf(logline, "%2d: `%s'\n", m, argv[m]); 
			showline(logline, 0);
		}
//		checkpause(0);
		checkpause(-1);
	}
}

#ifdef IGNORED
void checkexit (int n) {							/* 95/Oct/28 */
	checkpause(1);
	exit(n);
}
#endif

/*************************************************************************/

/* convert tilde to pseudotilde to hide it from TeX --- 95/Sep/26 */
/* convert space to pseudospace to hide it from TeX --- 97/Jun/5 */
/* called only if pseudotilde != 0 or pseudospace != 0 */
/* this is then undone in tex3.c both for fopen input and output */
/* not ideal, since pseudo name appears in log and in error messages ... */

void hidetwiddle (char *name) {
	char *s=name;
#ifdef DEBUGTWIDDLE
	if (traceflag) {
		sprintf(logline, "Hidetwiddle %s", name);
		showline(logline, 0);
	}
#endif
/*	while (*s != '\0' && *s != ' ')	{ */
	while (*s != '\0')	{
		if (*s == '~' && pseudotilde != 0)
			*s = (char) pseudotilde;	/* typically 254 */
		else if (*s == ' ' && pseudospace != 0)
			*s = (char) pseudospace;	/* typically 255 */
		s++;
	}
#ifdef DEBUGTWIDDLE
	if (traceflag) {
		sprintf(logline, "=> %s\n", name);
		showline(logline, 0);
	}
#endif
}

void deslashall (int ac, char **av) {
	char buffer[PATH_MAX];  
	char *s;

	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf(s, "%d", &usedviwindo);			/* 94/June/14 */

	if (usedviwindo) setupdviwindo();		// moved to yandytex ?

	checkenter(ac, av);						/* 95/Oct/28 */

/*	environment variables for output directories (as in PC TeX) */

/*	if ((s = getenv("TEXDVI")) != NULL) dvidirectory = s; */
	if ((s = grabenv("TEXDVI")) != NULL) dvidirectory = s;
/*	if ((s = getenv("TEXLOG")) != NULL) logdirectory = s; */
	if ((s = grabenv("TEXLOG")) != NULL) logdirectory = s;
/*	if ((s = getenv("TEXAUX")) != NULL) auxdirectory = s; */
	if ((s = grabenv("TEXAUX")) != NULL) auxdirectory = s;

	strcpy(buffer, av[0]);						/* get path to executable */
	if ((s = strrchr(buffer, '\\')) != NULL) *(s+1) = '\0';
	else if ((s = strrchr(buffer, '/')) != NULL) *(s+1) = '\0';
	else if ((s = strrchr(buffer, ':')) != NULL) *(s+1) = '\0';
	s = buffer + strlen(buffer) - 1;
	if (*s == '\\' || *s == '/') *s = '\0';		/* flush trailing PATH_SEP */
	texpath = xstrdup(buffer);

/*	Hmm, we may be operating on DOS environment variables here !!! */

	if (strcmp(dvidirectory, "") != 0) flushtrailingslash (dvidirectory);
	if (strcmp(logdirectory, "") != 0) flushtrailingslash (logdirectory);
	if (strcmp(auxdirectory, "") != 0) flushtrailingslash (auxdirectory);

	if (deslash) {
		  unixify (texpath);					/* 94/Jan/25 */
/* if output directories given, deslashify them also 1993/Dec/12 */
		  if (strcmp(dvidirectory, "") != 0) unixify(dvidirectory);
		  if (strcmp(logdirectory, "") != 0) unixify(logdirectory);
		  if (strcmp(auxdirectory, "") != 0) unixify(auxdirectory);
	}

/*	deslash TeX source file (and format, if format specified) */
/*	and check args to see whether format was specified */

	formatspec=0;
/*  NOTE: assuming that command line arguments are in writable memory ! */
/*	if (traceflag || debugflag)
		sprintf(logline, "optind %d ac %d\n", optind, ac); */		/* debugging */ 
/*	if (optind < ac) { */		 				/* bkph */
 	if (optind < ac && optind > 0) {			/* paranoia 94/Apr/10 */
		if (deslash) {
			if (traceflag || debugflag) {
				sprintf(logline, "deslash: k %d argv[k] %s (argc %d)\n",
					optind, av[optind], ac);
				showline(logline, 0);
			}
			unixify(av[optind]);
		}
		if (pseudotilde != 0 || pseudospace != 0)
			hidetwiddle (av[optind]);			/* 95/Sep/25 */
/*		if (*av[optind] == '&') { */					/* 95/Jan/22 */
/*		For Windows NT, lets allow + instead of & for format specification */
		if (*av[optind] == '&' || *av[optind] == '+') {
			formatspec = 1;				/* format file specified */
			formatname = xstrdup(av[optind]+1);			/* 94/Oct/25 */
/*			uppercase (formatname); */		/* why ? 98/Jan/31 */
			if (optind + 1 < ac) {
				if (deslash) {
					if (traceflag || debugflag) {
						sprintf(logline, "deslash: k %d argv[k] %s (argc %d)\n",
							optind+1, av[optind+1], ac);
						showline(logline, 0);
					}
					unixify(av[optind+1]);
				}
				if (pseudotilde != 0 || pseudospace != 0)
					hidetwiddle (av[optind+1]);	/* 95/Sep/25 */
			}
		}				  
	}
}

/* The above seems to assume that arguments that don't start with '-' */
/* are file names or format names - what if type in control sequences? */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* interaction == 0 => batch mode (omit all stops and omit terminal output) */
/* interaction == 1 => nonstop mode (omit all stops) */
/* interaction == 2 => scroll mode (omit error stops) */
/* interaction == 3 => error_stop mode (stops at every opportunity) */

/* main entry point follows */

/* this gets called pretty much right away in `main' in texmf.c */

/* note: those optarg == 0 test don't really work ... */
/* note: optarg starts at = in case of x=... */

int init (int ac, char **av) {
	char initbuffer[PATH_MAX];
	int k;
  
	debugfile = getenv("TEXDEBUG"); 		/* 94/March/28 */
	if (debugfile) debugflag = 1;
	else debugflag = 0;
	if (debugflag) {
		showline("TEXDEBUG\n", 0);
		traceflag = 1;						/* 94/April/14 */
	}

	if (sizeof(memoryword) != 8) {	/* compile time test */
		sprintf(logline, "ERROR: Bad word size %d!\n", sizeof(memoryword));
		showline(logline, 1);
	}

	starttime = clock();		/* get time */
	maintime = starttime;		/* fill in, in case file never opened */

	initbuffer[0] = '\0'; 				/* paranoia 94/Apr/10 */

/*	reset all allocatable memory pointers to NULL - in case we drop out */
	mainmemory = NULL;
	fontinfo = NULL;
	strpool = NULL;
	strstart = NULL;
#ifdef ALLOCATEZEQTB
	zeqtb = NULL;
#endif
#ifdef ALLOCATEHASH
	zzzae = NULL;
#endif
#ifdef ALLOCATESAVESTACK
	savestack = NULL; 
#endif
#ifdef ALLOCATEDVIBUF
	zdvibuf = NULL; 
#endif
#ifdef ALLOCATEBUFFER
	buffer = NULL;				/* new 1999/Jan/7 need to do early */
	currentbufsize = 0;
	buffer = reallocbuffer (initialbufsize);
/*	sprintf(logline, "buffer %x, currentbufsize %d\n", buffer, currentbufsize); */
#endif
	hyphlist = NULL;	hyphword = NULL;
	trietaken = NULL;	triehash = NULL;
	trier = NULL;	triec = NULL;	trieo = NULL;	triel = NULL;
	trietrc = NULL;	trietro = NULL;	trietrl = NULL;

	serialnumber = 0;			/* assumed high order first */
	for (k = 0; k < 4; k++) {
		serialnumber = serialnumber << 8;
		serialnumber = serialnumber | (newmagic[k+4] & 255);
/*		sprintf(logline, "Serial: %ld\n", serialnumber);  */
	}
	if (serialnumber == 1651208296) serialnumber = 0;	/* temporary */
/*	sprintf(logline, "Serial number %ld %ld\n",
		(serialnumber / REEXEC), (serialnumber % REEXEC)); */
/*	sprintf(logline, "Serial number %d %d\n",
		(int) (serialnumber / REEXEC), (int) (serialnumber % REEXEC)); */

	logopened = false ; 			/* so can tell whether opened */
	interaction = -1;				/* default state => 3 */
	missingcharacters=0;			/* none yet! */
	workingdirectory = false;		/* set from dviwindo.ini & command line */
	fontdimenzero = true;			/* \fontdimen0 for checksum 98/Oct/5 */
	ignorefrozen = false;			/* default is not to ignore 98/Oct/5 */
	suppressfligs = false;			/* default is not to ignore f-ligs */
/*	if (*av[1] == '-Y') reorderargflag = false; */	/* 94/April/14 */
	if (ac > 1 && *av[1] == '-Y') reorderargflag = false;

	if (reorderargflag) reorderargs(ac, av);  

	if (initcommands(ac, av))
		return -1;					// failure

	checkfixedalign(traceflag);				/* sanity check 1994/Jan/8 */

	formatfile = NULL;				/* to be set in openinou.c 94/Jun/21 */
	stringfile = NULL;				/* to be set in openinou.c 96/Jan/15 */
	sourcedirect = NULL;			/* to be set in openinou.c 98/Sep/29 */
	dvifilename = NULL;				/* to be set in openinou.c 00/Jun/18 */
	logfilename = NULL;				/* to be set in openinou.c 00/Jun/18 */

	firstpasscount = secondpasscount = finalpasscount = 0;	/* 96/Feb/9 */
	paragraphfailed = singleline = 0;						/* 96/Feb/9 */
	overfullhbox = underfullhbox = overfullvbox = underfullvbox = 0;

	closedalready=0;				// so can only do once

	if (traceflag) showline("Entering init (local)\n", 0);

/*	 Print version *after* banner ? */ /* does this get in log file ? */

	probememory ();							/* show top address */
	inimaxaddress = maxaddress;				/* initial max address */
	if (traceflag) showmaximums(stdout);
#ifdef HEAPWALK
	if (heapflag) (void) heapdump(stdout, 1);
#endif

	initialmemory();

	deslashall (ac, av);		/* deslash and note if format specified */

/*	if (want_version) sprintf(logline, "%s %d\n", version, serialnumber/REEXEC);	*/

/*	if (checkcopyright(copyright) != 0) exit (1); */
	if ((serialnumber % REEXEC) != 0 ||
			checkowner(hexmagic, initbuffer, sizeof(initbuffer)) != 0 ||
				checkcopyright(copyright) != 0) {
		showusage(av[0]);	/* show usage */
		return -1;			// failure
	}
/*	should crash machine ! */

/*	sprintf(logline, "%s\n", initbuffer); */		/* debugging, remove later */

#ifdef ALLOWDEMO
	if (bDemoFlag) dtime = checkdemo(initbuffer); /* assumes owner in initbuffer */
#endif

	nointerrupts = 0;

	if (formatspec && memspecflag) {
		showline(
  "WARNING: Cannot change initial main memory size when format specified",
			1);
	}

	 if (allocatememory() != 0) 	/* NOW, try and ALLOCATE MEMORY if needed */
		 return -1;					// if failed to allocate

/*	 following is more or less useless since most all things not yet alloc */
	 checkallocalign(traceflag);		/* sanity check 1994/Jan/8 */
#ifdef HEAPSHOW
	 if (traceflag) showaddresses();	/* debugging only 1996/Jan/20 */
#endif

#ifdef HEAPWALK
/*	 if (heapflag) heapdump(stdout, 1); */	/* redundant ? */
#endif

	  if (traceflag) showline("Leaving init (local)\n", 0);
	  return 0;					// success
}

/* #define CLOCKS_PER_SEC	1000 */ /* #define CLK_TCK  CLOCKS_PER_SEC */

/* void showinterval (clock_t start, clock_t end) { */
void showinterval (clock_t interval) {
/*	clock_t interval; */
/*	int seconds, tenths; */
/*	int seconds, tenths, hundredth;  */
	int seconds, tenths, hundredth, thousands;
/*	interval = end - start; */
/*	sanity check whether positive ? */
	if (interval >= CLK_TCK * 10) {
		tenths = (interval * 10 + CLK_TCK / 2) / CLK_TCK; 
		seconds = tenths / 10; 
		tenths = tenths % 10;
		sprintf(logline, "%d.%d", seconds, tenths);
		showline(logline, 0);
	}
	else if (interval >= CLK_TCK) {				/* 94/Feb/25 */
		hundredth = (interval * 100 + CLK_TCK / 2) / CLK_TCK;	
		seconds = hundredth / 100;
		hundredth = hundredth % 100;
		sprintf(logline, "%d.%02d", seconds, hundredth);
		showline(logline, 0);
	}
	else if (interval > 0) {					/* 94/Oct/4 */
		thousands = (interval * 1000 + CLK_TCK / 2) / CLK_TCK;	
		seconds = thousands / 1000;
		thousands = thousands % 1000;
		sprintf(logline, "%d.%03d", seconds, thousands);
		showline(logline, 0);
	}
	else showline("0", 0);					/* 95/Mar/1 */
}

/* final cleanup opportunity */ /* flag is non-zero if error exit */
/* shows various times, warning about missing chars */

int endit (int flag) {
/*	int msec; */
	finishtime = clock();
	if (missingcharacters != 0) flag = 1;
	if (missingcharacters) {
		sprintf(logline, "! There %s %d missing character%s --- see log file\n",
			(missingcharacters == 1) ? "was" : "were", 	missingcharacters,
				(missingcharacters == 1) ? "" : "s");
		showline(logline, 0);
	}
	if (freememory() != 0) flag++;
/*	dumpaccess(); */
/*	show per page time also ? */
	if (verboseflag) {
/*		sprintf(logline, "start %ld main %ld finish %ld\n",
			starttime, maintime, finishtime); */
		showline("Total ", 0);
/*		showinterval(starttime, finishtime); */
		showinterval(finishtime - starttime);
		showline(" sec (", 0);
/*		showinterval(starttime, maintime); */
		showinterval(maintime - starttime);
		showline(" format load + ", 0);
/*		showinterval(maintime, finishtime); */
		showinterval(finishtime - maintime);
		showline(" processing) ", 0);
		if (totalpages > 0) {
/*			msec = (finishtime - maintime) * 1000 / (CLK_TCK * totalpages); */
/*			sprintf(logline, " %d.%d sec per page", msec / 1000, msec % 1000); */
/*			sprintf(logline, " %d.%03d sec per page", msec / 1000, msec % 1000); */
			showinterval ((finishtime - maintime) / totalpages);
			showline(" sec per page", 0);
		}
		showline("\n", 0);
	}

#ifdef ALLOWDEMO
	if (bDemoFlag) 
		if (dtime > (onemonth * 6))
			for (;;);	/* SERIOUSLY EXPIRED ! */
#endif
	checkpause(flag);
//	checkpause(1);
	return flag;
}

/********************************************************************************/

/* addition 98/Mar/31 print_csnames Frank Mittelbach */

int textcolumn;

#define MAXCOLUMN 78

void printcsname (FILE *output, int h) {
	int c, textof, n;
	char *s;
	
	textof = hash [ h ] .v.RH;
	if (textof == 0) return;	/* ignore if text() == 0 */
	n = strstart[textof+1] - strstart[textof];
	if (textcolumn != 0) {
		sprintf(logline, ", ");
		if (output != NULL) fprintf(output, logline);
		else showline(logline, 0);
		textcolumn += 2;
	}
	if (textcolumn + n + 2 >= MAXCOLUMN) {
		sprintf(logline, "\n");
		if (output == stderr) showline(logline, 1);
		else if (output == stdout) showline(logline, 0);
		else fputs(logline, output);
		textcolumn=0;
	}
	s = logline;
	for (c = strstart[textof]; c < strstart[textof+1]; c++) {
		*s++ = strpool[c];
	}
	if (output == stderr) showline(logline, 1);
	else if (output == stdout) showline(logline, 0);
	else fprintf(output, logline);
	textcolumn += n;
}

int comparestrn (int, int, int, int); /* in tex9.c */

/* compare two csnames in qsort */

int comparecs (const void *cp1, const void *cp2) {
	int c1, c2, l1, l2, k1, k2, textof1, textof2;
	c1 = *(int *)cp1;
	c2 = *(int *)cp2;
	textof1 = hash [ c1 ] .v.RH;
	textof2 = hash [ c2 ] .v.RH;
	l1 = ( strstart [ textof1  + 1 ] -
		   strstart [ textof1 ] ) ; 
	l2 = ( strstart [ textof2 + 1 ] -
		   strstart [ textof2 ] ) ; 
	k1 = strstart [ textof1 ] ;	
	k2 = strstart [ textof2 ] ;	
/*	showstring (k1, l1); */
/*	showstring (k2, l2); */
	return comparestrn (k1, l1, k2, l2);
}

char *csused=NULL;

/* Allocate table of indeces to allow sorting on csname */
/* Allocate flags to remember which ones already listed at start */

void printcsnames (FILE *output, int pass) {
	int h, k, ccount, repeatflag;
	int *cnumtable;
	int hash_base = 514;	/* 1 act base + 256 act char + 256 single char + 1 */
	int nfcs = hash_base + hash_size + hash_extra;	/* frozen_control_sequence */

	if (pass == 0 && csused == NULL) {
		csused = malloc (nfcs);
		if (csused == NULL) return; 
#ifdef USEMEMSET
		memset(csused, 0, nfcs); 
#else
		for (h = 0; h < (hash_size+780); h++) csused[h] = 0;
#endif
	}

	ccount=0;
	for (h = hash_base+1; h < nfcs; h++) {
		if (pass == 1 && csused[ h ]) continue;
		if ( hash [ h ] .v.RH != 0) {
			if (pass == 0) csused[ h] = 1;
			ccount++;
		}
	}

	sprintf(logline, "\n%d %s multiletter control sequences:\n\n",
			ccount, (pass == 1) ? "new" : "");
	if (output == stderr) showline(logline, 1);	
	else if (output == stdout) showline(logline, 0);	
	else fprintf(output, logline);

	if (ccount > 0) {	/* don't bother to get into trouble */
		textcolumn=0;
		cnumtable = (int *) malloc (ccount * sizeof(int));
		if (cnumtable == NULL) return;

		ccount=0;
/*		for (h = 515; h < (hash_size + 780); h++) { */
		for (h = hash_base+1; h < nfcs; h++) {
			if (pass == 1 && csused[ h ]) continue; 
			if ( hash [ h ] .v.RH != 0) cnumtable[ccount++] = h;
		}

		qsort ((void *)cnumtable, ccount, sizeof (int), &comparecs);

		repeatflag = 0;
		for (k = 0; k < ccount; k++) {
			h = cnumtable[ k ];
			if (pass == 1 && csused[ h ]) continue; 
			printcsname(output, h ) ;
		}
		sprintf(logline, "\n");
		if (output == stderr) showline(logline, 1);
		else if (output == stdout) showline(logline, 0);
		else fprintf(output, logline);
		free((void *)cnumtable);
	}

	if (pass == 1 && csused != NULL) {
		free(csused);
		csused = NULL;
	}
}

/***************** font info listing moved from TEX9.C ******************/

void showstring(int k, int l) {
	char *s=logline;
	while (l-- > 0) *s++ = strpool[k++];
	*s++ = ' ';
	*s = '\0';
	showline(logline, 0);
}

/* compare two strings in strpool (not null terminated) */
/* k1 and k2 are positions in string pool */
/* l1 and l2 are lengths of strings */

int comparestrn (int k1, int l1, int k2, int l2) {
	int c1, c2;
/*	while (l1-- > 0 && l2-- > 0) { */
	while (l1 > 0 && l2 > 0) {
		c1 = strpool [ k1 ];
		c2 = strpool [ k2 ];
/*		sprintf(logline, "%c%d%c%d ", c1, l1, c2, l2); */
		if (c1 > c2) return 1;
		else if (c2 > c1) return -1;
		l1--; l2--;
		k1++; k2++;
	}
	if (l1 > 0) return 1;		/* first string longer */
	else if (l2 > 0) return -1;	/* second string longer */
	return 0;					/* strings match */
}

/* compare two font names and their at sizes in qsort */

int comparefnt (const void *fp1, const void *fp2) {
	int f1, f2, l1, l2, k1, k2, s;
	f1 = *(short *)fp1;
	f2 = *(short *)fp2;
	l1 = ( strstart [ fontname [ f1 ] + 1 ] -
		   strstart [ fontname [ f1 ] ] ) ; 
	l2 = ( strstart [ fontname [ f2 ] + 1 ] -
		   strstart [ fontname [ f2 ] ] ) ; 
	k1 = strstart [ fontname [ f1 ] ] ;	
	k2 = strstart [ fontname [ f2 ] ] ;	
/*	showstring (k1, l1); */
/*	showstring (k2, l2); */
	s = comparestrn (k1, l1, k2, l2);
/*	sprintf(logline, "%d\n", s); */
	if (s != 0) return s;
	if (fontsize [ f1 ] > fontsize [ f2 ]) return 1;
	else if (fontsize [ f1 ] < fontsize [ f2 ]) return -1;
	return 0;					/* should not ever get here */
}

/* compare two font names */

int comparefntname (int f1, int f2) {
	int l1, l2, k1, k2, s;
	l1 = ( strstart [ fontname [ f1 ] + 1 ] -
		   strstart [ fontname [ f1 ] ] ) ; 
	l2 = ( strstart [ fontname [ f2 ] + 1 ] -
		   strstart [ fontname [ f2 ] ] ) ; 
	k1 = strstart [ fontname [ f1 ] ] ;	
	k2 = strstart [ fontname [ f2 ] ] ;	
/*	showstring (k1, l1); */
/*	showstring (k2, l2); */
	s = comparestrn (k1, l1, k2, l2);
/*	sprintf(logline, "%d\n", s); */
	return s;
}

/* decode checksum information */

unsigned long checkdefault = 0x59265920;	/* default signature */

int decodefourty(unsigned long checksum, char *codingvector) {
	int c;
	int k;
/*	char codingvector[6+1]; */

/*	if (checksum == checkdefault) { */
	if (checksum == 0) {
/*		strcpy(codingvector, "unknown"); */
		strcpy(codingvector, "unknwn");
		return 1;
	}
	else if ((checksum >> 8) == (checkdefault >> 8)) {	/* last byte random */
/*		strcpy (codingvector,  "native"); */	/* if not specified ... */
		strcpy (codingvector,  "fixed ");		/* if not specified ... */
		return 1;								/* no info available */
	}
	else {
		for (k = 0; k < 6; k++) {
			c = (int) (checksum % 40);
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
/*	sprintf(logline, "Reconstructed vector %s\n", codingvector); */
	return 0;					/* encoding info returned in codingvector */
}

double sclpnt (long x) {
	double pt;
	pt = (double) x / 65536.0;
	pt = (double) ((int) (pt * 1000.0 + 0.5)) / 1000.0;
	return (pt);
}

// Shows list of fonts in log file

void dvifontshow ( internalfontnumber f, int suppressname ) {
	int a, l, k, n, for_end;
	unsigned long checksum;
	char checksumvector[8];
	char buffer[32];

/*	fprintf (logfile, "DAMN! %d ", suppressname); */
/*	fprintf (logfile, "%d ", suppressname); */
/*	suppressname = 0; */
	putc(' ', logfile);
	if (suppressname == 0) {
		a = (strstart [ fontarea [ f ] + 1 ] - strstart [ fontarea [ f ] ]) ; 
		l = (strstart [ fontname [ f ] + 1 ] - strstart [ fontname [ f ] ]) ; 
		k = strstart [ fontarea [ f ] ] ;
		for_end = strstart [ fontarea [ f ] + 1 ] - 1;
		if ( k <= for_end) do {
			putc(strpool [ k ], logfile);
		} while ( k++ < for_end, stdout) ; 
		k = strstart [ fontname [ f ] ] ;
		for_end = strstart [ fontname [ f ] + 1 ] - 1;
		if ( k <= for_end) do {
			putc(strpool [ k ], logfile);
		} while ( k++ < for_end) ;
	}
	else a = l = 0;
	for (k = a+l; k < 16; k++) putc(' ', logfile);
	sprintf(buffer, "at %lgpt ", sclpnt(fontsize [ f ]));
	fputs(buffer, logfile);
//	fprintf(logfile, "at %lgpt ", sclpnt(fontsize [ f ]));
	if (suppressname == 0) {
		n = strlen(buffer);
//		n = strlen(logfile);
		for (k = n; k < 16; k++) putc(' ', logfile);
		checksum = (((fontcheck [ f ] .b0) << 8 | fontcheck [ f ] .b1) << 8 |
					fontcheck [ f ] .b2) << 8 | fontcheck [ f ] .b3;
		decodefourty(checksum, checksumvector);
		fprintf(logfile, "encoding: %s..", checksumvector);
	}
	putc('\n', logfile);
}

/* Allocate table of indeces to allow sorting on font name */

void showfontinfo (void) {
	int k, m, fcount, repeatflag;
	short *fnumtable;

	fcount=0;
	for (k = 1; k <= fontptr; k++)
		if ( fontused [ k ] ) fcount++;

	if (fcount == 0) return;	/* don't bother to get into trouble */

	fnumtable = (short *) malloc (fcount * sizeof(short));

/*	if (verboseflag) sprintf(logline, "\nUsed %d fonts:\n", fcount); */

	fprintf(logfile, "\nUsed %d font%s:\n",
			fcount, (fcount == 1) ? "" : "s");

	fcount=0;
	for (k = 1; k <= fontptr; k++) 
		if ( fontused [ k ] ) fnumtable[fcount++] = (short) k;

	qsort ((void *)fnumtable, fcount, sizeof (short), &comparefnt);

	repeatflag = 0;
	for (m = 0; m < fcount; m++) {
		if (m > 0) {
			if (comparefntname ( fnumtable[m-1], fnumtable[m]) == 0)
				repeatflag = 1;
			else repeatflag = 0;
		}
		dvifontshow ( fnumtable[ m ], repeatflag ) ;
	}

	free((void *)fnumtable);
}

////////////////////////////////////////////////////////////////////////////

// Here follows the new stuff for the DLL version

#ifdef _WINDOWS

int showlineinx=0;

#define SHOWLINEBUFLEN 256

char showlinebuf[SHOWLINEBUFLEN];

// char logline[MAXLINE];

#define WHITESPACE " \t\n\r"

HINSTANCE hInstanceDLL=NULL;		/* remember for this DLL */

/* This is the callback function for the EDITTEXT Control in CONSOLETEXT */

#define GET_WM_COMMAND_CMD(wParam, lParam)	(HIWORD(wParam))
#define GET_WM_COMMAND_ID(wParam, lParam)	(LOWORD(wParam))
#define GET_WM_COMMAND_HWND(wParam, lParam)	((HWND)lParam)

HWND hConsoleWnd=NULL;		/* Console Text Window Handle passed from DVIWindo */

void ClearShowBuffer (void) {
	showlinebuf[showlineinx++] = '\0';		// clear out accumulated stuff
	if (hConsoleWnd != NULL)
		SendMessage(hConsoleWnd, ICN_ADDTEXT, (WPARAM) showlinebuf, 0L);
	showlineinx = 0;
}

// communicate with DVIWindo (for yandytex.dll)

void showline (char *line, int errflag) {			/* 99/June/11 */
	int ret;

	if (IsWindow(hConsoleWnd) == 0) {		// in case the other end died
		sprintf(line, "NO CONSOLE WINDOW? %08X %s", hConsoleWnd, line);
		ret = MessageBox(NULL, line, "YandYTeX",
						 MB_ICONSTOP | MB_OKCANCEL | MB_TASKMODAL);
		hConsoleWnd = NULL;
//		abortflag++;						// kill job in this case ???
		return;
	}

	if (showlineinx > 0) ClearShowBuffer();

	if (hConsoleWnd != NULL)
		SendMessage(hConsoleWnd, ICN_ADDTEXT, (WPARAM) line, 0L);

	if (errflag) {
		errlevel++;
		ret =  MessageBox(NULL, line, "YandYTeX",
						  MB_ICONSTOP | MB_OKCANCEL | MB_TASKMODAL);
		if (ret == IDCANCEL) {
//			abortflag++;
			uexit(1);		// dangerous reentry possibility ?
		}
	}
}

//	Provide means for buffering up individual characters

void showchar (int chr) {
	if (showlineinx +2 >= SHOWLINEBUFLEN) ClearShowBuffer();
	showlinebuf[showlineinx++] = (char) chr;
	if (chr == '\n') ClearShowBuffer();
}

void winshow(char *line) {
	(void) MessageBox(NULL, line, "YandYTeX",
					  MB_ICONINFORMATION | MB_OK | MB_TASKMODAL);
}

void winerror (char *line) {
	int ret;
	ret = MessageBox(NULL, line, "YandYTeX",
					 MB_ICONSTOP | MB_OKCANCEL | MB_TASKMODAL);
	if (ret == IDCANCEL) abortflag++;
}

// argument info constructed from command line 

int xargc;

char **xargv=NULL;

// need to be careful here because of quoted args with spaces in them
// e.g. -d="G:\Program Files\Adobe\Acrobat\*.pdf"

int makecommandargs (char *line) {
	int xargc;
//	char *s, *t;
	unsigned char *s, *t;				// fix 2000 June 18

	if (line == NULL) return -1;		/* sanity check */

//	winerror(line);						// debugging only

//	s = strtok(line, WHITESPACE);
//	while (s != NULL) {					/* count arguments */
//		xargc++;
//		s = strtok(NULL, WHITESPACE);
//	}

	xargc = 0;
	s = line;
	while (*s != '\0') {
		while (*s <= 32 && *s > 0) s++;
		if (*s == '\0') break;
		t = s;
		while (*t > 32 && *t != '\"') t++;
		if (*t == '\"') {
			t++;
			while (*t > 0 && *t != '\"') t++;
			if (*t == '\0') break;
			t++;
		}
//		xargv[xargc] = s;
		xargc++;
		if (*t == '\0') break;
//		*t = '\0';
		s = t+1;
	}

	if (xargc == 0) return -1;			/* nothing to do */

	xargv = (char **) malloc(xargc * sizeof(char *));
	if (xargv == NULL) {
		sprintf(logline, "ERROR: Unable to allocate memory for %s\n", "arguments");
		winerror(logline);
		return -1;
	}

	xargc = 0;
	s = line;
	while (*s != '\0') {
		while (*s <= ' ' && *s > '\0') s++;	/* eat white space */
		if (*s == '\0') break;
		t = s;
		while (*t > ' ' && *t != '\"') t++;
		if (*t == '\"') {
			t++;
			while (*t > 0 && *t != '\"') t++;
			if (*t == '\0') break;
			t++;
		}
//		winerror(s);		// debugging only
		xargv[xargc] = s;
		xargc++;
		if (*t == '\0') break;
		*t = '\0';
		s = t+1;
	}

//	s = line;
//	for (k = 0; k < xargc; k++) {	/* create pointers to args */
//		while (*s > '\0' && *s <= ' ') s++;	/* eat white space */
//		xargv[k] = s;
//		s += strlen(s) +1;
//	}

#ifdef DEBUGGING
	s = logline;
	*s = '\0';
	for (k = 0; k < xargc; k++) {
		sprintf(s, "%d\t%s\n", k, xargv[k]);
		s += strlen(s);
	}
	winshow(logline);
#endif
	return xargc;
}

// refers to TeXAsk in dviwindo.c

// int (* AskUserCall) (char *, char *) = NULL;	// callback for user questions
int (* AskUserCall) (char *, char *, char *) = NULL;	// callback for user questions

// called from tex0.c only	---  by initterm and terminput

//int ConsoleInput (char *question, char *buffer) 
int ConsoleInput (char *question, char *help, char *buffer) {
	int ret=0;
//	char *s;
	if (AskUserCall == NULL) return 0;
//	sprintf(logline, "strstart %x %x\n", strstart, strstart [831]);
//	showline(logline, 1);

	*buffer = '\0';
	ret = AskUserCall (question, help, buffer);		// value returned by dialogbox
//	strcpy(buffer, "x");
//	strcat(buffer, " ");			// ???
//	sprintf(logline, "strstart %x %x\n", strstart, strstart[831]);
//	showline(logline, 1);
//	input_line_finish();			// ???
//	s = buffer + strlen(buffer);
//	*s++ = ' ';						// space terminate
//	*s++ = '\0';					// and null terminate
//	returning != 0 means EOF or ^Z
	return ret;
}

//	This is the new entry point of DLL called from DVIWindo 
//	ARGS: console window to send messages to, command line, callback fun
//	no console window output if hConsole is NULL
//	returns -1 if it fails --- returns 0 if it succeeds

// MYLIBAPI int yandytex (HWND hConsole, char *line, int (* AskUser) (char *, char *)) {
MYLIBAPI int yandytex (HWND hConsole, char *line, int (* AskUser) (char *, char *, char *)) {
	int flag;

	abortflag = 0;						// redundant
	hConsoleWnd = NULL;					// redundant

	AskUserCall = AskUser;				// remember callback

	hConsoleWnd = hConsole;				// remember console window handle

//	can't getenv("DEBUGPAUSE") cause setupdviwindo not called yet
//	if (grabenv("DEBUGPAUSE") != NULL) {
//		showline(line, 0);					// debugging - show command line
//		showline("\n", 0);
//	}

	xargc = makecommandargs(line);			// sets up global *xargv[]

	if (xargc < 0) return -1;				// sanity check

	if (hConsoleWnd != NULL) 
		SendMessage(hConsoleWnd, ICN_SETTITLE, (WPARAM) "YandYTeX", 0L);
//	SendMessage(hConsoleWnd, ICN_RESET, 0, 0L); // if want to clear window


	(void) main(xargc, xargv);	// now run YandYTeX proper in texmf.c 

	if (errlevel > 0 || abortflag > 0) {
//		sprintf(logline, "ERRORS in Processing (err %d abort %d)\n",
//				errlevel, abortflag);
//		winerror(logline);
	}

//	if (psbufpos > 0) sendpsbuffer(output);		// empty out PS buffer
//	if (psbufpos > 0) PSputs("", output);		// output already closed

	if (hConsoleWnd != NULL) {
		if (errlevel > 0 || abortflag > 0) flag = 1;
		else flag = 0;							// pass along error indication
		SendMessage(hConsoleWnd, ICN_DONE, flag, 0);	// flush out console buffer
	}
//	PScallback = NULL;
	hConsoleWnd = NULL;

	if (xargv != NULL) free(xargv);
	if (abortflag) return -1;
	else return 0;
}

BOOL WINAPI DllMain (HINSTANCE hInstDll, DWORD fdwReason, LPVOID fImpLoad) {

	switch (fdwReason) {
		case DLL_PROCESS_ATTACH:
			// The DLL is being mapped into the process's address space
			// place to allocate memory ???
			// return FALSE if this fails
			hInstanceDLL = hInstDll;		/* remember it */
			break;

		case DLL_THREAD_ATTACH:
			// A thread is being created
			break;

		case DLL_THREAD_DETACH:
			// A thread is exiting cleanly
			break;

		case DLL_PROCESS_DETACH:
			// The DLL is being unmapped from the process's address space
			// place to free any memory allocated
			// but make sure it in fact *was* allocated
			hInstanceDLL = NULL;		/* forget it */
			break;
	}
	return(TRUE);	// used only for DLL_PROCESS_ATTACH
}
#endif	// end of new stuff for DLL version

//////////////////////////////////////////////////////////////////////////////



/*  NOTE: currenttfm = false (-c)
	not checking for TFM in current directory saves 0.1 sec
	(0.2 if filemethod = false (-G) */

/*  NOTE: testdiraccess = false (-b):
	not checking whether readable file is a directory saves maybe 0.5 sec
	BUT only if filemethod = false (-G) - otherwise its irrelevant */

/*  NOTE: dirmethod = false (-D) --- method for checking whether directory
	using fopen instead of _findfirst in dir_p slows job maybe 0.05 sec
	BUT not if currenttfm = false (-c) */

/*  NOTE: filemethod = false (-G) --- method for checking file accessible
	using _access (readable) instead of _findfirst (file_p) costs 0.5 sec */

/*	Fast flag combinations: nothing, bG, bcG, bcDG */

/* constants for _heapchk/_heapset/_heapwalk routines */
/* #define _HEAPEMPTY	(-1) */
/* #define _HEAPOK 	(-2) */
/* #define _HEAPBADBEGIN	(-3) */
/* #define _HEAPBADNODE	(-4) */
/* #define _HEAPEND	(-5) */
/* #define _HEAPBADPTR	(-6) */

/* new arg -H=8 for horizontal tab *//* tabstep = 0 means no tab replacement */

/* buffercopy no longer used */

/* To Knuthian reset right when command line interpreted */

