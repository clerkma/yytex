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

/* EXTROUTL - make outlines from decrypted output */
/* run PFBTOPFA to turn PFB into PFA format */
/* run DECRYPT twice to get required input for this (or use MAKEPLN.BAT) */
/* then run EXTROUTL */
/* run CONVERT to turn into HEX format */
/* then use SHOWCHAR to look at or FONTONE to make new font */
/* (see MAKEOUTL.BAT for details) */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/* #define MAXLINE 512 */
#define MAXLINE 2048
/* #define FNAMELEN 80 */
#define CHARNAME_MAX 64
#define MAXCHRS 256
#define MAXSUBRS 1024
#define MAXSTEMS 128
#define MAXENCODE 32

#define FLOATCOORDS

int allowflex = 1;			/* if Subrs used for FlexProc */
int assumeswitch = 1;		/* assume <n> <m> callsubr is hint switch */
int snaptoabs = 1;			/* perform snap to in final FlxProc call */

int verboseflag = 1;
int traceflag = 0;
int debugflag = 0;
int hintflag = 1;
int markswitch = 1;			/* insert hint switching markers */
int avoidzero = 1;			/* avoid zero width stem 96/Apr/22 */ 
int avoidnegative = 0;		/* avoid negative hint widths 96/Apr/22 */

int flxeps = 2;				/* max slop in quantized movement FlxProc */

int totalround=0;			/* how many have been rounded */

double scalef=1.0;			/* scale relative to 1000 per em */
							/* should get from FontMatrix */

#ifdef FLOATCOORDS
double xoffset=0.0;			/* from FontMatrix */
double yoffset=0.0;			/* from FontMatrix */
#else
int xoffset=0;				/* from FontMatrix */
int yoffset=0;				/* from FontMatrix */
#endif

FILE *errout=stdout;

double m11, m12, m21, m22, m13, m23;

int numsubrs=0;		/* number of Subrs */
int numchars=0;		/* number of CharStrings */
int numencod=0;		/* entries in encoding vector */
int numprocs=0;		/* number of outlines actually processed */

int subrlevel=0;	/* levels down in subroutines calling each other */

int nsubr=0;		/* subr number for this character */

int currentset=0;	/* non-zero after moveto in subpath */

int savedswitch=0;	/* non-zero if hint switch has been delayed */

int flexproc=0;		/* inside a FlexProc sequence - count of hits */

#ifdef FLOATCOORDS
double xflex[8];		/* x coordinates from FlexProc */
double yflex[8];		/* y coordinates from FlexProc */
double sbx, sby;
double widthx, widthy;
double x, y;
#else
int xflex[8];		/* x coordinates from FlexProc */
int yflex[8];		/* y coordinates from FlexProc */
int sbx, sby;
int widthx, widthy;
int x, y;
#endif

char *szFontName=NULL;

int onthestack=-1;	/* saved subr number in split callothersubr pop callsubr */

int xll=0, yll=0, xur=1000, yur=1000;

int chrs;				/* character currently working on */ 

static hstems[MAXSTEMS], hsteme[MAXSTEMS], vstems[MAXSTEMS], vsteme[MAXSTEMS];

int hsteminx=0, vsteminx=0;

char line[MAXLINE];			/* input line */
char buffer[MAXLINE];		/* used by check for div */

static long subrspointer[MAXSUBRS];

/* static long charspointer[MAXSUBRS]; */
static long charspointer[MAXCHRS];				/* 1995/Oct/24 */

/* static unsigned char encoding[MAXCHRS][MAXENCODE];  */
/* char encoding[MAXCHRS][MAXENCODE];  */
char *encoding[MAXCHRS];				/* 1994/June/29 */

/* static unsigned char *standardencoding[] = {  */
char *standardencoding[] = { 
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

void showencoding(void) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(encoding[k], "") == 0) continue;
		else printf("%3d\t%s\n", k, encoding[k]);
	}
}

int lookup(char *charname) { /* returns the first one */
	int k;
	if (strlen (charname) == 1) {		/* speedup 94/June/29 */
		k = *charname;
		if (strcmp(encoding[k], charname) == 0) return k;
	}
	for (k=0; k < MAXCHRS; k++)
		if (strcmp(encoding[k], charname) == 0) return k;
	return -1;
}

void makestandard (void) {
	int i;
/*	for (i=0; i < MAXCHRS; i++) strcpy(encoding[i], standardencoding[i]); */
	for (i=0; i < MAXCHRS; i++) encoding[i] = standardencoding[i];
	numencod=192;
}

void makeempty (void) {
	int i;
/*	for (i=0; i < MAXCHRS; i++) strcpy(encoding[i], ""); */
	for (i=0; i < MAXCHRS; i++) encoding[i] = "";
	numencod=0;
}

/* Deal with encoding vector in Windows 3.1 TT => T1 output */

void makehexacode(int chars, int chare) {		/* 1995/Jan/30 */
	int i;
	char buffer[8];
	for (i = chars; i <= chare; i++) {
		sprintf(buffer, "G%02X", i);
		encoding[i] = _strdup(buffer);
/*		printf("%02X %s\n", i, encoding[i]); */
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int kround (double x) {				/* 1993/Dec/19 */
	if (x == 0.0) return 0;
	if (x < 0) return - (int) (-x + 0.49999);
	else return  (int) (x + 0.49999);
}

/*
void inserthstem (int y, int dy) {
	hstems[hsteminx] = y + sby; 	hsteme[hsteminx] = y + dy + sby;
	hsteminx++;
	if (hsteminx >= MAXSTEMS) {
		fprintf(errout, "Too many hstems\n");
		hsteminx--;
	}
} */

/*
void insertvstem (int x, int dx) {
	vstems[vsteminx] = x + sbx; 	vsteme[vsteminx] = x + dx + sbx;
	vsteminx++;
	if (vsteminx >= MAXSTEMS) {
		fprintf(errout, "Too many vstems\n");
		vsteminx--;
	}
} */

/* sort them as they are inserted ? */
/* checkfor overlap as they are inserted ? */

#ifdef FLOATCOORDS
int inserthstem (double fy, double fdy) {
	int ys = kround(fy + sby);
	int ye = kround(fy + fdy + sby);
#else
int inserthstem (int y, int dy) {
	int ys = y + sby;
	int ye = y + dy + sby;
#endif
	int k;
	
	if (avoidzero && ys == ye) {
		if (verboseflag) putc('0', stdout);
		return hsteminx-1;
	}
	if (avoidnegative && ys > ye) {				/* 96/Apr/22 */
		int temp;
		if (verboseflag) putc('-', stdout);
		temp = ys; ys = ye; ye = temp;
	}
	for (k = 0; k < hsteminx; k++) {
		if (hstems[k] == ys && hsteme[k] == ye) return k;
	}
	hstems[hsteminx] = ys;	hsteme[hsteminx] = ye;
	hsteminx++;
	if (hsteminx >= MAXSTEMS) {
		fprintf(errout, "Too many hstems\n");
		hsteminx--;
	}
	return hsteminx-1;
} 

/* sort them as they are inserted ? */
/* checkfor overlap as they are inserted ? */

#ifdef FLOATCOORDS
int insertvstem (double fx, double fdx) { 
	int xs = kround (fx + sbx);
	int xe = kround (fx + fdx + sbx);
#else
int insertvstem (int x, int dx) {
	int xs = x + sbx;
	int xe = x + dx + sbx;
#endif
	int k;
	
	if (avoidzero && xs == xe) {
		if (verboseflag) putc('0', stdout);
		return hsteminx-1;
	}
	if (avoidnegative && xs > xe) {				/* 96/Apr/22 */
		int temp;
		temp = xs; xs = xe; xe = temp;
	}
	for (k = 0; k < vsteminx; k++) {
		if (vstems[k] == xs && vsteme[k] == xe) return k;
	}
	vstems[vsteminx] = xs;	vsteme[vsteminx] = xe;
	vsteminx++;
	if (vsteminx >= MAXSTEMS) {
		fprintf(errout, "Too many vstems\n");
		vsteminx--;
	}
	return vsteminx-1;
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int forceinteger=1; */

#ifdef IGNORE
void checkfordiv (char *line) {				/* 1993/Dec/19 */
	char *s, *t;
/*	long num, den, ratio; */
	double num, den, ratio;
	int k=0;
	char buffer[MAXLINE];				/* ... */

/*	fputs(line, stdout); */				/* debugging */
	while ((s = strstr(line, "div")) != NULL) {
		t = s + 3;							/* remember where */
		strcpy(buffer, t);					/* save the tail of this */
/*		printf("s = line + %d ", s - line); */	/* debugging */
#ifdef IGNORE
		while (*s-- > ' ' && s > line) ;	/* go back to space before div */
		while (*s-- <= ' ' && s > line) ;	/* go back over space to denom */
		while (*s-- > ' ' && s > line) ;	/* go back to space before denom */
		while (*s-- <= ' ' && s > line) ;	/* go back over space to numer */
		while (*s-- > ' ' && s > line) ;	/* go back to space before numer */
#endif
		while (*s > ' ' && s > line) s--;   /* back to space before `div' */
		while (*s <= ' ' && s > line) s--;  /* back over space(s) to denom */
		while (*s > ' ' && s > line) s--;   /* back to space before denom */
		while (*s <= ' ' && s > line) s--;  /* back over space(s) to num */
		while (*s > ' ' && s > line) s--;   /* back to space before num */

		if (s < line) {
/*			fprintf(errout, "Stepped past beginning\n"); */
			s = line;
		}
#ifdef IGNORE
		if (s > line) s++;					/* step forward to space */
		if (s > line) s++;					/* step forward past space */
#endif
		while (*s > ' ') s++;			    /* forward to num */

/*		printf("s = line + %d ", s - line); */	/* debugging */
/*		if (sscanf (s, "%ld %ld div", &num, &den) == 2) { */
		if (sscanf (s, "%lg %lg div", &num, &den) == 2) {
/*			if (num > 0) ratio = (num + den/2 - 1) / den;
			else ratio = - (-num + den/2 - 1) / den; */
			ratio = num / den; 		/* should be 16 bit at this point */
/*			sprintf(s, "%ld", ratio); */
			if (forceinteger) sprintf(s, "%d", kround(ratio));
			else sprintf(s, "%lg", ratio);
		}
		else {
/*			fprintf(errout, "Can't make sense of: %s", line);*/ /* 93/Dec/20 */
			fprintf(errout, "Can't make sense of: %s", s); /* 95/Mar/28 */
			break;
		}
/*		(void) memmove (s + strlen(s), t, strlen(t)+1); */
		strcat (s, buffer);					/* put back the tail end */
		if (traceflag != 0) {
			printf("num %lg den %lg ratio %lg: ", num, den, ratio);
			fputs(line, stdout);				
		}
/*		if (bAbort != 0) abortjob(); */
		if (k++ > 16) break; 				/* emergency exit */
	}
	totalround += k;
}
#endif

void showerror (char *task) {
	fprintf(errout, "Don't understand (%d) ", chrs);
	if (chrs >= 0 && chrs < MAXCHRS && strcmp(encoding[chrs], "") != 0)
		fprintf(errout, "(%s) ", encoding[chrs]);
	fprintf(errout, "(%s)  %s", task, line);
}

/* following revised 1994/June/21 - copy new code to SAFESEAC and PFATOAFM */
/* NOTE: converts ratios to doubles */

void checkfordiv (char *line) {
	char *s, *t;
/*	long num, den, ratio; */
	double num, den, ratio;
	int k=0;
/*	char buffer[MAXLINE]; */

	if (strstr(line, "div") == NULL) return;
	if (debugflag) printf("removing div: %s", line);
/*	fputs(line, stdout); */				/* debugging */
	while ((s = strstr(line, "div")) != NULL) {
/*		if (traceflag) printf("removin div: %s", line); */
		t = s + 3;							/* remember where */
		strcpy(buffer, t);					/* save the tail of this */
/*		printf("s = line + %d ", s - line); */	/* debugging */
/*		while (*s-- > ' ' && s > line) ; */	/* go back to space */
		while (*s > ' ' && s > line) s--;	/* go back to space */
/*		while (*s-- <= ' ' && s > line) ; */	/* go back over space */
		while (*s <= ' ' && s > line) s--;	/* go back over space */
		if (s == line) 	showerror("checkfordiv");
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
/*		if (*s < '0' || *s >= '9') s++; */	/* step over [, { etc */
		if (*s < '0' || *s > '9') s++;		/* step over [, { etc */
/*		printf("s = line + %d ", s - line); */	/* debugging */
/*		if (sscanf (s, "%ld %ld div", &num, &den) == 2) { */
		if (sscanf (s, "%lg %lg div", &num, &den) == 2) {
/*			if (num > 0) ratio = (num + den/2 - 1) / den;
			else ratio = - (-num + den/2 - 1) / den; */
			ratio = num / den; 		/* should be 16 bit at this point */
#ifdef FLOATCOORDS
			sprintf(s, "%lg", ratio); 
#else
			sprintf(s, "%d", kround(ratio)); /* 1994/June/27 */
#endif
		}
		else {
/*			fprintf(errout, "Can't make sense of: %s", line); */ /* 1993/Dec/20 */
			fprintf(errout, "Can't make sense of: %s", s); /* 1995/Mar/28 */
			break;
		}
/*		(void) memmove (s + strlen(s), t, strlen(t)+1); */
		strcat (s, buffer);					/* put back the tail end */
		if (debugflag) {
			printf("num %lg den %lg ratio %lg: ", num, den, ratio);
			fputs(line, stdout);				
		}
/*		if (traceflag) printf("removed div: %s", line); */
/*		if (bAbort != 0) abortjob(); */
		if (k++ > 16) break;				/* emergency exit */
	}
#ifdef FLOATCOORDS
#else
	totalround += k;
#endif
	if (debugflag) printf("removed div: %s", line);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int getFontName(FILE *input) {
	char *s, *t;
	if (verboseflag != 0) printf("Getting FontName ");
	rewind(input);			/* ??? */
	for (;;) {
		if (fgets(line, MAXLINE, input) == NULL) return 0;
		if ((s = strstr(line, "/FontName")) != NULL) {
			while (*s != '\0' && *s > ' ') s++;
			while (*s <= ' ' && *s != '\0') s++;
			if (*s == '/') s++;
			t = s;
			while (*t > ' ') t++;
			*t = '\0';
			szFontName = strdup(s);
			if (verboseflag) printf("FONTNAME: %s\n", szFontName);
			return 1;
		}
	}
/*	return 0; */
}

int getEncoding (FILE *input) {
	int charn, count=0;				/* ??? */
	char charname[CHARNAME_MAX];
	int n;
	int k, chars, chare;
	char *s, *t;
	
	if (verboseflag != 0) printf("Getting Encoding ");
/*	scan for encoding vector */
	for (;;) {
		if (fgets(line, MAXLINE, input) == NULL) return count;
/*		printf("GETENCODING: %s", line); */
/*		if (strncmp(line, "/FontName", 9) == 0) { */
		if (strstr(line, "/FontName") != NULL) {
			if (traceflag) printf("%s", line);
		}
		else if (strncmp(line, "/FontBBox", 9) == 0) {
/*			assuming fixed point in FontBBox */
			if(sscanf(line, "/FontBBox {%d %d %d %d} readonly",
				&xll, &yll, &xur, &yur) < 4) {
				if(sscanf(line, "/FontBBox [%d %d %d %d] readonly",
					&xll, &yll, &xur, &yur) < 4) {
					showerror("getencoding");
				}
			}
			if (verboseflag) printf("%s", line);
/* /FontBBox {-280 -337 1313 948} readonly def */
		}
		if (strncmp(line, "/FontMatrix", 11) == 0) {
			if (strstr(line, "div") != NULL) checkfordiv (line);
/*			printf("%s", line); */ /* debugging */
			if(sscanf(line, "/FontMatrix [%lg %lg %lg %lg %lg %lg]", 
				&m11, &m21, &m12, &m22, &m13, &m23) < 6) {
				if(sscanf(line, "/FontMatrix {%lg %lg %lg %lg %lg %lg}", 
					&m11, &m21, &m12, &m22, &m13, &m23) < 6) {
					showerror("getencoding");
				}
			}
			if (m11 != 0.001 || m13 != 0.0 || m23 != 0.0) 
				printf("Unusual FontMatrix: %s", line);
			else if (verboseflag != 0) printf("%s", line);
			if (m11 != 0.001) scalef = 0.001 / m11;
#ifdef FLOATCOORDS
			if (m13 != 0.0 || m23 != 0.0) {
				xoffset = (m13 / m11);
				yoffset = (m23 / m11);
				printf("Offset %lg %lg from FontMatrix\n", xoffset, yoffset);
			}
#else
			if (m13 != 0.0 || m23 != 0.0) {
				xoffset = (int) (m13 / m11);
				yoffset = (int) (m23 / m11);
				printf("Offset %d %d from FontMatrix\n", xoffset, yoffset);
			}
#endif
/* /FontMatrix [0.001 0 0 0.001 0 0] readonly def */
		}
		if (strncmp(line, "/Encoding", 9) == 0) {
			if (traceflag) printf(line);
//			if (verboseflag) printf(line);
			if (strstr(line, "StandardEncoding") != NULL) {
				makestandard(); 
			}
			else if ((s = strchr(line, '[')) != NULL) {	/* array format */
				makeempty();
				for (k = 0; k <= MAXCHRS; k++) {
					while (*s != '\0' && *s != ']' && *s != '/') s++;
					if (*s == '\0' || *s == ']') {
						numencod = k;
						printf("Encoding vector has %d elements\n", numencod);
						break;
					}
					if ((t = strchr(s, ' ')) == NULL) {
						if ((t = strchr(s, ']')) == NULL) {
							printf("ERROR: %s", line);
							break;
						}
					}
					*t = '\0';
					encoding[k] = strdup(s+1);
					s = t+1;
				}
			}
			else {
				makeempty();
				for(;;) {				/* scan up to encoding */
					if (fgets(line, MAXLINE, input) == NULL) {
						fprintf(errout, 
							"Unexpected EOF while searching for encoding\n");
						break;
/*						return count; */
					}
					if (traceflag) printf(line);
//					if (verboseflag) printf(line);
					if (strstr(line, "eexec") != NULL) {
						fprintf(errout,	"Unexpected end of encoding\n");
						return count;
					}
					if (strstr(line, "cleartomark") != NULL) {
						fprintf(errout,	"Unexpected end of encoding\n");
						return count;
					}
/*	 allow parsing of FreeHand manufactured bad fonts 97/May/19 */
					if (strncmp(line, "Encoding", 8) == 0) {
						char *s = line;
						while ((s = strstr(s, "Encoding")) != NULL) {
							strcpy(s, "dup");
							strcpy(s+3, s+8);
						}
						if (debugflag) printf(line);
					}
/* special hack for TrueType => Type 1 converted in Windows 3.1 */
					if (strstr(line, " FE") != NULL &&
						sscanf(line, "%d %d FE", &chars, &chare) == 2) {
						if (traceflag) printf("***** %s", line);
						makehexacode(chars, chare);
						return (chare - chars + 1);
					}								/* new 1995/Jan/30 */
/* skip when found first `dup 4 /Aogonek put' line */
					if (strstr(line, "dup") != NULL) break;
/* Following emergency exit added 95/Jan/30 */
					if (strncmp(line, "def", 3) == 0) break;
				}

/*	 Try and step up to first "dup" 1996/April/29 fix for Type Designer */
				s = strstr(line, "dup");
				if (s == NULL) {
					fprintf(errout, "Expecting `dup' not: %s", line);
					s = line;
				}
//				if (verboseflag) printf(line);
				for(;;) {				
					if (strstr(s, "readonly") != NULL) {
						if (verboseflag) printf("Ends on %s", s);
						break;
					}
					if (strstr(s, "put") == NULL &&
						strstr(s, "def") != NULL) {
						if (verboseflag) printf("Encoding vector ends on: %s", s);
						break;
					}
					if ((t = strstr(s, "dup")) != NULL) s = t;
					if (sscanf(s, "dup %d /%s put%n", 
							&charn, &charname, &n) < 2  &&
								sscanf(s, "dup %d/%s put%n", 
									&charn, &charname, &n) < 2) {
						if (strcmp(s, "") != 0)
							fprintf(errout, "Don't understand: %s", s); 
						if (fgets(line, MAXLINE, input) == NULL) {
							fprintf(errout,
									"Unexpected EOF while reading encoding\n");
							if (verboseflag) printf(line);
							break;
/*							return count; */
						}
/*	 allow parsing of FreeHand manufactured bad fonts 97/May/19 */
						if (strncmp(line, "Encoding", 8) == 0) {
							char *s = line;
							while ((s = strstr(s, "Encoding")) != NULL) {
								strcpy(s, "dup");
								strcpy(s+3, s+8);
							}
							if (debugflag) printf(line);
						}
						s = line;
						continue;			/* break; */
					}
					if (charn >= 0 && charn < MAXCHRS) {
/*						strcpy(encoding[charn], charname); */
						encoding[charn] = _strdup(charname); /* 1994/June/29 */
					}
					numencod++;
					count++;
					s += n;
					while (*s <= ' '&& *s != '\0') s++;
/*					if (fgets(line, MAXLINE, input) == NULL) return count; */
				} /* end of for(;;) */
			} /* end of else for normal format */

			if (traceflag) showencoding();
//			if (verboseflag) showencoding();
			return count;
		}
	}
}

void scantoeexec (FILE *input) {
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) return;
		if (strstr(line, "eexec") != NULL) return;
		if (strstr(line, "/Private") != NULL) return;	/* 99/Feb/26 */
		if (strncmp(line, "/FontBBox", 9) == 0) {
/* assuming fixed point in FontBBox */
			if(sscanf(line, "/FontBBox {%d %d %d %d} readonly", 
				&xll, &yll, &xur, &yur) < 4) {
				if(sscanf(line, "/FontBBox [%d %d %d %d] readonly", 
					&xll, &yll, &xur, &yur) < 4) {
					showerror("scantoeexec");
				}
			}
			if (verboseflag) printf("%s", line);
/* /FontBBox {-280 -337 1313 948} readonly def */
		}
		if (strncmp(line, "/FontMatrix", 11) == 0) {
			if (strstr(line, "div") != NULL) checkfordiv (line);
/*			printf("%s", line); */ /* debugging */
			if(sscanf(line, "/FontMatrix [%lg %lg %lg %lg %lg %lg]", 
				&m11, &m21, &m12, &m22, &m13, &m23) < 6) {
				if(sscanf(line, "/FontMatrix {%lg %lg %lg %lg %lg %lg}", 
					&m11, &m21, &m12, &m22, &m13, &m23) < 6) {
					showerror("scantoeexec");
				}
			}
			if (m11 != 0.001 || m13 != 0.0 || m23 != 0.0)
				printf("Unusual FontMatrix: %s", line);
			else if (verboseflag != 0) printf("%s", line);
			if (m11 != 0.001) scalef = 0.001 / m11;
#ifdef FLOATCOORDS
			if (m13 != 0.0 || m23 != 0.0) {
				xoffset = (m13 / m11);
				yoffset = (m23 / m11);
				printf("Offset %lg %lg from FontMatrix\n", xoffset, yoffset);
			}
#else
			if (m13 != 0.0 || m23 != 0.0) {
				xoffset = (int) (m13 / m11);
				yoffset = (int) (m23 / m11);
				printf("Offset %d %d from FontMatrix\n", xoffset, yoffset);
			}
#endif
/* /FontMatrix [0.001 0 0 0.001 0 0] readonly def */
		}
	}
}

int getsubrs(FILE *input, FILE *hints) {
	char *s;
	long current;
	int n, m, count=0;
	int copyflag=0;
	int wrapflag=0;

	if (verboseflag) printf("Getting Subrs positions ");
	if (traceflag) printf("(from byte %d) ", ftell(input));
	numsubrs = 0;

	for(n=0; n < MAXSUBRS; n++) subrspointer[n]=-1;

	for(;;) {
		current = ftell(input);			/* remember start of line position */
		if (fgets(line, MAXLINE, input) == NULL) {
			printf("Hit EOF while searching for Subrs\n");
			if (wrapflag++ == 0) {
				printf("Starting again at beginning of file\n");
				rewind(input);
			}
			else {
				if (traceflag) printf("Popping out %s", line);
				return count;
			}
		}
		if ((s = strstr(line, "/OtherSubrs")) != NULL) {
			fseek(input, current, SEEK_SET);
			if (traceflag) printf("Popping out %s", line);
			break;
		}
		if (strstr(line, "1183615869") != NULL) continue;	/* 99/Feb/26 */
		if ((s = strstr(line, "/Subrs")) != NULL) {
			fseek(input, current, SEEK_SET);
			if (traceflag) printf("Popping out %s", line);
			break;
		}
		if ((s = strstr(line, "/CharStrings")) != NULL) {
			fseek(input, current, SEEK_SET);
			if (traceflag) printf("Popping out %s", line);
			break;
		}
		if (copyflag) {
			if (strncmp(line, "/-|", 3) == 0) ;
			else if (strncmp(line, "/|-", 3) == 0) ;
			else if (strncmp(line, "/|", 2) == 0) ;
			else if (strncmp(line, "/RD", 2) == 0) ;
			else if (strncmp(line, "/ND", 2) == 0) ;
			else if (strncmp(line, "/NP", 2) == 0) ;
/*			else fputs(line, hints); */
			else if (hints != NULL) {	/* spit out font level hints */
/*				but flush the code for /Erode ... special case 96/Apr/20 */
				if (strncmp(line, "/Erode", 6) == 0) {
					while (strstr(line, "def") == NULL) {
						if (*line == '/' || *line == '[') break;
						current = ftell(input);
						if (fgets(line, MAXLINE, input) == NULL) return count;
					}
				}
/*				also remove /password, /MinFeature and /UniqueID ? */
				else if (strncmp(line, "/MinFeature", 11) == 0 ||
						 strncmp(line, "/password", 9) == 0 ||
						 strncmp(line, "/UniqueID", 9) == 0) ;
				else if (*line == '/') fputs(line, hints);
			}
		}
		if (strstr (line, "/Private") != NULL) copyflag=1;
	}

	if (traceflag) printf("Now scan forward to find Subrs\n");

	for (;;) {	/* scan forward until find Subrs */
		current = ftell(input);			/* remember start of line position */
		if (fgets(line, MAXLINE, input) == NULL) return count;
		if ((s = strstr(line, "/CharStrings")) != NULL) {
			fseek(input, current, SEEK_SET);
			printf(" - NO Subrs found ");
			return count; /* break; */
		}
		if (strstr(line, "1183615869") != NULL) continue;	/* 99/Feb/26 */
		if ((s = strstr(line, "/Subrs")) != NULL) {
			if (sscanf(s, "/Subrs %d array", &numsubrs) < 1) {
				showerror("getsubrs");
				exit(1);
			}
			else if (traceflag != 0) printf("%s", line);
			if (numsubrs > MAXSUBRS) {
				fprintf(stderr, "Too many Subrs\n");
				exit(4);				
			}
			for(;;) {		/* now accumulate information on Subrs */
				current = ftell(input);
				if (fgets(line, MAXLINE, input) == NULL) return count;
				if (strstr(line, "end ") != NULL) break; 
				if (strstr(line, "ND") != NULL) break;
				if (strstr(line, "|-") != NULL) break;
/* Following added 1993/Aug/13 */
/* should be safe, since here use "readonly put" for Subrs */
				if (strstr(line, "readonly def") != NULL) break;
/* Following added 1995/Jan/30 */
/* deal with situation where /CharStrings line directly follows /Subrs line */
				if (strstr(line, "/CharStrings") != NULL) {
					fseek(input, current, SEEK_SET);
					break;
				}
				if (strncmp(line, "dup ", 4) == 0) {  /* RD or -| */
					if (sscanf(line, "dup %d %d RD", &n, &m) < 2) {
						showerror("getsubrs");
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

int getchars (FILE *input) {
	char charname[CHARNAME_MAX];
	char *s;
	int n, m, count=0;
	
	if (verboseflag != 0) printf("Getting CharString positions ");

	for(n=0; n < MAXCHRS; n++) charspointer[n]=-1;

	if (traceflag != 0) printf("Next line %s", line);

	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) {
			printf("EOF - count %d\n", count);
			return count;
		}
		if ((s = strstr(line, "/CharStrings")) != NULL) {
			if (traceflag != 0) fputs(line, stdout);
/*			if (sscanf(s, "/CharStrings %d dict", &numchars) < 1) { */
			if (sscanf(s, "/CharStrings %d dict", &n) < 1) {
/* could be /CharStrings get begin	... */
				if (strstr(line, "get begin") == NULL) {
					showerror("getchars");
				}
			}
			else {
				numchars = n;				/* only copy if found */
				if (traceflag != 0) printf("%s", line);
			}
			for(;;) {
				if (fgets(line, MAXLINE, input) == NULL) {
					printf("EOF - count %d\n", count);
					return count;
				}
				if (strncmp(line, "readonly", 8) == 0) {
					printf("Ends on: %s", line);
					return count;
				}
/* RD or -| */
				if (sscanf(line, "/%s %d RD", &charname, &m) == 2) {
/*					printf("NAME: %s ", charname); */
					chrs = lookup(charname);
					if (traceflag) printf("%s %3d ", charname, chrs);
/*					if (chrs >= 0) { */
					if (chrs >= 0 && chrs < MAXCHRS) {
						charspointer[chrs] = ftell(input);
						count++;	/* AFTER /name n RD line */
					}
					else {
/*						if (traceflag != 0) printf("%s ", charname); */
						if (verboseflag) printf("\nNOT IN ENCODING: %s\n", charname);
/*						else putc('-', stdout); */		/* not in encoding */
					}
					for(;;) {		/* skip over rest of CharString */
						if (fgets(line, MAXLINE, input) == NULL) {
							printf("EOF - count %d\n", count);
							return count;
						}
						if (strstr(line, "endchar") == NULL) {
							break;
						}
					}
				}
			}
/*			return count; */
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void writehints(int, FILE *);

void closepath(FILE *output) {
	fprintf(output, "h\n");	
	if (savedswitch != 0) fprintf(errout, "Lost delayed hint switch\n");
	currentset=0;						/* reset for next subpath */
}

#ifdef FLOATCOORDS
void moveto(FILE *output, double fx, double fy) { 
	int x = kround(fx + xoffset);
	int y = kround(fy + yoffset);
	if (traceflag) printf("%d (%lg) %d (%lg) m\n", x, fx, y, fy);
#else
void moveto(FILE *output, int x, int y) { 
	if (xoffset != 0) x += xoffset;
	if (yoffset != 0) y += yoffset;
#endif
	fprintf(output, "%d %d m\n", x, y);
	if (savedswitch != 0) {				/* if a hint switch was delayed */
		if (traceflag != 0) printf(" delayed hints for %d\n", chrs);
		fprintf(output, "%d s %%\n", ++nsubr);
		savedswitch = 0;				/* reset the delay switch */
	}
	if (currentset != 0) {
		if (traceflag) putc('\n', errout);
		fprintf(errout, "Repeated moveto in `%s' (%d)\n",
			encoding[chrs], chrs);
		if (traceflag) fputs(line, errout);
	}
	currentset++;						/* have seen a moveto in subpath */
}

#ifdef FLOATCOORDS
void lineto(FILE *output, double fx, double fy) {
	int x = kround(fx + xoffset);
	int y = kround(fy + yoffset);
	if (traceflag) printf("%d (%lg) %d (%lg) l\n", x, fx, y, fy);
#else
void lineto(FILE *output, int x, int y) {
	if (xoffset != 0) x += xoffset;
	if (yoffset != 0) y += yoffset;
#endif
	fprintf(output, "%d %d l\n", x, y);
}

#ifdef FLOATCOORDS
void curveto(FILE *output, double fxa, double fya, double fxb, double fyb, double fxc, double fyc) {
	int xa = kround (fxa + xoffset);
	int ya = kround (fya + yoffset);
	int xb = kround (fxb + xoffset);
	int yb = kround (fyb + yoffset);
	int xc = kround (fxc + xoffset);
	int yc = kround (fyc + yoffset);
#else
void curveto(FILE *output, int xa, int ya, int xb, int yb, int xc, int yc) {
	if (xoffset != 0) {
		xa += xoffset;
		xb += xoffset;
		xc += xoffset;
	}
	if (yoffset != 0) {
		ya += yoffset;
		yb += yoffset;
		yc += yoffset;
	}
#endif
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
				fprintf(errout, "Don't understand: %s", line);
			insertvstem(x, dx);
		}
		else if (strstr(line, "hstem") != NULL) {
			if (sscanf(line, "%d %d hstem", &y, &dy) < 2) 
				fprintf(errout, "Don't understand: %s", line);
			inserthstem(y, dy);
		}
		else fprintf(errout, "Don't understand: %s", line);
	}
} */

void writehints(int chrs, FILE *stemout) {
	int k;
	if (hsteminx == 0 && vsteminx == 0) return;
	if (nsubr > 0) fprintf(stemout, "S %d ; ", nsubr);	/* NEW */
	fprintf(stemout, "C %d ; ", chrs);
	if (hsteminx > 0) {
		fprintf(stemout, "H ");
		for (k = 0; k < hsteminx; k++) {
			if (yoffset != 0) 
#ifdef FLOATCOORDS
				fprintf(stemout, "%d %d ",
						hstems[k] + yoffset, hsteme[k] + yoffset);
#else
				fprintf(stemout, "%d %d ",
					(int) (hstems[k] + yoffset), (int) (hsteme[k] + yoffset));
#endif
			else 
				fprintf(stemout, "%d %d ", hstems[k], hsteme[k]);
		}
		fprintf(stemout, "; ");
	}
	if (vsteminx > 0) {
		fprintf(stemout, "V ");
		for (k = 0; k < vsteminx; k++) {
			if (xoffset != 0)
#ifdef FLOATCOORDS
				fprintf(stemout, "%d %d ",
					(int) (vstems[k] + xoffset), (int) (vsteme[k] + xoffset));
#else
				fprintf(stemout, "%d %d ",
					vstems[k] + xoffset, vsteme[k] + xoffset);
#endif
			else 
				fprintf(stemout, "%d %d ", vstems[k], vsteme[k]);
		}
		fprintf(stemout, "; ");
	}
	putc('\n', stemout);
	hsteminx = 0; vsteminx = 0;		/* NEW: reset, ready for next lot */
}

int analyzepath(FILE *output, FILE *input, FILE *stemout) {
	long filepos;
	int na, nb, nc;
	int ns;
#ifdef FLOATCOORDS
	double dx, dy;
	double dxa, dya, dxb, dyb, dxc, dyc;
	double sx, sy;
	double flx, xabs, yabs;
	double xa, ya, xb, yb, xc, yc;
#else
	int dx, dy;
	int dxa, dya, dxb, dyb, dxc, dyc;
	int sx, sy;
	int flx, xabs, yabs;
	int xa, ya, xb, yb, xc, yc;
	long nsx, dsx, nsy, dsy, ndx, ddx, ndy, ddy;
#endif

/*	loop until hit EOF, endchar, ND, |-, return */
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) return 0;
		if (debugflag) fputs(line, stdout);
		if (strstr(line, "endchar") != NULL) {
			if (subrlevel > 0) subrlevel--;		/* in Subr */
			if (fgets(line, MAXLINE, input) == NULL) break;
			if (strstr(line, "ND") == NULL) return 0;
			if (strstr(line, "|-") == NULL) return 0;
			if (strstr(line, "return") == NULL) return 0; /* in Subr */
			break;	/* read "ND" line */
		}
/*		else if (strstr(line, "ND") != NULL || */
		if (strstr(line, "ND") != NULL ||
			strstr(line, "|-") != NULL) { /* endchar was in Subr ? */
			return 0;
		}
/*		else if (strstr(line, "return") != NULL)  *//* end of subr */
		if (strstr(line, "return") != NULL) { /* end of subr */
			if (subrlevel == 0) {
				fprintf(errout, "Met `return' while not in subr\n");
			}
			else subrlevel--;
			if (fgets(line, MAXLINE, input) == NULL) break;
			if (strstr(line, "noaccess") == NULL) return 0;
			break;	/* read "noacces put" line */
		}
/*		else if (strstr(line, "closepath") != NULL) */

		if (strstr(line, "div") != NULL) checkfordiv(line);	/* 93/Dec/19 */

		if (strstr(line, "closepath") != NULL)
			closepath(output);
		else if (strstr(line, "rmoveto") != NULL)  {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg %lg rmoveto", &dx, &dy) < 2) 
#else
			if (sscanf(line, "%d %d rmoveto", &dx, &dy) < 2) 
#endif
			{
				showerror("analyzepath rmoveto");
			}
			if (flexproc > 0) {			/* just save them up */
				xflex[flexproc] = xflex[flexproc-1] + dx; 
				yflex[flexproc] = yflex[flexproc-1] + dy;
			}
			else {
				x = x + dx; y = y + dy;
				moveto(output, x, y);				
/*				moveto(output, x, y, stemout); */
			}
		}
		else if (strstr(line, "vmoveto") != NULL)  {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg vmoveto", &dy) < 1) 
#else
			if (sscanf(line, "%d vmoveto", &dy) < 1) 
#endif
			{
				showerror("analyzepath vmoveto");
			}
			if (flexproc > 0) {			/* just save them up */
				xflex[flexproc] = xflex[flexproc-1];
				yflex[flexproc] = yflex[flexproc-1] + dy;
			}
			else {
				y = y + dy;
				moveto(output, x, y); 
/*				moveto(output, x, y, stemout); */
			}
		}
		else if (strstr(line, "hmoveto") != NULL)  {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg hmoveto", &dx) < 1) 
#else
			if (sscanf(line, "%d hmoveto", &dx) < 1) 
#endif
			{
				showerror("analyzepath hmoveto");
			}
			if (flexproc > 0) {			/* just save them up */
				xflex[flexproc] = xflex[flexproc-1] + dx;
				yflex[flexproc] = yflex[flexproc-1];
			}
			else {
				x = x + dx;
				moveto(output, x, y); 
/*				moveto(output, x, y, stemout); */
			}
		}
		else if (strstr(line, "rlineto") != NULL)  {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg %lg rlineto", &dx, &dy) < 2) 
#else
			if (sscanf(line, "%d %d rlineto", &dx, &dy) < 2) 
#endif
			{
				showerror("analyzepath rlineto");
			}
			x = x + dx, y = y + dy;
			lineto(output, x, y);
		}						
		else if (strstr(line, "vlineto") != NULL)  {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg vlineto", &dy) < 1) 
#else
			if (sscanf(line, "%d vlineto", &dy) < 1) 
#endif
			{
				showerror("analyzepath vlineto");
			}
			y = y + dy;
			lineto(output, x, y);
		}
		else if (strstr(line, "hlineto") != NULL)  {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg hlineto", &dx) < 1) 
#else
			if (sscanf(line, "%d hlineto", &dx) < 1) 
#endif
			{
				showerror("analyzepath hlineto");
			}
			x = x + dx;
			lineto(output, x, y);
		}
		else if (strstr(line, "rrcurveto") != NULL)  {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg %lg %lg %lg %lg %lg rrcurveto",
#else
			if (sscanf(line, "%d %d %d %d %d %d rrcurveto",
#endif
				&dxa, &dya, &dxb, &dyb, &dxc, &dyc) < 6) {
				showerror("analyzepath rrcurveto");
			}
			x = x + dxa; y = y + dya; xa = x; ya = y; 
			x = x + dxb; y = y + dyb; xb = x; yb = y; 
			x = x + dxc; y = y + dyc; xc = x; yc = y; 
			curveto(output, xa, ya, xb, yb, xc, yc);
		}						
		else if (strstr(line, "hvcurveto") != NULL)  {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg %lg %lg %lg hvcurveto",
#else
			if (sscanf(line, "%d %d %d %d hvcurveto",
#endif
				&dxa, &dxb, &dyb, &dyc) < 4) {
				showerror("analyzepath hvcurveto");
			}
			x = x + dxa; xa = x; ya = y; 
			x = x + dxb; y = y + dyb; xb = x; yb = y; 
			y = y + dyc; xc = x; yc = y; 
			curveto(output, xa, ya, xb, yb, xc, yc);
		}						
		else if (strstr(line, "vhcurveto") != NULL)  {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg %lg %lg %lg vhcurveto",
#else
			if (sscanf(line, "%d %d %d %d vhcurveto",
#endif
				&dya, &dxb, &dyb, &dxc) < 4) {
				showerror("analyzepath vhcurveto");
			}
			y = y + dya; xa = x; ya = y; 
			x = x + dxb; y = y + dyb; xb = x; yb = y; 
			x = x + dxc; xc = x; yc = y; 
			curveto(output, xa, ya, xb, yb, xc, yc);
		}						
		else if (strstr(line, "callsubr") != NULL &&
#ifdef FLOATCOORDS
			sscanf(line, "%lg %lg %lg %d callsubr", 
#else
			sscanf(line, "%d %d %d %d callsubr", 
#endif
/*				&xa, &ya, &xb, &yb) == 4)  */
				&flx, &xabs, &yabs, &ns) == 4) {
			if (debugflag) fputs(line, stdout);
/*			if (flexproc != 8 || yb != 0) */
			if (flexproc != 8 || ns != 0) {
				fprintf(errout, 
					"Invalid termination (%d) of FlexProc char %d\n", 
						flexproc, chrs);
				fprintf(errout, "%s", line);
			}
			else {		/* end of Flex Proc */
/*				if (debugflag) fputs("End of FlexProc\n", stdout); */
				curveto(output, xflex[2], yflex[2], xflex[3],
					yflex[3], xflex[4], yflex[4]); 
				curveto(output, xflex[5], yflex[5], xflex[6],
					yflex[6], xflex[7], yflex[7]); 
				x = xflex[7]; y = yflex[7];
/*				end of FlexProc */ /* expect xa == 50, yb == 4 */
				if (x < xabs - flxeps || x > xabs + flxeps ||
					y < yabs - flxeps || y > yabs + flxeps) {
/*			fprintf(errout, "drift: (%d %d) != (%d %d) ", ya, xb, x, y); */
#ifdef FLOATCOORDS
			fprintf(errout, "drift: (%lg %lg) != (%lg %lg) ",
				xabs, yabs, x, y);
#else
			fprintf(errout, "drift: (%d %d) != (%d %d) ", xabs, yabs, x, y);
#endif
				}
				if (snaptoabs) {
					x = xabs; y = yabs;		/* snap to absolute coords */
				}
				flexproc = 0;			/* reset pointer */
				if (debugflag) fputs("End of FlexProc\n", stdout);
			}		/* end of end-of-FlexProc case */
		}			/* end of x y z n callsubr case */
		else if (strstr(line, "callsubr") != NULL &&
			sscanf(line, "%d %d callsubr", &na, &nb) == 2) {
/* new stylized hint switching scheme */
/*			if (nb == 4) 	 */
			if (assumeswitch != 0 || nb == 4) {		/* 1993/Aug/30 */
				if (nb != 4) printf("Assuming hint switch: %s", line);
				if (markswitch != 0) {
					if (hintflag != 0) writehints(chrs, stemout);
					if (currentset != 0)	/* current point ok, just do it */
						fprintf(output, "%d s %%\n", ++nsubr); 
					else {
						if (traceflag != 0) 
							printf("Delaying switch for char %d -", chrs); 
						if (savedswitch != 0) 
							fprintf(errout, "Nested saved hint switches\n");
						savedswitch++;		/* save the switch for moveto */
					}
				}
				subrlevel++;
				filepos = ftell(input);
				if (na > MAXSUBRS || subrspointer[na] < 0) {
					fprintf(stderr, "Unknown Subr %d\n", na);
					exit(9);				
				}
				if (debugflag) printf("Going off to Subr %d (level %d)\n",
					na, subrlevel);
				fseek(input, subrspointer[na], SEEK_SET); /* go to subr */
				(void) analyzepath(output, input, stemout); /* recurse ! */
				if (debugflag) printf("Came back from Subr %d\n", na);
/*				if (feof(input) != 0) break;  */
				fseek(input, filepos, SEEK_SET);
			}						 /* end of x y callsubr case */
			else {
				showerror("analyzepath callsubr");
			}
		}
		else if (strstr(line, "callsubr") != NULL) {
			if (sscanf(line, "%d callsubr", &na) < 1) {
 				if (onthestack >= 0) {					/* 1996/July/18 */
/* do not treat as error, delayed pop callsubr part from callothersubr */
					if (traceflag) printf("analyzepath callsubr"); 
					na = onthestack;
					onthestack = -1;
				}
				else showerror("analyzepath callsubr");
			}

			if (allowflex != 0 && na == 1) {
				xflex[0] = x; yflex[0] = y;
				if (flexproc != 0) {
					fprintf(errout, 
						"Restart of FlexProc in char %d\n", chrs);
					flexproc = 0;
				}
				flexproc++;		/*	start of FlexProc */ 
			}
			else if (allowflex != 0 && na == 2) {
				if (flexproc++ >= 8) {	/*	repeating part of FlexProc */
					fprintf(errout, 
						"Too many steps in FlexProc in char %d\n", chrs);
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
				if (debugflag) printf("Going off to Subr %d (level %d)\n",
					na, subrlevel);
				fseek(input, subrspointer[na], SEEK_SET); /* go back subr */
				(void) analyzepath(output, input, stemout); /* recurse ! */
				if (debugflag) printf("Came back from Subr %d\n", na);
/*				if (feof(input) != 0) break;  */
				fseek(input, filepos, SEEK_SET);
			}
		} 						 /* end of n callsubr case */
		else if (strstr(line, "callothersubr") != NULL) {
/*		check that this is the expected stylized use of callothersubr: */
			if (sscanf(line, "%d %d %d callothersubr", &na, &nb, &nc) < 3) {
/* In Subrs 0, 1, 2, 3, 4 may find callothersubr with two args */
/* In which case we just pop out of here ... 94/Nov/1 WHY NOW ??? */
				if (na == 3 && nb == 0) return 0;		/* 94/Nov/1 */
				if (na == 0 && nb == 1) return 0;		/* 94/Nov/1 */
				if (na == 0 && nb == 2) return 0;		/* 94/Nov/1 */
				if (na == 1 && nb == 3) return 0;		/* 94/Nov/1 */
				showerror("analyzepath callothersubr");
			}
			if (nb != 1 || nc != 3) {
				showerror("analyzepath callothersubr");
			}
			if (markswitch != 0) {
/*				curveto(output, na, na, na, na, na, na); */
/*				fprintf(output, "%d s %%\n", na); */
				if (hintflag != 0) writehints(chrs, stemout);
				if (currentset != 0)		/* current point ok, just do it */
					fprintf(output, "%d s %%\n", ++nsubr); 
				else {
					if (traceflag != 0) 
						printf("Delaying switch for char %d -", chrs); 
					if (savedswitch != 0) 
						fprintf(errout, "Nested saved hint switches\n");
					savedswitch++;		/* save the switch for moveto */
				}
			}
			/* read expected `pop' line */
			if (fgets(line, MAXLINE, input) == NULL) return 0;
			if (strncmp(line, "pop", 3) != 0) {
/* following added 1996/July/16 to */
/* take care of case where pop callsubr are in a subr UGH */
				if (sscanf(line, "%d callsubr", &nc) > 0) {
					printf ("%d on stack (calling %d) ", na, nc);
					onthestack = na;	/* remember na from callothersubr */
					subrlevel++;
					filepos = ftell(input);
					if (nc > MAXSUBRS || subrspointer[nc] < 0) {
						fprintf(stderr, "Unknown Subr %d\n", nc);
						exit(9);				
					}
					if (debugflag) printf("Going off to Subr %d (level %d)\n",
										  nc, subrlevel);
					fseek(input, subrspointer[nc], SEEK_SET); /* go to subr */
					(void) analyzepath(output, input, stemout); /* recurse ! */
					if (debugflag) printf("Came back from Subr %d\n", nc);
/*					if (feof(input) != 0) break;  */
					fseek(input, filepos, SEEK_SET);
				}
				else showerror("analyzepath expecting pop");
				goto badcallother;
			}
			/* read expected `callsubr' line */
			if (fgets(line, MAXLINE, input) == NULL) return 0;
			if (strncmp(line, "callsubr", 8) != 0) {
				showerror("analyzepath expecting callsubr");
				goto badcallother;
			}
			subrlevel++;
			filepos = ftell(input);
			if (na > MAXSUBRS || subrspointer[na] < 0) {
				fprintf(stderr, "Unknown Subr %d\n", na);
				exit(9);				
			}
			if (debugflag) printf("Going off to Subr %d (level %d)\n",
				na, subrlevel);
			fseek(input, subrspointer[na], SEEK_SET); /* go back to subr */
			(void) analyzepath(output, input, stemout); /* recurse ! */
			if (debugflag) printf("Came back from Subr %d\n", na);
/*			if (feof(input) != 0) break;  */
			fseek(input, filepos, SEEK_SET);
badcallother: ;
		}		 /* end of callothersubr case */
		else if (strstr(line, "vstem3") != NULL) {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg %lg %lg %lg %lg %lg vstem3", 
#else
			if (sscanf(line, "%d %d %d %d %d %d vstem3", 
#endif
				&xa, &dxa, &xb, &dxb, &xc, &dxc) < 6) {
				showerror("analyzepath vstem3"); 
			}
			insertvstem(xa, dxa);
			insertvstem(xb, dxb);
			insertvstem(xc, dxc);			
		}
		else if (strstr(line, "vstem") != NULL) {
			if (strstr(line, "div") != NULL) {
#ifdef FLOATCOORDS
				fprintf(errout, "Illegal div!\n");	/* 1994/June/29 */
#else
				if (traceflag != 0) printf("%s", line);
				if (sscanf(line, "%ld %ld div %ld %ld div vstem", 
					&nsx, &dsx, &ndx, &ddx) == 4) {
					sx = (int) ((nsx + dsx/2)/dsx);
					dx = (int) ((ndx + ddx/2)/ddx);
				}
				else if (sscanf(line, "%ld %ld div %d vstem", 
					&nsx, &dsx, &dx) == 3) {
					sx = (int) ((nsx + dsx/2)/dsx);
				}
				else if (sscanf(line, "%d %ld %ld div vstem", 
					&sx, &ndx, &ddx) == 3) {
					dx = (int) ((ndx + ddx/2)/ddx);
				}
				else {
					showerror("analyzepath vstem");
				}
				if (traceflag != 0) printf("sx %d dx %d \n", sx, dx);
#endif
			}
#ifdef FLOATCOORDS
			else if (sscanf(line, "%lg %lg vstem", &sx, &dx) < 2) 
#else
			else if (sscanf(line, "%d %d vstem", &sx, &dx) < 2) 
#endif
			{
				showerror("analyzepath vstem");
			}
			if (dx < 0) {sx = sx + dx; dx = - dx; } /* crock of s... */
			insertvstem(sx, dx);
		}			/* end of vstem case */
		else if (strstr(line, "hstem3") != NULL) {
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg %lg %lg %lg %lg %lg hstem3", 
#else
			if (sscanf(line, "%d %d %d %d %d %d hstem3", 
#endif
				&ya, &dya, &yb, &dyb, &yc, &dyc) < 6) 
			{
				showerror("analyzepath hstem3"); 
			}
			inserthstem(ya, dya);
			inserthstem(yb, dyb);
			inserthstem(yc, dyc);			
		}
		else if (strstr(line, "hstem") != NULL) {
			if (strstr(line, "div") != NULL) {
#ifdef FLOATCOORDS
				fprintf(errout, "Illegal div!\n");	/* 1994/June/29 */
#else
				if (traceflag != 0) printf("%s", line);
				if (sscanf(line, "%ld %ld div %ld %ld div hstem", 
					&nsy, &dsy, &ndy, &ddy) == 4) {
					sy = (int) ((nsy + dsy/2)/dsy);
					dy = (int) ((ndy + ddy/2)/ddy);
				}
				else if (sscanf(line, "%ld %ld div %d hstem", 
					&nsy, &dsy, &dy) == 3) {
					sy = (int) ((nsy + dsy/2)/dsy);
				}
				else if (sscanf(line, "%d %ld %ld div hstem", 
					&sy, &ndy, &ddy) == 3) {
					dy = (int) ((ndy + ddy/2)/ddy);
				}
				else {
					showerror("analyzepath hstem");
				}
				if (traceflag != 0) printf("sy %d dy %d \n", sy, dy);
#endif
			}
#ifdef FLOATCOORDS
			else if (sscanf(line, "%lg %lg hstem", &sy, &dy) < 2) 
#else
			else if (sscanf(line, "%d %d hstem", &sy, &dy) < 2) 
#endif
			{
				showerror("analyzepath hstem");
			}
			if (dy < 0) {sy = sy + dy; dy = - dy; } /* crock of s... */
			inserthstem(sy, dy);
		}			/* end of hstem case */
		else if (strstr(line, "dotsection") != NULL) {	} /* ignore */
/*		else if (strstr(line, "pop") != NULL) {	} */
		/* callothersubr, callsubr, pop */
		/* seac, sbw, hstem, hstem3, vstem, vstem3, div */
		/* return, setcurrentpoint, dotsection,  */
		else if (strstr(line, "seac") != NULL) {
			fprintf(errout, "seac: %d ", chrs);
			return -1;
		}
		else if (strncmp(line, "pop", 3) == 0 && onthestack >= 0) {
/* do not treat as error, delayed pop callsubr part from callothersubr */
			if (traceflag) printf("analyzepath pop");		/* 1996/July/18 */
		}
		else {
			showerror("analyzepath unrecognized");
		}
	} /* end of for loop */
	if (debugflag) fputs("About to return from analyzepath\n", stdout);
	return 0;
}

void docharstrings (FILE *input, FILE *output, FILE *stemout) {
	int k;
#ifndef FLOATCOORDS
	int denx;
#endif
/*	char charname[CHARNAME_MAX];  */

	numprocs=0;
	for (k=0; k < MAXCHRS; k++) {
		chrs = k;
		if (charspointer[k] < 0) continue;
		if (traceflag != 0) printf("%d: %s ", k, encoding[k]);
		else putc('.', stdout);
		fseek(input, charspointer[k], SEEK_SET);
		if (fgets(line, MAXLINE, input) == NULL) continue;	/* EOF */
		if (strncmp(line, "readonly", 8) == 0) return; 
		numprocs++;
		hsteminx=0; vsteminx=0;
		currentset = 0;						/* no current point yet */
		savedswitch = 0;					/* no hint switch saved up */
		fprintf(output, "]\n"); 			/* possible show charname ? */
/*		fprintf(output, "] %% %s\n", encoding[k]); */

#ifdef FLOATCOORDS
		if (strstr(line, "div") != NULL) checkfordiv (line); /* 1994/June/29 */
#endif

/* restriction of this code is we assume hsbw occurs at top level ... */
		if (strstr(line, "hsbw") != NULL) {
/* should be able to deal with div here */
			if (strstr(line, "div") != NULL) { /* crude hack */
#ifdef FLOATCOORDS
				fprintf(errout, "Invalid appearance of div: %s", line);
#else
				if (sscanf(line, "%d %d %d div hsbw", 
					&sbx, &widthx, &denx) < 3) {
					showerror("docharstrings div");
				}
				sby = 0; widthx = widthx / denx; widthy = 0;
#endif
			}
#ifdef FLOATCOORDS
			else if (sscanf(line, "%lg %lg hsbw", &sbx, &widthx) < 2) 
#else
			else if (sscanf(line, "%d %d hsbw", &sbx, &widthx) < 2) 
#endif
				{
					showerror("docharstrings hsbw");
				}
			sby = 0; widthy = 0;
		}
		else if (strstr(line, "sbw") != NULL) {
/* should be able to deal with div here */
#ifdef FLOATCOORDS
			if (sscanf(line, "%lg %lg %lg %lg sbw", 
#else
			if (sscanf(line, "%d %d %d %d sbw", 
#endif
				&sbx, &sby, &widthx, &widthy) < 4) {
					showerror("docharstrings sbw");
			}
		}
		else {
			showerror("docharstrings expecting hsbw - could be in subr");
		}

/* allow for `div' here later maybe */ /* sbx is x offset to use */
/* allow for `sbw' here later maybe */
		x = sbx, y = sby;
/*		fprintf(output, "%d %d\n", chrs, widthx);  */
#ifdef FLOATCOORDS
		fprintf(output, "%d %d %% %s\n", chrs, kround(widthx), encoding[k]);  
#else
		fprintf(output, "%d %d %% %s\n", chrs, widthx, encoding[k]);  
#endif
		subrlevel=0; flexproc=0; nsubr=0;
/*		if (analyzepath(output, input, stemout) < 0) break; */	/* seac */
		(void) analyzepath(output, input, stemout);	/* seac */
		if (feof(input) != 0) break; /* ??? */
		if (flexproc != 0) 
			fprintf(errout, "Bad FlexProc sequence in char %d\n", chrs);
		if (subrlevel != 0) 
			fprintf(errout, "Non-zero (%d) subr level\n",
				subrlevel);
		if (hintflag != 0) writehints(chrs, stemout);
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

char *stripname (char *fname) {
	char *s;
	if ((s = strrchr(fname, '\\')) != NULL) return s+1;
	else if ((s = strrchr(fname, '/')) != NULL) return s+1;	
	else if ((s = strrchr(fname, ':')) != NULL) return s+1;	
	else return fname;
}

void writeheader(FILE *output) {
/*	write dummy header */
/*	fprintf(output, "10.0000 1.00\n"); */
	if (szFontName != NULL) fprintf(output, "%%%% FontName: %s\n", szFontName);
	else fprintf(errout, "WARNING: FontName not found\n");
	fprintf(output, "10.0000 %lg\n", scalef);
	if (xll == xur || yll == yur) {	/* default if failed to find */
		xll = -100; yll = -250; xur = 1000; yur = 900;
		if (scalef != 1.0) {
			xll = (int) (scalef * xll);
			xur = (int) (scalef * xur);
			yll = (int) (scalef * yll);
			yur = (int) (scalef * yur);
		}
	}
	fprintf(output, "%d %d %d %d\n", xll, yll, xur, yur);
/*	fprintf(stemout, "%% %s\n", fontname); */
}

void showusage(char *name) {
	printf("Useage: %s {-v} {-t} {-d} {-f} <font.pln>\n", name);
	printf("\tv	verbose mode\n");
	printf("\tt	turn on tracing output\n");
	printf("\td	turn on debugging output\n");
	printf("\tf	disable FlexProc use\n");
	exit(1);
}

void extendunder (char *filename) {
	char *s, *t;
	char exten[16];
	int k, n;
	
	if ((s = strrchr(filename, '.')) == NULL) return;
	strcpy(exten, s);				/* save extension */
	*s = '\0';						/* clip of extension */
	if ((t = stripname(filename)) == NULL) return;
	n = strlen(t);
	for (k = n; k < 8; k++) *s++ = '_';
	*s = '\0';
	strcpy (s, exten);				/* paste extension back in */
}

int main(int argc, char *argv[]) {       /* main program */
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_stm[FILENAME_MAX];
	FILE *fp_in, *fp_out;
	FILE *fp_stm=NULL;
	int nsubrs, nchars, firstarg=1;

/*	printf("Outline and hint extraction program version 1.2\n"); */
	printf("Outline and hint extraction program version 1.2.1\n");

/*	printf("First argv %s length %d\n", 
		argv[firstarg], strlen(argv[firstarg]));  */
	while (firstarg < argc && *argv[firstarg] == '-') {
		if (strcmp(argv[firstarg], "-f") == 0) {
			printf("Disallowing FlexProc use\n"); 
			allowflex = 0;					/* disallow FlexProc use */
		}
		else if (strcmp(argv[firstarg], "-s") == 0) {
			printf("Disallowing short hint switch code (except n == 4)\n"); 
			assumeswitch = 0;				/* disallow hint switch */
		}
		else if (strcmp(argv[firstarg], "-t") == 0) {
			printf("Turning trace on\n"); 
			traceflag = 1;					/* Turning trace on */
		}
		else if (strcmp(argv[firstarg], "-v") == 0) {
			printf("Verbose mode on\n"); 
			verboseflag = 1;					/* Turning debug on */
		}
		else if (strcmp(argv[firstarg], "-d") == 0) {
			printf("Debugging trace on\n"); 
			debugflag = 1;					/* Turning debug on */
		}
		else if (strcmp(argv[firstarg], "-?") == 0) {
			showusage(argv[0]);
		}
		else {
			fprintf(stderr, "Illegal command line argument %s\n",
				argv[firstarg]); 
			exit(1);
		}
		firstarg++;
	}
	if (firstarg >= argc) showusage(argv[0]);

	totalround = 0;

	strcpy(fn_in, argv[firstarg]);
	extension(fn_in, "pln");
	if ((fp_in = fopen(fn_in, "rb")) == NULL) { /* rb to avoid C-M C-J pro */
		extendunder (fn_in);
		if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
			perror(fn_in);
			exit(3);
		}
	}
	if (verboseflag != 0) printf("Processing %s\n", fn_in);
/*	strcpy(fn_out, argv[firstarg]); */
	strcpy(fn_out, stripname(fn_in));
	forceexten(fn_out, "out");
	if ((fp_out = fopen(fn_out, "w")) == NULL) {
		perror(fn_out);
		exit(3);
	}
	fprintf(fp_out, "%%%% %s\n", fn_out);
	if (hintflag != 0) {
/*		strcpy(fn_stm, argv[firstarg]); */
		strcpy(fn_stm, stripname(fn_in));
		forceexten(fn_stm, "stm");
		if ((fp_stm = fopen(fn_stm, "w")) == NULL) {
			perror(fn_stm);
			exit(3);
		}
		fprintf(fp_stm, "%% %s\n", fn_in);
	}
	getEncoding(fp_in);
	if (verboseflag != 0 || numencod == 0)
		printf(" - %d char names\n", numencod);
	scantoeexec(fp_in);
	nsubrs = getsubrs(fp_in, fp_stm);
	if (verboseflag != 0 || numsubrs == 0)
		printf(" - %d Subrs (%d recorded)\n", numsubrs, nsubrs);
/*	if (traceflag != 0)  */
/*		putc('\n', stdout); */	/* flushed */
	nchars = getchars(fp_in);
/*	if (traceflag != 0)  */
/*		putc('\n', stdout); */	/* flushed */
	if (verboseflag != 0 || numchars == 0 || nchars == 0)
		printf(" - %d CharStrings (%d recorded)\n",	numchars, nchars);	
	if (nchars == 0) {
		fprintf(stderr, "Premature EOF or bad file format\n");
		fclose(fp_in); 	
		fclose(fp_out);  
		if (hintflag != 0) 	fclose(fp_stm);
		exit(69);
	}
	getFontName(fp_in);
/*	writeheader(fp_out, fp_stm, fn_in); */
	writeheader(fp_out);
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
/*		fprintf(errout, "Outline file: "); */
		perror(fn_out);
		exit(5);
	}
	else fclose(fp_out);
	if (hintflag != 0) {
		if (ferror(fp_stm) != 0) {
/*			fprintf(errout, "Stem file: "); */
			perror(fn_stm);
			exit(5);
		}
		else fclose(fp_stm);
	}
	if (totalround) printf("Rounded %d coordinates\n", totalround);
	return 0;
}

#ifdef IGNORED

/*
 * Adobe Type 1 and Type 2 CharString commands; the Type 2 commands are
 * suffixed by "_2". Many of these are not used for computing the
 * weight vector but are listed here for completeness.
 */
#define HSTEM          1
#define VSTEM          3
#define VMOVETO        4
#define RLINETO        5
#define HLINETO        6
#define VLINETO        7
#define RRCURVETO      8
#define CLOSEPATH      9
#define CALLSUBR      10
#define RETURN        11
#define ESCAPE        12
#define HSBW          13
#define ENDCHAR       14

#define BLEND_2       16
#define HSTEMHM_2     18
#define HINTMASK_2    19
#define CNTRMASK_2    20

#define RMOVETO       21
#define HMOVETO       22

#define VSTEMHM_2     23
#define RCURVELINE_2  24
#define RLINECURVE_2  25
#define VVCURVETO_2   26
#define HHCURVETO_2   27
#define CALLGSUBR_2   29

#define VHCURVETO     30
#define HVCURVETO     31

/*
 * Adobe Type 1 and Type 2 CharString Escape commands; the Type 2
 * commands are suffixed by "_2"
 */
#define DOTSECTION       0
#define VSTEM3           1
#define HSTEM3           2

#define AND_2            3
#define OR_2             4
#define NOT_2            5

#define SEAC             6
#define SBW              7

#define STORE_2          8
#define ABS_2            9
#define ADD_2           10
#define SUB_2           11


#define DIV             12

#define LOAD_2          13
#define NEG_2           14
#define EQ_2            15

#define CALLOTHERSUBR   16
#define POP             17

#define DROP_2          18
#define PUT_2           20
#define GET_2           21
#define IFELSE_2        22
#define RANDOM_2        23
#define MUL_2           24
#define DIVX_2          25
#define SQRT_2          26
#define DUP_2           27
#define EXCH_2          28
#define INDEX_2         29
#define ROLL_2          30

#define SETCURRENTPOINT 33

#define HFLEX_2         34
#define FLEX_2          35
#define HFLEX1_2        36
#define FLEX1_2         37

#endif

/* character outlines and character hints are not sorted ... OK */

/* processing of flex proc may still be buggy */

/* suppress use of subrs for FlexProc using -f for LucidaBright - why? */

/* suppress use of subrs for FlexProc using -f for Fontographer crud */

/* now allows "div" operator in hstem & vstem - not yet in hstem3 & vstem3 */

/* now uses strdup for encoding vector ... */

/* starting to switch to allowing FLOATCOORDS */ /* make permanent ? */

/* should copy across font-level hints to output stem file ... */

/* may need to compile with increased stack allocation */
/* if Subrs nested deeper than about 4 levels */
/* since `analyzepath' now has double local variables ... */
/* Try compile4 extroutl 5000 */
