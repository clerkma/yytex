/* Copyright 1990, 1991, 1992 Y&Y
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

/* SEE ALSO C:\COLD\ADJSIDE.C */

/* program to adjust OUT file */
/* - produced by adjusting sidebearings */ /* used for AT&T project */

/* adds small left and right sidebearing to characters */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* #define FNAMELEN 80 */
#define MAXLINE 256
#define MAXCHRS 256
#define MAXNAMELEN 32

int verboseflag = 1;

int suppresscontrol=1;	/* don't pay attention to control points */
int usetangents=1;		/* use points of tangency */

char line[MAXLINE];

char buffer[MAXLINE];

char temp[MAXLINE];

char filename[FILENAME_MAX], fontname[FILENAME_MAX];
	
char names[MAXCHRS][MAXNAMELEN];
int shifts[MAXCHRS];

int widths[MAXCHRS]; /* not accessed */
int xlls[MAXCHRS], ylls[MAXCHRS], xurs[MAXCHRS], yurs[MAXCHRS]; /* NA */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int delaywrite=1;				/* check out bounding box first */
int projectflag=1;				/* project along slant direction */

int leftside=8;					/* added sidebearing on left */
int rightside=8;				/* added sidebearing on right */

int italicflag=0;				/* on if font assumed italic */
int smallcaps = 0;		/* font is smallcaps font */

double italicangle=-17.0;		/* ItalicAngle */

double slant;					/* tangent of italic angle */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* left and right side added space and xscale factor */
/* as function of point size */

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

/* find point (if any) on curve where it is tangent to given direction */
/* return -1.0 if none found */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double tangent(double xs, double ys, double xa, double ya, 
	double xb, double yb, double xe, double ye, double xmd, double ymd) {
		double ax, ay, bx, by, cx, cy, a, b, c;
		double t1, t2, dis, dsc;
/*		double xs, ys, xa, ya, xb, yb, xe, ye, xmd, ymd; */
		int flag1, flag2;

/*		xs = zs.x;   ys = zs.y;   xe = ze.x;   ye = ze.y; 
		xa = xs + alpha * xsd; ya = ys + alpha * ysd;
		xb = xe - beta * zed.x;  yb = ye - beta * zed.y;		
		xmd = zmd.x;  ymd = zmd.y; */

		ax = (xe - xs) + 3.0 * (xa - xb);
		ay = (ye - ys) + 3.0 * (ya - yb);
		
		bx = 2.0 * (xs - 2.0 * xa + xb);
		by = 2.0 * (ys - 2.0 * ya + yb);
		
		cx = xa - xs; cy = ya - ys;
		
		a = ax * ymd - ay * xmd;
		b = bx * ymd - by * xmd;		
		c = cx * ymd - cy * xmd;

		if (a < 0.0) {
			a = - a; b = -b; c = - c;
		}

/*		printf("a %lg b %lg c %lg\n", a, b, c);	getch(); */

		if (a == 0.0) return (- c / b);
		if (c == 0.0) return (- b / a);
		dis = b * b - 4.0 * a * c;
		if (dis < 0) {
/*			fprintf(stderr, 
				"Negative discriminant (tangent) a %lg b %lg c %lg\n",
					a, b, c);
			showcode(); 
			return(-b / (2.0 * a)); */
			return -1.0;
		}
		dsc = sqrt(dis);
		if (b < 0.0) {
			t1 =  (dsc - b) / (2.0 * a); t2 =  (2.0 * c) / (dsc - b);
		}
		else {
			t1 = - (2.0 * c) / (b + dsc); t2 = - (b + dsc) / (2.0 * a);
		}
/*		printf("b %lg dsc %lg t1 %lg t2 %lg\n", b, dsc, t1, t2); */

		if (t1 >= 0.0 && t1 <= 1.0) flag1 = -1; else flag1 = 0;
		if (t2 >= 0.0 && t2 <= 1.0) flag2 = -1; else flag2 = 0;
		if (flag1 != 0 && flag2 != 0) {
/*			fprintf(stderr, "Two tangents (tangent) T1 %lg T2 %lg", t1, t2);
			showcode(); */
			if (fabs(t1 - 0.5) < fabs(t2 - 0.5)) return t1;
			else return t2;
/*			return(-b / (2.0 * a)); */
		}
		if (flag1 == 0 && flag2 == 0) {
/*			fprintf(stderr, "No tangents (tangent) T1 %lg T2 %lg\n", t1, t2);
			showcode();
			return(-b / (2.0 * a)); */
			return -1.0;
		}
		if (flag1 != 0) return t1;	else return t2;
} 

double bezier(double zs, double za, double zb, double ze, double t) {
	double s = 1.0 - t;
	return (zs * s * s * s + 3.0 * za * t * s * s 
		+ 3.0 * zb * t * t * s + ze * t * t * t);
}

double bezierd(double zs, double za, double zb, double ze, double t) {
	return -3.0 * zs * (1.0 - t) * (1.0 - t)
		+ 3.0 * za * (1.0 - t) * (1.0 - 3.0 * t)
			+ 3.0 * zb * t * (2.0 - 3.0 * t)
				+ 3.0 * ze * t * t;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int xmin, xmax;

int round(double x) {
	if (x < 0.0) return - round(- x);
	else return (int) (x + 0.5);
}

void getskewed(FILE *input) {
	long startpos;
	int xa, ya, xb, yb, xc, yc, xd;
	double fxold, fyold, fxa, fya, fxb, fyb, fxc, fyc;
	double t, fxn, fyn, xtd, ytd;
	int xold=0, yold=0;
	int xn, yn;

	xmin=0; xmax=0;

	startpos = ftell(input);
	for(;;) {
		if (fgets(temp, MAXLINE, input) == NULL) break;
		if (*temp == ']') break;
		if (sscanf(temp, "%d %d %d %d %d %d c", 
			&xa, &ya, &xb, &yb, &xc, &yc) == 6) {
			if (suppresscontrol == 0) {
				xd = xa - (int) (ya * slant);
				if (xd < xmin) xmin = xd;
				if (xd > xmax) xmax = xd;
				xd = xb - (int) (yb * slant);
				if (xd < xmin) xmin = xd;
				if (xd > xmax) xmax = xd;
			}
			xd = xc - (int) (yc * slant);
			if (xd < xmin) xmin = xd;
			if (xd > xmax) xmax = xd;
			if (usetangents != 0) {
				fxold = (double) xold; 	fyold = (double) yold;
				fxa = (double) xa;	fya = (double) ya;
				fxb = (double) xb;	fyb = (double) yb;
				fxc = (double) xc;	fyc = (double) yc;
				t = tangent(fxold, fyold, fxa, fya, fxb, fyb, fxc, fyc,
					slant, 1.0);
				if (t > 0.0 && t < 1.0) {
					fxn = bezier(fxold, fxa, fxb, fxc, t);
					fyn = bezier(fyold, fya, fyb, fyc, t);
					xn = round(fxn); yn = round(fyn);
/*					zn = round(fxn - slant * fyn);	 */
					xd = round(fxn - slant * fyn);	
					xtd = bezierd(fxold, fxa, fxb, fxc, t);
					ytd = bezierd(fyold, fya, fyb, fyc, t);		
/*					if (xn < xll) xll = xn; if (xn > xur) xur = xn; */
/*				    if (yn < yll) yll = yn; if (yn > yur) yur = yn; */
/*					if (zn < zll) zll = zn; if (zn > zur) zur = zn; */
					if (xd < xmin) xmin = xd; if (xd > xmax) xmax = xd;
				}
			}
			xold = xc; yold = yc;
		}
		else if (sscanf(temp, "%d %d l", &xa, &ya) == 2) {
			xd = xa - (int) (ya * slant);
			if (xd < xmin) xmin = xd;
			if (xd > xmax) xmax = xd;
			xold = xa; yold = ya;
		}
		else if (sscanf(temp, "%d %d m", &xa, &ya) == 2) {
			xold = xa; yold = ya;
		}
	}
	fseek(input, startpos, SEEK_SET);
 }

void adjustsides (FILE *output, FILE *input) {
	int nchar, width, n;
	int xll, yll, xur, yur;
/*	int xskew; */
	char *s;
	
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == ']') {			/* found start of character */
			fputs(line, output);
/*	look for line with character number and width */
			fgets(line, MAXLINE, input);
			while (*line == '%') {
				fputs(line, output);
				fgets(line, MAXLINE, input);
			}
			if (sscanf(line, "%d %d%n", &nchar, &width, &n) < 2) {
				fprintf(stderr, "%s", line);
				fputs(line, output);
			}
			else {
				strcpy(buffer, line + n);
				if (delaywrite == 0) {
					sprintf(line, "%d %d", 
						nchar, width + leftside + rightside);
					strcat(line, buffer);
					fputs(line, output);
				}
			}
/*  look for character bounding box line */
			fgets(line, MAXLINE, input);	
			if (strncmp(line, "%%", 2) == 0) {
				if (sscanf(line+2, "%d %d %d %d", &xll, &yll, &xur, &yur) 
					<	4) {
					fputs(line, stderr);
					fputs(line, output);
				}
				else {
					if (delaywrite != 0) {
						if (italicflag != 0) width = xur - (int) (yur * slant);
						else width = xur;
						xmin = 0; xmax = width;
						if (italicflag != 0 && projectflag != 0) {
							getskewed(input);
							width = xmax - xmin;
						}
						sprintf(line, "%d %d", 
							nchar, width + leftside + rightside);
						strcat(line, buffer);
						fputs(line, output);						
						sprintf(line, "%%%% %d %d %d %d\n", 
							xll + leftside - xmin, yll, 
								xur + leftside - xmin, yur);
						fputs(line, output);
					}
					else {
						sprintf(line, "%%%% %d %d %d %d\n", 
							xll+leftside, yll, xur+leftside, yur);
						fputs(line, output);
					}
				}
			}
/*  look for line with initial moveto */
			fgets(line, MAXLINE, input);
			while (*line == '%') {
				fputs(line, output);
				fgets(line, MAXLINE, input);
			}
			if ((s = strchr(line, 'm')) == NULL) {
				fprintf(stderr, line);
				fputs(line, output);
			}
			else {
				if (italicflag != 0 && projectflag != 0)
					sprintf(s+1, " %% offset %d %d\n", leftside - xmin, 0);
				else sprintf(s+1, " %% offset %d %d\n", leftside, 0);
				fputs(line, output);
			}
		}
		else fputs(line, output);	/* not `]' line */
	}
}

double pi = 3.141592653;

int main(int argc, char *argv[]) {
	FILE *input, *output;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_bak[FILENAME_MAX];
	char *s;
	int firstarg=1, m;
	
	if (argc < 1) {
		fprintf(stderr, "Need argument specifying source file");
		exit(1);
	}

	slant = tan(- italicangle * pi / 180.0);
	if (verboseflag != 0) printf("Slant if needed %lg\n", slant);

	for (m = firstarg; m < argc; m++) {

		strcpy(fn_in, argv[m]);
		extension(fn_in, "out");
		if (strstr(s, "sm") != NULL ||
			strstr(s, "SM") != NULL) smallcaps = 1;	
		else smallcaps = 0;
		if (strstr(s, "it") != NULL ||
			strstr(s, "IT") != NULL) italicflag = 1;
		else italicflag = 0;

		suppresscontrol=1;			/* non-zero => don't consider cntrl pts */
		usetangents=1;				/* non-zero => use tangents in slant */

		if (italicflag == 0) {	/* for backward compatibility */
			suppresscontrol = 0;	
			usetangents = 0; 
		}

/* strip off pathname */
		if ((s = strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s = strrchr(fn_in, '/')) != NULL) s++;
		else if ((s = strrchr(fn_in, ':')) != NULL) s++;	
		else s = fn_in;

		if (verboseflag != 0) printf("Processing %s ", fn_in);

/*		if (strstr(fn_in, "it") != NULL ||
			strstr(fn_in, "IT") != NULL) {
			if (verboseflag !=0) printf(" Assumed Italic\n");
			italicflag=1;
		}
		else {
			italicflag=0;
			if (verboseflag !=0) printf("\n");
		}  */

		if (verboseflag != 0) {
			if (italicflag != 0) printf(" Assumed Italic\n");
			else printf("\n");
		}
	
/* output goes in current directory */
		strcpy(fn_out, s);
		forceexten(fn_out, "out");

		if (strcmp(fn_in, fn_out) == 0) {
			strcpy(fn_bak, fn_in);
			forceexten(fn_in, "bak");
			remove(fn_in);
			rename(fn_bak, fn_in);
		}
		if((input = fopen(fn_in, "r")) == NULL) {
			perror(fn_in);	exit(1);
		}	
		if((output = fopen(fn_out, "w")) == NULL) {
			perror(fn_out);	exit(1);
		}	

		adjustsides(output, input);

		fclose(input);
		if (ferror(output) != 0) {
			perror(fn_out); exit(3);
		}
		else fclose(output);
	}
		return 0;
}

/* several errors detected, but not acted upon - assuming good data */

/* need to adjust /StdVW and /StemSnapV also (say what?) */
