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

/* tool for massaging PFM files for non-text fonts */
/* changes CharSet=Symbol to CharSet=ANSI */
/* changes copyright to some default 60 character string */
/* changes defaultcharacter to something suitable for font */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHARSET 85

#define PITCHANDFAMILY 90

#define COMMENT 6

#define FIRSTCHAR 95

#define LASTCHAR 96

#define DEFAULTCHAR 97

#define BREAKCHAR 98

#define MAXCOMMENT 60

#define OPENMODE "r+"

char *newcomment="Copyright 1997 Y&Y Inc., Copyright 2007 TeX Users Group";


/***************************************************************************/

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

char *extractfilename(char *pathname) {
	char *s;

	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/************************************************************************/

int zfseek(FILE *file, long npos) {
	if (fseek(file, npos, SEEK_SET) != 0) {
		fprintf(stderr, "Seek to %ld failed\n", npos);
		return -1;
	}
	return 0;
}

void showcomment(FILE *file) {
	int c;
	c = getc(file);
	while (c > 0) {
		putc(c, stdout);
		c = getc(file);		
	}
/*	putc('\n', stdout); */
	putc(' ', stdout);
}

int defaultchar (char *filename) {
	if (_strnicmp(filename, "cm", 2) == 0 ||
		_strnicmp(filename, "em", 2) == 0) {		/* CM and EM fonts */
		if (_strnicmp(filename+2, "sy", 2) == 0) return 178; /* bullet */
		if (_strnicmp(filename+2, "bsy", 3) == 0) return 178; /* bullet */
		if (_strnicmp(filename+2, "mi", 2) == 0) return 58; /* period */
		if (_strnicmp(filename+2, "mib", 3) == 0) return 58; /* period */
		if (_strnicmp(filename+2, "ex", 2) == 0) return 160; /* space */

		if (_strnicmp(filename+2, "tt", 2) == 0) return 32; /* visiblespace */
		if (_strnicmp(filename+2, "sltt", 4) == 0) return 32; /* visiblespace */
		if (_strnicmp(filename+2, "itt", 3) == 0) return 32; /* visiblespace */
		if (_strnicmp(filename+2, "tcsc", 4) == 0) return 32; /* visiblespace */

		if (_strnicmp(filename+2, "r", 1) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "ti", 2) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "sl", 2) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "csc", 1) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "b", 1) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "bx", 2) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "ss", 2) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "ssi", 3) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "dunh", 4) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "fib", 3) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "ff", 2) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "fi", 2) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "fib", 3) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "u", 1) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "vtt", 3) == 0) return 46; /* period */
		if (_strnicmp(filename+2, "tex", 3) == 0) return 32; /* space */
		if (_strnicmp(filename+2, "inch", 4) == 0) return 32; /* space */
	}
	else if (_strnicmp(filename, "logo", 4) == 0) {
		return 32;	/* space */
	}
	else if (_strnicmp(filename, "msam", 4) == 0) {
		return 166;	/* 5 i.e. squaresmallsolid */
	}
	else if (_strnicmp(filename, "msbm", 4) == 0) {
		return 160;	/* space */
	}
	else if (_strnicmp(filename, "eur", 3) == 0) {
		return 58;	/* period */
	}
	else if (_strnicmp(filename, "eus", 3) == 0) {
		return 32;	/* space */
	}
	else if (_strnicmp(filename, "euf", 3) == 0) {
		return 46;	/* period */
	}
	else if (_strnicmp(filename, "wncy", 4) == 0) {
		return 46;	/* period */
	}
	else if (_strnicmp(filename, "lasy", 4) == 0 ||
			 _strnicmp(filename, "ilasy", 5) == 0) {
		return 32;	/* space */
	}
	else if (_strnicmp(filename, "lcmss", 4) == 0 ||
			 _strnicmp(filename, "ilcmss", 5) == 0) {
		return 46;	/* period */
	}
	else if (_strnicmp(filename, "icmtt", 5) == 0) return 32; /* visiblespace */
	else if (_strnicmp(filename, "icmsy", 5) == 0) return 178; /* bullet */
	else if (_strnicmp(filename, "icmmi", 5) == 0) return 58; /* period */
	else if (_strnicmp(filename, "icmex", 5) == 0) return 160; /* space */
	else if (_strnicmp(filename, "line", 4) == 0) {
		return 160;	/* space */
	}
	else if (_strnicmp(filename, "lcircle", 7) == 0) {
		return 160;	/* space */
	}
	else {
		printf("Not CM font %s\n", filename);
		return 32;
	}
	printf("UNKNOWN DEFAULT FOR %s\n", filename);
	return 160;				/* space */
}

int modifyfile(FILE *file, char *filename) {
	int c;
	zfseek(file, CHARSET);
	c = getc(file);
	if (c != 2 && c != 0) printf("Strange CharSet code: %d\n", c);
	printf("%d ", c);
	zfseek(file, CHARSET);
	putc(0, file);			/* CharSet=ANSI */
	zfseek(file, COMMENT);
	showcomment(file);
	zfseek(file, COMMENT);
	fprintf(file, "%s", newcomment);
	zfseek(file, DEFAULTCHAR);	
	c = defaultchar(extractfilename(filename));
/*	printf("default %d (%c)\n", c, c); */
	printf("%d (%c)\n", c, c);
	putc(c, file);
	return 0;
}

void dumpfile(FILE *file) {
	int c;
	zfseek(file, 0);
	c = getc(file);
	while (c >= 0) {
		printf("%d ", c);
		c = getc(file);
	}
	printf("\n", c);
}


int main(int argc, char *argv[]) {
	char filename[FILENAME_MAX];
	FILE *file;
	int m;
	for (m = 1; m < argc; m++) {
		strcpy(filename, argv[m]);
		if (strstr(filename, ".pfm") == NULL) continue;
		if ((file = fopen(filename, OPENMODE)) == NULL) {
			perror(filename);
			continue;
		}
		printf("%s ", filename);
/*		dumpfile(file); */
		modifyfile(file, filename);
		fclose(file);
	}
	return 0;
}
