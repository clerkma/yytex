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

/* replicate kerns for composites from base */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 256
#define MAXNAME 128

#define MAXCHRS 384

char line[MAXLINE];

char chara[MAXNAME], charb[MAXNAME];

double kpx;

char *composites[] = {

	"Aacute", 
	"Abreve", 
	"Acircumflex", 
	"Adieresis", 
	"Agrave", 
	"Aogonek", 
	"Aring", 
	"Atilde", 
	"AE", 

	"Cacute", 
	"Ccaron", 
	"Ccedilla", 

	"Dcaron",			/* ??? */
	"Dcroat",			/* ??? */
	"Eth",				/* D ??? */

	"Eacute", 
	"Ecaron", 
	"Ecircumflex", 
	"Edieresis", 
	"Egrave", 
	"Eogonek",
/*	"Eng", *			/* N ??? */	

	"Gbreve", 

	"Iacute", 
	"Icircumflex", 
	"Idieresis", 
	"Idotaccent",
	"Igrave", 

	"Lacute",
	"Lcaron",
	"Lslash",			/* ??? */

	"Ncaron",
	"Nacute",
	"Ntilde",

	"Oacute", 
	"Ocircumflex", 
	"Odieresis", 
	"Ograve", 
	"Ohungarumlaut", 
	"Otilde", 
	"Oslash", 
	"OE", 

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

	"",

	"aacute", 
	"abreve", 
	"acircumflex", 
	"adieresis", 
	"agrave", 
	"aogonek", 
	"aring", 
	"atilde", 
	"ae", 
	
	"cacute", 
	"ccaron", 
	"ccedilla", 
	
	"dcaron",			/* ??? */
	"dcroat",			/* ??? */
	"eth",				/* d ??? */
	
	"eacute", 
	"ecaron", 
	"ecircumflex", 
	"edieresis", 
	"egrave", 
	"eogonek",
	"eng",				/* n ??? */
	
	"gbreve", 
	
	"iacute", 
	"icircumflex", 
	"idieresis", 
	"idotaccent",
	"igrave", 
	
	"lacute",
	"lcaron",
	"lslash",			/* ??? */
	
	"ncaron",
	"nacute",
	"ntilde",
	
	"oacute", 
	"ocircumflex", 
	"odieresis", 
	"ograve", 
	"ohungarumlaut", 
	"otilde", 
	"oslash", 
	"oe", 
	
	"racute",
	"rcaron",
	
	"sacute",
	"scaron",
	"scedilla", 
	"scommaaccent",
	
	"tcaron", 
	"tcedilla", 
	"tcommaaccent",
	
	"uacute", 
	"ucircumflex", 
	"udieresis", 
	"ugrave", 
	"uhungarumlaut", 
	"uring", 
	
	"yacute", 
	"ydieresis", 
	
	"zacute",
	"zcaron",
	"zdotaccent",
	
	""

};

/************************************************************************/

char *strdup(char *str) {
	char *new;
	new = (char *) malloc(strlen(str) + 1);
	return new == NULL ? new : strcpy(new, str);
}

int isinlist (char *name, char *lst[]) {
	int k;
	for (k = 0; k < 256; k++) {
		if (*lst[k] == '\0') break;
/*		if (strcmp(name, lst[k]) == 0) return 1; */
		if (_stricmp(name, lst[k]) == 0) return 1;
	}
	return 0;
}



int scanafm(FILE *output, FILE *input) {
/*	double baseyur, accentyll; */
	int k=0;
	int kernpairs=0;
	double pi = 3.141592653;
	
	while (fgets(line, sizeof(line), input) != NULL) {
		fputs(line, output);
		if (strncmp(line, "StartCharMetrics", 16) == 0) break;
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		fputs(line, output);
		if (strncmp(line, "StartKernPairs", 14) == 0) break;
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (strncmp(line, "EndKernPairs", 12) == 0) break;
		if (sscanf(line, "KPX %s %s %lg", chara, charb, &kpx) < 3) {
			printf("ERROR: %s", line);
		}
		else {
			if (isinlist(chara, composites)) continue;
			if (isinlist(charb, composites)) continue;
			fputs(line, output);
			kernpairs++;
		}
	}
	fputs(line, output); 
	while (fgets(line, sizeof(line), input) != NULL) {
		fputs(line, output);
		if (strncmp(line, "EndFontMetrics", 14) == 0) break;
	}
	return kernpairs;
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

int main(int argc, char *argv[]) {
	char infilename[FILENAME_MAX];
	char outfilename[FILENAME_MAX];
	FILE *input, *output;
	int m, kernpairs;

	for (m = 1; m < argc; m++) {
		strcpy(infilename, argv[m]);
		extension(infilename, "afm");
		if ((input = fopen(infilename, "r")) == NULL) {
			perror(infilename);
			exit(1);
		}
		strcpy(outfilename, removepath(argv[m]));
		forceexten(outfilename, "stp");
		if ((output = fopen(outfilename, "w")) == NULL) {
			perror(outfilename);
			exit(1);
		}
		printf("File %s => %s\n", infilename, outfilename);
		kernpairs = scanafm(output, input);
		fclose(output);
		fclose(input);
		printf("%d kernpairs\n", kernpairs);
	}
	return 0;
}
