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

/* Decode Fontographer output in hex CharString form */
/* Decode Ikarus output in hex CharString form */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 256

#define MAXFILENAME 128

#define MAXCHARNAME 64

#define MAXSTACK 64

#define MAXCHRS 256

char line[MAXLINE];

int verbose=1;

int trace=1;

int hires=0;				/* which format ? Ikarus of Fontographer */

int scale=15;				/* scale used in Ikarus Type 3 output */

int ns=0;

int chr=0;					/* numeric code for character working on */

char charname[MAXCHARNAME];

/* Following are the aliases that Fontographer uses for character codes */

char *fontographer[] = {	/* Fontographer encoding */
"NUL", "Eth", "eth", "Lslash", "lslash", "Scaron", "scaron", "Yacute",
"yacute", "HT", "LF", "Thorn", "thorn", "CR", "Zcaron", "zcaron",
"DLE", "DC1", "DC2", "DC3", "DC4", "onehalf", "onequarter", "onesuperior",
"threequarters", "threesuperior", "twosuperior", "brokenbar", "minus", "multiply", "RS", "US",
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
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "DEL",
"Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis", "aacute",
"agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla", "eacute", "egrave",
"ecircumflex", "edieresis", "iacute", "igrave", "icircumflex", "idieresis", "ntilde", "oacute",
"ograve", "ocircumflex", "odieresis", "otilde", "uacute", "ugrave", "ucircumflex", "udieresis",
"dagger", "degree", "cent", "sterling", "section", "bullet", "paragraph", "germandbls",
"registered", "copyright", "trademark", "acute", "dieresis", "notequal", "AE", "Oslash",
"infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu", "partialdiff", "summation",
"product", "pi", "integral", "ordfeminine", "ordmasculine", "Omega", "ae", "oslash",
"questiondown", "exclamdown", "logicalnot", "radical", "florin", "approxequal", "Delta", "guillemotleft",
"guillemotright", "ellipsis", "nbspace", "Agrave", "Atilde", "Otilde", "OE", "oe",
"endash", "emdash", "quotedblleft", "quotedblright", "quoteleft", "quoteright", "divide", "lozenge",
"ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft", "guilsinglright", "fi", "fl",
"daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase", "perthousand",
"Acircumflex", "Ecircumflex", "Aacute",
"Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex",
"apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde",
"macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron"
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int lookup(char *name) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(name, fontographer[k]) == 0) return k;
	}
	return -1;
}

int getnext(FILE *input) {
	int c, d;
	while ((c = getc(input)) != EOF && c <= ' ') ;
	if (c == EOF) {
		fprintf(stderr, "Premature EOF\n");
		exit(1);
	}
	if (c == '>') {
		if (trace) fprintf(stderr, "End Of Character\n");
		return -1;
	}
	if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
	else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
	else if (c >= '0' && c <= '9') c = c - '0';
	else {
		fprintf(stderr, "Bad hex data\n");
		exit(1);
	}
	while ((d = getc(input)) != EOF && d <= ' ') ;
	if (d == EOF) {
		fprintf(stderr, "Premature EOF\n");
		exit(1);
	}
	if (d == '>') {
		if (verbose) fprintf(stderr, "Odd End Of Character\n");
		exit(1);
	}
	if (d >= 'A' && d <= 'F') d = d - 'A' + 10;
	else if (d >= 'a' && d <= 'f') d = d - 'a' + 10;
	else if (d >= '0' && d <= '9') d = d - '0';
	else {
		fprintf(stderr, "Bad hex data\n");
		exit(1);
	}	
	return (c << 4) | d;
}

int stackindex=0;

int stack[MAXSTACK];

int pop (void) {
 	if (stackindex <= 0) {
		fprintf(stderr, "Stack Underflow\n");
		exit(1);
	}
	return stack[--stackindex];
}

void push (int num) {
	if (stackindex >= MAXSTACK) {
		fprintf(stderr, "Stack Overflow\n");
		exit(1);
	}
	stack[stackindex++] = num;
}

int xold, yold;

void moveto (FILE *output) {
	int x, y;
	y = pop();
	x = pop();
	fprintf(output, "%d %d m\n", x, y);
	xold = x;  yold = y;
}

void rlineto (FILE *output) {
	int x, y;
	y = pop();
	x = pop();
	x += xold; y += yold;
	fprintf(output, "%d %d l\n", x, y);
	xold = x; yold = y;
}

void rrcurveto (FILE *output) {
	int xa, ya, xb, yb, xc, yc;
	yc = pop();
	xc = pop();
	yb = pop();
	xb = pop();
	ya = pop();
	xa = pop();
	xa += xold; ya += yold;
	xb += xa; yb += ya;
	xc += xb; yc += yb;
	fprintf(output, "%d %d %d %d %d %d c\n", xa, ya, xb, yb, xc, yc);
	xold = xc; yold = yc;
}

void setcache(FILE *output) {
	int wx, wy, xll, yll, xur, yur;
	int k;
	yur = pop();
	xur = pop();
	yll = pop();
	xll = pop();
	wy = pop();
	wx = pop();
	fprintf(output, "%% %d %d %d %d %d %d setcache\n",
		wx, wy, xll, yll, xur, yur);
	k = lookup (charname);
	if (k < 0)
		fprintf(stderr, "ERROR: Unable to find %s in encoding\n", charname);
	if (k >= 0) fprintf(output, "%d %d %% %s\n", k, wx, charname);
	else fprintf(output, "%d %d %% %s\n", chr, wx, charname);
	chr++;
}

void setlinewidth (FILE *output) {
	int width = pop();
	fprintf(output, "%% %lg setlinewidth\n", (double) width / 10.0);
}

void showint (FILE *output) {
	fprintf(output, "%% showint (stack %d)\n", stackindex);
}

void fillstroke (FILE *output) {
	int k = pop();
/* if PaintType != 0 then ignore argument and just stroke */
/* otherwise low two bits: 1 => fill, 2 => eofill, 3 => nothing */
/* and if bit 6 on: => bits 3 & 4 setlinecap, bits 4 & 5 setlinejoint stroke */
	fprintf(output, "%% fillstroke %x\n", k);
}

void setwidth (FILE *output) {
	fprintf(output, "%% setwidth (stack %d)\n", stackindex);
}

void muladd (FILE *output) {
	push (pop() * 100 + pop());
}

void togglens (FILE *output) {
	if (trace) printf("Toggling NS\n");
	ns = ~ns;
}

void closepath (FILE *output) {
	fprintf(output, "h\n");
}

void sg (FILE *output) {
	fprintf(output, "%% sg (stack %d)\n", stackindex);
}

/* Decode Fontographer CharStrings */

int decodecharlow (FILE *output, FILE *input) {
	int code, k;
	if (trace) printf("Decoding character\n");
	stackindex = 0;
	ns = 0;
	while ((code = getnext(input)) >= 0) {
/*		if (trace) printf("Code: %d (%x)\n", code, code); */
		if (code >= 233) {				/* coded command */
			code -= 233;
			switch(code) { 
				case 0: moveto (output); break;
				case 1: rlineto (output); break;
				case 2: rrcurveto (output); break;
				case 5: setcache (output); break;
				case 6: setlinewidth (output); break;
				case 7: showint (output); break;
				case 12: fillstroke(output); break;
				case 15: setwidth (output); break;
				case 16: muladd (output); break;
				case 17: togglens (output); break;
				case 19: closepath(output); break;
				case 21: sg(output); break;
				default:
					fprintf(stderr, "Unknown code %d (%x)\n", code, code);
					exit(1);
			}
		}
		else if (code >= 200) {				/* two century number */
			code -= 216;
			push (code);
			if (ns) {
				push ((1 << pop()) * (pop() * 100 + pop()));
			}
			else {
				push (pop() * 100 + pop());
			}
		}
		else push (code - 100);				/* just a number */
	}
	if (verbose) {
		printf("End Of Character (stack %d)\n", stackindex);
/*		printf("End Of Character ");
		if (stackindex != 0) printf("(stack %d)\n", stackindex);
		else putc('\n', stdout); */
		if (stackindex > 0) {
			for (k = 0; k < stackindex; k++) printf("%d ", stack[k]);
			putc('\n', stdout);
		}
	}
	fprintf(output, "]\n");	
	return 0;
}

/* ********************************************************************* */

int getbyte (FILE *input) {
	int c;
	c = getnext(input);
	if (c < 0) {
		fprintf(stderr, "Reached end of CharString by mistake\n");
	}
	return (c - 128);
}

int getword (FILE *input) {
	int c, d;
	long k;
	c = getnext(input);
	if (c < 0) {
		fprintf(stderr, "Reached end of CharString by mistake\n");
	}
	d = getnext(input);
	if (d < 0) {
		fprintf(stderr, "Reached end of CharString by mistake\n");
	}
	k = ((long) c << 8) | d;
	return (int) (k - 32768);
}

char subname[MAXCHARNAME];

void getname (FILE *input) {
	int k, len;
	char *s;
	s = subname;
	len = getnext(input);
	for (k = 0; k < len; k++) *s++ = (char) getnext(input);
	*s = '\0';
}

int round (int x) {
	if (x < 0) return - round (-x);
	return (int) (x + scale/2) / scale;
}

/* ********************************************************************* */

int cx, cy;

void charstarthi (FILE *input, FILE *output) {			/* code 0 */
	int k;
	k = getword(input);
/*	printf("%d width 0 ", k); */			/* width ? */
/*	if (trace) printf("%d %d %% %s\n", chr, round(k), charname); */
	fprintf(output, "%d %d %% %s\n", chr, round(k), charname); 
}

void setcachehi (FILE *input, FILE *output) {			/* code 1 */
	int xll, yll, xur, yur;
	xll = getword(input);
	yll = getword(input);
	xur = getword(input);
	yur = getword(input);
/*	if (trace) printf("%% %d %d %d %d setcache\n",
		round(xll), round(yll), round(xur), round(yur)); */
	fprintf(output, "%% %d %d %d %d setcache\n",
		round(xll), round(yll), round(xur), round(yur));
}

void movetohi (FILE *input, FILE *output) {		/* code 2 */
	cx = getword(input); cy = getword(input);
/*	if (trace) printf("%d %d m\n", round(cx), round(cy)); */
	fprintf(output, "%d %d m\n", round(cx), round(cy));
}

void linetohi (FILE *input, FILE *output) {		/* code 3 */
	cx = getword(input); cy = getword(input);
/*	if (trace) printf("%d %d l\n", round(cx), round(cy)); */
	fprintf(output, "%d %d l\n", round(cx), round(cy)); 
/*	fprintf(output, "%d %d l %% 3\n", round(cx), round(cy)); */
}

void curvetohi (FILE *input, FILE *output) {		/* code 4 */
	int ax, ay, bx, by;
	ax = getword(input); ay = getword(input);
	bx = getword(input); by = getword(input);
	cx = getword(input); cy = getword(input);
/*	if (trace) printf("%d %d %d %d %d %d c\n",
		round(ax), round(ay), round(bx), round(by), round(cx), round(cy)); */
	fprintf(output, "%d %d %d %d %d %d c\n",
		round(ax), round(ay), round(bx), round(by), round(cx), round(cy));
}

void docharhi (FILE *input, FILE *output) {		/* code 5 */
	int a, b, c, d;
	a = getword(input);
	b = getword(input);
	c = getword(input);
	d = getword(input);
	getname (input);
/*	if (trace) printf("%d %d %d %d %s\n",
		round(a), round(b), round(c), round(d), subname); */
	fprintf(output, "%d %d %d %d %s\n",
		round(a), round(b), round(c), round(d), subname);
}

void vlinetolong (FILE *input, FILE *output) {		/* code 6 */
	cy += getword(input); 
/*	if (trace) printf("%d %d l\n", round(cx), round(cy)); */
	fprintf(output, "%d %d l\n", round(cx), round(cy)); 
/*	fprintf(output, "%d %d l %% 6\n", round(cx), round(cy)); */
}

void hlinetolong (FILE *input, FILE *output) {		/* code 7 */
	cx += getword(input); 
/*	if (trace) printf("%d %d l\n", round(cx), round(cy)); */
	fprintf(output, "%d %d l\n", round(cx), round(cy)); 
/*	fprintf(output, "%d %d l %% 7\n", round(cx), round(cy)); */
}

void hlinetoshort (FILE *input, FILE *output) {		/* code 8 */
	cy += getbyte(input); 
/*	if (trace) printf("%d %d l\n", round(cx), round(cy)); */
	fprintf(output, "%d %d l\n", round(cx), round(cy)); 
/*	fprintf(output, "%d %d l %% 8\n", round(cx), round(cy)); */
}

void vlinetoshort (FILE *input, FILE *output) {		/* code 9 */
	cx += getbyte(input); 
/*	if (trace) printf("%d %d l\n", round(cx), round(cy)); */
	fprintf(output, "%d %d l\n", round(cx), round(cy)); 
/*	fprintf(output, "%d %d l %% 9\n", round(cx), round(cy)); */
}

void rlinetoshort (FILE *input, FILE *output) {		/* code 10 */
	cx += getbyte(input); 	cy += getbyte(input); 
/*	if (trace) printf("%d %d l\n", round(cx), round(cy)); */
	fprintf(output, "%d %d l\n", round(cx), round(cy)); 
/*	fprintf(output, "%d %d l %% 10\n", round(cx), round(cy)); */
}

void rcurvetoshort (FILE *input, FILE *output) {		/* code 11 */
	int ax, ay, bx, by;
	ax = getbyte(input) + cx; 	ay = getbyte(input) + cy;
	bx = getbyte(input) + cx; 	by = getbyte(input) + cy;
	cx = getbyte(input) + cx;	cy = getbyte(input) + cy;
/*	if (trace) printf("%d %d %d %d %d %d c\n",
		round(ax), round(ay), round(bx), round(by), round(cx), round(cy)); */
	fprintf(output, "%d %d %d %d %d %d c\n",
		round(ax), round(ay), round(bx), round(by), round(cx), round(cy));
}

void closepathhi(FILE *input, FILE *output) {			/* code 13 */
/*	if (trace) printf("h\n"); */
	fprintf(output, "h\n");
}

void strokeeofill (FILE *input, FILE *output) {		/* code 14 */
/*	printf("eofill\n"); */
/*	if (trace) printf("]\n"); */
	fprintf(output, "]\n");
}

void strokefill (FILE *input, FILE *output) {		/* code 15 */
/*	printf("fill\n"); */
/*	if (trace) printf("]\n"); */
	fprintf(output, "]\n");
}

/* *********************************************************************** */

/* Decode Fontographer CharStrings */

int	decodecharhigh (FILE *output, FILE *input) {
	int code;
	if (trace) printf("Decoding character\n");
	for (;;) {
		code = getnext(input);
		switch(code) {
			case 0: charstarthi (input, output); break;
			case 1: setcachehi (input, output); break;
			case 2: movetohi (input, output); break;	 /* absolute - word */
			case 3: linetohi (input, output); break;	 /* absolute - word */
			case 4: curvetohi (input, output); break;	 /* absolute - word */
			case 5: docharhi (input, output); break;	 /* getname dochar */
			case 6: vlinetolong (input, output); break;	 /* relative - word */
			case 7: hlinetolong (input, output); break;	 /* relative - word */
			case 8: hlinetoshort (input, output); break; /* relative - short */
			case 9: vlinetoshort (input, output); break; /* relative - short */
			case 10: rlinetoshort (input, output); break; /* relative - short */
			case 11: rcurvetoshort (input, output); break; /* relative - short */
		 /*		case 12: break; */
			case 13: closepathhi(input, output); break;
			case 14: strokeeofill(input, output); break; /* the end */
			case 15: strokefill(input, output); break;	 /* the end */
			default:
				fprintf(stderr, "Unknown code %d (%x)\n", code, code);
			exit(1);
		}
		if (code == 14 || code == 15) break;		/* the end */
	}
	return 0;
}

/* *********************************************************************** */

int scantocharlow (FILE *input) {
	int c, k=0;
	char *s;
	if (trace) printf("Scanning to character\n"); 
	while ((c = getc(input)) != EOF && c != '/') ;
	if (c == EOF) {
		if (verbose) printf("No more characters\n");
		return -1;
	}
	s = charname;
	while ((c = getc(input)) != EOF && c != '<' && c != '{' && c != ' ') {
		*s++ = (char) c;
		if (k++ >= MAXCHARNAME) {
			fprintf(stderr, "Character name too long %s", charname);
			exit(1);
		}
	}
	if (c == EOF) {
		if (verbose) printf("EOF in character\n");
		return -1;
	}
	*s = '\0';
	if (c == '{') {
		while ((c = getc(input)) != EOF && c != '}') ;
		if (trace) printf("Ignoring {...}\n");
		return scantocharlow (input);
	}
	if (strcmp(charname, ".notdef") == 0) {
		if (trace) printf("Ignoring .notdef\n");
		return scantocharlow (input);
	}
	if (strcmp(charname, "currentpacking") == 0) return -1;
	if (verbose) printf("Character: %s\n", charname);
	return 0;
}

int scantocharhigh (FILE *input) {
	int c, k=0;
	char *s;

	if (trace) printf("Scanning to character\n");
	while ((c = getc(input)) != EOF && c != '\n') ;
	c = getc(input);
	ungetc(c, input);
	if (c == '/') {
		if (verbose) printf("No more characters\n");
		return -1;		/* end of CharStrings */
	}
	s = line;
	for (;;) {
		while ((c = getc(input)) != EOF && c != '<' && c != '\n')
			*s++ = (char) c;
		if (c != '\n') break;			/* ignore lines without < */
	}
	*s = '\0';
	if (c == EOF) {
		if (verbose) printf("No more characters\n");
		return -1;
	}
	if (sscanf (line, "%d /%s", &chr, &charname) < 2) {
		fprintf(stderr, line); return -1;
	}
/*	if (verbose) printf("%d %d %% %s\n", chr, 0, charname); */
	return 0;
}

void scantocharstrings (FILE *input) {
	if (trace) printf("Scanning to CharStrings\n");
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "CharStrings begin", 17) == 0) {
			hires = 0;					/* one type */
/*			if (trace) printf("Low Res Type of Font\n"); */
			if (trace) printf("Fontographer Font\n");
			return;
		}
		if (strncmp(line, "/CharStrings", 12) == 0) {
			hires = 1;					/* another type */
/*			if (trace) printf("Hi Res Type of Font\n"); */
			if (trace) printf("Ikarus Font\n");
			return;
		}		
	}
	fprintf(stderr, "Can't find CharStrings\n");
	exit(1);
}

void decodefont(FILE *output, FILE *input) {
	if (trace) printf("Decoding a font\n");
	fprintf(output, "%lg %lg\n", 10.0, 1.0);
	fprintf(output, "%d %d %d %d\n", 0, -200, 1000, 1000);
	fprintf(output, "]\n");			/* maybe in hires mode only ? */
	scantocharstrings(input);
	for (;;) {
		if (hires) {
			if (scantocharhigh (input) != 0) break;
			decodecharhigh (output, input);
		}
		else {
			if (scantocharlow (input) != 0) break;
			decodecharlow (output, input);
		}
	}
	if (verbose) printf("Decoded %d characters\n", chr);
}

/* *********************************************************************** */

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

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	if (argc < 2) exit(1);
	strcpy(infilename, argv[1]);
	extension (infilename, "ps");
	if ((input = fopen(infilename, "r")) == NULL) {
		forceexten (infilename, "pfa");
		if ((input = fopen(infilename, "r")) == NULL) {
			perror(infilename); exit(1);
		}
	}
	strcpy(outfilename, stripname(argv[1]));
	extension (outfilename, "out");
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename); exit(1);
	}
/*	decodefont (stdout, input); */
	decodefont (output, input); 
	fclose (output);
	fclose (input);
	return 0;
}

/* Fontographer produces outlines running in the wrong direction */
