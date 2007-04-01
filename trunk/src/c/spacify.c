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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* #include <conio.h> */

/* interchange `suppress' and `space' in CM fonts */
/* interchange any two characters in any two fonts */

#define MAXFILENAME 128
#define MAXLINE 256

int verboseflag = 0;
int detailflag = 0;
int traceflag = 0;
/* int dopfmalso = 0; */
int dopfmalso = 1; 

int charaflag=0;
int charbflag=0;
int explicitflag= 0;			/* if chara or charb explicitly given */

char line[MAXLINE];

char space[48]="space";			/* char names to interchange */
char suppress[48]="suppress";	/* char names to interchange */

int spacecode=32;				/* expected character codes */
int suppresscode=160;			/* expected character codes */

long spaceptr;			/* pointer to `space'    in PFB */
long suppressptr;		/* pointer to `suppress' in PFB */

/* int spacechrs, suppresschrs;	*/	/* observed character codes */

int standardflag;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int gettoken (char *buff, FILE *input) {
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
}

void scanpfbfile(FILE *input) {
	int chrs;

/*	search for /Encoding first */
	for (;;) {
		if (gettoken(line, input) == EOF) {
			fprintf(stderr, "Unexpected EOF\n");
			exit(1);
		}
		if (strcmp(line, "/Encoding") == 0) break;
	}

	standardflag = 0;

/*	search for "space" and "suppress" in encoding */
	for (;;) {
		if (gettoken(line, input) == EOF) {
			fprintf(stderr, "Unexpected EOF\n");
			exit(1);
		}
		if (strcmp(line, "StandardEncoding") == 0) {
			standardflag = -1;
			break;
		}
		if (strcmp(line, "readonly") == 0) break;
		if (strcmp(line, "dup") == 0) {
			(void) gettoken(line, input);
			if (sscanf(line, "%d", &chrs) < 1) {
				fprintf(stderr, "%s", line);
				continue;
			}
			if (chrs < 0 || chrs > 255) {
				fprintf(stderr, "Bad char number %d\n", chrs);
				continue;
			}
			(void) gettoken(line, input);

			if (*line == '/' && chrs == spacecode) {
				strcpy(space, line+1);
				spaceptr = ftell(input) - strlen(space) - 2;				
			}
/*			if (*line == '/' &&	strcmp(line+1, space) == 0) {
				if (chrs == suppresscode || chrs == spacecode) {
					spaceptr = ftell(input) - strlen(space) - 2;
					spacechrs = chrs;
				}
			} */
			if (*line == '/' && chrs == suppresscode) {
				strcpy(suppress, line+1);
				suppressptr = ftell(input) - strlen(suppress) - 2;				
			}
/*			if (*line == '/' &&	strcmp(line+1, suppress) == 0) {
				if (chrs == spacecode || chrs == suppresscode) {
					suppressptr = ftell(input) - strlen(suppress) - 2;
					suppresschrs = chrs;
				}
			} */
			if (traceflag != 0) putc('.', stdout);
		}
	}	
}

int modifypfbfile(FILE *input, FILE *output) {
	int c;
	unsigned int i, n;
	long k=0;

	for(;;) {
		if (k == spaceptr) {		/* skip over `/space' */
			n = strlen(space) + 1;
/*			for (i = 0; i < strlen("/space"); i++) { */
			for (i = 0; i < n; i++) {
				(void) getc(input);
				k++;
			}
/*			fprintf(output, "/suppress"); */
			fprintf(output, "/%s", suppress);
		}
		else if (k == suppressptr) {
			n = strlen(suppress) + 1;
/*			for (i = 0; i < strlen("/suppress"); i++) { */
			for (i = 0; i < n; i++) {
				(void) getc(input);
				k++;
			}
/*			fprintf(output, "/space");*/
			fprintf(output, "/%s", space);
		}
		if ((c = getc(input)) == EOF) break;
		putc(c, output);
		k++;
	}
	return 0;
}

int sreadtwo(FILE *input) {
	int c, d;
	c = getc(input); d = getc(input);
	return (d << 8) | c;
}

long sreadfour(FILE *input) {
	int c, d, e, f;
	c = getc(input); d = getc(input);
	e = getc(input); f = getc(input);
	return ((long) f) << 24 | ((long) e) << 16 | ((long) d) << 8 | c;
}

void swritetwo(FILE *output, int num) {
	putc((num & 255), output);
	putc((num >> 8) & 255, output);
}

int modifypfmfile(FILE *input, FILE *output) {
	int c, d, first, last, hit;
	long k;
	long extenttable=0;
	long spaceposition, suppressposition;
	int spacewidth, suppresswidth;

/*	analyze the PFM file first */
	c = getc(input); d = getc(input);
	if (c != 0 || d != 1) {
		fprintf(stderr, "ERROR: This appears not to be a valid PFM file\n");
		return -1;
	}
	fseek(input, 95L, SEEK_SET); 
	first = getc(input);
	last = getc(input);
	if (first != 0 || last != 255) {
		fprintf(stderr, "First char (%d) not 0 or last char (%d) not 255\n",
			first, last);
		return -1;
	}
	fseek(input, 123L, SEEK_SET);
	extenttable = sreadfour(input);
	if(fseek(input, extenttable, SEEK_SET) != 0) {
		fprintf(stderr, "Seek error in PFM file (%ld)\n", extenttable);
		return -1;
	}
/*	now go to position of space character and read its width */
/*	spaceposition = extenttable + 2 * (spacechrs - first);*/
	spaceposition = extenttable + 2 * (spacecode - first);
	fseek (input, spaceposition, SEEK_SET);
	spacewidth = sreadtwo(input);
/*	now go to position of suppress character and read its width */
/*	suppressposition = extenttable + 2 * (suppresschrs - first); */
	suppressposition = extenttable + 2 * (suppresscode - first);
	fseek (input, suppressposition, SEEK_SET);
	suppresswidth = sreadtwo(input);

	if (spacewidth == suppresswidth) {
		printf("`%s' width equals `%s' width (%d)\n", 
			space, suppress, spacewidth);
	}
	else if (verboseflag != 0) 
		printf("`%s' width %d - `%s' width %d \n", 
			space, spacewidth, suppress, suppresswidth);
	
/*	now actually do copying */
	fseek(input, 0L, SEEK_SET);
	k = 0; hit = 0;
	for(;;) {
		if (k == spaceposition) {
			(void) getc(input); k++;
			(void) getc(input); k++;
			swritetwo(output, suppresswidth);
			hit++;
		}
		else if (k == suppressposition) {
			(void) getc(input); k++;
			(void) getc(input); k++;
			swritetwo(output, spacewidth);
			hit++;
		}
		c = getc(input);
		if (c == EOF) break;
		putc(c, output);
		k++;
	}
	if (hit != 2) {
		fprintf(stderr, "Did not hit the two width positions\n");
		return -1;
	}
	return 0;
}

void showusage(char *s) {
	printf("\n");
	printf("Usage: %s [-v] [-a=<code>] [-b-<code>]", s);
	printf(" <PFB file to modify> \n");
	if (detailflag == 0) exit(0);
	printf("\n");
	printf("\tv verbose mode (recommended)\n");
	printf("\ta character code (default `%d')\n", spacecode);
	printf("\tb character code (default `%d')\n", suppresscode);	
	printf("\n");
	printf("\t  PFM file assumed to be in PFM subdirectory\n");
	printf("\t  Modified PFB and PFM files appear in current directory");
	exit(1);
}

int decodeflag (int c) { 			/* decode command  line flag */
	switch(c) { 
		case 'v': if(verboseflag != 0) traceflag = 1; else verboseflag = 1; return 0; 
		case '?': detailflag = 1; return 0; 
/* the rest take arguments */
		case 'a': charaflag = 1; return -1; 
		case 'b': charbflag = 1; return -1; 
		default: {
				fprintf(stderr, "Invalid command line flag '%c'\n", c);
				exit(13);
		}
	}
/*	return 0; */	/* ??? */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break; 
			else if (decodeflag(c) != 0) { /* flag requires argument ? */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (charaflag != 0) {
					if (sscanf (s, "%d", &spacecode) < 1)
						fprintf(stderr, "Don't understand %s\n", s);
					charaflag = 0;
					explicitflag++;
				}
				if (charbflag != 0) {
					if (sscanf (s, "%d", &suppresscode) < 1)
						fprintf(stderr, "Don't understand %s\n", s);
					charbflag = 0;
					explicitflag++;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

int main(int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[MAXFILENAME];
	char bakfilename[MAXFILENAME];
	char outfilename[MAXFILENAME];	
	int m, flag, pfbcount=0, pfmcount=0;
	int flushflag=0, warnflag=0;
	int infiles=0, outfiles=0; 
	int firstarg = 1;
	char *s;

	if (argc < 2) showusage(argv[0]);

	firstarg = commandline(argc, argv, 1);
	
	if (suppresscode == spacecode) {
		fprintf(stderr, "%d == %d\n", spacecode, suppresscode);
		exit(17);
	}

	if (firstarg > argc - 1) showusage(argv[0]);

/*	printf("Program for interchanging two characters. Version 0.9\n"); */
	printf("Program for interchanging two characters. Version 1.0\n");

	if (argc < firstarg + 1) exit(1);

	for (m = firstarg; m < argc; m++) {

		strcpy(infilename, argv[m]);
		if (strchr(infilename, '.') == NULL) strcat(infilename, ".pfb");
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
/*			exit(1); */
			continue;
		}
		infiles++; 
		spaceptr = 0; suppressptr = 0;
		scanpfbfile(input);
		(void) fclose(input);
		infiles--;

		if (standardflag != 0) {
			printf("%s uses Adobe StandardEncoding\n", infilename);
			continue;
		}
/*		if (spaceptr == 0 || suppressptr == 0) {
			printf("%s does not contain both `%s' and `%s'\n",
				infilename, space, suppress);
			continue;
		} */
/*		if (spaceptr == 0 || suppressptr == 0) {
			printf("%s does not contain character `%d' or `%d'\n",
				infilename, spacecode, suppresscode);
			continue;
		} */
		flushflag = 0;				/* 1993/Aug/23 */
		if (spaceptr == 0) {
			printf("%s does not contain character `%d'\n",
				infilename, spacecode);
/*			continue; */
			flushflag++;
		}
		if (suppressptr == 0) {
			printf("%s does not contain character `%d'\n",
				infilename, suppresscode);
/*			continue; */
			flushflag++;
		}
		if (flushflag != 0) continue;
/*		if (spaceptr == suppressptr) { */
/*		if (spacecode == suppresscode) { 
			printf("%s has same character code for `%s' and `%s'\n",
				infilename, space, suppress);
			continue;
		} */

/*		complain if we are not interchanging `space' and `suppress' */
/*		unless user explicitly specified `to' and `from' characters */
/*		added 1993/Aug/23 */
		if (explicitflag == 0 &&
			(strcmp(space, "space") != 0 &&
				strcmp(suppress, "space") != 0) ||
					(strcmp(space, "suppress") != 0 &&
						strcmp(suppress, "suppress") != 0)) {
 printf("Interchanging `%s' (%d) and `%s' (%d)\n", 
	 space, spacecode, suppress, suppresscode);
 printf("WARNING: this does not appear to be a typical CM text font\n");
 printf("Do not use `spacify' on math fonts or non-CM fonts\n");
			 warnflag++;
		}
		else if (verboseflag != 0)
			printf("Interchanging `%s' (%d) and `%s' (%d)\n", 
				space, spacecode, suppress, suppresscode);


/*		strip path name from file name */
		if ((s = strrchr(infilename, '\\')) != NULL) s++;
		else if ((s = strrchr(infilename, '/')) != NULL) s++;
		else if ((s = strrchr(infilename, ':')) != NULL) s++;
		else s = infilename;
		strcpy(outfilename, s);
		if (strchr(outfilename, '.') == NULL) strcat(outfilename, ".pfb");
		strcpy(bakfilename, "");
		if (strcmp(outfilename, infilename) == 0) {
			strcpy(bakfilename, infilename);
			if ((s = strchr(bakfilename, '.')) != NULL) {
				strcpy(s, ".bak");
			}			
			(void) remove(bakfilename);
			if (rename(infilename, bakfilename) != 0) {
				fprintf(stderr, "Renaming of %s failed\n", infilename);
				continue;
			}
			strcpy(infilename, bakfilename);
		}

		if (verboseflag != 0) printf("%s => %s\n", infilename, outfilename);
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;
		}
		infiles++; 
		if ((output = fopen(outfilename, "wb")) == NULL) {
			(void) fclose(input);
			infiles--; 
			perror(outfilename);
			continue;
		}
		outfiles++; 

		flag = modifypfbfile(input, output);
		(void) fclose(input);
		infiles--; 
		if (ferror(output) != 0) {
			perror(outfilename);
			exit(3);
		}
		else fclose(output);
		outfiles--; 

		if (flag == 0) {
			pfbcount++;
			if (strcmp(bakfilename, "") != 0)
				(void) remove(bakfilename);		/* flush backup file */
		}

		if (dopfmalso == 0) continue;

/* now try and modify PFM file */
		strcpy(bakfilename, "");
		if ((s = strchr(infilename, '.')) != NULL) strcpy(s, ".pfm");
		if ((input = fopen(infilename, "rb")) == NULL) {
/* try and find it in PFM subdirectory if not in same diectory  as PFB */
			if ((s = strrchr(infilename, '\\')) != NULL) s++;
			else if ((s = strrchr(infilename, '/')) != NULL) s++;
			else if ((s = strrchr(infilename, ':')) != NULL) s++;			
			else {
				perror(infilename);
				continue;
			}
			strcpy(bakfilename, s);
			strcpy(s, "pfm\\");
			strcat(infilename, bakfilename);
			if ((input = fopen(infilename, "rb")) == NULL) {
				perror(infilename);
				continue;
			}
		}
		infiles++;
		(void) fclose(input);
		infiles--;
		
		if ((s = strrchr(infilename, '\\')) != NULL) s++;
		else if ((s = strrchr(infilename, '/')) != NULL) s++;
		else if ((s = strrchr(infilename, ':')) != NULL) s++;
		else s = infilename;
		strcpy(outfilename, s);
		if (strchr(outfilename, '.') == NULL) strcat(outfilename, ".pfm");
		strcpy(bakfilename, "");
		if (strcmp(outfilename, infilename) == 0) {
			strcpy(bakfilename, infilename);
			if ((s = strchr(bakfilename, '.')) != NULL) {
				strcpy(s, ".bak");
			}			
			(void) remove(bakfilename);
			if (rename(infilename, bakfilename) != 0) {
				fprintf(stderr, "Renaming of %s failed\n", infilename);
				continue;
			}
			strcpy(infilename, bakfilename);
		}

		if (verboseflag != 0) printf("%s => %s\n", infilename, outfilename);
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;
		} 
		infiles++;
		if ((output = fopen(outfilename, "wb")) == NULL) {
			(void) fclose(input);
			infiles--;
			perror(outfilename);
			continue;
		}
		outfiles++;
		flag = modifypfmfile(input, output);
		(void) fclose(input);
		infiles--;
		if (ferror(output) != 0) {
			perror(outfilename);
			exit(3);
		}
		else fclose(output);
		outfiles--;

		if (flag == 0) {
			pfmcount++;
			if (strcmp(bakfilename, "") != 0)
				(void) remove(bakfilename);	/* flush backup file */
		}
		if (infiles != 0 || outfiles != 0)
			fprintf(stderr, "infiles %d outfiles %d\n", infiles, outfiles);
	}

	if (pfbcount > 0 || pfmcount > 0) {	/* 1993/Aug/23 */
	printf("\nInterchanged characters in %d PFB files and %d PFM files\n", 
		pfbcount, pfmcount);
	printf("Modified PFB and PFM files deposited in current directory\n");
	}
	else printf("\nNo PFB or PFM files were modified\n");
	if (warnflag > 0)
	printf("WARNING: Do not use `spacify' on math fonts or non-CM fonts\n");
	return 0;
}
