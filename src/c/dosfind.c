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

/* Test code to step through all sub-directories */

/* Test code to check searching for file along path list */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dos.h>				/* for _dos_findfirst/next */
#include <io.h>					/* for access */

/* #define FNAMELEN 80 */

#define ACCESSCODE 4

#define SEPARATOR "\\"

int traceflag=1;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int findsubpath (char *filename, char *pathname, int recurse);	/* later */

/* Our searchalongpath is (somewhat) analogous to DOS _searchenv */
/* The name of the desired file is given in `filename' */
/* The list of paths is given in `pathlist' */
/* searchalongpath returns the full pathname of first match in `pathname' */
/* (make sure there is enough space there!) */
/* If the file is not found, then pathname contains "" */
/* and it also returns non-zero if it fails. */
/* It first searches in the current directory (control by flag?) */
/* If a path in `pathlist' ends in \, then its sub-directories are searched, */
/* (after the specified directory) */
/* If a path in `pathlist' ends in \\, then this works recursively */
/* (which may be slow and cause stack overflows ...) */

int searchalongpath (char *filename, char *pathlist, char *pathname, int current) {
/*	struct find_t c_file; *			/* need to preserve across calls to DOS */
	char *s, *t, *u;
	int c, n;
	int code;
	int recurse;
	
	if (current) {	/*	Try for exact match in current directory first ? */
		strcpy(pathname, filename);
		if (traceflag) printf("Trying: %s\n", pathname);
		if (access(pathname, ACCESSCODE) == 0) {	/* check for read access */
			if (traceflag) printf("SUCCESS: %s\n", pathname);
			return 0;							/* success */
		}
		if (traceflag) {							/* debugging */
			code = access(pathname, ACCESSCODE);
			printf("File %s Access %d\n",
				pathname, access(pathname, ACCESSCODE));
		}
	}

/*	Now step through paths in pathlist */
	s = pathlist;
	for (;;) {
		if (*s == '\0') break;				/* sanity check */
		if ((t = strchr(s, ';')) == NULL)
			t = s + strlen(s);				/* if last path */
		n = t - s;
		strncpy(pathname, s, n);
		u = pathname + n;
		*u-- = '\0';						/* null terminate */
		c = *u;								/* check whether ends on \ */
		if (c == '\\' || c == '/') {		/* yes it does */
			*u-- = '\0';					/* remove it */
			c = *u;							/* check whether ends on \\ */
			if (c == '\\' || c == '/') {	/* yes it does */
				*u-- = '\0';				/* remove it */
				recurse = 1;				/* recursive subdir search */
			}
			else recurse = 0;
			if (traceflag) printf("Trying subdir: %s\n", pathname);
			if (findsubpath (filename, pathname, recurse) == 0)
				return 0;	/* success */
		}
		else {									/* its just a directory */
			strcat (pathname, SEPARATOR);		/* \ or / */
			strcat (pathname, filename);
			if (traceflag) printf("Trying: %s\n", pathname);
			if (access (pathname, ACCESSCODE) == 0) {
				if (traceflag) printf("SUCCESS: %s\n", pathname);
				return 0;						/* success */
			}
			if (traceflag) {					/* debugging */
				code = access(pathname, ACCESSCODE);
				printf("File %s Access %d\n",
					pathname, access(pathname, ACCESSCODE));
			}
		}

		s = t;						/* move on to next item in list */
		if (*s == ';') s++;			/* step over separator */
		else break;					/* we ran off the end */
	}
	strcpy(pathname, "");			/* failed to find it */
	return -1;
}

/* This is the (recursive) sub-directory searching code */

/* We come in here with the directory name in pathname */
/* Which is also where we plan to return the result when we are done */
/* We search in this directory first, */
/* and we search one level subdirectories (if it ended in \), */
/* or recursively if the flag is set (when it ends in \\). */
/* Return non-zero if not found */

int findsubpath (char *filename, char *pathname, int recurse) {
	struct find_t c_file;				/* preserve across calls to DOS */
	char *s;
	int code;

	s = pathname + strlen(pathname);	/* remember the end of the dirpath */
	if (traceflag) printf("Entering findsubpath: %s %s %d\n",
		filename, pathname, recurse);

/*	first try directly in this directory (may want this conditional) */
	strcat (pathname, SEPARATOR);		/* \ or / */
	strcat (pathname, filename);

/*	Try for match in this directory first - precedence over files in subdirs */
	if (access(pathname, ACCESSCODE) == 0) { 		/* check for read access */
		if (traceflag) printf("SUCCESS: %s\n", pathname);
		return 0;						/* success */
	}
	if (traceflag) {					/* debugging */
		code = access(pathname, ACCESSCODE);
		printf("File %s Access %d\n", pathname, access(pathname, ACCESSCODE));
	}

	*s = '\0';						/* strip off the filename again */
/*	no luck, so try and find subdirectories */
	strcat (pathname, SEPARATOR);	/* \ or / */
	strcat (pathname, "*.*");
	if (_dos_findfirst (pathname, _A_NORMAL | _A_SUBDIR, &c_file) != 0) {
		perror(filename);			/* debugging only ??? bad path given */
		return -1;					/* failure */
	}
	*s = '\0';						/* strip off the \*.* again */
	for (;;) {
/*		now ignore all but sub-directories - also ignore . and .. */
		if ((c_file.attrib & _A_SUBDIR) == 0 ||	*c_file.name == '.') {
			if (_dos_findnext (&c_file) != 0) break;
			continue;
		}

/*		extend pathname with subdir name */
		strcat(pathname, SEPARATOR);
		strcat(pathname, c_file.name);
		if (traceflag) printf("Checking subdir: %s\n", pathname);
/*		OK, now try for match in this directory */
		if (recurse) {
			if (findsubpath(filename, pathname, recurse) == 0)
				return 0;						/* succeeded */
		}
		else {									/* not recursive */
			strcat (pathname, SEPARATOR);
			strcat (pathname, filename);
			if (traceflag) printf("Checking file: %s\n", pathname);
			if (access(pathname, ACCESSCODE) == 0) { /* check read access */
				if (traceflag) printf("SUCCESS: %s\n", pathname);
				return 0;						/* success */
			}
			if (traceflag) {							/* debugging */
				code = access(pathname, ACCESSCODE);
				printf("File %s Access %d\n",
					pathname, access(pathname, ACCESSCODE));
			}
		}

/*		No match in this directory, so continue */
		*s = '\0';
		if (traceflag) printf("Ready for dos_findnext: %s %s %d\n",
			filename, pathname, recurse);
		if (_dos_findnext (&c_file) != 0) break;
	}
	return -1;									/* failed */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main (int argc, char *argv[]) {
	int firstarg=1;
 	char pathname[FILENAME_MAX];
	int n;
	
	if (argc < firstarg + 2) {
		printf("%s <pathlist> <filename>\n", argv[0]);
		exit(1);
	}

	n = searchalongpath (argv[firstarg+1], argv[firstarg], pathname, 1);
	if (n == 0) {
		printf("Found: %s\n", pathname);
	}
	else {
		printf("Failed to find %s in %s\n",
			argv[firstarg+1], argv[firstarg]);
	}

	return 0;
}

/* File attribute constants */

/* #define _A_NORMAL	0x00 */	/* Normal file - No read/write restrictions */
/* #define _A_RDONLY	0x01 */	/* Read only file */
/* #define _A_HIDDEN	0x02 */	/* Hidden file */
/* #define _A_SYSTEM	0x04 */	/* System file */
/* #define _A_VOLID		0x08 */	/* Volume ID file */
/* #define _A_SUBDIR	0x10 */	/* Subdirectory */
/* #define _A_ARCH		0x20 */	/* Archive file */

#ifdef IGNORE
	if (argc < firstarg + 1) exit(1);

	strcpy (filename, argv[firstarg]);
	strcat (filename, SEPARATOR);
	strcat (filename, "*.*");

	if (_dos_findfirst (filename, _A_NORMAL | _A_SUBDIR, &c_file) != 0) {
		perror(filename);
		exit(1);
	}

	for (;;) {
		printf("File: %s (%ld bytes)\tattributes: %0X ",
			c_file.name, c_file.size, c_file.attrib);
		if ((c_file.attrib & _A_ARCH) != 0) printf("ARCH ");
		if ((c_file.attrib & _A_HIDDEN) != 0) printf("HIDDEN ");
		if ((c_file.attrib & _A_RDONLY) != 0) printf("RDONLY ");
		if ((c_file.attrib & _A_SUBDIR) != 0) printf("SUBDIR ");
		if ((c_file.attrib & _A_SYSTEM) != 0) printf("SYSTEM ");
		if ((c_file.attrib & _A_VOLID) != 0) printf("VOLID ");
		putc('\n', stdout);
		if (_dos_findnext (&c_file) != 0) break;
	}
	putc('\n', stdout);
	perror(filename);
	return 0;
}
#endif

/*	access codes */
/*	0 check existence only */
/*	2 write permission */
/*	4 read permission */
/*	6 read and write permission */

/* what about dirs with '.' ? */

/* what about files without extension ? */

/* what about top level dirs ? */
