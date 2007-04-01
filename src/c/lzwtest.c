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

/*****************************************************************************/

int debugflag=1;

int quoteflag=1;

int showcodeflag=0;

int codechar[256];	/* codechar[k] is code of the most recent string */
					/* defined that ends in byte k */
					/* If no strings ends in byte k, then this is -1 */

int codeindex;		/* index into the following --- next open slot */

unsigned char lastchar[4096];	/* lastchar[code] last char of string[code] */
						/* lastchar[codechar[k]] == k */

int startcode[4096];	/* startcode[code] is code of start of string[code] */
						/* that is, of string minus last byte */
						/* or -1 if first character and there are no more */

int prevcode[4096];		/* prevcode[code] code of string that ends same byte */
						/* or -1 if first string that ends this way */

int codelength=9;			/* how many bits needed for code at this stage */

/* See whether the given string is in the table */
/* String is defined by last character and code for the start of the string */

/* int stringcode (unsigned char last, int start) { */
int stringcode (unsigned int last, int start) {
	int code;
	if (start == -1) return last;	/* single byte string code */
	code = codechar[last];			/* code of last string that end in last */ 
	while (code >= 0) {
		if (startcode[code] == start) return code;	/* found match ? */
		code = prevcode[code];		/* follow link to previous string */
	}
	return -1;						/* failed to find in table */
}

/* Add a string to the table */
/* String is defined by last character and code for start of string */
/* Returns code of new string */
/* Returns -1 if table overflowed */

/* int addstring (unsigned char last, int start) { */
int addstring (unsigned int last, int start) {
	int code;
/*	codeindex is the next available slot in the table */
	if (start == -1) return last;		/* single byte string code */
	if (codeindex == 4096) {			/* 4095 ??? */
		if (debugflag) printf("Table overflow (%d)\n", codeindex);
		return -1;	/* overflow, table full */
	}
	prevcode[codeindex] = codechar[last];
	startcode[codeindex] = start;
	lastchar[codeindex] = (unsigned char) last;
	codechar[last] = codeindex;
/*	if codeindex 2^n then increment codelength */
	if (((codeindex+1) & codeindex) == 0) {	/* codindex-1 ??? */
		codelength++;
		if (debugflag) printf ("Incrementing codelength to %d (code %d)\n",
					codelength, codeindex);
	}
	code = codeindex;
	codeindex++;					/* increment code index for next one */
	return code;
}

void cleartable (void) {
	int k;
/*	for (k = 0; k < 256; k++) codechar[k] = -1; */
/*	for (k = 0; k < 4096; k++) lastchar[k] = 0; */
	for (k = 0; k < 4096; k++) startcode[k] = -1;
	for (k = 0; k < 4096; k++) prevcode[k] = -1;
/*	codeindex = 0; */
	for (k = 0; k < 256; k++) lastchar[k] = (unsigned char) k;
/*	for (k = 0; k < 256; k++) startcode[k] = -1; */
/*	for (k = 0; k < 256; k++) prevcode[k] = -1;	*/
	for (k = 0; k < 256; k++) codechar[k] = k;
/*	256 is clear 257 is EOD */
	codeindex = 258;				/* first available slot */
	codelength = 9;					/* initial code length */
}

/* Dump out table */
/* Note that this dumps strings with characters in reverse order */

void showtabler (void) {
	int k, c, code;
	putc('\n', stdout);

/*	for (k = 0; k < 256; k++) printf("%04d: %03d ", k, lastchar[k]); */
	printf("0256: CLEAR\n");
	printf("0257: EOD\n");
/*	putc('\n', stdout); */
	for (k = 258; k < codeindex; k++) {
		printf("%04d: ", k);
		code = k;
		while (code >= 0) {
/*			printf("%03d ", c); */
			c = lastchar[code];
			printf("%c ", c);
			code = startcode[code];
		}
		putc('\n', stdout);
	}
	putc('\n', stdout);
}

/* Note that this dumps strings with characters in correct order */
/* But may require large amount of stack space */

void showreverse (int code) {
	if (code < 0) return;				/* done */
	showreverse(startcode[code]);
/*	printf("%03d ", lastchar[code]); */
	printf("%c", lastchar[code]);
}

void showtablex (void) {
	int k, code;
	putc('\n', stdout);

/*	for (k = 0; k < 256; k++) printf("%04d: %03d ", k, lastchar[k]); */
	printf("0256: CLEAR\n");
	printf("0257: EOD\n");
	putc('\n', stdout);
	for (k = 258; k < codeindex; k++) {
		printf("%04d: ", k);
		code = k;
		showreverse (code);
		putc('\n', stdout);
	}
}

int leftover;		/* byte accumulated so far */
int bitsused;		/* bits used so far */

/* stuff codelength bits of c into output stream */

void lzwputc(int code, FILE *output) {
	int c;
	if (debugflag) {
		if (code >= (1 << codelength))
		printf("code %d too long for code length %d\n", code, codelength);
	}
	leftover = (leftover << codelength) | code;
	bitsused += codelength;
	if (bitsused >= 16) {
		c = leftover >> (bitsused - 8);	/* get left most 8 bits */
		putc(c & 255, output);
		bitsused -= 8;
	}
	c = leftover >> (bitsused - 8);	/* get left most 8 bits */
	putc(c & 255, output);
	bitsused -= 8;
}

void lzwputcinit(FILE *output) {
	leftover = 0;
	bitsused = 0;
}

void lzwputcflush(FILE *output) {
	int c;
	if (bitsused == 0) return;		/* nothing left to push out */
	c = (leftover << (8 - bitsused));
	putc(c & 255, output);			/* fill last byte with zeros */
/*	leftover = 0; */
/*	bitsused = 0; */
}

int code;	/* code of the string matched so far to input */
int last;	/* last character of input string */

void initLZW(FILE *output) { /*	initialization  */
	lzwputcinit(output);
	lzwputc(256, output); 	/* write CLEAR */
	if (debugflag) printf("CLEAR ");
	cleartable();
/*	codelength = 9; */
	code = -1;
	last = -1;
}

void flushLZW(FILE *output) { /*	termination */
/*	if (code >= 0) printf("CODE %03d ", code); */
	if (code >= 0) {
		lzwputc(code, output);
		if (showcodeflag) printf("%03d ", code);
		else {
			if (quoteflag) printf("`");
			showreverse (code);
			if (quoteflag) printf("'");
		}
		printf(" ");
	}
	lzwputc(257, output);
	if (debugflag) printf("EOD ");
	lzwputcflush(output);
/*	cleartable(); */
}

void writeLZW (FILE *output, unsigned char *s, unsigned long width) {
	int k, n;
	int newcode;

	k = 0;
	n = (int) width;
	while (k < n) {
		last = *s;
		newcode = stringcode(last, code);
		if (newcode >= 0) code = newcode;
		else {
			lzwputc(code, output);
			if (debugflag) {
				if (showcodeflag) printf("%03d ", code); 
				else {
					if (quoteflag) printf("`");
					showreverse (code);
					if (quoteflag) printf("'");
				}
			}
			if (addstring (last, code) < 0) {	/* table overflow ? */
				lzwputc(256, output); 	/* write CLEAR */
				if (debugflag) printf("CLEAR ");
				cleartable();
/*				codelength = 9; */
			}
			code = last;
			last = -1;
		}
		k++;
		s++;
	}	/* end of while k < n */
/*	This may leave unfinished business --- code >= 0 */
}

/*****************************************************************************/

int lzwgetc(FILE *input) {
	int c;
	c = getc(input);
	if (c == EOF) {
		printf("Premature EOF");
		exit(1);
	}
	leftover = (leftover << 8) | c;
	bitsused += 8;
	if (bitsused < codelength) {
		c = getc(input);
		if (c == EOF) {
			printf("Premature EOF");
			exit(1);
		}
		leftover = (leftover << 8) | c;
		bitsused += 8;
	}
	c = leftover >> (bitsused - codelength);
	bitsused -= codelength;
	return c & ((1 << codelength) - 1);
}

void showcodes (FILE *input) {
	leftover = 0;
	bitsused = 0;
	codelength = 9;
	printf("Show Codes: ");
	code = lzwgetc(input);
	printf("%04d ", code);
	while (code != 257) {				/* EOD yet ? */
		if ((code & (code+1)) == 0) codelength++;
		code = lzwgetc(input);
		printf("%04d ", code);
	}
}

/*****************************************************************************/

char *test1="ababaaacaaaad";
char *test2="ababcabcdabcdeabcdefabcdefgabcdefgh";
char *test3="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
char *test4="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
char *test5="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";

void runLZWtest(unsigned char *test, char *name) {
	FILE *input, *output;
	char *filename="lzwtest.bin";
	printf("%s\n", name);
	if ((output = fopen(filename, "wb")) == NULL) {
		perror(filename);
		exit(1);
	}
	initLZW(output);
	writeLZW(output, test, (unsigned long) strlen((char *) test));
	flushLZW(output);	
	showtablex();
	fclose(output);

	printf("%s\n", name);
	if ((input = fopen(filename, "rb")) == NULL) {
		perror(filename);
		exit(1);
	}
	showcodes(input);
	fclose(input);
	printf("\n");
	printf("\n");
}

int main (int argc, char *argv[]) {
	runLZWtest ((unsigned char *) test1, "test1");
	runLZWtest ((unsigned char *) test2, "test2");
	runLZWtest ((unsigned char *) test3, "test3");
	runLZWtest ((unsigned char *) test4, "test4");
	runLZWtest ((unsigned char *) test5, "test5");
	return 0;
}	
