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

/* code for recovering APL stroke font info */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXSTACK 128

#define MAXLINE 128

#define MAXBUFFER 1024

#define MAXCHARSTRING 512

#define MAXCHARNAME 32

#define MAXKNOTS 256

/* int stackPS[MAXSTACK]; */

double stackPS[MAXSTACK];

int stackindex=0;

char line[MAXLINE];

char buffer[MAXBUFFER];

char charname[MAXCHARNAME];

unsigned char charstring[MAXCHARSTRING];

double xknot[MAXKNOTS];

double yknot[MAXKNOTS];

int code[MAXKNOTS];

int knotindex;

double designsize=10.0;

double scale=1.0;

double wx, wy;
double xll, yll, xur, yur;

int verboseflag=1;
int traceflag=0;
int debugflag=0;
int showname=0;
int showcache=0;

int strk = 0;

int strokeflag;
int closedflag;

char *sample="/DLE<64DE6470DA24D6B8DEA4DBEE65F164DB64D6E96464DDEAC4DA64EAF7>def";

char *macencoding[256] = { 
	"NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
	"BS", "HT", "LF", "VT", "FF", "CR", "SO", "SI",
	"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
	"CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US",
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
	"registersans", "copyrightsans", "trademarksans", "acute", "dieresis", "notequal", "AE", "Oslash",
	"infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu", "partialdiff", "summation",
	"product", "pi", "integral", "ordfeminine", "ordmasculine", "Omega", "ae", "oslash",
	"questiondown", "exclamdown", "logicalnot", "radical", "florin", "approxequal", "Delta", "guillemotleft",
	"guillemotright", "ellipsis", "nbspace", "Agrave", "Atilde", "Otilde", "OE", "oe",
	"endash", "emdash", "quotedblleft", "quotedblright", "quoteleft", "quoteright", "divide", "lozenge",
	"ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft", "guilsinglright", "fi", "fl",
	"daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase",
	"perthousand", "Acircumflex", "Ecircumflex", "Aacute",
	"Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex",
	"apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde",
	"macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron"
}; 

/*********************************************************************************/

int lookup(char *charname) {
	int k;
	for (k = 0; k < 256; k++) {
		if (strcmp(macencoding[k], charname) == 0) return k;
	}
	printf("ERROR: %s not found\n", charname);
	return -1;
}

void showreverse (FILE *output) {
	int k;
	int oldcode;

	if (knotindex == 0) return;

#ifdef DEBUGGING
	for (k = 0; k < knotindex; k++) {
		fprintf(output, "\t%lg %lg %c\n", xknot[k], yknot[k], code[k]);
	}
#endif

	fprintf(output, "%% reverse\n");
	if (code[knotindex-1] == 'h') oldcode='m';
	else oldcode='l';
	k = knotindex-1;
/*	for (k = knotindex-1; k >= 0; k--) { */
	while (k >= 0) {
		if (code[k] != ' ') {
			switch (oldcode) {
				case 'm':
/*					fprintf(output, "%lg %lg m %% %d\n", xknot[k], yknot[k], k); */
					fprintf(output, "%lg %lg m\n", xknot[k], yknot[k]);
					oldcode = code[k];
					k--;
					break;
				case 'l':
/*					fprintf(output, "%lg %lg l %% %d\n", xknot[k], yknot[k], k); */
					fprintf(output, "%lg %lg l\n", xknot[k], yknot[k]);
					oldcode = code[k];
					k--;
					break;
				case 'c':
/*					fprintf(output, "%lg %lg %lg %lg %lg %lg c %% %d\n", 
						   xknot[k+2], yknot[k+2],
						   xknot[k+1], yknot[k+1],
						   xknot[k], yknot[k], k); */
					fprintf(output, "%lg %lg %lg %lg %lg %lg c\n", 
							xknot[k+2], yknot[k+2],
							xknot[k+1], yknot[k+1],
							xknot[k], yknot[k]);
					oldcode = code[k];
					k--;
					break;
				case 'h':
					fprintf(output, "%lg %lg l\n", xknot[k], yknot[k]); /* ??? */
					oldcode = code[k];
					k--;
					break;
				default:
					fprintf(output, "Code %c (%d)? %% %d\n", oldcode, oldcode, k);
					oldcode = code[k];
					k--;
					break;
			}
		}
		else k--;
	}
	fprintf(output, "h\n");
	knotindex = 0;
}

int convert(unsigned char *charstring, char *buffer) {
	char *s=buffer;
	unsigned char *t = charstring;
	int c, d;
	int nlen=0;
	if (traceflag) printf("%s\n", buffer);
	while (*s++ != '<') ;
	while (*s != '>' && *s > ' ') {
		c = *s++;
		d = *s++;
		if (debugflag) printf("c %d d %d ", c, d);
		if (c == '>' || c == '\0') break;
		if (d == '>' || d == '\0') break;
		if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
		else if (c >= '0' && c <= '9') c = c - '0';
		else printf ("ERROR c %d\n", c);
		if (d >= 'A' && d <= 'F') d = d - 'A' + 10;
		else if (d >= 'a' && d <= 'f') d = d - 'a' + 10;
		else if (d >= '0' && d <= '9') d = d - '0';
		else printf ("ERROR d %d\n", d);
		if (debugflag) printf("out %2X ", (c << 4) | d);
		*t++ = (unsigned char) ((c << 4) | d);
		nlen++;
	}
	if (traceflag) printf("Stopped on %d\n", *s);
	return nlen;
}

void resetstackPS (void) {
	stackindex = 0;
}

void checkstackPS (void) {
	if (stackindex != 0)
		printf("ERROR: Stack not empty %d\n", stackindex);
}

double pushPS (double c) {
	stackPS[stackindex++] = c;
	if (stackindex >= MAXSTACK) {
		printf("ERROR: Stack overflow\n");
		exit(1);
	}
	return c;
}

double popPS (void) {
	double c; 
	c = stackPS[--stackindex];
	if (stackindex < 0) {
		printf("ERROR: Stack underflow\n");
/*		exit(1); */
		stackindex = 1;
	}
	return c;
}

double x, y;

void closepath(FILE *output) {
/*	fprintf(output, "closepath\n"); */
	fprintf(output, "h\n");
	closedflag = 1;
	if (strokeflag) {
		xknot[knotindex] = xknot[0];
		yknot[knotindex] = yknot[0];
		code[knotindex] = 'h';
		knotindex++;
		if (knotindex >= MAXKNOTS) exit(1);		
	}
}

void rmoveto (FILE *output) {
	double dx, dy;
	dy = popPS();
	dx = popPS();
	x += dx;
	y += dy;
	if (strokeflag) {
		if (knotindex > 0) showreverse(output);
	}
/*	fprintf(output, ("%lg %lg moveto\n", x, y); */
	fprintf(output, "%lg %lg m\n", x, y);
	if (strokeflag) {
		xknot[knotindex] = x;
		yknot[knotindex] = y;
		code[knotindex] = 'm';
		knotindex++;
		if (knotindex >= MAXKNOTS) exit(1);
	}
	closedflag = 0;
}

void rlineto (FILE *output) {
	double dx, dy;
	dy = popPS();
	dx = popPS();
	x += dx;
	y += dy;
/*	fprintf(output, ("%lg %lg lineto\n", x, y); */
	fprintf(output, "%lg %lg l\n", x, y);
	if (strokeflag) {
		xknot[knotindex] = x;
		yknot[knotindex] = y;
		code[knotindex] = 'l';
		knotindex++;
		if (knotindex >= MAXKNOTS) exit(1);
	}
}

void rcurveto (FILE *output) {
	double dxa, dya, dxb, dyb, dxc, dyc;
	double xa, ya, xb, yb, xc, yc;
	dyc = popPS();
	dxc = popPS();
	dyb = popPS();
	dxb = popPS();
	dya = popPS();
	dxa = popPS();

	xa = x + dxa;
	ya = y + dya;
	xb = x + dxb;
	yb = y + dyb;
	xc = x + dxc;
	yc = y + dyc;
	x = xc;
	y = yc;
/*	fprintf(output, ("%lg %lg %lg %lg %lg %lg curveto\n", xa, ya, xb, yb, xc, yc); */
	fprintf(output, "%lg %lg %lg %lg %lg %lg c\n", xa, ya, xb, yb, xc, yc);
	if (strokeflag) {
		xknot[knotindex] = xa;
		yknot[knotindex] = ya;
		code[knotindex] = ' ';
		knotindex++;
		if (knotindex >= MAXKNOTS) exit(1);
		xknot[knotindex] = xb;
		yknot[knotindex] = yb;
		code[knotindex] = ' ';
		knotindex++;
		if (knotindex >= MAXKNOTS) exit(1);
		xknot[knotindex] = xc;
		yknot[knotindex] = yc;
		code[knotindex] = 'c';
		knotindex++;
		if (knotindex >= MAXKNOTS) exit(1);
	}

}

void ShowExt(FILE *output) {
	fprintf(output, "ShowExt();\n");
}

void concat(FILE *output) {
	fprintf(output, "concat();\n");
}

void Cache(FILE *output) {
	int k;
/*	fprintf(output, "cache %d: ", stackindex); */
/*	for (k = stackindex-1; k >= 0; k--) fprintf(output, "%lg ", popPS()); */

/*	for (k = stackindex-6; k < stackindex; k++)
		fprintf(output, "%lg ", stackPS[k]); */
	yur = popPS();
	xur = popPS();
	yll = popPS();
	xll = popPS();
	wy = popPS();
	wx = popPS();
	if (showcache) {
		fprintf(output, "%lg %lg %lg %lg %lg %lg ", wx, wy, xll, yll, xur, yur);
		fprintf(output, "setcachedevice\n", stackindex);
	}
/*	stackindex = stackindex - 6; */
	x = y = 0;
	strokeflag = 0;
	knotindex = 0;
	closedflag = 0;
	k = lookup(charname);
	fprintf(output, "%d %lg %% %s\n", k, wx, charname);
/*	stackindex = 0; */
}

void setlinewidth(FILE *output) {
	double linewidth;
	linewidth = popPS();
	fprintf(output, "%% %lg setlinewidth\n", linewidth);
}

void ShowInt(FILE *output) {
	fprintf(output, "ShowInt\n");
}

void setlinecap(FILE *output) {
	double linecap;
	linecap = popPS();
/*	fprintf(output, "%lg setlinecap\n", linecap); */
	fprintf(output, "%% %lg setlinecap\n", linecap);
	strokeflag = 1;
	knotindex = 0;
}

void setlinejoin(FILE *output) {
	double linejoin;
	linejoin = popPS();
/*	fprintf(output, ("%lg setlinejoin\n", linejoin); */
	fprintf(output, "%% %lg setlinejoin\n", linejoin);
	strokeflag = 1;
	knotindex = 0;
}

void gsave(FILE *output) {
	fprintf(output, "gsave\n");
}

void mark(FILE *output) {
	fprintf(output, "mark\n");
}

void Fill(FILE *output) {
/*	fprintf(output, "fill\n"); */
	fprintf(output, "%% fill\n");
	if (closedflag == 0) closepath(output);
}

void Eofill(FILE *output) {
	fprintf(output, "eofill\n");
}

void stroke(FILE *output) {
/*	fprintf(output, "stroke\n"); */
	fprintf(output, "%% stroke\n");
	showreverse(output);
/*	fprintf(output, "h\n"); */
}

void SetWid(FILE *output) {
	fprintf(output, "SetWid\n");
}

void Cp(FILE *output) {
	x = y = 0;
	closepath(output);
}

void Sstrk(FILE *output) {
	fprintf(output, "Sstrk\n");
	strk = 1;
}

void setgray(FILE *output) {
	fprintf(output, "setgray\n");
}

void hundredmuladd(void) {
	double new;
	new = popPS() * 100 + popPS();
	printf("hundredmuladd %lg\n", new);
	pushPS(new); 
}

void hundredmul(void) {
	double new;
	new = popPS() * 100;
	printf("hundredmul %lg\n", new);
	pushPS(new); 
}

void hundreddiv(void) {
	double new;
	new = popPS() / 100;
	printf("hundreddiv %lg\n", new);
	pushPS(new); 
}

void uvec(FILE *output, int k) {
	switch(k){
		case 0: rmoveto(output); break;
		case 1: rlineto(output); break;
		case 2: rcurveto(output); break;
		case 3: ShowExt(output); break;
		case 4: concat(output);	/* ] concat */ break;
		case 5: Cache(output); break;
		case 6: setlinewidth(output); break;
		case 7: ShowInt(output); break;
		case 8: setlinecap(output); break;
		case 9: setlinejoin(output); break;
		case 10: gsave(output); break;
		case 11: mark(output);	/* [ */ break;
		case 12: Fill(output); break;
		case 13: Eofill(output); break;
		case 14: stroke(output); break;
		case 15: SetWid(output); break;
		case 16: hundredmuladd(); break;
		case 17: hundredmul(); break;
		case 18: hundreddiv(); break;
		case 19: Cp(output);	/* closepath 0 0 moveto */ break;
		case 20: Sstrk(output); break;
		case 21: setgray(output); break;
		default: fprintf(output, "Unknown code %d\n"); break;
	}
}

void interpret (FILE *output, unsigned char *str, int nlen) {	/* UCS */
	unsigned char *s=str;
	int c;
	double new;
	resetstackPS();
	while (--nlen >= 0) {
		c = *s++;
		if (c == '\0') {
			if (traceflag) printf("nlen %d\n", nlen);
			break;
		}
		if (traceflag) printf("(%d) ", c);
		if (c < 200) {
			new = c - 100;
			if (traceflag) printf("pushPS %lg ", new);
			pushPS (new);
		}
		else if (c < 233) {
			new = (c - 216) * 100 + popPS();
			if (traceflag) printf("pushPS %lg ", new);
			pushPS (new);
		}
		else {
			if (traceflag) printf("uvec %d ", c - 233);
			uvec (output, c - 233);
		}
	}
	checkstackPS();
/*	putc('\n', stdout); */
	fprintf(output, "]\n");
}

int scantocharstrings(FILE *input) {
	while (fgets(line, sizeof(line), input) != NULL) {
		if (strncmp(line, "CharDefs", 8) == 0) return 0;
	}
	return -1;			/* EOF */
}


void removetrailing (char *line) {
	char *s = line + strlen(line);
	while (*s < ' ') *s-- = '\0';
}

int readcharstring(char *buffer, FILE *input) {
	char *t;
	if (fgets(line, sizeof(line), input) == NULL) return 0;	/* first line */
	if (strncmp(line, "end", 3) == 0) return 0;
	t = strchr(line, '<');
	if (t == NULL) {
		if (fgets(line, sizeof(line), input) == NULL) return 0;
		if (strncmp(line, "end", 3) == 0) return 0;
		t = strchr(line, '<');
		if (t == NULL) {
			printf("ERROR: Can't find start of CharString: %s", line);
			return 0;
		}
	}
	removetrailing(line+1);
	strncpy(charname, line+1, t - (line+1));
	charname[t-(line+1)]='\0';
	if (showname) printf("CharName: %s\n", charname);
	strcpy (buffer, t);
	if (strchr(line, '>') == NULL) {
		while (fgets(line, sizeof(line), input) != NULL) {
			removetrailing(line);
			strcat(buffer, line);
			if (strchr(line, '>') != NULL) {
				break;
			}
		}
	}
/*	else printf("LAST LINE: %s\n", line); */
	return strlen(buffer);
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

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int main (int argc, char *argv[]) {
	int nlen;
	char infilename[FILENAME_MAX];
	char outfilename[FILENAME_MAX];
	FILE *input, *output;
	
	if (argc < 2) exit(1);
	strcpy(infilename, argv[1]);
	extension(infilename, "ps");
	input = fopen(infilename, "r");
	if (input == NULL) {
		perror(infilename);
		exit(1);
	}
	strcpy(outfilename, removepath(argv[1]));
	forceexten(outfilename, "out");
	output = fopen(outfilename, "w");
	if (output == NULL) {
		perror(outfilename);
		exit(1);
	}
	fprintf(output, "%% %s\n", infilename);
	fprintf(output, "%lg %lg\n", designsize, scale);
/*	xll = -20; yll = -250; xur = 800; yur = 900; */
	xll = -20; yll = -230; xur = 620; yur = 820;
	fprintf(output, "%lg %lg %lg %lg\n", xll, yll, xur, yur);
	fprintf(output, "]\n");
	scantocharstrings(input);
	for(;;) {
		if (readcharstring(buffer, input) == 0) break;
		if (traceflag) 
			printf("Length %d characters\n", strlen(buffer));
		nlen = convert(charstring, buffer);
		if (traceflag) 
			printf("Length %d bytes\n", nlen);
		interpret(output, charstring, nlen);
	}
	fclose (output);
	fclose (input);
	return 0;
}

/* 1 setlinecap => round ends */

/* wx wy xll yll xur yur setcachedevice */

