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

int verboseflag=0;
int traceflag=0;
int debugflag=0;

int leadingzero=-1;		/* assume (nlen+1)/8 leading zeros (?) */
int nocomplement=-1;	/* consider only less than half full patterns */
int halffullonly=0;		/* look only at half full patterns (len-1)/2*/
int singlebitonly=0;	/* look only at patterns with single overlap */
int skipoutearly=0;		/* stop after finding first pattern */
int genhalfflag=0;		/* generate only half-full patterns */

#define MAXPATTERN 256

typedef unsigned __int64 UINT64;

UINT64 patterns[MAXPATTERN];

UINT64 singlebits[64];			/* shifted single bit (1 << k) */

int patterninx;

unsigned int bits[65536];		/* lookup table for number of bits */

unsigned int reversed[65536];	/* lookup table for reversed bits */ 

/**********************************************************************/

void print64 (UINT64 res) {
	unsigned long k1, k2;
	k2 = (unsigned long) (res & 0XFFFFFFFF);
	k1 = (unsigned long) ((res >> 32) & 0XFFFFFFFF);
	if (k1 == 0) printf("%8X", k2);
	else printf("%8X%08X", k1, k2);
}

void print64dec (UINT64 res) {
	int c;
	if (res == 0) return;
	c = (int) (res % 10);
	print64dec(res / 10);
	putc('0'+c, stdout);
}

void printbits (UINT64 res, int nlen) {
	unsigned long k1, k2;
	unsigned long topbit;
	int n;
	k2 = (unsigned long) (res & 0XFFFFFFFF);
	k1 = (unsigned long) ((res >> 32) & 0XFFFFFFFF);
	if (nlen > 32) {
		topbit = 1 << (nlen-32-1);
		for (n = nlen; n > 32; n--) {
			if (topbit & k1) putc('1', stdout);
			else putc('0', stdout);
			k1 = k1 << 1;
		}
		nlen = 32;
	}
	topbit = 1 << (nlen-1);
	for (n = nlen; n > 0; n--) {
		if (topbit & k2) putc('1', stdout);
		else putc('0', stdout);
		k2 = k2 << 1;
	}
}

void print64CR (UINT64 res) {
	print64 (res);
	putc('\n', stdout);
}

/**********************************************************************/

/* The slow way, used to set up the lookup table */

int countbits(UINT64 res) {			/* count number of bits */
	int nbits=0;
	if (traceflag) {
		printf("CountBits ");
		print64CR(res);
	}
	while (res != 0) {
		res = res & ~(res & (~res + 1));	/* knock out rightmost */
		nbits++;							/* count the bits */
	}
	return nbits;
}


int countbitsslow(int res) {	/* count number of bits 16 bit */
	int nbits=0;
	if (traceflag) {
		printf("CountBits ");
		printf("%ld", res);
	}
	while (res != 0) {
		res = res & ~(res & (~res + 1));	/* knock out rightmost */
		nbits++;							/* count the bits */
	}
	return nbits;
}

/* following no longer used - inserted inline for speed */

int countbitsfast(UINT64 res) {	/* uses table set up by countbits */
	unsigned long k1, k2;
	unsigned int m1, m2, m3, m4;
/*	print64 (res); */
	k2 = (unsigned long) (res & 0XFFFFFFFF);
	m4 = (unsigned int) (k2 & 0XFFFF);
	m3 = (unsigned int) ((k2 >> 16) & 0XFFFF);
	k1 = (unsigned long) ((res >> 32) & 0XFFFFFFFF);
	if (k1 == 0) {
		return bits[m3] + bits[m4];
	}
	else {
		m2 = (unsigned int) (k1 & 0XFFFF);
		m1 = (unsigned int) ((k1 >> 16) & 0XFFFF);
/*		printf("\t%X %X %X %X bits %d\n", m1, m2, m3, m4,
		   bits[m1] + bits[m2] + bits[m3] + bits[m4]); */
		return bits[m1] + bits[m2] + bits[m3] + bits[m4];
	}
}

UINT64 reverse (UINT64 pattern, int nlen) {	/* reverse pattern */
	UINT64 rever=0;
	int k;

	for (k = 0; k < nlen; k++) {
		if (pattern & 1) rever = (rever << 1) | 1;
		else rever = rever << 1;
		pattern = pattern >> 1;
	}
	return rever;
}

int reverseslow (int pattern) {	/* reverse pattern 16 bit */
	int rever=0;
	int k;

	for (k = 0; k < 16; k++) {
		if (pattern & 1) rever = (rever << 1) | 1;
		else rever = rever << 1;
		pattern = pattern >> 1;
	}
	return rever;
}

/* not clear this is really much faster ... also buggy sign bit >> */

UINT64 reversefast (UINT64 pattern, int nlen) {
	unsigned long k1, k2;
	unsigned int m1, m2, m3, m4;
	UINT64 rev;

/*	print64 (res); */
	k2 = (unsigned long) (pattern & 0XFFFFFFFF);
	m4 = (unsigned int) (k2 & 0XFFFF);
	m3 = (unsigned int) ((k2 >> 16) & 0XFFFF);
	k1 = (unsigned long) ((pattern >> 32) & 0XFFFFFFFF);
	m2 = (unsigned int) (k1 & 0XFFFF);
	m1 = (unsigned int) ((k1 >> 16) & 0XFFFF);
	k2 = (reversed[m2] << 16) | reversed[m1];
	k1 = (reversed[m4] << 16) | reversed[m3];
	printbits(pattern, 64);
	putc(' ', stdout);
	rev = ((UINT64) k1 << 32) | k2;
	printbits(rev, 64);
	putc(' ', stdout);
	rev = rev >> (64 - nlen);
	printbits(rev, 64);
	putc('\n', stdout);
	return rev;
}

/* following no longer used - inserted inline for speed */

UINT64 rotate (UINT64 pattern, int nlen) {	/* rotate pattern */
	UINT64 topbit;
/*	topbit = (UINT64) 1 << (nlen-1); */
	topbit = singlebits[nlen-1];
	if (pattern & topbit) return ((pattern & ~topbit) << 1) | 1;
	else return (pattern << 1);
}

UINT64 lowest (UINT64 pattern, int nlen) {	/* find lowest equiv pattern */
	UINT64 low, topbit;
/*	UINT64 carry = 1 << nlen; */
	int k;
/*	int nbits1, nbits2; */

	if (pattern == 0) return 0;		/* should never happen */

/*	pick the one which represents the lowest number */
/*	topbit = (UINT64) 1 << (nlen-1); */
	topbit = singlebits[nlen-1];
	low = pattern;
	for (k = 1; k < nlen; k++) {
/*		if (pattern & carry) pattern = (pattern & ~carry) | 1; */
		if (pattern & topbit)
			pattern = ((pattern & ~topbit) << 1) | 1;
		else pattern = pattern << 1;
		if (debugflag) {
			printf("Shift %d ", k);
			print64(pattern);
			putc('\n', stdout);
		}
		if (pattern < low) low = pattern;
	}
	if (debugflag) { 
		printf("Pattern ");
		print64(pattern);
		printf(" Low ");
		print64CR(low);
	} 
	return low;
}

int already (UINT64 pattern) {	/* see if already in table */
	int k;
	if (traceflag) {
		printf("Already in table? ");
		print64CR(pattern);
	}
	for (k = 0; k < patterninx; k++) {
		if (patterns[k] == pattern) return k;
	}
	return -1;
}

/* returns height of sidelobe or -1 if sidelobe is not even */

int checkit (UINT64 pattern, int nlen, int nbitszero) {
	UINT64 master, res, topbit;
	int nshift, nbits, nbitsone, npattern;
	unsigned long k1, k2;
	unsigned int m1, m2, m3, m4;

#ifdef IGNORED
	if (nlen > 63) {
		printf("Pattern too long (%d > 63)\n", nlen);
		return -1;
	}
	if (nlen < 3) {
		printf("Pattern too short (%d < 3)\n", nlen);
		return -1;
	}
#endif

/*	npattern = countbitsfast (pattern); */
/*	k2 = (unsigned long) (pattern & 0XFFFFFFFF);
	k1 = (unsigned long) ((pattern >> 32) & 0XFFFFFFFF);
	m4 = (unsigned int) (k2 & 0XFFFF);
	m3 = (unsigned int) ((k2 >> 16) & 0XFFFF);
	m2 = (unsigned int) (k1 & 0XFFFF);
	m1 = (unsigned int) ((k1 >> 16) & 0XFFFF);
	npattern = bits[m1] + bits[m2] + bits[m3] + bits[m4]; */

	npattern = nbitszero;

/*	not interested in single bit on or single bit off patterns */
	if (npattern == 1 || npattern == nlen-1) {
		return -1;
	}
	
	master = pattern;			/* remember original pattern */

/*	topbit = (UINT64) 1 << (nlen-1); */
	topbit = singlebits[nlen-1];
/*	pattern = pattern << 1; */
	if (pattern & topbit) pattern = ((pattern & ~topbit) << 1) | 1;
	else pattern = pattern << 1;

	nshift = 1;
	res = master & pattern;				/* and them together */

/*	as reference compute and remember correlation for shift of one */
/*	nbits = countbitsfast (res); */
	k2 = (unsigned long) (res & 0XFFFFFFFF);
	k1 = (unsigned long) ((res >> 32) & 0XFFFFFFFF);
	m4 = (unsigned int) (k2 & 0XFFFF);
	m3 = (unsigned int) ((k2 >> 16) & 0XFFFF);
	m2 = (unsigned int) (k1 & 0XFFFF);
	m1 = (unsigned int) ((k1 >> 16) & 0XFFFF);
	nbits = bits[m1] + bits[m2] + bits[m3] + bits[m4];
	if (singlebitonly) {
		if (nbits != 1) return -1;	
	}
	else if (halffullonly) {	/* half full pattern must follow this */
		if (nbits != (nbitszero-1)/2) return -1;
	}

	nbitsone = nbits;
	if (traceflag) {
		printf("%d\t", nshift);
		print64(master);
		printf(" ");
		print64(pattern);
		printf("%d\n", nbits);
	}
/*	pattern = pattern << 1; */
/*	if (pattern & topbit)pattern = ((pattern & ~topbit) << 1) | 1;
	else pattern = pattern << 1; */

	for (nshift = 2; nshift < nlen; nshift++) {
		if (pattern & topbit)
			pattern = ((pattern & ~topbit) << 1) | 1;
		else pattern = pattern << 1;
		res = master & pattern;				/* and them together */
/* 		nbits = countbitsfast (res); */
		k2 = (unsigned long) (res & 0XFFFFFFFF);
		k1 = (unsigned long) ((res >> 32) & 0XFFFFFFFF);
		m4 = (unsigned int) (k2 & 0XFFFF);
		m3 = (unsigned int) ((k2 >> 16) & 0XFFFF);
		m2 = (unsigned int) (k1 & 0XFFFF);
		m1 = (unsigned int) ((k1 >> 16) & 0XFFFF);
		nbits = bits[m1] + bits[m2] + bits[m3] + bits[m4];

		if (traceflag) {
			printf("%d\t", nshift);
			print64(master);
			printf(" ");
			print64(pattern);
			printf(" %d\n", nbits);
		}
		if (nbits != nbitsone) return -1;	/* sidelobes not flat */
/*		pattern = pattern << 1; */
	}
	return nbitsone;
}

UINT64 low, rever, revlow;

int processhit (UINT64 pattern, int nlen, int nbits, int nbitszero) {
	UINT64 low, revlow;
	low = lowest (pattern, nlen);
	if (debugflag) {
		printf("LOW ");
		print64CR(low);
	}
	if (already(low) >= 0) {	/* already in table ? */
		if (verboseflag) {
			print64(pattern);
			putc(' ', stdout);
			printbits(pattern, nlen);
			putc(' ', stdout);
			printf(" ");
			print64(low);
			putc(' ', stdout);
			printbits(low, nlen);
			putc(' ', stdout);
			printf(" already in table\n");
		}
/*		continue; */
		return 0;
	}
	rever = reverse (pattern, nlen); 
/*	rever = reversefast (pattern, nlen); */
	if (debugflag) {
		printf("REV ");
		print64CR(rever);
	}
	revlow = lowest (rever, nlen);
	if (debugflag) {
		printf("LOW REV ");
		print64CR(revlow);
	}
	if (already(revlow) >= 0) {
		if (verboseflag) {
			print64(pattern);
			printf(" ");
			print64(revlow);
			printf(" already in table\n");
		}
/*		continue; */
		return 0;
	}
	else {
		if (revlow < low) low = revlow;
		if (patterninx >= MAXPATTERN) {
			printf ("Too many patterns to store\n");
			exit(1);
		}
		else patterns[patterninx++] = low;
		printf("%2d HIT:%3d ", nlen, patterninx);
/*		print64(pattern); */
		print64(low);
		printf(" %2d / %2d ", nbits, nbitszero);
		putc(' ', stdout);
/*		printbits(pattern, nlen); */
		printbits(low, nlen);
		putc('\n', stdout);
		fflush(stdout);
		if (skipoutearly) return -1;
	}
	return 0;
}

/* generate the patterns we want to test */

void process(int nlen) {
	UINT64 pattern, oldpattern, high, xorres;
/*	UINT64 low, rev, revlow; */
	int nbits, nbitszero;
#ifdef CHECKCOUNTBITS
	unsigned long k1, k2;
	unsigned int m1, m2, m3, m4;
#endif

	patterninx=0;
/*	high = ((UINT64) 1 << nlen) - 1; */
/*	if (leadingzero) high = ((UINT64) 1 << (nlen - (nlen-1)/4))-1; */
/*	if (leadingzero) high = ((UINT64) 1 << (nlen - ((nlen-1)/4 -1)))-1; */
/*	if (leadingzero) high = ((UINT64) 1 << (nlen - (nlen+1)/8))-1;
	else high = ((UINT64) 1 << nlen)-1;  */
	if (leadingzero) high = singlebits[nlen - (nlen+1)/8]-1;
	else high = singlebits[nlen]-1;		/* nlen < 64 */

	if (verboseflag) {
		printf("nlen %d: 0 - ", nlen);
		print64(high);
		putc(' ', stdout);
		printbits(high, nlen);
		putc('\n', stdout);
	}
	nbitszero = 1;		/* initial value pattern == 1 */
	oldpattern = 1;
	for (pattern = 3; pattern < high; pattern = pattern+2) {
/*		if (pattern & 1) continue; */	/* not needed anymore */
/*		nbits = countbitsfast(pattern); */
#ifdef CHECKCOUNTBITS
/*		compute number of bits inline for speed */
		k2 = (unsigned long) (pattern & 0XFFFFFFFF);
		k1 = (unsigned long) ((pattern >> 32) & 0XFFFFFFFF);
		m4 = (unsigned int) (k2 & 0XFFFF);
		m3 = (unsigned int) ((k2 >> 16) & 0XFFFF);
		m2 = (unsigned int) (k1 & 0XFFFF);
		m1 = (unsigned int) ((k1 >> 16) & 0XFFFF);
		nbits = bits[m1] + bits[m2] + bits[m3] + bits[m4];
#endif
/*		attempt at incremental calculation of number of bits */
		xorres = pattern ^ oldpattern;
		if (xorres == 1*2) nbitszero++;
		else if (xorres == 3*2) ;
		else if (xorres == 7*2) nbitszero--;
		else if (xorres == 15*2) nbitszero -= 2;
		else if (xorres == 31*2) nbitszero -= 3;
		else if (xorres == 63*2) nbitszero -= 4;
		else if (xorres == 127*2) nbitszero -= 5;
		else {
			xorres = xorres >> 1;	/* first shift out trailing zero */
			xorres = xorres >> 2;	/* shift out bottom 11 also */
			while (xorres & 1) {
				nbitszero--;
				xorres = xorres >> 1;
			}
		}
		oldpattern = pattern;
#ifdef CHECKCOUNTBITS
		if (nbitszero != nbits) {
			printf("nbits %d nbitszero %d ", nbits, nbitszero);
			print64(pattern);
			putc(' ', stdout);
			print64CR(oldpattern);
		}
		else nbits = nbitszero;
#endif
		nbits = nbitszero;
		if (nocomplement) {
			if ((nbits > (nlen-1)/2)) continue;
					/* ignore, will meet complement later */
		}
		else if (halffullonly) {
			if (nbits != (nlen-1)/2) continue;
		}

/*		could also check whether lowest equivalent already in table */
		if (traceflag) {
			printf("Trying ");
			print64(pattern);
			putc(' ', stdout);
			printbits(pattern, nlen);
			putc('\n', stdout);
		}
		else if (verboseflag) putc('.', stdout);
/*		nbits = countbitsfast (pattern); */
		nbitszero = nbits;

		nbits = checkit (pattern, nlen, nbitszero);	/* work done here */

		if (nbits > 0) {		/* did it haven flat sidelobes ? */
			if (processhit(pattern, nlen, nbits, nbitszero) == 0) continue;
			else break;
		}
	}
	printf("Found %d distinct pattern%s\n", patterninx,
		  (patterninx == 1) ? "" : "s");
}

int nlenzero, nbitszero;
UINT64 trials;

/* look only for pattern with nbits ones in right most nlen bits */
/* calls itself recursively on right hand subpattern */

void processfixsub (UINT64 pattern, int nlen, int nbits) {
	int m, nbitsnew;
	UINT64 currentbit;
/*	currentbit = (1 << (nlen-1)); */
	currentbit =  singlebits[nlen-1];
	if (traceflag) {
		printf("nlen %2d nbits %2d ", nlen, nbits);
		print64CR(pattern);
	}

/*	if (nbits == 0) {
		nbitsnew = checkit(pattern, nlenzero, nbitszero);
		trials++;
		if (nbitsnew > 0) {
			processhit (pattern, nlenzero, (nbitszero-1)/2, nbitszero);
		}
		return;
	} */

/*	avoid bottom level recursive call */
	if (nbits == 1) {
		for (m = nlen; m > 0; m--) {
			nbitsnew = checkit(pattern | currentbit, nlenzero, nbitszero);
			trials++;
			if (nbitsnew > 0) {
				processhit (pattern | currentbit, nlenzero,
							(nbitszero-1)/2, nbitszero);
			}
			currentbit = currentbit >> 1;
		}
		return;
	}

	for (m = nlen; m >= nbits; m--) {
/*		currentbit = (1 << (m-1))); */
		processfixsub(pattern | currentbit, m-1, nbits-1);
		currentbit = currentbit >> 1;
	}
}

void processfix (int n) {
	int nlen;
	nlenzero = n;
	nbitszero = (n-1)/2;
	patterninx = 0;
	trials = 0;
	if (leadingzero) nlen = (n - (n+1)/8);
	else nlen = n;
	processfixsub((UINT64) 0, nlen, nbitszero);
	printf("Found %d distinct pattern%s\n", patterninx,
		   (patterninx == 1) ? "" : "s");
/*	if (trials < ((UINT64) 1 << 32))
		printf("%lu (decimal) trials\n", (unsigned long) trials);
	else {
		print64(trials);
		printf(" trials\n");
	} */
	print64dec(trials);
	printf(" trials\n");
}

int checkold (UINT64 pattern, int nlen) {
	UINT64 master, res;
	int nshift, nbits;

	if (nlen > 32) {
		printf("Pattern too long (%d > 32)\n", nlen);
		return 0;
	}
	master = pattern | (pattern << nlen);	/* replicate pattern */
	for (nshift = 0; nshift < nlen; nshift++) {
		nbits = 0;
		res = master & pattern;				/* and them together */
		while (res != 0) {					/* count number of bits */
			res = res & ~(res & ((~res) + 1));	/* knock out rightmost */
			nbits++;						/* count the bits */
		}
		printf("%d\t ", nshift);
		print64(master);
		printf(" ");
		print64(pattern);
		printf(" %d\n", nbits);
		pattern = pattern << 1;
	}
	return 0;
}

void showusage (char *progname) {
	printf("\
%s [-v] [-t] [-d] [-0] [-e] [-c] [-h] [-s]\n\
\t-v verbose mode\n\
\t-t trace mode\n\
\t-d debug mode\n\
\t-0 do not assume (n+1)/8 high bits are zeros\n\
\t-e exit early (when first hit found)\n\
\t-c do not ignore complementary patterns\n\
\t-h work only on pattern with (n-1)/2 bits\n\
\t-s work only on patterns with unit sidelobes\n\
\t-f generate only half full patterns to test\n\
", progname);
	exit(1);
}

int main (int argc, char *argv[]) {
	UINT64 pattern;
	int k, nlen;
	int firstarg=1;

/*	print64dec(12345678);
	putc('\n', stdout);
	print64dec(0XFFFFFFFF);
	putc('\n', stdout);
	print64dec(((UINT64)0XFFFFFFFF << 32) | 0XFFFFFFFF);
	putc('\n', stdout); */
/*	pattern = 1;
	for (k = 0; k < 64; k++) {
		print64(pattern);
		putc(' ', stdout);
		print64dec(pattern);
		putc('\n', stdout);
		pattern = pattern << 1;
	} */

	while (firstarg < argc && *(argv[firstarg]) == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag = ~verboseflag;
		if (strcmp(argv[firstarg], "-t") == 0) traceflag = ~traceflag;
		if (strcmp(argv[firstarg], "-d") == 0) debugflag = ~debugflag;
		if (strcmp(argv[firstarg], "-0") == 0) leadingzero = ~leadingzero;
		if (strcmp(argv[firstarg], "-e") == 0) skipoutearly = ~skipoutearly;
		if (strcmp(argv[firstarg], "-c") == 0) nocomplement = ~nocomplement;
		if (strcmp(argv[firstarg], "-h") == 0) halffullonly = ~halffullonly;
		if (strcmp(argv[firstarg], "-s") == 0) singlebitonly = ~singlebitonly;
		if (strcmp(argv[firstarg], "-f") == 0) genhalfflag = ~genhalfflag;
		firstarg++;
	}

	if (traceflag) printf("Setting up tables\n");

	pattern = 1;
	for (k = 0; k < 64; k++) {
		singlebits[k] = pattern;
		pattern = pattern << 1;
	}

/*	for (k = 0; k < 65536; k++)	bits[k] = countbits(k); */
	for (k = 0; k < 65536; k++)	bits[k] = countbitsslow(k);
/*	for (k = 0; k < 65536; k++)	reversed[k] = reverse(k); */
	for (k = 0; k < 65536; k++)	reversed[k] = reverseslow(k);
/*	for (k = 0; k < 64; k++) {
		printf("%5d\t", k);
		printbits (reversed[k], 16);
		putc('\n', stdout);
	} */

	if (traceflag) printf("Finished setting up tables\n");

	if (argc < firstarg+1) {
		showusage(argv[0]);
		exit(1);
	}

	if (argc < firstarg+2) {		/* one arg - use for nlen */
		if (sscanf(argv[firstarg], "%d", &nlen) < 1) {
			printf("Don't understand %s\n", argv[firstarg]);
		}
		if (traceflag) printf("nlen %d\n", nlen);
		if (genhalfflag) processfix (nlen);
		else process (nlen);
		return 0;
	}

	if (sscanf(argv[firstarg], "%X", &pattern) < 1) {
		printf("Don't understand %s\n", argv[firstarg]);
	}
	if (sscanf(argv[firstarg+1], "%d", &nlen) < 1) {
		printf("Don't understand %s\n", argv[firstarg+1]);
	}
	checkold(pattern, nlen);
	return 0;
}
