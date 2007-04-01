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

/* New version of outline font compression program - cleaner and sleeker */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* #include <conio.h> */
#include <time.h>			/* for struct tm etc. */

/* following needed so can copy date and time from PFA to PFB file */

#include <sys\stat.h>		/* for _stat etc. */
#include <sys\utime.h>		/* for _utimbuf etc. */
/* #include <sys\types.h> */		/* for time_t also defined in time.h */

/* #define FNAMELEN 80 */
#define LENBYTES 4				/* bytes in PFB length fields */
#define MAXLINE 4096			/* not really needed that long anymore ? */
#define MINHEXLINE 16			/* ad hoc zero detection kludge threshold */
#define MINZEROLINE 16			/* ad hoc zero detection kludge threshold */

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* #define VERSION "1.3" */			/* version of this program */

#define VERSION "1.4"				/* version of this program 95/Mar/1 */

/* #define MODDATE "1992 February 1" */	/* date of last modification */

char *copyright = "\
Copyright (C) 1990 - 1995  Y&Y.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1990 - 1994  Y&Y. All rights reserved. (978) 371-3286\ */

/* #define COPYHASH 6439867 */
/* #define COPYHASH 7728703 */
#define COPYHASH 8548688

/* char *copyright = "\
Copyright (C) 1990 - 1992  Y&Y. All rights reserved. (508) 371-3286\
"; */

/* #define COPYHASH 14251529 */

int verboseflag = 0;
int traceflag = 0;
int copydate=1;				/* copy input file date to output */
int flushreturns = 0;		/* flush returns from input file */
int converttoreturn = 1;	/* convert newlines to returns for output */
int currentdirect = 1;		/* place output file in current directory */

int avoidmacflag=1;			/* clean up non 7-bit characters */

int errorflag=0;			/* how many errors noted */

char line[MAXLINE];

FILE *errout=stdout;

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

	if (traceflag)
		printf("Getting date and time for file %s\n", filename);
	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(errout, "ERROR: Unable to obtain info on %s\n", filename);
		return -1;
	}
	s = ctime(&statbuf.st_atime); /* last accessed */
	lcivilize(s);								
	if (verboseflag != 0) printf("%s last accessed: %s", filename, s);
	s = ctime(&statbuf.st_mtime);		/* last modified */
	lcivilize(s);								
	if (verboseflag != 0) printf("%s last modified: %s", filename, s);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* returns number of hex characters or -1 if not all hex line */

int lineallhex(char *line) {
	char *s=line;
	int c, k=0;

	while((c = *s++) != '\0') {
		if ((c >= '0' && c <= '9') ||
			(c >= 'a' && c <= 'f') ||
			(c >= 'A' && c <= 'F'))	k++;
		else if (c <= ' ') ;
		else return -1;
	}
	return k;		/* all characters are hex and/or white space */
}

/* returns number of zeros or -1 if not all zeros */

int lineallzeros(char *line) {
	char *s=line;
	int c, k=0;

	while((c = *s++) != '\0') {
		if (c == '0') k++;
		else if (c <= ' ') ;
		else return -1;
	}
	return k;	/* all characters are zeros and/or white space */
}

/* check whether ends in 64 zeros - kludge 95/Mar/1 */

int lineendszeros(char *line) {			/* 95/Mar/1 */
	char *s, *t;

	s = line + strlen(line) - 1;
	t = s;
	while (s >= line && (*s == '0' || *s <= ' ')) s--;
	if (s < t) return t - s;
	else return 0;
}

/* attempt to deal with lack of line terminator before zeros at end */

int trimlineend (char *line, FILE *input, int flag, char *fn_in) {
	char *s, *t;
	int c;

	if (flag) {
		fprintf(errout,
	"\nWARNING: Cleaning up bad transition to 512 zeros at end in %s\n", fn_in);

	}
	s = line + strlen(line);
	s--;
	t = s;
	while (s >= line && (*s == '0' || *s <= ' ')) s--; 
	fseek(input, s - t - 1, SEEK_CUR);	/* step back over zeros */
	c = getc(input);
	while (c != '0' && c != EOF) {
		printf(" NOT ZERO %c ", c);
		c = getc(input);
	}
	ungetc(c, input);
	s++;
	*s++ = '\n';
	*s++ = '\0';
	return (t - s);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void writelength(FILE *output, unsigned long n) {
	int i;
	
	for (i = 0; i < LENBYTES; i++) {
		putc((unsigned int) (n & 255), output);
		n = n >> 8;
	}
}

void writeasciilength(FILE *output, unsigned long length) { /* type 1 field */
	putc(128, output); 		/* truncation of constant value */
	putc(1, output);		/* type 1 record --> ASCII */
	if (verboseflag != 0) printf("ASCII (%lu bytes) - ", length);
	writelength(output, length);
}

void writebinlength(FILE *output, unsigned long length) { /* type 2 field */
	putc(128, output); 		/* truncation of constant value */
	putc(2, output);		/* type 2 record --> Binary */
	if (verboseflag != 0) printf("Binary (%lu bytes) - ", length);
	writelength(output, length);
}

void writehexlength(FILE *output, unsigned long length) {
	if (length % 2 != 0) {
		fprintf(errout, "ERROR: Odd hex section length %lu\n", length);
		length++;
	}
	writebinlength(output, length/2);
}

void writeeof(FILE *output) {  					/* type 3 field */
	putc(128, output); 		/* truncation of constant value */
	putc(3, output);		/* type 3 record --> EOF */
	if (verboseflag != 0) printf("EOF\n");
/*	no length field in this record type header */
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

void fbinary(char *line, FILE *output) {
	char *s=line;
	int c, d;
	for (;;) {
		while ((c = *s++) <= ' ' && c != '\0') ;
		if (c == '\0') return;
		while ((d = *s++) <= ' ' && d != '\0') ;
		if (d == '\0') {
			fprintf(errout, "Uneven line length %s\n", line);
			d = '0';	/* to avoid redundant error message below */
		}
		if (c >= '0' && c <= '9') c = c - '0';
		else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
		else {
			fprintf(errout, "Non hex digit in binary section %s\n", line);
		}
		if (d >= '0' && d <= '9') d = d - '0';
		else if (d >= 'a' && d <= 'f') d = d - 'a' + 10;
		else if (d >= 'A' && d <= 'F') d = d - 'A' + 10;
		else {
			fprintf(errout, "Non hex digit in binary section %s\n", line);
		}		
		putc((c << 4) | d, output);
	}
}

/* `r' and `n' are hidden command line flags, not normally used or needed */

void showusage(char *s) {
	fprintf (errout, "Correct usage is:\n");
/*    fprintf (errout, "%s [-{v}{r}{n}{l}]  <pfa-file-1> <pfa-file-2> ...\n", s); */
    fprintf (errout, "%s [-{v}{n}{m}]  <pfa-file-1> <pfa-file-2> ...\n", s);
	fprintf (errout, "\tv: verbose mode\n");
/*	fprintf (errout, "\tr: do NOT convert `return'+`newline' to `newline' on
input\n"); */
	fprintf (errout, "\tn: do NOT convert `newline' to `return' on output\n");	
	fprintf (errout, "\tm: do NOT convert non-ASCII chars to asterisk\n");	
	fprintf (errout, 
"\t   (output in current directory, same name, extension 'pfb')\n");
	exit(1);
}

/* Modified fgets that terminates on EITHER \n or \r */
/* Combines \n and \r if found in sequence */ /* 1992 Jan 15 */
/* Could now reduce input buffer size (that is MAXLINE) ! */

char *fgetsnew (char *line, int maxline, FILE *input) {
	int c, d;
	char *s=line;

	c = getc(input);
	while (c != '\n' && c != '\r') {
		if (c == EOF) {		/* hit end of file ? */
			*s = '\0';
			return NULL;	/* ??? */
		}
		else *s++ = (char) c;
		if (s - line >= maxline-4) {
			*s++ = '\n';
			*s++ = '\0';
			if (lineallhex(line) > 0) {
				return line;	/* hexadecimal line, so OK 95/Mar/1 */
			}
			else {
				fprintf(errout, "ERROR: Input line too long (%d bytes)\n", 
					s - line);
				errorflag++;
				return NULL;
			}
		}
		c = getc(input);
	}
	if (c == '\r') { /* compress \r followed by \n into just \n */
		if ((d = getc(input)) == '\n') c = d;
		else (void) ungetc(d, input);
	}
	*s++ = (char) c;
	*s = '\0';
	return line;
}

/* return file name minus path when given fully qualified name */

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
	fprintf(errout, "HASHED %ld\n", hash);	
/*	(void) getch();  */
/*	(void) _getch();  */
	return hash;
}

int avoidmac (char *line) {					/* 95/Mar/1 */
	char *s=line;
	int count=0;
	unsigned char c;

/*	if (verboseflag) puts(line); */
	while ((c = *s) != '\0') {
		if (c > 128) {
			if (verboseflag) printf(" NOT ASCII: %u ", c);
			if (c == 169) *s = 'C';			/* copyright */
			else if (c == 170) *s = 'T';	/* trademark */
			else if (c == 168) *s = 'R';	/* registered */
			else if (c == 167) *s = 'B';	/* germandbls / beta */
			else *s = '*';
			count++;
		}
		s++;
	}
	return count;
}

int main(int argc, char *argv[]) {
	FILE *input, *output;
	char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	int linecount, c, m, k, firstarg=1;
	unsigned long asciicount, hexcount;
	long fbegin;
	long fend;			/* not accessed */
	char *flag, *s;
	char *mode;
	
/*	read command line - the crude approach */

	s = argv[firstarg];
/*	if (*s++ == '-') { */
/*	while (*s++ == '-') { */
	while (firstarg < argc && *s++ == '-') {
		while ((c = *s++) != '\0') {
			if (c == 'v') verboseflag = -1;
			else if (c == 't') traceflag = -1;			
/*			else if (c == 'r') flushreturns = 0; */
			else if (c == 'n') converttoreturn = 0;
			else if (c == 'm') avoidmacflag = 0;
		}
		firstarg++;
		s = argv[firstarg];
	}

	if (argc < firstarg + 1) {
		showusage(argv[0]);
		exit(1);
	}

	if (checkcopyright(copyright) != 0) {
		fprintf(errout, "HASH %lu", checkcopyright(copyright));
		exit(1);	/* check for tampering */
	}

	printf("PFAtoPFB font file compressor version %s\n", VERSION);

	for(m = firstarg; m < argc; m++) {	/* do each file in turn */
		strcpy(fn_in, argv[m]);
		extension(fn_in, "pfa");
		if (flushreturns == 0) mode = "rb";
		else mode = "r";
/*		if (flushreturns == 0)  {
			if((input = fopen(fn_in, "rb")) == NULL) {
				perror(fn_in); exit(3);
			}
			if (traceflag != 0) printf("Opened file in binary mode\n"); 
		}
		else {
			if((input = fopen(fn_in, "r")) == NULL) {
				perror(fn_in); exit(3);
			}
			if (traceflag != 0) printf("Opened file in text mode\n"); 
		} */
		input =  fopen(fn_in, mode);
		if (input == NULL) {
			underscore(fn_in);
			input =  fopen(fn_in, mode);
		}
		if (input == NULL) {
			perror(fn_in); exit(3);
		}
		else if (traceflag != 0) {
			if (flushreturns == 0) printf("Opened file in binary mode\n"); 
			else printf("Opened file in text mode\n"); 
		}
/*		if (verboseflag != 0) printf("Compressing %s\n", fn_in); */
		if (verboseflag != 0) printf("Compressing %s ", fn_in);
/*		strcpy(fn_out, path);   */

		if (currentdirect != 0) {
			if ((s=strrchr(fn_in, '\\')) == NULL) { /* flush path */
				if ((s=strrchr(fn_in, ':')) == NULL) s = fn_in;
				else s++;
			}
			else s++;
			strcpy(fn_out, s);  	/* strcat */
		}
		else strcpy(fn_out, fn_in);  /* strcat */
		
		forceexten(fn_out, "pfb");
		if((output = fopen(fn_out, "wb")) == NULL) {
			perror(fn_out); exit(4);
		}
		if (verboseflag != 0) printf("output goes to %s\n", fn_out);
		
		c = getc(input); ungetc(c, input);	/* 92/02/03 */
		if (c == 128) {
			fprintf(errout, "Does not appear to be a valid PFA file\n");
			exit(3);
		}

		for(;;) {					/* actual compression loop */
/*		now entering ASCII section */
			asciicount = 0;
			linecount = 0;
			fbegin = ftell(input); 
/*			(void) fgetpos(input, &fbegin); */
/*			while((flag = fgets(line, MAXLINE, input)) != NULL) { */
			while ((flag = fgetsnew(line, MAXLINE, input)) != NULL) {
/*				if (strchr(line, '\r') == NULL) {
				printf("%s", line);
				for(k = 0; k < strlen(line); k++) {
					printf("%d ", line[k]);
				}
				_getch();
				} */
				if (lineallhex(line) >= 0 && 
					strlen(line) > MINHEXLINE &&
						lineallzeros(line) < 0) break;
				asciicount = asciicount + strlen(line);
				linecount++;
			}

/*			now have came to end of ASCII section */
			fend = ftell(input); /* not used - at end of first hex line */
/*			(void) fgetpos(input, &fend); */

			if (asciicount > 0) {
				writeasciilength(output, asciicount);
				fseek(input, fbegin, SEEK_SET); 
/*				(void) fsetpos(input, &fbegin); */
				for(k = 0; k < linecount; k++) {
/*					(void) fgets(line, MAXLINE, input); */
					(void) fgetsnew(line, MAXLINE, input);
					if (converttoreturn != 0) {
						s = line + strlen(line) - 1;
						if (*s == '\n') *s = '\r';
					}
					if (avoidmacflag) avoidmac(line);	/* 95/Mar/1 */
					fputs(line, output);
				}
			}
			if (flag == NULL) break;		/* hit end-of-file ? */

/*			now entering BINARY section */
			fbegin = ftell(input); 
/*			(void) fgetpos(input, &fbegin); */

/*			c = getc(input); (void) ungetc(c, input); */
/*			printf(" %d ", c); */		/* debugging */

			hexcount = 0;
			linecount = 0;
/*			while((flag = fgets(line, MAXLINE, input)) != NULL) { */
			while ((flag = fgetsnew(line, MAXLINE, input)) != NULL) {
/*				if (linecount == 0) printf("%s", line); */ /* debugging */
/*				if (strchr(line, '\r') == NULL) {
				printf("%s", line);
				for(k = 0; k < strlen(line); k++) {
					printf("%d ", line[k]);
				}
				_getch();
				} */
				if (lineallhex(line) < 0) break;	/* no longer in hex */
				if (lineallzeros(line) >= 0) break;	/* struck all zeros */
				if (lineendszeros(line) > MINZEROLINE) {
					trimlineend(line, input, 0, fn_in);
					hexcount = hexcount + lineallhex(line);
					linecount++;
					break;
				}
				hexcount = hexcount + lineallhex(line);
				linecount++;
			}

/*			came to end of hex section */
			fend = ftell(input);		/* not used */
/*			(void) fgetpos(input, &fend);	*/

			if (hexcount > 0) {
				writehexlength(output, hexcount);
/*				(void) fsetpos(input, &fbegin);  */
				if (flushreturns != 0) {
					fseek(input, fbegin-1, SEEK_SET);	/* TOTAL KLUDGE */
					c = getc(input); 
					if (c > ' ') (void) ungetc(c, input); 
				}
				else fseek(input, fbegin, SEEK_SET); 

				for(k = 0; k < linecount; k++) {
/*					(void) fgets(line, MAXLINE, input); */
					(void) fgetsnew(line, MAXLINE, input);
/*					if (k == 0) printf("%s", line); */  /* debugging */
					if (lineendszeros(line) > MINZEROLINE)
						trimlineend(line, input, 1, fn_in);
					fbinary(line, output);
				}
			}
			if (flag == NULL) break;
		}

		writeeof(output);		/* write special EOF segment */
		if (ferror(input)) {
			perror(fn_in); putc('\a', errout); exit(3);
		}
		else fclose(input);
		if (ferror(output)) {
			perror(fn_out); putc('\a', errout); exit(5);
		}
		else fclose(output);

/* try and modify time/date of output file --- 1992/Oct/10 */
		if (copydate == 0) continue;
		if (getinfo(fn_in, traceflag) < 0) {
			exit(1);
		}
		timebuf.actime = statbuf.st_atime; 
/*		timebuf.modtime = statbuf.st_atime; */
		timebuf.modtime = statbuf.st_mtime; 

		if (_utime(fn_out, &timebuf) != 0) {
			fprintf(errout, "Unable to modify date/time\n");
			perror(fn_out);
			/*			exit(3); */
		}

		/*		see if it worked */
		if (getinfo(fn_out, traceflag) < 0) exit(1);
		if (verboseflag) fflush(stdout);
	}

/*	if (verboseflag != 0 && (argc - firstarg > 1)) */
	if (argc - firstarg > 1)
		printf("Compressed %d files\n", argc - firstarg);
	if (errorflag > 0) {
		fprintf(errout, "ERROR: some bad PFB files produced\n");
		exit(1);
	}
	return 0;
}
	
/* write output into current directory - OK */

/* convert lf/cr => lf on input ? --- NOT NEEDED -- already done */

/* begin ascii section with newline ? NO */

/* deal with odd number of hex characters in a line ? */

/* deal with odd length hex sections ? */

/* still puts extra newline after currentfile eexec ? */

/* new command line flags 'r' and 'n' */

/* 'r' open input in 'r' mode (loses returns after newline) */

/* 'n' replaces newlines at end of ASCII line with return */

/* Some printers like ALW cannot handle non 7-bit chars in ASCII */
