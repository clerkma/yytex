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

/* Program to get statistics of stems from character level hint files 
 * - needed for making up font level hints */

#include <stdio.h>
#include <conio.h>	
#include <string.h>
#include <stdlib.h> 
#include <errno.h>

/* #define FNAMELEN 80 */
#define MAXLINE 256

#define MAXCHRS 256

#define VERSION "1.0"				/* version of this program */

/* overall CM BBox: -106 -2959 1482 826 */

#define MAXWIDTH (1482 + 106 + 1) /* 1600 */
#define MAXHEIGHT (826 + 2959 + 1) /* 3800 */

int xoff = -106;
int yoff = -2959;

#define CURRENTDIRECT 1	/* non-zero => output goes in current directory */
#define TOPCOUNT 14		/* show this many of the most frequent ones */

int debugflag=0;	/* lots of tracing output */
int allflag=1;		/* do all 128 characters */
int upperflag=1;	/* do upper case letters */
int lowerflag=1;	/* do lower case letters */
int digitflag=1;	/* do digits */
int vstemposflag=0; 	/* show vstem positions */
int verboseflag = 1;

int extenflag = 0;	/* non-zero => next arg is extension */

char *ext = "sta";	/* extension to use for output file */

static char fontname[MAXLINE];

static int vlefts[MAXWIDTH], vrights[MAXWIDTH];
static int hbottoms[MAXHEIGHT], htops[MAXHEIGHT];
static int vwidth[MAXWIDTH];
static int hheight[MAXHEIGHT];

int getline (FILE *fp_in, char *line)	/* read a line up to newline */
{
	if (fgets(line, MAXLINE, fp_in) == NULL) return EOF;
	return strlen(line); 
}

#ifdef IGNORE
void extension(char *fname, char *ext)  /* supply extension if none present */
{
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) /* change extension if one present */
{
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
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

void resetstatistics(void) {
	int i;
	for (i = 0; i < MAXWIDTH; i++) {
		vlefts[i] = 0; 	vrights[i] = 0; vwidth[i] = 0;
	}
	for (i = 0; i < MAXHEIGHT; i++) {
		hbottoms[i] = 0; 	htops[i] = 0; hheight[i] = 0;
	}

}	

void showtopfew(FILE *fp_out, int arr[], int n, int ns, int off) {
	int maxv, maxi;
	int i, j, k;
	k = 0;
	for (i = 0; i < ns; i++) {
		if (k % 7 == 0) fprintf(fp_out, "\n");
		maxv=arr[0], maxi=0;
		for (j = 0; j < n; j++) {
			if (arr[j] > maxv) { maxv = arr[j]; maxi = j; }
		}
		fprintf(fp_out, "%5d(%3d)", maxi + off, maxv);
		arr[maxi] = 0;
		k++;
	}
}
				
void shownonzero(FILE *fp_out, int arr[], int n, int off) {
	int i, k;
	k = 0;
	for (i = 0; i < n; i++) {
		if (arr[i] != 0)  {
			if (k % 7 == 0) fprintf(fp_out, "\n");
			fprintf(fp_out, "%5d(%3d)", i+off, arr[i]);
			k++;
		}
	}
}

void showstatistics(FILE *fp_out) {

	if (allflag != 0) fprintf(fp_out, "All letters ");
	else {
		if (upperflag != 0) fprintf(fp_out, "upper case letters, ");
		if (lowerflag != 0) fprintf(fp_out, "lower case letters, ");
		if (digitflag != 0) fprintf(fp_out, "digits, ");
	}
	fprintf(fp_out,	"in %s\n", fontname);
	fprintf(fp_out, "\nHstem height:");
	shownonzero(fp_out, hheight, MAXHEIGHT, 0);
	fprintf(fp_out, "\nHstem height (ordered on frequency):");
	showtopfew(fp_out, hheight, MAXHEIGHT, TOPCOUNT, 0);
	fprintf(fp_out, "\nHstem bottom:");
	shownonzero(fp_out, hbottoms, MAXHEIGHT, yoff);
	fprintf(fp_out, "\nHstem bottom (ordered on frequency):");
	showtopfew(fp_out, hbottoms, MAXHEIGHT, TOPCOUNT, yoff);
	fprintf(fp_out, "\nHstem top:");
	shownonzero(fp_out, htops, MAXHEIGHT, yoff);
	fprintf(fp_out, "\nHstem top (ordered on frequency):");
	showtopfew(fp_out, htops, MAXHEIGHT, TOPCOUNT, yoff);
	fprintf(fp_out, "\nVstem width:");
	shownonzero(fp_out, vwidth, MAXWIDTH, 0);
	fprintf(fp_out, "\nVstem width (ordered on frequency):");
	showtopfew(fp_out, vwidth, MAXWIDTH, TOPCOUNT, 0);
	if (vstemposflag != 0) {
		fprintf(fp_out, "\nVstem left:");
		shownonzero(fp_out, vlefts, MAXWIDTH, xoff);
		fprintf(fp_out, "\nVstem right:");
		shownonzero(fp_out, vrights, MAXWIDTH, xoff);
	}
}	

void showusage (char *s) {
	fprintf (stderr, "Correct usage is:\n");
	fprintf (stderr, "%s [-{v}{a}{A}{0}] [-e <ext>] <file-1> <file-2>\n", 
		s);
	fprintf (stderr, "\n");
	fprintf (stderr, 
		"\te: next arg is extension for output (default '%s')\n", ext);
	fprintf (stderr, "\ta: include lower case letters\n");
	fprintf (stderr, "\tA: include upper case letters\n");
	fprintf (stderr, "\t0: include digits\n");
	fprintf (stderr, "\t   (default is to include all %d characters)\n",
		MAXCHRS);
	fprintf (stderr, "\tv: show vstem positions\n");
	exit(1);

}

void decodeflag (int c) {
/*	printf ("FLAG: %c%n", c); */
	switch(c) { 
		case 'v': vstemposflag = 1; break;
		case 'e': extenflag = 1; break;
		case 'a': {
			if (allflag != 0) {
				allflag = 0; upperflag = 0; lowerflag = 0; digitflag = 0;
			}
			lowerflag = 1; break;
		}
		case 'A': {
			if (allflag != 0) {
				allflag = 0; upperflag = 0; lowerflag = 0; digitflag = 0;
			}
			upperflag = 1; break;
		}
		case '0': {
			if (allflag != 0) {
				allflag = 0; upperflag = 0; lowerflag = 0; digitflag = 0;
			}
			digitflag = 1; break;
		}			
		default: {
				fprintf(stderr, "\nInvalid command line flag '%c'", c);
				exit(7);
		}
	}
}

int main (int argc, char *argv[]) {
	FILE *fp_in, *fp_out;
	static char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	static char line[MAXLINE];
	int vleft, vright, hbottom, htop, junk;
	char *s;
	int n, m, c, chrs;
	unsigned int i;
	int firstarg=1;

/*	while (argv[firstarg][0] == '-') */ /* check for command flags */
	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
/*		printf("LENGTH = %d\n", strlen(argv[firstarg]) ); */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			decodeflag(c);
		}
		firstarg++;
		if (extenflag != 0) {
			ext = argv[firstarg]; firstarg++; extenflag = 0;
		}
	}

	if (firstarg >= argc) showusage(argv[0]);

	printf( "Stem hinting file analysis program version %s\n",	VERSION);

	for (m = firstarg; m < argc; m++) {

		strcpy(fn_in, argv[m]);
		extension(fn_in, "stm");
		strcpy(fontname, argv[m]); 

		if (verboseflag != 0) printf("Processing stem file %s\n", fontname);

#if CURRENTDIRECT != 0
		if ((s=strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in;
		strcpy(fn_out, s);  	/* copy input file name minus path */
#else
		strcpy(fn_out, fn_in);  /* copy input file name */
#endif
		forceexten(fn_out, ext);	/* change extension */

/* watch for name conflict in = out ? */
	
		if ((fp_in = fopen(fn_in, "r")) == NULL) {
			fprintf(stderr, "\n"); perror(fn_in); exit(2);
		}

		if ((fp_out = fopen(fn_out, "w")) == NULL) {
			fprintf(stderr, "\n"); perror(fn_out); exit(2);
		}

		resetstatistics();
		while ((n = getline(fp_in, line)) != EOF) {
			if (n >= MAXLINE-2) {
				fprintf(stderr, "Line too long: %s", line);	break;
			}
			if (debugflag != 0) {printf("\n%s\n", line); getch();}
			s = line;
			if (*s == '/' || *s == '%') break;
			if (debugflag != 0) printf("%c", *s);
			sscanf(s, "C %n", &n);
			s = s + n;
			if (debugflag != 0) printf("%c", *s);
			if(sscanf(s, "%d %n", &chrs, &n) < 1) {
				fprintf(stderr, "Couldn't find char number in line %s", line);
				break;
			}
			s = s + n;
			if (debugflag != 0) printf("%c", *s);
			sscanf(s, "; %n", &n);
			s = s + n;
			if (debugflag != 0) printf("%c", *s);

			if (allflag != 0 || 
				(upperflag != 0 && chrs >= 'A' && chrs <= 'Z') ||
				(lowerflag != 0 && chrs >= 'a' && chrs <= 'z') ||
				(digitflag != 0 && chrs >= '0' && chrs <= '9')) {

				while (*s != '\0') {
					if (*s == 'H') {
						sscanf(s, "H %n", &n);
						s = s + n;
						if (debugflag != 0) printf("%c", *s);
						while (sscanf(s, "%d %d %n", &hbottom, &htop, &n) == 2)  {
							hbottoms[hbottom-yoff]++; htops[htop-yoff]++;
							hheight[htop-hbottom]++;
							s = s + n;
							if (debugflag != 0) printf("%c", *s);
						}
						sscanf(s, "; %n", &n);
						s = s + n;
						if (debugflag != 0) printf("%c", *s);
					}
					else if (*s == 'V') {
						sscanf(s, "V %n", &n);
						s = s + n;
						if (debugflag != 0) printf("%c", *s);
						while (sscanf(s, "%d %d %n", &vleft, &vright, &n) == 2)  {
							vlefts[vleft-xoff]++; vrights[vright-xoff]++;
							vwidth[vright-vleft]++;
							s = s + n;
							if (debugflag != 0) printf("%c", *s);
						}
						sscanf(s, "; %n", &n);
						s = s + n;
						if (debugflag != 0) printf("%c", *s);
					}	
					else { /* unrecognized code */
						sscanf(s, "%c %n", &c, &n);
						s = s + n;
						if (debugflag != 0) printf("%c", *s);
						while (sscanf(s, "%d %n", &junk, &n) == 1)  {
							s = s + n;
							if (debugflag != 0) printf("%c", *s);
							}				
							sscanf(s, "; %n", &n);
							s = s + n;
							if (debugflag != 0) printf("%c", *s);
					}
				}
			}
		}
		showstatistics(fp_out);

		fclose(fp_in);
		fclose(fp_out);
	}
	if (argc > firstarg + 1) 
		if (verboseflag != 0) printf("\nProcessed %d files", 
			argc - firstarg);

	return 0;
}	

/* buggy respone when fed a file with font-level hints */
