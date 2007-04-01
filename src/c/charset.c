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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Program for modifying CharSet and Family */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* #define FNAMELEN 80 */
#define BUFFERLEN 16000

#define MAXCHRS 256
#define MAXCHARNAME 32

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned char buffer[BUFFERLEN];

char programpath[FILENAME_MAX] = "";

int verboseflag = 0;
int nochangeflag = 0;
int traceflag = 0;
int detailflag = 0;

int charsetflag = 0;	/* next arg is new CharSet */
int familyflag = 0;		/* next arg is new Family */

char *charsetstring = "";
char *familystring = "";

int charset = -1;		/* remains -1 if not specified */
int family = -1;		/* remains -1 if not specified */
int weightflag = 0;		/* set to 1 if `b' or 'n' or 'l'  on command line */
int weight = 0;			/* set to weight desired */
int italicflag = 0;		/* set to 1 if `i' used on command line */
						/* set to -1 if `r' used on command line */

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

void showusage(char *s) {
	printf("Usage:\n");
	printf("%s [-{v}{b}{n}{i}{r}] [-c=<CharSet>] [-f=<Family>]\n\
\t\t<pfm-file-1> <pfm-file-2> ..\n", s);
/*	printf("\n");  */
	printf("\tv  verbose mode\n");
	printf("\tb  set bold flag\n");
	printf("\tn  reset bold flag\n");
	printf("\ti  set italic flag\n");
	printf("\tr  reset italic flag\n");
	printf("\tc  CharSet (ANSII, Symbol, Kanji, or OEM)\n");
	printf("\tf  Family (Roman, Swiss, Modern, Script, or Decorative)\n");
	printf("\n"); 
	printf("\t   (output files appear in current directory)\n");
	printf("\n"); 
	exit(0);
}

int decodeflag (int c) { 			/* decode command  line flag */
	switch(c) { 
		case 'v': if(verboseflag != 0) traceflag = 1; else verboseflag = 1; return 0; break; 
		case '?': detailflag = 1; return 0; break;
		case 'b': weightflag = 1; weight = 700; return 0; break;
		case 'n': weightflag = 1; weight = 400; return 0; break;
		case 'l': weightflag = 1; weight = 250; return 0; break;
		case 'i': italicflag = 1; return 0; break;
		case 'r': italicflag = -1; return 0; break;
		case 'c': charsetflag = 1; return -1; break;
		case 'f': familyflag = 1; return -1; break;
		default: {
				fprintf(stderr, "Invalid command line flag '%c'\n", c);
				exit(13);
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command line flags */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break; 
			else if (decodeflag(c) != 0) { /* flag requires argument ? */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (charsetflag != 0) {
					charsetstring = s;		charsetflag = 0;
				} 
				if (familyflag != 0) {
					familystring = s;		familyflag = 0;
				} 
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
	while ((c = *t++) != '\0') {
		if (c >= 'a' && c <= 'z') *s++ = (char) (c + 'A' - 'a');
		else *s++ = (char) c;
	}
	*s++ = (char) c;
}

long readpfmfile(FILE *input) { 
	long length = 0;
	unsigned char *s = buffer;
	int c;

	while ((c = getc(input)) != EOF && length++ < BUFFERLEN)
		*s++ = (unsigned char) c;
	
	if (length >= BUFFERLEN) return -1;
	else return length;
}

void writepfmfile(FILE *output, long length) { 
	unsigned char *s = buffer;

	while (length-- > 0) putc(*s++, output);
}

void writeweight(int weight) {
	int c, d;
	c = weight >> 8;
	d = weight & 255;
	*(buffer + 84) = (char) c;
	*(buffer + 83) = (char) d;
}

int readweight(void) {
	unsigned int c, d;
	c = *(buffer + 84);
	d = *(buffer + 83);
	return (c << 8 | d);
}

void showcharandfam (char *stuff) {
	int charset, family, weight, italic;
/*	putc('\n', stdout); */
	if (nochangeflag == 0) printf("%s", stuff);
	else putc(' ', stdout);
	charset = *(buffer + 85);
	family = (*(buffer + 90)) >> 4;
	weight = readweight();
	italic = *(buffer + 80); 
	if (weight > 400) printf("Bold");
	else printf("    ");
	if (italic != 0) printf("Italic");
	else printf("      ");
/*	if (weight > 400 || italic != 0) */
	putc(' ', stdout); 
	printf("CharSet: ");
	switch(charset) {
		case 0: printf("ANSI  "); break;
		case 2: printf("Symbol"); break;
		case 128: printf("Kanji "); break;
		case 255: printf("OEM   "); break;
		default: printf("Unknown (%d)", charset); break;
	}
/*	printf(", "); */ 
	putc(' ', stdout);
	printf("Family: ");
	switch(family) {
		case 0: printf("DontCare (Custom)"); break;
		case 1: printf("Roman (Serif)"); break;
		case 2: printf("Swiss (SansSerif)"); break;
		case 3: printf("Modern (Fixed)"); break;
		case 4: printf("Script (Cursive)"); break;
		case 5: printf("Decorative (Symbol)"); break;
		default: printf("Unknown (%d)", family); break;
	}
	putc('\n', stdout);
}

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX], oldfilename[FILENAME_MAX];
/*	char *s; */
	int firstarg = 1, filecount = 0;
	long length;
	int m, renameflag=0;
	
	if (argc < 2) showusage(argv[0]);
	
	strncpy(programpath, argv[0], sizeof(programpath));
	removepath(programpath);

	firstarg = commandline(argc, argv, 1);
	
	if (firstarg > argc - 1) showusage(argv[0]);

	printf("Program for changing CharSet and Family. Version 1.0\n");

/*  analyze CharSetString and FamilyString */
	
	if (strcmp(charsetstring, "") != 0) {
		uppercase(charsetstring, charsetstring);
		if (strcmp(charsetstring, "ANSI") == 0) charset = 0;
		else if (strcmp(charsetstring, "SYMBOL") == 0) charset = 2;
		else if (strcmp(charsetstring, "KANJI") == 0) charset = 128;
		else if (strcmp(charsetstring, "OEM") == 0) charset = 255;
		else {
			fprintf(stderr, "Do not recognize CharSet %s", charsetstring);
			exit(1);
		}
	}

	if (strcmp(familystring, "") != 0) {
		uppercase(familystring, familystring);
		if (strcmp(familystring, "CUSTOM") == 0) family = 0;
		else if (strcmp(familystring, "DONTCARE") == 0) family = 0;

		else if (strcmp(familystring, "ROMAN") == 0) family = 1;
		else if (strcmp(familystring, "SERIF") == 0) family = 1;
		else if (strcmp(familystring, "TIMESROMAN") == 0) family = 1;

		else if (strcmp(familystring, "SWISS") == 0) family = 2;
		else if (strcmp(familystring, "SANSSERIF") == 0) family = 2;
		else if (strcmp(familystring, "HELVETICA") == 0) family = 2;

		else if (strcmp(familystring, "MODERN") == 0) family = 3;
		else if (strcmp(familystring, "FIXED") == 0) family = 3;
		else if (strcmp(familystring, "COURIER") == 0) family = 3;

		else if (strcmp(familystring, "SCRIPT") == 0) family = 4;
		else if (strcmp(familystring, "CURSIVE") == 0) family = 4;

		else if (strcmp(familystring, "DECORATIVE") == 0) family = 5;
		else if (strcmp(familystring, "SYMBOL") == 0) family = 5;

		else {
			fprintf(stderr, "Do not recognize Family %s", familystring);
			exit(1);
		}
	}

	if (charset < 0 && family < 0 && weightflag == 0 && italicflag == 0) 
		nochangeflag = 1;

	for (m = firstarg; m < argc; m++) {
		strncpy(infilename, argv[m], sizeof(infilename));
		extension(infilename, "pfm");
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;		/* file not found - probably */
		}
		else fclose(input);		

		if (nochangeflag == 0) {			/* 95/Mar/25 */
			strncpy(outfilename, stripname(argv[m]), sizeof(outfilename));		

			forceexten(outfilename, "pfm");
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
		}

		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;		/* 	return (1); */
		}
		
		if (nochangeflag != 0) printf("%s", infilename);
		else if (verboseflag != 0) {
			printf("Processing %s ", infilename); 
			printf(" => %s\n", outfilename);
		}

		length = readpfmfile(input);
		fclose(input);

		if (*buffer > 1 || *(buffer+1) > 3) { /* should be 0, 1 (version) */
			fprintf(stderr, "Not a valid PFM file\n");
			continue;
		}
		if (length < 0) {
			fprintf(stderr, "File too long\n");
			continue;
		}

		if (verboseflag != 0 || nochangeflag != 0) showcharandfam("OLD: ");
		
/*		if (nochangeflag != 0) continue; */			/* 95/Mar/25 */

		if (charset >= 0) *(buffer + 85) = (unsigned char) charset;
		
		if (family >= 0) *(buffer + 90) = (unsigned char) ((family << 4) | 1);

		if (weightflag != 0) writeweight(weight);
		
		if (italicflag > 0) *(buffer + 80) = 1;
		else if (italicflag < 0) *(buffer + 80) = 0;

		if (nochangeflag == 0) {
			if (verboseflag != 0) showcharandfam("NEW: ");
			if ((output = fopen(outfilename, "wb")) == NULL) {
				perror(outfilename);
				fclose(input);
				if (renameflag != 0) rename(infilename, oldfilename);
				continue;	/* return (1); */
			}
			writepfmfile(output, length);
			if (fclose(output) == EOF) {
				perror(outfilename);
				break;				/* output device full - probably */
			}
			else filecount++;
		}
	}
/*	if (verboseflag != 0 && filecount > 0)  */
	if (filecount > 1) 
		printf("Processed %d PFM files\n", filecount);
		
	return 0;
}

