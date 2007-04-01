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

/* Program for changing case of FontName in PFB file */

/* #define FNAMELEN 80 */
#define BUFFERLEN 1024

/* #define MAXCHRS 256 */
/* #define MAXCHARNAME 32 */
/* #define MAXENCODING 256 * 64 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *copyright = "\
Copyright (C) 1991  Y&Y.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1991  Y&Y. All rights reserved. (508) 371-3286\ */

char buffer[BUFFERLEN];

char programpath[FILENAME_MAX] = "";

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;

int lowerflag = 0;				/* switch to lower case */
int upperflag = 0;				/* switch to upper case */

int usingreturn;				/* file uses return instead of linefeed */
int usingboth;					/* file uses return/linefeed combination */

long asciilength;

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
			usingboth = 1;
		}
		else {
			ungetc(c, input);
			usingreturn = 1;
		}
	}
	*s = '\0';
	return k;
}

void changetolinefeed(char *s) {	/* \r => \n */
	int c;
	while ((c = *s) != 0) {
		if (c == '\r') *s = '\n';
		s++;
	}
}

void changetoreturn(char *s) {	/* \n => \r */
	int c;
	while ((c = *s) != 0) {
		if (c == '\n') *s = '\r';
		s++;
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	printf("Usage:\n");
	printf("%s [-{v}{l}{u}] <pfb-file-1> <pfb-file-2> ...\n", s);
	printf("\n"); 
	printf("\tv  verbose mode\n");
	printf("\tl  convert FontName to lower case\n");
	printf("\tu  convert FontName to upper case\n");
	printf("\t   (default is to flip FontName case)\n");
	printf("\n"); 
	printf("\t   (output files appear in current directory)\n");
	printf("\n"); 
	exit(0);
}

int decodeflag (int c) { 			/* decode command  line flag */
	switch(c) { 
		case 'v': if(verboseflag != 0) traceflag = 1; else verboseflag = 1; return 0; 
		case 'l': lowerflag = 1; return 0; 
		case 'u': upperflag = 1; return 0; 
		case '?': detailflag = 1; return 0; 
		default: {
				fprintf(stderr, "Invalid command line flag '%c'\n", c);
				exit(13);
		}
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;			/* not accessed ? */

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break; 
			else if (decodeflag(c) != 0) { /* flag requires argument ? */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
/*				if (vectorflag != 0) {
					vector = s;		vectorflag = 0;
				} */
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

/* remove file name - keep only path - inserts '\0' to terminate */
void removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
}

void uppercase(char *s, char *t) { /* convert to upper case letters */
	int c;
	while ((c = *t) == ' ') {   /* skip leading spaces */
		*s++ = (char) c; t++;
	}
	while ((c = *t++) > ' ') {	/* up to white space or null */
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

void lowercase(char *s, char *t) { /* convert to lower case letters */
	int c;
	while ((c = *t) == ' ') {   /* skip leading spaces */
		*s++ = (char) c; t++;
	}
	while ((c = *t++) > ' ') {
		if (c >= 'A' && c <= 'Z') *s++ = (char) (c + 'a' - 'A');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

void changecase(char *s, char *t) { /* flip case */
	int c;
	while ((c = *t) == ' ') {   /* skip leading spaces */
		*s++ = (char) c; t++;
	}
	while ((c = *t++) > ' ') {
		if (c >= 'A' && c <= 'Z') *s++ = (char) (c + 'a' - 'A');
		else if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

void underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = stripname(filename);
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
/*	if (traceflag != 0) printf("n = %d m = %d ", n, m); */
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
/*	if (traceflag != 0) printf("Now trying file name: %s\n", s); */
/*	return 0; */
}

int copypfmfile(FILE *output, FILE * input) {			
	int c;
	long k, pfmextent, driverinfo;
	
	pfmextent = 117;
	for (k = 2; k < pfmextent + 22; k++) {
		c= fgetc(input); fputc(c, output);
	}
	driverinfo = readfour(input);
	writefour(driverinfo, output);
	if (driverinfo > 0 && driverinfo < 2000) {
		for (k = pfmextent + 22 + 4; k < driverinfo; k++) {
			c = fgetc(input); fputc(c, output);
		}
		while ((c = fgetc(input)) > 0) {
			if (c > 128) {
				fprintf(stderr, "Not a valid PFM file\n");
/*				fclose(input);
				if (renameflag != 0) 
					(void) rename(infilename, oldfilename);
				break; */
				return -1;
			}
			if (lowerflag != 0) {
				if (c >= 'A' && c <= 'Z') c = c + 'a' - 'A';
			}
			else if (upperflag != 0) {
				if (c >= 'a' && c <= 'z') c = c + 'A' - 'a';
			}
			else {
				if (c >= 'a' && c <= 'z') c = c + 'A' - 'a';
				else if (c >= 'A' && c <= 'Z') c = c + 'a' - 'A';
			}
			if (traceflag != 0) putc(c, stdout); /* debugging */
			fputc(c, output);
		}
		fputc(c, output);
		while ((c = fgetc(input)) != EOF) 	fputc(c, output);
		return 0;
	}
	else {
		fprintf(stderr, "Pointer out of range in PFM file\n");
		return -1;
	}
}

int copypfbfile(FILE *output, FILE *input) {
	int c, flag = -1;
	char *s;

/*	change font name in first line - if found */
	if (*buffer == '%' && *(buffer+1) == '!') {
		if ((s = strchr(buffer, ':')) != NULL) {
/*			printf("Old version: %s\n", s+1); */
			if (lowerflag != 0) lowercase(s+1, s+1);
			else if (upperflag != 0) uppercase(s+1, s+1);
			else changecase(s+1, s+1);
/*				printf("New version: %s\n", s+1); */
		}
		else fprintf(stderr, "No colon in first line: %s", buffer);
	}
	else fprintf(stderr, "First line non-standard: %s", buffer);
	fputs(buffer, output);			

	for(;;) {
/*		linestart = ftell(input); */
		if(readline(buffer, BUFFERLEN, input) == 0) break;	 /*	EOF */
/*			if (traceflag != 0) {
				if (usingreturn != 0) 
				changetolinefeed(buffer); 
				printf("%s", buffer);
			} */
		if ((s = strstr(buffer, "/FontName")) != NULL) {
			if (lowerflag != 0) lowercase(s + 10, s + 10);
			else if (upperflag != 0) uppercase(s + 10, s + 10);
			else changecase(s + 10, s + 10);
			if (traceflag != 0) fputs(buffer, stdout);
			fputs(buffer, output);
			flag = 0;
			break;
		} 
		else fputs(buffer, output);
	}

	while ((c = getc(input)) != EOF) putc(c, output); /* copy the rest */
	return flag;
}


int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX], oldfilename[FILENAME_MAX];
/*	char *s; */
/*	long k, pfmextent, driverinfo; */
	int firstarg = 1, filecount = 0;
	int m, c, d, renameflag;
	
	if (argc < 2) showusage(argv[0]);
	
	strncpy(programpath, argv[0], sizeof(programpath));
	removepath(programpath);

	firstarg = commandline(argc, argv, 1);
	
	if (firstarg > argc - 1) showusage(argv[0]);

	printf("Program for changing case of FontName. Version 1.0\n");

	for (m = firstarg; m < argc; m++) {
		strncpy(infilename, argv[m], sizeof(infilename));
		extension(infilename, "pfb");
		if ((input = fopen(infilename, "rb")) == NULL) {
			underscore(infilename);
			if ((input = fopen(infilename, "rb")) == NULL) {
				perror(infilename);
				continue;		/* file not found - probably */
			}
		}
		else fclose(input);		

/*		strncpy(outfilename, stripname(argv[m]), sizeof(outfilename)); */
		strncpy(outfilename, stripname(infilename), sizeof(outfilename));
		extension(outfilename, "pfb");		/* was forceexten */
		renameflag = 0;
		uppercase(infilename, infilename);
		uppercase(outfilename, outfilename);		
		if (traceflag != 0)
			printf("IN: %s OUT: %s\n", infilename, outfilename); /* debug */
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
		usingreturn = 0; usingboth = 0;

		c = fgetc(input); d = fgetc(input);
		if (c == 0 && d == 1) {					/* probably a PFM file */
			if ((output = fopen(outfilename, "wb")) == NULL) {
				perror(outfilename);
				fclose(input);
				if (renameflag != 0) (void) rename(infilename, oldfilename);
				break;
			}
			if (verboseflag != 0) printf(" => %s\n", outfilename);
			
			fputc(c, output); fputc(d, output);	/* copy first two bytes */

			if (copypfmfile(output, input) < 0) {
				fclose(input);
				if (renameflag != 0) (void) rename(infilename, oldfilename);
				fclose(output);
				break;
			}
			else fclose(input);
			
			if (fclose(output) == EOF) {
				perror(outfilename);
				break;			/* output device full - probably */
			}
			else filecount++;
		}
		else if (c == 128 && d == 1) {	/* probably a PFB file */

/*			if (verboseflag != 0) printf("Processing %s ", infilename); */

			if ((output = fopen(outfilename, "wb")) == NULL) {
				perror(outfilename);
				fclose(input);
				if (renameflag != 0) (void) rename(infilename, oldfilename);
				continue;	/* return (1); */
			}

			if (verboseflag != 0) printf(" => %s\n", outfilename);

			fputc(c, output); fputc(d, output);		/* copy first two bytes */
			asciilength = readfour(input);

/*			if (verboseflag != 0) {
				if (usingboth != 0) putc('B', stdout);
				else if (usingreturn != 0) putc('R', stdout);
				else putc('N', stdout);
				printf(" (ASCII %ld)\n", asciilength);
		} */

			writefour(asciilength, output);
			
			(void) readline(buffer, BUFFERLEN, input);
			if (copypfbfile(output, input) < 0) {
				fclose(input);
				if (renameflag != 0) (void) rename(infilename, oldfilename);
				fclose(output);
				break;
			}
			else fclose(input);

			fclose(input);
			if (fclose(output) == EOF) {
				perror(outfilename);
				break;				/* output device full - probably */
			}
			else filecount++;
		}
		else if (c == '%' && d == '!') {	/* probably a PFA file */

/*			if (verboseflag != 0) printf("Processing %s ", infilename); */

			if ((output = fopen(outfilename, "wb")) == NULL) {
				perror(outfilename);
				fclose(input);
				if (renameflag != 0) (void) rename(infilename, oldfilename);
				continue;	/* return (1); */
			}

			if (verboseflag != 0) printf(" => %s\n", outfilename);

			*buffer = '%'; *(buffer+1) = '!';
			(void) readline(buffer+2, BUFFERLEN-2, input);

			if (copypfbfile(output, input) < 0) {
				fclose(input);
				if (renameflag != 0) (void) rename(infilename, oldfilename);
				fclose(output);
				break;
			}
			else fclose(input);

			fclose(input);
			if (fclose(output) == EOF) {
				perror(outfilename);
				break;				/* output device full - probably */
			}
			else filecount++;
		}
		else {
			fprintf(stderr, "Not a valid PFB or PFM file\n");
			fclose(input);
			if (renameflag != 0) (void) rename(infilename, oldfilename);
		}
	}

/*	if (verboseflag != 0 && filecount > 0)  */
	if (filecount > 1) 
		printf("Changed case in %d font files\n", filecount);
		
	return 0;
}

/* Mostly of use with TeX Computer Modern fonts, which normally have
   lower case font names, but where the Type 1 version uses upper case
   to deal with Mac problem in 5 + 3 + 3 contraction non-uniqueness */
