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

/* New version of decompression program - for downloading */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <conio.h>
#include <signal.h>

/* #define FNAMELEN 80 */
/* #define MAXPATHLEN 128 */
#define MAXERRORS 4

#define CONTROLBREAK

int chatterflag = 1;
int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int wantcpyrght = 1;
int currentdirect = 1;		/* place output file in current directory */
int maxcolumns = 78;		/* maximum number of columns in output */
int multifile = 0;			/* open and close each time if non-zero */
int wantcontrold = 1;		/* want controld at end if going to printer */
int signatureflag = 1;		/* add origin signature */
int expandret = 1;			/* convert \r to  \r \n */
int directprint = 0;		/* non-zero if direct to prijnter */
int firsteofend = 0;		/* stop on first EOF record seen */
int queryflag = 0;			/* on if printer query job */

int errcount = 0;			/* quit if too many */
int permanentflag = 0;		/* download permanently */
int rebootflag = 0;			/* try to reboot printer */
int fontdirectory = 0;		/* print directory of downloaded fonts */
int stdinflag = 0;			/* read input from keyboard */
int outputflag = 0;			/* next arg is destination file name */
int passwordflag = 0;		/* next arg is password for permanent download */
int harddiskflag = 0;		/* next arg is font name download to hard disk */

volatile int bAbort=0;				/* set if user types control-C */

/* char *defaultout="PRN"; */

static	FILE *input, *output;
static	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX], fn_bak[FILENAME_MAX];

char *outputfile="";

char *fontname="";

char *password="";

/* char *programversion = "DOWNLOAD version 0.8"; */
/* char *programversion = "DOWNLOAD version 0.9"; */
/* char *programversion = "DOWNLOAD version 1.0"; */
char *programversion = "DOWNLOAD version 1.0.1";

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright = "\
Copyright (C) 1990-1993  Y&Y.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1990 - 1993  Y&Y. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 2670755 */
/* #define COPYHASH 14251529 */
/* #define COPYHASH 10345698 */
#define COPYHASH 7739626

/* WARNING:  CHANGE COPYHASH if COPYRIGHT message is changed !!! */

static char adobehead[] = "%!PS-Adobe-2.0";

static char signature[] = "\
%%Creator: DVIPSONE font downloader. Copyright (C) 1990, 1991 Y&Y\n";

/* static char downheader[] = "serverdict begin 0 exitserver\n"; */

/* /fd (fonts/<font-name>) (w) file def */

static char harddisk[]="\
/buff 128 string def\n\
{currentfile buff readstring\n\
{fd exch writestring}\n\
{dup length 0 gt {fd exch writestring}{pop} ifelse fd closefile exit}\n\
ifelse\n\
} bind loop\n\
";

/* static char reboot[] = 
"\serverdict begin 0 exitserver systemdict /quit get exec\n"; */

static char reboot[] = "systemdict /quit get exec\n";

/* %!PS-Adobe-2.0 Query\n\ */

static char directory[] = "\
/font/Helvetica-Bold findfont 10 scalefont def\n\
/x 36 def/y 720 def/LF{x currentpoint 12 sub 2 -1 roll pop dup 72 lt\n\
{/x x 180 add def/y 612 def pop pop x y}if moveto}bind def/name 80 string def\n\
x y moveto font setfont (Current Printer Status:) show LF LF\n\
/Helvetica findfont 10 scalefont setfont\n\
(Total Memory:  )show\n\
vmstatus dup name cvs show exch sub ( Bytes)show LF\n\
(Available Memory:  )show\n\
name cvs show pop ( Bytes)show LF\n\
(PostScript Interpreter Version: )show\n\
version show LF LF\n\
font setfont (Resident Fonts:)show LF LF\n\
/pointsize 10 def\n\
/name 100 string def\n\
/fonts 200 array def\n\
/ix 0 def\n\
/name1 100 string def\n\
/name2 50 string def\n\
FontDirectory{pop fonts ix 3 -1 roll put/ix ix 1 add def}forall\n\
0 1 ix 2 sub{/i exch def i 1 add 1 ix 1 sub{/j exch def fonts i get\n\
name1 cvs fonts j get name2 cvs gt{fonts i get/temp exch def fonts i fonts\n\
j get put fonts j temp put}if}for}for\n\
0 1 ix 1 sub{/i exch def fonts i get/fontname exch def\n\
/Helvetica findfont 10 scalefont setfont\n\
fontname name cvs show LF}for\n\
LF font setfont(Disk Fonts:)show LF LF\n\
/name 100 string def\n\
/fonts 200 array def\n\
/ix 0 def\n\
/name1 100 string def\n\
/name2 50 string def\n\
/filenameforall where{(fonts/*){dup length 6 sub 6 exch getinterval cvn\n\
fonts ix 3 -1 roll put/ix ix 1 add def}100 string filenameforall}if\n\
0 1 ix 2 sub{/i exch def i 1 add 1 ix 1 sub{/j exch def fonts i get\n\
name1 cvs fonts j get name2 cvs gt{fonts i get/temp exch def fonts i fonts\n\
j get put fonts j temp put}if}for}for\n\
0 1 ix 1 sub{/i exch def fonts i get/fontname exch def\n\
/Helvetica findfont 10 scalefont setfont fontname name cvs show LF}for\n\
showpage\n\
";

unsigned long readlength(FILE *fp_in) {   /* read binary length code */
	int i, c, k = 0;
	unsigned long length=0L;

	for (i=0; i < 4; i++) {
		c = getc(fp_in);
		length = length | ((unsigned long) c << k);
		k = k + 8;
	}
/*	if (verboseflag != 0) printf("Length is %lu - ", length);  */
	return length;
}

#ifdef CONTROLBREAK
void cleanup(void) {
	if (output != NULL) {
		if (directprint != 0) {
			if (wantcontrold != 0) {
				putc(3, output);			/* send control-D */
				putc(4, output);			/* send control-D */
			}
			fclose(output);				/* close output */
		}
		else {
			fclose(output);				/* close output */
			(void) remove(fn_out);				/* and remove bad file */
		}
	}
/*	fcloseall();  */
}
#endif

void abortjob (void) {	/* 1992/Nov/24 */
	cleanup();
	exit(3);
}

void asciisection(FILE *input, FILE *output) {
	int c;
	unsigned long length, k;

	length = readlength(input); k = length;
	if (verboseflag != 0) printf("ASCII section %lu bytes ", length);
	while ((c = getc(input)) < 128 && c != EOF) {
		k--;
		putc(c, output);
		if (expandret != 0 && c == '\r') {
			if((c = getc(input)) == '\n') k--;
			else (void) ungetc(c, input);
			putc('\n', output);
		}
		if (bAbort > 0) abortjob();			/* 1992/Nov/24 */
	}
	if (k != 0) {
		putc('\n', stderr);
		fprintf(stderr, 
			"WARNING: ASCII segment length (%lu) not as advertized (%lu)\n",
			length - k, length);
		if (errcount++ > MAXERRORS) {
			fprintf(stderr, "Bad file format, giving up\n"); exit(1);
		}
	}
	(void) ungetc(c, input);
	return;
}

void binarysection(FILE *input, FILE *output) {
	int c, d, column=0;
	unsigned long length, k;

	length = readlength(input); 
	if (verboseflag != 0) printf("Binary section %lu bytes ", length);
	for (k = 0; k < length; k++) {
		if ((c = getc(input)) == EOF) {
			putc('\n', stderr);
			fprintf(stderr, "WARNING: Unexpected EOF in binary section\n");
			if(errcount++ > MAXERRORS) {
				fprintf(stderr, "Bad file format, giving up\n"); exit(1);
			}
		}
		d = c & 15; c = c >> 4;
		if (c > 9) c = c + 'A' - 10; else c = c + '0';
		putc(c, output);
		if (d > 9) d = d + 'A' - 10; else d = d + '0';
		putc(d, output);
		if ((column = column + 2) >= maxcolumns) {
			putc('\r', output); putc('\n', output);
			column = 0;
		}
		if (bAbort > 0) abortjob();			/* 1992/Nov/24 */
	}
	putc('\r', output); putc('\n', output);	
	return;
}

void decompress(FILE *input, FILE *output) {
	int c;

	for(;;) {
		while ((c = getc(input)) < 128 && c != EOF) {
			putc(c, output);
			if (bAbort > 0) abortjob();				/* 1992/Nov/24 */
		}
		if (c == 128) {
			if((c = getc(input)) == 1) asciisection(input, output);
			else if (c == 2) binarysection(input, output);
			else if (c == 3) {
				if (verboseflag != 0) printf("EOF\n");	
				if (firsteofend != 0) return; /* legitimate EOF */
			}
			else {
				putc('\n', stderr);
				fprintf(stderr, "WARNING: Invalid segment type: %d\n", c);
				if(errcount++ > MAXERRORS)  {
					fprintf(stderr, "Bad file format, giving up\n"); exit(1);
				}
			}
		}
		else if (c == EOF) return;	/* possibly incomplete EOF */
		else { 
			putc('\n', stderr);
			fprintf(stderr, "WARNING: Binary data in ASCII segment: %d\n", c);
			if(errcount++ > MAXERRORS) {
				fprintf(stderr, "Bad file format, giving up\n"); exit(1);
			}
		}
		if (bAbort > 0) abortjob();			/* 1992/Nov/24 */
	}
}

#ifdef IGNORE
void extension(char *fname, char *ext) {  /* supply extension  */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension */
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);	
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

int decodeflag (int c) {
	switch(c) { 
		case '?': detailflag = 1; return 0;
		case 'v': verboseflag = 1; return 0;
		case 't': traceflag = 1; return 0;		
		case 'b': rebootflag = 1; return 0;		/*  multifile = 0; ??? */
		case 'f': fontdirectory = 1; return 0;  /*  multifile = 0; ??? */
		case 'p': permanentflag = 1; return 0; 
		case 'i': stdinflag = 1; return 0; 
		case 'J': wantcontrold = 0; return 0;
/* the rest take arguments */
		case 'k': passwordflag = 1; return -1; 
		case 'h': harddiskflag = 1; return -1; 
		case 'd': outputflag = 1; return -1; 
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
}

void showusage(char *s) {
	fprintf (stderr, /* "Correct usage is:\n\ */
"%s [-{v}{p}{f}{b}{i}] [-k=<password>]\n\
\t[-h=<font-name>] [-d=<destination>]\n\
\t\t<font-file-1> <font-file-2>...\n", 
s);
	if (detailflag == 0) exit(1);
	fprintf (stderr, "\
\tv: verbose mode\n\
\tp: load `permanently' to virtual memory (VM)\n\
\tf: print sorted font directory on printer\n\
\tb: reboot printer (to clear virtual memory)\n\
\ti: read PS input from keyboard - terminate with control-Z\n\
\tk: use specified password - number (default 0) or \"<string>\"\n\
\th: load to hard disk - using given font name\n\
\td: destination (PRN, LPT1 ... , AUX, COM1 ... , NUL or file)\n\
");
	exit(5);
}

/* \t   (default is file with same name as input, but extension `pfa')\n\ */

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (outputflag != 0) {
					outputfile = s; outputflag = 0; 
				}
				else if (harddiskflag != 0) {
					fontname = s;
					harddiskflag = 0; 
				}
				else if (passwordflag != 0) {
					password = s;
					passwordflag = 0; 
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
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

void lowercase(char *t, char *s) {
	int c;
	while ((c = *s++) != '\0') {
		if (c >= 'A' && c <= 'Z') *t++ = (char) (c - 'A' + 'a');
		else *t++ = (char) c;
	}
	*t = '\0';
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0;		/*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	(void) _getch(); 
	return hash;
}

#ifdef CONTROLBREAK
void ctrlbreak(int err) {
	(void) signal(SIGINT, SIG_IGN);		/* disallow control-C */
/*	cleanup();*/
	if (bAbort++ >= 3) exit(3);			/* emergency stop */
	signal(SIGINT, ctrlbreak);
}
#endif

/* static char downheader[] = "serverdict begin 0 exitserver\n"; */

void downhead(FILE * output) {
	unsigned int n, m;

	fprintf(output, "serverdict begin "); 
	n = strlen(password);
	m = strspn(password, "+-0123456789");
	if (strcmp(password, "") == 0) fprintf(output, "0");	/* default */
	else if (n == m) fprintf(output, "%s", password);		 /* number */
	else if (*password == '(' && *(password+n-1) == ')') {
		fprintf(output, "%s", password);
	}
	else fprintf(output, "(%s)", password);
	fprintf(output, " exitserver\n"); 	
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

int underscore(char *filename) { /* convert font file name to Adobe style */
	int k, n, m;
	char *s, *t;

	s = stripname(filename);
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
#ifdef DEBUGGING
	if (traceflag != 0) printf("n = %d m = %d ", n, m);
#endif
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
#ifdef DEBUGGING
	if (traceflag != 0) printf("Now trying file name: %s\n", s);
#endif
	return 0;
}

/***************************************************************************/

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
/*		if (n >= MAXPATHLEN) { */
		if (n >= FILENAME_MAX) {
			fprintf(stderr, "Path too long %s\n", searchpath);
			return NULL;
		}
		strncpy(pathname, searchpath, (unsigned int) n);
		*(pathname + n) = '\0';				/* terminate it */
		return(s + 1);						/* next pathname or NULL */
	}
	else {
		n = (int) strlen(searchpath);
/*		if (n >= MAXPATHLEN) { */
		if (n >= FILENAME_MAX) {
			fprintf(stderr, "Path too long %s\n", searchpath);
			return NULL;
		}
		strcpy(pathname, searchpath);
		return(searchpath + n);
	}
}

/***************************************************************************/

FILE *openpfbsub (char *name, char *fn_in, char *dir) {	/* moved 93/Dec/27 */
	FILE *input;
	char *s;
	
	if (traceflag) printf("name %s dir %s\n", name, dir);

	if (dir == NULL) return NULL;
/*	strcpy(fn_in, name); */
	if (strcmp(dir, "") == 0) strcpy(fn_in, name);
	else {
		strcpy(fn_in, dir);
		s = fn_in + strlen(fn_in);
		if (*s != '\\' && *s != '/') strcat (fn_in, "\\");
		strcat (fn_in, stripname(name));
	}
	extension(fn_in, "pfb"); 
	if((input = fopen(fn_in, "rb")) == NULL) {
		underscore(fn_in);
		if((input = fopen(fn_in, "rb")) == NULL) {
/*			strcpy(fn_in, name); */
			if (strcmp(dir, "") == 0) strcpy(fn_in, name);
			else {
				strcpy(fn_in, dir);
				s = fn_in + strlen(fn_in);
				if (*s != '\\' && *s != '/') strcat (fn_in, "\\");
				strcat (fn_in, stripname(name));
			}
			forceexten(fn_in, "pfa"); 
			if((input = fopen(fn_in, "rb")) == NULL) {
				underscore(fn_in);
				if((input = fopen(fn_in, "rb")) == NULL) {
/*					perror(name);  exit(3); */
				}
			}
		} 
	} 
	return input;
}

FILE *openpfb (char *name, char *fn_in, char *path) {	/* moved 93/Dec/27 */
	FILE *input;
	char *searchpath;
	char dir[FILENAME_MAX];
	
	if ((input = openpfbsub (name, fn_in, "")) != NULL) return input;
	searchpath = path;
	for (;;) {
		if ((searchpath=nextpathname(dir, searchpath)) == NULL) break;
		if (traceflag) printf("next dir %s\n", dir);
		if ((input = openpfbsub (name, fn_in, dir)) != NULL) return input;
	}
	return NULL;
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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	int m, firstarg=1;
	char *s;
	char *psfonts=NULL;
	
#ifdef CONTROLBREAK
	(void) signal(SIGINT, ctrlbreak);
#endif

	strcpy(fn_in, "");		/* in case not specified 1993/Aug/15 */

	firstarg = commandline(argc, argv, 1);
	
	if (firstarg > argc - 1 && 
		rebootflag == 0 && fontdirectory == 0 && stdinflag == 0)
		showusage(argv[0]);

	if (strcmp(outputfile, "") == 0) {	/* was output file specified ? */
/*	if user specified reboot, font directory, or input from stdin */
/*  or permanent download, or load to hard-disk then need destination */
		if (rebootflag != 0 || fontdirectory != 0 || stdinflag != 0 ||
			permanentflag != 0 || strcmp(fontname, "") != 0) { 
			if (firstarg > argc - 1) {			/* ??? */
				fprintf(stderr, "ERROR: No destination specified!\n");
		fprintf(stderr, "       Output will be placed in current directory\n");
				if (traceflag != 0) fprintf(stderr, "firstarg %d - argc %d\n",
						firstarg, argc);
				showusage(argv[0]);
			}
			else fprintf(stderr, "WARNING: No destination specified!\n");
		}
/*	following new 1993/Aug/16 */
		else fprintf(stderr, "WARNING: No destination specified!\n");
	}

	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("PSFONTS")) != NULL) psfonts = s;
/*	else if (usedviwindo) {  */
	if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("PSFONTS")) != NULL) psfonts = s;
	}

	if (psfonts == NULL) {	/* if can't find PSFONTS */
		if (useatmini) {
			if (setupatmini() != 0) {
				s = grabenvvar ("PFB_Dir", atmini, atmsection, useatmini);
			}
		}
	}		

/*	scivilize(compiledate);	 */
	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

/*	if (traceflag != 0) printf("Checking data\n"); */

	if (checkcopyright(copyright) != 0) exit(1);

/*	if (traceflag != 0) printf("Opening files\n"); */

	if (fontdirectory != 0 || rebootflag !=0) queryflag = -1;

	if (strcmp(outputfile, "") != 0) {	/* output file specified ? */
/*		strcpy(fn_out, outputfile); */
		lowercase(fn_out, outputfile);		
/*		printf("strlen %d ", strlen(fn_out)); */
		if (strcmp(fn_out, "prn") == 0 || 
			strcmp(fn_out, "aux") == 0 || 
			strcmp(fn_out, "ept") == 0 ||
				((strlen(fn_out) == 4) && 
				 (strncmp(fn_out, "lpt", 3) == 0 || 
				  strncmp(fn_out, "com", 3) == 0) &&
					 *(fn_out+3) >= '0' && *(fn_out+3) <= '5')) {
			directprint = 1;
			multifile = 0;
		}
		else {								/* not direct to printer */
/*			extension(fn_out, "pfa");  */	/* 1997/Dec/17 */
			directprint = 0; 
			multifile = 1;
		}
	}
	else {									/* no output file specified */
		strcpy(fn_in, argv[firstarg]);		/* temporary space */
		if (currentdirect != 0) {
			if ((s=strrchr(fn_in, '\\')) != NULL) s++;
			else if ((s=strrchr(fn_in, '/')) != NULL) s++;
			else if ((s=strrchr(fn_in, ':')) != NULL) s++; 
			else s = fn_in;
			strcpy(fn_out, s);  	/* strcat */
		}
		else strcpy(fn_out, fn_in);  /* strcat */

/*		strcpy(fn_out, argv[firstarg]); */
		forceexten(fn_out, "pfa");
		directprint = 0;
		multifile = 1;
/*		if (strcmp(fn_out, fn_in) == 0) {
			strcpy(fn_bak, fn_in);
			forceexten(fn_in, "bak");
			(void) remove(fn_in);
			(void) rename(fn_bak, fn_in);
		} */ 		/* removed again 1993/Aug/15 */
	}

	if (traceflag != 0) {
		printf("Output goes to %s ", fn_out); 
		printf("- multifile %d - directprint %d \n", multifile, directprint); 
	}

	if (fontdirectory != 0 || rebootflag !=0) multifile = 0; /* debug ??? */

	if (multifile == 0) { 
/*		if (strcmp(fn_out, fn_in) == 0) {
			strcpy(fn_bak, fn_in);
			forceexten(fn_in, "bak");
			(void) remove(fn_in);
			(void) rename(fn_bak, fn_in);
		} */ 		/* removed again 1993/Aug/15 */
		if ((output = fopen(fn_out, "wb")) == NULL) {
			perror(fn_out); exit(4);
		}
		if (rebootflag != 0) {
			downhead(output);
			fprintf(output, "%s", reboot);
			printf("Please wait for printer to reinitialize itself\n");
			if (wantcontrold != 0) 	putc(4, output);	/* ??? */
			fclose(output);
			exit(0);
		}
/*		if (fontdirectory != 0) {
			fprintf(output, "%s", directory);
			putc(4, output);
			fclose(output);
			exit(0);
		} */
		fprintf(output, "%s", adobehead);
		if (queryflag != 0) fprintf(output, " Query\n");
		else fprintf(output, "\n");
		if (signatureflag != 0) fprintf(output, "%s", signature);
		if (permanentflag != 0) downhead(output);
/*			fprintf(output, "%s", downheader); */
		else if (strcmp(fontname, "") != 0) {
			fprintf(output, "/fd (fonts/%s) (w) file def\n", fontname);
			fprintf(output, "%s", harddisk);
		}
	}

	if (stdinflag != 0) argc++;

	if (traceflag) printf("firstarg %d argc %d\n", firstarg, argc);

	for (m = firstarg; m < argc; m++) {
		if (stdinflag != 0) {
			strcpy(fn_in, "stdin");
			input = stdin;
		}
		else {
			if ((input = openpfb(argv[m], fn_in, psfonts)) == NULL) {
				perror(argv[m]);	exit(3);
			}
/*			strcpy(fn_in, argv[m]);
			extension(fn_in, "pfb"); 
			if((input = fopen(fn_in, "rb")) == NULL) {
				underscore(fn_in);
				if((input = fopen(fn_in, "rb")) == NULL) {
					strcpy(fn_in, argv[m]);
					forceexten(fn_in, "pfa"); 
					if((input = fopen(fn_in, "rb")) == NULL) {
						underscore(fn_in);
						if((input = fopen(fn_in, "rb")) == NULL) {
							perror(fn_in); exit(3);
						}
					}
				} 
			} */	/* moved 93/Dec/27 */
		}
		if (traceflag != 0) {
			if (input != NULL) printf("Have a file open\n");
			else printf("DO *not* have a file open\n");
		}
		if (chatterflag != 0 && stdinflag == 0) 
			printf("Decompressing %s\n", fn_in);

		if (multifile != 0) {
			if (currentdirect != 0) {
				if ((s=strrchr(fn_in, '\\')) != NULL) s++;
				else if ((s=strrchr(fn_in, '/')) != NULL) s++;
				else if ((s=strrchr(fn_in, ':')) != NULL) s++;
				else s = fn_in;
				strcpy(fn_out, s);  	/* strcat */
			}
			else strcpy(fn_out, fn_in);  /* strcat */
			forceexten(fn_out, "pfa");
/*			if (strcmp(fn_out, fn_in) == 0) {	
				strcpy(fn_bak, fn_in);
				forceexten(fn_in, "bak");
				(void) remove(fn_in);
				(void) rename(fn_bak, fn_in);
			} */
			if (strcmp(fn_out, fn_in) == 0) {
				forceexten(fn_out, "out");		/* safety valve ... */
				fprintf(stderr, "Output going to %s\n", fn_out);
			}
			if((output = fopen(fn_out, "wb")) == NULL) {
				perror(fn_out); exit(4);
			}
			fprintf(output, "%s", adobehead);
			if (queryflag != 0) fprintf(output, " Query\n");
			else fprintf(output, "\n");
			if (signatureflag != 0) fprintf(output, "%s", signature);
			if (permanentflag != 0) downhead(output);
/*				fprintf(output, "%s", downheader); */
		}

		decompress(input, output);

		if (chatterflag != 0) printf("\n");

		if (ferror(input)) {
			perror(fn_in); exit(3);
		}
		else fclose(input);

		if (multifile != 0) {
			if (ferror(output)) {
				perror(fn_out); exit(5);
			}
			else fclose(output);
		}
		if (bAbort > 0) abortjob();			/* 1992/Nov/24 */
	}

	if (multifile == 0) {
		if (fontdirectory != 0) fprintf(output, "%s", directory);
/*		if (wantcontrold != 0 && permanentflag != 0) putc(4, output); */
		if (wantcontrold != 0) putc(4, output);
		if (ferror(output)) {
			perror(fn_out); exit(5);
		}
		else fclose(output);
	}
	if (chatterflag != 0 && (argc - firstarg > 1))
		printf("Decompressed %d files\n", argc - firstarg);
	return 0;
}
	
/* Give user command line choices: */
/* download temporary or permanent (exitserver) */
/* download to VM or hard disk */
/* send to file or printer */

/* if it goes to printer, don't open and close for every file */

/* write output into current directory (if not to printer) */

/* possibly columns non-zero when start into binary ? */

/* insert newline at end of binary section ? */

/* strip out comment lines ? */

/* insert exitserver only after comment lines ? */

/* for long downloads may need MODE LPT1:,,B on parallel printer */

/* need a way to list font directory OK */
/* need a way to reset printer OK */

/* provide for input of PostScript from terminal or command line */
/* finish it off with control-Z ? use stdin OK */

/* deal with lower case LPT, COM, PRN, EPT ? */

/* problem when no output specified and it overwrites input file */

/* [Setup]
   PFM_Dir=c:\psfonts\pfm
   PFB_Dir=c:\psfonts */ /* in atm.ini */

