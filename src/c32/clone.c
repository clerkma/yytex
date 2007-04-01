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

/* Takes pln file and makes clones of glyphs under new names */
/* Moves affected charstings to Subrs and replaces them with calls to Subrs */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 512

#define MAXCHARSTRINGS 2048

int verboseflag=1;
int traceflag=0;

int cloneeuro=0;					/* uni20AC => Euro instead of table */

char line[MAXLINE];
char str[MAXLINE];

long charstrings[MAXCHARSTRINGS];	/* position of charstrings in file */
char *glyphnames[MAXCHARSTRINGS];	/* glyph names */
int subrused[MAXCHARSTRINGS];		/* which subr this char is in */

int nsubrs;
int ncharstrings;
int chrs;
int nsubrinc;	/* number of new subrs needed */
int nchrinc;	/* number of new charstrings needed */

char *readstring="RD";		/* or -| */
char *noaccessdef="ND";		/* or |- */
char *noaccessput="NP";		/* or | */

/* Table for adding Greek letters that have the same shape as Latin letters */

char *clonetable[][2]=
{
	{"A",	"Alpha"},
	{"B",	"Beta"},
	{"E",	"Epsilon"},
	{"H",	"Eta"},
	{"I",	"Iota"},
	{"K",	"Kappa"},
	{"M",	"Mu"},
	{"N",	"Nu"},
	{"O",	"Omicron"},
	{"P",	"Rho"},
	{"T",	"Tau"},
	{"X",	"Chi"},
//	{"Y",	"Upsilon1"},	/* should have been {"Y",	"Upsilon"}, */
//	{"Y",	"Upsilon2"},
	{"Z",	"Zeta"},
	{"o",	"omicron"},		/* only lower case clone */
	{"",	""},
};

/* { "Upsilon",   0x03A5 }, */	/* looks like upper case Y */
/* { "Upsilon1",  0x03D2 }, */	/* has curled over arms `with hook' */

/* The above assigns them the wrong way round to avoid conflict !!! */

char *zstrdup(char *str) {
	char *new=_strdup(str);
	if (new == NULL) {
		fprintf(stderr, "Out of memory for string %s\n", str);
		exit(1);
	}
	return new;
}

int needclone(char *glyph) {	/* predicate */
	int k;
	for (k=0; k < 32; k++) {
		if (strcmp(clonetable[k][0], "") == 0) return -1;
		if (strcmp(clonetable[k][0], glyph) == 0) return k;
	}
	return -1;
}

int glyphnumber(char *name) {
	int k;
	for (k = 0; k < chrs; k++) {
		if (strcmp(name, glyphnames[k]) == 0) return k;
	}
	return -1;
}

/* /Subrs 5 array */

/* 2 index /CharStrings 135 dict dup begin */

void maketables(FILE *input) {	/* table of where charstrings are in file */
	char *s=NULL;
	int k;
	long present;

	nsubrs = 0;
	while ((fgets(line, sizeof(line), input)) != NULL) {
		if ((s = strstr(line, "/Subrs")) != NULL) break;
	}
	if (s == NULL) {
		printf("ERROR: did not find /Subrs\n");
		return;
	}
	if (sscanf(s+7, "%d", &nsubrs) < 1) {
		printf("BAD: %s", line);
		exit(1);
	}
	if (verboseflag) printf(line);

	ncharstrings = 0;
	while ((fgets(line, sizeof(line), input)) != NULL) {
		if ((s = strstr(line, "/CharStrings")) != NULL) break;
	}
	if (sscanf(s+13, "%d", &ncharstrings) < 1) {
		printf("BAD: %s", line);
		exit(1);
	}
	if (verboseflag) printf(line);

	chrs = 0;
	for(;;) {
		present = ftell(input);
		while ((fgets(line, sizeof(line), input)) != NULL) {
			if (*line == '/') break;
			if (strstr(line, "definefont") != NULL) break;
			if (strstr(line, "currentfile") != NULL) break;
			present = ftell(input);
		}
		if (traceflag) printf(line);
		fflush(stdout);
		if (sscanf(line, "/%s", &str) < 1) {
			printf("EXIT %s", line);
			break;
		}
		glyphnames[chrs] = zstrdup(str);
		charstrings[chrs] = present;
		if (traceflag) printf("%d\t%ld\t%s\n", chrs, present, str);
		fflush(stdout);
		chrs++;
		if (chrs >= MAXCHARSTRINGS) {
			fprintf(stderr, "Too many CharStrings\n");
			exit(1);
		}
	}
	printf("Saw %d glyphs\n", chrs);
	fflush(stdout);
	for (k=0; k < chrs;k++) subrused[k]=-1;
}

/* foo bar chomp ... --- replace bar with new */

void replacetoken (char *start, char *new) {
	char *s, *t;
	int nnew, nold, nshift, ntail;
	
	if (verboseflag) printf("In `%s' replace with `%s'\n", start, new);
	s = start;
	while (*s > ' ') s++;	/* step up to white space */
	while (*s <= ' ' && *s != '\0') s++;	/* step over white space */
	t = s;
	while (*t > ' ') t++;	/* step up to white space */
	nold = t - s;			/* length of old string */
	nnew = strlen(new);		/* length of new string */
	nshift = nnew - nold;	/* how much to shift by */
	if (nshift != 0) {
		ntail = strlen(t+1);
		memcpy(t+1+nshift, t+1, ntail+1);
	}
	memcpy(s, new, nnew);
	if (verboseflag) printf("Result `%s'\n", start);
	fflush(stdout);
}

void updatensubr(FILE *output, int nsubrinc) {
	char *s;
	if ((s = strstr(line, "/Subrs")) != NULL) {
		sprintf(str, "%d", nsubrs + nsubrinc);
		replacetoken(line+6, str);
	}	
	fputs(line, output);
	if (verboseflag) printf("%s", line);
	fflush(stdout);
}

void copyuptosubrs(FILE *output, FILE *input) {
	char *s;
	while ((fgets(line, sizeof(line), input)) != NULL) {
		if ((s = strstr(line, "/Subrs")) != NULL) return;
		fputs(line, output);
		if (traceflag) printf("%s", line);
	}
	printf("Premature EOF\n");
	exit(1);
}

void copysubrs(FILE *output, FILE *input) {
	int nsub;
	for(;;) {
		while((fgets(line, sizeof(line), input)) != NULL) {
			fputs(line, output);
			if (strncmp(line, "dup ", 4) == 0) break;
		}
		if (sscanf(line+4, "%d", &nsub) < 1) {
			printf("BAD: %s", line);
		}
/*		fputs(line, output); */
		while((fgets(line, sizeof(line), input)) != NULL) {
/*		Note: ignore NP in Subr */
			if (strncmp(line, noaccessdef, 2) == 0) break; /* "ND" */
			fputs(line, output);
			if (*line < ' ') break;
			if (traceflag) printf("%s", line);
		}
		if (*line < ' ') ;
		else break;
	}
	if (verboseflag) printf("Finished with Subrs: %s", line);
}

/* following copies a CharString and creates a Subr from it */

void constructsubr(FILE *output, FILE *input, int nsub) {
	fprintf(output, "\n");
	fprintf(output, "dup %d %d %s\n", nsub, 0, readstring); /* RD */
	fgets(line, sizeof(line), input);	/* /glyphname line */
	if (*line != '/') {
		printf("BAD: %s", line);
		exit(1);
	}
	fgets(line, sizeof(line), input);	/* hsbw line */	
	if (strstr(line, "hsbw") == NULL) {	
		printf("BAD: %s", line);
		exit(1);
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (strncmp(line, "endchar", 7) == 0) break;
		if (strncmp(line, noaccessput, 2) == 0) break;	/* "NP" error */
		if (*line < ' ') break;			/* error */
		fputs(line, output);
	}
	fprintf(output, "return\n");
	fprintf(output, "%s\n", noaccessput);
}

void addnewsubrs(FILE *output, FILE *input) {
	int k, i=0;
	long present=ftell(input);
	for (k = 0; k < chrs; k++) {
		if (needclone(glyphnames[k]) >= 0) {
			printf("Making subr %d for %s ", nsubrs+i, glyphnames[k]);
			fseek(input, charstrings[k], SEEK_SET);
			constructsubr(output, input, nsubrs + i);
			subrused[k]= nsubrs + i;
			if (verboseflag) printf("%d %d\n", k, nsubrs + i);
			fflush(stdout);
			i++;
		}
	}
	fseek(input, present, SEEK_SET);
	fprintf(output, "%s\n", noaccessdef); /* ND */
}

void updatenchar(FILE *output, int ncharinc) {
	char *s;
	if ((s = strstr(line, "/CharStrings")) != NULL) {
		sprintf(str, "%d", ncharstrings + ncharinc);
		replacetoken(s+12, str);
	}	
	fputs(line, output);
	if (verboseflag) printf("%s", line);
	fflush(stdout);

}

void copyuptocharstrings(FILE *output, FILE *input) {
	char *s;
	if (verboseflag) printf("%s", line);
	while ((fgets(line, sizeof(line), input)) != NULL) {
		if ((s = strstr(line, "/CharStrings")) != NULL) return;
		fputs(line, output);
		if (verboseflag) printf("%s", line);
	}
	printf("Premature EOF\n");
	exit(1);
}

/* Turn original charstring into subr call style */

void suberize(FILE *output, FILE *input, int k) {
	fgets(line, sizeof(line), input);
	if (strstr(line, "hsbw") == NULL) {
		printf("BAD: %s", line);
		exit(1);
	}
	fputs(line, output);
	fprintf(output, "%d callsubr\n", k);
	while((fgets(line, sizeof(line), input)) != NULL) {
		if (strncmp(line, "endchar", 7) == 0) break;
		if (*line < ' ') break;
		if (strncmp(line, noaccessdef, 2) == 0) break;	/* ND */
	}
	fputs(line, output);
	fgets(line, sizeof(line), input);
	if (strncmp(line, noaccessdef, 2) != 0) {	/* ND */
		printf("BAD: %s", line);
		exit(1);
	}
	fputs(line, output);
}

void copychar(FILE *output, FILE *input) {
	while((fgets(line, sizeof(line), input)) != NULL) {
		if (*line < ' ') break;
		fputs(line, output);
		if (strncmp(line, noaccessdef, 2) == 0) break;	/* "ND" */
	}
	if (*line < ' ') fputs(line, output);
}

void copycharstrings(FILE *output, FILE *input) {
	int i, k;
	long present;
	for(;;) {
		while((fgets(line, sizeof(line), input)) != NULL) {
			if (*line == '/') break;
			fputs(line, output);
			if (strncmp(line, "end", 3) == 0) return;
			if (strstr(line, "definefont") != NULL)return;
			if (strstr(line, "currentfile") != NULL)return;
		}

		if (sscanf(line, "/%s", &str) < 1) {
			printf("BAD: %s", line);
			exit(1);
		}
		fputs(line, output);
		if ((k = needclone(str)) >= 0) {
			present = ftell(input);
			i = glyphnumber(str);
			if (i < 0) {
				printf("BAD: %s", line);
				exit(1);
			}
			if (verboseflag) printf("%d %s %d %s\n",
									k, clonetable[k][0],
									i, glyphnames[i]);
			fflush(stdout);
			if (subrused[i] < 0) {
				printf("Referring to impossible Subr %d for %d %s\n",
					  subrused[i], i, glyphnames[i]);
				exit(1);
			}
			suberize(output, input, subrused[i]);
			fseek(input, present, SEEK_SET);
			fprintf(output, "\n");
			fprintf(output, "/%s 0 %s\n", clonetable[k][1], readstring); /* RD */
			suberize(output, input, subrused[i]);
		}
		else copychar(output, input);
	}
}

void copytail(FILE *output, FILE *input) {
	while ((fgets(line, sizeof(line), input)) != NULL)
		fputs(line, output);
}

int countclones (void) {
	int k, total=0;
	for (k = 0; k < chrs; k++) {
		if (needclone(glyphnames[k]) >= 0) total++;
	}
	return total;
}

void processpln(FILE *output, FILE *input) {
	maketables(input);
	fflush(stdout);
	fseek(input, 0, SEEK_SET);			/* rewind */
	nsubrinc = nchrinc = countclones();
	printf("There are %d clonable charstrings\n", nsubrinc);
	fflush(stdout);
	copyuptosubrs(output, input);
	if (verboseflag) printf("Copied up to: %s", line);
	fflush(stdout);
	updatensubr(output, nsubrinc);
	copysubrs(output, input);
	addnewsubrs(output, input);
	copyuptocharstrings(output, input);
	if (verboseflag) printf("Copied up to: %s", line);
	fflush(stdout);
	updatenchar(output, nchrinc);
	copycharstrings(output, input);
	if (verboseflag) printf("Copied up to: %s", line);
	fflush(stdout);
	copytail(output, input);
}

/***************************************************************************/

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

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

/* return pointer to file name - minus path - returns pointer to filename */

char *extractfilename(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/************************************************************************/

int main(int argc, char *argv[]) {
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	FILE *input, *output;
	int m;
	if (cloneeuro) {
		clonetable[0][0]="uni20AC";
		clonetable[0][1]="Euro";
		clonetable[1][0]="";
		clonetable[1][1]="";
	}
	for (m = 1; m < argc; m++) {
		strcpy(infilename, argv[m]);
/*		if (strstr(infilename, ".pln") == NULL) continue; */
		extension(infilename, "pln");
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;
		}
		strcpy(outfilename, extractfilename(infilename));
		forceexten(outfilename, "sbr");
		printf("%s => %s\n", infilename, outfilename);
		fflush(stdout);
		if ((output = fopen(outfilename, "wb")) == NULL) {
			perror(outfilename);
			continue;
		}
		processpln(output, input);
		fclose(output);
		fclose(input);
	}
	return 0;
}


