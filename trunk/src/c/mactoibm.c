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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *encodingin[256]={
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
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "del", 
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
"daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase", "perthousand", "Acircumflex", "Ecircumflex", "Aacute", 
"Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex", 
"apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde", 
"macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron"
};

char *encodingout[256]={
"notdef", "smileface", "invsmileface", "heart", "diamond", "club", "spade", "bullet", 
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
"ydieresis", "Odieresis", "Udieresis", "cent", "sterling", "yen", "peseta", "florin", 
"aacute", "iacute", "oacute", "uacute", "ntilde", "Ntilde", "ordfeminine", "ordmasculine", 
"questiondown", "revlogicalnot", "logicalnot", "onehalf", "onequarter", "exclamdown", "guillemotleft", "guillemotright", 
"ltshade", "shade", "dkshade", "2502", "2524", "2561", "2562", "2556", 
"2555", "2563", "2551", "2557", "255d", "255c", "255b", "2510", 
"2514", "2534", "252c", "251c", "2500", "253c", "255e", "255f", 
"255a", "2554", "2569", "2566", "2560", "2550", "256c", "2567", 
"2568", "2564", "2565", "2559", "2558", "2552", "2553", "256b", 
"256a", "2518", "250c", "block", "dnblock", "lfblock", "rtblock", "upblock", 
"alpha", "germandbls", "Gamma", "pi", "summation", "sigma", "mu", "tau", 
"Phi", "Theta", "Omega", "delta", "infinity", "phi", "epsilon", "intersection", 
"equivalence", "plusminus", "greaterequal", "lessequal", "integraltp", "integralbt", "divide", "approxequal", 
"degree", "middot", "bulletsml", "radical", "nsuperior", "twosuperior", "filledbox", "nbspace"
};

int remap[256];

int lookup (char *name, char *encoding[]) {
	int k;
	for (k = 0; k < 256; k++) 
		if (strcmp(name, encoding[k]) == 0) return k;
	return -1;
}

void mapremap (int flag) {
	int k;
	for (k = 0; k < 256; k++) {
		if (strcmp(encodingin[k], "") == 0) remap[k] = k;
		else {
			remap[k] = lookup (encodingin[k], encodingout);
			if (remap[k] < 0)  {
				if (strncmp(encodingin[k], "quotedbl", 8) == 0) 
					remap[k] = lookup ("quotedbl", encodingout);
				else if (flag) remap[k] = k;
			}
		}
	}
}

void remapfile (FILE *output, FILE *input) {
	int c, d;
	while ((c = getc(input)) != EOF) {
		d = remap[c];
		if (d < 0) {
			printf("char %d %s ", c, encodingin[c]);
			d = c;
		} 
		putc(d, output);
	}
}

int main(int argc, char *argv[]) {
	char infile[255], outfile[255];
	FILE *input, *output;
	char *s;

	if (argc < 2) exit(1);
	strcpy(infile, argv[1]);
	if ((input = fopen(infile, "rb")) == NULL) {
		perror(infile); exit(1);
	}
	strcpy(outfile, argv[1]);
	if ((s = strrchr(outfile, '.')) == NULL) strcat(outfile, ".");
	if ((s = strrchr(outfile, '.')) == NULL) exit(1);
	strcpy (s+1, "ibm");
	if ((output = fopen(outfile, "wb")) == NULL) {
		perror(outfile); exit(1);
	}
	mapremap(0);	
	remapfile(output, input);
	fclose(output);
	fclose(input);
	return 0;
}
