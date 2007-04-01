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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFLEN 512
#define MAXNAME 32

char buffer[BUFLEN];

char charname[MAXNAME];

int charside, charwidth;

char encoding[256] = {
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
"endash", 
"dagger", 
"daggerdbl", 
"periodcentered", 
"paragraph", 
"bullet", 
"quotesinglbase", 
"quotedblbase", 
"quotedblright", 
"guillemotright", 
"ellipsis", 
"perthousand", 
"questiondown", 
"grave", 
"acute", 
"circumflex", 
"tilde", 
"macron", 
"breve", 
"dotaccent", 
"dieresis", 
"ring", 
"cedilla", 
"hungarumlaut", 
"ogonek", 
"caron", 
"emdash", 
"AE", 
"ordfeminine", 
"Lslash", 
"Oslash", 
"OE", 
"ordmasculine", 
"ae", 
"dotlessi", 
"lslash", 
"oslash", 
"oe", 
"germandbls", 


int main(int argc, arg *argv[]) {
	while (fgets(stdin, buffer, BUFLEN) > 0) {
		if (strchr(buffer, "/Encoding") != NULL) break;
	}


	while (fgets(stdin, buffer, BUFLEN) > 0) {
		if (strchr(buffer, "/CharStrings") != NULL) break;
	}
	while (fgets(stdin, buffer, BUFLEN) > 0) {
		if (*buffer = '/') {
			sscanf(buffer, "/%s", &charname);
			fprintf(stdout, "N charname ; ");
			fgets(stdin, buffer, BUFLEN);
			sscanf(buffer, "%d %d hsbw", &charside, &charwidth);
			fprintf(stdout, "WX %d ;", charwidth);
		}
		if (strchr(buffer, "/FontName") != NULL) break;
	}	

}
