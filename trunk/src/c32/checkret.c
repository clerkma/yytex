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
#include <io.h>  		/* for _access and _findfirst, _findnext, _findclose */

#define BUFSIZE 4096

char buffer[BUFSIZE];

int verboseflag=1;
int traceflag=0;
int debugflag=0;

int recurseflag=0;

int ignoreflag=1;

int useasterisks = 1;

int usepattern = 0;

int nsubdirs = 0;

int checkcount=0;

int ignored=0;

char infile[FILENAME_MAX];

char filename[FILENAME_MAX];	/* pattern given on command line e.g. *.* */

struct _finddata_t c_file;		/* make global to avoid typing up stack space*/

FILE *errout = stdout;

char *ignoreext[]=
{"exe", "obj", "zip", "gif", "doc", "tfm", "dll", "dvi", "vf", "pfb", "pfm", ""};

// MZ 144

int checkreturns (FILE *input, char *filename) {
	int c, i, n;
	long current;
	char *s;

	for (;;) {
		n = fread(&buffer, 1, sizeof(buffer), input);
		if (debugflag)
			printf("ftell %d n %d\n", ftell(input), n);
		if (n == 0) return 0;
		s = buffer;
		for (i = 0; i < n; i++) {
			c = *s++;
			if (c == '\r') {
				current = ftell(input) - n + i;
				printf("%s\tcontains <return> at byte %ld\n", filename, current);
				return -1;
			}
		}
	}
}

void perrormod (char *s) {
//	perror (s);
	fprintf(errout, "%s: %s\n", s, strerror(errno));
}


int checkext (char *filename) {
	char *s;
	int i;
	s = strrchr(filename, '.');
	if (s == NULL) return 0;
	for (i = 0; i < 32; i++) {
		if (ignoreext[i][0] == '\0') break;
		if (_stricmp(s+1, ignoreext[i]) == 0) return -1;
	}
	return 0;
}

int checkfile (char *filename) {
	FILE *input;
	int flag=0;
	if (ignoreflag) {
		if (checkext(filename) != 0) {
			ignored++;
			return 0;
		}
	}
	input = fopen(filename, "rb");
	if (input == NULL) {
		perrormod(filename);
		return -1;
	}
	flag = checkreturns(input, filename);
	checkcount++;
	fclose(input);
	return flag;
}

/* removes file name from path - path does not include separator at end */

void stripfilename(char *path) {	/* truncate full filename */
	char *s;
	if ((s = strrchr(path, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(path, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(path, ':')) != NULL) *(s+1) = '\0';
	else *path = '\0';
}

int recurse (char *name) {
	char path[FILENAME_MAX];		/* this part needs to be kept on stack */
	long hFind;						/* pointer to findfirst/next data */
	int ret;

	if (*name == '\0') {			/* sanity check */
		printf("ERROR: Called recurse checkret with blank file name\n");
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
			if (traceflag) {
				if (*path == '\0')
					printf("Found   %s %x\n",  c_file.name, c_file.attrib);
				else printf("Found   %s %x (in %s)\n",
							c_file.name, c_file.attrib, path);
			}
			strcpy(infile, path);
			if (*path != '\0') strcat(infile, "\\");
			strcat(infile, c_file.name);

			if ((c_file.attrib & _A_SUBDIR) == 0) {
				if (strcmp(c_file.name, ".") != 0 &&
					strcmp(c_file.name, "..") != 0) {
					if (traceflag) printf("Trying %s\n", infile);
					checkfile(infile);
					if (traceflag) printf("\n");
				}
			}

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
				nsubdirs++;
				strcpy(infile, path);
/*				if (*path != '\0' && *filename != '\0') */
				if (*path != '\0') strcat(infile, "\\");
				strcat(infile, c_file.name); 
				strcat(infile, "\\");
				if (useasterisks) strcat(infile, "*.*");
				else strcat(infile, filename);
				recurse(infile);	/* recursive call */
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


int commandline (int argc, char *argv[], int firstarg) {
	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (strncmp(argv[firstarg], "-s", 2) == 0) 
			recurseflag = ! recurseflag;
		else if (strncmp(argv[firstarg], "-t", 2) == 0) 
			traceflag = ! traceflag;
		else if (strncmp(argv[firstarg], "-v", 2) == 0) 
			verboseflag = ! verboseflag;
		else if (strncmp(argv[firstarg], "-d", 2) == 0) 
			debugflag = ! debugflag;
		else if (strncmp(argv[firstarg], "-i", 2) == 0) 
			ignoreflag = ! ignoreflag;
		firstarg++;
	}
	return firstarg;
}

void showusage (int argc, char *argv[]) {
	printf("%s [-s] *\n", argv[0]);
	printf("    -s recurse\n");
	exit(1);
}

int main (int argc, char *argv[]) {
	FILE *input;
	int m, firstarg=1;
	
	if (argc < 2) showusage(argc, argv);
	firstarg = commandline(argc, argv, firstarg);
	if (recurseflag) {
		if (firstarg == argc) recurse("*");
		else recurse(argv[firstarg]);
	}
	else {
		for (m = firstarg; m < argc; m++) {
			if (traceflag) printf("File %s\n", argv[m]);
			checkfile(argv[m]);
		}
	}
	printf("Checked %d files, ignored %d files, dealt with %d subdirs\n",
		   checkcount, ignored, nsubdirs);

	return 0;
}
