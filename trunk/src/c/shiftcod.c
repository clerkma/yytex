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

/* shiftcod.c --- Utility for adjusting character codes in OUT, AFM, HNT */

/* To change OUT, AFM, and HNT files for foo, use shiftcod c:\...\foo.* */

/* Modified files appear in current directory */

/* Does NOT resort entries in HNT file ... */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int low=64, high=127;		/* character code range of interest */
int shift=128;				/* amount to change code by */

#define MAXLINE 128

#define MAXFILENAME 80

char line[MAXLINE], back[MAXLINE];

int map[256];				/* where the character codes get mapped to */

int hit[256];				/* what codes have been output */

int verboseflag = 1;
int traceflag = 1;
int writeback = 0;			/* write back over original files */

char *filename (char *fullname) {	/* pointer to file name minus dirs */
	char *s;
	if ((s = strrchr(fullname, '\\')) != NULL) return s+1;
	if ((s = strrchr(fullname, '/')) != NULL) return s+1;
	if ((s = strrchr(fullname, ':')) != NULL) return s+1;
	else return fullname;
}

int processout (FILE *output, FILE *input) {
/*	int c; */
	int count=0, k, kmap, n;
	char *s;

	for (k=0; k < 256; k++) hit[k] = 0;

	for(;;) {
/* look for character start */
		while ((s = fgets (line, MAXLINE, input)) != NULL) {
			if (*line == ']') break;
			fputs(line, output);
		}
		if (s == NULL) break;		/* EOF */
		fputs(line, output);
/* look for character code and width */
		while ((s = fgets (line, MAXLINE, input)) != NULL) {
			if (*line != '%') break;
			fputs(line, output);
		}
		if (s == NULL) break;		/* EOF */
/* read character code */
		if (sscanf (line, "%d%n", &k, &n) < 1) {
			fprintf(stderr, "Say what: %s", line);
			return 0;
		}
/* change code if new code needed */
		kmap = map[k];
		if (k != kmap) {
			strcpy(back, line+n);
			sprintf(line, "%d", kmap);
			strcat(line, back);
			count++;
		}
		if (hit[kmap]++ > 0)
			fprintf(stderr, "ERROR: %d output more than once\n", kmap);
		fputs(line, output);
	}
	if (verboseflag != 0) printf("%d char codes were changed\n", count);
	return count;	/* EOF */
}

int processafm (FILE *output, FILE *input) {
/*	int c; */
	int count=0, k, kmap, n;
	char *s;

	for (k=0; k < 256; k++) hit[k] = 0;
	
	for(;;) {
/* look for character metrics line */
		while ((s = fgets (line, MAXLINE, input)) != NULL) {
			if (*line == 'C' && *(line+1) == ' ') break;
			fputs(line, output);
		}
		if (s == NULL) break;		/* EOF */
/* read character code */
		if (sscanf (line, "C %d%n", &k, &n) < 1) {
			fprintf(stderr, "Say what: %s\n", line);
			return 0;
		}
/* change character code if new code needed */
		kmap = map[k];
		if (k != kmap) {
			strcpy(back, line+n);
			sprintf(line, "C %d", kmap);
			strcat(line, back);
			count++;
		}
		if (hit[kmap]++ > 0)
			fprintf(stderr, "ERROR: %d output more than once\n", kmap);
		fputs(line, output);
	}
	if (verboseflag != 0) printf("%d char codes were changed\n", count);
	return count; /* EOF */
}

int processhnt (FILE *output, FILE *input) {
/*	int c; */
	int count=0, k, kmap, n, m;
	char *s;

	for (k=0; k < 256; k++) hit[k] = 0;

	for(;;) {
/* Search for character level hints and replacement hints */
		while ((s = fgets (line, MAXLINE, input)) != NULL) {
			if (*line == 'C' && *(line+1) == ' ') break;
			if (*line == 'S' && *(line+1) == ' ') break;
			fputs(line, output);
		}
		if (s == NULL) break;		/* EOF */
		if (*line == 'C') {
/* read character code */
			if (sscanf (line, "C %d%n", &k, &n) < 1) {
				fprintf(stderr, "Say what: %s", line);
				return 0;
			}
/* change character code if needed */
			kmap = map[k];
			if (k != kmap) {
				strcpy(back, line+n);
				sprintf(line, "C %d", kmap);
				strcat(line, back);
				count++;
			}
			if (hit[kmap]++ > 0)
				fprintf(stderr, "ERROR: %d output more than once\n", kmap);
		}
		else {
/* read character code */
			if (sscanf (line, "S %d ; C %d%n", &m, &k, &n) < 1) {
				fprintf(stderr, "Say what: %s", line);
				return 0;
			}
/* change character code if needed */
			kmap = map[k];
			if (k != kmap) {
				strcpy(back, line+n);
				sprintf(line, "S %d ; C %d", m, kmap);
				strcat(line, back);
				count++;
			}
			if (hit[kmap]++ > 0)
				fprintf(stderr, "ERROR: %d output more than once\n", kmap);
		}
		fputs(line, output);
	}
	if (verboseflag != 0) printf("%d char codes were changed\n", count);
	return count;
}

void lowercase (char *s) {
	int c;
	while ((c = *s) != '\0') {
		if (c >= 'A' && c <= 'Z') *s = (char) (c + 'a' - 'A');
		*s++;
	}
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

/* -r=<rmap-file> */

int readmap (char *mapfile) {
	FILE *inmap;
	char mapname[MAXFILENAME];
	int count=0, from, to;

	strcpy (mapname, mapfile);
	extension(mapname, "map");
	if ((inmap = fopen(mapname, "r")) == NULL) {
		perror(mapname); return 0;
	}
	if (verboseflag) printf("Scanning %s --- ", mapname);

	while (fgets(line, MAXLINE, inmap) != NULL) {
		if (*line == '%' || *line == ';' || *line == '#') continue;
		if (sscanf(line, "%d %d", &from, &to) < 2) {
			fprintf(stderr, "What: %s\n", line);
			continue;
		}
		map [from] = to;
		count++;
	}
	fclose (inmap);
	if (verboseflag != 0) printf ("%d char codes will get remapped\n", count);
	return count;
}

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	char bakfilename[MAXFILENAME];
	int c, k, firstarg=1;
/*	int outfile=0, afmfile=0, hntfile=0; */
	char *s;

	for (k = 0; k < 256; k++) map[k] = k;				/* idendity mapping */

/*	for (k = low; k <= high; k++) map[k] = k + shift; */	/* shift a range */

	if (firstarg + 1 > argc) {
		fprintf(stderr, "Too few arguments\n");
		exit(1);
	}

/*	while ((c = argv[firstarg][0]) == '-') { */
	while (firstarg < argc && argv[firstarg][0]) == '-') {
		if (argv[firstarg][1] == 'w') writeback = ~writeback;
		else if (argv[firstarg][1] == 'r') {
			if (!readmap(&argv[firstarg][3])) exit(2);
		}
		else fprintf(stderr, "What is: %s\n", argv[firstarg]);
		firstarg++;
	}

	if (firstarg + 1 > argc) {
		fprintf(stderr, "Too few arguments\n");
		exit(1);
	}

	for (k = firstarg; k < argc; k++) {

		strcpy(infilename, argv[k]);
		lowercase(infilename);
	 
		if ((s = strchr(infilename, '.')) == NULL) {
			if (traceflag != 0) printf("Ignoring   %s\n", infilename);
			continue;
		}
		if (strcmp (s+1, "out") != 0 &&
				strcmp (s+1, "afm") != 0 &&
					strcmp (s+1, "hnt") != 0) {
			if (traceflag != 0) printf("Ignoring   %s\n", infilename);
			continue;
		}

		if (verboseflag != 0) printf("Processing %s --- ", infilename);

		if (writeback != 0) {
			strcpy(outfilename, infilename);
			strcpy(bakfilename, infilename);
			forceexten(bakfilename, "bak");
			(void) remove (bakfilename);
			(void) rename (infilename, bakfilename);
			if ((input = fopen(bakfilename, "r")) == NULL) { 
				perror(infilename);
				exit(1);
			}
		}
		else {
			strcpy(outfilename, filename(infilename)); 
			if ((input = fopen(infilename, "r")) == NULL) { 
				perror(infilename);
				exit(1);
			}
		}

/*		strcpy(outfilename, filename(infilename)); */
		if (strcmp(outfilename, infilename) == 0) {
			fprintf(stderr, "%s == %s\n", outfilename, infilename);
			exit(1);
		}
	
		if ((output = fopen(outfilename, "w")) == NULL) {
			perror(outfilename);
			exit(1);
		}

		if ((s = strchr(infilename, '.')) != NULL) {
			if (strcmp (s+1, "out") == 0) processout(output, input);
			else if (strcmp (s+1, "afm") == 0) processafm(output, input);
			else if (strcmp (s+1, "hnt") == 0) processhnt(output, input);
		}
	
		fclose(input);
		fclose(output);	
	}
	return 0;
}
