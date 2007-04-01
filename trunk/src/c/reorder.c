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

#define MAXLINE 256

#define MAXLABEL 128

#define MAXPAGES 1024

#define MAXFILENAME 128

char line[MAXLINE];

char buffer[MAXLINE];

long pagestart[MAXPAGES];

long trailer=-1;

char *pagelabel[MAXPAGES];

int count1[MAXPAGES];

int count2[MAXPAGES];

int index[MAXPAGES];

int verboseflag=1;

int traceflag=1;

/****************************************************************************/

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

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/****************************************************************************/

/* scan the file building page table based on %%Page: DSC comments */

void buildpagetable (FILE *input) {
	long current;
	int pageno;
	char label[MAXLABEL];
	int ca, cb;
	
	trailer=-1;
	current = ftell(input);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == 12) strcpy(line, line+1);
		if (strncmp(line, "%%Trailer", 9) == 0 ||
			strncmp(line, "%%EOF", 5) == 0) {
			trailer = current;
			break;
		}
		if (strncmp(line, "%%Page:", 7) == 0) {
			if (sscanf(line+2, "Page: %s %d", label, &pageno) == 2) {
				if (pageno > 0 && pageno < MAXPAGES) {
					pagestart[pageno] = current;
					pagelabel[pageno] = _strdup(label);
					if (sscanf(label, "%d-%d", &ca, &cb) == 2) {
						count1[pageno] = ca;
						count2[pageno] = cb;
					}
				}
			}
		}
		current = ftell(input);
	}
	if (trailer < 0) trailer = current;
}

void showpagetable(void) {
	int k;
	for (k = 0; k < MAXPAGES; k++) {
		if (pagestart[k] >= 0) {
			printf("%d\t%ld\t%s\t", k, pagestart[k], pagelabel[k]);
			if (count1[k] >= 0 && count2[k] >= 0)
				printf("%d-%d", count1[k], count2[k]);
			putc('\n', stdout);
		}
	}
	if (trailer >= 0) printf("Trailer at %ld\n", trailer);
}

/* copy head of file up to first %%Page: DSC comment */

void copyheader (FILE *output, FILE *input) {
	long current;
	current = ftell(input);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == 12) {
			putc(12, output);
			current++;
			strcpy(line, line+1);
		}
		if (strncmp(line, "%%Page:", 7) == 0) break;
		if (strncmp(line, "%%Trailer", 9) == 0) break;
		if (strncmp(line, "%%EOF", 5) == 0) break;
		fputs(line, output);
		current = ftell(input);
	}
	fseek(input, current, SEEK_SET);
}

/* copy end of file */

void copytrailer (FILE *output, FILE *input) {
	while (fgets(line, sizeof(line), input) != NULL) {
		fputs(line, output);
	}
}

/* copy page with specified page number */

int copypageno (FILE *output, FILE *input, int pageno) {
	long start;

	if (verboseflag) printf("%d ", pageno);
	start = pagestart[pageno];
	if (start < 0) return -1;
	if (fseek (input, start, SEEK_SET) < 0) return -1;
	fgets(line, sizeof(line), input);	/* copy first line no matter what */
	if (strncmp(line, "%%Page:", 7) != 0) return -1;
	fputs(line, output);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == 12) {
			putc(12, output);
			strcpy(line, line+1);
		}
		if (strncmp(line, "%%Page:", 7) == 0) break;
		if (strncmp(line, "%%Trailer", 9) == 0) break;
		if (strncmp(line, "%%EOF", 5) == 0) break;
		fputs(line, output);
	}
	return 0;
}

/* copy page with specified page number */

int copypagelabel (FILE *output, FILE *input, char *label) {
	int k;

	if (verboseflag) printf("%s ", label);
	for (k = 0; k < MAXPAGES; k++) {
		if (pagelabel[k] == NULL) continue;
		if (strcmp(pagelabel[k], label) == 0) break;
	}
	if (k >= MAXPAGES) return -1;
	return copypageno(output, input, k);
}

/* sort pages based on first argument of %%Page: assumed n-m form */

void resortpages (void) {
	int i, j;
	int pi, pj;
	int swaps=0;

	for (i = 0; i < MAXPAGES-1; i++) {
		pi =  index[i];
		if (pi < 0) continue;
		for (j = i+1; j < MAXPAGES; j++) {
			pj =  index[j];
			if (pj < 0) continue;
			if ((count1[pi] > count1[pj]) ||
				((count1[pi] == count1[pj]) &&
					(count2[pi] > count1[pj]))) {
				index[j] = pi;
				index[i] = pj;
				pi = index[i];
				pj = index[j];
				swaps++;
			}					
		}
	}
	if (traceflag) printf("%d swaps\n", swaps);
}

/* print pages in order as sorted on page label */

int dosortedpages (FILE *output, FILE *input) {
	int k, pageno;
	long start;

	if (verboseflag) printf("Doing sorted pages\n");
	for (k = 0; k < MAXPAGES; k++) {
		pageno = index[k];
		start = pagestart[pageno];
		if (start < 0) continue;
		if (copypageno(output, input, pageno) < 0) return -1;
	}
	return 0;
}

/* format of page specification file: */
/* 1, 3, 5, 2, 4, 6, 13-19, 53-59 */
/* can be spread over multiple lines if convenient */

void dospecifiedpages(FILE *output, FILE *input, FILE *pages) {
	char *s;
	int pagestart, pageend, k, n;

	if (verboseflag) printf("Doing specified pages\n");
	while (fgets(buffer, sizeof(buffer), pages) != NULL) {
		s = buffer;
		if (traceflag) printf("SPEC: %s", buffer);
		while (sscanf(s, "%d%n", &pagestart, &n) > 0) {
			if (sscanf(s, "%d-%d%n", &pagestart, &pageend, &n) == 2) {
				if (pageend >= pagestart) {
					for (k = pagestart; k <= pageend; k++)
						copypageno(output, input, k);
				}
				else {
					for (k = pagestart; k >= pageend; k--)
						copypageno(output, input, k);
				}
			}
			else copypageno(output, input, pagestart);
			s += n;
			while (*s == ',' || *s == ' ' || *s == 9) s++;
			while (*s != '\0' && (*s < '0' || *s > '9') &&
				*s != '+' && *s != '-') s++;
		}
	}
}

/****************************************************************************/

/* first arg is name of input PS file */
/* second arg if any os name of file with page order requested */

int main (int argc, char *argv[]) {
	int k;
	FILE *input=NULL, *output=NULL, *pages=NULL;
	char infile[MAXFILENAME], outfile[MAXFILENAME], pagefile[MAXFILENAME];
	int firstarg=1;

	for (k = 0; k < MAXPAGES; k++) pagestart[k] = -1;
	for (k = 0; k < MAXPAGES; k++) pagelabel[k] = NULL;
	for (k = 0; k < MAXPAGES; k++) count1[k] = -1;
	for (k = 0; k < MAXPAGES; k++) count2[k] = -1;
	for (k = 0; k < MAXPAGES; k++) index[k] = k;

	if (argc < 2) exit(1);
	if (argc < firstarg+1) exit(1);
	if (argc < firstarg+2) ;
	else {
		strcpy(pagefile, argv[firstarg+1]);
		if ((pages = fopen(pagefile, "rb")) == NULL) {
			perror(pagefile);
			exit(1);
		}
	}
	strcpy(infile, argv[firstarg]);
	extension(infile, "ps");
	if ((input = fopen(infile, "rb")) == NULL) {
		perror(infile);
		exit(1);
	}
	buildpagetable(input);
	if (traceflag) showpagetable();
	resortpages();
	rewind(input);
	strcpy(outfile, removepath(infile));
	forceexten(outfile, "pso");
	if (strcmp(infile, outfile) == 0) {
		printf("In file same as out file\n");
		exit(1);
	}
	if ((output = fopen(outfile, "wb")) == NULL) {
		perror(outfile);
		exit(1);
	}	
	copyheader(output, input);
	if (pages == NULL) {
		dosortedpages(output, input);
	}
	else {
		dospecifiedpages(output, input, pages);
	}
	fseek(input, trailer, SEEK_SET);
	copytrailer(output, input);
	fclose(output);
	fclose(input);
	for (k = 0; k < MAXPAGES; k++)
		if (pagelabel[k] != NULL) free(pagelabel[k]);
	return 0;
}
