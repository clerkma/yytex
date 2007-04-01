/* Copyright 1999 Y&Y, Inc.
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

/* Code for massaging "Type 4" font binary sections into "decrypted form" */
/* Run the result trhough "decrypt -vsr" to remove the character level encoding */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAXLINE 2048

#define MAXCHRS 256

#define MAXGLYFS 512

char line[MAXLINE];

char *encoding[MAXCHRS];

char *glyfname[MAXGLYFS];

unsigned int nglyf=0;

unsigned int nsubrs=0;

int verboseflag=0;
int showencoding=0;
int showglyfflag=0;

unsigned long filelength, nlen;

unsigned long suboffset=0;

unsigned char *fontcode=NULL;

char *standard[] = { 
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
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
	"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "exclamdown", "cent", "sterling", "fraction", "yen", "florin", "section",
	"currency", "quotesingle", "quotedblleft", "guillemotleft", "guilsinglleft", "guilsinglright", "fi", "fl",
	"", "endash", "dagger", "daggerdbl", "periodcentered", "", "paragraph", "bullet",
	"quotesinglbase", "quotedblbase", "quotedblright", "guillemotright", "ellipsis", "perthousand", "", "questiondown",
	"", "grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
	"dieresis", "", "ring", "cedilla", "", "hungarumlaut", "ogonek", "caron",
	"emdash", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "",
	"", "AE", "", "ordfeminine", "", "", "", "",
	"Lslash", "Oslash", "OE", "ordmasculine", "", "", "", "",
	"", "ae", "", "", "", "dotlessi", "", "",
	"lslash", "oslash", "oe", "germandbls", "", "", "", "",
};

/* The standard glyph names arranged alphabetically */

char *alphanames[] = { ".notdef",
	"A", "AE", "Aacute", "Acircumflex", "Adieresis", "Agrave", "Aring",
	"Atilde", "B", "C", "Ccedilla", "D", "E", "Eacute", "Ecircumflex",
	"Edieresis", "Egrave", "Eth", "F", "G", "H", "I", "Iacute",
	"Icircumflex", "Idieresis", "Igrave", "J", "K", "L", "Lslash", "M",
	"N", "Ntilde", "O", "OE", "Oacute", "Ocircumflex", "Odieresis",
	"Ograve", "Oslash", "Otilde", "P", "Q", "R", "S", "Scaron", "T",
	"Thorn", "U", "Uacute", "Ucircumflex", "Udieresis", "Ugrave", "V",
	"W", "X", "Y", "Yacute", "Ydieresis", "Z", "Zcaron", "a", "aacute",
	"acircumflex", "acute", "adieresis", "ae", "agrave", "ampersand",
	"aring", "asciicircum", "asciitilde", "asterisk", "at", "atilde",
	"b", "backslash", "bar", "braceleft", "braceright", "bracketleft",
	"bracketright", "breve", "brokenbar", "bullet", "c", "caron",
	"ccedilla", "cedilla", "cent", "circumflex", "colon", "comma",
	"copyright", "currency", "d", "dagger", "daggerdbl", "degree",
	"dieresis", "divide", "dollar", "dotaccent", "dotlessi", "e",
	"eacute", "ecircumflex", "edieresis", "egrave", "eight", "ellipsis",
	"emdash", "endash", "equal", "eth", "exclam", "exclamdown", "f",
	"fi", "five", "fl", "florin", "four", "fraction", "g", "germandbls",
	"grave", "greater", "guillemotleft", "guillemotright",
	"guilsinglleft", "guilsinglright", "h", "hungarumlaut", "hyphen",
	"i", "iacute", "icircumflex", "idieresis", "igrave", "j", "k", "l",
	"less", "logicalnot", "lslash", "m", "macron", "minus", "mu", "multiply", "n",
	"nine", "ntilde", "numbersign", "o", "oacute", "ocircumflex",
	"odieresis", "oe", "ogonek", "ograve", "one", "onehalf",
	"onequarter", "onesuperior", "ordfeminine", "ordmasculine", "oslash",
	"otilde", "p", "paragraph", "parenleft", "parenright", "percent",
	"period", "periodcentered", "perthousand", "plus", "plusminus", "q",
	"question", "questiondown", "quotedbl", "quotedblbase",
	"quotedblleft", "quotedblright", "quoteleft", "quoteright",
	"quotesinglbase", "quotesingle", "r", "registered", "ring", "s",
	"scaron", "section", "semicolon", "seven", "six", "slash", "space",
	"sterling", "t", "thorn", "three", "threequarters", "threesuperior",
	"tilde", "trademark", "two", "twosuperior", "u", "uacute",
	"ucircumflex", "udieresis", "ugrave", "underscore", "v", "w", "x",
	"y", "yacute", "ydieresis", "yen", "z", "zcaron", "zero", ""
};

char *fgetsmod(char *buffer, int nlen, FILE *input) {
	int c, n=0;
	char *s=buffer;
	c = getc(input);
/*	printf("(%c %d) ", c, c); */
	if (c == EOF) return NULL;		/* no data and at end of file */
	while (c != EOF) {
		if (c == '\r') c = '\n';
		if (n++ == nlen) break;
		*s++ = (char) c;
		if (c == '\n') break;
		c = getc(input);
/*		printf("(%c %d) ", c, c); */
	}
	*s++ = '\0';
	return buffer;
}

int processencoding(char *line) {
	char *s=line;
	char *t;
	int count=0;
	int k;

	for (k = 0; k < MAXCHRS; k++) encoding[k] = NULL;
	if (strstr(line, "StandardEncoding") != NULL) {
		for (k = 0; k < MAXCHRS; k++) {
			if (strcmp(standard[k], "") != 0) {
				encoding[k] = strdup(standard[k]);
				count++;
			}
		}
		return count;
	}
	while (*s != '\0' && *s != '[') s++;
	if (*s == '\0') return 0;
	else s++;
	for (k = 0; k < MAXCHRS; k++) {
		if (*s != '/') return k;
		s++;
		t = s;
		while (*t != '\0' && *t != ' ' && *t != ']') t++;
		if (*s == '\0') return k;
		*t = '\0';
		encoding[k] = strdup(s);
		if (showencoding) printf("%3d\t%s\n", k, encoding[k]);
		s = t+1;
	}
	return MAXCHRS;
}


int processglyftable(FILE *input) {
	unsigned int k;
	char name[256];
	if (nglyf > 0) return 0;
	if (sscanf (line, "dup %d", &nglyf) < 1) {
		if (verboseflag) printf("%s", line);
		return 0;
	}
	for (k = 0; k < nglyf; k++) glyfname[k] = NULL;
	while (fgetsmod(line, sizeof(line), input) != NULL) {
		if (*line != '/') break;
		if (strncmp(line, "end", 3) == 0) break;
		if (sscanf(line+1, "%s %d", name, &k) < 2) break;
		if (k < 0 || k >= nglyf) continue;
		glyfname[k] = strdup(name);
		if (showglyfflag) printf("%3d\t%s\n", k, glyfname[k]);
	}	
	return nglyf;
}

unsigned long readsuboffset(char *s) {
	if (sscanf(s, "/SubOffset %d", &suboffset) > 0) {
		printf("SubOffset is %d\n", suboffset);
	}
	return suboffset;
}

unsigned long readnglyfs(char *s) {
	if (sscanf(s, "/DFTotal %d", &nglyf) > 0) {
		printf("DFTotal is %d\n", nglyf);
	}
	return nglyf;
}

int setupstdglyftable(void) {
	unsigned int k;
	for (k = 0; k < MAXCHRS; k++) glyfname[k] = NULL;
/*	for (k = 0; k < MAXCHRS; k++) {
		sprintf(buffer, "a%d", k);
		glyfname[k] = strdup(buffer);
	}
	nglyf = MAXCHRS; */
	nglyf = 0;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(alphanames[k], "") == 0) {
			break;
		}
		glyfname[k] = strdup(alphanames[k]);
		nglyf++;
	}
	return nglyf; 
}

void scantobinary(FILE *output, FILE *input) {
	char *s;
	while (fgetsmod(line, sizeof(line), input) != NULL) {
		if (verboseflag) fputs(line, stdout);
		fputs(line, output);
		if (strncmp(line, "/Encoding", 9) == 0)	processencoding(line);
		if ((s = strstr(line, "/SubOffset")) != NULL) readsuboffset(s);
		if ((s = strstr(line, "/DFTotal")) != NULL) readnglyfs(s);
		if (strstr(line, "/StdInverseEncoding") != NULL)
			setupstdglyftable();
		if (strncmp(line, "dup", 3) == 0 && strstr(line, "dict dup begin") != NULL)
			processglyftable(input);
		if (strstr(line, "currentfile closefile} exec") != NULL) break;
	}
}

void readfile(FILE *input) {
	long current;	
	current = ftell(input);
	nlen = filelength - current;
	printf("CharStrings and Subrs take %d bytes\n", nlen);
	printf("CharStrings %d bytes, Subrs %d bytes\n",
		   suboffset, nlen - suboffset);
	fontcode = (unsigned char *) malloc (nlen);
	if (fontcode == NULL) {
		fprintf(stderr, "Unable to allocate %u bytes\n", nlen);
		exit(1);
	}
	if (fread(fontcode, 1, nlen, input) == 0) {
		fprintf(stderr, "Unable to read %u bytes\n", nlen);
		exit(1);
	}
}

unsigned short reversebytes (unsigned short x) {
	unsigned int c, d;
	c = x >> 8;
	d = x & 255;
	return (unsigned short) ((d << 8) | c);
}

void showglyftable(unsigned char *fontcode) {
	unsigned short *table;
	unsigned int k;
	unsigned offset, length;
	char *s;
	table = (unsigned short *) fontcode;
	for (k = 0; k < nglyf; k++) {
		offset = reversebytes(table[k+k]);
		length = reversebytes(table[k+k+1]);
		s = glyfname[k];
		if (s == NULL) s = "BLANK";
		printf("%3d\t%4d\t%4d\t%s\n", k, offset, length, s);
	}
}

void writeglyfcode (FILE *output, unsigned char *fontcode, unsigned long nlen) {
	unsigned short *table;
	unsigned offset, length;
	unsigned int k;
	char *s;
	unsigned char *t;
	unsigned char *strngs;
	fprintf(output, "2 index /CharStrings %d dict dup begin\n", nglyf);
	printf("%d CharStrings\n", nglyf);
	table = (unsigned short *) fontcode;
	strngs = (unsigned char *) (fontcode + 4 * nglyf);
	for (k = 0; k < nglyf; k++) {
		offset = reversebytes(table[k+k]);
		length = reversebytes(table[k+k+1]);
		s = glyfname[k];
		if (s == NULL) s = "BLANK";
		if (verboseflag) printf("%3d\t%4d\t%4d\t%s\n", k, offset, length, s);
		fprintf(output, "/%s %d RD ", s, length);
		t = strngs + offset;
/*		for (i = 0; i < length; i++) {
			c = *t++;
			printf("%3d ", c);
		} */
		fwrite(t, 1, length, output);
/*		printf("\n"); */
		fprintf(output, " ND\n");
	}
	fprintf(output, "end\n");
	fprintf(output, "end\n");
	fprintf(output, "readonly put\n");	
	fprintf(output, "noaccess put\n");	
}

void showsubcode (unsigned char *fontcode, unsigned int suboffset, unsigned int nlen) {
	unsigned int i, k;
	unsigned int c, d, n;
	unsigned char *s;

	printf("Subrs:\n");
	s = fontcode + suboffset;
	nsubrs = *s++;
	printf("%d Subrs\n", nsubrs);
	for (k = 0; k < nsubrs; k++) {
		if (s >= fontcode + nlen) {
			printf("ERROR: k %d nsubrs %d\n", k, nsubrs);
			break;
		}
		c = *s++; d = *s++;
		n = (c << 8) | d;
		for (i = 0; i < n; i++) {
			c = *s++;
			printf("%3d ", c);
		} 
		printf("\n");
	}
	if (s != fontcode + nlen)
		printf("ERROR: %d bytes, not %d\n", s - fontcode, nlen);
}


void writesubcode (FILE *output, unsigned char *fontcode, unsigned int suboffset, unsigned int nlen) {
	unsigned int k;
	unsigned int c, d, n;
	unsigned char *s;

	s = fontcode + suboffset;
	nsubrs = *s++;
	printf("%d Subrs\n", nsubrs);
	if (verboseflag) printf("%d Subrs\n", nsubrs);
	fprintf(output, "/Subrs %d array\n", nsubrs);
	for (k = 0; k < nsubrs; k++) {
		if (s >= fontcode + nlen) {
			printf("ERROR: k %d nsubrs %d\n", k, nsubrs);
			break;
		}
		c = *s++; d = *s++;
		n = (c << 8) | d;
		fprintf(output, "dup %d %d RD ", k, n);
		fwrite(s, 1, n, output);
		fprintf(output, " NP\n");
		s += n;
	}
	fprintf(output, "ND\n");
	if (s != fontcode + nlen)
		printf("ERROR: %d bytes, not %d\n", s - fontcode, nlen);
}


void processfile (FILE *output, unsigned char *fontcode, unsigned long nlen) {
	if (verboseflag) showglyftable(fontcode);
	writesubcode(output, fontcode, suboffset, nlen);
	writeglyfcode(output, fontcode, nlen);
	if (suboffset > 0 && verboseflag) showsubcode(fontcode, suboffset, nlen);
}

#ifdef IGNORED
void processfilea(FILE *input) {
	unsigned int c, d, e, f;
	unsigned long cd, ef, w;
	int count=0;
	for (;;) {
		c = getc(input);
		d = getc(input);
		e = getc(input);
		f = getc(input);
		if (c == EOF || d == EOF) break;
		if (e == EOF || f == EOF) break;
		cd = (c << 8) | d;
		ef = (e << 8) | f;
		w = (cd << 16) | ef;
		printf("%4d %4d %4d\t(%3d %3d %3d %3d) (%5d %5d) (%9u)\n",
			   count, count/2, count/4, c, d, e, f, cd, ef, w);
		count += 4;
	}	
}
#endif

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

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

/* return pointer to file name - minus path - returns pointer to filename */

char *extractfilename(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infile[FILENAME_MAX];
	char outfile[FILENAME_MAX];
	
	if (argc < 2) exit(1);
	strcpy(infile, argv[1]);
	input = fopen(infile, "rb");
	if (input == NULL) {
		perror(infile);
		exit(1);
	}
	fseek(input, 0, SEEK_END);
	filelength = ftell(input);
	fseek(input, 0, SEEK_SET);	
	printf("File is %d bytes\n", filelength);
	strcpy(outfile, extractfilename(infile));
	forceexten(outfile, "dec");
	output = fopen(outfile, "wb");
	if (output == NULL) {
		perror(outfile);
		exit(1);
	}
	scantobinary(output, input);
	readfile(input);
	processfile(output, fontcode, nlen);
	fclose(output);
	fclose(input);
	if (fontcode != NULL) free(fontcode);
	return 0;
}

#ifdef IGNORED
int main (int argc, char *argv[]) {
	time_t mtime;
	mtime = time(NULL);
	printf("seconds %d minutes %d hours %d days %d\n",
		   mtime, mtime / 60, mtime / (60 * 60),  mtime / (60 * 60 * 24));

	return 0;
}
#endif
