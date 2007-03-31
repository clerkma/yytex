/* sidebear.c
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
#include <conio.h>

#include "metrics.h"

#define PADSPACES 20

int verboseflag = 0;
int traceflag = 0;
int batchmode = 0;		/* 1997/Nov/13 */
int detailflag = 0;
int debugflag = 0;
int wipeclean = 1;	
int dotsflag = 1;
int quietflag = 0;

int afmflag = 0;

int checkFlexProc=1;	// 99/Dec/11

int seenFlexProc=0;	// 99/Dec/11

/* for this one, MAXCHARINFONT 2048 is just too much ? */

int stretchflag = 0;	/* next command line arg is tracking or tracking * 100.0 */

double tracking=1.0;	/* amount to stretch or shrink */

int letterflag = 0;		/* next command line arg is letterspace 94/Nov/12 */

int letterspace=0;		/* letter spacing amount  94/Nov/12  */
int letterleft=0;
int letterright=0;

int lettertrackflag = 0;

int metricflag = 0;		/* next command line arg is metrics file */

int trackflag = 0;		/* institute tracking adjustment */

int renameflag = 0;

int eexecscan = 0;		/* scan up to eexec before starting */
int charscan = 0;		/* scan up to RD before starting */
int charenflag = 0;		/* non-zero charstring coding instead of eexec */
int decodecharflag = 0;	/* non-zero means also decode charstring */
int binaryflag = 0;		/* input is binary, not hexadecimal */

int extractonly = 1;	/* non-zero => extract info only */

int	afmfileflag = 0;

int pfafileflag = 1;

volatile int bAbort = 0;			/* set when user types control-C */

extern int encrypt(FILE *, FILE *);

extern int decrypt(FILE *, FILE *);

char *metricfile="";

char *tempdir="";		/* place to put temporary files */

FILE *errout=stderr;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
	 
char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_bak[FILENAME_MAX];

/* int stringindex=0; */

/* char stringspace[MAXSTRINGSPACE]; */	/* Place to get string allocations */

char *charnames[MAXCHARINFONT];

int sidebearing[MAXCHARINFONT];

long charnumer[MAXCHARINFONT], chardenom[MAXCHARINFONT];

int charindex;			/* index into table of new sidebearings and widths */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */ 

int wantcpyrght=1;

/* Font merging and normalizing program version */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* char *progname="SIDEBEAR"; */

char progname[16]=""; 

/* char *progversion="1.0"; */

/* char *progversion="1.1"; */

/* char *progversion="1.2"; */
/* char *progversion="1.3"; */
/* char *progversion="1.3.4"; */
// char *progversion="2.0";		/* 98/Dec/25 */
char *progversion="2.1";		/* 99/Dec/11 */

/* char *programversion = "SIDEBEAR - adjusts sidebearings & widths - version
1.0"; */

char *copyright = "\
Copyright (C) 1992-1999  Y&Y, Inc.  http://www.YandY.com\
";

/* Copyright (C) 1992--1997  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 2670755 */
/* #define COPYHASH 13986445 */
/* #define COPYHASH 5618236 */
/* #define COPYHASH 6490707 */
/* #define COPYHASH 2658560 */
/* #define COPYHASH 3956063 */
#define COPYHASH 7732596

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

struct ratio { /* structure for rational numbers */ /* 1997/June/30 */
	long numer;
	long denom;
};

/* compute good rational approximation to floating point number */
/* assumes number given is positive */

struct ratio rational(double x, long nlimit, long dlimit) {
	long p0=0, q0=1, p1=1, q1=0, p2, q2;
	double s=x, ds;
	struct ratio res;
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
		if (s > 1000000000.0 || s < -1000000000.0) break; /* 1992/Dec/6 */
	}
	assert(q2 != 0);
	res.numer = p2; res.denom = q2;
	return res;		/* the answer is p2/q2 */
}

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

#ifdef IGNORED
char *strdupx(char *name) {	/* our own cheap _strdup */
	char *s;
	int n = strlen(name) + 1;

	if (stringindex + n >= MAXSTRINGSPACE) {
		fprintf(stderr, "ERROR: Out of string pool space for character names\n");
		exit(1);
	}		
	s = stringspace + stringindex;
	strcpy (s, name);
	stringindex += n;
	return s;
}
#endif

char *zstrdup(char *name) {
	char *new=_strdup(name);
	if (new == NULL) {
		fprintf(stderr,
				"ERROR: Out of memory for character names (%s)\n", name);
		exit(1);
	}
	return new;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

static char line[MAXLINE];		/* buffer for getline */

static int getline(FILE *fp_in, char *line) {
	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	return strlen(line); 
} 

int lookup(char *name) {	/* look up name in table of new sbw */
	int k, flag = -1;
	for (k = 0; k < charindex; k++) {
/*		if (strcmp(charnames[k], "") == 0) continue; */
		if (charnames[k] == NULL) continue;
		if (strcmp(charnames[k], name) == 0) {
			flag = k;	break;
		}
	}
	return flag;
}

/* extractonly is non-zero if only extracting old information */ 
/* otherwise we also write a new file output */

int extractside(FILE *fp_out, FILE *fp_in, int extractonly) {
	char charname[CHARNAME_MAX];
	int k, n;
	int unknowns=0, count=0;
	char *s;
	int side, width, newside, newwidth;
	int fw, fx, fy, fn;			// 99/Dec/11
	
/*  scan up to eexec encrypted part */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "eexec") == NULL) {
		if (extractonly == 0) fputs(line, fp_out);
	}
	if (extractonly == 0) fputs(line, fp_out);
/*	scan up to CharStrings */
	while ((n = getline(fp_in, line)) != 0 && 
		strstr(line, "/CharStrings") == NULL) {
		if (extractonly == 0) {
			if (checkFlexProc) {
				if (strstr(line, " 0 callsubr") != NULL) {
					seenFlexProc++;
					if (traceflag) printf("SUBRS: %s", line);
					if (sscanf(line, "%d %d %d %d callsubr", &fw, &fx, &fy, &fn) == 4) {
						sprintf(line, "%d %d %d %d callsubr\n", fw, fx+letterleft, fy, fn);
						if (traceflag) printf("SUBRS: %s", line);
					}
				}
			}
			fputs(line, fp_out);
		}
	}
	if (extractonly == 0) fputs(line, fp_out);

/*	scan all of the CharStrings */
	for (;;) {
		n = getline(fp_in, line);
		if (n <= 0) break;
		if (strchr(line, '/') == NULL &&
			strstr(line, "endchar") == NULL && 
			strstr(line, "end") != NULL) break;
		if (strstr(line, "readonly put") != NULL) break;
		if (strstr(line, "/FontName") != NULL) break;

		if (extractonly == 0) {
			if (checkFlexProc) {
				if (strstr(line, " 0 callsubr") != NULL) {
					seenFlexProc++;
					if (traceflag) printf("CHARSTRINGS: %s", line);
					if (sscanf(line, "%d %d %d %d callsubr", &fw, &fx, &fy, &fn) == 4) {
						sprintf(line, "%d %d %d %d callsubr\n", fw, fx+letterleft, fy, fn);
						if (traceflag) printf("CHARSTRINGS: %s", line);
					}
				}
			}
			fputs(line, fp_out);
		}

		if ((s = strchr(line, '/')) != NULL) {
			sscanf(s, "/%s ", charname);
			if (dotsflag != 0) putc('.', stdout);
			while ((n = getline(fp_in, line)) != 0 && 	
				strstr(line, "sbw") == NULL) {
				if (extractonly == 0) fputs(line, fp_out);
			}

			if ((s = strstr(line, "sbw")) != NULL) {
				if (extractonly != 0) {
					fprintf(fp_out, "%s ", charname);
					n = strlen(charname);
					for (k = n+1; k < PADSPACES; k++) putc(' ', fp_out);
					fprintf(fp_out, "%s", line);
				}
				else {		/* replace line with new words */
					if (strstr(line, "hsbw") == NULL) {
						fprintf(errout, "Non-standard for char `%s': %s", 
							charname, line);
					}
//	adjust each width by specified percentage (tracking)
					if (trackflag != 0) {			/* 1992/Oct/31 */
						sscanf(line, "%d %d ", &side, &width);
						newwidth = (int) (tracking * width + 0.5);
						newside = side + (newwidth - width + 1)/2;
						if (traceflag)	printf("OLD %d %d NEW %d %d\n",
								side, width, newside, newwidth);
						fprintf(fp_out, "%d %d hsbw\n",
							newside, newwidth);
						letterleft = newside - side;
						letterright = newwidth - width - letterleft;
					}
//  adjust each widths by fixed amounts (letterleft and letterright)
					else if (lettertrackflag != 0) {		/* 1994/Nov/12 */
						sscanf(line, "%d %d ", &side, &width);
						newwidth = width + letterleft + letterright;
						newside = side + letterleft;
						if (traceflag)	printf("OLD %d %d NEW %d %d\n",
								side, width, newside, newwidth);
						fprintf(fp_out, "%d %d hsbw\n",
							newside, newwidth);
					}
//  adjust according to entry in specified table
					else {	/* not tracking, using specified table */
						k = lookup(charname);
						if (k >= 0) {
							sscanf(line, "%d %d ", &side, &width);
/*							if (charwidth[k] == UNKNOWN) charwidth[k] = width;*/
/*	deal with case where sidebearing is given, but not width */
							if (charnumer[k] == UNKNOWN) {
								charnumer[k] = width; chardenom[k] = 1;
							}
/*	deal with case when AFM file gives width (no sidebearing) */
							if (afmflag) sidebearing[k] = side; /* 97/June/30 */
							if (chardenom[k] == 1) 
/*								fprintf(fp_out, "%d %d hsbw\n", */
								fprintf(fp_out, "%d %ld hsbw\n",
/*									sidebearing[k], charwidth[k]); */
									sidebearing[k], charnumer[k]);
							else fprintf(fp_out, "%d %ld %ld div hsbw\n",
								sidebearing[k], charnumer[k], chardenom[k]);
							count++;
						}
						else {
							fputs(line, fp_out);
							if (traceflag != 0) printf("%s? ", charname);
							unknowns++;
						}
					}
				}
			}
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */		
	}		
	if (extractonly == 0) {		/* copy the tail across */
		if (n != 0) fputs(line, fp_out);
		while ((n = getline(fp_in, line)) != 0)	fputs(line, fp_out);
	}
	if (dotsflag != 0 || unknowns > 0) putc('\n', stdout);
	if (quietflag == 0) {
	if (count != 0) 
		printf("Sidebearings and widths for %d chars were altered\n", count);
	if (unknowns != 0) 
		printf("Sidebearings and widths for %d chars were not altered\n", unknowns);
	}
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

/* Read side bearing from file */

int readsides (FILE *fp_in) {
	char charname[CHARNAME_MAX];
	long numer, denom;
	int side, width, k, n, m, flag;
	double dwidth;
	char *s=NULL, *t=NULL;
	int chr;
	struct ratio rscale;				/* 1997/June/30 */

	if (traceflag != 0) printf("Reading sidebearing file\n");

	afmflag = 0;

	charindex = 0;

	for (k = 0; k < MAXCHARINFONT; k++)	charnames[k] = NULL;

/*	First check whether absurd format (i.e. part of a font file) */
	getline(fp_in, line);
	if (strncmp(line, "%!PS-AdobeFont", 14) == 0) {
/*		try and find metrics */
		while ((n = getline(fp_in, line)) != 0 &&
			(s = strstr(line, "/Metrics")) == NULL) ;
		if (s == NULL || n == 0) {
			fprintf(errout, "ERROR: Unable to find metric information\n");
			return -1;
		}
		strtok (s, " \t\n\r\f[]");	/* prime the token pump */
		for(;;) {
			s = gettoken(fp_in);
			if (s == NULL || strcmp(s, "end") == 0) break;
			if (*s == '/') {
				strcpy(charname, s+1);
				s = gettoken(fp_in);
				if (sscanf(s, "%d", &side) < 1)
					fprintf(errout, "WARNING: Don't understand: %s", s);
				s = gettoken(fp_in);
				if (sscanf(s, "%ld", &numer) < 1)
					fprintf(errout, "WARNING: Don't understand: %s", s);
				denom = 1;
				sidebearing[charindex] = side;
				charnumer[charindex] = numer; 
				chardenom[charindex] = denom;
				if (traceflag != 0) 
					printf("%s %d %ld %ld\n", charname, side, numer, denom);
				if (strlen(charname) >= MAXCHARNAME) {
					fprintf(errout, "Character name %s too long (%d)\n",
							charname, strlen(charname));
				}
				else {
/*					strcpy(charnames[charindex], charname); */
/*					charnames[charindex] = strdupx(charname); */
					charnames[charindex] = zstrdup(charname);	/* 98/Dec/18 */
					if (charindex++ >= MAXCHARINFONT) {
						fprintf(errout, "Too many characters in font\n");
						return -1;
					}
					if (dotsflag != 0) putc('.', stdout);
				}
			}
			if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
		}
		if (dotsflag != 0) putc('\n', stdout);
		if (traceflag != 0) {
			printf("Entries for %d characters found\n", charindex);
		}
		return 0;		/* done with absurd format */
	}

	rewind(fp_in);	/* `Normal' format - continue here - start over on file */

/*	Next check whether given an AFM file */	/* 1997/June/30 */
/*	This adjusts only advance widths - but handles fractional widths */
	getline(fp_in, line);
	if (strncmp(line, "StartFontMetrics", 16) == 0) {
		while ((n = getline(fp_in, line)) != 0 &&
			   (strncmp(line, "StartCharMetrics", 16) != 0)) ;
		while ((n = getline(fp_in, line)) != 0 &&
			   (strncmp(line, "EndCharMetrics", 14) != 0)) {
			n = sscanf(line, "C %d ; WX %lg ; N %s ; B",
					   &chr, &dwidth, &charname);
			if (n == 3) {
				sidebearing[charindex] = 0;
				if (dwidth >= 0.0) width = (int) (dwidth + 0.4999999);
				else width = - (int) (- dwidth + 0.4999999);
				if (dwidth == (double) width) {	/* integer width, normal case */
					charnumer[charindex] = width;
					chardenom[charindex] = 1;
				}
				else {	/* fractional width, use rational approximation */
					rscale = rational(dwidth, (long) NUMLIM, (long) DENLIM);
					charnumer[charindex] = rscale.numer;
					chardenom[charindex] = rscale.denom;
				}
/*				charnames[charindex] = strdupx(charname); */
				charnames[charindex] = zstrdup(charname); 	/* 98/Dec/18 */
				if (charindex++ >= MAXCHARINFONT) {
					fprintf(errout, "Too many characters in font\n");
					return -1;
				}
				if (dotsflag != 0) putc('.', stdout);
				flag = 1;	 /* ??? */
			}
			else fprintf(errout, "Don't understand %s", line);
		}
		if (dotsflag != 0) putc('\n', stdout);
		if (traceflag != 0) {
			printf("Entries for %d characters found\n", charindex);
		}
		afmflag = 1;
		return 0;		/* done with AFM format */
	}

	rewind(fp_in);	/* `Normal' format - continue here - start over on file */

	while ((n = getline(fp_in, line)) != 0) {
		if (*line == '%' || *line == ';' || *line < ' ') continue;
		flag = 0;								/* ignore comments */
		if (strstr(line, "hsbw") == NULL &&
			strstr(line, "sbw") != NULL)
				fprintf(errout, 
					"ERROR: sidebearing and width must be horizontal\n");
/*	problem with "divide" */ /* check that "div" occurs after white space */
/*  can `div' occur for computing sidebearing ??? */
		t = strpbrk(line, " \t\n\r\f");
		if ((s = strstr(line, "div")) != NULL && s > t) {	
			denom = 1;								/* non-integer widths */
			m = sscanf(line, "%s %d %ld %ld div", 
				charname, &side, &numer, &denom);
			if (m == 4) flag = 1;
			else fprintf(errout, "Don't understand %s", line);
			if (traceflag != 0) 
				printf("%s %d %ld %ld\n", charname, side, numer, denom);
			sidebearing[charindex] = side;
			charnumer[charindex] = numer; 
			chardenom[charindex] = denom;
/*			if (numer > 0) width = (int) ((numer + denom/2) / denom); 
			else width = (int) (-(-numer + denom/2) / denom); */
		}
		else {								/* integer character width */
			dwidth = 0.0;
/*			m = sscanf(line, "%s %d %d", charname, &side, &width); */
			m = sscanf(line, "%s %d %lg", charname, &side, &dwidth);
/*			if (traceflag != 0) printf("%s %d %d\n", charname, side, width); */
			if (traceflag != 0) printf("%s %d %lg\n", charname, side, dwidth);
			sidebearing[charindex] = side;
			if (dwidth >= 0.0) width = (int) (dwidth + 0.4999999);
			else width = - (int) (- dwidth + 0.4999999);
			if (m == 3) {
				if (dwidth == (double) width) {	/* integer width, normal case */
					charnumer[charindex] = width;
					chardenom[charindex] = 1;
				}
				else {	/* fractional width, use rational approximation */
					rscale = rational(dwidth, (long) NUMLIM, (long) DENLIM);
					charnumer[charindex] = rscale.numer;
					chardenom[charindex] = rscale.denom;
				}
			}
			if (m >= 2) flag = 1;
			else fprintf(errout, "Don't understand %s", line);
		}
		if (strlen(charname) >= MAXCHARNAME) {
			fprintf(errout, "Character name %s too long (%d)\n",
					charname, strlen(charname));
			flag = 0;
		}
		else if (flag != 0) {
/*			strcpy(charnames[charindex], charname); */
/*			charnames[charindex] = strdupx(charname); */
			charnames[charindex] = zstrdup(charname); 
			if (charindex++ >= MAXCHARINFONT) {
				fprintf(errout, "Too many characters in font\n");
				return -1;
			}
			if (dotsflag != 0) putc('.', stdout);
		}
		if (bAbort != 0) abortjob();			/* 1992/Nov/24 */
	}
	if (dotsflag != 0) putc('\n', stdout);
	if (traceflag != 0) {
		printf("Entries for %d characters found\n", charindex);
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* C 33 ; WX 333 ; N exclam ; B 130 -9 238 676 ; */

int processafmfile (FILE *output, FILE *input) {
	int chr, n, k;
//	double width;
	int width, newwidth;
	int xll, yll, xur, yur, newxll, newxur;
	char charname[CHARNAME_MAX];
	char buffer[MAXLINE / 2];
	
	while (fgets(line, MAXLINE, input) != NULL) {
/*		if (*line != '\n' && *line != '%' && *line != ';') { */
			if (strncmp(line, "C ", 2) == 0) {
				if (traceflag != 0) printf (line);
				if (sscanf (line,
					"C %d ; WX %ld ; N %s ; B %d %d %d %d ;%n",
				&chr, &width, charname, &xll, &yll, &xur, &yur, &n) == 7) {
					strcpy(buffer, line+n);
//	adjust each width by specified percentage (tracking)
					if (trackflag != 0) {
						newwidth = (int) (tracking * width + 0.5);
						newxll = xll + (newwidth - width + 1)/2;
						newxur = xur + (newwidth - width + 1)/2;
						sprintf(line,
								"C %d ; WX %ld ; N %s ; B %d %d %d %d ;",
								chr, newwidth, charname,
								newxll, yll, newxur, yur);
						strcat(line, buffer);
						if (traceflag != 0) printf (line);
					}					
//  adjust each widths by fixed amounts (letterleft and letterright)
					else if (lettertrackflag) {
						newwidth = (int) width + letterleft + letterright;
						newxll = xll + letterleft;
						newxur = xur + letterleft;
						sprintf(line,
								"C %d ; WX %ld ; N %s ; B %d %d %d %d ;",
								chr, newwidth, charname,
								newxll, yll, newxur, yur);
						strcat(line, buffer);
						if (traceflag != 0) printf (line);
					}
//  adjust according to entry in specified table
					else if ((k = lookup(charname)) >= 0) {
						sprintf(line,
								"C %d ; WX %ld ; N %s ; B %d %d %d %d ;",
								chr, charnumer[k]/chardenom[k], charname,
								sidebearing[k], yll,
								(xur-xll) + sidebearing[k], yur);
						strcat(line, buffer);
						if (traceflag != 0) printf (line);
					}
					else if (traceflag) fprintf(errout, "%s? ", charname);
				}
				else fprintf(errout, "Do not understand %s", line);
			}
			fputs(line, output);
/*		} */
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	if (quietflag == 0) {
/*		printf("%s <pfa-file> [<metric file>]\n", s); */
		printf("%s [-m=<metrics file>] [-S=<extra>] <pfa-file>\n", s);
		printf("\tm file with `char lsb width'\n");
		printf("\tS added sidebearing (left and right)\n");
		printf("\n");
printf("Without -m=... or -S=..., this produces file of `char lsb width'\n");
printf("If -S=... is used twice, first is left side, second is right side\n");
	}
	exit(9);
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
/*		case 'v': verboseflag = 1; dotsflag = 0; return 0; */
		case 'v': verboseflag = 1; return 0;
		case 'b': batchmode = 1; return 0;
		case 't': traceflag = 1; return 0;
		case '?': detailflag++; return 0;
		case 'f': checkFlexProc = ! checkFlexProc; return 0;
		case 'd': wipeclean++; return 0;
		case 's': stretchflag = 1; return -1;	/* needs argument */
		case 'S': letterflag = 1; return -1;	/* needs argument */
		case 'm': metricflag = 1; return -1;	/* needs argument */
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
/*	if (n > 0 && *(name + n - 1) != '\\') strcat(new, "\\"); */
	if (n > 0 && *(new + n - 1) != '\\') strcat(new, "\\");
	strcat(new, name);		
	makename(new, ext);
/*	Try it!  See if can write to this place! */
	if ((test = fopen(new, "wb")) == NULL) {
		fprintf(errout, "WARNING: Temporary directory `%s' does not exist\n",
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

/* char *programversion = "SIDEBEAR - adjusts sidebearings & widths - version 1.0"; */

char *progfunction="adjust sidebearings & widths";

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
	int c;
	
	if (*sarg == '-' || *sarg == '/') sarg++;	/* step over `-' or `/' */
	while ((c = *sarg++) != '\0') {				/* until end of string */
		if (decodeflag(c) != 0) {				/* flag requires argument ? */
/*			if ((s = strchr(sarg, '=')) == NULL) { */
			if (*sarg != '=' && *sarg != ':') {	/* arg in same string ? */
				if (next != NULL) {
					firstarg++; s = next;	/* when `=' or `:' is NOT used */
				}
				else {
					fprintf(errout, "Don't understand: %s\n", command);
					return firstarg;
				}
			}
/*			else s++;	*/		/* when `=' IS used */
			else s = sarg+1;	/* when `=' or `:' IS used */

/* now analyze the various flags that could have gotten set */
			if (stretchflag != 0) {
				if (sscanf(s, "%lg", &tracking) < 1)
					fprintf(errout, "Don't understand %s\n", s);
				if (tracking > 10.0) tracking = tracking / 100.0;
				trackflag++;
				stretchflag = 0; 
			}
			else if (letterflag != 0) {			/* 94/Nov/12 */
				if (sscanf(s, "%d", &letterspace) < 1)
					fprintf(errout, "Don't understand %s\n", s);
				if (lettertrackflag == 0) letterleft = letterspace;
				letterright = letterspace;
				lettertrackflag++;
				letterflag = 0; 
			}
			else if (metricflag != 0) {
				metricfile = s;
				metricflag = 0; 
				if (traceflag) printf("Specified metrics file %s\n", metricfile);
			}
/*			else fprintf(errout, "Don't understand %s\n", s); */
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

int processfile(char *fn_in) {
	FILE *fp_in, *fp_out;
	int c, d, flag;
	char *s;

	if (verboseflag != 0) printf("Processing file %s\n", fn_in);

	if ((fp_in = fopen(fn_in, "rb")) == NULL) {	/* see whether exists */
		underscore(fn_in);						/* 1993/May/30 */
		if ((fp_in = fopen(fn_in, "rb")) == NULL) {/* see whether exists */
			perror(fn_in);	exit(13);
		}
	}
	c = fgetc(fp_in); d = fgetc(fp_in);
	if (c == 'S' && d == 't') {			/* is it an AFM file ? */
		afmfileflag = 1; pfafileflag = 0;
		fclose(fp_in);						/* Don't want binary mode */
	}
	else if (c == '%' || d == '!') {
		afmfileflag = 0; pfafileflag = 1;
		fclose(fp_in);
	}
	else {
		fprintf(stderr, "ERROR: This does not appear to be a PFA file\n");
		exit(13);							/* 1993/May/30 */
	}

	if ((s=strrchr(fn_in, '\\')) != NULL) s++;
	else if ((s=strrchr(fn_in, ':')) != NULL) s++;
	else s = fn_in;
	strcpy(fn_out, s);			/* copy input file name minus path */

	if (strcmp(fn_in, fn_out) == 0 && extractonly == 0) {
		strcpy(fn_bak, fn_in);
		forceexten(fn_in, "bak");
		printf("Renaming %s to %s\n", fn_bak, fn_in);
		remove(fn_in);		/* in case backup version already present */
		rename(fn_bak, fn_in);
		renameflag++;
	}

	if (verboseflag != 0 && extractonly == 0) 
		printf("Output is going to %s\n", fn_out);

	if (afmfileflag) {
		if ((fp_in = fopen(fn_in, "r")) == NULL) {
			perror(fn_in);
			exit(1);
		}
		if ((fp_out = fopen(fn_out, "w")) == NULL) {
			perror(fn_out);
			exit(1);
		}
		flag = processafmfile(fp_out, fp_in); 
		return flag;
	}

/* drop through here if it is a PFA file */

/*	(void) tmpnam(fn_dec); */	/* create temporary file name */
/*	strcpy(fn_dec, fn_out); */
/*	makename(fn_dec, "dec"); */
	maketemporary(fn_dec, fn_out, "dec");
	if (traceflag != 0) printf("Using %s as temporary\n", fn_dec);
/*	strcpy(fn_pln, fn_out); */
/*	makename(fn_pln, "pln"); */
	maketemporary(fn_pln, fn_out, "pln");
	if (traceflag != 0) printf("Using %s as temporary\n", fn_pln);
	if (extractonly == 0) {
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

	seenFlexProc=0;
	if (extractonly != 0) {		/* no need to go upward */
		if ((fp_pln = fopen(fn_pln, "rb")) == NULL) {
			perror(fn_pln); cleanup(); exit(33);
		}
		forceexten(fn_out, "sid");
		if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
			perror(fn_out); cleanup();	exit(2);
		}

		(void) extractside(fp_out, fp_pln, 1);	/* extract side bearings */

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

	if (traceflag != 0) printf("Adjustment of sidebearing starting\n"); 

	if ((fp_pln = fopen(fn_pln, "rb")) == NULL) { 
		perror(fn_pln);  cleanup(); exit(2);
	}
	if ((fp_adj = fopen(fn_adj, "wb")) == NULL) { 
		perror(fn_adj); cleanup(); exit(2);
	}

	(void) extractside(fp_adj, fp_pln, 0);

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

	if (seenFlexProc && verboseflag) printf("WARNING: font uses FlexProc\n");

	return 0;
}

int _cdecl main(int argc, char *argv[]) {       /* main program */
    FILE *fp_in;
/*	FILE *fp_out; */
/*	unsigned int i; */
	int firstarg=1;
	int m;
/*	int c, d, flag; */
	char *s; 

	if (strlen(progname) < 16) strcpy(progname, compilefile);
	if ((s = strchr(progname, '.')) != NULL) *s = '\0';
	uppercase(progname);

/*	see whether EXE has changed name since it was compiled */
/*	That is whether it is being used with MathTime fonts batch file */
	strcpy (line, argv[0]);
	uppercase (line);
	if (strstr(line, progname) == NULL) quietflag = 1;

	if (argc < firstarg+1) showusage(argv[0]); 

	firstarg = commandline(argc, argv, 1);

	if (wipeclean == 4) wipeclean = 0;	/* `ddd' on command line */

	if (argc <= firstarg) showusage(argv[0]); 
		
	if (quietflag != 0) {
		verboseflag = 0; traceflag = 0; dotsflag = 0; 
		tracking=1.0;
	}

/*	scivilize(compiledate);	 */
	if (quietflag == 0) stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	(void) signal(SIGINT, ctrlbreak);

	if ((s = getenv("TMP")) != NULL) tempdir = s;
	if ((s = getenv("TEMP")) != NULL) tempdir = s;

/*	printf( "Sidebearing and advance width modification program version %s\n",
		VERSION); */
	
	if (checkcopyright(copyright) != 0) {
/*		fprintf(stderr, "HASH %ld", checkcopyright(copyright)); */
		exit(1);	/* check for tampering */
	}

	if (verboseflag) {
		if (trackflag) 
			printf("Adjusting advance widths by %lg\n", tracking);
		else if (lettertrackflag) 
		printf("Adjusting left sidebearing by %d & right sidebearing by %d\n",
			letterleft, letterright);
	}	

/*	stringindex=0; */		/* reset string pool table */
/*	for (k = 0; k < MAXCHARINFONT; k++)	charnames[k] = NULL; */ /* ? */

	extractonly = 0;		/* default is not to just extract old info */

/*	if (strcmp(metricfile, "") == 0) { */
	if (strcmp(metricfile, "") == 0 && batchmode == 0) {
/*		following is for back-ward compatability 1994/June/2 */
		if (argc > firstarg + 1) {
			metricfile = argv[firstarg+1];	/* second arg is metrics file */
			if (verboseflag)
				printf("Assuming that %s is metrics file\n",
					  metricfile);
/*			firstarg++; */
		}
	}

	if (strcmp(metricfile, "") != 0) {	/* metrics file specified */
/*		m = firstarg; */
/*		strcpy(fn_in, argv[m]); */	/* get file name */
		strcpy(fn_in, metricfile);	/* get file name */
		extension(fn_in, "sid");			/* extension is not given */
		if (verboseflag != 0) printf("Reading file %s\n", fn_in);
		if ((fp_in = fopen(fn_in, "r")) == NULL) {	/* see whether exists */
			underscore(fn_in);						/* 1993/May/30 */
			if ((fp_in = fopen(fn_in, "r")) == NULL) {/* see whether exists */
				perror(fn_in);	exit(13);
			}
		}
		if (readsides (fp_in) != 0) {	/* read new side-bearing and widths */
			fprintf(stderr, "Bad Metrics File %s\n", fn_in);
			fclose(fp_in);
			exit(1);
		}
		fclose(fp_in);
	}
	else if (trackflag == 0 && lettertrackflag == 0) extractonly = 1;

	if (bAbort != 0) abortjob();			/* 1992/Nov/24 */

	if (batchmode == 0) {
		m = firstarg;
		strcpy(fn_in, argv[m]);			/* get file name */
		extension(fn_in, "pfa");
		processfile(fn_in);
		cleanup();
	}
	else {
		for (m = firstarg; m < argc; m++) {
			strcpy(fn_in, argv[m]);			/* get file name */
			extension(fn_in, "pfa");
			processfile(fn_in);
			cleanup();
			if (bAbort != 0) abortjob();
		}
	}
	return 0;
}	

/* does not deal with `sbw' form of metric info */

/* does not deal properly with `fractional widths' */
