/* newcopy.c stick in new copyright into PFM files 1996/Jun/20 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h>

#define MAXFILENAME 80

#define BUFFERLEN 128

#define MAXCOPY 60

char buffer[BUFFERLEN];

int verboseflag=1;

int usenewflag=1;

int useamsflag=1;

int keepdateflag=0;

char oldcopyright[MAXCOPY] = "\
Copyright 1996 Y&Y, Inc. All Rights Reserved (978) 371-3286"; /* 60 chars */

char newcopyright[MAXCOPY] = "\
Copyright 1996 Y&Y Inc. (978) 371-3286 http://www.YandY.com"; /* 60 chars */

/*  Copyright 1996 Y&Y Inc. (508) 371-3286 http://www.YandY.com"; */

char amscopyright[MAXCOPY] = "\
Copyright (c) 1997 American Mathematical Society."; /* 60 chars */

/*****************************************************************************/

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

char *stripname(char *name) {
	char *s;
	if ((s = strrchr(name, '\\')) != NULL) return (s+1);
	if ((s = strchr(name, ':')) != NULL) return (s+1);
	else return name;
}

/*****************************************************************************/

char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char newyear[6];

	strcpy (newyear, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = newyear[k];
	date[4] = ' ';
	return;
}

struct _stat statbuf;		/* struct stat statbuf;	 */

struct _utimbuf timebuf;	/* struct utimbuf timebuf; */

int getinfo(char *filename, int verboseflag) {
	char *s;
	int result;

	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(stderr, "Unable to obtain info on %s\n", filename);
		return -1;
	}
	s = ctime(&statbuf.st_atime);		/* ltime */
	lcivilize(s);								
	if (verboseflag != 0) printf("%s last modified: %s", filename, s);
	return 0;
}

int domodification (char *filename, int verboseflag) {
	if (verboseflag != 0)
		printf("Will now try and modify time of %s\n", filename);
	if (_utime(filename, &timebuf) != 0) {
		fprintf(stderr, "Unable to modify date/time\n");
		perror(filename);
		return -1;
	}
	if (verboseflag) printf("Modified date and time of %s\n", filename);
/*	if (getinfo(filename, traceflag) < 0) return -1; */
	if (getinfo(filename, verboseflag) < 0) return -1;
	return 0;
}

int modifycopy(char *buffer) {				/* modify copyright */
	char *s=buffer;
	char year[]="1997";						/* default if not found */
	if (verboseflag) printf("OLD: %s\n", buffer);
	if (keepdateflag) {
		while ((s = strstr(s, "19")) != NULL) {	
			strncpy(year, s, 4);				/* use last date found */
			s += 4;
		}
	}
	if (useamsflag) strncpy(buffer, amscopyright, 60);
	else if (usenewflag) strncpy(buffer, newcopyright, 60);
	else strncpy(buffer, oldcopyright, 60);

	if (keepdateflag) {
		if ((s = strstr(buffer, "19")) != NULL)
			strncpy(s, year, 4);
	}
	if (verboseflag) printf("NEW: %s\n", buffer);
	return 1;
}

int main(int argc, char *argv[]) {
	char infilename[MAXFILENAME];
	char outfilename[MAXFILENAME];
	FILE *input, *output;
	int m;
	int flag=0;
	size_t nlen;
/*	time_t timenow, targettime, sourcetime; */

	for (m = 1; m < argc; m++) {
		strcpy(infilename, argv[m]);	/* first check if exists */
		extension(infilename, "pfm");
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;
		}
		else fclose(input);
		if (verboseflag) printf("Now working on %s\n", infilename);
		getinfo(infilename, verboseflag);
		strcpy(outfilename, infilename);	/* rename to *.bak */
		forceexten(infilename, "bak");
/*		if (rename(outfilename, infilename) != 0) { */
		if (rename(outfilename, stripname(infilename)) != 0) {
			printf("Unable to rename %s to %s\n", outfilename,
				   stripname(infilename));
			continue;
		}
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;
		}
		nlen = fread(buffer, 1, 6, input);	/* header */
		if (*buffer != 0 || *(buffer+1) != 1) {
			printf("%s does not appear to be a PFM file\n", infilename);
			fclose(input);
			continue;
		}
		if ((output = fopen(outfilename, "wb")) == NULL) {
			perror(outfilename);
			fclose(input);
			continue;
		}
		fwrite(buffer, 1, nlen, output);
		nlen = fread(buffer, 1, 60, input);	/* read copyright */
		modifycopy(buffer);					/* modify copyright */
		fwrite(buffer, 1, nlen, output);
		while ((nlen = fread(buffer, 1, BUFFERLEN, input)) > 0) {
			if (nlen == 0 && ferror(input)) {
				perror(infilename);
				break;
			}
			 if (fwrite(buffer, 1, nlen, output) < nlen) {
				 perror(outfilename);
				 flag = 1;
				 break;
			 }
		}
		fclose(output);
		fclose(input);

		timebuf.actime = statbuf.st_atime;
		timebuf.modtime = statbuf.st_atime;

		domodification (outfilename, verboseflag);
		if (verboseflag) printf ("Deleting %s\n", infilename);
		if (remove(infilename) != 0) {
			printf("Unable to delete the file %s\n", infilename);
			continue;
		}
	}

	return flag;
}