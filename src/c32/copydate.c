/* Copyright 1990, 1991, 1992, 1993, 1994, 1995, 1996, 1997 Y&Y, Inc.
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

/* Copy file data and time of access/modification from another file */
/* First file is the one to be modified - second file is source */

/* This is the 32 bit version */

#include <stdio.h>
#include <stdlib.h>			/* for ctime etc. */
#include <string.h>			/* for strstr etc. */

#include <time.h>			/* for struct tm etc. */
#include <sys\stat.h>		/* for _stat etc. */
#include <sys\utime.h>		/* for _utimbuf etc. */
/* #include <sys\types.h> */		/* for time_t also defined in time.h */

/* #define _stat stat */
/* #define _utime utime */
/* #define _utimbuf utimbuf */

/* #define FNAMELEN 260 */	/* for 32 bit version */

#define MAXLINE 256

int verboseflag = 0;
int compareflag = 0;			/* 1994/May/11 */
int traceflag = 0;
int debugflag = 0;
int showageflag = 0;
int showinfoflag = 0;
int showaccessflag = 0;
int showmodifyflag = 0;
int dategiven = 0;
int timegiven = 0;
int reduceonehour=1;
int frombackup=0;

int civilized=1;

int usefile = 1;

char listfile[FILENAME_MAX]="";
char filename[FILENAME_MAX]="";
char line[MAXLINE];

FILE *errout=stdout;

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

/* reads file statistics into statbuf */

#ifdef IGNORED
int getinfo(char *filename, int verboseflag) {
	char *s;
	int result;

	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(errout, "Unable to obtain info on %s\n", filename);
		return -1;
	}
/*	st_atime ? st_mtime ? st_ctime ? */
	s = ctime(&statbuf.st_mtime);		/* time_t ltime */
/*	if (verboseflag) printf("%s last modified: %s", filename, s); */
	if (civilized) lcivilize(s);								
	if (verboseflag) printf("%s last modified: %s", filename, s);
	return 0;
}
#endif

int getinfo(char *filename, int verboseflag) {
	char *s;
	int result;

	if (traceflag)
		printf("Getting date and time for file %s\n", filename);
	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(errout, "ERROR: Unable to obtain info on %s\n", filename);
		return -1;
	}
	s = ctime(&statbuf.st_ctime);
	if (traceflag) printf("CTIME (of ctime %0X): %s", statbuf.st_ctime, s);
	if (s == NULL) exit(1);
	if (civilized) lcivilize(s);
	if (verboseflag && ! showaccessflag && ! showmodifyflag)
		printf("%s created:       %s", filename, s);
	s = ctime(&statbuf.st_atime);
	if (traceflag != 0) printf("CTIME (of atime %0X): %s", statbuf.st_atime, s);
	if (s == NULL) exit(1);
	if (civilized) lcivilize(s);
	if (verboseflag && showaccessflag)
		printf("%s last accessed: %s", filename, s);
	s = ctime(&statbuf.st_mtime);
	if (traceflag) printf("CTIME (of mtime %0X): %s", statbuf.st_mtime, s);
	if (s == NULL) exit(1);
	if (civilized) lcivilize(s);
	if (verboseflag && showmodifyflag)
		printf("%s last modified: %s", filename, s);
/*	strcpy (filetime, s); */
	return 0;
}

int year, month, day, hour, minute, second;

int defyear, defmonth, defday;

struct tm *tm;

struct tm newtimemt;

time_t newtime;

/* allow for various formats ? */

void readdate(char *date) {
	int temp;

/*	assumes System Internationale order: y-m-d */
	if (sscanf(date, "-d=%d-%d-%d", &year, &month, &day) < 3) {
		if (sscanf(date, "-d=%d:%d:%d", &year, &month, &day) < 3) {
			if (sscanf(date, "-d=%d/%d/%d", &year, &month, &day) < 3) {
				fprintf(errout, "Don't understand %s\n", date);
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
	if (year < 1990 || year > 1999) fprintf(errout, "Year %d?\n", year);
	month--;
	if (month < 0 || month > 11) fprintf(errout, "Month %d?\n", month);
	if (day <= 0 || day > 31) fprintf(errout, "Day %d?\n", day);

	dategiven++;
}

void readtime(char *times) {
	if (sscanf(times, "-t=%d-%d-%d", &hour, &minute, &second) < 3) {
		if (sscanf(times, "-t=%d:%d:%d", &hour, &minute, &second) < 3) {
			if (sscanf(times, "-t=%d/%d/%d", &hour, &minute, &second) < 3) {
				fprintf(errout, "Don't understand %s\n", times);
				exit(1);
			}
		}
	}
	if (hour < 0 || hour > 24) fprintf(errout, "hour %d?\n", hour);
	if (minute <= 0 || minute > 60) fprintf(errout, "minute %d?\n", minute);
	if (second <= 0 || second > 60) fprintf(errout, "second %d?\n", second);

	timegiven++;
}

int readfile(char *file) {
/*	if (sscanf(file, "-f=%s", listfile) < 1) return 0; */
	if (sscanf(file, "-f=%s", &listfile) < 1) return 0;
	else {
		strcpy(listfile, "00Contents");
		printf("copydate -f=00Contents\n");
		return 1;
	}
}

int domodification (char *filename) {
	if (verboseflag != 0)
		printf("Will now try and modify time of %s\n", filename);
	if (_utime(filename, &timebuf) != 0) {
		fprintf(errout, "Unable to modify date/time\n");
		perror(filename);
		return -1;
	}
/*	if (getinfo(filename, traceflag) < 0) return -1; */
	if (getinfo(filename, verboseflag) < 0) return -1;
	return 0;
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
		fprintf(errout, "Cannot make time!\n");
		return -1;
	}
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* -rw-rw-r--   1 14001    mirror      1325 Jun 10 13:40 Makefile.unx */
/* -rw-rw-r--   1 14001    mirror      1666 Jun 10 13:40 bugs.txt */
/* -rw-rw-r--   1 14001    mirror     99783 Sep 26 12:39 changes.txt */

void dofilelist (char *listfile) {
	FILE *input;
	char *s, *t;
	int k, n;
/*	int year=1994, month, day; */
/*	int hour, minute, second=0; */
	
	if (traceflag) printf("Trying to open %s\n", listfile);
	if ((input = fopen(listfile, "r")) == NULL) {
		perror(listfile);
		exit(1);
	}
	while (fgets (line, MAXLINE, input) != NULL) {
		if (strcmp(line, "\n") == 0) continue;
		if (strncmp(line, "total ", 6) == 0) continue; 
		if (strncmp(line, "drw", 3) == 0) continue; 
/*		year = 1994; */
/*		year = 1995; */	/* default year */
		year = defyear;	/* default year */
		second = 0;
		s = line + MAXLINE;
		month = 13;
		for (k = 0; k < 12; k++) {
			if ((t = strstr(line, months[k])) != NULL) {
				if (t < s) {
					s = t;
					month = k;
				}
			}
		}
/*		if month is later than current month, it must be previous year */
		if (month > defmonth) {
			year = year - 1;
			if (traceflag) printf("Decrement year to %d\n", year);
		}

		if (month > 11) {
			fprintf(errout, "BAD MONTH: %s", line);
			continue;
		}
		s += 4;
		if (sscanf(s, "%d%n", &day, &n) < 1) {
			fprintf(errout, "BAD DAY:   %s", line); 
			continue;
		}
		s += n;
		if (sscanf(s, "%d:%d%n", &hour, &minute, &n) < 2) {
/*	Try other form for old dates, where year appears instead of hour:min */
			if (sscanf(s, "%d%n", &year, &n) < 1) {
				fprintf(errout, "BAD TIME:  %s", line);
				continue;
			}
			if (traceflag) printf("Old date year year %d\n", year);
			hour = 0; minute=0;
		}
		s += n;
		if (sscanf(s, "%s", &filename) < 1) {
			fprintf(errout, "BAD FILE:  %s", line);			
			continue;
		}
		if (strcmp(filename, ".") == 0) continue;
		if (strcmp(filename, "..") == 0) continue;		
		s = filename + strlen(filename) - 1;
		if (*s == '*') *s='\0';
		if (*s == '/') continue;	/* sub-directory */
		if (traceflag) printf("WILL TRY: %s %d %d %d %d %d %d\n",
			filename, year, month, day, hour, minute, second);
		if (getinfo(filename, verboseflag) < 0) {
			continue;
/*			exit(1); */
		}
		if (reduceonehour) {
			if (hour > 0) hour = hour - 1;
		}
		makenewtime();
		timebuf.actime = newtime;
		timebuf.modtime = newtime;
		if (domodification(filename) != 0) {
			continue;
/*			exit(1); */
		}
	}
	fclose(input);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage (char *name) {
	printf(
"Usage: %s {-v}{-x}{-s}{-r} [-d=<date>] [-t=<time>] [target] [source1]...\n",
		   name);
	putc('\n', stdout);
	printf("\tv  verbose mode\n");
	printf("\ti  just show file date/time info\n");
	printf("\ta  just show last access date/time\n");
	printf("\ts  show age\n");
	printf("\tr  reduce one hour\n");
	printf("\tb  copy date from backup file (.bak)\n");
	printf("\tx  set non-zero DOS exit code if target older than any source\n");
	putc('\n', stdout);
	printf("\tCOPYDATE -d=93-7-3 -t=23:10:11 [target]  set date on target\n");
	printf("\tCOPYDATE [target] [source]  copy date: target <= source\n");
	printf("\tCOPYDATE -x [target] [source1] [source2] ... sets DOS exit code\n");
	printf("\tCOPYDATE -f=[Unix style directory list]\n");
	putc('\n', stdout);
	printf("\tDefault is COPYDATE -f=00Contents\n");
	exit(1);
}

/* -f=<datefile> */

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

int commandline(int argc, char *argv[], int firstarg) {	
	while (firstarg < argc && *argv[firstarg] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag++;
		if (strcmp(argv[firstarg], "-?") == 0) showusage(argv[0]);
		if (strcmp(argv[firstarg], "-x") == 0) compareflag++;
		if (strcmp(argv[firstarg], "-s") == 0) showageflag++;
		if (strcmp(argv[firstarg], "-i") == 0) showinfoflag++;
		if (strcmp(argv[firstarg], "-a") == 0) showaccessflag++;
		if (strcmp(argv[firstarg], "-m") == 0) showmodifyflag++;
		if (strcmp(argv[firstarg], "-r") == 0) reduceonehour = ! reduceonehour;
		if (strcmp(argv[firstarg], "-b") == 0) frombackup++;
		if (strcmp(argv[firstarg], "-z") == 0) civilized = ! civilized;
		if (strcmp(argv[firstarg], "-*t") == 0) traceflag++; 
		if (strncmp(argv[firstarg], "-t", 2) == 0) readtime(argv[firstarg]);
		if (strncmp(argv[firstarg], "-d", 2) == 0) readdate(argv[firstarg]);
		if (strncmp(argv[firstarg], "-f", 2) == 0) readfile(argv[firstarg]);
		firstarg++;
	}
	if (verboseflag > 1) traceflag++;
	return firstarg;
}


int main(int argc, char *argv[]) {
	time_t timenow;
	int m, k, firstarg=1;
	time_t targettime, sourcetime;
	char bakupfile[FILENAME_MAX];

/*	First lay in background of current date and time */
	timenow = time (NULL);
	if (timenow == -1) fprintf(errout, "Time does not exist!\n");
	else if (debugflag) printf("Time does exist!\n");
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

	if (debugflag) printf("Analyze command line\n");

	if (firstarg >= argc) {
/*		showusage(argv[0]); */
		strcpy(listfile, "00Contents");
		printf("copydate -f=00Contents\n");
	}

	firstarg = commandline(argc, argv, firstarg);

	if (debugflag) printf("Analyzed command line!\n");

/*	if (argc < 2) showusage(argv[0]); */

	if (strcmp(listfile, "") != 0) {
		dofilelist(listfile);
		return 0;
	}

	if (showinfoflag || showaccessflag) {
		for (m = firstarg; m < argc; m++) 
			getinfo(argv[m], verboseflag);
		return 0;
	}


	if (dategiven != 0 || timegiven != 0) {
		makenewtime ();						/* broken out 94/Oct/16 */
		usefile = 0;
	}

	if (frombackup) usefile = 0;

/*	if (traceflag != 0) verboseflag++; */

/*	if (argc < firstarg+2) exit(1); */

	if (argc < firstarg+1 || (argc < firstarg+2 && usefile != 0)) {
		printf("firstarg %d argc %d usefile %d frombackup %d\n",
			   firstarg, argc, usefile, frombackup);
		showusage(argv[0]);
	}

/*	copydate <target-file> <source-file> */

	if (compareflag) {			/* 1994/May/11 */
/*		if target doesn't exist treat it as *older* than source */
		if (getinfo(argv[firstarg], verboseflag) < 0) exit(1);
/*		st_atime ? st_mtime ? st_ctime ? */
		targettime = statbuf.st_mtime;
		for (m = firstarg+1; m < argc; m++) {
/*			if (getinfo(argv[m], verboseflag) < 0) exit(1); */
			if (getinfo(argv[m], verboseflag) < 0) {
/*				if source not found treat as error */
				fprintf(errout, "ERROR: source file %s not found\n", argv[m]);
				continue;
/*				exit(1); */
			}
/*			st_atime ? st_mtime ? st_ctime ? */
			sourcetime = statbuf.st_mtime;
/*			if target is older than source, return non-zero errorlevel */
			if (targettime < sourcetime) {
				if (showageflag) {
					printf("%s is older than", argv[firstarg]);
					for (k = firstarg+1; k < argc; k++) {
						putc(' ', stdout);
						fputs(argv[k], stdout);
					}
					putc('\n', stdout);
				}
				exit(1);
			}
		}
		return 0;	/* none of the source files younger than target */
	}

	if (usefile) {								/* 1993/July/3 */
		if (getinfo(argv[firstarg+1], verboseflag) < 0) exit(1);
/*		st_atime ? st_mtime ? st_ctime ? */
		timebuf.actime = statbuf.st_mtime;
/*		st_atime ? st_mtime ? st_ctime ? */
		timebuf.modtime = statbuf.st_mtime;
		if (domodification(argv[firstarg]) != 0) {
/*			exit(1); */
		}
		return 0;
	}
	
	if (frombackup) {
		for (m = firstarg; m < argc; m++) {
			if (getinfo(argv[m], verboseflag) < 0) continue;
			strcpy(bakupfile, argv[m]);
			forceexten(bakupfile, "bak");
			if (getinfo(bakupfile, verboseflag) < 0) continue;
/*			st_atime ? st_mtime ? st_ctime ? */
			timebuf.actime = statbuf.st_mtime;
/*			st_atime ? st_mtime ? st_ctime ? */
			timebuf.modtime = statbuf.st_mtime;
			if (domodification(argv[m]) != 0) {
/*			exit(1); */
			}
		}
		return 0;
	}

	for (m = firstarg; m < argc; m++) {
		if (getinfo(argv[firstarg], verboseflag) < 0) exit(1);
		timebuf.actime = newtime;
		timebuf.modtime = newtime;
		if (domodification(argv[m]) != 0) {
/*			exit(1); */
		}
	}	
	return 0;
}
