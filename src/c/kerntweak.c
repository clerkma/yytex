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

/* tweak kern pairs produced by Kernus to make them useable in EM */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXLINE 512
#define MAXKERNS 2096
#define MAXNAMES 512
#define MAXCHARNAME 64
#define MAXHISTOGRAM 400

char line[MAXLINE];

char *charnames[MAXNAMES];

int charindex;

struct tagKERNPAIR {
	int a;
	int b;
	int kern;
} KERNPAIR;

struct tagKERNPAIR kernpairs[MAXKERNS];

int kernindex;

int histogram[MAXHISTOGRAM];

int kerninterval=2;			/* kern rounding */

int interval=10;			/* histogramming interval */

int sortonname=1;

int epsilon=5;

int verboseflag=1;
int traceflag=0;
int debugflag=0;

int roundflag=0;
int leftcompositesflag=0;
int rightcompositesflag=0;

int difthresh=50;
/* int difthresh=40; */
/* int difthresh=35; */

/**************************************************************************/

int lookup (char *charname) {	/* lookup character name */
	int k;
	for (k = 0; k < charindex; k++) {
		if (strcmp(charname, charnames[k]) == 0) return k;
	}
	return -1;
}

int getcharnum (char *charname) {	/* get unique character ID */
	int k;
	k = lookup(charname);
	if (k >= 0) return k;
	if (charindex >= MAXNAMES) {
		printf ("Too many distinct character names\n");
		exit(1);
	}
	k = charindex;
	charnames[charindex++] = _strdup(charname);
	return k;
}

int round (double x) {
	if (x < 0.0) return -round (- x);
	return (int) (x + 0.5);
}

/*********************************************************************/

char *composites[] = {
	"Aacute",
	"Abreve",
	"Acircumflex",
	"Adieresis",
	"Agrave",
	"Aogonek",
	"Aring",
	"Atilde",
	"Cacute",
	"Ccaron",
	"Ccedilla",
/*	"Dcroat", */		/* ? */
	"Eacute",
	"Ecaron",
	"Ecircumflex",
	"Edieresis",
	"Egrave",
	"Eogonek",
	"Gbreve",
	"Iacute",
	"Icircumflex",
	"Idieresis",
	"Idotaccent",
	"Igrave",
	"Lacute",			/* ? */
	"Lcaron",			/* ? */
	"Lslash",			/* ? */
	"Nacute",
	"Ncaron",
	"Ntilde",
	"Oacute",
	"Ocircumflex",
	"Odieresis",
	"Ograve",
	"Ohungarumlaut",
	"Otilde",
	"Oslash",
	"Racute",
	"Rcaron",	
	"Sacute",
	"Scaron",
	"Scedilla",
	"Scommaaccent",
	"Tcaron",
	"Tcedilla",
	"Tcommaaccent",
	"Uacute",
	"Ucircumflex",
	"Udieresis",
	"Ugrave",
	"Uhungarumlaut",
	"Uring",
	"Yacute",
	"Ydieresis",
	"Zacute",
	"Zcaron",
	"Zdotaccent",
	""
};

int iscomposite (char *name) {
	int k, m;
	char base[2]=" ";

	for (k = 0; k < 128; k++) {
		if (*(composites[k]) == '\0') break;
		if (_strcmpi(composites[k], name) == 0) {
			base[0] = *name;
			base[1] = '\0';
			m = lookup(base);
			if (m < 0) printf("%s base %s not found\n", name, base);
			else if (traceflag) printf("%s based on %s\n", name, base);
			if (m < 0) return m;
/*			else return m; */
			else return base[0];
		}
	}
	if (traceflag) printf("%s not composite\n", name);
	return -1;
}

void showcharnames (void) {
	int k;
	for (k = 0; k < charindex; k++) {
		printf("%3d %s\n", k, charnames[k]);
	}
	putc('\n', stdout);
}

/**************************************************************************/

char charnamea[MAXCHARNAME], charnameb[MAXCHARNAME];

int readkernfile (FILE *input) {
	int count = 0;
	int chara, charb;
	double kern;
	
	charindex = 0;
	kernindex = 0;
	while (fgets(line, sizeof(line), input) != NULL) {
		if (sscanf(line, "KPX %s %s %lg", charnamea, charnameb, &kern) == 3) {
			if (kernindex >= MAXKERNS) {
				printf("Too many kern pairs\n");
				exit(1);
			}
			chara = getcharnum(charnamea);
			charb = getcharnum(charnameb);
			kernpairs[kernindex].a = chara;
			kernpairs[kernindex].b = charb;
			kernpairs[kernindex].kern = round (kern);
			kernindex++;
			count++;
		}
	}
	printf("Read %d kern pairs\n", count);
	return count;
}

void showkernfile(FILE *output) {
	int k, count=0;
	for (k = 0; k < kernindex; k++) {
		if (kernpairs[k].kern >= epsilon || kernpairs[k].kern <= epsilon)
			count++;
	}
	fprintf(output, "StartKernPairs %d\n", count);
	for (k = 0; k < kernindex; k++) {
		if (kernpairs[k].kern > epsilon || kernpairs[k].kern < epsilon)
			fprintf(output, "KPX %s %s %d\n",
					charnames[kernpairs[k].a],
					charnames[kernpairs[k].b],
					kernpairs[k].kern);
	}
	fprintf(output, "EndKernPairs\n");	
}

int findkern (int chara, int charb) {
	int k;
/*	printf("%s %d %s %d ?\n",
		   charnames[chara], chara, charnames[charb], charb); */
	for (k = 0; k < kernindex; k++) {
		if (kernpairs[k].a == chara && kernpairs[k].b == charb) return k;
	}
	return -1;
}

int comparekernfile (FILE *input) {
	int count = 0;
	int chara, charb, k;
	double kern;
	int kernold, kernnew, kerndif;
	int shownew=0;
	
/*	charindex = 0; */
/*	kernindex = 0; */
	while (fgets(line, sizeof(line), input) != NULL) {
		if (sscanf(line, "KPX %s %s %lg", charnamea, charnameb, &kern) == 3) {
			if (kernindex >= MAXKERNS) {
				printf("Too many kern pairs\n");
				exit(1);
			}
			chara = getcharnum(charnamea);
			charb = getcharnum(charnameb);
/*			kernpairs[kernindex].kern = round (kern); */
			k = findkern(chara, charb);
/*			printf("%s %d %s %d %d\n",
					charnamea, chara, charnameb, charb, k); */
			if (k >= 0) {
				kernold = kernpairs[k].kern;
				kernnew = round(kern);
				kerndif = kernnew - kernold;
				if (kerndif > difthresh || kerndif < - difthresh)
					printf("COM: KPX %s %s\t%d %lg\t(%d)\n",
					   charnamea, charnameb, kernold, kern, kerndif);
/*				if (kernnew * kernold < 0.0) printf(" SIGN REVERSAL"); */
			}
			else {
				kerndif = kernnew = round(kern);
				if (shownew) printf("NEW: %s", line);
				if (kerndif > difthresh || kerndif < - difthresh)
					printf("NEW: KPX %s %s %lg\t(%d)\n",
						   charnamea, charnameb, kern, kerndif);					
			}
			count++;
		}
	}
	printf("Read %d kern pairs\n", count);
	return count;
}

/* MAXHISTOGRAM/2 corresponds to zero kern */

void makehistogram (void) {
	int k, m;
	int kern;
	for (k=0; k < MAXHISTOGRAM; k++) histogram[k]=0;
	for (k=0; k < kernindex; k++) {
		kern = kernpairs[k].kern;
		m = kern + MAXHISTOGRAM/2;
		if (m >= 0 && m < MAXHISTOGRAM)	histogram[m]++;
	}
}

void showhistogram (int limit) {
	int k, m, count=0;
	long sum=0, sumk=0, sumkk=0;
	for (k=0; k < MAXHISTOGRAM; k++) {
		if (histogram[k] != 0) count++;
		sum += histogram[k];
		sumk += (long) histogram[k] * (k-MAXHISTOGRAM/2);
		sumkk += (long) histogram[k] * (k-MAXHISTOGRAM/2)*(k-MAXHISTOGRAM/2);
	}
	printf("%ld kern pairs, mean %lg, st dev %lg, distinct values %d\n",
		   sum, (double) sumk / sum,
		   sqrt(((double) sumkk - sumk * sumk / sum) / sum), count);
	if (count < limit) {
		for (k=0; k < MAXHISTOGRAM; k++) {		
			if (histogram[k] > 0)
				printf("%4d\t%3d\n", k-MAXHISTOGRAM/2, histogram[k]);
		}
	}
	else {
		for (k=1; k < MAXHISTOGRAM/2; k+=interval) {
			sum = 0;
			for (m = 0; m < interval; m++) {
				if (debugflag) printf("%d ", k+m - MAXHISTOGRAM/2);
				sum += histogram[k+m];
			}
			if (debugflag) putc('\n', stdout);
			if (sum > 0) {
				printf("%4d\t%3d\n", k+interval/2-MAXHISTOGRAM/2-1, sum);
			}
		}
/* Note: kern distance of 0 is counted both in -9 to 0 and 0 to +9 range */
		for (k=MAXHISTOGRAM/2; k < MAXHISTOGRAM-interval; k+=interval) {
			sum = 0;
			for (m = 0; m < interval; m++) {
				if (debugflag) printf("%d ", k+m - MAXHISTOGRAM/2);
				sum += histogram[k+m];
			}
			if (debugflag) putc('\n', stdout);
			if (sum > 0) {
				printf("%4d\t%3d\n", k+interval/2-MAXHISTOGRAM/2, sum);
			}
		}
	}
}


int tagleftcomposites (void) {
	int k, m, nchara, ncharb, kern;
	int count=0;
	char base[2]=" ";
	for (k=0; k < kernindex; k++) {
		nchara = kernpairs[k].a;
		ncharb = kernpairs[k].b;
		kern = kernpairs[k].kern;
		if ((m = iscomposite(charnames[nchara])) >= 0) {
			base[0] = (char) m;
			base[1] = '\0';
			if (lookup(base) >= 0) {
				kernpairs[k].a = 0;
				kernpairs[k].b = 0;
				kernpairs[k].kern = 0;
				count++;
			}
		}
	}
	return count;
}

int tagrightcomposites (void) {
	int k, m, nchara, ncharb, kern;
	int count=0;
	char base[2]=" ";
	for (k=0; k < kernindex; k++) {
		nchara = kernpairs[k].a;
		ncharb = kernpairs[k].b;
		kern = kernpairs[k].kern;
		if ((m = iscomposite(charnames[ncharb])) >= 0) {
			base[0] = (char) m;
			base[1] = '\0';
			if (lookup(base) >= 0) {
				kernpairs[k].a = 0;
				kernpairs[k].b = 0;
				kernpairs[k].kern = 0;
				count++;
			}
		}
	}
	return count;
}


int removetagged (void) {
	int i=0, j=0, count=0;
	while (j < kernindex) {
		if (kernpairs[j].a == 0 && kernpairs[j].b == 0 &&
			kernpairs[j].kern == 0) {
			count++;
		}
		else {
			if (i < j) kernpairs[i] = kernpairs[j];
			i++;
		}
		j++;
	}
	kernindex = i;
	return count;
}

void roundkerning (int interval) {
	int k;
	int kern;
	for (k = 0; k < kernindex; k++) {
		kern = kernpairs[k].kern;
/*		printf("%d -> ", kern); */
		if (kern < 0) kern = - ((-kern + interval/2-1) / interval) * interval;
		else if (kern > 0) kern = ((kern + interval/2-1) / interval) * interval;
/*		printf("%d ", kern); */
		kernpairs[k].kern = kern;
	}
}

int comparekernpair(const void *a, const void *b) {
	const struct tagKERNPAIR *kern1;
	const struct tagKERNPAIR *kern2;

	kern1 = (const struct tagKERNPAIR *) ((char *) a);
	kern2 = (const struct tagKERNPAIR *) ((char *) b);

	if (sortonname) {	/* sort on character name */		/* 97/Oct/30 */
		int ret;
		ret = strcmp(charnames[kern1->a], charnames[kern2->a]);
		if (ret != 0) return ret;
		else return strcmp(charnames[kern1->b], charnames[kern2->b]);
	}
	else {				/* sort on character code */
		if (kern1->a < kern2->a) return -1; 
		else if (kern1->a > kern2->a) return 1;
		if (kern1->b < kern2->b) return -1; 
		else if (kern1->b > kern2->b) return 1;
		return 0;	/* same kern pair */
	}
}

void sortkerning (void) {
	printf("Sorting %d kern pairs\n", kernindex);
	qsort((void *) kernpairs, kernindex, sizeof(struct tagKERNPAIR),
		  comparekernpair);
}

void freeencoding (void) {
	int k;
	for (k=1; k < MAXNAMES; k++) {
		if (*(charnames[k]) != '\0') free(charnames[k]);
	}
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

int main (int argc, char *argv[]) {
	char infilename[FILENAME_MAX];
	FILE *input;
	int k, count, firstarg=1;
	
/*	while (*(argv[firstarg]) == '-') */
	while (firstarg < argc && *(argv[firstarg]) == '-') {
		if (strcmp(argv[firstarg], "-F") == 0)
			leftcompositesflag=!leftcompositesflag;
		if (strcmp(argv[firstarg], "-G") == 0)
			rightcompositesflag=!rightcompositesflag;
		if (strcmp(argv[firstarg], "-r") == 0)
			roundflag=!roundflag;
		firstarg++;
	}

	if (argc < firstarg+1) exit(1);

	for (k=1; k < MAXNAMES; k++) charnames[k]="";

	strcpy(infilename, argv[firstarg]);
	extension(infilename, "afm");
	input = fopen(infilename, "r");
	if (input == NULL) {
		perror(infilename);
		exit(1);
	}

	readkernfile(input);
	fclose(input);
	printf("Kern pairs reference %d distinct characters\n", charindex);
/*	showcharnames(); */
	makehistogram();
/*	showhistogram(200); */
	showhistogram(30);
	if (roundflag) {
		roundkerning(kerninterval);
		makehistogram();
/*		showhistogram(200); */
		showhistogram(30);
	}
	if (leftcompositesflag) {
		count = tagleftcomposites();
		printf("Found %d leftcomposites\n", count);
	}
	if (rightcompositesflag) {
		count = tagrightcomposites();
		printf("Found %d rightcomposites\n", count);
	}
	count = removetagged();
	printf("Removed %d tagged kern pairs, leaving %d\n", count, kernindex);
	sortkerning();
	makehistogram();
	showhistogram(30);	
/*	if(traceflag) */
		showkernfile(stdout);

	if (argc == firstarg+2) {
		strcpy(infilename, argv[firstarg+1]);
		extension(infilename, "afm");
		input = fopen(infilename, "r");
		if (input == NULL) {
			perror(infilename);
			exit(1);
		}
		comparekernfile(input);
		fclose(input);
	}

	freeencoding();
	return 0;
}


/* Use -F to remove pairs with left member accented */
/* The add these back in in AFMtoTFM using -F flag there */
