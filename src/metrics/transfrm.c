/* transfrm.c
   Copyright Y&Y 1992
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

#pragma warning(disable:4032)	// conio.h

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <io.h>						/* for mktemp ? */
#include <signal.h>
#include <math.h>
#include <conio.h>

#include "metrics.h"

/* following has to do with rational number approximation */

/* If we want to limit to 16 bit for LaserMaster bug: */

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int debugflag = 0;
int wipeclean = 1;	
int dotsflag = 1;
int quietflag = 0;

int switchcase = 0;		/* switch case of base characters in seac */

int usemoveto = 1;		/* use initial rmoveto for vertical shift m32 */

int magnifyflag = 0;
int xflag = 0;
int yflag = 0;
int skewflag = 0;
int rotateflag = 0;
int transformflag = 0;
int xoffsetflag = 0;
int yoffsetflag = 0;

/* int roundflag = 0; */	/* round all coordinates */
int roundflag = 1;		/* round all coordinates */ /* 1993/Dec/20 */
int roundwidths = 1;	/* round widths and sidebearings */
int horizontal = 1;		/* force escapement to be purly horizontal */
int killhstem = 0;		/* kill all hstem and hstem3 (if m21 != 0) */
int killvstem = 0;		/* kill all vstem and vstem3 (if m12 != 0) */

double magnify = 1.0;
double skew = 0.0;
double rotate = 0.0;

double xscale = 1.0;
double yscale = 1.0;

double pi = 3.141592653;

double defxheight=480.0, defcapheight=680.0;

double xheight, capheight;

int matrixflag=0;	/* non-zero if transform matrix is given */

/* transformation parameters */

double m11=1.0, m12=0.0, m21=0.0, m22=1.0, m31=0.0, m32=0.0;

int forceoutname = 1;	/* don't use input file name for output file name */

char *outfilename = "transfrm.pfa";

int renameflag = 0;

int eexecscan = 0;		/* scan up to eexec before starting */
int charscan = 0;		/* scan up to RD before starting */
int charenflag = 0;		/* non-zero charstring coding instead of eexec */
int decodecharflag = 0;	/* non-zero means also decode charstring */
int binaryflag = 0;		/* input is binary, not hexadecimal */

int extractflag = 0;	/* non-zero => extract info only */

static int hires=0;			/* indicate hybrid font 94/Jan/6 */

/* int seensubrs=0; */			/* how many times seen subrs */

volatile int bAbort = 0;			/* set when user types control-C */

extern int encrypt(FILE *, FILE *);

extern int decrypt(FILE *, FILE *);

char *tempdir="";		/* place to put temporary files */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	 
char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_bak[FILENAME_MAX];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

int wantcpyrght=1;

/* Font merging and normalizing program version */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* char *programversion = "TRANSFRM - permanent coordinate transform - version 1.0"; */

/* char *progname="TRANSFRM"; */

char progname[16]="";

/* char *progversion="1.0"; */
/* char *progversion="1.1"; */
/* char *progversion="1.2"; */
char *progversion="2.0";	/* 98/Dec/25 */

char *copyright = "\
Copyright (C) 1992-1998  Y&Y, Inc. (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1992--1994  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 13986445 */
/* #define COPYHASH 14501591 */
/* #define COPYHASH 5618236 */
/* #define COPYHASH 7657875 */
#define COPYHASH 13388439

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void abortjob(void);

#ifdef IGNORE
static void extension(char *fname, char *ext)  /* supply extension if none present */
{
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

static void forceexten(char *fname, char *ext) /* change extension if one present */
{
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}
#endif

static void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

static void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);	
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

static char line[MAXLINE];		/* buffer for getline */
static char buffer[MAXLINE];	/* buffer for div adjustments */

static int getline(FILE *fp_in, char *line) {
	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	return strlen(line); 
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

struct ratio { /* structure for rational numbers */
	long numer;
	long denom;
};

/* compute good rational approximation to floating point number */
/* assumes number given is positive */

struct ratio rational(double x, long nlimit, long dlimit) {
	long p0=0, q0=1, p1=1, q1=0, p2, q2;
	double s, ds;
	struct ratio res;
	int k;					/* safety counter */

/*	printf("Entering rational %lg %ld %ld\n", x, nlimit, dlimit);  */
	if (x == 0.0) {		/* zero */
		res.numer = 0; res.denom = 1;
		return res;		/* the answer is 0/1 */
	}
	if (x < 0.0) {		/* negative */
		res = rational(-x, nlimit, dlimit);
		res.numer = - res.numer; 
		return res;
	}
	s = x;
	k = 0;
	for(;;) {
		p2 = p0 + (long) s * p1;
		q2 = q0 + (long) s * q1;
/*		printf("%ld %ld %ld %ld %ld %ld %lg\n", p0, q0, p1, q1, p2, q2, s); */
		if (p2 > nlimit || q2 > dlimit || p2 < 0 || q2 <= 0) {
			p2 = p1; q2 = q1; break;
		}
		if ((double) p2 / (double) q2 == x) break;
		ds = s - (double) ((long) s);
		if (ds == 0.0) break;
		p0 = p1; q0 = q1; p1 = p2; q1 = q2;	s = 1/ds;
/*		catch large s that will overflow when converted to long */
		if (s > 1000000000.0 || s < -1000000000.0) break;
		if (k++ > 64) {
			fprintf(stderr, 
		"k %d x %lg s %lg ds %lg p0 %ld q0 %ld p1 %ld q1 %ld p2 %ld q2 %ld\n",
				k, x, s, ds, p0, q0, p1, q1, p2, q2);
			break;
		}
		if (bAbort != 0) abortjob();		/* emergency exit */
	}
	assert(q2 != 0);
	res.numer = p2; res.denom = q2;
	return res;		/* the answer is p2/q2 */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int round (double x) {
	if (x >= 0.0) return (int) (x + 0.5);
	else return - (int) (-x + 0.5);
} */

long round (double x) {
	if (x >= 0.0) return (long) (x + 0.5);
	else return - (long) (-x + 0.5);
}

/* int xmap(int x, int y) {
	if (x == 0 && y == 0) return 0;
	else return round(m11 * x + m21 * y);
} */

/* int ymap(int x, int y) {
	if (x == 0 && y == 0) return 0;
	else return round(m12 * x + m22 * y);
} */

double xmap(double x, double y) {
	double xnew = 0.0;
	if (x != 0.0 || y != 0.0) xnew = (m11 * x + m21 * y);
	return xnew;
}

double ymap(double x, double y) {
	double ynew = 0.0;
	if (x != 0.0 || y != 0.0) ynew = (m12 * x + m22 * y);
	if (usemoveto != 0) ynew += m32;		/* 1993/May/9 */
	return ynew;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void spitnumber (char *line, double z) {
	char *s = line + strlen(line);
	struct ratio zratio;
	long znum, zden;

	if (roundflag != 0) sprintf(s, "%ld ", round(z));
	else {
		zratio = rational(z, (long) NUMLIM, (long) DENLIM);
		znum = zratio.numer; zden = zratio.denom;
		if (zden == 1) sprintf(s, "%ld ", znum);
		else sprintf(s, "%ld %ld div ", znum, zden);
	}
}

void spitnumber2 (char *line, double x, double y) {
	spitnumber(line, x); spitnumber(line, y);
}

void spitnumber4 (char *line, double x, double y, double z, double w) {
	spitnumber2(line, x, y); spitnumber2(line, z, w);
}

void spitnumber6 (char *line, double a, double b, double c, 
				double d, double e, double f) {
	spitnumber2(line, a, b); spitnumber2(line, c, d); spitnumber2(line, e, f);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int moved;		/* non-zero after first moved in outline */
				/* use to adjust character position by adjusting dy */

void sbw (double sbx, double sby, double wx, double wy) {
	double isbx, isby, iwx, iwy;
	int oldroundflag;

	oldroundflag = roundflag;		/* save rounding state */
	roundflag = roundwidths;		/* set rounding for widths and sidebear */
  	isbx = xmap(sbx, sby) - xmap(0.0, 0.0) + m31;
	isby = ymap(sbx, sby) - ymap(0.0, 0.0);
	if (usemoveto == 0) isby = isby + m32;	/* do vertical shift here ? */
	iwx = xmap(wx, wy) - xmap(0.0, 0.0);
	iwy = ymap(wx, wy) - ymap(0.0, 0.0);	
	if (horizontal != 0 && wy == 0.0) iwy = 0.0;
	*line = '\0';
  	if (isby == 0.0 && iwy == 0.0) {
		spitnumber2(line, isbx, iwx);
/*		sprintf(line, "%d %d hsbw\n", isbx, iwx); */
		strcat(line, "h");
	}
	else {
		spitnumber4(line, isbx, isby, iwx, iwy);
/*		sprintf(line, "%d %d %d %d sbw\n", isbx, isby, iwx, iwy); */
	}
	strcat(line, "sbw\n");
	roundflag = oldroundflag;		/* restore rounding state */
	moved = 0;
}

void rmoveto (double dx, double dy) {
	double idx, idy;
	if (usemoveto != 0 && moved == 0) dy = dy + m32;
	idx = xmap(dx, dy) - xmap(0.0, 0.0);
	idy = ymap(dx, dy) - ymap(0.0, 0.0);
	*line = '\0';
	if (idy == 0.0) {
		spitnumber(line, idx);
/*		sprintf(line, "%d hmoveto\n", idx); */
		strcat(line, "h");
	}
	else if (idx == 0.0) {
		spitnumber(line, idy);
/*		sprintf(line, "%d vmoveto\n", idy); */
		strcat(line, "v");
	}
	else {
		spitnumber2(line, idx, idy);
/*		sprintf(line, "%d %d rmoveto\n", idx, idy); */
		strcat(line, "r");
	}
	strcat(line, "moveto\n");
	moved++;
}

void rlineto (double dx, double dy) {
	double idx, idy;
	idx = xmap(dx, dy) - xmap(0.0, 0.0);
	idy = ymap(dx, dy) - ymap(0.0, 0.0);
	*line = '\0';
	if (idy == 0.0) {
		spitnumber(line, idx);
/*		sprintf(line, "%d hlineto\n", idx); */
		strcat(line, "h");
	}
	else if (idx == 0.0) {
		spitnumber(line, idy);
/*		sprintf(line, "%d vlineto\n", idy); */
		strcat(line, "v");
	}
	else {
		spitnumber2(line, idx, idy);
/*		sprintf(line, "%d %d rlineto\n", idx, idy); */
		strcat(line, "r");
	}
	strcat(line, "lineto\n");
}

void rrcurveto (double dx1, double dy1, double dx2, double dy2, double dx3, double dy3) {
	double idx1, idy1, idx2, idy2, idx3, idy3;
	idx1 = xmap(dx1, dy1) - xmap(0.0, 0.0);
	idy1 = ymap(dx1, dy1) - ymap(0.0, 0.0);
	idx2 = xmap(dx1+dx2, dy1+dy2) - xmap(dx1, dy1);
	idy2 = ymap(dx1+dx2, dy1+dy2) - ymap(dx1, dy1);
	idx3 = xmap(dx1+dx2+dx3, dy1+dy2+dy3) - xmap(dx1+dx2, dy1+dy2);
	idy3 = ymap(dx1+dx2+dx3, dy1+dy2+dy3) - ymap(dx1+dx2, dy1+dy2);
	*line = '\0';
	if (idy1 == 0.0 && idx3 == 0.0) {
		spitnumber4(line, idx1, idx2, idy2, idy3);
/*		sprintf(line, "%d %d %d %d hvcurveto\n", idx1, idx2, idy2, idy3); */
		strcat (line, "hv");
	}
	else if (idx1 == 0.0 && idy3 == 0.0) {
		spitnumber4(line, idy1, idx2, idy2, idx3);
/*		sprintf(line, "%d %d %d %d vhcurveto\n", idy1, idx2, idy2, idx3); */
		strcat (line, "vh");
	}
	else {
		spitnumber6(line, idx1, idy1, idx2, idy2, idx3, idy3);
/*		sprintf(line, "%d %d %d %d %d %d rrcurveto\n", 
				idx1, idy1, idx2, idy2, idx3, idy3); */
		strcat (line, "rr");
	}
	strcat(line, "curveto\n");
}

void hstem3 (double ya, double dya, double yb, double dyb, double yc, double dyc) {
	double iya, idya, iyb, idyb, iyc, idyc;
	iya = ymap(0.0, ya);
	idya = ymap(0.0, ya+dya) - ymap(0.0, ya);
	iyb = ymap(0.0, yb);
	idyb = ymap(0.0, yb+dyb) - ymap(0.0, yb);
	iyc = ymap(0.0, yc);
	idyc = ymap(0.0, yc+dyc) - ymap(0.0, yc);
	*line = '\0';
	spitnumber6(line, iya, idya, iyb, idyb, iyc, idyc);
/*	sprintf(line, "%d %d %d %d %d %d hstem3\n",  
		iya, idya, iyb, idyb, iyc, idyc); */
	strcat(line, "hstem3\n");
}

void vstem3 (double xa, double dxa, double xb, double dxb, double xc, double dxc) {
	double ixa, idxa, ixb, idxb, ixc, idxc;
	ixa = xmap(xa, 0.0);
	idxa = xmap(xa+dxa, 0.0) - xmap(xa, 0.0);
	ixb = xmap(xb, 0.0);
	idxb = xmap(xb+dxb, 0.0) - xmap(xb, 0.0);
	ixc = xmap(xc, 0.0);
	idxc = xmap(xc+dxc, 0.0) - xmap(xc, 0.0);
	*line = '\0';
	spitnumber6(line, ixa, idxa, ixb, idxb, ixc, idxc);
/*	sprintf(line, "%d %d %d %d %d %d vstem3\n", 
		 ixa, idxa, ixb, idxb, ixc, idxc); */
	strcat(line, "vstem3\n");
}

void hstem (double y, double dy) {
	double iy, idy;
	iy = ymap(0, y);
	idy = ymap(0, y+dy) - ymap(0, y);
	*line = '\0';
	spitnumber2(line, iy, idy);
/*	sprintf(line, "%d %d hstem\n", iy, idy); */
	strcat(line, "hstem\n");
}

void vstem (double x, double dx) {
	double ix, idx;
	ix = xmap(x, 0);
	idx =xmap(x+dx, 0) - xmap(x, 0);
	*line = '\0';
	spitnumber2(line, ix, idx);
/*	sprintf(line, "%d %d vstem\n", ix, idx); */
	strcat(line, "vstem\n");
}

void seac (double asb, double adx, double ady, int bchar, int achar) {
	double iasb, iadx, iady;
	int oldroundflag;
	char *s;

	oldroundflag = roundflag;		/* save rounding state */
	roundflag = roundwidths;		/* set rounding for widths and sidebear */
  	iasb = xmap(asb, 0.0) - xmap(0.0, 0.0) + m31;
	roundwidths = roundflag;		/* restore rounding state */
	*line = '\0';
	spitnumber(line, iasb);

	iadx = xmap(adx, ady + xheight) - xmap(0.0, 0.0);
	iady = ymap(adx, ady + xheight) - ymap(0.0, 0.0);
	spitnumber2(line, iadx, iady - xheight);

	if (switchcase != 0) {
		if (bchar >= 'A' && bchar <= 'Z') bchar = bchar + 'a' - 'A';
		else if (bchar >= 'a' && bchar <= 'z') bchar = bchar + 'A' - 'a';
/*		else if (bchar == 245) bchar = 'i';	 */
/*		else if (bchar == 'I') bchar = 245;	 */
/*		putc(bchar, stdout);	*/		/* debugging */
	}
	s = line + strlen(line);
	sprintf(s, "%d %d ", bchar, achar);
	strcat(line, "seac\n");
/*	fputs(line, stdout); */			/* debugging */
}

/* flx is the FlxProc control height usually 50 (0.5 pixle height) */
/* dx and dy are the final coordinates absolute relative to char origin */

/* void flexsubr (int flex, int dx, int dy) { */	/* 1994/June/21 */
void flexsubr (double flex, double dx, double dy) {	/* 1994/June/21 */
	double idx, idy;
	*line = '\0';							/* make line empty */
	idx = xmap(dx, dy) - xmap(0.0, 0.0);
	idy = ymap(dx, dy) - ymap(0.0, 0.0);
/*	sprintf(line, "%d ", flex); */
	spitnumber(line, flex);					/* 1994/June/27 */
	spitnumber2(line, idx, idy);
	strcat (line, "0 callsubr\n");
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following revised 1994/June/21 - copy new code to SAFESEAC and PFATOAFM */

void checkfordiv (char *line) {
	char *s, *t;
/*	long num, den, ratio; */
	double num, den, ratio;
	int k=0;

/*	fputs(line, stdout); */				/* debugging */
	while ((s = strstr(line, "div")) != NULL) {
		t = s + 3;							/* remember where */
		strcpy(buffer, t);					/* save the tail of this */
/*		printf("s = line + %d ", s - line); */	/* debugging */
/*		while (*s-- > ' ' && s > line) ; */	/* go back to space */
		while (*s > ' ' && s > line) s--;	/* go back to space */
/*		while (*s-- <= ' ' && s > line) ; */	/* go back over space */
		while (*s <= ' ' && s > line) s--;	/* go back over space */
		if (s == line) fprintf(stderr, "Don't understand %s", line);
/*		while (*s-- > ' ' && s > line) ; */	/* go back to space */
		while (*s > ' ' && s > line) s--;	/* go back to space */
/*		while (*s-- <= ' ' && s > line) ; */	/* go back over space */
		while (*s <= ' ' && s > line) s--;	/* go back over space */
/*		while (*s-- > ' ' && s > line) ; */	/* go back to space */
		while (*s > ' ' && s > line) s--;	/* go back to space */
/*		if (s < line) s = line; */				/* 1993/Dec/19 */
/*		if (s > line) s++; */					/* step forward to space */
/*		if (s > line) s++; */					/* step forward past space */
		while (*s <= ' ') s++;				/* step forward again */
/*		printf("s = line + %d ", s - line); */	/* debugging */
/*		if (sscanf (s, "%ld %ld div", &num, &den) == 2) { */
		if (sscanf (s, "%lg %lg div", &num, &den) == 2) {
/*			if (num > 0) ratio = (num + den/2 - 1) / den;
			else ratio = - (-num + den/2 - 1) / den; */
			ratio = num / den; 		/* should be 16 bit at this point */
/*			sprintf(s, "%ld", ratio); */
			sprintf(s, "%lg", ratio);
		}
		else {
			fprintf(stderr, "Can't make sense of: %s", line); /* 1993/Dec/20 */
			break;
		}
/*		(void) memmove (s + strlen(s), t, strlen(t)+1); */
		strcat (s, buffer);					/* put back the tail end */
		if (debugflag != 0) {
			printf("num %lg den %lg ratio %lg: ", num, den, ratio);
			fputs(line, stdout);				
		}
		if (bAbort != 0) abortjob();
		if (k++ > 16) break;				/* emergency exit */
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int checkline(void) {
/*	int sbx, sby, wx, wy; */
	double sbx, sby, wx, wy;
/*	int x, y, dx, dy; */
	double x, y, dx, dy;
/*	int xa, ya, dxa, dya, xb, yb, dxb, dyb, xc, yc, dxc, dyc; */
	double xa, ya, dxa, dya, xb, yb, dxb, dyb, xc, yc, dxc, dyc;
/*	int dx1, dy1, dx2, dy2, dx3, dy3; */
	double dx1, dy1, dx2, dy2, dx3, dy3;
	double asb, adx, ady;
	int bchar, achar;
	int flag=0;
/*	int flex, finx, finy; */
	double flex, finx, finy;	/* 94/June/21 */
	int nsubr;

/*	see whether something needs to be modified */
/*	ignoring `endchar' `closepath' `dotsection' */
/*	ignoring `callsubr' `callothersubr' `pop' `return' `setcurrentpoint' */
/*	problems with FlexProc ??? */ /* ffff xxxx yyyy 0 callsubr */
	if (strstr(line, "endchar") != NULL) return 0;
	if (strstr(line, "closepath") != NULL) return 0;
/*	if (strstr(line, "callsubr") != NULL) return 0; */ /* 1994/June/21 */
	if (strstr(line, "callothersubr") != NULL) return 0;
	if (strstr(line, "dotsection") != NULL) return 0;
	if (strstr(line, "pop") != NULL) return 0;
	if (strstr(line, "return") != NULL) return 0;
	if (strstr(line, "setcurrentpoint") != NULL) return 0;
/*  first check for `div' */
	checkfordiv(line);
/*	the lineto's */
	if (strstr(line, "hlineto") != NULL) {
		if (sscanf (line, "%lg", &dx) == 1) rlineto (dx, 0.0);
		else fputs(line, stderr);
	}
	else if (strstr(line, "vlineto") != NULL) {
		if (sscanf (line, "%lg", &dy) == 1) rlineto (0.0, dy);
		else fputs(line, stderr);
	}
	else if (strstr(line, "rlineto") != NULL) {
		if (sscanf (line, "%lg %lg", &dx, &dy) == 2) rlineto (dx, dy);
		else fputs(line, stderr);
	}
/*	now for the curveto's */
	else if (strstr(line, "hvcurveto") != NULL) {
		if (sscanf (line, "%lg %lg %lg %lg", &dx1, &dx2, &dy2, &dy3) == 4) 
			rrcurveto (dx1, 0.0, dx2, dy2, 0.0, dy3);
		else fputs(line, stderr);
	}
	else if (strstr(line, "vhcurveto") != NULL) {
		if (sscanf (line, "%lg %lg %lg %lg", &dy1, &dx2, &dy2, &dx3) == 4) 
			rrcurveto (0.0, dy1, dx2, dy2, dx3, 0.0);
		else fputs(line, stderr);
	}
	else if (strstr(line, "rrcurveto") != NULL) {
		if (sscanf (line, "%lg %lg %lg %lg %lg %lg", 
			&dx1, &dy1, &dx2, &dy2, &dx3, &dy3) == 6) 
				rrcurveto (dx1, dy1, dx2, dy2, dx3, dy3);
		else fputs(line, stderr);		
	}
	else if (strstr(line, "hstem3") != NULL) {
		if (sscanf (line, "%lg %lg %lg %lg %lg %lg", 
			&ya, &dya, &yb, &dyb, &yc, &dyc) == 6) 
				hstem3(ya, dya, yb, dyb, yc, dyc);
		else fputs(line, stderr);
		if (killhstem != 0) flag++;
	}
	else if (strstr(line, "vstem3") != NULL) {
		if (sscanf (line, "%lg %lg %lg %lg %lg %lg", 
			&xa, &dxa, &xb, &dxb, &xc, &dxc) == 6) 
				vstem3(xa, dxa, xb, dxb, xc, dxc);
		else fputs(line, stderr);
		if (killvstem != 0) flag++;
	}
	else if (strstr(line, "hstem") != NULL) {
		if (sscanf (line, "%lg %lg", &y, &dy) == 2) hstem (y, dy);
		else fputs(line, stderr);
		if (killhstem != 0) flag++;
	}
	else if (strstr(line, "vstem") != NULL) {
		if (sscanf (line, "%lg %lg", &x, &dx) == 2) vstem (x, dx);
		else fputs(line, stderr);
		if (killvstem != 0) flag++;
	}
/*	the side-bearing and width setting code */
	else if (strstr(line, "hsbw") != NULL) {
		if (sscanf (line, "%lg %lg", &sbx, &wx) == 2) 
			sbw (sbx, 0.0, wx, 0.0);
		else fputs(line, stderr);
	}
	else if (strstr(line, "sbw") != NULL) {
		if (sscanf (line, "%lg %lg %lg %lg", &sbx, &sby, &wx, &wy) == 4) 
			sbw (sbx, sby, wx, wy);
		else fputs(line, stderr);
	}
/*	now for the moveto's */
	else if (strstr(line, "hmoveto") != NULL) {
		if (sscanf (line, "%lg", &dx) == 1) rmoveto (dx, 0.0);
		else fputs(line, stderr);
	}
	else if (strstr(line, "vmoveto") != NULL) {
		if (sscanf (line, "%lg", &dy) == 1) rmoveto (0.0, dy);
		else fputs(line, stderr);
	}
	else if (strstr(line, "rmoveto") != NULL) {
		if (sscanf (line, "%lg %lg", &dx, &dy) == 2) rmoveto (dx, dy);
		else fputs(line, stderr);
	}
	else if (strstr(line, "seac") != NULL) {
/*		fputs(line, stdout); */			/* debugging */
		if (sscanf (line, "%lg %lg %lg %d %d", 
			&asb, &adx, &ady, &bchar, &achar) == 5) 
				seac (asb, adx, ady, bchar, achar);
		else fputs(line, stderr);
	}
	else if (strstr(line, "callsubr") != NULL) {	/* 1994/June/21 */
/*     if (sscanf (line, "%d %d %d %d callsubr", &flex, &finx, &finy, &nsubr)*/
		if (sscanf (line, "%lg %lg %lg %d callsubr", &flex, &finx, &finy, &nsubr) 
			< 4) return 0;	/* not a FlxProc call */
		else {
			if (nsubr == 0) flexsubr (flex, finx, finy);
			else fputs(line, stderr);
		}
	}
/*	otherwise we don't know what it is ! */
	else fputs(line, stderr);
	return flag;
}

int scansubrs (FILE *fp_out, FILE *fp_in) {
	int n, subrnum, subrlen;
	char *s;
	int count=0;
/*	seensubrs++; */
/*	scan all of the Subrs */
	for (;;) {
		n = getline(fp_in, line);
		if (n <= 0) break;
		if (strstr(line, "hires") != NULL) hires++;	/* 94/Jan/6 */
		if (hires == 0) {
			if (strchr(line, '/') != NULL) break;
			if (strstr(line, "|-") != NULL) break;
			if (strstr(line, "ND") != NULL) break;
			if (strstr(line, "noaccess def") != NULL) break; /* 1993/Dec/20 */
		}
		else {
			if (strstr(line, "hires not") != NULL) break;	/* 1993/Jan/6 */
			if (strstr(line, "end noaccess put") != NULL) break;
		}
		if (strstr(line, "CharString") != NULL) break;		
		if (strstr(line, "end") != NULL &&
			strstr(line, "endchar") == NULL) break;	/* 1993/Dec/20 */
		if (extractflag == 0) fputs(line, fp_out);
		if ((s = strstr(line, "dup")) != NULL) {	/* start of a Subr */
			(void) sscanf(s, "dup %d %d", &subrnum, &subrlen);
			if (traceflag != 0) fputs(line, stdout);
			count++;
			if (dotsflag != 0) putc(':', stdout);
			while ((n = getline(fp_in, line)) != 0 && 	
				strstr(line, "NP") == NULL &&
					strchr(line, '|') == NULL) {
				if (strstr(line, "return") != NULL) break; /* not robust */
				if (checkline() == 0) {
					if (extractflag == 0) fputs(line, fp_out);
				}
				if (bAbort != 0) abortjob();	/* 1992/Dec/6 */
			}
			if (extractflag == 0) fputs(line, fp_out); 
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */		
	}		
/*	return 0; */
	return count;
}

int scanchars (FILE *fp_out, FILE *fp_in) {
	int n;
	char *s;
	int count = 0;
	char charname[CHARNAME_MAX];
/*	scan all of the CharStrings */
	for (;;) {
		n = getline(fp_in, line);
		if (n <= 0) break;
		if (strstr(line, "hires") != NULL) hires++;	/* 94/Jan/6 */
		if (hires == 0) {
			if (strchr(line, '/') == NULL &&
				strstr(line, "endchar") == NULL && 
					strstr(line, "end") != NULL) break;
		}
		else {							/* hybrid font case  94/Jan/6 */
			if (strncmp(line, "mark ", 5) == 0) break;	  /* 94/Jan/6 */
			if (strncmp(line, "00000000", 8) == 0) break; /* 94/Jan/6 */
		}
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;

		if (extractflag == 0) fputs(line, fp_out);

		if ((s = strchr(line, '/')) != NULL) {	/* start of CharString */
			if (s > line && *(s-1) > ' ') {			/* 94/Jan/6 hires */
/*				if (extractflag == 0) fputs(line, fp_out); */
				continue;
			}
			(void) sscanf(s, "/%s ", charname);
			if (traceflag != 0) fputs(line, stdout);
			if (dotsflag != 0) putc('.', stdout);
			count++;
			while ((n = getline(fp_in, line)) != 0 && 	
				strstr(line, "ND") == NULL &&
					strstr(line, "|-") == NULL) {
				if (strstr(line, "endchar") != NULL) break; /* not robust */
				if (checkline() == 0) {
					if (extractflag == 0) fputs(line, fp_out);
				}
				if (bAbort != 0) abortjob();	/* 1992/Dec/6 */
			}
			if (extractflag == 0) fputs(line, fp_out);
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */		
	}		
/*	return 0;*/
	return count;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* extractflag is non-zero if only extracting old information */ 

/* crude estimate of xheight and capheight */

int estimateheight (void) {
	double ylow, yhigh;
	double dx, dc;
	int n;
	char *s;

	xheight = 0.0; capheight = 0.0;
	if ((s = strstr(line, "BlueValues")) == NULL) return 0;
	if ((s = strchr(s, '[')) == NULL) return 0;
	s++;
	while (sscanf(s, "%lg %lg %n", &ylow, &yhigh, &n) == 2) {
		if (xheight > defxheight) dx = xheight - defxheight;
		else dx = defxheight - xheight;
		if (defxheight - dx < ylow && ylow < defxheight + dx) 
			xheight = ylow;
		if (capheight > defcapheight) dc = capheight - defcapheight;
		else dc = defcapheight - capheight; 
		if (defcapheight - dc < ylow && ylow < defcapheight + dc)
			capheight = ylow;
/*		printf("dx %lg dc %lg\n", dx, dc); */
		s = s + n;
	}
/*	fputs(s, stdout); */
/*	if (traceflag != 0)  */
		printf("Capheight %lg XHeight %lg\n", capheight, xheight);
	return 0;
}

int transform(FILE *fp_out, FILE *fp_in, int extractflag) {
	int n;
	int unknowns=0;
	
	hires=0;										/* 94/Jan/6 */
/*	seensubrs=0; */
	xheight = 453.0; capheight = 750.00;	/* default */
/*  scan up to eexec encrypted part */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
		if (extractflag == 0) fputs(line, fp_out);
		if (bAbort != 0) abortjob();	/* 1992/Dec/6 */
	}
	if (extractflag == 0) fputs(line, fp_out);

/*	scan up to Subrs */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "/Subrs") == NULL) {
		if (extractflag == 0) fputs(line, fp_out);
		if (strstr(line, "BlueValues") != NULL) estimateheight();
		if (strstr(line, "hires") != NULL) hires++;	/* 94/Jan/6 */
		if (bAbort != 0) abortjob();
	}
	if (extractflag == 0) fputs(line, fp_out);
	if (scansubrs(fp_out, fp_in) == 0) 
		fprintf(stderr, "No Subrs?\n");
	if (bAbort != 0) abortjob();
/*	if (extractflag == 0) fputs(line, fp_out); */	/* removed 93/Dec/20 */

	if (dotsflag != 0) putc('\n', stdout);
/*	scan up to CharStrings */	/* old version */
/*	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "/CharStrings") == NULL) {
		if (extractflag == 0) fputs(line, fp_out);
		if (bAbort != 0) abortjob();	
	} */
	while (strstr(line, "/CharStrings") == NULL) {	/* revised 93/Dec/20 */
		if (extractflag == 0) fputs(line, fp_out);
		if (strstr(line, "hires") != NULL) hires++;	/* 94/Jan/6 */
		if (bAbort != 0) abortjob();	
		if ((n = getline(fp_in, line)) == 0) break;
	}
	if (extractflag == 0) fputs(line, fp_out);
	if (scanchars(fp_out, fp_in) == 0)
		fprintf(stderr, "ERROR: No CharStrings?\n");
	if (bAbort != 0) abortjob();

	if (extractflag == 0) {		/* copy the tail across */
		if (n != 0) fputs(line, fp_out);
		while ((n = getline(fp_in, line)) != 0)	{
			fputs(line, fp_out);
			if (bAbort != 0) abortjob();
		}
	}
	if (dotsflag != 0 || unknowns > 0) putc('\n', stdout);

	return 0;
}

char *gettoken(FILE *fp_in) {
	char *t;
	t = strtok (NULL, " \t\n\r\f[]");
	if (t == NULL) {
		if (getline(fp_in, line) == 0) return NULL;	/* EOF */
		t = strtok (line, " \t\n\r\f[]");
	}
	return t;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	if (quietflag != 0) exit(1);
	fprintf(stderr, "%s [-{v}{a}{w}{H}{V}]\n", s);
	fprintf(stderr, "\t[-m=<magnification>] [-x=<x-scale>] [-y=<y-scale>]\n");
	fprintf(stderr, "\t[-s=<skew-angle>] [-r=<rotate-angle>]\n");
/*	fprintf(stderr, "\t[-f=<m11>,<m12>,<m21>,<m22>,<m31>,<m32>]\n); */
	fprintf(stderr, "\t[-f=<m11>,<m12>,<m21>,<m22>]\n");
	fprintf(stderr, "\t<pfa-file>\n");
	if (detailflag == 0) exit(1);
	fprintf(stderr, "\tv verbose mode\n");
/*	fprintf(stderr, "\ta round outline and hint coordinates\n"); */
	fprintf(stderr, "\tw do NOT round widths and sidebearings\n");
	fprintf(stderr, "\ta do NOT round outline and hint coordinates\n");
	fprintf(stderr, "\th allow character escapement to have vertical component\n");
	fprintf(stderr, "\tV remove all vertical stem hints (for m21 not zero)\n");
	fprintf(stderr, "\tH remove all horizontal stem hints (for m12 not zero)\n");
	fprintf(stderr, "\tm magnification (isotropic)\n");
	fprintf(stderr, "\tx horizontal scaling\n");
	fprintf(stderr, "\ty vertical scaling\n");
	fprintf(stderr, "\ts skew angle in degrees (anti-clockwise)\n");
	fprintf(stderr, "\tr rotation in degrees (anti-clockwise)\n");
	fprintf(stderr, "\tf transformation matrix (overrides m, x, y, s, r)\n");
	exit(1);
}

void makename(char *fname, char *str) {	/* make unique temporary filename */
    char *s;
    s = strrchr(fname,'\\');			/* search back to directory part */
    if (s == NULL) {
		s = strrchr(fname,':');			/* search back to drive part */
		if (s == NULL) s = fname;
		else s++;
	}
	else s++;
	strcpy(s+2, "XXXXXX");		/* stick in template six characters */
/*    (void) mktemp(fname); */		/* replace template by unique string */
    (void) _mktemp(fname);		/* replace template by unique string */
	forceexten(fname, str);		/* force appropriate extension */
}

FILE *fp_dec=NULL, *fp_pln=NULL, *fp_adj=NULL;

char fn_dec[FILENAME_MAX], fn_pln[FILENAME_MAX], fn_adj[FILENAME_MAX];

void wipefile (char *name) {
	FILE *fp;
	long length, n;
	int k;
	
	if ((fp = fopen(name, "r+b")) == NULL) return;
	fseek (fp, 0, SEEK_END);
	length = ftell(fp);
	fseek (fp, 0, SEEK_SET);
	for (n = 0; n < length; n++) {
		for(;;) {
			k = rand() >> 8;
			if (k == '\n' || k == '\t' || k == '\r') break;
			if (k < ' ') k = k + 'A';
			if (k >= 128) k = k - 128;
			if (k>= ' ' && k < 128) break;
		}
		putc (k, fp);
	}
	fclose (fp);
}

void cleanup(void) {
	if (wipeclean == 0) return;
	if (fp_dec != NULL) {
		fclose(fp_dec);						/* close output */
/*		fp_dec = fopen(fn_dec, "wb");		
		if (fp_dec != NULL) fclose(fp_dec); */
		wipefile (fn_dec);
		if (wipeclean > 0) (void) remove(fn_dec);	/* remove bogus file */
	}
	if (fp_pln != NULL) {
		fclose(fp_pln);				/* close output */
/*		fp_pln = fopen(fn_pln, "wb");	
		if (fp_pln != NULL) fclose(fp_pln); */
		wipefile (fn_pln);
		if (wipeclean > 0) (void) remove(fn_pln); /* remove bogus file */
	}
	if (fp_adj != NULL) {
		fclose(fp_adj);				/* close output */
/*		fp_adj = fopen(fn_adj, "wb");	
		if (fp_adj != NULL) fclose(fp_adj); */
		wipefile (fn_adj);
		if (wipeclean > 0) (void) remove(fn_adj); /* remove bogus file */
	}
}

void abortjob() {
	fprintf(stderr, "\nUser Interrupt - Exiting\n"); 
	cleanup();
	if (renameflag != 0) rename(fn_in, fn_bak);
	exit(3);
}

/*	problem:        wants void (__cdecl *)(int); */
/*	gets #define SIG_IGN (void (__cdecl *)(int))1 */

#undef SIG_IGN
#define SIG_IGN (void (__cdecl *)(int))1L

void ctrlbreak(int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
/*	fprintf(stderr, "\nUser Interrupt - Exiting\n");  */
/*	cleanup(); */
/*	if (renameflag != 0) rename(fn_in, fn_bak); */
	if (bAbort++ >= 15) exit(3);			/* emergency exit */
	(void) signal(SIGINT, ctrlbreak); 
}

int decodeflag (int c) {
	switch(c) { 
		case 'v': verboseflag++; dotsflag = 1; return 0;
		case 't': traceflag = 1; return 0;
/*		case 'a': roundflag = 1; return 0; */
		case 'a': roundflag = 0; return 0;		/* 1993/Dec/20 */
		case 'h': horizontal = 0; return 0;
		case 'w': roundwidths = 0; return 0;
		case 'l': switchcase = 1; return 0;
		case 'H': killhstem = 1; return 0;
		case 'V': killvstem = 1; return 0;		
		case '?': detailflag++; return 0;
		case 'd': wipeclean++; return 0;
		case 'o': usemoveto = 0; return 0;
		case 'm': magnifyflag = 1; return -1;	/* needs argument */
		case 'x': xflag = 1; return -1;			/* needs argument */
		case 'y': yflag = 1; return -1;			/* needs argument */
		case 's': skewflag = 1; return -1;		/* needs argument */
		case 'r': rotateflag = 1; return -1;	/* needs argument */
		case 'f': transformflag = 1; return -1;	/* needs argument */
		case 'X': xoffsetflag = 1; return -1;
		case 'Y': yoffsetflag = 1; return -1;		
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
}

/* void addslash(char *name) {
	int n;
	n = strlen(name);
	if (n == 0) return;
	if (*(name + n - 1) != '\\') strcat(name, "\\");
} */

/* Modified 1993/June/21 to complain if TEMP env var set wrong */

void maketemporary (char *new, char *name, char *ext) {
	int n;
	FILE *test;
	
	strcpy(new, tempdir);
	n = strlen(new);
/* 	if (n > 0 && *(name + n - 1) != '\\') strcat(new, "\\"); */
	if (n > 0 && *(new + n - 1) != '\\') strcat(new, "\\");
	strcat(new, name);		
	makename(new, ext);
/*	Try it!  See if can write to this place! */
	if ((test = fopen(new, "wb")) == NULL) {
		fprintf(stderr, "WARNING: Temporary directory `%s' does not exist\n",
			tempdir);
		tempdir = "";
		strcpy(new, name);		
		makename(new, ext);
	}
	else fclose(test);
}

/* Sep 27 1990 => 1990 Sep 27 */

void scivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 7);
	for (k = 5; k >= 0; k--) date[k+5] = date[k];
/*	date[11] = '\0'; */
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	
	(void) _getch(); 
	return hash;
}

char *progfunction = "permanent coordinate transform";

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);	 
/*	fprintf(output, "%s (%s %s)\n", programversion, date, compiletime); */
	fprintf(output, "%s - %s - version %s (%s %s)\n", 
		progname, progfunction, progversion, date, compiletime);
}

int decodearg(char *command, char *next, int firstarg) {
	char *s;
	char *sarg=command;
	int c, n;
	
	if (*sarg == '-' || *sarg == '/') sarg++;	/* step over `-' or `/' */
	while ((c = *sarg++) != '\0') {				/* until end of string */
		if (decodeflag(c) != 0) {				/* flag requires argument ? */
/*			if ((s = strchr(sarg, '=')) == NULL) { */
			if (*sarg != '=' && *sarg != ':') {	/* arg in same string ? */
				if (next != NULL) {
					firstarg++; s = next;	/* when `=' or `:' is NOT used */
				}
				else {
					fprintf(stderr, "Don't understand: %s\n", command);
					return firstarg;
				}
			}
/*			else s++;	*/		/* when `=' IS used */
			else s = sarg+1;	/* when `=' or `:' IS used */

/* now analyze the various flags that could have gotten set */
			if (magnifyflag != 0) {
				if (sscanf(s, "%lg", &magnify) < 1)
					fprintf(stderr, "Don't understand %s\n", s);
				if (magnify > 33.0) magnify = magnify / 1000.0;
				else if (magnify < 0.033) magnify = magnify * 1000.0;
				magnifyflag = 0; 
			}
			if (skewflag != 0) {
				if (sscanf(s, "%lg", &skew) < 1)
					fprintf(stderr, "Don't understand %s\n", s);
				skewflag = 0; 
			}
			if (rotateflag != 0) {
				if (sscanf(s, "%lg", &rotate) < 1)
					fprintf(stderr, "Don't understand %s\n", s);
				rotateflag = 0; 
			}
			if (xflag != 0) {
				if (sscanf(s, "%lg", &xscale) < 1)
					fprintf(stderr, "Don't understand %s\n", s);
				xflag = 0; 
			}
			if (yflag != 0) {
				if (sscanf(s, "%lg", &yscale) < 1)
					fprintf(stderr, "Don't understand %s\n", s);
				yflag = 0; 
			}
			if (xoffsetflag != 0) {
				if (sscanf(s, "%lg", &m31) < 1)
					fprintf(stderr, "Don't understand %s\n", s);
				xoffsetflag = 0; 
			}
			if (yoffsetflag != 0) {
				if (sscanf(s, "%lg", &m32) < 1)
					fprintf(stderr, "Don't understand %s\n", s);
				yoffsetflag = 0; 
			}
			if (transformflag != 0) {
				m11 = 1.0; m12 = 0.0; 
				m21 = 0.0; m22 = 1.0; 
				m31 = 0.0; m32 = 0.0;
				n = sscanf(s, "%lg,%lg,%lg,%lg,%lg,%lg",
					&m11, &m12, &m21, &m22, &m31, &m32);
				if (n < 4) fprintf(stderr, "Don't understand %s\n", s);
				else matrixflag++;
				transformflag = 0;
			}
			break;	/* default - no flag set */
		}
	}
	return firstarg;
}

/*	check command line flags and command line arguments */

int commandline(int argc, char *argv[], int firstarg) {
	int c;
	
	if (argc < 2) showusage(argv[0]);
	c = argv[firstarg][0];
	while (c == '-' || c == '/') {
		firstarg = decodearg(argv[firstarg], argv[firstarg+1], firstarg+1);
		if (firstarg >= argc) break;			/* safety valve */
		c = argv[firstarg][0];
	}
	return firstarg;
}

void uppercase (char *s) {
	int c;
	while ((c = *s) != '\0') {
		if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
		*s = (char) c;
		s++;
	}
}

/* return file name minus path when given fully qualified name */

char *strippath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = strippath(filename);
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
}

int _cdecl main(int argc, char *argv[]) {       /* main program */
    FILE *fp_in, *fp_out;
/*	unsigned int i; */
	int firstarg=1;
	int c, d;
	int m;
	char *s;
	double im11, im12, im21, im22;
	double cost, sint;

	if (strlen(progname) < 16) strcpy(progname, compilefile);
	if ((s = strchr(progname, '.')) != NULL) *s = '\0';
	uppercase(progname);

	strcpy (line, argv[0]);
	uppercase (line);
	if (strstr(line, progname) == NULL) quietflag = 1;

	if (argc < firstarg+1) showusage(argv[0]); 

	firstarg = commandline(argc, argv, 1);

	if (wipeclean == 4) wipeclean = 0;	/* `ddd' on command line */

	if (verboseflag == 3) debugflag++;

	if (argc <= firstarg) showusage(argv[0]); 
		
	if (checkcopyright(copyright) != 0) {
/*		fprintf(stderr, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	if (matrixflag == 0) {
		if (magnify == 1.0 && xscale == 1.0 && yscale == 1.0 &&
				skew == 0.0 && rotate == 0.0) {
			fprintf(stderr, "Identity transformation specified\n");
			exit(1);
		}
		m11 = 1.0; m12 = 0.0; m21 = 0.0; m22 = 1.0;
		if (magnify != 1.0) {
			m11 = m11 * magnify; m12 = m12 * magnify;
			m21 = m21 * magnify; m22 = m22 * magnify;
		}
		if (xscale != 0) {
			m11 = m11 * xscale;	m12 = m12 * xscale;
		}
		if (yscale != 0) {
			m21 = m21 * yscale;	m22 = m22 * yscale;
		}
		if (skew != 0.0) {
			m21 = m21 - m11 * tan(skew * pi / 180.0);
		}
/* this assumes rotation AFTER skew and anistropic scaling */
		if (rotate != 0.0) {
			cost = cos(rotate * pi / 180.0);
			sint = - sin(rotate * pi / 180.0);
			im11 = m11 * cost - m12 * sint;
			im12 = m11 * sint + m12 * cost;
			im21 = m21 * cost - m22 * sint;
			im22 = m21 * sint + m22 * cost;
			m11 = im11; m12 = im12; m21 = im21; m22 = im22;
		}
/*		m31 = 0.0;	m32 = 0.0; */		/* removed 1993/May/9 */
	}

	if (quietflag != 0) {
		verboseflag = 0; traceflag = 0; dotsflag = 0;
	}

/*	scivilize(compiledate);	 */
	if (quietflag == 0) stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	(void) signal(SIGINT, ctrlbreak);

	if ((s = getenv("TMP")) != NULL) tempdir = s;
	if ((s = getenv("TEMP")) != NULL) tempdir = s;

	extractflag = 0;		/* default is not to just extract old info */

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

	if (verboseflag != 0)
		printf("Transformation matrix: [%lg %lg %lg %lg %lg %lg]\n", 
			m11, m12, m21, m22, m31, m32);

	if (m21 != 0.0 && killvstem == 0)
		printf("Maybe use `V' command line flag  (since m21 non-zero)\n");
	if (m21 == 0.0 && killvstem != 0)
		printf("WARNING: Killing vstem hints, yet m21 is zero\n");

	if (m12 != 0.0 && killhstem == 0)
		printf("Maybe use `H' command line flag (since m12 non-zero)\n");
	if (m12 == 0.0 && killhstem != 0)
		printf("WARNING: Killing hstem hints, yet m12 is zero\n");

	if (verboseflag != 0) {
		if (roundflag != 0) printf("Rounding");
		else printf("Not rounding");
		printf(" outline & hint coordinates -- ");
		if (roundwidths != 0) printf("rounding");
		else printf("not rounding");
		printf(" widths & sidebearings\n");
	}

	m = firstarg;
	strcpy(fn_in, argv[m]);			/* get file name */
	extension(fn_in, "pfa");

	if (verboseflag != 0) printf("Processing file %s\n", fn_in);

	if ((fp_in = fopen(fn_in, "rb")) == NULL) {	/* see whether exists */
		underscore(fn_in);						/* 1993/May/30 */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
			perror(fn_in);	exit(13);
		}
	}
/*	else { */
		c = fgetc(fp_in); d = fgetc(fp_in);
		if (c != '%' || d != '!') {
			fprintf(stderr, "ERROR: This does not appear to be a PFA file\n");
			exit(13);							/* 1993/May/30 */
		}
		fclose(fp_in);
/*	} */

	if ((s=strrchr(fn_in, '\\')) != NULL) s++;
	else if ((s=strrchr(fn_in, ':')) != NULL) s++;
	else s = fn_in;
	strcpy(fn_out, s);			/* copy input file name minus path */

	if (forceoutname != 0) strcpy(fn_out, outfilename);

	if (strcmp(fn_in, fn_out) == 0 && extractflag == 0) {
		strcpy(fn_bak, fn_in);
		forceexten(fn_in, "bak");
		printf("Renaming %s to %s\n", fn_bak, fn_in);
		remove(fn_in);		/* in case backup version already present */
		rename(fn_bak, fn_in);
		renameflag++;
	}
	
	if (verboseflag != 0 && extractflag == 0) 
		printf("Output is going to %s\n", fn_out);

/*	(void) tmpnam(fn_dec); */	/* create temporary file name */
/*	strcpy(fn_dec, fn_out); */
/*	makename(fn_dec, "dec"); */
	maketemporary(fn_dec, fn_out, "dec");
	if (traceflag != 0) printf("Using %s as temporary\n", fn_dec);
/*	strcpy(fn_pln, fn_out); */
/*	makename(fn_pln, "pln"); */
	maketemporary(fn_pln, fn_out, "pln");
	if (traceflag != 0) printf("Using %s as temporary\n", fn_pln);
	if (extractflag == 0) {
/*		strcpy(fn_adj, fn_out); */
/*		makename(fn_adj, "adj"); */
		maketemporary(fn_adj, fn_out, "adj");
		if (traceflag != 0) printf("Using %s as temporary\n", fn_adj);
	}

	if (traceflag != 0) printf("Pass A (down) starting\n");			/* */
	
	if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
		underscore(fn_in);						/* 1993/May/30 */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) {	
			perror(fn_in); exit(2);
		}
	}
	if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
		perror(fn_dec); exit(2);
	}

	eexecscan = 1;
	charscan = 0;  decodecharflag = 0;  charenflag = 0; binaryflag = 0;

	(void) decrypt(fp_dec, fp_in);		/* actually go do the work */

	fclose(fp_in);
	if (ferror(fp_dec) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_dec); cleanup();	exit(2);
	}
	else fclose(fp_dec);

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	
	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass B (down) starting\n");			/* */
		
	if ((fp_dec = fopen(fn_dec, "rb")) == NULL) { 
		perror(fn_dec); cleanup(); exit(2);
	}
	if ((fp_pln = fopen(fn_pln, "wb")) == NULL) { 
		perror(fn_pln); cleanup(); exit(2);
	}

	eexecscan = 0;
	charscan = 1;  decodecharflag = 1;  charenflag = 1;  binaryflag = 1;

	(void) decrypt(fp_pln, fp_dec);		/* actually go do the work */

	fclose(fp_dec);
	if (ferror(fp_pln) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_pln); cleanup();
		exit(2);
	}
	else fclose(fp_pln);

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	
	if (debugflag != 0) _getch();
	
	if (extractflag != 0) {		/* no need to go upward */
		if ((fp_pln = fopen(fn_pln, "rb")) == NULL) {
			perror(fn_pln); cleanup(); exit(33);
		}
		forceexten(fn_out, "xxx");
		if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
			perror(fn_out); cleanup();	exit(2);
		}

#ifdef DEBUGFLUSH
		setbuf(fp_out, NULL); 
#endif

		(void) transform(fp_out, fp_pln, 1);	/* extract side bearings */

		fclose(fp_pln);
/*		if (wipeclean > 0) remove(fn_pln);  */
		if (ferror(fp_out) != 0) {
			fprintf(stderr, "Output error ");
			perror(fn_out); cleanup(); exit(2);
		}
		else fclose(fp_out);
		cleanup();
		return 0;
	}

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	
	if (traceflag != 0) printf("Transforming!\n"); 

	if ((fp_pln = fopen(fn_pln, "rb")) == NULL) { 
		perror(fn_pln);  cleanup(); exit(2);
	}
	if ((fp_adj = fopen(fn_adj, "wb")) == NULL) { 
		perror(fn_adj); cleanup(); exit(2);
	}

	(void) transform(fp_adj, fp_pln, 0);
		
	fclose(fp_pln);
/*	if (wipeclean > 0) remove(fn_pln);  */
	if (ferror(fp_adj) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_adj); cleanup(); exit(2);
	}
	else fclose(fp_adj);

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	
	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass B (up) starting\n"); 

	if ((fp_adj = fopen(fn_adj, "rb")) == NULL) { 
		perror(fn_adj);  cleanup(); exit(2);
	}
	if ((fp_dec = fopen(fn_dec, "wb")) == NULL) { 
		perror(fn_dec); cleanup(); exit(2);
	}

	eexecscan = 1; charenflag = 1; charscan = 1;

	(void) encrypt(fp_dec, fp_adj);	/* actually go do the work */
		
	fclose(fp_adj);
/*	if (wipeclean > 0) remove(fn_adj);  */
	if (ferror(fp_dec) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_dec); cleanup(); exit(2);
	}
	else fclose(fp_dec);

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	
	if (debugflag != 0) _getch();

	if (traceflag != 0) printf("Pass A (up) starting\n"); 

	if ((fp_dec = fopen(fn_dec, "rb")) == NULL) { 
		perror(fn_out); cleanup();	exit(2);
	}
	if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
		perror(fn_out); cleanup();	exit(2);
	}

	eexecscan = 1; charenflag = 0; charscan = 0;

	(void) encrypt(fp_out, fp_dec);	/* actually go do the work */
		
	fclose(fp_dec);

	if (debugflag != 0) _getch();

/*	if (wipeclean > 0) remove(fn_dec);  */
	if (ferror(fp_out) != 0) {
		fprintf(stderr, "Output error ");
		perror(fn_out); cleanup();	exit(2);
	}
	else fclose(fp_out);

	cleanup();
	return 0;
}	

/* adjust font level hints also ? */

/* adjust FontBBox */

/* how to really use m32 ? for first rmoveto ? */

/* this doesn't deal with synonyms for ND etc */
