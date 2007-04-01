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

/* scan AFM files for accent position offset */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 256
#define MAXNAME 128

#define MAXCHRS 384

char line[MAXLINE];

char str[MAXNAME];

int xll[MAXCHRS], yll[MAXCHRS], xur[MAXCHRS], yur[MAXCHRS];

int charcode[MAXCHRS];

double width[MAXCHRS];

char *roundbase[] = {
"a", "c", "e", "o", "s", "n", "r", "",
"g", ""
};

char *flatbase[] = {
"dotlessi", "u", "y", "z", "",
"w", ""
};

char *highaccent[] = {
	"tilde", "macron", "dieresis", "dotaccent", ""
};

char *lowaccent[] = {
	"grave", "acute", "hungarumlaut", "circumflex", "caron", "ring", "",
	"breve", ""
};

char *charname[MAXCHRS];

int roundbasecount, flatbasecount, highaccentcount, lowaccentcount;
int roundbasetotal, flatbasetotal, highaccenttotal, lowaccenttotal;
int dieresisyll, circumflexyll, oyur, eyur;
int minaccentyll, maxbaseyur;
int minyllk, maxyurk;
int totaldiff, odieresisdiff, ecircumflexdiff;
int totalcount, odieresiscount, ecircumflexcount;

int debugflag=0;

int quitearly=1;

/************************************************************************/

char *strdup(char *str) {
	char *new;
	new = (char *) malloc(strlen(str) + 1);
	return new == NULL ? new : strcpy(new, str);
}

int isinlist (char *name, char *lst[]) {
	int k;
	for (k = 0; k < 32; k++) {
		if (*lst[k] == '\0') break;
		if (strcmp(name, lst[k]) == 0) return 1;
	}
	return 0;
}

void analyzechar(char *name, int k) {
	if (isinlist(name, roundbase)) {
		roundbasetotal += yur[k];
		roundbasecount++;
		if (yur[k] > maxbaseyur) {
			maxbaseyur = yur[k];
			maxyurk = k;
		}
	}
	if (isinlist(name, flatbase)) {
		flatbasetotal += yur[k];
		flatbasecount++;
		if (yur[k] > maxbaseyur) {
			maxbaseyur = yur[k];
			maxyurk = k;
		}
	}
	if (isinlist(name, highaccent)) {
		highaccenttotal += yll[k];
		highaccentcount++;
		if (yll[k] < minaccentyll) {
			minaccentyll = yll[k];
			minyllk = k;
		}
	}
	if (isinlist(name, lowaccent)) {
		lowaccenttotal += yll[k];
		lowaccentcount++;
		if (yll[k] < minaccentyll) {
			minaccentyll = yll[k];
			minyllk = k;
		}
	}
	if (strcmp(name, "dieresis") == 0) dieresisyll = yll[k];
	if (strcmp(name, "circumflex") == 0) circumflexyll = yll[k];
	if (strcmp(name, "o") == 0) oyur = yur[k];
	if (strcmp(name, "e") == 0) eyur = yur[k];
}

void scanafm(FILE *input) {
/*	double baseyur, accentyll; */
	int baseyur, accentyll;
	int k=0;
	int alloccount=0;
	double pi = 3.141592653;
	
	roundbasecount=flatbasecount=highaccentcount=lowaccentcount=0;
	roundbasetotal=flatbasetotal=highaccenttotal=lowaccenttotal=0;
	dieresisyll = circumflexyll = oyur = eyur = 0;
	minaccentyll = 1000;
	maxbaseyur = 0;
	while (fgets(line, sizeof(line), input) != NULL) {
		if (strncmp(line, "StartCharMetrics", 16) == 0) break;
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (strncmp(line, "EndCharMetrics", 14) == 0) break;
		if (sscanf(line, "C %d ; WX %lg ; N %s ; B %d %d %d %d ;",
				   &charcode[k], &width[k], str,
				   &xll[k], &yll[k], &xur[k], &yur[k]) < 7) {
			printf("ERROR: %s", line);
			if (quitearly) return;
		}
		else {
			if (charname[k] != NULL) {
				printf("Dealloc %d %s\n", k, charname[k]);
				free(charname[k]);
			}
			charname[k] = _strdup(str); 
/*			charname[k] = strdup(str); */
/*			printf("\t\t\tAllocating %d bytes\n", strlen(str)+1); */
			alloccount += strlen(str)+1;
			if (charname[k] == NULL) {
				printf("ERROR: Unable to allocate memory\n");
				exit(1);
			}
			analyzechar(charname[k], k);
			k++;
			if (k >= MAXCHRS) {
				printf("No more space for characters\n");
				break;
			}
		}
	}
	if (debugflag) printf("\t\t\tAllocated %d bytes\n", alloccount);
	if (roundbasecount > 0)
		printf("roundbase  %d (%d)\t",
			   roundbasetotal / roundbasecount, roundbasecount);
	if (flatbasecount > 0)
		printf("flatbase   %d (%d)\t",
			   flatbasetotal / flatbasecount, flatbasecount);
	if (roundbasecount > 0 || flatbasecount > 0) putc('\n', stdout);
	if (highaccentcount > 0)
		printf("highaccent %d (%d)\t",
			   highaccenttotal / highaccentcount, highaccentcount);
	if (lowaccentcount > 0)
		printf("lowaccent  %d (%d)\t",
			   lowaccenttotal / lowaccentcount, lowaccentcount);
	if (highaccentcount > 0 || lowaccentcount > 0) putc('\n', stdout);
	if (roundbasecount + flatbasecount == 0) {
		printf("No base characters\n");
		return;
	}
	if (highaccentcount + lowaccentcount == 0) {
		printf("No accent characters\n");
		return;
	}
	baseyur = (roundbasetotal + flatbasetotal) /
			  (roundbasecount + flatbasecount);
	accentyll = (highaccenttotal + lowaccenttotal) /
				(highaccentcount + lowaccentcount);
	printf("averbase   %d (%d)\taveraccent %d (%d)\tdiff %d (aver)",
		   baseyur, (roundbasecount + flatbasecount),
		   accentyll, (highaccentcount + lowaccentcount),
		   accentyll - baseyur);
	if (accentyll > baseyur) {
		totaldiff += (accentyll - baseyur);
		totalcount++;
	}
	putc('\n', stdout);
	if (dieresisyll > 0 && oyur > 0)
		printf("o %d dieresis %d\tdiff %d",
			   oyur, dieresisyll, dieresisyll - oyur);
	if (dieresisyll > oyur) {
		odieresisdiff += (dieresisyll - oyur);
		odieresiscount++;
	}
	putc('\n', stdout);
	if (circumflexyll > 0 && eyur > 0)
		printf("e %d circumflex %d\tdiff %d",
			   eyur, circumflexyll, circumflexyll - eyur);
	if (circumflexyll > eyur) {
		ecircumflexdiff += (circumflexyll - eyur);
		ecircumflexcount++;
	}
	putc('\n', stdout);
	printf("%s %d %s %d\t\tdiff %d (min)\n",
		   charname[maxyurk], maxbaseyur,
		   charname[minyllk], minaccentyll,
		   minaccentyll - maxbaseyur);
	if (maxbaseyur >= minaccentyll)
		printf("ERROR: potential overlap %d >= %d (%d)\n",
			   maxbaseyur, minaccentyll, minaccentyll - maxbaseyur);
	putc('\n', stdout);
}

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

int main(int argc, char *argv[]) {
	char filename[FILENAME_MAX];
	FILE *input;
	int k, m, freecount;
	for (k = 0; k < MAXCHRS; k++) charname[k] = NULL;
	totalcount=odieresiscount=ecircumflexcount=0;
	totaldiff=odieresisdiff=ecircumflexdiff=0;
	for (m = 1; m < argc; m++) {
		strcpy(filename, argv[m]);
		extension(filename, "afm");
		if ((input = fopen(filename, "r")) == NULL) {
			perror(filename);
			exit(1);
		}
		printf("File %s\n", filename);
		scanafm(input);
		fclose(input);
		freecount = 0;
		for (k = 0; k < MAXCHRS; k++) {
			if (charname[k] != NULL) {
/*				printf("\t\t\tFreeing %d bytes\n",
					   strlen(charname[k])+1); */
				freecount += strlen(charname[k])+1;
				free(charname[k]);
				charname[k] = NULL;
			}
		}
		if (debugflag) printf("\t\t\tFreed %d bytes\n", freecount);
	}
	putc('\n', stdout);
	if (totalcount > 0 && odieresiscount > 0)
		printf("Average difference %d (%d) odieresis difference %d (%d)\n",
			   totaldiff / totalcount, totalcount,
			   odieresisdiff / odieresiscount, odieresiscount);
	if (totalcount > 0 && ecircumflexcount > 0)
		printf("Average difference %d (%d) ecircumflex difference %d (%d)\n",
			   totaldiff / totalcount, totalcount,
			   ecircumflexdiff / ecircumflexcount, ecircumflexcount);
	return 0;
}
