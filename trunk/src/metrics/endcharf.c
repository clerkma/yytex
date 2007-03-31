/* ENDCHARF.C
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Fix fonts so problems with SAFESEAC are avoided */
/* where a Subr is called that ends in an endchar */

/* find all places where an endchar occurs in a Subr */
/* remove it and remember which Subr number it was */

/* find all places where these marked Subrs are called */
/* and add an endchar after */

#define MAXSUBRS 2048

#define MAXLINE 512

char badend[MAXSUBRS];

char line[MAXLINE];

int verboseflag=1;
int traceflag=1;
int debugflag=0;

/* returns how many new badend[] entries marked - zero when done */

int findbadsubrs(FILE *input) {
	int nsubr, ncall, nlen, nsubrs=0;
	int count=0, badcount=0, changed=0;

/*	for (k = 0; k < MAXSUBRS; k++) badend[k] = 0;	 */

	if (verboseflag) printf("Scanning up to Subrs\n");

/*	scan up to Subrs */
	while (fgets (line, MAXLINE, input) != NULL) {	
/*		fputs(line, output); */
		if (sscanf(line, "/Subrs %d array", &nsubrs) == 1) break;
	}

	if (nsubrs > MAXSUBRS) printf("Too many Subrs %d\n", nsubrs);

	if (verboseflag) printf("Scanning %d Subrs\n", nsubrs);

/*	scan the Subrs up to CharStrings */
	while (fgets (line, MAXLINE, input) != NULL) {	
/*		fputs(line, output); */
		if (strstr(line, "/CharStrings ") != NULL) break;
		if (strstr(line, "callsubr") != NULL) {
			if (sscanf(line, "%d callsubr", &ncall) == 1) {
				if (badend[ncall]) {
					badend[nsubr] = 1;
					changed++;
				}
			}
			printf("WARNING: Subr call in Subrs:\t %s", line);
			badcount++;
		}
		if (sscanf(line, "dup %d %d", &nsubr, &nlen) == 2) {
			if (debugflag) fputs(line, stdout);
			while (fgets (line, MAXLINE, input) != NULL) {	
				if (strncmp(line, "endchar", 7) == 0) {
					if (traceflag) printf("endchar\t\tSubr %d\n", nsubr);
					if (nsubr >= 0 && nsubr < MAXSUBRS) {
						badend[nsubr] = 1;
					}
					count++;
				}	/* flush the endchar - and remember which Subr */
/*				else fputs(line, output); */
				if (strncmp(line, "return", 6) == 0) break;
				if (*line == '\n' || *line == '\r') break;
			}
		}
	}
	if (badcount > 0) {
		fprintf(stderr, "WARNING: check those Subrs calling Subrs\n");
	}
	return changed;
}

int cleanupends(FILE *output, FILE *input) {
	int nsubr, nlen, nsubrs=0;
	int count=0, badcount=0;
	char charname[64];

/*	for (k = 0; k < MAXSUBRS; k++) badend[k] = 0;	 */

	if (verboseflag) printf("Scanning up to Subrs\n");

/*	scan up to Subrs */
	while (fgets (line, MAXLINE, input) != NULL) {	
		fputs(line, output);
		if (sscanf(line, "/Subrs %d array", &nsubrs) == 1) break;
	}

	if (nsubrs > MAXSUBRS) printf("Too many Subrs %d\n", nsubrs);

	if (verboseflag) printf("Scanning %d Subrs\n", nsubrs);

/*	scan the Subrs up to CharStrings */
	while (fgets (line, MAXLINE, input) != NULL) {	
		fputs(line, output);
		if (strstr(line, "/CharStrings ") != NULL) break;
/*		if (strstr(line, "callsubr") != NULL) {
			printf("WARNING: Subr call in Subrs:\t %s", line);
			badcount++;
		} */
		if (sscanf(line, "dup %d %d", &nsubr, &nlen) == 2) {
/*			if (debugflag) fputs(line, stdout); */
			while (fgets (line, MAXLINE, input) != NULL) {	
				if (strncmp(line, "endchar", 7) == 0) {
/*					if (traceflag) printf("endchar\t\tSubr %d\n", nsubr);*/
					if (nsubr >= 0 && nsubr < MAXSUBRS) {
						badend[nsubr] = 1;
					}
					count++;
				}	/* flush the endchar - and remember which Subr */
				else fputs(line, output);
				if (strncmp(line, "return", 6) == 0) break;
				if (*line == '\n' || *line == '\r') break;
			}
		}
	}

	if (verboseflag) printf("Scanning the CharStrings\n");

/*	scan the CharStrings */
	while (fgets (line, MAXLINE, input) != NULL) {	
		fputs(line, output);
		if (sscanf(line, "/%s %d", &charname, &nlen) == 2) {
			if (debugflag) fputs(line, stdout);
			while (fgets (line, MAXLINE, input) != NULL) {	
				fputs(line, output);
				if (strstr(line, "callsubr") != NULL) {
					if (sscanf(line, "%d callsubr", &nsubr) == 1) {
						if (nsubr >= 0 && nsubr < MAXSUBRS) {
							if (badend[nsubr]) {
								if (traceflag)
									printf("%s\t\t%s", charname, line);
								fputs("endchar\n", output);
							}
						}
					}
				}
				if (strncmp(line, "endchar", 6) == 0) break;
				if (*line == '\n' || *line == '\r') break;
			}
		}
		if (strstr(line, "/FontName") != NULL) break;
		if (strstr(line, "definefont") != NULL) break;
	}

	if (verboseflag) printf("Copying to the end of the file\n");

/*	copy to end of file */
	while (fgets (line, MAXLINE, input) != NULL) {	
		fputs(line, output);
	}
	return count;
}

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

int main (int argc, char *argv[]) {
	int m, k, count;
	FILE *output, *input;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];

	for (m = 1; m < argc; m++) {
		strcpy(infilename, argv[m]);
		extension(infilename, "pln");
		if ((input = fopen(infilename, "r")) == NULL) {
			perror(infilename);
			continue;
		}
		strcpy(outfilename, argv[m]);
		forceexten(outfilename, "end");
		if (strcmp(infilename, outfilename) == 0) continue;
/*		if ((output = fopen(outfilename, "w")) == NULL) { */
		if ((output = fopen(outfilename, "wb")) == NULL) {
			perror(outfilename);
			fclose(input);
/*			continue; */
			exit(1);
		}
		for (k = 0; k < MAXSUBRS; k++) badend[k] = 0;
		while (findbadsubrs(input) > 0) rewind(input);
		rewind (input);
		count = cleanupends(output, input);
		fclose(output);
		fclose(input);
	}
	return 0;
}
