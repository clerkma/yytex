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

/* OLD, see EXTROUTL.C instead */

/* MAKEOUTL - make outlines from decrypted output */
/* run PFBTOPFA to turn PFB into PFA format */
/* run DECRYPT twice to get required input for this (or use MAKEPLN.BAT) */
/* then run MAKEOUTL */
/* run CONVERT to turn into HEX format */
/* then use SHOWCHAR to look at or FONTONE to make new font */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#define MAXLINE 512
/* #define FNAMELEN 80 */
#define CHARNAME_MAX 64
#define MAXCHRS 256
#define MAXENCODE 32
#define MAXSUBRS 1024
#define MAXSTEMS 128

int allowflex = 1;					/* if Subrs used for FlexProc */
int verboseflag = 1;
int traceflag = 0;
int hintflag = 1;

int numsubrs=0;		/* number of Subrs */
int numchars=0;		/* number of CharStrings */
int numencod=0;		/* entries in encoding vector */
int numprocs=0;		/* number of outlines actually processed */

int subrlevel=0;	/* levels down in subroutines calling each other */

int flexproc=0;		/* inside a FlexProc sequence - count of hits */

int xflex[8];		/* x coordinates from FlexProc */
int yflex[8];		/* y coordinates from FlexProc */

int xll=0, yll=0, xur=1000, yur=1000;

int sbx, sby, widthx, widthy, nchar, x, y;

static hstems[MAXSTEMS], hsteme[MAXSTEMS], vstems[MAXSTEMS], vsteme[MAXSTEMS];

int hsteminx=0, vsteminx=0;

char line[MAXLINE];

static long subrspointer[MAXSUBRS];

static long charspointer[MAXSUBRS];

static unsigned char encoding[MAXCHRS][MAXENCODE]; 

static unsigned char *standardencoding[] = { 
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright",
 "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash",
 "zero", "one", "two", "three", "four", "five", "six", "seven",
 "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question",
 "at", "A", "B", "C", "D", "E", "F", "G",
 "H", "I", "J", "K", "L", "M", "N", "O",
 "P", "Q", "R", "S", "T", "U", "V", "W",
 "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore",
 "quoteleft", "a", "b", "c", "d", "e", "f", "g",
 "h", "i", "j", "k", "l", "m", "n", "o",
 "p", "q", "r", "s", "t", "u", "v", "w",
 "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "exclamdown", "cent", "sterling", "fraction", "yen", "florin", "section",
 "currency", "quotesingle", "quotedblleft", "guillemotleft", "guilsinglleft", "guilsinglright", "fi", "fl",
 "", "endash", "dagger", "daggerdbl", "periodcentered", "", "paragraph", "bullet",
 "quotesinglbase", "quotedblbase", "quotedblright", "guillemotright", "ellipsis", "perthousand", "", "questiondown",
 "", "grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
 "dieresis", "", "ring", "cedilla", "", "hungarumlaut", "ogonek", "caron",
 "emdash", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "AE", "", "ordfeminine", "", "", "", "",
 "Lslash", "Oslash", "OE", "ordmasculine", "", "", "", "",
 "", "ae", "", "", "", "dotlessi", "", "",
 "lslash", "oslash", "oe", "germandbls", "", "", "", "",
};

int lookup(char *charname) { /* returns the first one */
	int k;
	for (k=0; k < MAXCHRS; k++)
		if (strcmp(encoding[k], charname) == 0) return k;
	return -1;
}

void makestandard (void) {
	int i;
	for (i=0; i < MAXCHRS; i++) strcpy(encoding[i], standardencoding[i]);
	numencod=192;
}

void makeempty (void) {
	int i;
	for (i=0; i < MAXCHRS; i++) strcpy(encoding[i], "");
	numencod=0;
}

/*
void inserthstem (int y, int dy) {
	hstems[hsteminx] = y + sby; 	hsteme[hsteminx] = y + dy + sby;
	hsteminx++;
	if (hsteminx >= MAXSTEMS) {
		fprintf(stderr, "Too many hstems\n");
		hsteminx--;
	}
} */

/*
void insertvstem (int x, int dx) {
	vstems[vsteminx] = x + sbx; 	vsteme[vsteminx] = x + dx + sbx;
	vsteminx++;
	if (vsteminx >= MAXSTEMS) {
		fprintf(stderr, "Too many vstems\n");
		vsteminx--;
	}
} */

int inserthstem (int y, int dy) {
	int k, ys, ye;
	
	ys = y + sby, ye = y + dy + sby;
	for(k = 0; k < hsteminx; k++) {
		if (hstems[k] == ys && hsteme[k] == ye) return k;
	}
	hstems[hsteminx] = ys;	hsteme[hsteminx] = ye;
	hsteminx++;
	if (hsteminx >= MAXSTEMS) {
		fprintf(stderr, "Too many hstems\n");
		hsteminx--;
	}
	return hsteminx-1;
} 

int insertvstem (int x, int dx) {
	int k, xs, xe;
	
	xs = x + sbx, xe = x + dx + sbx;
	for(k = 0; k < vsteminx; k++) {
		if (vstems[k] == xs && vsteme[k] == xe) return k;
	}
	vstems[vsteminx] = xs;	vsteme[vsteminx] = xe;
	vsteminx++;
	if (vsteminx >= MAXSTEMS) {
		fprintf(stderr, "Too many vstems\n");
		vsteminx--;
	}
	return vsteminx-1;
} 

int getencoding (FILE *input) {
	int charn, count=0;				/* ??? */
	char charname[CHARNAME_MAX];
	
/*	scan for encoding vector */
	for (;;) {
		if (fgets(line, MAXLINE, input) == NULL) return count;
		if (strncmp(line, "/FontBBox", 9) == 0) {
			if(sscanf(line, "/FontBBox {%d %d %d %d} readonly",
				&xll, &yll, &xur, &yur) < 4) {
				if(sscanf(line, "/FontBBox [%d %d %d %d] readonly",
					&xll, &yll, &xur, &yur) < 4) {
					fprintf(stderr, "Don't understand: %s", line);
				}
			}
			if (verboseflag != 0) printf("%s", line);
/* /FontBBox {-280 -337 1313 948} readonly def */
		}
		if (strncmp(line, "/Encoding", 9) == 0) {
			if (strstr(line, "StandardEncoding") != NULL) {
				makestandard(); 
			}
			else {
				makeempty();
				for(;;) {
					if (fgets(line, MAXLINE, input) == NULL) return count;
					if (strstr(line, "dup") != NULL) break;
				}
				for(;;) {				
					if (strstr(line, "readonly") != NULL) break;
					if (sscanf(line, "dup %d /%s put", &charn, &charname) 
						< 2) break;
					strcpy(encoding[charn], charname);
					numencod++; count++;
					if (fgets(line, MAXLINE, input) == NULL) return count;
					if (strstr(line, "dup") == NULL) break;
				}
			}
			return count;
		}
	}
}

void scantoeexec(FILE *input) {
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) return;
		if (strstr(line, "eexec") != NULL) return;
		if (strncmp(line, "/FontBBox", 9) == 0) {
			if(sscanf(line, "/FontBBox {%d %d %d %d} readonly", 
				&xll, &yll, &xur, &yur) < 4) {
				if(sscanf(line, "/FontBBox [%d %d %d %d] readonly", 
					&xll, &yll, &xur, &yur) < 4) {
					fprintf(stderr, "Don't understand: %s", line);
				}
			}
			if (verboseflag != 0) printf("%s", line);
/* /FontBBox {-280 -337 1313 948} readonly def */
		}
	}

}

int getsubrs(FILE *input) {
	char *s;
	int n, m, count=0;
	
	for(n=0; n < MAXSUBRS; n++) subrspointer[n]=-1;

	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) return count;
		if ((s = strstr(line, "/Subrs")) != NULL) {
			if (sscanf(s, "/Subrs %d array", &numsubrs) < 1) {
				fprintf(stderr, "Don't understand: %s", line);
				exit(1);
			}
			else if (traceflag != 0) printf("%s", line);
			if (numsubrs > MAXSUBRS) {
				fprintf(stderr, "Too many Subrs\n");
				exit(4);				
			}
			for(;;) {
				if (fgets(line, MAXLINE, input) == NULL) return count;
				if (strstr(line, "end ") != NULL) break; 
				if (strstr(line, "ND") != NULL) break;
				if (strstr(line, "|-") != NULL) break;				
				if (strncmp(line, "dup ", 4) == 0) {  /* RD or -| */
					if (sscanf(line, "dup %d %d RD", &n, &m) < 2) {
						fprintf(stderr, "Don't understand: %s", line);
						exit(2);
					}
					if (n > numsubrs) exit(7);
					subrspointer[n] = ftell(input); /* AFTER dup n m RD */
					count++;
					for(;;) { /* scan to end of Subr */
						if (fgets(line, MAXLINE, input) == NULL) return count;
						if (strstr(line, "noaccess") != NULL) break;
						if (strstr(line, "NP") != NULL) break;
						if (strstr(line, "|") != NULL) break;
					}
				}
			}
			return count;
		}
	}
}

int getchars(FILE *input) {
	char charname[CHARNAME_MAX];
	char *s;
	int n, m, count=0;
	
	for(n=0; n < MAXCHRS; n++) charspointer[n]=-1;

	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) return count;
		if ((s = strstr(line, "/CharStrings")) != NULL) {
			if (sscanf(s, "/CharStrings %d dict", &numchars) < 1) {
				fprintf(stderr, "Don't understand: %s", line);
			}
			else if (traceflag != 0) printf("%s", line);
			for(;;) {
				if (fgets(line, MAXLINE, input) == NULL) return count;
				if (strncmp(line, "readonly", 8) == 0) return count;
/* RD or -| */
				if (sscanf(line, "/%s %d RD", &charname, &m) == 2) {
/*					printf("NAME: %s ", charname); */
					nchar = lookup(charname);
/*					printf("%d ", nchar); */
					if (nchar >= 0) {
						charspointer[nchar] = ftell(input);
						count++;	/* AFTER /name n RD line */
					}
					else {
						if (traceflag != 0) printf("%s ", charname);
						else putc('.', stdout);
					}
					for(;;) {		/* skip over rest of CharString */
						if (fgets(line, MAXLINE, input) == NULL) return count;
						if (strstr(line, "endchar") == NULL) break;
					}
				}
			}
/*			return count; */
		}
	}
}

void closepath(FILE *output) {
	fprintf(output, "h\n");	
}

void moveto(FILE *output, int x, int y) {
	fprintf(output, "%d %d m\n", x, y);
}

void lineto(FILE *output, int x, int y) {
	fprintf(output, "%d %d l\n", x, y);
}

void curveto(FILE *output, int xa, int ya, int xb, int yb, int xc, int yc) {
	fprintf(output, "%d %d %d %d %d %d c\n", xa, ya, xb, yb, xc, yc);
}

/* presently assumes hstems and vstems appear in groups */
/* doesn't allow for hstem3 and vstem3 yet */
/* doesn't handle multiple callsubrs to change stem hints yet */
/* assumes subr contains only hinting information */
/* doesn't allow for recursive calls */

/*
void callsubr(int na, FILE *input, FILE *output) {
	int x, y, dx, dy;

	fseek(input, subrspointer[na], SEEK_SET); 
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) return;
		if (strncmp(line, "return", 5) == 0) return;
		else if (strstr(line, "vstem") != NULL) {
			if (sscanf(line, "%d %d vstem", &x, &dx) < 2) 
				fprintf(stderr, "Don't understand: %s", line);
			insertvstem(x, dx);
		}
		else if (strstr(line, "hstem") != NULL) {
			if (sscanf(line, "%d %d hstem", &y, &dy) < 2) 
				fprintf(stderr, "Don't understand: %s", line);
			inserthstem(y, dy);
		}
		else fprintf(stderr, "Don't understand: %s", line);
	}
} */

void writehints(int nchar, FILE *stemout) {
	int k;
	if (hsteminx == 0 && vsteminx == 0) return;
	fprintf(stemout, "C %d ; ", nchar);
	if (hsteminx > 0) {
		fprintf(stemout, "H ");
		for (k = 0; k < hsteminx; k++) {
			fprintf(stemout, "%d %d ", hstems[k], hsteme[k]);
		}
		fprintf(stemout, "; ");
	}
	if (vsteminx > 0) {
		fprintf(stemout, "V ");
		for (k = 0; k < vsteminx; k++) {
			fprintf(stemout, "%d %d ", vstems[k], vsteme[k]);
		}
		fprintf(stemout, "; ");
	}
	putc('\n', stemout);
}

void analyzepath(FILE *output, FILE *input) {
	long filepos;
	int na, nb, nc;
	int xa, ya, xb, yb, xc, yc;
	int dx, dy, dxa, dya, dxb, dyb, dxc, dyc;
	int sx, sy;

	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) return;
		if (strstr(line, "endchar") != NULL) {
			if (subrlevel > 0) subrlevel--;		/* in Subr */
			if (fgets(line, MAXLINE, input) == NULL) break;
			if (strstr(line, "ND") == NULL) return;
			if (strstr(line, "|-") == NULL) return;
			if (strstr(line, "return") == NULL) return; /* in Subr */
			break;	/* read "ND" line */
		}
		else if (strstr(line, "ND") != NULL ||
			strstr(line, "|-") != NULL) { /* endchar was in Subr ? */
			return;
		}
		else if (strstr(line, "return") != NULL) { /* end of subr */
			if (subrlevel == 0) {
				fprintf(stderr, "Met `return' while not in subr\n");
			}
			else subrlevel--;
			if (fgets(line, MAXLINE, input) == NULL) break;
			if (strstr(line, "noaccess") == NULL) return;
			break;	/* read "noacces put" line */
		}
		else if (strstr(line, "closepath") != NULL)
			closepath(output);
		else if (strstr(line, "rmoveto") != NULL)  {
			if (sscanf(line, "%d %d rmoveto", &dx, &dy) < 2) 
				fprintf(stderr, "Don't understand: %s", line);
			if (flexproc > 0) {			/* just save them up */
				xflex[flexproc] = xflex[flexproc-1] + dx; 
				yflex[flexproc] = yflex[flexproc-1] + dy;
			}
			else {
				x = x + dx; y = y + dy;
				moveto(output, x, y);
			}
		}
		else if (strstr(line, "vmoveto") != NULL)  {
			if (sscanf(line, "%d vmoveto", &dy) < 1) 
				fprintf(stderr, "Don't understand: %s", line);
			if (flexproc > 0) {			/* just save them up */
				xflex[flexproc] = xflex[flexproc-1];
				yflex[flexproc] = yflex[flexproc-1] + dy;
			}
			else {
				y = y + dy;
				moveto(output, x, y);
			}
		}
		else if (strstr(line, "hmoveto") != NULL)  {
			if (sscanf(line, "%d hmoveto", &dx) < 1) 
				fprintf(stderr, "Don't understand: %s", line);
			if (flexproc > 0) {			/* just save them up */
				xflex[flexproc] = xflex[flexproc-1] + dx;
				yflex[flexproc] = yflex[flexproc-1];
			}
			else {
				x = x + dx;
				moveto(output, x, y);
			}
		}
		else if (strstr(line, "rlineto") != NULL)  {
			if (sscanf(line, "%d %d rlineto", &dx, &dy) < 2) 
				fprintf(stderr, "Don't understand: %s", line);
			x = x + dx, y = y + dy;
			lineto(output, x, y);
		}						
		else if (strstr(line, "vlineto") != NULL)  {
			if (sscanf(line, "%d vlineto", &dy) < 1) 
				fprintf(stderr, "Don't understand: %s", line);
			y = y + dy;
			lineto(output, x, y);
		}
		else if (strstr(line, "hlineto") != NULL)  {
			if (sscanf(line, "%d hlineto", &dx) < 1) 
				fprintf(stderr, "Don't understand: %s", line);
			x = x + dx;
			lineto(output, x, y);
		}
		else if (strstr(line, "rrcurveto") != NULL)  {
			if (sscanf(line, "%d %d %d %d %d %d rrcurveto",
				&dxa, &dya, &dxb, &dyb, &dxc, &dyc) < 6) 
				fprintf(stderr, "Don't understand: %s", line);
			x = x + dxa; y = y + dya; xa = x; ya = y; 
			x = x + dxb; y = y + dyb; xb = x; yb = y; 
			x = x + dxc; y = y + dyc; xc = x; yc = y; 
			curveto(output, xa, ya, xb, yb, xc, yc);
		}						
		else if (strstr(line, "hvcurveto") != NULL)  {
			if (sscanf(line, "%d %d %d %d hvcurveto",
				&dxa, &dxb, &dyb, &dyc) < 4) 
				fprintf(stderr, "Don't understand: %s", line);
			x = x + dxa; xa = x; ya = y; 
			x = x + dxb; y = y + dyb; xb = x; yb = y; 
			y = y + dyc; xc = x; yc = y; 
			curveto(output, xa, ya, xb, yb, xc, yc);
		}						
		else if (strstr(line, "vhcurveto") != NULL)  {
			if (sscanf(line, "%d %d %d %d vhcurveto",
				&dya, &dxb, &dyb, &dxc) < 4) 
				fprintf(stderr, "Don't understand: %s", line);
			y = y + dya; xa = x; ya = y; 
			x = x + dxb; y = y + dyb; xb = x; yb = y; 
			x = x + dxc; xc = x; yc = y; 
			curveto(output, xa, ya, xb, yb, xc, yc);
		}						
		else if (strstr(line, "callsubr") != NULL &&
			sscanf(line, "%d %d %d %d callsubr", 
				&xa, &ya, &xb, &yb) == 4) {
			if (flexproc != 8 || yb != 0) {
				fprintf(stderr, 
					"Invalid termination (%d) of FlexProc char %d\n", 
						flexproc, nchar);
				fprintf(stderr, "%s", line);
			}
			else {		/* end of Flex Proc */
				curveto(output, xflex[2], yflex[2], xflex[3],
					yflex[3], xflex[4], yflex[4]); 
				curveto(output, xflex[5], yflex[5], xflex[6],
					yflex[6], xflex[7], yflex[7]); 
				x = xflex[7]; y = yflex[7];
/*				end of FlexProc */
				flexproc = 0;			/* reset pointer */
			}
		}
		else if (strstr(line, "callsubr") != NULL) {
			if (sscanf(line, "%d callsubr", &na) < 1) 
				fprintf(stderr, "Don't understand: %s", line);
			if (allowflex != 0 && na == 1) {
				xflex[0] = x; yflex[0] = y;
				if (flexproc != 0) {
					fprintf(stderr, 
						"Restart of FlexProc in char %d\n", nchar);
					flexproc = 0;
				}
				flexproc++;		/*	start of FlexProc */ 
			}
			else if (allowflex != 0 && na == 2) {
				if (flexproc++ >= 8) {	/*	repeating part of FlexProc */
					fprintf(stderr, 
						"Too many steps in FlexProc in char %d\n", nchar);
					flexproc = 0;
				}
			}
			else {
				subrlevel++;
				filepos = ftell(input);
				if (na > MAXSUBRS || subrspointer[na] < 0) {
					fprintf(stderr, "Unknown Subr %d\n", na);
					exit(9);				
				}
				fseek(input, subrspointer[na], SEEK_SET); /* go back to subr */
				analyzepath(output, input); /* recurse ! */
				fseek(input, filepos, SEEK_SET);
			}
		}
		else if (strstr(line, "callothersubr") != NULL) {
/*	check that this is the epxected stylized use of callothersubr: */
			if(sscanf(line, "%d %d %d callothersubr", &na, &nb, &nc) < 3) 
				fprintf(stderr, "Don't understand: %s", line);
			if (nb != 1 || nc != 3) 
				fprintf(stderr, "Don't understand: %s", line);
			/* read `pop' line */
			if (fgets(line, MAXLINE, input) == NULL) return;
			if (strncmp(line, "pop", 3) != 0) 
				fprintf(stderr, "Don't understand: %s", line);
			/* read `callsubr' line */
			if (fgets(line, MAXLINE, input) == NULL) return;
			if (strncmp(line, "callsubr", 8) != 0) 
				fprintf(stderr, "Don't understand: %s", line);
			subrlevel++;
			filepos = ftell(input);
			if (na > MAXSUBRS || subrspointer[na] < 0) {
				fprintf(stderr, "Unknown Subr %d\n", na);
				exit(9);				
			}
			fseek(input, subrspointer[na], SEEK_SET); /* go back to subr */
			analyzepath(output, input); /* recurse ! */
			fseek(input, filepos, SEEK_SET);
		}
		else if (strstr(line, "vstem3") != NULL) {
			if (sscanf(line, "%d %d %d %d %d %d vstem3", 
				&xa, &dxa, &xb, &dxb, &xc, &dxc) < 6) 
				fprintf(stderr, "Don't understand: %s", line); 
			insertvstem(xa, dxa);
			insertvstem(xb, dxb);
			insertvstem(xc, dxc);			
		}
		else if (strstr(line, "vstem") != NULL) {
			if (sscanf(line, "%d %d vstem", &sx, &dx) < 2) 
				fprintf(stderr, "Don't understand: %s", line);
			if (dx < 0) {sx = sx + dx; dx = - dx; } /* crock of s... */
			insertvstem(sx, dx);
		}
		else if (strstr(line, "hstem3") != NULL) {
			if (sscanf(line, "%d %d %d %d %d %d hstem3", 
				&ya, &dya, &yb, &dyb, &yc, &dyc) < 6) 
				fprintf(stderr, "Don't understand: %s", line); 
			inserthstem(ya, dya);
			inserthstem(yb, dyb);
			inserthstem(yc, dyc);			
		}
		else if (strstr(line, "hstem") != NULL) {
			if (sscanf(line, "%d %d hstem", &sy, &dy) < 2) 
				fprintf(stderr, "Don't understand: %s", line);
			if (dy < 0) {sy = sy + dy; dy = - dy; } /* crock of s... */
			inserthstem(sy, dy);
		}
		else if (strstr(line, "dotsection") != NULL) {	} /* ignore */
/*		else if (strstr(line, "pop") != NULL) {	} */
		/* callothersubr, callsubr, pop */
		/* seac, sbw, hstem, hstem3, vstem, vstem3, div */
		/* return, setcurrentpoint, dotsection,  */
		else fprintf(stderr, "Don't understand: %s", line);
	}
}

void docharstrings(FILE *input, FILE *output, FILE *stemout) {
	int k, denx;
/*	char charname[CHARNAME_MAX];  */

	numprocs=0;
	for(k=0; k < MAXCHRS; k++) {
		nchar = k;
		if (charspointer[k] < 0) continue;
		if (traceflag != 0) printf("%d: %s ", k, encoding[k]);
		else putc('.', stdout);
		fseek(input, charspointer[k], SEEK_SET);
		if (fgets(line, MAXLINE, input) == NULL) continue;
		if (strncmp(line, "readonly", 8) == 0) return; 
		numprocs++;
		hsteminx=0; vsteminx=0;
		fprintf(output, "]\n");
		if (strstr(line, "hsbw") != NULL) {
/* should be able to deal with div here */
			if (strstr(line, "div") != NULL) { /* crude hack */
				if (sscanf(line, "%d %d %d div hsbw", 
					&sbx, &widthx, &denx) < 3) 
					fprintf(stderr, "Don't understand: %s", line);
				sby = 0; widthx = widthx / denx; widthy = 0;
			}
			else if (sscanf(line, "%d %d hsbw", &sbx, &widthx) < 2) 
				fprintf(stderr, "Don't understand: %s", line);
			sby = 0; widthy = 0;
		}
		else if (strstr(line, "sbw") != NULL) {
/* should be able to deal with div here */
			if (sscanf(line, "%d %d %d %d sbw", 
				&sbx, &sby, &widthx, &widthy) < 4) 
					fprintf(stderr, "Don't understand: %s", line);
		}
		else fprintf(stderr, "Don't understand: %s", line);

/* allow for `div' here later maybe */ /* sbx is x offset to use */
/* allow for `sbw' here later maybe */
		x = sbx, y = sby;
		fprintf(output, "%d %d\n", nchar, widthx);
		subrlevel=0; flexproc=0;
		analyzepath(output, input);
		if (flexproc != 0) 
			fprintf(stderr, "Bad FlexProc sequence in char %d\n", nchar);
		if (subrlevel != 0) 
			fprintf(stderr, "Non-zero (%d) subr level\n",
				subrlevel);
			if (hintflag != 0) writehints(nchar, stemout);
	}
}

#ifdef IGNORE
void extension(char *fname, char *ext) { /* supply extension if none */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
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

void writeheader(FILE *output, FILE *stemout, char *fontname) {
	/* write dummy header */
	fprintf(output, "10.0000 1.00\n");
	fprintf(output, "%d %d %d %d\n", xll, yll, xur, yur);
	fprintf(stemout, "%% %s\n", fontname);
}

void showusage(char *name) {
	printf("Useage: %s {-f} {-t}\n", name);
	printf("\tf	disable FlexProc use\n");
	printf("\tt	turn on tracing output\n");
	exit(1);
}

int main(int argc, char *argv[]) {       /* main program */
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_stm[FILENAME_MAX];
	FILE *fp_in, *fp_out, *fp_stm;
	int nsubrs, nchars, firstarg=1;

	if (firstarg == argc) return -1;

	printf("Outline and hint extraction program version 1.0\n");

/*	printf("First argv %s length %d\n", 
		argv[firstarg], strlen(argv[firstarg])); */
	if (strcmp(argv[firstarg], "-f") == 0) {
		printf("Disallowing FlexProc use\n"); 
		allowflex = 0;					/* disallow FlexProc use */
		firstarg++;
	}
	if (strcmp(argv[firstarg], "-t") == 0) {
		printf("Turning trace on\n"); 
		traceflag = 1;					/* Turning trace on */
		firstarg++;
	}
	if (firstarg >= argc) showusage(argv[0]);

	strcpy(fn_in, argv[firstarg]);
	extension(fn_in, "pln");
	if ((fp_in = fopen(fn_in, "rb")) == NULL) { /* rb to avoid C-M C-J pro */
		perror(fn_in);	exit(3);
	}
	if (verboseflag != 0) printf("Processing %s\n", fn_in);
	strcpy(fn_out, argv[firstarg]);
	forceexten(fn_out, "out");
	if ((fp_out = fopen(fn_out, "w")) == NULL) {
		perror(fn_out);	exit(3);
	}
	if (hintflag != 0) {
		strcpy(fn_stm, argv[firstarg]);
		forceexten(fn_stm, "stm");
		if ((fp_stm = fopen(fn_stm, "w")) == NULL) {
			perror(fn_stm);	exit(3);
		}
	}
	if (verboseflag != 0) printf("Getting character encoding ");
	getencoding(fp_in);
	if (verboseflag != 0) printf(" - %d Names\n", numencod);
	scantoeexec(fp_in);
	if (verboseflag != 0) printf("Getting Subrs positions ");
	nsubrs = getsubrs(fp_in);
	if (verboseflag != 0) printf(" - %d Subrs (%d recorded)\n",
		numsubrs, nsubrs);
	if (verboseflag != 0) printf("Getting CharString positions ");
/*	if (traceflag != 0)  */
		putc('\n', stdout);
	nchars = getchars(fp_in);
/*	if (traceflag != 0)  */
		putc('\n', stdout);
	if (verboseflag != 0) printf(" - %d CharStrings (%d recorded)\n",
		numchars, nchars);	
	writeheader(fp_out, fp_stm, fn_in);
	if (verboseflag != 0) printf("Processing CharStrings ");
/*	if (traceflag != 0) */
		putc('\n', stdout);
	docharstrings(fp_in, fp_out, fp_stm);
/*	if (traceflag != 0)  */
		putc('\n', stdout);
/*	if (verboseflag != 0) printf(" - %d CharStrings\n", numchars);	 */
	if (verboseflag != 0) printf("Finished - %d CharStrings processed\n", 
		numprocs);
	fclose(fp_in);
	if (ferror(fp_out) != 0) {
		perror(fn_out); exit(5);
	}
	else fclose(fp_out);
	if (hintflag != 0) {
		if (ferror(fp_stm) != 0) {
			perror(fn_stm); exit(5);
		}
		else fclose(fp_stm);
	}
	return 0;
}

/* character outlines and character hints are not sorted ... OK */

/* should be able to deal with div operator ... */

/* should copy across font-level hints to output stem file ... */

/* doesn't deal with `div' in say `hstem' or `hstem3' lines */

/* processing of flex proc may still be buggy */

/* suppres use of subrs for FlexProc using -f for LucidaBright */
