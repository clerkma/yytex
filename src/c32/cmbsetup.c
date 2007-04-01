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

/* This one *only* modifies serial.num in d:\texsourc */
/* Then inserts same info into serial.num in d:\dvisourc and d:\winsourc */
/* Does *not* customize any files */
/* Writes serial.ini file */ /* new 1996/Sep/29 */
/* Still has a lot of old junk in it */

/* Enter owner name and serial number and generate date */
/* creates serial.num  to record serial number for future reference */
/* creates texowner.txt containing owner, serial and time */

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
/* #define MAXFILENAME 128 */

/* #define MAXENCRYPTED 64 */		/* max space for encrypted data */
#define MAXENCRYPTED 96		/* max space for encrypted data */

char *ynytexdir="c:\\texsourc";
/* following no longer relevant ... 96/Sep/12 */
char *ynytexalt="c:\\yandytex";

char *executable="yandytex.exe";
char *exealter="y&ytex.exe";

char *ownerfile="";
char *magicfile="";
char *exefile="";
char *serialfile="";		/* used as reference for input */

/* following are hard-wired here for the moment */
char *dviserialfile="d:\\dvisourc\\serial.num";		/* for dvipsone */
char *winserialfile="d:\\winsourc\\serial.num";		/* for dviwindo */
char *texserialfile="d:\\texsourc\\serial.num";		/* for yandytex */
char *customfile="c:\\yyinstal\\serial.ini";		/* for Installation Program */

char *author = "bkph";

unsigned short int cryptee; 	/* current seed for eexec encryption */
unsigned short int cryptch; 	/* current seed for charstring encryption */

char line[MAXLINE];				/* temp space for input line */

char *ownertext=NULL;
char *serialtext=NULL;
char *oldowner=NULL;
char *fontsets=NULL;
char *filetime=NULL;			/* 26 characters at most */

char *outputdir="d:\\texsourc";	/* if directory to work in was specified */
								/* get serial.num from here also 96/Sep/12 */

char *outputexe="yandytex.exe";	/* name of executable to alter 96/Sep/12 */

int usedos850 = 1;		/* use code page 850, otherwise use code page 437 */

int verboseflag = 1;
int traceflag = 0;
int debugflag = 0;			/* non-zero => don't actually run make */
int testkeyflag = 0;		/* non-zero => only testing key calculation */

int oldsysflag=0;			/* do 16 bit system */
int newsysflag=0;			/* do 32 bit system */

int accentsok=1;			/* use DOS 850 accents directly */

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

int monthx=0;				/* debugging month info */
int dayx=0;					/* debugging date info */
int serialx=0;				/* debugging serial number info */

long key;					/* installation key */

char *stime=NULL;

struct _stat statbuf;		/* struct stat statbuf;	 */

struct _utimbuf timebuf;	/* struct utimbuf timebuf; */

int winedtflag=1;			/* ask for winedt info (reset to zero by -e=PFE) */
long winedtmagic=0;			/* code supplied by Aleksander and Adriana */
long winedtcode=0;
long winedtinst=0;
long winedtzzxc=0;
long winedtzzxx=-14500006;
long winedtzzxy=0;
char *winedtdate=NULL;
char *winedtname=NULL;
char *winedtver="WinEdt 1.414";

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
"degree", "SD630000", "SP320000", "radical", "nsuperior", "twosuperior", "filledbox", ".notdef", 
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
"degree", "dieresis", "periodcentered", "onesuperior", "threesuperior", "twosuperior", "filledbox", "nbspace", 
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Creates fully qualified file name from path and file name */

char *makefilename(char *directory, char *filename) {
	char str[FILENAME_MAX];
	char *s;

	strcpy(str, directory);
	s = str + strlen(str) - 1;
	if (*s != '\\') strcat(str, "\\");
	strcat(str, filename);
	return _strdup(str);
}

int setupfilesub (char *directory, char *executable, char *exealter) {
	FILE *input;

	if (*outputexe != '\0')
		exefile=makefilename(directory, outputexe);
	else exefile=makefilename(directory, executable);
/*	if (traceflag) printf("Trying to open %s\n", exefile); */
	if ((input = fopen(exefile, "rb")) == NULL) {
		perror(exefile);
/*		directory = alternate; */
/*		exefile=makefilename(directory, "y&ytex.exe"); */
		if (*outputexe != '\0')
			exefile=makefilename(directory, outputexe);
		else exefile=makefilename(directory, exealter);
		if ((input = fopen(exefile, "rb")) == NULL) {
			perror(exefile);
			return -1;
		}
/*		if (traceflag) printf("Using secondary directory of %s\n", exefile); */
	}
/*	else if (traceflag) printf("Using prime directory of %s\n", exefile); */
	fclose (input);

/*	use data base files in the normal place ? */
	serialfile=makefilename(directory, "serial.num");
	ownerfile=makefilename(directory, "texowner.txt");
	magicfile=makefilename(directory, "texmagic.c");
/*	but use executables in specified directory when told */
	if (*outputexe != '\0')
		exefile=makefilename(directory, outputexe);
	else if (outputflag) {
		 exefile=makefilename(outputdir, executable); 
		 if ((input = fopen(exefile, "rb")) == NULL) {
/*			directory = alternate; */
/*			exefile=makefilename(directory, "y&ytex.exe"); */
			 exefile=makefilename(outputdir, exealter);
			 if ((input = fopen(exefile, "rb")) == NULL) {
				 perror(exefile);
				 return -1;
			 }
		 }
	}
	fclose (input);
	printf("Customizing %s\n", exefile);
/*	else exefile=makefilename(directory, executable); */
	return 0;
}

int setupfilenames (char *directory, char *alternate) {
	int ret;
/*	ret = setupfilesub (directory, alternate, exename); */
	ret = setupfilesub (directory, executable, exealter);
	if (ret == 0) return ret;
/*	ret = setupfilesub (directory, alternate, exealter); */
	ret = setupfilesub (alternate, exealter, executable);
	return ret;
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
		return -1;
	}
	s = ctime(&statbuf.st_atime);		/* ltime */
	if (traceflag != 0) printf("CTIME: %s", s);
	if (s == NULL) exit(1);
	lcivilize(s);
	if (verboseflag != 0) printf("%s last modified: %s", filename, s);
/*	strcpy (filetime, s); */
	filetime = _strdup(s);		/* but never used */
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
	for (k = 32; k < 265; k++) {
		if (usedos850) {
			if (strcmp(dos850[k], charname) == 0) return k;
		}
		else {
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

/* Watch out this can make the string longer - no longer used */

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

void extension (char *fname, char *str) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten (char *fname, char *str) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

int retrieveowner (int number) {		/* Try and find old record */
	FILE *input;
	int oldserial, flag=0, n;
	char *s;

/*	if ((input = fopen(serialfile, "r")) == NULL) return -1; */
	if ((input = fopen(texserialfile, "r")) == NULL) return -1;
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line <= ' ') continue;
		if (sscanf(line, "%d%n", &oldserial, &n) < 1) continue;
		if (*(line+n) == ' ') {
			printf("Character after serial is %d\n", *(line+n));	// debugging
			printf("%s", line);
			*(line+n) = '\t';	/* replace space with tab */
		}
		if (oldserial == number) {
			if ((s = strchr(line, '\t')) != NULL)  {
/*				strcpy(ownertext, s+1); */
/*				removenewline(ownertext); */
/*				removecomments(ownertext); */
				printf("OLD CUSTOMIZATION RECORD:\n");
				printf(line);		// show old customization
				removecomments(s+1);
				if (ownertext != NULL) free(ownertext);
				ownertext = _strdup(s+1);
				printf("Will use `%s' as owner\n", ownertext);
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
	int oldserial, flag=0, n;
	char *s;

/* 	printf("Searching for %s\n", name);	*/		/* debugging */
/*	if ((input = fopen(serialfile, "r")) == NULL) return -1; */
	if ((input = fopen(texserialfile, "r")) == NULL) return -1;
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line <= ' ') continue;
		if (sscanf(line, "%d%n", &oldserial, &n) < 1) continue;
		if (*(line+n) == ' ') {
			printf("Character after serial is %d\n", *(line+n));	// debugging
			printf("%s", line);
			*(line+n) = '\t';	/* replace space with tab */
		}
		if (strstr(line, name) != NULL) {
			serialnumber = oldserial;
/*			printf("Have identified line: %s", line); */ /* debug */
			if ((s = strchr(line, '\t')) != NULL)  {
/*				strcpy(ownertext, line); */
/*				strcpy(ownertext, s+1);	 */
/*				removecomments(ownertext); */
				printf("OLD CUSTOMIZATION RECORD:\n");
				printf(line);		// show old customization
				removecomments(s+1);
				if (ownertext != NULL) free(ownertext);
				ownertext = _strdup(s+1);
				printf("Will use `%s' as owner\n", ownertext);
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
	int serial, n;
	char *s;

	oldserial = -1;
/*	if ((input = fopen(serialfile, "r")) == NULL) return -1; */
	if ((input = fopen(texserialfile, "r")) == NULL) return -1;
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line <= ' ') continue;
		if (sscanf(line, "%d%n", &serial, &n) < 1) continue;
		if (*(line+n) == ' ') {
			printf("Character after serial is %d\n", *(line+n));	// debugging
			printf("%s", line);
			*(line+n) = '\t';	/* replace space with tab */
		}
		if (serial > oldserial) {
			if ((s = strchr(line, '\t')) != NULL)  {
				oldserial = serial;
/*				strcpy(oldowner, s+1); */
				oldowner = _strdup(s+1);	/* but never used */
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
		exit(1);
	}
	strcpy (file, s+1);
}

void spitoutuserline (FILE *output, int serialnumber, char *ownertext,
					 char *stime, long key, char *fontsets) {
	char buffer[MAXLINE];
	char *s;
	fprintf(output, "%d\t%s\t; %s ; %ld", serialnumber, ownertext, stime, key);
/*	if (fontsets != NULL) fprintf(output, " %s", fontsets); */
	if (fontsets != NULL) fprintf(output, " (%s)", fontsets);
	else fprintf(output, " (UNKNOWN)");
	if (winedtflag && winedtname != NULL && winedtmagic != 0) {
		strcpy(buffer, winedtname);
		if ((s = strstr(buffer, " (YandY TeX)")) != NULL) {
			strcpy(s, s + strlen(" (YandY TeX)"));	/* remove the (YandY TeX) */
		}
		fprintf(output, " \"%s\" %ld", buffer, winedtmagic);
	}
	fprintf(output, "\n");
}

/* insert new record in appropriate place - replace old record if present */

/* Try and find latest record */

/* Now we pass in the serial.num file name */

int insertinfile (int newserial, char *stime, char *serialfile) {
	FILE *input, *output;
	char serialback[FILENAME_MAX];
	int serial=-1;
/*	char *s; */

	if ((input = fopen(serialfile, "r")) == NULL) {
		perror(serialfile);
		if ((output = fopen(serialfile, "w")) == NULL) {
			perror(serialfile);
			return -1; 
		}
		else fclose(output);
/*		return -1; */
	}
	else fclose(input);
	
	if (verboseflag)
/*		printf ("Will now insert serial %d (%s %s) in %s\n", */
		printf ("Serial %d (%s %s) -> %s\n",
				newserial, ownertext, stime, serialfile);
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
									printf("Updating old entry: %s", line);
									putc(';', output); putc(' ', output);
									fputs(line, output);
								}
								else printf("Replacing old entry: %s", line);
							}
							spitoutuserline(output, serialnumber, ownertext, stime, key, fontsets);
							if (serial > newserial) fputs(line, output);
							break;
						}
					}
				}
				fputs(line, output);
			}
			if (serial == -1) printf("ERROR in insertinfile\n");
/*			if it wasn't inserted, then need to append it */
			if (serial < newserial)  /* has serial been setup ? */
				spitoutuserline(output, serialnumber, ownertext, stime, key, fontsets);

			while (fgets(line, MAXLINE, input) != NULL) fputs(line, output);
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
	if (ownertext == NULL || serialtext == NULL) {
		printf("ERROR!");
		exit(1);
	}
 	cryptee = REEXEC; clm = 4;
/*	encbytes = 0; */
	encryptstring(output, author);
	encryptstring(output, ownertext);
	encryptstring(output, "@");			/* " " ? */
	encryptstring(output, serialtext);	
	encryptstring(output, "@");			/* " " ? */
	encryptstring(output, stime);
	encryptend(output);
	if (traceflag) printf("author `%s' ", author);
	if (verboseflag) printf("owner `%s' serial `%s' stime %s",
		ownertext, serialtext, stime);	/* debugging */
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

#ifdef IGNORED
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
/*	if (!accentsok) accentize(ownertext); */ /* ??? */
	if (accentsok) accentize(ownertext);	/* wrong ? */
/*	printf("%s\n", ownertext); */		/* debugging, remove later */
	doencrypted (output);
	if (encbytes >= MAXENCRYPTED) {
		fprintf(stderr,
			"ERROR: Exceeded byte limit on encrypted data %d > %d\n",
				encbytes, MAXENCRYPTED);
		fprintf(stderr, "ERROR: the EXE file may have been corrupted!\n");
/*		exit(1); */
	}
	while (encbytes < MAXENCRYPTED) {	/* fill out with junk */
		encryptstring(output, " ");
	}
	binaryout = 0;
	if (traceflag) printf("Closing file %s\n", exefile);

	fclose (output);

	if (copydate == 0) return 0;
	if (traceflag) printf("Modifying time and data of %s\n", exefile);
	timebuf.actime = statbuf.st_atime;
	timebuf.modtime = statbuf.st_atime;
/*	if (traceflag) if (getinfo(exefile, verboseflag) < 0) exit(1); */
	if (traceflag) printf("Trying to modify date in %s\n", exefile);
	if (_utime(exefile, &timebuf) != 0) {
		fprintf(stderr, "Unable to modify date/time\n");
		perror(exefile);
		/*			exit(3); */
	}
	/*		see if it worked */
	if (getinfo(exefile, traceflag) < 0) exit(1);
	return 0;
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void encryptfile (FILE *output, FILE *input) {
	int k;
	if (ownertext == NULL || serialtext == NULL) {
		printf("ERROR!");
		exit(1);
	}
	for (k = 0; k < 2; k++) { /* copy first two lines */
		fgets(line, MAXLINE, input);
		fputs(line, output);
	}
	rewind(input);
	fprintf(output, "currentfile eexec\n");
	cryptee = REEXEC; clm = 0;
	encryptstring(output, author);
	encryptstring(output, "% ");
	encryptstring(output, ownertext);
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
	timebuf.modtime = statbuf.st_atime;
	if (traceflag) printf("Trying to modify date in %s\n", outfile);
	if (_utime(outfile, &timebuf) != 0) {
		fprintf(stderr, "Unable to modify date/time\n");
		perror(outfile);
		/*			exit(3); */
	}
	/*		see if it worked */
	if (getinfo(exefile, traceflag) < 0) exit(1);
	return 1;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void showusage(char *s) {
	printf("Usage: %s -d=disk -e=editor [-m=MM:DD] [-s=serial]\n", s);
	printf("\n");
	printf("\t-d d: (32 bit system) or c: (16 bit system)\n");
	printf("\t-m MM:DD month day to force in key calculation (debugging)\n");
	printf("\t-s serial number to force in key calculation (debugging)\n");
	exit(1);
}

long createkey(char *stime) {		/* create installation `key' and show */
	char keystring[16];
	int month, day, garbled;
/*	long key; */
	int k;
	char *s;

/*	first reverse serial number, padded with zeros on left if needed */
	if (serialx > 0) serialnumber = serialx;		/* debugging */
/*	sprintf(serialtext, "%04d", serialnumber); */
	sprintf(line, "%04d", serialnumber);
/*	for (k = 0; k < 4; k++) keystring[k] = serialtext[3-k]; */
	for (k = 0; k < 4; k++) keystring[k] = line[3-k];
	keystring[4] = '\0';
	if (sscanf(keystring, "%d", &garbled) < 1) {
		printf("This can't happen %s!\n", keystring);
		garbled = 9999;
	}
	if (traceflag) printf("Serial %s (%d) Reversed serial %s (%d)\n",
		   line, serialnumber, keystring, garbled);
/*	use stime => 1990 Sep 27 06:26:35 */
	if ((s = strchr(stime, '\n')) != NULL) *s = '\0';
	if (dayx > 0) {
		sprintf(stime+9, "%02d", dayx);
		*(stime+11) = ' ';
	}
	if (monthx > 0) {
		strncpy(stime+5, months[monthx-1], 3);
	}
	if (sscanf(stime+9, "%d", &day) < 1) {
		printf("This can't happen %s!\n", stime+9);
		day = 1;
	}
	if (dayx > 0) day = dayx;		/* debugging */
	for (k = 0; k < 12; k++) if (strncmp(stime+5, months[k], 3) == 0) break;
	month = k;
	if (k == 12) {
		printf("This can't happen %s!\n", stime+5);
		month = 1;
	}
	if (monthx > 0) month = monthx-1;		/* debugging */
	if (traceflag)
		printf ("Date %s - month %d day %d\n", stime, month+1, day);
	key = ((long) (day+10) * 100 + (month+1)) * 10000L + garbled - 999;
	if (verboseflag) printf("The installation key is: %ld\n", key);
/*	printf("\nPlease make a note of it.\n"); */
	return key;
}

/* #define NW 201
   #define EW 205
   #define SW 200
   #define SE 188
   #define NE 187
   #define NS 186 */

#define NW '\311'
#define EW '\315'
#define SW '\310'
#define SE '\274'
#define NE '\273'
#define NS '\272'

/* #define COLUMNS 64 */
#define COLUMNS 49

void topline(void) {
	int k;
	putc(NW, stdout);
	for (k = 0; k < COLUMNS; k++) putc(EW, stdout);
	putc(NE, stdout);
	putc('\n', stdout);	
}

void blankline(void) {
	int k;
	putc(NS, stdout);
	for (k = 0; k < COLUMNS; k++) putc(' ', stdout);
	putc(NS, stdout);
	putc('\n', stdout);
}

void bottomline(void) {
	int k;
	putc(SW, stdout);
	for (k = 0; k < COLUMNS; k++) putc(EW, stdout);
	putc(SE, stdout);
}

// Allows usual console editing, backspace, F3 

int getconsoleline (char *buffer, int inlen) {
	int outlen;
	char *s;
	buffer[0] = (char) inlen;	/* maxium number of characters accepted */
	buffer[2] = '\0';			/* just in case */
	(void) _cgets(buffer);
	outlen = buffer[1];			/* characters actually entered */
	strcpy(buffer, buffer+2);	/* get rid of length information */
	s = buffer + strlen(buffer)-1;	/*	get rid of trailing spaces */
	while (s >= buffer && *s == ' ') *s-- = '\0';
	s = buffer;					/*	get rid of leading spaces */
	while (*s != '\0' && *s == ' ') strcpy(s, s+1);
	return strlen(buffer);	/* outlen */
}

void cleandigits (char *line) {	/* squeeze out all but digits here */
	char *s = line;
	int n;
	while (*s != '\0') {
		n = strspn(s, "0123456789");
		s += n;
		if (*s == '\0') break;
		strcpy(s, s+1);
	}
}

int checkedt (int argc, char *argv[], int m){
	char *s;
	s = argv[m];
	if (strstr(s, "WINEDT") != NULL) return 1;
	if (strstr(s, "winedt") != NULL) return 1;
	if (strstr(s, "WinEdt") != NULL) return 1;
	if (strstr(s, "WinEDT") != NULL) return 1;
	if (strstr(s, "PFE") != NULL) return 0;
	if (strstr(s, "pfe") != NULL) return 0;
	return -1;
}

char *YandYtex="YandY TeX";
char *authorname="Berthold Horn";

int main (int argc, char *argv[]) {
	FILE *output;
	time_t ltime;		/* for time and date */
	struct tm *tm;		/* structured time and date */
	int firstarg=1;
	char *s, *t;
	int namedflag=0;
	int k, inlen, outlen;

/*	if (argc > 1) { */
	while (firstarg < argc && *argv[firstarg] == '-') {
//		printf("k %d argc %d argv[k] %s\n", firstarg, argc, argv[firstarg]);
		fflush(stdout);
/*		if (strcmp(argv[firstarg], "-d") == 0) debugflag = 1; */
/*		else if (strcmp(argv[firstarg], "-v") == 0) verboseflag = 1; */
		if (strcmp(argv[firstarg], "-?") == 0) showusage(argv[0]);
		else if (strcmp(argv[firstarg], "-v") == 0) verboseflag = 1;
		else if (strcmp(argv[firstarg], "-t") == 0) traceflag = 1;
/*		debugging override of month and day -m=MM:DD */
		else if (strncmp(argv[firstarg], "-m", 2) == 0) {
//			if (sscanf(argv[firstarg], "-m=%d:%d", &monthx, &dayx) < 2) 
			s = argv[firstarg];
			if (sscanf(s+3, "%d:%d", &monthx, &dayx) < 2) {
				printf("Do not understand argument: %s (%s)\n", s, s+3);
				exit(1);
			}
			testkeyflag++;
		}
/*		debugging override of serial number -s=... */
		else if (strncmp(argv[firstarg], "-s", 2) == 0) {
			s = argv[firstarg];
//			if (sscanf(argv[firstarg], "-s=%d", &serialx) < 1) 
			if (sscanf(s+3, "%d", &serialx) < 1) { 
				printf("Do not understand argument: %s (%s)\n", s, s+3);
				exit(1);
			}
			testkeyflag++;
		}
		else if (strncmp(argv[firstarg], "-d", 2) == 0) {
			if (strstr(argv[firstarg], "c:") != NULL) oldsysflag=1;
			else if (strstr(argv[firstarg], "d:") != NULL) newsysflag=1;
			else {
				firstarg++;
				if (strstr(argv[firstarg], "c:") != NULL) oldsysflag=1;
				else if (strstr(argv[firstarg], "d:") != NULL) newsysflag=1;
				else {
					printf("Do not understand argument: %s\n", argv[firstarg]);
					exit(1);
				}
			}
		}
		else if (strncmp(argv[firstarg], "-e", 2) == 0) {		/* new 99/Mar/ 8 */
			if (strlen(argv[firstarg]) > 2)
				winedtflag = checkedt(argc, argv, firstarg);
			else if (firstarg++ < argc)
				winedtflag = checkedt(argc, argv, firstarg);
			else fprintf(stderr, "ERROR: bad arguments\n");
		}
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
			if (*outputdir == 'd') accentsok = 1;
			else if (*outputdir == 'c') accentsok = 0;
		}
		firstarg++;
	}
	printf("Read arguments\n");
	fflush(stdout);	

	if (!oldsysflag && !newsysflag) {
/*		printf ("Use -d=16 or -d=32 to select system\n"); */
		printf ("Use -d=d: or -d=c: to select 32 bit or 16 bit system\n\n");
		showusage(argv[0]);
	}
	if (oldsysflag) {		/* do 16 bit version on drive c: */
		*dviserialfile = 'c';
		*winserialfile = 'c';
		*texserialfile = 'c';
		if (outputflag == 0) *outputdir = 'c';
	}

	if ((s = getenv("YANDYTEXDIR")) != NULL) ynytexdir = s;

	if (*outputdir != '\0') {	/* set up file names */
		if (setupfilenames(outputdir, ynytexalt) != 0)
			exit(1);
	}
	else {								/* set up file names */
		if (setupfilenames(ynytexdir, ynytexalt) != 0)
			exit(1);
	}

/*	usingold = 0; */
/*	oldserial = 0; */
	time(&ltime);				/* get seconds since 1970 */
/*	stime = ctime(&ltime); */
	stime = _strdup(ctime(&ltime));
	lcivilize(stime);
	if (traceflag != 0) printf("Time: %s", stime); 

	tm = localtime(&ltime);
	strftime(line, sizeof(line), "%A, %B %d, %Y at %H:%M", tm);
	winedtdate = _strdup(line);
/*	printf("%s --- %s\n", stime, winedtdate);	getch(); */

/*	extract information from last line in file */
/*	(void) extractold(); */
	findlatest();

/*	strcpy(serialtext, ""); */
	serialtext = NULL;

/*	if (traceflag != 0)
		printf("firstarg %d argc %d argv[firstarg] %s\n",
			firstarg, argc, argv[firstarg]); */
	if (firstarg < argc) {		/* owner given on command line ? */
/*		strcpy(ownertext, argv[firstarg]); */
		ownertext = _strdup(argv[firstarg]);
		namedflag++;
		if (sscanf(ownertext, "%d", &serialnumber) == 0) {
			if (retrieveserial(ownertext) == 0) {
				printf("Sorry, do not know a `%s'\n", ownertext);
/*				strcpy(ownertext, ""); */
/*				return -1; */
				exit(1);
			}
		}
		else {
			if (retrieveowner(serialnumber) == 0) {
				printf("Sorry, do not know serial number %d\n", serialnumber);
/*				return -1; */
				exit(1);
			}
			else {
/*				strcpy(ownertext, argv[firstarg]); */
				ownertext = _strdup(argv[firstarg]);
			}
		}
		if (verboseflag != 0)
			printf("serialnumber %d owner %s\n", serialnumber, ownertext);
		goto given;			/* user name or number given on command line */
	}

loop:			/* loop until get information straight ... */

/*	strcpy(serialtext, ""); */
	serialtext = NULL;

	for(;;) {
		printf("\n");
		printf("Please enter owner's name  OR  old serial number:\n"); 
		printf(".................................      <- max of 32 chars (use backspace)\r");
/*		if (oldserial > 0) printf("(last was: `%s')\n", oldowner); */
/*		gets(ownertext); */
		inlen = 255;
		fflush(stdout);
		outlen = getconsoleline(line, inlen);
/*		ownertext = strcpy(line); */
		if (ownertext != NULL) free(ownertext);
		ownertext = _strdup(line);

given:
		checkexit(ownertext);

		if (sscanf(ownertext, "%d", &serialnumber) == 1) {
/*			strcpy(serialtext, ownertext); */
			serialtext = ownertext;
/*			strcpy(ownertext, ""); */
			ownertext = NULL;
			printf("Using %d as serial number\n", serialnumber);
			break;
		}
		if (ownertext == NULL || *ownertext == '\0') {
/*			strcpy(ownertext, oldowner); */	/* use old name */
/*			printf("Using `%s' as new owner's name\n", ownertext); */
/*			try and retrieve owner name later */
			break;
		}
		else if (strlen(ownertext) > MAXOWNER) {	 /* was 36, 32 */
			fprintf(stderr, 
				"Sorry, owner's name too long (%d > %d)\n",
					strlen(ownertext), MAXOWNER);
/*			for (k = 0; k < MAXOWNER; k++) putc(ownertext[k], stdout); */
			ownertext[MAXOWNER] = '\0';
			printf("%s\n", ownertext);
/*			putc('\n', stdout); */
			printf("Type F3 to get the string back so you can edit it\n");
/*			exit(1); */
		}
		else {
			if (!accentsok) (void) accentize(ownertext);	/* 1992/Jan/03 */
			break;		/* name OK */
		}
	}

/*	if (strcmp(serialtext, "") != 0) goto skipserial; */
	if (serialtext != NULL && *serialtext != '\0') goto skipserial;

	for(;;) {
		printf("\n");
		printf("Please enter owner's serial number: ");
/*		if (oldserial > 0) printf("(next is %d) ", oldserial+1); */
		if (oldserial > 0) printf("(last was: %d) ", oldserial);
/*		gets(serialtext); */
		inlen = 255;
		fflush(stdout);
		outlen = getconsoleline(line, inlen);
		cleandigits(line);
		if (serialtext != NULL) free (serialtext);
		serialtext = _strdup(line);

/*		printf("%3u %3u %s\n", inlen, outlen, serialtext+2); */
/*		_getch(); */
		checkexit(serialtext);

/*		serialnumber = 0; */
		serialnumber = -1;
		if (serialtext == NULL || sscanf(serialtext, "%d", &serialnumber) < 1) {
			if (ownertext == NULL || *ownertext == '\0') {
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
				sprintf(line, "%d", serialnumber);
				if (serialtext != NULL) free (serialtext);
				serialtext = _strdup(line);
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

	if (ownertext == NULL || *ownertext == '\0') {
		if (retrieveowner(serialnumber) == 0) {
			printf("Sorry, no record for serial number %d\n", serialnumber);
			goto loop;
		}
	}

/*	added 98/Nov/29 */
	printf("\n");
	printf("Please enter abbreviation for font set(s) supplied with TeX System\n");
	printf("e.g. CM, LB, MT, EM, CM+LB, CM+MT, LB+MT, CM+LB+MT etc.\n");

	inlen = 255;
	fflush(stdout);
	outlen = getconsoleline(line, inlen);
/*	squezze out spaces if any */
	while ((s = strchr(line, ' ')) != NULL) strcpy(s, s+1);
	fontsets = _strdup(line);

	(void) createkey(stime);

	if (testkeyflag) goto skipfiles;
	
	if (winedtflag) {
		printf("\n");
		printf("Please enter WinEdt user name (or hit ENTER key if unknown)\n");
		printf("e.g. Jane Doe (%s) <- you can omit the \" (%s)\"\n",
			  YandYtex, YandYtex);
		inlen = 255;
		fflush(stdout);
		outlen = getconsoleline(line, inlen);
		if (outlen > 0) {
//			if "YandY TeX" not entered, append it (unless author)
			if (strstr(line, YandYtex) == NULL &&
				strstr(line, authorname) == NULL) {
//				strcat(line, " (YandY TeX)");
				s = line + strlen(line);
				sprintf(s, " (%s)", YandYtex);
				printf("EXTENDED TO: %s\n", line);
			}
			winedtname = _strdup(line);
			printf("\n");
			printf("Please enter WinEdt code number (or hit ENTER key if unknown)\n");
			inlen = 255;
			fflush(stdout);
			if (strstr(line, authorname) == NULL) 
				outlen = getconsoleline(line, inlen);
			else strcpy(line, "73272016");
			cleandigits(line);
			if (sscanf(line, "%d", &winedtmagic) > 0) {
				winedtinst = ltime / 60 +  194074261;
				winedtcode = winedtmagic - 283753219;
				winedtzzxc = winedtmagic -  17641597;
				winedtzzxy = winedtinst  -  17641597;
				if (traceflag) {
					printf("CODE %d INST %d DATE %s VER %s\n",
					   winedtcode, winedtinst, winedtdate, winedtver); 
					printf("ZZXC %d ZZXX %d ZZXY %d\n",
						   winedtzzxc, winedtzzxx, winedtzzxy);
					fflush(stdout); 
					_getch();
				}
			}
		}

	}
	else {
		printf("\n");
		printf("NOTE: Assuming PFE Type System (not WinEdt)\n");
	}

	(void) createkey(stime);

	if (testkeyflag) goto skipfiles;

	if (ownertext == NULL || serialtext == NULL) {
		printf("ERROR!");
		exit(1);
	}
	if (traceflag) printf("Trying to open owner file %s\n", ownerfile);
	if ((output = fopen(ownerfile, "w")) == NULL) {
		perror(ownerfile);
	}
	else {
		fprintf(output, "%s %s %s\n", ownertext, serialtext, stime);
/*		if (verboseflag != 0) printf("%s %s %s", ownertext, serialtext, stime); */
		if (verboseflag != 0) printf("%s %s %s\n", ownertext, serialtext, stime);
		fclose(output);
	}
	
	if ((output = fopen(customfile, "w")) == NULL) {
		perror(customfile);
	}
	else {
/*		fputs("[Serial]\n", output); */
		fputs("[Registration]\n", output);
/*		fprintf(output, "Serial=%04d\n", serialnumber); */
/*		fprintf(output, "Serial=%s\n", serialtext); */
		fprintf(output, "Serial=%d\n", serialnumber);
		{
			int n;
			if (sscanf (serialtext, "%d", &n) == 1) {
				if (n != serialnumber)
					fprintf(stderr, "Serial number mismatch %d %s\n",
							n, serialtext);
			}
			else fprintf(stderr, "Serial text is not number %s\n", serialtext);
		}
		fprintf(output, "Owner=%s\n", ownertext);
		fprintf(output, "Time=%s\n", stime);
		if (winedtflag && winedtname != NULL && winedtmagic != 0) {
			fputs("\n", output);
			fputs("[Software]\n", output);
			fprintf(output, "CODE=%d\n", winedtcode);
			fprintf(output, "INST=%d\n", winedtinst);
/*			fprintf(output, "DATE=%s", stime); */
			fprintf(output, "DATE=%s\n", winedtdate);
			fprintf(output, "NAME=%s\n", winedtname);
/*			fprintf(output, "VER=%d", winedtver); */
			fputs("\n", output);
			fputs("[Policies]\n", output);
			fprintf(output, "ZZXC=%d\n", winedtzzxc);
			fprintf(output, "ZZXY=%d\n", winedtzzxy);
/*			fprintf(output, "ZZXX=%d", -14500006); */
		}
		fclose(output);
	}

/*	if (usingold == 0) (void) addtofile();	*/	/* record new info */

/*	if (usingold == 0) (void) insertinfile(serialnumber, stime); */

/*	(void) insertinfile(serialnumber, stime, serialfile); */
	(void) insertinfile(serialnumber, stime, texserialfile);
	(void) insertinfile(serialnumber, stime, dviserialfile);
	(void) insertinfile(serialnumber, stime, winserialfile);

	printf("This program *only* sets up serial.num entries and serial.ini\n");

skipfiles:
/*	createkey(stime); */
/*	printf("\n"); */

	topline();

	blankline();

	putc(NS, stdout);
	printf("        The serial number is:        %4d        ", serialnumber);
	putc(NS, stdout);
	putc('\n', stdout);

	blankline();

	putc(NS, stdout);
	printf("        The installation key is: %ld        ", key);
	putc(NS, stdout);
	putc('\n', stdout);

	blankline();

	putc(NS, stdout);
	printf("        Please make a note of it.        ");
	for (k = 0; k < 8; k++) putc(' ', stdout);
	putc(NS, stdout);
	putc('\n', stdout);

	blankline();

	bottomline();

	return 0;				/* no customization calls */

#ifdef IGNORED
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
	return 0;
#endif
}

#ifdef OLDCUSTOMIZATION
char *nmakepath="c:\\msvc32s\\bin\\nmake.exe"; 	/* 32 bit version */
/* char *nmakepath="c:\\msvcnt\\bin\\nmake.exe"; */ /* new 32 bit version */
#endif

#ifdef OLDCUSTOMIZE
int domagicold (void) {
	FILE *output;

	output = fopen(magicfile, "w");
/*	fprintf(output, "#include \"dvihead.h\"\n"); */
	fprintf(output, "#define REEXEC 55665\n"); 
	putc('\n', output);
	fprintf(output, "char *hexmagic = \n\"");
/*	if (usebinflag != 0) fprintf(output, "\\x"); */ /* later maybe */
	encbytes = 0; 
	doencrypted(output);
	fprintf(output, "\";\n");
	putc('\n', output);
	fprintf(output, "/* %s %s %s */\n", ownertext, serialtext, stime);
	putc('\n', output);	
	fprintf(output, "long serialnumber = REEXEC * %d;\n", serialnumber);
	fclose(output);
	return 0;
}
#endif

