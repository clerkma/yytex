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

/* Enter owner name and serial number and generate date */
/* creates serial.num  to record serial number for future reference */
/* creates dviowner.txt containing owner, serial and time */
/* creates dvipream.enc from dvipream.ps */

#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#pragma warning(disable:4032)	// different type when promoted
#include <conio.h>
#pragma warning(default:4032)	// different type when promoted

/* following needed so can copy date and time from original to new file */

#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>

#define CRYPT_MUL 52845u	/* pseudo-random number generator constant */
#define CRYPT_ADD 22719u	/* pseudo-random number generator constant */

#define REEXEC 55665 		/* seed constant for eexec encryption */
#define RCHARSTRING 4330 	/* seed constant for charstring encryption */

#define MAXLINE 256
#define MAXCHARNAME 32
#define MAXOWNER 33			/* maximum length of owner's name */
/* #define MAXFILENAME 64 */
#define MAXFILENAME 128

/* #define MAXENCRYPTED 64 */		/* max space for encrypted data */
#define MAXENCRYPTED 96		/* max space for encrypted data */

char *dvipsonedir="d:\\dvisourc";
/* following no longer relevant ... 96/Sep/12 */
/* char *dvipsonealt="c:\\dvipsone"; */
char *dvipsonealt1="d:\\yandy\\dvipsone";
char *dvipsonealt2="c:\\yandy\\dvipsone";

char *serialfile="";
char *ownerfile="";
char *magicfile="";
char *exefile="";

char *preamblepsfile="";
char *preambleencfile="";
char *font3psfile="";
char *font3encfile="";
char *tpicspsfile="";
char *tpicsencfile="";

char *author = "bkph";

unsigned short int cryptee; 	/* current seed for eexec encryption */
unsigned short int cryptch; 	/* current seed for charstring encryption */

static char line[MAXLINE];
static char owner[MAXLINE];
static char oldowner[MAXLINE];
static char serialtext[MAXLINE];
static char serialstring[MAXLINE];	/* 96/Oct/2 */
static char filetime[26];	/* 1995/May/30 */

char *outputdir="c:\\dvisourc";	/* if directory to work in was specified */
								/* get serial.num from here also 96/Sep/12 */

char *outputexe="dvipsone.exe";	/* name of executable to alter 96/Sep/12 */

int usedos850 = 1;		/* use code page 850, otherwise use code page 437 */

int verboseflag = 1;
int traceflag = 0;
int serialflag = 0;			/* serial number from c:\yyinstal\serial.ini */
int debugflag=0;			/* non-zero => don't actually run make */

int dodllflag = 0;			/* DLL instead of EXE */

int showcreateflag = 1;
int showaccessflag = 0;
int showmodifyflag = 0;

int accentsok=0;			/* use DOS 850 accents directly */

int outputflag = 0;			/* flag that we do not want to work in standard */
int usebinflag = 1;
int maxcolumn = 76;
int binaryout = 0;
int copydate=1;				/* preserve date - time of original */

int clm;
int encbytes;
int counting=1;				/* are we counting encrypted bytes ? */
int serialnumber=0;
int oldserial=0;
int usingold=0;				/* don't add new record to serial.num */
int keepold=1;				/* comment out old version instead of flushing */

char *stime=NULL;

struct _stat statbuf;		/* struct stat statbuf;	 */

struct _utimbuf timebuf;	/* struct utimbuf timebuf; */

/* IBM OEM encoding (used by DOS) --- may be somewhat code page sensitive */

char *dos437[] = {
".notdef", "SS000000", "SS010000", "heart", "diamond", "club", "spade", "bullet", 
"SM570001", "SM750000", "SM750002", "male", "female", "musicalnote", "musicalnotedbl", "SM690000", 
"SM590000", "SM630000", "SM760000", "exclamdbl", "paragraph", "section", "SM700000", "SM770000", 
"arrowup", "arrowdown", "arrowright", "arrowleft", "SA420000", "arrowboth", "SM600000", "SV040000", 
"space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright", 
"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", 
"zero", "one", "two", "three", "four", "five", "six", "seven", 
"eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question", 
"at", "A", "B", "C", "D", "E", "F", "G", 
"H", "I", "J", "K", "L", "M", "N", "O", 
"P", "Q", "R", "S", "T", "U", "V", "W", 
"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "circumflex", "underscore", 
"quoteleft", "a", "b", "c", "d", "e", "f", "g", 
"h", "i", "j", "k", "l", "m", "n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w", 
"x", "y", "z", "braceleft", "brokenbar", "braceright", "tilde", "SM790000", 
"Ccedilla", "udieresis", "eacute", "acircumflex", "adieresis", "agrave", "aring", "ccedilla", 
"ecircumflex", "edieresis", "egrave", "idieresis", "icircumflex", "igrave", "Adieresis", "Aring", 
"Eacute", "ae", "AE", "ocircumflex", "odieresis", "ograve", "ucircumflex", "ugrave", 
"ydieresis", "Odieresis", "Udieresis", "cent", "sterling", "yen", "Pts", "florin", 
"aacute", "iacute", "oacute", "uacute", "ntilde", "Ntilde", "ordfeminine", "ordmasculine", 
"questiondown", "SM680000", "logicalnot", "onehalf", "onequarter", "exclamdown", "guillemotleft", "guillemotright", 
"SF140000", "SF150000", "SF160000", "SF110000", "SF090000", "SF190000", "SF200000", "SF210000", 
"SF220000", "SF230000", "SF240000", "SF250000", "SF260000", "SF270000", "SF280000", "SF030000", 
"SF020000", "SF070000", "SF060000", "SF080000", "SF100000", "SF050000", "SF360000", "SF370000", 
"SF380000", "SF390000", "SF400000", "SF410000", "SF420000", "SF430000", "SF440000", "SF450000", 
"SF460000", "SF470000", "SF480000", "SF490000", "SF500000", "SF510000", "SF520000", "SF530000", 
"SF540000", "SF040000", "SF010000", "SF610000", "SF570000", "SF580000", "SF590000", "SF600000", 
"alpha", "germandbls", "Gamma", "pi", "Sigma", "sigma", "mu", "tau", 
"Phi", "Theta", "Omega", "delta", "infinity", "phi", "epsilon", "intersection", 
"equivalence", "plusminus", "greaterequal", "lessequal", "SS260000", "SS270000", "divide", "approxequal", 
"degree", "SD630000", "SP320000", "radical", "nsuperior", "twosuperior", "filledbox", ".notdef", ""
};

char *dos850[] = {
".notdef", "smileface", "invsmileface", "heart", "diamond", "club", "spade", "bullet", 
"invbullet", "circle", "invcircle", "male", "female", "musicalnote", "musicalnotedbl", "sun", 
"triagrt", "triaglf", "arrowupdn", "exclamdbl", "paragraph", "section", "filledrect", "arrowupdnbse", 
"arrowup", "arrowdown", "arrowright", "arrowleft", "orthogonal", "arrowboth", "triagup", "triagdn", 
"space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright", 
"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", 
"zero", "one", "two", "three", "four", "five", "six", "seven", 
"eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question", 
"at", "A", "B", "C", "D", "E", "F", "G", 
"H", "I", "J", "K", "L", "M", "N", "O", 
"P", "Q", "R", "S", "T", "U", "V", "W", 
"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", 
"quoteleft", "a", "b", "c", "d", "e", "f", "g", 
"h", "i", "j", "k", "l", "m", "n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w", 
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "house", 
"Ccedilla", "udieresis", "eacute", "acircumflex", "adieresis", "agrave", "aring", "ccedilla", 
"ecircumflex", "edieresis", "egrave", "idieresis", "icircumflex", "igrave", "Adieresis", "Aring", 
"Eacute", "ae", "AE", "ocircumflex", "odieresis", "ograve", "ucircumflex", "ugrave", 
"ydieresis", "Odieresis", "Udieresis", "oslash", "sterling", "Oslash", "multiply", "florin", 
"aacute", "iacute", "oacute", "uacute", "ntilde", "Ntilde", "ordfeminine", "ordmasculine", 
"questiondown", "registered", "logicalnot", "onehalf", "onequarter", "exclamdown", "guillemotleft", "guillemotright", 
"ltshade", "shade", "dkshade", "2502", "2524", "Aacute", "Acircumflex", "Agrave", 
"copyright", "2563", "2551", "2557", "255d", "cent", "yen", "2510", 
"2514", "2534", "252c", "251c", "2500", "253c", "atilde", "Atilde", 
"255a", "2554", "2569", "2566", "2560", "2550", "256c", "currency", 
"eth", "Eth", "Ecircumflex", "Edieresis", "Egrave", "dotlessi", "Iacute", "Icircumflex", 
"Idieresis", "2518", "250c", "block", "dnblock", "brokenbar", "Igrave", "upblock", 
"Oacute", "germandbls", "Ocircumflex", "Ograve", "otilde", "Otilde", "mu", "thorn", 
"Thorn", "Uacute", "Ucircumflex", "Ugrave", "yacute", "Yacute", "macron", "acute", 
"sfthyphen", "plusminus", "underscoredbl", "threequarters", "paragraph", "section", "divide", "cedilla", 
"degree", "dieresis", "periodcentered", "onesuperior", "threesuperior", "twosuperior", "filledbox", "nbspace", ""
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void complain (char *fname) {
	printf("ERROR ERROR ERROR ERROR ERROR ERROR ERROR \n");
//	printf("%s is probably open, hence cannot be modified \n", "DVIPSONE.exe");
	printf("%s may be open, hence cannot be modified \n", fname);
	fflush(stdout);
	putc(7, stdout);
	fflush(stdout);
	(void) _getch(); 
}

/* Creates fully qualified file name from path and file name */

char *makefilename (char *directory, char *filename) {
	char str[MAXFILENAME];
	char *s;

	strcpy(str, directory);
	s = str + strlen(str) - 1;
	if (*s != '\\') strcat(str, "\\");
	strcat(str, filename);
	return _strdup(str);
}

// returns -1 if it can't find the file

int setupfilenames (char *directory, char *alternate) {
	FILE *input;
	char *s;

	if (traceflag) printf("Will now try and setup file names\n");
	if (strcmp(outputexe, "") != 0)
		exefile=makefilename(directory, outputexe);
	else exefile=makefilename(directory, "dvipsone.exe");
	if (traceflag) printf("Trying to open %s\n", exefile);
	if ((input = fopen(exefile, "rb")) == NULL) {
		perror(exefile);
		complain(exefile);
		directory = alternate;
		if (strcmp(outputexe, "") != 0)
			exefile=makefilename(directory, outputexe);
		else exefile=makefilename(directory, "dvipsone.exe");
		if (traceflag) printf("Trying to open %s\n", exefile);
		if ((input = fopen(exefile, "rb")) == NULL) {
			perror(exefile);
			complain(exefile);
			return -1;
		}
		if (traceflag) printf("Using secondary directory of %s\n", exefile);
	}
	else if (traceflag) printf("Using prime directory of %s\n", exefile);
	fclose (input);

/*	use data base files in the normal place ? */
/*	special case 1.1 => 1.2 upgrade call */
	if ((s = strstr(directory, "releas12")) != NULL) {
		char altdirectory[MAXFILENAME];
		strcpy(altdirectory, "c:\\dvisourc");
/*		strcpy(altdirectory+2, s+strlen("releas12")); */
		serialfile = makefilename(altdirectory, "serial.num");
		ownerfile = makefilename(altdirectory, "dviowner.txt");
		magicfile = makefilename(altdirectory, "dvimagic.c");
	}
	else {
		serialfile = makefilename(directory, "serial.num");
		ownerfile = makefilename(directory, "dviowner.txt");
		magicfile = makefilename(directory, "dvimagic.c");
	}
/*	but use executables in specified directory when told */
	if (outputflag) directory = outputdir;
/*	exefile=makefilename(directory, "dvipsone.exe"); */
/*	use specified executable name if given */ /* redundant ? */
	if (strcmp(outputexe, "") != 0)
		exefile=makefilename(directory, outputexe);
	else exefile=makefilename(directory, "dvipsone.exe");

	preamblepsfile=makefilename(directory, "dvipream.ps");
	preambleencfile=makefilename(directory, "dvipream.enc");
	font3psfile=makefilename(directory, "dvifont3.ps");
	font3encfile=makefilename(directory, "dvifont3.enc");
	tpicspsfile=makefilename(directory, "dvitpics.ps");
	tpicsencfile=makefilename(directory, "dvitpics.enc");
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	if (date == NULL) return;
	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
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

int getinfo(char *filename, int verboseflag) {
	char *s;
	int result;

	if (traceflag != 0)
		printf("Getting date and time for file %s\n", filename);
	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(stderr, "ERROR: Unable to obtain info on %s\n", filename);
		complain(filename);
		return -1;
	}
	s = ctime(&statbuf.st_ctime);
	if (traceflag) printf("CTIME (of ctime): %s", s);
	if (s == NULL) exit(1);
	lcivilize(s);
	if (traceflag || showcreateflag) printf("%s created:       %s", filename, s);
	s = ctime(&statbuf.st_atime);
	if (traceflag) printf("CTIME (of atime): %s", s);
	if (s == NULL) exit(1);
	lcivilize(s);
	if (traceflag || showaccessflag) printf("%s last accessed: %s", filename, s);
	s = ctime(&statbuf.st_mtime);
	if (traceflag) printf("CTIME (of mtime): %s", s);
	if (s == NULL) exit(1);
	lcivilize(s);
	if (traceflag || showmodifyflag) printf("%s last modified: %s", filename, s);
	strcpy (filetime, s);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned char encryptbyte (unsigned char plain, unsigned short *crypter) {
	unsigned char cipher;
/*	cipher = (plain ^ (unsigned char) (*crypter >> 8)); */
	cipher = (unsigned char) ((plain ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return cipher;
}

unsigned char decryptbyte (unsigned char cipher, unsigned short *crypter) {
	unsigned char plain;
/*	plain = (cipher ^ (unsigned char) (*crypter >> 8)); */
	plain = (unsigned char) ((cipher ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return plain;
} 

void writehex(FILE *output, int num) {	/* write two hex digits given char */
	int a, b;

	if (counting) {
		if (encbytes++ >= MAXENCRYPTED) return;
	}
	if (binaryout) {					/* new 1994/Dec/6 */
		putc(num, output);				/* just put the byte out ! */
/*		encbytes++; */
/*		if (encbytes >= MAXENCRYPTED) preventcorrupt();  */
		return;
	}								
	if (clm >= maxcolumn) {
		if (usebinflag != 0) putc('\\', output); 
		putc('\n', output); clm = 0;
	}
	if (num < 0 || num > 255) {		/* should never happen */
		fprintf(stderr, "\nHex num %d out of range", num);
		complain("");
	}
	if (usebinflag != 0) { /* for now */
		fprintf(output, "\\x"); clm = clm + 2;
	}
	a = (num >> 4);  b = (num & 017);
	if (a < 10) putc(a + '0', output);
	else putc(a + '7', output);
	if (b < 10)	putc(b + '0', output);
	else putc(b + '7', output);
	clm = clm + 2;
/*	encbytes++; */
/*	if (encbytes >= MAXENCRYPTED) preventcorrupt(); */
}

void encryptstring(FILE * output, char *s) {
	int c;
	if (s == NULL) {
		printf("Encryptstring %s\n", s);
		return;
	}
	while ((c = *s++) != '\0') 	
		writehex(output, (int) encryptbyte((unsigned char) c, &cryptee));
}

void encryptend(FILE * output) {
	writehex(output, (int) encryptbyte((unsigned char) 0, &cryptee));
}

void encryptcopy(FILE *output, FILE *input) {
	int c;
	while ((c = getc(input)) != EOF) {
		writehex(output, (int) encryptbyte((unsigned char) c, &cryptee));
	}
}

void writezeros(FILE *output) {
	int i, j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 64; j++) putc('0', output);
		putc('\n', output);
	}
}

void removecomments(char *s) {
	int c;
	while ((c = *s) != '\0') {
		if (c < ' ' || c == '%' || c == ';') {
			*s-- = '\0';
			while (*s == ' ') *s-- = '\0';
			break;
		}
		s++;
	}
}

void removenewline(char *s) {	/* no longer used */
	int c;
	c = *s++;
	while (c != '\0' && c != '\n' && c != '\r') c = *s++;
	*s--;
	*s = '\0';
}

int lookupibm (char *charname) {
	int k;
	for (k = 32; k < 256; k++) {	/* NOT 265 ! */
		if (usedos850) {
			if (strcmp(dos850[k], "") == 0) break;
			if (strcmp(dos850[k], charname) == 0) return k;
		}
		else {
			if (strcmp(dos850[k], "") == 0) break;
			if (strcmp(dos437[k], charname) == 0) return k;
		}
	}
	return -1;
}

/* accented characters are characters preceeded by accent */
/* ` grave, ' acute, ~ tilde, * ring, */
/* " dieresis / cedilla, ^ circumflex / caron */
/* use "s for germandbls */
/* ogonek, dotaccent, macron, hungarumlaut do not occur */
/* Use `` and '' to get quoteleft and quoteright  */

int makeibm(int base, int accent) {
	char charname[MAXCHARNAME];
	int k;

/*	DOS 437 versus DOS 850 shouldn't matter for base and accent ... */
/*	strcpy(charname, ibmoem[base]); */
	if (usedos850) strcpy(charname, dos850[base]);
	else strcpy(charname, dos437[base]);
/*	quoting mechanism '' => ' and `` => `` and "" => " etc */
	if (base == accent) return base;			/* redundant */
	if (accent == '\"') {
		if (base == 's') strcpy(charname, "germandbls");
		else strcat(charname, "dieresis");
	}
	else if (accent == '~') strcat(charname, "tilde");		/* asciitilde */
	else if (accent == '^') {
		if (base == 's' || base == 'S')	strcat(charname, "caron");
		else strcat(charname, "circumflex");	/* asciicircum */
	}
	else if (accent == '\'') strcat(charname, "acute");
	else if (accent == '`') strcat(charname, "grave");
	else if (accent == '*') {
		if (base == 'c' || base == 'C') strcat(charname, "cedilla");
		else strcat(charname, "ring");
	}
	else if (accent == '\\')  strcat(charname, "slash");
	else if (accent == '|') {
		if (base == 'a') strcpy(charname, "ae");
		else if (base == 'A') strcpy(charname, "AE");
		else if (base == 'o') strcpy(charname, "oe");
		else if (base == 'O') strcpy(charname, "OE");
		else if (base == 'd') strcpy(charname, "eth");
		else if (base == 'D') strcpy(charname, "Eth");
		else if (base == 'p') strcpy(charname, "thorn");
		else if (base == 'P') strcpy(charname, "Thorn");
		else strcat(charname, "bar");
	}
	else if (usedos850) strcat(charname, dos850[accent]);
	else strcat(charname, dos437[accent]);
/*	printf("CHARNAME: %s\n", charname); */
/*	But here, for accented/composite, DOS 437 versus DOS 850 does matter */
	k = lookupibm(charname);
	if (k < 0) printf("Can't find %s --- ", charname);
	return k;
}

int accentize (char *line) {
	char *s=line;
	char buffer[MAXLINE];
	int a, b, ibmcode;
	int flag = 0;
	
	while ((s = strpbrk(s, "\"`'^~*\\|")) != NULL) {
		a = *s++;
		b = *s++;
		if (a == b) {				/* deal with `` => ` and '' => ' etc. */
			s--;
			strcpy(s, s+1);			/* throw one copy away */
			if (a == '\'') *(s-1) = (char) lookupibm("quoteright");
			else if (a == '`') *(s-1) = (char) lookupibm("quoteleft");
			else if (a =='^') *(s-1) = (char) lookupibm("asciicircum");
			else if (a == '~') *(s-1) = (char) lookupibm("asciitilde");
			continue;
		}
		strcpy(buffer, s);			/* first save the rest of the string */
		*s = '\0';
		s = s - 2;
		if ((ibmcode = makeibm(b, a)) >= 0) {
/*			printf("IBM CODE: %d\n", ibmcode); */
			if (accentsok) {
				*s++ = (char) ibmcode;			/* 97/May/24 */
				*s = '\0';
			}
			else sprintf(s, "\\%d", ibmcode);
		}
		else {
			printf("(%c%c)\n", a, b);
			sprintf(s, "%c%c", a, b);
			flag++;
		}
		s = s + strlen(s);
		strcpy(s, buffer);
	}
/*	printf("CODE: %s\n", line); */
	return flag;
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

int retrieveowner (int number) {		/* Try and find old record */
	FILE *input;
	int oldserial, flag=0;
	char *s;

	if ((input = fopen(serialfile, "r")) == NULL) return -1;
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line < ' ') continue;
		if (sscanf(line, "%d", &oldserial) < 1) continue;
		if (oldserial == number) {
			if ((s = strchr(line, '\t')) != NULL)  {
				strcpy(owner, s+1);
/*				removenewline(owner); */
				removecomments(owner);
				printf("Will use `%s' as owner\n", owner);
				usingold = 1;
				flag = 1;
				break;
			}
		}
	}
	fclose (input);
	return flag;
}

int retrieveserial (char *name) {		/* Try and find old record */
	FILE *input;
	int oldserial, flag=0;
	char *s;

/* 	printf("Searching for %s\n", name);	*/		/* debugging */
	if ((input = fopen(serialfile, "r")) == NULL) return -1;
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line < ' ') continue;
		if (sscanf(line, "%d", &oldserial) < 1) continue;
		if (strstr(line, name) != NULL) {
			serialnumber = oldserial;
/*			printf("Have identified line: %s", line); */ /* debug */
			if ((s = strchr(line, '\t')) != NULL)  {
				strcpy(owner, line);
/*				strcpy(owner, s+1);	 */
				removecomments(owner);
				printf("Will use `%s' as owner\n", owner);
/*				usingold = 1; */
			}
			flag = 1;
			break;
		}
	}
	fclose (input);
	return flag;
}

int findlatest (void) {		/* Try and find latest record */
	FILE *input;
	int serial;
	char *s;

	oldserial = -1;
	if ((input = fopen(serialfile, "r")) == NULL) return -1;
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line < ' ') continue;
		if (sscanf(line, "%d", &serial) < 1) continue;
		if (serial > oldserial) {
			if ((s = strchr(line, '\t')) != NULL)  {
				oldserial = serial;
				strcpy(oldowner, s+1);
/*				removenewline(oldowner); */
				removecomments(oldowner);
/*				printf("Will use `%s' as owner\n", oldowner); */
/*				break; */
			}
		}
	}
	fclose (input);
	return oldserial;
}

void removepath (char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) ;
	else if ((s = strrchr(file, '/')) != NULL) ;
	else if ((s = strrchr(file, ':')) != NULL) ;
	else {
		fputs(file, stderr);
		complain(file);
		exit(1);
	}
	strcpy (file, s+1);
}

/* insert new record in appropriate place - replace old record if present */

int insertinfile (int newserial, char *stime) {/* Try and find latest record */
	FILE *input, *output;
	char serialback[MAXFILENAME];
	int serial;
/*	char *s; */

	if (traceflag) printf("Trying to open file %s for input\n", serialfile);
	if ((input = fopen(serialfile, "r")) == NULL) {
		perror(serialfile);
		complain(serialfile);
		if (traceflag) printf("Trying to open file %s for output\n", serialfile);
		if ((output = fopen(serialfile, "w")) == NULL) {
			perror(serialfile);
			complain(serialfile);
			return -1; 
		}
		else fclose(output);
/*		return -1; */
	}
	else fclose(input);
	
/*	if (verboseflag) printf ("Will now insert serial %d (%s %s) in %s\n",
							newserial, owner, stime, serialfile); */
	strcpy(serialback, serialfile);
	if (debugflag == 0) {
		forceexten(serialback, "bak");
		(void) remove(serialback);
		(void) rename(serialfile, serialback);
	}
	else removepath(serialfile);	/* output in current dir */

	if ((input = fopen(serialback, "r")) != NULL) {
		if ((output = fopen(serialfile, "w")) != NULL) {
			while (fgets(line, MAXLINE, input) != NULL) {
				if (*line != '%' && *line != ';' && *line >= ' ') {
					if (sscanf(line, "%d", &serial) == 1) {
/*						right place to insert new line ??? */
						if (serial >= newserial) {
							if (serial == newserial) {
								if (keepold != 0) {
/*									printf("Updating old entry: %s", line); */
									printf("Updating: %s", line);
									putc(';', output); putc(' ', output);
									fputs(line, output);
								}
/*								else printf("Replacing old entry: %s", line); */
								else printf("Replacing: %s", line); 
							}
							fprintf(output, "%d\t%s\t; %s",
								serialnumber, owner, stime);
							if (serial > newserial) fputs(line, output);
							break;
						}
					}
				}
				fputs(line, output);
			}
/*			if it wasn't inserted, then need to append it */
			if (serial < newserial)			/* has serial always been set ? */
				fprintf(output, "%d\t%s\t; %s", serialnumber, owner, stime); 

			while (fgets(line, MAXLINE, input) != NULL) 
				fputs(line, output);
			fclose(output);	
		}
		fclose (input);
	}
	return newserial;
}

void checkexit (char *str) {
	if (strncmp(str, "exit", 4) == 0 ||
		strncmp(str, "EXIT", 4) == 0) {
			printf("OK boss, I will exit!\n");
			exit(1);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/*	The character string is filled with: */
/*	(i) the pattern, i.e. "bkph" */
/*	(ii) four bytes, the encrypted serial number, most significant first */
/*	(iii) the encrypted owner serialtext and stime */

/* Need pattern length (4 bytes) - Serial number (4 bytes) */
/* Encrypt pump prime (4 bytes) - length owner + serialtext + time */ 

void doencrypted (FILE *output) {		/* common part */
 	cryptee = REEXEC; clm = 4;
/*	encbytes = 0; */
	encryptstring(output, author);
	encryptstring(output, owner);
	encryptstring(output, "@");			/* " " ? */
	encryptstring(output, serialtext);	
	encryptstring(output, "@");			/* " " ? */
	encryptstring(output, stime);
	encryptend(output);
	if (traceflag) printf("author `%s' ", author);
	if (verboseflag) printf("owner `%s' serial `%s' stime %s",
		owner, serialtext, stime);	/* debugging */
}

/* returns non-zero if it fails */

int findpattern (FILE *output, char *pattern, int insensitive) {
	int c, k=0;
	if (debugflag) printf("\nPATTERN: %s\n:", pattern);		/* debugging */
	while ((c = getc(output)) != EOF) {
/*		if (c == pattern[k]) { */
		if (c == pattern[k] || (insensitive &&
			((c >= 'A' && c <= 'Z' && c + 32 == pattern[k]) ||
			 (c >= 'a' && c <= 'z' && c == pattern[k] + 32)))) {
			if (k > 0) {
				if (debugflag)	putc('0'+k, stdout);		/* debugging */
			}
			k++;
			if (pattern[k] == '\0') {
				if (debugflag) putc('\n', stdout);			/* debugging */
				return 0;
			}
		}
		else k = 0;
	}
	return -1;
}

int domagicnew (void) {
	FILE *output;
	long encryptserial;
	int k;
	
	if (verboseflag) printf("Getting date and time on %s\n", exefile);
	if (getinfo(exefile, verboseflag) < 0) {
		exit(1);
	}
	if (traceflag) printf("Opening file: %s\n", exefile);
	output = fopen(exefile, "rb+");	/* need to be able to read and write */
	if (output == NULL) {
		perror(exefile);
		complain(exefile);
/*		fprintf(stderr, "Can't open %s\n", exefile); */
		exit(1);
	}
/*	rewind(output); */
/*	look for author signature - to be replaced with encrypted stuff */
/*	if (findpattern (output, author) != 0) { */
	if (findpattern (output, author, 0) != 0) {
		fprintf(stderr,
			"ERROR: Can't find magic pattern (%s) in EXE file\n",
				author);
		complain(exefile);
/*		return -1; */
		exit(1);
	}
	fseek(output, 0, SEEK_CUR);	/* required between in and out */
	encbytes = 4;				/* author marker which we step over */
	encryptserial = REEXEC * serialnumber;
	for (k = 0; k < 4; k++) {
		putc((char) (encryptserial >> 24), output);
		encryptserial = encryptserial << 8;
	}
	encbytes = encbytes+4;		/* for serial number */
/*	encbytes = 0; */
	binaryout = 1;
	if (accentsok) accentize(owner);					/* 97/May/24 */
/*	printf("%s\n", owner); */		/* debugging, remove later */
	doencrypted (output);
	if (encbytes >= MAXENCRYPTED) {
		fprintf(stderr,
			"ERROR: Exceeded byte limit on encrypted data %d > %d\n",
				encbytes, MAXENCRYPTED);
		fprintf(stderr, "ERROR: the EXE file may have been corrupted!\n");
		complain(exefile);
/*		exit(1); */
	}
	while (encbytes < MAXENCRYPTED) {	/* fill out with junk */
		encryptstring(output, " ");
	}
	binaryout = 0;
	if (traceflag) printf("Closing file %s\n", exefile);

	fclose (output);

	if (copydate == 0) return 0;
	if (traceflag) printf("Modifying time and date of %s\n", exefile);
	timebuf.actime = statbuf.st_atime;
/*	timebuf.modtime = statbuf.st_atime; */
	timebuf.modtime = statbuf.st_mtime;
/*	if (traceflag) if (getinfo(exefile, verboseflag) < 0) exit(1); */
	if (traceflag) printf("Trying to modify date in %s\n", exefile);
	if (_utime(exefile, &timebuf) != 0) {
		fprintf(stderr, "Unable to modify date/time\n");
		perror(exefile);
		complain(exefile);
/*			exit(3); */
	}
/*		see if it worked */
	if (getinfo(exefile, traceflag) < 0) exit(1);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void encryptfile (FILE *output, FILE *input) {
	int k;
	for (k = 0; k < 2; k++) { /* copy first two lines */
		fgets(line, MAXLINE, input); fputs(line, output);
	}
	rewind(input);
	fprintf(output, "currentfile eexec\n");
	cryptee = REEXEC; clm = 0;
	encryptstring(output, author);
	encryptstring(output, "% ");
	encryptstring(output, owner);
	encryptstring(output, " ");
	encryptstring(output, serialtext);	
	encryptstring(output, " ");
	encryptstring(output, stime);
	encryptcopy(output, input);
	encryptstring(output, "\n");
	encryptstring(output, "mark currentfile closefile\n");
	putc('\n', output);
	writezeros(output);
	fprintf(output, "cleartomark\n");
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int encryptheader(char *outfile, char *infile) {
	FILE *output, *input;

	usebinflag = 0; maxcolumn = 78;

/*	open preamble in text mode, so see only newlines, no returns */

	if ((input = fopen(infile, "r")) != NULL) {
		output = fopen(outfile, "w");
		encryptfile(output, input);
		fclose(input);
		fclose(output);
	}


	if (copydate == 0) return 0;
	if (getinfo(infile, verboseflag) < 0) {
		exit(1);
	}
	if (traceflag) printf("Modifying time and data of %s\n", outfile);
	timebuf.actime = statbuf.st_atime;
/*	timebuf.modtime = statbuf.st_atime; */
	timebuf.modtime = statbuf.st_mtime;
	if (traceflag) printf("Trying to modify date in %s\n", outfile);
	if (_utime(outfile, &timebuf) != 0) {
		fprintf(stderr, "Unable to modify date/time\n");
		perror(outfile);
		complain(outfile);
/*			exit(3); */
	}
	/*		see if it worked */
	if (getinfo(exefile, traceflag) < 0) exit(1);
	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *serialini="c:\\yyinstal\\serial.ini";

int readserial (void) {
	FILE *input;
	int flag = 0;
	char *s;
	
	if (traceflag) printf("Trying to open serial number file %s\n", serialini);
	if ((input = fopen(serialini, "r")) == NULL) {
		perror(serialini);
		complain(serialini);
		return 0;
	}
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line < ' ') continue;
		if (_strnicmp(line, "Serial=", 7) == 0) {
			s = line + strlen(line) - 1;
			while (s > 0 && *s <= ' ') *s-- = '\0';
			s = line+7;
			while (*s > 0 && *s <= ' ') s++;
			strcpy(serialtext, s);	/* not used at the moment */
			strcpy(serialstring, s);
			flag = 1;
/*			break; */
		}
		else if (_strnicmp(line, "Owner=", 6) == 0) {
			s = line + strlen(line) - 1;
			while (s > 0 && *s <= ' ') *s-- = '\0';
			s = line+6;
			while (*s > 0 && *s <= ' ') s++;
			strcpy(owner, s);		/* not used at the moment */
		}
	}
	fclose(input);
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	FILE *output;
	time_t ltime;		/* for time and date */
	int firstarg=1;
	char *s, *t;
/*	int namedflag=0; */
	int k;

/*	if (argc > 1) { */
	while (firstarg < argc && *argv[firstarg] == '-') {
		if (strcmp(argv[firstarg], "-d") == 0) debugflag = 1;
		else if (strcmp(argv[firstarg], "-v") == 0) verboseflag = 1;
		else if (strcmp(argv[firstarg], "-t") == 0) traceflag = 1;
		else if (strcmp(argv[firstarg], "-i") == 0) serialflag = 1;
		else if (strncmp(argv[firstarg], "-o", 2) == 0) {
			outputflag = 1;				/* indicate non-standard directory */
			s = argv[firstarg] + 3;
			outputdir = s;
			if ((t = strrchr(s, '\\')) != NULL) {
				*t = '\0';
				outputexe = (t+1);
			}
			if (strstr(outputexe, "dll") != NULL ||
				strstr(outputexe, "DLL") != NULL) {
				dodllflag = 1;
			}
/*			printf("Will modify EXE file in %s\n", s); */
			printf("Will modify %s file %s in %s\n",
				   dodllflag ? "DLL" : "EXE", outputexe, outputdir);
//			if (*outputdir == 'd') accentsok = 1;
//			else if (*outputdir == 'c') accentsok = 0;
			accentsok = 1;
		}
		firstarg++;
	}

	if ((s = getenv("DVIPSONEDIR")) != NULL) dvipsonedir = s;

	if (strcmp(outputdir, "") != 0) {	/* set up file names */
		if (setupfilenames(outputdir, dvipsonealt1) != 0) {
			if (setupfilenames(outputdir, dvipsonealt2) != 0) {
				exit(1);
			}
		}
	}
	else {								/* set up file names */
		if (setupfilenames(dvipsonedir, dvipsonealt1) != 0) {
			if (setupfilenames(dvipsonedir, dvipsonealt2) != 0) {
				exit(1);
			}
		}
	}

/*	usingold = 0; */
/*	oldserial = 0; */
	time(&ltime);				/* get seconds since 1970 */
/*	stime = ctime(&ltime); */
	stime = _strdup(ctime(&ltime));
	lcivilize(stime);
	if (traceflag != 0) printf("Time: %s", stime); 

/*	extract information from last line in file */
/*	(void) extractold(); */
	findlatest();

	strcpy(serialtext, "");

	if (serialflag) {				/* 96/Oct/2 */
		if (readserial() == 0) {	/* reads into serialstring */
			printf("Sorry, unable to read serial.ini\n");
			complain(serialini);
			return -1;
		}
		else if (sscanf(serialstring, "%d", &serialnumber) == 0) {
			printf("Sorry, do not understand serial number %s\n", serialstring);
			complain(serialini);
			return -1;
		}
		else if (retrieveowner (serialnumber) == 0) {
			printf("Sorry, do not know serial number %d\n", serialnumber);
			complain(serialini);
			return -1;
		}
		strcpy(owner, serialstring);
		goto given;
	}

	if (firstarg < argc) {		/* owner given on command line ? */
		strcpy(owner, argv[firstarg]);
/*		namedflag++; */
		if (sscanf(owner, "%d", &serialnumber) == 0) {
			if (retrieveserial (owner) == 0) {
				printf("Sorry, do not know a `%s'\n", owner);
/*				strcpy(owner, ""); */
				complain(serialini);
				return -1;
			}
		}
		else {		/* a number was specified */
			if (retrieveowner (serialnumber) == 0) {
				printf("Sorry, do not know serial number %d\n", serialnumber);
				complain(serialini);
				return -1;
			}
			else strcpy(owner, argv[firstarg]);
		}
		if (verboseflag != 0)
			printf("serialnumber %d owner %s\n", serialnumber, owner);
		goto given;			/* user name or number given on command line */
	}

loop:			/* loop until get information straight ... */

	strcpy(serialtext, "");

	for(;;) {
		printf("Please enter owner's name: ");
		if (oldserial > 0) printf("(last was: `%s')\n", oldowner);
		gets(owner);
given:
		checkexit(owner);

		if (sscanf(owner, "%d", &serialnumber) == 1) {
			strcpy(serialtext, owner);
			strcpy(owner, "");
			break;
		}
		if (strlen(owner) == 0) {
/*			strcpy(owner, oldowner); */	/* use old name */
/*			printf("Using `%s' as new owner's name\n", owner); */
/*			try and retrieve owner name later */
			break;
		}
		else if (strlen(owner) > MAXOWNER) {	 /* was 36, 32 */
			fprintf(stderr, 
				"Sorry, owner's name too long (%d > %d)\n",
					strlen(owner), MAXOWNER);
			for (k = 0; k < MAXOWNER; k++) putc(owner[k], stdout);
			putc('\n', stdout);				/* 1993/Nov/9 */
/*			exit(1); */
		}
		else {
			if (!accentsok) (void) accentize(owner);	/* 1992/Jan/03 */
			break;		/* name OK */
		}
	}

	if (strcmp(serialtext, "") != 0) goto skipserial;

	for(;;) {
		printf("Please enter owner's serial number: ");
/*		if (oldserial > 0) printf("(next is %d) ", oldserial+1); */
		if (oldserial > 0) printf("(last was: %d) ", oldserial);
		gets(serialtext);
		checkexit(serialtext);

/*		serialnumber = 0; */
		serialnumber = -1;
		if (sscanf(serialtext, "%d", &serialnumber) < 1) {
			if (strcmp(owner, "") == 0) {
				printf("Sorry, need to specify owner or serial number\n");
				exit(1);			/* changed his mind */
			}
		}
		if (serialnumber == 0) {
			printf("Sorry, serial number should not be zero\n");
			exit(1);			/* changed his mind */
		}
/*		if (serialnumber == 0) { */
		if (serialnumber < 0) {
			if(oldserial > 0) {
				serialnumber = oldserial;
				sprintf(serialtext, "%d", serialnumber);
				printf("Using %d as new serialnumber\n", serialnumber);
				break;
			}
			else {
				fprintf(stderr, 
					"Don't understand owner's serial number: %s\n", 
						serialtext);
			}
		}
		else break;		/* serial number and owner OK */
	}

skipserial:

	if (strcmp(owner, "") == 0) {
		if (retrieveowner (serialnumber) == 0) {
			printf("Sorry, no record for serial number %d\n", serialnumber);
			goto loop;
		}
	}

	if (traceflag) printf("Trying to open owner file %s\n", ownerfile);
	if ((output = fopen(ownerfile, "w")) == NULL) {
		perror(ownerfile);
		complain(ownerfile);
	}
	else {
		fprintf(output, "%s %s %s\n", owner, serialtext, stime);
		if (verboseflag != 0) printf("%s %s %s", owner, serialtext, stime);
		fclose(output);
	}
	
/*	if (usingold == 0) (void) addtofile();	*/	/* record new info */

/*	if (usingold == 0) (void) insertinfile(serialnumber, stime); */

/*	don't update serial.num if already done by cmbsetup.exe */

	if (serialflag == 0) (void) insertinfile(serialnumber, stime);

	if (debugflag != 0) {
		printf("Debug exit!\n");
		return 0;			/* if just debugging don't go on ! */
	}

	if (verboseflag != 0) printf("Encrypting owner, serial and time\n");

	counting=1;
#ifdef OLDCUSTOMIZE
	domagicold ();			/* split out now */
#else
	domagicnew ();
#endif
	if (! dodllflag) {
		counting=0;
		if (verboseflag != 0) printf("Encrypting preambles\n");

		encryptheader(preambleencfile, preamblepsfile);
		encryptheader(font3encfile, font3psfile);
		encryptheader(tpicsencfile, tpicspsfile);
	}

	return 0;
}

/**************************************************************************/

