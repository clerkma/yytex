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
#include <math.h>

#define MAXLINE 256

#define MAXKNOTS 256

#define MAXCHARNAME 256

char line[MAXLINE];

int index;

int chr, wx;

int xi[MAXKNOTS], yi[MAXKNOTS], ci[MAXKNOTS];

int xo[MAXKNOTS], yo[MAXKNOTS], co[MAXKNOTS];

double pi = 3.141592653;

double xcenter, ycenter, radius, theta;

int verboseflag=1;

int traceflag=0;

double minsize = 10.0;

double mingap = 6.0;

double costhres = -0.1;

/* strips file name, leaves only path */

void removefilename(char *file) {	/* modifies arg file */
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

/* return pointer to file name - minus path - returns pointer to filename */

char *getfilename(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

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

void insert(int x, int y, int c) {
	if (index >= MAXKNOTS) {
		printf("Too many knots\n");
		exit(1);
	}
	xi[index] = x;
	yi[index] = y;
	ci[index] = c;
	index++;
}

void moveto(void) {
	int x, y;
	if (sscanf(line, "%d %d m", &x, &y) < 2) {
		printf("BAD moveto: %s\n", line);
	}
	insert(x, y, 'm');
}

void lineto(void) {
	int x, y;
	if (sscanf(line, "%d %d l", &x, &y) < 2) {
		printf("BAD lineto: %s\n", line);
	}
	insert(x, y, 'l');
}

void curveto(void) {
	int xa, ya, xb, yb, xc, yc;
	if (sscanf(line, "%d %d %d %d %d %d c",
			   &xa, &ya, &xb, &yb, &xc, &yc) < 6) {
		printf("BAD curveto: %s\n", line);
	}
	insert(xa, ya, ' ');
	insert(xb, yb, ' ');
	insert(xc, yc, 'c');
}

int readoutline(FILE *input) {
	if (traceflag) printf("Reading outline: %s", line);
/*	if (fgets(line, sizeof(line), input) == NULL) return -1; */
	index = 0;
	while (*line == '%' || *line == ';' || *line == '\n')
		fgets(line, sizeof(line), input);
	if (strchr(line, 'm') == NULL) {
		printf("BAD moveto: %s\n", line);
	}
	else moveto();
	while ((fgets(line, sizeof(line), input)) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (*line == 'h') break;
		else if (strchr(line, 'l') != NULL) lineto();
		else if (strchr(line, 'c') != NULL) curveto();
		else {
			printf("BAD line: %s\n", line);
		}
		if (*line == ']') {
			break;
		}
	}
/*	close if not closed */
	if (xi[index-1] != xi[0] || yi[index-1] != yi[0]) {
		printf("closing the path\n");
		insert(xi[0], yi[0], 'l');
	}
	return 0;
}

void writeoutline(FILE *output, int flag) {
	int k, knext, first=1;
	for (k = 0; k < index; k++) {
		if (flag && first) {
			for (knext = k; knext < k+3; knext++) {
				if (ci[knext] != ' ') break;
			}
			printf("%d\t", knext);
			first = 0;
		}
		fprintf(output, "%d %d ", xo[k], yo[k]);
		if (co[k] != ' ') {
			putc(co[k], output);
			putc('\n', output);			
			first = 1;
		}
	}
	fprintf(output, "h\n");
}

void copyoutline (int offset) {
	int k;
	if (offset != 0) printf("Copying with offset %d\n");
	xo[0] = xi[offset];
	yo[0] = yi[offset];	
	co[0] = 'm';
	for (k = 1; k < index-offset; k++) {
		xo[k] = xi[k+offset];
		yo[k] = yi[k+offset];
		co[k] = ci[k+offset];
	}
	if (offset == 0) return;
/*	xo[index-offset] = xi[0]; */
/*	yo[index-offset] = yi[0]; */
/*	co[index-offset] = 'l'; */
	for (k = index-offset; k < index; k++) {
		xo[k] = xi[k-(index-offset)+1];
		yo[k] = yi[k-(index-offset)+1];
		co[k] = ci[k-(index-offset)+1];
	}
}

void transformoutline(void);

void processoutline(FILE *output, FILE *input) {
	if (traceflag) printf("Processing outline: %s", line);
	readoutline (input);
	transformoutline();
	writeoutline(output, 0);
}

int processcharacter(FILE *output, FILE *input) {
	char charname[MAXCHARNAME];
	
	if (fgets(line, sizeof(line), input) == NULL) return -1;
	if (sscanf(line, "%d %d %% %s", &chr, &wx, charname) < 3) {
		printf("BAD start: %s\n", line);
	}
	putc('\n', stdout);
	printf("Working on character %d (%s)\n", chr, charname);
	fputs(line, output);		/* chr wx % name */
	while ((fgets(line, sizeof(line), input)) != NULL) {
		if (*line == ']') break;
		processoutline(output, input);
/*		if (*line == ']') break; */
	}
}

void processfile(FILE *output, FILE *input) {
/*	copy up to first character */
	while ((fgets(line, sizeof(line), input)) != NULL) {
		if (*line == ']') break;
		fputs(line, output);
	}
	fputs(line, output);		/* ] */
	for (;;) {
		if (processcharacter(output,input) < 0) break;
		if (*line == ']') fputs(line, output);	/* ] */
	}
	printf("Hit EOF in processcharacter\n");
}

int main(int argc, char *argv[]) {
	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
	FILE *input, *output;
	int m=1;
	
	if (argc < 2) exit(1);
	strcpy(infile, argv[m]);
	extension(infile, "out");
	strcpy(outfile, getfilename(argv[m]));
	forceexten(outfile, "fix");
	if ((input = fopen(infile, "r")) == NULL) {
		perror(infile);
		exit(1);
	}
	if ((output = fopen(outfile, "w")) == NULL) {
		perror(outfile);
		exit(1);
	}
	processfile(output, input);
	fclose(output);
	fclose(input);
	
	return 0;
}


/* following arranges start point to lie within the end segment near origin */

#ifdef IGNORED
void transformoutline(void) {
	int k, k1, k2;
	long r1s, r2s;
	copyoutline(0);			/* in case we pop out early */
	for (k = 0; k < index; k++) {
		if (ci[k] == 'c') break;
	}
	if (k >= index) {
		printf("BAD first c\n");
		return;
	}
	k1 = k;			/* first of two stretches of c's */
	for (k = k; k < index; k++) {
		if (ci[k] != 'c' && ci[k] != ' ') break;
	}
	for (k = k; k < index; k++) {
		if (ci[k] == 'c') break;
	}
	if (k >= index) {
		printf("BAD second c\n");
		return;
	}
	k2 = k;		/* second of two stretches of c's */
	r1s = (long) xi[k1] * xi[k1] + yi[k1] * yi[k1];
	r2s = (long) xi[k2] * xi[k2] + yi[k2] * yi[k2];
	printf("k1 %d (%ld) k2 %d (%ld)\n", k1, r1s, k2, r2s);
	if (r1s < r2s) copyoutline(k1);
	else copyoutline(k2);
}
#endif

double dotproduct (double x1, double y1, double x2, double y2) {
	return x1 * x2 + y1 * y2;
}

double crossproduct (double x1, double y1, double x2, double y2) {
	return x1 * y2 - x2 * y1;
}

double norm (double x, double y) {
	double rs = x * x + y * y;
	if (rs < 0.0) return 0.0;
	else return sqrt(rs);
}

double degrees(double radians) {
	return radians * 180.0 / pi;
}

/* following finds circles passing through outlines */

int findcenter(double x1, double y1, double x2, double y2, double x3, double y3) {
	double dxa, dya, xam, yam;
	double dxb, dyb, xbm, ybm;
	double a, b, c, d, e, f, det,rsq1, rsq2, rsq3;
	double radius1, radius2, radius3;
	double rsin, rcos;

	dxa = (x2 - x1);
	dya = (y2 - y1);
	xam = (x1 + x2) / 2.0;
	yam = (y1 + y2) / 2.0;
	dxb = (x3 - x2);
	dyb = (y3 - y2);
	xbm = (x2 + x3) / 2.0;
	ybm = (y2 + y3) / 2.0;
	a = dxa; b = dya; c = dxb; d = dyb;
	e = dxa * xam + dya * yam;
	f = dxb * xbm + dyb * ybm;
	det = a * d - b * c;
	if (det == 0) {
		printf("Failed on %ld %ld %ld %ld %ld %ld\n",
			   x1, y1, x2, y2, x3, y3);
		return -1;
	}
	xcenter = (e * d - f * b) / det;
	ycenter = (f * a - e * c) / det;
	rsq1 = (x1 - xcenter) * (x1 - xcenter) + (y1 - ycenter) * (y1 - ycenter);
	rsq2 = (x2 - xcenter) * (x2 - xcenter) + (y2 - ycenter) * (y2 - ycenter);
	rsq3 = (x3 - xcenter) * (x3 - xcenter) + (y3 - ycenter) * (y3 - ycenter);
	radius1 = sqrt(rsq1);
	radius2 = sqrt(rsq2);
	radius3 = sqrt(rsq3);
	rsin = crossproduct(x3 - xcenter, y3 - ycenter, x1 - xcenter, y1 - ycenter);
	rcos = dotproduct(x3 - xcenter, y3 - ycenter, x1 - xcenter, y1 - ycenter);
	theta = atan2 (rsin, rcos);
	printf("chr %d radius %lg %lg %lg center (%lg, %lg) theta %lg\n",
		   chr, radius1, radius2, radius3, xcenter, ycenter, degrees(theta));
	radius = (radius1 + radius2 + radius3) / 3.0;
	return 0;
}

long distancesq(int x1, int y1, int x2, int y2) {
	return (long) (x2 - x1) * (x2 - x1) + (long) (y2 - y1) * (y2 - y1);
}

int round(double x) {
	if (x >= 0.0) return (int) (x + 0.5);
	else return - (int) (- x + 0.5);
}

double overrelaxr=2.0;
double overrelaxc=2.0;

int maxiter=32;

double fiterror(int k1, int k2) {
	int k;
	int n=0;
	double eps, x, y;
	double sum = 0.0;
	for (k = k1; k <= k2; k++) {
		x = xi[k];
		y = yi[k];
		eps = sqrt((x - xcenter) * (x - xcenter) +
				   (y - ycenter) * (y - ycenter)) - radius;
		sum += eps * eps;
		n++;
	}
	return sqrt(sum) / (double) n;
}

void improveradius(int k1, int k2) {
	int k;
	int n=0;
	double x, y;
	double sum = 0.0;
	double newradius;
	for (k = k1; k <= k2; k++) {
		x = xi[k];
		y = yi[k];
		sum += sqrt((x - xcenter) * (x - xcenter) +
					(y - ycenter) * (y - ycenter));
		n++;
	}
	newradius = sum / (double) n;
	radius = radius + overrelaxr * (newradius - radius);
}

#ifdef IGNORED
void improvecenter(int k1, int k2) {
	int k;
	int n=0;
	double x, y;
	double sum=0.0, sumx=0.0, sumy=0.0;
	double r, rat;
	for (k = k1; k <= k2; k++) {
		x = xi[k];
		y = yi[k];
		r = sqrt((x - xcenter) * (x - xcenter) +
				 (y - ycenter) * (y - ycenter));
		rat = (radius - r) / r;
		sum += rat;
		sumx += x * rat;
		sumy += y * rat;
		n++;
	}
	xcenter = sumx / sum;
	ycenter = sumy / sum;
}
#endif

void improvecenter(int k1, int k2) {
	int k;
	int n=0;
	double x, y;
	double sumx=0.0, sumy=0.0;
	double sumrx=0.0, sumry=0.0;
	double xnew, ynew;
	double r;
	for (k = k1; k <= k2; k++) {
		x = xi[k];
		y = yi[k];
		r = sqrt((x - xcenter) * (x - xcenter) +
				 (y - ycenter) * (y - ycenter));
		sumx += x;
		sumy += y;
		sumrx += (xcenter - x) * radius / r;
		sumry += (ycenter - y) * radius / r;
		n++;
	}
	xnew = (sumx + sumrx) / (double) n;
	ynew = (sumy + sumry) / (double) n;
	xcenter = xcenter + overrelaxc * (xnew - xcenter);
	ycenter = ycenter + overrelaxc * (ynew - ycenter);
}

int searchalong(int k1, int k2, double ddr, double ddx, double ddy) {
	double radiusold=radius, xcenterold=xcenter, ycenterold=ycenter;
	int k, flag=0;
	double olderr, err;
	double epsilon=1.0;

	printf("Search along %lg %lg %lg\n", ddr, ddx, ddy);
	olderr = fiterror(k1, k2);
	for (k=0; k < 8; k++) {
		radius += - ddr * epsilon;
		xcenter += - ddx * epsilon;
		ycenter += - ddy * epsilon;
		err = fiterror(k1, k2);
		if (err > olderr) {
			epsilon = epsilon * 0.5;
			radius = radiusold;
			xcenter=xcenterold;
			ycenter = ycenterold;
		}
		else flag++;
	}
	return flag;
}

void gradientdescent(int k1, int k2) {
	double ddr, ddx, ddy, dd;
	double sumr=0.0, sumx=0.0, sumy=0.0;
	double x, y, r;
	int k, n=0;
	
	for (k = k1; k <= k2; k++) {
		x = xi[k];
		y = yi[k];
		r = sqrt((x - xcenter) * (x - xcenter) +
				 (y - ycenter) * (y - ycenter));
		sumr += (radius - r);
		sumx += (x - xcenter) * (radius - r) / r;
		sumy += (y - ycenter) * (radius - r) / r;
		n++;
	}	
	ddr = sumr / (double) n;
	ddx = sumx / (double) n;
	ddy = sumy / (double) n;
	dd = sqrt(ddr * ddr + ddx * ddx + ddy * ddy);
	printf("ddr %lg ddx %lg ddy %lg dd %lg\n", ddr, ddx, ddy, dd);
/*	searchalong(k1, k2, ddr/dd, ddx/dd, ddy/dd); */
	searchalong(k1, k2, ddr, ddx, ddy);
}

void improvesolution(int k1, int k2) {
	double radiusold, xcenterold, ycenterold, errold;
	double err;
	int k;
	err = fiterror(k1, k2);
	printf("iter %d radius %lg center (%lg, %lg) error %lg\n",
		   0, radius, xcenter, ycenter, err);
	for (k = 1; k < maxiter; k++) {
		radiusold = radius;
		xcenterold = xcenter;
		ycenterold = ycenter;
		errold = err;
/*		improveradius (k1, k2); */
/*		improvecenter (k1, k2); */
		gradientdescent (k1, k2);
		err = fiterror(k1, k2);
		printf("iter %d radius %lg center (%lg, %lg) error %lg\n",
			   k, radius, xcenter, ycenter, err);
		if (err > errold) {
			xcenter = xcenterold;
			ycenter = ycenterold;
			radius = radiusold;
			err = errold;
			break;
		}
	}
}

double x1d, y1d, x2d, y2d;	/* new knots for curveto */

/* (xcenter, ycenter), radius, theta, x1, y2, x2, y2 */

void makebezier(double x1, double y1, double x2, double y2) {
	double dx1, dy1, dx2, dy2;
	double kon;
	kon = (4.0 / 3.0) * (1 - cos(theta/2.0)) / sin(theta/2.0);
	dx1 = x1 - xcenter;
	dy1 = y1 - ycenter;
	x1d = x1 + dy1 * kon;
	y1d = y1 - dx1 * kon;
	dx2 = x2 - xcenter;
	dy2 = y2 - ycenter;
	x2d = x2 - dy2 * kon;
	y2d = y2 + dx2 * kon;	
	printf("Bezier k %lg: %d %d %d %d %d %d %d %d\n",
		   kon, /* degrees(theta), radius, */
		   (int) x1, (int) y1, (int) x1d, (int) y1d,
		   (int) x2d, (int) y2d, (int) x2, (int) y2);
}

int findmiddle (int k1, int k2) {
	int km1, km2;
	long d1s, d2s;
	if (k2 - k1 < 2) {
		printf("Bad k1 %d k2 %d\n", k1, k2);
		return -1;
	}
	if (((k2 - k1) % 2) == 0) return (k1 + k2) / 2;
	km1 = (k2 + k1 - 1) / 2;
	km2 = (k2 + k1 + 1) / 2;
	d1s = distancesq (xi[k1], yi[k1], xi[km1], yi[km1]);
	d2s = distancesq (xi[k2], yi[k2], xi[km2], yi[km2]);
	if (d1s < d2s) return km2;
	else return km1;
}

int splicebezier (int k1, int k2) {
	int k, shift;
	shift = (k2 - k1) - 3;
	if (shift != 0)  {
		for (k = k1+1; k < index; k++) {
			xi[k] = xi[k+shift];
			yi[k] = yi[k+shift];
			ci[k] = ci[k+shift];
		}
	}
	xi[k1+1] = (int) x1d;
	yi[k1+1] = (int) y1d;
	ci[k1+1] = ' ';
	xi[k1+2] = (int) x2d;
	yi[k1+2] = (int) y2d;
	ci[k1+2] = ' ';
	ci[k1+3] = 'c';
	index = index - shift;
	return shift;
}

int fitcircle (int k0) {
	int k, k1, k2, km;
	k1 = 0;
	for (k = k0+1; k < index; k++) {	/* scan forward to lineto */
		if (ci[k] == 'l') break;
		k1 = k;							/* last before start of `l's */
	}	/* start point of sequence of `l' */
	if (k == index) {
		printf("Hit end %d\n", index);
		return -1;
	}
	k2 = k;
	for (k = k; k < index; k++) {		/* scan to end of lineto's */
		if (ci[k] == 'c') break;
		if (ci[k] == 'l') k2 = k;		/* last of the 'l's */
	}	/* end point of sequence of 'l' */
	if (k == index) {
		printf("Hit end %d\n", index);
		return -1;
	}
	km = findmiddle(k1, k2);
	if (km < 0) {
		return -1;
	}
	findcenter((double) xi[k1], (double) yi[k1],
			   (double) xi[km], (double) yi[km],
			   (double) xi[k2], (double) yi[k2]);
/*	improvesolution (k1, k2); */
	makebezier((double) xi[k1], (double) yi[k1], (double) xi[k2], (double) yi[k2]);
	splicebezier(k1, k2);
/*	return k; */
	return k - (k2 - k1 - 3);		/* account for shift */
}

#ifdef IGNORED
void transformoutline(void) {
	int k;
	k = fitcircle(0);
	if (k < 0) {
		printf("char %d fitcircle failed\n", chr);
		copyoutline(0);
		writeoutline (stdout, 1);
	}
	else {
	}
	k = fitcircle(k);
	if (k < 0) {
		printf("char %d fitcircle failed\n", chr);
		copyoutline(0);
		writeoutline (stdout, 1);
	}
	else {
	}
	copyoutline(0);
}
#endif

void lmerge(int k) {
	int m;
	printf("LMERGE %d (%d %d) and %d (%d %d)\n",
		   k-1, xi[k-1], yi[k-1], k, xi[k], yi[k]);
	if (k < 1) return;
	xi[k-1] = (xi[k-1] + xi[k]) / 2;
	yi[k-1] = (yi[k-1] + yi[k]) / 2;
	for (m = k; m < index-1; m++) {
		xi[m] = xi[m+1]; 
		yi[m] = yi[m+1]; 
		ci[m] = ci[m+1];
	}
	if (k == index-1) {
		printf("LAST ONE\n");
		xi[0] = xi[k-1];
		yi[0] = yi[k-1];
	}
	index--;
}

/* not sure cmerge works correctly ... */

void cmerge(int k) {
	int m;
	printf("CMERGE %d (%d %d) and %d (%d %d)\n",
		   k-3, xi[k-3], yi[k-3], k, xi[k], yi[k]);
	if (k < 3) return;
	xi[k-3] = (xi[k-3] + xi[k]) / 2;
	yi[k-3] = (yi[k-3] + yi[k]) / 2;
	for (m = k; m < index-3; m++) {
		xi[m] = xi[m+3]; 
		yi[m] = yi[m+3]; 
		ci[m] = ci[m+3];
	}
	if (k == index-1) {
		printf("LAST ONE\n");
		xi[0] = xi[k-1];
		yi[0] = yi[k-1];
	}
	index = index - 3;
}

int removereversals(void) {
	int k, m, count=0;
	int x, y, c, xold, yold, cold, xanc, yanc, canc;
	double dx1, dy1, dx2, dy2, costh, mag1, mag2;
	
/*	copyoutline(0); */
/*	writeoutline (stdout, 1); */
	xanc = xi[0];
	yanc = yi[0];
	canc = ci[0];
	xold = xi[0];
	yold = yi[0];
	cold = ci[0];
	m = index-1;
	if (xi[m] != xi[0] || yi[m] != yi[0]) {
		xold = xi[m];
		yold = yi[m];
		cold = ci[m];
	}
	else {
		for (k = m-1; k > 0; k--) {
			if (ci[k] != ' ') {
				xold = xi[k];
				yold = yi[k];
				cold = ci[k];
				m = k;
				if (traceflag) printf("m %d\n", m); 
				break;
			}
		}
	}
	if (traceflag)
	printf("k %d (%d) xold %d yold %d cold %c\n", m, index, xold, yold, cold);

	for (k = m-1; k > 0; k--) {
		if (ci[k] != ' ') {
			xanc = xi[k];
			yanc = yi[k];
			canc = ci[k];
			m = k;
			if (traceflag) printf("m %d\n", m); 
			break;
		}
	}
	if (traceflag)
	printf("k %d (%d) xanc %d yanc %d canc %c\n", m, index, xanc, yanc, canc);

	for (k = 0; k < index; k++) {
		if (ci[k] == ' ') continue;
		x = xi[k];
		y = yi[k];
		c = ci[k];
		dx1 = (double) xold - xanc;
		dy1 = (double) yold - yanc;
		dx2 = (double) x - xold;
		dy2 = (double) y - yold;
		mag1 = norm (dx1, dy1);
		mag2 = norm (dx2, dy2);
		costh = dotproduct(dx1, dy1, dx2, dy2);
		if (mag1 != 0.0 && mag2 != 0.0)
			costh = costh / (mag1 * mag2);
		else costh = 1.0;
		if (mag1 == 0.0 || mag2 == 0.0) {
			printf("chr %3d k %2d (%4d %4d) costh %lg mag1 %lg (%c) mag2 %lg (%c)\n",
				   chr, k, x, y, 1.0, mag1, cold, mag2, c);
		}
/*		else if (mag1 < minsize || mag2 < minsize) {
	printf("chr %3d k %2d (%4d %4d) costh %lg mag1 %lg (%c) mag2 %lg (%c)\n",
				   chr, k, x, y, costh, mag1, cold, mag2, c);
		} */
		if (mag1 > 0.0 || mag2 > 0.0) {
			if (costh < costhres) {
	printf("chr %3d k %2d (%4d %4d) costh %lg mag1 %lg (%c) mag2 %lg (%c)\n",
					   chr, k, x, y, costh, mag1, cold, mag2, c);
				count++;
				if (c == 'l') {
					lmerge (k);
					xold = xi[k-1];
					yold = yi[k-1];
					cold = ci[k-1];
					continue;
				}
				else if (cold == 'l') {
					for (m = k; m > 0; m--) {
						if (ci[m] == 'l') break;
					}
					lmerge (m);
					xanc = xi[m-1];
					yanc = yi[m-1];
					canc = ci[m-1];
					continue;
				}
				else printf("WARNING: NEITHER ONE WAS A LINETO\n");
			}
		}
		xanc = xold; yanc = yold; canc = cold;
		xold = x; yold = y; cold = c;
	}
	return count;
}

void cleanends (void) {
	int xold, yold, x, y;
	double dx, dy, mag;
	if (ci[index-1] == 'l') {
		xold = xi[index-2]; yold = yi[index-2];
		x = xi[index-1]; y = yi[index-1];
		dx = (double) (x - xold);
		dy = (double) (y - yold);
		mag = norm(dx, dy);
		if (mag < minsize) lmerge(index-1);
	}
	if (ci[1] == 'l') {
		xold = xi[0]; yold = yi[0];
		x = xi[1]; y = yi[1];
		dx = (double) (x - xold);
		dy = (double) (y - yold);
		mag = norm(dx, dy);
		if (mag < minsize) lmerge(1);
	}
}

void mergeshort(void) {
	int k;
	double dx, dy, mag;
	double xold, yold, x, y;
	xold = xi[0]; yold = yi[0];
	for (k = 1; k < index; k++) {
		if (ci[k] == ' ') continue;
		x = xi[k]; y = yi[k];
		dx = (double) (x - xold); dy = (double) (y - yold);
		mag = norm(dx, dy);
		if (mag < mingap) {
			printf("ELIMINATING %d %c %lg\n", k, ci[k], mag);
			if (ci[k] == 'l') lmerge(k);
			else if (ci[k] == 'c') cmerge(k);
		}
		xold = x; yold = y;
	}
}

void transformoutline(void) {
	int k;
	copyoutline(0);
	writeoutline (stdout, 1);
	cleanends();
	mergeshort();
	k = removereversals();
	if (k < 0) {
		printf("char %d removereversals failed\n", chr);
	}
	copyoutline(0);
}


/* Near optimal Bezier approximation to arc of circle of angle 2 theta */
/* k = (4/3) (1 - cos theta) / sin theta */
/* for circle radius 1, center at origin - knots at: */
/* (cos theta, - sin theta) */
/* (cos theta, - sin theta) + k (sin theta, + cos theta) */
/* (cos theta, + sin theta) + k (sin theta, - cos theta) */
/* (cos theta, + sin theta) */
