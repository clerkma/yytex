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

/* Program to modify ATM so dotlessi and caron work correctly */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* #define FNAMELEN 80 */

#define MAXLINE 128

#define MAXNAME 32

#define RINGSIZE 256

#define MAXHITS 1024

#define LOWCOUNT 16

#define MAXBUCKETS 100

#define MAXFILESIZE 200000

#define MAXSEPARATION (MAXFILESIZE / MAXBUCKETS)

unsigned int ring[RINGSIZE];	/* ring buffer for bytes coming in */

long hittable[MAXHITS];			/* table of matches found */

int buckets[MAXBUCKETS];		/* coarse buckets for clustering */

int hitindex = 0;

int atm=0, atm16=0, atm32=0;

int verboseflag = 0;
int traceflag = 0;
int detailflag = 0;
int modifyflag = 1;				/* actually write a new output file */
int renameflag = 0;				/* set if input file gets renamed */
int vectorflag=0;				/* non-zero if encoding vector specified */

int modified=0;					/* how many suspected modifications */

char vector[FILENAME_MAX];			/* name of encoding vector */

char *vecpath = "c:\\dvipsone";		/* default encoding vectors paths */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* old format for data */

/* replace DOTLESSIOLD => DOTLESSINEW && CARONOLD => CARONNEW */

/* int dotlessiold=1;	*/	/* in ATM 2.0 */
/* int dotlessinew=157;	*//* desired position */

/* long dotlessi, dotlessidash;	*//* file seek positions */

/* int caronold=2;	*/		/* in ATM 2.0 */
/* int caronnew=141;	*/	/* desired position */

/* long caron, carondash;	*/	/* file seek positions */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXACCENTS 32

/* new format for data */

int accentindex=2;			/* how many accents */

char accents[MAXACCENTS][MAXNAME] = {
	"dotlessi", "caron", "",
};

int newcode[MAXACCENTS] = {	/* dotlessinew, caronnew etc */
	157, 141, -1,
};

int oldcode[MAXACCENTS] = {	/* dotlessiold, caronold etc */
	1, 2, -1,
};

/* file position inside remapping table from StandareEncoding to ANSI */

long fileposa[MAXACCENTS];	/* dotlessi, caron etc */

/* file position inside lookup table from character name to ANSI */

long fileposb[MAXACCENTS];	/* dotlessidash, carondash etc */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* #define MAPCOUNT (37 + 1) */

/* ATM mapping from StandardEncoding (mapa) to `ANSI' (mapb) in ATM 2.0 */

int mapa[] = {						/* codes in StandardEncoding */
245, 207, 160, 193, 195, 177, 196, 168, 
200, 227, 202, 194, 203, 235, 225, 233, 
251, 241, 249, 169, 184, 166, 185, 188, 
178, 179, 189, 172, 234, 170, 186, 183, 
208, 173, 250, 197, 180, 0};
	
int mapb[] = {						/* codes in new Windows ANSI */
  1,   2,  32,  96, 136, 150, 152, 164, 
168, 170, 176, 180, 184, 186, 198, 216, 
223, 230, 248,  39, 130, 131, 132, 133, 
134, 135, 137, 139, 140, 147, 148, 149, 
151, 155, 156, 175, 183, 0};

/* dotlessi, caron, nbspace, grave, circumflex, endash, tilde, currency,
   dieresis, ordfeminine, ring?, acute, cedilla, ordmasculine, AE, Oslash,
   germandbls, ae, oslash, quotesingle, quotesinglbase, florin, quotedblbase, ellipsis,
   dagger, daggerdbl, perthousand, guilsinglleft, OE, quotedblleft,  quotedblright, bullet,
   emdash, quilsinglright, oe, macron, periodcentered */

/* DOES NOT INCLUDE, for example (ATM explicitly maps these to ZERO):
   breve, dotaccent, hungarumlaut, ogonek, 
   fi, fl, fraction, Lslash, lslash, minus */

/* character codes in ATM 2.0 sorted alphabetically on charname */

/* #define MAXCHRS (225 + 1) */

int alphacodes[] =
{65, 198, 193, 194, 196, 192, 197, 195, 66, 67, 199, 68, 69, 201, 202, 203,
200, 208, 70, 71, 72, 73, 205, 206, 207, 204, 74, 75, 76, 0, 77, 78, 209, 79,
140, 211, 212, 214, 210, 216, 213, 80, 81, 82, 83, 138, 84, 222, 85, 218,
219, 220, 217, 86, 87, 88, 89, 221, 159, 90, 97, 225, 226, 180, 228, 230,
224, 38, 229, 94, 126, 42, 64, 227, 98, 92, 124, 123, 125, 91, 93, 0, 166,
149, 99, 2, 231, 184, 162, 136, 58, 44, 169, 164, 100, 134, 135, 168, 247,
36, 0, 1, 101, 233, 234, 235, 232, 56, 133, 151, 150, 61, 240, 33, 161, 102,
0, 53, 0, 131, 52, 0, 103, 223, 96, 62, 171, 187, 139, 155, 104, 0, 45, 105,
237, 238, 239, 236, 106, 107, 108, 60, 172, 0, 109, 175, 0, 181, 215, 110,
57, 241, 35, 111, 243, 244, 246, 156, 0, 242, 49, 189, 188, 185, 170, 186,
248, 245, 112, 182, 40, 41, 37, 46, 183, 137, 43, 177, 113, 63, 191, 34, 132,
147, 148, 145, 146, 130, 39, 114, 174, 176, 115, 154, 167, 59, 55, 54, 47,
32, 163, 116, 254, 51, 190, 179, 152, 153, 50, 178, 117, 250, 251, 252, 249,
95, 118, 119, 120, 121, 253, 255, 165, 122, 48, -1};

#define MAXALPHAS 225

/* character codes in ATM 2.0 sorted alphabetically on charname */

char *alphanames[] = {
"A", "AE", "Aacute", "Acircumflex", "Adieresis", "Agrave", "Aring", "Atilde", 
"B", "C", "Ccedilla", "D", "E", "Eacute", "Ecircumflex", "Edieresis", 
"Egrave", "Eth", "F", "G", "H", "I", "Iacute", "Icircumflex", 
"Idieresis", "Igrave", "J", "K", "L", "Lslash", "M", "N", 
"Ntilde", "O", "OE", "Oacute", "Ocircumflex", "Odieresis", "Ograve", "Oslash", 
"Otilde", "P", "Q", "R", "S", "Scaron", "T", "Thorn", 
"U", "Uacute", "Ucircumflex", "Udieresis", "Ugrave", "V", "W", "X", 
"Y", "Yacute", "Ydieresis", "Z", "a", "aacute", "acircumflex", "acute", 
"adieresis", "ae", "agrave", "ampersand", "aring", "asciicircum",  "asciitilde", "asterisk", 
"at", "atilde", "b", "backslash", "bar", "braceleft", "braceright", "bracketleft",  
"bracketright", "breve", "brokenbar", "bullet", "c", "caron", "ccedilla", "cedilla", 
"cent", "circumflex", "colon", "comma", "copyright", "currency", "d", "dagger",
"daggerdbl", "dieresis", "divide", "dollar", "dotaccent", "dotlessi", "e", "eacute", 
"ecircumflex", "edieresis", "egrave", "eight", "ellipsis", "emdash", "endash", "equal", 
"eth", "exclam", "exclamdown", "f", "fi", "five", "fl", "florin", 
"four", "fraction", "g", "germandbls", "grave", "greater", "guillemotleft", "guillemotright", 
"guilsinglleft", "guilsinglright", "h", "hungarumlaut", "hyphen", "i", "iacute", "icircumflex", 
"idieresis", "igrave", "j", "k", "l", "less", "logicalnot", "lslash", 
"m", "macron", "minus", "mu", "multiply", "n", "nine", "ntilde", 
"numbersign", "o", "oacute", "ocircumflex", "odieresis", "oe", "ogonek", "ograve",  
"one", "onehalf", "onequarter", "onesuperior", "ordfeminine", "ordmasculine", "oslash", "otilde", 
"p", "paragraph", "parenleft", "parenright", "percent", "period", "periodcentered", "perthousand", 
"plus", "plusminus", "q", "question", "questiondown", "quotedbl",  "quotedblbase", "quotedblleft", 
"quotedblright", "quoteleft", "quoteright", "quotesinglbase", "quotesingle", "r", "registered", "ring",  
"s", "scaron", "section", "semicolon", "seven", "six", "slash", "space", 
"sterling", "t", "thorn", "three", "threequarters", "threesuperior", "tilde", "trademark", 
"two", "twosuperior", "u", "uacute", "ucircumflex", "udieresis", "ugrave", "underscore", 
"v", "w", "x", "y", "yacute", "ydieresis", "yen", "z", 
"zero", ""
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *standard[] = { 
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
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
 "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "exclamdown", "cent", "sterling", "fraction", "yen", "florin", "section",
 "currency", "quotesingle", "quotedblleft", "guillemotleft", "guilsinglleft", "guilsinglright", "fi", "fl",
 "", "endash", "dagger", "daggerdbl", "periodcentered", "", "paragraph", "bullet",
 "quotesinglbase", "quotedblbase", "quotedblright", "guillemotright", "ellipsis", "perthousand", "", "questiondown",
 "", "grave", "acute", "circumflex", "tilde", "macron", "breve", "dotaccent",
 "dieresis", "", "ring", "cedilla", "", "hungarumlaut", "ogonek", "caron",
 "emdash", "", "", "", "", "", "", "",
 "", "", "", "", "", "", "", "",
 "", "AE", "", "ordfeminine", "", "", "", "",
 "Lslash", "Oslash", "OE", "ordmasculine", "", "", "", "",
 "", "ae", "", "", "", "dotlessi", "", "",
 "lslash", "oslash", "oe", "germandbls", "", "", "", "",
}; 

/* Look for these accents in encoding file */
/* The last four don't appear in ATM remapping */

char *baseaccents[] = {
	"grave", "acute", "dieresis", "circumflex", "tilde", "caron", "ring", 
	"cedilla", 
	"dotlessi", 
	"macron", 
	"dotaccent", "hungarumlaut", "breve", "ogonek", ""
};

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int findatmcode (int stancode) {	/* given SE code, find ANSI code */
	int k;
/*	for (k = 0; k < MAPCOUNT; k++) { */
	for (k = 0; k < 256; k++) {
		if (mapa[k] == 0 || mapb[k] == 0) return -1;
		if (mapa[k] == stancode) return mapb[k];
	}
	return -1;
}

int standardlookup(char *name, int k) {		/* lookup StandardEncoding code */
	int i;
	if (k >= 0 && strcmp(standard[k], name) == 0) return k;
	for (i = 0; i < 256; i++) if (strcmp(standard[i], name)== 0) return i;
	return -1;
}

/* could speed up above using k = atmlookup() ??? if k < 128 ... */

/* lookup ATM sequence number */

int atmlookup(char *name) {				/* do binary search */
	int i, k;
	int comp;
	int low=0, high = MAXALPHAS;		/* 224 */

	for (i=0; i < 256; i++) {
		k = (low + high) / 2;
/*		printf("low %d k %d high %d ", low, k, high); */
		comp = strcmp(alphanames[k], name);
/*		printf("alpha %s name %s comp %d\n", alphanames[k], name, comp); */
		if (comp == 0) return k;
		else if (comp > 0) {
			if (high == k) return -1;
			high = k;
		}
		else {
			if (low == k) return -1;
			low = k;
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* catch positions where `chara' is followed by `charb' with offset `shift' */
/* enter all of these in hit table for later analysis */

int cooccurrence(FILE *input, int chara, int charb, int shift, int maxcount) {
	int old, new;
	long current;
	int k, index, count = 0;

	if (hitindex >= MAXHITS) return -1;
	rewind(input);
/* start by filling the ring buffer */
	for (k = 0; k < shift; k++) ring[k] = getc(input);
/*		ring[k] = (char) getc(input); */

	index = 0;
	while ((new = getc(input)) != EOF) {
		old = ring[index];							/* look back by shift */
		ring[index++] = new;						/* insert new in ring */
		if (index >= shift) index = 0;				/* wrap around */
		if (old == chara && new == charb) {			/* coincidence ? */
			current = ftell(input) - shift - 1;
			hittable[hitindex++] = current;
			if (count++ > maxcount) return 0;		/* not discriminating */
			if (hitindex >= MAXHITS) return -1;		/* enter in hit table */
/*			printf("%6ld: ", current);
			printf("%3d ", old);
			for (k = 0; k < shift; k++) {
				n = k + index;
				if (n >= shift) n -= shift;
				printf("%3d ", ring[n]);
			}
			putc('\n', stdout); */
		}
	}
	return 0;
}

long findpeak(void) {					/* find peak in hit table */
	int k, i, peakindex, peakcount, count;
	long lasthit=0, eps, focus, total;

	for (k = 0; k < hitindex; k++) {	/* find largest hit */
		if (hittable[k] > lasthit) lasthit = hittable[k];
	}
	eps = lasthit / (MAXBUCKETS - 1) + 1;		/* divide into increments */
	for (k = 0; k < MAXBUCKETS; k++) buckets[k] = 0;	/* reset */
	for (k = 0; k < hitindex; k++) {			/* enter into buckets */
		i = (int) (hittable[k] / eps);	
		buckets[i]++; buckets[i]++;
		i = (int) ((hittable[k] + eps/2) / eps);
		buckets[i]++;
		i = (int) ((hittable[k] - eps/2) / eps);
		buckets[i]++;
	}
	peakcount = 0; peakindex = -1;
	for (k = 0; k < MAXBUCKETS; k++) {	/* find peak */
		if (buckets[k] > peakcount) {
			peakcount = buckets[k]; peakindex = k;
		}
	}
	if (peakindex < 0) {
		fprintf(stderr, 
			"ERROR: Could not identify main focus - perhaps wrong file?\n");
		exit(1);
	}
	focus = (peakindex * eps + eps/2);

/*  now refine focus by averaging hits in main bucket */	
	total = 0; count = 0;
	for (k = 0; k < hitindex; k++) {			/* enter into buckets */
		i = (int) (hittable[k] / eps);	
		if (i == peakindex) {
			total += hittable[k]; count++;
			total += hittable[k]; count++;			
		}
		i = (int) ((hittable[k] + eps/2) / eps);
		if (i == peakindex) {
			total += hittable[k]; count++;
		}
		i = (int) ((hittable[k] - eps/2) / eps);
		if (i == peakindex) {
			total += hittable[k]; count++;
		}
	}
	if (total != 0) focus = total / count;
	else fprintf(stderr, "\nWARNING: Focus refinement failed\n");

	if (verboseflag != 0) 
		printf(" %d near main focus at %ld\n", peakcount/2, focus);
	return focus;
}

long findnearest(long focus) {
	int k;
	long place, distance, newplace, newdistance;
	place = hittable[0]; 
	distance = place - focus;
	if (distance < 0) distance = - distance;
	for (k = 0; k < hitindex; k++) {
		newplace = hittable[k];
		newdistance = newplace - focus;
		if (newdistance < 0) newdistance = - newdistance;
		if (newdistance < distance) {
			distance = newdistance;
			place = newplace;
		}
	}
	return place;
}

long findspecific(FILE *input, 
		int from, int to, int inc, long focus, char *name) {
	long place, separation;

	if (traceflag != 0)
		printf("Search for %d to %d inc %d focus %ld (%s)\n",
			from, to, inc, focus, name);
	hitindex = 0;
	place = cooccurrence(input, from, to, inc, MAXHITS);
	if (hitindex == 0) {
		fprintf(stderr, 
		"ERROR: Can't find `%s' mapping at all\n", 
				name);
		modified++;
		return -1;
	}
	place = findnearest(focus);
	separation = place - focus;
	if (separation < 0) separation = - separation;
	if (separation > MAXSEPARATION) {
		fprintf(stderr, 
	"ERROR: Can't find `%s' mapping near focus\n", 	name);
		modified++;
		return -1;
	}
/*	if (verboseflag != 0) printf("Will modify byte %ld\n", place + inc); */
	return place;
}

/* find the remapping table */

long findmaptable(FILE *input, int inc) {
	int k;
	long focus;
/*	int previous, next; */

	hitindex = 0;

/*	for (k = 0; k < MAPCOUNT; k++) { */
	for (k = 0; k < 256; k++) {
		if (mapa[k] == 0 || mapb[k] == 0) break;
		if (cooccurrence(input, mapa[k], mapb[k], inc, LOWCOUNT/2) != 0) {
			if (verboseflag != 0) printf("Filled hit tables\n");
			break;
		}
		if (verboseflag != 0) putc(':', stdout);
	}
	if (verboseflag != 0) putc('\n', stdout);
	if (verboseflag != 0) printf("Found %d hits -", hitindex);
	focus = findpeak();
/*	if (verboseflag != 0) printf(" Focussing on area near %ld\n", focus); */
	return focus;
}

/* find file positions of characters of interest */

/* void findmapitems(FILE *input, int inc, long focus) {
	int dotlessistand=245;
	int caronstand=207;
	
	dotlessistand = standardlookup ("dotlessi", 245);
	if (traceflag != 0) printf("dotlessi standard %d\n", dotlessistand);
	dotlessiold = findatmcode(dotlessistand);
	if (traceflag != 0) printf("dotlessi old %d\n", dotlessiold);
	dotlessi = findspecific(input, dotlessistand, dotlessiold, inc, focus, "dotlessi");
	if (dotlessi > 0) dotlessi +=  inc;
	if (verboseflag != 0 && dotlessi > 0) 
		printf("Will modify byte %ld\n", dotlessi);

	caronstand = standardlookup ("caron", 207);
	if (traceflag != 0) printf("caron standard %d\n", caronstand);
	caronold = findatmcode(caronstand);
	if (traceflag != 0) printf("caron old %d\n", caronold);
	caron = findspecific(input, caronstand, caronold, inc, focus, "caron");
	if (caron > 0) caron +=  inc;
	if (verboseflag != 0 && caron > 0) 
		printf("Will modify byte %ld\n", caron);
} */

int findmapitems(FILE *input, int inc, long focus) {	/* new version */
	int k, count=0;
	int accentstand, accentold;
	long filepos;
	
	for (k = 0; k < accentindex; k++) {
		if (strcmp(accents[k], "") == 0) break;
		accentstand = standardlookup(accents[k], -1);
		if (accentstand < 0) {
			fprintf(stderr, "WARNING: `%s' not in StandardEncoding\n", 
				accents[k]);
			fileposa[k] = -1;
			continue;
		}
		if (traceflag != 0) 
			printf("`%s' standard %d ", accents[k], accentstand);
		accentold = findatmcode(accentstand);
		if (accentold < 0) {
			fprintf(stderr, "WARNING: `%s' not in ATM accent list\n", 
				accents[k]);
			fileposa[k] = -1;
			oldcode[k] = 0;			/* ??? */
			continue;
/*			break;		 */
		}
		oldcode[k] = accentold;
		if (traceflag != 0) 
			printf("- old %d\n", accents[k], accentold);
		filepos = findspecific(input, accentstand, accentold, inc, 
			focus, accents[k]);
		if (filepos < 0) {
			fprintf(stderr, "WARNING: `%s' not found in file\n", accents[k]);
		}
		else filepos = filepos + inc;
		fileposa[k] = filepos;
		if (verboseflag != 0 && filepos > 0) 
			printf("Will modify byte %ld (%s)\n", filepos, accents[k]);
		if (filepos > 0) count++;
	}
	return count;
}

/* find table of character names sorted alphabetically */

long findalphatable(FILE *input, int delta) {
/*	long dotlessione, dotlessitwo, caronone, carontwo; */
	long focus;
/*	int previous, next; */

	hitindex = 0;

  	cooccurrence(input, 'B', 'C', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'D', 'E', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'F', 'G', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'G', 'H', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'H', 'I', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'J', 'K', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'K', 'L', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'M', 'N', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'P', 'Q', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'Q', 'R', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'R', 'S', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'V', 'W', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'W', 'X', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'X', 'Y', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'j', 'k', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'k', 'l', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'v', 'w', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'w', 'x', delta, LOWCOUNT); putc('.', stdout);
	cooccurrence(input, 'x', 'y', delta, LOWCOUNT); putc('.', stdout);
/*	if (verboseflag != 0) putc('\n', stdout); */
	putc('\n', stdout);
	if (verboseflag != 0) printf("Found %d hits -", hitindex);
	focus = findpeak();
	return focus;
}

/* void findalphaitems(FILE *input, int delta, long focus) {
	long dotlessione, dotlessitwo, caronone, carontwo;
	int previous, next;
	int dotlessiindex, caronindex;

	dotlessiindex = atmlookup("dotlessi");
	dotlessiold = alphacodes[dotlessiindex];
	if (traceflag != 0) printf("dotlessi old %d\n", dotlessiold);
	previous = alphacodes[dotlessiindex-1];
	
	dotlessione = findspecific(input, previous, dotlessiold, delta, 
		focus, "dotlessi");
	if (dotlessione > 0) dotlessione += delta;
	next = alphacodes[dotlessiindex+1];
	dotlessitwo = findspecific(input, dotlessiold, next, delta, 
		focus, "dotlessi");
	if (dotlessione != dotlessitwo) {
		fprintf(stderr, 
			"WARNING: Uncertain dotlessi position: %ld versus %ld\n", 
				dotlessione, dotlessitwo);
	}
	if (dotlessitwo > 0) dotlessidash = dotlessitwo;
	else dotlessidash = dotlessione;
	if (verboseflag != 0 && dotlessidash > 0) 
		printf("Will modify byte %ld\n", dotlessidash);

	caronindex = atmlookup("caron");
	caronold =  alphacodes[caronindex];
	if (traceflag != 0) printf("caron old %d\n", caronold);
	previous = alphacodes[caronindex-1];
	caronone = findspecific(input, previous, caronold, delta, focus, "caron");
	if (caronone > 0)  caronone += delta;
	next = alphacodes[caronindex+1];
	carontwo = findspecific(input, caronold, next, delta, focus, "caron"); 
	if (caronone != carontwo) {
		fprintf(stderr, "WARNING: Uncertain caron position: %ld versus %ld\n",
			caronone, carontwo);
	}
	if (carontwo > 0) carondash = caronone;
	else carondash = carontwo;
	if (verboseflag != 0 && carondash > 0)
		printf("Will modify byte %ld\n", carondash);	
} */

int findalphaitems(FILE *input, int delta, long focus) { /* new version */
	int k, count=0;
	int accentold;
	long filepos1, filepos2;
	int index, previous, next;

	for (k = 0; k < accentindex; k++) {
		if (strcmp(accents[k], "") == 0) break;
		index = atmlookup(accents[k]);	/* index in sorted alpha table */
		if (index < 0) { 
			fprintf(stderr, "WARNING: no ATM accent  %s\n", accents[k]);
			fileposb[k] = -1;
			continue;
		}
		if (traceflag != 0) 
			printf("`%s' at %d in ATM table ", accents[k], index);
		accentold = alphacodes[index];
		if (traceflag != 0) printf("- old %d\n", accents[k], accentold);
		previous = alphacodes[index-1];
/*		what if index is first in accent table ? (A) */
		filepos1 = findspecific(input, previous, accentold, delta, 
			focus, accents[k]);

		if (filepos1 > 0) filepos1 = filepos1 + delta;
		next = alphacodes[index+1];
/*		what if index is last in table ? (zero) */
		filepos2 = findspecific(input, accentold, next, delta, 
			focus, accents[k]);
		if (filepos2 != filepos1) {
			fprintf(stderr, 
				"WARNING: Uncertain `%s' position: %ld versus %ld\n", 
					accents[k], filepos1, filepos2);
		}
		if (filepos2 > 0) fileposb[k] = filepos2;
		else fileposb[k] = filepos1;
		if (fileposb[k] > 0) count++;
		if (verboseflag != 0 && fileposb[k] > 0) 
			printf("Will modify byte %ld (%s)\n", fileposb[k], accents[k]);
	}
	return count;
}

int stringsearch(FILE *input, char *string) {
	int c, d;
	char *s=string;
	rewind(input);
	while ((c = getc(input)) != EOF) {
		if ((d = *s++) != c) s = string;
		else if (*s == '\0') return -1;
	}
	return 0;
}

int checkversion(FILE *input) {
	int c;
	char buffer[256];
	char *s=buffer;
	char *s16, *s32;

	if (stringsearch(input, "v2.") == 0) {
		fprintf(stderr, 
			"ERROR: File does not appear to be correct version\n");
		return 0;
	}
	printf("VERSION: V2.");
	while ((c = getc(input)) > 0) {
		*s++ = (char) c;
		putc(c, stdout); 
	}
	putc('\n', stdout);
/*	if (strstr(buffer, "32") != NULL) atm32++; 
	else if (strstr(buffer, "16") != NULL) atm16++;	 */
	s16 = strstr(buffer, "16");
	s32 = strstr(buffer, "32");
	if (s16 != NULL && (s32 == NULL || s16 < s32)) atm16++;
	if (s32 != NULL && (s16 == NULL || s32 < s16)) atm32++;
	return -1;
}

void shownames(FILE *input, long table, long string) {
	int k, c, d, e, f;
	int chr;
	long pos;

	if (atm32 != 0) {

	for (k = 0; k < 256; k++) {
		fseek(input, table + k * 8, SEEK_SET);
/*		chr = getc(input); */
/*		printf("%3d: ", chr); */
/*		(void) getc(input); (void) getc(input); (void) getc(input); */
		c = getc(input); d = getc(input);
		pos = ((long) d << 8) | c;
		(void) getc(input); (void) getc(input); 
		pos = pos + string;
		chr = getc(input); 
		printf("%3d: ", chr); 
		if (fseek(input, pos, SEEK_SET) != 0) {
			fprintf(stderr, "Seek error %ld\n", pos);
			return;
		}
		while ((c = getc(input)) != 0) putc(c, stdout);
		putc('\n', stdout);
	}
	return; }
	else if (atm16 != 0 || atm != 0) {
		pos = string;
		for (k = 0; k < 256; k++) {
			fseek(input, table + k * 6, SEEK_SET);
			c = getc(input); d = getc(input); 
			e = getc(input); f = getc(input);
			chr = getc(input); f = getc(input);
			if (c != 255 || d != 255 || e != 0 || f != 0) break;
			printf("%3d: ", chr);
			fseek(input, pos, SEEK_SET);
			while ((c = getc(input)) != '\0') putc(c, stdout);
			putc('\n', stdout);
			pos = ftell(input);
		}
	}
}

void showmapping(FILE *input, long start, int inc) {
	int k, c, d, e, f, g;
	fseek(input, start, SEEK_SET);
	for (k = 0; k < 256; k++) {
		c = getc(input); 
		e = getc(input); 
		if (inc > 2) {
			f = getc(input); g = getc(input);
			if (e != 0 || f != 0 || g != 0) break;		
		}
		else if (e != 0) break;
		d = getc(input); 
		e = getc(input); 
		if (inc > 2) {
			f = getc(input); g = getc(input);		
			if (e != 0 || f != 0 || g != 0) break;
		}
		else if (e != 0) break;
		printf("%3d => %3d\n", c, d);
	}
}

void extension(char *fname, char *str) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
				((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
}

/* int scanandreplace (FILE *output, *input) {
	int errorflag = 0;
	count = 0;
	while ((c = getc(input)) != EOF) {
		if (count == dotlessi || count == dotlessidash) {
			if (c == dotlessiold) putc(dotlessinew, output);
			else {
				errorflag++; break;
			}
		}
		else if (count == caron || count == carondash) {
			if (c == caronold) putc(caronnew, output);
			else {
				errorflag++; break;
			}
		}
		else putc(c, output);
		count++;
	}
	return errorflag;
} */

void copyfile (FILE *output, FILE *input, long count) {
	long k;
	for (k = 0; k < count; k++) putc(getc(input), output);
}

/* should really sort file pointers */

/* This is inefficient version - lots of comparing each byte */

int scanandreplace (FILE *output, FILE *input) {	/* new version */
	int k, errorflag = 0;
	int c, count=0;
	long filepos = 0;

	while ((c = getc(input)) != EOF) {
		for (k = 0; k < accentindex; k++) {
			if (fileposa[k] == filepos || fileposb[k] == filepos) {
				if (c == oldcode[k]) {
					c = newcode[k];
					count++;
				}
				else {
					fprintf(stderr, "at %ld: see %d, expected %d\n",
						filepos, c, oldcode[k]);
					errorflag++;
				}
				if (verboseflag != 0) putc('*', stdout);
				break;
			}
		}
		putc(c, output);
		filepos++;
	}
	if (verboseflag != 0) putc('\n', stdout);
	if (errorflag != 0) return -1;
	else return count;
}

int modifyfile (char *filename) {
	FILE *input, *output;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX], bakfilename[FILENAME_MAX];
	char *s;
	int k;
	int errorflag = 0, count;

	strcpy(infilename, filename);

/*  strip off path to put output in current directory */	
	if ((s = strrchr(filename, '\\')) != NULL) s++;
	else if ((s = strrchr(filename, '/')) != NULL) s++;
	else if ((s = strrchr(filename, ':')) != NULL) s++;	
	else s = filename;

	strcpy(outfilename, s);
	forceexten(outfilename, "dll");
	if (strcmp(infilename, outfilename) == 0) {
		strcpy(bakfilename, infilename);
		forceexten(infilename, "bak");
		printf("Renaming %s to %s\n", bakfilename, infilename);
		renameflag++;
		if (rename(bakfilename, infilename) != 0) {
			printf("Oops, first have to delete %s\n", infilename);
			remove(infilename);
			if (rename(bakfilename, infilename) != 0) {
				fprintf(stderr, "ERROR: Unable to rename file %s\n", 
					infilename);
				return -1;
			}
		}
	}
	if (verboseflag != 0) 
		printf("Input: %s => output: %s\n", infilename, outfilename);

	if (modifyflag == 0) return -1;

	if ((input = fopen(infilename, "rb")) == NULL) {
		perror(infilename);
		return -1;
	}
	if ((output = fopen(outfilename, "wb")) == NULL) {
		perror(outfilename);
		fclose(input);
		return -1;
	}	

	if (traceflag != 0) {
		printf("File position and byte alteration table:\n");
		for (k = 0; k < accentindex; k++) {
			printf("%d %ld %ld oldcode %d newcode %d (%s)\n", 
			k, fileposa[k], fileposb[k], oldcode[k], newcode[k], accents[k]);
		}
	}

	count = scanandreplace(output, input);

	fclose(input);
	if (ferror(output) != 0) {
		perror(outfilename);
		return -1;
	}
	else fclose(output);

	if (count < 0) {
		fprintf(stderr, "ERROR: Impossible file inconsistency\n");
		if (renameflag != 0) {
			(void) remove (bakfilename);
			rename(infilename, bakfilename);
		}
		exit(3);
	}
	return count;
}

/* for debugging purposes: show alpha-sorted table of ATM codes */

void showalphas (FILE *output) {
	int k;
	for (k=0; k < 256; k++) {
		if (alphacodes[k] < 0) return;
		if (*alphanames[k] == '\0') return;
		fprintf(output, "%d\t%s\n", alphacodes[k], alphanames[k]);
	}
}

void showusage(char *prog) {
	fprintf(stderr, "Usage:\t%s [-{v}{m}] [-c=<vector-file>] <DLL name>\n", prog);
/*	fprintf(stderr, "\te.g. atmfix -v c:\\windows\\system\\atm32.dll\n");*/
	if (detailflag == 0) exit(3);
	putc('\n', stderr);
	fprintf(stderr, "\tv verbose mode\n");
	fprintf(stderr, "\tm suppress output\n");
	fprintf(stderr, "\tc use accent positions in specified encoding vector\n");
	fprintf(stderr, "\t(Output appears in current directory)\n");
	exit(3);
}

int isbase (char *charname) {
	int k;

	for (k = 0; k < 16; k++) {
		if (strcmp(baseaccents[k], "") == 0) break;
		if (strcmp(baseaccents[k], charname) == 0) {
			return k;
		}
	}
	return -1;
}

int readencoding(char *vector) {
	char filename[FILENAME_MAX];
	char line[MAXLINE];
	char charname[MAXNAME];
	int charcode, atmcode, dropcode;
	int i, flag;
/*	int k, flag; */
	FILE *input;

	strcpy(filename, vector);
	extension(filename, "vec");
	input = fopen(filename, "r");
	if (input == NULL) {			/* a bit crude ... */
		strcpy(filename, vecpath);
		strcat(filename, "\\");
		strcat(filename, vector);
		extension(filename, "vec");
		input = fopen(filename, "r");
	}
	if (input == NULL) {
		perror(filename); return -1;
	}
	accentindex = 0;
	if (verboseflag != 0) printf("Accent remapping table\n");
	for(;;) {
		if (fgets(line, MAXLINE, input) == NULL) break;
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (sscanf(line, "%d %s", &charcode, charname) == 2) {
			if (isbase(charname) < 0) continue;	/* want only accents */
			atmcode = atmlookup(charname);
			if (atmcode < 0) continue;			/* not ATM character name */
			dropcode = alphacodes[atmcode];
			if (dropcode == charcode) continue;	/* no change in position */
			flag = 0;
			for (i=0; i < accentindex; i++) {	/* is it a repeat */
				if (strcmp(accents[i], charname) == 0) {
					fprintf(stderr, 
						"WARNING: repeated accent `%s' (%d and %d)\n", 
							charname, newcode[i], charcode);
					flag++;
					break;
				}
			}
			if (flag != 0) continue;
			strncpy(accents[accentindex], charname, MAXNAME);
			oldcode[accentindex] = dropcode;
			newcode[accentindex] = charcode;
			if (verboseflag != 0) {
				printf("`%s'", charname);
				if (strlen(charname) < 6) putc('\t', stdout);
				printf("\tfrom %d", dropcode);
				if (dropcode < 100) putc('\t', stdout);
				printf("\tto %d\n", charcode);
			}
			accentindex++;
			if (accentindex >= MAXACCENTS) {
				fprintf(stderr, "ERROR: too many accents\n");
				accentindex--;
			}
			continue;
		}
	}
	fclose(input);
	if (verboseflag != 0) putc('\n', stdout);
	if (verboseflag != 0) printf("%d accents recorded\n", accentindex);
	return accentindex;
}

int main(int argc, char *argv[]) {
	char filename[FILENAME_MAX];
	FILE *input;
/*	int chara, charb; */
	int shift=1, increment = 4, delta = 8;
	long tablestart=0, nameoffset = 0, maptable = 0;
	int firstarg = 1;
	int c, d, k, flag;
	long focus;
	int counta, countb, countc;
	char *s;

	if (argc < 2) showusage(argv[0]);

	s = argv[firstarg];
	c = *s++;
/*	while (c == '-' || c == '/') { */
	while (firstarg < argc && (c == '-' || c == '/')) {
/*		printf("Command line argument %s\n", argv[firstarg]); */
		c = *s++;
		for (;;) {
			if (c == 0) break;
			else if (c == '?') detailflag++;
			else if (c == 'v') verboseflag++;
			else if (c == 't') traceflag++;
			else if (c == 'm') modifyflag = 0;
			else if (c == 'c') {
				if (*s++ == '=') ;
				else {
					firstarg++;
					s = argv[firstarg];
				}
				if (sscanf(s, "%s",  &vector) == 1) vectorflag++;
				else fprintf(stderr, "Don't understand: %s\n", s);
				break;
			}
			c = *s++;
		}
		firstarg++;
		s = argv[firstarg];
		c = *s++;
	}

	if (traceflag > 1) {
		showalphas(stdout); 
		exit(0);
	}

	if (argc < firstarg+1) showusage(argv[0]);

	if (verboseflag > 1) traceflag++;

	if ((s = getenv("VECPATH")) != NULL) vecpath = s;

	if (vectorflag != 0) {			/* encoding or such specified */
		if (readencoding(vector) < 0) {
			fprintf(stderr, "ERROR: unable to understand encoding file\n");
			exit(3);
		}
	}

	strncpy(filename, argv[firstarg], FILENAME_MAX);
	if (strcmp(filename, "") == 0) {
/*		strcpy(filename, "c:\\windows\\system\\atm32.dll"); */
		fprintf(stderr, "ERROR: no file name specified\n");
		exit(3);
	} 
	extension(filename, "dll");
	if ((input = fopen(filename, "rb")) == NULL) {
		perror(filename);
		exit(2);
	}

/*	try and guess from file name whether 16 or 32 bit version */
	if (strstr(filename, "32") != NULL) atm32++;
	else if (strstr(filename, "16") != NULL) atm16++;
	else atm++;

/*	if (shift < 0) shownames(input, tablestart, nameoffset); */
/*	else if (shift == 0) showmapping(input, maptable, increment); */
/*	else cooccurrence(input, chara, charb, shift); */

/*	check whether reasonable file format */
	c = getc(input); d = getc(input);
	if (c != 'M' || d != 'Z') {
		fclose(input);
		fprintf(stderr, "ERROR: Sorry, %s is not an EXE file\n", filename);
		exit(1); 
	}

/*	Try and get version information and 16 versus 32 bit information */
	checkversion(input);

	if (atm16 != 0 && atm32 != 0) 
		fprintf(stderr, 
			"ERROR: Claims to be both 16 bit and 32 bit version\n");
	if (atm16 == 0 && atm32 == 0)
		fprintf(stderr, 
			"ERROR: Appears to be neither 16 bit nor 32 bit version\n");
		
/*	following are guesses, not actually used */
/*	except for increment and delta, which ARE definitely needed ... */
	if (atm32 != 0) {
		tablestart = 65108;
		nameoffset = 69932 - 15884;
		maptable = 64812;
		increment = 4;
		delta = 8;
	}
	else if (atm16 != 0) {
		tablestart = 152350;
		nameoffset = 150634 - 0;
		maptable = 152202;
		increment = 2;
		delta = 6;
	}
	else if (atm != 0) {
		tablestart = 156020;
		nameoffset = 154410 - 0;
		maptable = 155944;
		increment = 2;
		delta = 6;		
	}

	modified = 0;
	if (verboseflag != 0) 
		printf("Searching for StandardEncoding to ATM mapping table\n");
	focus = findmaptable(input, increment);
	counta = findmapitems(input, increment, focus);
	if (verboseflag != 0) printf("Searching for sorted character name table\n");
	focus = findalphatable(input, delta);
	countb = findalphaitems(input, delta, focus);
	fclose(input);

	if (counta == 0 || countb == 0) {
		fprintf(stderr, "ERROR: unable to find needed data in file\n");
		fprintf(stderr, "ERROR: giving up - output file not written\n");
		exit(3);
	}
	if (modified > 0) 
		fprintf(stderr, 
			"ERROR: file appears to have been already modified (%d)\n",
				modified);

/*	if (dotlessi > 0 && caron > 0 && 
		dotlessidash > 0 && carondash > 0) {
		if (modifyfile(filename) == 0) {
			printf("File %s successfully converted\n", filename);
			printf("Modified file appears in current directory\n");
		}
	} */

	if (verboseflag != 0) printf("Modifying file now\n");
	countc = modifyfile(filename);
	if (countc < 0) {
		fprintf(stderr, "ERROR: too many unknowns - file not processed\n");
		exit(1);
	}
	flag = 0;
	if (counta < accentindex || countb < accentindex ||
		countc < accentindex * 2) {
		fprintf(stderr,
			"WARNING: Unable to locate all accent translation points for:\n");

		for (k = 0; k < accentindex; k++) {
			if (fileposa[k] < 0 || fileposb[k] < 0) {
				fprintf(stderr, "%s ", accents[k]);
				if (isbase(accents[k]) < 9) flag++;
			}
		}
		putc('\n', stderr);
		if (flag == 0)
		fprintf(stderr, "OK for: breve, dotaccent, hungarumlaut, ogonek\n"); 
	}
	if (flag == 0) 
		printf("File %s successfully converted\n", filename);

	printf("Modified file appears in current directory\n");

	return 0;
}

/* NOTE: explicitly mapped to zero in ATM 2.0: */

/* breve, dotaccent, hungarumlaut, ogonek, */
/* fi, fl, fraction, Lslash, lslash, minus */
