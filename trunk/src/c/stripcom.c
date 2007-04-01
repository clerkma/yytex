/* Copyright 1990, 1991, 1992 Y&Y, Inc.
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

/* Program for stripping comments from files & stripping control M */
/* copy file data and time of access/modification from source file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>

/* #define FNAMELEN 80 */
#define MAXLINE 1024

char buffer[MAXLINE];

int verboseflag = 0;
int traceflag = 0;

int linestrip = 0;		/* strip whole line comments */
int inlineflag = 0;		/* strip comments inside a line */
int commentflag = 0;	/* strip comments inside lines starting `Comment' */

int stripcontrolm = 0;	/* strip out control M's at end of line */
int checkcontrol = 0;	/* check for bad control characters */

int deleteback = 0;		/* delete old file if same name */

/* char *startchars="%"; */	/* all characters counted as comment characters */

char startchar = '%';

/* char *inlinechars="%"; *//* all characters counted as comment characters */

char inlinechar = '%';

int destflag = 0;

char *destination="";

int linestripped;
int inlinestrip;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

struct _stat statbuf;	/* struct stat statbuf;	 */

struct _utimbuf timebuf;	/* struct utimbuf timebuf; */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int getinfo(char *filename, int flag) {
	char *s;
	int result;

	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(stderr, "Unable to obtain info on %s\n", filename);
		return -1;
	}
	s = ctime(&statbuf.st_atime);		/* ltime */
	lcivilize(s);								
	if (flag != 0) printf("%s last modified: %s", filename, s);
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int stripcomments (char *line) {
	char *s;

/*	if (linestrip != 0 && strchr(startchars, *buffer) != NULL) { */
	if (linestrip != 0 && *buffer == startchar) {
		linestripped++;
		*buffer = '\0';
		return -1;
	}
	else if (inlineflag != 0) {
		if (commentflag == 0 && strncmp(line, "Comment ", 8) == 0)
			return 0;
/*		if ((s = strpbrk(line+1, inlinechars)) != NULL) { */
		if ((s = strchr(line+1, inlinechar)) != NULL) {
			inlinestrip++;
			*s++ = '\n';
			*s++ = '\0';
		}
	}
	return 0;
}

char *oklist="\n\r\t\f";		/* not \v\b\a */

int scancontrol (char *line) {
	char *s;
	int c;
	int count=0;

	s = line;
	while ((c = *s++) != 0) {
		if (c < 32) {
			if (strchr(oklist, c) == NULL) {
				fprintf(stderr, " C-%c", c+64);
				s--; *s++ = ' ';		/* replace with space ... */
				count++;
			}
		}
	}
	if (count != 0) fprintf(stderr, "Saw %d bad control characters\n", count);
	return count;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *stripname (char *filename) {
	char *s;
	if ((s = strrchr(filename, '\\')) != NULL) s++;
	else if ((s = strrchr(filename, '/')) != NULL) s++;	
	else if ((s = strrchr(filename, ':')) != NULL) s++;	
	else s = filename;
	return s;
}
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* -l to strip comment lines */
/* -i to strip comments inside line */
/* -p to strip comments off Comment lines */
/* -r to strip control M at end of line */
/* -d to specify destination directory for output file */

void showusage(int argc, char *argv[] ){
	printf("%s [-{v}{l}{i}{p}{r}] [-d <directory>] <source>\n", argv[0]);
	printf("\tv  Verbose mode\n");
	printf("\tl  Strip comment lines\n");
	printf("\ti  Strip comments inside lines\n");
	printf("\tp  Strip comments inside lines that start with `Comment'\n");
	printf("\tr  Remove control-M before control-J\n");
	printf("\tc  Check for bad control characters\n");
	printf("\tf  Flush old file if output in same directory\n");
	printf("\td  Next argument is destination directory\n");
	exit(3);
}

int main(int argc, char *argv[]) {
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX], bakfilename[FILENAME_MAX];
	FILE *input, *output;
	int m, firstarg = 1;
	int backflag;		/* whether had to rename in file */
	char *s;

	if (argc < firstarg + 1) showusage(argc, argv);
	
/*	while (*argv[firstarg] == '-') */
	while (firstarg < argc && *argv[firstarg] == '-') {
		if (strchr(argv[firstarg], '?') != NULL) showusage(argc, argv);
		if (strchr(argv[firstarg], 'v') != NULL) verboseflag = 1;
		if (strchr(argv[firstarg], 't') != NULL) traceflag = 1;
		if (strchr(argv[firstarg], 'l') != NULL) linestrip = 1;
		if (strchr(argv[firstarg], 'i') != NULL) inlineflag = 1;
		if (strchr(argv[firstarg], 'p') != NULL) commentflag = 1;
		if (strchr(argv[firstarg], 'r') != NULL) stripcontrolm = 1;
		if (strchr(argv[firstarg], 'c') != NULL) checkcontrol = 1;
		if (strchr(argv[firstarg], 'f') != NULL) deleteback = 1;
		if (strchr(argv[firstarg], 'd') != NULL) {
			firstarg++;
			destination = argv[firstarg];
		}
		firstarg++;
	}

	for (m = firstarg; m < argc; m++) {
		backflag=0; linestripped=0; inlinestrip=0;
		strcpy(infilename, argv[m]);
		if (strcmp(destination, "") != 0) {
			strcpy(outfilename, destination);
			strcat(outfilename, "\\");
			strcat(outfilename, stripname(argv[m]));
		}
		else strcpy(outfilename, stripname(argv[m]));
		if (strcmp(infilename, outfilename) == 0) {
			strcpy (bakfilename, infilename);
			if ((s = strrchr(infilename, '.')) != NULL) {
				strcpy (s, ".bak");
				remove (infilename);
				rename (bakfilename, infilename);
				backflag++;
			}
		}
		if (traceflag != 0) printf("Opening %s and %s\n", 
			infilename,	outfilename);
		if ((input = fopen(infilename, "r")) == NULL) {
			perror(infilename); exit(1);
		}
		if (stripcontrolm != 0) {
			if ((output = fopen(outfilename, "wb")) == NULL) {
				perror(outfilename); exit(1);
			}
		}
		else {
			if ((output = fopen(outfilename, "w")) == NULL) {
				perror(outfilename); exit(1);
			}
		}
		if (traceflag != 0) printf("Stripping %s\n", infilename);
		while (fgets(buffer, MAXLINE, input) != NULL) {
			if (stripcomments (buffer) == 0) {
				if (checkcontrol != 0) scancontrol(buffer);
				if (fputs(buffer, output) == EOF) {
					perror(outfilename); break;
				}
			}
		}

		if (traceflag != 0) printf("Closing %s and %s\n", 
			infilename, outfilename);
		fclose(input);
		if (ferror(output) != 0) {
			perror(outfilename); exit(1);
		}
		else fclose(output);

		if (verboseflag != 0) {
			if (linestripped > 0 || inlinestrip > 0) {
				printf("Stripped ");
				if (linestripped > 0) 
					printf("%d complete lines ", linestripped);
				if (inlinestrip > 0) 
					printf("%d comments in lines ", inlinestrip); 
				putc('\n', stdout);
			}				
		}
		if (traceflag != 0) printf("Copying date and time\n");
		if (getinfo(infilename, verboseflag) < 0) {
			exit(1);
		}
	
		timebuf.actime = statbuf.st_atime;
		timebuf.modtime = statbuf.st_atime;
		if (_utime(outfilename, &timebuf) != 0) {
			fprintf(stderr, "Unable to modify date/time\n");
			perror(outfilename);
/*			exit(3); */
		}

/*		see if it worked */
		if (getinfo(outfilename, traceflag) < 0) exit(1);
		
		if (backflag != 0 && deleteback != 0)
			remove(infilename);
	}
	return 0;
}
