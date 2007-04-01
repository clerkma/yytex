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

/**********************************************************************
* Program to insert new encoding vector into AFM file 
* Usage: REENCODE <encode-vec> <afm-file-1> <afm-file-2> ... 
**********************************************************************/


#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>

/* #define FNAMELEN 80 */
#define CHARNAME_MAX 64
#define MAXLINE 256
#define CURRENTDIRECT 1

int verboseflag = 1;

int getline(FILE *input, char *line) {/* read a line up to newline */
	if (fgets(line, MAXLINE, input) == NULL) return 0;
	return strlen(line); 
}

void reencode(FILE *fp_enc, FILE *fp_afm, FILE *fp_out) {
	char line[MAXLINE];
	int xll, yll, xur, yur, chra, chrb;
	double width;
	char namea[CHARNAME_MAX], nameb[CHARNAME_MAX];

	if(getline(fp_afm, line) == 0) {
		fprintf(stderr, "Unexpected EOF\n"); exit(3);
	}
	while (strstr(line, "StartCharMetrics") == NULL) {
		fprintf(fp_out, "%s", line);
		if(getline(fp_afm, line) == 0) {
			fprintf(stderr, "Unexpected EOF\n"); exit(3);
		}
	}
	fprintf(fp_out, "%s", line);
	if(getline(fp_afm, line) == 0) {
		fprintf(stderr, "Unexpected EOF\n"); exit(3);
	}

	while (strstr(line, "EndCharMetrics") == NULL) {		
		while(*line == '%' || *line == '\n') {
			fprintf(fp_out, "%s", line);
			if(getline(fp_afm, line) == 0) {
				fprintf(stderr, "Unexpected EOF on AFM\n"); exit(3);
			}
		}
		if (strstr(line, "EndCharMetrics") != NULL) break;
		if(sscanf(line, 
			"C %d ; WX %lg ; N %s ; B %d %d %d %d ; ",
				&chra, &width, &namea, &xll, &yll, &xur, &yur) != 7) {
				fprintf(stderr, "Don't understand AFM: %s", line);
				exit(5);
		}
		getline(fp_enc, line);
		while(*line == '%' || *line == '\n') {
/*			if (verboseflag != 0) printf("%s", line); */
			if(getline(fp_enc, line) == 0) {
				fprintf(stderr, "Unexpected EOF on ENC\n"); exit(3);
			}
		}
		if (sscanf(line, "C %d ; N %s; ", &chrb, &nameb) != 2) {
			if(sscanf(line, "%d %s", &chrb, &nameb) != 2) {
				fprintf(stderr, "Don't understand ENC: %s", line);
				exit(8);
			}
		}
		if (chra != chrb) {
			fprintf(stderr, "Sequence error %d != %d\n", chra, chrb);
			exit(7);
		}
		fprintf(fp_out, "C %d ; WX %g ; N %s ; B %d %d %d %d ;\n",
			chra, width, nameb, xll, yll, xur, yur);
		if(getline(fp_afm, line) == 0) {
			fprintf(stderr, "Unexpected EOF\n"); exit(3);
		}
	}

	while (strstr(line, "EndFontMetrics") == NULL) {
		fprintf(fp_out, "%s", line);
		if(getline(fp_afm, line) == 0) {
			fprintf(stderr, "Unexpected EOF\n"); break;
		}
	}
	fprintf(fp_out, "%s", line);	
}

#ifdef IGNORE
void extension(char *fname, char *ext) { /* supply extension if not present */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
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

int main(int argc, char *argv[]) {       /* main program */
    FILE *fp_afm, *fp_enc, *fp_out;
    char fn_afm[FILENAME_MAX], fn_enc[FILENAME_MAX], fn_out[FILENAME_MAX];
	char *s, *ext = "out";
	int m, firstarg = 1;

    if (argc < 3) {
    	fprintf(stderr, "Not enough arguments\n");
		fprintf(stderr, 
			"Usage: %s <encode-vec> <afm-file-1> <afm-file-2> ...\n",
				argv[0]);
		exit(1);
	}

	if (strstr(argv[firstarg], "-e") != NULL) {
		firstarg++;
		ext = argv[firstarg]; 
		firstarg++; 
	}
		
	strcpy(fn_enc, argv[firstarg]); firstarg++;
	
	for (m = firstarg; m < argc; m++) {

	strcpy(fn_afm, argv[m]);

	if (verboseflag != 0) printf("Processing AFM file: %s\n", fn_afm);

#if CURRENTDIRECT != 0
	if ((s=strrchr(fn_afm, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_afm, ':')) != NULL) s++;
		else s = fn_afm;
		strcpy(fn_out, s);  	/* copy input file name minus path */
#else
	strcpy(fn_out, fn_afm);  /* copy input file name */
#endif
	forceexten(fn_out, ext);	/* change extension */

	if ((fp_afm = fopen(fn_afm, "r")) == NULL) {
		perror(fn_afm); exit(2);
	}

	if ((fp_enc = fopen(fn_enc, "r")) == NULL) {
		perror(fn_enc); exit(2);
	}	

	if ((fp_out = fopen(fn_out, "w")) == NULL) {
		perror(fn_out); exit(2);
	}	

	reencode(fp_enc, fp_afm, fp_out);

	fclose(fp_out);
	fclose(fp_enc);
	fclose(fp_afm);

	}

	if (argc > 3 && verboseflag != 0) 
		printf("Processed %d AFM files\n", argc - 2);
	
	return 0;
}
