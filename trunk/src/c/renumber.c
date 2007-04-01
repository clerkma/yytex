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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* #define FNAMELEN 80 */
#define CHARNAME_MAX 64
#define MAXCHRS 256
#define MAXLINE 256

int sortflag=0;				/* sort on new numbers if on */
int verboseflag=0;			/* verbose mode */

int hntflag=0;				/* on if hinting file */
int afmflag=0;				/* on if AFM file */
int outflag=0;				/* on if outline file */
int pfaflag=0;				/* on if PFA file */

int numbers[MAXCHRS];

long charpos[MAXCHRS];

char line[MAXLINE];
char buffer[MAXLINE];

char *stripname(char *filename) {
	char *s;
	if ((s = strrchr(filename, '\\')) != NULL) return s+1;
	if ((s = strrchr(filename, '/')) != NULL) return s+1;
	if ((s = strrchr(filename, ':')) != NULL) return s+1;
	return filename;
}

void extension(char *filename, char *ext) {
	char *s, *t;
	t = stripname(filename);
	if ((s = strrchr(filename, '.')) != NULL && s > t) strcpy(s+1, ext);
	else {
		strcat(filename, ".");
		strcat(filename, ext);
	}
}

void renumberafm(FILE *output, FILE *input) {
	char *s;
	long current;
	int n, chr;

	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (strncmp(line, "StartCharMetrics", 16) == 0) break;
		fputs(line, output);
	}
	if (s == NULL) return;
	fputs(line, output);
	current = ftell(input);
	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (strncmp(line, "EndCharMetrics", 14) == 0) break;
		if (*line == '%' || *line == '\n') {
			fputs(line, output); continue;
		}
		if (sscanf(line, "C %d ;%n", &chr, &n) == 1) {
			if (chr >= 0 && chr < MAXCHRS) 
				if (numbers[chr] >= 0) {
					strcpy(buffer, line+n);
					chr = numbers[chr];
					sprintf(line, "C %d ;", chr);
					strcat(line, buffer);
				}
		}
		else fprintf(stderr, "Don't understand: %s (%ld byte)", line, current);
		fputs(line, output);
		current = ftell(input);
	}
	if (s == NULL) return;
	fputs(line, output);
	while ((s = fgets(line, MAXLINE, input)) != NULL) fputs(line, output);
}

/* fontone.c requires hints to be ordered --- so this reorders on new chr */

void sorthints(FILE *output, FILE *input) {
	long previous=-1;			/* beginning of line in file */
	long firstone=-1;			/* where char level hints actually start */
	char *s;
	int k, n, chr, chrnew, fsubr;
	long current;

/*	scan first for character hint start locations */
	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (*line == '%' || *line == '\n' || *line == '/') 	{
			previous = ftell(input);
			continue;
		}
		if (*line == 'S') {				/* redundant */
			previous = ftell(input);
			continue;
		}
		if (sscanf(line, "C %d ;%n", &chr, &n) == 1) {
			if (chr >= 0 && chr < MAXCHRS) {
				chrnew = numbers[chr];
				if (chrnew >= 0) {
					if (charpos[chrnew] < 0) charpos[chrnew] = previous;
					else fprintf(stderr, "Collision %d => %d\n", chr, chrnew);
				}
				else if (charpos[chr] < 0) charpos[chr] = previous;
				else fprintf(stderr, "Collision %d => %d\n", chr, chrnew);
				if (firstone < 0) firstone = previous;
			}
		}
		previous = ftell(input);
	}

/*	now copy font level hints */
	rewind(input);
	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		fputs(line, output);
		if (ftell (input) >= firstone) break;
	}

/*	now grab char level hints in order */
	for (k = 0; k < MAXCHRS; k++) {
		if (charpos[k] < 0) continue;
		fseek (input, charpos[k], SEEK_SET);
		fgets(line, MAXLINE, input);
/*		do first line for this character --- initial hints */
		if (sscanf(line, "C %d ;%n", &chr, &n) == 1) {
			if (chr >= 0 && chr < MAXCHRS) {
				if (numbers[chr] >= 0) chr = numbers[chr];
				if (chr != k) fprintf(stderr, "%d <> %d\n", chr, k);
				strcpy(buffer, line+n);
				sprintf(line, "C %d ;", chr);
				strcat(line, buffer);
			}
			else fprintf(stderr, "%s", line);
				fputs(line, output);
		}
		else fprintf(stderr, "%s", line);

/*		do rest for this character --- hint replacements */
		current = ftell(input);
		while ((s = fgets(line, MAXLINE, input)) != NULL) {
			if (*line == '%' || *line == ';' || *line != '\n') {
				fputs(line, output);		/* want this ??? */
				continue;
			}
			if (*line == 'C') break;		/* redundant - next character */
			if (sscanf(line, "S %d ; C %d ;%n", &fsubr, &chr, &n) == 2) {
				if (chr >= 0 && chr < MAXCHRS)
					if (numbers[chr] >= 0) { 
						strcpy(buffer, line+n);
						chr = numbers[chr];
						sprintf(line, "S %d ; C %d ;", fsubr, chr);
						strcat(line, buffer);
					}
			}
			else {
				fprintf(stderr, "Don't understand: %s (%ld byte)",
					line, current);
				break;
			}
			fputs(line, output);
			current = ftell(input);
		}
	}
}

/* NOTE: this does not reorder the darn hints !!! */

void renumberhnt(FILE *output, FILE *input) {
	char *s;
	int n, chr, fsubr;
	long current;

	if (sortflag) {
		sorthints(output, input);
		return;
	}

/*	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (*line != '/' && *line != '%' && *line != '\n') break;
		fputs(line, output);
	}
	if (s == NULL) return;
	fputs(line, output); */

	current = ftell(input);
	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (*line == '%' || *line == '\n' || *line == '/') {
			fputs(line, output); continue;
		}
		if (sscanf(line, "C %d ;%n", &chr, &n) == 1) {
			if (chr >= 0 && chr < MAXCHRS) 
				if (numbers[chr] >= 0) {
					strcpy(buffer, line+n);
					chr = numbers[chr];
					sprintf(line, "C %d ;", chr);
					strcat(line, buffer);
				}
		}
		else if (sscanf(line, "S %d ; C %d ;%n", &fsubr, &chr, &n) == 2) {
			if (chr >= 0 && chr < MAXCHRS)
				if (numbers[chr] >= 0) { 
					strcpy(buffer, line+n);
					chr = numbers[chr];
					sprintf(line, "S %d ; C %d ;", fsubr, chr);
					strcat(line, buffer);
				}
		}
		else fprintf(stderr, "Don't understand: %s (%ld byte)", line, current);
		fputs(line, output);
		current = ftell(input);
	}
}

void renumberout(FILE *output, FILE *input) {
	char *s;
	int n, chr, width;
	int newchar=0;
	long current;

	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (*line != '%' && *line != '\n') break;
		fputs(line, output);
	}
	if (s == NULL) return;
	fputs(line, output);

	current = ftell(input);
	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (*line == '%' || *line == '\n') {
			fputs(line, output); continue;
		}
		if (newchar) {
			if (sscanf(line, "%d %d %n", &chr, &width, &n) == 2) {
				if (chr >= 0 && chr < MAXCHRS)
				if (numbers[chr] >= 0) {
					strcpy(buffer, line+n);
					chr = numbers[chr];
					sprintf(line, "%d %d ", chr, width);
					strcat(line, buffer);
				}
			}
			else {
				fprintf(stderr, "Don't understand: %s (%ld byte)",
					line, current);
			}
			newchar = 0;
		}
		else if (*line == ']') newchar++;
		fputs(line, output);
		current = ftell(input);
	}
}

void renumberpfa(FILE *output, FILE *input) {
	char *s;
	int chr;
	int newchar=0;
	char charname[CHARNAME_MAX];

	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (strncmp(line, "/Encoding", 9) == 0) break;
		fputs(line, output);
	}
	if (s == NULL) return;
	fputs(line, output);

	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (strncmp(line, "def", 3) == 0) break;
		if (*line == '%' || *line == '\n') {
			fputs(line, output); continue;
		}
		if (sscanf(line, "dup %d /%s ", &chr, &charname) == 2) {
			if (chr >= 0 && chr < MAXCHRS) {
				if (numbers[chr] >= 0) chr = numbers[chr];
				sprintf(line, "dup %d /%s put\n", chr, charname);
			}
		}
		fputs(line, output);
	}
	if (s == NULL) return;
	fputs(line, output);
	while ((s = fgets(line, MAXLINE, input)) != NULL) fputs(line, output);
}

void readnumbers (FILE *input) {
	int from, to;
	char *s;

	while ((s = fgets(line, MAXLINE, input)) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (sscanf(line, "%d %d", &from, &to) == 2) {
			if (from >= 0 && from < MAXCHRS && to >= 0 && to < MAXCHRS)
				numbers[from] = to;
			else fprintf(stderr, "Out of range: %s", line);
				
		}
		else fprintf(stderr, "Don't understand: %s", line);
	}
}

int main(int argc, char *argv[]) {
	char infilename[FILENAME_MAX];
	char outfilename[FILENAME_MAX];
	char bakfilename[FILENAME_MAX];
	FILE *input, *output;
	int firstarg=1, k, m;

	if (argc < firstarg+1) {
		printf("renumber <RENUMBER list>  <OUT file>\n");
		return 0;
	}
/*	while (*argv[firstarg] == '-') */
	while (firstarg < argc && *argv[firstarg] == '-') { 
		if (strcmp(argv[firstarg], "-s") == 0) {
			sortflag++;
			firstarg++;
		}
		if (strcmp(argv[firstarg], "-v") == 0) {
			verboseflag++;
			firstarg++;
		}
		if (argc < firstarg+1) return 0;
	}

	for (k = 0; k < MAXCHRS; k++) numbers[k] = -1;

	if (verboseflag) printf("Now reading renumbering file\n");
	strcpy(infilename, argv[firstarg]);
	extension(infilename, "num");
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);
		exit(1);
	}
	readnumbers (input);
	fclose (input);
	firstarg++;
	if (argc < firstarg+1) return 0;

	if (verboseflag) printf("Now starting renumbering\n");
	for (m = firstarg; m < argc; m++) {
		hntflag=0;				/* on if hinting file */
		afmflag=0;				/* on if AFM file */
		outflag=0;				/* on if outline file */
		pfaflag=0;				/* on if PFA file */
		for (k = 0; k < MAXCHRS; k++) charpos[k] = -1;
		strcpy(infilename, argv[m]);
		if (verboseflag) printf("Trying %s (arg %d) ", infilename, m);
		if ((input = fopen(infilename, "r")) == NULL) {
			perror(infilename);
			continue;
		}
		strcpy(outfilename, stripname(infilename));
		if (verboseflag) printf("=> %s\n", outfilename);
		if (strcmp(outfilename, infilename) == 0) {
			fclose(input);
			strcpy(bakfilename, infilename);
			extension(bakfilename, "bak");
			if (verboseflag != 0) 
				printf("Renaming %s => %s\n", infilename, bakfilename);
			if (remove(bakfilename) == 0)
				printf("Deleted old %s\n", bakfilename);
			if (rename(infilename, bakfilename) != 0) {
				switch(errno) {
					case EACCES:
						printf("%s already exists or invalid path\n",
							bakfilename);
						break;
					case ENOENT:
						printf("%s not found\n", infilename);
						break;
					case EXDEV:
						printf("Attempt to move %s to %s\n",
							infilename, bakfilename);
						break;
					default:
						break;
				}
			}
			else printf("Renamed %s => %s\n", infilename, bakfilename);
			if ((input = fopen(bakfilename, "r")) == NULL) {
				perror(bakfilename);
				continue;
			}
		}		
		if ((output = fopen(outfilename, "w")) == NULL) {
			perror(outfilename);
			continue;
		}		
		if (strstr(outfilename, ".afm") != NULL ||
			strstr(outfilename, ".AFM") != NULL) afmflag = 1;
		if (strstr(outfilename, ".hnt") != NULL ||
			strstr(outfilename, ".HNT") != NULL) hntflag = 1;
		if (strstr(outfilename, ".out") != NULL ||
			strstr(outfilename, ".OUT") != NULL) outflag = 1;
		if (strstr(outfilename, ".pfa") != NULL ||
			strstr(outfilename, ".PFA") != NULL) pfaflag = 1;
		if (afmflag) renumberafm(output, input);
		if (hntflag) renumberhnt(output, input);
		if (outflag) renumberout(output, input);
		if (pfaflag) renumberpfa(output, input);
		fclose (output);
		fclose (input);
	}
	return 0;
}
