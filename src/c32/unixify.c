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

/* convert line terminations in plain text files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <io.h>  		/* for _access and _findfirst, _findnext, _findclose */

#include <time.h>
#include <direct.h>		/* for _getcwd */

#include <sys\stat.h>
#include <sys\utime.h>

/* #define BUFFERLEN 8192 */
#define BUFFERLEN 4095

char inbuffer[BUFFERLEN+BUFFERLEN];

char outbuffer[BUFFERLEN+BUFFERLEN];

char infile[FILENAME_MAX], outfile[FILENAME_MAX], bakfile[FILENAME_MAX];

char filename[FILENAME_MAX];	/* pattern given on command line e.g. *.* */

char str[FILENAME_MAX];			/* temporary space 98/Jan/26 */

struct _finddata_t c_file;		/* make global to avoid typing up stack space*/

#define PCtoUNIX 1
#define UNIXtoPC 2
#define PCtoMAC 3
#define MACtoPC 4
#define UNIXtoMAC 5
#define MACtoUNIX 6
#define IDENTITY 7

int verboseflag = 0;
int traceflag = 0;
int debugflag = 0;
int outcurrent = 0;
int recurseflag = 0;
int useasterisks = 0;
int usepattern = 0;
int lowername = 0;
int uppercaseflag = 0;
int copydateflag = 1;
int modeflag = IDENTITY;

int errors;

int filecount;

int changed;

char *pattern=NULL;

FILE *errout=stdout;

FILE *input, *output;

/* ************************************************************************ */

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
	date[20] = '\n';
	date[21] = '\0';
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
/*	if (traceflag) printf("LCIVILIZE: %s", date); */	/* already has \n */
	return;
}

struct _stat statbuf;		/* struct stat statbuf;	 */

struct _utimbuf timebuf;	/* struct utimbuf timebuf; */

int getinfo(char *filename, int verboseflag) {
	char *s;
	int result;

	if (verboseflag) printf("Trying to get info on `%s'\n", filename);
	if ((result = _stat(filename, &statbuf)) != 0) {
		fprintf(errout, "Unable to obtain info on `%s'\n", filename);
		return -1;
	}
#ifdef _WIN32
	s = ctime(&statbuf.st_mtime);		/* ltime */
#else
	s = ctime(&statbuf.st_atime);		/* ltime */
#endif
/*	if (traceflag) printf("CTIME:     %s", s); */	/* already has \n */
	lcivilize(s);								
	if (verboseflag != 0) printf("%s last modified: %s", filename, s);
	if (traceflag) printf("size %ld\n", statbuf.st_size);
	return 0;
}



/* ************************************************************************* */

int lowercase (char *name) {
	char *s=name;
	int c, flag=0;

	while ((c = *s) != '\0') {
		if (c >= 'A' && c <= 'Z') {
			c = c + 'a' - 'A';
			flag++;
		}
		*s = (char) c;
		s++;
	}
	return flag;
}

int uppercase (char *name) {
	char *s=name;
	int c, flag=0;

	while ((c = *s) != '\0') {
		if (c >= 'a' && c <= 'z') {
			c = c + 'A' - 'a';
			flag++;
		}
		*s = (char) c;
		s++;
	}
	return flag;
}

/* char *stripname(char *filename) {
	char *s;
	if ((s = strrchr(filename, '\\')) != NULL) return s+1;
	if ((s = strrchr(filename, '/')) != NULL) return s+1;
	if ((s = strrchr(filename, ':')) != NULL) return s+1;
	return filename;
} */

char *extractfilename(char *path) {	/* return pointer to filename part only */
	char *s;
	if ((s = strrchr(path, '\\')) != NULL) return s+1;
	else if ((s = strrchr(path, '/')) != NULL) return s+1;
	else if ((s = strrchr(path, ':')) != NULL) return s+1;
	else return path;
}

void forceexten (char *filename, char *ext) {
	char *s;
	char *t = extractfilename(filename);
	if ((s = strrchr(filename, '.')) != NULL && s > t) {
		strcpy(s+1, ext);
	}
	else {
		strcat(filename, ".");
		strcat(filename, ext);
	}
}


void badcontrol(int c, char *s) {
	long current;
	printf("ERROR: control char: %d (C-%c) ", c, c+64);
	current = ftell(input);
	printf("at byte %ld\n", (current-BUFFERLEN)+(s-inbuffer));
}

void badsequence(int c, char *s) {
	long current;
	printf("ERROR: \\r followed by char: %d not \\n ", c);
	current = ftell(input);
	printf("at byte %ld\n", (current-BUFFERLEN)+(s-inbuffer));
}

/* returns number of bytes in output buffer, unless trouble then -1 */

int processbuffer(char *s, char *t, int nr) {
	char *send = s + nr;
	char *tstart = t;
	int c;
	
	switch (modeflag) {
		case IDENTITY:
			memcpy(t, s, nr);
			t += nr;
			break;
		case UNIXtoMAC:
			while (s < send) {
				c = *s++;
				if (c < 32) {
					if (c == '\n') {
						c = '\r';
						changed++;
					}
					else if (c != '\t' && c != '\f') {
						badcontrol(c, s);	/* even \r is bad ? */
						return -1;
					}
				}
				*t++ = (char) c;
			}
			break;
		case MACtoUNIX:
			while (s < send) {
				c = *s++;
				if (c < 32) {
					if (c == '\r') {
						c = '\n';
						changed++;

					}
					else if (c != '\t' && c != '\f') {
						badcontrol(c, s);	/* even \n is bad ? */
						return -1;
					}
				}
				*t++ = (char) c;
			}			
			break;
		case UNIXtoPC:
			while (s < send) {
				c = *s++;
				if (c < 32) {
					if (c == '\n') {
						*t++ = '\r';
/*						c = '\n'; */
						changed++;
					}
					else if (c != '\t' && c != '\f') {
						badcontrol(c, s);	/* even \r is bad ? */
						return -1;
					}
				}
				*t++ = (char) c;
			}
			break;
		case PCtoUNIX:
			while (s < send) {
				c = *s++;
				if (c < 32) {
					if (c == '\r') {
						c = *s++;
						if (c != '\n') {
							s--;
							badsequence(c, s);
							return -1;
						}
						c = '\n';
						changed++;
					}
					else if (c != '\t' && c != '\f') {
						badcontrol(c, s);	/* even \n (without \r) is bad ? */
						return -1;
					}
				}
				*t++ = (char) c;
			}
			break;
		case MACtoPC:
			while (s < send) {
				c = *s++;
				if (c < 32) {
					if (c == '\r') {
						*t++ = '\r';
						c = '\n';
						changed++;
					}
					else if (c != '\t' && c != '\f') {
						badcontrol(c, s);	/* even \n is bad ? */
						return -1;
					}
				}
				*t++ = (char) c;
			}
			break;
		case PCtoMAC:
			while (s < send) {
				c = *s++;
				if (c < 32) {
					if (c == '\r') {
						c = *s++;
						if (c != '\n') {
							badsequence(c, s);
							return -1;
							s--;
						}
						c = '\r';
						changed++;
					}
					else if (c != '\t' && c != '\f') {
						badcontrol(c, s);
						return -1;
					}
				}
				*t++ = (char) c;
			}
			break;
	}
	return (t - tstart);
}

void complainascii(void) {
	switch(modeflag) {
		case PCtoUNIX:
		case UNIXtoPC:
		case UNIXtoMAC:
			printf("ERROR: not plain ASCII %s file\n", "UNIX");
			break;
		case MACtoPC:
		case MACtoUNIX:
			printf("ERROR: not plain ASCII %s file\n", "MAC");
			break;
		case PCtoMAC:
			printf("ERROR: not plain ASCII %s file\n", "IBM PC");
			break;
		case IDENTITY:
			printf("ERROR: not plain ASCII %s file\n", "");
			break;
	}
}

/* int copyfile(FILE *output, FILE *input) {*/
long copyfile(FILE *output, FILE *input) {
	long nr, nw;		/* size_t nr, nw; */
	long filelength=0;
	char *s;
	int c;

	changed = 0;
	for(;;) {
		nr = fread(inbuffer, 1, BUFFERLEN, input);
		filelength += nr;
		if (nr < BUFFERLEN) {
			if (ferror(input)) {
				perror(infile);
				errors++;
				return -1;
			}
		}
		if (nr == 0) {
			if (traceflag) printf("Read zero bytes -");
			break;
		}

		s = inbuffer+nr-1;
		c = *s++;			/* last char */
		if (modeflag == PCtoUNIX || modeflag == PCtoMAC) {
			if (c == '\r') {
				if (traceflag) printf("Avoiding buffer termination on %d ", c);
				fread(s, 1, 1, input);
				c = *s++;		/* last char */			
				if (traceflag) printf(" next char %d\n", c);
				nr++;
			}
		}
/*		while (c == '\r' || c == '\n') {
			if (traceflag) printf("Avoiding buffer termination on %d ", c);
			fread(s, 1, 1, input);
			c = *s++;
			printf(" next char %d\n", c);
			nr++;
		} */
/*		if (macmodeflag) rettolif(buffer, nr, reverseflag);
		else if (reverseflag) nr = cleanlineend(buffer, nr); */
		nr = processbuffer(inbuffer, outbuffer, nr);
		if (nr < 0) {
			complainascii();
			return -1;					/* failure return */
		}
		nw = fwrite(outbuffer, 1, nr, output);
		if (nw < nr) {
			if (ferror(output)) {
				perror(outfile);
/*				return -1; */
				exit(1);
			}
		}
		if (feof(input)) {
			if (traceflag)
/*				printf("Reached EOF on input (%d < %d) - ", nr, BUFFERLEN);*/
				printf("Reached EOF on input\n");
			break;
		}
/*		if (verboseflag) printf("Out file name: `%s'\n", outfile); */
	}
/*	if (verboseflag) printf("Output file length %ld bytes\n", filelength);*/
/*	return 0; */
	return filelength;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* need direct.h for this */

int samefile (char *infilename, char *outfilename) {	/* 95/Mar/1 */
	char cwd[FILENAME_MAX]; /* more space than needed */
	char *s;
	if (traceflag)
		printf("Comparing in: `%s' and out: `%s'\n", infilename, outfilename);
/*	if (strcmp(infilename, outfilename) == 0) return 1; */
	if (_stricmp(infilename, outfilename) == 0) return 1;
	_getcwd(cwd, sizeof(cwd));
	s = cwd + strlen(cwd) - 1;
/*	strcat(cwd, "\\"); */
	if (*s != '\\') strcat(cwd, "\\"); 
	strcat(cwd, outfilename);
/*	uppercase(cwd, cwd); */		/* 97/Dec/26 */
	if (traceflag)
		printf("Comparing in %s and out %s\n", infilename, cwd);
/*	if (strcmp(infilename, cwd) == 0) return 1; */
	if (_stricmp(infilename, cwd) == 0) return 1;
	return 0;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage (char *argzero) {
	printf("%s [-v][-t] ... <file-1> <file-2> ...\n", argzero);
	printf("\t? showusage\n");
	printf("\tv verbose mode\n");
	printf("\tt trace mode\n");
	printf("\td debug mode (no file writing)\n");
	printf("\tc put output in current directory\n");
	printf("\tl force output file name lower case\n");
	printf("\tu force output file name upper case\n");
	printf("\tr recurse subdirectories\n");
	printf("\ta use *.* in subdirectories\n");
	printf("\tp use specified pattern in subdirectories\n"); 
	printf("\t  wild card arguments welcome\n");
	printf("\tID just copy the file (default)\n");
	printf("\tPU PC to Unix\n");
	printf("\tUP Unix to PC\n");
	printf("\tPM PC to Mac\n");
	printf("\tMP Mac to PC\n");
	printf("\tUM Unix to Mac\n");
	printf("\tMU Mac to Unix\n");
/*	printf("\t  output file appears in current directory\n"); */
	exit(0);
}

int isexecutable (char *infile){
	if (strstr(infile, ".bak") != NULL ||
		strstr(infile, ".BAK") != NULL ||
		strstr(infile, ".zip") != NULL ||
		strstr(infile, ".ZIP") != NULL ||
		strstr(infile, ".exe") != NULL ||
		strstr(infile, ".EXE") != NULL ||
		strstr(infile, ".dll") != NULL ||
		strstr(infile, ".DLL") != NULL ||
		strstr(infile, ".obj") != NULL ||
		strstr(infile, ".OBJ") != NULL ||
		strchr(infile, '.') == NULL) return 1;
	else return 0;
}

void showmode (int modeflag, char *infile) {
/*			if (macmodeflag) {
				if (reverseflag) printf("Macifying a Unix file %s\n", infile);
				else printf("Unixifying a Mac file %s\n", infile);
			}
			else if (reverseflag) printf("DOSifying a Unix file %s\n", infile);
			else printf("Unixifying a DOS file %s\n", infile); */
	switch (modeflag) {
		case IDENTITY:
			printf("COPY FILE %s\n", infile);
			break;
		case PCtoUNIX:
			printf("PC -> UNIX %s\n", infile);
			break;
		case UNIXtoPC:
			printf("UNIX -> PC %s\n", infile);
			break;
		case PCtoMAC:
			printf("PC -> MAC %s\n", infile);
			break;
		case MACtoPC:
			printf("MAC -> PC %s\n", infile);
			break;
		case UNIXtoMAC:
			printf("UNIX -> MAC %s\n", infile);
			break;
		case MACtoUNIX:
			printf("MAC -> UNIX %s\n", infile);
			break;
		default:
			printf("Impossible value of modeflag %d\n", modeflag);
			exit(1);
	}
}

int commandline (int argc, char *argv[]) {
	int k=1;
	char *s;
	while (k < argc && *argv[k] == '-') {
		if (traceflag) printf("k %d arg %s\n", k, argv[k]);
		if (strchr(argv[k], 'p') != NULL) {
			usepattern = !usepattern;
			if ((s = strchr(argv[k], '=')) != NULL) {
				pattern = s+1;
			}
			else {
				k++;
				if (k < argc) pattern = argv[k];
			}
			k++;
			continue;
		}
		if (strchr(argv[k], '?') != NULL) showusage(argv[0]);
		if (strchr(argv[k], 't') != NULL) traceflag = !traceflag;
		if (strchr(argv[k], 'v') != NULL) verboseflag = !verboseflag;
		if (strchr(argv[k], 'd') != NULL) debugflag = !debugflag;
		if (strchr(argv[k], 'r') != NULL) recurseflag = !recurseflag;
		if (strchr(argv[k], 'a') != NULL) useasterisks = !useasterisks;
		if (strchr(argv[k], 'c') != NULL) outcurrent = !outcurrent;
		if (strchr(argv[k], 'l') != NULL) lowername = !lowername;
		if (strchr(argv[k], 'u') != NULL) uppercaseflag = !uppercaseflag;
		if (strchr(argv[k], 'z') != NULL) copydateflag = !copydateflag;
		if (strstr(argv[k], "ID") != NULL) modeflag = IDENTITY;
		if (strstr(argv[k], "UM") != NULL) modeflag = UNIXtoMAC;
		if (strstr(argv[k], "MU") != NULL) modeflag = MACtoUNIX;
		if (strstr(argv[k], "UP") != NULL) modeflag = UNIXtoPC;
		if (strstr(argv[k], "PU") != NULL) modeflag = PCtoUNIX;
		if (strstr(argv[k], "MP") != NULL) modeflag = MACtoPC;
		if (strstr(argv[k], "PM") != NULL) modeflag = PCtoMAC;
		k++;
	}
/*	if (recurseflag) outcurrent=0; */	/* ??? */
	return k;
}

/***************************************************************************/

void copydate(void) {
	long inlength;
	if (getinfo(infile, traceflag) < 0) {
		exit(1);
	}
	timebuf.actime = statbuf.st_atime;
#ifdef _WIN32
	timebuf.modtime = statbuf.st_mtime;
#else
	timebuf.modtime = statbuf.st_atime;
#endif
	inlength = statbuf.st_size;
/*		if (verboseflag) printf("Input file length  %ld bytes\n", inlength); */
	if (verboseflag) printf("Trying to change date/time on `%s'\n", outfile);
	if (_utime(outfile, &timebuf) != 0) {
		fprintf(errout, "Unable to modify date/time on `%s'\n", outfile);
/*			perror(outfile); */
/*			exit(3); */
	}

/*		see if it worked */
	if (getinfo(outfile, traceflag) < 0) {
		exit(1);
	}
}

int processonefile(char *file) {
	int renameflag=0;
	long inlength, outlength;

	strcpy(infile, file);
	if (isexecutable(infile)) {
/*		should really avoid doing directories ... */
		printf("SORRY: don't want to risk processing %s\n", infile);
		return -1;
	}
/*	if (verboseflag) */
	showmode(modeflag, infile);

	if (outcurrent) strcpy(outfile, extractfilename(file));
	else strcpy(outfile, file);

	if (lowername) lowercase(outfile);
	else if (uppercaseflag) uppercase(outfile);

	if (debugflag) return 0;	/* don't actually do it */

	if ((outcurrent == 0) || samefile(infile, outfile)) {
		renameflag=1;
		strcpy(bakfile, infile);
		forceexten(infile, "bak");
		(void) remove(infile);	/* remove old backup copy if any */
		if (verboseflag) printf("Renaming `%s' => `%s'\n", bakfile, infile);
		rename (bakfile, infile);	/* rename infile *.bak */
	}
	if ((input = fopen(infile, "rb")) == NULL) {
		perror(infile);
		return -1;
	}
	if (verboseflag) printf("Have opened `%s' for input\n", infile);
	fseek(input, 0, SEEK_END);
	inlength = ftell(input);
	if (inlength == 0) {
		printf("ERROR: Empty file (zero length) %s\n", infile);
		fclose(input);
		return -1;
	}
	fseek(input, 0, SEEK_SET);
	if ((output = fopen(outfile, "wb")) == NULL) {
		perror(outfile);
		return -1;
	}
	if (verboseflag) printf("Have opened `%s' for output\n", outfile);

	outlength = copyfile (output, input);	/* actually not outlength ... */

	fclose(output);
	fclose(input);
	if (verboseflag) printf("Input file length  %ld bytes\n", inlength); 
	if (outlength > 0) {
/*		try and modify time/date of output file --- 1992/Oct/10 */
		if (copydateflag) copydate();

		outlength = statbuf.st_size;
		if (verboseflag) {
			printf("Output file length %ld bytes\n", outlength);
			if (changed == 0) printf("NOTE: no change in file `%s'\n",	 outfile);
			else printf ("Changed %d line terminations\n", changed);
		}

		if (renameflag) {
			(void) remove(infile);
			if (verboseflag) printf("Deleting `%s'\n", infile);
		}
		filecount++;
	}
	else {		/* file copy failed */
/*		printf("ERROR: not plain ASCII files\n"); */
		if (renameflag) {
			(void) remove (bakfile);
			if (verboseflag)
				printf("Renaming `%s' to `%s' again\n", infile, bakfile);
			rename (infile, bakfile);	/* rename *.bak to infile */
		}
	}
	return 0;
}


/***************************************************************************/

/* removes file name from path - path doe snot include separator at end */

void stripfilename(char *path) {	/* truncate full filename */
	char *s;
	if ((s = strrchr(path, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(path, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(path, ':')) != NULL) *(s+1) = '\0';
	else *path = '\0';
}

int noneed, failed, unixified, subdirs;

int unixifyfiles(char *name) {		/* this may call itself recursively */
	char path[FILENAME_MAX];		/* this part needs to be kept on stack */
	long hFind;						/* pointer to findfirst/next data */
	int ret;

	if (*name == '\0') {			/* sanity check */
		printf("ERROR: Called unixifyfiles with blank file name\n");
		return -1;
	}
	if (traceflag) printf("ATTEMPTING: `%s'\n", name);
	strcpy(infile, name);
	if (strcmp(name, ".") == 0) strcpy(infile, "*.*");	/* user convenience */
	strcpy(path, name);
	stripfilename(path);
	if (traceflag) {
		printf("Remembered path is `%s'\n", path);
		if (useasterisks) printf("Remembered pattern is `*.*'\n");
		else if (usepattern) printf("Remembered pattern is `%s'\n", pattern);
	}
	c_file.attrib =  _A_NORMAL;					/* zero - files only */
/*	c_file.attrib =  _A_NORMAL | _A_SUBDIR;	*/
/*		_A_SUBDIR | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_ARCH */
	hFind = _findfirst (infile, &c_file);
	if (hFind <= 0) {
		if(traceflag) {
/*			printf("FINDFIRST: failed on `%s'\n", infile); */
			printf("No files match: `%s'\n", infile);
			printf("\n");	/* no files match this pattern */
		}
	}
	else { /* found some files */
		for (;;) {			/* findfirst / findnext loop - for files */
/*			if ((c_file.attrib & _A_SUBDIR) != 0) {
				if (traceflag)
					printf("IGNORING:  %s %x\n", c_file.name, c_file.attrib);
			} */		/* can't happen anymore */
			if (traceflag) {
				if (*path == '\0')
					printf("Found   %s %x\n",  c_file.name, c_file.attrib);
				else printf("Found   %s %x (in %s)\n",
							c_file.name, c_file.attrib, path);
			}
			strcpy(infile, path);
			if (*path != '\0') strcat(infile, "\\");
			strcat(infile, c_file.name);
/*			if (currentdir) strcpy(outfile, c_file.name); */
/*			else strcpy(outfile, infile); */
			strcpy(outfile, c_file.name);
/*			if (traceflag) printf("Trying %s => %s\n", infile, outfile); */
			if (outcurrent == 0) {				/* splice in path */
				strcpy(str, outfile);
				strcpy(outfile, path);
				if (*path != '\0') strcat(outfile, "\\");
				strcat(outfile, str);
			}	/* actually, processonefile constructs its own outfile ... */
			if (traceflag) printf("Trying %s => %s\n", infile, outfile);
			
			processonefile(infile);
			if (traceflag) printf("\n");
			ret = _findnext (hFind, &c_file);
			if (ret < 0) {
				if (traceflag) {
					if (*path == '\0') printf("FINDNEXT: found no more %s\n",
											  "files");
					else printf("FINDNEXT: found no more %s in `%s'\n",
								"files", path);
				}
				break;	/* break out of loop if no more */
			}
		} /*end of _findfirst/_findnext for(;;) loop */
	} /* end of _findfirst found something */

	if (recurseflag ==0) return 0;	/* skip out if not recursive */

/*	now do sub-directories in case pattern given */
	strcpy(infile, name);
	if (strcmp(name, ".") == 0) strcpy(infile, "*.*");
	strcpy(path, name);
	stripfilename(path);
/*	an attempt */
	stripfilename(infile);
	if (*infile != '\0') strcat(infile, "\\");
	strcat(infile, "*.*");		/* now replace pattern with *.* */
	if (traceflag) printf("TRYING: `%s' for subdir search\n", infile);
	if (traceflag) {
		printf("Remembered path is `%s'\n", path);
		if (useasterisks) printf("Remembered pattern is `*.*'\n");
		else if (usepattern) printf("Remembered pattern is `%s'\n", pattern);
	}
	c_file.attrib =  _A_NORMAL | _A_SUBDIR;	/* list SUBDIRs also */
/*	_A_SUBDIR | _A_SYSTEM | _A_HIDDEN | _A_RDONLY | _A_ARCH */
	hFind = _findfirst (infile, &c_file);
	if (hFind <= 0) {
		if (traceflag) {
/*			printf("FINDFIRST: failed on `%s'\n", infile); */
			printf("No files or directories match: `%s'\n", infile);
			printf("\n");
		}
		return -1;
	}
	for (;;) {			/* findfirst / findnext loop */
/*		deal only with SUBDIRs */
		if ((c_file.attrib & _A_SUBDIR) != 0) {
/*			this can include . and .. `subdirs */
/*			if (traceflag) printf("Subdir  %s %x\n",
								  c_file.name, c_file.attrib); */
			if (strcmp(c_file.name, ".") != 0 &&
				strcmp(c_file.name, "..") != 0) {
				if (traceflag) printf("SUBDIR:  `%s' %x\n",
									  c_file.name, c_file.attrib);
				subdirs++;
				strcpy(infile, path);
/*				if (*path != '\0' && *filename != '\0') */
				if (*path != '\0') strcat(infile, "\\");
				strcat(infile, c_file.name); 
				strcat(infile, "\\");
				if (useasterisks) strcat(infile, "*.*");
				else if (usepattern) strcat(infile, pattern);
				else strcat(infile, filename);
					unixifyfiles(infile);	/* recursive call */
			}
		}
		ret = _findnext (hFind, &c_file);
		if (ret < 0) {
			if (traceflag) {
				if (*path == '\0') printf("FINDNEXT: found no more %s in \n",
										 "dirs");
				else printf("FINDNEXT: found no more %s in `%s'\n",
							"dirs",	path);
			}
			break;
		}
	} /*end of _findfirst/_findnext for(;;) loop */

	return 0;
}

/***************************************************************************/

int main (int argc, char *argv[]) {
	int firstarg=1;
	int m;
/*	int renameflag; */
/*	char *inmode, *outmode; */
/*	FILE *input, *output; */
/*	char bakfile[FILENAME_MAX]; */
/*	long filelength; */
/*	long inlength, outlength; */

	for (m = 0; m < argc; m++) printf("%d\t%s\n", m, argv[m]);
#ifdef _WIN32
	if (verboseflag) printf("Convert line terminations (32)\n");
#else
	if (verboseflag) printf("Convert line terminations\n");
#endif
	if (argc <= firstarg) showusage(argv[0]);

	modeflag = IDENTITY;					/* default */
	firstarg = commandline(argc, argv);

	filecount = 0;
	if (traceflag) printf("Ready to process %d files\n", argc - firstarg);

	errors = 0;

	for (m=firstarg; m < argc; m++) {
		strcpy(filename, extractfilename(argv[m]));	/* strip path */
		if (*filename == '\0') strcpy(filename, "*.*"); /* default */
/*		if (strcmp(filename, ".") == 0) *filename = '\0'; */
		if (strcmp(filename, ".") == 0) strcpy(filename, "*.*");
		if (traceflag) printf ("Next arg %d: %s (%s)\n", m, argv[m], filename);
/*		processonefile(argv[m]); */
		unixifyfiles(argv[m]);			/* new changed */
		if (traceflag) printf("\n");
		if (errors > 0) exit(1);
	}
	if (verboseflag) {
/* 		if (reverseflag) printf("Processed %d files\n", argc - firstarg); */
		printf("Processed %d files\n", filecount);
	}
/*	if (errors > 0) exit(1); */
	fflush(stdout);
	fflush(stderr);
	return 0;
}

/* io.h: #define _A_NORMAL	0x00 */	/* Normal file - No read/write restrictions */
/* io.h: #define _A_RDONLY	0x01 */	/* Read only file */
/* io.h: #define _A_HIDDEN	0x02 */	/* Hidden file */
/* io.h: #define _A_SYSTEM	0x04 */	/* System file */
/* io.h: #define _A_SUBDIR	0x10 */	/* Subdirectory */
/* io.h: #define _A_ARCH 	0x20 */	/* Archive file */

/* use COMPILEZ to link WITHOUT setargv.obj */
