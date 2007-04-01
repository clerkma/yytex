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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXROW 128

#define MAXHEXSTRING 16000

#define MAXCOLUMN 78

#define MAXLINE 256

#define MAXFILENAME 128

unsigned char buffer[MAXHEXSTRING];
unsigned char output[MAXHEXSTRING];
unsigned char *schar;				/* pointer into the above for PK */

char line[MAXLINE];

char jobname[MAXLINE]="";

unsigned char rw[MAXROW];	/* place to build row */

int gp;						/* pointer into source string */
int cp;						/* column pointer */
int rc;						/* row repeat count */
int endrow;					/* flag to indicate hit end of row */
int cplast;					/* how far we got in last row */

int verboseflag=1;
int traceflag=1;
int debugflag=0;
int dotsflag=1;
int showflag=1;

unsigned int nlen;

unsigned int outnlen, outk;

int cc, advance, ndict, nchars;

long hsize, vsize;

int magnification, hdpi, vdpi;

char font[16]="";

/***************************************************************************/

/* read hex string such as <0123456789ABCDEF> into buffer supplied */
/* args: file stream, buffer for result, buffer length */
/* returns: length of string or 0 if EOF --- ignores white space */
/* start at < --- reads up to an including > */
/* complains invalid characters, odd number of hex characters */

unsigned int readhexstring (FILE *input, unsigned char *buffer, unsigned int nlen) 
{
	int c, d;
	unsigned char *s=buffer;

	c = getc(input);
	while (c != '<' && c != EOF) c = getc(input);
/*	if (c == EOF) return -1; */
	if (c == EOF) return 0;
	if (c == '<') c = getc(input);					/* always */
	while (c != '>') {
		while (c <= ' ' && c != EOF) c = getc(input); /* ignore white space */
		if (c == EOF) {
			fprintf (stderr, "EOF in hex string\n");
/*			return -1; */
			return 0;
		}
		if (c == '>') break;
		if (c >= '0' && c <= '9') c = c - '0';
		else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
		else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
		else {
			fprintf (stderr, "Bad character in hex string %c\n", c);
			break;
		}
		d = getc(input);
		while (d <= ' ' && d != EOF) d = getc(input); /* ignore white space */
		if (d == EOF) {
			fprintf (stderr, "EOF in hex string\n");
/*			return -1; */
			return 0;
		}
		if (d == '>') {
			fprintf (stderr, "Odd number of hex characters\n");
			*s++ = (unsigned char) ((c << 4));	/* treat last as zero */
			c = d;			/* to stop scan to next > */
			break;
		}
		if (d >= '0' && d <= '9') d = d - '0';
		else if (d >= 'A' && d <= 'F') d = d - 'A' + 10;
		else if (d >= 'a' && d <= 'f') d = d - 'a' + 10;
		else {
			fprintf (stderr, "Bad character in hex string %c\n", d);
			break;
		}
/*		if ((s - buffer) >= nlen-1)  */
		if (s >= buffer + nlen-1) {
			fprintf(stderr, "Exceeded hex string buffer size (%u bytes)\n",
					nlen);
			break;
		}
		*s++ = (unsigned char) ((c << 4) | d);
		c = getc(input);
	}
	*s = '\0';					/* just for fun ! */
	if (c != '>') while (c != '>' && c != EOF) c = getc(input);
	if (c == EOF) {
		fprintf (stderr, "EOF in hex string\n");
/*		return -1; */
		return 0;
	}
	return (s - buffer);
}

void showhexstring(unsigned char *buffer, unsigned int nlen, int width, int cc) 
{
	int c, d, n=0, maxcolumn, column;
	unsigned int k;
	if (debugflag) printf("nlen %u, width %d, cc %d\n", nlen, width, cc);
	if (nlen == 0) return;
	if (width == 0) return;
	printf("<\n");
	column = 0;
	if (width != 0) {
		n = ((width + 7) / 8);			/* bytes per image row */
/*		each image row will take n * 2 + 1 characters in output */
/*		maxcolumn = (MAXCOLUMN / (n * 2 + 1)) * (n * 2 + 1); */
		maxcolumn = (MAXCOLUMN / (n * 2 + 1)) * (n * 2 + 1) - 1;
	}
	else maxcolumn = MAXCOLUMN;

	for (k = 0; k < nlen; k++) {
		c = (buffer[k] >> 4) & 15;
		d = buffer[k] & 15;
		if (c < 10) c = c + '0';
		else c = c - 10 + 'A';
		if (d < 10) d = d + '0';
		else d = d - 10 + 'A';
		if (column >= maxcolumn) {
			putc('\n', stdout);
			column = 0;
		}
		if (column != 0 && n != 0 && k % n == 0) {
			putc(' ', stdout);
			column++;
		}
		putc(c, stdout);
		putc(d, stdout);
		column += 2;
	}
/*	if ((k % n) != 0) printf("Length not multiple of width\n"); */
	printf("\n> %d\n", cc);  
/*	printf("\n>\n"); */
}

/***************************************************************************/

/* read simple token - ended by white space or / ( [ { < } */
/* return -1 if hit end of file */ /* returns 0 if run into delimiter */

/* int readtoken(FILE *input, char *buffer, int nlen) */
int readtoken(FILE *input, char *buffer, unsigned int nlen) {
	char *s=buffer;
	int c;

	*s = '\0';					/* in case skip out early */
	c = getc(input);
	while (c <= ' ' && c != EOF) c = getc(input);
	if (c == EOF) return -1;	/* nothing seen before EOF */
	if (c == '/') {
		*s++ = (char) c;
		c = getc(input);
	}
	while (c > ' ') {
		if (c == '\\') c = getc(input);
		if (c == '/' || c == '(' || c == '[' || c == '{' || c == '<') { /* } */
/*			ungetc(c, input); */
			break;
		}
/*		if ((s - buffer) >= nlen-1) */
		if (s >= buffer + nlen-1) {
			fprintf(stderr, "Exceeded token buffer size (%u bytes)\n", nlen);
			break;
		}
		*s++ = (char) c;
		c = getc(input);
	}
	*s = '\0';
	if (c != EOF) ungetc (c, input);
/*	if (traceflag) printf("TOKEN: %s (%c)\n", buffer, c); */
	if (debugflag) printf("TOKEN: %s (%c)\n", buffer, c);
	return (s - buffer);
}

/* crude hack to read PS string ... for jobname only */

int readstring(FILE *input, char *buffer, unsigned int nlen) {
	char *s=buffer;
	int nesting=0;
	int c;

	*s = '\0';					/* in case skip out early */
	c = getc(input);
	while (c <= ' ' && c != EOF) c = getc(input);
	if (c == EOF) return -1;	/* nothing seen before EOF */
	if (c != '(') {
		fprintf(stderr, "ERROR: string not found\n");
		ungetc(c, input);
		return 0;
	}
	for (;;) {
		c = getc(input);
		if (c == EOF) return -1;
		if (c == '\\') {
			*s++ = (char) c;
			c = getc(input);
			*s++ = (char) c;
			continue;
		}
		if (c == ')') {
			if (nesting == 0) break;
			else nesting--;
		}
		else if (c == '(') nesting++;
		*s++ = (char) c;
		if (s >= buffer + nlen-1) {
			fprintf(stderr, "Exceeded token buffer size (%u bytes)\n", nlen);
			return -1;
		}
	}
	*s = '\0';
	return (s - buffer);
}

/* try and skip over more complex token in PS file */

int skipovercomplex (FILE *input) {
	int c, depth=0;
	c = getc(input);
	if (c == EOF) return -1;
	if (c == '(') {
		for (;;) {
			if (c == '\\') c = getc(input);
			if (c == '(') depth++;
			else if (c == ')') {
				depth--;
				if (depth == 0) break;
			}
			c = getc(input);
			if (c == EOF) return -1;
		}
		return 0;
	}
	else if (c == '[') {
		for (;;) {
			if (c == '\\') c = getc(input);
			if (c == '[') depth++;
			else if (c == ']') {
				depth--;
				if (depth == 0) break;
			}
			c = getc(input);
			if (c == EOF) return -1;
		}
	}
	else if (c == '{') {			/* } */
		for (;;) {
			if (c == '\\') c = getc(input);
			if (c == '{') depth++;	/* } */
			else if (c == '}') {	/* { */
				depth--;
				if (depth == 0) break;
			}
			c = getc(input);
			if (c == EOF) return -1;
		}
	}
	else if (c == '<') {
		for (;;) {
			if (c == '\\') c = getc(input);
			if (c == '<') depth++;
			else if (c == '>') {
				depth--;
				if (depth == 0) break;
			}
			c = getc(input);
			if (c == EOF) return -1;
		}
	}
}

char *textext[32] = {
"Gamma", "Delta", "Theta", "Lambda", "Xi", "Pi", "Sigma", "Upsilon", 
"Phi", "Psi", "Omega", "ff", "fi", "fl", "ffi", "ffl", 
"dotlessi", "dotlessj", "grave", "acute", "caron", "breve", "macron", "ring", 
"cedilla", "germandbls", "ae", "oe", "oslash", "AE", "OE", "Oslash"
};

int readoneglyph (FILE *input) {
	int c;
	unsigned int nbyte;
	long current;
	unsigned int width, height;
	int xoff, yoff;
/*	int advance; */
	char schr[16];

	c = getc(input);
	while (c != '<' && c != EOF) c = getc(input);
	if (c == EOF) return -1;
	ungetc(c, input);
	current = ftell(input);
/*	nlen = readhexstring(input, buffer, sizeof(buffer)); */
	nlen = readhexstring(input, buffer, MAXHEXSTRING-5);
/*	if (nlen < 0) return-1; */
	if (nlen == 0) return-1;			/* EOF */
	width = buffer[nlen-5];
	height = buffer[nlen-4];
	xoff = 128 - buffer[nlen-3];
	yoff = buffer[nlen-2] - 127;
	advance = buffer[nlen-1];

/*	if (showflag) showhexstring(buffer, nlen, width, cc); */
	nbyte = ((width + 7) / 8) * height;
	if (nlen != nbyte + 5)
		printf("Mismatch in string size %u <> %u\n", nlen, nbyte+5);
	readtoken (input, line, sizeof(line));
	if (sscanf (line, "%d", &cc) == 0) {
		if (strcmp(line, "I") != 0) 
			fprintf(stderr, "Don't understand char code: %s\n", line);
		cc++;	/* I */
	}
	if (traceflag) {
		if (cc >= 32 && c < 128) {
			schr[0] = (char) cc;
			schr[1] = '\0';
		}
		else if (cc < 32) {
			strcpy(schr, textext[cc]);
		}
		else {
			schr[0] = ' ';
			schr[1] = '\0';
		}
		printf("%s, %d (%s), %u bytes at %ld\tW %d H %d X %d Y %d A %d\n",
			   font, cc, schr,
			   nlen, current, width, height, xoff, yoff, advance);
	}
	if (showflag) showhexstring(buffer, nlen, width, cc); 
	return 0;
}

/* we expect after %%EndProcSet the following */
/* TeXDict begin 40258431 52099146 1000 300 300 (epslatex.dvi) @start */
/* hsize, vsize, magnification, hdpi, vdpi jobname string */
/* but may see another %%BeginProcSet first ... */

int scantoendprocset (FILE *input) {
	int c, n;
/*	int k; */

	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line != '%') continue;			/* look only at comments */
		if (strncmp(line, "%%EndProcSet", 12) == 0) {
			c = getc(input); ungetc(c, input);
			if (c == '%') continue;			/* probably %%BeginProcSet */
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (strcmp(line, "TeXDict") != 0)
				printf("ERROR: expecting %s, not %s\n", "TeXDict", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (strcmp(line, "begin") != 0)
				printf("ERROR: expecting %s, not %s\n", "begin", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%ld", &hsize) < 1)
				printf("ERROR: expecting %s, not %s\n", "40258431", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%ld", &vsize) < 1)
				printf("ERROR: expecting %s, not %s\n", "52099146", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%d", &magnification) < 1)
				printf("ERROR: expecting %s, not %s\n", "magnification", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%d", &hdpi) < 1)
				printf("ERROR: expecting %s, not %s\n", "horiz dpi", line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (sscanf(line, "%d", &vdpi) < 1)
				printf("ERROR: expecting %s, not %s\n", "vert dpi", line);
			n = readstring(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
/*			jobname = xstrdup(line); */
			strcpy(jobname, line);
			n = readtoken(input, line, sizeof(line));
			if (n < 0) return -1;		/* hit EOF */
			if (strcmp(line, "@start") != 0) 
				printf("ERROR: expecting %s, not %s\n", "@start", line);
			if (traceflag)
				printf("TeXDict begin %ld %ld %d %d %d (%s) @start\n",
					  hsize, vsize, magnification, hdpi, vdpi, jobname);
			if (verboseflag)
		printf("hsize: %lg vsize: %lg magnification: %d hdpi: %d vdpi: %d\n",
			   (double) hsize / 65781.76, (double) vsize / 65781.76,
				   magnification, hdpi, vdpi);
			if (verboseflag)
				printf("jobname: %s\n", jobname);
/*			if (n == 0) {
				while ((c = getc(input)) > ' ') ;
			} */
			return 0; 
		}
	}
	return -1;
}

void decompressglyph(unsigned char *, int);

/* extract bitmap charsting from PS file */
/* returns 0 at end of this font, if another follows */
/* returns -1 if no more fonts, or if hit EOF */

int extractbitmap (FILE *input) {
	int n;
	int c;
/*	double fscale; */

/*	First scan to start of font, and read name: /Fe ... */

	for (;;) {								/* get to /Fa ... */
		n = readtoken(input, line, sizeof(line));
		if (n < 0) {
			fprintf(stderr, "Hit EOF in extractbitmap\n");
			return -1;				/* hit EOF */
		}
		if (n == 0) {						/* hit ( [  < */
			if (skipovercomplex(input) < 0) return -1;
		}
		if (*line == '/') {					/* remember the name */
			strcpy(font, line+1);			/* /Fa 1 55 df <...> */
			n = readtoken(input, line, sizeof(line));
			sscanf (line, "%d", &nchars);	/* how many chars */
			n = readtoken(input, line, sizeof(line));
			ndict = 0;
			sscanf (line, "%d", &ndict);	/* dict size needed */
/*			if (n == 0) printf("Font: %s %d (NOT PK)\n", font, nchars); */
/*			if (verboseflag) */
			if (verboseflag && n > 0) { 
				if (dotsflag) putc('\n', stdout);
				printf("Font: %s %d %d\n", font, nchars, ndict);
			}
			break;							/* time to move on */
		}
	}
/*	/Fa 167[141 46[69 69 40[{}3 100.000000 /MTEX rf */

	for (;;) {							/* loop over chars */
		for (;;) {							/* look for next char */
			c = getc(input); ungetc(c, input);
			if (c == '/') {				/* hit next font --- no chars here */
				n = 0;
				return 0;
			}
			n = readtoken (input, line, sizeof(line));
			if (n < 0) {
				if (traceflag) printf("Return on EOF?\n");
				return -1;			/* EOF */
			}
			if (n == 0) {						/* hit ( [  < */
				if (debugflag) {
					c = getc(input); ungetc(c, input);
					printf("Break out on n == 0 (%c)?\n", c);
				}
				break;				/* found < presumably */
			}
		}

		if (n > 0) {				/* cannot happen */
			if (traceflag)
				printf("Hit end of bitmap font definitions: %s\n", line);
/*				break; */					/* hit end of fonts */
			return -1;					/* hit end of fonts */
		}
		readoneglyph(input);				/* read actual hex data */
		decompressglyph(buffer, nlen);
		if (traceflag) printf("Finished with glyph %d\n", cc);
		
		for (;;) {							/* get to next glyph */
			c = getc(input);
			while (c != EOF && c <= ' ') c = getc(input);
			ungetc(c, input);
			if (c == '/') {
/*				ungetc(c, input); */		/* put it back */
				n = 0;
				return 0;					/* hit next font */
			}
/*			if (c == '<') ungetc(c, input); */
			n = readtoken (input, line, sizeof(line));
			if (*line == '/') printf("WRONG! %s %d\n", line, n);
			if (n < 0) {
				if (traceflag) printf("Hit EOF\n");
				return -1;			/* EOF */
			}
			if (n == 0) break;				/* presumably hit < */
			if (strcmp(line, "end") == 0) {
				if (traceflag) printf("Break out on %s\n", line);
				return -1;
/*				break; */
			}
			if (strncmp(line, "%%", 2) == 0) {
				if (traceflag) printf("Break out on %s\n", line);
				return -1;
/*				break; */
			}
			if (*line == '/') {				/* should not happen */
				strcpy(font, line+1);		/* /Fa 1 55 df <...> */
				printf("ERROR FONT: %s\n", font);
/*				freecharstrings(); */
				n = 0;
				break;
			}
			if (n == 0) break;				/* hit next char ? */
		}									/* end of loop for next char */
/*		if (n > 0) break;					/* Hit the end */
		if (n > 0) return -1; 				/* Hit the end */
	}
/*	return 0; */							/* normal return - next font */
/*	if (traceflag) printf("End of extractbitmap\n"); */
/*	freecharstrings(); */
}

/***************************************************************************/

void printhex (int n) {		/* debugging output */
	int c,d;
	c = (n >> 4) & 15;
	d = n & 15;
	if (c < 10) c = c + '0';
	else c = c - 10 + 'A';
	if (d < 10) d = d + '0';
	else d = d - 10 + 'A';
	printf("%c%c ", c, d);
}

void adv(int count) {		/* advance column */
	cp += count;
	if (count > 17) fprintf(stderr, "COUNT ERROR %d ", count);
	if (debugflag) printf("adv %d ", count);
	if (cp >= MAXROW) {
		fprintf(stderr, "Exceeded row space\n");
		exit(1);
	}
}

void chg(int count) {		/* get character data */
	int k;
	if (count > 17) fprintf(stderr, "COUNT ERROR %d ", count);
	if (debugflag) printf("chg %d ", count);
	for (k = 0; k < count; k++)	{
		rw[cp+k] = buffer[gp+k];
		if (debugflag) printhex(rw[cp+k]);
	}
	gp += count;
	adv(count);
}

void nd(void) {				/* reset column pointer and end row */
	if (debugflag) printf("nd ");
	cplast = cp;			/* remember how far we got on that one */
	cp = 0;
	endrow = 1;
}

void lsh(void) {			/* left shift */
	int n = rw[cp], m;
	if (debugflag) printf("lsh %d ", n);
	if (n == 0) m = 1;
	else {
		if (n == 255) m = 254;
		else {
			m = ((n * 2) & 255) | (n & 1);
		}
	}
	if (debugflag) printhex(m);
	rw[cp] = (unsigned char) m;
	adv(1);
}

void rsh(void) {			/* right shift */
	int n = rw[cp], m;
	if (debugflag) printf("rsh %d ", n);
	if (n == 0) m = 128;
	else {
		if (n == 255) m = 127;
		else {
			m = (n / 2) | (n & 128);
		}
	}
	if (debugflag) printhex(m);
	rw[cp] = (unsigned char) m;
	adv(1);
}

void clr(int count) {		/* n zero bytes */
	int k;
	if (count > 17) fprintf(stderr, "COUNT ERROR %d ", count);
	if (debugflag) printf("clr %d ", count);
	for (k = 0; k < count; k++) rw[cp+k] = 0;
	adv(count);
}

/* original actually has a bug in that it gets string one too long */

void set(int count) {		/* n all one bytes */
	int k;
	if (count > 17) fprintf(stderr, "COUNT ERROR %d ", count);
	if (debugflag) printf("set %d ", count);
	for (k = 0; k < count; k++) rw[cp+k] = 255;
	adv(count);
}

void setrc (int count) {
	if (debugflag) printf("set rc %d ", count);	/* repeat count */
	rc = count;
}

void decompressglyph(unsigned char *buffer, int nlen) {
	int c, op, count, k, m, width, height, nw;
	unsigned int nbytes;
	width = buffer[nlen-5];				/* width in bits */
	height = buffer[nlen-4];			/* height in bits */
	nw = (width + 7) / 8;				/* width in bytes */
	nbytes = ((width + 7) / 8) * height;	/* byte in decompressed string */
	gp = 0;								/* index into source bytes */
	outk = 0;							/* index into decompressed bytes */
	for (k = 0; k < nw; k++) rw[nw] = 0;	/* reset rw initially to 0 */
	for (;;) {							/* until exhaust source bytes */
		endrow=0;
		rc = 0;							/* reset row repeat count */
		while (endrow == 0) {
/*			c = *buffer++; */
			c = buffer[gp++];			/* grab next source byte */
			op = c / 18;			/* op code 0, 1, ... 13 */
			count = c % 18;			/* byte count 0, 1, ... 17 */
			if (debugflag) printf("\ncount %d op %d\n", count, op);
			switch (op) {
				case 0:
					adv(count);		/* skip count bytes, insert 1 byte */
					chg(1);
					break;
				case 1:
					adv(count);
					chg(1);
					nd();			/* and end row */
					break;
				case 2:
					chg(count+1);	/* insert count+1 source bytes */
					break;
				case 3:
					chg(count+1);
					nd();			/* and end row */
					break;
				case 4:
					adv(count);		/* skip count bytes and left shift */
					lsh();
					break;
				case 5:
					adv(count);
					lsh();
					nd();			/* and end row */
					break;
				case 6:
					adv(count);		/* skip count bytes and right shift */
					rsh();
					break;
				case 7:
					adv(count);
					rsh();
					nd();			/* and end row */
					break;
				case 8:
					adv(count+1);	/* skip count+1 bytes */
					break;
				case 9:
					setrc(count);
					nd();
					break;
				case 10:
					set(count+1);	/* count+1 all one bytes */
					break;
				case 11:
					clr(count+1);	/* count+1 all zero bytes */
					break;
				case 12:
					adv(count);		/* skip count bytes get 2 source bytes */
					chg(2);
					break;
				case 13:
					adv(count);
					chg(2);
					nd();			/* and end row */
					break;
				case 14:
/*					printf("END? "); */
					nd();			/* end row, ignore count */
					break;
				default:
					fprintf(stderr, "Impossible op code!\n");
					break;
			}	/* end of switch */
		}	/* end of while (endrow == 0) */
		if (debugflag) putc('\n', stdout);
		for (m = 0; m < rc+1; m++) {
			if (debugflag) {
				printf("ENDROW: ");
/*				for (k = 0; k < cplast; k++) printhex(rw[k]); */
				for (k = 0; k < nw; k++) printhex(rw[k]);
				putc('\n', stdout);
			}
/*			transfer to output buffer */
			for (k = 0; k < nw; k++) output[outk+k] = rw[k];
			outk += nw;
		}
		if (gp >= nlen-5) {
			if (debugflag)
				printf("Exhausted compressed data %d %d\n", gp, nlen);
			for (k = 0; k < 5; k++) output[outk+k] = buffer[nlen-5+k];
			outk += 5;
			outnlen = outk;
			if (verboseflag)
				showhexstring(output, outnlen, width, cc);
			if (outnlen != nbytes + 5) {
				fprintf(stderr, "ERROR: decompressed to %d instead of %d\n",
						outnlen-5, nbytes);
			}				
			putc('\n', stdout);
			break;
		}
	}	/* end of loop over rows */
}

int main (int argc, char *argv[]) {
	char filename[MAXFILENAME];
	FILE *input;
	if (argc < 2) exit(1);
	strcpy(filename, argv[1]);
	if ((input = fopen(filename, "rb")) == NULL) {
		perror(filename);
		exit(1);
	}
	scantoendprocset(input);
	extractbitmap(input);
	fclose(input);
	return 0;
}
	
