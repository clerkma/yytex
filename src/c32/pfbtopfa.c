/****************************************************************************/
/* pfbtopfa.c		(c) Copyright Y&Y 1990 - 1992							*/
/*					All Rights Reserved										*/
/*					Unpublished and confidential material.					*/
/*					--- Do not Reproduce or Disclose ---					*/
/****************************************************************************/

/****************************************************************************
*																			*
* Read compressed binary format Adobe Type 1 font files and expand			*
* binary sections in hex, download to printer if output directed there.		*
*																			*
* Will also download ordinary ASCII files.									*
* Can handle multiple files on command line, as well as wild cards.			*
*                                                                         	*
* Copyright (C) 1990 - 1992 Y&Y --- All rights reserved	        			*
*                                                                         	*
*	NOTE: This used be used for more than just PFB to PFA conversion:		*
*                                                                         	*
* Typical old use: 	pfbtopfa -p c:\cm -e pfb -f foo.log foo.ps				*
* where the font files are in directory c:\cm, with extension "pfb",		* 
* while a list of font files is found in foo.log, 							*
* and the PS file of the text itself in foo.ps,								*
* the output goes to the line printer, in this case.						*
*																			*
****************************************************************************/

#include <io.h>			/* for mktemp */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* following needed so can copy date and time from PFA to PFB file */

#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>

/* #define VERSION "1.3" */				/* version of this program */
#define VERSION "1.4"				/* version of this program */

/* #define MODDATE "1992 February 1" */	/* date of last modification */

/* #define FNAMELEN 80 */
#define PSNAMELEN 80		/* max PS name length */
#define MAXLINE 512			/* maximum line length in command file */

/* #define MAXENCODE 22 */	/* (14) maximum characters in character name */

/* #define TRACE */				/* voluminous output */
/* #define DEBUGFLUSH */		/* non-zero means no buffering */

#define MAXCHRS 256

/* #define PACKCHARUSED 1 */	/* want char used packed in hex */
/* #define ENCODCHRS 1 */	/* use modified StandardEncoding ??? */

#define NLONEOF 0			/* add new line at end of file */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* following not referenced */

char *copyright = "\
Copyright (C) 1990-1992  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* #define COPYHASH 9820367 */
#define COPYHASH 9820367

FILE *errout=stdout;

/* char *copyright = "\
Copyright (C) 1990 - 1992  Y&Y. All rights reserved. (508) 371-3286\
"; */

/* #define ENDECRYPT 0 */	/* non-zero => allow for encryption/decryption */

/* this will take more work  since the CharString is encoded differently */
/* this will take more work  since binary stuff is in hex if visible */
/* this will take more work  since output has to be re-encrypted */

int convertcr=1;			/* convert CR to NL on input */
int flushcr=0;				/* flush CR on input */	/* not used */
int flushnl=0;				/* flush NL on input */ /* not used */

int nextdsp = 0;			/* 64 column, lower case hex */

int columns=78;			/* max line length in hex output */

int hexten = 'A';			/* hex 10 */

char *task;  					/* current activity - for error message */

int verboseflag=0;		/* non-zero => lots of output */
int traceflag=0;		/* non-zero => lots of output */
int detailflag=0;
int copydate=1;			/* copy input file date to output */
int directprint=0;		/* non-zero => go to printer */
int clmnflag=0;			/* next arg is number of columns */
int outputflag=0;		/* next arg is output file */
int charonly=1;			/* load only characters specified (needed) */
int clm=0;				/* current output column */
int firstline=1;		/* for first line of first file ? */
int errlevel=0;			/* returned at end - changed if unhappy */

/*	int minzeros=0;	 */	/* 256 number zeros required after binary section */
/* 	int extenflag=0; */		/* next arg is extension */
/*	int pathflag= 0; */		/* next arg is path */
/*	int commandflag=0; */	/* non-zero read file names from command file */

char *outputfile = "";		/* default output file */

/* char *ext = "pfb"; */		/* default extension for font files */
/* char *path = "c:\\cm"; */	/* default path for font files */ /* NA */
/* char *commandfile = ""; */	/* command file if specified */ /* NA */

static char line[MAXLINE];

/*	int encodeflag=ENCODCHRS;	*/

/* The magic encoding vector used to allow across-job font caching (C) */
/* not needed for straight PFB to PFA conversion, obviously */

#ifdef ENCODCHRS

/* static unsigned char encoding[MAXCHRS][MAXENCODE]; */

/* The magic encoding vector used to allow across-job font caching (C) */
static char *defaultencoding[] = {
 "Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Upsilon", 
 "Phi", "Psi", "Omega", "alpha", "beta", "gamma", "delta", "epsilon", 
 "zeta", "eta", "theta", "iota", "kappa", "lambda", "mu", "nu", 
 "xi", "pi", "rho", "sigma", "tau", "upsilon", "phi", "chi", 
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
 "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "dieresis" };

/* First half of standard encoding vector */
/* not needed for straight PFB to PFA conversion, obviously */

static char *standardencoding[] = {
"", "", "", "", "", "", "", "",
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"space", "exclam", "quotedbl",  "numbersign",  "dollar",  "percent",  "ampersand",  "quoteright", 
"parenleft",  "parenright",  "asterisk",  "plus",  "comma",  "hyphen",  "period",  "slash", 
"zero",  "one",  "two",  "three",  "four",  "five",  "six",  "seven", 
"eight",  "nine",  "colon",  "semicolon",  "less",  "equal",  "greater",  "question", 
"at", "A", "B", "C", "D", "E", "F", "G", 
"H", "I", "J", "K", "L", "M", "N", "O", 
"P", "Q", "R", "S", "T", "U", "V", "W", 
"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", 
"quoteleft", "a", "b", "c", "d", "e", "f", "g", 
"h", "i", "j", "k", "l", "m", "n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w", 
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", ""};

#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

struct _stat statbuf;	/* struct stat statbuf;	 */

struct _utimbuf timebuf;	/* struct utimbuf timebuf; */

int getinfo(char *filename, int verboseflag) {
	char *s;
	int result;

	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(errout, "Unable to obtain info on %s\n", filename);
		return -1;
	}
	s = ctime(&statbuf.st_atime); 
	lcivilize(s);								
	if (verboseflag != 0) printf("%s last accessed: %s", filename, s);
	s = ctime(&statbuf.st_mtime);
	lcivilize(s);								
	if (verboseflag != 0) printf("%s last modified: %s", filename, s);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void giveup(int code) { 	/* graceful exit with meaningful error message */
	fprintf(errout, " while %s", task);
	fprintf(errout, " ");
/* 	fclose(fp_out);	remove(fn_out); */
	exit(code);
}

void tellwhere(FILE *input) {
	long position;
	position = ftell(input);
	fprintf(errout, " at byte %ld ", position-1);
}

/* maybe need a faster (hashing) scheme for the above ? if not ordered */

#ifdef IGNORE
void extension(char *fname, char *ext)  /* supply extension if none present */
{
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) /* change extension if one present */
{
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
/*	printf("FORCED EXTENSION %s\n", fname); */
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

/* void makename(char *fname, char *ext) {
    char *s;
    s = strrchr(fname,'\\');			
    if (s == NULL) {
		s = strrchr(fname,':');			
		if (s == NULL) strcpy(fname+2, "XXXXXX");
		else strcpy(s+2, "XXXXXX");		
	}
    (void) mktemp(fname);	
	forceexten(fname, ext);	
} */

unsigned long readlength(FILE *fp_in) {   /* read binary length code */
	int i, c, k = 0;
	unsigned long length=0L;

	for (i=0; i < 4; i++) {
		c = getc(fp_in);
#ifdef TRACE
/*		printf("%d ", c); */
#endif
/*		length = length | ((unsigned long) c << k); */
		length = length | (((unsigned long) c) << k);
		k = k + 8;
	}
	if (verboseflag != 0) printf("Length is %lu - ", length); 
	return length;
}

/* write a binary section unpacked in hex format */
int hexifybin(FILE *fp_in, FILE *fp_out, unsigned long length) {
	unsigned long k;
	int zeros=0;
	int c, d;
#ifdef TRACE
	long fbegin, fend;
#endif

/*	printf("CLM %d ", clm); */
	if (verboseflag != 0) printf (" Entering binary section\n"); 	
#ifdef TRACE
	fbegin = ftell(fp_in);
#endif
	for(k = 0; k < length; k++) {
		if ((c = getc(fp_in)) == EOF) {
			fprintf(errout, 
"ERROR: Unexpected EOF at character %lu of binary section of length %lu\n",
	k, length);
			break;
		}
		if (c == 0) zeros++; 
		else zeros = 0;

			if (clm > columns) { putc('\n', fp_out); clm = 0; }
			d = c & 15; 
			c = (c >> 4) & 15;
			if (c < 10) putc(c + '0', fp_out);
/*			else putc(c + 'A' - 10, fp_out); */
			else putc(c + hexten - 10, fp_out);
			clm++;
/*			if (clm >= columns) { putc('\n', fp_out); clm = 0; } */
			if (d < 10) putc(d + '0', fp_out);
/*			else putc(d + 'A' - 10, fp_out); */
			else putc(d + hexten - 10, fp_out);
			clm++;
			if (clm >= columns) { putc('\n', fp_out); clm = 0; }

	}
/*	if (verboseflag != 0) printf (" End of binary section %lu ", k); */
#ifdef TRACE
	fend = ftell(fp_in);
	if (verboseflag != 0) printf (" file advanced %ld ", fend - fbegin); 
#endif
	return zeros;
}

/************************************************************************/

int getline(char *s, int maxlen, FILE *fp_in) {
	/* read a line up to newline or EOF or buffer full */
	int c, k = 0 ;

	while ((c = getc(fp_in)) != '\n' && c != EOF && k++ < maxlen-2) 
/*		*s++ = (unsigned char) c; */
		*s++ = (char) c;
	if (c != EOF) {
/*		*s++ = (unsigned char) c;  */
		*s++ = (char) c; 
		k++;
	}
	*s++ = '\0'; 
	if (c == EOF && k == 0) return EOF;
	else return k;
}

/* output what is there in standard ATM format - or some other ? */
/* or in abbreviated format - since it is going to the printer ? */
/* problem due to remapping idea */

/* make output encoding vector only as large as it really needs to be */
/* use ZapfDingbat encoding on output ? */

/************************************************************************/

void flushchar(FILE *fp_in) { /* flush this CharString */
	unsigned long n, k;
	int c;

#ifdef TRACE
	printf(".");
#endif
	if ((c = getc(fp_in)) != '<') {
		fprintf(errout, "ERROR: Expecting CharString at this point %d\n", c);
		tellwhere(fp_in);
		giveup(7);
	}
	c = getc(fp_in);
	if (c == 128) {
		if ((c = getc(fp_in)) != 2) {
			fprintf(errout,
				"ERROR: Expecting binary section at this point, not %d\n", c);
			tellwhere(fp_in);
			giveup(7);
		}
		n = readlength(fp_in);
		for (k = 0L; k < n; k++) (void) getc(fp_in);
	}
	else { 		/* not binary encoded ? */ 
		while ((c = getc(fp_in)) != '>' && c != EOF) ;
		if (c == EOF) {
			fprintf(errout, "ERROR: Unexpected EOF in <...>\n");
			tellwhere(fp_in);
			giveup(7);
		}
		(void) ungetc(c, fp_in);	/* put back terminating > */
	}
	while ((c = getc(fp_in)) <= ' ') ;
	if (c != '>') {
		fprintf(errout, 
			"ERROR: Expecting end of CharString at this point %d\n", c);
		tellwhere(fp_in);
		giveup(7);
	}
	while ((c = getc(fp_in)) <= ' ') ;
	if (c != 'N') {
		fprintf(errout, 
			"ERROR: Expecting end of CharString at this point %d\n", c);
		tellwhere(fp_in);
		giveup(7);
	}
	c = getc(fp_in); 		/* suck in the 'D' in 'ND' */
	while ((c = getc(fp_in)) > ' ') ;   	/* skip to white space */
	while ((c = getc(fp_in)) <= ' ') ;		/* skip over white space */
	(void) ungetc(c, fp_in); 
	clm = 0; 
}

void dumpbuf(FILE *fp_out, char *namebuf) { /* dump out name buffer */
	int c;

	putc('/', fp_out);
	while ((c = *namebuf++) != '\0') {
		putc(c, fp_out); clm++;
	}
}

void dumpnum(FILE *fp_out, char *namebuf) { /* dump out name buffer */
	int c;

/*	putc('/', fp_out); */
	while ((c = *namebuf++) != '\0') {
		putc(c, fp_out); clm++;
	}
}

char *specialchar (int c) {					/* 1994/Oct/13 */
	if (c == 167) return "(beta)";
	else if (c == 168) return "(R)";
	else if (c == 169) return "(C)";
	else if (c == 170) return "(TM)";
	else if (c == 208) return "--";
	else if (c == 209) return "---";
	else return NULL;
}

void decompress(FILE *fp_in, FILE *fp_out, char *wantchrs, int count) { 
/*	decompress file */
	int c, d, t, fchr;
	int chr = -1, encodingseen = 0, password = 0;
/*	int nevec; */
	int charuse=0;
	int zeros = -1;				/* never accessed */
	char namebuf[PSNAMELEN];
	char *nameptr;
	unsigned long length, k;
	int badchar=0;
	char *s;

/*	if (verboseflag != 0) printf ("Entering decompression loop \n");   */

	clm=0;

	c = getc(fp_in); 
	(void) ungetc(c, fp_in);	/* 92/02/03 */
	if (c != 128) {
		fprintf(errout,
			"Does not appear to be a valid PFB file, starts with %d\n", c);
		return;
	}

	while ((c = getc(fp_in)) != EOF) {
		if (convertcr != 0) {
			if (c == '\r') {
				c = '\n';
/* check ahead whether `return' `newline' sequence */
				d = getc(fp_in);
				if (d != '\n') (void) ungetc(d, fp_in);
			}
		}
		else if (flushcr != 0) {
			if (c == '\r') c = getc(fp_in);
		}
		else if (flushnl != 0) {
			if (c == '\n') c = getc(fp_in);
		}
		if (c < 128) {					/* normal character */
/*			if (c == '\n')  */
			if (c == '\n' || c == '\r')		/* 1992 Jan 15 ??? */
				clm = 0; 
			else clm++;  
			if ((clm == 1) && (charonly != 0)) {
				if (chr >= 0 && c >= '0' && c <= '9') { /* numeric chr code */
					fchr = c - '0';
					while ((c = getc(fp_in)) >= '0' && c <= '9') 
						fchr = fchr * 10 + (c - '0');
					if (c != '<') {
						(void) ungetc(c, fp_in);
						sprintf(namebuf, "%d", fchr);
						dumpnum(fp_out, namebuf);
					}
					else { /* start of a CharString */
						if (fchr < 0 || wantchrs[fchr] == 0) {
							(void) ungetc(c, fp_in);
							flushchar(fp_in); 
						}
						else { /* want this character */
							(void) ungetc(c, fp_in);
							sprintf(namebuf, "%d", fchr);
							dumpnum(fp_out, namebuf);
						}
						chr++; /* count characters seen in this font file */ 
					}
				}	
				else if (c == '/') { 		/* get PS name */
					nameptr = namebuf;
					c = getc(fp_in);
					while ((c >= 'A' && c <= 'Z') || 
							(c >= 'a' && c <=	'z')) {
/*						*nameptr++ = (unsigned char) c; */  /* overflow ? */
						*nameptr++ = (char) c;
						assert(nameptr - namebuf < PSNAMELEN);
						c = getc(fp_in);
					}
					*nameptr++ = '\0'; 
					(void) ungetc(c, fp_in);
					if (chr < 0) {	/* not started on characters yet */
						if (encodingseen == 0 && count > 0) {
/*							if (strcmp(namebuf, "Encoding") == 0) {
								nevec = readencoding(fp_in);
								charuse = 
									writeencoding(fp_out, wantchrs, nevec);
								encodingseen = 1;
								*namebuf = '\0';
							} 
							else dumpbuf(fp_out, namebuf); */
							dumpbuf(fp_out, namebuf);
						}
						else {
							dumpbuf(fp_out, namebuf);
							if (password == 0) { /* not seen password yet */
								if (strcmp(namebuf, "password") == 0) 
									password = 1;
							}
							else if (strcmp(namebuf, "UniqueID") ==	0) {
								(void) getline(line, MAXLINE, fp_in);
								clm = 0;
								printf("/UniqueID %s", line);
								fprintf(fp_out, "%s", line);
							/* seen UniqueID line, ready for characters */
								(void) getline(line, MAXLINE, fp_in);
								clm = 0;
								printf("%s", line);
								if (strstr(line, "CharStrings") == NULL) { 
									fprintf(errout, "ERROR: Expecting CharStrings");
									tellwhere(fp_in);
									giveup(7);
								}
								fprintf(fp_out, 
								"2 index /CharStrings %d dict dup begin\n", 
									charuse); /* ugh */
								chr = 0; /* ready for CharStrings now */
							}
						}
					}
					else {	/*  already searching for characters */
#ifdef TRACE
/*						printf("Found character %s\n", namebuf); */
						printf(" %s", namebuf);
#endif
/*						if (encodeflag != 0) {  */
/*						fchr = encodeindex(namebuf, chr); */
						fchr = chr;	/* what bullshit ? */
					    if (fchr < 0 || wantchrs[fchr] == 0) 
							flushchar(fp_in); 
						else dumpbuf(fp_out, namebuf);
/*						} else {	
						if (wantchrs[chr] == 0) flushchar(fp_in);
						else dumpbuf(fp_out, namebuf); } */
						chr++; /* count characters seen in this font */
					}
				}
				else putc(c, fp_out);	/* copy normal character to output */
			}
			else putc(c, fp_out);	/* copy normal character to output */
		}
		else if ((s = specialchar(c)) != NULL) {	/* not < 128 */
			fputs(s, fp_out);
			if (verboseflag) fputs(s, stdout);
		}
		else if (c > 128) {				/* not a magic mark */
/*			following imported from MACtoPFA 1994/June/15 */ /* now above */
/*			if (c == 169) {
				fprintf(fp_out, "(C)");
				if (verboseflag != 0) printf("(C)\n");
			}
			else if (c == 168) {
				fprintf(fp_out, "(R)");
				if (verboseflag != 0) printf("(R)\n");
			}
			else if (c == 209) {
				fprintf(fp_out, "---");
				if (verboseflag != 0) printf("---\n");
			}
			else if (c == 208) {
				fprintf(fp_out, "--");
				if (verboseflag != 0) printf("--\n");
			}
			else { */
/*				if (badchar++ > 32) { */
				if (badchar++ > 128) {
				fprintf(errout, "ERROR: Too many bad characters\n");
					giveup(3);
				}
				fprintf(errout, 
					"WARNING: Meta character (%d) in ASCII section ", c);
				tellwhere(fp_in);
/*				giveup(3); */
/*			following imported from MACtoPFA 1994/June/15 */
				c -= 128;
				if (c < 32) {
					c += 64;
					fprintf(errout, "M-C-%c ", c);
/*					fprintf(fp_out, "M-C-%c", c); */
				}
				else {
					fprintf(errout, "M-%c ", c);
/*					fprintf(fp_out, "M-%c", c); */
				}
				putc('\n', stdout);
/*			} */
		}
		else if (c == 128) {		/* magic marker */
			t = getc(fp_in);		/* get type of section */
/*			printf("CLM %d T  %d ", clm, t); */
			if (t == 3) {
/*				fclose(fp_out); exit(0);	*/
				break;		/* EOF */
			}
			if (t < 1 || t > 4) {
				fprintf(errout, "ERROR: Unrecognized record type %d\n", t);
				tellwhere(fp_in);
				giveup(3);
			}	
			length = readlength(fp_in); 	/* get length */
			if (t == 2) zeros = hexifybin(fp_in, fp_out, length);
			else if (t == 4) 
				for (k = 0; k < length; k++)  {
					putc('0', fp_out); clm++;
					if (clm >= columns) { putc('\n', fp_out); clm = 0; }
				}
			else {						/* type 1 - ASCII section */
				if (firstline != 0) {	/* flush cr and nl at first */
					while ((c = getc(fp_in)) == '\r' || c == '\n') ;
					(void) ungetc(c, fp_in);
					firstline = 0;
				}
			if (verboseflag != 0) printf(" Entering ASCII section\n");
			if (clm > 0) putc('\n', fp_out);	/* end hexadecimal line ? */
/*				if (zeros >= 0 && zeros < minzeros) {
					if (verboseflag != 0) 
						printf("Inserting %d zeros \n", minzeros - zeros);
					while (zeros++ < minzeros) putc(0, fp_out);
					zeros = -1;
				} */
				clm = 0; 
			}
		}
	}
	if (verboseflag != 0) printf ("EOF\n");
#if NLONEOF != 0
	putc('\n', fp_out);  /* add new line between files */
#endif
}

void decodeflag (int c) { 			/* decode command  line flag */
	switch(c) { 
		case '?': detailflag = 1; break;
		case 'v': verboseflag = 1; break;
		case 'r': flushcr = 1; break;
		case 'n': convertcr = 0; break;		
		case 'd': outputflag = 1; break;
/*		case 'e': extenflag = 1; break; */
		case 'a': charonly = 0; break;
		case 'h': nextdsp = 1; break;
/*		case 'f': commandflag = 1; break; */
/*		case 'p': pathflag = 1; break; */
/*		case 'z': minzeros = 0; break; */
		default: {
				fprintf(errout, "Invalid command line flag '%c'\n", c);
				giveup(7);
		}
	}
}

/*	this used to be used for  thigns other than PFB to PFA conversion ... */

/*	fprintf (errout, "\ta: load all characters (even if not used)\n"); */
/*	fprintf (errout, "\tf: next argument is file that contains list of font files\n"); */
/*	fprintf (errout, 
"\tp: next argument is path for font files (default '%s')\n", path); */
/*	fprintf (errout, 
"\te: next argument is extension for font files (default '%s')\n", ext); */
/*	fprintf (errout, "\tz: add no zeros at end of binary sections\n"); */

/*		"%s [-{v}] [-c <clms>] [-o <out-file>] <file-1> ...\n", s); */
/*	fprintf(errout, */
/* "\tc: next argument is number of columns to use in output (default
'%d')\n",  columns); */

void showusage(char *s) {
	fprintf (errout, "Correct usage is:\n");
    fprintf (errout, "%s [-{v}{n}{h}] [-d=<destination>] <pfb-file-1> <pfb-file-2> ...\n", s);
	fprintf (errout, "\tv: verbose mode\n");
/*	fprintf (errout, "\tr: flush `return's in text of input\n"); */
	fprintf (errout, "\tn: do NOT convert `return's to `newlines's on input\n");
	fprintf (errout, "\th: use lower case in hex code (and use 64 columns)\n");
	fprintf (errout, "\td: next argument is destination\n");
	fprintf (errout, 
"\t   (default is file in current directory, same name, extension 'pfa')\n");
	exit(1);
}

void dofile(FILE *fp_in, FILE *fp_out, char *wantchrs, int count) {
	int c;
/*	if (verboseflag != 0) printf( "Opened Files\n");  */
		
	c = getc(fp_in); 
	(void) ungetc(c, fp_in);	/* 92/02/03 */
	if (c != 128) {
		fprintf(errout, "Does not appear to be a valid PFB file\n");
		return;
	}
	task = "decompressing file contents";
	decompress(fp_in, fp_out, wantchrs, count);

	task = "closing input file";
	fclose(fp_in);

/*	if (verboseflag != 0) printf("Closed files\n");  */
}

/* OLD Command usage:   */
/* 		loadcomp [-{v}{c}] [-p <path>] [-e <ext>] [-f <com-file>] */
/*						 <file-1> <file-2> ... {LPT|<out-file>} */

char *stripname(char *pathname) {
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

	s = stripname(filename);
	n = (int) strlen(s);
	if ((t = strchr(s, '.')) == NULL) t = s + strlen(s);
	m = t - s;	
	memmove(s + 8, t, (unsigned int) (n - m + 1));
	for (k = m; k < 8; k++) s[k] = '_';
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(errout, "HASHED %ld\n", hash);	/* (void) _getch();  */
/*	fflush(errout); */
	return hash;
}

int main(int argc, char *argv[])  {      /* main program */
    FILE *fp_in, *fp_out;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
/*	char fontname[MAXLINE]; */
	unsigned int i;
	char *s;
	int c, m, n;
	int k;
	int firstarg=1;
	char wantchrs[MAXCHRS];		/* get rid of this later ... */
 
	task = "interpreting command line";

/*	printf("%d arguments \n", argc); */
/* 	for (i=0; i < argc; i++) printf("%s ", argv[i]); */

	if (argc < 2) showusage(argv[0]);  /* check for no argument case */
		
/*	if ((s = getenv("CMFONTS")) != NULL) path = s; */

/*	while (argv[firstarg][0] == '-') /* check for command flags */
	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
/*		printf("LENGTH = %d\n", strlen(argv[firstarg]) ); */
		c = 0;						/* to keep compiler quite */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			c = argv[firstarg][i];
			if (c == '\0' || c == '=') break;
			else decodeflag(c);
		}
		if (outputflag != 0) { /* if output file specified */
			if (c == '=') outputfile = argv[firstarg]+i+1;
			else outputfile = argv[++firstarg]; 
			outputflag = 0;
		}
		if (clmnflag != 0) { /* number of columns specified */
			if (c == '=') s = argv[firstarg]+i+1;
			else s = argv[++firstarg];
			sscanf(s, "%d", &columns); 
			clmnflag = 0;
		} 
/*		if (pathflag != 0) {
			path = argv[firstarg];	firstarg++; pathflag = 0;
		}
		if (extenflag != 0) {	
			ext = argv[firstarg]; firstarg++; extenflag = 0;
		}
		if (commandflag != 0) {		
			commandfile = argv[firstarg]; firstarg++; commandflag = 0; 
		} */
		firstarg++; 
	}

	if (nextdsp != 0) {		/* 64 columns and lower case hex */
		columns = 64;
		hexten = 'a';
	}

	if (detailflag != 0) showusage(argv[0]);

	if (firstarg > argc - 1) showusage(argv[0]);

	if (checkcopyright(copyright) != 0) {
		fprintf(errout, "HASH %lu", checkcopyright(copyright));
		exit(1);	/* check for tampering */
	}

/*	if (verboseflag != 0) */
		printf( "PFBtoPFA font decompression program version %s\n",	VERSION);

/*	if (strlen(argv[firstarg]) <= 3) {ext = argv[firstarg]; firstarg++;} */

	strcpy(fn_in, argv[firstarg]); 	/* get input file name */

	if (strcmp(outputfile, "") != 0) 
		directprint = 1;			/* stick all output in one file */
	else directprint = 0;			/* one output file per input file */
	strcpy(fn_out, outputfile);
	
/*	if (strcmp(outputfile, "") == 0) {
		if ((s=strrchr(fn_in, '\\')) != NULL) s++;
		else if ((s=strrchr(fn_in, ':')) != NULL) s++;
		else s = fn_in;
		strcpy(fn_out, s);			
	}
	else strcpy(fn_out, outputfile); */
	
	if (directprint != 0) {
		if (strcmp(fn_out, "LPT1") == 0 || 
			strcmp(fn_out, "LPT2") == 0 || 
			strcmp(fn_out, "COM1") == 0 ||
			strcmp(fn_out, "COM2") == 0 ||
			strcmp(fn_out, "PRN") == 0) {
				n = (int) strlen(fn_out); 
				if (fn_out[n - 1] == ':')  fn_out[n - 1] = '\0'; 
		}
		else {
			forceexten(fn_out, "pfa");		/* add extension if file */
		}
		if (verboseflag != 0) printf("Output to %s\n", fn_out); 
	}

/*	if (firstarg >= argc - 1) {	
		showusage(argv[0]);  
		forceexten(fn_out, "out"); 
	}  */
	
#ifdef DEBUGFLUSH
 	setbuf(stdout, NULL);  /* serious stuff ! */
#endif

	task = "initializing files";

	if (directprint != 0) {
		fp_out = fopen(fn_out, "wb");
		if (fp_out == NULL) {  
			perror(fn_out); 
			giveup(2);
		}
#ifdef DEBUGFLUSH
	setbuf(fp_out, NULL); /* serious stuff */
#endif
	}
	else {	/* not direct print ! */
		fp_out = NULL;		/* to keep compiler quite */
	}		/* output file opened once per input file in this case */
	
	firstline = 1;

	for (k = 0; k < MAXCHRS; k++) wantchrs[k] = 1; 

	charonly = 0;		/* rest of files cannot be treated this way */

	for (m = firstarg; m < argc; m++) { /* do each file in command line */
		clm = 0;						/* ??? */
		strcpy(fn_in, argv[m]);			/* get next file name */
		extension(fn_in, "pfb");		/* if extension omitted */
		task = "opening files";
		fp_in = fopen(fn_in, "rb");
	    if (fp_in == NULL) {
			underscore(fn_in);			/* 1992/Nov/22 */
			fp_in = fopen(fn_in, "rb");
		}
		if (fp_in == NULL) {
			fprintf(errout, "Could not open file %s\n", fn_in); 
			errlevel++;
		}
		else {
			if (verboseflag != 0)
/*				printf( "Decompressing/Loading file %s\n", fn_in);  */
				printf( "Decompressing %s  ", fn_in);  
			if (strcmp(outputfile, "") == 0) {
				if ((s=strrchr(fn_in, '\\')) != NULL) s++;
				else if ((s=strrchr(fn_in, ':')) != NULL) s++;
				else s = fn_in;
				strcpy(fn_out, s);			
			}
			else strcpy(fn_out, outputfile);
			forceexten(fn_out, "pfa");
			if (directprint == 0) {
				if (verboseflag != 0) printf("output goes to %s\n", fn_out); 
				fp_out = fopen(fn_out, "wb");
				if (fp_out == NULL) {  
					perror(fn_out); 
					giveup(2);
				}
#ifdef DEBUGFLUSH
				setbuf(fp_out, NULL); /* serious stuff */
#endif
			}

			c = getc(fp_in); 
			(void) ungetc(c, fp_in);	/* 92/02/03 */
			if (c != 128) {
				fprintf(errout, "Does not appear to be a valid PFB file\n");
				exit(1);
			}
/* Worry about fp_out not being initialized ??? */
			dofile(fp_in, fp_out, wantchrs, 0);
			if (ferror(fp_out) != 0) {
				fprintf(errout, "Output error ");
				perror(fn_out); 
				exit(2);
			}
			if (directprint == 0) {
				fclose(fp_out); 

/* try and modify time/date of output file --- 1992/Oct/10 */
				if (copydate == 0) continue;
				if (getinfo(fn_in, traceflag) < 0) {
					exit(1);
				}
				timebuf.actime = statbuf.st_atime;
/*				timebuf.modtime = statbuf.st_atime; */
				timebuf.modtime = statbuf.st_mtime;
				if (_utime(fn_out, &timebuf) != 0) {
					fprintf(errout, "Unable to modify date/time\n");
					perror(fn_out);
					/*			exit(3); */
				}

				/*		see if it worked */
				if (getinfo(fn_out, traceflag) < 0) exit(1);
				
			}
		}
		if (verboseflag) fflush(stdout);
	}
	if (directprint != 0) {
		if (ferror(fp_out) != 0) {
			fprintf(errout, "Output error ");
			perror(fn_out); exit(2);
		}
		else fclose(fp_out); 
	}
/*	if (argc > firstarg + 1) printf( "Decompressed/Loaded %d files\n\n", */
	if (argc > firstarg + 1) printf( "Decompressed %d files\n\n", 
		argc - firstarg);
	return errlevel;
}

/* need to strip CRs when not in binary section of file OK */

/* is it always approriate to flush CR after ASCII length field ? */

/* do we need to worry about whether cr and nl are counted correctly ? */ 

/* return non-zero error level when a font could not be found ? OK */

/* alternatively, allow use of Encoding vector instead */

/* add ability to decrypt on input and encrypt on output ?  FLUSHED */

/* assumes characters appear in order in font files, starting at zero NO */ 

/* CharStrings need no longer be in numerical order */

/* Encoding vector may need to be updated */

/* fopen fp_out "w" ? */ 

/* translate ^M into ^J on input for Adobe fonts ? */

/* modify CharString Directory size according to need ? */

/* separate treatment of font files from that of other files ? */

/* modify the Encoding vector on output to contain only what is needed ? */

/* also, pick up Encoding vector on input so that character codes can
   be properly interpreted - not by using standard encoding vector ! */

/* Maybe loadcomp should also strip comments ? Too hard ? */

/* somewhat kludgey retro-fit of encoding vector read and write ... */ 

/* reencode fonts to minimize size of Encoding vector ! caching ? NO */

/* reduce size of CharStrings array ALSO */

/* want to treat /.notdef separately ? */

/* may need to do something about files where \r appears instead of \n */

/* particularly as regards clm = 0 for hex output */

/* convert \r to \n ? */

/* maybe only output \r in hexadecimal sections when input only has \r's */

/* give warning if ASCII section has wrong length ? */

/* do something about the letter duplicated at the end (cleartomarkk?) */

/* why replicates last character sometimes ? */

/* multiple files now merged into one output file ... */

/* may wish to convert C-M in input into C-M C-J ? */
/* may want to use C-M C-J instead of just C-J in hexadecimal output ? */
/* may wish to add newline at the very end ? */

/* takes multiple files for input, but concatenates all output to one file */

/* lots of options not mentioned in showusage anymore to avoid confusion */

/* should zeros at the end have `newlines' instead of `returns' ? */

/* lots of old stuff --- encoding vector stuff ? */

/* This already produces file with only ^J (not ^M^J) - convenient for Unix */
