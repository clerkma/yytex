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

/* Code for getting rid of overlaps */
/* That is, find intersections and then retrace the paths */

/* do all the other good stuff first, such as rewind & resplice */ 

/* Data items are segments: */
/* For each: xs, ys, xa, ya, xb, yb, xf, xf, code, used */
/* `code' is 'C', or 'L' (in which case xa, ya, xb, yb zero) or 'H' */
/* `used' is initially zero, but is set to 1 when segment used */

/* Entries in queue are pointers to paths to try */
/* `visible' tells whether starting segment is exterior or interior */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>

/* #define _getch getch */

#define MAXFILENAME 128

#define MAXLINE 256

#define MAXPATH 512

#define MAXQUEUE 64

#define MAXNAME 32

char line[MAXLINE];		/* input line buffer */

int queue[MAXQUEUE];	/* pointers to paths that still need following */

int visible[MAXQUEUE];	/* visible / invisible flag (outside or inside) */

int queinx;				/* pointer into queue */

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
int traceflag=0;
int alwaysclose=0;		/* always show last lineto closing a curve */
/* int alwaysclose=1; */		/* always show last lineto closing a curve */
int dumpflag=0;			/* show details of characters various steps */
int splitflag=1;		/* allow introduction of intersection points */
int enablecross=1;		/* enable cross-over test - visibility level count */
int suppressflag=0;		/* suppress finding of path ends in same point */
int breakout=-1;		/* break out early after so many splits */
int trycurves=0;		/* try intersect lines and curves */
int chopupflag=0;		/* chop up curveto's after reading in */
int dontprocess=0;		/* just spit out paths again */
int straightened=0;		/* how many curveto's straightened out */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void saywhere (void) {
	printf ("char %d ", chrs);
}

void addtoque(int pinx, int flag) {		/* add item to queue */
	if (queinx == MAXQUEUE) {
		fprintf(stderr, "Overflowed the queue\n");
		exit(3);
	}
	queue[queinx]  = pinx; visible[queinx] = flag;
	queinx++;
}

double minfour(double a, double b, double c, double d)  {
	double low;
	low = a;
	if (b < low) low = b;
	if (c < low) low = c;
	if (d < low) low = d;
	return low;
}

double maxfour(double a, double b, double c, double d)  {
	double high;
	high = a;
	if (high < b) high = b;
	if (high < c) high = c;
	if (high < d) high = d;
	return high;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void chopupcurve(int);

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int xoffset=0, yoffset=0;

void moveto(int axs, int ays) {		/* only sets up xold, yold */
	xold = axs; /* 	xold = axs + xoffset; */
	yold = ays; /*	yold = ays + yoffset; */
}

void lineto(int axf, int ayf) {
	if (pathinx == MAXPATH) {
		fprintf(stderr, "Path too long");
		exit(3);
	}
	if (axf == xold && ayf == yold) {
		printf("Zero length lineto in input\n");
		return;
	}
	xs[pathinx] = xold + xoffset;
	ys[pathinx] = yold + yoffset;
	xf[pathinx] = axf + xoffset;
	yf[pathinx] = ayf + yoffset;
	code[pathinx] = 'L'; 
	used[pathinx] = 0; pathinx++;
	xold = axf; /*		xold = axf + xoffset; */
	yold = ayf; /*		yold = ayf + yoffset; */
}

void curveto(int axa, int aya, int axb, int ayb, int axf, int ayf) {
	if (pathinx == MAXPATH) {
		fprintf(stderr, "Path too long");
		exit(3);
	}
	if (axf == xold && ayf == yold) {
		printf("Zero length curveto in input\n");
		return;
	}
	xs[pathinx] = xold + xoffset; 
	ys[pathinx] = yold + yoffset;
	xa[pathinx] = axa + xoffset; 
	ya[pathinx] = aya + yoffset;
	xb[pathinx] = axb + xoffset; 
	yb[pathinx] = ayb + yoffset;
	xf[pathinx] = axf + xoffset; 
	yf[pathinx] = ayf + yoffset;
	code[pathinx] = 'C'; 
	used[pathinx] = 0; pathinx++;
	xold = axf;		/*	xold = axf + xoffset; */
	yold = ayf;		/*	yold = ayf + yoffset; */
}

void closepath(int start) {
	if (pathinx == MAXPATH) {
		fprintf(stderr, "Path too long");
		exit(3);
	}
/*  check whether outline is closed already - if not, complete it lineto */ 
/*	if (xold != xs[start] || yold != ys[start]) { */
	if (xold + xoffset != xs[start] || yold + yoffset != ys[start]) {
/*		printf("*");	*/	/* debugging */
		lineto(xs[start], ys[start]);
	}
	xs[pathinx] = start;		/* hide backward pointer ! */
	code[pathinx] = 'H'; used[pathinx] = 0;  pathinx++;
	addtoque(pathinx, 1);			/* start of next subpath (if any) */
}

/* return -1 when nothing more to scan - hit end of file */
/* scan in outlines for next character */

int scaninchar(FILE *input) {
	int c;
	char *s;
	int startinx=0;
	int axs, ays, axa, aya, axb, ayb, axf, ayf;

	xoffset=0; yoffset=0;
	pathinx = 0;
	if (fgets(line, MAXLINE, input) == NULL) return -1;
	while (*line == '%' || *line == '\n') 
		if (fgets(line, MAXLINE, input) == NULL) return -1;
	if (*line != ']') {
		fprintf(stderr, "Not start of char %s", line);
		return -1;
	}
	if (fgets(line, MAXLINE, input) == NULL) {
		fprintf(stderr, "Hit EOF in scaninchar\n");
		return -1;
	}
	while (*line == '%' || *line == '\n') {
		if (fgets(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "Hit EOF in scaninchar\n");
			return -1;
		}
	}
	if (sscanf(line, "%d %d", &chrs, &width) < 2) {
		fprintf(stderr, "Not char number and width: %s", line);
		if (fgets(line, MAXLINE, input) != NULL)
			fprintf(stderr, "Next line: %s", line);
		else fprintf(stderr, "Hot EOF in scaninchar\n");
	}
	if ((s = strchr(line, '%')) != NULL) sscanf(s+2, "%s", charname);
	else strcpy(charname, "");
	queinx = 0;
	addtoque(0, 1); 
/*	scan up to start of next character or end of file */
	while ((c = getc(input)) != ']') {
		(void) ungetc(c, input);    
		if (fgets(line, MAXLINE, input) == NULL) {	/* EOF */
			return -1;
		}
		if (*line == '%' || *line == '\n') continue; /* try this comments */
		if (strchr(line, 'h') != NULL) {
			closepath(startinx);	/* hide starting point info */
		}
		else if (strchr(line, 'm') != 0) {
			if (sscanf(line, "%d %d m", &axs, &ays) < 2) {
				fprintf(stderr, "Don't understand moveto: %s", line);
/*				return -1; */
				continue;
			}
			else {
				if ((s = strstr(line, "offset")) != NULL) {
					if (sscanf(s, "offset %d %d", &xoffset, &yoffset) < 2)
						fprintf(stderr, "Don't understand offset: %s", line);
				}
				startinx = pathinx;	/* remember starting point */
				moveto(axs, ays); 
			}
		}
		else if (strchr(line, 'l') != 0) {
			if (sscanf(line, "%d %d l", &axf, &ayf) < 2) {
				fprintf(stderr, "Don't understand lineto: %s", line);
/*				return -1; */
				continue;
			}
			else lineto(axf, ayf);
		}
		else if (strchr(line, 'c') != 0) {
			if (sscanf(line, "%d %d %d %d %d %d c", 
				&axa, &aya, &axb, &ayb, &axf, &ayf) < 6) {
			 fprintf(stderr, "Don't understand curveto: %s", line);
			 return -1;
			}
			else curveto(axa, aya, axb, ayb, axf, ayf);
			if (strchr(line, 'x') != NULL) {
				chopupcurve(pathinx-1);					/* 1993/Aug/16 */
			}
		}
		else {
			fprintf(stderr, "Don't understand command: %s", line);
/*			return -1; */
			continue;
		}
	}
	(void) ungetc(c, input);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* try and find equal and opposite segement */
/* int findmatch(int k) { 
	int i;
	int axs, ays, axf, ayf;
	
	axs = xs[k]; ays = ys[k]; axf = xf[k]; ayf = yf[k];
	for (i = 0; i < pathinx; i++) {
		if (code[i] == 'H') continue;
		if (xf[i] == axs && yf[i] == ays &&
			xs[i] == axf && ys[i] == ayf) {
			if (verboseflag) {
				saywhere();
				printf("Matched %d to %d ", i, k);
			}
			else printf(".");
			return i;	
		}
	}
	return -1;
} */

/* try and find other path segment that ends in the same point */ /* NEW */

int findmatch(int k, int vis) { 
	int i;
	int axf, ayf;
	
	if (suppressflag != 0) return -1;
	axf = xf[k]; ayf = yf[k];
	for (i = 0; i < pathinx; i++) {
		if (i == k) continue;
/*		if (used[i] != 0) continue; */
/*		if (used[i+1] != 0) continue; */
		if (code[i] == 'H') continue;
		if (xf[i] == axf && yf[i] == ayf) {
			if (vis != 0) {
				if (traceflag != 0) {
					saywhere();
					printf("Join %d with %d\n", i, k);
				}
				else printf(".");
			}
			return i;			/* found a collision */
		}
	}
	return -1;			/* nothing found */
}

void showsegment (int k) {
	if (code[k] == 'H') printf("%d: h (%d)\n", k, xs[k]);
	else if (code[k] == 'L') 
		printf("%d: %d %d %d %d l\n", k, xs[k], ys[k], xf[k], yf[k]);
	else if (code[k] == 'C') 
		printf("%d: %d %d %d %d %d %d %d %d c\n",  k,
			xs[k], ys[k], xa[k], ya[k], xb[k], yb[k], xf[k], yf[k]);
}

void dumpchar (int n) {
	int k;
	for (k = 0;  k < n; k++) showsegment(k);
	(void) _getch();
}

void showdetails(int k, int i) {
	showsegment(k); showsegment(k+1);
	showsegment(i); showsegment(i+1);	
	(void) _getch();
}

void degencurve(int k) {
	saywhere();
	printf("Degenerate Curveto %d %d %d %d %d %d %d %d\n", 
		xs[k], ys[k], xa[k], ya[k], xb[k], yb[k], xf[k], yf[k]);
}

/* do path actually cross ? or just touch ? */
/* need to look at segment k & k+1 as well as i & i+1 */

/* returns > 0 if crossing from inside to outside of trace */
/* returns   0 if staying outside or staying inside */
/* returns < 0 if crossing from outside to inside of trace */

int crossover(int k, int i) {
	double dxa, dya, dxb, dyb, dxc, dyc, dxd, dyd;
	double rad, crossbefore, crossafter;

	if (code[k] == 'L') {
		dxa = xs[k] - xf[k]; dya = ys[k] - yf[k];
	}
	else {
		dxa = xb[k] - xf[k]; dya = yb[k] - yf[k];
		if (dxa == 0.0 && dya == 0.0) {
			dxa = xa[k] - xf[k]; dya = ya[k] - yf[k];
			if (dxa == 0.0 && dya == 0.0) {
				degencurve(k);
				return 0;
			}
		}
	}
	rad = sqrt(dxa * dxa + dya * dya);
	if (rad == 0.0) return 0;			/* can't decide */
	dxa = dxa / rad; dya = dya / rad;
	if (code[i] == 'L') {	
		dxb = xs[i] - xf[i]; dyb = ys[i] - yf[i];
	}
	else {
		dxb = xb[i] - xf[i]; dyb = yb[i] - yf[i];	
		if (dxb == 0.0 && dyb == 0.0) {
			dxb = xa[i] - xf[i]; dyb = ya[i] - yf[i];
			if (dxb == 0.0 && dyb == 0.0) {
				degencurve(i);
				return 0;
			}
		}
	}
	rad = sqrt(dxb * dxb + dyb * dyb);
	if (rad == 0.0) return 0;			/* can't decide */
	dxb = dxb / rad; dyb = dyb / rad;
	if (code[k+1] == 'L') {
		dxc = xf[k+1] - xs[k+1]; dyc = yf[k+1] - ys[k+1];
	}
	else {
		dxc = xa[k+1] - xs[k+1]; dyc = ya[k+1] - ys[k+1];		
		if (dxc == 0.0 && dyc == 0.0) {
			dxc = xb[k+1] - xs[k+1]; dyc = yb[k+1] - ys[k+1];
			if (dxc == 0.0 && dyc == 0.0) {
				degencurve(k+1);
				return 0;
			}
		}
	}
	rad = sqrt(dxc * dxc + dyc * dyc);
	if (rad == 0.0) return 0;			/* can't decide */
	dxc = dxc / rad; dyc = dyc / rad;
	if (code[i+1] == 'L') {		
		dxd = xf[i+1] - xs[i+1]; dyd = yf[i+1] - ys[i+1];
	}
	else {
		dxd = xa[i+1] - xs[i+1]; dyd = ya[i+1] - ys[i+1];
		if (dxd == 0.0 && dyd == 0.0) {
			dxd = xb[i+1] - xs[i+1]; dyd = yb[i+1] - ys[i+1];
			if (dxd == 0.0 && dyd == 0.0) {
				degencurve(i+1);
				return 0;
			}
		}
	}
	rad = sqrt(dxd * dxd + dyd * dyd);
	if (rad == 0.0) return 0;		/* can't decide */
	dxd = dxd / rad; dyd = dyd / rad;
/*	cross product (a - b) x (d - b) */
	crossbefore = (dxa - dxb) * (dyd - dyb) - (dya - dyb) * (dxd - dxb);
/*	cross product (c - b) x (d - b) */
	crossafter = (dxc - dxb) * (dyd - dyb) - (dyc - dyb) * (dxd - dxb);	
	if (crossbefore > 0.0 && crossafter < 0.0) return -1;
	if (crossbefore < 0.0 && crossafter > 0.0) return +1;	
	else {
		saywhere();
		printf("DID NOT CROSSOVER %d over %d (%lg %lg)\n", 
			k, i, crossbefore, crossafter);
		printf("a (%lg %lg) b (%lg %lg) c (%lg %lg) d (%lg %lg)\n",
			dxa, dya, dxb, dyb, dxc, dyc, dxd, dyd);
		showdetails(k, i);

				return 0;					/* did not move in or out of countour */
	}
}

void avoidinnerstart(void) {
	fprintf(stderr,
	"NOTE: Start of path should not be in overlap (char %d %s)\n",
			chrs, charname);
}

/* This is the core routine for tracing outlines */
/* Start to follow path at startinx */

void followout(FILE *output, int startinx, int vis) {	
	int i, k, dcross;
	int xold, yold;						/* 1994/Feb/8 */
/*	xold, yold may be used before defined ? */
	int xstart, ystart;
	int flag=0;		/* flag turned on once path is non-empty */

	k = startinx;
	if (used[k] != 0) return;	/* already used up - nothing to do */

	if (vis == 0) {							/* on interior trace ? */
/*	if on interior trace, first try and get to an exterior path segment */
		for(;;) {
			if (code[k] == 'H') k = xs[k];	/* just loop back to start */
			if (used[k] != 0) return;		/* already covered segment ? */
			used[k] = 1;					/* no, mark as used, go on */
/* later test whether really crossed over here ... */
			if ((i = findmatch(k, vis)) < 0) k++;	/* no coincidence  */
			else if (enablecross == 0) {	/* allow only double overlap */
				vis = 1 - vis;				/* assumes become visible now! */
				break;						/* break out of this loop */
			}
			else {
				dcross = crossover(k, i);	/* change in covering level? */
				vis = vis + dcross;
				if (vis > 0) break;
				else {
					saywhere();
					printf("Did NOT go outside %d over %d\n", k, i);
				}
				k++;						
			}
		}
		k++;
	}

	if (used[k] != 0) return;	/* already used up - nothing to do */

	xold = yold = 0;			/* keep compiler quiet */

/*  on visible trace here for sure now */
	xstart = xs[k]; ystart = ys[k]; 
	for (;;) {
		if (code[k] == 'H') k = xs[k];	/* just loop back to start */
		if (used[k] != 0) {				/* already covered this segment ? */
			if (flag != 0) 	fprintf(output, "h\n"); /* closepath */
			if (xs[k] != xstart || ys[k] != ystart) {
				saywhere();
				fprintf(stderr, "Ends don't match (%d %d) (%d %d)\n",
					xs[k], ys[k], xstart, ystart);
				avoidinnerstart();
				(void) _getch();
			}
			return; 
		}
		if (flag++ == 0) {
			fprintf(output, "%d %d m\n", xs[k], ys[k]);
			xold = xs[k]; yold = ys[k];			/* 1994/Feb/8 */
		}
		if (code[k] == 'L') {
			if (code[k+1] == 'H' && alwaysclose == 0) {
/*				printf("%d %d l\n", xf[k], yf[k]); */	/* debugging */
			}				/* flush last L - 1994/Oct/8 */
			else fprintf(output, "%d %d l\n", xf[k], yf[k]);
			xold = xf[k]; yold = yf[k];			/* 1994/Feb/8 */
		}
		else if (code[k] == 'C') {
			if ((xold == xa[k] && xa[k] == xb[k] && xb[k] == xf[k]) ||
				(yold == ya[k] && ya[k] == yb[k] && yb[k] == yf[k])) {
				fprintf(output, "%d %d l\n", xf[k], yf[k]);	
				if (verboseflag)
					printf("Straightened out %d %d (%d)\n", xf[k], yf[k], chrs);
				straightened++;
			}
			else  {
				fprintf(output, "%d %d %d %d %d %d c\n", 
					xa[k], ya[k], xb[k], yb[k], xf[k], yf[k]);
			}
			xold = xf[k]; yold = yf[k];			/* 1994/Feb/8 */
		}
		else {
			fprintf(stderr, "Invalid code %d\n", code[k]);	return;
		}
		used[k] = 1;
		if ((i = findmatch(k, vis)) < 0) k++;	/* no collision ? */
		else {					/* we are crossing over another contour */
			if (enablecross == 0) {	/* found a match - assume disappear */
				addtoque(k+1, 0);	/* put continuation on queue, interior */
				k = i+1;			/* jump to other path - assumed visible */
			}
			else {
				dcross = crossover(k, i);	/* change in covering level? */
				vis = vis + dcross;
				if (vis > 1) {
					printf("ERROR: More visible than visible!\n");
					(void) _getch();
					vis = 1;				/* emergency measure ... */
				}
				if (vis <= 0) {				/* if diving under */
					addtoque(k+1, 0);		/* add to queue, interior */
					k = i+1;				/* jump to other path */
				}
				else {
					saywhere();
					printf("Did NOT go inside %d over %d\n", k, i);
					k++;					/* no coverage, continue */
				}
			}
		}
	}
}

/* simply dump the outline as computed - no clever tracing */

void dumpoutchar(FILE *output) {
	int k=0, nocurr=1;
	int xold, yold;							/* 1994/Feb/8 */
/*	xold, yold may be used before defined ? */

	fprintf(output, "]\n");
	fprintf(output, "%d %d", chrs, width);
	if (strcmp(charname, "") != 0) fprintf(output, " %% %s", charname);
	putc('\n', output);
	xold = yold = 0;			/* keep compiler quiet */
	for (k = 0; k < queinx; k++) {		/* while there is stuff to do ... */
		if (nocurr != 0) {
			fprintf(output, "%d %d m\n", xs[k], xs[k]);
			nocurr = 0;
			xold = xs[k]; yold = ys[k];			/* 1994/Feb/8 */
		}
		if (code[k] == 'H') {
			fprintf(output, "h\n");
			nocurr = 1;
		}
		else if (code[k] == 'L') {
			if (code[k+1] == 'H' && alwaysclose == 0) {
/*				printf("%d %d l\n", xf[k], yf[k]); */		/* debugging */
			}				/* flush last L - 1994/Oct/8 */
			else fprintf(output, "%d %d l\n", xf[k], yf[k]);
			xold = xf[k]; yold = yf[k];			/* 1994/Feb/8 */
		}
		else if (code[k] == 'C') {
			if ((xold == xa[k] && xa[k] == xb[k] && xb[k] == xf[k]) ||
				(yold == ya[k] && ya[k] == yb[k] && yb[k] == yf[k])) {
					fprintf(output, "%d %d l\n", xf[k], yf[k]);
				if (verboseflag)
					printf("Straightened out %d %d (%d)\n", xf[k], yf[k], chrs);
				straightened++;
			}		
			else {
				fprintf(output, "%d %d %d %d %d %d c\n", 
					xa[k], ya[k], xb[k], yb[k], xf[k], yf[k]);
			}
			xold = xf[k]; yold = yf[k];			/* 1994/Feb/8 */
		}
		else fprintf(stderr, "Don't recognize code %c\n", code[k]);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* do the hairy tracing routine */

void scanoutchar(FILE *output) {
	int k=0;

	fprintf(output, "]\n");
	fprintf(output, "%d %d", chrs, width);
	if (strcmp(charname, "") != 0) fprintf(output, " %% %s", charname);
	putc('\n', output);
	while (k < queinx) {		/* while there is stuff to do ... */
		followout(output, queue[k], visible[k]); /* may add stuff to queue */
		k++;
	}
}

void checkallused(void) {		/* see whether everything got used up */
	int k;
	
	for (k = 0; k < pathinx; k++) {
		if (code[k] == 'H') continue;
		if (used[k] == 0) fprintf(stderr, "Unused segment %d ", k);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int round(double z) {

	if (z < 0.0) return (- (int) (-z + 0.5));
	else return ((int) (z + 0.5));
}

double xintersect, yintersect;

double alpha, beta;

/* find intersection between lines from (asx, asy) to (axf, ayf)  and */
/* from (bxs, bys) to (bxf, byf)  - return zero if segments don't intersect */

int intersectlines(double axs, double ays, double axf, double ayf,
		double bxs, double bys, double bxf, double byf, int flag) {
	double det, numa, numb;
	double xinta, yinta, xintb, yintb, dx, dy;
	
/*  cross-product (af - as) x (bf - bs) */
	det = (axf - axs) * (byf - bys) - (ayf - ays) * (bxf - bxs); 
	if (det == 0.0)  return 0;  /* parallel lines don't intersect */

/*	cross-product (bs - as) x (bf - bs) */
	numa = (bxs - axs) * (byf - bys) - (bys - ays) * (bxf - bxs);
	alpha = numa / det;  
	if (flag == 0) {
		if (alpha <= 0.0 || alpha >= 1.0) return 0;
	}

/*	cross-product (bs - as) x (af - as) */
	numb = (bxs - axs) * (ayf - ays) - (bys - ays) * (axf - axs);
	beta = numb / det;
	if (flag == 0) {
		if (beta <= 0.0 || beta >= 1.0) return 0;
	}

	xinta = (1.0 - alpha) * axs + alpha * axf;
	xintb = (1.0 - beta) * bxs + beta * bxf;		
	yinta = (1.0 - alpha) * ays + alpha * ayf;
	yintb = (1.0 - beta) * bys + beta * byf;		

	dx = xintb - xinta; dy = yintb - yinta;
	
	if (dx < -0.1 || dx > 0.1 || dy < -0.1 || dy > 0.1)
		fprintf(stderr, "(%lg %lg) <> (%lg %lg)\n", 
			xinta, yinta, xintb, yintb);

	xintersect = (xinta + xintb) * 0.5;
	yintersect = (yinta + yintb) * 0.5;	

	if (alpha <= 0.0 || alpha >= 1.0) return 0;
	if (beta <= 0.0 || beta >= 1.0) return 0;	

	return 1;
}

double bezier(double t, double zs, double za, double zb, double zf) {
	double s;
	s = 1.0 - t;
	return (s * s * s * zs + 3.0 * t * s * s * za + 
		3.0 * t * t * s * zb + t * t * t * zf);
}

double bezierderiv(double t, double zs, double za, double zb, double zf) {
	double s;
	s = 1.0 - t;
/*	return 3.0 * (- s * s * zs + (1.0 - 3.0 * t) * s * za + 
		t * (2.0 - 3.0 * s) * zb + t * t * zf); */
	return 3.0 * (- s * s * zs + s * (s  - 2.0 * t) * za + 
		t * (2.0 * s - t) * zb + t * t * zf);
}

double eps = 0.0001;
int maxloop = 1024;

/* intersect lineto and curveto - return zero if don't intersect */
/* heuristic - numerical - assumes curveto shallow */

int intersectlinecurvesub(double axs, double ays, double axf, double ayf,
		double bxs, double bys, double bxa, double bya, 
			double bxb, double byb, double bxf, double byf) {
	double xt, yt, xl, yl, dx, dy;
	double dlx, dly, dcx, dcy;
	double det, numa, numb;
	int n=0;

/*	first intersect chord with line - get approximate beta */
	(void) intersectlines(axs, ays, axf, ayf, bxs, bys, bxf, byf, 1);
	if (alpha < -0.5 || alpha > 1.5) return 0;
	if (beta < -0.5 || beta > 1.5) return 0;

/* then iteratively try to improve it */
	for (;;) {
		if (alpha < -0.5 || alpha > 1.5) return 0;
		if (beta < -0.5 || beta > 1.5) return 0;	
/*	compute point on curve */
		xt = bezier(beta, bxs, bxa, bxb, bxf);
		yt = bezier(beta, bys, bya, byb, byf);
/*  compute point on line */
		xl = (1.0 - alpha) * axs + alpha * axf;
		yl = (1.0 - alpha) * ays + alpha * ayf;		
/*	compute error */
		dx = xl - xt; dy = yl - yt;
		if (dx > - eps && dx < eps && dy > -eps && dy < eps) { /* converge */
			if (alpha < 0.0 || alpha > 1.0) return 0;
			if (beta < 0.0 || beta > 1.0) return 0;		
			xintersect = xl; yintersect = yl;
			return 1;	/* yes, they intersect right here ! */
		}
		if (n++ > maxloop) return 0;
/*	compute tangents along lineto and curveto */
		dlx = axf - axs; dly = ayf - ays;
		dcx = bezierderiv(beta, bxs, bxa, bxb, bxf);
		dcy = bezierderiv(beta, bys, bya, byb, byf);
		det = dlx * dcy - dly * dcx;
		if (det == 0.0) return 0;
		numa = dx * dcy - dy * dcx;
		numb = dx * dly - dy * dlx;
		alpha -= numa / det;
		beta -= numb /det;
	}
}

/* -1 => no intersection 0 => don't know +1 => intersection */

int intersectlinecurve(double axs, double ays, double axf, double ayf,
		double bxs, double bys, double bxa, double bya, 
			double bxb, double byb, double bxf, double byf) {
	double xminc, yminc, xmaxc, ymaxc;
	double xminl, yminl, xmaxl, ymaxl;	
	double bxsa, bysa, bxab, byab, bxbf, bybf;
	double bxsab, bysab, bxabf, byabf, bxsabf, bysabf;

/*	first see whether enclosing rectangles intersect at all */
	xminc = minfour(bxs, bxa, bxb, bxf);
	yminc = minfour(bys, bya, byb, byf);
	xmaxc = maxfour(bxs, bxa, bxb, bxf);
	ymaxc = maxfour(bys, bya, byb, byf);
	xminl = axs; if (axf < xminl) xminl = axf;
	xmaxl = axs; if (axf > xmaxl) xmaxl = axf;	
	yminl = ays; if (ayf < yminl) yminl = ayf;
	ymaxl = ays; if (ayf > ymaxl) ymaxl = ayf;		
	if (xminl > xmaxc || xminc > xmaxl ||
		yminl > ymaxc || yminc > ymaxl) return -1;
	if (intersectlinecurvesub(axs, ays, axf, ayf, 
		bxs, bys, bxa, bya, bxb, byb, bxf, byf) != 0) return 1;
	else {
/*	attempt to split curveto into two parts */
		bxsa = (bxs + bxa) * 0.5; bysa = (bys + bya) * 0.5;
		bxab = (bxa + bxb) * 0.5; byab = (bya + byb) * 0.5;	
		bxbf = (bxb + bxf) * 0.5; bybf = (byb + byf) * 0.5;	
		bxsab = (bxsa + bxab) * 0.5;  bysab = (bysa + byab) * 0.5;
		bxabf = (bxab + bxbf) * 0.5;  byabf = (byab + bybf) * 0.5;
		bxsabf = (bxsab + bxabf) * 0.5; bysabf = (bysab + byabf) * 0.5;
/*		return 0; */

/*	now try each of the two parts */
		if (intersectlinecurvesub(axs, ays, axf, ayf, 
			bxs, bys, bxsa, bysa, bxsab, bysab, bxsabf, bysabf) != 0) {
			beta = beta * 0.5;	/* ??? */
			return 1;
		}
		if (intersectlinecurvesub(axs, ays, axf, ayf, 
			bxsabf, bysabf, bxabf, byabf, bxbf, bybf, bxf, byf) != 0) {
			beta = 1.0 - 0.5 * (1.0 - beta);
			return 1;
		}
	}
/*	try splitting even finer ? */
	return 0;
}


int intersectcurvessub(double axs, double ays, double axa, double aya,
		double axb, double ayb, double axf, double ayf,
			double bxs, double bys, double bxa, double bya, 
				double bxb, double byb, double bxf, double byf) {
	double axt, ayt, bxt, byt, dx, dy;
	double dax, day, dbx, dby;
	double det, numa, numb;
	int n=0;
	double temp;

/*	first intersect the two chords - get approximate alpha and beta */
	(void) intersectlines(axs, ays, axf, ayf, bxs, bys, bxf, byf, 1);
	if (alpha < -0.5 || alpha > 1.5 ||
		beta < -0.5 || beta > 1.5) {
/*		return 0;		 */
/*  maybe intersect one line with one curveto using the above ? */
		if (trycurves == 0) return 0;
		if (intersectlinecurve (axs, ays, axf, ayf,
			bxs, bys, bxa, bya, bxb, byb, bxf, byf) < 1) {
			if (intersectlinecurve (bxs, bys, bxf, byf,
				axs, ays, axa, aya, axa, aya, axf, ayf) < 1)
					return 0;
			temp = alpha; alpha = beta; beta = temp;
		}
	}

/*	then iteratively try to improve it */
	for (;;) {
		if (alpha < -0.5 || alpha > 1.5) return 0;
		if (beta < -0.5 || beta > 1.5) return 0;
/*	compute point on curve */
		axt = bezier(alpha, axs, axa, axb, axf);
		ayt = bezier(alpha, ays, aya, ayb, ayf);
/*  compute point on other curve */
		bxt = bezier(beta, bxs, bxa, bxb, bxf);
		byt = bezier(beta, bys, bya, byb, byf);
/*	compute error */
		dx = axt - bxt; dy = ayt - byt;
		if (dx > - eps && dx < eps && dy > -eps && dy < eps) {
			xintersect = (axt + bxt) * 0.5; 
			yintersect = (ayt + byt) * 0.5;
			if (alpha < 0.0 || alpha > 1.0) return 0;
			if (beta < 0.0 || beta > 1.0) return 0;		
			return 1;	/* yes, they intersect right here ! */
		}
		if (n++ > maxloop) return 0;	/* give up ... */
/*	compute tangents along lineto and curveto */
		dax = bezierderiv(alpha, axs, axa, axb, axf);
		day = bezierderiv(alpha, ays, aya, ayb, ayf);
		dbx = bezierderiv(beta, bxs, bxa, bxb, bxf);
		dby = bezierderiv(beta, bys, bya, byb, byf);
		det = dax * dby - day * dbx;
		if (det == 0.0) return 0;		/* tangents parallel */
		numa = dx * dby - dy * dbx;
		numb = dx * day - dy * dax;
		alpha -= numa / det;
		beta -= numb /det;
	}
}

/* -1 => no intersection 0 => don't know +1 => intersection */

int intersectcurves(double axs, double ays, double axa, double aya,
		double axb, double ayb, double axf, double ayf,
			double bxs, double bys, double bxa, double bya, 
				double bxb, double byb, double bxf, double byf) {
	double xmina, ymina, xmaxa, ymaxa;
	double xminb, yminb, xmaxb, ymaxb;	

	xmina = minfour(axs, axa, axb, axf);
	ymina = minfour(ays, aya, ayb, ayf);
	xmaxa = maxfour(axs, axa, axb, axf);
	ymaxa = maxfour(ays, aya, ayb, ayf);
	xminb = minfour(bxs, bxa, bxb, bxf);
	yminb = minfour(bys, bya, byb, byf);
	xmaxb = maxfour(bxs, bxa, bxb, bxf);
	ymaxb = maxfour(bys, bya, byb, byf);		

	if (xmina > xmaxb || xminb > xmaxa ||
		ymina > ymaxb || yminb > ymaxa) return -1;
	if (intersectcurvessub(axs, ays, axa, aya, axb, ayb, axf, ayf, 
		bxs, bys, bxa, bya, bxb, byb, bxf, byf) != 0) return 1;
	else return 0;
}

int intersect(int k, int i) {
	int flag;
	double temp;

/*	doesn't count as intersection if intersects at end-points */
	if (xs[k] == xs[i] && ys[k] == ys[i]) return 0;
	if (xs[k] == xf[i] && ys[k] == yf[i]) return 0;
	if (xf[k] == xs[i] && yf[k] == ys[i]) return 0;	
	if (xf[k] == xf[i] && yf[k] == yf[i]) return 0;	
	
	if (code[k] == 'L' && code[i] == 'L') {
		flag = intersectlines(xs[k], ys[k], xf[k], yf[k], 
			xs[i], ys[i], xf[i], yf[i], 0);
		return flag;
	}
	else if (code[k] == 'L' && code[i] == 'C') {
		flag = intersectlinecurve(xs[k], ys[k], xf[k], yf[k], 
			xs[i], ys[i], xa[i], ya[i], xb[i], yb[i], xf[i], yf[i]);
		return flag;
	}
	else if (code[k] == 'C' && code[i] == 'L') {
		flag = intersectlinecurve(xs[i], ys[i], xf[i], yf[i], 
			xs[k], ys[k], xa[k], ya[k], xb[k], yb[k], xf[k], yf[k]);
		temp = alpha; alpha = beta; beta = temp;
		return flag;
	}
	else if (code[k] == 'C' && code[i] == 'C') {
		flag = intersectcurves(xs[k], ys[k], xa[k], ya[k], 
			xb[k], yb[k], xf[k], yf[k], 
				xs[i], ys[i], xa[i], ya[i], xb[i], yb[i], xf[i], yf[i]);
		return flag;
	}	
	else return 0;
}

/* move all path segments up - segment at k is duplicated at k and k+1 */

void makespace(int k) {
	int i;
	for (i = pathinx; i > k; i--) {
		xs[i] = xs[i-1]; ys[i] = ys[i-1]; 
		xa[i] = xa[i-1]; ya[i] = ya[i-1]; 
		xb[i] = xb[i-1]; yb[i] = yb[i-1]; 
		xf[i] = xf[i-1]; yf[i] = yf[i-1]; 		
		code[i] = code[i-1]; used[i] = used[i-1];
		if (code[i] == 'H' && xs[i] > k) xs[i] = xs[i]+1;/* bump pointer up */
	}
	pathinx++;
	for (i = 0; i < queinx; i++) {
		if (queue[i] > k) queue[i] = queue[i] + 1;
	}
}

/* Split line k at (x, y) into two lines */

void splitline (int k, double x, double y) {
	int i, j;

	i = round(x); j = round(y);
	if (xs[k] == round(x) && ys[k] == round(y)) {
		saywhere();
		printf("Bogus lineto split, (%d, %d) to (%d, %d) <= (%lg, %lg)\n", 
			xs[k], ys[k], xf[k], yf[k], x, y);
		return;
	}
	if (xf[k] == round(x) && yf[k] == round(y)) {
		saywhere();
		printf("Bogus lineto split, (%d, %d) to (%d, %d) <= (%lg, %lg)\n", 
			xs[k], ys[k], xf[k], yf[k], x, y);
		return;
	}
	makespace(k);
	xf[k] = i; yf[k] = j;	xs[k+1] = i; ys[k+1] = j;
}

void splitcurve (int k, double x, double y, double alp) {
	double f0, f1, f2, f3;
	double a, b, c, d, mal;
	double an, bn, cn, dn;
	
	if (xs[k] == round(x) && ys[k] == round(y)) {
		saywhere();
		printf("Bogus curveto split, (%d, %d) to (%d, %d) <= (%lg, %lg)\n", 
			xs[k], ys[k], xf[k], yf[k], x, y);
		return;
	}
	if (xf[k] == round(x) && yf[k] == round(y)) {
		saywhere();
		printf("Bogus curveto split, (%d, %d) to (%d, %d) <= (%lg, %lg)\n", 
			xs[k], ys[k], xf[k], yf[k], x, y);
		return;
	}

	makespace(k);

/*  now for the hard part */ /* the curveto is split at t = alp */

	f0 = xs[k]; f1 = xa[k]; f2 = xb[k]; f3 = xf[k];
	
	a = (f3 - f0) - 3.0 * (f2 - f1);
	b = 3.0 * (f2 - 2.0 * f1 + f0);
	c = 3.0 * (f1 - f0);	
	d = f0;
	
	an = a * alp * alp * alp;

	bn = b * alp * alp;
	cn = c * alp;
	dn = d;

	if (xs[k] != round(dn))  /*	xs[k] should equal round(dn) */
		printf("xs[%d] (%d) <> dn (%d)\n", k, xs[k], round(dn));
	xa[k] = round(dn + cn / 3.0);
	xb[k] = round(dn + (2.0 * cn + bn) / 3.0);
	xf[k] = round(x);
	if (xf[k] != round(dn+cn+bn+an))  /* xf[k] = round(dn + cn + bn + an); */
		printf("xf[%d] (%d) <> dn+cn+bn+an (%d)\n", 
			k, xf[k], round(dn+cn+bn+an));

	mal = 1.0 - alp;
	an =  a * mal * mal * mal;
	bn =  (3.0 * a * alp + b) * mal * mal;
	cn =  (alp * (3.0 * a * alp + 2.0 * b) + c) * mal;
	dn =  alp * ( alp * (alp * a + b) + c) + d; 
	
	xs[k+1] = round(x);
	if (xs[k+1] != round(dn))  /*	xs[k+1] should equal round(dn) */
		printf("xs[%d+1] (%d) <> dn (%d)\n", k, xs[k+1], round(dn));
	xa[k+1] = round(dn + cn / 3.0);
	xb[k+1] = round(dn + (2.0 * cn + bn) / 3.0);
	if (xf[k+1] != round(dn+cn+bn+an))  /* xf[k+1] = round(dn+cn+bn+an); */
		printf("xf[%d+1] (%d) <> dn+cn+bn+an (%d)\n", 
			k, xf[k+1], round(dn+cn+bn+an));

	f0 = ys[k]; f1 = ya[k]; f2 = yb[k]; f3 = yf[k];
	
	a = (f3 - f0) - 3.0 * (f2 - f1);
	b = 3.0 * (f2 - 2.0 * f1 + f0);
	c = 3.0 * (f1 - f0);	
	d = f0;
	
	an = a * alp * alp * alp;
	bn = b * alp * alp;
	cn = c * alp;
	dn = d;

	if (ys[k] != round(dn))  /*	ys[k] should equal round(dn) */
		printf("ys[%d] (%d) <> dn (%d)\n", k, ys[k], round(dn));
	ya[k] = round(dn + cn / 3.0);
	yb[k] = round(dn + (2.0 * cn + bn) / 3.0);
	yf[k] = round(y);
	if (yf[k] != round(dn+cn+bn+an))  /* yf[k] = round(dn + cn + bn + an); */
		printf("yf[%d] (%d) <> dn+cn+bn+an (%d)\n", 
			k, yf[k], round(dn+cn+bn+an));

	mal = 1.0 - alp;
	an =  a * mal * mal * mal;
	bn =  (3.0 * a * alp + b) * mal * mal;
	cn =  (alp * (3.0 * a * alp + 2.0 * b) + c) * mal;
	dn =  alp * ( alp * (alp * a + b) + c) + d; 
	
	ys[k+1] = round(y);
	if (ys[k+1] != round(dn))  /*	ys[k+1] should equal round(dn) */
		printf("ys[%d+1] (%d) <> dn (%d)\n", k, ys[k+1], round(dn));
	ya[k+1] = round(dn + cn / 3.0);
	yb[k+1] = round(dn + (2.0 * cn + bn) / 3.0);
	if (yf[k+1] != round(dn+cn+bn+an))  /* yf[k+1] = round(dn+cn+bn+an); */
		printf("yf[%d+1] (%d) <> dn+cn+bn+an (%d)\n", 
			k, yf[k+1], round(dn+cn+bn+an));

/*	now check whether introduced bogus curveto ! */		
/*	if (xs[k] == xf[k] && ys[k] == yf[k]) {
		printf("BAD split curveto\n"); 
	}  */
/*	if (xs[k+1] == xf[k+1] && ys[k+1] == yf[k+1]) {
		printf("BAD split curveto\n");
	}  */
}

void splitsegment (int k, double x, double y, double alp) {
	if (code[k] == 'L') splitline(k, x, y);
	else if (code[k] == 'C') splitcurve(k, x, y, alp);
}

int splitatcrossings() {
	int i, k, crosscount=0;

/*	crosscount = 0; */
	for (k = 0; k < pathinx; k++) {
		if (code[k] == 'H') continue;
		for (i = k+1; i < pathinx; i++) {
			if (code[i] == 'H') continue;
			if (intersect(k, i) > 0) {
				if (traceflag != 0) {
					saywhere();
					printf("Intersect %d %d at %lg %lg\n", 
						k, i, xintersect, yintersect);
				}
				splitsegment(i, xintersect, yintersect, beta);
				splitsegment(k, xintersect, yintersect, alpha);
				crosscount++;
				if (dumpflag != 0) dumpchar(pathinx);
				if (breakout > 0 && crosscount == breakout) 
					return crosscount;	/* break out early before mess */
			}
		}
	}
	return crosscount;
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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage (char *name) {
	printf("%s [{-v}{-d}{-s}{-c}{-p}] [0-9] <outline-file>\n",
		name);
	printf("WARNING: segment start should not be in overlap\n");
	printf("\
\tv verbose\n\
\td show segments of character\n\
\tl do not output implicit lineto's closing curves\n\
\ts disable splitting\n\
\tc disable cross\n\
\tp suppress finding path ends in same point\n\
\ti try using line/curve intersection\n\
\tz chop up curveto's first\n\
\t<n> breakout early after <n> splits\n");
}

/* \tl output implicit lineto's closing curves\n\ */ /* 1995/Jan/25 */

void chopupcurve(int i) {
	double bxs, bys, bxa, bya, bxb, byb, bxf, byf;
	double bxsa, bysa, bxab, byab, bxbf, bybf;
	double bxsab, bysab, bxabf, byabf, bxsabf, bysabf;

	makespace(i);
	bxs = xs[i]; bys = ys[i];
	bxa = xa[i]; bya = ya[i];
	bxb = xb[i]; byb = yb[i];
	bxf = xf[i]; byf = yf[i];
	bxsa = (bxs + bxa) * 0.5; bysa = (bys + bya) * 0.5;
	bxab = (bxa + bxb) * 0.5; byab = (bya + byb) * 0.5;	
	bxbf = (bxb + bxf) * 0.5; bybf = (byb + byf) * 0.5;	
	bxsab = (bxsa + bxab) * 0.5;  bysab = (bysa + byab) * 0.5;
	bxabf = (bxab + bxbf) * 0.5;  byabf = (byab + bybf) * 0.5;
	bxsabf = (bxsab + bxabf) * 0.5; bysabf = (bysab + byabf) * 0.5;

	xs[i] = (int) bxs; ys[i] = (int) bys;
	xa[i] = (int) bxsa; ya[i] = (int) bysa;
	xb[i] = (int) bxsab; yb[i] = (int) bysab;
	xf[i] = (int) bxsabf; yf[i] = (int) bysabf;
	i++;
	xs[i] = (int) bxsabf; ys[i] = (int)  bysabf;
	xa[i] = (int) bxabf; ya[i] = (int) byabf;
	xb[i] = (int) bxbf; yb[i] = (int) bybf;
	xf[i] = (int) bxf; yf[i] = (int) byf;
}

/* divide all curveto's in half */

void chopupchar(void) {
/*	double bxs, bys, bxa, bya, bxb, byb, bxf, byf;
	double bxsa, bysa, bxab, byab, bxbf, bybf;
	double bxsab, bysab, bxabf, byabf, bxsabf, bysabf; */
	int i;
	if (verboseflag) printf("Chopping up path with %d knots\n", pathinx);
	for (i = 0; i < pathinx; i++) {	
		if (code[i] == 'C') {
			chopupcurve(i);
			i++;
/*			makespace(i);
			bxs = xs[i]; bys = ys[i];
			bxa = xa[i]; bya = ya[i];
			bxb = xb[i]; byb = yb[i];
			bxf = xf[i]; byf = yf[i];
			bxsa = (bxs + bxa) * 0.5; bysa = (bys + bya) * 0.5;
			bxab = (bxa + bxb) * 0.5; byab = (bya + byb) * 0.5;	
			bxbf = (bxb + bxf) * 0.5; bybf = (byb + byf) * 0.5;	
			bxsab = (bxsa + bxab) * 0.5;  bysab = (bysa + byab) * 0.5;
			bxabf = (bxab + bxbf) * 0.5;  byabf = (byab + bybf) * 0.5;
			bxsabf = (bxsab + bxabf) * 0.5; bysabf = (bysab + byabf) * 0.5;

			xs[i] = (int) bxs; ys[i] = (int) bys;
			xa[i] = (int) bxsa; ya[i] = (int) bysa;
			xb[i] = (int) bxsab; yb[i] = (int) bysab;
			xf[i] = (int) bxsabf; yf[i] = (int) bysabf;
			i++;
			xs[i] = (int) bxsabf; ys[i] = (int)  bysabf;
			xa[i] = (int) bxabf; ya[i] = (int) byabf;
			xb[i] = (int) bxbf; yb[i] = (int) bybf;
			xf[i] = (int) bxf; yf[i] = (int) byf; */
		}
	}
	if (verboseflag) printf("Chopped up path has %d knots\n", pathinx);
}

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int main(int argc, char *argv[]) {
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	FILE *input, *output;
	int flag = 0, firstarg=1, crosscount;
	int c;
	char *s;
	
	if (argc < 2) {
		showusage (argv[0]);
		exit(3);
	}

	printf("UNIONIZE - program for taking the union of sets\n");

	straightened=0;

	s = argv[firstarg];
	while (firstarg < argc && *s++ == '-') {
		while ((c = *s++) != '\0') {
			if (c == 'v') verboseflag = !verboseflag;
			if (c == 't') traceflag = !traceflag;
			if (c == 'l') alwaysclose = !alwaysclose;
			if (c == 'd') dumpflag = !dumpflag;			
			if (c == 's') splitflag = !splitflag;
			if (c == 'c') enablecross = !enablecross;
			if (c == 'p') suppressflag = !suppressflag;
			if (c == 'i') trycurves =!trycurves;
			if (c == 'z') chopupflag =!chopupflag;
			if (c >= '0' && c <= '9') breakout = c - '0';
		}
		firstarg++;
		s = argv[firstarg];
	}

	strcpy(infilename, argv[firstarg]);
	extension(infilename, "out");
	if ((input = fopen(infilename, "r")) == NULL) {
		forceexten(infilename, "res");
		if ((input = fopen(infilename, "r")) == NULL) {
			forceexten(infilename, "out");
			if ((input = fopen(infilename, "r")) == NULL) {
				perror(infilename); exit(1);
			}
		}
	}

/*	strcpy(outfilename, argv[firstarg]); */
	strcpy(outfilename, stripname(argv[firstarg]));
	forceexten(outfilename, "uni");

	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename); exit(2);
	}	

/*	just copy first two lines (ptsize, scale and BBox) */
	(void) fgets(line, MAXLINE, input);
	while (*line == '%' || *line == '\n') {
		fputs(line, output);
		(void) fgets(line, MAXLINE, input);
	}
	fputs(line, output);			/* first non-comment line */
	(void) fgets(line, MAXLINE, input);
	while (*line == '%' || *line == '\n') {
		fputs(line, output);
		(void) fgets(line, MAXLINE, input);
	}
	fputs(line, output);			/* second non-comment line */

/*	if (verboseflag) putc('\n', stdout); */
	while (flag == 0) {
		flag = scaninchar(input);
		if (dumpflag != 0) dumpchar(pathinx);
		if (chopupflag != 0) chopupchar();
		queinx--;					/* remove false start at end */
		if (dumpflag != 0) dumpchar(pathinx);
		crosscount = 0;
		if (splitflag != 0) {
/*			if (verboseflag) printf(" %d", chrs); */
/*	need to do after each subpath added ? */
			if ((crosscount = splitatcrossings()) > 0) 
				printf("%d", crosscount);
			else putc('.', stdout);
		}
		if (traceflag != 0) putc('\n', stdout);
		if (crosscount % 2 == 0) scanoutchar(output);
		else {
			fprintf(stderr, " Odd crossing count (%d)\n", crosscount);
			avoidinnerstart();
			dumpoutchar(output);
		}
		checkallused();
	}
	fclose(input);
	if(ferror(output) != 0) {
		perror(outfilename);
	}
	else fclose(output);
	if (verboseflag) putc('\n', stdout);
	if (straightened)
		printf("Converted %d flat curveto's to lineto's\n", straightened);
	return 0;
}

/* FUTURE EXTENSIONS: */

/* add split of curveto's to intersectcurves also, */
/* (presently only do split in intersectlinecurve) */
/* possibly split the line as well - and do FOUR tests instead of two ? */

/* possibly split recursively - maybe get stack overflow problems ? */
/* need to limit depth of recursion */ /* count level */
/* possibly by not splitting when curveto is too short (or too shallow) */

/* possibly extend the recursive split to go all the way to solution */
/* that is, flush present numerical search along curves ? */
