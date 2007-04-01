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

/* add extar characters to CM fonts */
/* lets start with ij and IJ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>			/* for tangent */

#define MAXFILENAME 128

#define MAXLINE 256

#define MAXCHRS 512

#define MAXCHARNAME 32

char line[MAXLINE];

char str[MAXLINE];

int chr;					/* character number */
int xll, yll, xur, yur;		/* character bounding box */
double width;				/* character width */
double suppresskern;		/* from KPX suppress l ... */
double quad;				/* em size */
double italicangle;			/* italic angle from AFM */
double slant;				/* tangent of italic angle */

double skew=0.0;			/* forced slanting from command line 96/Sep/2 */

int verboseflag = 1;
int traceflag = 0;
int commentflag = 1;

int doafmflag=1;
int dooutflag=1;
int dohntflag=1;

int importflag=0;			/* import character from another font */

int compositeflag=0;		/* add new composites */
int quotebaseflag=0;		/* do quotesinglbase and quotedblbase */

int dontmerge=0;			/* if we don't want suppress merged */

int threshvert=300;

int rulethickness=40;		/* 40 default rule thickness for regular */
							/* 60 default rule thickness for sans serif */
							/* 80 default rule thickness for bold */

int italicflag=0;
int boldflag=0;
int sansserif=0;
int fixedpitch=0;

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

int roundit (double x) {
	if (x > 0.0) return (int) (x + 0.5);
	else if (x < 0.0) return (int) (x - 0.5);
	else return 0;
}

/*****************************************************************************/

/* scan up to non-comment line that contains desired string */

char *scanto(FILE *input, char *match) {
	char *s=NULL;
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if ((s = strstr(line, match)) != NULL) return s;
	}
	return NULL;		/* hit EOF */
}

/* copy up to but not including line with matching string */
/* copy to end of file if match string is "" */

char *copyto(FILE *output, FILE *input, char *match) {
	char *s=NULL;
	if (traceflag) printf("COPYTO: %s\n", match);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (strcmp(match, "") == 0) {		/* nothing to match */
			fputs(line, output);			continue;
		}
		if (*line == '%' || *line == ';' || *line == '\n') {
			fputs(line, output);			continue;
		}
		if ((s = strstr(line, match)) != NULL) {
			if (traceflag) printf("FOUND: %s", line);
			return s;
		}
		fputs(line, output);
	}
	return NULL;		/* hit EOF */
}

int getbbox (char *fontname, char *charname) {
	char filename[MAXFILENAME];
	FILE *input;
	char *s;

	strcpy(filename, fontname);
	forceexten(filename, "afm");
	if ((input = fopen(filename, "r")) == NULL) {
		perror(filename);		exit(1);
	}
	sprintf(str, " N %s ; ", charname);
	if ((s = scanto (input, str)) == NULL) {
		printf("Can't find AFM line for %s\n", charname);
		exit(1);
	}
	if ((s = strstr(s, " ; B ")) == NULL) {
		printf("Can't find BBox info for %s\n", charname);
		exit(1);
	}
	if (sscanf(s+5, "%d %d %d %d", &xll, &yll, &xur, &yur) < 4) {
		printf("Unable to read BBox info for %s\n", charname);
		exit(1);
	}
	if (sscanf(line, "C %d ; WX %lg", &chr, &width) < 2) {
		printf("Unable to read width info for %s\n", charname);
		exit(1);
	}

	if (strcmp(charname, "l") == 0 ||
		strcmp(charname, "L") == 0) {
		if (scanto (input, "StartKernPairs") == NULL) {
			printf("Can't find %s\n", "StartKernPairs");
			exit(1);
		}
		sprintf(str, "KPX suppress %s ", charname);
		if (scanto (input, str) == NULL) {
			printf("Can't find %s\n", str); exit(1);
		}
		if (sscanf (line+15, "%lg", &suppresskern) < 1) {
			printf("Don't understand %s", line); exit(1);
		}
	}
	fclose (input);
	return 0;				/* success */
}

/* scan to numbered outline in shape file */

int scantooutline (FILE *input, int chr) {
	int nchr, width;
	int nline=0;
	char charname[MAXCHARNAME];
	while (fgets(line, sizeof(line), input) != NULL) {
		nline++;
/*		if (*line == '%' || *line == ';' || *line == '\n') continue; */
		if (*line != ']') continue;
		while (fgets(line, sizeof(line), input) != NULL) {
			nline++;
			if (*line == '%' || *line == ';' || *line == '\n') continue;
			break;
		}
		if (sscanf (line, "%d %d %% %s", &nchr, &width, &charname) < 2) {
			printf("Can't parse line %d %s", nline, line); exit(1);
		}
		if (nchr == chr) {
			if (traceflag)
				printf("Found char %d on line %d at byte %ld\n",
					   chr, nline, ftell(input));
			return chr;
		}
	}
	return -1;
}

/*****************************************************************************/

int intersect (int, int, int, int, int);
int intersectx (int, int, int, int, int, int, int, int);
int intersecty (int, int, int, int, int, int, int, int);

int xcoor[4], ycoor[4];

void rotateoutline (int xcoor[], int ycoor[], int n) {
	int x, y, k;
	x = xcoor[0]; y = ycoor[0];
	for (k = 0; k < n-1; k++) {
		xcoor[k] = xcoor[k+1];
		ycoor[k] = ycoor[k+1];
	}
	xcoor[n-1] = x;
	ycoor[n-1] = y;
}

int ordered (void) {
	int xd[4];
	int k;
	for (k = 0 ; k < 4; k++) xd[k] = xcoor[k] - (ycoor[k] / 8);
	if (xd[0] > xd[1] || xd[0] > xd[2] || xd[0] > xd[3]) return 0;
	else return 1;
}

/**************************************************************************/

int readsuppress (FILE *input, int offset) {
	int x, y, k;
	int xa, ya, xb, yb, xc, yc;

	if (scantooutline (input, 32) < 0) {
		printf("Can't find outline for suppress\n"); exit(1);
	}

/*	read the corners of the character */
	for (k = 0; k < 4; k++) {
		fgets(line, sizeof(line), input);
		if (sscanf(line, "%d %d %d %d %d %d ", &xa, &ya, &xb, &yb, &xc, &yc)
			== 6) {
			printf("Suppress is rounded - use -u command line flag\n");
			x = xc; y = yc;
		}
		else if (sscanf(line, "%d %d ", &x, &y) < 2) {
			printf("Can't parse %s", line);
		}
		xcoor[k] = x+offset;
		ycoor[k] = y;
	}
/*	now order the points */
	while (!ordered ()) rotateoutline (xcoor, ycoor, 4);
	return 0;
}

int makelslash (FILE *output, FILE *input, int nchr) {
	int xa, ya, xb, yb, xc, yc;
	int xlast, ylast;
	int first = 1, last = 0;
	int xfirst=0, yfirst=0;
	int xinter, yinter;
	int count = 0;

	if (scantooutline (input, nchr) < 0) {
		printf("Can't find outline for %d\n", nchr); exit(1);
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (*line == ']') 	break;
		if (*line == 'h' && count < 2) {
			sprintf(line, "%d %d l\n", xfirst, yfirst);
			last = 1;
		}
		if (sscanf(line, "%d %d %d %d %d %d c", &xa, &ya, &xb, &yb, &xc, &yc)
			== 6) {
			xlast = xc;
			ylast = yc;
		}
		else if (sscanf(line, "%d %d l", &xa, &ya) == 2) {
			if (first == 0) {
				if (ya < ylast - threshvert) {	/* found downstroke */
					if (traceflag) {
						printf("%d %d %d %d ", xa, ya, xlast, ylast);
					
						printf("%d %d %d %d ",
							   xcoor[0], ycoor[0], xcoor[3], ycoor[3]);
					}
					if (xa == xlast) {
						xinter = xa;
						yinter = intersect(xinter,
						   xcoor[0], ycoor[0], xcoor[3], ycoor[3]);
					}
					else {
						xinter = intersectx (xa, ya, xlast, ylast,
						   xcoor[0], ycoor[0], xcoor[3], ycoor[3]);
						yinter = intersecty (xa, ya, xlast, ylast,
						   xcoor[0], ycoor[0], xcoor[3], ycoor[3]);
					}
					if (traceflag) printf("%d %d\n", xinter, yinter);
					fprintf (output, "%d %d l\n", xinter, yinter);
					fprintf (output, "%d %d l\n", xcoor[0], ycoor[0]);
					fprintf (output, "%d %d l\n", xcoor[1], ycoor[1]);
					if (traceflag) {
						printf("%d %d %d %d ", xa, ya, xlast, ylast);
						printf("%d %d %d %d ",
							   xcoor[1], ycoor[1], xcoor[2], ycoor[2]);
					}
					if (xa == xlast) {
						xinter = xa;
						yinter = intersect(xinter,
						   xcoor[1], ycoor[1], xcoor[2], ycoor[2]);
					}
					else {
						xinter = intersectx (xa, ya, xlast, ylast,
						   xcoor[1], ycoor[1], xcoor[2], ycoor[2]);
						yinter = intersecty (xa, ya, xlast, ylast,
						   xcoor[1], ycoor[1], xcoor[2], ycoor[2]);
					}
					if (traceflag) printf("%d %d\n", xinter, yinter);
					fprintf (output, "%d %d l\n", xinter, yinter);
					count++;
				}
				else if (ya > ylast + threshvert) { /* found upstroke */
					if (traceflag) {
						printf("%d %d %d %d ", xa, ya, xlast, ylast);
						printf("%d %d %d %d ",
							   xcoor[1], ycoor[1], xcoor[2], ycoor[2]);
					}
					if (xa == xlast) {
						xinter = xa;
						yinter = intersect(xinter,
						   xcoor[1], ycoor[1], xcoor[2], ycoor[2]);
					}
					else {
						xinter = intersectx (xa, ya, xlast, ylast,
						   xcoor[1], ycoor[1], xcoor[2], ycoor[2]);
						yinter = intersecty (xa, ya, xlast, ylast,
						   xcoor[1], ycoor[1], xcoor[2], ycoor[2]);
					}
					if (traceflag) printf("%d %d\n", xinter, yinter);
					fprintf (output, "%d %d l\n", xinter, yinter);
					fprintf (output, "%d %d l\n", xcoor[2], ycoor[2]);
					fprintf (output, "%d %d l\n", xcoor[3], ycoor[3]);
					if (traceflag) {
						printf("%d %d %d %d ", xa, ya, xlast, ylast);
						printf("%d %d %d %d ",
							   xcoor[0], ycoor[0], xcoor[3], ycoor[3]);
					}
					if (xa == xlast) {
						xinter = xa;
						yinter = intersect(xinter,
						   xcoor[0], ycoor[0], xcoor[3], ycoor[3]);
					}
					else {
						xinter = intersectx (xa, ya, xlast, ylast,
						   xcoor[0], ycoor[0], xcoor[3], ycoor[3]);
						yinter = intersecty (xa, ya, xlast, ylast,
						   xcoor[0], ycoor[0], xcoor[3], ycoor[3]);
					}
					if (traceflag) printf("%d %d\n", xinter, yinter);
					fprintf (output, "%d %d l\n", xinter, yinter);
					count++;
				}
			}
			else {
				xfirst = xa, yfirst = ya;
				first = 0;
			}
			xlast = xa; ylast = ya;
		}
		fputs(line, output);
		if (last) {
			fputs("h\n", output);
			last = 0;
		}
	}
	if (count < 2) {
		printf("Incomplete lslash intersection\n");
	}
	return 0;
}

/*****************************************************************************/

int addlinetoafm (char *fontname, char *source, char *before, int incr) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	char str[MAXLINE];
	int nchrs;
	FILE *input, *output;

	if (traceflag) 
		printf("FONT: %s BEFORE: %s INCR: %d\n", fontname, before, incr);
	strcpy(str, source);		/* save the source */
	strcpy(infilename, fontname);
	forceexten(infilename, "bak");
	strcpy(outfilename, fontname);
	forceexten(outfilename, "afm");
	(void) remove(infilename);
	rename(outfilename, infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);		exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);		exit(1);
	}
/*	If we need to increment StartCharMetrics count do this first */
	if (incr > 0) {
		if (copyto (output, input, "StartCharMetrics") == NULL) {
			printf("Can't find %s\n", "StartCharMetrics"); exit(1);
		}
		if (traceflag) printf("OLD STARTCHARMETRICS: %s", line);
		if (sscanf (line + 17, "%d", &nchrs) < 1) {
			printf("Can't parse %s", line); exit(1);
		}
		sprintf(line, "%s %d\n", "StartCharMetrics", nchrs+incr);
		if (traceflag) printf("NEW STARTCHARMETRICS: %s", line);
		fputs(line, output);
	}
	if (copyto (output, input, before) == NULL) {
		printf("Can't find %s\n", before); exit(1);
	}
/*	stick new line in just before EndCharMetrics */
	if (traceflag) printf("NEW LINE: %s", str);
	fputs(str, output);
/*	output EndCharMetrics line */
	if (traceflag) printf("ENDCHARMETRICS: %s", line);
	fputs(line, output);
/*	copy to end of file */
	if (copyto (output, input, "") != NULL) {
		printf("Can't find end of file\n"); exit(1);
	}
	fclose(output);
	fclose(input);
	return 0;
}

int	copyoutline (FILE *output, FILE *input, int xoffset, int yoffset) {
	char str[64];
	char *s;
	if (xoffset != 0 || yoffset != 0) {
		while (fgets(line, sizeof(line), input) != NULL) {
			if (*line == '%' || *line == ';' || *line == '\n') {
				fputs(line, output); 
				continue;
			}
			break;
		}
/*		strip off line terminator */		
		s = line + strlen(line) - 1;
		while (*s == '\n' || *s == '\r' || *s == ' ') *s-- = '\0';
		sprintf(str, " %% offset %d %d\n", xoffset, yoffset);
		strcat (line, str);
		fputs(line, output); 
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == ']') return 1;
		fputs(line, output); 
	}
	return 0;
}

int	addchartoshape(char *fontname, char *chij, int nij, int ni, int nj,
				   double width, int offset) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	FILE *input, *output;
	char *s;
	int endsinbracket;
	
	strcpy(infilename, fontname);
	forceexten(infilename, "bak");
	strcpy(outfilename, fontname);
	forceexten(outfilename, "out");
	(void) remove(infilename);
	rename(outfilename, infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);		exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);		exit(1);
	}
	s = copyto (output, input, "] end");	/* copy the whole file */
	if (s == NULL && *line == ']') endsinbracket = 1;
	else endsinbracket = 0;
	if (nj != 32 || dontmerge) {
		rewind(input);
		if (scantooutline (input, ni) < 0) {
			printf("Can't find char %d\n", ni); exit(1);
		}
		if (endsinbracket == 0) fputs ("]\n", output);
		fprintf(output, "%d %d %% %s\n", nij, roundit (width), chij);
		copyoutline (output, input, 0, 0);
		rewind(input);
		if (scantooutline (input, nj) < 0) {
			printf("Can't find char %d\n", nj); exit(1);
		}
		copyoutline (output, input, offset, 0);
	}
	else {			/* special case overlapping Lslash / lslash */
		rewind (input);
		readsuppress(input, offset);
		if (endsinbracket == 0) fputs("]\n", output);
		fprintf(output, "%d %d %% %s\n", nij, roundit (width), chij);
		makelslash (output, input, ni);
	}
/*	fputs ("]\n", output); */
	fclose (output);
	fclose (input);
	return 0;
}

int	addoffsetchar(char *fontname, char *chij, int nij, int nj,
				   double width, int xoffset, int yoffset) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	FILE *input, *output;
	char *s;
	int endsinbracket;
	
	strcpy(infilename, fontname);
	forceexten(infilename, "bak");
	strcpy(outfilename, fontname);
	forceexten(outfilename, "out");
	(void) remove(infilename);
	rename(outfilename, infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);		exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);		exit(1);
	}
	s = copyto (output, input, "] end");	/* copy the whole file */
	if (s == NULL && *line == ']') endsinbracket = 1;
	else endsinbracket = 0;

	rewind(input);
	if (scantooutline (input, nj) < 0) {
		printf("Can't find char %d\n", nj); exit(1);
	}
	if (endsinbracket == 0) fputs ("]\n", output);
	fprintf(output, "%d %d %% %s\n", nij, roundit (width), chij);
	copyoutline (output, input, xoffset, yoffset);

	fclose (output);
	fclose (input);
	return 0;
}

/* copy a single character from one font to another */

int	copyoutchar(char *tofontname, char *charname, int nchr, char *fromfontname,
				   int ichr) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	char sourcefilename[MAXFILENAME];
	FILE *input, *output, *source;
	char *s;
	int endsinbracket;
	int jchr;
	double jwidth;
	char jname[MAXCHARNAME];
	
	strcpy(sourcefilename, fromfontname);
	forceexten(sourcefilename, "out");	
	if ((source = fopen(sourcefilename, "r")) == NULL) {
		perror(sourcefilename);		exit(1);
	}
	strcpy(infilename, tofontname);
	forceexten(infilename, "bak");
	strcpy(outfilename, tofontname);
	forceexten(outfilename, "out");
	(void) remove(infilename);
	rename(outfilename, infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);		exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);		exit(1);
	}
	s = copyto (output, input, "] end");	/* copy the whole file */
	if (s == NULL && *line == ']') endsinbracket = 1;
	else endsinbracket = 0;

	if (scantooutline (source, ichr) < 0) {
		printf("Can't find char %d\n", ichr); exit(1);
	}
	if (sscanf (line, "%d %lg %% %s", &jchr, &jwidth, &jname) < 2) {
		printf("Can't parse %s", line);
	}
	if (jchr != ichr || strcmp(jname, charname) != 0) {
		printf("jchr %d ichr %d jname `%s' charname `%s'\n",
			   jchr, ichr, jname, charname);	/* impossible */
	}
	if (endsinbracket == 0) fputs ("]\n", output);
	if (commentflag)
		fprintf(output, "%% imported from %s (%d)\n", fromfontname, ichr);
/*	fprintf(output, "%d %d %% %s\n", nchr, roundit (width), charname); */
	fprintf(output, "%d %d", nchr, roundit (width));
	if (skew != 0.0) fprintf(output, " %% slant %lg", skew);	/* 96/Sep/2 */
	fprintf(output, " %% %s\n", charname);
	copyoutline (output, source, 0, 0);

	fclose (output);
	fclose (source);
	fclose (input);
	return 0;
}


int	addunderscoreout(char *fontname, int nchr, double width, 
					 int xll, int yll, int xur, int yur) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	FILE *input, *output;
	char *s;
	int endsinbracket;
	int xlld, xurd;
	
	if (italicflag) {
		xlld = xll + (int) ((double) yur * 0.2);
		xurd = xur + (int) ((double) yur * 0.2);
	}
	else {
		xlld = xll; xurd = xur;
	}

	strcpy(infilename, fontname);
	forceexten(infilename, "bak");
	strcpy(outfilename, fontname);
	forceexten(outfilename, "out");
	(void) remove(infilename);
	rename(outfilename, infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);		exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);		exit(1);
	}
	s = copyto (output, input, "] end");	/* copy the whole file */
	if (s == NULL && *line == ']') endsinbracket = 1;
	else endsinbracket = 0;

	if (endsinbracket == 0) fputs("]\n", output);
	fprintf(output, "%d %d %% %s\n", nchr, roundit(width), "underscore");
	fprintf(output, "%d %d m\n", xll, yll);
	fprintf(output, "%d %d l\n", xur, yll);
	fprintf(output, "%d %d l\n", xurd, yur);
	fprintf(output, "%d %d l\n", xlld, yur);
	fputs("h\n", output);
	fclose (output);
	fclose (input);
	return 0;
}

int addcompositehint(char *fontname, int ijchr, int ichr, int jchr, int offset) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	FILE *input, *output;
	char str[64];
	char buffer[MAXLINE];
	char *s;
	int vleft, vright, n;

	strcpy(infilename, fontname);
	forceexten(infilename, "bak");
	strcpy(outfilename, fontname);
	forceexten(outfilename, "hnt");
	(void) remove(infilename);
	rename(outfilename, infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);		exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);		exit(1);
	}
	copyto(output, input, "");
	rewind(input);
	sprintf(str, "C %d ; ", ichr);
	if (scanto (input, str) == NULL) {
		printf("Can't find HNT line for %d\n", ichr);
		exit(1);
	}
	if ((s = strchr(line, ';')) == NULL) {
		printf("Can't parse %s", line);
		strcpy(buffer, line);				/* save it */
	}
	else sprintf(buffer, "C %d %s", ijchr, s);
/*	don't copy hints from "suppress" */
	if (jchr != 32) {
		rewind(input);
		sprintf(str, "C %d ; ", jchr);
		if (scanto (input, str) == NULL) {
			printf("Can't find HNT line for %d\n", jchr);
			exit(1);
		}
		s = buffer + strlen(buffer) - 1;
		while (s > buffer && (*s == '\n' || *s == ';' || *s == ' ')) *s-- = '\0';
		strcat(s, " ");
		if ((s = strchr (line, 'V')) == NULL) {
			printf("No vertical hint in %s", line);
		}
		else {
			s += 2;
			while (sscanf(s, "%d %d%n", &vleft, &vright, &n) == 2) {
				sprintf(str, "%d %d ", vleft + offset, vright + offset);
				strcat(buffer, str);
				s += n;
			}
		}
		strcat (buffer, "; \n");
	}
	fputs(buffer, output);
	
	fclose(output);
	fclose(input);
	return 0;
}

int addoffsethint(char *fontname, int ijchr, int jchr, int xoffset, int yoffset) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	FILE *input, *output;
	char str[64];
	char buffer[MAXLINE];
	char *s;
	int vleft, vright, n;

	strcpy(infilename, fontname);
	forceexten(infilename, "bak");
	strcpy(outfilename, fontname);
	forceexten(outfilename, "hnt");
	(void) remove(infilename);
	rename(outfilename, infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);		exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);		exit(1);
	}
	copyto(output, input, "");

	rewind(input);
	sprintf(str, "C %d ; ", jchr);
	if (scanto (input, str) == NULL) {
		printf("Can't find HNT line for %d\n", jchr);
		exit(1);
	}
	if ((s = strchr(line, ';')) == NULL) {
		printf("Can't parse %s", line);
		strcpy(buffer, line);				/* save it */
	}
	else sprintf(buffer, "C %d ;", ijchr);
	s = buffer + strlen(buffer) - 1;
	strcat(s, " ");
	if ((s = strchr (line, 'H')) == NULL) {
		printf("No horizontal hint in %s", line);
	}
	else {
		strcat (buffer, "H ");
		s += 2;
		while (sscanf(s, "%d %d%n", &vleft, &vright, &n) == 2) {
			sprintf(str, "%d %d ", vleft + yoffset, vright + yoffset);
			strcat(buffer, str);
			s += n;
		}
	}
	strcat (buffer, "; ");
	if ((s = strchr (line, 'V')) == NULL) {
		printf("No vertical hint in %s", line);
	}
	else {
		strcat (buffer, "V ");
		s += 2;
		while (sscanf(s, "%d %d%n", &vleft, &vright, &n) == 2) {
			sprintf(str, "%d %d ", vleft + xoffset, vright + xoffset);
			strcat(buffer, str);
			s += n;
		}
	}
	strcat (buffer, "; \n");
	fputs(buffer, output);
	
	fclose(output);
	fclose(input);
	return 0;
}

int copyhintchar(char *tofontname, int nchr, char *fromfontname, int jchr) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	char sourcefilename[MAXFILENAME];
	FILE *input, *output, *source;
	char str[64];
	char buffer[MAXLINE];
	char *s;

	strcpy(sourcefilename, fromfontname);
	forceexten(sourcefilename, "hnt");
	if ((source = fopen(sourcefilename, "r")) == NULL) {
		perror(sourcefilename);		exit(1);
	}
	strcpy(infilename, tofontname);
	forceexten(infilename, "bak");
	strcpy(outfilename, tofontname);
	forceexten(outfilename, "hnt");
	(void) remove(infilename);
	rename(outfilename, infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);		exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);		exit(1);
	}
	copyto(output, input, "");

	sprintf(str, "C %d ; ", jchr);
	if (scanto (source, str) == NULL) {
		printf("Can't find HNT line for %d\n", jchr);
		exit(1);
	}
	if ((s = strchr(line, ';')) == NULL) {
		printf("Can't parse %s", line);
		strcpy(buffer, line);				/* save it */
	}
	else sprintf(buffer, "C %d %s", nchr, s);
	if (skew != 0.0) {			/* flush vertical hints if char skewed */
		if ((s = strstr(buffer, " V ")) != NULL) strcpy(s, "\n");
	}
	fputs(buffer, output);
	for(;;) {
		int repl, ichr;
		if (fgets(line, sizeof(line), source) == NULL) break;
		if (*line == 'C') break;
		if (sscanf(line, "S %d ; C %d ", &repl, &ichr) < 2) {
			printf("Can't parse %s", line); break;
		}
		if ((s = strchr(line, ';')) == NULL) break;
		if ((s = strchr(s+1, ';')) == NULL) break;		
		sprintf(buffer, "S %d ; C %d %s", repl, nchr, s);
		fputs(buffer, output);		
	}
	fclose(output);
	fclose(source);
	fclose(input);
	return 0;
}

int addhintline(char *fontname, char *line) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	FILE *input, *output;
	char buffer[MAXLINE];

	strcpy(infilename, fontname);
	forceexten(infilename, "bak");
	strcpy(outfilename, fontname);
	forceexten(outfilename, "hnt");
	(void) remove(infilename);
	rename(outfilename, infilename);
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);		exit(1);
	}
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);		exit(1);
	}
	strcpy(buffer, line);
	copyto(output, input, "");
	fputs(buffer, output);
	
	fclose(output);
	fclose(input);
	return 0;
}

int docomposite (char *fontname, char *chi, char *chj, char *chij, int ijchr) {
	int ixll, iyll, ixur, iyur;
	int jxll, jyll, jxur, jyur;
	int iright, jleft;
	int kerning;					/* adjustment between two chars */
	int offset;						/* offset of second char */
	int ijxll, ijyll, ijxur, ijyur;
	double iwidth, jwidth, ijwidth;
	int ichr, jchr;

	if (getbbox(fontname, chi) < 0) {
		printf("Can't get bounding box for %s\n", chi);
		exit(1);
	}
	ixll=xll; iyll=yll; ixur=xur; iyur=yur;
	iwidth = width; ichr = chr;
	if (getbbox(fontname, chj) < 0) {
		printf("Can't get bounding box for %s\n", chj);
		exit(1);
	}
/*	compute kerning, advance width, and bounding box */
	jxll=xll; jyll=yll; jxur=xur; jyur=yur;
	jwidth = width; jchr = chr;
	iright = roundit (iwidth) - ixur;
	jleft = jxll;
	if (ichr == 'i' && jchr == 'j') {
		if (jleft < 0) kerning = - iright;	/* `normal' case */
		else kerning = - (iright + jleft);
		if (kerning > 0) kerning = 0;	/* never move further apart */
	}
	else if (ichr == 'I' && jchr == 'J') {
		if (jleft < 0) kerning = -iright-jleft;	/* `normal' case */
		else if (jleft < iright) kerning = -jleft;
		else kerning = -iright;
		if (kerning > 0) kerning = 0;	/* never move further apart */
	}
	else if (ichr == 'S' && jchr == 'S') {
		if (jleft < iright) kerning = -iright;
		else kerning = -jleft;
		if (kerning > 0) kerning = 0;	/* never move further apart */
	}
	else if (ichr == 'l' && jchr == 32) {
/*		kerning = roundit (suppresskern); *//* superimpose without offset */
		offset = roundit(- suppresskern - jwidth);
		kerning = offset - roundit(iwidth) ;
	}
	else if (ichr == 'L' && jchr == 32) {
/*		kerning = roundit (suppresskern); *//* superimpose with some offset */
		offset = roundit(- suppresskern - jwidth);
		kerning = offset - roundit(iwidth);
/*		not quite correct yet ? */
	}
	else {
		kerning = 0;
		printf("Not i and j or S and S or L and suppress\n"); 
	}
	ijwidth = (iwidth + jwidth) + kerning;	/* width of composite char */

	if (jchr == 32) ijwidth = iwidth;		/* special case suppress */

	offset = roundit (iwidth) + kerning;	/* offset if second rel first */

	ijxll = ixll;							/* bounding box */
	ijxur = jxur + roundit (iwidth) + kerning;

	if (iyll < jyll) ijyll = iyll;
	else ijyll = jyll;
	if (iyur > jyur) ijyur = iyur;
	else ijyur = jyur;
	if (doafmflag) {
		if (verboseflag) printf("Working on AFM file\n");
/*		first deal with AFM file modifications */
		sprintf(line, "C %d ; WX %lg ; N %s ; B %d %d %d %d ;\n",
				ijchr, ijwidth, chij, ijxll, ijyll, ijxur, ijyur);
/*		add new line just before EndCharMetrics */
		addlinetoafm(fontname, line, "EndCharMetrics", 1);
	}
	if (dooutflag) {
		if (verboseflag) printf("Working on OUT file\n");
/*		now make up composite outline */
		addchartoshape(fontname, chij, ijchr, ichr, jchr, ijwidth, offset);
	}
	if (dohntflag) {
		if (verboseflag) printf("Working on HNT file\n");
/*		now make up composite outline */
		addcompositehint(fontname, ijchr, ichr, jchr, offset);
	}
	return 0;
}

/* make quotesinglbase (norg == 39) and quotedblbase (norg == 34) */
/* based on quoteright and quotedblright (using comma for adjustment) */
/* interchange right and left sidebearing while we are at it */

int doquotebase (char *fontname, char *chj, char *chij, int ijchr) {
	int ichr, ixll, iyll, ixur, iyur;
	int jchr, jxll, jyll, jxur, jyur;
	int ijxll, ijyll, ijxur, ijyur;
	double iwidth, jwidth, ijwidth;
	int xoffset, yoffset;
	int jleft, jright;

	if (getbbox(fontname, "comma") < 0) {
		printf("Can't get bounding box for %s\n", "comma");
		exit(1);
	}
	ixll=xll; iyll=yll; ixur=xur; iyur=yur;
	iwidth = width; ichr = chr;
	if (getbbox(fontname, chj) < 0) {
		printf("Can't get bounding box for %s\n", chj);
		return -1;
	}
	jxll=xll; jyll=yll; jxur=xur; jyur=yur;
	jwidth = width; jchr = chr;

	yoffset = (iyll + iyur) / 2 - (jyll + jyur) / 2;	/* negative */
	
	jright = roundit (jwidth) - jxur;
	jleft = jxll;
	xoffset = jright - jleft;	/* swap right and left sidebearings */
	if (italicflag)	xoffset = xoffset - (int) (yoffset * slant);

	ijyll = jyll + yoffset;
	ijyur = jyur + yoffset;
	ijxll = jxll + xoffset;
	ijxur = jxur + xoffset;

	ijwidth = jwidth;
	
	if (doafmflag) {
		if (verboseflag) printf("Working on AFM file\n");
/*		first deal with AFM file modifications */
		sprintf(line, "C %d ; WX %lg ; N %s ; B %d %d %d %d ;\n",
				ijchr, ijwidth, chij, ijxll, ijyll, ijxur, ijyur);
/*		add new line just before EndCharMetrics */
		addlinetoafm(fontname, line, "EndCharMetrics", 1);
	}
	if (dooutflag) {
		if (verboseflag) printf("Working on OUT file\n");
/*		now make up composite outline */
/*		addchartoshape */
		addoffsetchar(fontname, chij, ijchr, jchr, ijwidth, xoffset, yoffset);
	}
	if (dohntflag) {
		if (verboseflag) printf("Working on HNT file\n");
/*		now make up composite outline */
/*		addcompositehint */
		addoffsethint(fontname, ijchr, jchr, xoffset, yoffset);
	}
	return 0;
}

int getfontinfo (char *fontname) {
	char filename[MAXFILENAME];
	FILE *input;
	char *s;

	sansserif = 0;
	if (strncmp(fontname, "CMSS", 4) == 0) sansserif = 1;
	else if (strncmp(fontname, "cmss", 4) == 0) sansserif = 1;

	strcpy(filename, fontname);
	forceexten(filename, "afm");
	if ((input = fopen(filename, "r")) == NULL) {
		perror(filename);		exit(1);
	}
	quad = 1000.0;
	if ((s = scanto (input, "Comment Quad")) == NULL) {
		printf("Can't find AFM line for %s\n", "Comment Quad");
		exit(1);
	}
	else if (sscanf (line+13, "%lg", &quad) < 1) {
		printf("Unable to parse %s", line);
		exit(1);
	}

	italicangle = 0.0;
	italicflag = 0;
	rewind (input);
	if ((s = scanto (input, "ItalicAngle")) == NULL) {
		printf("Can't find AFM line for %s\n", "ItalicAngle");
	}
	else if (sscanf (line+12, "%lg", &italicangle) < 1) {
		printf("Unable to parse %s", line);
		exit(1);
	}	
	if (italicangle < 0.0) italicflag = 1;
	slant = tan (-italicangle * 3.141592653 / 180.0);
	if (traceflag) printf("Slant %lg\n", slant);

	boldflag = 0;
	rewind (input);
	if ((s = scanto (input, "Weight")) == NULL) {
		printf("Can't find AFM line for %s\n", "Weight");
	}
	else {
		if (strstr (line+7, "Bold") != NULL) boldflag = 1;
		else if (strstr (line+7, "Heavy") != NULL) boldflag = 1;
		else if (strstr (line+7, "Black") != NULL) boldflag = 1;
		else if (strstr (line+7, "Demi") != NULL) boldflag = 1;
	}

	fixedpitch = 0;
	if ((s = scanto (input, "IsFixedPitch")) == NULL) {
		printf("Can't find AFM line for %s\n", "IsFixedPitch");
	}
	else {
		if (strstr (line+12, "true") != NULL) fixedpitch = 1;
		else if (strstr (line+12, "false") != NULL) fixedpitch = 0;
	}
	if (verboseflag)
		printf("%s%s%s%s%s quad %lg slant %lg\n",
			   (!boldflag && !italicflag) ? "REGULAR" : "",
			   boldflag ? "BOLD" : "",
			   italicflag ? "ITALIC" : "",
			   sansserif ? " SANS SERIF" : "",
			   fixedpitch ? " FIXED PITCH": "",
			   quad, slant);
							
	fclose (input);
	return 0;				/* success */
}

int dounderscore (char *fontname, int nchr) {
	double width;
	int xll, yll, xur, yur, xurd;

	width = 0.36 * quad;
	xll = (int) (0.06 * quad);
	yll = 0;
	xur = (int) width;
	yur = rulethickness;
	if (italicflag) xurd = xur + (int) ((double) yur * 0.2);
	else xurd = xur;
	if (doafmflag) {
		sprintf(line, "C %d ; WX %lg ; N %s ; B %d %d %d %d ;\n",
				nchr, width, "underscore", xll, yll, xurd, yur);
		addlinetoafm (fontname, line, "EndCharMetrics", 1);
	}
	if (dooutflag) {
		addunderscoreout (fontname, nchr, width, xll, yll, xur, yur);
	}
	if (dohntflag) {
		sprintf(line, "C %d ; H %d %d ; V %d %d ; \n",
				nchr, yll, yur, xll, xurd);
		addhintline (fontname, line);
	}
	return 0;
}

/***********************************************************************/

int	importchar (char *tofontname, int nchr, char *fromfontname, char *charname) {
/*	char buffer[MAXLINE]; */
	char *s;
	if (getbbox(fromfontname, charname) < 0) {
		printf("Can't get bounding box for %s\n", charname);
		return -1;
	}
/*	chr, width, xll, yll, xur, yur set up at this point */
	if (doafmflag) {
		if (verboseflag) printf("Working on AFM file\n");
		if (traceflag) printf("LINE: %s", line);
/*		and AFM line for source character is in line */
		if ((s = strchr(line, ';')) == NULL) {
			printf("Can't parse %s", line);
			return -1;
		}
		sprintf(str, "C %d %s", nchr, s);
/*		will have the wrong BBox info if skew != 0.0 ... no easy way to fix */
		if (traceflag) printf("BUFFER: %s", str);
		addlinetoafm (tofontname, str, "EndCharMetrics", 1);
	}
	if (dooutflag) {
		if (verboseflag) printf("Working on OUT file\n");
/*		will add % slant ... to character line if skew != 0.0 */
		copyoutchar(tofontname, charname, nchr, fromfontname, chr);
	}
	if (dohntflag) {
		if (verboseflag) printf("Working on HNT file\n");
/*		should flush vertical hints if skew != 0.0 */
		copyhintchar(tofontname, nchr, fromfontname, chr);
	}
}

/***********************************************************************/

/* addchars <flags> <base name> */
/* which then reads <base name>.out <base name>.afm <base name>.hnt */

int main(int argc, char *argv[]) {
	int firstarg=1;
	int m, ret;
	int count = 0;

/*	while (*argv[firstarg] == '-') */
	while (firstarg < argc && *argv[firstarg] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag = !verboseflag;
		else if (strcmp(argv[firstarg], "-t") == 0) traceflag = !traceflag;
		else if (strcmp(argv[firstarg], "-u") == 0) dontmerge = !dontmerge;
		else if (strcmp(argv[firstarg], "-c") == 0) compositeflag = 1;
		else if (strcmp(argv[firstarg], "-q") == 0) quotebaseflag = 1;
		else if (strcmp(argv[firstarg], "-x") == 0) importflag = 1;
		else if (strcmp(argv[firstarg], "-a") == 0) commentflag = !commentflag;
		else if (strncmp(argv[firstarg], "-s", 2) == 0) {
			if (sscanf(argv[firstarg], "-s=%lg", &skew) < 1) {
				firstarg++;
				if (sscanf(argv[firstarg], "%lg", &skew) < 1) {
					fprintf(stderr, "Don't understand -s %s\n",
							argv[firstarg]);
					exit(1);
				}
			}
		}
/*		else if (strcmp(argv[firstarg], "-s") == 0) rulethickness = 60; */
/*		else if (strcmp(argv[firstarg], "-b") == 0) rulethickness = 80; */
		firstarg++;
	}
/*	alternate use: addchars -x <dest> <ndest> <source> <charname> */
	if (importflag) {
		char *tofontname, *fromfontname;
		char *charname;
		int nchr;

		if (argc < firstarg+3) exit(1);
		tofontname = argv[firstarg];
		if (sscanf(argv[firstarg+1], "%d", &nchr) < 1) {
			printf("Can't parse %s\n", argv[firstarg+1]);
			exit(1);
		}
		fromfontname = argv[firstarg+2];
		charname = argv[firstarg+3];
		if (verboseflag) printf("%s (%d) <= %s (%s)\n",
					tofontname, nchr, fromfontname, charname);
		importchar (tofontname, nchr, fromfontname, charname);
		return 0;
	}

/*	get here if importflag is not set */

	for (m = firstarg; m < argc; m++) {
		printf("Processing %s\n", argv[m]);
		getfontinfo(argv[m]);
		rulethickness = 40;
		if (sansserif) rulethickness = 60;
		if (boldflag) rulethickness = 80;
		if (compositeflag) {
			ret = docomposite (argv[m], "i", "j", "ij", 130);
			if (ret == 0) count++;
			ret = docomposite (argv[m], "I", "J", "IJ", 131);
			if (ret == 0) count++;
			ret = docomposite (argv[m], "S", "S", "SS", 132);
			if (ret == 0) count++;
			ret = docomposite (argv[m], "l", "suppress", "lslash", 133);
			if (ret == 0) count++;
			ret = docomposite (argv[m], "L", "suppress", "Lslash", 134);
			if (ret == 0) count++;
			ret = dounderscore (argv[m], 135);
			if (ret == 0) count++;
		}
		if (quotebaseflag) {
			ret = doquotebase (argv[m], "quoteright", "quotesinglbase",
							   136);
			if (ret == 0) count++;
			ret = doquotebase (argv[m], "quotedblright", "quotedblbase",
							   137);
			if (ret == 0) count++;
		}
		printf("Added %d characters to %s\n", count, argv[m]);
	}
	return 0;
}

/* alternate use: addchars -x <dest> <ndest> <source> <charname> */

/***************************************************************************/

/* intersect line from (x1, y1) to (x2, y2) with x = x0 */

int intersect (int x0, int x1, int y1, int x2, int y2) {
	long numer;
	int denom, y0;
	
	denom = x2 - x1;
	if (denom == 0) {
		printf("Division by zero\n"); exit(1);
	}
	numer = (long) (y2 - y1) * (long) (x0 - x1);
	y0 = y1 + (int) (numer / denom);
	return y0;
}

int intersectx (int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
	double dx43, dx21, dy43, dy21;
	double r1, r2;
	double numer, denom;
	double x;
	
	dx43 = (double) x4 - x3;
	dx21 = (double) x2 - x1;
	dy43 = (double) y4 - y3;
	dy21 = (double) y2 - y1;
	r1 = dy21 * x1 - dx21 * y1;
	r2 = dy43 * x3 - dx43 * y3;
	denom = dx43 * dy21 - dy43 * dx21;
	numer = r1 * dx43 - r2 * dx21;
	x = numer / denom;
	return roundit (x);
}

int intersecty (int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
	double dx43, dx21, dy43, dy21;
	double r1, r2;
	double numer, denom;
	double y;
	
	dy43 = (double) y4 - y3;
	dy21 = (double) y2 - y1;
	dx43 = (double) x4 - x3;
	dx21 = (double) x2 - x1;
	r1 = dx21 * y1 - dy21 * x1;
	r2 = dx43 * y3 - dy43 * x3;
	denom = dy43 * dx21 - dx43 * dy21;
	numer = r1 * dy43 - r2 * dy21;
	y = numer / denom;
	return roundit (y);
}

/*************************************************************************/

