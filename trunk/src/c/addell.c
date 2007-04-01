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

/* construct addditional glyphs for European Modern EM fonts */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 256
#define MAXNAME 128

#define MAXOUTLINE 256

#define EXTENSION "ext"

char line[MAXLINE];

long present;

int verboseflag=1;
int showcharstart=0;
int traceflag=1;

int ellipsisflag=1;
int perthousandflag=1;
int quotesingleflag=1;
int brokenbarflag=1;
int dcroatflag=1;
int underlineflag=1;
int asciitildeflag=1;
int asciicircumflag=1;
int ogonekflag=1;
int quotereversedflag=1;
int quotedblreversedflag=1;
int commaaccentflag=1;
int commaaccentedflag=1;
int periodcenteredflag=1;
int bulletflag=1;

int upright=1;				/* not italic or slanted or funky */
int italicflag=0;			/* -14.036 degrees, slant -0.25 */
int slantedflag=0;			/*  -9.46  degrees, slant -0.1666666 */
int oblique=0;				/* -12.0   degrees, slant -0.2125566 */
int funkyflag=0;			/* +/-5.71 degrees, slant +/-0.1 */

int fixedwidth=0;
int boldflag=0;
int smallcaps=0;
int sansserif=0;

double slant;

int axis=250;

/************************************************************************************/

/* the following glyphs get modified, so should not be copied directly */

char *omitnames[] = {"underline", "asciitilde", "asciicircum", "ogonek", "" };

int outindex;

int xout[MAXOUTLINE], yout[MAXOUTLINE], knot[MAXOUTLINE];

/************************************************************************************/

int round (double x) {
	if (x < 0) return (- round(-x));
	else return (int) (x + 0.5);
}

/************************************************************************************/

int isititalic(char *name) {
	if (strstr(name, "ti") == NULL &&
		strstr(name, "TI") == NULL &&
		strchr(name, 'i') == NULL &&
		strchr(name, 'I') == NULL) return 0;
	if (_strnicmp(name, "emti", 4) == 0) return 1;		/* 8, 9, 10, 12 */
	if (_strnicmp(name, "embxti", 6) == 0) return 1;	/* 10 */
	if (_strnicmp(name, "emitt", 5) == 0) return 1;		/* 10 */
	if (_strnicmp(name, "dmti", 4) == 0) return 1;		/* 8, 9, 10, 12 */
	if (_strnicmp(name, "dmbxti", 6) == 0) return 1;	/* 10 */
	if (_strnicmp(name, "dmitt", 5) == 0) return 1;		/* 10 */
	return 0;
}

int isitslanted(char *name) {
	if (strstr(name, "sl") == NULL &&
		strstr(name, "SL") == NULL) return 0;
	if (_strnicmp(name, "emsl", 4) == 0) return 1;		/* 8, 9, 10, 12 */
	if (_strnicmp(name, "embxsl", 6) == 0) return 1;	/* 10 */
	return 0;
}

int isitoblique(char *name) {
	if (strchr(name, 'i') == NULL &&
		strchr(name, 'I') == NULL) return 0;
	if (strstr(name, "ss") == NULL &&
		strstr(name, "SS") == NULL) return 0;
	if (_strnicmp(name, "emssi", 5) == 0) return 1;		/* 8, 9, 10, 12, 17 */
	if (_strnicmp(name, "emssqi", 6) == 0) return 1;	/* 8 */
	return 0;
}

int isitfunky(char *name) {
	if (strchr(name, 'f') == NULL &&
		strchr(name, 'F') == NULL) return 0;
	if (_strnicmp(name, "emff", 4) == 0) return 1;
	if (_strnicmp(name, "emfi", 4) == 0 &&
		_strnicmp(name, "emfib", 5) != 0) return 1;
	return 0;

}

int isitbold(char *name) {
	if (strstr(name, "bx") == NULL &&
		strstr(name, "BX") == NULL &&
		strchr(name, 'b') == NULL &&
		strchr(name, 'B') == NULL) return 0;
	if (_strnicmp(name, "embx", 4) == 0) return 1;		/* 5, 6, 7, 8, 9, 10, 12 */
	if (_strnicmp(name, "emb", 3) == 0) return 1;		/* 10 */
	if (_strnicmp(name, "emssbx", 6) == 0) return 1;	/* 10 */
	return 0;

}

int isitfixedwidth(char *name) {
	if (strstr(name, "tt") == NULL &&
		strstr(name, "TT") == NULL &&
		strstr(name, "tc") == NULL &&
		strstr(name, "TC") == NULL) return 0;
	if (_strnicmp(name, "emtt", 4) == 0) return 1;		/* 8, 9, 10, 12 */
	if (_strnicmp(name, "emitt", 5) == 0) return 1;		/* 10 */
	if (_strnicmp(name, "emsltt", 6) == 0) return 1;	/* 10 */
	if (_strnicmp(name, "emtcsc", 6) == 0) return 1;	/* 10 */
	if (_strnicmp(name, "emvtt", 5) == 0) return 0;		/* 10 */
	return 0;
}

int isitsmallcaps(char *name) {
	if (strstr(name, "csc") == NULL &&
		strstr(name, "CSC") == NULL) return 0;
	if (_strnicmp(name, "emcsc", 5) == 0) return 1;		/* 8, 9, 10 */
	if (_strnicmp(name, "emtcsc", 6) == 0) return 1;	/* 10 */
	return 0;

}

int isitsansserif(char *name) {
	if (strstr(name, "ss") == NULL &&
		strstr(name, "SS") == NULL) return 0;
	if (_strnicmp(name, "emss", 4) == 0) return 1;
	return 0;

}

void getstyle(char *name) {
	italicflag = isititalic(name);
	slantedflag = isitslanted(name);
	oblique = isitoblique(name);
	funkyflag = isitfunky(name);
	boldflag = isitbold(name);
	fixedwidth = isitfixedwidth(name);
	smallcaps = isitsmallcaps(name);
	sansserif = isitsansserif(name);
	if (italicflag || slantedflag || oblique || funkyflag) upright = 0;
	else upright = 1;
	if (italicflag) slant = -0.25;
	else if (oblique) slant = -0.2125566;
	else if (slantedflag) slant = -0.16666666;
	else if (funkyflag) {
		if (_strnicmp(name, "emfi", 4) == 0) slant = -0.1;
		else if (_strnicmp(name, "emff", 4) == 0) slant = 0.1;
		else slant = 0;
	}
	else slant = 0.0;

/*	if (verboseflag)
	printf("STYLE: %s%s%s%s%s%s%s%s",
		(upright && boldflag == 0) ? "REGULAR " : "",
		boldflag ? "BOLD " : "",
		italicflag ? "ITALIC " : "",
		slantedflag ? "SLANTED " : "",
		oblique ? "OBLIQUE " : "",
		funkyflag ? "FUNKY " : "",
		fixedwidth ? "FIXEDWIDTH " : "",
		sanserifflag ? "SANSSERIF " : "",
		smallcaps ? "SMALLCAPS "	 : "");
		if (slant != 0.0) printf(" SLANT %lg\n", slant);
		else printf("\n"); */
}

/****************************************************************************************/

int readoutline (FILE *in) {
	int x, y;
	int xa, ya, xb, yb, xc, yc;
	outindex = 0;
	while (fgets(line, sizeof(line), in) != NULL) {
		if (*line == '%' || *line <= ' ') continue;
		if (*line == 'h') return 0;			/* end of an outline */
		if (*line == ']') {					/* nothing more */
			if (outindex > 0) printf("End of char outindex %d not zero\n", outindex);
			return -1;
		}
		if (strstr(line, " m") != NULL) {
			sscanf (line, "%d %d", &x, &y);
			xout[outindex] = x;
			yout[outindex] = y;
			knot[outindex] = 'm';
			outindex++;
		}
		else if (strstr(line, " l") != NULL) {
			sscanf (line, "%d %d", &x, &y);
			xout[outindex] = x;
			yout[outindex] = y;
			knot[outindex] = 'l';
			outindex++;
		}
		else if (strstr(line, " c") != NULL) {
			sscanf (line, "%d %d %d %d %d %d", &xa, &ya, &xb, &yb, &xc, &yc);
			xout[outindex] = xa;
			yout[outindex] = ya;
			knot[outindex] = ' ';
			outindex++;
			xout[outindex] = xb;
			yout[outindex] = yb;
			knot[outindex] = ' ';
			outindex++;
			xout[outindex] = xc;
			yout[outindex] = yc;
			knot[outindex] = 'c';
			outindex++;
		}
		
	}
}

void flipsignx(void) {
	int k;
	for (k = 0; k < outindex; k++) {
		xout[k] = - xout[k];
	}
}

void reverseoutline (FILE *out, int xoffset, int yoffset, double slant) {
	int k;
	int x, y;
	int previous='m';

	for (k = outindex-1; k >= 0; k--) {
		x = - xout[k];					/* flip sign on x */
		y =   yout[k];
		if (slant != 0.0) x = x - round (2.0 * slant * (double) y);
		if (previous == 'm') {
			if (xoffset != 0 || yoffset != 0) 
				fprintf(out, "%d %d m %% offset %d %d\n", x, y, xoffset, yoffset);
			else fprintf(out, "%d %d m\n", x, y);
		}
		else if (previous == 'l')
			fprintf(out, "%d %d l\n", x, y);
		else if (previous == 'c') 
			fprintf(out, "%d %d ", x, y);
		else if (previous == ' ') {
			fprintf(out, "%d %d ", x, y);
			if (knot[k] != ' ') fprintf(out, "c\n");
		}
		previous = knot[k];
	}
	if (knot[0] != 'm') printf("First knot not moveto\n");
	fprintf(out, "h\n");
}

/************************************************************************************/

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

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/************************************************************************************/

int bracketflag=0;

void copysimple(FILE *out, FILE *in) {
	while (fgets(line, sizeof(line), in) != NULL) {
		if (*line == ']') bracketflag = 1;
		else if (*line == 'h') bracketflag = 0;
		else if (*line >= '0' && *line <= '9') bracketflag = 0;
		fputs(line, out);
	}
}

int ignorechar (char *name) {
	int k;
	for(k=0; k < 256; k++) {
		if (strcmp(omitnames[k], "") == 0) return 0;
		if (strcmp(omitnames[k], name) == 0) return 1;
	}
	return 0;
}

void copyfile(FILE *out, FILE *in) {
	int chr, wx;
	char charname[MAXNAME];
	int copyflag=1;

	while (fgets(line, sizeof(line), in) != NULL) {
		char *s;
		if (*line == ']') {
			bracketflag = 1;
			copyflag = 1;
			while ((s = fgets(line, sizeof(line), in)) != NULL) {
				if (*line != '%' && *line > ' ') break;
			}
			if (s == NULL) {
				if (copyflag) fputs("]\n", out);
				return;				/* hit EOF */
			}
			if (sscanf (line, "%d %d %% %s", &chr, &wx, charname) < 3) {
				printf("Don't understand %s", line);
			}
			if (ignorechar (charname)) copyflag = 0;
			if (copyflag) fputs("]\n", out);
		}
		else if (*line == 'h') bracketflag = 0;
		else if (*line >= '0' && *line <= '9') bracketflag = 0;
		if (copyflag) fputs(line, out);
	}
}

int scantochar(FILE *in, char *name) {
	char *s, *t;
	while (fgets(line, sizeof(line), in) != NULL) {
		if (*line == ']') {
			present = ftell(in);
			while (fgets(line, sizeof(line), in) != NULL) {
				if (*line != '%' && *line > ' ') break;
			}
			if ((s = strchr(line, '%')) != NULL) {
				s++;
				while (*s <= ' ' && *s != '\0') s++;
				t = s + strlen(s) - 1;
				while (*t <= ' ' && t > s) *t-- = '\0';
/*				if (traceflag) printf("Testing %s versus %s\n", s, name); */
				if (strcmp(s, name) == 0) {	/* put back terminator */
					t++;
					*t++ = '\n';
					*t++ = '\0';
					return 0;
				}
			}
		}
	}
	printf("WARNING: Unable to find %s\n", name);
	return -1;					/* failed */
}

int copychar (FILE *out, FILE *in, int xoffset, int yoffset) {
	char *s;
	(void) fgets(line, sizeof(line), in);
	if (strstr(line, " m") == NULL)
		printf("Line not moveto line: %s", line);
	if (xoffset != 0 || yoffset != 0) {
		s = line + strlen(line) - 1;
		while (*s <= ' ' && s > line) *s-- = '\0';
		sprintf(s+1, " %% offset %d %d\n", xoffset, yoffset);
	}
	fputs(line, out);
	while (fgets(line, sizeof(line), in) != NULL) {
		if (*line == ']') return 0;
		if (strstr(line, " s %") != NULL) continue;	/* flush hint switch */
		fputs(line, out);
	}
	return -1;				/* failed */
}

int copyfirstoutline (FILE *out, FILE *in, int xoffset, int yoffset) {
	char *s;
	(void) fgets(line, sizeof(line), in);
	if (strstr(line, " m") == NULL)
		printf("Line not moveto line: %s", line);
	if (xoffset != 0 || yoffset != 0) {
		s = line + strlen(line) - 1;
		/*		printf("s: %s", s); */
		while (*s <= ' ' && s > line) *s-- = '\0';
		sprintf(s+1, " %% offset %d %d\n", xoffset, yoffset);
	}
	fputs(line, out);
	while (fgets(line, sizeof(line), in) != NULL) {
		if (*line == ']') return 0;
		if (strstr(line, " s %") != NULL) continue;	/* flush hint switch */
		fputs(line, out);
		if (*line == 'h') return 0;		/* return after end of first outline */
	}
	return -1;				/* failed */
}

int copysecondoutline (FILE *out, FILE *in, int xoffset, int yoffset) {
	char *s;
	while (fgets(line, sizeof(line), in) != NULL) {
		if (*line == 'h') break;
		if (*line == ']') return -1;	/* failed */
	}
	(void) fgets(line, sizeof(line), in);
	if (strstr(line, " m") == NULL)
		printf("Line not moveto line: %s", line);
	if (xoffset != 0 || yoffset != 0) {
		s = line + strlen(line) - 1;
		/*		printf("s: %s", s); */
		while (*s <= ' ' && s > line) *s-- = '\0';
		sprintf(s+1, " %% offset %d %d\n", xoffset, yoffset);
	}
	fputs(line, out);
	while (fgets(line, sizeof(line), in) != NULL) {
		if (*line == ']') return 0;
		if (strstr(line, " s %") != NULL) continue;	/* flush hint switch */
		fputs(line, out);
		if (*line == 'h') return 0;		/* return after end of first outline */
	}
	return -1;				/* failed */
}

int outlinenum;

int xll, yll, xur, yur;

#define MAXOUTLINES 16	

int xlls[MAXOUTLINES];
int ylls[MAXOUTLINES];
int xurs[MAXOUTLINES];
int yurs[MAXOUTLINES];

void updatebbox (int x, int y) {
	if (x < xll) xll = x;
	else if (x > xur) xur = x;	
	if (y < yll) yll = y;
	else if (y > yur) yur = y;	
}

void combinebbox(void) {
	int k;
	xll = xlls[0]; yll = ylls[0]; xur = xurs[0]; yur = yurs[0];
/*	printf("outlinenum %d\n", outlinenum); */
/*	printf("xll %d yll %d xur %d yur %d\n", xll, yll, xur, yur); */
	for (k = 1; k < outlinenum; k++) {
		updatebbox(xlls[k], ylls[k]);
		updatebbox(xurs[k], yurs[k]);
/*		printf("xll %d yll %d xur %d yur %d\n", xll, yll, xur, yur); */
	}
}

int charbbox(FILE *in) {
	int x=0, y=0;
	int xa, ya, xb, yb, xc, yc;

	outlinenum=0;
/*	(void) fgets(line, sizeof(line), in);
	if (strstr(line, " m") != NULL)
		sscanf (line, "%d %d", &x, &y);
	else printf("Line not moveto line: %s", line); */
/*	xll = x; yll = y; xur = x; yur = y; */

	while (fgets(line, sizeof(line), in) != NULL) {
		if (strstr(line, " m") != NULL) {
			sscanf (line, "%d %d", &x, &y);
			xll = x; yll = y; xur = x; yur = y;		/* reset for outline */
		}
		else if (strstr(line, " l") != NULL) {
			sscanf (line, "%d %d", &x, &y);
			updatebbox(x, y);
		}
		else if (strstr(line, " c") != NULL) {
			sscanf (line, "%d %d %d %d %d %d", &xa, &ya, &xb, &yb, &xc, &yc);
			updatebbox(xa, ya);
			updatebbox(xb, yb);
			updatebbox(xc, yc);
		}
		else if (*line == 'h') {
			xlls[outlinenum] = xll;
			ylls[outlinenum] = yll;
			xurs[outlinenum] = xur;
			yurs[outlinenum] = yur;
			outlinenum++;
			if (outlinenum >= MAXOUTLINES)
				printf("TOO MANY OUTLINES IN CHARACTER\n");
			continue;
		}
		else if (*line == ']') {
			combinebbox();
			return 0;
		}
	}
	combinebbox();
	return -1;				/* failed */
}

/**************************************************************************************/

int reduceellipsis=50;		/* How much to clip out of gap between */

double alpha = 0.7071;		/* lsb and rsb in ellipsis */
double beta = 1.732;		/* gap between dots in ellipsis */

int chr, wx;

long here;

int makeellipsissub (FILE *out, FILE *in) {
	int newwx, dotsize, offset1, offset2, offset3;
	int side, gap;
	double xscale;

	dotsize = xur - xll;
	if (fixedwidth) {
		offset1 = reduceellipsis - xll;
		offset2 = 2 * reduceellipsis + dotsize + offset1;
		offset3 = 2 * reduceellipsis + dotsize + offset2;
/*		newwx = offset3 + reduceellipsis + dotsize; */
		newwx = offset3 + xur + reduceellipsis;
	}
	else {
/*		offset1 = 0;
		offset2 = wx - reduceellipsis;
		offset3 = wx * 2 - reduceellipsis * 2;
		newwx   = wx * 3 - reduceellipsis * 2; */
/*		side = round(alpha * dotsize); */
		side = (round(alpha * dotsize) + xll) / 2;
/*		gap  = round(beta * dotsize) ; */
		gap  = (round(beta * dotsize) + 2 * xll) / 2 ;
		offset1 = side - xll;
		offset2 = side + dotsize + gap - xll;
		offset3 = side + (dotsize + gap) * 2 - xll;		
		newwx = (side + gap) * 2 + dotsize * 3;
	} 

	if (fixedwidth) {
		xscale = (double) wx / newwx;
		sprintf(line, "%d %d %% %s %% scale %lg %lg\n",
							133, newwx, "ellipsis", xscale, 1.0);
	}
	else sprintf(line, "%d %d %% %s\n", 133, newwx, "ellipsis");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	fseek (in, here, SEEK_SET);
	if (copychar(out, in, offset1, 0)) return -1;
	fseek (in, here, SEEK_SET);
	if (copychar(out, in, offset2, 0)) return -1;
	fseek (in, here, SEEK_SET);
	if (copychar(out, in, offset3, 0)) return -1;
	return 0;
}

int makeellipsis(FILE *out, FILE *em, FILE *cm) {
	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(cm);
	if (scantochar(cm, "period")) return -1;
	here = ftell(cm);
	sscanf(line, "%d %d", &chr, &wx);
	charbbox(cm);
	rewind(cm);
	if (scantochar(cm, "period")) return -1;	
	makeellipsissub(out, cm);
	bracketflag = 0;
}

int ogonekshift=100;

int makeogonek(FILE *out, FILE *em, FILE *cm) {
	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(em);
	if (scantochar(em, "ogonek")) return -1;
/*	here = ftell(cm); */
	sscanf(line, "%d %d", &chr, &wx);
	if (fixedwidth == 0)
		sprintf(line, "%d %d %% %s\n", chr, wx+ogonekshift, "ogonek");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	if (fixedwidth) copychar(out, em, 0, 0);
	else copychar(out, em, ogonekshift, 0);
	bracketflag = 0;
}


int reduceperthousand=60;		/* How much to clip out of gap between */

int makeperthousand (FILE *out, FILE *em, FILE *cm) {
	int chr1, wx1, chr2, wx2;
	int newwx, offset;
	double xscale;
	
	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(cm);
	if (scantochar(cm, "percent")) return -1;
	sscanf(line, "%d %d", &chr1, &wx1);
/*	offset = wx1 - reduceperthousand; */
	rewind(em);
	if (scantochar(em, "perzero")) return -1;
	charbbox(em);
	if (fixedwidth) offset = wx1 - xll;
	else offset = wx1 - reduceperthousand;
	rewind(em);
	if (scantochar(em, "perzero")) return -1;
	sscanf(line, "%d %d", &chr2, &wx2);
	if (fixedwidth) newwx = wx1 + (xur - xll) + reduceperthousand;
	else newwx = wx1 + wx2 - reduceperthousand;
	xscale = (double) wx1 / newwx;
	if (fixedwidth)
		sprintf(line, "%d %d %% %s %% scale %lg %lg\n", 138, newwx,
				"perthousand", xscale, 1.0);
	else sprintf(line, "%d %d %% %s\n", 138, newwx, "perthousand");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	copychar(out, cm, 0, 0);
	copychar(out, em, offset, 0);
	bracketflag = 0;
}


int makequotesingle (FILE *out, FILE *em, FILE *cm) {
	int chr2, wx2;
	int newwx;
	int xoffset=0;
	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(em);
	if (scantochar(em, "quotedbl")) return -1;
	sscanf(line, "%d %d", &chr2, &wx2);
	charbbox (em);					/* get bounding box */
	newwx = wx2 - (xur - xurs[0]);
	if (fixedwidth) xoffset = (wx - newwx) / 2;
	else xoffset = 0;
	rewind(em);
	if (scantochar(em, "quotedbl")) return -1;
	if (fixedwidth) sprintf(line, "%d %d %% %s\n", 35, wx, "quotesingle");
	else sprintf(line, "%d %d %% %s\n", 35, newwx, "quotesingle");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	copyfirstoutline(out, em, xoffset, 0);
	bracketflag = 0;
}


int makebrokenbar (FILE *out, FILE *em, FILE *cm) {
	int chr, wx;
	double scale;
	int xoffset1, xoffset2;
	int yoffset1, yoffset2;

	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(em);
	if (scantochar(em, "bar")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	charbbox (em);					/* get bounding box */
	scale = 320.0 / ((double) yur - yll);
	if (slant == 0)
		sprintf(line, "%d %d %% %s %% scale 1.0 %lg\n", 140, wx, "brokenbar", scale);
	else sprintf(line, "%d %d %% %s %% scale 1.0 %lg 0 0 %lg\n",
				 140, wx, "brokenbar", scale, -slant);
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	xoffset1 = 0;
	if (slant != 0.0) xoffset1 = round(slant * yll);
	yoffset1 = (-yll);
/*	printf("Offset1 %d (-yll) %d scale %lg\n", offset1, (-yll), scale); */
	rewind(em);
	if (scantochar(em, "bar")) return -1;
	copychar(out, em, xoffset1, yoffset1);
	yoffset2 = (-yll) + round (430.0 / scale);
	if (slant == 0) xoffset2 = 0;
	else xoffset2 = round (slant * ((double) yll - (430.0 / scale)));

	rewind(em);
	if (scantochar(em, "bar")) return -1;
	copychar(out, em, xoffset2, yoffset2);
	bracketflag = 0;
}

int makedcroat (FILE *out, FILE *em, FILE *cm) {
	int chr, wx;

	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(em);
	if (scantochar(em, "Eth")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	sprintf(line, "%d %d %% %s\n", 142, wx, "Dcroat");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	copychar(out, em, 0, 0);
	bracketflag = 0;
}

int makeunderline (FILE *out, FILE *em, FILE *cm) {
	int chr, wx;
	int offset;

	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(em);
	if (scantochar(em, "underline")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	charbbox (em);					/* get bounding box */

	offset = -100 - yur;
	rewind(em);
	if (scantochar(em, "underline")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	copychar(out, em, 0, offset);
	bracketflag = 0;
}

double asciitildestretch= 1.21;
/* int asciitildeaxis=250; */

int makeasciitilde (FILE *out, FILE *em, FILE *cm) {
	int chr, wx;
	int xoffset, yoffset, aver;
	char *s;

	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(em);
	if (scantochar(em, "asciitilde")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	charbbox (em);					/* get bounding box */

	aver = (yll + yur) / 2;
/*	printf("xll %d yll %d xur %d yur %d aver %d\n", xll, yll, xur, yur, aver); */
	yoffset = round ((double) axis / asciitildestretch - aver);
	xoffset = round (-slant * (double) yoffset);
	rewind(em);
	if (scantochar(em, "asciitilde")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	s = line + strlen(line) - 1;
	while (*s <= ' ' && s > line) *s-- = '\0';
	sprintf(s+1, " %% scale 1.0 %lg\n", asciitildestretch);
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	copychar(out, em, xoffset, yoffset);
	bracketflag = 0;
}

double asciicircumstretch= 1.41;

int makeasciicircum (FILE *out, FILE *em, FILE *cm) {
	int chr, wx;
	char *s;
	int xoffset, yoffset;

	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(em);
	if (scantochar(em, "asciicircum")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	charbbox (em);					/* get bounding box */
	yoffset = round ((double) yur * (1.0 / asciicircumstretch - 1.0));
	xoffset = round (-slant * (double) yoffset);
	rewind(em);
	if (scantochar(em, "asciicircum")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	s = line + strlen(line) - 1;
	while (*s <= ' ' && s > line) *s-- = '\0';
	sprintf(s+1, " %% scale 1.0 %lg\n", asciicircumstretch);
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	copychar(out, em, xoffset, yoffset);
	bracketflag = 0;
}

int makequotereversed (FILE *out, FILE *em, FILE *cm) {
	int chr, wx;
	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(cm);
	if (scantochar(cm, "quoteright")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	sprintf(line, "%d %d %% %s\n", 39, wx, "quotereversed");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	readoutline(cm);
/*	flipsignx(); */
	reverseoutline (out, wx, 0, slant);
	bracketflag = 0;
}

/* NOTE: fixewd width fonts don't have quoteblright ... */

int makequotedblreversed (FILE *out, FILE *em, FILE *cm) {
	int chr, wx;
	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(cm);
	if (scantochar(cm, "quotedblright")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	sprintf(line, "%d %d %% %s\n", 40, wx, "quotedblreversed");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	readoutline(cm);
/*	flipsignx(); */
	reverseoutline (out, wx, 0, slant);
	readoutline(cm);
/*	flipsignx(); */
	reverseoutline (out, wx, 0, slant);
	bracketflag = 0;
}

/* int periocenteredaxis=250; */

int makeperiodcentered (FILE *out, FILE *em, FILE *cm) {
	int yoffset;
	int chr, wx;
	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(cm);
	if (scantochar(cm, "period")) return -1;
	charbbox(cm);
	yoffset = axis - (yur + yll) / 2;
	rewind(cm);
	if (scantochar(cm, "period")) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	sprintf(line, "%d %d %% %s\n", 183, wx, "periodcentered");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	copychar(out, cm, 0, yoffset);
	bracketflag = 0;
}


int makebullet (FILE *out, FILE *em, FILE *cm) {
	int chr, wx, newwx;
	int xcenter, ycenter, diameter;
	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(cm);

	if (scantochar(cm, "asterisk")) return -1;
	sscanf(line, "%d %d", &chr, &wx);	
	charbbox(cm);
	newwx = wx;
	diameter = (2 * (xur - xll) + (yur - yll)) / 3 ;
	xcenter = wx / 2;
	ycenter = axis;
/*	fprintf(out, "]\n"); */
	sprintf(line, "%d %d %% %s\n", 150, newwx, "bullet");	/* 149 ??? */
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	xll = xcenter - diameter / 2;
	xur = xcenter + diameter / 2;
	yll = ycenter - diameter / 2;
	yur = ycenter + diameter / 2;
	sprintf(line, " %d %d %d %d x\n", xll, yll, xur, yur);
	fputs(line, out);
	bracketflag = 0;
}

int commaclear=-60;		/* top of commaaccent */

int makecommaaccent (FILE *out, FILE *em, FILE *cm) {
	int chr, wx, newwx;
	int xll, yll, xur, yur;
	int xoffset, yoffset;
	
	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(cm);
	if (scantochar(cm, "s")) return -1;
	sscanf(line, "%d %d", &chr, &wx);	
	newwx = wx;
	rewind(em);
	if (scantochar(em, "lcaron")) return -1;
/*	sscanf(line, "%d %d", &chr, &wx); */
	charbbox(em);
	if (xlls[0] < xlls[1]) {	/* accent comes second */
		xll = xlls[1]; yll = ylls[1]; xur = xurs[1]; yur = yurs[1];
	}
	else {	/* accent comes first ! */
		xll = xlls[0]; yll = ylls[0]; xur = xurs[0]; yur = yurs[0];
	}	
	yoffset = commaclear - yur;
	xoffset = (newwx  - xll - xur) / 2;
	if (slant != 0.0) xoffset = xoffset + round (slant * 100);
	rewind(em);
	if (scantochar(em, "lcaron")) return -1;
	sprintf(line, "%d %d %% %s\n", 255, newwx, "commaaccent");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	if (xlls[0] < xlls[1]) {	/* accent comes second */
		copysecondoutline(out, em, xoffset, yoffset);	
	}
	else {
		copyfirstoutline(out, em, xoffset, yoffset);	
	}
   bracketflag = 0;
}

int tcommaadjust=20;

int makecommaaccented (FILE *out, FILE *em, FILE *cm, char *base) {
	int chr, wx, newwx;
	int xll, yll, xur, yur;
	int xoffset, yoffset;

	if (bracketflag == 0) fprintf(out, "]\n");
	rewind(cm);
	if (scantochar(cm, base)) return -1;
	sscanf(line, "%d %d", &chr, &wx);	
	newwx = wx;

	rewind(em);
	if (scantochar(em, "lcaron")) return -1;
/*	sscanf(line, "%d %d", &chr, &wx); */
	charbbox(em);
	if (xlls[0] < xlls[1]) {	/* accent comes second */
		xll = xlls[1]; yll = ylls[1]; xur = xurs[1]; yur = yurs[1];
	}
	else {	/* accent comes first ! */
		xll = xlls[0]; yll = ylls[0]; xur = xurs[0]; yur = yurs[0];
	}	
	yoffset = commaclear - yur;
	xoffset = (newwx  - xll - xur) / 2;
	if (*base == 't') xoffset += tcommaadjust;	/* adjust right in tcommaaccent */
	if (slant != 0.0) xoffset = xoffset + round (slant * 100);

	rewind(cm);
	if (scantochar(cm, base)) return -1;
	sscanf(line, "%d %d", &chr, &wx);
	sprintf(line, "%d %d %% %s%s\n", *base, wx, base, "commaaccent");
	fputs(line, out);
	if (showcharstart) fputs(line, stdout);
	copychar(out, cm, 0, 0);
	rewind(em);
	if (scantochar(em, "lcaron")) return -1;
/*	Use char as character code */
/*	sprintf(line, "%d %d %% %s\n", *base, newwx, "commaaccent"); */
/*	fputs(line, out); */
/*	if (showcharstart) fputs(line, stdout); */
	if (xlls[0] < xlls[1]) {	/* accent comes second */
		copysecondoutline(out, em, xoffset, yoffset);	
	}
	else {
		copyfirstoutline(out, em, xoffset, yoffset);	
	}
	bracketflag = 0;
}

/************************************************************************************/

int main(int argc, char *argv[]) {
	char emfile[FILENAME_MAX];
	char cmfile[FILENAME_MAX];
	char outfile[FILENAME_MAX];
	FILE *em, *cm, *out;

	int m, count=0;

	for (m = 1; m < argc; m++) {
		strcpy(emfile, argv[m]);
		extension(emfile, "out");
		strcpy(cmfile, "c:\\cmbsr\\");
		strcat(cmfile, argv[m]);
		*(cmfile+9) = 'c';				/* emfoo => cmfoo */
		extension(cmfile, "out");
		strcpy(outfile, argv[m]);
		forceexten(outfile, EXTENSION);		
		if ((em = fopen(emfile, "r")) == NULL) {
			perror(emfile);
			continue;
		}
		if ((cm = fopen(cmfile, "r")) == NULL) {
			perror(cmfile);
			continue;
		}
		if ((out = fopen(outfile, "w")) == NULL) {
			perror(outfile);
			exit(1);
		}

		getstyle(outfile);
/*		printf("%s + %s => %s\t", emfile, removepath(cmfile), outfile); */
		printf("%s +\t%s > \t%s\t", emfile, removepath(cmfile), outfile);
		if (verboseflag)
/*			printf("STYLE: %s%s%s%s%s%s%s%s", */
			printf("%s%s%s%s%s%s%s%s%s",
				   (upright && boldflag == 0) ? "REGULAR " : "",
				   boldflag ? "BOLD " : "",
				   italicflag ? "ITALIC " : "",
				   slantedflag ? "SLANTED " : "",
				   oblique ? "OBLIQUE " : "",
				   sansserif ? "SANSSERIF " : "",
				   funkyflag ? "FUNKY " : "",
				   fixedwidth ? "FIXEDWIDTH " : "",
				   smallcaps ? "SMALLCAPS "	 : "");
		if (slant != 0.0) printf(" SLANT %lg\n", slant);
		else printf("\n");
		copyfile(out, em);
		if (ellipsisflag) makeellipsis (out, em, cm);
		if (perthousandflag) makeperthousand (out, em, cm);
		if (quotesingleflag) makequotesingle (out, em, cm);
		if (brokenbarflag) makebrokenbar (out, em, cm);
		if (dcroatflag) makedcroat (out, em, cm);
		if (underlineflag) makeunderline (out, em, cm);
		if (asciitildeflag) makeasciitilde (out, em, cm);
		if (asciicircumflag) makeasciicircum (out, em, cm);
		if (ogonekflag) makeogonek(out, em, cm);
		if (quotereversedflag) makequotereversed (out, em, cm);
		if (quotedblreversedflag) makequotedblreversed (out, em, cm);
		if (periodcenteredflag) makeperiodcentered (out, em, cm);
		if (bulletflag) makebullet (out, em, cm);
		if (commaaccentflag) makecommaaccent (out, em, cm);
		if (commaaccentedflag) {
			makecommaaccented (out, em, cm, "S");
			makecommaaccented (out, em, cm, "T");
			makecommaaccented (out, em, cm, "s");
			makecommaaccented (out, em, cm, "t");
		}
		fclose (out);
		fclose (cm);
		fclose (em);
		count++;
	}
	printf("Added glyphs to %d font files\n", count);
	return 0;
}

/************************************************************************************/
