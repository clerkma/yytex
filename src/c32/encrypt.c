/* Copyright 1990,1991,1992,1993,1994,1995,1996,1997,1998,1999 Y&Y, Inc.
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

/****************************************************************************
* eexec encrypt string and add zeros at end								*
* Input in binary format, output in hex format								*
* Command line usage is:	encrypt <in-file> [<out-file>]					*
*		if <out-file> omitted, output is to stdout							*
****************************************************************************/

/* The following are for function prototypes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <time.h>					/* for CreationDate */

/* #define FNAMELEN 80 */
#define MAXLINE 512 
/* #define MAXLINE 1024 */
/* #define CHARSTRINGLEN 1024 */
/* #define CHARSTRINGLEN 2048 */
#define CHARSTRINGLEN 4096
#define MAXTOKEN 256
/* #define MAXTIMECAPSULE 512 */
#define MAXTIMECAPSULE 1024

/* #define VERSION "1.3" */
/* #define VERSION "1.4" */
/* #define VERSION "1.5" */
/* #define VERSION "1.5.1" */
/* #define VERSION "1.5.2" */
#define VERSION "1.5.3"

#define EXTRACHAR 4			/* required junk at beginning eexec */
#define LENIV 4				/* required junk at beginning CharString */

#define CRYPT_MUL 52845u	/* pseudo-random number generator constant */
#define CRYPT_ADD 22719u	/* pseudo-random number generator constant */

#define REEXEC 55665 		/* seed constant for eexec encryption */
#define RCHARSTRING 4330 	/* seed constant for charstring encryption */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Following are set from command line to control behaviour */

int eexecscan=0;		/* scan up to eexec before starting */
int charscan=0;			/* scan up to RD before starting */
int charenflag=0;		/* non-zero charstring coding instead of eexec */
int cformat=0;			/* use backslash before newline etc */
int datestamp=0;		/* insert date stamp in time capsule */
int wantzeros=1;		/* want zeros at end */
int flushcomment=1;		/* flush comment lines */ /* 1993/April/25 */

int hiddenchar=0;		/* 0 => zeros used for junk at start */
						/* 1 => use random initial bytes */
						/* 2 => use command line supplied string */
						/* 3 => use time capsule file */
						/* 4 => use signature */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int verboseflag=0;		
int traceflag=0;
int detailflag=0;

char timecapfile[FILENAME_MAX]=""; 	/* time capsule file name */
char *timecapptr=NULL;		/* pointer in the time capsule */

char *outputfile="";	/* output file name if specified */
char *starttext;		/* starting text to use */

int columns=78;			/* number of columns in output lines */

char rdsynon[8]="-|";	/* used for `currentfile exch readstring pop' */

int lenIV=LENIV;

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

static char timecapsule[MAXTIMECAPSULE];	/* place for time capsule */

int	charstring=0;	/* indicates whether we have hit charstrings */
int subrhit=0;		/* indicates whether we have hit subrs yet */

int codenum=0, oldlen=0;					/* 97/Jan/10 */

char opening[FILENAME_MAX]="";
char charnam[FILENAME_MAX]="";	/* 97/Jan/10 */

/***************************************************************************/

/* static unsigned char charstring[CHARSTRINGLEN];*/ /* place charstring */
/* static unsigned char crypstring[CHARSTRINGLEN];*/ /* place for encrypted */
/* unsigned char *charptr, *crypptr; */ /* pointers into above */

unsigned char encryptbyte (unsigned char plain, unsigned short *crypter)
{
	unsigned char cipher;
/*	cipher = (plain ^ (unsigned char) (*crypter >> 8)); */
	cipher = (unsigned char) ((plain ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return cipher;
}

unsigned char decryptbyte (unsigned char cipher, unsigned short *crypter)
{
	unsigned char plain;
/*	plain = (cipher ^ (unsigned char) (*crypter >> 8)); */
	plain = (unsigned char) ((cipher ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return plain;
} 

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

int getline(FILE *input, char buffer[]) {
	if (fgets(buffer, MAXLINE, input) == NULL) return 0;
	return strlen(buffer); 
} 

int scanuptoeexec(FILE *fp_in, FILE *fp_out) { /* return zero upon EOF */
	int n;

	while ((n = getline(fp_in, line)) != 0 && strstr(line, "eexec") == NULL) 
/*		fprintf(fp_out, "%s", line); */
		fputs(line, fp_out);
/*	if (n != 0) fprintf(fp_out, "%s", line); */
	if (n != 0) fputs(line, fp_out);
/*	else if (verboseflag) printf("EOF in scan up to eexec\n"); */
	else if (verboseflag) printf("EOF\n");
	return n;
}

void copyrestfile(FILE *fp_in, FILE *fp_out) { 
	int c;
	while ((c = getc(fp_in)) != EOF) putc(c, fp_out); 
}

#ifdef IGNORE
void extension(char *fname, char *ext)  /* supply extension if none present */
{
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) /* change extension if one present */
{
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

/* returns EOF if it hit EOF before "closefile" */

int unstraight(FILE *fp_in, FILE *fp_out) {
	int i, c;
	char *s=pstoken;
	char *t;

	clm = 0;
	if (charenflag != 0)
		cryptee = RCHARSTRING;  /* encryption seed for charstring coded */
	else cryptee = REEXEC;		/* encryption seed for eexec coded stuff */

	if (cformat != 0) {putc('"', fp_out); clm++;}
	if (hiddenchar == 0) {		/* use zeros */
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
				timecapptr = timecapsule;	// wrap around
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
/*		putc(c, fp_out); */
		if (c == EOF) {
			if (eexecscan) {				/* 97/July/20 */
				char *s="mark currentfile closefile\n";	/* \n ? */
				while ((c = *s++) != '\0') heputc(c, fp_out);
				c = EOF;
			}
		}
		else putc(c, fp_out);		/* typically \n */
/*		skip over white space following encrypted section ??? */
/*		while ((c = getc(fp_in)) != EOF && c <= ' ') ;
		ungetc(c, fp_in);  */ /* ?????????? */
	}
	return c;
}

/* end
   end
   readonly put
   noaccess put
   dup/FontName get exch definefont pop
   mark currentfile closefile */	/* normal end of encrypted section */

void nextout(int n) {			/* put encrypted byte in buffer */
	int d;
	d = encryptbyte((char) n, &cryptch);		/* encrypt it */
	crypstring[charinx++] = (char) d;	/* stick in buffer */
	if (charinx >= CHARSTRINGLEN) {
		fprintf(stderr, "ERROR: CharString too long (%d) ", charinx);
		if (charstring)
			printf("Character `%s' old length %d\n", charnam, oldlen);
		else
			printf("Subr %d old lenth %d\n", codenum, oldlen);
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

int getrealline(FILE *fp_in, char *line, int primed) {
	if (primed == 0) {
		if (getline(fp_in, line) == 0) return 0;		/* EOF */
	}
/*  flush blank lines, and comment lines if asked */
/*	while (*line == '\n' || *line == '%') */			/* ??? */
	while (*line == '\n' || 
		   strcmp(line, "\r\n") == 0 ||
		   (flushcomment != 0 && *line == '%')) {
		if (getline(fp_in, line) == 0) return 0;
	}
	return 1;
}

int encryptcore (FILE *fp_in) {
	int i, c;
	char *s;

/*	encryption seed for charstring coded */
	cryptch = RCHARSTRING;  
	charinx = 0; 
/*	then prime the pump with lenIV bytes  */ 
	if (hiddenchar == 0) for (i = 0; i < lenIV; i++) nextout(0);
	else if (hiddenchar == 1) for (i = 0; i < lenIV; i++) nextout(rand());
	else if (hiddenchar == 2) {
		s = starttext;	for (i = 0; i < lenIV; i++) nextout(*s++);
	}
	else if (hiddenchar == 3) {			/* use time capsule */
		for (i = 0; i < EXTRACHAR; i++) {
			if ((c = *timecapptr++) == 0) {	
				timecapptr = timecapsule;	// wrap around
				c = *timecapptr++;	
			}
			nextout(c);
		}
	}
	else {	/* shouldn't happen */
/*		nextout('B'); nextout('K'); nextout('P'); nextout('H'); */
		nextout('Y'); nextout('A'); nextout('N'); nextout('Y'); 
	}

	if (getrealline(fp_in, line, 0) == 0) {
		if (verboseflag) printf("EOF\n");
		return 0;	/* return if EOF */
	}
/*	if (getline(fp_in, line) == 0) return 0;
	while (*line == '\n' || 
		(flushcomment != 0 && *line == '%') || 
		strcmp(line, "\r\n") == 0)  {
		if (getline(fp_in, line) == 0) return 0;
	} */
	while (strncmp(line, "ND", 2) != 0 && 
		   strncmp(line, "|-", 2) != 0 &&
		   strncmp(line, "NP", 2) != 0 && 
		   strncmp(line, "noaccess put", 12) != 0 && 
		   strncmp(line, "end end", 7) != 0 &&	 /* 97/July/20 */
		   strncmp(line, "|", 1) != 0) {
/*	added following to deal with comments in weird places 1993/July/21 */
		if (getrealline(fp_in, line, 1) == 0) {
			fprintf(stderr, "Unexpected EOF in encrypted section\n");
			return 0;	/* return if EOF */
		}
		while (*line == '%') {
			if (getrealline(fp_in, line, 0) == 0) {
				fprintf(stderr, "Unexpected EOF in encrypted section\n");
				return 0;
			}
			if (traceflag) putc('%', stdout);
		}
/*		while (*line == '\n' ||
			(flushcomment != 0 && *line == '%') || 
			strcmp(line, "\r\n") == 0)
			if(getline(fp_in, line) == 0) return 0; */
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
		if (getrealline(fp_in, line, 0) == 0) {
			fprintf(stderr, "Unexpected EOF in encrypted section\n");
			return 0;	/* return if EOF */
		}
/*		if(getline(fp_in, line) == 0) return 0;
		while (*line == '\n' || 
			(flushcomment != 0 && *line == '%') || 
			strcmp(line, "\r\n") == 0) {
			if (getline(fp_in, line) == 0) return 0;
		} */
	}
	if (traceflag) printf("exiting on %s", line);
	return -1;	/* normal return */
}

/* pln => dec CharString + Subr section */

int charstrike(FILE *fp_in, FILE *fp_out) {
/*	int codenum, oldlen; */
/*	int	charstring=0; */	/* indicates whether we have hit charstrings */
/*	int subrhit=0; */		/* indicates whether we have hit subrs yet */
/*	char opening[FILENAME_MAX], charnam[FILENAME_MAX]; */
	char *s;
	
	charstring=0;
	subrhit=0;
	if (getrealline(fp_in, line, 0) == 0) return 0;	/* return if EOF */
/*	if (getline(fp_in, line) == 0) return 0;
	while (*line == '\n' || 
		(flushcomment != 0 && *line == '%') || 
		strcmp(line, "\r\n") == 0)  {
		if (getline(fp_in, line) == 0) return 0;
	} */
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
		if (getrealline(fp_in, line, 0) == 0) return 0;	/* return if EOF */
/*		if (getline(fp_in, line) == 0) return 0;
		while (*line == '\n' ||
			(flushcomment != 0 && *line == '%') ||
			strcmp(line, "\r\n") == 0) {
			if (getline(fp_in, line) == 0) return 0;
		} */
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

void decodeflag (int c) {
	switch(c) { 
		case 'v': if (verboseflag++ > 0) traceflag++; break;
		case '?': detailflag = 1; break;
		case 'n': flushcomment = 0; break;	
		case 'b': hiddenchar = 1; break;	
		case 'r': charscan = 1; eexecscan = 1;	/* => charstring encrypting */
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
}

/* \th: use special starting sequence\n\ */

void showusage(char *s) {
	fprintf (stderr, "\
Correct usage is:\n\
%s [-{v}{e}{c}{r}{z}{f}{d}{b}] [-s <start>]\n\
\t[-t <capsule>] [-o <out-file>] <in-file> \n", s);
	fprintf (stderr, "\
\tv: verbose mode\n\
\te: scan up to eexec before starting to encrypt\n\
\tc: use CharString encrypting instead of eexec\n\
\tr: scan up to successive RDs to encrypt CharStrings\n\
\tz: omit zeros at end\n\
\tf: create c-style character string\n\
\td: put time stamp in time capsule\n\
\tb: use random numbers for starting bytes\n\
\tn: do not flush comment lines\n\
\ts: next arg is string to use for starting bytes\n\
\tt: next arg is time capsule file\n\
\to: next arg is output file\n\
\t   (default current directory, same name, extension 'pfa' or 'enc')\n\
", columns);
	exit(1);
}

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

void padzeros(FILE *fp_out) {					/* 97/July/20 */
	int i, j;
	putc('\n', fp_out);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 64; j++) {
			putc('0', fp_out);
		}
		putc('\n', fp_out);
	}
	fprintf(fp_out, "cleartomark\n");
}

void encrypt(FILE *fp_out, FILE *fp_in) {
	if (eexecscan == 0) {
		unstraight(fp_in, fp_out);
		if (wantzeros != 0) addzeros(fp_out);
	}
	else if (charscan == 0) {			/* scan up to eexec and encrypt */
		scanuptoeexec(fp_in, fp_out);
/*		unstraight(fp_in, fp_out);	*/
		if (unstraight(fp_in, fp_out) == EOF) padzeros(fp_out); /* ??? */
		while (scanuptoeexec(fp_in, fp_out) != 0) {
			if (unstraight(fp_in, fp_out) == EOF) padzeros(fp_out);
		}
	}
	else {			/* scan up to CharStrings and encode and encrypt */
		scanuptoeexec(fp_in, fp_out);
		charstrike(fp_in, fp_out);
		while (scanuptoeexec(fp_in, fp_out) != 0) {
			charstrike(fp_in, fp_out);	
		}
	}
}

int main(int argc, char *argv[]) {       /* main program */
/* command line usage is: decrypt <in-file> <out-file> ... */
    FILE *fp_in=NULL, *fp_out=NULL; /* *fp_cod */
    char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	unsigned int i;
	int c, m, k, firstarg=1;
	char *s, *t;
	char *inmode="rb";
	time_t ltime; /* for time and date */

	if (argc < 2) showusage(argv[0]);

/*	while (argv[firstarg][0] == '-') */ /* check for command flags */
	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else decodeflag(c);
		}
		firstarg++;
		if(outflag != 0) {
			outputfile = argv[firstarg]; firstarg++; outflag = 0; 
		}
		if (startbyteflag != 0) {
			starttext = argv[firstarg]; firstarg++; startbyteflag = 0;
			hiddenchar = 2;
		}
		if (timecapflag != 0) {
			strcpy(timecapfile, argv[firstarg]);	/* 1993/April/16 */
			firstarg++; timecapflag = 0;
			hiddenchar = 3;
		}
		if (widthflag != 0) {
			if (sscanf(argv[firstarg], "%d", &columns) < 1) {
				fprintf(stderr, "Don't understand width: %s\n", 
					argv[firstarg]);
			}
			firstarg++; widthflag = 0;
		}		
	}

	if (detailflag != 0) showusage(argv[0]);

	if (argc < firstarg+1) showusage(argv[0]);

	if (strcmp(timecapfile, "") != 0) {
		extension(timecapfile, "txt");
		fp_in = fopen(timecapfile, "r");
		if (fp_in == NULL) {
			strcpy(line, "c:\\txt\\");
			strcat(line, timecapfile);
			strcpy(timecapfile, line);
			fp_in = fopen(timecapfile, "r");
			if (fp_in == NULL) {
				fprintf(stderr, "Can't open time capsule\n");
				perror(timecapfile);
				hiddenchar = 4;				// 99/Dec/10
			}
		}
		if (fp_in != NULL) {
			if (verboseflag) printf("Using %s\n", timecapfile);
			for (k = 0; k < MAXTIMECAPSULE; k++) {
				if ((c = getc(fp_in)) == EOF) {
					timecapsule[k] = '\0';
					break;
				}
				timecapsule[k]  = (char) c;
			}
			fclose(fp_in);
			timecapptr = timecapsule;
			if (datestamp != 0) {	/* insert date stamp */
				time(&ltime);		/* get seconds since 1970 */
				s = ctime(&ltime);
				lcivilize(s);
				t = timecapsule + 12;
				for (k=0; k < 21; k++) *t++ = *s++;
			}
		}
	}

	if (firstarg > argc) showusage(argv[0]);

	printf( "Font encryption program version %s\n",	VERSION);

	for (m = firstarg; m < argc; m++) {
		hires=0;					/* reset hybrid font flag 94/Jan/6 */
		strcpy(fn_in, argv[m]); 	/* get file name */
		if (charenflag != 0) extension(fn_in, "pln");
		else extension(fn_in, "dec");		/* extension if needed */
		
		if (verboseflag != 0) printf ("Input from %s ", fn_in);

		if (strcmp(outputfile, "") == 0) {
			if ((s=strrchr(fn_in, '\\')) != NULL) s++;
			else if ((s=strrchr(fn_in, ':')) != NULL) s++;
			else s = fn_in;
			strcpy(fn_out, s);  	/* copy input file name minus path */
/*			deliceflag = 0; */				/* raw bytes out */
			if (charenflag != 0) {
				if (strstr(fn_in, ".dec") != NULL)
					forceexten(fn_out, "enc");	/* avoid file name collision */
				else forceexten(fn_out, "dec"); 
			}
			else {
				if (strstr(fn_in, ".pfa") != NULL)
					forceexten(fn_out, "enc");	/* avoid file name collision */
				else forceexten(fn_out, "pfa");
			}
		}
		else {
			strcpy(fn_out, outputfile);
			if (charenflag != 0) extension (fn_out, "dec");
			else extension(fn_out, "pfa");
		}

		if (verboseflag != 0) printf("- output is going to %s\n", fn_out);

/* when CharString encrypting, read in text mode */
/* when eexec encrypting, read in binary mode 1996/Jan/15 */
		if (charenflag) inmode = "r";
		else inmode = "rb";
		
		if (fn_in[0] != '\0') {
/*			if ((fp_in = fopen(fn_in, "rb")) == NULL) { */
			if ((fp_in = fopen(fn_in, inmode)) == NULL) {
				perror(fn_in); exit(2);
			}
		}
		else {
			fprintf(stderr, "No input file specified\n");
			exit(3);
		}
		if (fn_out[0] != '\0') {
/*			if ((fp_out = fopen(fn_out, "w")) == NULL) {  */
			if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
				perror(fn_out); exit(2);
			}
		}
		else {
			fprintf(stderr, "No output file specified\n");
			exit(3);
		}		

/*		setbuf(fp_out, NULL); 	setbuf(stdout, NULL); */

		if (traceflag != 0) printf("Encryption attempt starting\n"); 

		encrypt(fp_out, fp_in);	/* go off and actually do the work */

		fclose(fp_in);
		if (ferror(fp_out) != 0) {
			fprintf(stderr, "Output error");
			perror(fn_out); exit(2);
		}
		fclose(fp_out);
	}
	return 0;
}	

#ifdef IGNORED

/*
 * Adobe Type 1 and Type 2 CharString commands; the Type 2 commands are
 * suffixed by "_2". Many of these are not used for computing the
 * weight vector but are listed here for completeness.
 */
#define HSTEM          1
#define VSTEM          3
#define VMOVETO        4
#define RLINETO        5
#define HLINETO        6
#define VLINETO        7
#define RRCURVETO      8
#define CLOSEPATH      9
#define CALLSUBR      10
#define RETURN        11
#define ESCAPE        12
#define HSBW          13
#define ENDCHAR       14

#define BLEND_2       16
#define HSTEMHM_2     18
#define HINTMASK_2    19
#define CNTRMASK_2    20

#define RMOVETO       21
#define HMOVETO       22

#define VSTEMHM_2     23
#define RCURVELINE_2  24
#define RLINECURVE_2  25
#define VVCURVETO_2   26
#define HHCURVETO_2   27
#define CALLGSUBR_2   29

#define VHCURVETO     30
#define HVCURVETO     31

/*
 * Adobe Type 1 and Type 2 CharString Escape commands; the Type 2
 * commands are suffixed by "_2"
 */
#define DOTSECTION       0
#define VSTEM3           1
#define HSTEM3           2

#define AND_2            3
#define OR_2             4
#define NOT_2            5

#define SEAC             6
#define SBW              7

#define STORE_2          8
#define ABS_2            9
#define ADD_2           10
#define SUB_2           11


#define DIV             12

#define LOAD_2          13
#define NEG_2           14
#define EQ_2            15

#define CALLOTHERSUBR   16
#define POP             17

#define DROP_2          18
#define PUT_2           20
#define GET_2           21
#define IFELSE_2        22
#define RANDOM_2        23
#define MUL_2           24
#define DIVX_2          25
#define SQRT_2          26
#define DUP_2           27
#define EXCH_2          28
#define INDEX_2         29
#define ROLL_2          30

#define SETCURRENTPOINT 33

#define HFLEX_2         34
#define FLEX_2          35
#define HFLEX1_2        36
#define FLEX1_2         37

#endif

/* most of this file is junk that can be flushed ! */

/* this program is rarely needed ? */

/* make those zeros at the end appear in groups of 64 */

/* problem if extra blank line between two CharStrings ? */

/* problem if blank line between last CharString and `end' ? */

/* Try and deal with T1UTILS style {...} grouping 95/Nov/17 */
/* T1UTILS style omits the zeros and the cleartomark at end ... */
