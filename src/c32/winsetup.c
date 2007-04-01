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
/* creates winowner.txt containing owner, serial and time */

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

char *dviwindodir="d:\\winsourc";
/* following no longer relevant ... 96/Sep/12 */
/* char *dviwindoalt="c:\\windev\\samples\\dviwindo"; */
char *dviwindoalt1="d:\\yandy\\dviwindo";
char *dviwindoalt2="c:\\yandy\\dviwindo";

/* char *serialfile="c:\\windev\\samples\\dviwindo\\serial.num"; */
/* char *ownerfile="c:\\windev\\samples\\dviwindo\\winowner.txt"; */
/* char *magicfile="c:\\windev\\samples\\dviwindo\\winmagic.c"; */

char *serialfile="";
char *ownerfile="";
char *magicfile="";
char *exefile="";

char *author = "bkph";

unsigned short int cryptee; 	/* current seed for eexec encryption */
unsigned short int cryptch; 	/* current seed for charstring encryption */

static char line[MAXLINE];
static char owner[MAXLINE];
static char oldowner[MAXLINE];
static char serialtext[MAXLINE];
static char serialstring[MAXLINE];	/* 96/Oct/2 */
static char filetime[26];	/* 1995/May/30 */

char *outputdir="c:\\winsourc";	/* if directory to work in was specified */
								/* get serial.num from here also 96/Sep/12 */

char *outputexe="dviwindo.exe";	/* name of executable to alter 96/Sep/12 */

int PEstyle=0;				/* WIN32 (as opposed to WIN16) */
int stubflag = 1;			/* modify the stub of the EXE file */
int wanttimeflag = 0;		/* copy compile time if set */
int wantserialflag = 0;		/* add serial number to first line */
int usefiletimeflag = 1;	/* use creation time of EXE file */

int verboseflag = 1;
int traceflag = 0;
int serialflag = 0;			/* serial number from c:\yyinstal\serial.ini */
int debugflag=0;			/* non-zero => don't actually run make */

int showcreateflag = 1;
int showaccessflag = 0;
int showmodifyflag = 0;

int accentsok=0;			/* use Windows ANSI for accents directly */

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

/* MicroSoft Windows ANSI encoding */

char *ansinew[] = {
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle",
"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
"zero", "one", "two", "three", "four", "five", "six", "seven",
"eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
"at", "A", "B", "C", "D", "E", "F", "G",
"H", "I", "J", "K", "L", "M", "N", "O", 
"P", "Q", "R", "S", "T", "U", "V", "W", 
"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", 
"grave", "a", "b", "c", "d", "e", "f", "g", 
"h", "i", "j", "k", "l", "m", "n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w",
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "notdef",
"notdef", "notdef", "quotesinglbase", "florin", "quotedblbase", "ellipsis",
"dagger", "daggerdbl",
"circumflex", "perthousand", "Scaron", "guilsinglleft", "OE", "caron",
"notdef", "notdef",
"notdef", "quoteleft", "quoteright", "quotedblleft", "quotedblright",
"bullet", "endash", "emdash",
"tilde", "trademark", "scaron", "guilsinglright", "oe", "dotlessi", "notdef",
"Ydieresis",
"space", "exclamdown", "cent", "sterling", "currency", "yen", "brokenbar", "section",
"dieresis", "copyright", "ordfeminine", "guillemotleft", "logicalnot",
"hyphen", "registered", "macron",
"degree", "plusminus", "twosuperior", "threesuperior", "acute", "mu",
"paragraph", "periodcentered",
"cedilla", "onesuperior", "ordmasculine", "guillemotright", "onequarter",
"onehalf", "threequarters", "questiondown",
"Agrave", "Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE", "Ccedilla",
"Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave", "Iacute",
"Icircumflex", "Idieresis",
"Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply",
"Oslash", "Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute", "Thorn", "germandbls",
"agrave", "aacute", "acircumflex", "atilde", "adieresis", "aring", "ae", "ccedilla",
"egrave", "eacute", "ecircumflex", "edieresis", "igrave", "iacute",
"icircumflex", "idieresis",
"eth", "ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis", "divide",
"oslash", "ugrave", "uacute", "ucircumflex", "udieresis", "yacute", "thorn", "ydieresis", ""
};

/* used for stub only */

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
//	printf("%s is probably open, hence cannot be modified \n", "DVIWindo.exe");
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

int setupfilenames (char *directory, char *alternate) {
	FILE *input;
	char *s;

	if (traceflag) printf("Will now try and setup file names\n");
	if (strcmp(outputexe, "") != 0)
		exefile=makefilename(directory, outputexe);
	else exefile=makefilename(directory, "dviwindo.exe");
	if (traceflag) printf("Trying to open %s\n", exefile);
	if ((input = fopen(exefile, "rb")) == NULL) {
		perror(exefile);
		complain(exefile);
		directory = alternate;
		if (strcmp(outputexe, "") != 0)
			exefile=makefilename(directory, outputexe);
		else exefile=makefilename(directory, "dviwindo.exe");
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
		strcpy(altdirectory, "c:\\winsourc");
/*		strcat(altdirectory, s+strlen("releas12")); */
		serialfile = makefilename(altdirectory, "serial.num");
		ownerfile = makefilename(altdirectory, "winowner.txt");
		magicfile = makefilename(altdirectory, "winmagic.c");
	}
	else {
		serialfile = makefilename(directory, "serial.num");
		ownerfile = makefilename(directory, "winowner.txt");
		magicfile = makefilename(directory, "winmagic.c");
	}
/*	but use executables in specified directory when told */
	if (outputflag) directory = outputdir;
/*	exefile=makefilename(directory, "dviwindo.exe"); */
/*	use specified executable name if given */ /* redundant ? */
	if (strcmp(outputexe, "") != 0)
		exefile=makefilename(directory, outputexe);
	else exefile=makefilename(directory, "dviwindo.exe");
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

int lookupansi (char *charname) {
	int k;
	for (k = 32; k < 256; k++) {	/* NOT 265 */
		if (strcmp(ansinew[k], "") == 0) break;
		if (strcmp(ansinew[k], charname) == 0) return k;
	}
	return -1;
}

int lookup850 (char *charname) {
	int k;
	for (k = 32; k < 256; k++) {	/* NOT 265 */
		if (strcmp(dos850[k], "") == 0) break;
		if (strcmp(dos850[k], charname) == 0) return k;
	}
	return -1;
}

/* Accented characters are characters preceeded by accent */
/* ` grave, ' acute, ~ tilde, * ring, */
/*  " dieresis / cedilla, ^ circumflex / caron */
/* use "s or $ for germandbls */
/* ogonek, dotaccent, macron, hungarumlaut do not occur */
/* Use `` and '' to get quoteleft and quoteright  */

int makeansi(int base, int accent) {
	char charname[MAXCHARNAME];
	int k;

	strcpy(charname, ansinew[base]);
/*	quoting mechanism '' => ' and `` => `` and "" => " etc */
	if (base == accent) return base;			/* redundant */
	if (accent == '\"') {
		if (base == 's') strcpy(charname, "germandbls");
		else strcat(charname, "dieresis");
	}
	else if (accent == '~') strcat(charname, "tilde");	/* asciitilde */
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
	else if (accent == '\\') strcat(charname, "slash");
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
	else strcat(charname, ansinew[accent]);
/*	printf("CHARNAME: %s\n", charname); */
	k = lookupansi(charname);
	if (k < 0) printf("Can't find %s --- ", charname);
	return k;
}

int accentize (char *line) {
	char *s=line;
	char buffer[MAXLINE];
	int a, b, ansicode;
	int flag = 0;
	
	while ((s = strpbrk(s, "\"`'^~*\\|")) != NULL) {
		a = *s++;
/*		if (a == '$') {	
			ansicode = lookupansi("germandbls");
			*(s-1) = (char) ansicode;
			continue;
		} */
		b = *s++;
		if (a == b) {				/* deal with `` => ` and '' => ' etc. */
			s--;
			strcpy(s, s+1);			/* throw one copy away */
			if (a == '\'') *(s-1) = (char) lookupansi("quoteright");
			else if (a == '`') *(s-1) = (char) lookupansi("quoteleft");
			else if (a =='^') *(s-1) = (char) lookupansi("asciicircum");
			else if (a == '~') *(s-1) = (char) lookupansi("asciitilde");
			continue;
		}
		strcpy(buffer, s);			/* first save the rest of the string */
		*s = '\0';
		s = s - 2;
		if ((ansicode = makeansi(b, a)) >= 0) {
/*			printf("ANSI CODE: %d\n", ansicode); */
			if (accentsok) {
				*s++ = (char) ansicode;			/* 97/May/24 */
				*s = '\0';
			}
			else sprintf(s, "\\%d", ansicode);	/* the old way */
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
			if (serial < newserial) 
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

int insertstub (FILE *);

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
		fclose (output);
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
	if (stubflag) insertstub (output);

	fclose (output);

	if (copydate == 0) return 0;
	if (traceflag) printf("Modifying time and data of %s\n", exefile);
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

/* scan in next alphanumeric field */

int scaninnext (char *s, FILE *input, int n) {
	int c;
	char *start=s;							/* remember start */

	for (;;) {
		while ((c = getc(input)) == '\0') ;	/* step over null */
		ungetc(c, input);
		while ((c = getc(input)) != '\0' && c != EOF) {
			if (c < 32 || c > 127) {	/* it is junk ? */
				s = start;
				break;
			}
			*s++ = (char) c;
			if (s - start >= n) {
				*(start + n - 1) = '\0';
				return -1;
			}
		}
		if (c == EOF) {
			*s = '\0';
			return -1;
		}
		if (s > start) break;			/* did we make it ? */
		while ((c = getc(input)) != '\0' && c != EOF) ;	/* over junk */
		ungetc(c, input);
	}
	*s = '\0';
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* This is used to convert from Windows ANSI to DOS 850 for the stub */

void convert850(char *line) {					/* 1997/May/24 */
	char *s=line;
	int c, k;
	unsigned int m;
/*	printf("Before line: %s", line); */
	while ((c = *s) != '\0') {
		m = (unsigned char) c;
		if (m <= 32) {
			*s++;
			continue;
		}
		if (strcmp(ansinew[m], dos850[m]) != 0) {
			k = lookup850(ansinew[m]);
			if (k > 0) *s = (char) k;
			else printf("?%s (%u at %d)?\n", ansinew[m], m, s-line);
		}
		s++;
	}
/*	printf("Converted line: %s\n", line); */
}

int insertstub (FILE *input) {
/*	FILE *input; */
	char timebuffer[256];
	char stubbuffer[256];
	char *s;
	int c;
	long lpos;
	int nlen;
/*	char *dviwindo="DVIWindo"; */
/*	char *dviwindoc="dviwindo.c"; */
/* 	char *thisprogram="This program"; */

/*	if ((input = fopen(exefile, "rb+")) == NULL) {
		perror(exefile); return -1;
	} */
	fseek(input, 128, SEEK_SET);
	if (getc(input) == 'P' && getc(input) == 'E') {
		printf("PE style file (WIN32)\n");
		PEstyle = 1;
	}
	rewind(input);
/*	look for "dviwindo.c" of "dviwindo.C" --- date, time info follows */
/*	if (findpattern(input, dviwindoc) != 0) { */
	if (findpattern(input, "dviwindo.c", 1) != 0) {
		fprintf(stderr, "Magic string (%s) not found in %s\n",
			"dviwindo.c", exefile);
		complain("");
/*		fclose (input); */
		return -1;
	}
	while ((c = getc(input)) == '\0') ;	/* step over ^@^@ */
	ungetc(c, input);
	s = timebuffer;
/*	while ((c = getc(input)) != '\0' && c != EOF) {
		*s++ = (char) c; 
		if (s - timebuffer >= sizeof(timebuffer)) {
			fprintf(stderr, "Exeeded time buffer space\n");
			exit(1);
		}
	}
	*s = '\0'; */
	scaninnext(s, input, sizeof(timebuffer));
	if (debugflag) printf("DATEBUFFER: %s\n", timebuffer);	/* debugging */
	scivilize (timebuffer);
	while ((c = getc(input)) == '\0') ;					/* step over ^@^@ */
	ungetc(c, input);
	s = s + strlen(s);
	if (wanttimeflag) {
		*s++ = ' ';
/*		while ((c = getc(input)) != '\0' && c != EOF) {
			*s++ = (char) c;	
			if (s - timebuffer >= sizeof(timebuffer)) {
				fprintf(stderr, "Exeeded time buffer space\n");
				exit(1);
			}
		} */
		scaninnext(s, input, sizeof(timebuffer));
	}
	else  {
/*		while ((c = getc(input)) != '\0' && c != EOF) ;	*/ /* time */
		scaninnext(stubbuffer, input, sizeof(stubbuffer));	/* discard */
	}
/*	printf("TIMEBUFFER: %s\n", timebuffer); */			/* debugging */
	if (usefiletimeflag) {		/* 1995/May/30 */
		strcpy (timebuffer, filetime);
		if (!wanttimeflag) {
		s = timebuffer + strlen(timebuffer) - 1;
		while (s > timebuffer && *s != ' ') s--;/* trim off time 95/May/30 */
		*s = '\0';
		}
/*		s = timebuffer + strlen(timebuffer)-1;*/
	}
	if (debugflag) printf("DATEBUFFER: %s\n", timebuffer);	/* debugging */
	if (wantserialflag) {
		*s++ = ' ';
		*s = '\0';
		strcat (timebuffer, " SN ");		/* 95/May/30 */
		strcat (timebuffer, serialtext);	/* 95/May/30 */
		s = s + strlen(s);
	}
	*s++ = '\r';
	*s++ = '\n';
/*	*s++ = '$'; */
	*s++ = '\0';
	s = stubbuffer;
	while ((c = getc(input)) == '\0') ;
	ungetc(c, input);
/*	while ((c = getc(input)) != '\0' && c != EOF) {
		*s++ = (char) c;	
		if (s - stubbuffer >= sizeof(stubbuffer)) {
			fprintf(stderr, "Exceeded stub buffer size\n");
			exit(1);
		}
	} */ /* DVIWindo */
/*	scaninnext (s, input, sizeof(stubbuffer));
	if (strncmp(s, "DVIWindo", 8) != 0) {
		fprintf(stderr, "Failed to find %s, found %s\n", "DVIWindo", s);
		return -1;
	}  */
	if (findpattern(input, "DVIWindo", 0) != 0) {
		fprintf(stderr,
			"Magic string (%s) not found in %s\n", "DVIWindo", exefile);
		complain("");
		return -1;
	}
	strcpy (s, "DVIWindo"); 
	s = s + strlen(s);
	*s++ = ' ';
	while ((c = getc(input)) == '\0') ;
	ungetc(c, input);
/*	while ((c = getc(input)) != '\0' && c != EOF) {
		*s++ = (char) c;
		if (s - stubbuffer >= sizeof(stubbuffer)) {
			fprintf(stderr, "Exceeded stub buffer size\n");
			exit(1);
		}
	} */ 	/* 1.2.1 */
	scaninnext (s, input, sizeof(stubbuffer));
	if (strchr(s, '.') == NULL) {
		fprintf(stderr, "Failed to find version number (%s)\n", s);
		complain("");
		return -1;		
	}
	s = s + strlen(s);
	*s++ = ' ';
	*s++ = '\0';
	if (PEstyle == 0)
		strcat(stubbuffer, timebuffer);
	nlen = strlen(stubbuffer);
	if (traceflag)
		printf("STUBBUFFER (%d bytes): %s", nlen, stubbuffer);
	if (nlen >= 82) {			/* truncate the sucker ... */
		stubbuffer[81] = '$';
		stubbuffer[82] = '\0';
	}
	if (PEstyle && nlen >= 50) {
		stubbuffer[49] = '$';
		stubbuffer[50] = '\0';
	}
	sprintf(line, "%s %s %s", owner, serialtext, stime);
	convert850(line);					/* 1997/May/24 */
/*	Berthold K.P. Horn 69 1995 May 30 21:57:30 */
	s = line + strlen(line) - 1;
	while (s > line && *s != ' ') s--;	/* trim off time 95/May/30 */
	*s = '\0';
	strcat(stubbuffer, line);
	s = stubbuffer + strlen(stubbuffer);
	if (*(s-1) == '\n') s--;
	*s++ = '\r';
	*s++ = '\n';
	*s++ = '$';
	*s++ = '\0';
	nlen = strlen(stubbuffer);
/*	if (traceflag)*/
	printf("New Stub (%d bytes, max %d bytes):\n%s",
		   nlen, PEstyle ? 48 : 80, stubbuffer);
	if (nlen >= 82) {
/*		fclose(input); */
/*		return -1; */
		stubbuffer[81] = '$';
		stubbuffer[82] = '\0';		
	}
	if (PEstyle && nlen >= 50) {
		stubbuffer[49] = '$';
		stubbuffer[50] = '\0';
	}
	rewind (input);
/*	If program has not been customized yet, will find "This Program..." */
/*	if (findpattern(input, thisprogram) != 0) { */ /* requires Windows */
	if (findpattern(input, "This program", 0) != 0) { /* requires Windows */
		rewind (input);
/*		If program has been customized, will find "DVIWindo ..." */
/*		if (findpattern(input, dviwindo) != 0) { */
		if (findpattern(input, "DVIWindo", 0) != 0) {
			fprintf(stderr,
				"Magic string (%s or %s) not found in %s\n",
					"This program", "DVIWindo", exefile);
/*			fclose (input); */
			complain("");
			return -1;
		}
		else fseek (input, -8, SEEK_CUR);
		if (verboseflag) printf("Previously customized\n");
	}	
	else {
		if (verboseflag) printf("First time customization\n");
		fseek (input, -12, SEEK_CUR);
	}
	lpos = ftell(input);
	if ((lpos != 515 && lpos != 78) || traceflag)
		printf("Found at %ld bytes (expected 515)\n", lpos);
	if (lpos > 524) {							/* should be at 515 */
/*		fclose (input); */
		printf("Found string too far in - not safe to trust it...\n");
		return -1;
	}
	s = stubbuffer;
	while ((c = *s++) != '\0') putc(c, input);	/*overwrite stub string */	
/*	fclose (input); */
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *serialini="c:\\yyinstal\\serial.ini";

int readserial (void) {
	FILE *input;
	int flag = 0;
	char *s;
	
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
		else if (strcmp(argv[firstarg], "-s") == 0) stubflag = 0;
		else if (strncmp(argv[firstarg], "-o", 2) == 0) {
			outputflag = 1;				/* indicate non-standard directory */
			s = argv[firstarg] + 3;
			outputdir = s;
			if ((t = strrchr(s, '\\')) != NULL) {
				*t = '\0';
				outputexe = (t+1);
			}
/*			printf("Will modify EXE file in %s\n", s); */
			printf("Will modify EXE file %s in %s\n", outputexe, outputdir);
/*			allow accents in customization string for 32 bit version */
//			if (*outputdir == 'd') accentsok = 1;
//			else if (*outputdir == 'c') accentsok = 0;
			accentsok = 1;
		}
		firstarg++;
	}

	if ((s = getenv("DVIWINDODIR")) != NULL) dviwindodir = s;

	if (strcmp(outputdir, "") != 0) {	/* set up file names */
		if (setupfilenames(outputdir, dviwindoalt1) != 0) {
			if (setupfilenames(outputdir, dviwindoalt2) != 0) {
				exit(1);
			}
		}
	}
	else {								/* set up file names */
		if (setupfilenames(dviwindodir, dviwindoalt1) != 0) {
			if (setupfilenames(dviwindodir, dviwindoalt2) != 0) {
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
			complain("");
			return -1;
		}
		else if (sscanf(serialstring, "%d", &serialnumber) == 0) {
			printf("Sorry, do not understand serial number %s\n", serialstring);
			complain("");
			return -1;
		}
		else if (retrieveowner (serialnumber) == 0) {
			printf("Sorry, do not know serial number %d\n", serialnumber);
			complain("");
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
				complain("");
				return -1;
			}
		}
		else {		/* a number was specified */
			if (retrieveowner (serialnumber) == 0) {
				printf("Sorry, do not know serial number %d\n", serialnumber);
				complain("");
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
		complain("");
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
	counting=0;
	printf("SUCCESS\n");
	return 0;
}

/**************************************************************************/
