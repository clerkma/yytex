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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MAXBUFFER 512

#define MAXCHRS 256

int verboseflag=1;
int traceflag=0;
int debugflag=0;

int splitflag = 1;

char line[MAXBUFFER];

double m11=0.001, m12=0.0, m21=0.0, m22=0.001, m31=0.0, m32=-0.16;

int FontMatrixSeen=0;
int FontBBoxSeen=0;

double ptsize=10.0;

double scale=1.0;

int xll=0, yll=-160, xur=1000, yur=1000;

char *FontName="DUMMY";

char *static0="findfont 400 scalefont setfont";
char *static1="0 setlinewidth newpath 0 0 moveto 400 0 lineto";
char *static2="400 400 lineto 0 400 lineto 0 0 lineto stroke";
/* char *static3="50 50 translate"; */
char *static3="0 1000 hsbw";

int sidebear, width, chrs;
int splits;
int lineflushed;
int straightened;
unsigned int charcode;
int closed;
int x, y;
int charcount;

int hits[MAXCHRS];

FILE *errout=stdout;

/*************************************************************************/

void docurveto(FILE *, int, int, int, int, int, int, int, int);

#define ABS(x) (((x) < 0) ? (-x) : (x))
#define MAX(x, y) (((x) < (y)) ? (y) : (x))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

int round(double x) {
	if (x < 0) return (- (round (- x)));
	else return (int) (x + 0.5);
}

/* check whether Bezier curve has extremum - gives t of extremum if found */

double extrema(int z0, int z1, int z2, int z3) {
	int minz, maxz;
	int az, bz, cz;
	double discr;
	double rootdiscr, root, root1, root2;

	minz = MIN(z0, z3); maxz = MAX(z0, z3);
	if (z1 >= minz && z1 <= maxz && z2 >= minz && z2 <= maxz) return -1.0;
	az = z3 - 3 * (z2 - z1) - z0;
	bz = 3 * (z2 - 2 * z1 + z0);
	cz = 3 * (z1 - z0);
	if (az == 0) {
		if (bz == 0) return -1.0;
		else {
			root = - (double) cz / (2.0 * (double) bz);
			if (root > 0.0 && root < 1.0) return root;
			else return -1.0;
		}
	}
	discr = (double) bz * (double) bz - 3 * (double) az * (double) cz;
	if (discr < 0) return -1.0;
	rootdiscr = sqrt((double) discr);
	root1 = (- (double) bz + rootdiscr) / (3.0 * (double) az);
	if (root1 > 0.0 && root1 < 1.0) return root1;
	root2 = (- (double) bz - rootdiscr) / (3.0 * (double) az);
	if (root2 > 0.0 && root2 < 1.0) return root2;
	return -1.0;
}

/* split Bezier curve at t = alp */

void splitextrema(FILE *output, double alp, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
	double f0, f1, f2, f3;
	double a, b, c, d, mal;
	double an, bn, cn, dn;
	int xn0, xn1, xn2, xn3;
	int yn0, yn1, yn2, yn3;
	int xm0, xm1, xm2, xm3;
	int ym0, ym1, ym2, ym3;

	if (traceflag) printf ("Splitting (%lg) (%d %d) %d %d %d %d %d %d\n",
						   alp, x0, y0, x1, y1, x2, y2, x3, y3);

	f0 = (double) x0;
	f1 = (double) x1;
	f2 = (double) x2;
	f3 = (double) x3;

	a = (f3 - f0) - 3.0 * (f2 - f1);
	b = 3.0 * (f2 - 2.0 * f1 + f0);
	c = 3.0 * (f1 - f0);	
	d = f0;

	an = a * alp * alp * alp;
	bn = b * alp * alp;
	cn = c * alp;
	dn = d;

	if (x0 != round(dn))
		fprintf(errout, "x0 %lg round(dn)\n", x0, round(dn));;

	xn0 = x0;
	xn1 = round(dn + cn / 3.0);
	xn2 = round(dn + (2.0 * cn + bn) / 3.0);
	xn3 = round(dn + cn + bn + an);

	mal = 1.0 - alp;
	an =  a * mal * mal * mal;
	bn =  (3.0 * a * alp + b) * mal * mal;
	cn =  (alp * (3.0 * a * alp + 2.0 * b) + c) * mal;
	dn =  alp * ( alp * (alp * a + b) + c) + d;

	xm0 = xn3;
	xm1 = round(dn + cn / 3.0);
	xm2 = round(dn + (2.0 * cn + bn) / 3.0);
	xm3 = x3;
	if (xm3 != round(dn+cn+bn+an))
		fprintf(errout, "xm3 %lg round(dn+cn+bn+an)\n", 
				xm3, round(dn+cn+bn+an));;

	f0 = (double) y0;
	f1 = (double) y1;
	f2 = (double) y2;
	f3 = (double) y3;

	a = (f3 - f0) - 3.0 * (f2 - f1);
	b = 3.0 * (f2 - 2.0 * f1 + f0);
	c = 3.0 * (f1 - f0);	
	d = f0;

	an = a * alp * alp * alp;
	bn = b * alp * alp;
	cn = c * alp;
	dn = d;

	if (y0 != round(dn)) 
		fprintf(errout, "y0 %lg round(dn)\n", y0, round(dn));;
	yn0 = y0;
	yn1 = round(dn + cn / 3.0);
	yn2 = round(dn + (2.0 * cn + bn) / 3.0);
	yn3 = round(dn + cn + bn + an);

	mal = 1.0 - alp;
	an =  a * mal * mal * mal;
	bn =  (3.0 * a * alp + b) * mal * mal;
	cn =  (alp * (3.0 * a * alp + 2.0 * b) + c) * mal;
	dn =  alp * ( alp * (alp * a + b) + c) + d;

	ym0 = yn3;
	ym1 = round(dn + cn / 3.0);
	ym2 = round(dn + (2.0 * cn + bn) / 3.0);
	ym3 = y3;

	if (ym3 != round(dn+cn+bn+an)) {
		fprintf(errout, "ym3 %lg round(dn+cn+bn+an)\n", 
				ym3, round(dn+cn+bn+an));;
	}
/*	recursive call for first Bezier */
	docurveto(output, xn0, yn0, xn1, yn1, xn2, yn2, xn3, yn3); 
/*	recursive call for second Bezier */
	docurveto(output, xm0, ym0, xm1, ym1, xm2, ym2, xm3, ym3); 
	splits++;		/* count it */
}

/*************************************************************************/

/* KanjiEncoding, KanjiSubEncoding */

int readheader (FILE *input) {
	char *s;
	rewind(input);
	while (fgets(line, sizeof(line), input) != NULL) {
/*		version, FullName, FamilyName, Weight, ItalicAngle, isFixedPitch */
/*		Encoding, FDepVector, UniqueID, FontName */
		if (FontMatrixSeen == 0 && strncmp(line, "/FontMatrix", 9) == 0) {
			s = line;
			while (*s != '[' && *s != '{' && *s != '\0') s++;
			if (sscanf (s, "[%lg %lg %lg %lg %lg %lg]",
						&m11, &m21, &m12, &m22, &m31, &m32) < 6) {
				FontMatrixSeen=1;
			}
		}
		else if (FontBBoxSeen == 0 && strncmp(line, "/FontBBox", 8) == 0) {
			s = line;
			while (*s != '[' && *s != '{' && *s != '\0') s++;
			if (sscanf (s, "[%d %d %d %d]",
						&xll, &yll, &xur, &yur) < 4) {
				FontBBoxSeen=1;
			}
		}
		else if ((s = strstr(line, "CompNF")) != NULL) {
			if (s > line && *(s-1) != '/') {
				if (traceflag) printf("FontName line: %s", line);
				if ((s = strchr(line, '/')) != NULL) {
					if (s > line) strcpy(line, s);
				}
				if ((s = strchr(line, ' ')) != NULL) *s = '\0';
				FontName = _strdup(line+1);
/*				if (traceflag) printf("End of prolog at %s\n", line); */
				return 0;
			}
		}
		else if (strstr(line, static0) != NULL) return 0;
		else if (strstr(line, static1) != NULL) return 0;
		else if (strstr(line, static2) != NULL) return 0;
		else if (strstr(line, static3) != NULL) return 0;
	}
	if (traceflag) printf("End of prolog at %s", line);
	return -1;
}

int getfontname (FILE *input) {
	char *s;
	rewind(input);
	while (fgets(line, sizeof(line), input) != NULL) {
/*		version, FullName, FamilyName, Weight, ItalicAngle, isFixedPitch */
/*		Encoding, FDepVector, UniqueID, FontName */
		if ((s = strstr(line, "CompNF")) != NULL) {
			if (strncmp(line, "/CompNF", 7) == 0) continue;
			if (*line == '/') {
				s = line;
				while (*s > ' ') s++;
				*s = '\0';
				FontName = _strdup(line+1);
				if (verboseflag) printf("FontName %s\n", FontName);
				return 0;
			}
		}
		else if (strstr(line, static0) != NULL) break;
		else if (strstr(line, static1) != NULL) break;
		else if (strstr(line, static2) != NULL) break;
		else if (strstr(line, static3) != NULL) break;
	}
	if (traceflag) printf("End of prolog at %s", line);
	return -1;
}

unsigned int getcharcode(FILE *input) {
	long current;
	int k, c;
	unsigned int code=0;
	char *s;
	
	current = ftell(input);
	while ((s = fgets(line, sizeof(line), input)) != NULL) {
		if (strstr(line, "endchar") != NULL) break;
	}
	if (s == NULL) {
		if (verboseflag) putc('\n', stdout);
		if (verboseflag) printf ("WARNING: hit EOF in getcharcode\n");
		return 0XFFFF;
	}
	fgets(line, sizeof(line), input);
	if (*line != '<') {
		printf("Not CompD line %s", line);
		return 0XFFFF;
	}
	s = line+1;
	for (k = 0; k < 4; k++) {
		c = *s++;
		if (c >= '0' && c <= '9') c = c - '0';
		else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
		else printf("Not hex %d\n", c);
		code = (code << 4) | c;
	}
	
	fseek (input, current, SEEK_SET);
	return code;
}

/*************************************************************************/

int isstraight (int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
	int dx, dy, dxa, dya, dxb, dyb;
	dx = x3 - x0;
	dy = y3 - y0;	
	dxa = x1 - x0;
	dya = y1 - y0;
	dxb = x3 - x2;
	dyb = y3 - y2;	
	if ((dx * dya - dy * dxa) == 0 &&
		(dx * dyb - dy * dxb) == 0) return 1;
	else return 0;
}

void doclosepath(FILE *output) {
	fputs("h\n", output);
	closed = 1;
}


void domoveto(FILE *output, int x, int y) {
	fprintf(output, "%d %d m\n", x, y);
	closed = 0;
}

void dolineto(FILE *output, int xold, int yold, int x, int y) {
	if (xold == x && yold == y) {
		if (traceflag) printf("(%d %d) %d %d lineto removed\n", xold, yold, x, y);
		lineflushed++;
	}
	else fprintf(output, "%d %d l\n", x, y);
}

/* can get called recursively from splitextrema */

void docurveto(FILE *output, int xold, int yold,
			   int x1, int y1, int x2, int y2, int x3, int y3) {
	double t;
	if ((xold == x1 && x1 == x2 && x2 == x3) ||
		(yold == y1 && y1 == y2 && y2 == y3)) {
		if (traceflag)
			printf("(%d %d) %d %d %d %d %d %d curveto converted to (%d %d) %d %d lineto\n",
				   xold, yold, x1, y1, x2, y2, x3, y3, xold, yold, x, y);
		dolineto(output, xold, yold, x3, y3);
		straightened++;
	}
	else if (xold == x1 && yold == y1 && x2 == x3 && y2 == y3) {
		if (traceflag)
			printf("(%d %d) %d %d %d %d %d %d curveto converted to (%d %d) %d %d lineto\n",
				   xold, yold, x1, y1, x2, y2, x3, y3, xold, yold, x, y);
		dolineto(output, xold, yold, x3, y3);
		straightened++;
	}
	else if (isstraight(xold, yold, x1, y1, x2, y2, x3, y3)) {
		if (traceflag)
			printf("(%d %d) %d %d %d %d %d %d curveto converted to (%d %d) %d %d lineto\n",
				   xold, yold, x1, y1, x2, y2, x3, y3, xold, yold, x, y);
		dolineto(output, xold, yold, x3, y3);
		straightened++;
	}
	else if (splitflag && (t = extrema(xold, x1, x2, x3)) > 0.0 && t < 1.0) 
		splitextrema(output, t, xold, yold, x1, y1, x2, y2, x3, y3);
	else if (splitflag && (t = extrema(yold, y1, y2, y3)) > 0.0 && t < 1.0) 
		splitextrema(output, t, xold, yold, x1, y1, x2, y2, x3, y3);
	else fprintf(output, "%d %d %d %d %d %d c\n", x1, y1, x2, y2, x3, y3);
}

void closepath(FILE *output) {
	doclosepath(output);
}

void moveto(FILE *output) {
	int dx=0, dy=0;
	if (closed == 0) {
		if (traceflag) printf("Error: path not closed: %s", line);
		closepath(output);
	}
	if (debugflag) printf("MOVETO line: %s", line);
/*	rmoveto, vmoveto, hmoveto */	
	if (strstr(line, "rmoveto") != NULL) {
		if (sscanf (line, "%d %d", &dx, &dy) < 2) {
			printf("Don't understand %s", line);
		}
	}
	else if (strstr(line, "hmoveto") != NULL) {
		if (sscanf (line, "%d", &dx) < 1) {
			printf("Don't understand %s", line);
		}
	}
	else if (strstr(line, "vmoveto") != NULL) {
		if (sscanf (line, "%d", &dy) < 1) {
			printf("Don't understand %s", line);
		}
	}
	else printf("Don't understand %s", line);
	x += dx;
	y += dy;
	domoveto(output, x, y);
}

void lineto(FILE *output) {
	int dx=0, dy=0;
	int xold = x, yold = y;
	if (debugflag) printf("LINETO line: %s", line);
	if (closed) printf("Error: path closed: %s", line);
/*	rlineto, vlineto, hlineto */	
	if (strstr(line, "rlineto") != NULL) {
		if (sscanf (line, "%d %d", &dx, &dy) < 2) {
			printf("Don't understand %s", line);
		}
	}
	else if (strstr(line, "hlineto") != NULL) {
		if (sscanf (line, "%d", &dx) < 1) {
			printf("Don't understand %s", line);
		}
	}
	else if (strstr(line, "vlineto") != NULL) {
		if (sscanf (line, "%d", &dy) < 1) {
			printf("Don't understand %s", line);
		}
	}
	else printf("Don't understand %s", line);
	x += dx;
	y += dy;
	dolineto(output, xold, yold, x, y);
}

void curveto(FILE *output) {
	int dx1=0, dy1=0, dx2=0, dy2=0, dx3=0, dy3=0;
	int x1, y1, x2, y2, x3, y3;
	int xold = x, yold = y;

	if (debugflag) printf("CURVETO line: %s", line);
	if (closed) printf("Error: path closed: %s", line);
/*	rrcurveto, vhcurveto, hvcurveto */	
	if (strstr(line, "rrcurveto") != NULL) {
		if (sscanf (line, "%d %d %d %d %d %d",
					&dx1, &dy1, &dx2, &dy2, &dx3, &dy3) < 6) {
			printf("Don't understand %s", line);
		}
	}
	else if (strstr(line, "hvcurveto") != NULL) {
		if (sscanf (line, "%d %d %d %d",
					&dx1, &dx2, &dy2, &dy3) < 4) {
			printf("Don't understand %s", line);
		}

	}
	else if (strstr(line, "vhcurveto") != NULL) {
		if (sscanf (line, "%d %d %d %d",
					&dy1, &dx2, &dy2, &dx3) < 4) {
			printf("Don't understand %s", line);
		}
	}
	else printf("Don't understand %s", line);
	x1 = x + dx1;
	y1 = y + dy1;
	x2 = x1 + dx2;
	y2 = y1 + dy2;
	x3 = x2 + dx3;
	y3 = y2 + dy3;
	x += (dx1 + dx2 + dx3);
	y += (dy1 + dy2 + dy3);
/*	dx1 dy1 (dx1+dx2) (dy1 + dy2) (dx1+dx2+dx3) (dy1+dy2+dy3) rcurveto */
	docurveto(output, xold, yold, x1, y1, x2, y2, x3, y3);
}

/*************************************************************************/

int processsub  (FILE *output, FILE *input) {

	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line == '\n') continue;
		if (strstr(line, "hsbw") != NULL) break;
		if (strstr(line, "showpage") != NULL) return -1;
	}
	if (sscanf(line, "%d %d hsbw", &sidebear, &width) < 2) {
		printf("Don't understand %s", line);
	}
	charcode = getcharcode(input);
	if (charcode == 0xFFFF) {
		printf("WARNING: hit EOF while looking for next charcode\n");
		return -1;
	}
	chrs = charcode & 255;
	if (chrs < 33 || chrs > 126) printf("WARNING: unusual char code %d\n", chrs);
	if (hits[chrs] != 0) printf("WARNING: repeated char code %d\n", chrs);
	hits[chrs]++;
	charcount++;
	if (traceflag) printf("%04X %d %d %d hsbw\n", charcode, chrs, sidebear, width);
	else if (verboseflag) putc('.', stdout);
	fprintf(output, "%d %d %% %04X\n", chrs, width, charcode);
	closed = 1;
	x = sidebear;
	if (m31 != 0.0) x += (int) (m31 / m11);
	y = 0;
	if (m32 != 0.0) y += (int) (m32 / m22);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (debugflag) printf("WHILE line: %s", line);
		if (strstr(line, "showpage") != NULL) return -1;
		if (strstr(line, "endchar") != NULL) break;
		if (strstr(line, "moveto") != NULL) moveto(output);
		if (strstr(line, "lineto") != NULL) lineto(output);
		if (strstr(line, "curveto") != NULL) curveto(output);
		if (strstr(line, "closepath") != NULL) closepath(output);
	}
	fputs("]\n", output);
/*	chrs++; */
	return 0;
}

int processoutlines (FILE *output, FILE *input) {
	int k;
/*	chrs=33; */						/* arbitrary */
	splits = lineflushed = straightened = 0;
	charcount = 0;
	for (k = 0; k < MAXCHRS; k++) hits[k] = 0;
/*	spit out header */
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line == '\n') continue;
		if (strstr(line, "showpage") != NULL) {
			printf("Hit showpage at %ld bytes in %s",
				   ftell(input), line);
			break;
		}
		if (processsub(output, input) < 0) {
			if (traceflag)
				printf("Hit end at %ld bytes in %s", ftell(input), line);
			break;
		}
	}	
	if (verboseflag) putc('\n', stdout);
	printf("Processed %d glyphs\n", charcount);
	if (splits > 0)
		printf("Split %d curveto's at points of tangency\n", splits);
	if (straightened > 0)
		printf("Turned %d straight curveto into lineto\n", straightened);
	if (lineflushed > 0)
		printf("Flushed %d zero length lineto\n", lineflushed);
	return 0;
}

void writeheader (FILE *output, char *outfile) {
	fprintf(output, "%%%% %s %s\n", outfile, FontName);
	fprintf(output, "%lg %lg\n", ptsize, scale);
	fprintf(output, "%d %d %d %d\n", xll, yll, xur, yur);
	fprintf(output, "]\n");
}

/**************************************************************************/

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

char *extractfilename(char *pathname) {
	char *s;
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

int commandline (int argc, char *argv[], int firstarg) {
	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag = ! verboseflag;
		if (strcmp(argv[firstarg], "-t") == 0) traceflag = ! traceflag;
		if (strcmp(argv[firstarg], "-d") == 0) debugflag = ! debugflag;
		if (strcmp(argv[firstarg], "-s") == 0) splitflag = ! splitflag;
		
		firstarg++;
	}
	return firstarg;
}

int main(int argc, char *argv[]) {
	char infile[FILENAME_MAX], outfile[FILENAME_MAX];
	FILE *output, *input;
	int firstarg=1;
	int m, psfiles=0;

	if (argc < 2) exit(1);

	firstarg = commandline (argc, argv, firstarg);

	for (m = firstarg; m < argc; m++) {
		strcpy(infile, argv[m]);
		extension(infile, "dec");
		strcpy(outfile, extractfilename(infile));
		forceexten(outfile, "out");
		if ((input = fopen(infile, "rb")) == NULL) {
			perror(infile);
			continue;
		}
		if ((output = fopen(outfile, "w")) == NULL) {
			perror(outfile);
			fclose(input);
			continue;
		}
		if (verboseflag) printf("%s => %s\n", infile, outfile);
		if (getfontname(input) < 0) {
			fprintf(errout, "WARNING: Failed to read font name\n");
		}
		if (readheader(input) < 0) {
			fprintf(errout, "ERROR: Failed to read header\n");
			fclose(output);
			fclose(input);
			continue;
		}
		if (traceflag) printf("Writing header for %s\n", FontName);
		writeheader(output, outfile);
		if (verboseflag) printf("Processing %s (%s)\n", FontName, infile);
		processoutlines(output, input);
		if (verboseflag) printf("Processed  %s (%s)\n", FontName, outfile);
		fclose(output);
		fclose(input);
		fflush(stdout);
		psfiles++;
	}
	printf("Processed %d compound fonts\n", psfiles);
	return 0;
}

/* avoid 'h' when closing empty character ? */

/*	decrypt -vsrb *.ps */
/*	dejis *.dec */
/*	converto -v *.out */
/*	fontone -v *.hex */

