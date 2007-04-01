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
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/* postproc.c program to process PS file from DVIPSONE: */
/* (1) smear and subsample monochrome images in PS files */
/* (2) expand extended f ligatures */
/* (3) replace upright Greek with glyphs from Symbol font */
/* (4) reorder pages based on DSC comments */

/* This will apply `binary convolution using 3x3 pixel mask */
/* it will also hunt down use of extra f-ligatures in text fonts */
/* it will also hunt down use of upright upper case Greek in text fonts */
/* keeps track of which fonts are text fonts (built-in list of math fonts) */

/* Looking for the pattern as follows: */

/* pattern of monochrome image call by DVIPSONE:

save
currentscreen pop dviscreen
/picstr 231 string def
currentpoint translate 29175399 36575847 scale
white
0 0 moveto 1 0 lineto 1 1 lineto 0 1 lineto closepath fill
black
1848 2317 true
[1848 0 0 2317 0 0]
{currentfile picstr readhexstring pop} bind
imagemask
......................HEX DATA.................
restore

*/

/* or pattern of monochrome image call in HPTAG produced PS file:

TE
TB 354 2839 translate 1848 1 scale
/Tiffrow 231 string def
1848 2317 true [1848 0 0 -1 0 0]
{ currentfile Tiffrow readhexstring pop } bind imagemask
......................HEX DATA.................
TE
TB

 */

#define MAXFILENAME 128

#define MAXLINE 512

#define MAXROW 4096						/* 13.65" at 300 dpi */

/* 8.5 inch * 300 dpi = 2550 */
/* typical HP TAG image is 6.5 in = 1952 pixel wide */

#define INTERVAL 100

#define MAXFONTS 256

#define USEMEMCPY

/**************************************************************************/

#define MAXLABEL 128

#define MAXPAGES 1024

#define MAXSECTIONS 256

/**************************************************************************/

char line[MAXLINE];

char oldline[MAXLINE];

/* buffers for pixels in three lines of the image */

char onebefore[MAXROW];

char previous[MAXROW];

char inrow[MAXROW];

char outrow[MAXROW];

char *fontnames[MAXFONTS];

long ptsizes[MAXFONTS];

/* sections are ordered according to following if section file read */

int sectionstart[MAXSECTIONS];

int sectionend[MAXSECTIONS];

int nsections=0;

int inlinelen, inwidth, inheight;
int outlinelen, outwidth, outheight;

/* controls on verbosity */

int verboseflag=0;
int traceflag=0;
int debugflag=0;
int dotflag=1;
int showpageflag=0;		/* show only page table */
int showfontsflag=0;	/* show only font table in HP TAG PS file */

int detailflag=0;
int thresholdflag=0;
int extensionflag=0;
int sectionfileflag=0;

int darkenflag=0;		/* `darken' thin lines in TIFF images */
int ligexpandflag=0;	/* expand extended f-ligatures ff, ffi, ffl */
int greekreplace=0;		/* draw Greek from Symbol font */
int strippkfonts=0;		/* Replace font definitions in HP TAG PS file */
int reorderflag=0;		/* reorder pages based on DSC comments */
int sectionorder=0;		/* reorder pages using section number file */
int passicons=1;		/* don't swell small TIFF images 96/Mar/21 */

int hptagflag=0;		/* reading hptag produced PS, not DVIPSONE output */
int iconflag=0;			/* set if TIFF image is small 96/Mar/21 */

int maxcolumn=78;		/* 72 for HP TAG PS files */

int suppressshort=0;

int smear2x2flag=0;		/* don't turn both on ! */
int smear3x3flag=1;		/* don't turn both on ! */

int threshold=2;		/* when do we turn the pixel on ? */

int subsample=0;		/* avoid this ... */

int nimages=0;			/* how many images processed */

int nfiles=0;			/* how many files processed */

int renameflag=0;		/* whether input file had to be renamed */

char *exten="psm";		/* default extension for output */

char pagelabel[32]="";	/* label of page --- counter[0] --- logical */

int pageordinal;		/* page number sequential physical */

/* char *oldplace=NULL; */

int outcolumn = 1;

int nswaps=0;

/* count of extended f-ligatures, dotlessi's etc */

int nfi=0;
int nfl=0;
int nff=0;
int nffi=0;
int nffl=0;
int ndotlessi=0;
int ndotlessj=0;
int nshort=0;
int ngreek=0;

int altcolumn=0; 

int currentfont=-1;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

long pagestarts[MAXPAGES];		/* seek in file to page start */

char *pagelabels[MAXPAGES];		/* labels of pages from DSC comments */

int count1[MAXPAGES];			/* TeX counter[1] --- section number */

int count2[MAXPAGES];			/* TeX counter[2] --- page number in section */

int index[MAXPAGES];			/* pointer into above for sorting */

long trailer=-1;

char *sectionfilename;				/* name of file with page order spec */

/**************************************************************************/

/* Is it something other than plain vanilla text font ? */

char *nottextfonts[] = {
/* "cmsy", "cmmi", "cmex", "cmbsy", "cmmib", */
"cm",
"keys10", "hb", "hpbats", "hplogo", "hppi",
"dm10", "ml10", "wl10", "ky10", "line",
"circle", "lcircle", "lasy", "manfnt", 
"mtmi", "mtsy", "mtex", "rmtmi", "lbm", "mh", ""
};

/* names of Upper case Greek letters in CM fonts --- not used anymore */

char *Greek[] = {
"Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", 
"Sigma", "Upsilon", "Phi", "Psi",
"Omega", "" /* "Ohm", "" */
};

/* char codes of upright upper case Greek in Symbol font */

/* int GreekCodes[] = {
71, 68, 81, 76, 88, 80, 83, 85, 70, 89, 87, -1
}; */

char *GreekCodes[]={
"G", "D", "Q", "L", "X", "P", "S", "U", "F", "Y", "W", ""
};

/* char *GreekProlog="\
dvidict begin\n\
/Greek /Symbol findfont 10 scalefont def\n\
/G{Greek setfont show}bind def\n\
end\n\
"; */

/* char *GreekProlog="\
/Greek /Symbol findfont 10 scalefont def\n\
/G{Greek setfont show}bind def\n\
"; */

/* following assumes %%EndProlog is preceeded with `dvidict begin' */
/* and that we want `dvidict begin' correspondingly after this definition */
/* odd, result of DVIPSONE wrapping the whole job in dvidict begin/end ... */

char *GreekProlog=
"/Greek /Symbol 10 65536 mul mf\n"
"/G{currentfont exch Greek setfont show setfont}bind def\n"
"end % dvidict\n"
"dvidict begin\n";

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void resetoutrow(int outwidth) {
#ifndef USEMEMCPY
	int k;
	for (k = 0; k < outwidth; k++) outrow[k] = 0;
#else
	memset(outrow, 0, outwidth * sizeof(int));
#endif
}

/* just copy input to output row */

void copyarow(int inwidth) {
#ifndef USEMEMCPY
	int k;
	for (k = 0; k < inwidth; k++) outrow[k] = inrow[k];
#else
	memcpy(outrow, inrow, inwidth * sizeof(int));
#endif
}

/* new pixel is OR of 2 x 2 block old pixels */

void smear2x2row(int inwidth) {
	int kinc, k, count;

	if (subsample) kinc = 2;
	else kinc = 1;
/*	for (k = 0; k < inwidth; k++) { */
	if (threshold == 0 || threshold == 1) {
#ifdef USEMEMCPY
		resetoutrow(outwidth);
#endif
		for (k = 0; k < inwidth; k += kinc) {
/*			if (inrow[k]) outrow[k] = 1;
			else if (inrow[k+1]) outrow[k] = 1;
			else if (previous[k]) outrow[k] = 1;
			else if (previous[k+1]) outrow[k] = 1; */
			if (inrow[k] || inrow[k+1] ||
				previous[k] || previous[k+1])
				outrow[k] = 1;
#ifndef USEMEMCPY
			else outrow[k] = 0;
#endif
		}
	}
	else {
		for (k = 0; k < inwidth; k += kinc) {
			count = 0;
/*			count = inrow[k] + inrow[k+1] +
					previous[k] + previous[k+1]; */
			if (inrow[k]) count++;
			if (inrow[k+1]) count++;
			if (previous[k]) count++;
			if (previous[k+1]) count++;
			if (count >= threshold) outrow[k] = 1;
			else outrow[k] = 0;
		}
	}
}

/* new pixel is OR of 3 x 3 block old pixels */

void smear3x3row(int inwidth) {
	int kinc, k, count;

	if (subsample) kinc = 3;
	else kinc = 1;
/*	for (k = 0; k < inwidth; k++) { */
	if (threshold == 0 || threshold == 1) {
#ifdef USEMEMCPY
		resetoutrow(outwidth);
#endif
		for (k = 0; k < inwidth; k += kinc) {
/*			if (inrow[k]) outrow[k] = 1;
			else if (inrow[k+1]) outrow[k] = 1;
			else if (inrow[k+2]) outrow[k] = 1;
			else if (previous[k]) outrow[k] = 1;
			else if (previous[k+1]) outrow[k] = 1;
			else if (previous[k+2]) outrow[k] = 1;
			else if (onebefore[k]) outrow[k] = 1;
			else if (onebefore[k+1]) outrow[k] = 1;
			else if (onebefore[k+2]) outrow[k] = 1; */
			if (inrow[k] || inrow[k+1] || inrow[k+2] ||
				previous[k] || previous[k+1] || previous[k+2] ||
				onebefore[k] ||	onebefore[k+1] || onebefore[k+2])
				outrow[k] = 1;
#ifndef USEMEMCPY
			else outrow[k] = 0;
#endif
		}
	}
	else {
		for (k = 0; k < inwidth; k += kinc) {
			count = 0;
/*			count = inrow[k] + inrow[k+1] + inrow[k+2] +
					previous[k] + previous[k+1] + previous[k+2] +
					onebefore[k] + onebefore[k+1] + onebefore[k+2]; */
			if (inrow[k]) count++;
			if (inrow[k+1]) count++;
			if (inrow[k+2]) count++;
			if (previous[k]) count++;
			if (previous[k+1]) count++;
			if (previous[k+2]) count++;
			if (onebefore[k]) count++;
			if (onebefore[k+1]) count++;
			if (onebefore[k+2]) count++;
			if (count >= threshold) outrow[k] = 1;
			else outrow[k] = 0;
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* scan ahead to get image dimension so can calculate string buffer required */
/* note: this reuses the global line buffer */
/* so we remember where we where in the file so we can go back */

int getwidthandheight (FILE *input) {
	long current;
	char *s;

	current = ftell(input);
	while ((s = fgets (line, sizeof(line), input)) != NULL) {
		if (strstr(line, " true") != NULL ||
			strstr(line, " false") != NULL) {
			if (sscanf(line, "%d %d true", &inwidth, &inheight) == 2)
				break;
		}
/*		if (*line == '[') {
			if (sscanf(line, "[%d 0 0 %d 0 0 ]", &inwidth, &inheight) == 2)
				break;
		} */
	}
/*	if it is less than 1" by 1" treat as icon and don't fatten it */
	if (inwidth < 300 && inheight < 300) iconflag = 1;
	else iconflag = 0;
	if ((smear2x2flag && (inwidth + 1) >= MAXROW) ||
		(smear3x3flag && (inwidth + 2) >= MAXROW) ||
		inwidth >= MAXROW) {
		fprintf (stderr, "ERROR: Image width (%d) too large (> %d)\n",
				 inwidth, MAXROW);
		exit(2);
	}
	if (s == NULL) {
		fprintf(stderr, "ERROR: Premature EOF in image header\n");
		exit(2);
/*		return -1; */
	}
	fseek(input, current, SEEK_SET);
	return 1;
}

/**************************************************************************/

/* is it something other than plain vanilla text font */

int istextfont (int k) {
	int i, n;
	if (k < 0) return 1;
	if (k >= MAXFONTS) return 1;
	if (fontnames[k] == NULL) return 1;
	for (i = 0; i < 64; i++) {
		if (strcmp(nottextfonts[i], "") == 0) break;
		n = strlen(nottextfonts[i]);
		if (_strnicmp(fontnames[k], nottextfonts[i], n) == 0) return 0;
	}
	return 1;
}

void showfont(int k) {
	int ptint, ptdec;
	char *s;
	if (k < 0) return;
	s = fontnames[k];
	if (s == NULL) s = "unknown";
	ptdec = (int) (ptsizes[k] * 10 / 65536);
	ptint = ptdec / 10;
	ptdec = ptdec - ptint * 10;
/*	printf("`%s' (font %d) at %d.%1dpt: ", s, k, ptint, ptdec); */
	printf("font `%s' at %d.%1dpt ", s, ptint, ptdec);
}

/**************************************************************************/

/* by the way, \050 and \051 are just ( and ) */

int processoctal (char *u) {		/* arg points at \016 say */
	int n=0, c;
	char *s=u+1;
	int count=0;
	char buffer[256];
	
/*	if (traceflag) printf("PROCESSOCTAL: %s", line); */
/*	if it is not a text font we don't try and be clever at all */
/*	remember no CM fonts are plain text fonts ... */
	if (!istextfont(currentfont)) return 0;

	c = *s;
	if (c >= '0' && c <= '7') {
		while (c >= '0' && c <= '7') {
			c = c - '0';
			n = n << 3;
			n = n | c;
			s++;
			c = *s;
		}
	}
	else {
		switch (c) {
		case 'b': n = 8; break;
		case 't': n = 9; break;
		case 'n': n = 10; break;
		case 'f': n = 12; break;
		case 'r': n = 13; break;
		default: {
			if (altcolumn > 0) {
				putc('\n', stdout);
				altcolumn = 0;
			}
			printf("BAD OCTAL CODE %d on page %s (%d) in ",
				   c, pagelabel, pageordinal);
			showfont(currentfont);
			printf(":\n%s", line);
			return -1;						/* bad code */
		}
		}
		s++;
	}
/*	if (n == 0) return -1; */				/* not octal code */
	if (n == 37) return -1;					/* % */
/*	here u still points at the initial \ and s points just past octal code */
/*  heuristic to avoid replacing (\16) and (\17) in math symbol font */
	if (suppressshort) {
		if (u > line && *(u-1) == '(' && *s == ')') {
			nshort++;
			return 0;	
		}  /* not needed anymore ??? can't get here in math font */
	}
	strcpy(buffer, s);						/* save end of string */
	switch(n) {
		case 11:
			*u = 'f';
			*(u+1) = 'f';
			strcpy(u+2, buffer);
			nff++;
			count++;
/*			if (traceflag) printf(line); */
			break;

		case 14:
			*u = 'f';
/*			*(u+1) = 'f';
			*(u+2) = 'i';
			strcpy(u+3, buffer); */
			*(u+1) = '\\';
			*(u+2) = 'f';			/* \012 = fi */
			strcpy(u+3, buffer);
			nffi++;
			count++;
/*			if (traceflag) printf(line); */
			break;

		case 15:
			*u = 'f';
/*			*(u+1) = 'f';
			*(u+2) = 'l';
			strcpy(u+3, buffer); */
			*(u+1) = '\\';
			*(u+2) = 'r';			/* \013 = fl */
			strcpy(u+3, buffer);			
			nffl++;
			count++;
			break;

		case 12:
			nfi++;
/*			count++; */
			break;

		case 13:
			nfl++;
/*			count++; */
			break;

		case 16:
			ndotlessi++;
/*			count++; */
			break;

		case 17:
			ndotlessj++;
/*			count++; */
			break;
			
		default: {
			if (altcolumn > 0) {
				putc('\n', stdout);
				altcolumn = 0;
 			}
			if (greekreplace && n < 11) {
				if (traceflag) {
					printf("REPLACING CODE %d on page %s (%d) in ",
						   n, pagelabel, pageordinal);
					showfont(currentfont);
					printf("with `%s' ", Greek[n]);
					putc('\n', stdout);
				}
				ngreek++;
/*				if (traceflag) printf("%s", line); */
/*				strcpy(u-1, " ");
				strcat(u, Greek[n]);
				strcat(u, " ");
				strcat(u, buffer); */
/*				check for empty string - but watch for \( instead of ( */
/*				if (u > line && *(u-1) != '(') strcpy(u, ")s("); */
/*				if (u > line && *(u-1) != '(' &&
					(u < line+1 || *(u-2) == '\\'))
					strcpy(u, ")s(");
				else *u = '\0';	*/
				if (u > line && *(u-1) == '(' &&
					(u < line+1 || *(u-2) != '\\'))
					*u = '\0';				/* remove ()s */
				else strcpy(u, ")s(");
				strcat(u, GreekCodes[n]);
/*				strcat(u, ")G("); */
				strcat(u, ")G");
				if (strncmp(buffer, ")s", 2) == 0) 
					strcat(u, buffer+2);	/* remove ()s */
				else if (strncmp(buffer, ")S", 2) == 0) {
					strcat(u, " w");
					strcat(u, buffer+2);	/* replace ()S with w */
				}
				else {
					strcat(u, "(");
					strcat(u, buffer);
				}
				count++;
/*				if (traceflag) printf(line); */		/* debug output */
			}
			else {
				printf("SUSPICIOUS CODE %d on page %s (%d) in ",
				   n, pagelabel, pageordinal);
				showfont(currentfont);
				printf(":\n%s", line); 
/*				printf(" %s", u); */
			}
			break;
		 }
	}
/*	return 0; */
	return count;
}

char *breakchars="btnfr";		/* \b \t \n \f \r in PS strings */

int expandligatures(void) {
	char *s=line, *t, *u;
	int c;
	int flag;
	int count=0;
/*	char oldline[MAXLINE]; */

/*	if (traceflag) printf("EXPANDLIGATURES: %s", line); */
/*	if (traceflag) strcpy(oldline, line); */

	if ((u = strchr(line, '\\')) == NULL) 
		return 0;	/* nothing to do */
	if (traceflag) strcpy(oldline, line);			/* 1996/Feb/12 */
/*	Look for start of PostScript string */
	while ((s = strchr(s, '(')) != NULL) {
/*		Look for end of PostScript string */
		t = s;
		for (;;) {
			t = strchr(t, ')');
			if (t == NULL) break;			/* should not happen */
			if (*(t-1) != '\\') break;
			t++;
		}
		if (t == NULL) break;				/* should not happen */
		u = s;
		for (;;) {
/*			Now look for octal codes in between s and t */
			u = strchr(u, '\\');
			if (*(u+1) == '\\') {
				u += 2; continue;
			}
			if (u == NULL) break;
			if (u > t) break;
			c = *(u+1);
			if (c == '(' || c == ')' || c == '\\' || c == '%') {
				u++;
				continue;
			}
			if (c >= '0' && c <= '7') {
				flag = processoctal(u);
				if (flag > 0) count = count + flag;
			}
/*			also do b, t, n, f, r */
			else if (strchr(breakchars, c) != NULL) {
				flag = processoctal(u);
				if (flag > 0) count = count + flag;
			}
			u++;
		}
		if (u == NULL) break;				/* nothing more to do */
		s = t;								/* affected by moves ? */
/*		if (traceflag) printf("NEXT STRING: %s", s); */
	}
	if (traceflag) {
		if (count > 0) {
			printf("BEFORE:\t%s", oldline);
			printf("AFTER:\t%s", line);
		}
	}
	*oldline = '\0';
}
/***********************************************************************/

/* scans for image, keeps track of font, expands ligatures if needed */
/* returns +1 if it finds an image */
/* can return 0 (if reorderflag == 0) when it hits end of page (or %%EOF) */
/* returns -1 if it hits end of file */

int scantoimage(FILE *output, FILE *input) {
	char *s, *t;
	int n, nw, nh;
	int k;
	char buffer[256];
	int notfirst=0;			/* not on first line --- allow initial %%Page */

/*	*oldline = '\0'; */ /* just to be safe ??? */
	while ((s = fgets(line, sizeof(line), input)) != NULL) {
		if (*line == 12) {
			putc(12, output);
			strcpy(line, line+1);
		}
/*		when reordering pages stop at end of page */
		if (reorderflag && notfirst) {
			if (strncmp(line, "%%Page:", 7) == 0) return 0;
			if (strncmp(line, "%%Trailer", 9) == 0) return 0;
			if (strncmp(line, "%%EOF", 5) == 0) return 0;
		}
		if (strncmp(line, "%%Page:", 7) == 0) {
/*			notfirst++; */
/*			printf("LINE: %s", line); */
/*			sscanf(line+2, "Page: %s %d", &pagelabel, &pageordinal); */
			sscanf(line+2, "Page: %s %d", pagelabel, &pageordinal);
/*			if (traceflag) fputs(line, stdout); */
			if (traceflag || showpageflag) fputs(line, stdout);
			else if (dotflag) {
				if (verboseflag == 0)	/* don't do asterisks if verbose */
					putc('*', stdout);
				altcolumn++;
			}
		}
/*		Rewritten to deal with both /picstr *and* /Tiffrow */
		if (sscanf (line, "/picstr %d string def", &inlinelen) > 0 ||
			sscanf (line, "/Tiffrow %d string def", &inlinelen) > 0) {
			if (debugflag) fputs(line, stdout); 
			strcpy (buffer, line);
			getwidthandheight(input);			/* get width and height */
			strcpy (line, buffer);
			if (inlinelen != (inwidth + 7) / 8)
fprintf(stderr, "How can line length be %d bytes when width is %d pixels?\n",
					   inlinelen, inwidth);
			if (subsample) {
				if (smear2x2flag) outlinelen = (inwidth/2 + 7) / 8;
				else if (smear3x3flag) outlinelen = (inwidth/3 + 7) / 8;
				else outlinelen = inlinelen;
			}
			else outlinelen = inlinelen;
/*			if (subsample)  { */
/*				sprintf(line, "/picstr %d string def\n", outlinelen); */
/*			Rewritten to deal with both /picstr *and* /Tiffrow */
				t = line;
				while (*t > ' ') t++;			/* step to white space */
				while (*t <= ' ' && *t != '\0') t++;	/* over white space */
				sprintf(t, "%d string def\n", outlinelen);	/* 96/Jan/28 */
				/*			printf("CONSTRUCTED: %s END\n", line); */
/*			} */
			fputs(line, output);
			break;						/* hit an image */
		}
/*		font switch ? */
		if (*line == 'f' && sscanf(line, "f%d", &k) == 1)
			currentfont = k;
/*		want to expand possible ligatures ? */
		if (ligexpandflag) expandligatures ();
		fputs(line, output);
		notfirst++;
	}
/*	if (traceflag) printf("Now have an image: %s", line); *//* debug */
	if (s == NULL) return EOF;
/*	now have an image get details */
	while ((s = fgets(line, sizeof(line), input)) != NULL) {
		if (strstr(line, "true") != NULL || strstr(line, "false") != NULL) {
			if (sscanf(line, "%d %d%n true", &inwidth, &inheight, &n) == 2) {
				if (subsample) {
					if (smear2x2flag) {
						outwidth = inwidth / 2;
						outheight = inheight / 2;
					}
					else if (smear3x3flag) {
						outwidth = inwidth / 3;
						outheight = inheight / 3;
					}
					else {
						outwidth = inwidth;
						outheight = inheight;
					}
					strcpy(buffer, line+n);	/* save tail end */
					sprintf(line, "%d %d true ", outwidth, outheight);
					strcat(line, buffer);	/* append tail end */
				}
				else {
					outwidth = inwidth ;
					outheight = inheight;
				}
			}
			fputs(line, output);
			break;
		}
		fputs(line, output);
	}
/*	if (traceflag) printf("Now have an image: %s", line); *//* debug */
	if (s == NULL) return EOF;
	if (hptagflag == 0) {
	while ((s = fgets(line, sizeof(line), input)) != NULL) {
		if (sscanf(line, "[%d 0 0 %d 0 0]", &nw, &nh) == 2) {
			if (nw != inwidth || nh != inheight)
fprintf(stderr, "Inconsistent width and heigth info (%d %d) versus (%d %d)\n",
		   inwidth, nw, inheight, nh);
			if (subsample) {
				sprintf(line, "[%d 0 0 %d 0 0]\n", outwidth, outheight);
			}				
			fputs(line, output);
			break;
		}
		fputs(line, output);
	}
	}
/*	if (traceflag) printf("Now have an image: %s", line); *//* debug */
	if (s == NULL) return EOF;
	while ((s = fgets(line, sizeof(line), input)) != NULL) {
		fputs(line, output);
/*		if (strncmp(line, "imagemask", 9) == 0) break; */
		if (strstr(line, "imagemask") != NULL) break;
	}
/*	if (traceflag) printf("Now have an image: %s", line); *//* debug */
	if (s == NULL) return EOF;

	if (verboseflag) {
		if (subsample) {
			if (pageordinal > 0) {
				printf("On page %s (%d).\t", pagelabel, pageordinal);
				printf("\n");
			}
			printf("Image in: width %d height %d (bytes per row %d)\n",
				   inwidth, inheight, inlinelen);
			printf("Image out: width %d height %d (bytes per row %d)\n",
				   outwidth, outheight, outlinelen);
		}
		else {
			if (pageordinal > 0)
				printf("On page %s (%d).\t", pagelabel, pageordinal);
			printf("Image: width %d height %d (bytes per row %d)\n",
						inwidth, inheight, inlinelen);
		}
	}
/*	if (traceflag) printf("Now exiting scantoimage\n"); *//* debug */
	return 1;			/* success --- found an image */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int readarow(FILE *input, int inwidth) {
	int i, j, c;
	char *s=line;
	
	i = 0;
	for (;;) {
		if (*oldline != '\0') {
			strcpy(line, oldline);
			s = line;
			*oldline = '\0';
		}
		else s = fgets(line, sizeof(line), input);
		if (s == NULL) {
			fprintf(stderr, "HIT EOF while reading image row\n");
			return EOF;
		}
		if (strncmp(line, "restore", 7) == 0 ||
			strncmp(line, "TE", 2) == 0) {
			fprintf(stderr, "HIT `restore' or `TE' while reading image row\n");
			return 0;
		}
		while ((c = *s) != '\0') {
			if (c <= ' ') break;						/* end of line */
			if (c >= '0' && c <= '9') c = c - '0';
			else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;				
			else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;				
			else {
				fprintf(stderr, "ERROR: HIT bad character (%c %d) in image",
						c, c);
				fprintf(stderr, " i %d inwidth %d\n", i, inwidth);
				fprintf(stderr, "Bad line is: %s", line);
				return 0;
			}
			for (j = 0; j < 4; j++) {
				if (c & 8) inrow[i] = 1;
				else inrow[i] = 0;
				c = c << 1;
				i++;
			}
			s++;
			if (hptagflag) {
				if (i >= inwidth-1) {
/*					oldplace = s; */
					strcpy(oldline, s);
					if (debugflag)
						printf("Line ends column %d (i %d inwidth %d)\n",
							   s-line, i, inwidth);
					break;
				}
			}
		}
		if (i >= inwidth-1) break;
	}
	return 1;
}

int writearow(FILE *output, int outwidth) {
	int i, j, c, n;
	char *s;
	
	if (hptagflag == 0) outcolumn = 1;
	i = 0;
	n = 0;								/* how many hex digits output */
	s = line;
	for (;;) {
		c = 0;
		for (j = 0; j < 4; j++) {
			c = c << 1;
			if (outrow[i]) c = c | 1;
			if (subsample) {
				if (smear2x2flag) i += 2;
				else if (smear3x3flag) i += 3;
				else i++;
			}
			else i++;
		}
		if (c < 10) c = c + '0';
		else c = c + 'A' - 10;
		*s++ = (char) c;
		n++;
		if (outcolumn++ >= maxcolumn) {
			*s++ = '\n';
			*s = '\0';
			fputs(line, output);
			s = line;
			outcolumn = 1;
		}
/*		if (i >= outwidth-1) break; */
/*		if (i >= inwidth-1) break; */
/*		check only after even number of hex digits ... */
		if ((n % 2) == 0 && i >= inwidth-1) break;
	}
	if (hptagflag == 0) {
		*s++ = '\n';
		outcolumn = 1;
	}
	*s = '\0';
	fputs(line, output);
/*	if (n / 2 != outlinelen) */
	if ((n % 2) != 0 || (n / 2) != outlinelen)
		fprintf(stderr, "Output %d bytes instead if %d\n", n / 2, outlinelen);
	if (ferror(output)) {
		fprintf(stderr, "ERROR: Output error ");
		perror("");
		exit(2);
	}
	return 1;
}	

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void makeblankrow(int inwidth) {
#ifndef USEMEMCPY
	int k;
	for (k = 0; k < inwidth; k++) inrow[k] = 0;
#else
	memset(inrow, 0, inwidth * sizeof(int));
#endif
}

void copytoprevious (void) {
#ifndef USEMEMCPY
	int k;
	for (k = 0; k < inwidth; k++) previous[k] = inrow[k];
#else
	memcpy(previous, inrow, inwidth * sizeof(int));
#endif
}

void copytoonebefore (void) {
#ifndef USEMEMCPY
	int k;
	for (k = 0; k < inwidth; k++) onebefore[k] = previous[k];
#else
	memcpy(onebefore, previous, inwidth * sizeof(int));
#endif
}

/**************************************************************************/

int processimage(FILE *output, FILE *input) {
	int k, flag;
	int kstart = 0;
	int nlines=0;
	long current;

/*	if (hptagflag) oldplace=NULL; */
	if (hptagflag) *oldline = '\0';
	if (hptagflag) outcolumn = 1;
/*	prime the pump with 1 or 2 lines if smearing by 2 or 3 */
	if (smear2x2flag) {
		flag = readarow(input, inwidth); 
		kstart++; 
/*		makeblankrow(inwidth); */
	}
	if (smear3x3flag) {
		copytoprevious();
		flag = readarow(input, inwidth); 
		kstart++; 
/*		makeblankrow(inwidth); */
	}
/*	for (k = kstart; k < inheight; k++) { */
/*	for (k = 0; k < inheight - kstart; k++) {  */
	for (k = 0; k < inheight; k++) { 
		if (smear3x3flag) copytoonebefore();
		if (smear2x2flag || smear3x3flag) copytoprevious();
/*	at end insert blank rows to compensate for priming the pump */
		if (k >= inheight - kstart) makeblankrow(inwidth);
		else {
			if ((flag = readarow(input, inwidth)) <= 0) {
				fprintf(stderr, "EOF while processing image\n");
				fprintf(stderr, "k %d inheight %d\n", k, inheight);
				return flag;
			}
		}
		if (traceflag) {
			if ((k % 4) == 0)			/* cut down on output somewhat */
				putc('.', stdout);
		}
		else if (verboseflag && (k % INTERVAL) == 0) putc('.', stdout);
		if (!subsample) {
			if (smear2x2flag) smear2x2row(inwidth); 
			else if (smear3x3flag) smear3x3row(inwidth);
			else copyarow(inwidth);
			writearow(output, outwidth);
			nlines++;
		}

		if (subsample) {
			if (smear2x2flag) {
				if ((k % 2) == 0) {
					smear2x2row(inwidth);
					writearow(output, outwidth);
					nlines++;
				}
			}
			else if (smear3x3flag) {
				if ((k % 3) == 0) {
					smear3x3row(inwidth);
					writearow(output, outwidth);
					nlines++;
				}
			}
			else {
				copyarow(inwidth);
				writearow(output, outwidth);
				nlines++;
			}
		}
	}	/* end of for loop over image lines */

	if (hptagflag && outcolumn > 0) {
		putc('\n', output);
		outcolumn = 0;
	}
/*	if (traceflag) putc('\n', stdout); */
	if (traceflag || verboseflag) putc('\n', stdout);
	if (nlines != outheight)
		fprintf(stderr, "Wrote %d lines instead of expected %d\n",
			nlines, outheight);
	current = ftell(input);
/*	check on next line to see if end of image */
	fgets(line, sizeof(line), input);
	if (strncmp(line, "restore", 7) != 0 &&
		strncmp(line, "TE", 2) != 0) {
		fputs("Line after image is not `restore' or `TE'\n", stdout);
		fputs(line, stdout);
	}
	fseek(input, current, SEEK_SET);
	return 1;
}

int copyimage(FILE *output, FILE *input) {
	while (fgets(line, sizeof(line), input) != NULL) {
		fputs(line, output);
		if (strncmp(line, "restore", 7) == 0) return 0; /* DVIPSONE PS */
		if (strncmp(line, "TE", 2) == 0) return 0;	/* HP TAG PS 96/Mar/21 */
	}
	return -1;									/* hit end of file */
}

/**************************************************************************/

void freefontnames (void) {
	int k;
	for (k=0; k < MAXFONTS; k++) {
		if (fontnames[k] != NULL) {
			free(fontnames[k]);
			fontnames[k] = NULL;
		}
	}
}

void showfonttable (void) {
	int k, count=0;
	
	for (k=0; k < MAXFONTS; k++) {
		if (fontnames[k] != NULL) {
			printf("%d\t", k);
			showfont(k);
			putc('\n', stdout);
			count++;
		}
	}
	if (count == 0) exit(1);	/* no fonts */
}

/* may want to protect that load with some error checking ? */

char *newcode=
"/SF{load exch pop setfont} bdf\n"
"/DefFont { % /fontname /FontName resolution(dpi) ptsize\n"
"exch 72 div mul % dpi * ptsize / 72\n"
"exch findfont exch scalefont def} bdf\n";

long current;

int extractfonttable(FILE *, FILE *, int);

/* Work HP TAG PS section between %%EndProlog and %%Page */
/* Strip out bitmapped fonts and replace with calls for outline fonts */

int stripbitmaps (FILE *output, FILE *input) {
	fputs(line, output);				/* output %%EndProlog line */
/*	fputs(newcode, output); */			/* insert new PS code */
	return extractfonttable(output, input, 1);
}

/* Reads and copies header, extracts font definitions, inserts Greek prolog */
/* This is for DVIPSONE PS file format */
/* Stops at %%EndSetup (i.e. before first %%Page:) */

int tryreadfontdefs (FILE *output, FILE *input) { 
	char *s;
	int fn, k;
	long ptsize;
	char fontname[64];
	int prologflag=0;
/*	long current; */
	
	for (k = 0; k < MAXFONTS; k++) fontnames[k] = NULL;
	for (k = 0; k < MAXFONTS; k++) ptsizes[k] = 0;
	current = ftell(input);
	while ((s = fgets(line, sizeof(line), input)) != NULL) { 
		if (*line == 12) {
			putc(12, output);
			current++;
			strcpy(line, line+1);
		}
		if (strncmp(line, "/TeXdict", 8) == 0) {	/* 96/Jan/28 */
			hptagflag++;
			maxcolumn=72;
			printf("This is not a PS file produced by DVIPSONE\n");
			printf("Will assume it is a file produced by HP TAG PS driver\n");
			if (ligexpandflag) printf("Will not expand f ligatures\n");
			ligexpandflag=0;
			if (greekreplace) printf("Will not replace Greek letters\n");
			greekreplace=0;
/*			if (reorderflag) printf("Will not rearrange page order\n");
			reorderflag=0; */
/*			fputs(line, output); */		/* can't quit yet ? */
/*			return 0; */				/* can't quit yet ? */
		}
		if (strncmp(line, "%%EndProlog", 9) == 0) {
/*			if (hptagflag == 0) */
			if (greekreplace != 0 && hptagflag == 0)
				fputs(GreekProlog, output);	/* insert new Greek Symbol code */
			if (strippkfonts) stripbitmaps (output, input);
			prologflag++;
		}
		if (strncmp(line, "%%Page:", 7) == 0) {	/* HP TAG exit */
			fseek (input, current, SEEK_SET);	/* step back one */
			return 0;							/* end of header */
/*			break; */
		}
		fputs(line, output);
		if (strncmp(line, "% Font Defs", 11) == 0) break; 
		current = ftell(input);
	}

	if (!prologflag) fputs("Did not find %%Prolog\n", stdout);
	if (s == NULL) return -1;						/* hit end of file */

	while ((s = fgets(line, sizeof(line), input)) != NULL) {
		fputs(line, output);
		if (strncmp(line, "fstart", 6) == 0) break;
	}
	if (s == NULL) return -1;						/* hit end of file */

	while ((s = fgets(line, sizeof(line), input)) != NULL) {
		fputs(line, output);
		if (traceflag) fputs(line, stdout);
		if (sscanf(line, "/fn%d /%s %ld mf /f%d{",
				   &fn, fontname, &ptsize, &k) == 4) {
			if (k >= 0 && k < MAXFONTS) {
				fontnames[k] = _strdup(fontname);
				ptsizes[k] = ptsize;
			}
			else printf("Bad font number %d\n", k);
		}
		if (strncmp(line, "fend", 4) == 0) break;
	}
	if (traceflag) printf("Constructed Font Table:\n");
	if (traceflag) showfonttable();
	if (s == NULL) return -1;						/* hit end of file */

	if (traceflag) printf("Copying rest of header up to first page\n");
/*	continue scanning for %%EndSetup */				/* 1996/Feb/13 */
	while ((s = fgets(line, sizeof(line), input)) != NULL) {
		fputs(line, output);
		if (strncmp(line, "%%Page:", 7) == 0 ||
			strncmp(line, "%%Trailer", 9) == 0 ||
			strncmp(line, "%%EOF", 5) == 0) {
			fprintf(stderr, "ERROR: While copying header hit: %s", line);
			break;	/* wrong ! */
		}
		if (strncmp(line, "%%EndSetup", 10) == 0) break;	/* normal */
	}
	if (s == NULL) return -1;						/* hit end of file */
	return 0;
}

/***********************************************************************/

/* just process the file front to end - do not reorder pages */

int	processfilelinear(FILE *output, FILE *input) {
	int flag=1;

	nfi=nfl=nff=nffi=nffl=ndotlessi=ndotlessj=nshort=ngreek=0;
	nimages=0;
	pageordinal=0;
	altcolumn= 0;
	outcolumn= 0;
	currentfont=-1;
	strcpy(pagelabel, "");
	if (tryreadfontdefs(output, input) < 0) return -1;
	while (flag > 0) {
		flag = scantoimage (output, input);
/*		ignore end of page transitions (flag == 0) in this linear mode */
		while (flag == 0) scantoimage (output, input); /* ??? */
/*		if (flag <= 0) { */
		if (flag < 0) {
			if (altcolumn > 0) putc('\n', stdout);
			printf("Normal EOF on input\n");
			nfiles++;
			return 1;
		}
/*		if (verboseflag) putc('*', stdout); */
		if (passicons != 0 && iconflag != 0) { 
			if (darkenflag) 
				printf("Passing small TIFF image (%d x %d)\n",
				   inwidth, inheight);
			copyimage(output, input);
		}
		else if (darkenflag) {
/*		if (darkenflag)  */
			flag = processimage(output, input);
			if (flag <= 0) {
				fprintf(stderr, "Failed to process image\n");
				return flag;
			}
			nimages++;
		}
		else copyimage(output, input);
	}
	fprintf(stderr, "Can't get here!\n");
	return flag;
}

/**************************************************************************/

void copytrailer (FILE *, FILE *);

void buildpagetable(FILE *);

void showpagetable(int);

void resortpages (void);

int	processfilereorder(FILE *output, FILE *input) {
	int k, pageno;
	int flag=1;
	long start;

	nfi=nfl=nff=nffi=nffl=ndotlessi=ndotlessj=nshort=ngreek=0;
	nimages=0;
	pageordinal=0;
	altcolumn= 0;
	outcolumn= 0;
	currentfont=-1;
	strcpy(pagelabel, "");
/*	following copies header, extracts font defs, inserts Greek prolog */
	if (traceflag) printf("Copying header, extracting font info\n");
	if (tryreadfontdefs(output, input) < 0) return -1;
	if (traceflag) printf("Process individual pages now\n");
/*  now ready to copy pages in requested order */
	for (k = 0; k < MAXPAGES; k++) {
		pageno = index[k];
		if (pageno < 0) continue;			/* not valid */
		start = pagestarts[pageno];
		if (start < 0) continue;
		if (verboseflag) {
			printf("Page %d (%s) at %ld\n", pageno, pagelabels[pageno], start);
		}
		if (fseek (input, start, SEEK_SET) < 0) continue;	/* error */
		for (;;) {
			flag = scantoimage (output, input);
			if (flag == 0) break;			/* found end of page */
			if (flag < 0) {					/* found end of file */
				if (altcolumn > 0) putc('\n', stdout);
				printf("ERROR: unexpected EOF on input\n");
				nfiles++;
				return 1;
			}
/*			if (verboseflag) putc('*', stdout); */
			if (darkenflag) {
				flag = processimage(output, input);
				if (flag <= 0) {
					fprintf(stderr, "Failed to process image\n");
					return flag;
				}
				nimages++;
			}
			else copyimage(output, input);
		}
	}
/*	now done all pages */
	if (altcolumn > 0) putc('\n', stdout);	/* ??? */
	fseek(input, trailer, SEEK_SET);
	if (traceflag) printf("Copying trailer\n");
	copytrailer(output, input);
	nfiles++;
/*	fprintf(stderr, "Can't get here!\n"); */
	return flag;
}

int reindex (void);

/* set up arrays for page index information */

void setuporder (FILE *input, int flag) {
	int k;
	for (k = 0; k < MAXPAGES; k++) pagestarts[k] = -1;
	for (k = 0; k < MAXPAGES; k++) pagelabels[k] = NULL;
	for (k = 0; k < MAXPAGES; k++) count1[k] = -1;
	for (k = 0; k < MAXPAGES; k++) count2[k] = -1;
	for (k = 0; k < MAXPAGES; k++) index[k] = k;
	if (flag == 0) return;
	if (traceflag || showpageflag) printf("Building page table\n");
	buildpagetable(input);
	if (traceflag || showpageflag)  {
		printf("Constructed page table:\n");
		showpagetable(0);
	}
	if (traceflag || showpageflag) printf("Sorting page table\n");
	if (sectionorder) reindex();
	else resortpages();
/*	if (nswaps > 0) { */
	if (nswaps > 0 || sectionorder) {
		if (traceflag || showpageflag) {
			printf("Sorted page table:\n");
			showpagetable(1);
		}
	}
	if (traceflag) printf("Rewinding\n");
	rewind(input);
}

void clearuporder (void) {
	int k;
	for (k = 0; k < MAXPAGES; k++)
		if (pagelabels[k] != NULL) free(pagelabels[k]);
}

int processfile (FILE *output, FILE *input, int showpageflag) {
	int flag;
	if (reorderflag) {					/* new 1996/Feb/13 */
		setuporder(input, reorderflag);
		if (showpageflag) return 0;			/* just for showing page table */
		flag = processfilereorder(output, input);
		clearuporder();
	}
	else flag = processfilelinear(output, input);
	return flag;
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

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/****************************************************************************/

/* scan the file building page table based on %%Page: DSC comments */

void buildpagetable (FILE *input) {
	long current;
	int pageno;
	char label[MAXLABEL];
	int ca, cb;
	
	trailer=-1;
	current = ftell(input);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == 12) {
			strcpy(line, line+1);
			current++;
		}
		if (*line != '%') {				/* attempt to speed up */
			current = ftell(input);		/* maybe this slows it ? */
			continue;
		}
		if (strncmp(line, "%%Trailer", 9) == 0 ||
			strncmp(line, "%%EOF", 5) == 0) {
			trailer = current;
			break;
		}
		if (strncmp(line, "%%Page:", 7) == 0) {
/*			if (traceflag) fputs(line, stdout); */
			if (traceflag || showpageflag) fputs(line, stdout);
			if (sscanf(line+2, "Page: %s %d", label, &pageno) == 2) {
				if (pageno > 0 && pageno < MAXPAGES) {
					pagestarts[pageno] = current;
					pagelabels[pageno] = _strdup(label);
					if (sscanf(label, "%d-%d", &ca, &cb) == 2) {
						count1[pageno] = ca;
						count2[pageno] = cb;
					}
				}
			}
		}
		current = ftell(input);
	}
	if (trailer < 0) trailer = current;
}

void showpagetable(int flag) {
	int k;
	int pageno;
	for (k = 0; k < MAXPAGES; k++) {
		if (flag) pageno = index[k];		/* go via (sorted) index */
		else pageno = k;					/* go sequentially */
		if (pageno < 0) continue;			/* not valid */
		if (pagestarts[pageno] < 0) continue;
		printf("%d\t%ld\t%s\t", pageno, pagestarts[pageno], pagelabels[pageno]);
		if (count1[pageno] >= 0 && count2[pageno] >= 0)
			printf("%d-%d", count1[pageno], count2[pageno]);
		putc('\n', stdout);
	}
	if (trailer >= 0) printf("Trailer at %ld\n", trailer);
}

/* sort pages based on first argument of %%Page: assumed n-m form */

void resortpages (void) {
	int i, j;
	int pi, pj;
/*	int nswaps=0; */

	nswaps=0;
	for (i = 0; i < MAXPAGES-1; i++) {
		pi =  index[i];
		if (pi < 0) continue;
		if (count1[pi] < 0) continue;
		for (j = i+1; j < MAXPAGES; j++) {
			pj =  index[j];
			if (count1[pj] < 0) continue;
			if (pj < 0) continue;
			if ((count1[pi] > count1[pj]) ||
				((count1[pi] == count1[pj]) &&
					(count2[pi] > count2[pj]))) {
				if (debugflag) 
					printf("%d-%d (%d) %d-%d (%d)\n",
					   count1[pi], count2[pi], pi, count1[pj], count2[pj], pj);
				index[j] = pi;
				index[i] = pj;
				pi = index[i];
				pj = index[j];
				nswaps++;
			}					
		}
	}
	if (traceflag || showpageflag) printf("%d swaps\n", nswaps);
}

/****************************************************************************/

/* copy head of file up to first %%Page: DSC comment */

void copyheader (FILE *output, FILE *input) {
	long current;
	current = ftell(input);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == 12) {
			putc(12, output);
			current++;
			strcpy(line, line+1);
		}
		if (strncmp(line, "%%Page:", 7) == 0) break;
		if (strncmp(line, "%%Trailer", 9) == 0) break;
		if (strncmp(line, "%%EOF", 5) == 0) break;
		fputs(line, output);
		current = ftell(input);
	}
	fseek(input, current, SEEK_SET);
}

/* copy end of file */

void copytrailer (FILE *output, FILE *input) {
	while (fgets(line, sizeof(line), input) != NULL) {
		fputs(line, output);
	}
}

/* copy page with specified page number */

int copypageno (FILE *output, FILE *input, int pageno) {
	long start;

	if (verboseflag) printf("%d ", pageno);
	start = pagestarts[pageno];
	if (start < 0) return -1;
	if (fseek (input, start, SEEK_SET) < 0) return -1;
	fgets(line, sizeof(line), input);	/* copy first line no matter what */
	if (strncmp(line, "%%Page:", 7) != 0) return -1;
	fputs(line, output);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == 12) {
			putc(12, output);
			strcpy(line, line+1);
		}
		if (strncmp(line, "%%Page:", 7) == 0) break;
		if (strncmp(line, "%%Trailer", 9) == 0) break;
		if (strncmp(line, "%%EOF", 5) == 0) break;
		fputs(line, output);
	}
	return 0;
}

/* copy page with specified page number */

int copypagelabel (FILE *output, FILE *input, char *label) {
	int k;

	if (verboseflag) printf("%s ", label);
	for (k = 0; k < MAXPAGES; k++) {
		if (pagelabels[k] == NULL) continue;
		if (strcmp(pagelabels[k], label) == 0) break;
	}
	if (k >= MAXPAGES) return -1;
	return copypageno(output, input, k);
}

/* print pages in order as sorted on page label */

int dosortedpages (FILE *output, FILE *input) {
	int k, pageno;
	long start;

	if (verboseflag) printf("Doing sorted pages\n");
	for (k = 0; k < MAXPAGES; k++) {
		pageno = index[k];
		if (pageno < 0) continue;		/* not valid */
		start = pagestarts[pageno];
		if (start < 0) continue;
		if (copypageno(output, input, pageno) < 0) return -1;
	}
	return 0;
}

/* format of page specification file: */
/* 1, 3, 5, 2, 4, 6, 13-19, 53-59 */
/* can be spread over multiple lines if convenient */

void dospecifiedpages(FILE *output, FILE *input, FILE *pages) {
	char *s;
	int pagestart, pageend, k, n;

	if (verboseflag) printf("Doing specified pages\n");
	while (fgets(oldline, sizeof(oldline), pages) != NULL) {
		s = oldline;
		if (traceflag) printf("SPEC: %s", oldline); 
		while (sscanf(s, "%d%n", &pagestart, &n) > 0) {
			if (sscanf(s, "%d-%d%n", &pagestart, &pageend, &n) == 2) {
				if (pageend >= pagestart) {
					for (k = pagestart; k <= pageend; k++)
						copypageno(output, input, k);
				}
				else {
					for (k = pagestart; k >= pageend; k--)
						copypageno(output, input, k);
				}
			}
			else copypageno(output, input, pagestart);
			s += n;
			while (*s == ',' || *s == ' ' || *s == 9) s++;
			while (*s != '\0' && (*s < '0' || *s > '9') &&
				*s != '+' && *s != '-') s++;
		}
	}
}

/****************************************************************************/

void uppercase (char *str) {
	char *s=str;
	while (*s != '\0') {
		if (*s >= 'a' && *s <= 'z') *s = (char) (*s - ('a' - 'A'));
		s++;
	}
}

char *mapnames[][2] = {
	{"hplogo", "HPlogo"},
	{"hppi30", "HPPI30"},
	{"hpbats", "HPBATS"},
	{"hb", "HB"},
	{"keys10", "KEYS10"},
	{"manual", "MANUAL"},
	{"", ""}
};

/* /cmss8_1500_100 NF % /usr/contrib/hptag/fontbits/laser/300/cmss8.pk */

void analyzefont (FILE *output, char *str, int flag) {
	char *s, *t, *u=NULL;
	char name[32];
	char dpistr[32];
	int k, dpi;
	int ptsize=10;
	int magnif=0;
	int fontid=0;

	t = strchr(str, '%');
/*	replace two underscores with null */
	if ((s = strchr(str+1, '_')) != NULL) {
		*s = '\0';
		if ((t = strchr(s+1, '_')) != NULL) {
			*t = '\0';
			sscanf(t+1, "%d", &fontid);
		}
		else fprintf(stderr, "ERROR: Missing _ in %s", s+1);
		sscanf(s+1, "%d", &magnif);
		u = s-1;
		while (u > str && *u >= '0' && *u <= '9') u--;
		if (u < s-1) sscanf(u+1, "%d", &ptsize);
	}
	else fprintf(stderr, "ERROR: Missing _ in %s", str);
	strcpy(name, str+1);
/*	special case HPlogo pt size to be 38 pt */
	if (strncmp(name, "hplogo", 6) == 0) ptsize = 38;
/*	uppercase most names */
	if (strncmp(name, "cm", 2) == 0) uppercase(name);
	else if (strncmp(name, "dm10", 4) == 0) uppercase(name);
	else if (strncmp(name, "ml10", 4) == 0) uppercase(name);
	else if (strncmp(name, "wl10", 4) == 0) uppercase(name);
/*	Use Helvetica-Bold instead of Geneva Bold */
	else if (strncmp(name, "gvb", 3) == 0) strcpy(name, "Helvetica-Bold");
/*	Use table of translations from TFM name to PS FontName */
	else {
		for (k = 0; k < 32; k++) {
			if (strcmp(mapnames[k][0], "") == 0) break;
			if (strcmp(mapnames[k][0], name) == 0) {
				strcpy(name, mapnames[k][1]);
				break;
			}
		}
	}
	dpi = (magnif+3)/5;
	sprintf(dpistr, "/%d/", dpi);
	if (strstr(t+1, dpistr) == NULL)
		fprintf(stderr, "WARNING: mismatch in DPI (%d)\n", dpi); 
/*		fprintf(stderr, "WARNING: mismatch in DPI (%s)\n", dpistr); */
	if (verboseflag) {
		if (flag == 0) fputs("% ", output);
		fprintf(output, "/%s_%d_%d /%s %d %d DefFont\n",
			   str+1, magnif, fontid, name, dpi, ptsize);
	}
}

int skiptoendprolog (FILE *input) {
/*	Look for %%EndProlog */
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line != '%') continue; 
		if (strncmp(line, "%%EndProlog", 10) == 0) break;
	}
	if (*line != '%') {
		fprintf(stderr, "ERROR: Premature EOF looking for Prolog\n");
		return -1;
	}
	return 0;
}

/* we assume we are at %%EndProlog in input stream at this point */

int extractfonttable(FILE *output, FILE *input, int flag) {	/* 96/Mar/31 */
	int count=0;
	char *s;

	if (output == NULL) output = stdout;	/* happens if flag == 0 */

/*	if (flag == 0) putc('\n', output); */
/*	skip over TeXdict begin */
	fgets(line, sizeof(line), input);
/*	should check whether this is `TeXdict begin' */
	if (flag) fputs(line, output);
/*	skip over BOJ */
	fgets(line, sizeof(line), input);
/*	should check whether this is `BOJ' */
	if (flag) fputs(line, output);
/*	insert new PS code for /SF and /DefFont */
	if (flag) fputs(newcode, output);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (flag) fputs(line, output);
		if (*line == '/') break;
	}
/*	skip to after definition of /TB, /TE, /BB. and /BE using bdf */
	while (fgets(line, sizeof(line), input) != NULL) {
		if (flag) fputs(line, output);
/*		if (strstr(line, " bdf") == NULL) break; */
		if (strstr(line, " NF ") != NULL) break;	/* definition of BB */
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (flag) fputs(line, output);
		if (strstr(line, "/BE") != NULL) break;	/* definition of BE */
		if (strstr(line, " bdf") == NULL) break; /* emergency exit */
	}

	putc('\n', output);
/*	look for lines defining font --- ignore bitmap definitions */
	for (;;) {
		while (fgets(line, sizeof(line), input) != NULL) {
			if (strncmp(line, "%%Page:", 7) == 0) return count;
			current = ftell(input);		/* remember start of line */
			if (strchr(line, '/') == NULL) continue;
			s = strstr(line, " NF ");	/* look for font defs */
			if (s != NULL) {		/* scan back to / */
				if (flag) fputs("% ", output);
				while (s > line && *s != '/') s--;
				fputs(s, output);
				analyzefont (output, s, flag);
				fgets(line, sizeof(line), input);
				current = ftell(input);		/* remember start of line */
				fputs(line, output);
				putc('\n', output);
				count++;
			}
		}
	}
}

/****************************************************************************/

#ifdef IGNORED
int main (int argc, char *argv[]) {
	int k;
	FILE *input=NULL, *output=NULL, *pages=NULL;
	char infile[MAXFILENAME], outfile[MAXFILENAME], sectionfile[MAXFILENAME];
	int firstarg=1;

	for (k = 0; k < MAXPAGES; k++) pagestarts[k] = -1;
	for (k = 0; k < MAXPAGES; k++) pagelabels[k] = NULL;
	for (k = 0; k < MAXPAGES; k++) count1[k] = -1;
	for (k = 0; k < MAXPAGES; k++) count2[k] = -1;
	for (k = 0; k < MAXPAGES; k++) index[k] = k;

	if (argc < 2) exit(1);
	if (argc < firstarg+1) exit(1);
	if (argc < firstarg+2) ;
	else {
		strcpy(sectionfile, argv[firstarg+1]);
		if ((pages = fopen(sectionfile, "rb")) == NULL) {
			perror(sectionfile);
			exit(1);
		}
	}
	strcpy(infile, argv[firstarg]);
	extension(infile, "ps");
	if ((input = fopen(infile, "rb")) == NULL) {
		perror(infile);
		exit(1);
	}
	buildpagetable(input);
	if (traceflag) showpagetable();
	resortpages();
	rewind(input);
	strcpy(outfile, removepath(infile));
	forceexten(outfile, "pso");
	if (strcmp(infile, outfile) == 0) {
		printf("In file same as out file\n");
		exit(1);
	}
	if ((output = fopen(outfile, "wb")) == NULL) {
		perror(outfile);
		exit(1);
	}	
	copyheader(output, input);
	if (pages == NULL) {
		dosortedpages(output, input);
	}
	else {
		dospecifiedpages(output, input, pages);
	}
	fseek(input, trailer, SEEK_SET);
	copytrailer(output, input);
	fclose(output);
	fclose(input);
	for (k = 0; k < MAXPAGES; k++)
		if (pagelabels[k] != NULL) free(pagelabels[k]);
	return 0;
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage (char *name) {
	printf("Usage:\n");
	printf(
"%s [-v] [-d] [-l] [-g] [-r] [-2] [-3] [-R] [-F] [-h=...] [-e=...] [-o=...] <PS file>\n",
		   name);
	printf("\n");
	printf("\t-v verbose mode\n");
	if (darkenflag)
		printf("\t-d do not `darken' image / `smear' lines\n");
	else printf("\t-d `darken' image / `thicken' lines\n");
	if (ligexpandflag)
		printf("\t-l do not expand extra ligatures (ff, ffi, ffl)\n");
	else printf("\t-l expand extra ligatures (ff, ffi, ffl)\n");
	if (greekreplace) 
		printf("\t-g do not replace uppercase upright Greek letters\n");
	else printf("\t-g replace uppercase upright Greek letters\n");
	if (reorderflag) 
		printf("\t-r do not rearrange page order\n");
	else printf("\t-r reorder pages based on DSC comments\n");
	printf("\t-2 Operate on 2 x 2 pixel blocks ");
	if (smear2x2flag) printf("(default)\n");
	else printf("\n");
	printf("\t-3 Operate on 3 x 3 pixel blocks ");
	if (smear3x3flag) printf("(default)\n");
	else printf("\n");
/*	printf("\t-s sub sample (off by default)\n"); */
	printf("\t-R show page table in HP TAG PS file\n");
	printf("\t-F show font table in HP TAG PS file\n");
	printf("\t-h threshold (default %d)\n", threshold);
	printf("\t-e extension for output file (default `.%s')\n", exten);
	printf("\t-o section order file\n");
	printf("\n");
/*	printf("\t   Input must be a PS file made by DVIPSONE\n"); */
	printf("\t   Input is normally a PS file made by DVIPSONE\n");
/*	printf("\n"); */
	printf("\t   Output appears in current directory (default `.%s')", exten);	
	exit(1);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* still available flags: g, n, o */

int decodeflag (int c) {
/*	printf ("FLAG: %c%n", c); */
	switch(c) {
		case '?': detailflag = 1; return 0;
		case 'v': verboseflag = !verboseflag; return 0;
		case 't': traceflag = !traceflag; return 0;
		case 'd': darkenflag = !darkenflag; return 0;
		case 'l': ligexpandflag = !ligexpandflag; return 0;
		case 'g': greekreplace = !greekreplace; return 0;
		case 'f': strippkfonts = !strippkfonts; return 0;
		case 'r': reorderflag = !reorderflag; return 0;
		case 'R': showpageflag = !showpageflag; return 0;
		case 'F': showfontsflag = !showfontsflag; return 0;
		case 'D': debugflag = !debugflag; return 0;
		case 'a': suppressshort = !suppressshort; return 0;
		case 's': subsample = !subsample; return 0;
		case '1': smear2x2flag = 0;	smear3x3flag = 0; return 0;
		case '2': smear2x2flag = 1;	smear3x3flag = 0; return 0;
		case '3': smear2x2flag = 0;	smear3x3flag = 1; return 0;
/* following take arguments */
		case 'h': thresholdflag = 1; break;
		case 'e': extensionflag = 1; break;
		case 'o': sectionfileflag = 1; break;
		default: {
			 fprintf(stderr, "Invalid command line flag '%c'", c);
			 exit(1);
				 }
	}
	return -1;		/* need argument */
}

/* Flags and Arguments start with `-' */
/* Also allow use of `/' for convenience */
/* Normal use of `=' for command line arguments */
/* Also allow use of `:' for convenience */
/* Archaic: use space to separate - only for backward compatability */

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
					fprintf(stderr, "Don't understand: %s\n", command);
					return firstarg;
				}
			}
/*			else s++;	*/		/* when `=' IS used */
			else s = sarg+1;	/* when `=' or `:' IS used */

/* now analyze the various flags that could have gotten set */
			if (thresholdflag != 0) {
				sscanf(s, "%d", &threshold);
				thresholdflag = 0;
			}
			else if (extensionflag != 0) {
				exten = s;
				extensionflag = 0;
			}
			else if (sectionfileflag != 0) {
				sectionfilename = s;
				sectionorder = 1;
				reorderflag = 1;		/* for now */
				sectionfileflag = 0;
			}
			break;	/* default - no flag set */
		}
	}
	return firstarg;
}

/*	check command line flags and command line arguments */

int commandline (int argc, char *argv[], int firstarg) {
	int c;
	
	if (argc < firstarg+1) showusage(argv[0]);
	c = firstarg < argc && argv[firstarg][0];
	while (c == '-' || c == '/') {
		firstarg = decodearg(argv[firstarg], argv[firstarg+1], firstarg+1);
		if (firstarg >= argc) break;			/* safety valve */
		c = argv[firstarg][0];
	}
	if (argc < firstarg+1) showusage(argv[0]);
	return firstarg;
}

#ifdef IGNORED
	while (*argv[firstarg] == '-') {
		if (strcmp(argv[firstarg], "-?") == 0) {	/* usage */
			showusage(argv[0]);
		}
		else if (strcmp(argv[firstarg], "-1") == 0) {	/* no smearing ? */
			smear2x2flag = 0;	smear3x3flag = 0;
		}
		else if (strcmp(argv[firstarg], "-2") == 0) {	/* 2 x 2 smearing */
			smear2x2flag = 1;	smear3x3flag = 0;
		}
		else if (strcmp(argv[firstarg], "-3") == 0) {	/* 3 x 3 smearing */
			smear2x2flag = 0;	smear3x3flag = 1;
		}
		else if (strcmp(argv[firstarg], "-s") == 0) {	/* subsample */
			subsample = !subsample;
		}
		else if (strcmp(argv[firstarg], "-t") == 0) {	/* trace mode */
			traceflag = !traceflag;
		}
		else if (strcmp(argv[firstarg], "-D") == 0) {	/* debug mode */
			debugflag = !debugflag;
		}
		else if (strcmp(argv[firstarg], "-v") == 0) {	/* verbose mode */
			verboseflag = !verboseflag;
		}
		else if (strcmp(argv[firstarg], "-a") == 0) {	/* short strings */
			suppressshort = !suppressshort;
		}
		else if (strcmp(argv[firstarg], "-g") == 0) {	/* Greek letters */
			greekreplace = !greekreplace;
		}
		else if (strcmp(argv[firstarg], "-r") == 0) {	/* reorder pages */
			reorderflag = !reorderflag;
		}
		else if (strcmp(argv[firstarg], "-d") == 0) {	/* darken mode */
			darkenflag = !darkenflag;
		}
		else if (strcmp(argv[firstarg], "-l") == 0) {	/* ligature expand */
			ligexpandflag = !ligexpandflag;
		}
		else if (strncmp(argv[firstarg], "-h", 2) == 0) {	/* set threshold */
			if (sscanf(argv[firstarg], "-h=%d", &threshold) == 0) {
				if (firstarg+1 < argc && *argv[firstarg+1] != '-' &&
					sscanf(argv[firstarg+1], "%d", &threshold) != 0) {
					firstarg++;
				}
				else threshold = 0;
			}
		}
		else if (strncmp(argv[firstarg], "-e", 2) == 0) {	/* set extension */
			if (sscanf(argv[firstarg], "-e=%s", &exten) == 0) {
				if (firstarg+1 < argc && *argv[firstarg+1] != '-' &&
					sscanf(argv[firstarg+1], "%s", &exten) != 0) {
					firstarg++;
				}
				else strcpy(exten, "ps");
			}
		}
		else if (strncmp(argv[firstarg], "-p", 2) == 0) {	/* set page file */
			if (strlen(argv[firstarg]) > 2) {
				sectionfilename= argv[firstarg]+3;
			}
			else if (firstarg+1 < argc) {
				sectionfilename = argv[firstarg+1];
				firstarg++;
			}
			else sectionfilename = "";
		}
		firstarg++;
	}
#endif

int readsectionfile (char *name) {
	FILE *input;
	char *s;
	int pagestart, pageend, k, n;
	
	if ((input = fopen(name, "r")) == NULL) {
		perror(name);
		return -1;
	}
	k = 0;
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		s = line;
/*		if (traceflag) printf("SPEC: %s ", line); */
		while (sscanf(s, "%d%n", &pagestart, &n) > 0) {
/*			if (traceflag) printf("NEXT: %s ", s); */
			if (sscanf(s, "%d-%d%n", &pagestart, &pageend, &n) == 2) {
				sectionstart[k] = pagestart;
				sectionend[k] = pageend;
			}
			else {
				sectionstart[k] = pagestart;
				sectionend[k] = pagestart;
			}
			k++;
			if (k >= MAXSECTIONS) {
				fprintf(stderr, "ERROR: Too many section ranges!\n");;
				k--;
			}
			s += n;
			while (*s == ',' || *s == ' ' || *s == 9) s++;
			while (*s != '\0' && (*s < '0' || *s > '9') &&
				*s != '+' && *s != '-') s++;
		}
	}
	if (traceflag) printf("\n");
	fclose(input);
	nsections=k;
	if (traceflag) {
		printf("%d section ranges:\n", nsections);
		for (k = 0; k < nsections; k++) 
			printf("%d\tsection range %d-%d\n",
				   k, sectionstart[k], sectionend[k]);
	}
	return 0;
}

/* rearrange index based on section table */

int reindex (void) {
	int k, i, j;
	int pagestart, pageend;
	int kk = 0;							/* pointer into index array */

	if (traceflag) printf("%d section ranges to step through\n", nsections);
/*	step through all specified section ranges */
	for (k = 0; k < nsections; k++) {
		pagestart = sectionstart[k];
		pageend = sectionend[k];
		if (traceflag)
		printf("section range %d-%d\n", sectionstart[k], sectionend[k]);
/*	step through all sections within a given range */
		for (i = pagestart; i <= pageend; i++) {
/*	i is the number of the section to consider now */
			for (j = 0; j < MAXPAGES; j++) {
/*	include this page at this point if it is in this section */
				if (count1[j] == i) {
					index[kk++] = j;
					if (traceflag) printf("%d\t%d\n", kk-1, j);
				}
			}
		}
	}
	if (traceflag) printf("Found %d pages in given sections\n", kk);
	for (k = kk; k < MAXPAGES; k++) index[k] = -1;	/* mark as not used */
	return 0;
}

int main(int argc, char *argv[]) {
	FILE *input=NULL;
	FILE *output=NULL;
	char infile[MAXFILENAME];
	char bakfile[MAXFILENAME];
	char outfile[MAXFILENAME];
	int m, flag, firstarg=1;

/*	Three programs in one!  Depending on what it is called */

	if (_strnicmp(argv[0], "darken", 6) == 0) darkenflag=1;
	if (_strnicmp(argv[0], "ligexpnd", 8) == 0) ligexpandflag=1;	
	if (_strnicmp(argv[0], "reorder", 7) == 0) reorderflag=1;	

	if (argc < 2) showusage(argv[0]);

	firstarg = commandline(argc, argv, firstarg); /* check for command flags */

/*	2 x 2 and 3 x 3 mutually exclusive */
	if (smear2x2flag) smear3x3flag=0;
/*	makes no sense to subsample when not smearing */
	if (!smear2x2flag && !smear3x3flag) subsample=0;
/*	threshold must be less than or equal to window size */
	if (smear2x2flag && threshold > 4) threshold=4;
	else if (smear3x3flag && threshold > 9) threshold=9;

	if (showpageflag) {
		darkenflag = 0;
		ligexpandflag = 0;
		greekreplace = 0;
		reorderflag = 1;
		verboseflag = 1;
/*		traceflag = 1; */
	}

	if (showfontsflag) {
		darkenflag = 0;
		ligexpandflag = 0;
		greekreplace = 0;
		reorderflag = 1;
		verboseflag = 1;
/*		traceflag = 1; */
	}

	if (strippkfonts) {
		ligexpandflag = 0;
		greekreplace = 0;
	}

	if (!darkenflag && !ligexpandflag && !greekreplace &&
		!reorderflag && !strippkfonts)
			printf(
"Nothing to do (no darken, no ligature expand, no font replace, no reorder)?\n");

	if (argc < firstarg+1) showusage(argv[0]);	/* mention a file at least */

	nsections=0;

/*	if (*sectionfilename != '\0') { */
	if (sectionorder) {
		if (readsectionfile(sectionfilename) != 0) exit(1);
	}

	putc('\n', stdout);
/*	printf("POSTPROC version 1.1  Y&Y, Inc.  (508) 371-3286\n"); */
	printf("POSTPROC version 1.1  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\n");
	putc('\n', stdout);

	nfiles=0;
	for (m=firstarg; m < argc; m++) {
/*	strcpy(infile, argv[firstarg]); */
	strcpy(infile, argv[m]);
	extension(infile, "ps");

	strcpy(outfile, removepath(infile));
	forceexten(outfile, exten);
	if (strcmp(infile, outfile) == 0) {
		if (verboseflag)
			printf("Output file (%s) same as input file (%s)\n",
			   outfile, infile);
		strcpy(bakfile, infile);
		forceexten(infile, "bak");
		if (verboseflag)
			printf("Renaming %s to be %s\n", bakfile, infile);
		(void) remove (infile);				/* remove `foo.bak' if it exists */
		(void) rename (bakfile, infile);	/* rename `foo.ps' to `foo.bak' */
/*		exit(1); */
		renameflag = 1;
	}
	else renameflag = 0;
	
/*	if ((input = fopen(infile, "r")) == NULL) {  */
	if ((input = fopen(infile, "rb")) == NULL) {  /* ??? */
		perror(infile);
/*		exit(1); */
		continue;
	}
/*	if (showpageflag == 0) { */
	if (showpageflag == 0 && showfontsflag == 0) {
/*		if ((output = fopen(outfile, "w")) == NULL) { */
		if ((output = fopen(outfile, "wb")) == NULL) { /* ??? */
			perror(outfile);
			fclose(input);
/*			exit(1); */
			continue;
		}
	}
	else output = NULL;

	if (verboseflag) {
/*		printf("Reading `%s', writing `%s'\n", infile, outfile); */
		printf("Reading `%s'", infile);
/*		if (showpageflag == 0) */
		if (showpageflag == 0 && showfontsflag == 0)
			printf(", writing `%s'", outfile);
		printf("\n");
		if (darkenflag) {
			printf("* Darkening lines:\t");
			if (smear2x2flag) printf("Operating on 2 x 2 pixel blocks ");
			else if (smear3x3flag) printf("Operating on 3 x 3 pixel blocks ");
			if (threshold) printf("--- threshold is %d ", threshold);
			if (subsample) printf("--- subsampling ");
			putc('\n', stdout);
		}
		if (ligexpandflag) printf("* Expanding ff, ffi, ffl ligatures\n");
		if (greekreplace)
			printf("* Borrowing upright upper case Greek from Symbol\n");
/*		if (showpageflag == 0) */
		if (showpageflag == 0 && showfontsflag == 0) {
			if (reorderflag) printf("* Rearranging page order\n");
		}
/*		if (strippkfonts) printf("* Replacing bitmapped fonts\n"); */
		if (strippkfonts) printf("* Stripping bitmapped fonts\n");
		if (showpageflag) printf("* Showing page table\n");
		if (showfontsflag) printf("* Showing old font table\n");
	}
	if (showfontsflag) {
		skiptoendprolog (input);						/* 96/Mar/31 */
		flag = extractfonttable(output, input, 0);		/* 96/Mar/31 */
		if (verboseflag) printf("Seen %d font definitions\n", flag);
		flag = 0;
	}
	else {
/*		flag = processfile(output, input); */
		flag = processfile(output, input, showpageflag);
	}
	if (output != NULL) fclose (output);
	fclose (input);
	freefontnames();
/*	if (verboseflag) */
	if (traceflag) printf("\n");
/*	if (showpageflag == 0)  */
	if (showpageflag == 0 && showfontsflag == 0) 
		printf("Output in `%s'\n", outfile);
	if (nimages > 0) printf("* `Darkened' %d images\n", nimages);
/*	if (nfi > 0 || nfl > 0 || nff > 0 || nffi > 0 || nffl > 0)
		printf("Seen %d  fi %d fl %d ff %d ffi %d ffl\n",
			   nfi, nfl, nff, nffi, nffl); */
	if (nff > 0 || nffi > 0 || nffl > 0)
		printf("* Expanded %d ff's, %d ffi's, %d ffl's,\n", nff, nffi, nffl);
	if (ngreek > 0)
		printf("* Replaced %d upright upper case Greek letters\n", ngreek);
	if (showpageflag == 0 && showfontsflag == 0) {
		if (reorderflag) printf("* Reordered pages\n");
	}
	if (strippkfonts) printf("* Stripped bitmapped fonts\n");
	if (nshort > 0)
		printf("Ignored %d short strings\n", nshort);
	if (renameflag) {
		if (flag) remove(infile);	/* delete `foo.bak' */
		else {
			printf("Renaming %s to be %s\n", infile, bakfile);
			remove (bakfile);		/* delete `foo.ps' */
			rename (infile, bakfile);	/* rename `foo.bak' to `foo.ps' */
		}
	}
	}	/* end of loop over files */
	if (nfiles > 1) printf("Processed %d files\n", nfiles);
	return 0;
}

/*

/SF{
load	% if given /fontname instead of fontname directly
exch pop setfont
} bdf

/DefFont {	% /fontname  /FontName  resolution(dpi)  ptsize
exch 
% Resolution div mul % e.g. 329 / 300 * 8pt
% Resolution 72 div mul	% undo current scaling
72 div mul % resolution * ptsize / 72
exch findfont exch scalefont def
} bdf

*/
