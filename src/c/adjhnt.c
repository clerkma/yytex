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

/* program to adjust HNT file based on AUX file - */
/* - produced by adjusting sidebearings */ /* used for AT&T project */

/* program to adjust HNT file based on scale and sidebearing - */
/* - given on command line --- use for Euler project */

/* command line form: adjhnt ptsize fontname */
/* old hint file made using leftextra=0 rightextra=0 and xscale=oldxscale */
/* vstem hint at x shifts to (x + leftextra) * newxscale / oldxscale */
/* source hint files assumed to be in c:\euler */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* #include <errno.h> */
/* #include <conio.h> */

/* #define FNAMELEN 80 */
#define MAXLINE 256
#define MAXCHRS 256
#define MAXNAMELEN 32

int verboseflag = 1;

char line[MAXLINE];

char filename[FILENAME_MAX], fontname[FILENAME_MAX];
	
char names[MAXCHRS][MAXNAMELEN];
int shifts[MAXCHRS];

int widths[MAXCHRS]; /* not accessed */
int xlls[MAXCHRS], ylls[MAXCHRS], xurs[MAXCHRS], yurs[MAXCHRS]; /* NA */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int eulerflag=0;

int programem = 3700;				/* scaling from bitpad units */

int leftside=0;					/* added sidebearing on left */
int rightside=0;				/* added sidebearing on right */
double xscale=1.0;		/* new aspect ratio */
double oldxscale=1.0;		/* old aspect ratio */

int shift=0;			/* current shift value */

int xmap(int x) {
	if (eulerflag == 0) return (x + shift);
	else return (int) (((double) x  + 
		((double) leftside * 1000 / programem)) *
			xscale / oldxscale + 0.5);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double ptsize=10.0; /* not accessed */

/* left and right side added space and xscale factor */
/* as function of point size */

#define MAXSIZES 6		/* number of point sizes possible */

#define MAXNAMES 6		/* number of font names possible */

/* Following are same for Euler Roman, Script, and Fraktur (medium & bold) */

int ptsizes[MAXSIZES] = {5, 6, 7, 8, 9, 10};
int leftsides[MAXSIZES] = {300, 150, 100, 0, 0, 0};
int rightsides[MAXSIZES] = {300, 150, 100, 0, 0, 0};

/* Following is different for each of the six fonts */

char *fontnames[MAXNAMES] = {"eurm", "eusm", "eufm", "eurb", "eusb", "eufb"};

double xscales[MAXNAMES][MAXSIZES] = {
{1.2, 1.16, 1.13, 1.08, 1.03, 1.0},
{1.2, 1.16, 1.13, 1.09, 1.04, 1.0},
{1.2, 1.14, 1.08, 1.04, 1.02, 1.0},
{1.28, 1.23, 1.2, 1.17, 1.12, 1.1},
{1.28, 1.24, 1.21, 1.18, 1.15, 1.13},
{1.3, 1.25, 1.2, 1.19, 1.18, 1.18}
};

/* for EURM (Euler Roman Medium) */
/* double xscales[MAXSIZES] = {1.2, 1.16, 1.13, 1.08, 1.03; 1.0}; */

/* for EUSM (Euler Script Medium) */
/* double xscales[MAXSIZES] = {1.2, 1.16, 1.13, 1.09, 1.04; 1.0}; */

/* for EUFM (Euler Fraktur Medium) */
/* double xscales[MAXSIZES] = {1.2, 1.14, 1.08, 1.04, 1.02; 1.0}; */

/* for EURB (Euler Roman Bold) */
/* double xscales[MAXSIZES] = {1.28, 1.23, 1.2, 1.17, 1.12; 1.1}; */

/* for EUSB (Euler Script Bold) */
/* double xscales[MAXSIZES] = {1.28, 1.24, 1.21, 1.18, 1.15; 1.13}; */

/* for EUFB (Euler Fraktur Bold) */
/* double xscales[MAXSIZES] = {1.3, 1.25, 1.2, 1.19, 1.18; 1.18}; */


int setupfont(int pt, char *name, int flag) {
	int k, l;

	if (flag != 0) 	printf("Font: %s size: %d", name, pt);
	ptsize = (double) pt;
	for(k = 0; k < MAXSIZES; k++) {
		if (ptsizes[k] == pt) break;
	}
	if (k == MAXSIZES) {
		fprintf(stderr, "Invalid point size %d\n", pt);
		return -1;
	}
	leftside = leftsides[k];  rightside = rightsides[k];
	if (flag != 0) printf(" --- leftside %d", leftside);
	if (flag != 0) printf(" --- rightside %d", rightside);
	for (l = 0; l < MAXNAMES; l++) {
		if (strcmp(fontnames[l], name) == 0) break;
	}
	if (l == MAXNAMES) {
		fprintf(stderr, "Invalid font name %s\n", name);
		return -1;
	}
	xscale = xscales[l][k];
	if (flag != 0) printf(" --- xscale %lg\n", xscale);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void readaux(FILE *input) {
	int k, code, width, xll, yll, xur, yur;
/*	int shift; */
	char name[MAXNAMELEN];
	char *s;
	
	for(k = 0; k < MAXCHRS; k++) {
		widths[k] = 0; shifts[k] = 0;
		xlls[k] = 0; ylls[k] = 0; xurs[k] = 0; yurs[k] = 0;
		strcpy(names[k], "");
	}
	(void) fgets(line, MAXLINE, input);	/* read title line */
	if((s = strstr(line, "file")) == NULL) {
	}
/*	if (sscanf(s, "file %s", &filename) < 1) { */
	if (sscanf(s, "file %s", filename) < 1) {
	}
	if((s = strstr(line, "FontName")) == NULL) {
	}
/*	if (sscanf(s, "FontName: %s", &fontname) < 1) { */
	if (sscanf(s, "FontName: %s", fontname) < 1) {
	}
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) break;
		if(sscanf(line, "C %d ; WX %d ; N %s ; B %d %d %d %d ; S %d",
/*			&code, &width, &name, &xll, &yll, &xur, &yur, &shift) < 8) { */
			&code, &width, name, &xll, &yll, &xur, &yur, &shift) < 8) {
		}
		widths[code] = width;
		strcpy(names[code], name);
		xlls[code] = xll; ylls[code] = yll;
		xurs[code] = xur; yurs[code] = yur;		
		shifts[code] = shift;
	}
}

/* lookup char code given char name */
/* int lookup(char *name) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(names[k], name) == 0) return k;
	}
	return -1;
} */

void adjusthnt(FILE *input, FILE *output, int flag) {
	int c, n, code, xl, xr, yb, yt;
/*	int shift; */
	int xl1, xl2, xl3, xr1, xr2, xr3;
	int yb1, yb2, yb3, yt1, yt2, yt3;
	int stdvw, vw1, vw2;
	char *s;

/* scan for character level hints */
	for(;;) {
		c = getc(input); (void) ungetc(c, input);
		if (c == 'C') break;
		(void) fgets(line, MAXLINE, input);
		if (strncmp(line, "/StdVW", 6) == 0) {
			if (sscanf(line, "/StdVW [%d] def", &stdvw) < 1) 
				fprintf(stderr, "WARNING:Don't understand %s", line);
			else {
				sprintf(line, "/StdVW [%d] def\n", 
					(int) ((double) stdvw * xscale / oldxscale));
			}
		}
		else if (strncmp(line, "/StemSnapV", 10) == 0) {
			if (sscanf(line, "/StemSnapV [%d %d] def", &vw1, &vw2) < 2) 
				fprintf(stderr, "WARNING:Don't understand %s", line);
			else {
				sprintf(line, "/StemSnapV [%d %d] def\n", 
					(int) ((double) vw1 * xscale / oldxscale),
						(int) ((double) vw2 * xscale / oldxscale));
			}
		}
		fputs(line, output);
	}
/* process lines up to End of File */
	for (;;) {
		if(fgets(line, MAXLINE, input) == NULL) break;
		if (*line == '%') {
			fputs(line, output);
			continue;
		}
		if (sscanf(line, "C %d ; %n", &code, &n) < 1) {
			fputs(line, output);
			continue;
		}
		if (flag == 0) shift = shifts[code];
		else shift = leftside;

		fprintf(output, "C %d ; ", code);
		s = line + n;
		for (;;) {
			while (*s == ' ') s++;
			if (*s == '\n') break;
			if (*s == 'H') {
				fprintf(output, "H ");
				s = s + 2;
				for(;;) {
					if (*s == ';') {
						fprintf(output, "; ");
						s = s++;
						break;
					}
					if (sscanf (s, "%d %d %n", &yb, &yt, &n) < 2) {	
					}
					fprintf(output, "%d %d ", yb, yt);
					s = s + n;
				}
			}
			else if (*s == 'V') {
				fprintf(output, "V ");
				s = s++;
				for(;;) {
					if (*s == ';') {
						fprintf(output, "; ");
						s = s + 1;
						break;
					}
					if (sscanf (s, "%d %d %n", &xl, &xr, &n) < 2) {	
					}
					fprintf(output, "%d %d ", xmap(xl), xmap(xr));
					s = s + n;
				}
			}
			else if (*s == 'E') {
				s = s + 2;
				if (sscanf(s, "%d %d %d %d %d %d %n", 
					&yb1, &yt1, &yb2, &yt2, &yb3, &yt3, &n) < 6) {
				}
				fprintf(output, "E %d %d %d %d %d %d ; ",
					yb1, yt1, yb2, yt2, yb3, yt3); 
				s = s + n + 1;
			}
			else if (*s == 'M') {
				s = s + 2;
				if (sscanf(s, "%d %d %d %d %d %d %n", 
					&xl1, &xr1, &xl2, &xr2, &xl3, &xr3, &n) < 6) {
				}
				fprintf(output, "M %d %d %d %d %d %d ; ",
					xmap(xl1), xmap(xr1), xmap(xl2), 
						xmap(xr2), xmap(xl3), xmap(xr3)); 
				s = s + n + 1;
			}
			else {
				fprintf(stderr, "Unrecognized hint: %s", s);
			}
			if (*s == '\0') break;
			s++;
			if (*s == '\0') break;			
		}
		putc('\n', output);
	}
}

void extension(char *fname, char *ext) {  /* supply extension if none */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);
}

int main(int argc, char *argv[]) {
	FILE *input, *output, *auxil;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_aux[FILENAME_MAX];
	char *s;
	int m=0;
	char fname[FILENAME_MAX];
	
	if (argc < 2) {
		fprintf(stderr, "Need argument specifying source file");
		exit(1);
	}

	if (sscanf(argv[1], "%d", &m) == 1) {
/*		if ((sscanf(argv[1], "%d", &extraleft) < 1) ||
			(sscanf(argv[2], "%d", &extraright) < 1) ||
			(sscanf(argv[3], "%d", &newxscale) < 1) ||
			(sscanf(argv[4], "%d", &oldxscale) < 1)) {
			fprintf(stderr, "Need numeric arguments");
			exit(2);
		} */
		eulerflag = 1;
		strcpy(fname, argv[2]);
		if (setupfont(10, fname, 0) != 0) exit(3);
		oldxscale = xscale;
		if (setupfont(m, fname, 1) != 0) exit(3);

		strcpy(fn_in, "c:\\euler\\");		/* input file given as argument */
		strcat(fn_in, fname);
		strcat(fn_in, "10");
		extension(fn_in, "hnt");
		if((input = fopen(fn_in, "r")) == NULL) {
			perror(fn_in);	exit(1);
		}
	
		if (verboseflag != 0) printf("Processing %s\n", fn_in);

/* strip off pathname */
		if ((s = strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s = strrchr(fn_in, '/')) != NULL) s++;
		else if ((s = strrchr(fn_in, ':')) != NULL) s++;	
		else s = fn_in;
	
/* output goes in current directory */
		strcpy(fn_out, fname);
		sprintf(fn_out + strlen(fn_out), "%d", m);
		forceexten(fn_out, "hnt");
		if((output = fopen(fn_out, "w")) == NULL) {
			perror(fn_out);	exit(1);
		}	

/*		printf("Ready to adjust HNT\n"); getch(); */
		adjusthnt(input, output, 1);

		fclose(input);
		if (ferror(output) != 0) {
			perror(fn_out); exit(3);
		}
		else fclose(output);
		return 0;
	}
	else {	/* no, old style --- used for AT&T hint files */

	for(m = 1; m < argc; m++) {

		strcpy(fn_aux, argv[m]);
		forceexten(fn_aux, "aux");
		if((auxil = fopen(fn_aux, "r")) == NULL) {
			perror(fn_aux);	exit(1);
		}

		readaux(auxil);
		fclose(auxil);

		strcpy(fn_in, argv[m]);	/* input file given as argument */
		extension(fn_in, "hnt");
		if((input = fopen(fn_in, "r")) == NULL) {
			perror(fn_in);	exit(1);
		}
	
		if (verboseflag != 0) printf("Processing %s\n", fn_in);

/* strip off pathname */
		if ((s = strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s = strrchr(fn_in, '/')) != NULL) s++;
		else if ((s = strrchr(fn_in, ':')) != NULL) s++;	
		else s = fn_in;
	
/* output goes in current directory */
		strcpy(fn_out, s);
		forceexten(fn_out, "hnt");
		if((output = fopen(fn_out, "w")) == NULL) {
			perror(fn_out);	exit(1);
		}	

/*		printf("Ready to adjust HNT\n"); getch(); */
		adjusthnt(input, output, 0);

		fclose(input);
		if (ferror(output) != 0) {
			perror(fn_out); exit(3);
		}
		else fclose(output);

	}
	if (argc> 2) printf("Processed %d hnt files", argc - 1);
	return 0;
	}
}

/* several errors detected, but not acted upon - assuming good data */

/* need to adjust /StdVW and /StemSnapV also */
