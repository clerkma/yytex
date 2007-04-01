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

/* Code for unwinding charpaths in new format files */
/* Used for Euler MF stuff */
/* Assumes the input is well formed */
/* First version just reads stuff, stores it and writes it out again */
/* Also removes lineto retraces */
/* assumes all outlines are convex bolbs to be fill with ink - no holes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #define MAXKNOTS 512 */
#define MAXKNOTS 1024
#define MAXLINE 128

#ifdef _WIN32
#define short int
#endif

int verboseflag=-1;
int traceflag=0;

int alwaysreverse=0;			/* simply reverse everything */

int selfclosingflag=-1;			/* split self-intersecting curves */
int cleanoverflag=-1;			/* change `C' to `L' in overlap regions */
int checkrewrite=-1;			/* check for overlapping linetos */
								/* stops it from running reversing code */
int splitcontours=0;			/* split single contours with retrace */
int mergecontours=0;			/* merge two contours with retrace */

int rewritecount;

typedef struct knot {
	short x;
	short y;
	short code;
};

struct knot knots[MAXKNOTS];

struct knot closeknot = {0, 0, 'H'};

char buffer[MAXLINE];

int index;							/* end of current path index */

int chrs, width;

char charname[64];

FILE *errout=stdout;

void resettables(void) {
	index = 0;
}

long windingtables(int ks, int ke) {
	int k; 
	int xs, ys, xf, yf;
	long area=0;

/*	xs = knots[0].x; ys = knots[0].y; */
	xs = knots[ks].x; ys = knots[ks].y;
/*	for (k = 1; k < index; k++) { */
	for (k = ks+1; k < ke; k++) {
		if (knots[k].code != ' ') {
			xf = knots[k].x; yf = knots[k].y;
			area += ((long) xs * yf - (long) xf * ys);
			xs = xf; ys = yf;
		}
	}
	return area;
} 

/***************************************************************************/

void dumptables() {		/* debugging output */
	int k;
	printf("contours of %d knots in %d %s\n", index, chrs, charname);
	for (k = 0; k < index; k++) {
		printf("%d\t%d %d %c\n", k, knots[k].x, knots[k].y, knots[k].code);
	}
	putc('\n', stdout);
}

/* move segment  ks <= k < ke  to  knew <= k < knew + (ke- ks) */

int movecontour (int ks, int ke, int knew) {
	int j;
	if (traceflag)
		printf("MOVECONTOUR %d <= k < %d to %d <= k < %d\n",
			   ks, ke, knew, knew + (ke-ks));
	if (ks == ke) return 0;			/* nothing to move */
	if (knew == ks) return 0;		/* not going anywhere */
	if (ks < 0 || ke > index) {
		printf("Error in movecontour\n");
		exit(1);
	}
	if (knew + (ke - ks) > MAXKNOTS) {
		printf("No space for shifted contours\n");
		exit(1);
	}
	if (knew < ks)		/* downward case */
		for (j = 0; j < ke-ks; j++) knots[knew+j] = knots[ks+j];
	else {				/* upward case*/
/*		for (j = 0; j < ke-ks; j++) knots[index+j] = knots[ks+j]; */
/*		for (j = 0; j < ke-ks; j++) knots[knew+j] = knots[index+j]; */
		for (j = ke-ks-1; j >= 0; j--) knots[knew+j] = knots[ks+j];		
	}
	return (ke - ks);		/* count of knots moved */
}

/* Additions 97/Dec/1 */

void convertCtoL(int i) {	/* convert a curveto to a lineto */
	if (traceflag) 
		printf("Converting %d %d %d %d %d %d C into %d %d L\n",
		   knots[i-2].x, knots[i-2].y,
		   knots[i-1].x, knots[i-1].y,
		   knots[i].x, knots[i].y,
		   knots[i].x, knots[i].y);
	movecontour (i, index, i-2);		/* code `C' is at i */
	knots[i-2].code = 'L';
	index -= 2;				/* data shrunk by two entries */
}

/* break up self-intersecting curves */

int selfclosing (void) {
	int i, count=0;
	int xstart=-1, ystart=-1;

	for (i = 0; i < index; i++) {
		if (knots[i].code == 'M') {
			xstart = knots[i].x;
			ystart = knots[i].y;
		}
		else if (knots[i].code == ' ') ;
		else if	(knots[i].x == xstart && knots[i].y == ystart &&
				 knots[i+1].code != 'H') {
			if (i+1 >= index) printf("ERROR %d >= %d", i+1, index);
			if (traceflag)
			printf("Splitting self-intersecting curve in %d %s at (%d %d)\n",
				   chrs, charname, xstart, ystart);
			movecontour (i, index, i+2);	/* duplicate this point */
			knots[i+1] = closeknot;			/* finish off first part */
			knots[i+2].code = 'M';			/* start of second part */
			index += 2;
			count++;
		}
	}
	return count;					/* nothing found anymore */
}

/* look for equal L and C, C and L, C and C and convert to L and L */
/* so that later processing can recognize these types of overlaps */
/* returns non-zero if something was changed in the process */

int cleanoverlapsub (void) {
	int i, j;
	int iold, jold;			/* set but not used ? */
	int xcurri, ycurri, xoldi, yoldi;
	int xcurrj, ycurrj, xoldj, yoldj;

	for (i = 0; i < index; i++) {
		if (knots[i].code == ' ') continue;
		xcurri = knots[i].x;
		ycurri = knots[i].y;
		if (i > 0) {
			for (j = i; j < index; j++) {
				if (knots[j].code == ' ') continue;
				xcurrj = knots[j].x;
				ycurrj = knots[j].y;
				if (j > i) {
					if (xcurri == xoldj && ycurri == yoldj &&
						xoldi == xcurrj && yoldi == ycurrj) {
						if (knots[i].code == 'C' || knots[j].code == 'C') {
							if (knots[j].code == 'C') convertCtoL(j);
							if (knots[i].code == 'C') convertCtoL(i);
/*							printf("Fix up in %d\n", chrs); */
							return -1;	/* dangerous to continue */
						}
					}
				}
				xoldj = xcurrj;
				yoldj = ycurrj;
				jold = j;
			}
		}
		xoldi = xcurri;
		yoldi = ycurri;
		iold = i;
	}
	return 0;					/* nothing found anymore */
}

int cleanoverlaps (void) {
	int count=0;
	while (cleanoverlapsub() != 0) count++;
	if (count > 0 && traceflag)
		printf("Cleaned up %d retrace%s in %d %s\n",
			   count, (count == 1) ? "" : "s", chrs, charname);
	return count;
}

/* Additions 1997/Nov/25 */

#ifdef IGNORED
void copyup (int k, int i) {	/* make a copy of a piece of code at end */
/*	int j; */
/*	for (j = 0; j <= i-k; j++) knots[index+j] = knots[k+j]; */
	movecontour (k, i, index);
	knots[index].code = 'M';	/* change first lineto into moveto */
	index = index + (i-k);
}

void stripout (int k, int i) {	/* strip out a piece of code */
/*	int j; */
/*	for (j = 0; j <= index-k; j++) knots[k+j] = knots[i+j]; */
	movecontour (i, index + (i-k), k);
	index = index - (i-k);
}
#endif

#ifdef IGNORED
/* here we read only one contour at a time */

/* single contour (A) k-1, k (B) i, i+1 (C) `h' */
/* becomes two -> (A) (C) `h' (B) `h' */

void removeover (void) {	/* overlaps within a contour */
	int i, k;

	for (k = 1; k < index; k++) {
		if (knots[k].code == 'L') {
			for (i = k+1; i < index; i++) {		/* search for match */
				if (knots[i].x == knots[k].x && knots[i].y == knots[k].y &&
					knots[i+1].x == knots[k-1].x && knots[i+1].y == knots[k-1].y &&
					knots[i+1].code == 'L') {
					if (traceflag)
						printf("Rewrite in char %d: %d %d %d %d\n",
							   chrs, knots[k-1].x, knots[k-1].y, knots[k].x, knots[k].y);
					rewritecount++;
					copyup (k, i+1);
					knots[index++] = closeknot;
/* it's i+2 here to avoid i+1 which is just a repeat of what is in k-1 */
					stripout (k, i+2);
					return;				/* assume only one such */
				}
			}
		}
	}
}
#endif

/* overlap within a contour (A) `h' (B) k-1, k (C) i, i+1 (D) `h' (E) `h' */
/* becomes two contours  -> (A) `h' (B) k-1 i+2 (D) `h' (C) `h' (E) `h' */

int removeover (void) {	/* overlaps within a contour */
	int i, k, j;
	int kstart=-1, jend, kcurr, indexold;
	int count=0;

	for (k = 1; k < index; k++) {
		if (knots[k].code == 'M') {		/* remember where contour starts */
			kstart = k;
			continue;
		}
		if (knots[k].code == 'L') {
			for (i = k+1; i < index; i++) {		/* search for match */
				if (knots[i].code == 'H') break;	/* end of this contour */
				if (knots[i].x == knots[k].x && knots[i].y == knots[k].y &&
					knots[i+1].x == knots[k-1].x &&
					knots[i+1].y == knots[k-1].y &&
					knots[i+1].code == 'L') {
					if (traceflag)
						printf("Rewrite in char %d: %d %d %d %d\n",
						   chrs, knots[k-1].x, knots[k-1].y, knots[k].x, knots[k].y);
					rewritecount++;
					for (j = i+1; j < index; j++) {	/* find end of contour */
						if (knots[j].code == 'H') break;
					}
					if (j < index) jend = j+1;
					else jend = j;					/* end of character */
					kcurr = k;
					movecontour(k, jend, index);	/* move up (C) (D) */
					indexold = index;
					index += (jend - k);
					movecontour((i+2) - k  + indexold,
								jend - k + indexold, kcurr); /* (D) */
					kcurr += jend - (i+2);
					movecontour(indexold, (i+1)-k + indexold, kcurr); /* (C) */
					knots[kcurr].code = 'M';
					kcurr += (i+1-k);
					knots[kcurr++] = closeknot;		/* 'H' */
					index = indexold;
					if (kcurr < jend) {		/* close gap if any (E) */
						movecontour(jend, indexold, kcurr);
						index -= (jend - kcurr);
					}
/*					dumptables(); */
					k = kcurr-1;	/* inc at end of loop to kcurr */
					count++;
					break;		/* do only one such within one contour*/
				} /* end of splicing episode */
			} /* end of for (i...) */
		} /* end of 'L' */
	} /* end of for (k...) */
	return count;
}


/* here we read all contours of a letter together */

/* two contour   (A) k-1, k (B) `h' (C) `h' (D) i, i+1 (E) `h' (F) `h' */
/* become one -> (A) (E) (D) (B) `h' (C) 'h' (F) `h' */

int removemoreover (void) {	/* overlaps between different countours */
	int i, j, k, kend, ishift, istart, iend, indexold, indextemp, kcurrent;
	int changed, count=0;

/*	dumptables(); */
	for(;;) {
		changed=0;
		for (k = 1; k < index; k++) {
			if (knots[k].code == 'L') {
				for (i = k+1; i < index; i++) {
					if (knots[i].code == 'H') break;	/* scan to end of A */
				}
				kend = i;
				istart = -1;
/*				if (i >= index) continue; */
				if (i >= index) break;		/* no more contours (or just one) */
				for (i = i+1; i < index; i++) {
					if (knots[i].code == 'M') istart = i;
					if (knots[i].x == knots[k].x && knots[i].y == knots[k].y &&
						knots[i+1].x == knots[k-1].x && knots[i+1].y == knots[k-1].y &&
						knots[i+1].code == 'L') {
						if (traceflag)
						printf("Rewrite in char %d: %d %d %d %d\n",
						   chrs, knots[k-1].x, knots[k-1].y, knots[k].x, knots[k].y);
						rewritecount++;
/*						dumptables(); */
						indexold = index;
						if (istart < 0) {
							printf("istart not set\n");
							continue;
						}
						changed++;
						count++;
						for (j = i+1; j < index; j++) {
							if (knots[j].code == 'H') break;	/* scan to end of A */
						}
						iend = j;	/* scan to end of (E) */
/*						move up (B) (C) (D) up and out of the way */
/*						movecontour (k, istart, index); */
						movecontour (k, i+1, index);
/*						index += (istart - k); */
						ishift = (index - k);	/* (B)(C)(D) shifted up (index-k) */
						index += (i+1 - k);
						indextemp = index;		/* current end of array */
/*						note: end of (A) == start of moved (E) */
						kcurrent = k;	/* where to append stuff */
/*						move down (E) NOTE: i+1 same as k-1 & drop `H' at end */
/*						movecontour (i+2, indexold-1, k); */
						movecontour (i+2, iend, k);
/*						kcurrent += (indexold-1 - (i+2)); */
						kcurrent += (iend - (i+2));
/*						move down (D) */
/*						movecontour (ishift + istart, ishift + i+1, kcurrent); */
						movecontour (ishift + istart+1, ishift + i+1, kcurrent);
/*						drop the 'M' */
/*						knots[kcurrent].code = 'L';	*/ /* change `M' to `L' */
						kcurrent += (i - istart);
/*						moved down (B) and (C) */
						movecontour (ishift+k+1, ishift+istart, kcurrent);
						kcurrent += (istart-(k+1));
/*						now close the gap */
						movecontour(iend+1, indexold, kcurrent);
						kcurrent += (indexold - (iend+1));
						index = kcurrent;
/*						dumptables(); */
/*						exit(1); */
/*						return; */		/* assume only one such */
						break;			/* deal with one at a time */
					} /* match and 'L' */
				} /* scan i advance */
			} /* code 'L' */
		} /* scan k advance */
		if (changed == 0) break;
	}	/* for (;;) */
/*	dumptables(); */
	return count;
}

/***************************************************************************/

void writetables(FILE *output) {
	int k, first=1, closed=0;

	for (k = 0; k < index; k++) {
		if (knots[k].code == 'M') {
			if (first++ == 0) fprintf(output, "h\n");
			fprintf(output, "%d %d m\n", knots[k].x, knots[k].y);
		}
		else if (knots[k].code == 'L') fprintf(output, "%d %d l\n", knots[k].x, knots[k].y);
		else if (knots[k].code == 'C') fprintf(output, "%d %d %d %d %d %d c\n", 
				knots[k-2].x, knots[k-2].y, knots[k-1].x, knots[k-1].y, knots[k].x, knots[k].y);
		else if (knots[k].code == 'H') {
			if (closed == 0) fprintf(output, "h\n");
			closed++;
			continue;
		}
		else if (knots[k].code == ' ') ;
		else printf("Bad code %c %d (%d %d)\n", knots[k].code, knots[k].code, k, index);
		closed = 0;
	}
	if (closed == 0) fprintf(output, "h\n");
}

void writeinverted(FILE *output) {
	int k, oldcode;

	oldcode = 'M';
	for (k = index -1; k >= 0; k--) {
		if (knots[k].code != ' ') {
			if (oldcode == 'M') fprintf(output, "%d %d m\n", knots[k].x, knots[k].y);
			else if (oldcode == 'L') fprintf(output, "%d %d l\n", knots[k].x, knots[k].y);
			else if (oldcode == 'C') 
				fprintf(output, "%d %d %d %d %d %d c\n", 
					knots[k+2].x, knots[k+2].y, knots[k+1].x, knots[k+1].y, knots[k].x, knots[k].y);
			oldcode = knots[k].code;
		}
	}
	fprintf(output, "h\n");
}

/****************************************************************************/

int fround(double z) {
	if (z > 0.0) return (int) (z+0.5);
	else if (z < 0.0) return - (int) (-z+0.5);
	else return 0;
}

short xstart, ystart;	/* from last `M' seen */

int kstart;				/* from last 'M' seen */

void moveto(void) {
	double fxs, fys;
	short xs, ys;

/*	if (index != 0) */	/* not any more */
/*	if (index != 0 && mergecontours == 0) */
	if (index != 0 && checkrewrite == 0)
		fprintf(errout, "Moveto not at start of charpath\n");
/*	resettables(); */	/* not any more */
/*	if (sscanf(buffer, "%d %d m", &xs, &ys) < 2) */
	if (sscanf(buffer, "%lg %lg m", &fxs, &fys) < 2)
		fprintf(errout, "Bad moveto (char %d): %s", chrs, buffer);
	xs = fround(fxs); ys = fround(fys);
	knots[index].x = xs; knots[index].y = ys; knots[index].code = 'M';
	xstart = xs; ystart = ys; kstart = index;
	index++;
}

void lineto(void) {
	double fxf, fyf;
	short xf, yf;

/*	if (sscanf(buffer, "%d %d l", &xf, &yf) < 2) */
	if (sscanf(buffer, "%lg %lg l", &fxf, &fyf) < 2)
		fprintf(errout, "Bad lineto? (char %d): %s", chrs, buffer);
	xf = fround(fxf); yf = fround(fyf);
/*	check if this retraces previous lineto */
	if (knots[index-1].code == 'L' &&		/* was previous a lineto ? */
		knots[index-2].x == xf && knots[index-2].y == yf) { /* same point */
			index--;					/* just remove previous lineto ! */
			fprintf(errout, "Lineto retrace (char %d): %s ", chrs, buffer);
	}
	else {
		knots[index].x = xf; knots[index].y = yf; knots[index].code = 'L';
		index++;
	}
}

void curveto(void) {
	double fxa, fya, fxb, fyb, fxc, fyc;
	short xa, ya, xb, yb, xc, yc;

/*	if (sscanf(buffer, "%d %d %d %d %d %d c", &xa, &ya, &xb, &yb, &xc, &yc) */
	if (sscanf(buffer, "%lg %lg %lg %lg %lg %lg c",
			   &fxa, &fya, &fxb, &fyb, &fxc, &fyc) 
		< 6) fprintf(errout, "Don't understand: %s", buffer);
	xa = fround(fxa); ya = fround(fya);
	xb = fround(fxb); yb = fround(fyb);
	xc = fround(fxc); yc = fround(fyc);
	knots[index].x = xa; knots[index].y = ya; knots[index].code = ' ';
	index++;
	knots[index].x = xb; knots[index].y = yb; knots[index].code = ' ';
	index++;	
	knots[index].x = xc; knots[index].y = yc; knots[index].code = 'C';
	index++;		
}

void closepath(FILE *output) {
	int k;
	int dx, dy;

/*	If last point not first point, close the subpath */
/*	if (knots[index-1].x != knots[0].x || knots[index-1].y != knots[0].y) { */
	if (knots[index-1].x != xstart || knots[index-1].y != ystart) {
/*		printf("open path (%d)\n", chrs); */
/*		printf(" open path (%d) ", chrs); */
/*		dx = knots[index-1].x - knots[0].x;
		dy = knots[index-1].y - knots[0].y; */
		dx = knots[index-1].x - xstart;			/* ? */
		dy = knots[index-1].y - ystart;			/* ? */
		if (-1 <= dx && dx <= 1 && -1 <= dy && dy <= 1) {
/*			knots[index-1].x = knots[0].x;
			knots[index-1].y = knots[0].y; */
			knots[index-1].x = xstart;			/* just tweak it */
			knots[index-1].y = ystart;			/* just tweak it */
			printf(" slightly open path (%d) ", chrs);
		}
		else {
/*			knots[index].x = knots[0].x; knots[index].y = knots[0].y; */
			knots[index].x = xstart;
			knots[index].y = ystart;
			knots[index].code = 'L';
			index++;
			printf(" wide open path (%d) ", chrs);
		}
	}
/*	check whether last lineto undoes first lineto */
/*	if (knots[index-1].code == 'L' && knots[1].code == 'L' && */
	if (knots[index-1].code == 'L' && knots[kstart+1].code == 'L' &&
/*		knots[index-2].x == knots[1].x  && knots[index-2].y == knots[1].y) */
		knots[index-2].x == knots[kstart+1].x  &&
		knots[index-2].y == knots[kstart+1].y) {
		fprintf(errout, "Lineto retrace at end (char %d): %s ", 
			chrs, buffer);
/*		for (k = 1; k < index; k++) */
		for (k = kstart+1; k < index; k++)
			knots[k-1] = knots[k];	/* move down */
		index = index - 2; /* drop first and last - which cancelled */
/*		knots[0].code = 'M'; */
		knots[kstart].code = 'M';	/* Why bother ? */
/*		if (knots[index-1].x != knots[0].x || knots[index-1].y != knots[0].y) { */
		if (knots[index-1].x != xstart || knots[index-1].y != ystart) {
			fprintf(errout, "\nMajor screw up!  Ends don't match\n");
			fprintf(errout, "%d %d %c <> %d %d %c\n",
					xstart, ystart, 'M',
					knots[index-1].x, knots[index-1].y, knots[index-1].code);
		}
	}

/*	now check clockwise versus anti-clockwise and reverse if needed */
	if (checkrewrite == 0) {
/*		if (windingtables() <= 0 || alwaysreverse) { */
		if (windingtables(0, index) <= 0 || alwaysreverse) {
			writeinverted(output);
			if (verboseflag != 0) putc('-', stdout);
		}
		else {
			if (verboseflag != 0) putc('+', stdout);
			writetables(output);
		}
		resettables();
	}
	else {						/* checkrewrite != 0 */
		knots[index++] = closeknot;
/*		if (splitcontours) {
			removeover();
			if (traceflag) dumptables();
			writetables(output);
			resettables();
		} */
	}
/*	resettables(); */
}

int docharpaths(FILE *output, FILE *input) {
	int rewritecountold, count;

	rewritecount=0;
	if (fgets(buffer, MAXLINE, input) == NULL) {
		return -1;
	}

	for(;;) {					/* loop over characters */
		fputs(buffer, output); 
		if (fgets(buffer, MAXLINE, input) == NULL) {/* char number and width */
			fprintf(errout, "Hit EOF\n");
			return -1;
		}
		if (*buffer == '%') continue;	/* 1993/June/20 */
		*charname = '\0';
		if (sscanf(buffer, "%d %d %% %s", &chrs, &width, charname) < 2) {
			fprintf(errout, "Bad char number and width %s\n", buffer);
			fgets(buffer, MAXLINE, input);
			fprintf(errout, "Next line: %s", buffer);
			return -1;					/* give up ? */
		}
		fputs(buffer, output);
		if (verboseflag != 0) printf("%d: ", chrs);
		if (traceflag != 0) printf("\n");
		resettables();

		for(;;) {				/* scan and process one character */
			if (fgets(buffer, MAXLINE, input) == NULL) {
				if (rewritecount > 0)
					printf("\nFixed %d retracings\n", rewritecount);
				return -1;
			}
			if (*buffer == '%') continue;	/* 1993/June/20 */
			if (strchr(buffer, ']') != NULL) break;
			else if (strchr(buffer, 'm') != NULL) moveto();
			else if (strchr(buffer, 'l') != NULL) lineto();
			else if (strchr(buffer, 'c') != NULL) curveto();
			else if (strchr(buffer, 'h') != NULL) closepath(output);
			else fprintf(errout, "Bad op code (char %d): %s", chrs, buffer);
		}
		if (traceflag) dumptables();
		if (selfclosingflag) {
			count = selfclosing();
			if (count) while (count-- > 0)printf("&");
		}
		if (cleanoverflag) {
			count = cleanoverlaps();
			if (count) while (count-- > 0)printf("!");
			if (traceflag) dumptables();
		}
		for (;;) {
			rewritecountold = rewritecount;
			if (splitcontours) {
				count = removeover();
				if (count) while (count-- > 0)printf("@");
				if (traceflag) dumptables();
			} 
			if (mergecontours) {
				count = removemoreover();
				if (count) while (count-- > 0)printf("$");
				if (traceflag) dumptables();
			}
			if (rewritecount == rewritecountold) break;
		}
		writetables(output);
		resettables();
		if (verboseflag != 0) putc(' ', stdout);
	}
}

/************************************************************************/

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

char *extractfilename(char *pathname) {
	char *s;
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void showusage(char *s) {
	fprintf (errout, "\
Correct usage is:\n\n\
%s [-[v][t][a][c] <file-1> <file-2> ...\n\n", s);
	fprintf (errout, "\
\tv: verbose mode\n\
\tt: tracing mode\n\
\ta: always reverse\n\
\to: check rewrite\n\
\tc: clean over\n\
\ts: split contours\n\
\tm: merge contours\n\
");
}

int main(int argc, char *argv[]) {
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	FILE *input, *output;
	int m, firstarg=1, count=0;

/*	while (*(argv[firstarg]) == '-') */
	while (firstarg < argc && *(argv[firstarg]) == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag = ~verboseflag;
		if (strcmp(argv[firstarg], "-t") == 0) traceflag = ~traceflag;
		if (strcmp(argv[firstarg], "-a") == 0) alwaysreverse = ~alwaysreverse;
		if (strcmp(argv[firstarg], "-o") == 0) checkrewrite = ~checkrewrite;
		if (strcmp(argv[firstarg], "-c") == 0) cleanoverflag = ~cleanoverflag;
		if (strcmp(argv[firstarg], "-s") == 0) splitcontours = ~splitcontours;
		if (strcmp(argv[firstarg], "-m") == 0) mergecontours = ~mergecontours;
		if (strcmp(argv[firstarg], "-?") == 0) ;
		firstarg++;
	}
/*	printf("alwaysreverse %d\n", alwaysreverse); */

	if (argc < firstarg+1) {
		showusage(argv[0]);
		exit(1);
	}
	
	printf("REWIND - program for fixing winding number problems\n");

	for (m = firstarg; m < argc; m++) {
		strcpy(infilename, argv[m]);
		extension(infilename, "out");
		if ((input = fopen(infilename, "r")) == NULL) {
			perror(infilename);
			continue;
		}
		strcpy(outfilename, extractfilename(argv[m]));
		forceexten(outfilename, "unw");
		if (strcmp(outfilename, infilename) == 0) {
			printf("Input same as output \n");
			exit(1);
		}
		if ((output = fopen(outfilename, "w")) == NULL) {
			perror(outfilename);
			exit(3);
		}

		printf("Processing %s\n", infilename);
		fprintf(output, "%%%% %s\n", outfilename);

/*		first two lines, point size, scale, and FontBBox */
		for (;;) {
		fgets(buffer, MAXLINE, input);
		if (*buffer != '%') break;
		}
		fputs(buffer, output);
		for (;;) {
			fgets(buffer, MAXLINE, input);
			if (*buffer != '%') break;
		}
		fputs(buffer, output);	

		docharpaths(output,input);	/* go off to do the real work */
	
		fclose(input);
		if(ferror(output) != 0) {
			perror(outfilename);
			exit(1);
		}
		else fclose(output);
		putc('\n', stdout);
		count++;
	}

	printf("Processed %d outline files\n", count);
	return 0;
}


/* For Euler fonts use rewind -c -s -m *.out */

/* if (strcmp(argv[firstarg], "-c") == 0) cleanoverflag = ~cleanoverflag; */
/* if (strcmp(argv[firstarg], "-s") == 0) splitcontours = ~splitcontours; */
/* if (strcmp(argv[firstarg], "-m") == 0) mergecontours = ~mergecontours; */
