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

/* Program for inserting composite character information in AFM files */

/* The 58 `standard' composite characters are wired in */

/* With `lslash' and `Lslash' would make for a total of 60 composites */
/* But the last two will not work, because `suppress' not in SE */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>					/* only for tan */

#define MAXFILENAME 128

#define MAXLINE 256

#define NUMCHRS 256

#define MAXCHARLEN 32

int verboseflag = 0;
int traceflag = 0;
int keepcomments = 1;		/* keep comment lines in AFM file */
int simpleyposition = 1;	/* use simple positioning of base line accents */
int westernflag = 1;		/* use Western composite character set */
int easternflag = 1;		/* use Eastern composite character set */
int bothflag = 0;			/* use both composite character sets */

int liftthreshold = 25;		/* how much higher lc char can be than xheight */

/* int includelslash = 0; *//* include lslash & Lslash */

double xheight, capheight, italicangle;

int uclift;					/* dy for uc characters */

int lclift;					/* dy for lc characters */

int lcfixlift=0;			/* non-zero if uc lift specified */

int ucfixlift=0;			/* non-zero if lc lift specified */

double kernl, kernL;		/* kerning for Polish characters */

double fontxll, fontyll, fontxur, fontyur;

#define PI 3.141529653

double slant;			/* tan (- italicangle) */

int islant;				/* (int) (1000 * slant) */	/* not accessed */

char line[MAXLINE];

char charnames[NUMCHRS][MAXCHARLEN];

double charwidths[NUMCHRS];

double charxlls[NUMCHRS], charylls[NUMCHRS];
double charxurs[NUMCHRS], charyurs[NUMCHRS];

int nchars;					/* number of entries in old charmetrics section */

int nkerns;					/* number of kern pairs actually found */

#define MAXCOMPOSITES 128

int ncomposites=58; 		/* number of entries in following table */

char *task="";

/* The following Western European accents occupy 58 slots */

char *composites[MAXCOMPOSITES];

char *westerncomposites[]= {
"Aacute", "Acircumflex", "Adieresis", "Agrave", "Aring", "Atilde", 
"Ccedilla", 
"Eacute", "Ecircumflex", "Edieresis", "Egrave", 
"Iacute", "Icircumflex", "Idieresis", "Igrave", 
"Ntilde", 
"Oacute", "Ocircumflex", "Odieresis", "Ograve", "Otilde", 
"Scaron", 
"Uacute", "Ucircumflex", "Udieresis", "Ugrave", 
"Yacute", "Ydieresis", 
"Zcaron", 
"aacute", "acircumflex", "adieresis", "agrave", "aring", "atilde", 
"ccedilla", 
"eacute", "ecircumflex", "edieresis", "egrave", 
"iacute", "icircumflex", "idieresis", "igrave", 
"ntilde", 
"oacute", "ocircumflex", "odieresis", "ograve", "otilde", 
"scaron", 
"uacute", "ucircumflex", "udieresis", "ugrave", 
"yacute", "ydieresis", 
"zcaron",
/* "lslash", "Lslash",  */
""};

/* can't do the aogonek and eogonek, because CM has no `ogonek' */
/* also may be problem with positioning of ogonek and leg of A */
/* can't do Dmacron, dmacron since `macron' has to cross d in low down */
/* can't do dcaron, lcaron, tcaron - need apostrophe, not caron */
/* maybe can't do scedilla, tcedilla - need commaaccent, not cedilla ? */
/* also may be problem with positioning of cedilla and leg of T */
/* hungarumlaut = dblaccent = hungar = umlaut */
/* dot = dotaccent */ /* cedilla = commaaccent */

/* There are presently 29 + 28 = 57 of these */

/* char *extracompo[] = { */
char *easterncomposites[] = {
"Abreve", 				/* Aogonek --- missing ogonek */
"Cacute", "Ccaron",
"Dcaron", 				/* Dmacron --- not macron accent */
"Ecaron",				/* Eogonek --- missing ogonek */
"Gbreve",
"Idotaccent",
"Lacute",				/* Lcaron --- lapostrophe */
						/* "Ldotaccent" --- requires special dot position */
						/* also, does not occur in Cork or ISO Latin X */
"Nacute", "Ncaron",	
"Ohungarumlaut",
"Racute", "Rcaron",
"Sacute",				/* "Scaron" ---  duplication */
"Scedilla",
"Tcaron",
"Tcedilla",				/* Tcedilla ? */
"Uhungarumlaut", "Uring",
						/* "Yacute" --- duplication */
"Zacute",				/* "Zcaron" --- duplication */
"Zdotaccent",

"abreve",				/* "aogonek --- missing ogonek */
"cacute", "ccaron",
						/* "dcaron" --- dapostrophe */
						/* "dmacron" --- not simple macron accent */
"ecaron",				/* "eogonek" --- missing ogonek accent */
"gbreve",  
/* "idotaccent", ha ha */
"lacute",				/* "lcaron" --- lapostrophe */
						/* "ldotaccent" --- requires special dot position */
						/* also, does not occur in Cork or ISO Latin X */
"nacute", "ncaron",
"ohungarumlaut",
"racute", "rcaron",
"sacute",				/* "scaron" --- duplication */
"scedilla",
						/* "tcaron" --- tapostrophe */
"tcedilla",				/* "tcedilla" ? */
"uhungarumlaut", "uring",
						/* "yacute" --- duplication */
"zacute",				/* "zcaron" --- duplication */
"zdotaccent",
""};

int iscomposite(char *name) {	/* returns 0 if not composite */
	int k;
/*	for (k = 0; k < ncomposites; k++) { */
	for (k = 0; k < MAXCOMPOSITES; k++) {
		if (strcmp(composites[k], "") == 0) break;			/* 93/Mar/20 */
		if (strcmp(composites[k], name) == 0) return -1;
	}
	return 0;
}

int lookup(char *name) {
	int k;
	for (k = 0; k < NUMCHRS; k++) {
		if (strcmp(charnames[k], name) == 0) return k;
	}
	return -1;
}

int roundit (double x) {
	if (x == 0.0) return 0;
	else if (x > 0.0) return (int) (x + 0.5);
	else return - (int) (-x + 0.5);
}

/* Generate the composite characters in the CompositeMetrics section */
/* (a lot of duplication w.r.t. to below */

int generatecomp(FILE *output, int flag) {
	int k, c, n=0;
	int neededlift = 0;
	int maxlift = 0;
	int overlapflag = 0;
	int maxoverlap = 0;
	int overlap;
	int bchar, achar, dx, dy;
	double bwidth, awidth;
	double bheight, aheight;
	char basechar[MAXCHARLEN], accentchar[MAXCHARLEN];

/*	for (k=0; k < ncomposites; k++) { */
	for (k=0; k < MAXCOMPOSITES; k++) {
		if (strcmp(composites[k], "") == 0) break;			/* 93/Mar/20 */
		strncpy(basechar, composites[k], 1); basechar[1] = '\0';
		if (strcmp(basechar, "i") == 0) strcpy(basechar, "dotlessi");
		strcpy(accentchar, composites[k]+1);
/*		if (strcmp(accentchar, "slash") == 0) strcpy(accentchar, "suppress"); */
		if (strcmp(accentchar, "dot") == 0) strcpy(accentchar, "dotaccent");
		if (strcmp(accentchar, "umlaut") == 0) strcpy(accentchar, "hungarumlaut");
		if (strcmp(accentchar, "hungar") == 0) strcpy(accentchar, "hungarumlaut");
		bchar = lookup(basechar);
		if (bchar < 0) {
/*			if (flag == 0) fprintf(stderr, "%s?  ", basechar); */
			continue;
		}
		achar = lookup(accentchar);
		if (achar < 0) {
			if (strcmp(accentchar, "circumflex") == 0) {
				strcpy(accentchar, "asciicircum");
				achar = lookup(accentchar);
			}
			else if (strcmp(accentchar, "tilde") == 0) {
				strcpy(accentchar, "asciitilde");
				achar = lookup(accentchar);
			}
			else if (strcmp(accentchar, "suppress") == 0) {
				strcpy(accentchar, "hyphen");
				achar = lookup(accentchar);
			}
			if (achar < 0) {
/*				if (flag == 0) fprintf(stderr, "%s?  ", accentchar); */
				continue;
			}
		}
		bwidth = charwidths[bchar];
/*		if (bwidth < -1400.0 || bwidth > 1400.0)
			fprintf(stderr, "Width %lg for char %s?  ", bwidth, basechar); */
		awidth = charwidths[achar];
/*		if (awidth < -1400.0 || awidth > 1400.0)
			fprintf(stderr, "Width %lg for char %s?  ", awidth, accentchar); */
/*		dx = ((int) bwidth - awidth + 1) / 2; */
		dx = roundit((bwidth - awidth) / 2.0);
		bheight = charyurs[bchar];			/* top of base 96/Aug/3 */
		aheight = charylls[achar];			/* bottom of accent 96/Aug/3 */
		c = basechar[0];
		dy = 0;
/*		baseline `accents' first */		
		if (strcmp(accentchar, "cedilla") == 0 ||
			strcmp(accentchar, "ogonek") == 0) {
/*			if (c >= 'a' && c <= 'z') dy = +10; */
			dy += roundit(charylls[bchar] - charyurs[achar] + 1);
/* get accent to just touch character */
			if (dy < 0) dy = dy / 2;	/* totally ad hoc ! cmtt* mostly */
/*			if (dy < 0) fprintf(stderr, 
				"dy % d base.yll %lg accent.yur %lg\n", 
					dy, charylls[bchar], charyurs[achar]); */
			if (simpleyposition != 0) {
				if (charylls[bchar] > charyurs[achar])	/* avoid gap */
					dy = roundit(charylls[bchar] - charyurs[achar]);
				else dy = 0;
			}
		}
		else {
/* adjust normal accent height for upper case letters */
			if (c >= 'A' && c <= 'Z') {
/*				dy = (int) (capheight - xheight + 0.5); */
/*				dy = roundit (capheight - xheight); */
				dy = uclift;
			}
/* for lower case use dy = 0 ---- set above */			
			else if (c >= 'a' && c <= 'z') {
				dy = lclift;
				if (bheight - xheight > liftthreshold && lcfixlift == 0) {
					dy = roundit (bheight - xheight);
/*	only complain if its low lower case character ... */
					if (strchr("bdfhklt", c) == NULL) {
						if (verboseflag)
						printf("Lift of %d needed for %s and %s\n",
							   dy, charnames[bchar], charnames[achar]);
						neededlift++;
						if (dy > maxlift) maxlift = dy;
					}
					else {	/* it is a `tall' lower case character */
						dy = uclift;
/*	uclift is only appropriate if tall character has capheight */
						if (bheight - capheight > liftthreshold) {
							dy = roundit (bheight - capheight);
						}
					}
				}
			}
/*			if (charylls[achar] + dy <= charyurs[bchar]) { */
			if (charylls[achar] + dy <= charyurs[bchar] + liftthreshold) {
				overlap = roundit (charyurs[bchar] - (charylls[achar] + dy));
				fprintf(stderr,
						"Accent %s (%d) overlaps base %s (%d) by %d\n",
						charnames[achar], achar, charnames[bchar], bchar,
						overlap);
				overlapflag++;
				dy += overlap + liftthreshold;
/*				overlap = charyurs[bchar] - charylls[achar] - dy; */
				if (overlap > maxoverlap) maxoverlap = overlap;
			}
		}

#ifdef IGNORED
/*		special case for Polish characters */
		if (c == 'l' && strcmp(accentchar, "suppress") == 0) {
			if (kernl == 0.0) fprintf(stderr, "No KPX suppress l (C) \n");
			dx = - roundit (kernl + awidth);
			if (dx != 0) 
				fprintf(stderr, "lslash DX not zero (%lg + %lg => %d)\n", 
					kernl, awidth, dx);
			if (kernl == 0.0) fprintf(stderr, "No KPX suppress l (D) \n");
			dy = 0;
		}
		else if (c == 'L' && strcmp(accentchar, "suppress") == 0) {
			dx = - roundit (kernL + awidth);
			dy = 0;
		}
#endif

		if (dx < 0 && traceflag != 0) { /* dx < 0 if accent wider than base */
			fprintf(stderr, "DX (%d) < 0 (%s) ", dx, composites[k]);
			fprintf(stderr, "basewidth %lg accentwidth %lg\n",
				bwidth, awidth);
		}
		if (dy < 0 && strcmp(accentchar, "cedilla") != 0) {
			fprintf(stderr, "DY (%d) < 0 (%s) ", dy, composites[k]);
			fprintf(stderr, "CapHeight %lg Xheight %lg\n",
				capheight, xheight);
		}
/*		adjust for slant if needed */
		if (slant != 0.0 && dy != 0)
/*			dx += (int) ((slant * dy) + 0.5); */
			dx += roundit (slant * dy);
		n++;
		if (flag > 0) 
			fprintf(output, "CC %s 2 ; PCC %s 0 0 ; PCC %s %d %d ;\n",
				composites[k], basechar, accentchar, dx, dy);
	}
	if (neededlift) {
		fprintf(stderr,
				"WARNING: %d lower case chars needed accent lift (as much as %d)\n",
				neededlift, maxlift);
		fprintf(stderr, "Maybe use -l=... to account for xheight differences\n");
	}
	if (overlapflag) {
		fprintf(stderr,
				"WARNING: %d accents appeared to overlap base (as much as %d)\n", 
			   overlapflag, maxoverlap);
		fprintf(stderr, "Maybe due to TFMtoAFM producing yll = 0 for accent\n");
	}
	return n;
}

/* Generate the composite metrics entries in CharMetrics section */
/* (a lot of duplication w.r.t. to the above */

int addcomposites(FILE *output, int flag) {
	int k, c, n=0;
	int bchar, achar, dx, dy;
	double xll, yll, xur, yur;
	double bwidth, awidth;
	char basechar[MAXCHARLEN], accentchar[MAXCHARLEN];

	if (traceflag) printf("Now adding Composites\n");

/*	for (k=0; k < ncomposites; k++) { */
	for (k=0; k < MAXCOMPOSITES; k++) {
		if (strcmp(composites[k], "") == 0) break;			/* 93/Mar/20 */
		strncpy(basechar, composites[k], 1); basechar[1] = '\0';
		if (strcmp(basechar, "i") == 0) strcpy(basechar, "dotlessi");
		strcpy(accentchar, composites[k]+1);
/*		if (strcmp(accentchar, "slash") == 0) strcpy(accentchar, "suppress"); */
		if (strcmp(accentchar, "dot") == 0) strcpy(accentchar, "dotaccent");
		if (strcmp(accentchar, "umlaut") == 0) strcpy(accentchar, "hungarumlaut");
		if (strcmp(accentchar, "hungar") == 0) strcpy(accentchar, "hungarumlaut");
		bchar = lookup(basechar);
		if (bchar < 0) {
			if (flag == 0) fprintf(stderr, "%s?  ", basechar);
			continue;
		}
		achar = lookup(accentchar);
		if (achar < 0) {
			if (strcmp(accentchar, "circumflex") == 0) {
				strcpy(accentchar, "asciicircum");
				achar = lookup(accentchar);
			}
			else if (strcmp(accentchar, "tilde") == 0) {
				strcpy(accentchar, "asciitilde");
				achar = lookup(accentchar);
			}
			else if (strcmp(accentchar, "suppress") == 0) {
				strcpy(accentchar, "hyphen");
				achar = lookup(accentchar);
			}
			if (achar < 0) {
				if (flag == 0) fprintf(stderr, "%s?  ", accentchar);
				continue;
			}
		}
		bwidth = charwidths[bchar];
		if (bwidth < -1400.0 || bwidth > 1400.0)
			fprintf(stderr, "Width %lg for char %s?  ", bwidth, basechar);
		awidth = charwidths[achar];
		if (awidth < -1400.0 || awidth > 1400.0)
			fprintf(stderr, "Width %lg for char %s?  ", awidth, accentchar);
/*		dx = ((int) bwidth - awidth + 1) / 2; */
		dx = roundit(bwidth - awidth / 2.0);
		c = basechar[0];
/*		adjust for height */
		dy = 0;
/*		baseline `accents' first */
		if (strcmp(accentchar, "cedilla") == 0 ||
			strcmp(accentchar, "ogonek") == 0) {
/*		special hack to bring cedilla up a whee bit : CM font specific */
/*		only for lower case letters */
/*			if (c >= 'a' && c <= 'z') dy = +10; */
			dy += roundit(charylls[bchar] - charyurs[achar] + 1);
/* get accent to just touch character */
			if (dy < 0) dy = dy / 2;	/* totally ad hoc ! cmtt* mostly */
		} /* adjust normal accent height for upper case letters */
		else {
			if (c >= 'A' && c <= 'Z') 
/*				dy = (int) (capheight - xheight + 0.5); */
/*				dy = roundit (capheight - xheight); */
				dy = uclift;
		}

#ifdef IGNORED
/*		special case for polish characters */
		if (c == 'l' && strcmp(accentchar, "suppress") == 0) {
			if (flag != 0) {
				if (kernl == 0.0) 
					fprintf(stderr, "No KPX suppress l (F)\n");
			}
			dx = - roundit (kernl + awidth);
			if (flag != 0) {
				if (dx != 0) 
					fprintf(stderr, "lslash DX not zero (%lg + %lg => %d)\n", 
						kernl, awidth, dx);
			}
			else dx = 0;			/* first pass assume comes out OK */
			if (flag != 0 && kernl == 0.0) 
				fprintf(stderr, "No KPX suppress l (G)\n");
			dy = 0;
		}
		else if (c == 'L' && strcmp(accentchar, "suppress") == 0) {
			dx = - roundit (kernL + awidth);
			dy = 0;
		}
#endif

/*		adjust for slant if needed */
		if (slant != 0.0 && dy != 0) 
/*			dx += (int) ((slant * dy) + 0.5); */
			dx += roundit(slant * dy);

/*		BBox of base character */
		xll = charxlls[bchar]; yll = charylls[bchar];
		xur = charxurs[bchar]; yur = charyurs[bchar];
/*		BBox of accent character */
		if (charxlls[achar] + dx < xll) xll = charxlls[achar] + dx; 
		if (charylls[achar] + dy < yll) yll = charylls[achar] + dy;
		if (charxurs[achar] + dx > xur) xur = charxurs[achar] + dx; 
		if (charyurs[achar] + dy > yur) yur = charyurs[achar] + dy;
		if (flag > 0)
			fprintf(output, "C -1 ; WX %lg ; N %s ; B %lg %lg %lg %lg ;\n",
				bwidth, composites[k], xll, yll, xur, yur);
/*		adjust FontBBox */
		if (xll < fontxll) fontxll = xll;
		if (xur > fontxur) fontxur = xur;
		if (yll < fontyll) fontyll = yll;
		if (yur > fontyur) fontyur = yur;
		n++;
	}
	return n;
}

/* AFM file is read twice - flag = 0 first time, flag = 1 second time */
/* no output is generated in the first pass */

int readafmfile (FILE *input, FILE *output, int flag) {
	int charnum, k, n;
	double charwid, charxll, charyll, charxur, charyur, kerning;
	char charnam[MAXCHARLEN], lchar[MAXCHARLEN], rchar[MAXCHARLEN];
	int seenbbox, terminate=0;

	if (traceflag) printf("Now reading AFM file ---- flag %d\n", flag);

	if (flag == 0) {
		capheight = xheight = 0.0; italicangle = 360.0;
		fontxll = 0.0;  fontyll = 0.0; fontxur = 0.0; fontyur = 0.0;
/*		fprintf(stderr, "Resetting kernl and kernL\n"); */
		kernl = kernL = 0.0;
	}
	seenbbox = 0;

	for (k = 0; k < NUMCHRS; k++) {
		strcpy(charnames[k], "");
		charwidths[k] = 0.0;
	}

	task = "Looking for StartCharMetrics";
	for(;;) {	/* read up to StartCharMetrics (character information) */
		if (fgets(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "Premature EOF %s\n", task);
			return -1;
		}
		if (*line == '%') {
			if (flag != 0 && keepcomments != 0) fputs(line, output);
			continue;			/* comment line ??? */
		}

		if (strncmp(line, "StartCharMetrics", 16) == 0) break;
		if (strncmp(line, "CapHeight", 9) == 0) {
			if(sscanf(line, "CapHeight %lg", &capheight) < 1)
				fprintf(stderr, "Don't understand capheight: %s", line);
		}
		else if (strncmp(line, "XHeight", 7) == 0) {
			if(sscanf(line, "XHeight %lg", &xheight) < 1)
				fprintf(stderr, "Don't understand xheight: %s", line);
		}
		else if (strncmp(line, "ItalicAngle", 11) == 0){
			if(sscanf(line, "ItalicAngle %lg", &italicangle) < 1)
				fprintf(stderr, "Don't understand italicangle: %s", line);
		}
		else if (strncmp(line, "FontBBox", 8) == 0){
/*	replace FontBBox line */
			if (flag > 0) sprintf(line, "FontBBox %lg %lg %lg %lg\n", 
				fontxll, fontyll, fontxur, fontyur);				
			seenbbox++;
		}
		if (flag > 0) fputs(line, output);
	}

	if (flag == 0) sscanf(line, "StartCharMetrics %d", &nchars);

	if (seenbbox == 0) {
		fprintf(stderr, "Didn't find FontBBox\n");
		if (flag > 0) fprintf(output, "FontBBox %lg %lg %lg %lg\n", 
			fontxll, fontyll, fontxur, fontyur);				
	}

	if (capheight == 0.0 || xheight == 0.0) {
		fprintf(stderr, "Didn't find CapHeight (%lg)  or XHeight (%lg)\n",
			capheight, xheight);
		return -1;
	}
	if (capheight < xheight) {
		fprintf(stderr, "CapHeight (%lg) < XHeight (%lg)\n",
			capheight, xheight);
		return -1;
	}
	if (ucfixlift == 0) uclift = roundit (capheight - xheight);
	if (lcfixlift == 0) lclift = 0;
	if (verboseflag != 0 && flag == 0) 
		printf("Capheight - XHeight = %d\n", uclift);
	if (italicangle == 360.0) {
		fprintf(stderr, "Didn't find ItalicAngle - assuming 0.0\n");
		italicangle = 0.0;  /* return -1; */
	}
	slant = tan(- italicangle * PI / 180.0);
	islant = (int) (1000.0 * slant + 0.5);
	if (slant != 0.0 && flag == 0) {
		if (verboseflag != 0) printf("Using slant of %lg\n", slant);
/*		if (verboseflag != 0) printf("Using slant of %ld/1000\n", islant); */
	}

	if (flag > 0) 
		fprintf(output, "StartCharMetrics %d\n", nchars + ncomposites);

	if (traceflag) printf("Now reading Char Metrics\n");

	nchars = 0;		/* do an honest count ! don't believe the file */

	task = "looking for EndCharMetrics";
	for(;;) {		/* reading up to EndCharMetrics */
		if (fgets(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "Premature EOF %s\n", task);
			return -1;
		}
		if (*line == '%') {
			if (flag != 0 && keepcomments != 0) fputs(line, output);
			continue;			/* comment line ??? */
		}
		if (strncmp(line, "EndCharMetrics", 14) == 0) break;
/*		if (flag > 0) fputs(line, output); */
		if (sscanf(line, "C %d ; WX %lg ; N %s ; B %lg %lg %lg %lg", 
			&charnum, &charwid, charnam, 
				&charxll, &charyll, &charxur, &charyur) < 7)
			fprintf(stderr, "Don't understand charmetric: %s", line);
		else if (charnum >= 0 && charnum < NUMCHRS) {
			charwidths[charnum] = charwid;
			charxlls[charnum] = charxll; charylls[charnum] = charyll;
			charxurs[charnum] = charxur; charyurs[charnum] = charyur;
			strcpy(charnames[charnum], charnam);
			if (charyur > fontyur) fontyur = charyur;
			if (charxur > fontxur) fontxur = charxur;
			if (charyll < fontyll) fontyll = charyll;
			if (charxll < fontxll) fontxll = charxll;
			if (traceflag)
				printf("C %d ; WX %lg ; N %s ; B %lg %lg %lg %lg ;\n",
					charnum, charwid, charnam,
						charxll, charyll, charxur, charyur);
		}
		if (charnum != -1 || iscomposite(charnam) == 0) {
			if (flag > 0) fputs(line, output);
			nchars++;
		}
	}

	n = addcomposites(output, flag);
	if (flag > 0) 
		fprintf(output, "EndCharMetrics\n");  /* EndCharMetrics */

	if (n == 0) 
		fprintf(stderr, "Added no lines composites\n");
	else if (n != ncomposites) 
		fprintf(stderr, "Only added %d lines for composites\n", n);

	task = "looking for StartKernPairs, StartComposites or EndFontMetrics";
	for(;;) {	/*	search for kerning table */
		if (fgets(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "Premature EOF %s\n", task);
			return -1;
		}
		if (*line == '%') {
			if (flag != 0 && keepcomments != 0) fputs(line, output);
			continue;			/* comment line ??? */
		}
		if (strncmp(line, "StartKernPairs", 14) == 0 ) break;
		if (strncmp(line, "EndFontMetrics", 14) == 0 ) {
			terminate = 1;
			break;
		}
		if (strncmp(line, "StartComposites", 15) == 0) {
			if (flag == 0)
				fprintf(stderr, 
					"WARNING: AFM file already contains composites\n");
			task = "looking for EndComposites";
			for (;;) {
				if(fgets(line, MAXLINE, input) == NULL) {
					fprintf(stderr, "Premature EOF %s\n", task);
					return -1;
				}
				if (strncmp(line, "EndComposites", 13) == 0) break;
			}				
			continue;	/* ??? */
		}
		if (flag > 0) fputs(line, output);
	}

	if (terminate != 0) goto finish;

	if (flag == 0) sscanf(line, "StartKernPairs %d", &nkerns);

/*	if (flag > 0) fputs(line, output); */
	if (flag > 0) fprintf(output, "StartKernPairs %d\n", nkerns);

	nkerns = 0;			/*  get real count, don't trust file */

	task = "looking for EndKernPairs";
	for (;;) { /*	now read kern pairs - search for special ones */
		if(fgets(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "Premature EOF %s\n", task);
			return -1;
		}
		if (*line == '%') {
			if (flag != 0 && keepcomments != 0) fputs(line, output);
			continue;			/* comment line ??? */
		}
		if (strncmp(line, "EndKernPairs", 12) == 0 ) break;
		if (strncmp(line, "EndFontMetrics", 14) == 0 ) {
			terminate = 1;
			break;
		}
		if (flag > 0) fputs(line, output);
		if (sscanf(line, "KPX %s %s %lg", lchar, rchar, &kerning) < 3)
			fprintf(stderr, "Don't understand: %s", line);
		else {
			if (strcmp(lchar, "suppress") == 0) {
				if (strcmp(rchar, "l") == 0) kernl = kerning;
				else if (strcmp(rchar, "L") == 0) kernL = kerning;
/*				fprintf(stderr, "Setting kern%s\n", rchar); */
			}
		}
		nkerns++;
	}

#ifdef IGNORED
	if (kernl == 0.0 && includelslash != 0)
		 fprintf(stderr, "No KPX suppress l (I)\n");
	if (kernL == 0.0 && includelslash != 0) 
		fprintf(stderr, "No KPX suppress L (I)\n");
#endif

	if (terminate != 0) goto finish;
	if (flag > 0) fputs(line, output);

	task = "Looking for EndFontMetrics";
	for(;;) {	/* continue to end of file */
		if(fgets(line, MAXLINE, input) == NULL) {
			fprintf(stderr, "Premature EOF %s\n", task);
			return -1;
		}
		if (*line == '%') {
			if (flag != 0 && keepcomments != 0) fputs(line, output);
			continue;			/* comment line ??? */
		}
		if (strncmp(line, "EndFontMetrics", 14) == 0 )break;
		if (strncmp(line, "StartComposites", 15) == 0) {
			if (flag == 0)
				fprintf(stderr, 
					"WARNING: AFM file already contains composites\n");
			task = "looking for EndComposites";
			for(;;) {
				if(fgets(line, MAXLINE, input) == NULL) {
					fprintf(stderr, "Premature EOF %s\n", task);
					return -1;
				}
				if (strncmp(line, "EndComposites", 13) == 0) break;
			}				
			continue; /* ??? */
		}
		if (flag > 0) fputs(line, output);
	}

finish:
	if (flag > 0) fprintf(output, "StartComposites %d\n", ncomposites);
	n = generatecomp(output, flag);				/* insert composites */
	if (flag > 0) fprintf(output, "EndComposites\n");

	if (n == 0) {
		fprintf(stderr, "Failed to create any composites\n");
		return -1;
	}
	else if (n != ncomposites) {
		fprintf(stderr, "Only generated %d composites\n", n);
/*		return -1; */
	}
	if (flag > 0) fprintf(output, "EndFontMetrics\n");
	return 0;			
}

int fillcomposites (void) {
	int k, m, count, ncomposites=0;

	if (westernflag) {
		count = 0;
		for(m=0; m < MAXCOMPOSITES; m++) {	/* Western composites */
			composites[ncomposites+m] = westerncomposites[m];	
			if (strcmp(westerncomposites[m], "") == 0) break;
			count++;
		}
		ncomposites += count;
	}
	if (easternflag) {
		count = 0;
		for(m=0; m < MAXCOMPOSITES; m++) {	/* Eastern composites */
			composites[ncomposites+m] = easterncomposites[m];	
			if (strcmp(easterncomposites[m], "") == 0) break;
			count++;
		}
		ncomposites += count;
	}
	for (m = 1; m < ncomposites; m++) {
		for (k = 0; k < m; k++) {
			if (strcmp(composites[k], composites[m]) == 0) {
				fprintf(stderr, "ERROR: %s appears twice (%d and %d)\n",
						charnames[k], k, m);
			}
		}
	}
	return ncomposites;
}

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);	
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

char *stripname(char *name) {
	char *s;
	if ((s = strrchr(name, '\\')) != NULL) return (s+1);
	if ((s = strchr(name, ':')) != NULL) return (s+1);
	else return name;
}

void showusage (char *s) {
	printf("\t-v\tverbose mode\n");
	printf("\t-t\ttrace mode\n");
	printf("\t-s\tdo not keep comment lines in AFM file\n");
	printf("\t-w\tuse Western European composite set only\n");
	printf("\t-e\tuse Eastern European composite set only\n");
	printf("\t\t(default is to use both composite sets)\n");
	printf("\t-p\tdo not use simple positioning (dy=0) of cedilla and ogonek\n");
/*	printf("\t-l\tinclude Lslash and lslash\n"); */
	printf("\t-u=...\tuc lift to use instead of capheight - xheight\n");
	printf("\t-l=...\tlc lift to use instead of 0 (may need for smallcaps)\n");
	exit(1);
}

int main(int argc, char *argv[]) {       /* main program */
	FILE *input, *output;
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	char bakfilename[MAXFILENAME];
	int m, flag;
	int firstarg=1;
	char *s;
/*	int count; */

	if (argc < firstarg +1) showusage(argv[0]);
	
/*	while (*argv[firstarg] == '-')  */
	while (firstarg < argc && *argv[firstarg] == '-') {
		s = argv[firstarg];
		if (strchr(s, 'v') != NULL) verboseflag = (1 - verboseflag);
		else if (strchr(s, 't') != NULL) traceflag = (1 - traceflag);
		else if (strchr(s, 's') != NULL) keepcomments = (1 - keepcomments);
/*		if (strchr(s, 'l') != NULL) includelslash = (1 - includelslash); */
		else if (strchr(s, 'w') != NULL) {
			easternflag = 0;		westernflag = 1;
		}
		else if (strchr(s, 'e') != NULL) {
			easternflag = 1;		westernflag = 0;
		}
		else if (strchr(s, 'p') != NULL) simpleyposition = (1 - simpleyposition);
		else if (strchr(s, 'u') != NULL) {
			if (sscanf(s, "-u=%d", &uclift) > 0) ucfixlift++;
			else fprintf(stderr, "Don't understand %s\n", s);
		}
		else if (strchr(s, 'l') != NULL) {
			if (sscanf(s, "-l=%d", &lclift) > 0) lcfixlift++;
			else fprintf(stderr, "Don't understand %s\n", s);
		}
		else fprintf(stderr, "Don't understand %s\n", s);
		firstarg++;
	}

	if (argc < firstarg +1) showusage(argv[0]);
	
	ncomposites = fillcomposites();

	if (verboseflag != 0) printf("There are %d composites\n", ncomposites);

	for (m = firstarg; m < argc; m++) {
		strcpy(infilename, argv[m]);
		if (strstr(infilename, ".bak") != NULL) {
			printf("Ignoring %s\n", infilename);
			continue;
		}
		extension(infilename, "afm");
		strcpy(outfilename, stripname(infilename));
		forceexten(outfilename, "afm");
/*		if (strcmp(infilename, outfilename) == 0) { */
		if (_strcmpi(infilename, outfilename) == 0) {
			fprintf(stderr, "Output and input file name equal %s\n",
				infilename);
			strcpy(bakfilename, infilename);	/* 1993/March/20 */
			forceexten(infilename, "bak");
			(void) remove(infilename);
			rename(bakfilename, infilename);
/*			exit(5); */
		}
		if ((input = fopen(infilename, "r")) == NULL) {
			perror(infilename); exit(3);
		}
		if ((output = fopen(outfilename, "w")) == NULL) {
			perror(outfilename); exit(4);
		}
/*		if (verboseflag != 0)  */
			printf("Processing %s\n", infilename);

		flag = readafmfile(input, output, 0);	/* prescan */
		if (flag == 0) {
			rewind(input);
			flag = readafmfile(input, output, 1);	/* produce output */
		}
		if (fclose(input) < 0) {
			perror(infilename); exit(7);
		}
		if (fclose (output) < 0) {
			perror(outfilename); exit(7);
		}
		if (flag < 0) {
			fprintf(stderr, "Failed to process file %s\n", infilename);
			(void) remove(outfilename);
		}
	}
	return 0;
}

/* does this replace old composite section with newly computed values ? */

/* this does recompute the FontBBox */

/* addcompo -v -b c:\cmyandy\cmr10 */			/* normal use */

/* addcompo -v -b -l=76 c:\cmyandy\cmcsc8 */	/* deal with xheight error */

/* addcompo -v -b -l=96 c:\cmyandy\cmcsc9 */	/* deal with xheight error */

/* addcompo -v -b -l=86 c:\cmyandy\cmcsc10 */	/* deal with xheight error */

/* addcompo -v -b -l=42 c:\cmyandy\cmtcsc10 */	/* deal with xheight error */

