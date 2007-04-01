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

/* Program to process conforming PS file to print two up format */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* #include <conio.h> */
#include <signal.h>
/* #include <errno.h> */

/* #define FNAMELEN 80 */
/* #define MAXPATHLEN 64 */		/* maximum path name length */
#define MAXLINEIN 4096		/* input buffer for input file */
#define MAXLINEOUT 128		/* output buffer for TWOUP generated lines */
#define MAXPAGES 4096
#define INFINITY 32767

#define CONTROLBREAK

struct coord {				/* structure for a coordinate pair */
	double w; double h;
};

double outpageheight = 11.0 * 72;
double outpagewidth = 8.5 * 72 ;
double inpageheight = 11.0 * 72;
double inpagewidth = 8.5 * 72 ;

double magnification = 1.0;	/* user specified magnification */

double zoom = 0.0;			/* user specified ABSOLUTE magnification */

double xoffset = 0;
double yoffset = 0;

/* following imported from DVIPSONE 1994/Jan/22 */

#define UNKNOWNOFFSET -32767.0

double xoffsete=UNKNOWNOFFSET;			/* x offset even pages */
double yoffsete=UNKNOWNOFFSET;			/* y offset even pages*/

double xoffseto=UNKNOWNOFFSET;			/* x offset odd pages */
double yoffseto=UNKNOWNOFFSET;			/* y offset odd pages */

int evenoddoff = 0;			/* non-zero if offset differs even/odd pages */

int verboseflag = 0;
int traceflag = 0;
int inlineflag=1;		/* put code in line instead of header */
int extrasafe=1;		/* put save restore around each page */
int wantcpyrght=1;		/* want copyright message in output file */
int currentdirect=1;	/* put in current directory */
int wantcontrold=1;		/* want control-D in printer output */
int retplusnew=0;		/* non-zero => use `return' + `newline' */
/* actually:	if (retplusnew != 0) mode = "w"; else mode = "wb"; */
int alternateflag=0;	/* invert alternate pages output */
/* int tripout=0; */	/* trip out on %%Trailer and %%EOF */
int tripout=1;			/* trip out on %%Trailer and %%EOF */
						/* useful for output from Nelson Beebe's lptops */
/* which does a `restore' in the Trailer to match a `save' in Prolog */
/* but dangerous otherwise, since included EPS files may have this */

int prologflag=0;		/* next arg => prolog file */
int epsflag=0;			/* next arg => eps file path */

int evenoddflag=0;		/* > 0 print odd on right side, even on left */
						/* < 0 print even on right side, odd on left */
int secondflag=0;		/* use second number of %%Page: comments */
int invertflag=0;		/* invert pages (i.e. rotate 180 degrees) */

int reverseflag=0;		/* invert the order of pages --- 95/June/23 */

int saddleflag=0;		/* print for saddle backstapling */
int evenflag = 0;		/* print (1, 2n) (3, 2n-2) */
int oddflag = 0;		/* print (2, 2n-1) (4, 2n-3) */
int duplexflag = 0;		/* ask printer to go into duplex mode */
int tumbleflag = 0;		/* ask printer to tumble in duplex 96/Dec/20 */
int copies=1;			/* number of copies to print */

int directprint=0;		/* output directly to printer */

volatile int bAbort=0;			/* set by user typing control-C */

int detailflag = 0;
int magnifyflag = 0;
int zoomflag = 0;
int xoffsetflag = 0;
int yoffsetflag = 0;
int copiesflag = 0;
int outputflag = 0;
int lpaperflag = 0;
int ppaperflag = 0;

int knext;				/* slots used in page table */
int pagemin;			/* min page number seen in prescan */
int pagemax;			/* max page number seen in prescan */
int nstart;				/* start page in saddle back printing */
int nfinish;			/* end page in saddle back printing */
int ncenter;			/* n in saddle back printing (center page left) */
int npages;				/* number of sides to print in saddles */

int startup;			/* non-zero before first page seen */

/* double scale; */		/* computed from in and out page format */

double scalefactor;		/* magnification * scale */

char *prologfile = "";		/* user prolog file or empty */
char *outputfile = "";		/* output file or empty */
char *lpapertype = "";		/* "letter" or "" ? */
char *ppapertype = "";		/* "letter" or "" ? */

char *epspath="";			/* dummy */

char dvipath[FILENAME_MAX]="";		/* pathname of dvi file - command */

static FILE *input, *output; 
static char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX]; 

/* char *programversion = "TWOUP version 0.7"; */
/* char *programversion = "TWOUP version 0.8"; */
/* char *programversion = "TWOUP version 0.8.1"; */
/* char *programversion = "TWOUP version 0.9"; */
/* char *programversion = "TWOUP version 0.9.1"; */
/* char *programversion = "TWOUP version 0.9.9"; */
/* char *programversion = "TWOUP version 1.0.1"; */
/* char *programversion = "TWOUP version 1.0.2"; */
/* char *programversion = "TWOUP version 1.1"; */	/* 1994/June/9 */
/* char *programversion = "TWOUP version 1.2"; */	/* 1995/June/23 */
/* char *programversion = "TWOUP version 1.3"; */	/* 1996/Aug/12 */
/* char *programversion = "TWOUP version 1.3.1"; */	/* 1996/Dec/19 */
/* char *programversion = "TWOUP version 1.3.2"; */	/* 1997/Nov/17 */
char *programversion = "TWOUP version 1.3.3";		/* 1998/Nov/27 */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* #define COPYHASH 2670755 */
/* #define COPYHASH 14251529 */
/* #define COPYHASH 8124949 */
/* #define COPYHASH 13279621 */
/* #define COPYHASH 2385634 */
/* #define COPYHASH 8268863 */
/* #define COPYHASH 14507841 */
/* #define COPYHASH 11957132 */
#define COPYHASH 5001165

char *copyright = "\
Copyright (C) 1990-1998  Y&Y, Inc. (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1990-1996  Y&Y, Inc. All rights reserved. (978) 371-3286\ */

static char line[MAXLINEIN];

static char buffer[MAXLINEOUT];

static int pagenumber[MAXPAGES];
static long pagebegin[MAXPAGES];

/* to try and get around obnoxious Adobe PostScript `invalidrestore' error */

static char *header = "\
/pagestart{ } def\n\
/pageend{ showpage } def\n\
/leftstart { /twoupstate save def /showpage {} def \n\
OutPageWidth 2 div OutPageHeight 4 div translate\n\
[0 1 -1 0 0 0] concat % 90 rotate\n\
scalefactor dup scale\n\
InPageWidth 2 div neg InPageHeight 2 div neg translate\n\
horizontaloff verticaloff translate\n\
} def\n\
/leftend { twoupstate restore } def\n\
/rightstart { /towupstate save def /showpage {} def \n\
OutPageWidth 2 div OutPageHeight 4 div 3 mul translate\n\
[0 1 -1 0 0 0] concat % 90 rotate\n\
scalefactor dup scale\n\
InPageWidth 2 div neg InPageHeight 2 div neg translate\n\
horizontaloff neg verticaloff translate\n\
} def\n\
/rightend { twoupstate restore } def\n\
"; 

/* static char *duplexcode=
"statusdict /setduplex known {statusdict begin true setduplex end} if\n\
"; */	/* removed 96/Dec/19 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void newexten(char *fname, int outflag) {	/* change extension if present */
	char *s;

	s = stripname(fname);		/* 1995/July/18 */
	if (outflag) {
/*		if user specifies file *without* extension, use "ps2" */
		if ((s = strrchr(fname, '.')) == NULL) {
			strcat(fname, "."); strcat(fname, "ps2");
		}
/*		if user explicitly specified output file extension, keep it */
	}
	else {
		if ((s = strrchr(fname, '.')) == NULL) {
			strcat(fname, "."); strcat(fname, "ps2");
		}
		else if (strcmp(s + 1, "ps") == 0) strcpy(s+1, "ps2");
		else if (strcmp(s + 1, "ps2") == 0) strcpy(s+1, "ps4");	
		else if (strcmp(s + 1, "ps4") == 0) strcpy(s+1, "ps8");	
		else if (strcmp(s + 1, "ps8") == 0) strcpy(s+1, "p16");	
		else strcpy(s+1, "ps2");
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Fiddle with line termination (output \r\n), if retplusnew == 0 */

int fputsmod (char *text, FILE *output) {
	int n;
	char *s;
	
	if (retplusnew != 0) return fputs(text, output);
/*	if output open in "wb" mode may want to add `return' before `newline' */
/*	but only if the line actually ends on `newline' ... */
	s = text + strlen(text) - 1;
	if (*s == '\n') {
		*s = '\0';					/* flush newline for now */
		n = fputs(text, output);
		putc('\r', output);			/* return */
		putc('\n', output);			/* newline */
		*s = '\n';					/* put back newline now */
	}
	else {
		n = fputs(text, output);
		fprintf(stderr, "ERROR: no newline (%d): %s\n", *s, text);
	}
	return n;
}
 
int fputsmulti (char *text, FILE *output) {
	char *s=text;
	int c;
	
	if (retplusnew != 0) return fputs (text, output);
/*	if output open in "wb" mode may want to add `return' after `newline' */
	while ((c = *s++) != '\0') {
		if (c == '\n') putc ('\r', output);	/* insert `return' here */
		putc (c, output);
	}
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef CONTROLBREAK
void cleanup(void) {
	if (output != NULL) {
		if (directprint != 0) {
			if (wantcontrold != 0) {
/*				putc(3, output); */			/* send control-C */
				putc(4, output);			/* send control-D */
			}
			fclose(output);				/* close output */
		}
		else {
			fclose(output);				/* close output */
			(void) remove(fn_out);		/* and remove bad file */
		}
	}
/*	fcloseall();  */
}
#endif

void abortjob (void) {
	cleanup();
	exit(3);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Extract next pathname from a searchpath - and write into pathname */
/* return NULL if there are no more pathnames, */
/* otherwise returns pointer to NEXT pathname in searchpath */
/* searchpath = pathname1;pathname2; ... ;pathnamen */

/* used for pfb search path and eps search path */
/* this version also allows space as separator */

char *nextpathname(char *pathname, char *searchpath) {
	int n;
	char *s;

	if (*searchpath == '\0') return NULL;	/* nothing left */
	else if (((s = strchr(searchpath, ';')) != NULL) ||
		     ((s = strchr(searchpath, ' ')) != NULL)) {
		n = (s - searchpath);
		if (n >= FILENAME_MAX) {
			fprintf(stderr, "Path too long: %s\n", searchpath);
			return NULL;
		}
		strncpy(pathname, searchpath, (unsigned int) n);
		*(pathname + n) = '\0';				/* terminate it */
		return(s + 1);						/* next pathname or NULL */
	}
	else {
		n = (int) strlen(searchpath);
		if (n >= FILENAME_MAX) {
			fprintf(stderr, "Path too long: %s\n", searchpath);
			return NULL;
		}
		strcpy(pathname, searchpath);
		return(searchpath + n);
	}
}

/* FINDEPSFILE:  Try to open the file to be included (or overlayed) */

FILE *findepsfile(char *name, int warnflag) {
	FILE *special=NULL;
	char *s, *searchpath, *epsname;
	int foundfile = 0;
	char epsfilename[FILENAME_MAX];			/* NEW */
	char filename[FILENAME_MAX];

	strcpy(epsfilename, name);			/*  filename as given */
	extension(epsfilename, "eps");		/*  add extension if none there */
	epsname = stripname(epsfilename);	/*	strip path from eps file name */

/*	printf("epsfilename  %s epsname = %s\n", epsfilename, epsname);  */

/*  maybe consider only fully qualified if it contains `:' ??? */
/*	if fully qualified name, try that first */
	if (strchr(epsfilename, '\\') != NULL ||
		strchr(epsfilename, '/') != NULL ||
		strchr(epsfilename, ':') != NULL) {		/* fully qualified name */
		strcpy(filename, epsfilename);			/* try using as is ! */
		if ((special = fopen(filename, "rb")) != NULL) foundfile = 1; 
	}

/*	if not successful, try each path in EPSPATH in turn */
	if (foundfile == 0 && strcmp(epspath, "") != 0) {
		searchpath = epspath;
		for(;;) {
			if((searchpath=nextpathname(filename, searchpath)) == NULL) {
				foundfile = 0; break;
			}
/*			printf("NEXTPATH %s %d", filename, strlen(filename)); */
			s = filename + strlen(filename) - 1;
			if (*s != '\\' && *s != '/') strcat(filename, "\\"); 
			strcat(filename, epsname);
/*			extension(epsfilename, "eps"); */
			if ((special = fopen(filename, "rb")) != NULL) { /* "r" ? */
				foundfile = 1; break;
			}
		}
	}

	if (foundfile == 0) {				/* if not found on search path */
		strcpy(filename, dvipath);		/* try in directory of dvi file */
		if (*filename != '\0') {
			s = filename + strlen(filename) - 1;
			if (*s != '\\' && *s != '/') strcat(filename, "\\");
		}
		strcat(filename, epsfilename);		/* 1992/May/05 */
		if ((special = fopen(filename, "rb")) != NULL) 	foundfile = 1; 
	}

	if (foundfile == 0 && strcmp(epsfilename, epsname) != 0) {
		strcpy(filename, dvipath);	/* try in directory of dvi file */
		if (*filename != '\0') {
			s = filename + strlen(filename) - 1;
			if (*s != '\\' && *s != '/') strcat(filename, "\\");
		}
		strcat(filename, epsname);			/* try qualified name */
		if ((special = fopen(filename, "rb")) != NULL) foundfile = 1; 
	}	

	if (foundfile == 0) {
		strcpy(filename, epsfilename);	/* try in current directory */
		if ((special = fopen(filename, "rb")) != NULL) foundfile = 1;
	}

	if (foundfile == 0 && strcmp(epsfilename, epsname) != 0) {
		strcpy(filename, epsname);	/* try in current directory */
		if ((special = fopen(filename, "rb")) != NULL) 	foundfile = 1;
	}
	
	if (foundfile == 0) {
		if (warnflag != 0) {
			fprintf(stderr, " Can't find file: %s", epsname);
			/* file to be inserted */	/*	perror(epsname); */
/*			errcount(); */
		}
		return NULL;
	}

	if (verboseflag != 0) 
		printf(" %s", filename);				/* announce it */
/*	printf("- FILENAME %s - EPSNAME %s - ", filename, epsname); */
	return special;
}

void copyprologfile(char *filename, FILE *outfile) {
	FILE *infile;
	int c;

	if (verboseflag != 0) printf("[Header"); /* putc('[', stdout); */
	if ((infile = findepsfile(filename, 1)) == NULL) {
/*	if ((infile = findepsfile(filename)) == NULL) { */
/*		fprintf(stderr, "Can't find prolog file ");
		perror(filename); errcount(); */
	}
	else {
		if (directprint == 0) {
/*			fprintf(outfile, "%%%%BeginProcSet: \"%s\" \"\" \"\"\n",  */
			sprintf(buffer, "%%%%BeginResource: procset %s\n",
				filename);   /* should have version and revision? */
			fputsmod(buffer, output);
		}
/*		fputsmod("dvidict begin\n", outfile); */	/* flushed 96/Dec/19 */
		while ((c = getc(infile)) != EOF) {
			if (c != 4) putc(c, outfile);			/* ignore ^D 94/Feb/21 */
			if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
		}
		fclose(infile);
		fputsmod ("\n", outfile);
/*		fputsmod ("end % dvidict\n", outfile); */	/* flushed 96/Dec/19 */

		if (directprint == 0) {
/*			fprintf(outfile, "%%%%EndProcSet\n"); */
/*			fprintf(outfile, "%%%%EndResource\n");  */
			fputsmod("%%EndResource\n", outfile);  		/* 1992/July/18 */
		}
	}
	if (verboseflag != 0) printf("] ");		/* putc(']', stdout); */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* copy input up to structuring comment */ /* does not copy the comment */
/* watch for ^L and other nonsense ! 94/Feb/21 */

int scantocomment(FILE * input, FILE *output, int flag) {
	for(;;) {
		if (fgets(line, MAXLINEIN, input) == NULL) {
/*			fprintf(stderr, "Unexpected EOF\n"); */		/* removed ??? */
			return -1;
		}
		if (*line == '\f') strcpy(line, line+1);	/* flush ^L! 94/Feb/21 */
		if (*line == 4)	strcpy(line, line+1);		/* flush ^D! 94/Feb/21 */

		if (*line == '%' && *(line + 1) == '%') return 0;	/* %% comment */
		if (flag != 0) {
			if (fputs(line, output) == EOF) {	/* copy input as is */
				perror(" Output error");
				return -1;
			}
		}
		if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
	}
}

/* copy input up to structuring comment containing %%Page */
/* (this may start right after previous %%Page ... */
/* return -INFINITY if real EOF */
/* return -INFINITY if %%Trailer %%EOF (not nested) */ /* NOT ANYMORE ? */

int scantopage(FILE * input, FILE *output, int flag) {
	char label[FILENAME_MAX];
	char fontname[FILENAME_MAX];
	int pageno, page1, page2, ignore;
	int insidefont=0;
	int nestinglevel=0;								/* NEW */

	for(;;) {
		ignore = 0;
		if (scantocomment(input, output, flag) < 0) 
			return -INFINITY;								/* hit EOF */
		else if (strncmp(line + 2, "BeginFont", 9) == 0) {
			if (verboseflag != 0) {
				putc('[', stdout);
				if(sscanf(line + 2 + 10, "%s", fontname) > 0) 
					printf("%s", fontname);
			}
			else putc('*', stdout);
		}
		else if(strncmp(line + 2, "BeginResource: font", 19) == 0) {
			insidefont = 1;		/* flag that font Resource is being scanned */
			if (verboseflag != 0) {
				putc('[', stdout);
				if(sscanf(line + 2 + 20, "%s", fontname) > 0) 
					printf("%s", fontname);
			}
			else putc('*', stdout);
		}
		else if (strncmp(line + 2, "EndFont", 7) == 0) {
			if (verboseflag != 0) printf("] ");
		}
		else if (strncmp(line + 2, "EndResource", 11) == 0) {
			if (insidefont != 0) {
				if (verboseflag != 0) printf("] ");
				insidefont = 0;
			}
		}
		else if (strncmp(line + 2, "BeginDocument", 13) == 0) 
			nestinglevel++;	 /* NEW */
		else if (strncmp(line + 2, "EndDocument", 11) == 0) 
			nestinglevel--;	 /* NEW */
/* The following were removed, since inserted EPSF files can contain them */
/* This used to be used for termination in case file ends in binary garbage */
/*		else if (strncmp(line + 2, "Trailer", 7) == 0) return -INFINITY; */
/*		else if (strncmp(line + 2, "Trailer", 7) == 0) { */
		else if (nestinglevel == 0 &&
			strncmp(line + 2, "Trailer", 7) == 0) {
			if (tripout) return -INFINITY; 
			ignore = 1;
		}
/*		else if (strncmp(line + 2, "EOF", 3) == 0) return -INFINITY; */
/*		else if (strncmp(line + 2, "EOF", 3) == 0) { */
		else if (nestinglevel == 0 &&
			strncmp(line + 2, "EOF", 3) == 0) {
			if (tripout) return -INFINITY; 
			ignore = 1;
		}
/* The following provides for three forms: */
/* (i) %%Page: 20 20 (ii) %%Page: "20" 20 and (iii) %%Page: twenty 20 */
/* uses the first number in case (i) and (ii), second in (iii) */
		else if (nestinglevel == 0 &&					/* NEW */
			strncmp(line + 2, "Page:", 5) == 0) {
			page2 = 0;				/* in case not read */
			if(sscanf(line + 2 + 6, "%d %d", &page1, &page2) < 1) {
				if (sscanf(line + 2 + 6, "\"%d\" %d", &page1, &page2) < 1) {
					if (sscanf(line + 2 + 6, "%s %d", label, &page2) < 2) {
						fprintf(stderr, "Don't understand: %s", line);
						return 0;
					}
					page1 = page2;	/* since page1 was `label' */
				}
			}
			if (secondflag != 0) pageno = page2; else pageno = page1;
			if (pageno < 0 || pageno > MAXPAGES) {
				fprintf(stderr, "Page number %d out of range\n", pageno);
				pageno = 0;
			}
			return pageno;
		}
		else if (startup != 0 && strncmp(line + 2, "Pages:", 6) == 0) {
			page2 = 1;		/* in case not read */
			if (sscanf(line + 2 + 7, "%d %d", &page1, &page2) < 1) {
/*				fprintf(stderr, "Don't understand %s", line);  */
				if (directprint == 0) fputs(line, output); /* copy as is */
/*				kludge to deal with %%Pages: (at end) */
			}
			else if (directprint == 0) {
				sprintf(buffer, "%%%%Pages: %d %d\n", (page1 + 1)/2, page2);
				fputsmod(buffer, output);	/* insert our version instead */
			}
			continue;
/*			page2 should be -1, 0, or 1 - or be absent */
		}
		else if (startup != 0 && strncmp(line + 2, "BeginProlog", 11) == 0) {
			if (directprint == 0) fputs(line, output); /* copy as is */
			if (strcmp(prologfile, "") != 0) {
				copyprologfile(prologfile, output);		/* 1992/Nov/11 */
			}
			strcpy(line, "");		/* already used this line 96/Dec/19 */
		}
/*		don't copy across %%Tailer ? %%EOF ? */	
/*		if (flag != 0 && ignore == 0) { */	/* don't copy across %%EOF ? */	
/*		if (flag != 0) { */
		if (flag != 0 && strcmp(line, "") != 0) {	/* 96/Dec/19 */
			if (fputs(line, output) == EOF) {		/* copy as is */
				perror(" Output error");
				return -INFINITY;
			}
		}
		if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
	}
}

/* copy to end of file */	/* not used anymore ? */
/* get rid of ^D 94/Feb/21 - only if alone on line ? */

int copytoend(FILE *input, FILE *output) {
	if (*line == '\f')	strcpy(line, line+1);	/* flush ^L! 94/Feb/21 */
	if (*line == 4)	strcpy(line, line+1);		/* flush ^D! 94/Feb/21 */
	if (strlen(line) > 0) fputs(line, output);

	while (fgets(line, MAXLINEIN, input) != NULL) {
		if (*line == '\f') strcpy(line, line+1);	/* flush ^L! 94/Feb/21 */
		if (*line == 4)	strcpy(line, line+1);		/* flush ^D! 94/Feb/21 */
		if (strlen(line) > 0) {						/* 94/Feb/21 */
			if (fputs(line, output) == EOF) {
				perror(" Output error");
				return -1;
			}
		}
		if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
	}
	return 0;
}

void pagestart(FILE *output) {
	if (inlineflag == 0) fputs("pagestart ", output);
/*	else fprintf(output, "save /showpage {} def\n"); */
/*	else fprintf(output, "\n"); */
	else fputsmod ("\n", output);
}

void pageend(FILE *output) {
	if (inlineflag == 0) fputsmod("pageend\n", output);
/*	else fprintf(output, "restore showpage\n"); */
/*	else fprintf(output, "showpage\n"); */
	else fputsmod("showpage\n", output);
}

void leftstart(FILE *output) {
	if (inlineflag == 0) fputsmod("leftstart\n", output);
	else {
/*		if (extrasafe != 0) fprintf(output, "/twoupstate save def\n"); */
		if (extrasafe != 0) fputsmod("/twoupstate save def\n", output);
/*		fprintf(output, "/showpage {} def\n"); */
		fputsmod("/showpage {} def\n", output);
		if (invertflag != 0) {
			sprintf(buffer,	/* 1992/Jan/4 */
			"%lg %lg translate -1 -1 scale\n", outpagewidth, outpageheight);
			fputsmod(buffer, output);
		}
		sprintf(buffer,
			"%lg 2 div %lg 4 div translate\n", outpagewidth, outpageheight);
		fputsmod(buffer, output);
/*		fprintf(output, "[0 1 -1 0 0 0] concat %% 90 rotate\n"); */
		fputsmod("[0 1 -1 0 0 0] concat % 90 rotate\n", output);
		sprintf(buffer, "%lg dup scale\n", scalefactor); 
		fputsmod(buffer, output);
/*		fprintf(output, "90 rotate %lg dup scale\n", scalefactor); */
		sprintf(buffer, 
		"%lg 2 div neg %lg 2 div neg translate\n", inpagewidth, inpageheight);
		fputsmod(buffer, output);
/*		if (xoffset != 0.0 || yoffset != 0.0) */	/* 1994/Jan/22 */
		if (xoffsete != 0.0 || yoffsete != 0.0) {
/*			fprintf(output, "%lg neg %lg translate\n", xoffset, yoffset); */
			sprintf(buffer, "%lg neg %lg translate\n", xoffsete, yoffsete);
			fputsmod(buffer, output);
		}
	}
}

void leftend(FILE *output) {
/*	if (inlineflag == 0) fprintf(output, "leftend\n"); */
	if (inlineflag == 0) fputsmod("leftend\n", output);
/*	else if (extrasafe != 0) fprintf(output, "twoupstate restore\n"); */
	else if (extrasafe != 0) fputsmod("twoupstate restore\n", output);
}

void rightstart(FILE *output) {
	if (inlineflag == 0) fputsmod("rightstart\n", output);
	else {
/*		if (extrasafe != 0) fprintf(output, "/twoupstate save def\n"); */
		if (extrasafe != 0) fputsmod("/twoupstate save def\n", output);
		fputsmod("/showpage {} def\n", output);
		if (invertflag != 0) {
			sprintf(buffer,	/* 1992/Jan/4 */
			"%lg %lg translate -1 -1 scale\n", outpagewidth, outpageheight);
			fputsmod(buffer, output);			
		}
		sprintf(buffer, 
			"%lg 2 div %lg 4 div 3 mul translate\n", outpagewidth, outpageheight);
		fputsmod(buffer, output);
		fputsmod("[0 1 -1 0 0 0] concat % 90 rotate\n", output);
		sprintf(buffer, "%lg dup scale\n", scalefactor);
		fputsmod(buffer, output);
/*		fprintf(output, "90 rotate %lg dup scale\n", scalefactor); */
		sprintf(buffer, 
		"%lg 2 div neg %lg 2 div neg translate\n", inpagewidth, inpageheight);
		fputsmod(buffer, output);
/*		if (xoffset != 0.0 || yoffset != 0.0) */ /* 1994/Jan/22 */
		if (xoffseto != 0.0 || yoffseto != 0.0) {
/*			fprintf(output, "%lg %lg translate\n", xoffset, yoffset); */
			sprintf(buffer, "%lg %lg translate\n", xoffseto, yoffseto);
			fputsmod(buffer, output);
		}
	}
}

void rightend(FILE *output) {
	if (inlineflag == 0) fputsmod("rightend\n", output);
/*	else if (extrasafe != 0) fprintf(output, "twoupstate restore\n"); */
	else if (extrasafe != 0) fputsmod("twoupstate restore\n", output);
}

void writeparameters(FILE *output) {
	fputsmod("%%BeginSetup\n", output); /* ??? */
	fputsmod("% Start Prolog for TwoUp\n", output);
	fputsmulti(header, output);			/* multi-line text */
	sprintf(buffer, "/OutPageHeight %lg def\n", outpageheight);
	fputsmod(buffer, output);
	sprintf(buffer, "/OutPageWidth %lg def\n", outpagewidth);
	fputsmod(buffer, output);
	sprintf(buffer, "/InPageHeight %lg def\n", inpageheight);
	fputsmod(buffer, output);
	sprintf(buffer, "/InPageWidth %lg def\n", inpagewidth);
	fputsmod(buffer, output);
	sprintf(buffer, "/scalefactor %lg def\n", scalefactor);
	fputsmod(buffer, output);
	sprintf(buffer, "/horizontaloff %lg def\n", xoffset);
	fputsmod(buffer, output);
	sprintf(buffer, "/verticaloff %lg def\n", yoffset);
	fputsmod(buffer, output);
	fputsmod("%% End Prolog for TwoUp\n", output);
	fputsmod("%%EndSetup\n", output); /* ??? */
}

/* find place in file for given page - look in obvious place first */

long pageposition(int page) {
	int k;
	if (pagenumber[page + 1] == page) return pagebegin[page + 1]; 
	if (pagenumber[page + 2] == page) return pagebegin[page + 2]; 
	if (pagenumber[page] == page)	  return pagebegin[page]; 
	for (k = 0; k < MAXPAGES; k++) { /* ok, slow and tedious */
		if (pagenumber[k] == page)    return pagebegin[k]; 
	}
	return -1;
}

/* pre-scan file to get positions of pages */
/* also copies prolog if desired */
/* Note: byte pointer is to start of line AFTER %%Page: */

long prescan(FILE *input, FILE *output) {
	long pagebyte, prologend;
	int k, pagen;

	pagemax = -INFINITY; pagemin = INFINITY;
	for (k = 0; k < MAXPAGES; k++) pagenumber[k]=-1;
	knext = 0;
	startup = -1;
	pagen = scantopage(input, output, 1);
	startup = 0;
	if (pagen == -INFINITY) return 0;
	if (pagen > pagemax) pagemax = pagen;
	if (pagen < pagemin) pagemin = pagen;
	pagebyte = ftell(input);
	pagenumber[knext] = pagen;
	pagebegin[knext] = pagebyte;
	prologend = pagebyte;

	knext++;

	for(;;) {
		pagen = scantopage(input, output, 0);
		if (pagen == -INFINITY) break;
		if (pagen > pagemax) pagemax = pagen;
		if (pagen < pagemin) pagemin = pagen;
		pagebyte = ftell(input);
		pagenumber[knext] = pagen;
		pagebegin[knext] = pagebyte;
		knext++;
		if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
	}
	return prologend;
}

/*
void showpagetable(void) {
	int k;
	for (k = 0; k < knext; k++) {
		printf("page %d position %ld\n", pagenumber[k], pagebegin[k]);
	}
	(void) _getch();
} */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void setscale(void) {
	double scalewidth, scaleheight;
	double scale;			/* computed from in and out page format */

	if (zoom != 0.0) {
		scale = zoom;
		scalefactor = magnification * scale;	/* ha ha */
	}
	else {
		scaleheight = outpagewidth / inpageheight;
		scalewidth = (outpageheight / 2.0) / inpagewidth;
		if (scaleheight < scalewidth) scale = scaleheight;
		else scale = scalewidth;	
		scalefactor = magnification * scale;
		if (verboseflag != 0) {
			printf("Natural scale factor is %lg ", scale);
			if (magnification != 1.0) 
				printf("- Scale factor actually used is %lg", scalefactor);
			putc('\n', stdout);
		}
	}
}

void setpaper(FILE *output) {
	if (strcmp(ppapertype, "") != 0 && strcmp(ppapertype, "letter") != 0) {
		sprintf(buffer, "{%s} stopped pop\n", ppapertype);
		fputsmod(buffer, output);
	}
}

void docommon(FILE *output) {			/* separated out 1993/June/30 */
	setscale();
	if (inlineflag == 0) writeparameters(output);
	setpaper(output);
	if (copies != 1) {
		sprintf(buffer, "/#copies %d def\n", copies);
		fputsmod(buffer, output);
	}
/*	if (duplexflag != 0) {
		fputsmulti(duplexcode, output);	
	} */ /* 1993/June/30 */
	if (duplexflag) {								/* 1996/Dec/19 */
		fputsmod("[{\n", output);
/*		fputsmod("%%BeginFeature: *Duplex DuplexNoTumble\n", output); */
		fputs("%%BeginFeature: *Duplex ", output);
		if (tumbleflag) fputsmod("DuplexTumble\n", output);
		else fputsmod("DuplexNoTumble\n", output);
/*		fputsmod("true statusdict /setduplexmode get exec\n", output); */
		fputs("true ", output);
		fputsmod("statusdict /setduplexmode get exec\n", output);
/*		fputsmod("false statusdict /settumble get exec\n", output); */
		if (tumbleflag) fputs("true ", output);
		else fputs("false ", output);
		fputsmod("statusdict /settumble get exec\n", output);
		fputsmod("%%EndFeature\n", output); 
		fputsmod("} stopped cleartomark\n", output);
	}
}

void writetrailer(FILE *output) {		/* write our own trailer */
	fputsmod("%%Trailer\n", output);
	fputsmod("%%EOF\n", output);	
}

/* print two pages next to one another - ignoring evenness/oddness */

int copytwoup(FILE *input, FILE *output) {
	int pagen = 1;
	int pagel, pager;			/* not accessed */

	if (traceflag) printf("copytwoup\n");
	startup = -1;
	pagel = scantopage(input, output, 1);
	startup = 0;
	if (verboseflag != 0) putc('\n', stdout);
	if (pagel == -INFINITY) return -1;
	docommon(output);
	for(;;) {
		sprintf(buffer, "%%%%Page: %d %d\n", pagen, pagen); /* ? */
		fputsmod(buffer, output);
		pagestart(output);
		if (verboseflag != 0) putc('[', stdout);
		if (verboseflag != 0) printf("%d", pagel);
		else putc('.', stdout);
		leftstart(output);
		pager = scantopage(input, output, 1);
		leftend(output);
		if(pager != -INFINITY) {
			rightstart(output);
			if (verboseflag != 0) printf(" %d", pager);
			else putc('.', stdout);
			pagel = scantopage(input, output, 1);
			rightend(output);
		}
		if (verboseflag != 0) printf("] ");
		pageend(output);
		if (pager == -INFINITY || pagel == -INFINITY) break;
		pagen++;
		if (alternateflag != 0) invertflag = ~invertflag;	/* 1993/June/30 */
		if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
	}
/*	if (copytoend(input, output) < 0) return -1;
	else return 0; */
	writetrailer(output);					/* experiment 96/Aug/13 */
	return 0;
}

/* get even pages on left and following odd pages on right */

int copyevenodd(FILE *input, FILE *output) {
	int pagen = 1, pageo, pages, module;
	
	if (traceflag) printf("copyevenodd\n");
	if (evenoddflag > 0) module = 0; else module = 1;
	startup = -1;
	pages = scantopage(input, output, 1);
	startup = 0;
	if (verboseflag != 0) putc('\n', stdout);
	if (pages == -INFINITY) return -1;
	docommon(output);
	for(;;) {
		sprintf(buffer, "%%%%Page: %d %d\n", pagen, pagen); /* ? */
		fputsmod(buffer, output);
		pagestart(output);
		if (verboseflag != 0) putc('[', stdout);
		if (pages % 2 == module) {		/* even goes on the left */
			pageo = pages;
			if (verboseflag != 0) printf("%d", pages);
			else putc('.', stdout);
			leftstart(output);
			pages = scantopage(input, output, 1);
			leftend(output);
		} 
		else pageo = pages - 1;
		if(pages != -INFINITY && pages == pageo + 1) {
			pageo = pages;
			if (verboseflag != 0) printf(" %d", pages);
			else putc('.', stdout);
			rightstart(output);
			pages = scantopage(input, output, 1);
			rightend(output);
		}
		if (verboseflag != 0) printf("] ");
		pageend(output);
		if (pages == -INFINITY) break;
		pagen++;
		if (alternateflag != 0) invertflag = ~invertflag;	/* 1993/June/30 */
		if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
	}
/*	if (copytoend(input, output) < 0) return -1;
	else return 0; */
	writetrailer(output);					/* experiment 96/Aug/13 */
	return 0;
}

/* arrange for saddle back stapling (1 & 2n) (2 & 2n-1) (3 & 2n-2) */

void copysaddle(FILE *input, FILE *output) {
	int m, ml, mr, mt, pagen = 1, pagel, pager;
	int npagesrnd;
	long prologend, pp;

	if (traceflag) printf("copysaddle\n");
	prologend = prescan(input, output);
	fseek(input, prologend, SEEK_SET);
/*	if (verboseflag != 0) showpagetable(); */
	if (verboseflag != 0) putc('\n', stdout);
	nstart = ((pagemin - 1) / 2) * 2 + 1;			/* odd floor */
	nfinish = nstart + (((pagemax - nstart) / 4 ) + 1) * 4 - 1;
/*	ncenter = (pagemax + 1) / 2; */
	ncenter = (nstart + nfinish - 1) / 2;
	npages = (nfinish - nstart + 1) / 2;
/*	for reverse order printing need to know final page number used */
	npagesrnd = npages;			 			/* 1995/June/23 */ 
	if ((evenflag != 0 && oddflag == 0) ||
		(evenflag == 0 && oddflag != 0)) {
		if (npages % 2 == 0) npagesrnd = npages - 1;
	}
/*	printf("nstart %d nfinish %d ncenter %d npages %d npagesrnd %d\n",
		nstart, nfinish, ncenter, npages, npagesrnd); */ /* debugging */
/*	if (verboseflag != 0) 
		printf("pagemax %d  - ncenter %d\n", pagemax, ncenter); */

/*	startup = -1; */
/*	pagel = scantopage(input, output, 1); */
	startup = 0;
/*	if (pagel < 0) return; */
	docommon(output);
	for(m = 0; m < npages; m++) {
		if (evenflag == 0) {
/*			mr = m; ml = 2 * ncenter + 1 - m; */
			if (reverseflag == 0) {
				mr = nstart + m;
				ml = nfinish - m;
			}
			else {								/* 1995/June/23 */
				mr = nstart + (npagesrnd - m - 1);
				ml = nfinish - (npagesrnd - m -1);
			}
		}
		else { /* do `bottom' pages in reverse order */
/*			mr = ncenter + m; ml = ncenter + 1 - m; */
			if (reverseflag == 0) {
				mr = ncenter - m;
				ml = ncenter + 1 + m;
			}
			else {							/* 1995/June/23 */
				mr = ncenter - (npagesrnd - m - 1);
				ml = ncenter + 1 + (npagesrnd - m -1);
			}			
		}
		if (mr % 2 == 0) {
			mt = mr; mr = ml; ml = mt;
		}
		sprintf(buffer, "%%%%Page: %d %d\n", pagen, pagen); /* ? */
		fputsmod(buffer, output);
		pagestart(output);
		pp = pageposition(ml);
		if (verboseflag != 0) printf("[%d ", ml);
		else putc('.', stdout);
		if (pp >= 0) {
			leftstart(output);
			fseek(input, pp, SEEK_SET);
			pager = scantopage(input, output, 1);
			leftend(output);
		}
		if (verboseflag != 0) printf("%d] ", mr);
		else putc('.', stdout);
		pp = pageposition(mr);		
		if (pp >= 0) {
			rightstart(output);
			fseek(input, pp, SEEK_SET);
			pagel = scantopage(input, output, 1);
			rightend(output);
		}
		pageend(output);
		pagen++;
/*	Advance in steps of two unless both evenflag & oddflag are set ... */
		if ((evenflag != 0 && oddflag == 0) ||
			(evenflag == 0 && oddflag != 0))  m++;
		if (alternateflag != 0) invertflag = ~invertflag;	/* 1993/June/30 */
		if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
	}
	writetrailer(output);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***  */

/* The following three arrays must be `parallel' */

/* int npapertypes = 7 + 6; */		/* number of papertypes flush 94/May/6 */

/* Also now handle Letter, A4, A5 etc as landscape versions 94/July/1 */

char *papertypes[] = {
"letter", "note", "legal", "folio", "ledger", "tabloid", "executive", 
"a3", "a4", "a5", "b4", "b5", "quarto", ""};

double pagewidths[] = { /* in inches */
 8.5, 8.5, 8.5, 8.5, 11, 11, 7.25, 
 297/25.4, 210/25.4, 148/25.4, 250/25.4, 176/25.4, 215/25.4, 1};

double pageheights[] = { /* in inches */
 11,  11,  14,  13,  17, 17, 10.5,
 420/25.4, 297/25.4, 210/25.4, 354/25.4, 250/25.4, 275/25.4, 1};

/* `statement' 5 1/2 x 8 1/2 */

/* may need to do something here to tell printer what is going on also ? */
/* PS letter, note, legal, ledger ? */
/* use table for neatness */ /* returns values in points */

struct coord analpapertype(char *papertype) {	/* new version 94/July/1 */
/* void analpapertype(char *papertype, double *pagewidth, double *pageheight) { */
	double pagewidth, pageheight;
	struct coord shape;
	int flag = 0;
	int k;

/*	pageheight = 11.0;	pagewidth = 8.5; */  /* in case all fails */
	pageheight = 11*72;	pagewidth = 8.5*72;		/* default */

	if (strcmp(papertype, "") == 0) {		/* default is letter */
/*		shape.w = pagewidth * 72.0; shape.h = pageheight * 72.0; */
		shape.w = pagewidth; shape.h = pageheight;
		return shape;
	}
	for (k= 0; k < 32; k++) {					/* 94/May/6 */
		if (strcmp(papertypes[k], "") == 0) break;
/*	Try portrait versions ... */
		if (strcmp(papertypes[k], papertype) == 0) {
			pagewidth = pagewidths[k] * 72;
			pageheight = pageheights[k] * 72;
/*			return; */
			flag = 1; break;
		}
/*	Try landscape versions ... (In which first letter upper case) 94/July/1 */
/*		if (strcmpi(papertypes[k], papertype) == 0) { */
		if (_strcmpi(papertypes[k], papertype) == 0) {
			pageheight = pagewidths[k] * 72;
			pagewidth = pageheights[k] * 72;
/*			return; */
			flag = 1; break;
		}
	}
	if (flag == 0) {
		fprintf(stderr, "Don't understand papertype: %s\n", papertype);
/*		errcount(); */
	}
/*	shape.w = pagewidth * 72.0; shape.h = pageheight * 72.0; */
	shape.w = pagewidth; shape.h = pageheight;
	return shape;
}

/* what type of page is it - device specific */

/* `statement' 5 1/2 x 8 1/2 */

/* a4tray, b5tray, legaltray, lettertray  - or setpageparams ? */


#ifdef IGNORED
struct coord analpapertype(char *papertype) { /* what type of page is it */
	double pagewidth, pageheight;
	struct coord shape;

	pageheight = 11.0;	pagewidth = 8.5;  /* in case all fails */

	if (strcmp(papertype, "") == 0 ||
		strcmp(papertype, "letter") == 0 ||
		strcmp(papertype, "note") == 0) {				/* 8.5" x 11" */
		pagewidth = 8.5;	pageheight = 11.0;
	}
	else if (strcmp(papertype, "legal") == 0) {			/* 8.5" x 14" */
		pagewidth = 8.5;	pageheight = 14.0;
	}
	else if (strcmp(papertype, "folio") == 0) {			/* 8.5" x 13" */
		pagewidth = 8.5;	pageheight = 13.0;
	}
	else if (strcmp(papertype, "ledger") == 0 ||
		     strcmp(papertype, "tabloid") == 0) {		/* 11" x 17" */
		pagewidth = 11.0;	pageheight = 17.0;
	}
	else if (strcmp(papertype, "executive") == 0) {		/* 7.25" x 10.5" */
		pagewidth = 7.25;	pageheight = 10.5;		/* ??? */
	}	
	else if (strcmp(papertype, "a3") == 0) {			/* 297mm x 420mm */
		pagewidth = 297.0/25.4;	pageheight = 420.0/25.4;
	}
	else if (strcmp(papertype, "a4") == 0) {			/* 210mm x 297mm */
		pagewidth = 210.0/25.4;	pageheight = 297.0/25.4;
	}
	else if (strcmp(papertype, "a5") == 0) {			/* 148mm x 210mm */
		pagewidth = 148.0/25.4;	pageheight = 210.0/25.4;
	}
	else if (strcmp(papertype, "b4") ==0) {				/* 250mm x 353mm */
		pagewidth = 250.0/25.4;	pageheight = 353.0/25.4;
	} 
	else if (strcmp(papertype, "b5") ==0) {				/* 176mm x 250mm */
		pagewidth = 176.0/25.4;	pageheight = 250.0/25.4; /* ??? */
	} 
	else if (strcmp(papertype, "quarto") ==0) {			/* 215mm x 275mm */
		pagewidth = 215.0/25.4;	pageheight = 275.0/25.4;
	} 	
	else {
		fprintf(stderr, "Don't understand papertype: %s\n", papertype);
/*		exit(3); */
	}
	shape.w = pagewidth * 72.0; shape.h = pageheight * 72.0;
	return shape;
}
#endif

/* also a3 ? */

/* a4tray, b5tray, legaltray, lettertray  - or setpageparams ? */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***  */

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0;
		case 'v': verboseflag = 1; return 0;
		case 't': traceflag = 1; return 0;
		case 'o': evenoddflag = 1; return 0;
		case 'e': evenoddflag = -1; return 0;
		case 'g': oddflag = 1; saddleflag = 1; return 0;
		case 'h': evenflag = 1; saddleflag = 1; return 0;
		case 's': saddleflag = 1; return 0;
		case 'r': reverseflag = 1; return 0;
		case 'z': secondflag = 1; return 0;
		case 'i': invertflag = 1; return 0;
		case 'a': alternateflag = 1; return 0;
		case 'O': duplexflag = 1; return 0;
		case 'T': tumbleflag = 1; return 0;	/* 96/Dec/20 */
		case 'J': wantcontrold = (1 - wantcontrold); return 0;
/*		case 'n': tripout = 1; return 0; */
		case 'n': tripout = (1 - tripout); return 0;
/*		the rest take arguments */
		case 'm': magnifyflag = 1; break;
		case 'M': zoomflag = 1; break;		
		case 'x': xoffsetflag = 1; break;
		case 'y': yoffsetflag = 1; break;		
		case 'c': copiesflag = 1; break;
		case 'd': outputflag = 1; break;
		case 'l': lpaperflag = 1; break;
		case 'p': ppaperflag = 1; break;		
		case 'w': prologflag = 1; break;
/*		case 'i': epsflag = 1; break;	*/	/* EPS insert search path */
		default: {
				fprintf(stderr, "Invalid command line flag '%c'\n", c);
				exit(7);
		}
	}
	return -1;		/* need argument */
}

/* [-h=<head>] */

void showusage(char *s) {
	fprintf (stderr, /* "Correct usage is:\n\ */
"%s [-{v}{o}{e}{s}{g}{h}{r}{z}{i}{a}]\n\
\t[-x=<xoff>] [-y=<yoff>] [-m=<mag>] [-M=<mag>]\n\
\t[-w=<prolog>] [-l=<pagetype>] [-p=<pagetype>] [-c <copies>]\n\
\t[-d=<destination>] <ps-file-1> <ps-file-2> ...\n\
",	s);
	if (detailflag == 0) exit(7);
	fprintf (stderr, "\
\tv: verbose mode\n\
\to: odd pages on right - even pages on the left\n\
\te: even pages on the right - odd pages on the left\n\
\ts: print for saddle back stapling  (2n, 1), (2, 2n-1) ...\n\
\tg: print `top' pages (2n, 1), (2n-2, 3) ... (implies `s')\n\
\th: print `bottom' pages (2, 2n-1), (4, 2n-3) ... (implies `s')\n\
\tr: reverse order of pages (for saddle back printing)\n\
\tz: use second number in %%%%Page: comments instead of first\n\
\ti: invert all pages (i.e. rotate 180 degrees)\n\
\ta: invert alternating output pages\n\
\tx: horizon. offset (in pts, outward positive - repeat for left & right)\n\
\ty: vertical offset (in pts, up positive)\n\
\tm: magnify by given factor  (relative - default 1.0)\n\
\tM: magnify by given factor  (absolute)\n\
\tc: number of copies  (default 1)\n\
\tw: prolog file to be inserted\n\
\tl: logical page - papertype for input (default `letter')\n\
\tp: physical page - papertype for output (default `letter')\n\
\td: destination (PRN, LPT1 ... , AUX, COM1 ... , NUL or file)\n\
\t   (default is file with same name, but extension `ps2')\
");
	exit(7);
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command line flags */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break; 
			else if (decodeflag(c) != 0) { /* flag requires argument ? */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (copiesflag != 0) {
					if (sscanf(s, "%d", &copies) < 1) {
						fprintf(stderr, "Don't understand copies: %s\n", s);
					}
					copiesflag = 0;
				}
				else if (magnifyflag != 0) {
					if(sscanf(s, "%lg", &magnification) < 1) {
						fprintf(stderr, "Don't understand magnify: %s\n", s);
					}
					magnifyflag = 0;
				}
				else if (zoomflag != 0) {
					if(sscanf(s, "%lg", &zoom) < 1) {
						fprintf(stderr, "Don't understand magnify: %s\n", s);
					}
					zoomflag = 0;
				}
				else if (xoffsetflag != 0) {
					if(sscanf(s, "%lg", &xoffset) < 1) {
						fprintf(stderr, "Don't understand x off: %s\n", s);
					}
/* following imported from DVIPSONE 1994/Jan/22 */
					if (xoffsete != UNKNOWNOFFSET)  {
						if (xoffseto != UNKNOWNOFFSET) 
							fprintf(stderr, "Too many x offsets\n");
						else xoffseto= xoffset;
					}
					else xoffsete= xoffset;
					xoffsetflag = 0;
				}
				else if (yoffsetflag != 0) {
					if(sscanf(s, "%lg", &yoffset) < 1) {
						fprintf(stderr, "Don't understand y off: %s\n", s);
					}
/* following imported from DVIPSONE 1994/Jan/22 */
					if (yoffsete != UNKNOWNOFFSET)  {
						if (yoffseto != UNKNOWNOFFSET) 
							fprintf(stderr, "Too many y offsets\n");
						else yoffseto= yoffset;
					}
					else yoffsete= yoffset;
					yoffsetflag = 0;
				}
				else if (outputflag != 0) {
					outputfile = s;		outputflag = 0;
				}
				else if (lpaperflag != 0) {
					lpapertype = s;		lpaperflag = 0;
				}
				else if (ppaperflag != 0) {
					ppapertype = s;		ppaperflag = 0;
				}
				else if (prologflag != 0) {
					prologfile = s;		prologflag = 0;
				}				
				else if (epsflag != 0) {
					epspath = s;		epsflag = 0;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

void lowercase(char *t, char *s) {
	int c;
	while ((c = *s++) != '\0') {
		if (c >= 'A' && c <= 'Z') *t++ = (char) (c - 'A' + 'a');
		else *t++ = (char) c;
	}
	*t = '\0';
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

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);
	fprintf(output, "%s (%s %s)\n", 
		programversion, date, compiletime);
}

long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	
/*	(void) _getch(); */
	return hash;
}

#ifdef CONTROLBREAK
void ctrlbreak(int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
/*	cleanup(); */
/*	fprintf(stderr, "\nUser Interrupt"); */
	if (bAbort++ >= 3) exit(3);				/* emergency exit */
/*	abort(); */
/*	(void) signal(SIGINT, ctrlbreak); */
}
#endif

/* remove file name - keep only path - inserts '\0' to terminate */
void removepath(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) ;
	else if ((s=strrchr(pathname, '/')) != NULL) ;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	*s = '\0';
}


/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindo = "";			/* full file name for dviwindo.ini with path */

#define MAXLINE 128

/* set up full file name for ini file and check for specified section */
/* e.g. dviwindo = setupinifile ("dviwindo.ini", "[Environment]") */

char *setupinifile (char *ininame, char *section) {	
	char fullfilename[FILENAME_MAX];
	FILE *input;
	char *windir;
	char line[MAXLINE];
	int m;

/*	Easy to find Windows directory if Windows runs */
/*	Or if user kindly set WINDIR environment variable */
/*	Or if running in Windows NT */	
	if ((windir = getenv("windir")) != NULL ||	/* 1994/Jan/22 */
		(windir = getenv("WINDIR")) != NULL ||
		(windir = getenv("winbootdir")) != NULL ||
		(windir = getenv("SystemRoot")) != NULL ||
 		(windir = getenv("SYSTEMROOT")) != NULL) { /* 1995/Jun/23 */
		strcpy(fullfilename, windir);
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);
	}
	else _searchenv (ininame, "PATH", fullfilename);

	if (strcmp(fullfilename, "") == 0) {		/* ugh, try standard place */
		strcpy(fullfilename, "c:\\windows");
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);		
	}

/*	if (*fullfilename != '\0') { */
/*	check whether ini file actually has required section */
		if ((input = fopen(fullfilename, "r")) != NULL) {
			m = strlen(section);
			while (fgets (line, sizeof(line), input) != NULL) {
				if (*line == ';') continue;
/*				if (strncmp(line, section, m) == 0) { */
				if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
					fclose(input);
					return _strdup(fullfilename);
				}
			}					
			fclose(input);
		}
/*	} */
	return "";							/* failed, for one reason or another */
}

int setupdviwindo (void) {
	if (usedviwindo == 0) return 0;		/* already tried and failed */
	if (*dviwindo != '\0') return 1;	/* already tried and succeeded */
/*	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");  */
	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");
	if (*dviwindo == '\0') usedviwindo = 0; /* failed, don't try this again */
	return (*dviwindo != '\0');
}

char *grabenvvar (char *varname, char *inifile, char *section, int useini) {
	FILE *input;
	char line[MAXLINE];
	char *s;
	int m, n;

	if (useini == 0 || *inifile == '\0')
		return getenv(varname);	/* get from environment */
	if ((input = fopen(inifile, "r")) != NULL) {
		m = strlen(section);
/* search for [...] section */
		while (fgets (line, sizeof(line), input) != NULL) {
			if (*line == ';') continue;
/*			if (strncmp(line, section, m) == 0) { */
			if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
/* search for varname=... */
				n = strlen(varname);
				while (fgets (line, sizeof(line), input) != NULL) {
					if (*line == ';') continue;
					if (*line == '[') break;
/*					if (*line == '\n') break; */	/* ??? */
					if (*line <= ' ') continue;		/* 95/June/23 */
/*					if (strncmp(line, varname, n) == 0 && */
					if (_strnicmp(line, varname, n) == 0 &&
						*(line+n) == '=') {	/* found it ? */
							fclose (input);
							/* flush trailing white space */
							s = line + strlen(line) - 1;
							while (*s <= ' ' && s > line) *s-- = '\0';
							if (traceflag)  /* DEBUGGING ONLY */
								printf("%s=%s\n", varname, line+n+1);
							return _strdup(line+n+1);
					}							
				}	/* end of while fgets */
			}	/* end of search for [Environment] section */
		}	/* end of while fgets */
		fclose (input);
	}	/* end of if fopen */
/*	useini = 0; */				/* so won't try this again ! need & then */
	return getenv(varname);	/* failed, so try and get from environment */
}							/* this will return NULL if not found anywhere */

char *grabenv (char *varname) {	/* get from [Environment] in dviwindo.ini */
	return grabenvvar (varname, dviwindo, "[Environment]", usedviwindo);
}

#ifdef NEEDATMINI
/* grab setting from `atm.ini' 94/June/15 */

/*	if (useatmini)  setupatmini(); 	*/ /* need to do this before use */

int useatmini = 1;			/* use [Setup] section in `atm.ini' */
							/* reset if setup of atm.ini file fails */

char *atmininame = "atm.ini";		/* name of ini file we are looking for */

char *atmsection = "[Setup]";		/* ATM.INI section */

char *atmini = "";				/* full file name for atm.ini with path */

int setupatmini (void) {
	if (useatmini == 0) return 0;		/* already tried and failed */
	if (*atmini != '\0') return 1;		/* already tried and succeeded */
/*	atmini = setupinifile ("atm.ini", "[Setup]");  */
	atmini = setupinifile (atmininame, atmsection);
	if (*atmini == '\0') useatmini = 0;	/* failed, don't try this again */
	return (*atmini != '\0');
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	int m, firstarg;
/*	FILE *input, *output; */
/*	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX]; */
	char *s;
	char *mode;
	struct coord lshape, pshape;
	
#ifdef CONTROLBREAK
	(void) signal(SIGINT, ctrlbreak);
#endif

	if (argc < 2) showusage(argv[0]);
	
	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("PSPATH")) != NULL) epspath = s; 
/*	else if (usedviwindo) { */
	if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("PSPATH")) != NULL) epspath = s;
	}

	firstarg = commandline(argc, argv, 1);

/*	following imported from DVIPSONE 1994/Jan/22 */
	if (xoffseto != UNKNOWNOFFSET || yoffseto != UNKNOWNOFFSET) evenoddoff = 1;
	else evenoddoff = 0;
	if (xoffsete == UNKNOWNOFFSET) xoffsete = 0.0;
	if (xoffseto == UNKNOWNOFFSET) xoffseto = xoffsete;
	if (yoffsete == UNKNOWNOFFSET) yoffsete = 0.0;
	if (yoffseto == UNKNOWNOFFSET) yoffseto = yoffsete;	
	
	if (firstarg >= argc) showusage(argv[0]);

/*	scivilize(compiledate);	 */
	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	if (checkcopyright(copyright) != 0) exit(1);

	lshape = analpapertype(lpapertype);
	inpagewidth = lshape.w; inpageheight = lshape.h;
	pshape = analpapertype(ppapertype);
	outpagewidth = pshape.w; outpageheight = pshape.h;

	if (strcmp(outputfile, "") != 0) {	/* output file specified ? */
		lowercase(fn_out, outputfile); 
/*		allow for trailing colon after device name --- 95/June/26 */
		s = fn_out + strlen(fn_out);
		if (*(s-1) == ':' && s >= fn_out+3) *(s-1) = '\0';
		if (strcmp(fn_out, "prn") == 0 || 
			strcmp(fn_out, "aux") == 0 || 
			strcmp(fn_out, "ept") == 0 ||
				((strlen(fn_out) == 4) &&
				 (strncmp(fn_out, "lpt", 3) == 0 || 
				  strncmp(fn_out, "com", 3) == 0) &&
					 *(fn_out+3) >= '0' && *(fn_out+3) <= '5')) {
			directprint = 1;
		}
		else {								/* apparently not a printer */
			lowercase(fn_out, fn_out);		/* 1993/Aug/26 */
			newexten(fn_out, 1);			/* output file name specified */
			directprint = 0;
		}
/*		printf("fn_out %s strlen %d *(fn_out+3) %c direct %d\n", 
			fn_out, strlen(fn_out), *(fn_out+3), directprint); */
/*		if (traceflag != 0) printf("Output file name: %s\n", fn_out); */
/*		open output in binary mode to avoid \r after \n */
		if (retplusnew != 0) mode = "w"; else mode = "wb";
/*		if ((output = fopen(fn_out, "wb")) == NULL) { */
		if ((output = fopen(fn_out, mode)) == NULL) {
			fprintf(stderr, "Can't open output file ");
			perror(fn_out); exit(7);
		} 
	}

/* open in and out in  'b' mode to avoid lossage with nl rt */

	for (m = firstarg; m < argc; m++) {
		strcpy (fn_in, argv[m]);
		lowercase(fn_in, fn_in);				/* 1993/Aug/26 */
		extension (fn_in, "ps");
		strcpy(dvipath, fn_in);					/* possible use by dvispeci */
		removepath(dvipath);
		if((input = fopen(fn_in, "rb")) == NULL) {
			fprintf(stderr, "Can't open input file "); 
			perror(fn_in);	exit(2);
		}

		if (strcmp(outputfile, "") == 0) { /* output file specified ? */
			if (currentdirect != 0) {	   /* write in current directory ? */
				s = stripname(fn_in);
				strcpy(fn_out, s);  
			}
			else strcpy(fn_out, fn_in);	/* write in same place as input */
			newexten(fn_out, 0);		/* output file name not specified */
			directprint = 0;

			if (strcmp(fn_out, fn_in) == 0) {
				s = fn_out + strlen(fn_out) - 1;
				*s++;				/* mess with last character ! */
				fprintf(stderr,
				"WARNING: output file same as input (%s), using %s instead\n",
					fn_in, fn_out);
			}
/*		open output in binary mode if want to avoid \r after \n */
			if (retplusnew != 0) mode = "w"; else mode = "wb";
/*			if ((output = fopen(fn_out, "wb")) == NULL) { */
			if ((output = fopen(fn_out, mode)) == NULL) {
				fprintf(stderr, "Can't open output file ");
				perror(fn_out); exit(7);
			}
		}
		
		if (evenoddflag != 0) copyevenodd(input, output); 
		else if (saddleflag != 0) copysaddle(input, output);
		else copytwoup(input, output); 
		
		fclose(input);

/*	if output file was NOT specified close output for each input file */
		if (strcmp(outputfile, "") == 0) { /* output file specified ? */
			if (directprint != 0 && wantcontrold != 0) putc(4, output);
			if(ferror(output) != 0) {
/*				fprintf(stderr, "Error in output file "); */
				fputs("Error in output file ", stderr);
				perror(fn_out); exit(3);
			}
			else fclose(output);
		}
		if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
	}
/*	if output file specified, close only ONCE (for all input files) */
	if (strcmp(outputfile, "") != 0) { /* output file specified ? */
		if (directprint != 0 && wantcontrold != 0) putc(4, output);
		if(ferror(output) != 0) {
/*			fprintf(stderr, "Error in output file "); */
			fputs("Error in output file ", stderr);
			perror(fn_out); exit(3);
		}
		else fclose(output);
	}

	if (argc > firstarg + 1) 
		printf("Processed %d PS files\n", argc - firstarg);

	return 0;
}

/* modify %%Pages: %d %d ? */

/* problems if file uses control-M instead of control-J ? */

/* deal with negative pagenumbers from DVI output ? */

/* problem if %%Pages says (at end) ? */

/* wired in assumption about starting at page 1 ? */

/* wired in assumption that pages are sequential ? */

/* insert %%PageTrailer ? */ /* strip out old %%PageTrailer ? */

/* need page selection ranges ? -b= -e= ? too Confusing ? */

/* some of the machinery for insertion of prolog exists, but not wired in */

/* input is *always* opened in "rb" mode -- to deal with Mac EPS files etc */

/* output is normally opened in "wb" mode -- unless (retplusnew != 0) */
