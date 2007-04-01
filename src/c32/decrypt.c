/* Copyright 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997 Y&Y, Inc.
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

/****************************************************************************
* Analyze encrypted strings in attempt to decode them						*
* Input in hex format, output in binary										*
* Command line usage is:	decrypt <in-file> <out-file>					*
****************************************************************************/

/* The following are for function prototypes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
/* #include <errno.h> */

/* #define FNAMELEN 80 */
/* #define MAXLINE 1024 */
#define MAXLINE 2048
#define MAXCHARNAME 128

#define EXTRACHAR 4			/* default junk at beginning */
#define MAXTOKEN 1024

#define MAXOVERFLOW 64

/* #define VERSION "1.3" */
/* #define VERSION "1.3.1" */
#define VERSION "1.4"

#define CRYPT_MUL 52845u	/* pseudo-random number generator constant */
#define CRYPT_ADD 22719u	/* pseudo-random number generator constant */
#define REEXEC 55665 		/* seed constant for eexec encryption */
#define RCHARSTRING 4330 	/* seed constant for charstring encryption */

char line[MAXLINE+1];

char charname[MAXCHARNAME]="UNINITIALIZED";

int lenIV=EXTRACHAR;		/* assumed to be four unless specified */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int verboseflag=0;
int traceflag=0;
int debugflag=0;
int detailflag=0;

int charenflag=0;		/* non-zero means use charencoding, not eexec */
int decodecharflag=0;	/* non-zero means also decode charstring */
int absoluteflag=0;		/* show absolute coordinate commands */
int showextra=0; 		/* non-zero means show random bytes */
int eexecscan=0;		/* scan up to eexec before decrypting */
int charscan=0;			/* scan up to RD before decrypting */
int binaryflag=0;		/* input is binary, not hexadecimal */
int hexadecflag=0;		/* output in hexadecimal, not ASCII/binary */
int convertreturn=0;	/* convert returns in input to newlines */
int newtailflag=1;		/* deal with final token in char code */
int removeleniv=0;		/* default changed 98/Mar/8 ? */
int composflag=1;		/* allow hew string style composite font */
int composseen=0;		/* on when <XXXXXX....XXXXX> <XXXX> CompD seen */

int readhexflag=0;		/* if CharStrings in hex instead of in binary */
int notencrypted=0;		/* if /lenIV 0 def */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int outflag=0;			/* next argument is output file name */
int invalidflag=0;		/* count of invalid codes */
int maxinvalid=8;		/* stop complaining after this many ... */

int overflow;

int deliceflag=0; 		/* non-zero to get rid of meta and control */

char *outputfile="";		/* pointer to output file name */

static char pstoken[MAXTOKEN];		/* place to accumulate tokens */

unsigned short int cryptee; 	/* current seed for charstring encryption */

FILE *errout=stdout;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following not used here */

unsigned char encryptbyte (unsigned char plain, unsigned short *crypter) {
	unsigned char cipher;
/*	cipher = (plain ^ (unsigned char) (*crypter >> 8)); */
	cipher = (unsigned char) ((plain ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return cipher;
}

unsigned char decryptbyte (unsigned char cipher, unsigned short *crypter) {
	unsigned char plain;
/*	plain = (cipher ^ (unsigned char) (*crypter >> 8)); */
	plain = (unsigned char) ((cipher ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return plain;
} 

int getline (FILE *fp_in, char *line) {
	int c, n=0;
	char *s=line;

	c = getc(fp_in);
	while (c != '\n' && c != '\r' && c != EOF) {
		n++;
		if (n >= MAXLINE) {
			*s = '\0';
			fprintf(errout, "ERROR: Input line too long\n");
/*			fprintf(errout, "%s", line);  */
			fputs(line, errout);	/* ha ha */
			exit(13);
		}
		*s++ = (char) c;
		c = getc(fp_in);
	}
	if (c == EOF) n = 0;		/* signal EOF */
	else if (c == '\n') {
		*s++ = (char) c;
		n++;
	}
	else if (c == '\r') {
		if (convertreturn) {
			*s++ = (char) c;
			n++;
			*s++ = '\n';
			n++;
			c = getc(fp_in); 
/*			if (c == EOF) n = 0;  */
			if (c != '\n') {
				(void) ungetc(c, fp_in);
//				printf("UNGETC %d\n", c);	// debugging only
			}
		}
		else {
			*s++ = (char) c;
			n++;
			c = getc(fp_in);
/*			if (c == EOF) n = 0; */
			if (c != '\n') {
				(void) ungetc(c, fp_in);
//				printf("UNGETC %d\n", c);	// debugging only
			}
			else {
				*s++ = (char) c;
				n++;
			}
		}
	}
	*s = '\0';
	return n;
/*	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	else return strlen(line); */
} 

void extension (char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

void forceexten (char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

/* returns -1 if EOF,  and -2 if invalid hex digit and -3 if end of string */

int getnextchar (FILE *fp_in, FILE *fp_out) {
	int c, d, cn, dn, n, m;
/*	printf(" %d ", binaryflag); */
	if (binaryflag == 0) {
//		while ((c = getc(fp_in)) <= ' ' && c != EOF) ;
//		if (c == EOF) return -1;
		while ((c = getc(fp_in)) <= ' ')	// ignore white space
			if (c == EOF) return -1;
		if (c >= '0' && c <= '9') cn = c - '0';
		else if (c >= 'A' && c <= 'F') cn = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f') cn = c - 'a' + 10;		
		else if (c == '>') {
			return -3;			/* end of hex string */
		}
		else {
			fprintf(errout, "ERROR: Not hex %c (%d)\n", c, c);   /* ? */
/*			putc(c, errout); */ 
			(void) ungetc(c, fp_in);
//			printf("UNGETC %d\n", c);	// debugging only
			if (c >= 128) binaryflag = 1;
/*			putc(c, fp_out); */
			return -2;
		}
		assert (cn < 16);
//		while ((d = getc(fp_in)) <= ' ' && d != EOF) ;
//		if (d == EOF) {
//			fprintf(errout, "ERROR: Odd number of characters\n");
//			return -1;
//		}
		while ((d = getc(fp_in)) <= ' ') {
			if (d == EOF) {
				fprintf(errout, "ERROR: Odd number of characters\n");
				return -1;
			}
		}
		if (d >= '0' && d <= '9') dn = d - '0';
		else if (d >= 'A' && d <= 'F') dn = d - 'A' + 10;
		else if (d >= 'a' && d <= 'f') dn = d - 'a' + 10;		
		else if (c == '>') {
			fprintf(errout, "ERROR: Odd number of characters\n");
			return -3;			/* end of hex string */
		}
		else {
/*			fprintf(errout, "Not hex %c (%d)\n", d, d); */
			putc(d, errout);
//			printf("BAD: %c %d\n", d, d);	// debugging only
			putc(c, fp_out);
			(void) ungetc(d, fp_in);
//			printf("UNGETC %d\n", d);	// debugging only
			if (d >= 128) binaryflag = 1;
/*			putc(d, fp_out); */
			return -2;
		}
		assert (dn < 16);
		n = cn << 4 | dn;
		assert (n < 256);
	}
	else { 		/* binary input mode */
		if ((n = getc(fp_in)) == EOF) return -1;
	}
//	if (traceflag) printf("(b: %d) ", n);
/*	m = decryptbyte((char) n, &cryptee); */
	m = decryptbyte((unsigned char) n, &cryptee); /* ??? */
	return m;
}

void showchar (int m, FILE *fp_out) {
	int c, d;
	if (hexadecflag) {
		c = m >> 4; 
/*		d = m & 017; */
		d = m & 15;		
		if (c < 10) putc(c + '0', fp_out);
		else putc(c + 'A' - 10, fp_out);
		if (d < 10) putc(d + '0', fp_out);
		else putc(d + 'A' - 10, fp_out);
	}
	else {
//		if (deliceflag)
		if (deliceflag || fp_out == stdout) {
			if (m > 127) {
				putc('M', fp_out);
				putc('-', fp_out);
				m = m - 128;
			}
			if (m < 32 && m != '\n') {
				putc('C', fp_out);
				putc('-', fp_out);
				m = m + 64;
			}
		}
		putc(m, fp_out);
	}
}

/* returns -1 upon invalid hex --- 0 upon EOF */

int straight (FILE *fp_in, FILE *fp_out) {
	int k=0; /* long */
	int m;
	char *s=pstoken;

	if (charenflag)
		cryptee = RCHARSTRING;  /* encryption seed for charstring coded */
	else cryptee = REEXEC;  /* encryption seed for eexec coded stuff */

	for(;;) {
		m = getnextchar(fp_in, fp_out);
		if (traceflag) printf("n: %d ", m);
		if (m == -1) return -1;			/* EOF */
		if (m == -2) return -2;			/* invalid hex digit */
		if (m == -3) return 0;			/* end of <xxxxxx> */
		
/*		if (k == EXTRACHAR) putc('\n', fp_out);  */
		if (k >= lenIV)	{
			showchar(m, fp_out);
			if (m <= ' ') {
				*s = '\0';					/* terminate token */
				if (strcmp(pstoken, "closefile") == 0) break;
				s = pstoken;				/* start next token */
			}
			else {
				*s++ = (char) m;					/* build up token */
				if (s >= pstoken + MAXTOKEN) s--;
			}
		}
		else {
			if (traceflag) printf("%d ", m);
			if (showextra) showchar(m, stdout); 
			k++;
		}
/*		k++; */
	}

//	if (m < ' ') (void) ungetc(m, fp_in);	// experiment
	
//	m = getc(fp_in);
//	(void) ungetc(m, fp_in);
//	printf("NEXT CHAR %c %d\n", m, m);

	if (verboseflag) {
/*		show control character terminating `closefile' token (in encrypted) */
		if (m < ' ') printf(" %s C-%c\n", "closefile", m+64);
		else printf(" %s %c\n", "closefile", m);
	}

//	m = getc(fp_in);
//	(void) ungetc(m, fp_in);
//	printf("NEXT CHAR %c %d\n", m, m);

/*  swallow next character if white space (reading non-encrypted now) */
	m = getc(fp_in); 
/*	changed to read to end of line to deal with junk 96/Feb/22 */
	while (m >= ' ') {
		putc(m, stdout);		// debugging only
//		printf("%c (%d) ", m, m);	// debugging only
		m = getc(fp_in);
	}
	(void) ungetc(m, fp_in);	/* do we need this ? */
//	printf("UNGETC %d\n", m);	// debugging only
	return 0;
}

/* static unsigned char *normalcode[] = { */

#define NNORMALCODE 32

static char *normalcode[] = {
	"CODE-0", "hstem", "CODE-2", "vstem", "vmoveto", "rlineto", "hlineto",
		"vlineto", "rrcurveto", "closepath", "callsubr", "return",
			"escape", "hsbw", "endchar", "CODE-15", "CODE-16", "CODE-17",
				"CODE-18", "CODE-19", "CODE-20", "rmoveto", "hmoveto",
					"CODE-23", "CODE-24", "strokewidth", "baseline",
						"capheight", "bover", "xheight", "vhcurveto",
							"hvcurveto", "" }; 

/* static unsigned char *escapecode[] = { */
	
#define NESCAPECODE 34

static char *escapecode[] = {
	"dotsection", "vstem3", "hstem3", "ESCAPE-3", "ESCAPE-4",
		"ESCAPE-5", "seac", "sbw", "ESCAPE-8", "ESCAPE-9",
			"ESCAPE-10", "ESCAPE-11", "div", "ESCAPE-13",
				"ESCAPE-14", "ESCAPE-15", "callothersubr", "pop", 
					"ESCAPE-18", "ESCAPE-19", "ESCAPE-20",
						"ESCAPE-21", "ESCAPE-22", "ESCAPE-23",
							"ESCAPE-24", "ESCAPE-25", "ESCAPE-26",  
								"ESCAPE-27", "ESCAPE-28", "ESCAPE-29", 
									"ESCAPE-30", "ESCAPE-31", "ESCAPE-32", 
										"setcurrentpoint", "" };

/* stack probable should perhaps be double, not long - to accomadate "div" */

/* #define MAXNUMBERS 20 */

#define MAXNUMBERS 32

static long numstack[MAXNUMBERS];

int numinx;
int xold, yold;
int sbx, sby;
/* int widthx, widthy; */

void pushstack (long num) {				/* push a number on the stack */
	numstack[numinx++] = num;
	if (numinx >= MAXNUMBERS) {
		fprintf(errout, "ERROR: Overflowed number stack\n"); 
		if (overflow++ > MAXOVERFLOW) {
			fprintf(errout, "EXIT: too many errors\n"); 
			exit(1);
		}
		numinx = MAXNUMBERS-1;
	}
}

void dumpstack (FILE *fp_out, char *name) { /* dump numbers in stack */
	int k;
	for (k = 0; k < numinx; k++) {
		fprintf(fp_out, "%ld ", numstack[k]);
	}
	numinx = 0;
/*	fprintf(fp_out, "%s", name); */
	fputs(name, fp_out);				/* 1992/Dec/5 */
/*	if (strcmp(name, "div") == 0) fprintf(fp_out, " "); */
	if (strcmp(name, "div") == 0) putc(' ', fp_out);
/*	else fprintf(fp_out, "\n"); */
	else putc('\n', fp_out);
}

void dumprelative (FILE *fp_out, char *name) {
	assert(numinx == 2);
	xold = xold + (int) numstack[0];
	yold = yold + (int) numstack[1];
	fprintf(fp_out, "%d %d %s\n", xold, yold, name);
	numinx = 0;
}

void dumprcurveto (FILE *fp_out, char *name) {
	int k;
	assert(numinx == 6);

	for (k=0; k < 3; k++) {
		xold = xold + (int) numstack[2*k];
		yold = yold + (int) numstack[2*k+1];
		fprintf(fp_out, "%d %d ", xold, yold);
	}
/*	fprintf(fp_out, "%s\n", name); */
	fputs(name, fp_out);
	putc('\n', fp_out);
	numinx = 0;
}

void dumphstack (FILE *fp_out, char *name) {
	int k;
	for (k = 0; k < numinx/2; k++) 
		numstack[2*k] = numstack[2*k] + (long) sby;
	dumpstack(fp_out, name);
}

void dumpvstack (FILE *fp_out, char *name) {
	int k;
	for (k = 0; k < numinx/2; k++) 
		numstack[2*k] = numstack[2*k] + (long) sbx;
	dumpstack(fp_out, name);
}

void cleanoutstack (FILE *fp_out, char *name) {
/*	long ratio; */
	if (absoluteflag == 0) 	dumpstack(fp_out, name);
	else {
		if (strcmp(name,"div") == 0) {
/*			ratio = numstack[2]/numstack[1]; */
			dumpstack(fp_out, name);
/*			numinx = numinx - 2; */
/*			pushstack(ratio);	 */
		}
		else if (strcmp(name, "hsbw") == 0) {
			sbx = (int) numstack[0]; sby = 0; 
/*			widthx = (int) numstack[1]; widthy = 0; */
			xold = sbx; yold = 0;
			dumpstack(fp_out, name);
		}
		else if (strcmp(name, "sbw") == 0) {
			sbx = (int) numstack[0]; sby = (int) numstack[1]; 
/*			widthx = numstack[2]; widthy = numstack[3]; */
			xold = sbx; yold = sby;
			dumpstack(fp_out, name);
		}
		else if (strcmp(name, "rmoveto") == 0) dumprelative(fp_out, "moveto");
		else if (strcmp(name, "hmoveto") == 0) {
			numstack[1] = 0; numinx++;
			dumprelative(fp_out, "moveto");
		}
		else if (strcmp(name, "vmoveto") == 0) {
			numstack[1] = numstack[0]; numinx++; numstack[0]=0;
			dumprelative(fp_out, "moveto");
		}
		else if (strcmp(name, "rlineto") == 0) dumprelative(fp_out, "lineto");
		else if (strcmp(name, "hlineto") == 0) {
			numstack[1] = 0; numinx++;
			dumprelative(fp_out, "lineto");
		}
		else if (strcmp(name, "vlineto") == 0) {
			numstack[1] = numstack[0]; numinx++; numstack[0]=0;
			dumprelative(fp_out, "lineto");
		}
		else if (strcmp(name, "rrcurveto") == 0) dumprcurveto(fp_out, "curveto");
		else if (strcmp(name, "vhcurveto") == 0) {
			numstack[5] = 0;
			numstack[4] = numstack[3];
			numstack[3] = numstack[2];
			numstack[2] = numstack[1];
			numstack[1] = numstack[0];
			numstack[0] = 0;
			numinx = numinx + 2;
			dumprcurveto(fp_out, "curveto");
		}
		else if (strcmp(name, "hvcurveto") == 0) {
			numstack[5] = numstack[3];
			numstack[4] = 0;
			numstack[3] = numstack[2];
			numstack[2] = numstack[1];
			numstack[1] = 0; 
			numinx = numinx + 2;
			dumprcurveto(fp_out, "curveto");
		}
		else if (strstr(name, "hstem") != NULL) dumphstack(fp_out, name);
		else if (strstr(name, "vstem") != NULL) dumpvstack(fp_out, name);
		else dumpstack(fp_out, name);
	}
}

int chardecode (FILE *fp_in, FILE *fp_out, int maxb) {
	int k=0, i=0, j, n, m;
	long ln;
	long position;
	char *name="";			/* used before defined ??? */

/*	printf("%d  ", maxb); */
	numinx=0;
/*	fprintf(fp_out, "\n"); */
	cryptee = RCHARSTRING;  /* encryption seed for charstring coded */

	sbx = 0; sby = 0; 		/* in case it is a subr and has no hsbw */
	xold = sbx; yold = 0;

	if (traceflag) printf("Entering chardecode (maxb %d binaryflag %d)\n",
						  maxb, binaryflag);
	
	for(;;) {
		if (i >= maxb) return -1;	/* enough bytes for this string */
		m = getnextchar(fp_in, fp_out); i++;
		if (traceflag) printf("m: %d ", m);
		if (debugflag && k < 4) printf("%d ", m);		/* debugging */
		if (m == -1) break;			/* EOF */
		if (m == -2) return -1;		/* invalid hex digit */
		if (m == -3) return -1;		/* end of <xxxxxx> */
		assert(m >= 0); assert (m <= 255);
/*		if (k == EXTRACHAR && showextra) putc('\n', stdout);  */
		if (k < lenIV) {			/* ignore leading junk */
			if (traceflag) printf("%d ", m);
			if (showextra) showchar(m, stdout); /* fp_out */
		}
		else {
			if (m == 12) {					/* escape code */
				n = getnextchar(fp_in, fp_out); i++;
				if (traceflag) printf("n: %d ", n);
				if (n == -1) break;			/* EOF */
				if (n == -2) return -1;		/* invalid hex digit */
				if (n == -3) return -1;		/* end of <xxxxxx> */
/*				if (n < 0 || n > 33)   */
				if (n < 0 || n >= NESCAPECODE) {
					fprintf(errout, 
						"ERROR: Code following escape (12) is bad (%d) ", n);
					position = ftell(fp_in);
					fprintf(errout, "at byte %ld ", position);
					fprintf(errout,	"in %s\n", charname);
					invalidflag++;
					cleanoutstack(fp_out, "ESCAPE");
/*					exit(7); */
					name = "";
					fflush(stdout);
				}
				else name = escapecode[n];
/*				if (strncmp(name, "ESCAPE", 4) == 0) { */
				if (strncmp(name, "ESCAPE", 6) == 0) {
					if (invalidflag++ < maxinvalid) {
						fprintf(errout, "ERROR: Invalid escape code (%d) ", n);
						position = ftell(fp_in);
						fprintf(errout, "at byte %ld ", position);
						fprintf(errout,	"in %s\n", charname);
					}
				}
				cleanoutstack(fp_out, name);
/*				fprintf(fp_out, "%s", name); */
/*				if (n != 12) fprintf(fp_out, "\n");
				else fprintf(fp_out, " "); */
			}
/*			else if (m <= 31) {  */
			else if (m < NNORMALCODE) { 
				name = normalcode[m];
				if (strncmp(name, "CODE", 4) == 0) {
					if (invalidflag++ < maxinvalid) {
						fprintf(errout, "ERROR: Invalid code (%d) ", m);
						position = ftell(fp_in);
						fprintf(errout, "at byte %ld ", position);
						fprintf(errout, "in %s\n", charname);
					}
				}
				cleanoutstack(fp_out, name);
/*				fprintf(fp_out, "%s\n", name); */
			}
/*			else if (m >= 32 && m <= 246) */
			else if (m >= NNORMALCODE && m <= 246) 
				pushstack((long) (m - 139));
/*				fprintf(fp_out, "%d ", m - 139); */
			else if (m >= 247 && m <= 250) {
				n = getnextchar(fp_in, fp_out); i++;
				if (traceflag) printf("n: %d ", n);
				if (n == -1) break;			/* EOF */
				if (n == -2) return -1;		/* invalid hex digit */
				if (n == -3) return -1;		/* end of <xxxxxx> */
				assert (n >= 0);
				pushstack((long) (((m - 247) << 8) + n + 108)); 
/*				fprintf(fp_out, "%d ", ((m - 247) << 8) + n + 108); */
			}
			else if (m >= 251 && m <= 254)  {
				n = getnextchar(fp_in, fp_out); i++;
				if (traceflag) printf("n: %d ", n);
				if (n == -1) break;			/* EOF */
				if (n == -2) return -1;		/* invalid hex digit */
				if (n == -3) return -1;		/* end of <xxxxxx> */
				assert (n >= 0);
				pushstack((long) -(((m - 251) << 8) + n + 108)); 
/*				fprintf(fp_out, "%d ", -(((m - 251) << 8) + n + 108));  */
			}
			else {			/* m = 255 */
/*				printf ("m = %d ", m); */
				assert (m == 255);
				ln = 0L;
				for (j=0; j < 4; j++) {
					n = getnextchar(fp_in, fp_out); i++;
					if (traceflag) printf("n: %d ", n);
					if (n == -1) return 0;		/* EOF */
					if (n == -2) return -1;		/* invalid hex digit */
					if (n == -3) return -1;		/* end of <xxxxxx> */
					assert (n >= 0);
					assert (n <= 255);
					ln = (ln << 8) | (long) n;
				}
				pushstack(ln);
/*				fprintf(fp_out, "%ld ", ln); */
			}
		}
		k++;
	}
	if (i != maxb) 	putc('\n', fp_out);
	return 0;
}

/* Following is for test purposes only:

int numdecode(void) { 
	unsigned int num, nxt;

  num = getbyte();
  if (num >= 32 && num <= 246) return num - 139;
  else if (num >= 247 && num <= 250) {
	nxt = getbyte();
	return ((num - 247) << 8) + nxt + 108;
  }
  else if (num >= 251 && num <= 254) {
	nxt = getbyte();
	return (-(num - 251) << 8) - nxt - 108;
  }
  else {
	nxt = getbyte(); nxt = getbyte();
	num = getbyte(); nxt = getbyte();
	return (num << 8) | nxt;
  }
} */
	 
/* NOTE:  a => d => c  and  b => e */

void decodeflag (int c) {
	switch(c) { 
		case 'v': if (verboseflag++ > 0) traceflag = 1; break;
		case 't': traceflag = 1; break;
		case '?': detailflag = 1; break;
		case 'a': absoluteflag = 1; charscan = 1;
		case 'd': decodecharflag = 1;			/* implies charenflag */
		case 'c': charenflag = 1; break;
		case 's': showextra = 1; break;
		case 'h': hexadecflag = 1; break;
		case 'r': charscan = 1;					/* also implies binary */
				  decodecharflag = 1;	
				  charenflag = 1;	
/*		case 'b': binaryflag = 1; break; */
		case 'b': binaryflag = !binaryflag; break;
		case 'o': outflag = 1; break;
		case 'n': convertreturn = 1; break;
		case 'e': eexecscan = 1; break;		/* implies not binary */
		case 'f': newtailflag = 0; break;	/* 1995/Jan/30 */
		case 'l': removeleniv = !removeleniv;	/* 98/Mar/8 */
		default: {
				fprintf(errout, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
}

/* \tb: input is binary, not hexadecimal\n\ */
/* \th: produce output in hexadecimal, not ASCII/binary\n\ */

void showusage(char *s) {
	fprintf (errout, "\
Correct usage is:\n\
%s [-{v}{s}{e}{r}{c}{d}{a}{n}] <in-file> \n", s); /*  [-o <out-file>] */
	fprintf (errout, "\
\tv: verbose mode\n\
\ts: show random starting bytes\n\
\te: scan up to eexec before decrypting\n\
\tr: scan up to successive RDs to decrypt CharStrings (=> c, d)\n\
\t\tc: use CharString decoding instead of eexec\n\
\t\td: decode CharString in addition to decrypting\n\
\tb: binary instead of hex input or vice versa\n\
\ta: decode CharString and show absolute commands (=> c, d)\n\
\tn: convert <return> to <return> + <newline> (Mac => PC)\n\
\t   output file current directory, same name, extension 'dec' or 'pln'\n\
");
	exit(1);
}

/* \to: next argument is output file\n\ */

int scanuptoeexec (FILE *fp_in, FILE *fp_out) { /* return zero upon EOF */
	int n;
	char *s;
	static char line[MAXLINE];
	long current;

	if (traceflag) printf("Scanning up to eexec\n");
/*	while ((n = getline(fp_in, line)) && (strstr(line, "eexec") == NULL)*/
/*	deal with `currentfile eexec}bind' in Windows 95 PSCRIPT header */
	current = ftell(fp_in);
	while ((n = getline(fp_in, line)) != 0 &&
		   (strstr(line, "eexec") == NULL || strstr(line, "bind") != NULL)) {
/*		deal with Type1Hdr implied eexec in Windows 95 PSCRIPT output */
		if ((s = strstr(line, "Type1Hdr")) != NULL && *(s-1) == ' ') break;
		fputs(line, fp_out);
		if (traceflag) {
/*			printf("%s", line);  */
			fputs(line, stdout); 
			if (strchr(line, '\n') == NULL) putc('\n', stdout);
		}
		current = ftell(fp_in);
	}

	if (n != 0) {
		if ((s = strstr(line, "eexec")) != NULL) {
			while (*s != '\0' && *s != '\n' && *s != ' ' && *s != '\t') s++;
			if (*s == ' ' || *s == '\t') {
				*s++ = '\n';
				*s = '\0';
				fseek(fp_in, current + (s-line), SEEK_SET);
			}
		}
		fputs(line, fp_out);		/* output the "currentfile eexec" line */
		if (traceflag)	{
/*			printf("%s", line); */
			fputs(line, stdout); 
			if (strchr(line, '\n') == NULL) putc('\n', stdout);
		}
	}
	return n;
}

/* Different forms to take care of: */
/* /charname <m> RD ...... ND */  /* /charname <m> -| ...... | */
/* Possibly spaces before /charname (Digital Typeface Fonts) ? */
/* dup <n> <m> RD ...... ND */    /* dup <n> <m> -| ...... | */ /* funky */
/* also <XXXXXX...XXX> <XXXX> CompD (second arg is code in hex) */

int scanuptoRD (FILE *fp_in, FILE *fp_out) {		/* return zero upon EOF */
	int c, k, m;
/*	static char line[MAXLINE+1]; */
/*	static char charname[MAXLINE]; */
	char *s, *t;
	int n = 0;

	composseen = 0;
	readhexflag = 0;
	notencrypted = 0;
	if (traceflag) printf("Scanning up to RD\n");
	s = line;
/*	accumulate a `token' in line, n character count */
	while ((c=getc(fp_in)) != '\n' && c != EOF) {
/*		putc(c, fp_out);  */
		if (c == '\r') {			/* see if \r\n pair */
			c = getc(fp_in); 
			if (c == '\n')  {		/* it is, copy it */
				*s++ = (char) '\r';
				n++;
			}
			else {					/* it is not - undo getc */
				(void) ungetc(c, fp_in);
//				printf("UNGETC %d\n", c);	// debugging only
				c = '\n';			/* isolated \r turned into \n */
			}
			break;					/* end of line */
		} 
		if (composflag && c == '<' && n == 0) {	/* composite font format */
			composseen = 1;
			if (traceflag) printf("Found <...> string\n");
			return 32000;	/* inifinite */
		}
		if (c == ' ' && n > 3) {	/* space way after DUP or whatever */
			*s = '\0';
			if (debugflag) printf("LINE: %s\n", line);
#ifdef IGNORED
/*			new code for lenIV */
			if (traceflag) printf("LINE: %s\n", line);
			if (strncmp(line, "/lenIV", 6) == 0) {	/* 97/July/20 */
				if (sscanf(line, "/lenIV %d", &lenIV) == 1) {
					if (lenIV == 4) printf("Encrypted: %s", line);
					else if (lenIV == 0) {
						notencrypted = 1;
						printf("Unencrypted: %s", line);
					}
					else printf("/lenIV %d def ???\n", lenIV);
					if (removeleniv) {	/* comment out ? */
						memmove(line+2, line, strlen(line));  /* +1 ? */
						*line = '%';
						*(line+1) = ' ';
						s += 2; n += 2;
/*						printf("LINE: %s", line); */
					}
				}
/*				printf("%s ???\n", line); */
			}
/*			end of new code */
#endif
#ifdef IGNORED
			if (strncmp(s-3, "/RD", 3) == 0 ||
				strncmp(s-3, "/-|", 3) == 0 ||
				strncmp(s-3, "/-!", 3) == 0) {
				if (strstr(line, "readhexstring") != NULL) {
					readhexflag = 1;
					printf("CharStrings in hex format instead of binary: %s\n", line);
					binaryflag = 0;
				}
			}
#endif
			if (strncmp(s-3, " RD", 3) == 0 ||
				strncmp(s-3, " -|", 3) == 0 ||
				strncmp(s-3, " -!", 3) == 0) {	/* 97/Jul/20 */
				*s++ = (char) c;
				n++;	/* ??? */
				*s++ = '\0';
				n++;	/* ??? */
/* special case code for output from Windows 95 PSCRIPT driver */
				if (strncmp(line, "/MSTT", 5) == 0) {
					t = s;				/* search back to /Gnm */
					while (t > line && *t != '/') t--;
					if (*(t-1) == ' ') *(t-1) = '\0';
/*					if (traceflag) printf("OUT:  %s\n", line); */
					fputs(line, fp_out);	/* flush start to output */
					strcpy(line, t);		/* bring CharString to head */
					n = strlen(line);		/* ??? */
				}
/* end of special case code */

/*				t = line + strspn(line, " \t"); */ /* skip over whitespace */
				t = line;
/*				while (*t == ' ' || *t == '\t') t++; */
/*				skip over space, tab, and null 98/Apr/20 */
				while (*t == ' ' || *t == '\t' || *t == '\0')  /* ??? */
					t++;
/*				printf("s-line %d t-line %d\n", s-line, t-line); */
				if (sscanf(t, "/%s %d ", charname, &m) < 2) {
/* well, maybe it is in that funky format we used to use ? */
					if (sscanf(t, "%s %d %d ", charname, &k, &m) < 3) {
/* sigh, we have a bad CharString */
						fprintf(errout, "WARNING: Missing char name: %s\n", line);
/* maybe just the name is missing ? */ /* Corel Draw bug ! */
/*						if (sscanf(line, "/ %d ", &m) != 1) { */
						if (sscanf(t, "/ %d ", &m) < 1) {
/* sigh, we REALLY have a bad CharString */
							fprintf(errout, "ERROR: Don't understand: %s\n", line);
							exit(3);
						}
					}
				}
/*				fprintf(fp_out, "\n%s\n", line); */
/*				fprintf(fp_out, "\n%s\n", t); */
				putc('\n', fp_out);
/*				if (traceflag && strlen(t) > 256) printf("What? %s\n", t); */
/*				if (traceflag) printf("OUT:  t); */
				fputs(t, fp_out);
				putc('\n', fp_out);
/*				printf("%d ", m);*/
				if (m < 0 || m > 20000) {
					fprintf(errout, "Bad encrypted string length %d\n", m);
					exit(1);
				}
				if (traceflag) printf("%s %d\n", charname, m);
				return m;
			} /* end of if (*s(-3) ... ) RD -| etc. */
		} /* end of if (c == ' ' && n > 3) */
		*s++ = (char) c;
		n++;
		if (n >= MAXLINE) {
				*s++ = '\0';
/*				fprintf(fp_out, "%s ", line); */
				fputs(line, fp_out);
				putc(' ', fp_out);
				fprintf(errout, "LINE TOO LONG: %s\n", line);
				exit(3);
		}
	} /* end of while ((c = fgetc... )) */

	*s++ = (char) c;
	n++;	/* ??? */
	*s++ = (char) '\0';
	n++;	/* ??? */
	if (c == EOF) {
		if (traceflag) printf("EOF\n");
		return -1;
	}
	else {
		if (traceflag) printf("LINE: %s\n", line);
		if ((s = strstr(line, "/lenIV")) != NULL) {
			if (sscanf(s, "/lenIV %d", &lenIV) == 1) {
				if (lenIV == 4) printf("Encrypted: %s", line);
				else if (lenIV == 0) {
					notencrypted = 1;
					printf("Unencrypted: %s", line);
				}
				else printf("/lenIV %d def ???\n", lenIV);
				if (removeleniv) {	/* comment out ? */
					memmove(line+2, line, strlen(line));  /* +1 ? */
					*line = '%';
					*(line+1) = ' ';
					s += 2; n += 2;
				}
			}
		}
		if (strstr(line, "/RD") != NULL ||
			strstr(line, "/-|") != NULL ||
			strstr(line, "/-!") != NULL) {
			if (strstr(line, "readhexstring") != NULL) {
				readhexflag = 1;
				printf("hex format instead of binary: %s\n", line);
				binaryflag = 0;
			}
		}

		if (traceflag) printf("OUT:  %s", line);
		fputs(line, fp_out);
/*		putc(c, fp_out); */ /* nl */
		if (traceflag) fputs(line, stdout);
		return 0;
	}
}

/* deal with NP, ND, |- or whatever at end of Subr or CharString */
/* basically method for skipping a token */
/* return value not currently used */

int readtailchar (FILE *fp_in, FILE *fp_out) {
	char *s;
	char line[MAXLINE];
	int c;

	if (traceflag) printf("Entering readtailchar\n");
	if (traceflag) fflush(stdout);
	s = line;
	while ((c = getc(fp_in)) != '\n' && c != EOF) {	/* skip white space */
/*		putc(c, fp_out);   */	/* ignore white space here if any */
		if (c > ' ') break;
	}
	if (c == EOF) return -1;
	(void) ungetc(c, fp_in); 
//	printf("UNGETC %d\n", c);	// debugging only
	while ((c = getc(fp_in)) != '\n' && c != EOF) {	/* accumulate token */
		if (c <= ' ') break;
		putc(c, fp_out);
		*s++ = (char) c;
	}
/*	if (c != '\n') */		 /* create separation whether its newline or not */
	putc ('\n', fp_out); /* create separation */

	if (c == EOF) return -1;
	*s = '\0';
	if (strcmp(line, "ND") == 0) return 0;
	if (strcmp(line, "NP") == 0) return 0;
	if (strcmp(line, "|-") == 0) return 0;
	if (strcmp(line, "|") == 0) return 0;
/*	if (c == EOF) return -1; */			/* can't happen */
//	added code to deal with noaccess put in very old fonts 2000 June 7
	if (strcmp(line, "noaccess") == 0) {
		s = line;
		while ((c = getc(fp_in)) != '\n' && c != EOF) {	/* skip white space */
/*		putc(c, fp_out);   */	/* ignore white space here if any */
			if (c > ' ') break;
		}
		if (c == EOF) return -1;
		(void) ungetc(c, fp_in); 
//	printf("UNGETC %d\n", c);	// debugging only
		while ((c = getc(fp_in)) != '\n' && c != EOF) {	/* accumulate token */
			if (c <= ' ') break;
			putc(c, fp_out);
			*s++ = (char) c;
		}
		*s = '\0';
		if (strcmp(line, "put") == 0) return 0;
		if (strcmp(line, "def") == 0) return 0;
	}
	fprintf(errout, "NON-STANDARD ENDING: `%s' (expecting ND NP |- or |) ", line);
	return 0;			/* ??? */
}

int readhexcode (FILE *fp_out, FILE *fp_in) {
	int c, k=0;
	char *s=line;

	while ((c = getc(fp_in)) != '<' && c != EOF) ;
/*		putc(c, fp_out); */
	if (c == EOF) return -1;
	putc(c, fp_out);
	while ((c = getc(fp_in)) != '>' && c != EOF && k++ < 4) {
		*s++ = (char) c;
		putc(c, fp_out);
	}
	*s++ = '\0';
	putc(c, fp_out);
	if (traceflag) printf("Char code %s\n", line);
	return 0;
}

void decrypt (FILE *fp_out, FILE *fp_in) {
	int c, n, ret;

	if (decodecharflag == 0) {		/* if not decoding chardefs */
		if (eexecscan) (void) scanuptoeexec(fp_in, fp_out);
		while (straight(fp_in, fp_out) == 0) {
			if (scanuptoeexec(fp_in, fp_out) == 0) break;
		}
	}
	else if (charscan) {   /* if decoding chardefs */
		while ((n = scanuptoRD(fp_in, fp_out)) >= 0) {
			if (n > 0) {
				ret = chardecode(fp_in, fp_out, n);
				if (traceflag) printf("Return %d from chardecode\n", ret);
				if (ret >= 0) break;
				if (traceflag) fflush(stdout);
				if (composseen) readhexcode(fp_out, fp_in);
				else if (newtailflag) readtailchar(fp_in, fp_out);
				else {						/* above new 95/Jan/30 */
					c = getc(fp_in);
					if (c == EOF) {
						fprintf(errout, "Unexpected EOF\n");
						exit(3);
					}
					if (c != ' ') putc(c, fp_out);
				}
			}
		} /* end of while */
	}		
	else {
		while (chardecode(fp_in, fp_out, -1) < 0) {
			c = getc(fp_in); 
			if (c == EOF) {
				fprintf(errout, "Unexpected EOF\n");
				exit(3);
			}
			putc(c, fp_out);
			if (c == '<') putc('\n', fp_out);
		} /* end of while */
	}
}

int main(int argc, char *argv[]) {      /* main program */
/* command line usage is: decrypt <in-file> <out-file> ... */
    FILE *fp_in=NULL;
	FILE *fp_out=NULL;					/* *fp_cod */
    char fn_in[FILENAME_MAX], fn_out[FILENAME_MAX];
	unsigned int i;
	int c, m, firstarg=1;
/*	int n; */
	char *s;

/*	strcpy(fn_out, ""); */

	if (argc < 2) showusage(argv[0]);

/*	while (argv[firstarg][0] == '-') */ /* check for command flags */
	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else decodeflag(c);
		}
		firstarg++;
		if(outflag) {
			outputfile = argv[firstarg]; outflag = 0; firstarg++;
		}
	}

	if (argc <= firstarg) showusage(argv[0]);
		
	printf( "Font decryption program version %s\n",	VERSION);

	for (m = firstarg; m < argc; m++) {
		
		invalidflag=0;
		overflow = 0;
		strcpy(fn_in, argv[m]);			/* get file name */
/*		extension(fn_in, "pfa");	*/	/* extension if needed */

		if (verboseflag) printf("Decrypting file %s\n", fn_in);

		if (strcmp(outputfile, "") == 0) {
			if ((s=strrchr(fn_in, '\\')) != NULL) s++;
			else if ((s=strrchr(fn_in, ':')) != NULL) s++;
			else s = fn_in;
			strcpy(fn_out, s);			/* copy input file name minus path */
			deliceflag = 0; 			/* raw bytes out */
		}
		else strcpy(fn_out, outputfile);

		if (strstr(fn_in, ".dec") != NULL ||
			strstr(fn_in, ".DEC") != NULL)
			forceexten(fn_out, "pln");	/* avoid file name collision */
		else forceexten(fn_out, "dec");		
		
/*		extension(fn_out, "dec"); */
	
		if (verboseflag) printf("Output is going to %s\n", fn_out);

		if (fn_in[0] != '\0') {
			extension(fn_in, "pfa");		/* extension if needed */
			if (binaryflag == 0) {
/*				if ((fp_in = fopen(fn_in, "r")) == NULL) { */
				if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
					forceexten(fn_in, "ps");
/*					if ((fp_in = fopen(fn_in, "r")) == NULL) { */
					if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
						printf("Attempt to open for input %s\n", fn_in);
						perror(fn_in);
						exit(2);
					}
				}
			}
			else {
				if ((fp_in = fopen(fn_in, "rb")) == NULL) {
					forceexten(fn_in, "ps");
					if ((fp_in = fopen(fn_in, "rb")) == NULL) { 
						printf("Attempt to open for input %s\n", fn_in);
						perror(fn_in);
						exit(2);
					}
				}
			}
		}
		else fp_in = NULL;			/* ??? */
		if (fn_out[0] != '\0') {				/* don't corrupt nl */
			if ((fp_out = fopen(fn_out, "wb")) == NULL) { 
				printf("Attempt to open for output %s\n", fn_out);
				perror(fn_out);
				exit(2);
			}
		}
		else fp_out = NULL;			/* ??? */
		
/*		setbuf(fp_out, NULL); 	setbuf(stdout, NULL); */

		if (traceflag) printf("Decryption attempt starting\n"); 
		
		decrypt(fp_out, fp_in);	/* actually go do the work */
		
		fclose(fp_in);
		if (ferror(fp_out) != 0) {
			fprintf(errout, "Output error");
			printf("ferror in %s\n", fn_out);
			perror(fn_out);
			exit(2);
		}
		else fclose(fp_out);
		if (showextra) printf("\n");
	}
	if (argc > firstarg + 1) 
		printf("Processed %d files\n", argc - firstarg);
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

/* most of this file is junk that can now be flushed ! OK */
/* provide for binary input OK FOO - */
/* provide for hexadecimal output OK FOO - */

/* deal properly with "div" ==> double, not long ? */

/* Way of doing the full enchilada double decrypt and decode: */ 

/* decompre decrypt decrypt */

/* check on lenIV ? presently assumes extrachar = 4 */

/* about losing last line (cleartomark) in decrypting CharStrings ? */

/* stop decrypting when long string of zeros is found ? */
/* for this, key on closefile in decrypted section ? OK */

/* make "ps" second option as extensio for input (after "pfa" */

/* in scan to eexec mode, can read past EOF and just keep going ... */

/* why is last character in file sometimes repeated ? */

/* stops decrypting when it sees "closefile" in decrypted output */

/* in addition to scanning up to RD's also scan up to `readstring' */
/* and just copy across binary stuff ? */

/* may break with files containing returns instead of line feeds ? */
