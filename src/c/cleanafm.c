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

/* program to clean up afm file (remove control encoding and comments) 
 *
 * gets rid of comments following '%' 
 * changes character code numbers in range 0 - 31 and 128 - 159 to -1 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <conio.h>

/* #define FNAMELEN 80 */
#define MAXLINE 512
#define MAXNAME 48

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;

int reencode = 1;			/* change control character encoding */
int stripcomment = 1;		/* strip comments from file */

int rescale = 0;			/* rescale the AFM file */
int hintfile = 0;			/* process hint file instead of AFM */

double xscale=1.0, yscale=1.0;

static char line[MAXLINE];
static char buffer[MAXLINE];

double round(double x) {
	if (x >= 0) return (double) ((int) (x + 0.5));
	else return - (double) ((int) (-x + 0.5));
}

#ifdef IGNORE
void extension(char *fname, char *ext)  /* supply extension if none present */
{
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) /* change extension if one present */
{
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
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

int getline(FILE *input, char *line) {/* read a line up to newline */
	if (fgets(line, MAXLINE, input) == NULL) return EOF;
	return 0;
}

int getrealline(FILE *input, char *line) { /* ignore blanks & comments */
	int k;
	k = getline(input, line);		
	while (*line == '%' || *line == '\n') k = getline(input, line);
	return k;
}

void cleanfile(FILE *output, FILE *input) {
	int chr, n;
	double width, xll, yll, xur, yur, dx, dy, kerning;
	char name[MAXNAME], base[MAXNAME], accent[MAXNAME];
	char kerna[MAXNAME], kernb[MAXNAME];
	char *s;

	while(getline(input, line) != EOF) {
/*		if (*line == '%' || *line == '\n') continue; */
		if (stripcomment != 0 && (*line == '%' || *line == '\n')) continue;
		if (stripcomment != 0 && (s = strchr(line, '%')) != NULL) {
			*s++ = '\n'; *s = '\0';
		}
		if (strncmp(line, "C ", 2) == 0) {
			if(sscanf(line, "C %d%n ;", &chr, &n) > 0) {
				if (reencode != 0 && 
					((chr >= 0 && chr < 32) || (chr >= 128 && chr < 160))) {
						strcpy(buffer, line + n);
						strcpy(line, "C -1");
						strcat(line, buffer);
/*						fprintf(output, "C %d", -1); */
/*						strcpy(line, line + n); */
					}
			}
			else fputs(line, stderr);
			if (sscanf(line, "C %d ; WX %lg ; N %s ; B %lg %lg %lg %lg%n",
				&chr, &width, name, &xll, &yll, &xur, &yur, &n) == 7) {
				if (rescale != 0 && (xscale != 1.0 || yscale != 1.0)) {
					width = round (width * xscale);
					xll = round (xll * xscale);
					xur = round (xur * xscale);
					yll = round (yll * yscale);
					yur = round (yur * yscale);
					strcpy(buffer, line + n);
					sprintf(line, "C %d ; WX %lg ; N %s ; B %lg %lg %lg %lg",
						chr, width, name, xll, yll, xur, yur);
					strcat(line, buffer);
				}
			}
			else fputs(line, stderr);
		}
		else if (strncmp(line, "CC ", 3) == 0) {
			if (sscanf(line, "CC %s 2 ; PCC %s 0 0 ; PCC %s %lg %lg",
				name, base, accent, &dx, &dy) == 5) {
				if (rescale != 0 && (xscale != 1.0 || yscale != 1.0)) {
					dx = round (dx * xscale);
					dy = round (dy * yscale);
					sprintf(line, "CC %s 2 ; PCC %s 0 0 ; PCC %s %lg %lg ;\n",
						name, base, accent, dx, dy);
				}
			}
			else fputs(line, stderr);			
			
		}
		else if (strncmp(line, "KPX ", 4) == 0) {
			if (sscanf(line, "KPX %s %s %lg", kerna, kernb, &kerning) == 3) {
				if (rescale != 0 && (xscale != 1.0 || yscale != 1.0)) {
					kerning = round (kerning * xscale);
					sprintf(line, "KPX %s %s %lg\n", kerna, kernb, kerning);
				}
			}
			else fputs(line, stderr);			
			
		}
/*		if ((s = strchr(line, '%')) != NULL) { */
/*		fprintf(output, "%s", line); */
		fputs(line, output);
	}
}

void adjusthints (FILE *output, FILE *input) {
	char *s, *t;
	int c, n, subr, chr;
	double hs, he, vs, ve;

	while(getline(input, line) != EOF) {
		if (stripcomment != 0 && (*line == '%' || *line == '\n')) continue;
		if (stripcomment != 0 && (s = strchr(line, '%')) != NULL) {
			*s++ = '\n'; *s = '\0';
		}
		s = line;
		if (strncmp(s, "S ", 2) == 0) {
			if (sscanf(line, "S %d ; %n", &subr, &n) > 0) s += n;
			else fputs(line, stderr);
			if (traceflag != 0) printf("%d advance in S to %s", n, s);
		}
		if (strncmp(s, "C ", 2) == 0) {
			if(sscanf(s, "C %d ; %n", &chr, &n) > 0) {
				s += n;
				if (traceflag != 0) printf("%d advance in C to %s", n, s);
				t = line;
				while (t < s) putc(*t++, output);
				for(;;) {
					c = *s;
					putc(c, output);
					if (c == 0 || c == '\n') break;
					if (c == 'H' || c == 'E') {
						while(sscanf(s+1, "%lg %lg%n", &hs, &he, &n) == 2) {
							hs = round(hs * yscale);
							he = round(he * yscale);			
							fprintf(output, " %lg %lg", hs, he);
							s += n;
						}
					}
					else if (c == 'V' || c == 'M') {
						while(sscanf(s+1, "%lg %lg%n", &vs, &ve, &n) == 2) {
							vs = round(vs * xscale);
							ve = round(ve * xscale);			
							fprintf(output, " %lg %lg", vs, ve);
							s += n;
						}
					}
					s++;
				}
			}
			else fputs(line, stderr);
		}
		else fputs(line, output);
	}	
}

void showusage(char *s) {
	printf("%s [-{v}{u}{s}{h}{x}]\n", s);
	printf("\tv verbose mode\n");
	printf("\tu do not unencode control characters\n");
	printf("\ts do not strip comments\n");
	printf("\th process hint file\n");
	printf("\tx scale in x\n");
	printf("\ty scale in y\n");	
	exit(1);
}

int main(int argc, char *argv[]) {
	FILE *fp_in, *fp_out;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_bak[FILENAME_MAX];
	char *s;
	int m, firstarg = 1;

	if (firstarg == argc) return -1;
	
	s = argv[firstarg];
	while (*s == '-') {
		if (strchr(s, 'v') != 0) verboseflag = (1 - verboseflag);
		if (strchr(s, 't') != 0) traceflag = (1 - traceflag);
		if (strchr(s, 'u') != 0) reencode = (1 - reencode);
		if (strchr(s, 's') != 0) stripcomment = (1 - stripcomment);
		if (strchr(s, 'h') != 0) hintfile = (1 - hintfile);
		if (strchr(s, 'x') != 0) {
			rescale++;
			if (sscanf(s, "-x=%lg", &xscale) < 1) 	fputs(s, stderr);
		}
		if (strchr(s, 'y') != 0) {
			rescale++;
			if (sscanf(s, "-y=%lg", &yscale) < 1) 	fputs(s, stderr);		
		}
		firstarg++;
		s = argv[firstarg];		
	}

	if (firstarg >= argc) showusage(argv[0]);

	for(m = firstarg; m < argc; m++) {
 
		strcpy(fn_in, argv[m]);
		if (hintfile == 0) {
			extension(fn_in, "afm");
			printf("Processing AFM file %s\n", fn_in);
		}
		else {
			extension(fn_in, "hnt");
			printf("Processing HNT file %s\n", fn_in);
		}

		if ((fp_in = fopen(fn_in, "r")) == NULL) {
			fprintf(stderr, "Can't open input");
			perror(fn_in); continue;
		} 
		else fclose(fp_in);
		
		if ((s=strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in;
		strcpy(fn_out, s);  	/* copy input file name minus path */

		if (hintfile == 0) 	forceexten(fn_out, "afm");	/* change extension */ 
		else forceexten(fn_out, "hnt");					/* change extension */ 

		if (strcmp(fn_out, fn_in) == 0) { /* file name conflict */
			strcpy(fn_bak, fn_in);
			forceexten(fn_bak, "bak");
			if (rename(fn_in, fn_bak) != 0) {
				fprintf(stderr, "Rename of input file failed\n");
				continue;
			}
			strcpy(fn_in, fn_bak);
		}

		if ((fp_in = fopen(fn_in, "r")) == NULL) {
			fprintf(stderr, "Can't open input");
			perror(fn_in); continue;
		}

		if ((fp_out = fopen(fn_out, "w")) == NULL) {
			fprintf(stderr, "Can't open output");
			perror(fn_out); fclose(fp_in); continue;
		}		

		if (hintfile != 0) adjusthints(fp_out, fp_in);
		else cleanfile(fp_out, fp_in);

		if (ferror(fp_in) != 0) {
			fprintf(stderr, "Input error");
			perror(fn_in); 
		}
		else fclose(fp_in);

		strcpy(fn_out, argv[m]);
		forceexten(fn_out, "cln");
		if (ferror(fp_out) != 0) {
			fprintf(stderr, "output error");
			perror(fn_out); 
		}
		else fclose(fp_out);
	}
	return 0;
}
