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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* #include <conio.h> */

/* convert Ikarus M Type 3 format to canonical outlines */

#define MAXFILENAME 128
#define MAXLINE 256
#define MAXCHARNAME 32
#define MAXFONTNAME 64
#define MAXCHAR 512

int verboseflag = 1;
int traceflag = 0;
int scaling = 0;		/* non-zero if scaling output */

char charnames[MAXCHAR][MAXCHARNAME];

char *bufptr;

char line[MAXLINE];

int finish;

int replenish (FILE *input) {
	if (fgets(line, MAXLINE, input) == NULL) {
		fprintf(stderr, "Unexpected EOF (replenish)\n");
		exit(11);
	}
	if (traceflag != 0) printf ("NEXT LINE: %s", line);
	bufptr = line;
	return 0;
}

unsigned int readbyte (FILE *input) {
	unsigned int c, d;
	while ((c = (unsigned int) *bufptr++) <= ' ') {
		if (c < ' ') (void) replenish (input);
	}
	while ((d = (unsigned int) *bufptr++) <= ' ') {
		if (d < ' ') (void) replenish (input); 	
	}
	if (c == '>' || d == '>') {
		finish = -1;
		return 0;
	}
	if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
	else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
	else if (c >= '0' && c <= '9') c = c - '0';
	else fprintf(stderr, "BAD CHAR %c (%d) ", c, c);
	if (d >= 'A' && d <= 'F') d = d - 'A' + 10;
	else if (d >= 'a' && d <= 'f') d = d - 'a' + 10;
	else if (d >= '0' && d <= '9') d = d - '0';
	else fprintf(stderr, "BAD CHAR %c (%d) ", d, d);
	return ((c << 4) | d);
}

int getbyte (FILE *input) {
	return (int) readbyte(input) - 128;
}

unsigned int readword(FILE *input) {
	unsigned int a, b;
	a = readbyte(input); b = readbyte(input);
	return ((a << 8) | b);
}

int getword (FILE *input) {
	unsigned int wrd;
	wrd = readword(input);
	return (int) (wrd - 32768);
}

/* int gettoken (char *buff, FILE *input) {
	int c, n=0;
	char *s=buff;

	for (;;) {
		c = getc(input);
		if (c == EOF) return EOF;
		if (n++ >= MAXLINE) return n;
		if (c <= ' ') {
			*s++ = '\0';
			if (traceflag != 0) printf("%s\n", buff);
			return n;
		}
		else *s++ = (char) c;
	}
} */

void getname(FILE *input, char *name) {
	int k, len;
	char *s = name;

	len = readbyte(input);
/*	printf("%d ", len); */
	for (k = 0; k < len; k++) *s++ = readbyte(input);
	*s++ = '\0';
}

int scale(int n) {
	if (scaling == 0) return n;
	else if (n < 0) return (- scale(-n));
	else return ((n + 7) / 15);
}

int	scaninchar(FILE *input, FILE *output, char *charname, int chrs) {
	char *s;
	int wid, ax, ay, bx, by, cx, cy;
	int xll, yll, xur, yur;
	int para, parb, parc, pard;
	unsigned int byte;
	char component[MAXFONTNAME];
	
	if ((s = strchr(line, '<')) == NULL) {
		fprintf(stderr, "Expecting line with < not: %s", line);
		return -1;
	}
	if (traceflag != 0) printf ("NEW LINE: %s", s);
	bufptr = s+1;
	fprintf(output, "]\n");

	finish = 0;
	cx = 0; cy = 0;

	for(;;) {
		byte = readbyte(input);
		switch(byte) {
			case 0:									/* width */
				wid = getword(input); 
				fprintf(output, "%d %d %% %s\n", chrs, scale(wid), charname);
				if (verboseflag != 0)
					printf("%d %d %% %s ", chrs, scale(wid), charname);
				break;

			case 1:		/* setcache device */		/* newpath */
				xll = getword(input); yll = getword(input);
				xur = getword(input); yur = getword(input);
				if (traceflag != 0) 
					printf("BBOX: %d %d %d %d \n", 
						scale(xll), scale(yll), scale(xur), scale(yur));
				break;

			case 2:									/* moveto */
				cx = getword(input); cy = getword(input);
				fprintf(output, "%d %d m\n", scale(cx), scale(cy));
				break;
				
			case 3:									/* lineto */
				cx = getword(input); cy = getword(input);
				fprintf(output, "%d %d l\n", scale(cx), scale(cy));
				break;
				
			case 4:									/* curveto */
				ax = getword(input); ay = getword(input);
				bx = getword(input); by = getword(input);
				cx = getword(input); cy = getword(input);				
				fprintf(output, "%d %d %d %d %d %d c\n", 
					scale(ax), scale(ay), scale(bx), scale(by), scale(cx), scale(cy));
				break;
				
			case 5:								/* char call */
/*				fprintf(stderr, "Character call not implemented (%s)\n", 
					charname); */
				para = getword(input); parb = getword(input);
				parc = getword(input); pard = getword(input);
				getname(input, component);
				if (verboseflag != 0) printf("PCC %s ", component);
				if (verboseflag != 0)	 {
					if (para == 256 && parb == 256) 
						printf("%d %d ; ", parc, pard);
					else printf("%d %d %d %d ; ", para, parb, parc, pard); 
				}
/*				putc('\n', stdout); */
				break;
				
			case 6:								/* 0 dy rlineto */
				cy += getword(input);
				fprintf(output, "%d %d l\n", scale(cx), scale(cy));
				break;

			case 7:								/* dx 0 rlineto */
				cx += getword(input);
				fprintf(output, "%d %d l\n", scale(cx), scale(cy));
				break;

			case 8:								/* 0 dy rlineto */
				cy += getbyte(input);
				fprintf(output, "%d %d l\n", scale(cx), scale(cy));
				break;
				
			case 9:								/* dx 0 rlineto */
				cx += getbyte(input);
				fprintf(output, "%d %d l\n", scale(cx), scale(cy));
				break;				
				
			case 10:							/* dx dy rlineto */
				cx += getbyte(input); cy += getbyte(input);
				fprintf(output, "%d %d l\n", scale(cx), scale(cy));
				break;	
				
			case 11:							/* rcurveto */
				ax = cx + getbyte(input); ay = cy + getbyte(input);	
				bx = cx + getbyte(input); by = cy + getbyte(input);	
				cx = cx + getbyte(input); cy = cy + getbyte(input);	
				fprintf(output, "%d %d %d %d %d %d c\n", 
					scale(ax), scale(ay), scale(bx), scale(by), 
						scale(cx), scale(cy));
				break;
				
			case 12:
				fprintf(stderr, "Invalid code %d (%s)\n", byte, charname);
				break;
			
			case 13:							/* closepath */
				fprintf(output, "h\n");
				break;
				
			case 14:							/* stroke/eofill */
				finish = -1;
				if (traceflag != 0) printf("FINISH\n");
				if (verboseflag != 0) putc('\n', stdout);
				break;
			
			case 15:							/* stroke/fill */
				finish = -1;
				if (traceflag != 0) printf("FINISH\n");
				if (verboseflag != 0) putc('\n', stdout);
				break;
			
			default:
				fprintf(stderr, "Invalid code %d (%s)\n", byte, charname);
				break;
		}
		if (finish != 0) break;		
	}
	return 0;
}

int main(int argc, char *argv[]) {
	FILE *input, *output;
	char infile[MAXFILENAME], outfile[MAXFILENAME];
	int m;
	int chrs, ndict=0;
	char charname[MAXCHARNAME];
	char fontname[MAXFONTNAME]="";	
	char *s;
	int firstarg=1;

	if (firstarg == argc) return -1;
	
	if (*argv[firstarg] == '-') {
		scaling++; firstarg++;
	}

	if (argc < firstarg+1) {
		fprintf(stderr, "No file name given\n");
		exit(1);
	}

	for (m = firstarg; m < argc; m++) {

		strcpy(infile, argv[m]);
		if (strchr(infile, '.') == NULL) strcat(infile, ".ps");
		if ((input = fopen(infile, "r")) == NULL) {
			perror(infile);
			exit(1);
		}
		strcpy(outfile, infile);
		if ((s = strchr(outfile, '.')) == NULL) {
			fprintf(stderr, "Can't find extension\n");
			exit(3);
		}
		strcpy(s+1, "out");
		if ((output = fopen(outfile, "w")) == NULL) {
			perror(outfile);
			exit(1);
		}

		fprintf(output, "%lg %lg\n", 10.0, 1.0);
		fprintf(output, "%d %d %d %d\n", 0, -300, 1000, 800);

		for (;;) {
			if (fgets(line, MAXLINE, input)  == NULL) {
				fprintf(stderr, "Unexpected EOF (main)\n");
				exit(5);
			}
			if (strncmp(line, "%%BeginFont:", 12) == 0) 
				sscanf(line, "%%%%BeginFont: %s", fontname);
			if (strncmp(line, "/CharStrings", 12) == 0) 
				sscanf(line, "/CharStrings %d dict def", &ndict);
			if (strchr(line, '<') != NULL ||
				strchr(line, '>') != NULL) break;
		}

		if (verboseflag != 0 && ndict > 0)
			printf("FONTNAME: %s NDICT: %d\n", fontname, ndict);

		for(;;) {
			if (strncmp(line, "%%EOF", 5) == 0) break;
			else if (*line == '%') {
				fgets(line, MAXLINE, input); continue;
			}
			else if (strncmp(line, "DefChar", 7) == 0) {
				fgets(line, MAXLINE, input); continue;
			}
			else if (*line == '/') break;
			else if (strstr(line, "/.notdef") != NULL) {
				strcpy(charname, ".notdef"); chrs = MAXCHAR-1;
			}
			else if (sscanf(line, "%d /%s", &chrs, charname) < 2) {
				fprintf(stderr, "Don't understand: %s\n", line);
				if (fgets(line, MAXLINE, input)  == NULL) {
					fprintf(stderr, "Unexpected EOF (next char)\n");
					break;
				}
				continue;
			}
			if (traceflag != 0) 
				printf("CHARNAME: %s CHARCODE: %d\n", charname, chrs);
			strncpy(charnames[chrs], charname, MAXCHARNAME);

			if(scaninchar(input, output, charname, chrs) < 0) {
				if (verboseflag != 0) printf("SCANINCHAR < 0\n");
				break;
			}
			
			if (fgets(line, MAXLINE, input)  == NULL) {
				fprintf(stderr, "Unexpected EOF (next char)\n");
				break;
			}
			if (strchr(line, '<') == NULL) {
				if (traceflag != 0) printf("LINE DOES NOT CONTAIN <\n");
/*				break; */
			}
		}
		
		fclose(input);
		if (fclose(output) == EOF) {
			perror(outfile); exit(9);
		}
	}
	return 0;
}

