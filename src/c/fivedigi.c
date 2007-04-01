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

#define MAXLINE 256

/* int minnum=10024; */
/* long minnum=10000; */
/* long minnum=9000; */
/* long minnum = 1073742336 - 100000; */
unsigned long minnum = 10001;
/* int maxnum=14000; */
/* long maxnum=99999;  */
/* long maxnum=9999; */
/* long maxnum = 1073742336 + 100000; */
/* unsigned long maxnum = 2147483647; */
/* unsigned long maxnum = 4294967295; */
unsigned long maxnum = 10024;

int showline=0;
int eqtbflag=0;
int ignorecomments=1;
int numberonly=0;

#define MAXTABLE 10000

int tableinx=0;

unsigned long numtable[MAXTABLE];

/* char separators=" \t"; */
char *separators=" \t=,;()<>[]{}-+/%~!^&|*";

int findnum (unsigned long num) {
	int k;
	for (k = 0; k < tableinx; k++) {
		if (numtable[k] == num) return 1;
	}
	return 0;
}

void insertnum (unsigned long num) {
	if (findnum(num)) return;
	numtable[tableinx++] = num;
	if (tableinx == MAXTABLE) {
		fprintf(stderr, "Table Overflow\n");
		exit(1);
	}
}

void sort_table (void) {
	int k, l;
	unsigned long temp;
	for (k = 0; k < tableinx; k++) {
		for (l = 0; l < tableinx - k; l++) {
			if (numtable [l+1] < numtable [l]) {
				temp = numtable[l+1];
				numtable[l+1] = numtable[l];
				numtable[l] = temp;
			}
		}
	}
}

void show_table(void) {
	int k;
	printf("\n");
	for (k = 0; k < tableinx; k++) {
		printf("%lu\n", numtable[k]);
	}
}

int main (int argc, char *argv[]) {
	FILE *input;
	char filename[MAXLINE];
	char line[MAXLINE];
	char buffer[MAXLINE];
	int linenum;
	int flag;
	int firstarg=1;
	int k;
	unsigned long num;
	char *s;
	
	if (argc < 2) exit(1);
/*	while (*(argv[firstarg]) == '-') { */
	while (firstarg < argc && *(argv[firstarg]) == '-') {
		if (strcmp(argv[firstarg], "-l") == 0) showline=0;
		if (strcmp(argv[firstarg], "-e") == 0) eqtbflag=1;
		if (strcmp(argv[firstarg], "-c") == 0) ignorecomments=1;
		firstarg++;
	}
	for (k = firstarg; k < argc; k++) {
		if (strchr(argv[k], '\\') == 0) strcpy(filename, "c:\\y&ytex\\");
		else strcpy (filename, "");
		strcat(filename, argv[k]);
		if (strchr(filename, '.') == NULL) strcat(filename, ".c");
		if ((input = fopen(filename, "r")) == NULL) {
			perror(filename);
			exit(1);
		}
		linenum=0;
	while (fgets(line, MAXLINE, input) != NULL) {
		linenum++;
		if (ignorecomments && strncmp(line, "/*", 2) == 0) continue;
		if (strpbrk(line, "0123456789") == NULL) continue;
		flag = 0;
		strcpy(buffer, line);
		s = strtok(buffer, separators);
		while (s != NULL) {
			if (*s >= '0' && *s <= '9') {
				num = atol(s);
				if (num >= minnum && num <= maxnum) {
					flag = 1;
					break;
				}				
/*				if (- num >= minnum && - num <= maxnum) {
					flag = 1;
					break;
				} */
			}
			s = strtok(NULL, separators);
		}
		if (eqtbflag) {
			if (strstr(line, "eqtbextra") != NULL) flag = 0;
		}
		if (flag) {
			if (numberonly)	{
				printf("%lu\n", num);
				insertnum (num);
			}
			else if (showline)
				printf("%s(%d) : %s", filename, linenum, line);
			else
				printf("%s: %s", filename, line);
		}
	}
	fclose(input);
	}
	if (numberonly) {
		sort_table();
		show_table();
	}
	return 0;
}
