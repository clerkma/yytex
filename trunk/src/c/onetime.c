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

/* one time correction of composites in AFM file */
/* program to adjust AFM file based on AUX file - */
/* - produced by adjusting sidebearings */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <conio.h>

/* #define FNAMELEN 80 */
#define MAXLINE 256
#define MAXCHRS 256
#define MAXNAMELEN 32

int verboseflag = 1;
/* int flushkernflag = 1; */
int flushkernflag = 0;

char line[MAXLINE];

char filename[FILENAME_MAX], fontname[FILENAME_MAX];
	
char names[MAXCHRS][MAXNAMELEN];
int widths[MAXCHRS], shifts[MAXCHRS];
int xlls[MAXCHRS], ylls[MAXCHRS], xurs[MAXCHRS], yurs[MAXCHRS];

void readaux(FILE *input) { /* fake */
	int k, code, width, xll, yll, xur, yur, shift;
	char name[MAXNAMELEN];
	char *s;
	
	for(k = 0; k < MAXCHRS; k++) {
		widths[k] = 0; shifts[k] = 0;
		xlls[k] = 0; ylls[k] = 0; xurs[k] = 0; yurs[k] = 0;
		strcpy(names[k], "");
	}
	fgets(line, MAXLINE, input);	/* read title line */
	if((s = strstr(line, "file")) == NULL) {
	}
	if (sscanf(s, "file %s", &filename) < 1) {
	}
	if((s = strstr(line, "FontName")) == NULL) {
	}
	if (sscanf(s, "FontName: %s", &fontname) < 1) {
	}
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) break;
		if(sscanf(line, "C %d ; WX %d ; N %s ; B %d %d %d %d ; S %d",
			&code, &width, &name, &xll, &yll, &xur, &yur, &shift) < 8) {
		}
		widths[code] = width;
		strcpy(names[code], name);
		xlls[code] = xll; ylls[code] = yll;
		xurs[code] = xur; yurs[code] = yur;		
/*		shifts[code] = shift; */
		shifts[code] = 0;
	}
}

/* lookup char code given char name */
int lookup(char *name) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(names[k], name) == 0) return k;
	}
	return -1;
}

void adjustafm(FILE *input, FILE *output) {
	int i, k, n, code, width;
	int xll, yll, xur, yur;
	int nxll, nyll, nxur, nyur;
	char name[MAXNAMELEN], base[2];

/* scan for StartCharMetrics */
	for(;;) {
		fgets(line, MAXLINE, input);
		fputs(line, output);
		if (strstr(line, "StartCharMetrics") != NULL) break;
	}
/* process up to EndCharMetrics */
	for (;;) {
		fgets(line, MAXLINE, input);
		if (strstr(line, "EndCharMetrics") != NULL) break;
		if (sscanf(line, "C %d ; WX %d ; N %s ; B %d %d %d %d ;%n",
			&code, &width, &name, &xll, &yll, &xur, &yur, &n) < 7) {
		}
/*	assuming that xll gives sbx accurately */
		if (code >= 0) {	/* override stuff from AUX file */
			xlls[code] = xll; ylls[code] = yll;
			xurs[code] = xur; yurs[code] = yur;			
		}
		fputs(line, output);
	}
	fputs(line, output); /* output EndCharMetrics line */
}

int scantocomp(FILE *input, FILE *output) {
	for(;;) {
		if(fgets(line, MAXLINE, input) == NULL) {
		}
		fputs(line, output);
		if (strstr(line, "StartComposites") != NULL) return 1;
		if (strstr(line, "EndFontMetrics") != NULL) return 0;
	}
}

/* look for kerning data and flush it if found */
int flushkern(FILE *input, FILE *output) {
	for(;;) {
		if(fgets(line, MAXLINE, input) == NULL) {
		}
		if (strstr(line, "StartKernData") != NULL) break;
		fputs(line, output);
		if (strstr(line, "StartComposites") != NULL) return 1;
		if (strstr(line, "EndFontMetrics") != NULL) return 0;
	}
	for(;;) {
		if(fgets(line, MAXLINE, input) == NULL) {
		}
		if (strstr(line, "EndKernData") != NULL) break;		
	}
	for(;;) {
		if(fgets(line, MAXLINE, input) == NULL) {
		}
		fputs(line, output);
		if (strstr(line, "StartComposites") != NULL) return 1;
		if (strstr(line, "EndFontMetrics") != NULL) return 0;
	}
/*	return -1; */
}

void adjustcompose(FILE *input, FILE *output) {
	char name[MAXNAMELEN], base[MAXNAMELEN], accent[MAXNAMELEN];
	int ka, kb, n, xb, yb, xa, ya, xad, yad;
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) return;	/* EOF */
		if (strstr(line, "EndComposites") != NULL) break;
		if (sscanf(line, "CC %s %d ; PCC %s %d %d ; PCC %s %d %d ;",
			&name, &n, &base, &xb, &yb, &accent, &xa, &ya) < 8) {
			fprintf(stderr, "Don't understand composite: %s", line);
		}
		if (n != 2 || xb != 0 || yb != 0) {
			fprintf(stderr, "Don't understand composite: %s", line);
		}
		if((kb = lookup(base)) < 0) {
			fprintf(stderr, "Can't find base %s\n", base);
		}
		if((ka = lookup(accent)) < 0) {
			fprintf(stderr, "Can't find accent %s\n", accent);
		}
/*		xad = xa + shifts[kb] - shifts[ka]; */
/* the one time correction is (bsb - asb) */
/*	assuming that xlls[] gives sbx[] accurately */
		xad = xa + xlls[kb] - xlls[ka];
		yad = ya;
		fprintf(output, "CC %s %d ; PCC %s %d %d ; PCC %s %d %d ;\n",
			name, n, base, xb, yb, accent, xad, yad);
	}
	fputs(line, output);	/* EndComposites line */
}

void copyrest(FILE *input, FILE *output) {
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) return;
		fputs(line, output);
	}
}

#ifdef IGNORE
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

int main(int argc, char *argv[]) {
	FILE *input, *output, *auxil;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_aux[FILENAME_MAX];
	char *s;
	int m, k;
	
	if (argc < 2) {
		fprintf(stderr, "Need argument specifying source file");
		exit(1);
	}

	for(m = 1; m < argc; m++) {

		strcpy(fn_aux, argv[m]);
		forceexten(fn_aux, "aux");
		if((auxil = fopen(fn_aux, "r")) == NULL) {
			perror(fn_aux);	exit(1);
		}

		readaux(auxil);
		fclose(auxil); 

		strcpy(fn_in, argv[m]);	/* input file given as argument */
		extension(fn_in, "afm");
		if((input = fopen(fn_in, "r")) == NULL) {
			perror(fn_in);	exit(1);
		}
	
		if (verboseflag != 0) printf("Processing %s\n", fn_in);

/* strip off pathname */
		if ((s = strrchr(fn_in, '\\')) != NULL) *s++;
		else if ((s = strrchr(fn_in, '/')) != NULL) *s++;
		else if ((s = strrchr(fn_in, ':')) != NULL) *s++;	
		else s = fn_in;
	
/* output goes in current directory */
		strcpy(fn_out, s);
		forceexten(fn_out, "afm");
		if((output = fopen(fn_out, "w")) == NULL) {
			perror(fn_out);	exit(1);
		}	

/*		printf("Ready to adjust AFM\n"); getch(); */
		adjustafm(input, output);
/*		printf("Ready to flush kern\n"); getch(); */
		if (flushkernflag != 0)	k = flushkern(input, output);
		else k = scantocomp(input, output);
		if (k > 0) {
/*			printf("Ready to adjust composites\n"); getch(); */
			adjustcompose(input, output);
		}
/*		printf("Ready to copy the rest\n"); getch(); */
		copyrest(input, output);

		fclose(input);
		if (ferror(output) != 0) {
			perror(fn_out); exit(3);
		}
		else fclose(output);

	}
	if (argc> 2) printf("Processed %d afm files", argc - 1);
	return 0;
}

/* several errors detected, but not acted upon - assuming good data */
