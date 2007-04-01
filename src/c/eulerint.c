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

/* Code for scanning Euler MF files and cranking out m, l, c, h codes */
/* specifiy eurmch, eurbch, eusmch, eusbch, eufmch, eufbch on command line */
/* specify point size also if not equal to ten */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXLINE 256
#define MAXNAME 32
#define MAXCHRS 128

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int verboseflag = 0;
int traceflag = 0;
int complainflag = 0;

int noscaling = 0;		/* don't rescale output coordinates - debugging */

int tripledot = -1;		/* implement `...' instead of just `..' */

int allowrotate=1;		/* implement rotate and shift on output */

int allowslant=1;		/* implement slant on input */

int line;				/* input file line number */ 

int chrs;				/* and character number */

char charname[MAXNAME];	/* character name if any */

char buffer[MAXLINE];

int left[MAXCHRS], right[MAXCHRS];		/* side bearings */

int offsets[MAXCHRS];			/* sidebearing offset from endchar */

int widths[MAXCHRS];			/* character widths from charbegin */

double mathcorr[MAXCHRS];		/* math correction */

double ptsize=10.0;

int xll=-50, yll=-200, xur=1000, yur=700;	/* FontBBox */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double programem=3700.0;		/* same for all six font ? */

double skew=0.0;			/* font specific - programem/36 ?*/

double scale=1.0;	/* computed scale used in output = programem/1000.0 */

double xscalefactor=1.0;		/* aspect ratio factor non-unity for small */

double leftside=0.0, rightside=0.0;	/* depend on font ptsize */

double moreside=0.0;	/* to add onto each side of char (from more_side) */
/* 100h for lower case, dotless, Greek, not for punctuation, figures */

double sidebearing;		/* current left sidebearing (from endchar) */

double mathcorrection;	/* math correction (from mathcorr) */

double rotate;			/* some characters are rotated */

double cosrot, sinrot;	/* cosine and sine of above */

double xshift, yshift;	/* some characters are shifted */

double xcenter, ycenter;	/* some rotate about a center point */

double initshift=0.0;	/* vertical shift (from initrot) */

double slant=0.0;		/* slant */

FILE *errout=stdout;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* from eubase.mf

if unknown xscale_factor: xscale_factor := 1; fi
h# = ptsize * xscale_factor / programem;
v# = ptsize / programem;
h = h#*hppp;
v = v#*vppp;

*/

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* left and right side added space and xscale factor */
/* as function of point size */

#define MAXSIZES 6		/* number of point sizes possible */

#define MAXNAMES 6		/* number of font names possible */

/* Following are same for Euler Roman, Script, and Fraktur (medium & bold) */

int ptsizes[MAXSIZES] = {5, 6, 7, 8, 9, 10};
int leftsides[MAXSIZES] = {300, 150, 100, 0, 0, 0};
int rightsides[MAXSIZES] = {300, 150, 100, 0, 0, 0};

/* Following is different for each of the six fonts */

char *fontnames[MAXNAMES] = {"eurm", "eusm", "eufm", "eurb", "eusb", "eufb"};

double xscales[MAXNAMES][MAXSIZES] = {
{1.2, 1.16, 1.13, 1.08, 1.03, 1.0},
{1.2, 1.16, 1.13, 1.09, 1.04, 1.0},
{1.2, 1.14, 1.08, 1.04, 1.02, 1.0},
{1.28, 1.23, 1.2, 1.17, 1.12, 1.1},
{1.28, 1.24, 1.21, 1.18, 1.15, 1.13},
{1.3, 1.25, 1.2, 1.19, 1.18, 1.18}
};

/* for EURM (Euler Roman Medium) */
/* double xscales[MAXSIZES] = {1.2, 1.16, 1.13, 1.08, 1.03; 1.0}; */

/* for EUSM (Euler Script Medium) */
/* double xscales[MAXSIZES] = {1.2, 1.16, 1.13, 1.09, 1.04; 1.0}; */

/* for EUFM (Euler Fraktur Medium) */
/* double xscales[MAXSIZES] = {1.2, 1.14, 1.08, 1.04, 1.02; 1.0}; */

/* for EURB (Euler Roman Bold) */
/* double xscales[MAXSIZES] = {1.28, 1.23, 1.2, 1.17, 1.12; 1.1}; */

/* for EUSB (Euler Script Bold) */
/* double xscales[MAXSIZES] = {1.28, 1.24, 1.21, 1.18, 1.15; 1.13}; */

/* for EUFB (Euler Fraktur Bold) */
/* double xscales[MAXSIZES] = {1.3, 1.25, 1.2, 1.19, 1.18; 1.18}; */

int setupfont(int pt, char *name) {
	int k, l;
	printf("Font: %s size: %d", name, pt);
	ptsize = (double) pt;
	for(k = 0; k < MAXSIZES; k++) {
		if (ptsizes[k] == pt) break;
	}
	if (k == MAXSIZES) {
		fprintf(errout, "Invalid point size %d\n", pt);
		return -1;
	}
	leftside = leftsides[k]; rightside = rightsides[k];
	printf(" --- leftside %lg", leftside);
	printf(" --- rightside %lg", rightside);
	for (l = 0; l < MAXNAMES; l++) {
		if (strcmp(fontnames[l], name) == 0) break;
	}
	if (l == MAXNAMES) {
		fprintf(errout, "Invalid font name %s\n", name);
		return -1;
	}
	xscalefactor = xscales[l][k];
	printf(" --- xscalefactor %lg\n", xscalefactor);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void saywhere(FILE *output) {
	fprintf(output, "(line %d chrs %d) ", line, chrs);
}

void dontunder(void) {
	saywhere(errout);
	fprintf(errout, "Say what? %s",	buffer);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int wmap(double w) {
	if (noscaling) return (int) (floor(w + 0.5));
	return (int) (floor(w /	scale * xscalefactor + 0.5));
}

int xmap(double x, double y) {
	double xd;
/*	double yd; */
	if (noscaling) return (int) (floor(x + 0.5));
	if (allowrotate == 0 || rotate == 0.0)
		return (int) 
			(floor((x + sidebearing + leftside + moreside + xshift) / 
				scale * xscalefactor + 0.5));
	else {
		xd = x * cosrot - y * sinrot + xshift;
/*		yd = x * sinrot + y * cosrot + yshift; */
		return (int) (floor((xd + sidebearing + leftside + moreside) / 
			scale * xscalefactor + 0.5));		
	}
}

int ymap(double x, double y) {
/*	double xd; */
	double yd;
	if (noscaling) return (int) (floor(y + 0.5));
	if (allowrotate == 0 || rotate == 0.0)
		return (int) (floor((y + initshift + yshift) / scale + 0.5));
	else {
/*		xd = x * cosrot - y * sinrot + xshift; */
		yd = x * sinrot + y * cosrot + yshift; 
		return (int) (floor((yd + initshift) / scale + 0.5));
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Hobby's magic mock-optimal constants */

double ha=1.4142136;	/* sqrt(2) */
double hb=0.0625;		/* 1 / 16 */
double hc=0.381966;		/* (3 - sqrt(5)) / 2 */

void setconstants(void) {
  ha = sqrt(2.0);
  hb = 1.0 / 16.0;
  hc = (3.0 - sqrt(5.0)) / 2.0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double dotproduct(double xa, double ya, double xb, double yb) {
  return ( xa * xb + ya * yb );
}

double crossproduct(double xa, double ya, double xb, double yb) {
  return ( xa * yb - xb * ya );
}

/* double anglebetween(double xa, double ya, double xb, double yb) {
  return atan2(crossproduct(xa, ya, xb, yb), dotproduct(xa, ya, xb, yb));
} */

/*
struct direction{
  double x; double y;
}; */

/* struct direction directionbetween(double xa, double ya, 
		double xb, double yb) {
  double rcos, rsin, rad;
  struct direction dir;
  rsin = crossproduct(xa, ya, xb, yb);
  rcos = dotproduct(xa, ya, xb, yb);
  rad = sqrt(rsin * rsin + rcos * rcos);
  if (rad == 0.0) fprintf(errout, "zero length vector");
  dir.x = rcos / rad; dir.y = rsin /rad;
  return dir;
} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double compalpha(double ctheta, double stheta, double cphi, double sphi) {
  return (ha * (stheta - hb * sphi) * (sphi - hb * stheta) * (ctheta - cphi));
}

double comprho(double ctheta, double cphi, double alpha) {
  return ((2.0 + alpha) / (1.0 + (1.0 - hc) * ctheta + hc * cphi));
}

double compsigma(double ctheta, double cphi, double alpha) {
  return ((2.0 - alpha) / (1.0 + (1.0 - hc) * cphi + hc * ctheta));
}

double epsilon=0.045;	/* limit on lack of parallelism 2.5 degrees */

/* double depsilon=1.0; */	/* limit on lack of parallelism */

int parallel(double dx, double dy, double us, double vs) {
	double dotd, dots, cross;
/*	double sdot; */

	dotd = dotproduct(dx, dy, dx, dy); 
	dots = dotproduct(us, vs, us, vs); 
/*	sdot = sqrt(us * us + vs * vs);
	if (sdot == 0.0) {
		saywhere(errout);
		fprintf(errout, "Zero tangent direction\n");	return 1;
	}
	us = us / sdot; vs = vs / sdot; */
	cross = crossproduct(dx, dy, us, vs);
/*	if (cross < 0.0) cross = - cross; */
/*	if (cross < depsilon)  return 1; */
	if (cross * cross < epsilon * epsilon * dotd * dots)  return 1; 
	else return 0;	
}

/* should be more lenient above for very short pieces ? */

int insidetriangle(double xs, double ys, double xa, double ya, 
		double xb, double yb, double xf, double yf) {
	double cscross, cbcross, cfcross, cacross;
	cscross = crossproduct(xa-xs, ya-ys, xf-xs, yf-ys);
	cbcross = crossproduct(xb-xs, yb-ys, xf-xs, yf-ys);
	if ((cscross < 0.0 && cbcross > 0.0) ||
		(cscross > 0.0 && cbcross < 0.0)) return 0;
	cfcross = crossproduct(xb-xf, yb-yf, xs-xf, ys-yf);
	cacross = crossproduct(xa-xf, ya-yf, xs-xf, ys-yf);
	if ((cfcross < 0.0 && cacross > 0.0) ||
		(cfcross > 0.0 && cacross < 0.0)) return 0;	
	return 1;
	

}

void lineto(FILE *output, double xs, double ys, double xf, double yf) {
	int ixs, iys, ixf, iyf;
	ixs = xmap(xs, ys); iys = ymap(xs, ys);
	ixf = xmap(xf, yf); iyf = ymap(xf, yf);	
	if (ixs == ixf && iys == iyf) {
		saywhere(stdout);
		printf("Worthless lineto (%lg %lg %lg %lg)\n", xs, ys, xf, yf);	
	}
	else fprintf(output, "%d %d l\n", ixf, iyf);
}

void curveto(FILE *output, double xs, double ys, double xa, double ya, 
		double xb, double yb, double xf, double yf) {
	int ixs, iys, ixa, iya, ixb, iyb, ixf, iyf;
	ixs = xmap(xs, ys); iys = ymap(xs, ys);
	ixa = xmap(xa, ya); iya = ymap(xa, ya);
	ixb = xmap(xb, yb); iyb = ymap(xb, yb);	
	ixf = xmap(xf, yf); iyf = ymap(xf, yf);	

	if (ixs == ixf && iys == iyf) {
		saywhere(stdout);
		printf("Worthless curveto (%lg %lg %lg %lg %lg %lg %lg %lg)\n", 
			xs, ys, xa, ya, xb, yb, xf, yf);	
	}
	else fprintf(output, "%d %d %d %d %d %d c\n", 
		 ixa, iya, ixb, iyb, ixf, iyf);
}


/* From (xs, ys) to (xf, yf) with tangents (us, vs) and (uf, vf) */

int smoothknots(FILE *output, double xs, double ys, double us, double vs,
		 double xf, double yf, double uf, double vf) {
	 double dx, dy, chord;
	 double xa, ya, xb, yb;
	 double scos, ssin, srad;
	 double fcos, fsin, frad;
	 double tcos, tsin;
	 double rho, sigma, alpha;
	 double rhomax, sigmamax, duv;
	 double ctheta=0.0, stheta=0.0, cphi=0.0, sphi=0.0;
	 double mus=0.0, mvs=0.0, muf=0.0, mvf=0.0;
	 
	 dx = xf - xs;	dy = yf - ys;
	 chord = sqrt(dx * dx + dy * dy);
	 if (chord == 0.0) {
		 saywhere(errout);
		 fprintf(errout, "Zero length vector (dx dy) = (%lg %lg)\n", dx, dy);
		 return 0;
	 }
	 
	 if (uf == us && vf == vs) {   /* if tangent directions exactly equal ? */
		 if (parallel(dx, dy, us, vs) > 0) {
			 if (traceflag != 0) {
				 saywhere(stdout); printf("Made into lineto\n"); 
			 }
			 lineto(output, xs, ys, xf, yf);
/*			 fprintf(output, "%d %d l\n", xmap(xf, yf), ymap(xf, yf)); */
			 return 1;					/* is this OK ? */
		 }
		 else {
 /* if (traceflag != 0) printf(  */
			 saywhere(stdout);
			 printf("Chord (%lg %lg) not parr tang (%lg %lg)\n", 
				 dx, dy, us, vs);
		 }
		 
	 }
	 
	 tcos = dotproduct(us, vs, uf, vf);
	 if (tcos < 0.0) { /* warning about ridiculous turning rate */
		 tsin = crossproduct(us, vs, uf, vf);
		 if ((tsin > 0.0 && tsin < -tcos * 2.0) ||
			 (tsin < 0.0 && -tsin < -tcos * 2.0)) {
			 saywhere(stdout);
			 printf("Rapid turning (%lg %lg) (%lg %lg)\n", us, vs, uf, vf); 
		 }
	 }

	 scos = dotproduct(dx, dy, us, vs);
	 ssin = crossproduct(dx, dy, us, vs);
	 srad = sqrt(ssin * ssin + scos * scos);
	 if (srad == 0.0) {
		 saywhere(errout);
		 fprintf(errout, "Zero Length Vector (us vs) = (%lg %lg)\n", us, vs);
	 }
	 else {
		 ctheta = scos / srad; stheta = ssin / srad;
	 }
	 
	 if (scos < 0.0) { /* warning about ridiculous turning rate */
		 if ((ssin > 0.0 && ssin < -scos * 2.0) ||
			 (ssin < 0.0 && -ssin < -scos * 2.0)) {
			 saywhere(stdout);
			 printf("Bad start tangent (%lg %lg) (%lg %lg)\n",
				 us, vs, dx, dy);
		 }
	 }

	 fcos = dotproduct(dx, dy, uf, vf);
	 fsin = crossproduct(dx, dy, uf, vf);
	 frad = sqrt(fsin * fsin + fcos * fcos);
	 if (frad == 0.0) {
		 saywhere(errout);
		 fprintf(errout, "Zero Length Vector (uf vf) = (%lg %lg)\n", uf, vf);
	 }
	 else {
		 cphi = fcos / frad; sphi = fsin / frad;
	 }
	 
	 if (fcos < 0.0) { /* warning about ridiculous turning rate */
		 if ((fsin > 0.0 && fsin < -fcos * 2.0) ||
			 (fsin < 0.0 && -fsin < -fcos * 2.0)) {
 			 saywhere(stdout);
			 printf("Bad end tangent (%lg %lg) (%lg %lg)\n",
				 uf, vf, dx, dy);
		 }
	 }

	 srad = sqrt(us * us + vs * vs);
	 if (srad == 0.0) {
		 saywhere(errout);
		 fprintf(errout, "Zero Length Vector (us vs) = (%lg %lg)\n", us, vs);
	 }
	 else {
		 mus = us / srad; mvs = vs / srad;
	 }
	 
	 frad = sqrt(uf * uf + vf * vf);
	 if (frad == 0.0) {
		 saywhere(errout);
		 fprintf(errout, "Zero Length Vector (uf vf) = (%lg %lg)\n", uf, vf);
	 }
	 else {
		 muf = uf / frad; mvf = vf / frad;
	 }
	 
	 alpha = compalpha(ctheta, stheta, cphi, sphi);
	 rho = comprho(ctheta, cphi, alpha);
	 sigma = compsigma(ctheta, cphi, alpha);
	 
	 duv = (mus * mvf - muf * mvs) * (chord / 3.0);
	 if (duv != 0.0) {		/* avoid problems if tangent parallel */
		 rhomax = (dx * mvf - dy * muf) / duv;
		 sigmamax = (dy * mus - dx * mvs) / duv;
	 
/*	take care of numerical round off problems */
		 if (rhomax > -0.00001 && rhomax < 0.00001) rhomax = 0.0;
		 if (sigmamax > -0.00001 && sigmamax < 0.00001) sigmamax = 0.0;

		 if (tripledot != 0 && rhomax >= 0.0 && rho > rhomax) {
			 if (complainflag != 0 && rho > 10.0 * rhomax) {
				 saywhere(stdout);
				 printf("rho (%lg) > rhomax (%lg)\n", rho, rhomax); 
			 }
			 rho = rhomax; 
		 }
		 if (tripledot != 0 && sigmamax >= 0 && sigma > sigmamax) {
			 if (complainflag != 0 && sigma > 10.0 * sigmamax) {
				 saywhere(stdout); 
				 printf("sigma (%lg) > sigmamax (%lg)\n", sigma, sigmamax); 
			 }
			 sigma = sigmamax; 
		 }
	 }
	 else {	/* tangents parallel - avoid problems when referencing later */
		 rhomax = rho; sigmamax = sigma;
	 }

	 xa = xs + rho * mus * chord / 3.0;
	 ya = ys + rho * mvs * chord / 3.0;
	 
	 xb = xf - sigma * muf * chord / 3.0;
	 yb = yf - sigma * mvf * chord / 3.0;
	 
	 if (duv != 0 && sigmamax > 0.0 && rhomax > 0.0 &&
		 insidetriangle(xs, ys, xa, ya, xb, yb, xf, yf) == 0) {
		 saywhere(errout);
		 fprintf(errout, "Violates triangle (rho %lg) (sigma %lg) ",
			 rho, sigma);
		 if (duv != 0) {
			 fprintf(errout, "(rhomax %lg) (sigmamax %lg)\n", rhomax, sigmamax);
		 }
		 else fprintf(errout, "tangents parallel\n");
	 }

	 curveto(output, xs, ys, xa, ya, xb, yb, xf, yf);
/*	 fprintf(output, "%d %d %d %d %d %d c\n", 
		 xmap(xa, ya), ymap(xa, ya), xmap(xb, yb), 
			 ymap(xb, yb), xmap(xf, yf), ymap(xf, yf)); */
	 return 0;
}

void replacedigit(char *s) {
	int c = s[0];
	switch(c) {
		case '0': strcpy(s, "zero"); break;
		case '1': strcpy(s, "one"); break; 
		case '2': strcpy(s, "two"); break;
		case '3': strcpy(s, "three"); break;
		case '4': strcpy(s, "four"); break;
		case '5': strcpy(s, "five"); break;
		case '6': strcpy(s, "six"); break;
		case '7': strcpy(s, "seven"); break;
		case '8': strcpy(s, "eight"); break;
		case '9': strcpy(s, "nine"); break;
		default:
			fprintf(errout, "Funny digit %d\n", c); break;
	}
}

int scantoend(FILE *input) { /* scan to end of charpath */

	if (traceflag != 0) printf("Skipping to end of character %d\n", chrs);
	for (;;) {
		if (fgets(buffer, MAXLINE, input) == NULL) {
			fprintf(errout, "EOF\n"); return -1;
		}
		line++;
		if (strstr(buffer, "cycle") != NULL) return 0;
	}
}

double pi = 3.141592653;

/* parse one subpath */

int parseeuler(FILE *output, FILE *input) {
	double xstart, ystart;
	int n, nn, lineflag=0, lineflagn;
	double xs, ys, us, vs, xf, yf, uf, vf;
	double xsn, ysn, usn, vsn, ufn, vfn;
	double w;
	int width, widthbogus, wa, wb, m;
	int slannum=0, slanden=1;
	char *s;

/*	xf, yf, nn may be used before being initialized ? */
/*	lineflagn  may be used before being initialized ? */

	for(;;) {			/* scan over preamble - up to `charbegin' */
		if (fgets(buffer, MAXLINE, input) == NULL) {
			fprintf(errout, "EOF\n"); return -1;
		}
		line++;		
		if (*buffer == '%' || *buffer == '\n') continue;
		if (strstr(buffer, "adj_fill") != NULL) break;	/* continue char */
/*		if (strstr(buffer, "((") != NULL) break;	*//* continue char */
		if (strncmp(buffer, "endinput", 8) == 0) return -1; /* EOF */
		if (strncmp(buffer, "more_side", 9) == 0) {
			if(sscanf(buffer, "more_side(%lg", &moreside) < 1) {
				dontunder();
			}
			else if (traceflag != 0) printf("%s (%lg)", buffer, moreside);
		}
		if (strncmp(buffer, "initrot", 7) == 0) {
			if (sscanf(buffer, "initrot:=identity shifted(0,%lgv)", 
				&initshift) < 1) {
				if (strncmp(buffer, "initrot:=identity;",  18) == 0)
					initshift=0;
				else dontunder();
			}
			if (traceflag != 0) printf("%s (%lg)", buffer, initshift);
		}
		if (strncmp(buffer, "charbegin", 9) == 0) {
			if (traceflag != 0) printf("%s", buffer);
			s = buffer;
			if (strchr(s, '"') != NULL) {			/* quoted char name */
				if (sscanf(s, "charbegin( \"%c\",%n", charname, &m) > 0) {
					s += m;
					charname[1] = '\0';
					chrs = (int) charname[0];
					if (charname[0] >= '0' && charname[0] <= '9')
						replacedigit(charname);
				}
				else dontunder();
			}
			else {									/* character number */
				if (sscanf(s, "charbegin( %d,%n", &chrs, &m) > 0) {
					s += m;
					charname[0] = '\0';
				}
				else dontunder();
			}
			widthbogus = 0;
			if (sscanf(s, "%dh", &width) > 0) {			/* normal case */
				if (strchr(s, '+') != NULL ||
					strchr(s, '-') != NULL) {
					widthbogus = 1;
					if (traceflag != 0) printf("Width (%d) Bogus\n", width);
				}
			}
			else if (sscanf(s, " (%d+%d)*", &wa, &wb) == 2) {
				width = wa + wb;
			}
			else if (sscanf(s, " (%d-%d)*", &wa, &wb) == 2) {
				width = wa - wb;
			}			
			else dontunder();
			width = widths[chrs];				/* NEW */
			sidebearing = offsets[chrs];		/* NEW */
			mathcorrection = mathcorr[chrs];	/* NEW */
			w =  (double) width + moreside * 2.0 - mathcorrection;
			w = w + leftside + rightside;
			if (charname[0] != '\0')
				fprintf(output, "]\n%d %d %% %s\n", chrs, wmap(w), charname);
			else fprintf(output, "]\n%d %d\n", chrs, wmap(w)); 
	
/*	deal with more complex width formats above ? OK */	
	
			if (traceflag != 0) 
				printf("Read charbegin for char %d width %d\n", chrs, width);

			rotate = 0.0; cosrot = 1.0; sinrot = 0.0;
			xshift = 0.0; yshift = 0.0; slant = 0.0;

			break;
		}
	}

	for(;;) {				/* scan over preamble - up to first moveto */
		if (fgets(buffer, MAXLINE, input) == NULL) {
			fprintf(errout, "Unexpected EOF (%s)\n", "A"); return -1;
		}
		line++;		
		if (*buffer == '%' || *buffer == '\n') continue;
		if ((s = strstr(buffer, "rot :=")) != NULL) {
			if (traceflag != 0) printf("chrs %d %s", chrs, buffer);
			if (strstr(s, "rotatedaround") != NULL) {
				if (sscanf(s, "rot := identity rotatedaround((%lgh,%lgv),%lg)",
					&xcenter, &ycenter, &rotate) < 3) dontunder();
				rotate = rotate * pi / 180.0;
				cosrot = cos(rotate); sinrot = sin(rotate);
				xshift = xcenter -xcenter * cos(rotate) + ycenter * sin(rotate);
				yshift = ycenter - xcenter * sin(rotate) - ycenter * cos(rotate);
			}
			else if(sscanf(s, "rot := identity rotated %lg%n", &rotate, &n) 
					== 1) {
				s += n;
				rotate = rotate * pi / 180.0;
				cosrot = cos(rotate); sinrot = sin(rotate);
				xshift = 0.0; yshift = 0.0;	/* ??? */
				if(sscanf(s, " shifted (%lgh,%lgv)", &xshift, &yshift) < 2) {
					if(sscanf(s, " shifted (0,%lgv)", &yshift) < 1) {
						if(sscanf(s, " shifted (%lgh,0)", &xshift) < 1)	
							dontunder();
					}
				}
			}
			else if(sscanf(s, "rot := rot shifted (%lgh,%lgv)", 
				&xshift, &yshift) == 2) {  /* debugging only */
				printf("dx %lg dy %lg char %d\n", xshift, yshift, chrs);
			}
			else if (sscanf(s, "rot := identity slanted %d/%d", 
				&slannum, &slanden) == 2) {
				slant = (double) slannum / slanden;
				printf("Switching to slant %lg\n", slant);	/* debug */
			}
			else  dontunder();
		}
		if ((s = strstr(buffer, "slant")) != NULL) { /* new */
			if (sscanf(s, "slant %d/%d", &slannum, &slanden) > 0) {
				if (slannum == 0) slanden = 1;
				slant = (double) slannum / slanden; 
				printf("Switching to slant %lg\n", slant);	/* debug */
			}
			else if (strstr(buffer, "slanted") == NULL) dontunder();
		}
		if (strstr(buffer, "...") != NULL) {
			lineflag = 0; break;
		}
		else if (strstr(buffer, "--") != NULL) {
			lineflag = 1; break;
		}
/*		if (strstr(buffer, "((") != NULL) break; */
	}
	
	if (traceflag != 0) printf("%s", buffer);
	
	if (lineflag == 0) {
		n = sscanf(buffer, " ((%lg,%lg){%lg,%lg}...{%lg,%lg}",
			&xs, &ys, &us, &vs, &uf, &vf);
		if (n < 4) 	{ 
			dontunder();
			(void) scantoend(input);
			return 0;
		}
		if (allowslant != 0 && slant != 0.0) {
			xs = xs + ys * slant;
			us = us + vs * slant;
			uf = uf + vf * slant;
		}
/*		if (traceflag != 0) printf("Read ... form\n"); */
	}
	else {
		n = sscanf(buffer, " ((%lg,%lg)--",	&xs, &ys);
		if (n < 2) 	{
			dontunder();
			(void) scantoend(input);
			return 0;
		}
		if (allowslant != 0 && slant != 0.0) {
			xs = xs + ys * slant;
		}
/*		if (traceflag != 0) printf("Read -- form\n"); */
	}

	xstart = xs; ystart = ys;   /* remember for closing at end */
	fprintf(output, "%d %d m\n", xmap(xs, ys), ymap(xs, ys));  /* moveto  */
/*	if (traceflag != 0) 
		printf("Starting point (%lg %lg)\n", xstart, ystart); */

/*	read charpath - continue until hit line containing "cycle" */

	for(;;) {
		if (fgets(buffer, MAXLINE, input) == NULL) {
			fprintf(errout, "Unexpected EOF (%s)\n", "B"); return -1;
		}
		line++;
		if (*buffer == '%' || *buffer == '\n') continue;
		if ((s = strstr(buffer, "slant")) != NULL) { /* new */
			if (sscanf(s, "slant %d/%d", &slannum, &slanden) > 0) {
				if (slannum == 0) slanden = 1;
				slant = (double) slannum / slanden; 
				printf("Switching to slant %lg\n", slant);	/* debug */
			}
			else dontunder();	
			continue;
		}
		if (strstr(buffer, "((") != NULL) {
			dontunder();	
			(void) scantoend(input);
			break;
		}
		else if (strchr(buffer, '+') != NULL) {
			dontunder();			
			(void) scantoend(input);
			break;
		}
		else if (strstr(buffer, "...") != NULL) {
			lineflagn = 0;
			nn = sscanf(buffer, " (%lg,%lg){%lg,%lg}...{%lg,%lg}",
				&xsn, &ysn, &usn, &vsn, &ufn, &vfn);
			if (nn < 4)	{
				dontunder();
				continue;		/* IGNORE THIS POINT IF POSSIBLE */
			}
			if (allowslant != 0 && slant != 0.0) {
				xsn = xsn + ysn * slant;
				usn = usn + vsn * slant;
				ufn= ufn + vfn * slant;
			}
			if (n < 6) {   /* copy finish tangent direction previous */
				uf = usn; vf = vsn;
			}
		}
		else if (strstr(buffer, "--") != NULL) {
			lineflagn = 1;
			nn = sscanf(buffer, " (%lg,%lg)--",	&xsn, &ysn);
			if (nn < 2)	{
				dontunder();
				continue;		/* IGNORE THIS POINT IF POSSIBLE */
			}
			if (allowslant != 0 && slant != 0.0) {
				xsn = xsn + ysn * slant;
			}
			if (n < 6) {   /* copy finish tangent direction previous */
				if (lineflag == 0) {	/* OK if previous was "--" also */
					  saywhere(stdout);
					  printf("Incomplete curveto before lineto\n");
					  uf = us; vf = vs;    /* is this OK ??? */
				}
			}
		}
		xf = xsn; yf = ysn;		/* finish of previous is start of new */

		if (lineflag == 0) 
			(void) smoothknots(output, xs, ys, us, vs, xf, yf, uf, vf);
		else if (xf != xs || yf != ys) 
/*			fprintf(output, "%d %d l\n", xmap(xf, yf), ymap(xf, yf)); */
			lineto(output, xs, ys, xf, yf);
		else {	
			saywhere(stdout);
			printf("Coincident ends (%lg %lg)\n", xf, yf);
		}

/*		get ready for next */
		xs = xsn; ys = ysn; us = usn; vs = vsn; uf = ufn, vf = vfn; 
		lineflag = lineflagn; n = nn;
		
		if (strstr(buffer, "cycle") != NULL) {
/*			if (lineflag == 0) 
				(void) smoothknots(output, xs, ys, us, vs, xf, yf, uf, vf);
			else fprintf(output, "%d %d l\n", xmap(xf, yf), ymap(xf, yf)); */
			break;
		}
	}

/* this may be the end of the character, or there may be other paths */

	if (traceflag != 0) printf("%s", buffer);

/*	close the loop now */
	if (xstart != xf || ystart != yf) /* don't output worthless lineto */
/*		fprintf(output, "%d %d l\n", 
			xmap(xstart, ystart), ymap(xstart, ystart)); */
		lineto(output, xs, ys, xstart, ystart);
	else if (traceflag != 0) printf("Path already closed\n");
	fprintf(output, "h\n");
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int scansidebearings(FILE *input) {
	char chara, charb, charc, chard;
	int lefts, rights, k, n;
	int w, w1, w2;
	int off, cor, flag;

	for (k = 0; k < MAXCHRS; k++) {
		left[k] = 0; right[k] = 0; offsets[k] = 0; mathcorr[k] = 0;
	}

	if (fgets(buffer, MAXLINE, input) == NULL) {
		fprintf(errout, "Unexpected EOF (%s)\n", "C"); return -1;
	}
	flag = 0;
	line++;
	while (strncmp(buffer, "left", 4) != 0) {
		if (strncmp(buffer, "charbegin", 9) == 0) {
			flag=1; break;
		}
		if (fgets(buffer, MAXLINE, input) == NULL) {
			fprintf(errout, "Unexpected EOF (%s)\n", "D"); return -1;
		}
		line++;
	}
	if (flag == 0) {	/* scan in side bearings */
		while (*buffer > ' ') {
			if (*buffer != '%') {
				if (sscanf(buffer, "left%c# = %dh#; right%c# = %dh#;",
					&chara, &lefts, &charb, &rights) < 4) {
					dontunder();
				}
				if (chara != charb) dontunder();
				left[chara] = lefts; right[charb] = rights;
			}
			if (fgets(buffer, MAXLINE, input) == NULL) {
				fprintf(errout, "Unexpected EOF (%s)\n", "E"); return -1;
			}
			line++;	
		}
	}
	
	for(;;)  {
		if (strncmp(buffer, "endinput", 8) == 0) break; /* EOF */
/* now scan for charbegin */
		while (strncmp(buffer, "charbegin", 9) != 0) {
			if (fgets(buffer, MAXLINE, input) == NULL) break;
			line++;
		}
/* analyze charbegin */
		if (sscanf(buffer, "charbegin( \"%c\",%n", &chara, &n) != 0) ;
		else if (sscanf(buffer, "charbegin( %d,%n", &k, &n) != 0) 
			chara = (char) k;
		else dontunder();
		
		if(sscanf(buffer+n, " (%d+%d)*h#", &w1, &w2) == 2) w = w1 + w2;
		else if(sscanf(buffer+n, " (%d-%d)*h#", &w1, &w2) == 2) w = w1 - w2;
		else if (sscanf(buffer+n, " %dh# + left%c# + right%c#,", 
			&w, &charc, &chard) == 3) {
			if (charc != chard) dontunder();
			w = w + left[charc] + right[chard];
		}
		else if (sscanf(buffer+n, " %dh#", &w) == 1) ;
		else dontunder();
		widths[chara] = w;
/* now scan for endchar */
		while (strncmp(buffer, "endchar", 7) != 0) {
			if (strncmp(buffer, "mathcorr", 8) == 0) {
				if (sscanf(buffer, "mathcorr(%d", &cor) == 1) {
					if (strstr(buffer, "skew") != NULL)
						mathcorr[chara] = cor * skew;
					else mathcorr[chara] = cor;
				}
				else dontunder();
			}
			if (fgets(buffer, MAXLINE, input) == NULL) {
				fprintf(errout, "Unexpected EOF (%s)\n", "F"); return -1;
			}
			line++;
		}
/* analyze endchar */
		charb = chara;
		if (sscanf(buffer, "endchar(left%c#);", &charb) > 0) {
			if (charb != chara) {
				saywhere(errout);
				fprintf(errout, "Mismatched sidebearing info\n");
			}
			offsets[chara] = left[charb];
		}
		else if (sscanf(buffer, "endchar(%d);", &off) > 0) 
			offsets[chara] = off;
		else dontunder();

		if (fgets(buffer, MAXLINE, input) == NULL) break;
		line++;
	}
}

void showtables(void) {
	int k;
	putc('\n', stdout);
	for (k = 0; k < MAXCHRS; k++) {
		printf("%3d left %4d right %4d offset %4d width %5d mathcorr %lg\n",
			k, left[k], right[k], offsets[k], widths[k], mathcorr[k]);
	}
	putc('\n', stdout);
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
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	FILE *input, *output;
	int c, firstarg=1;
	int pt=10;			/* pt size */
	char name[16];		/* Font ID */
	char *s;

	if (firstarg == argc) return -1;

/*	analyze command line flags */

/*	if (*argv[firstarg] == '-') { */
	while (firstarg < argc && *argv[firstarg] == '-') {
		s = argv[firstarg]; 
		s++;
		while ((c = *s++) != '\0') {
			if (c == 'n') noscaling=1;
			else if (c == 'v') verboseflag=1;
			else if (c == 't') traceflag=1;			
			else if (c == 'r') allowrotate=0; /* don't allow rotation */
			else if (c == 's') tripledot=0;	/* don't imnplement ... */
			else fprintf(errout, "Don't understand %c flag\n", c);
		}
		firstarg++;
	}

	if (argc < firstarg+1) {	/* useage */
		printf("%s -v -t -r -s -n <mf-file>  <pt-size>\n",  argv[0]);
		printf("e.g. eulerint -v c:\\amsfonts\\mf\\eurmch 10\n");
		exit(1);
	}
	
	printf("EULERINT - program for converting METAFONT code\n");

	if (argc > firstarg+1) {	/* font size specified ? */
		s = argv[firstarg+1];
		if(sscanf(s, "%d", &pt) < 1) {
			fprintf(errout, "Don't understand point size: %s\n", s);
		}
	}

/*	get rid of path name */
	if ((s = strrchr(argv[firstarg], '\\')) == NULL) {
		if ((s = strrchr(argv[firstarg], '/')) == NULL)	{
			if ((s = strrchr(argv[firstarg], ':')) == NULL)	
				s = argv[firstarg];
			else s++;	
		}
		else s++;
	}
	else s++;	
	strncpy(name, s, 4);		/* extract font name */
	*(name+4) = '\0';
	if(setupfont(pt, name) < 0) {
		fprintf(errout, "Failed to set up font\n");
		exit(3);
	}

	strcpy(infilename, argv[firstarg]);
	extension(infilename, "mf");
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename);
		exit(3);
	}
/*	strcpy(outfilename, argv[firstarg]); */
	strcpy(outfilename, name);
	sprintf(outfilename+4, "%d", pt);
	forceexten(outfilename, "out");
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);
		exit(3);
	}
	if (traceflag != 0) printf("Files open\n");
	setconstants();
	if (traceflag != 0) printf("a %lg b %lg c %lg\n", ha, hb, hc);
	scale = programem / 1000.0;  
	skew = programem / 36.0;		/* for eurm font specific */

	line = 0;
	(void) scansidebearings(input);
	if (verboseflag != 0) {
		showtables();
		printf("Exiting after verbose output of tables\n");
		return 0;
	}
	rewind(input);

	line = 0;
	fprintf(output, "%lg %lg\n", ptsize, 1.0);	/* ptsize and scale */
	fprintf(output, "%d %d %d %d\n", xll, yll, xur, yur);	/* FontBBox */

	for(;;) {
		if (parseeuler(output, input) != 0) break;
	}

	if (traceflag != 0) printf("Files will be closed\n");
	fclose(input);
	if(ferror(output) != 0) {
		perror(outfilename);
	}
	else fclose(output);
	return 0;
}

/* deal with omitted final tangent directions ? */
/* what is termination ? */
/* deal with `--' */
/* can `..' ever occur */

/* read the lefta# and righta# at the beginning to get sidebearings */

/* read stem rounding hints ? set_stem_round */

/* read charbegin lines for character widths (height and depth) info */

/* charbegin first arg either "char"or number */

/* charbegin char, width, height, depth */

/* care about endchar ? */

/* lcbody, ascender, baseline, descender, capheight, */
/* programem, depthy, theight, etc */
/* --- all given at head of file */

/* input scaling: bitpad numbers (.001") */
/* output scale by: double programem=3700.0; */
/* h# = ptsize * xscale_factor / programem */
/* v# = ptsize / programem */

/* widthbogus flag not used yet */ /* lefta, righta etc */

/* rot := identity rotated 3 shifted (30h,40v) */ /* not yet output */

/* rotatedaround has not been checked for sign errors and such */\

/* hinting information not recorded yet */

/* if tangent directions exactly equal ? really want a lineto ? */
/* need to check whether aligned with chord ? */

/* get character name if possible from file ? */

/* deal with offset found in endchar */

/* don't rotate and shift characters yet ? */

/* read only up to `endinput' */
