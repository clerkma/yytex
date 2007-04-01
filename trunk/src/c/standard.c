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

/* program for expanding StandardEncoding Vector in PFB file */

/* #define FNAMELEN 80 */
#define BUFFERLEN 1024

#include <stdio.h>
#include <string.h>

char buffer[BUFFERLEN];

int verboseflag = 1;
int traceflag = 0;

int usingreturn;				/* file uses return instead of linefeed */

long asciilength, newlength, encodingline, linestart;

char *standard1 = 
"/Encoding 256 array\n"
"0 1 255 {1 index exch /.notdef put} for\n"
"dup 32 /space put\n"
"dup 33 /exclam put\n"
"dup 34 /quotedbl put\n"
"dup 35 /numbersign put\n"
"dup 36 /dollar put\n"
"dup 37 /percent put\n"
"dup 38 /ampersand put\n"
"dup 39 /quoteright put\n"
"dup 40 /parenleft put\n"
"dup 41 /parenright put\n"
"dup 42 /asterisk put\n"
"dup 43 /plus put\n"
"dup 44 /comma put\n"
"dup 45 /hyphen put\n"
"dup 46 /period put\n"
"dup 47 /slash put\n"
"dup 48 /zero put\n"
"dup 49 /one put\n"
"dup 50 /two put\n"
"dup 51 /three put\n"
"dup 52 /four put\n"
"dup 53 /five put\n"
"dup 54 /six put\n"
"dup 55 /seven put\n"
"dup 56 /eight put\n"
"dup 57 /nine put\n"
"dup 58 /colon put\n"
"dup 59 /semicolon put\n"
"dup 60 /less put\n"
"dup 61 /equal put\n"
"dup 62 /greater put\n"
"dup 63 /question put\n"
"dup 64 /at put\n"
"dup 65 /A put\n"
"dup 66 /B put\n"
"dup 67 /C put\n"
"dup 68 /D put\n"
"dup 69 /E put\n"
"dup 70 /F put\n"
"dup 71 /G put\n"
"dup 72 /H put\n"
"dup 73 /I put\n"
"dup 74 /J put\n"
"dup 75 /K put\n"
"dup 76 /L put\n"
"dup 77 /M put\n"
"dup 78 /N put\n"
"dup 79 /O put\n"
"dup 80 /P put\n"
"dup 81 /Q put\n"
"dup 82 /R put\n"
"dup 83 /S put\n"
"dup 84 /T put\n"
"dup 85 /U put\n"
"dup 86 /V put\n"
"dup 87 /W put\n"
"dup 88 /X put\n"
"dup 89 /Y put\n"
"dup 90 /Z put\n"
"dup 91 /bracketleft put\n"
"dup 92 /backslash put\n"
"dup 93 /bracketright put\n"
"dup 94 /asciicircum put\n"
"dup 95 /underscore put\n"
"dup 96 /quoteleft put\n"
"dup 97 /a put\n"
"dup 98 /b put\n"
"dup 99 /c put\n"
"dup 100 /d put\n"
"dup 101 /e put\n"
"dup 102 /f put\n"
"dup 103 /g put\n"
"dup 104 /h put\n"
"dup 105 /i put\n"
"dup 106 /j put\n"
"dup 107 /k put\n"
"dup 108 /l put\n"
"dup 109 /m put\n"
"dup 110 /n put\n"
"dup 111 /o put\n"
"dup 112 /p put\n"
"dup 113 /q put\n"
"dup 114 /r put\n"
"dup 115 /s put\n"
"dup 116 /t put\n"
"dup 117 /u put\n"
"dup 118 /v put\n"
"dup 119 /w put\n"
"dup 120 /x put\n"
"dup 121 /y put\n"
"dup 122 /z put\n"
"dup 123 /braceleft put\n"
"dup 124 /bar put\n"
"dup 125 /braceright put\n"
"dup 126 /asciitilde put\n";

char *standard2 =
"dup 161 /exclamdown put\n"
"dup 162 /cent put\n"
"dup 163 /sterling put\n"
"dup 164 /fraction put\n"
"dup 165 /yen put\n"
"dup 166 /florin put\n"
"dup 167 /section put\n"
"dup 168 /currency put\n"
"dup 169 /quotesingle put\n"
"dup 170 /quotedblleft put\n"
"dup 171 /guillemotleft put\n"
"dup 172 /guilsinglleft put\n"
"dup 173 /guilsinglright put\n"
"dup 174 /fi put\n"
"dup 175 /fl put\n"
"dup 177 /endash put\n"
"dup 178 /dagger put\n"
"dup 179 /daggerdbl put\n"
"dup 180 /periodcentered put\n"
"dup 182 /paragraph put\n"
"dup 183 /bullet put\n"
"dup 184 /quotesinglbase put\n"
"dup 185 /quotedblbase put\n"
"dup 186 /quotedblright put\n"
"dup 187 /guillemotright put\n"
"dup 188 /ellipsis put\n"
"dup 189 /perthousand put\n"
"dup 191 /questiondown put\n"
"dup 193 /grave put\n"
"dup 194 /acute put\n"
"dup 195 /circumflex put\n"
"dup 196 /tilde put\n"
"dup 197 /macron put\n"
"dup 198 /breve put\n"
"dup 199 /dotaccent put\n"
"dup 200 /dieresis put\n"
"dup 202 /ring put\n"
"dup 203 /cedilla put\n"
"dup 205 /hungarumlaut put\n"
"dup 206 /ogonek put\n"
"dup 207 /caron put\n"
"dup 208 /emdash put\n"
"dup 225 /AE put\n"
"dup 227 /ordfeminine put\n"
"dup 232 /Lslash put\n"
"dup 233 /Oslash put\n"
"dup 234 /OE put\n"
"dup 235 /ordmasculine put\n"
"dup 241 /ae put\n"
"dup 245 /dotlessi put\n"
"dup 248 /lslash put\n"
"dup 249 /oslash put\n"
"dup 250 /oe put\n"
"dup 251 /germandbls put\n"
"readonly def\n";

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef IGNORE
void extension(char *name, char *ext) {
	if (strchr(name, '.') == NULL) {
		strcat(name, ".");
		strcat(name, ext);
	}
}

void forceexten(char *name, char *ext) {
	char *s;
	if ((s = strchr(name, '.')) != NULL) *s = '\0';
	strcat(name, ".");
	strcat(name, ext);
}
#endif

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

char *stripname(char *name) {
	char *s;
	if ((s = strrchr(name, '\\')) != NULL) return (s+1);
	if ((s = strchr(name, ':')) != NULL) return (s+1);
	else return name;
}

long readfour(FILE *input) {
	int c, d, e, f;
	c = getc(input); d = getc(input); e = getc(input); f = getc(input);
	return (((long) f) << 24) | (((long) e) << 16) | (((long) d) <<	8) | c;
}

void writefour(long n, FILE *output) {
	int k;
	for (k = 0; k < 4; k++) {
		fputc((int) (n & 255), output);
		n = n >> 8;
	}
}

int readline(char *s, int n, FILE *input) {
	int c, d, k=0;

	c = getc(input);
	while (c != '\n' && c != '\r' && c != EOF) {
		if (c > 127 || (c < 32 && c != '\t' && c != '\f')) {
			if (c != 128) {
				fprintf(stderr, "Bad character (%d) in ASCII section\n", c);
				return 0;
			}
			d = getc(input);
			if (d != 1) {
				fprintf(stderr, "Bad character (%d) in ASCII section\n", c);
				return 0;
			}
			fprintf(stderr, 
"\nRecommend passing this MAC style font file through PFBTOPFA & PFATOPFB\n");
			fprintf(stderr, 
"ATM for Windows may have problems with multiple binary records");
			asciilength += readfour(input);
		}
		*s++ = (char) c;
		if (k++ > n-2) {
			fprintf(stderr, "Input line too long\n");
			return 0;
		}
		c = getc(input);		
	}
	if (c == EOF) {
		if (n == 0) {
			fprintf(stderr, "Unexpected EOF\n");
			return 0;
		}
		else {
			*s = '\0';
			return k;
		}
	}
	*s++ = (char) c;
	k++;
	if (c == '\r') {
		c = getc(input);
		if (c == '\n') {
			*s++ = (char) c;
			k++;
		}
		else {
			usingreturn = 1;
			ungetc(c, input);
		}
	}
	*s = '\0';
	return k;
}

void changereturn(char *s) {	/* \r => \n */
	int c;
	while ((c = *s) != 0) {
		if (c == '\r') *s = '\n';
		s++;
	}
}

void changelinefeed(char *s) {	/* \n => \r */
	int c;
	while ((c = *s) != 0) {
		if (c == '\n') *s = '\r';
		s++;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX], oldfilename[FILENAME_MAX];
	int firstarg = 1, filecount = 0;
	int m, c, d, renameflag, encodinglength;
	
	printf("StandardEncoding expansion program. Version 1.0\n");
	for(m = firstarg; m < argc; m++) {
		strncpy(infilename, argv[m], sizeof(infilename));
		extension(infilename, "pfb");
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;		/* file not found - probably */
		}
		else fclose(input);		
		strncpy(outfilename, stripname(argv[m]), sizeof(outfilename));	
		forceexten(outfilename, "pfb");
		renameflag = 0;
		if (strcmp(infilename, outfilename) == 0) {
			strcpy(oldfilename, infilename);
			forceexten(infilename, "bak");
			if (rename(oldfilename, infilename) == 0) renameflag = 1;
			else if (remove(infilename) == 0) {
					if (rename(oldfilename, infilename) == 0) renameflag = 1;
			}
			if (renameflag == 0) {
				fprintf(stderr, "Failed to rename input file\n");
				break;
			}
			else if (verboseflag != 0) 
				printf("Renamed %s to %s\n", oldfilename, infilename);
		}

		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;		/* 	return (1); */
		}
		
		if (verboseflag != 0) printf("Processing %s ", infilename); 

		usingreturn = 0;
		c = fgetc(input); d = fgetc(input);
		if (c != 128 || d != 1) {
			fprintf(stderr, "Not a valid PFB file\n");
			fclose(input);
			if (renameflag != 0) rename(infilename, oldfilename);
			continue;		/* return (1); */
		}

/*		if (verboseflag != 0) printf("Processing %s ", infilename); */

		encodinglength = -1;
		asciilength = readfour(input); /* read over length code */
/*		if (verboseflag != 0) 
			printf("ASCII section %ld bytes\n", asciilength); */
		for(;;) {
			linestart = ftell(input);
			if(readline(buffer, BUFFERLEN, input) == 0) {
				linestart = -1;	
				fprintf(stderr, "Unexpected EOF\n");
				break;
			}
			if (traceflag != 0) {
				if (usingreturn != 0) changereturn(buffer); /* side effect */
				printf("%s", buffer);
			}
			if (strstr(buffer, "/Encoding") != NULL) {
				if (strstr(buffer, "StandardEncoding") == NULL) {

					if (verboseflag != 0) {
						changereturn(buffer);	 /* rt => lf */
						printf(" Not StandardEncoding: %s", buffer);
					}
					linestart = -1; break;		/* not what we expected */
				}
				else {							/* found the magic line */
					encodinglength = strlen(buffer);	/* line length */
					break;
				}
			}
		}

		if(linestart < 0 || encodinglength < 0) { /* did not find line */
			fclose(input);
			if (renameflag != 0) rename(infilename, oldfilename);
			fprintf(stderr, "Did not find StandardEncoding - ");
			fprintf(stderr, "file not processed\n");
			continue;
		}

		encodingline = linestart;
		rewind(input);
		c = fgetc(input); d = fgetc(input);

		if ((output = fopen(outfilename, "wb")) == NULL) {
			perror(outfilename);
			fclose(input);
			if (renameflag != 0) rename(infilename, oldfilename);
			continue;	/* return (1); */
		}

		if (verboseflag != 0) printf(" => %s ", outfilename);

		fputc(c, output); fputc(d, output);
		asciilength = readfour(input);
		newlength = asciilength + strlen(standard1) + strlen(standard2)
			- encodinglength;
		if (verboseflag != 0) {
			if (usingreturn != 0) putc('*', stdout);
			else putc(' ', stdout);
			printf(" (ASCII %ld => %ld)\n", asciilength, newlength);
		}

		writefour(newlength, output);
		for(;;) {
			linestart = ftell(input);
			if(readline(buffer, BUFFERLEN, input) == 0) break;
			if (traceflag != 0) {
				if (usingreturn != 0) changereturn(buffer); /* side effect */
				printf("%s", buffer);
			}
			if (strstr(buffer, "/Encoding") != NULL) {
				if (strstr(buffer, "StandardEncoding") == NULL) {
					if (verboseflag != 0) 
						printf("Not StandardEncoding?\n");
					linestart = -1; break;
				}
				else {
					if (usingreturn == 0) changereturn(standard1);
					else changelinefeed(standard1);
					fprintf(output, "%s", standard1);
					if (usingreturn == 0) changereturn(standard2);
					else changelinefeed(standard2);
					fprintf(output, "%s", standard2);
					break;
				}
			} 
			else fputs(buffer, output);
		}

		while ((c = getc(input)) != EOF) putc(c, output);

		fclose(input);
		if (fclose(output) == EOF) {
			perror(outfilename);
			break;				/* output device full - probably */
		}
		else filecount++;
	}
	if (verboseflag != 0 && filecount > 0) 
		printf("Reencoded %d outline font files\n", filecount);
		
	return 0;
}
