/* encrypt.c
   Copyright Y&Y 1990, 1991, 1992
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

/* This is the MODULE - not the full program */

/****************************************************************************
*                                                                         	*
* Eeexec encrypt string and add zeros at end								*
*                                                                         	*
* Input in binary format, output in hex format								*
*                                                                         	*
* Command line usage is:	encrypt <in-file> [<out-file>]					*
*																			*
*		if <out-file> ommitted, output is to stdout							*
*																			*
****************************************************************************/

/* The following are for function prototypes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <time.h>					/* for Time Stamp */

#include "metrics.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* char *compilefile = __FILE__; */
/* char *compiledate = __DATE__; */
/* char *compiletime = __TIME__; */

/* Following are set from command line to control behaviour */

extern int eexecscan;		/* scan up to eexec before starting */
extern int charscan;		/* scan up to RD before starting */
extern int charenflag;		/* non-zero charstring coding instead of eexec */

int cformat=0;			/* use backslash before newline etc */
int datestamp=1;		/* insert date stamp in time capsule */
int wantzeros=1;		/* want zeros at end */
int flushcomment=1;		/* flush comment lines */ /* 1993/April/25 */

int hiddenchar=3;		/* 0 => zeros used for junk at start */
						/* 1 => use random initial bytes */
						/* 2 => use command line supplied string */
						/* 3 => use time capsule file */
						/* 4 => use signature */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

extern int verboseflag;		
extern int traceflag;
extern int detailflag;

char *timecapfile="";	/* time capsule file name */
char *timecapptr=NULL;	/* pointer in the time capsule */

char *starttext="Y&Y ";		/* starting text to use */

int columns=78;			/* number of columns in output lines */

char rdsynon[8]="-|";	/* used for `currentfile exch readstring pop' */

/* int deliceflag=1; */		/* non-zero to get rid of meta and control */
/* int flushcr=1; */	/* flush C-M C-J in encrypted part NO ! */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int startbyteflag=0;	/* next arg is starting junk text */
int timecapflag=0;		/* next arg is time capsule file name */
int widthflag=0;		/* next arg is columns per output line */
int outflag=0;			/* next arg is output file name */

int clm = 0;			/* current column in output */

int hires=0;			/* hybrid font indicator new 94/Jan/6 */

unsigned short int cryptee; 	/* current seed for eexec encryption */
unsigned short int cryptch; 	/* current seed for charstring encryption */

int charinx;					/* index into crypstring */

static unsigned char crypstring[CHARSTRINGLEN];	/*  encrypted version */

static char line[MAXLINE];		/* buffer for getline */

static char pstoken[MAXTOKEN];		/* place to accumulate tokens */

static char timecapsule[] = "\
Y&Y  Type 1\n\
01234567890123456789    \n\
Copyright (C) 1992-1998 Y&Y.  All rights reserved\n\
Do not copy or distribute without written permission.\n\
This font was converted using the\n\
`Font Manipulation Package' from:  Y&Y, Inc.\n\
\n\
http://www.YandY.com\n\
mailto:sales@YandY.com\n\
\n\
(978) 371-3286 (voice) (978) 371-2004 (fax)\n\
\n\
45 Walden Street, Concord, MA 01742 USA\n\
\n\
\n"; 


/* static unsigned char charstring[CHARSTRINGLEN];*/ /* place charstring */
/* static unsigned char crypstring[CHARSTRINGLEN];*/ /* place for encrypted */
/* unsigned char *charptr, *crypptr; */ /* pointers into above */

static unsigned char encryptbyte (unsigned char plain, unsigned short *crypter)
{
	unsigned char cipher;
/*	cipher = (plain ^ (unsigned char) (*crypter >> 8)); */
	cipher = (unsigned char) ((plain ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return cipher;
}

/* following function not used here ... */

#ifdef IGNORED
static unsigned char decryptbyte (unsigned char cipher, unsigned short *crypter)
{
	unsigned char plain;
/*	plain = (cipher ^ (unsigned char) (*crypter >> 8)); */
	plain = (unsigned char) ((cipher ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return plain;
}
#endif

void writehex(FILE *fp_out, int num)	/* write two hex digits given char */
{
	int a, b;
		if (num < 0 || num > 255) {		/* should never happen */
			fprintf(stderr, "\nERROR: Hex num %d out of range", num);
		}
/*		a = (num >> 4);  b = (num & 017); */
		a = (num >> 4);  b = (num & 15);
		if (a < 10) putc(a + '0', fp_out);
		else putc(a + '7', fp_out);
		if (b < 10)	putc(b + '0', fp_out);
		else putc(b + '7', fp_out);
}

static int getline(FILE *fp_in, char *line) {
	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	return strlen(line); 
} 

static int scanuptoeexec(FILE *fp_in, FILE *fp_out) { /* return zero upon EOF */
	int n;

	while ((n = getline(fp_in, line)) != 0 && strstr(line, "eexec") == NULL) 
/*		fprintf(fp_out, "%s", line); */
		fputs(line, fp_out);
/*	if (n != 0) fprintf(fp_out, "%s", line); */
	if (n != 0) fputs(line, fp_out);
	return n;
}

void copyrestfile(FILE *fp_in, FILE *fp_out) { 
	int c;
	while ((c = getc(fp_in)) != EOF) putc(c, fp_out); 
}

/* void extension(char *fname, char *ext)  {
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
} */

/* void forceexten(char *fname, char *ext) {
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);	
} */

void hputc(int c, FILE *fp_out) {
	int a, b;

	assert(c >= 0 && c <= 256);
	if (clm >= columns) {
		if (cformat != 0) putc('\\', fp_out);
		putc('\n', fp_out);	
		clm = 0;
	}
	clm = clm + 2;
/*	a = (c >> 4);  b = (c & 017); */
	a = (c >> 4);  b = (c & 15);
	if (a < 10) putc(a + '0', fp_out);
	else putc(a + 'A' - 10, fp_out);
	if (b < 10)	putc(b + '0', fp_out);
	else putc(b + 'A' - 10, fp_out);
}

void heputc(int c, FILE *fp_out) {
	hputc(encryptbyte((char) c, &cryptee), fp_out);
}

void unstraight(FILE *fp_in, FILE *fp_out) {
	int i, c;
	char *s=pstoken;
	char *t;

	clm = 0;
	if (charenflag != 0)
		cryptee = RCHARSTRING;  /* encryption seed for charstring coded */
	else cryptee = REEXEC;  /* encryption seed for eexec coded stuff */

	if (cformat != 0) {putc('"', fp_out); clm++;}
	if (hiddenchar == 0) { /* use zeros */
		for (i = 0; i < EXTRACHAR; i++) heputc(0, fp_out);
	}
	else if (hiddenchar == 1) { /* use random initial bytes */
		for (i = 0; i < EXTRACHAR; i++) heputc(rand(), fp_out);
	}
	else if (hiddenchar == 2) { /* use command line string */
		t = starttext;
		for (i = 0; i < EXTRACHAR; i++) heputc(*t++, fp_out);
	}
	else if (hiddenchar == 3) { /* use time capsule - assumes eexec */
		for (i = 0; i < EXTRACHAR; i++) {
			if ((c = *timecapptr++) == 0) {	
				timecapptr = timecapsule; 
				c = *timecapptr++;	
			}
			heputc(c, fp_out);
		}
	}
	else  {						/* not one of the above */
		if (charenflag != 0) {
			heputc(174, fp_out);	heputc(179, fp_out);
			heputc(232, fp_out);	heputc(75, fp_out);				
		}
		else {
			heputc(103, fp_out);	heputc(174, fp_out);
			heputc(114, fp_out);	heputc(136, fp_out);
		}
	}

	while ((c = getc(fp_in)) != EOF) {
/*      can't flush C-M C-J combination, because of binary stuff after RD ! */
/*		if (c == '\r' && flushcr != 0) {
			c = getc(fp_in);
			if (c != '\n') {
				(void) ungetc(c, fp_in);
				c = '\n';
			}
		} */
/*		can't safely turn C-M into C-J in encrypted part either */
		heputc(c, fp_out);
		if (c <= ' ') {
			*s = '\0';					/* terminate token */
			if (strcmp(pstoken, "closefile") == 0) {
/*				heputc('\n', fp_out); */		/* NO ??? */
				break;
			}
			s = pstoken;				/* start next token */
		}
		else {
			*s++ = (char) c;					/* build up token */
			if (s >= pstoken + MAXTOKEN) s--;
		}
	}
	if (cformat != 0) {
		putc('"', fp_out); clm++;
		putc(';', fp_out); clm++;
	}
	else {
/*		output terminating C-J ??? */
/*		if (c == '\r') c = '\n'; */
		putc(c, fp_out);
/*		skip over white space following encrypted section ??? *//* 92/Jun/28*/
		while ((c = getc(fp_in)) != EOF && c <= ' ') ; 
		ungetc(c, fp_in); 
	}
}

void nextout(int n) {			/* put encrypted byte in buffer */
	int d;
	d = encryptbyte((char) n, &cryptch);		/* encrypt it */
	crypstring[charinx++] = (char) d;	/* stick in buffer */
	if (charinx >= CHARSTRINGLEN) {
		fprintf(stderr, "ERROR: CharString too long (%d)\n", charinx);
		exit(17);
	}
}

void numencode(int num) {
	if (num >= -107 && num <= 107) nextout(num + 139); /* one byte */
	else if (num >= 108 && num <= 1131) {  /* two bytes */
		nextout(((num-108) >> 8) + 247); 
		nextout((num-108) &	0377);
	}
	else if (num >= -1131 && num <= -108) { /* two bytes */
		nextout(((-num-108) >> 8) + 251); 
		nextout((-num-108) & 0377);
	}
	else if (num > 1131) { /* five bytes */
		nextout(255); nextout(0); nextout(0);
		nextout(num >> 8); nextout(num & 0377);
	}
	else if (num < -1131) { /* five bytes */
		nextout(255); nextout(255); nextout(255);
		nextout(num >> 8); nextout(num & 0377);
	}
}

void lnumencode(long num) { /* version used for long numbers */
	if (num < 32767 && num > - 32767) numencode((int) num);
	else {
		nextout(255); /* indicate 5 byte coding used */
		nextout((int) ((num >> 24) & 0377));
		nextout((int) ((num >> 16) & 0377));
		nextout((int) ((num >> 8) & 0377));
		nextout((int) (num & 0377));
/*			printf(" %ld ", num); debugging */
	}
}

/* Adobe Font 1 CharString BuildChar Encoding: */

int wordencode(char *token) {
	int n;
	if(strcmp(token, "hstem") == 0) nextout(1);
	else if(strcmp(token, "vstem") == 0) nextout(3);
	else if(strcmp(token, "vmoveto") == 0) nextout(4);
	else if(strcmp(token, "rlineto") == 0) nextout(5);
	else if(strcmp(token, "hlineto") == 0) nextout(6);
	else if(strcmp(token, "vlineto") == 0) nextout(7);
	else if(strcmp(token, "rrcurveto") == 0) nextout(8);
	else if(strcmp(token, "closepath") == 0) nextout(9);
	else if(strcmp(token, "callsubr") == 0) nextout(10);
	else if(strcmp(token, "return") == 0) nextout(11);
	else if(strcmp(token, "escape") == 0) nextout(12);
	else if(strcmp(token, "hsbw") == 0) nextout(13);
	else if(strcmp(token, "endchar") == 0) nextout(14);
	else if(strcmp(token, "rmoveto") == 0) nextout(21);
	else if(strcmp(token, "hmoveto") == 0) nextout(22);
	else if(strcmp(token, "strokewidth") == 0) nextout(25);
	else if(strcmp(token, "baseline") == 0) nextout(26);
	else if(strcmp(token, "capheight") == 0) nextout(27);
	else if(strcmp(token, "bover") == 0) nextout(28);
	else if(strcmp(token, "xheight") == 0) nextout(29);
	else if(strcmp(token, "vhcurveto") == 0) nextout(30);
	else if(strcmp(token, "hvcurveto") == 0) nextout(31);
	else if(strncmp(token, "CODE-", 5) == 0) {			/* 1994/Jan/6 */
		fputs(token, stderr); putc('\n', stderr);
		if (sscanf(token, "CODE-%d", &n) > 0) nextout(n);
		else fputs(" SAY WHAT? ", stderr);
	}
	else {
		nextout(12);					/* escape */
		if(strcmp(token, "dotsection") == 0) nextout(0);
		else if(strcmp(token, "vstem3") == 0) nextout(1);
		else if(strcmp(token, "hstem3") == 0) nextout(2);
		else if(strcmp(token, "seac") == 0) nextout(6);
		else if(strcmp(token, "sbw") == 0) nextout(7);
		else if(strcmp(token, "div") == 0) nextout(12);
		else if(strcmp(token, "callothersubr") == 0) nextout(16);
		else if(strcmp(token, "pop") == 0) nextout(17);
		else if(strcmp(token, "setcurrentpoint") == 0) nextout(33);
		else if(strncmp(token, "ESCAPE-", 7) == 0) {			/* 1994/Jan/6 */
			fputs(token, stderr); putc('\n', stderr);
			if (sscanf(token, "ESCAPE-%d", &n) > 0) nextout(n);
			else fputs(" SAY WHAT?", stderr);
		}
		else {
			fprintf(stderr, "WARNING: Don't understand token: %s ", token);
			fprintf(stderr, "in line: %s", line); /* ??? */
			return -1;		/* error return */
/*			exit(15); */
		}
	}
	return 0;			/* normal return */
}

int encryptline(char *line) {
	long cord;
	int n;
	char token[MAXTOKEN];
	char *s=line;

	for (;;) {
		if (sscanf(s, "%ld%n", &cord, &n) == 1) {
			lnumencode(cord);
			s = s + n;
		}
		else if (sscanf(s, "%s%n", &token, &n) == 1) {
			if (wordencode(token) < 0) return -1;
			s = s + n;
		}
		else return 0;
	}
/*	return 0; */
}

void dumpbuffer(FILE *fp_out) {
	int k;
	for (k = 0; k < charinx; k++) {
		putc(crypstring[k], fp_out);
	}
}

/*
void removenewline(char *s) {
	int c;
	while ((c = *s) != 0) {
		if (c == '\n') {
			*s = '\0'; return;
		}
		else s++;
	}
} */

int encryptcore (FILE *fp_in) {
	int i, c;
	char *s;
/*	encryption seed for charstring coded */
	cryptch = RCHARSTRING;  
	charinx = 0; 
/*	then prime the pump with LENIV bytes  */ 
	if (hiddenchar == 0) for (i = 0; i < LENIV; i++) nextout(0);
	else if (hiddenchar == 1) for (i = 0; i < LENIV; i++) nextout(rand());
	else if (hiddenchar == 2) {
		s = starttext;	for (i = 0; i < LENIV; i++) nextout(*s++);
	}
	else if (hiddenchar == 3) {			/* use time capsule */
		for (i = 0; i < EXTRACHAR; i++) {
			if ((c = *timecapptr++) == 0) {	
				timecapptr = timecapsule; 
				c = *timecapptr++;	
			}
			nextout(c);
		}
	}
	else {	/* shouldn't happen */
/*		nextout('B'); nextout('K'); nextout('P'); nextout('H');  */
		nextout('Y'); nextout('A'); nextout('N'); nextout('Y'); 
	}

	if (getline(fp_in, line) == 0) return 0;		/* EOF */
/*  flush blank lines and comment lines */
/*	while (*line == '\n' || *line == '%') */				/* ??? */
	while (*line == '\n' || *line == '%' || strcmp(line, "\r\n") == 0) 
		if (getline(fp_in, line) == 0) return 0;
	while (strncmp(line, "ND", 2) != 0 && 
		strncmp(line, "|-", 2) != 0 &&
			strncmp(line, "NP", 2) != 0 && 
				strncmp(line, "noaccess put", 12) != 0 && 
					strncmp(line, "|", 1) != 0) {
/*	added following to deal with comments in weird places 1993/July/21 */
			while (*line == '\n' ||
				(flushcomment != 0 && *line == '%') || 
				strcmp(line, "\r\n") == 0)
				if(getline(fp_in, line) == 0) return 0;
/* Try and deal with T1UTILS styel termination 95/Nov/17 */
			if ((s = strchr(line, '}')) != NULL) {
				if (strncmp(line, "\t} ", 3) == 0) strcpy(line, line+3);
				else fprintf(stderr, "Unexpected `}' in %s\n", line);
				break;	/* now at | or |- --- end of group ... */
			}
			if (encryptline(line) < 0) {
				if(getline(fp_in, line) == 0) return 0;
				fprintf(stderr, "Next line: %s", line);
				return 0; /* error */ 
			}
			if(getline(fp_in, line) == 0) return 0; /* EOF */
/*			flush blank lines and comment lines */
/*			while (*line == '\n' || *line == '%') */	/* ??? */
			while (*line == '\n' || *line == '%' || strcmp(line, "\r\n") == 0)
				if (getline(fp_in, line) == 0) return 0;
	}
	return -1;	/* normal return */
}

/* pln => dec CharString + Subr section */

int charstrike(FILE *fp_in, FILE *fp_out) {
	int codenum, oldlen;
	int	charstring=0;	/* indicates whether we have hit charstrings */
	int subrhit=0;		/* indicates whether we have hit subrs yet */
	char opening[CHARNAME_MAX], charnam[CHARNAME_MAX];
	char *s;
	
	if (getline(fp_in, line) == 0) return 0;
/*  flush blank lines and comment lines */
/*	while (*line == '\n' || *line == '%') */			/* ??? */
	while (*line == '\n' || *line == '%' || strcmp(line, "\r\n") == 0) 
		if (getline(fp_in, line) == 0) return 0;
/*	while (strncmp(line, "end", 3) != 0) { */
	for (;;) {									/* CHANGED -  NEW */
		if (strstr(line, "hires") != NULL) hires++;	/* 94/Jan/6 */
/*		if (charstring != 0 && strncmp(line, "end", 3) == 0) break; */
		if (hires == 0) {						/* normal font */
			if (charstring != 0 && strncmp(line, "end", 3) == 0) break;
		}
		else {									/* hybrid font case */
			if (strncmp(line, "mark ", 5) == 0) {
				fprintf(stderr, "Unexpected `mark'?\n");
				break;	/* 94/Jan/6 */
			}
		}
		if (strncmp(line, "00000000", 8) == 0) {
			fprintf(stderr, "Unexpected `0000000000'?\n");
			break; /* emergency exit */
		}
/*		collect information on Subrs and CharStrings */
		if (strstr(line, "/Subrs") != NULL) subrhit++;
		if (strstr(line, "/CharStrings") != NULL) charstring++;
		if (subrhit == 0 && charstring == 0) {
			if (strncmp(line, "/RD{", 4) == 0) strcpy(rdsynon, "RD");
			if (strncmp(line, "/-|{", 4) == 0) strcpy(rdsynon, "-|");
		}
/*		Try and deal with T1UTILS style {...} grouping 95/Nov/17 */
		if ((s = strchr(line, '{')) != NULL) {
			if ((subrhit && strncmp(line, "dup ", 4) == 0) ||
				(charstring && *line == '/')) {
				strcpy(s, "0 ");
				strcat(s, rdsynon);
				strcat(s, "\n");
			}
		}
/*		Following should not happen at this level ... */
		else if ((s = strchr(line, '}')) != NULL) {
			if (strncmp(line, "\t} ", 3) == 0) strcpy(line, line+3);
			else if (subrhit) fprintf(stderr, "Unexpected `}' in %s", line);
		}
/*		is it a Subr ? */
		if (sscanf(line, "dup %d %d %s ", &codenum, &oldlen, &opening) == 3 &&
			(strcmp(opening, "RD") == 0 || strcmp(opening, "-|") ==	0)) {
			if (traceflag != 0) fputs(line, stdout);
			if (encryptcore(fp_in) == 0) return 0; /* EOF */
			fprintf(fp_out, "dup %d %d %s ", codenum, charinx, opening);
			dumpbuffer(fp_out); 
			putc(' ', fp_out);
/*			removenewline(line); */
/*			fprintf(fp_out, line); */
			fputs(line, fp_out);
		}
/*		is it a CharString ? */
		else if (sscanf(line, "/%s %d %s ", &charnam, &oldlen, &opening) == 3
			 && (strcmp(opening, "RD") == 0 || strcmp(opening, "-|") == 0)) {
			if (traceflag != 0) fputs(line, stdout);
			if (encryptcore(fp_in) == 0) return 0; /* EOF */
			fprintf(fp_out, "/%s %d %s ", charnam, charinx, opening);
			dumpbuffer(fp_out); 
			putc(' ', fp_out);
/*			removenewline(line); */
/*			fprintf(fp_out, line); */
			fputs(line, fp_out);
		}
		else {
			if (traceflag != 0) fputs(line, stdout);	/* 1992/Dec/5 */
/*			fprintf(fp_out, line); */	/* MISTAKE !!! */
			fputs(line, fp_out);
		}
		if (getline(fp_in, line) == 0) return 0;
/*		fputs(line, stdout);				*/		/* debugging */
/*		flush blank lines and comment lines ??? */
/*		while (*line == '\n' || *line == '%') */ /* ??? */
		while (*line == '\n' ||
			(flushcomment != 0 && *line == '%') ||
			strcmp(line, "\r\n") == 0) {
			if (getline(fp_in, line) == 0) return 0;
		}
/*		fputs(line, stdout);	*/				/* debugging */
	}
/*	fprintf(fp_out, line);		*/ /* print line containing "end" */
	fputs(line, fp_out);
	return -1;
/*	then copy the rest */
}

void addzeros(FILE *fp_out) {
	int k;
	for (k = 0; k < 512; k++) {
		if ((k % columns) == 0) putc('\n', fp_out);
		putc('0', fp_out);
	}
	putc('\n', fp_out);
}

/*		case 'h': hiddenchar = 4; break; */

/* void decodeflag (int c) {
	switch(c) { 
		case 'v': verboseflag = 1; break;
		case '?': detailflag = 1; break;
		case 'b': hiddenchar = 1; break;	
		case 'r': charscan = 1; eexecscan = 1;	
		case 'c': charenflag = 1; break;
		case 's': startbyteflag = 1; break;
		case 't': timecapflag = 1; break;
		case 'd': datestamp = 1; break;
		case 'w': widthflag = 1; break;
		case 'e': eexecscan = 1; break;
		case 'o': outflag = 1; break;
		case 'f': cformat = 1; columns = 64; break;
		case 'z': wantzeros = 0; break;
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
} */

/* \th: use special starting sequence\n\ */

/* void showusage(char *s) {
	fprintf (stderr, "\
Correct usage is:\n\
%s [-{v}{c}{e}{z}{f}{r}] [-s <start>]\n\
\t[-o <out-file>] <in-file> \n", s);
	fprintf (stderr, "\
\tv: verbose mode\n\
\te: scan up to eexec before starting to encrypt\n\
\tc: use CharString encrypting instead of eexec\n\
\tr: scan up to successive RDs to encrypt CharStrings\n\
\tz: omit zeros at end\n\
\tf: create c-style character string\n\
\tb: use random numbers for starting bytes\n\
\ts: next arg is string to use for starting bytes\n\
\to: next arg is output file\n\
\t   (default current directory, same name, extension 'pfa' or 'enc')\n\
", columns);
	exit(1);
} */

/* [-w <clmns>]  */

/* \tw: next arg is number of columns in output (default '%d')\n\ */

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

int encrypt(FILE *fp_out, FILE *fp_in) {
	time_t ltime; /* for time and date */
	int k;
	char *s, *t;

	if (datestamp != 0) { /* insert date stamp */
		time(&ltime); /* get seconds since 1970 */
		s = ctime(&ltime);
		lcivilize(s);
		t = timecapsule + 12;
		for (k=0; k < 21; k++) *t++ = *s++;  
	}
	timecapptr = timecapsule; 

	if (eexecscan == 0) {
		unstraight(fp_in, fp_out);
		if (wantzeros != 0) addzeros(fp_out);
	}
	else if (charscan == 0) {			/* scan up to eexec and encrypt */
		scanuptoeexec(fp_in, fp_out);
		unstraight(fp_in, fp_out);	
		while (scanuptoeexec(fp_in, fp_out) != 0) {
			unstraight(fp_in, fp_out);
		}
	}
	else {			/* scan up to CharStrings an encode and encrypt */
		scanuptoeexec(fp_in, fp_out);
		charstrike(fp_in, fp_out);
		while (scanuptoeexec(fp_in, fp_out) != 0) {
			charstrike(fp_in, fp_out);	
		}
	}
	return 0;
}

