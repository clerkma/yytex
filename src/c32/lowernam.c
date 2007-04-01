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

/* make file names lower case */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <io.h>			/* for _access and _findfirst, _findnext, _findclose */

int verboseflag=1;
int traceflag=0;
int debugflag=0;		/* don't actually rename */
int uppercaseflag=0;	/* uppercase rather than lower case */
int recurseflag=0;		/* lowercase files in subdirectpries also */
int useasterisks=0;		/* use *.* rather than given filename pattern */
int usepattern = 0;
/* int prependpath=1; */	/* construct new name using saved path info */
int outcurrent = 0;

char *pattern=NULL;

char filename[FILENAME_MAX];	/* pattern given on command line e.g. *.* */

char str[FILENAME_MAX];			/* temporary space 98/Jan/26 */

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

void stripfilename(char *path) {	/* truncate full filename */
	char *s;
	if ((s = strrchr(path, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(path, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(path, ':')) != NULL) *(s+1) = '\0';
	else *path = '\0';
}

char *extractfilename(char *path) {	/* return pointer to filename part only */
	char *s;
	if ((s = strrchr(path, '\\')) != NULL) return s+1;
	else if ((s = strrchr(path, '/')) != NULL) return s+1;
	else if ((s = strrchr(path, ':')) != NULL) return s+1;
	else return path;
}

int noneed, failed, renamed, subdirs;

struct _finddata_t c_file;		/* make global to avoid typing up stack space*/

char infile[FILENAME_MAX], outfile[FILENAME_MAX];	/* avoid stack useage */

/* some redundancy here infile and outfile already set up */

int renameonefile(char *name) {

	strcpy(infile, name);

	if (outcurrent) strcpy(outfile, extractfilename(name));
	else strcpy(outfile, infile);

	if (uppercaseflag) uppercase(outfile);
	else lowercase (outfile);

	if (strcmp(outfile, infile) == 0) {
		noneed++;
		if (verboseflag) printf("No need %s => %s\n", infile, outfile);
		return 0;
	}

	if (traceflag) printf("Renameonefile %s => %s\n", infile, outfile);

	if (strcmp(outfile, c_file.name) == 0) {
		noneed++;
		if (verboseflag) printf("No need %s => %s\n", infile, outfile);
		return 0;
	}
/*	if (traceflag) printf("Trying  %s => %s\n", infile, outfile); */
	if (debugflag) 	return 0;

	if (rename(infile, outfile)) {
		printf("FAILED on %s => %s\n", infile, outfile);
		failed++;
	}
	else {
		if (verboseflag || traceflag)
			printf("Renamed %s => %s\n", infile, outfile);
		renamed++;
	}
	if (verboseflag || traceflag) fflush(stdout);
}


int renamefiles (char *name) {		/* this may call itself recursively */
	char path[FILENAME_MAX];		/* this part needs to be kept on stack */
	long hFind;						/* pointer to findfirst/next data */
	int ret;

	if (*name == '\0') {			/* sanity check */
		printf("ERROR: Called renamefiles with blank file name\n");
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
/*	c_file.attrib =  _A_NORMAL | _A_SUBDIR; */
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
		for (;;) {			/* findfirst / findnext loop */
/*			if ((c_file.attrib & _A_SUBDIR) != 0) {
				if (traceflag) printf("Subdir  %s %x\n",
									  c_file.name, c_file.attrib);
			} */
			if (traceflag) {
				if (*path == '\0')
					printf("Found   %s %x\n",  c_file.name, c_file.attrib);
				else printf("Found   %s %x (in %s)\n",
							c_file.name, c_file.attrib, path);
			}

			strcpy(infile, path);
			if (*path != '\0') strcat(infile, "\\");
			strcat(infile, c_file.name);
			if (outcurrent) strcpy(outfile, c_file.name); 
			else strcpy(outfile, infile);
/*			if (traceflag) printf("Trying %s => %s\n", infile, outfile); */

			if (verboseflag) printf("Trying  %s => %s\n", infile, outfile);

			renameonefile(infile);

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
				renamefiles(infile);	/* recursive call */
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

void showusage (char *argzero) {
	printf("Useage: %s [v][t] ...  [files]\n", argzero);
	printf("\t? showusage\n");
	printf("\tv verbose mode\n");
	printf("\tt trace mode (show more verbage)\n");
	printf("\td debug mode (don't actually rename)\n");
	printf("\tu uppercase names (instead of lowercase)\n");
/*	printf("\tc put output in current directory\n"); */
	printf("\tr recurse subdirectories\n");
	printf("\ta use *.* in subdirectories\n"); 
	printf("\tp use specified pattern in subdirectories\n"); 
	printf("\t  wild card arguments welcome\n");
	exit(1);
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
		if (strchr(argv[k], 'v') != NULL) verboseflag = !verboseflag;
		if (strchr(argv[k], 't') != NULL) traceflag = !traceflag;
		if (strchr(argv[k], 'u') != NULL) uppercaseflag = !uppercaseflag;
		if (strchr(argv[k], 'd') != NULL) debugflag = !debugflag;
		if (strchr(argv[k], 'r') != NULL) recurseflag = !recurseflag;
		if (strchr(argv[k], 'a') != NULL) useasterisks = !useasterisks;
/*		if (strchr(argv[k], 'c') != NULL) outcurrent = !outcurrent; */
		k++;
	}
	return k;
}

int main (int argc, char *argv[]) {
	int m, firstarg=1;

	for (m = 0; m < argc; m++) printf("%d\t%s\n", m, argv[m]);
#ifdef _WIN32
	printf("Make file names all lower case (32)\n");
#else
	printf("Make file names all lower case\n");	
#endif
	noneed = failed = renamed = subdirs = 0;
	if (argc < 2) showusage(argv[0]);
	firstarg = commandline (argc, argv);
	if (argc < firstarg + 1) showusage(argv[0]);
	if (traceflag) printf ("%d arguments\n", argc);
	for (m = firstarg; m < argc; m++) {
		strcpy(filename, extractfilename(argv[m]));	/* strip path */
		if (*filename == '\0') strcpy(filename, "*.*"); /* default */
/*		if (strcmp(filename, ".") == 0) *filename = '\0'; */
		if (strcmp(filename, ".") == 0) strcpy(filename, "*.*");
		if (traceflag) printf ("Next arg %d: %s (%s)\n", m, argv[m], filename);
		renamefiles(argv[m]);
	}
	printf("Renamed %d files, ignored %d files and %d subdirs, failed on %d files\n",
		   renamed, noneed, subdirs, failed);
	return 0;
}


/* io.h: #define _A_NORMAL	0x00 */	/* Normal file - No read/write restrictions */
/* io.h: #define _A_RDONLY	0x01 */	/* Read only file */
/* io.h: #define _A_HIDDEN	0x02 */	/* Hidden file */
/* io.h: #define _A_SYSTEM	0x04 */	/* System file */
/* io.h: #define _A_SUBDIR	0x10 */	/* Subdirectory */
/* io.h: #define _A_ARCH 	0x20 */	/* Archive file */

/* use COMPILEZ to link WITHOUT setargv.obj */
