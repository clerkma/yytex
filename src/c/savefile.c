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

/* Copy newer files onto back up archive */
/* Last argument is destination - rest are files to copy */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <sys\utime.h> 

/* #define FNAMELEN 80 */

/* #define MAXFILENAME 128 */

int verboseflag = 0;

int traceflag = 0;	/* extra output for checking */

int safeflag = 0;	/* don't overwrite existing files if on */

int outputflag = 1;	/* actual do the copying operation */

int copydate = 1;	/* copy date from input file to output */

char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];

/* char destination[FILENAME_MAX]=""; */

char *destination="";

int startchar=0;
int endchar=255;

char *threshold="";			/* threshold date */

int thresholdflag=0;

int year, month, day, hour, minute, second;

int dategiven = 0;

int timegiven = 0;

struct tm *tm;

struct tm newtimemt;

time_t newtime;

int defyear, defmonth, defday;

char *months[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/* Thu Sep 27 06:26:35 1990 => 1990 Sep 27 06:26:35 */

void lcivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 20);
	for (k = 18; k >= 0; k--) date[k+1] = date[k];
/*	date[20] = '\n'; */
/*	date[21] = '\0'; */
	date[20] = '\0'; 
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

/* struct stat statbuf; */

struct _stat statbuf[2];		/* struct stat statbuf[2]; */

struct _utimbuf timebuf;			/* struct utimbuf timebuf; */

/* struct utimbuf timebuf[2]; */

char *timeptr;

int getinfo(char *filename, int flag) {
/*	char *s; */
	int result;

/*	if ((result = stat(filename, &statbuf)) != 0) { */
/*	if ((result = stat(filename, &statbuf[flag])) != 0) { */
	if ((result = _stat(filename, &statbuf[flag])) != 0) {
		if (flag == 0)
			fprintf(stderr, "Unable to obtain info on %s\n", filename);
		return -1;
	}
/*	timeptr = ctime(&statbuf.st_atime);	*/	/* ltime */
	timeptr = ctime(&statbuf[flag].st_atime);		/* ltime */
	lcivilize(timeptr);	
	
/*	printf("%s last modified: %s", filename, timeptr); */
	return 0;
}

/*			while ((c = getc(input)) != EOF) putc(c, output); */

#define BUFLEN 8192

char buffer[BUFLEN];

/* Try and speed up copying using buffer */

void copyfile(FILE *output, FILE *input) {
	size_t n, m;

	for(;;) {
		n = fread(buffer, 1, BUFLEN, input);
		if (ferror(input) != 0) {
			perror(infilename);
			break;
		}
		m = fwrite(buffer, 1, n, output);	
		if (m != n) printf("Wrote only %u when asked to write %u\n", m, n);
		if (ferror(output) != 0) {
			perror(outfilename);
			break;
		}
		if (feof(input) != 0) break;
	}
}

void showusage (int argc, char *argv[]) {
	printf("Usage:\n");
/*	printf("%s [-v] [-s=<s>] [-e=<e>] [-d=<date>] <file-1> ... <destination>", */
printf("%s [-v][-t][-n][-c] [-d=<date>] [-s=<s>] [-e=<e>] [-o=<dest>] <file-1> ... \n",
		  argv[0]);
	printf("\tv verbose mode\n");
	printf("\tt trace mode\n");
	printf("\tn don't overwrite existing files (even if it is older)\n");
	printf("\tc do not copy file creation date from input to output file\n");
	printf("\tz do not write any output (debugging)\n");
	printf("\td threshold date (copy only younger files)\n");
	printf("\ts start - first char of file name\n");
	printf("\te end   - first char of file name\n");
	printf("\to destination drive:\\directory\n");
	exit(1);
}

void readdate(char *date) {
	int temp;

/*	assumes System International order: y-m-d */
	if (sscanf(date, "%d-%d-%d", &year, &month, &day) < 3) {
		if (sscanf(date, "%d:%d:%d", &year, &month, &day) < 3) {
			if (sscanf(date, "%d/%d/%d", &year, &month, &day) < 3) {
				fprintf(stderr, "Don't understand %s\n", date);
				exit(1);
			}
		}
	}
	if (day > 90) {	/* assume wrong order: m-d-y */
		temp = day; day = month; month = year; year = temp;
	}
	if (year == 0) year = 1993;			/* intense laziness */
/*	if (year < 90) year += 90;	*/		/* laziness */
	if (year < 10) year += 90;			/* laziness */
	if (year < 1900) year += 1900;
	if (year < 1990 || year > 1999) fprintf(stderr, "Year %d?\n", year);
	month--;
	if (month < 0 || month > 11) fprintf(stderr, "Month %d?\n", month);
	if (day <= 0 || day > 31) fprintf(stderr, "Day %d?\n", day);

/*	dategiven++; */
}

void readtime(char *times) {
	if (sscanf(times, "%d-%d-%d", &hour, &minute, &second) < 3) {
		if (sscanf(times, "%d:%d:%d", &hour, &minute, &second) < 3) {
			if (sscanf(times, "%d/%d/%d", &hour, &minute, &second) < 3) {
				fprintf(stderr, "Don't understand %s\n", times);
				exit(1);
			}
		}
	}
	if (hour < 0 || hour > 24) fprintf(stderr, "hour %d?\n", hour);
	if (minute <= 0 || minute > 60) fprintf(stderr, "minute %d?\n", minute);
	if (second <= 0 || second > 60) fprintf(stderr, "second %d?\n", second);

/*	timegiven++; */
}

int makenewtime (void) {
	int flag = 0;

/*	Make sure we have valid data more or less */		
	if (year < 1900) year = 1900;
	else if (year > 2099) year = 2099;
	if (month < 0) month = 0;
	else if (month > 11) month = 11;
	if (day < 1) day = 1;
	else if (day > 31) day = 31;
	if (hour > 23) hour = 23;
	else if (hour < 0) hour = 0;
	if (minute > 59) minute = 59;
	else if (minute < 0) minute = 0;
	if (second > 59) second = 59;
	else if (second < 0) second = 0;
	if (verboseflag != 0) {
		printf("%4d %s %02d ", year, months[month], day);
		printf("%02d:%02d:%02d ", hour, minute, second);
		putc('\n', stdout);
	}
	newtimemt.tm_sec = second;
	newtimemt.tm_min = minute;
	newtimemt.tm_hour = hour;
	newtimemt.tm_mday = day;
	newtimemt.tm_mon = month;
	newtimemt.tm_year = year - 1900;
	newtimemt.tm_wday = 0; 
	newtimemt.tm_yday = 0;
	newtimemt.tm_isdst = 0;

	newtime = mktime(&newtimemt);
	if (newtime == -1) {
		fprintf(stderr, "Cannot make time!\n");
		return -1;
	}
	return flag;
}

int startflag=0, endflag=0, dateflag=0, outflag=0;

int decodeflag(int c) {
	if (c == 'v')  {
		verboseflag = 1;
		return 0;
	}
	else if (c == 't')  {
		traceflag = 1;
		return 0;
	}
	else if (c == 'n')  {
		safeflag = 1;
		return 0;
	}
	else if (c == 'z')  {
		outputflag = 0;
		return 0;
	}
	else if (c == 'c')  {
		copydate = 0;
		return 0;
	}
	else if (c == 's') {
		startflag = 1;
		return -1;
	}
	else if (c == 'e') {
		endflag = 1;
		return -1;
	}
	else if (c == 'd') {
		dateflag = 1;
		return -1;
	}
	else if (c == 'o') {
		outflag = 1;
		return -1;
	}
	else fprintf(stderr, "Don't understand coomand line arg %s\n", c);
	return 0;
}

int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (startflag) {
					if (c >= 'A' && c <= 'Z') startchar = c;
					else if (c >= 'a' && c <= 'z') startchar = c - 'a' + 'A';
					startflag=0;
				}
				else if (endflag) {
					if (c >= 'A' && c <= 'Z') endchar = c;
					else if (c >= 'a' && c <= 'z') endchar = c - 'a' + 'A';
					endflag=0;
				}
				else if (dateflag) {
					threshold = s;
					readdate(threshold);
					makenewtime ();
					thresholdflag = 1;
					dateflag=0;
				}
				else if (outflag) {
					destination = s;
					outflag=0;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

char *stripfilename (char *filename) {
	char *s;
	if ((s = strrchr(filename, '\\')) != NULL) s++;
	else if ((s = strrchr(filename, '/')) != NULL) s++;
	else if ((s = strrchr(filename, ':')) != NULL) s++;
	else s = filename;
	return s;
}

/* savefile <file-1> <file-2> <file-3> ... <file-n>  destination-directory */

/* savefile [-o=<destination>] <file-1> <file-2> ... <file-n>  */

int main(int argc, char *argv[]) {
	int k, i, n, c;
	int firstarg=1;
	FILE *input, *output;
/*	char destination[FILENAME_MAX]; */
	char infilecivil[FILENAME_MAX], outfilecivil[FILENAME_MAX];
	time_t infiletime, outfiletime;
	char *s;
	time_t timenow;
/*	time_t targettime, sourcetime; */

/*	First lay in background of current date and time */
	timenow = time (NULL);
	if (timenow == -1) fprintf(stderr, "Time does not exist!\n");
	tm = localtime (&timenow);
	year = tm->tm_year + 1900;
	month = tm->tm_mon;
	day = tm->tm_mday;
	hour = tm->tm_hour;
	minute = tm->tm_min;
	second = tm->tm_sec;

	defyear = year;				/* remember current year */
	defmonth = month;			/* remember current month */
	defday = day;				/* remember current day */

/*	if (argc < firstarg + 2) { */
	if (argc < firstarg + 1) showusage (argc, argv);

	firstarg = commandline(argc, argv, firstarg);

/*	if (argc < firstarg + 2) { */
	if (argc < firstarg + 1) showusage(argc, argv);

/*	if (verboseflag != 0)  */
	if (verboseflag != 0 && (startchar != 0 || endchar != 255)) 
		printf("Start %d (%c) end %d (%c)\n", 
			startchar, startchar, endchar, endchar);

	if (strcmp(destination, "") == 0) {	/* last arg is destination direct ? */
/*		strcpy(destination, argv[argc-1]); */
		destination = argv[argc-1];		/* the old way of doing this */
		argc--;
		printf("WARNING: destination not specified, using %s\n", destination);
	}

	if (verboseflag) printf("Destination is %s\n", destination);

/*	for (k = firstarg; k < argc-1; k++) { */
	for (k = firstarg; k < argc; k++) {

		strcpy(infilename, argv[k]);
		s = stripfilename(infilename);
		c = *s;										/* 1992/Oct/ 11 */
		if (c >= 'a' && c <= 'z') c = c  + 'A' - 'a';
		if (c < startchar || c > endchar) continue;

		strcpy(outfilename, destination);
		strcat(outfilename, "\\");
		strcat(outfilename, s);
	
		if (getinfo(infilename, 0) < 0) continue;
/*		infiletime = statbuf.st_atime; */
		infiletime = statbuf[0].st_atime;
		strcpy(infilecivil, timeptr);

		if (traceflag != 0) printf("Considering file %s\n", infilename);

		if (thresholdflag) {
			if (infiletime < newtime){
				if (verboseflag)
					printf("%s not younger than threshold\n", infilename);
				continue;
			}
		}

		if (getinfo(outfilename, 1) < 0) {
			outfiletime = 0;
			strcpy(outfilecivil, "");			
		} 
		else {
/*			outfiletime = statbuf.st_atime; */
			outfiletime = statbuf[1].st_atime;
			strcpy(outfilecivil, timeptr);
		}

/*		if (outfiletime == 0 || outfiletime < infiletime) { */
		if (outfiletime != 0 && outfiletime >= infiletime) {
			if (traceflag) printf("Not younger than destination\n");
			continue;
		}
		
/*		printf("Copying %s ", infilename); */
		printf("Copying %s ", s);
/*		if (strcmp(outfilecivil, "") != 0) { */
		n = strlen(s);
		for (i = n; i < 14; i++) putc(' ', stdout);
		printf("new: %s   ", infilecivil);
		if (strcmp(outfilecivil, "") != 0) 
			printf("old: %s", outfilecivil);
/*		} */
		printf("\n");
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename);
			continue;
		}
		if (outputflag == 0) {
			printf("Skipping %s\n", outfilename);
			fclose(input);
			continue;
		}
		if (safeflag != 0) { /* temporary, until debugged */
			if ((output = fopen(outfilename, "rb")) != NULL) {
				fclose(output);
				printf("%s already exist\n", outfilename);
				fclose(input);
				continue;
			}
		}
		if ((output = fopen(outfilename, "wb")) == NULL) {
			fclose(input);
			perror(outfilename);
			exit(3);
		}
/*		while ((c = getc(input)) != EOF) putc(c, output); */
		copyfile(output, input);
		if (ferror(input) != 0) {
			perror(infilename);
		}
		fclose(input);
		if (ferror(output) != 0) {
			perror(outfilename);
			exit(5);
		}
		fclose(output);
		if (copydate != 0) {
			timebuf.actime = statbuf[0].st_atime; 
			timebuf.modtime = statbuf[0].st_atime; 
/*			if (utime(argv[k], &timebuf) != 0) {  */
/*			if (utime(outfilename, &timebuf) != 0) {  */
			if (_utime(outfilename, &timebuf) != 0) { 
				fprintf(stderr, "Unable to modify date/time\n");
				perror(argv[1]);
				exit(3);
			} 
		}
/*		} */

/*		if (traceflag != 0) {
			if (getinfo(argv[1]) < 0) exit(1);
		} */
	}
	return 0;
}
