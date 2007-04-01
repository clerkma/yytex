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

/* Prepare outlines from Doug into normal OUT format */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 256

#define MAXCHRS 256

#define NOWIDTH -32000

int verboseflag = 1;

char *encoding[MAXCHRS] = {
"grave", "acute", "circumflex", "tilde", "dieresis", "hungarumlaut", "ring",
"caron", "breve", "macron", "dotaccent", "cedilla", "ogonek",
"quotesinglbase", "guilsinglleft", "guilsinglright", 
"quotedblleft", "quotedblright", "quotedblbase", "guillemotleft",
"guillemotright", "endash", "emdash", "cwm", "perzero", "dotlessi",
"dotlessj", "ff", "fi", "fl", "ffi", "ffl", 
"visiblespace", "exclam", "quotedbl", "numbersign", "dollar", "percent",
"ampersand", "quoteright", "parenleft", "parenright", "asterisk", "plus",
"comma", "hyphen", "period", "slash", 
"zero", "one", "two", "three", "four", "five", "six", "seven", "eight",
"nine", "colon", "semicolon", "less", "equal", "greater", "question", 
"at", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
"O", 
"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft",
"backslash", "bracketright", "asciicircum", "underline", 
"quoteleft", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
"n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft", "bar",
"braceright", "asciitilde", "sfthyphen", 
"Abreve", "Aogonek", "Cacute", "Ccaron", "Dcaron", "Ecaron", "Eogonek",
"Gbreve", "Lacute", "Lcaron", "Lslash", "Nacute", "Ncaron", "Eng",
"Ohungarumlaut", "Racute", 
"Rcaron", "Sacute", "Scaron", "Scedilla", "Tcaron", "Tcedilla",
"Uhungarumlaut", "Uring", "Ydieresis", "Zacute", "Zcaron", "Zdotaccent",
"IJ", "Idotaccent", "dcroat", "section", 
"abreve", "aogonek", "cacute", "ccaron", "dcaron", "ecaron", "eogonek",
"gbreve", "lacute", "lcaron", "lslash", "nacute", "ncaron", "eng",
"ohungarumlaut", "racute", 
"rcaron", "sacute", "scaron", "scedilla", "tcaron", "tcedilla",
"uhungarumlaut", "uring", "ydieresis", "zacute", "zcaron", "zdotaccent",
"ij", "exclamdown", "questiondown", "sterling", 
"Agrave", "Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE",
"Ccedilla", "Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave",
"Iacute", "Icircumflex", "Idieresis", 
"Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde", "Odieresis",
"OE", "Oslash", "Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute",
"Thorn", "SS", 
"agrave", "aacute", "acircumflex", "atilde", "adieresis", "aring", "ae",
"ccedilla", "egrave", "eacute", "ecircumflex", "edieresis", "igrave",
"iacute", "icircumflex", "idieresis", 
"eth", "ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis",
"oe", "oslash", "ugrave", "uacute", "ucircumflex", "udieresis", "yacute",
"thorn", "germandbls"
};

int widths[MAXCHRS];

int readline (FILE *input, char *buffer, int nlen) {
	int c, n=0;
	char *s=buffer;
	while ((c = getc(input)) != EOF) {
		if (c == '\r') c = '\n';
		*s++ = (char) c;
		if (n++ >= nlen-3) {
			fprintf(stderr, "Line too long\n");
			break;
		}
		if (c < ' ') break;
	}
	*s = '\0';
	if (c == EOF) return EOF;
	return n;
}

int preprocess (FILE *output, FILE *input, char *name) {
	char *s;
	char line[MAXLINE];
	int c, chr, wx;
	
	if ((s = strchr(name, '.')) != NULL) *s = '\0';
	fprintf(output, "%%%% %s.out\n", name);
	fprintf(output, "10.0000 1\n");
	fprintf(output, "%d %d %d %d\n", -50, -300, 1000, 900);
	fprintf(output, "]\n");
	if (s != NULL) *s = '.';
	c = getc(input);
	ungetc(c, input);
	*line = c;
	if (c == '%') {
		while (*line != ']')  {
			if (readline(input, line, sizeof(line)) < 0) break;
/*			fputs(line, output); */
		}
		if (*line !=  ']') {
			fprintf(stderr, "Premature EOF\n");
			return -1;
		}
	}
	for (;;) {
		if (readline(input, line, sizeof(line)) < 0) break;
		if (*line < ' ') continue;			/* ignore blank lines */
		if (*line == '%') continue;			/* ignore comment lines */
		if (sscanf(line, "%d %d", &chr, &wx) < 2) {
			fprintf(stderr, "Expecting chr and wx not: %s", line);
		}
		if (chr < 0 || chr > 255) {
			fprintf(stderr, "Bad character code %d\n", chr);
			return -1;
		}
		fprintf(output, "%d %d %% %s\n", chr, wx, encoding[chr]);
		if (widths[chr] != NOWIDTH)
			fprintf(stderr, "Repeated character code %d\n", chr);			
		widths[chr] = wx;
		while (*line != ']')  {
			if (readline(input, line, sizeof(line)) < 0) break;
			fputs(line, output);
		}
		if (*line != ']')  break;
	}
	return 0;
}

int writeafm (FILE *output, char *name) {
	int k, count = 0;
	char *s;
	for (k = 0; k < MAXCHRS; k++) if (widths[k] != NOWIDTH) count++;	
	if ((s = strchr(name, '.')) != NULL) *s = '\0';
	fprintf(output, "StartFontMetrics\n");
	fprintf(output, "FontName %s\n", name);
	fprintf(output, "StartCharMetrics %d\n", count);
	if (s != NULL) *s = '.';
	for (k = 0; k < MAXCHRS; k++) {
		if (widths[k] != NOWIDTH)
			fprintf(output, "C %d ; WX %d ; N %s ; \n",
					k, widths[k], encoding[k]);
	}
	fprintf(output, "EndCharMetrics\n");
	fprintf(output, "EndFontMetrics\n");
	return count;
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

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
#ifdef MAKEBACKUP
	char bakfile[FILENAME_MAX];
#endif
	int k, m, count=0;
	for (m=1; m <argc; m++) {
		for (k = 0; k < MAXCHRS; k++) widths[k] = NOWIDTH;
		strcpy(infile, argv[m]);
		extension(infile, "org");
#ifdef MAKEBACKUP
		input = fopen(infile, "rb");
		if (input == NULL) {
			perror(infile);
			continue;
		}
		fclose(input);
		if (verboseflag) printf("Input %s\n", infile);
		strcpy(bakfile, infile);
		forceexten(infile, "bak");
		if (rename(bakfile, infile) != 0) {		/* foo.out =>  foo.bak */
			printf("Unable to rename %s to %s\n", bakfile,
				   infile);
			continue;

		}
#endif
		input = fopen(infile, "rb");
		if (input == NULL) {
			perror(infile);
			continue;
		}
		if (verboseflag) printf("Input %s\n", infile);
		strcpy(outfile, argv[m]);
		forceexten(outfile, "out");
		output = fopen(outfile, "w");
		if (output == NULL) {
			perror(outfile);
			fclose(output);
			fclose(input);
#ifdef MAKEBACKUP
			rename(infile, bakfile);
#endif
			exit(1);
		}
		if (verboseflag) printf("Output %s\n", outfile);
		preprocess(output, input, infile);
		count++;
		fclose(output);
		fclose(input);
		strcpy(outfile, argv[m]);
		forceexten(outfile, "afm");
		output = fopen(outfile, "w");		
		if (output == NULL) {
			perror(outfile);
			exit(1);
		}
		if (verboseflag) printf("Output %s\n", outfile);
		writeafm(output, outfile);
		fclose(output);
	}
	if (count > 0) printf("Processed %d files\n", count);
	return 0;
}
