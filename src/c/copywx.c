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

/* Copy metrics WX from EC AFM files to EM AFM files */

/* args: DM / EM file and EC file */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXCHARNAME 128
#define MAXLINE 256

char line[MAXLINE];

int verboseflag=1;
int traceflag=0;

int mismatch, notfound;

/************************************************************************************/

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

/* strips file name, leaves only path */

void stripname(char *file) {
	char *s;
	if ((s = strrchr(file, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(file, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(file, ':')) != NULL) *(s+1) = '\0';	
	else *file = '\0';
}

/* return pointer to file name - minus path - returns pointer to filename */

char *removepath(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/************************************************************************************/

#define MISSING -1234.0

double findchar(FILE *in, char *name) {
	int chr;
	double wx;
	char charname[MAXCHARNAME];

	rewind(in);
	while (fgets(line, sizeof(line), in) != NULL) {
		if (strncmp(line, "StartCharMetrics", 16) == 0) break;
	}
	while (fgets(line, sizeof(line), in) != NULL) {
		if (strncmp(line, "EndCharMetrics", 14) == 0) break;
		if (sscanf(line, "C %d ; WX %lg ; N %s ;", &chr, &wx, charname) == 3)
		{
			if (strcmp(charname, name) == 0) return wx;
		}
	}
	return MISSING;
}

/* double epsilon=1.0; */

double epsilon=1.5;

int processlinesub(FILE *ec, int chr, double wx, char *charname, char *realname) {
	double wxnew, diff;
	if ((wxnew = findchar(ec, charname)) != MISSING) {
		diff = wxnew - wx;
		if (diff > -epsilon && diff < epsilon) {
			sprintf(line, "C %d ; WX %lg ; N %s ; \n", chr, wxnew, realname);
			if (traceflag)
				printf("         %d %s\t%lg\t%lg\n", chr, realname,wx, wxnew);
		}
		else {
			sprintf(line, "C %d ; WX %lg ; N %s ; \n", chr, wx, realname);
			printf("Mismatch %d %s\t%lg\t%lg\n", chr, realname,wx, wxnew);
			mismatch++;
		}
		return 1;
	}
	else return 0;
}

int processline(FILE *ec, int chr, double wx, char *charname) {
	if (processlinesub(ec, chr, wx, charname, charname) != 0) return 1;
	if (strcmp(charname, "brokenbar") == 0)
		return processlinesub(ec, chr, wx, "bar", charname);
	if (strcmp(charname, "Dcroat") == 0)
		return processlinesub(ec, chr, wx, "Eth", charname);
	if (strcmp(charname, "periodcentered") == 0)
		return processlinesub(ec, chr, wx, "period", charname);
	if (strcmp(charname, "quotereversed") == 0)
		return processlinesub(ec, chr, wx, "quoteright", charname);
	if (strcmp(charname, "quotedblreversed") == 0)
		return processlinesub(ec, chr, wx, "quotedblright", charname);
	if (strcmp(charname, "commaaccent") == 0)
		return processlinesub(ec, chr, wx, "s", charname);
	if (strcmp(charname, "scommaaccent") == 0)
		return processlinesub(ec, chr, wx, "s", charname);
	if (strcmp(charname, "tcommaaccent") == 0)
		return processlinesub(ec, chr, wx, "t", charname);
	if (strcmp(charname, "Scommaaccent") == 0)
		return processlinesub(ec, chr, wx, "S", charname);
	if (strcmp(charname, "Tcommaaccent") == 0)
		return processlinesub(ec, chr, wx, "T", charname);
	return 0;
}

void copymetrics (FILE *out, FILE *dm, FILE *ec) {
	int chr;
	double wx;
	char charname[MAXCHARNAME];

	mismatch=0;
	notfound=0;
	rewind(dm);
	while (fgets(line, sizeof(line), dm) != NULL) {
		fputs(line, out);
		if (strncmp(line, "StartCharMetrics", 16) == 0) {
			if (traceflag) printf(line);
			break;
		}
	}
	while (fgets(line, sizeof(line), dm) != NULL) {
		if (strncmp(line, "EndCharMetrics", 14) == 0) {
			if (traceflag) printf(line);
			fputs(line, out);
			break;
		}
		if (sscanf(line, "C %d ; WX %lg ; N %s ;", &chr, &wx, charname) == 3)
		{
			if (traceflag) printf(line);
			if (processline(ec, chr, wx, charname) == 0) {
				sprintf(line, "C %d ; WX %lg ; N %s ; \n", chr, wx, charname);
				if (verboseflag) printf("%s\tnot found\n", charname);
				notfound++;
			}
		}
		fputs(line, out);
	}
	while (fgets(line, sizeof(line), dm) != NULL) {
		fputs(line, out);
	}
}

int main(int argc, char *argv[]) {
	char ecfilename[FILENAME_MAX];
	char dmfilename[FILENAME_MAX];
	char outfilename[FILENAME_MAX];
	FILE *ec, *dm, *out;

	if (argc < 3) {
		printf("copywx [DM AFM file name] [EC AFM file name]\n");
		exit(1);
	}
	strcpy(dmfilename, "d:\\doug\\fonts\\");
	if (_strnicmp(argv[1], "dm", 2) != 0) printf(argv[1]);
	strcat(dmfilename, argv[1]);
	extension(dmfilename, "afm");
	if ((dm = fopen(dmfilename, "r")) == NULL) {
		perror(dmfilename);
		exit(1);
	}
	if (_strnicmp(argv[2], "ec", 2) != 0) printf(argv[2]);
	strcpy(ecfilename, "d:\\dc\\");
	strcat(ecfilename, argv[2]);
	extension(ecfilename, "afm");
	if ((ec = fopen(ecfilename, "r")) == NULL) {
		perror(ecfilename);
		exit(1);
	}
	strcpy(outfilename, removepath(argv[1]));
	forceexten(outfilename, "mod");
	if ((out = fopen(outfilename, "w")) == NULL) {
		perror(outfilename);
		exit(1);
	}
	printf("%s and %s => %s\n", dmfilename, ecfilename, outfilename);
	copymetrics(out, dm, ec);
	printf("%d not found\t%d mismatches\n", notfound, mismatch);
	fclose(out);
	fclose(ec);
	fclose(dm);
	return 0;
}
