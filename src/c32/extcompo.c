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

/* Extract composite information from OUT file + PLN file */
/* used for undoing Ikarus method in Bigelow and Holmes fonts */

/* depends strongly on particular arrangement of PLN file - see end*/

/* produces output that can be attached to AFM file under composites */
/* this in turn is used by FONTONE to generate composites using `seac' */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #define FNAMELEN 80 */
#define CHARNAME_MAX 64
#define MAXBUF 512
#define MAXCHR 256
#define MAXSUBRS 128
#define MAXCHARNAME 32
#define SINFINITY 32767

int verboseflag = 1;
int traceflag = 0;

int subrcount;

char buffer[MAXBUF];

int widths[MAXCHR];

int startx[MAXCHR], starty[MAXCHR], endx[MAXCHR], endy[MAXCHR];

char charnames[MAXCHR][MAXCHARNAME];			/* encoding */

char subrnames[MAXSUBRS][MAXCHARNAME];

/* scan outlines (OUT file) and extract start and end points */

/* remember widths and start (x, y) and end (x, y) */

void scanoutlines(FILE *input) {
	int k, w, xs, ys, xe, ye, xa, ya, xb, yb, xc, yc;

/*	initialize arrays first */
	for (k = 0; k < MAXCHR; k++) {
		widths[k] = -SINFINITY;
		startx[k] = 0; starty[k] = 0; endx[k] = 0; endy[k] = 0;
	}
	while (fgets(buffer, MAXBUF, input) != NULL) {
		if (*buffer == ']') break;			/* scan up to first character */
	}
	for(;;) {
		if (fgets(buffer, MAXBUF, input) == NULL) break;
		if (sscanf(buffer, "%d %d", &k, &w) == 2) {
			widths[k] = w;
			(void) fgets(buffer, MAXBUF, input);				
			if (*buffer == ']') continue;	/* no outline */
			if (sscanf(buffer, "%d %d m", &xs, &ys) == 2) {				
				startx[k] = xs;
				starty[k] = ys;
				for(;;) {
					if (fgets(buffer, MAXBUF, input) == NULL) break;
					if (*buffer == ']') break;
					if (*buffer == 'h') continue;
					if (strncmp(buffer, "cp", 2) == 0) continue;
					if (strchr(buffer, 'l') != NULL) {
						if (sscanf(buffer, "%d %d l", &xe, &ye) == 2) {
							endx[k] = xe; endy[k] = ye;
						}
						else fprintf(stderr, "Don't understand: %s", buffer);
					}
					else if (strchr(buffer, 'c') != NULL) {
						if (sscanf(buffer, "%d %d %d %d %d %d c", 
							&xa, &ya, &xb, &yb, &xc, &yc) == 6) {
							endx[k] = xc; endy[k] = yc;
						}
						else fprintf(stderr, "Don't understand: %s", buffer);
					}
				}
			}
			else fprintf(stderr, "Don't understand: %s", buffer);
		}
		else fprintf(stderr, "Don't understand: %s", buffer);
	}
}

void showstartend(FILE *output) {
	int k;
	for (k = 0; k < MAXCHR; k++) {
		if (widths[k] != -SINFINITY) {
			fprintf(output, "C %d ; WX %d ; S %d %d ; E %d %d ;\n",
				k, widths[k], startx[k], starty[k], endx[k], endy[k]);
		}
	}
}

int lookup(char *name) {
	int k;
	for (k = 0; k < MAXCHR; k++) {
		if (strcmp(charnames[k], name) == 0) return k;
	}
	return -1;
}

void readencoding (FILE *input) {
	int k;
	char charname[CHARNAME_MAX];

/*	read encoding (assumes NOT standard encoding) */
	while(fgets(buffer, MAXBUF, input) != NULL) {	
		if (strstr(buffer, "def") != NULL &&
			strstr(buffer, ".notdef") == NULL) break;
		if (strstr(buffer, ".notdef") != NULL) continue;
		if(sscanf(buffer, "dup %d /%s def", &k, charname) == 2) {
			strcpy(charnames[k], charname);
		}
		else fprintf(stderr, "Don't understand: %s", buffer);
	}
}

/* scan plain file */ /* reads encoding */ /* done in two passes */

void scanplain(FILE *input, int pass) {
	char charname[CHARNAME_MAX];
/*	int k;  */
	int n, sbx, width;
	int charbase, characcent;
	int subrone, subrtwo;
	int delx, dely;
	int offx, offy;

/*	scan up to Encoding */
	while(fgets(buffer, MAXBUF, input) != NULL) {
		if (strstr(buffer, "/Encoding") != NULL) break;
	}

	readencoding (input);

/*	scan up to CharStrings */
	while(fgets(buffer, MAXBUF, input) != NULL) {
		if (strstr(buffer, "/CharStrings") != NULL) break;
	}
	while(fgets(buffer, MAXBUF, input) != NULL) {	
		if (strstr(buffer, "/FontName") != NULL) break;
/*		scan up to next character program */
		if (*buffer == '/') {
			if(sscanf(buffer, "/%s %d RD", charname, &n) < 2)
				fprintf(stderr, "Don't understand: %s", buffer);
			else {
				(void) fgets(buffer, MAXBUF, input);
				if(sscanf(buffer, "%d %d hsbw", &sbx, &width) < 2)
					fprintf(stderr, "Don't understand: %s", buffer);
				else {
					(void) fgets(buffer, MAXBUF, input); /* move to */
					(void) fgets(buffer, MAXBUF, input); /* 1st callsubr */
					if (strstr(buffer, "callsubr") != NULL) {
						sscanf(buffer, "%d callsubr", &subrone);
						(void) fgets(buffer, MAXBUF, input); /* move to */
						if (strstr(buffer, "endchar") != NULL) {
							if (strcmp(subrnames[subrone], "") == 0) {
								strcpy(subrnames[subrone], charname);
							}
						}
/* modified 92/03/03 */ /* do following only if above does not kick in ? */
						if (strstr(buffer, "rmoveto") != NULL) {
							if (sscanf(buffer, "%d %d rmoveto", &delx, &dely) 
								<	2) 
								fprintf(stderr, "Don't understand rmoveto %s", buffer);
						} 
						else if (strstr(buffer, "vmoveto") != NULL) {
							delx = 0;
							if (sscanf(buffer, "%d vmoveto", &dely) 
								<	1) 
								fprintf(stderr, "Don't understand vmoveto %s", buffer);									
							if (traceflag && pass != 0) 
								printf("vmoveto in %s: %s", charname, buffer);
						}
						else if (strstr(buffer, "hmoveto") != NULL) {
							dely = 0;
							if (sscanf(buffer, "%d dmoveto", &delx) 
								<	1) 
								fprintf(stderr, "Don't understand hmoveto %s", buffer);	
							if (traceflag && pass != 0) 
								printf("hmoveto in %s: %s", charname, buffer);
						}
						else if (strstr(buffer, "endchar") != 0) {
/* just a plain subr call and that's it ... */
						}
						else if (pass != 0)
							fprintf(stderr, "Don't understand %s %s", 
								charname, buffer);

						(void) fgets(buffer, MAXBUF, input);/* 2nd callsubr */
						if (strstr(buffer, "callsubr") != NULL) {
							if (pass == 0) subrcount++;
							sscanf(buffer, "%d callsubr", &subrtwo);
							if (verboseflag && pass > 0) {
/*								printf(
				"`%s' calls `%s' (%d) and `%s' (%d) =>  dx: %d dy: %d\n",
									charname, subrnames[subrone], subrone,
										subrnames[subrtwo], subrtwo, 
											delx, dely); */
								charbase = lookup(subrnames[subrone]);
								characcent = lookup(subrnames[subrtwo]);
								if (charbase >= 0 && characcent >= 0) {
								offx = startx[characcent] - endx[charbase];
								offy = starty[characcent] - endy[charbase];
	printf("CC %s 2 ; PCC %s 0 0 ; PCC %s %d %d ; \n",
		charname, subrnames[subrone], subrnames[subrtwo], 
			delx - offx, dely - offy); 
								}
								else fprintf(stderr, 
									"Chars (%s or %s) not found\n",
									subrnames[subrone], subrnames[subrtwo]);
							} // verboseflag && pass > 0
						}	// strstr(buffer, "callsubr") != NULL				
					}	// strstr(buffer, "callsubr") != NULL
				}
			}
		}
	}
}

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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	FILE *input;
	char infilename[FILENAME_MAX];
	int k;
	int firstarg = 1;

	if (firstarg == argc) return -1;
	
	if (strcmp(argv[firstarg], "-t") == 0) {
		traceflag=1;
		firstarg++;
	}

	if (firstarg > argc - 1) {
		printf("EXTCOMPO <OUT file> <PLN file>\n");
		exit(1);
	}

	strcpy(infilename, argv[firstarg]);
	extension(infilename, "out");
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename); exit(3);
	}
	scanoutlines(input);
	fclose(input);
	if (traceflag) showstartend(stdout);
	
	strcpy(infilename, argv[firstarg]);
	extension(infilename, "pln");
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename); exit(3);
	}
/*	clean out char names of subrs */
	for (k = 0; k < MAXSUBRS; k++) strcpy(subrnames[k], "");
	subrcount = 0;

	scanplain(input, 0);
	rewind(input);
	if (verboseflag) printf("StartComposites %d\n", subrcount);
	scanplain(input, 1);

	fclose(input);
	if (verboseflag) printf("EndComposites\n");
/*	if (traceflag != 0) showstartend(stdout); */

	return 0;

}

/* following is format of PLN file CharStrings that above depends upon:

/A 16 RD 
7 679 hsbw
163 262 rmoveto
0 callsubr
endchar
ND

/dieresis 17 RD 
171 625 hsbw
193 711 rmoveto
16 callsubr
endchar
ND

/Adieresis 22 RD 
7 679 hsbw
163 262 rmoveto
0 callsubr
-24 543 rmoveto
16 callsubr
endchar
ND

*/
