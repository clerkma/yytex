/* Copyright 1998, 1999, 2000 Y&Y, Inc.
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* DVI one byte commands: */

enum dvicom {
/* set_char_0 = 0, set_char_1, set_char_2, */
	set1 = 128, set2, set3, set4,
	set_rule = 132, 
	put1 = 133, put2, put3, put4,
	put_rule = 137,
	nop = 138, bop, eop, push, pop,
	right1 = 143, right2, right3, right4,
	w0 = 147, w1, w2, w3, w4,
	x0 = 152, x1, x2, x3, x4,
	down1 = 157, down2, down3, down4,
	y0 = 161, y1, y2, y3, y4,
	z0 = 166, z1, z2, z3, z4,
/* fnt_num_0 = 171, font_num_1, font_num_2, font_num_3, */
	fnt1 = 235, fnt2, fnt3, fnt4,
	xxx1 = 239, xxx2, xxx3, xxx4,
	fnt_def1 = 243, fnt_def2, fnt_def3, fnt_def4,
	pre = 247, post, post_post
};

/* srefl = 250, erefl = 251 used for `right-to-left' languages in TeX-XeT */
/* need for these was later removed in TeX--XeT */

#define MAXFONTS 512

#define MAXNAME 256

#define MAXSTACK 128

#define MAXLINE 256

unsigned long fc[MAXFONTS], fs[MAXFONTS], fd[MAXFONTS];

char *fontnames[MAXFONTS];

char line[MAXLINE];


/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

FILE *errout=stdout;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

long h; 		/* horizontal position */
long v; 		/* vertical position */

long w;  		/* horizontal spacing */
long x;  		/* horizontal spacing */
long y;  		/* vertical spacing */
long z;  		/* vertical spacing */

long hstack[MAXSTACK];
long vstack[MAXSTACK];

long wstack[MAXSTACK];
long xstack[MAXSTACK];
long ystack[MAXSTACK];
long zstack[MAXSTACK];


long counter[10];

int stinx;					/* stack index */

int pagenumber;

int skipflag=0;

long ff;

int finish;

int postpost;

int usepts=1;

int roundpts=1;

int truedimen=1;

unsigned long num, den, mag;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

unsigned long checkdefault = 0x59265920;	/* default signature */

/* Takes first six letters of encoding vector name and compresses */
/* into four bytes using base 40 coding */ /* 40^6 < 2^32 */
/* Treats lower case and upper case the same, ignores non alphanumerics */
/* Except handles -, &, and _ */

unsigned long codefourty(char *codingvector) {
	unsigned long result=0;
	int c, k;
	char *s=codingvector;

/*	printf("Given coding vector %s\n", codingvector); */
	if (*codingvector == '\0') {
		codingvector = "native";		/* if not specified ... */
		return checkdefault;			/* use default signature */
	}
	for (k = 0; k < 6; k++) {
		for (;;) {
			c = *s++;
			if (c >= 'A' && c <= 'Z') c = c - 'A';
			else if (c >= 'a' && c <= 'z') c = c - 'a';
			else if (c >= '0' && c <= '9') c = (c - '0') + ('Z' - 'A') + 1;
			else if (c == '-') c = 36;
			else if (c == '&') c = 37;
			else if (c == '_') c = 38;
			else c = 39;					/* none of the above */
/*			else continue; */				/* not alphanumeric character */
/*			result = result * 36 + c; */
			result = result * 40 + c;
			break;
		}
	}
/*	printf("Computed CheckSum %08lx\n", result); */
	return result;
}

int decodefourty(unsigned long checksum, char *codingvector) {
	int c;
	int k;
/*	char codingvector[6+1]; */

/*	if (checksum == checkdefault) */
	if (checksum == 0) {
//		strcpy(codingvector, "unknown");
		strcpy(codingvector, "unknwn");
		return 1;
	}
	else if ((checksum >> 8) == (checkdefault >> 8)) {	/* last byte random */
		strcpy (codingvector,  "native");		/* if not specified ... */
		return 1;								/* no info available */
	}
	else {
		for (k = 0; k < 6; k++) {
			c = (int) (checksum % 40);
			checksum = checksum / 40;
			if (c <= 'z' - 'a' ) c = c + 'a';
			else if (c < 36) c = (c + '0') - ('z' - 'a') - 1;
			else if (c == 36) c = '-';
			else if (c == 37) c = '&';
			else if (c == 38) c = '_';
			else c = '.';				/* unknown */
			codingvector[5-k] = (char) c;
		}
		codingvector[6] = '\0';
	}
/*	printf("Reconstructed vector %s\n", codingvector); */
	return 0;					/* encoding info returned in codingvector */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

double round (double z) {
	if (z < 0) return -round(-z);
	return (double) ((long) (z * 500.0 + 0.5)) / 500.0;
}

double upts (unsigned long sc) {
/*	if (truedimen) return (((double) sc / 65536.0) * (mag / 1000.0));
	else return ((double) sc / 65536.0); */
	double lsc;
	lsc = (double) sc * ((double) num / den) * (72.27 / 254000.0);	/* 99/Apr/18 */
	if (truedimen) return lsc * (double) mag / 1000.0;
	else return lsc;
}

double spts (long sc) {
/*	if (truedimen) return (((double) sc / 65536.0) * (mag / 1000.0));
	else return ((double) sc / 65536.0); */
	double lsc;
	lsc = (double) sc * ((double) num / den) * (72.27 / 254000.0);	/* 99/Apr/18 */
	if (truedimen) return lsc * (double) mag / 1000.0;
	else return lsc;
}

int ushow (unsigned long x) {
	if (usepts) {
		double pts = upts(x);
		if (roundpts) pts = round(pts);
//		printf("%lgpt ", pts);
		sprintf(line, "%lgpt", pts);
	}
	else {
//		printf("%lu ", x);
		sprintf(line, "%lu", x);
	}
	printf(line);
	return strlen(line);
}

int sshow (long x) {
	if (usepts) {
		double pts = spts(x);
		if (roundpts) pts = round(pts);
//		printf("%lgpt ", pts);
		sprintf(line, "%lgpt", pts);
	}
	else {
//		printf("%ld ", x);
		sprintf(line, "%ld", x);
	}
	printf(line);
	return strlen(line);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* we don't have to worry about sign extension here - no need for short int */

unsigned int ureadone (FILE *input) {
	return getc(input);
}

unsigned int ureadtwo (FILE *input) {
	return (getc(input) << 8) | getc(input); 
}

unsigned long ureadthree (FILE *input) {
	int c, d, e;
/*	unsigned int c, d, e; */
	c = getc(input);	d = getc(input);	e = getc(input);
	return ((((unsigned long) c << 8) | d) << 8) | e;
}

unsigned long ureadfour (FILE *input) {
	int c, d, e, f;
	c = getc(input);	d = getc(input);
	e = getc(input);    f = getc(input);
	return ((((((unsigned long) c << 8) | (unsigned long) d) << 8) | e) << 8) | f;
}

/* we do have to worry about sign extension here - use short int if needed */

int sreadone (FILE *input) {
	int c;
	c = getc(input);
	if (c > 127) return (c - 256);
	else return c;
}

int sreadtwo (FILE *input) {
	int c, d;
	c = getc(input);	d = getc(input);
	if (c > 127) c = c - 256;
	return c << 8 | d;
}

long sreadthree (FILE *input) {
	int c, d, e;
	c = getc(input);	d = getc(input);	e = getc(input);
	if (c > 127) c = c - 256; 
	return ((((long) c << 8) | d) << 8) | e;
}

long sreadfour (FILE *input) {
	int c, d, e, f;
	c = getc(input);	d = getc(input);
	e = getc(input);	f = getc(input);
	return ((((((long) c << 8) | (long) d) << 8) | e) << 8) | f;
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void pushstack (void) {
	if (stinx >= MAXSTACK) {
		fprintf(errout, "\nSTACK OVERFLOW");
		exit(1);
	}
	hstack[stinx] = h;
	vstack[stinx] = v;
	wstack[stinx] = w;
	xstack[stinx] = x;
	ystack[stinx] = y;
	zstack[stinx] = z;
	stinx++;
}

void popstack (void) {
	stinx--;
	if (stinx < 0) {
		fprintf(errout, "\nSTACK UNDERFLOW");
		exit(1);
	}
	h = hstack[stinx];
	v = vstack[stinx];
	w = wstack[stinx];
	x = xstack[stinx];
	y = ystack[stinx];
	z = zstack[stinx];
}

void do_push(FILE *input) {
	int c;
/*	push (h, v, w, x, y, z) on stack */
	pushstack();
	printf("\npush ");
	while ((c = getc(input)) == (int) push) {
		pushstack();
		printf("push ");
	}
	(void) ungetc(c, input);
}

void do_pop(FILE *input) {
	int c;
	popstack();
	printf("\npop ");
	while ((c = getc(input)) == (int) pop) {
		popstack();
		printf("pop ");
	}
	(void) ungetc(c, input);
/*	pop (h, v, w, x, y, z) off stack */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */


void do_set1(FILE *input) { /* new version */
	unsigned int c;
	c = ureadone(input);
	printf("\nset1: %u ", c);
/* set character c and increase h by width of character */
/* used (normally only) for characters in range 128 to 255 */
}	

void do_set2(FILE *input) { /* NOT REALLY NEEDED ! */
	unsigned int c;
	c = ureadtwo(input);
	printf("\nset2: %u ", c);
}

void do_set3(FILE *input) { /* NOT REALLY NEEDED ! */
	unsigned long c;
	c = ureadthree(input);
	printf("\nset3: %lu ", c);
}

void do_set4(FILE *input) { /* NOT REALLY NEEDED ! */
	unsigned long c;
	c = ureadfour(input);
	printf("\nset4: %lu ", c);
}

/* set character c and DO NOT increase h by width of character */

void do_put1(FILE *input) {	/* rewritten 1995/June/30 */
	unsigned int c;
	c = ureadone(input);
	printf("\nput1: %u ", c);
}

void do_put2(FILE *input) { /* NOT NEEDED */
	unsigned int c; 
	c = ureadtwo(input); 
	printf("\nput2: %u ", c);
}

void do_put3(FILE *input) { /* NOT NEEDED */
	unsigned long c; 
	c = ureadthree(input); 
	printf("\nput3: %lu ", c);
}

void do_put4(FILE *input) { 
	unsigned long c; 
	c = ureadfour(input); 
	printf("\nput4: %lu ", c);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void do_set_rule (FILE *input) {
	long a, b;										/* height, width */
	a = sreadfour(input);
	b = sreadfour(input);
	printf("\nsetrule: ");
/*	printf("A %ld B %ld  ", a, b); */	/* DEBUGGING ONLY */
	(void) sshow(a);
	putc(' ', stdout);
	(void) sshow(b);
	putc(' ', stdout);
}

void do_put_rule (FILE *input) {
	long a, b; 
	a = sreadfour(input);
	b = sreadfour(input);
	printf("\nputrule: ");
/*	printf("A %ld B %ld  ", a, b); */	/* DEBUGGING ONLY */
	(void) sshow(a);
	putc(' ', stdout);
	(void) sshow(b);
	putc(' ', stdout);
}

void showcounters(void) { /* write TeX /counter's */
	int k;
	int kmax=0;
	printf("[");
	printf("%ld ", counter[0]);	 /* *always* write first one */
	for (k = 10-1; k > 0; k--) {			 /* 1996/Mar/2 */
		if (counter[k] != 0) {			
			kmax = k;
			break;
		}
	}
	for (k = 1; k <= kmax; k++) {	 /* write others if non-zero */
		printf("%ld ", counter[k]);
	}
	printf("]");
}

void do_bop(FILE *input) { /* beginning of page */
	int k;
	long present, previous;

	present = ftell(input)-1;
	pagenumber++;				/* increment page counter */
	stinx = 0;					/* reset stack counter */
	h = 0; v = 0; w = 0; x = 0; y = 0; z = 0;
	ff = -1;					/* undefined font */
/*	reset_stack();		*/		/* empty the stack */
	for (k = 0; k < 10; k++) counter[k] = sreadfour(input); 
	previous = sreadfour(input);
	printf("\nbop (%ld) ", present);
	printf("prev: %ld ", previous);
	showcounters();
	putc('\n', stdout);
}

void do_eop(FILE *input) { /* end of page */
	int c;

	putc('\n', stdout);
 	fputs("\neop ", stdout);	/* 1995/Mar/25 */
	fputs("% ", stdout);		/* 1992/July/18 */ /* eop */
	showcounters(); 
	putc('\n', stdout);
	if (stinx != 0) {
		fprintf(errout, "\nStack not empty at eop (%d)", stinx);
	}
	c = getc(input);
	ungetc(c, input);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void do_right1(FILE *input) { /* rare */
	int b;
	b = sreadone(input);
	printf("\nright1: ");
	(void) sshow(b);
	putc(' ', stdout);
	h = h + b; 
}

void do_right2(FILE *input) {
	int b;
	b = sreadtwo(input);
	printf("\nright2: ");
	(void) sshow(b);
	putc(' ', stdout);
	h = h + b; 
} 

void do_right3(FILE *input) {
	long b;
	b = sreadthree(input);
	printf("\nright3: ");
	(void) sshow(b);
	putc(' ', stdout);
	h = h + b; 
} 

void do_right4(FILE *input) {
	long b;
	b = sreadfour(input);
	printf("\nright4: ");
	(void) sshow(b);
	putc(' ', stdout);
	h = h + b; 
} 

void do_w0(void) {
	fputs("\nwright ", stdout);	/* wright */	/* 1992/Dec/31 */
	printf("    ");
	printf("(");
	(void) sshow(w);
	printf(") ");
	h = h + w;
}

void do_w1(FILE *input) { /* rare */
/*	int w; */
	w = sreadone(input);
	printf("\nwsetright1: "); /* wsetright */
	(void) sshow(w);
	putc(' ', stdout);
	h = h + w;
}

void do_w2(FILE *input) {
/*	int w; */
	w = sreadtwo(input);
	printf("\nwsetright2: "); /* wsetright */
	(void) sshow(w);
	putc(' ', stdout);
	h = h + w; 
} 

void do_w3(FILE *input) {
/*	long w; */
	w = sreadthree(input);
	printf("\nwsetright3: "); /* wsetright */
	(void) sshow(w);
	putc(' ', stdout);
	h = h + w; 
} 

void do_w4(FILE *input) {
/*	long w; */
	w = sreadfour(input);
	printf("\nwsetright4: "); /* wsetright */
	(void) sshow(w);
	putc(' ', stdout);
	h = h + w; 
} 

void do_x0(void) {
	fputs("\nxright ", stdout); /* xright */	/* 1992/Dec/31 */
	printf("    ");
	printf("(");
	(void) sshow(x);
	printf(") ");
	h = h + x; 
}

void do_x1(FILE *input) { /* rare */
/*	int x; */
	x = sreadone(input);
	printf("\nxsetright1: "); /*  xsetright */
	(void) sshow(x);
	putc(' ', stdout);
	h = h + x; 
}

void do_x2(FILE *input) {
/*	int x; */
	x = sreadtwo(input);
	printf("\nxsetright2: "); /* xsetright */
	(void) sshow(x);
	putc(' ', stdout);
	h = h + x; 
} 

void do_x3(FILE *input) {
/*	long x; */
	x = sreadthree(input);
	printf("\nxsetright3: "); /* xsetright */
	(void) sshow(x);
	putc(' ', stdout);
	h = h + x;

}

void do_x4(FILE *input) {
/*	long x; */
	x = sreadfour(input);
	printf("\nxsetright4: "); /* xsetright */
	(void) sshow(x);
	putc(' ', stdout);
	h = h + x;
}

void do_down1(FILE *input) { /* rare */
	int a;
	a = sreadone(input);
	printf("\ndown1: "); /* down */
	(void) sshow(a);
	putc(' ', stdout);
	v = v + a; 
}

void do_down2(FILE *input) { /* rare */
	int a;
	a = sreadtwo(input);
	printf("\ndown2: "); /* down */
	(void) sshow(a);
	putc(' ', stdout);
	v = v + a; 
} 

void do_down3(FILE *input) {
	long a;
	a = sreadthree(input);
	printf("\ndown3: "); /* down */
	(void) sshow(a);
	putc(' ', stdout);
	v = v + a;
}

void do_down4(FILE *input) {
	long a;
	a = sreadfour(input);
	printf("\ndown4: "); /* down */
	(void) sshow(a);
	putc(' ', stdout);
	v = v + a; 
} 

void do_y0(void) {
	fputs("\nydown ", stdout); /* ydown */	/* 1992/Dec/31 */
	printf("(");
	(void) sshow(y);
	printf(") ");
	v = v + y; 
}

void do_y1(FILE *input) { /* rare */
/*	int y;	*/
	y = sreadone(input);
	printf("\nysetdown1: "); /* ysetdown */
	(void) sshow(y);
	putc(' ', stdout);
	v = v + y; 
}

void do_y2(FILE *input) {
/*	int y; */
	y = sreadtwo(input);
	printf("\nysetdown2: "); /*  ysetdown */
	(void) sshow(y);
	putc(' ', stdout);
	v = v + y; 
} 


void do_y3(FILE *input) {
/*	long y; */
	y = sreadthree(input);
	printf("\nysetdown3: "); /*  ysetdown */
	(void) sshow(y);
	putc(' ', stdout);
	v = v + y;
} 

void do_y4(FILE *input) { /* not used */
/*	long y; */
	y = sreadfour(input);
	printf("\nysetdown4 "); /*  ysetdown */
	(void) sshow(y);
	putc(' ', stdout);
	v = v + y; 
} 

void do_z0(void) {
	fputs("\nzdown ", stdout); /* zdown */	/* 1992/Dec/31 */
	printf("(");
	(void) sshow(z);
	printf(") ");
	v = v + z;
}

void do_z1(FILE *input) {  /* rare */
/*	int z; */
	z = sreadone(input);
	printf("\nzsetdown1: "); /* zsetdown */
	(void) sshow(z);
	putc(' ', stdout);
	v = v + z; 
}

void do_z2(FILE *input) {
/*	int z; */
	z = sreadtwo(input);
	printf("\nzsetdown2: "); /* zsetdown */
	(void) sshow(z);
	putc(' ', stdout);
	v = v + z;
} 

void do_z3(FILE *input) {
/*	long z; */
	z = sreadthree(input);
	printf("\nzsetdown3: "); /* zsetdown */
	(void) sshow(z);
	putc(' ', stdout);
	v = v + z;
} 

void do_z4(FILE *input) {
/*	long z; */
	z = sreadfour(input);
	printf("\nzsetdown4: "); /* zsetdown */
	(void) sshow(z);
	putc(' ', stdout);
	v = v + z; 
} 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showfont (unsigned long k) {
	printf("(%s @ ", fontnames[k]);
	(void) ushow(fs[k]);
	printf(") ");
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void do_fnt1(FILE *input) { /* switch fonts */
	unsigned int k;
	k = ureadone(input);
	printf("\nfnt1: %d", k);
	showfont(k);
	ff = (long) k;
}

void do_fnt2(FILE *input) { /* switch fonts */
	unsigned int k;
	k = ureadtwo(input);
	printf("\nfnt2: %d", k);
	showfont(k);
	ff = (long) k;
}

void do_fnt3(FILE *input) { /* switch fonts */
	unsigned long k;
	k = ureadthree(input); 
	printf("\nfnt3: %ld", k);
	showfont(k);
	ff = (long) k;
}

void do_fnt4(FILE *input) { /* switch fonts */
	long k;
	k = sreadfour(input);
	if (k < 0) fprintf(errout, "\nFont code %ld < 0\n", k);
	printf("\nfnt4: %ld", k);
	showfont(k);
	ff = (long) k;
}

void readspecial (FILE *input, unsigned long n) {
	int c;
	unsigned long k;
	putc('"', stdout);
	for (k = 0; k < n; k++) {
		c = getc(input);
		putc(c, stdout);
	}
	putc('"', stdout);
	putc(' ', stdout);
/*	putc('\n', stdout); */
}

void do_xxxi(FILE *input, unsigned long n) {
	unsigned long k;
	if (skipflag != 0)
		for(k = 0; k < n; k++) (void) getc(input);
	else readspecial(input, n);
}

void do_xxx1(FILE *input) { /* for /special */
	unsigned int n;
	n = ureadone(input);
	printf("\nXXX1 (%u): ", n);
	do_xxxi(input, n);
}

void do_xxx2(FILE *input) { /* for /special */
	unsigned int n;
	n = ureadtwo(input);
	printf("\nXXX2 (%u): ", n);
	do_xxxi(input, n);
}


void do_xxx3(FILE *input) { 
	unsigned long n;
	n = ureadthree(input);
	printf("\nXXX3 (%lu): ", n);
	do_xxxi(input, n);
}

void do_xxx4(FILE *input) { 
	unsigned long n;
	n = ureadfour(input);
	printf("\nXXX4 (%lu): ", n);
	do_xxxi(input, n);
}

/* need to do this even if skipping pages */

void fnt_def (FILE *input, unsigned long k) {
	unsigned int na, nl, i, nlen;
/*	unsigned long fc, fs, fd; */
	int c;
/*	int newfont=1; */
	char name[MAXNAME];
	char *s;

/*  checksum, at size, and design size */
	fc[k] = ureadfour(input); 
	fs[k] = ureadfour(input); 
	fd[k] = ureadfour(input); 
	printf("check: %08lX ", fc[k]);
	decodefourty(fc[k], name);			/* 99/Jan/7 */
	printf("(%s..) ", name);
	printf("size: ");
	nlen = ushow(fs[k]);
//	printf(" %d", nlen);		// debugging only
	while (nlen++ < 8) 	putc(' ', stdout);
	putc(' ', stdout);
	printf("design: ");
	nlen = ushow(fd[k]);
//	printf(" %d", nlen);		// debugging only
	while (nlen++ < 8) 	putc(' ', stdout);
	putc(' ', stdout);
/*	font name */
 	na = ureadone(input); 
	nl = ureadone(input); 
//	printf("(%d,%d) ", na, nl);			// debugging only
	s = name;
	for (i = 0; i < na; i++) {
		c = getc(input);
		putc(c, stdout);
		*s++ = (char) c;
	}
	for (i = 0; i < nl; i++) {
		c = getc(input);
		putc(c, stdout);
		*s++ = (char) c;
	}
	*s = '\0';
	fontnames[k] = _strdup(name);
}

void do_fnt_def1 (FILE *input) { /* define font */
	unsigned int k;

	k = ureadone(input); 
	printf("\nfnt_def1 %3u ", k);
	fnt_def(input, k);
}

void do_fnt_def2 (FILE *input) { /* define font */
	unsigned int k;

	k = ureadtwo(input);
	printf("\nfnt_def2 %3u ", k);
	fnt_def(input, k);
}

void do_fnt_def3 (FILE *input) { /* define font */
	unsigned long k;

	k = ureadtwo(input);
	printf("\nfnt_def3 %3lu ", k);
	fnt_def(input, k);
}

void do_fnt_def4 (FILE *input) { /* define font */
	long k;

	k = sreadfour(input);
	if (k < 0) {
		fprintf(errout, "\nFont code %ld < 0", k);
	}
	printf("\nfnt_def4 %3ld ", k);
	fnt_def(input, k);
}

#define ID_BYTE 2

/* need to do this even if skipping pages */

void do_pre (FILE *input) { /* doesn't do output */
	unsigned int i, k, j;
	int c;

	i = ureadone(input);	/* DVI ID byte */
	if (i != ID_BYTE) {
		fprintf(errout, "\nFile is DVI version %d", i);
	} 
	num = ureadfour(input); 
	den = ureadfour(input);
	mag = ureadfour(input);
	printf("pre id: %u num: %ld den: %ld mag: %ld ", i, num, den, mag);
	k = ureadone(input);
	printf("\ncomment(%u): ", k);
	for (j = 0; j < k; j++) {
		c = getc(input);
		putc(c, stdout);
	}
	putc('\n', stdout); 
} 

/* need to do this even if skipping pages */

void do_post (FILE *input) { /* doesn't do output */
	int nlen;
	long previous, present;
	unsigned long num, den, mag;
	unsigned long l, u;
	unsigned int s, t;

	present = ftell(input) - 1;
	previous = sreadfour(input);	/* was ureadfour ... */
	num = ureadfour(input);
	den = ureadfour(input);
	mag = ureadfour(input);
	printf("\npost ");
	printf("(%ld) ", present);
	printf("num: %lu den: %lu mag: %lu ", num, den, mag);
	printf("prev: %ld ", previous);
	l = ureadfour(input);
	u = ureadfour(input); 
/*	printf("%lu %lu ", l, u); */
	printf("\nheight+depth: ");
	nlen = ushow(l);
	while (nlen++ < 8) 	putc(' ', stdout);
	putc(' ', stdout);
	printf("width: ");
	nlen = ushow(u);
	while (nlen++ < 8) 	putc(' ', stdout);
	putc(' ', stdout);
	s = ureadtwo(input);	
	t = ureadtwo(input);
	printf("stack: %u pages: %u ", s, t);
	putc('\n', stdout);
}

void do_post_post (FILE *input) { /* only in reverse */
	unsigned long q;
	unsigned int i; 
 	q = ureadfour(input);		/* backward pointer to post */
	i = ureadone(input); 		/* DVI ID byte */
/*	followed by at least four 223's */
	putc('\n', stdout);
	printf("\npost_post prev: %lu id: %d ", q, i);
	postpost = 1;
}

void normalchar (int c) {
	if (c < 32 || c == 127) printf("C-%c", c + 64);
	else putc(c, stdout);
}

/* main entry point to this part of the program */

void scandvifile (FILE *input) {
	int c, k;
	long filptr; 

	pagenumber = 0;			/* value from earlier scan already used */

	finish = 0;
	stinx = 0;
	postpost = 0;

	for(;;) {
		c = getc(input);
		if (c == EOF) {
			fputs("\nEOF", errout);
			finish = -1;
			break;
		}
		if (c < 128) {
			normalchar(c);
/* set character in current font and advance h by width of character */
		}
		else if (c >= 171 && c <= 234) { /*	switch to font (c - 171) */
			if (postpost) {
				if (c != 223) printf("\nERROR post_post");
				while (c == 223) {
					printf("223 ");
					c = getc(input);
				}
				ungetc(c, input);
			}
			else {
				k = (c - 171);
				printf("\nfnt %d ", k);
				showfont(k);
			}
		}
		else {
			switch(c) {
				case set1: do_set1(input); break;
				case set2: do_set2(input); break;  /* silly */
				case set3: do_set3(input); break;  /* silly */
				case set4: do_set4(input); break;  /* silly */
				case set_rule: do_set_rule(input); break;
				case put1: do_put1(input); break ;
				case put2: do_put2(input); break;	/* silly */
				case put3: do_put3(input); break;	/* silly */
				case put4: do_put4(input); break;	/* silly */
				case put_rule: do_put_rule(input); break;	
				case nop: break;				/* easy, do nothing ! */
				case bop: do_bop(input); break;
				case eop: do_eop(input); break;
				case push: do_push(input); break;
				case pop: do_pop(input); break;
				case right1: do_right1(input); break;
				case right2: do_right2(input); break;  
				case right3: do_right3(input); break; 
				case right4: do_right4(input); break; 
				case w0: do_w0(); break;
				case w1: do_w1(input); break;
				case w2: do_w2(input); break; 
				case w3: do_w3(input); break; 
				case w4: do_w4(input); break;		/* not used ? */
				case x0: do_x0(); break;
				case x1: do_x1(input); break;
				case x2: do_x2(input); break; 
				case x3: do_x3(input); break; 
				case x4: do_x4(input); break;		/* not used ? */
				case down1: do_down1(input); break;
 				case down2: do_down2(input); break; 
				case down3: do_down3(input); break; 
				case down4: do_down4(input); break; 
				case y0: do_y0(); break;
				case y1: do_y1(input); break;
				case y2: do_y2(input); break; 
				case y3: do_y3(input); break; 
				case y4: do_y4(input); break;		/* not used ? */
				case z0: do_z0(); break;
				case z1: do_z1(input); break;
				case z2: do_z2(input); break; 
				case z3: do_z3(input); break; 
				case z4: do_z4(input); break;		/* not used ? */
				case fnt1: do_fnt1(input); break;
				case fnt2: do_fnt2(input); break;	/* silly */
				case fnt3: do_fnt3(input); break;	/* silly */
				case fnt4: do_fnt4(input); break;	/* silly */
				case xxx1: do_xxx1(input); break;
				case xxx2: do_xxx2(input); break;	/* not used ? */
				case xxx3: do_xxx3(input); break;	/* not used ? */
				case xxx4: do_xxx4(input); break; 
				case fnt_def1: do_fnt_def1(input); break;
				case fnt_def2: do_fnt_def2(input); break; /* silly */
				case fnt_def3: do_fnt_def3(input); break; /* silly */
				case fnt_def4: do_fnt_def4(input); break; /* silly */
				case post: do_post(input); break;
				case pre: do_pre(input); break;
				case post_post: do_post_post(input); break;
	
				default: {			/* includes EOF ? */
					printf("INVALID CODE %d", c);
					finish = -1;	/* ??? */
/* this should normally not happen: */
					filptr = ftell(input);
					if (filptr > 0)				/* 95/Dec/10 */
						fprintf(errout, " at byte %ld in DVI file", filptr-1);
				}
						 break;
			} /* end of switch(c) */
		}
		if (finish != 0) break;
	}
	putc('\n', stdout);
}

void extension (char *fname, char *str) { /* supply extension if none */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten (char *fname, char *str) { /* change extension if present */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

int main (int argc, char *argv[]){
	char infilename[FILENAME_MAX];
	FILE *input;

	if (argc < 2) exit(1);
	printf("DVIDUMP 1.1 Copyright (C) 2000 Y&Y, Inc. http://www.YandY.com\n");
	printf("\n");
	strcpy(infilename, argv[1]);
	extension(infilename, "dvi");
	if ((input = fopen(infilename, "rb")) == NULL) {
		perror(infilename);
		exit(1);
	}
	scandvifile(input);
	fclose(input);
	return 0;
}

