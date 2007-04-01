/* Copyright 1990, 1991, 1992 Y&Y
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

/* print out addresses with barcodes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* #define FNAMELEN 80 */
#define MAXLINE 256
#define MAXROWS 16

int verboseflag = 1;
int logflag = 1;			/* write to log file */
int forceletter = 1;		/* force printer to use `letter' size */

char *winini = "c:\\windows\\win.ini";

char *barcodename="c:\\ps\\barcode.ps";

char outfilename[FILENAME_MAX]="c:\\ps\\params.ps";

char *logfile = "c:\\txt\\addresses.txt";

char buffer[MAXLINE];

char lines[MAXROWS][MAXLINE];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

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
"oslash", "ugrave", "uacute", "ucircumflex", "udieresis", "yacute", "thorn", "ydieresis",
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXCHARNAME 32

/* accented characters are characters preceeded by accent */
/* ` grave, ' acute, ^ circumflex, ~ tilde,  */
/* " dieresis, * ring, | cedilla */
/* caron, ogonek, dotaccent, macron, hungaraccent do not occur */

/* special hack for germandbls use: *s */

int makeibm(int base, int accent) {
	char charname[MAXCHARNAME];
	int k;

	strcpy(charname, ansinew[base]);
/* following two lines 1993/Aug/2 */
	if (base == '`' && accent == '`') return 34;		/* quotedbl */
	if (base == '\'' && accent == '\'') return 34;		/* quotedbl */
	if (accent == '\"') strcat(charname, "dieresis");
	else if (accent == '~') strcat(charname, "tilde");	/* asciitilde */
	else if (accent == '^') strcat(charname, "circumflex");	/* asciicircum */
	else if (accent == '\'') strcat(charname, "acute");	/* quotesingle */
	else if (accent == '*') strcat(charname, "ring");	/* asterisk */
	else if (accent == '|') strcat(charname, "cedilla");	/* bar */
	else strcat(charname, ansinew[accent]);
	if (strcmp(charname, "sring") == 0) strcpy(charname, "germandbls");
/*	printf("CHARNAME: %s ", charname);  */
	for (k = 0; k < 265; k++) {
		if (strcmp(ansinew[k], charname) == 0) return k;
	}
	printf("Can't find %s --- ", charname);
	return -1;
}

/* ` grave, ' acute, ^ circumflex, ~ tilde,  */
/* " dieresis, * ring, | cedilla */
/* *s => germandbls */  /* no oslash Oslash in IBM OEM encoding */

int accentize (char *line) {
	char *s=line;
	char buffer[MAXLINE];
	int a, b, ibmcode;
	int flag = 0;
	
	while ((s = strpbrk(s, "\"`'^~*|")) != NULL) {
		a = *s++; b = *s++;
		if (b == '\0') return flag;		/* 1993/Aug/2 */
		strcpy(buffer, s);				/* save the rest of it */
		*s = '\0';
		s = s - 2;
		if ((ibmcode = makeibm(b, a)) >= 0) {
/*			printf("IBM CODE: %o\n", ibmcode);  */
			sprintf(s, "\\%o", ibmcode);
		}
		else {
			printf("(%c%c)\n", a, b);
			sprintf(s, "%c%c", a, b);
			flag++;
		}
		s = s + strlen(s);
		strcpy(s, buffer);
	}
/*	printf("CODE: %s\n", line);  */
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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

/* int readaddress(FILE *input) {
	int i, j,  c, cold=-1;

	for(i=0; i < MAXROWS; i++) {
		for(j=0; j < MAXLINE; j++) {
			lines[i][j] = '\0';
		}
	}

	i = 0;
	j = 0;
	for (;;) {
		c = getc(input);
		if (c == 3) return -1;
		else if (c == 127) {
			lines[i][j] = '\0';
			j--;
		}
		else if (c == '\n') {
			lines[i][j] =  '\0';
			i++; 
			j = 0;
			if (cold == c) {
				lines[i][j] = '\0';
				return 0;
			}
		}
		else {
			lines[i][j] = (char) c; 
			j++;
		}
		cold = c;
	}
} */

int readaddress(FILE *input) {
	int i, c;
	char *s, *b;

	while ((c = getc(input)) == ' ' && c != EOF) ;	/* skip over white-space */
	if (c == EOF) return -1;						/* no address found */
	ungetc(c, input);								/* put back non-blank */

	for (i = 0; i < MAXROWS; i++) {
		*lines[i] = '\0';
		b = fgets(lines[i], MAXLINE, input);
		if ((s = strchr(lines[i], '\n')) != NULL) *s = '\0';
		c = *lines[i];
/* escape if line starts with ^C, ^D, ^I, ^Z
		if (c == 3 || c == 4 || c == 9 || c == 26)
			return -1;			/* 1993/July/19 */
/*		if (b == NULL) return -1; */
		if (*lines[i] == '\0') return 0;		/* blank lines terminates */
		(void) accentize(lines[i]);				/* 1993/Jan/14 */
	}
	return 0;		/* ran out of space, but OK? ... */
}

/* void writeaddress(FILE *output) {
	int i, j, c, cold=-1;
	
	for (i=0; i < MAXROWS; i++) {
		c = lines[i][0];
		if (c == '\0') {
			putc('\n', output);
			break;
		}
		for (j=0; j < MAXLINE; j++) {
			c = lines[i][j];
			if (c == '\0') {
				putc('\n', output);
				break;
			}
			else putc(c, output);
		}
	}
} */

/* void writeaddress(FILE *output) {
	int i;
	for (i = 0; i < MAXROWS; i++) {
		if (*lines[i] == '\0') return;
		fputs (lines[i], output);
	}
} */

void writeaddress(FILE *output, int psflag) {
	int i;
	if (psflag != 0) {
		putc('[', output); 	putc('\n', output);
	}
	for (i = 0; i < MAXROWS; i++) {
		if (*lines[i] == '%'  || *lines[i] == ';') continue;
		if (*lines[i] == '\0') break;
		if (psflag != 0) fprintf(output, "(%s)", lines[i]);
		else fputs (lines[i], output);
		putc('\n', output);
	}
	if (psflag != 0) {
		putc(']', output);
		putc('\n', output);
	}
	putc('\n', output);
}

void copyfile(FILE *output, FILE *input) {
	while (fgets(buffer, MAXLINE, input) != NULL) {
		fputs(buffer, output);
	}
}

/* old method, looked for [PostScript, LPT1] section
int guessprinter(void) {
	FILE *input;
	char *s;
	if ((input = fopen(winini, "r")) == NULL) {
		perror (winini); return -1;
	}
	while (fgets(buffer, MAXLINE, input) != NULL) {
		if (*buffer != '[') continue;
		if (strncmp(buffer+1, "PostScript", 10) != 0) continue;
		if (strncmp(buffer+1+10+1, "FILE", 4) == 0) continue;
		strcpy(outfilename, buffer+1+10+1);
		if ((s = strchr(outfilename, ']')) != NULL) {
			*s= '\0';
		}
		fclose (input);
		return 0;
	}
	fclose (input);
	return -1;
} */

/* new method, looks for [windows] section and `device=' entry */

int guessprinter(void) {
	FILE *input;
	char *s;
	if ((input = fopen(winini, "r")) == NULL) {
		perror (winini); return -1;
	}
	while (fgets(buffer, MAXLINE, input) != NULL) {
		if (*buffer != '[') continue;
		if (strncmp(buffer+1, "windows", 6) != 0) continue;
		while (fgets(buffer, MAXLINE, input) != NULL) {
			if (*buffer < ' ') break;		/* end of section */
			if (strncmp(buffer, "device", 6) != 0) continue;
			s = buffer;
			if ((s = strchr(s, ',')) == NULL) break;
			s++;
			if ((s = strchr(s, ',')) == NULL) break;
			s++;
			strcpy(outfilename, s);
			if ((s = strchr(outfilename, '\r')) != NULL) *s = '\0';
			if ((s = strchr(outfilename, '\n')) != NULL) *s = '\0';
			if ((s = strchr(outfilename, ':')) != NULL) *s = '\0';
			fclose (input);
			return 0;
		}
		fclose (input);
		return -1;
	}
	fclose (input);
	return -1;
}

int main(int argc, char *argv[]) {
	FILE *input, *output, *file, *barcode;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	int firstarg=1;
	int batchmode=0;

	if (argc > firstarg) {		/* address file name specified ? */
		batchmode = 1;
		strcpy(fn_in, argv[firstarg]);
		if ((input = fopen(fn_in, "r")) == NULL) {
			perror(fn_in); exit(1);
		}
	}
	else input = stdin;

	(void) guessprinter();	/* try and guess what printer to use */

	strcpy(fn_out, outfilename);
	if ((output = fopen(fn_out, "w")) == NULL) {
		perror(fn_out); exit(1);
	}
	
	if ((file = fopen(logfile, "a")) == NULL) {
		perror(logfile); exit(1);
	}

	if (verboseflag != 0) {
		printf("Output goes to %s  ", outfilename);
		if (batchmode != 0) printf("Input comes from %s  ", fn_in);
		putc('\n', stdout);
	}

	if (batchmode == 0) putc('\n', stdout);

	for(;;) {
		if (readaddress(input) != 0) break;		/* if hit EOF */
	
		fputs("%!PS-Adobe\n", output);

/*		writeaddress(stdout, 0); */

		if (logflag != 0 && batchmode == 0) writeaddress(file, 0);

		writeaddress(output, 1);

/*		fclose(input); */

		if ((barcode = fopen (barcodename, "r")) == NULL) {
			perror(barcodename); exit(1);
		}

		if (forceletter != 0) fputs("{letter} stopped pop\n", output); 
		copyfile(output, barcode);
		putc('\n', output);
		putc(4, output);
		fclose (barcode);

		if (batchmode == 0) break;		/* if reading from console */
	}

	fclose(file);
	fclose(input);
	fclose (output);
	return 0;
}

/* envelope tray moves coordinates over by 1.25 inches in x ... */
