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

/* This is used to compare two C files made by WEB2C with different */
/* settings for hash_size and hash_prime */
/* It produces an output file where numeric constant involving */
/* hash_size and hash_prime are replaced with symbol references */

long hash_size_a = 25000;			/* NEW */
long hash_size_b = 9500;			/* OLD */
long hash_prime_a = 21247;			/* NEW */
long hash_prime_b = 7919;			/* OLD */

long size_diff = 25000 - 9500;				/* 15500 */
long prime_diff = 21247 - 7919;				/* 13328 */

int modify=1;						/* make modifications */

int traceflag=1;

char buffer[256];
char linea[256];
char lineb[256];

int nochange=0;
int	sizechange=0;
int	primechange=0;

void showerror (char *msg, char *linea, char *lineb) {
	fputs(msg, stderr);
	fputs(linea, stderr);
	fputs(lineb, stderr);
}

char *replace (char *s, long val, char *name) {
	long num, diff;
	int n;
	char *t;
	int longflag=0;
/*	char buffer[256]; */

	if (sscanf(s, "%ld%n", &num, &n) == 0) {
		fprintf(stderr, "Say what?\n");
	}
	t = s + n;
	if (*t == 'L') {
		longflag++;
		t++;						/* step over L */
	}
/*	strcpy(buffer, s+n); */
	strcpy(buffer, t);
	if (traceflag) fputs(s, stdout);
	diff = num - val;
	if (diff == 0) sprintf(s, "%s", name);
/*	else if (diff > 0) sprintf(s, "(%s + %ld)", name, diff); */
/*	else if (diff < 0) sprintf(s, "(%s - %ld)", name, -diff); */
	else sprintf(s, "(%s %s %ld%s)", name, (diff > 0) ? "+" : "-",
				 (diff > 0) ? diff : -diff, longflag ? "L" : "");
	t = s + strlen(s);
	strcat(s, buffer);
	if (traceflag) fputs(s, stdout);
	return t;
}

int compareline (char *linea, char *lineb) {
	char *sa=linea;
	char *sb=lineb;
	int count=0, na, nb;
	long numa, numb, ndiff;
	
	while (*sa != '\0' && *sb != '\0') {
		if (*sa != *sb) {
/* step back to nearest white space (or beginning of line) */			
/*			while (sa > linea && *sa > ' ') sa--; */
			while (sa > linea && *sa >= '0' && *sa <= '9') sa--;
/*			while (sb > lineb && *sb > ' ') sb--; */
			while (sb > lineb && *sb >= '0' && *sb <= '9') sb--;
/*			if (sa > linea && (*sa == '-' || *sa == '+')) sa--; */
/*			if (sb > linea && (*sb == '-' || *sb == '+')) sb--; */
/*			if (*sa <= ' ') sa++; */
/*			if (*sa < '0' || *sa > '9') sb++; */
			if ((*sa < '0' || *sa > '9') && (*sa != '-' && *sa != '+')) sa++;
/*			if (*sb <= ' ') sb++; */
/*			if (*sb < '0' || *sb > '9') sb++; */
			if ((*sb < '0' || *sb > '9') && (*sb != '-' && *sb != '+')) sb++;
			if ((sa > linea && *(sa-1) > ' ') ||
				(sb > lineb && *(sb-1) > ' ')) {
				showerror("WARNING: Possible problem here!\n", linea, lineb);
			}
			na = sscanf (sa, "%ld", &numa);
			nb = sscanf (sb, "%ld", &numb);
			if (na == 0 || nb == 0) {
				showerror("WARNING: Non-numeric difference\n", linea, lineb);
			}
			else {
				if (numa < 0 || numb < 0) {
					showerror("WARNING: Negative values\n", linea, lineb);
				}
				ndiff = numa - numb;	/* expect a larger than b */
				if (ndiff == size_diff) {
					if (modify)
						sa = replace (sa, hash_size_a, "hash_size");
					sizechange++;
				}
				else if (ndiff == prime_diff) {
					if (modify)
						sa = replace (sa, hash_prime_a, "hash_prime");
					primechange++;	
			}
				else {
					showerror("WARNING: Non-standard difference\n", linea, lineb);
				}
				count++;
			}
/* step forward to nearest white space (or end of line) */			
			if (*sa == '-' || *sa == '+') sa++;
/*			while (*sa != '\0' && *sa > ' ') sa++; */
			while (*sa != '\0' && *sa >= '0' && *sa <= '9') sa++;
			if (*sa == 'L') sa++;
			if (*sb == '-' || *sb == '+') sb++;
/*			while (*sb != '\0' && *sb > ' ') sb++; */
			while (*sb != '\0' && *sb >= '0' && *sb <= '9') sb++;
			if (*sb == 'L') sb++;
		}
		else {
			sa++; sb++;
		}
	}
	if (*sa == '\0' && *sb != '\0') {
		showerror("WARNING: Line A shorter than line B\n", linea, lineb);
	}
	if (*sa != '\0' && *sb == '\0') {
		showerror("WARNING: Line B shorter than line A\n", linea, lineb);
	}
	return count;
}

int comparefiles(FILE *filea, FILE *fileb, FILE *output) {
/*	char linea[256]; */
/*	char lineb[256]; */
	int numeric=0, n;

	nochange=0;
	sizechange=0;
	primechange=0;

	for(;;) {
		if (fgets(linea, sizeof(linea), filea) == NULL) {
			if (fgets(lineb, sizeof(lineb), fileb) != NULL) {
				fprintf(stderr, "A ends before B!\n");
				fputs(lineb, stderr);
			}
			break;
		}
		if (fgets(lineb, sizeof(lineb), fileb) == NULL) {
			if (fgets(linea, sizeof(linea), filea) != NULL) {
				fprintf(stderr, "B ends before A!\n");
				fputs(linea, stderr);
			}
			break;
		}
		if (strcmp(linea, lineb) == 0) {
			if (output != NULL) fputs(linea, output);
			nochange++;
			continue;
		}
		n = compareline(linea, lineb);
		numeric += n;
		if (output != NULL) fputs(linea, output);
	}
	return numeric;
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

void addpath(char *filename, char *pathname) {
	char *s;
	if (strcmp(filename, "") == 0) return;
	if (strchr(filename, '\\') != NULL ||
		strchr(filename, '/') != NULL ||
		strchr(filename, ':') != NULL) return;
	strcpy(buffer, filename);
	strcpy(filename, pathname);
	s = filename + strlen(filename) - 1;
	if (*s != '\\' && *s != '/') strcat(filename, "\\");
	strcat(filename, buffer);
}

int main(int argc, char *argv[]) {
	char filenamea[128], filenameb[128];
	char filenamec[128]="";
	FILE *inputa, *inputb;
	FILE *output=NULL;
	int n;
	int firstarg=1;

	size_diff = hash_size_a - hash_size_b;
	prime_diff = hash_prime_a - hash_prime_b;

	if (argc < firstarg+1) exit(1);
	if (argc >= firstarg+2) {
		strcpy (filenamea, argv[firstarg]);
		strcpy (filenameb, argv[firstarg+1]);
		if (argc == firstarg+3) strcpy(filenamec, argv[firstarg+2]);
	}
	else {
		strcpy(filenamea, argv[firstarg]);
		strcpy(filenameb, argv[firstarg]);
		strcpy(filenamec, argv[firstarg]);
	}
	addpath(filenamea, "d:\\texsourc\\large");
	extension(filenamea, "c");
	addpath(filenameb, "d:\\texsourc\\small");
	extension(filenameb, "c");
	addpath(filenamec, "d:\\texsourc");
	extension(filenamec, "c");

	if ((inputa = fopen(filenamea, "r")) == NULL) {
		perror(filenamea); exit(1);
	}
	if ((inputb = fopen(filenameb, "r")) == NULL) {
		perror(filenameb); exit(1);
	}
	if (strcmp(filenamec, "") != 0) {
		if ((output = fopen(filenamec, "r")) != NULL) {
			fprintf(stderr, "File %s already exists!\n", filenamec);
			exit(1);
		}
		else fclose(output);
		if ((output = fopen(filenamec, "w")) == NULL) {
			perror(filenamec); exit(1);
		}
	}
	printf("Large hash: %s, Small hash: %s\nOutput: %s\n",
		   filenamea, filenameb, filenamec);
/*	output = NULL; */
	n = comparefiles(inputa, inputb, output);
/*	printf("Noted %d numeric differences\n", n); */
	printf("Passed %d lines unchanged, %d hash_size, %d hash_prime changes\n",
		   nochange, sizechange, primechange);
	if (output != NULL) fclose(output);
	fclose(inputb);
	fclose(inputa);
	return 0;
}
