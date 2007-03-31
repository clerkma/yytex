/* decrypt.c
   Copyright Y&Y 1990, 1991, 1992
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

/****************************************************************************/

/* This is the MODULE - not the full program */

/****************************************************************************
*                                                                         	*
* Analyze encrypted strings in attempt to decode them						*
*                                                                         	*
* Input in hex format, output in binary										*
*                                                                         	*
* Command line usage is:	decrypt <in-file> <out-file>					*
*																			*
****************************************************************************/

/* The following are for function prototypes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
/* #include <errno.h> */

#include "metrics.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* char *compilefile = __FILE__; */
/* char *compiledate = __DATE__; */
/* char *compiletime = __TIME__; */

extern int verboseflag;
extern int traceflag;		/* not used yet */
extern int detailflag;		/* not used */

extern int charenflag;		/* non-zero means use charencoding, not eexec */
extern int eexecscan;		/* scan up to eexec before decrypting */
extern int charscan;		/* scan up to RD before decrypting */
extern int decodecharflag;	/* non-zero means also decode charstring */
extern int binaryflag;		/* input is binary, not hexadecimal */

int absoluteflag=0;		/* show absolute coordinate commands */
int showextra=0; 		/* non-zero means show random bytes */
int hexadecflag=0;		/* output in hexadecimal, not ASCII/binary */
int convertreturn=0;	/* convert returns in input to newlines */

int removelenIV=1;	
int lenIV;				/* EXTRACHAR */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* int outflag=0; */			/* next argument is output file name */

int invalidflag=0;		/* count of invalid codes */
int maxinvalid=8;		/* stop complaining after this many ... */

int deliceflag=0; 		/* non-zero to get rid of meta and control */

static char pstoken[MAXTOKEN];		/* place to accumulate tokens */

unsigned short int cryptee; 	/* current seed for charstring encryption */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* following not used here */

#ifdef IGNORED
static unsigned char encryptbyte (unsigned char plain, unsigned short *crypter) {
	unsigned char cipher;
/*	cipher = (plain ^ (unsigned char) (*crypter >> 8)); */
	cipher = (unsigned char) ((plain ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return cipher;
}
#endif

static unsigned char decryptbyte (unsigned char cipher, unsigned short *crypter) {
	unsigned char plain;
/*	plain = (cipher ^ (unsigned char) (*crypter >> 8)); */
	plain = (unsigned char) ((cipher ^ (unsigned char) (*crypter >> 8)));
/*	*crypter = (cipher + *crypter) * CRYPT_MUL + CRYPT_ADD; */
	*crypter = (unsigned short) ((cipher + *crypter) * CRYPT_MUL + CRYPT_ADD);
	return plain;
} 

static int getline(FILE *fp_in, char *line) {
	int c, n=0;
	char *s=line;
	c = getc(fp_in);
	while (c != '\n' && c != '\r' && c != EOF) {
		n++;
		if (n >= MAXLINE) {
			fprintf(stderr, "ERROR: Input line too long\n");
/*			fprintf(stderr, "%s", line); */
			fputs(line, stderr);	/* ha ha */
			exit(13);
		}
		*s++ = (char) c;
		c = getc(fp_in);
	}
	if (c == EOF) n = 0;		/* signal EOF */
	else if (c == '\n') {
		*s++ = (char) c; n++;
	}
	else if (c == '\r') {
		if (convertreturn != 0) {
			*s++ = (char) c; n++;
			*s++ = '\n'; n++;
			c = getc(fp_in); 
/*			if (c == EOF) n = 0;  */
			if (c != '\n') ungetc(c, fp_in);
		}
		else {
			*s++ = (char) c; n++;
			c = getc(fp_in);
/*			if (c == EOF) n = 0; */
			if (c != '\n') ungetc(c, fp_in);
			else {
				*s++ = (char) c; n++;
			}
		}
	}
	*s = '\0';
	return n;
/*	if (fgets(line, MAXLINE, fp_in) == NULL) return 0;
	else return strlen(line); */
} 

/* void extension(char *fname, char *ext)  {
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
} */

/* void forceexten(char *fname, char *ext) {
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);	
} */

/* returns -1 if EOF,  and -2 if invalid hex digit */

int getnextchar(FILE *fp_in, FILE *fp_out) {
	int c, d, cn, dn, n, m;
/*	printf(" %d ", binaryflag); */
	if (binaryflag == 0) {
		while ((c = getc(fp_in)) <= ' ' && c != EOF) ;
		if (c == EOF) return -1;
		if (c >= '0' && c <= '9') cn = c - '0';
		else if (c >= 'A' && c <= 'F') cn = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f') cn = c - 'a' + 10;		
		else {
			fprintf(stderr, "ERROR: Not hex %c (%d)\n", c, c);   /* ? */
/*			putc(c, stderr); */ 
			ungetc(c, fp_in);
			if (c >= 128) binaryflag = 1;
/*			putc(c, fp_out); */
			return -2;
		}
		assert (cn < 16);
		while ((d = getc(fp_in)) <= ' ' && d != EOF) ;
		if (d == EOF) {
			fprintf(stderr, "ERROR: Odd number of characters\n");
			return -1;
		}
		if (d >= '0' && d <= '9') dn = d - '0';
		else if (d >= 'A' && d <= 'F') dn = d - 'A' + 10;
		else if (d >= 'a' && d <= 'f') dn = d - 'a' + 10;		
		else {
/*			fprintf(stderr, "Not hex %c (%d)\n", d, d); */
			putc(d, stderr);
			putc(c, fp_out);
			ungetc(d, fp_in);
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
/*	m = decryptbyte((char) n, &cryptee); */
	m = decryptbyte((unsigned char) n, &cryptee); /* ??? */
	return m;
}

void showchar(int m, FILE *fp_out) {
	int c, d;
	if (hexadecflag != 0) {
		c = m >> 4; 
/*		d = m & 017; */
		d = m & 15;		
		if (c < 10) putc(c + '0', fp_out);
		else putc(c + 'A' - 10, fp_out);
		if (d < 10) putc(d + '0', fp_out);
		else putc(d + 'A' - 10, fp_out);
	}
	else {
		if (deliceflag != 0) {
			if (m > 127) {
				putc('M', fp_out);	putc('-', fp_out);
				m = m - 128;
			}
			if (m < 32 && m != '\n') {
				putc('^', fp_out);
				m = m + 64;
			}
		}
		putc(m, fp_out);
	}
}

/* returns -1 upon invalid hex --- 0 upon EOF */

int straight(FILE *fp_in, FILE *fp_out) {
	int k=0; /* long */
	int m;
	char *s=pstoken;

	if (charenflag != 0)
		cryptee = RCHARSTRING;  /* encryption seed for charstring coded */
	else cryptee = REEXEC;  /* encryption seed for eexec coded stuff */

	for(;;) {
		m = getnextchar(fp_in, fp_out);
		if (m == -1) return -1;			/* EOF */
		if (m == -2) return -2;			/* invalid hex digit */
		
/*		if (k >= EXTRACHAR)	*/
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
			if (showextra != 0) showchar(m, stdout); 
			k++;
		}
/*		k++; */
	}
	if (traceflag != 0) {
/*	show control character terminating `closefile' token (in encrypted) */
		if (m < ' ') printf(" C-%c ", m+64);
		else printf(" %c ", m);
	}
/*  swallow next character if white space (reading non-encrypted now) */
	m = getc(fp_in); 
/*	changed to read to end of line to deal with junk 96/Feb/22 */
	while (m >= ' ') m = getc(fp_in);
	(void) ungetc(m, fp_in);	/* do we need this */
/*	if (m < ' ') {
		if (verboseflag) printf("C-%c ", m+64);
	} 
	else (void) ungetc(m, fp_in);  */
/*	putc('\n', fp_out); */  /* ? */
	return 0;
}

#ifdef IGNORED
	if (traceflag != 0) {
/*	show control character terminating `closefile' token */
		if (m < ' ') printf(" C-%c ", m+64);
		else printf(" %c ", m);	
/*  swallow next character if white space */
		m = getc(fp_in); 
		if (m < ' ') printf("C-%c ", m+64);
		else (void) ungetc(m, fp_in);
	}
#endif

/* static unsigned char *normalcode[] = { */

static char *normalcode[] = {
	"CODE-0", "hstem", "CODE-2", "vstem", "vmoveto", "rlineto", "hlineto",
		"vlineto", "rrcurveto", "closepath", "callsubr", "return",
			"escape", "hsbw", "endchar", "CODE-15", "CODE-16", "CODE-17",
				"CODE-18", "CODE-19", "CODE-20", "rmoveto", "hmoveto",
					"CODE-23", "CODE-24", "strokewidth", "baseline",
						"capheight", "bover", "xheight", "vhcurveto",
							"hvcurveto" }; 

/* static unsigned char *escapecode[] = { */
	
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
										"setcurrentpoint" };

/* stack probable should perhaps be double, not long - to accomadate "div" */

static long numstack[MAXNUMBERS];

int numinx;
int xold, yold;
int sbx, sby;
/* int widthx, widthy; */

void pushstack(long num) {				/* push a number on the stack */
	numstack[numinx++] = num;
	if (numinx >= MAXNUMBERS) {
		fprintf(stderr, "ERROR: Overflowed number stack "); 
		numinx = MAXNUMBERS-1;
	}
}

void dumpstack(FILE *fp_out, char *name) { /* dump numbers in stack */
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

void dumprelative(FILE *fp_out, char *name) {
	assert(numinx == 2);
	xold = xold + (int) numstack[0];
	yold = yold + (int) numstack[1];
	fprintf(fp_out, "%d %d %s\n", xold, yold, name);
	numinx = 0;
}

void dumprcurveto(FILE *fp_out, char *name) {
	int k;
	assert(numinx == 6);

	for (k=0; k < 3; k++) {
		xold = xold + (int) numstack[2*k];
		yold = yold + (int) numstack[2*k+1];
		fprintf(fp_out, "%d %d ", xold, yold);
	}
/*	fprintf(fp_out, "%s\n", name); */
	fputs(name, fp_out); putc('\n', fp_out);
	numinx = 0;
}

void dumphstack(FILE *fp_out, char *name) {
	int k;
	for (k = 0; k < numinx/2; k++) 
		numstack[2*k] = numstack[2*k] + (long) sby;
	dumpstack(fp_out, name);
}

void dumpvstack(FILE *fp_out, char *name) {
	int k;
	for (k = 0; k < numinx/2; k++) 
		numstack[2*k] = numstack[2*k] + (long) sbx;
	dumpstack(fp_out, name);
}

void cleanoutstack(FILE *fp_out, char *name) {
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

int chardecode(FILE *fp_in, FILE *fp_out, int maxb) {
	int k=0, i=0, j, n, m;
	long ln;
	long position;
	char *name;

/*	printf("%d  ", maxb); */
	numinx=0;
/*	fprintf(fp_out, "\n"); */
	cryptee = RCHARSTRING;  /* encryption seed for charstring coded */

	sbx = 0; sby = 0; 		/* in case it is a subr and has no hsbw */
	xold = sbx; yold = 0;

	for(;;) {
		if (i >= maxb) return -1;
		m = getnextchar(fp_in, fp_out); i++;
		if (m == -1) break;			/* EOF */
		if (m == -2) return -1;		/* invalid hex digit */
		assert(m >= 0); assert (m <= 255);
/*		if (k < EXTRACHAR) { */
		if (k < lenIV) {
			if(showextra != 0) showchar(m, stdout); /* fp_out */
		}
		else {
			if (m == 12) {
				n = getnextchar(fp_in, fp_out); i++;
				if (n == -1) break;			/* EOF */
				if (n == -2) return -1;		/* invalid hex digit */
				if (n < 0 || n > 34) {
					fprintf(stderr, 
						"ERROR: Code following escape (12) is bad (%d)\n", n);
					exit(7);
				}
/*				assert (n >= 0); */
/*				assert (n < 34); */
				name = escapecode[n];
				if (strncmp(name, "ESCAPE", 4) == 0) {
					if (invalidflag++ < maxinvalid) {
						fprintf(stderr, "ERROR: Invalid escape code (%d) ", n);
						position = ftell(fp_in);
						fprintf(stderr, "at byte %ld\n", position);
					}
				}
				cleanoutstack(fp_out, name);
/*				fprintf(fp_out, "%s", name); */
/*				if (n != 12) fprintf(fp_out, "\n");
				else fprintf(fp_out, " "); */
			}
			else if (m <= 31) {
				name = normalcode[m];
				if (strncmp(name, "CODE", 4) == 0) {
					if (invalidflag++ < maxinvalid) {
						fprintf(stderr, "ERROR: Invalid code (%d) ", m);
						position = ftell(fp_in);
						fprintf(stderr, "at byte %ld\n", position);
					}
				}
				cleanoutstack(fp_out, name);
/*				fprintf(fp_out, "%s\n", name); */
			}
			else if (m >= 32 && m <= 246) 
				pushstack((long) (m - 139));
/*				fprintf(fp_out, "%d ", m - 139); */
			else if (m >= 247 && m <= 250) {
				n = getnextchar(fp_in, fp_out); i++;
				if (n == -1) break;			/* EOF */
				if (n == -2) return -1;		/* invalid hex digit */
				assert (n >= 0);
				pushstack((long) (((m - 247) << 8) + n + 108)); 
/*				fprintf(fp_out, "%d ", ((m - 247) << 8) + n + 108); */
			}
			else if (m >= 251 && m <= 254)  {
				n = getnextchar(fp_in, fp_out); i++;
				if (n == -1) break;			/* EOF */
				if (n == -2) return -1;		/* invalid hex digit */
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
					if (n == -1) return 0;		/* EOF */
					if (n == -2) return -1;		/* invalid hex digit */
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

/* void decodeflag (int c) {
	switch(c) { 
		case 'v': if (verboseflag++ > 0) traceflag = 1; break;
		case '?': detailflag = 1; break;
		case 'a': absoluteflag = 1; charscan = 1;
		case 'd': decodecharflag = 1;			
		case 'c': charenflag = 1; break;
		case 's': showextra = 1; break;
		case 'h': hexadecflag = 1; break;
		case 'r': charscan = 1;					
				  decodecharflag = 1;	
				  charenflag = 1;	
		case 'b': binaryflag = 1; break;
		case 'o': outflag = 1; break;
		case 'n': convertreturn = 1; break;
		case 'e': eexecscan = 1; break;		
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
} */

/* \tb: input is binary, not hexadecimal\n\ */
/* \th: produce output in hexadecimal, not ASCII/binary\n\ */

/* void showusage(char *s) {
	fprintf (stderr, "\
Correct usage is:\n\
%s [-{v}{s}{e}{r}{c}{d}{a}{n}] <in-file> \n", s); 
	fprintf (stderr, "\
\tv: verbose mode\n\
\ts: show random starting bytes\n\
\te: scan up to eexec before decrypting\n\
\tr: scan up to successive RDs to decrypt CharStrings (=> c, d)\n\
\t\tc: use CharString decoding instead of eexec\n\
\t\td: decode CharString in addition to decrypting\n\
\ta: decode CharString and show absolute commands (=> c, d)\n\
\tn: convert <return> to <return> + <newline> (Mac => PC)\n\
\t   output file current directory, same name, extension 'dec' or 'pln'\n\
");
	exit(1);
} */

/* \to: next argument is output file\n\ */


static int scanuptoeexec(FILE *fp_in, FILE *fp_out) { /* return zero upon EOF */
	int n;
	static char line[MAXLINE];
	char *s;
	long current;

	if (traceflag != 0) printf("Scanning up to eexec\n");
	current = ftell(fp_in);
	while ((n = getline(fp_in, line)) != 0 && strstr(line, "eexec") == NULL) {
		fputs(line, fp_out);
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
		fputs(line, fp_out);
	}
	return n;
}

/* Different forms to take care of: */
/* /charname <m> RD ...... ND */  /* /charname <m> -| ...... | */
/* Possibly spaces before /charname (Digital Typeface Fonts) ? */
/* dup <n> <m> RD ...... ND */    /* dup <n> <m> -| ...... | */ /* funky */

int scanuptoRD(FILE *fp_in, FILE *fp_out) {		/* return zero upon EOF */
	int c, k, m;
	static char line[MAXLINE+1];
	static char name[MAXLINE];
	char *s, *t;
	int n = 0;

/*	if (traceflag != 0) printf("Scanning up to RD\n"); */
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
				ungetc(c, fp_in);
				c = '\n';			/* isolated \r turned into \n */
			}
			break;
		} 
		if (c == ' ' && n > 3) {	/* space way after DUP or whatever */
/*		new code for lenIV */
			*s = '\0';
/*			if (debugflag) printf("LINE: %s\n", line); */
			if (strncmp(line, "/lenIV", 6) == 0) {	/* 97/July/20 */
				if (sscanf(line, "/lenIV %d", &lenIV) == 1) {
					if (lenIV == 4) printf("%s", line);
					else printf("/lenIV %d def ???\n", lenIV);
					if (removelenIV) {	/* comment out ? */
						memmove(line+2, line, strlen(line)); 
						*line = '%';
						*(line+1) = ' ';
						s += 2; n += 2;
/*						printf("LINE: %s", line); */
					}
				}
/*				printf("%s ???\n", line); */
			}
/* end of new code */
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
					fputs(line, fp_out);	/* flush start to output */
					strcpy(line, t);		/* bring CharString to head */
					n = strlen(line);		/* ??? */
				}
/* end of special case code */
/*				t = line + strspn(line, " \t"); */ /* skip over whitespace */
				t = line;
/*				skip over space, tab, and null 98/Apr/20 */
				while (*t == ' ' || *t == '\t' || *t == '\0') t++;
				if (sscanf(t, "/%s %d ", name, &m) < 2) {
/* well, maybe it is in that funky format we used to use ? */
					if (sscanf(t, "%s %d %d ", name, &k, &m) < 3) {
/* sigh, we have a bad CharString */
						fprintf(stderr, "WARNING: Missing char name: %s\n", line);
/* maybe just the name is missing ? */ /* Corel Draw bug ! */
/*						if (sscanf(line, "/ %d ", &m) != 1) { */
						if (sscanf(t, "/ %d ", &m) < 1) {
/* sigh, we REALLY have a bad CharString */
							fprintf(stderr, "ERROR: Don't understand: %s\n", line);
							exit(3);
						}
					}
				}
/*				fprintf(fp_out, "\n%s\n", line); */
/*				fprintf(fp_out, "\n%s\n", t); */
				putc('\n', fp_out);
				fputs(t, fp_out);
				putc('\n', fp_out);
/*				printf("%d ", m);*/
				if (m < 0 || m > 20000) {
					fprintf(stderr, "Bad encrypted string length %d\n", m);
					exit(1);
				}
/*				if (traceflag != 0) printf("%s %d\n", rname, m); */
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
				fprintf(stderr, "LINE TOO LONG: %s\n", line);
				exit(3);
		}
	}
	*s++ = (char) c;
	n++;	/* ??? */
	*s++ = (char) '\0';
	n++;	/* ??? */
	if (c == EOF) return -1;
	else {
/*		fprintf(fp_out, "%s", line); */
		fputs(line, fp_out);
/*		putc(c, fp_out); */ /* nl */
		return 0;
	}
}

int decrypt(FILE *fp_out, FILE *fp_in) {
	int c, n;

	lenIV=EXTRACHAR;				/* moved here 98/Jun/30 */
	if (decodecharflag == 0) {		/* if not decoding chardefs */
		if (eexecscan != 0) (void) scanuptoeexec(fp_in, fp_out);
		while (straight(fp_in, fp_out) == 0)
			if (scanuptoeexec(fp_in, fp_out) == 0) break;
	}
	else if (charscan != 0) {   /* if decoding chardefs */
		while ((n = scanuptoRD(fp_in, fp_out)) >= 0) {
			if (n > 0) {
				if (chardecode(fp_in, fp_out, n) >= 0) break;
				c = getc(fp_in);
				if (c == EOF) {
					fprintf(stderr, "Unexpected EOF\n");
					exit(3);
				}
				if (c != ' ') putc(c, fp_out);
			}
		}
	}		
	else {
		while (chardecode(fp_in, fp_out, -1) < 0) {
			c = getc(fp_in); 
			if (c == EOF) {
				fprintf(stderr, "Unexpected EOF\n");
				exit(3);
			}
			putc(c, fp_out);
			if (c == '<') putc('\n', fp_out);
		}
	}
	return 0;
}

