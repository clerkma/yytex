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

/* coalesce Subrs to reduce total number required */
/* renumber the Subrs */
/* remove redundant Subrs */ /* remove unused Subrs */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 256
/* #define FNAMELEN 80 */
#define MAXSUBRS 2048

/* FROMCHAR if Subr called from character, FROMSUBR if called from Subr */

#define FROMCHAR 1
#define FROMSUBR 2

int verboseflag=1;
int traceflag=0;

/* hvcblmi_.pln: 922 callsubr */ /* normal subroutine */
/* hvcblmi_.pln: 750 1 3 callothersubr */ /* hint replacement subroutine */
/* optionally translate to 750 4 callsubr */

int usesubrfour=0;		/* Subr 4 appears to be set up for hint switching */
int replaceone=1;		/* replace old fashioned Subr calls */

/* 109 1 3 callsubr, pop, callsubr => 109 4 callsubr */

char line[MAXLINE];
char buffer[MAXLINE];

int nsubrs, nchars;

/* not sure distinction between `normal' Subrs and hint replacement needed */

int normsubr[MAXSUBRS];

int hintsubr[MAXSUBRS];

int checksubr[MAXSUBRS];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension(char *fname, char *str) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int scantosubrs (FILE *output, FILE *input) {
	char *s;

	if (traceflag) printf("Scanning up to Subrs\n");
	while (fgets(line, MAXLINE, input) != NULL) {
		if ((s = strstr(line, "/Subrs ")) != NULL) {
			if (sscanf(s, "/Subrs %d array", &nsubrs) == 1) {
				return nsubrs;
			}
		}
		if (output != NULL) fputs(line, output);
	}
	fprintf(stderr, "Cannot find /Subrs line\n");
	exit(1);
}

/* scan Subrs and CharStrings for callsubr and callothersubr */

/* this gets run recursively */

int scanforsubrs (FILE *input) {
	char *s;
	int k;
	int section=1;	/* one for Subrs, 2 for CharStrings */
	int subr=0;		/* current Subr we are in */
	int len;

	if (traceflag) printf("Scanning for Subrs\n");
	while (fgets(line, MAXLINE, input) != NULL) {
/* keep track of which section we are in */
		if ((s = strstr(line, "/CharStrings ")) != NULL) {
			if (sscanf(s, "/CharStrings %d dict", &nchars) == 1) {
				section=2;
				subr=0;
			}
		}
/* keep track of which Subr we are in */
		if (section == 1) {		
			if (strncmp(line, "dup ", 4) == 0) {
				if (sscanf (line, "dup %d %d RD", &subr, &len) == 2) {
				}
			}
		}
		if (strncmp(line, "1 3 callothersubr", 17) == 0) {
			if (subr == 4) {
				usesubrfour=1;
			}
		}
		else if ((s = strstr(line, " 4 callsubr")) != NULL) {
			if (sscanf(line, "%d 4 callsubr", &k) == 1) {
				if (section > 1 || checksubr[subr])
					if (k >=0 && k < MAXSUBRS) hintsubr[k]=section;
			}
		}
		else if ((s = strstr(line, " callsubr")) != NULL) {
			if (sscanf(line, "%d callsubr", &k) == 1) {
				if (section > 1 || checksubr[subr])
					if (k >= 0 && k < MAXSUBRS) normsubr[k]=section;
			}
		}
		else if ((s = strstr(line, " 1 3 callothersubr")) != NULL) {
			if (sscanf(line, "%d 1 3 callothersubr", &k) == 1) {
				if (section > 1 || checksubr[subr])
					if (k >= 0 && k < MAXSUBRS) hintsubr[k]=section;
			}
		}
	}
	return nchars;
}

void renumbersubrs (FILE *output, FILE *input) {
	char *s;
	int k, len, n, newk;

	if (traceflag) printf("Now renumbering the Subrs\n");
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "dup ", 4) == 0) {
			if (sscanf (line, "dup %d %d%n RD", &k, &len, &n) == 2) {
				strcpy(buffer, line+n);
				if (normsubr[k] >= 0) newk = normsubr[k];
				else if (hintsubr[k] >= 0) newk = hintsubr[k];
				else newk = -1;
				if (newk >= 0) {
					sprintf(line, "dup %d %d", newk, len);
					strcat(line, buffer);
				}
				else {					/* flush to end of Subr */
					while (fgets(line, MAXLINE, input) != NULL) {
						if (strncmp(line, "return", 6) == 0) {
							fgets(line, MAXLINE, input);
							fgets(line, MAXLINE, input);
							if (*line == '\n') {
								*line = '\0'; 		/* nuke this line too */
							}
							break;
						}
						if (*line == '\n') {
							*line = '\0'; 		/* nuke this line too */
							break;
						}
					}
				}
			}
			if (traceflag) fputs(line, stdout);
		}
		else if ((s = strstr(line, " 4 callsubr")) != NULL) {
			if (sscanf(line, "%d 4 callsubr", &k) == 1) {
				if (hintsubr[k] >= 0)
					sprintf(line, "%d 4 callsubr\n", hintsubr[k]);
				else fprintf(stderr, "Impossible! %s", line);
			}
			else fprintf(stderr, "Impossible!, %s", line);
			if (traceflag) fputs(line, stdout);
		}
		else if ((s = strstr(line, " callsubr")) != NULL) {
			if (sscanf(line, "%d callsubr", &k) == 1) {
				if (normsubr[k] >= 0)
					sprintf(line, "%d callsubr\n", normsubr[k]);
				else fprintf(stderr, "Impossible! %s", line);
			}
			else fprintf(stderr, "Impossible! %s", line);
			if (traceflag) fputs(line, stdout);
		}
		else if ((s = strstr(line, " 1 3 callothersubr")) != NULL) {
			if (sscanf(line, "%d 1 3 callothersubr", &k) == 1) {
				if (hintsubr[k] >= 0) {
					if (usesubrfour && replaceone) {
/* 						sprintf (line, "%d 4 callsubr\n", hintsubr[k]); */
						fgets(line, MAXLINE, input);
						if (strncmp(line, "pop", 3) != 0)
							fprintf(stderr, "Impossible! %s", line);
						fgets(line, MAXLINE, input);
						if (strncmp(line, "callsubr", 3) != 0)
							fprintf(stderr, "Impossible! %s", line);
						sprintf (line, "%d 4 callsubr\n", hintsubr[k]);
					}
					else sprintf(line, "%d 1 3 callothersubr\n", hintsubr[k]);
				}
				else fprintf(stderr, "Impossible! %s", line);
				}
			else fprintf(stderr, "Impossible! %s", line);
			if (traceflag) fputs(line, stdout);
		}
		fputs(line, output);
	}
}

int shownormsubrs(void) {
	int k, count=0;
	if (traceflag) printf("Normal Subrs needed: ");
	for (k = 0; k < MAXSUBRS; k++) {
		if (normsubr[k] >= 0) {
			if (traceflag) printf("%d ", k);
			count++;
		}
	}
	if (traceflag) putc('\n', stdout);
	return count;
}

int showhintsubrs(void) {
	int k, count=0;
	if (traceflag) printf("Hint replacement Subrs needed: ");
	for (k = 0; k < MAXSUBRS; k++) {
		if (hintsubr[k] >= 0) {
			if (traceflag) printf("%d ", k);
			count++;
		}
	}
	if (traceflag) putc('\n', stdout);
	return count;
}

int assignnumbers (void) {
	int next=0;
	int k;

	if (traceflag) printf("Assigning new Subr numbers\n");
	for (k = 0; k < MAXSUBRS; k++) {
		if (normsubr[k] >= 0) normsubr[k] = next++;
		else if (hintsubr[k] >= 0) hintsubr[k] = next++;
	}
	return next;
}

void insertnewcount(char *line, int nsubrs) {
	char *s;
	int nold, n;

	if ((s = strstr(line, "/Subrs ")) != NULL) {
		s += 7;
		if (sscanf(s, "%d%n", &nold, &n) == 1) {
			strcpy(buffer, s + n);
			sprintf(s, "%d", nsubrs);
			strcat(line, buffer);
		}
	}
}

/* prune list of subrs to check */ /* returns those dropped this round */

int prunesubr (void) {
	int k, dropped=0;

	if (traceflag) printf("Now attempting to prune list of Subrs needed\n");
/*	for (k = 0; k < MAXSUBRS; k++) { */
	for (k = 0; k < nsubrs; k++) {
		if (checksubr[k] == 0) continue;		/* already removed */
		if (hintsubr[k] == FROMCHAR ||
			normsubr[k] == FROMCHAR) continue;	/* called from character */
		if (hintsubr[k] < 0 && normsubr[k] < 0) {
			if (checksubr[k]) dropped++;		/* was used in last round */
			checksubr[k] = 0;					/* nobody calling this one */
		}
	}
	for (k = nsubrs; k < MAXSUBRS; k++) checksubr[k] = 0;
	if (verboseflag)
		if (dropped) printf("Dropping %d Subrs\n", dropped);
	return dropped;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	int k, m, n, firstarg=1;
	FILE *input, *output;
!	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
	int  normcount, hintcount;
	
	if (argc < 2) exit(1);

/*	while (*argv[firstarg] == '-') */
	while (firstarg < argc && *argv[firstarg] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag=1-verboseflag;
		if (strcmp(argv[firstarg], "-t") == 0) traceflag=1-traceflag;
		if (strcmp(argv[firstarg], "-f") == 0) replaceone=1-replaceone;
		firstarg++;
	}
	if (traceflag) printf("Firstarg is %d %s\n", firstarg, argv[firstarg]);

	if (argc < firstarg+1) exit(1);

	for (m=firstarg; m < argc; m++) {

	for (k = 0; k < MAXSUBRS; k++) checksubr[k]=1;

	strcpy(infile, argv[m]);
	extension(infile, "pln");
	if ((input = fopen(infile, "r")) == NULL) {
		perror(infile);
		exit(1);
	}
	if (verboseflag) printf("Now processing %s\n", infile);
	for (n = 0; n < 32; n++) {
		for (k = 0; k < MAXSUBRS; k++) normsubr[k]=-1;
		for (k = 0; k < MAXSUBRS; k++) hintsubr[k]=-1;
		for (k = 0; k < 4; k++) normsubr[k]=1;
		if (verboseflag) printf("Subr pruning pass %d\n", n+1);
		nsubrs = scantosubrs(NULL, input);
		if (nsubrs < 0) {
			fprintf(stderr, "Can't find Subrs array\n");
			fclose(input);
			exit(1);
		}
		if (n == 0) 
			if (verboseflag) printf("Font contains %d Subrs\n", nsubrs);

		nchars = scanforsubrs(input);
		if (n == 0) {
			if (verboseflag) {
				printf("Font contains %d CharStrings\n", nchars);
				if (usesubrfour)
					printf("Can use Subr 4 for efficient hint switching\n");
			}
		}

		if (usesubrfour >= 0) normsubr[4] = 1;
		rewind (input);
		if (prunesubr() == 0) {
			if (verboseflag) printf("No more Subrs can be stripped out\n");
			break;
		}
	}

	normcount = shownormsubrs();
	hintcount = showhintsubrs();
	printf("Need %d normal Subrs and %d hint switching Subrs\n",
		normcount, hintcount);
	if (normcount + hintcount == nsubrs) {
		printf("Nothing to do!\n");
		fclose(input);
		return 0;
	}
	assignnumbers ();

	rewind(input);
	strcpy(outfile, infile);
	forceexten(outfile, "coa");
	if ((output = fopen(outfile, "w")) == NULL) {
		perror(outfile);
		exit(1);
	}
	scantosubrs(output, input);
	insertnewcount(line, normcount + hintcount);
	if (traceflag) fputs(line, stdout);
	printf("Will strip out %d unused Subrs (leaving %d)\n",
		nsubrs - (normcount + hintcount), normcount + hintcount); 
	fputs(line, output);
	renumbersubrs(output, input);
	fclose(output);
	fclose(input);
	}
	return 0;
}

/* need to run this recursively, since some subrs may only be called */
/* from within other Subrs that happen to get eliminated */
