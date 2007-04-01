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

/* Code for splicing together curves that should be together */
/* That is, share equal and opposite segments */

/* Data items are segments: */
/* For each: xs, ys, xa, ya, xb, yb, xf, xf, code, used */
/* `code' is 'C', or 'L' (in which case xa, ya, xb, yb zero) or 'H' */
/* `used' is initially zero, but is set to 1 when segment used */
/* Equal and opposite segments are xed out with coe = `H' */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXFILENAME 128

#define MAXLINE 256

#define MAXPATH 512

#define MAXQUEUE 64

#define MAXNAME 32

char line[MAXLINE];		/* input line buffer */

int queue[MAXQUEUE];	/* pointers to paths that still need following */

int queinx;

int xs[MAXPATH], ys[MAXPATH];
int xa[MAXPATH], ya[MAXPATH];
int xb[MAXPATH], yb[MAXPATH];
int xf[MAXPATH], yf[MAXPATH];
int code[MAXPATH], used[MAXPATH];

int pathinx;

int chrs, width;

char charname[MAXNAME];

int xold, yold;

int verboseflag=0;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void addtoque(int pinx) {		/* add item to queue */
	if (queinx == MAXQUEUE) {
		fprintf(stderr, "Overflowed the queun");
		exit(3);
	}
	queue[queinx++]  = pinx;
}

void moveto(int axs, int ays) {		/* only sets up xold, yold */
	xold = axs; yold = ays;
}

void closepath(int start) {
	if (pathinx == MAXPATH) {
		fprintf(stderr, "Path too long");
		exit(3);
	}
	xs[pathinx] = start;		/* hide backward pointer ! */
	code[pathinx] = 'H'; used[pathinx] = 0;  pathinx++;
	addtoque(pathinx);			/* start of next subpath (if any) */
}

void lineto(int axf, int ayf) {
	if (pathinx == MAXPATH) {
		fprintf(stderr, "Path too long");
		exit(3);
	}
	xs[pathinx] = xold; ys[pathinx] = yold;
	xf[pathinx] = axf; yf[pathinx] = ayf;
	code[pathinx] = 'L'; used[pathinx] = 0; pathinx++;
	xold = axf; yold = ayf;
}

void curveto(int axa, int aya, int axb, int ayb, int axf, int ayf) {
	if (pathinx == MAXPATH) {
		fprintf(stderr, "Path too long");
		exit(3);
	}
	xs[pathinx] = xold; ys[pathinx] = yold;
	xa[pathinx] = axa; ya[pathinx] = aya;
	xb[pathinx] = axb; yb[pathinx] = ayb;
	xf[pathinx] = axf; yf[pathinx] = ayf;
	code[pathinx] = 'C'; used[pathinx] = 0; pathinx++;
	xold = axf; yold = ayf;
}

/* return -1 when nothing more to scan - hit end of file */
/* scan in outlines for next character */

int scaninchar(FILE *input) {
	int c;
	char *s;
	int startinx=0;
	int axs, ays, axa, aya, axb, ayb, axf, ayf;

	pathinx = 0;
	if (fgets(line, MAXLINE, input) == NULL) return -1;
	while (*line == '%' || *line == '\n') 
		if (fgets(line, MAXLINE, input) == NULL) return -1;
	if (*line != ']') {
		fprintf(stderr, "Not start of char %s", line);
		return -1;
	}
	(void) fgets(line, MAXLINE, input);
	if (sscanf(line, "%d %d", &chrs, &width) < 2) {
		fprintf(stderr, "Not char number and width %s", line);
	}
	if ((s = strchr(line, '%')) != NULL) sscanf(s+2, "%s", &charname);
	else strcpy(charname, "");
	queinx = 0;
	addtoque(0); 
/*	scan up to start of next character or end of file */
	while ((c = getc(input)) != ']') {
		(void) ungetc(c, input);    
		if (fgets(line, MAXLINE, input) == NULL) {	/* EOF */
			return -1;
		}
		if (strchr(line, 'h') != NULL) {
			closepath(startinx);	/* hide starting point into */
		}
		else if (strchr(line, 'm') != 0) {
			if (sscanf(line, "%d %d m", &axs, &ays) < 2) {
				fprintf(stderr, "Don't understand moveto %s", line);
				return -1;
			}
			else {
				startinx = pathinx;	/* remember starting point */
				moveto(axs, ays);
			}
		}
		else if (strchr(line, 'l') != 0) {
			if (sscanf(line, "%d %d l", &axf, &ayf) < 2) {
				fprintf(stderr, "Don't understand lineto %s", line);
				return -1;
			}
			else lineto(axf, ayf);
		}
		else if (strchr(line, 'c') != 0) {
			if (sscanf(line, "%d %d %d %d %d %d c", 
				&axa, &aya, &axb, &ayb, &axf, &ayf) < 6) {
			 fprintf(stderr, "Don't understand curveto %s", line);
			 return -1;
			}
			else curveto(axa, aya, axb, ayb, axf, ayf);
		}
		else {
			fprintf(stderr, "Don't understand %s", line);
			return -1;
		}
	}
	(void) ungetc(c, input);
	return 0;
}

int findmatch(int k) { /* try and find equal and opposite segement */
	int i;
	int axs, ays, axf, ayf;
	
	axs = xs[k]; ays = ys[k]; axf = xf[k]; ayf = yf[k];
	for (i = 0; i < pathinx; i++) {
		if (code[i] == 'H') continue;
		if (xf[i] == axs && yf[i] == ays &&
			xs[i] == axf && ys[i] == ayf) {
			if (verboseflag != 0) printf("Matched %d to %d ", i, k);
			else printf(".");
			return i;	/* found it */
		}
	}
	return -1;			/* nothing found */
}

void outlineto(FILE *output, int k) {
	if (xs[k] == xf[k] && ys[k] == yf[k])
		printf("Worthless lineto %d %d %d %d\n",
			xs[k], ys[k], xf[k], yf[k]); 
	else fprintf(output, "%d %d l\n", xf[k], yf[k]);
}

void outcurveto(FILE *output, int k) {
	if (xs[k] == xf[k] && ys[k] == yf[k])
		printf("Worthless curveto %d %d %d %d %d %d %d %d\n",
			xs[k], ys[k], xa[k], ya[k], xb[k], yb[k], xf[k], yf[k]); 
	else fprintf(output, "%d %d %d %d %d %d c\n", 
		xa[k], ya[k], xb[k], yb[k], xf[k], yf[k]);
}


/* start following path at startinx */

void followout(FILE *output, int startinx) {	
	int k;
	int xstart, ystart;
	int i, flag=0;

	k = startinx;
	if (used[k] != 0) return;	/* already used up - nothing to do */
	xstart = xs[k]; ystart = ys[k]; 
	for(;;) {
		if (code[k] == 'H') {
			k = xs[k];			/* loop back to start */
/*			if (flag != 0) 	fprintf(output, "h\n"); */
/*			return; */
		}
		if (used[k] != 0) {
			if (flag != 0) 	fprintf(output, "h\n"); 
			return; 
		}
		if ((i = findmatch(k)) < 0) {
			if (flag++ == 0) fprintf(output, "%d %d m\n", xs[k], ys[k]);
			if (used[k] != 0) fprintf(stderr, "Reusing segment\n");
			if (code[k] == 'L') outlineto(output, k);
			else if (code[k] == 'C') outcurveto(output, k);
			else {
				fprintf(stderr, "Invalid code %d\n", code[k]);
				return;
			}
			used[k++] = 1;
		}
		else {		/* found a match */
/*			code[k] = 'H';   */
/*			code[i] = 'H';   */
			used[k] = 1;			/* eliminate one of the two */
			addtoque(k+1);
			k = i+1;
/*			if (xs[k] == xstart && ys[k] == ystart) {	
				if (flag != 0) fprintf(output, "h\n");				
				return;
			} */
		}
	}
}

void scanoutchar(FILE *output) {
	int k=0;

	fprintf(output, "]\n");
	fprintf(output, "%d %d", chrs, width);
	if (strcmp(charname, "") != 0) fprintf(output, " %% %s", charname);
	putc('\n', output);
	while (k < queinx) {		/* while there is stuff to do ... */
		followout(output, queue[k]);	/* this may add stuff to queue */
		k++;
	}
}

void checkallused(void) {
	int k;
	
	for (k = 0; k < pathinx; k++) {
		if (code[k] == 'H') continue;
		if (used[k] == 0) fprintf(stderr, "Unused segment %d ", k);
	}
}

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

int main(int argc, char *argv[]) {
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	FILE *input, *output;
	int flag = 0;

	if (argc < 2) exit(3);

	printf("RESPLICE - program for splicing out opposite and parallel\n");

	strcpy(infilename, argv[1]);
	extension(infilename, "unw");

	strcpy(outfilename, argv[1]);
	forceexten(outfilename, "res");

	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename); exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename); exit(2);
	}	

/*	just copy first two lines (ptsize, scale and BBox) */
	fgets(line, MAXLINE, input);
	fputs(line, output);
	fgets(line, MAXLINE, input);
	fputs(line, output);	

	while (flag == 0) {
		flag = scaninchar(input);
		queinx--;					/* remove false start */
		if (verboseflag != 0) printf("\nchar %d ", chrs);
		else printf(" %d", chrs);
		scanoutchar(output);
		checkallused();
	}
	fclose(input);
	if(ferror(output) != 0) {
		perror(outfilename);
	}
	else fclose(output);

	putc('\n', stdout);

	return 0;
}
