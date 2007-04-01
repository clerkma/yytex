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

/* make file names lower case --- recursive 32 bit version */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <io.h>			/* for _access and _findfirst, _findnext, _findclose */

int verboseflag=1;
int traceflag=0;
int debugflag=0;		/* don't actually rename */
int uppercaseflag=0;	/* uppercase rather than lower case */
int recurseflag=0;		/* lowercase files in subdirectories also */
int useasterisks=0;		/* use *.* rather than given filename pattern */
int usepattern = 0;
int dontcheckcase = 0;		/* dont check case first */
/* int prependpath=1; */	/* construct new name using saved path info */
int outcurrent = 0;		/* rename to current directory ? what */

char *pattern=NULL;				/* pattern given on command line e.g. *.* */

char filename[FILENAME_MAX];

char str[FILENAME_MAX];			/* temporary space 98/Jan/26 */

FILE *errout=stdout;

/**********************************************************************************/

int lowercase (char *name) {
	char *s=name;
	int c, flag=0;

	while ((c = *s) != '\0') {
		if (c >= 'A' && c <= 'Z') {
			c = c + 'a' - 'A';
			flag++;
		}
		*s++ = (char) c;
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
		*s++ = (char) c;
	}
	return flag;
}

int islowercase (char *name) {
	char *s=name;
	int c;
	while ((c = *s++) != '\0') {
		if (c >= 'A' && c <= 'Z') return 0;
	}
	return 1;
}

int isuppercase (char *name) {
	char *s=name;
	int c;
	while ((c = *s++) != '\0') {
		if (c >= 'a' && c <= 'z') return 0;
	}
	return 1;
}

int iswrongcase (char *name) {
	if (dontcheckcase) return 1;
	if (uppercaseflag) return islowercase(name);
	else return isuppercase(name);
}

void stripfilename(char *path) {	/* truncate full filename - modifies arg */
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

/***************************************************************************/

int noneed, failed, renamed, subdirs;

struct _finddata_t c_file;		/* make global to avoid tying up stack space*/

char infile[FILENAME_MAX], outfile[FILENAME_MAX];	/* avoid stack useage */

/* Try to renaming a single file */

int RenameOneFile(char *name) {
	strcpy(infile, name);
	if (outcurrent) strcpy(outfile, extractfilename(name));
	else strcpy(outfile, name);
	if (uppercaseflag) uppercase(outfile);
	else lowercase (outfile);
	if (strcmp(outfile, infile) == 0) {
		noneed++;
		if (verboseflag) printf("No need %s => %s\n", infile, outfile);
		return 0;
	}
/*	if (strcmp(outfile, c_file.name) == 0) { */
	if (strcmp(outfile, extractfilename(infile)) == 0) {
		noneed++;
		if (verboseflag) printf("No need %s => %s\n", infile, outfile);
		return 0;
	}
	if (traceflag) printf("RenameOneFile %s => %s\n", infile, outfile);
	if (verboseflag || traceflag) fflush(stdout);
	if (debugflag) 	return 0;		/* Don't actually do it */
	if (rename(infile, outfile)) {
		fprintf(errout, "RENAME FAILED on %s => %s\n", infile, outfile);
		failed++;
		return -1;
	}
	else {
		if (verboseflag || traceflag)
			printf("Renamed       %s => %s\n", infile, outfile);
		renamed++;
	}
	return 0;
}

/**********************************************************************************/

char *xstrdup(char *s) {
	char *new = strdup(s);
	if (new == NULL) {
		fprintf(errout, "ERROR: Unable to allocate %d bytes for %s\n",
				strlen(s)+1, s);
		exit(1);
	}
	return new;
}

/**********************************************************************************/

/* stuff for saving context during recursive calls */

#define MAXSTACK 32

int nStackIndex=0;

int nNamesSaved[MAXSTACK];

char **szFileListSaved[MAXSTACK];

int nNames=0;

char **szFileList=NULL;

int FreeFileList (void) {
	int k;
	if (szFileList == NULL) return -1;		/* ERROR */
	for (k = 0; k < nNames; k++)
		if (szFileList[k] != NULL) free (szFileList[k]);
	free(szFileList);
	szFileList = NULL;
	nNames = 0;
	return 0;
}

int AllocateFileList(int nNames) {
	int k;
	int nlen=nNames * sizeof(char *);

	if (szFileList != NULL) FreeFileList();	/* ERROR */
	if (nNames == 0) return -1;
	szFileList = (char **) malloc(nlen);
	if (szFileList == NULL) {
		fprintf(errout, "Failed to allocate %d bytes\n", nlen);
		exit(1);
	}
	for (k = 0; k < nNames; k++) szFileList[k] = NULL;
	return 0;
}

void PushFileList (void) {
	if (nStackIndex >= MAXSTACK) {
		fprintf(errout, "ERROR: Stack overflow\n");
		exit(1);
	}
	if (traceflag) printf("Push File Stack %d\n", nStackIndex);
	nNamesSaved[nStackIndex] = nNames;
	szFileListSaved[nStackIndex] = szFileList;
	nStackIndex++;
	nNames = 0;
	szFileList = NULL;
}

void PopFileList (void) {
	if (nStackIndex <= 0) {
		fprintf(errout, "ERROR: Stack underflow\n");
		exit(1);
	}
	nStackIndex--;
	if (traceflag) printf("Pop File Stack %d\n", nStackIndex);
	nNames = nNamesSaved[nStackIndex];
	szFileList = szFileListSaved[nStackIndex];
}

void decodeattribute(int at) {
	if (at & _A_ARCH) printf("ARCH ");
	if (at & _A_SUBDIR) printf("SUBDIR ");
	if (at & _A_SYSTEM) printf("SYSTEM ");	
	if (at & _A_HIDDEN) printf("HIDDEN ");	
	if (at & _A_RDONLY) printf("RDONLY ");	
	if (at == 0) printf("NORMAL ");	
}

/* count number of files listed in this directory so we can allocate array */

int CountFileNames (char *name, int subdirflag) {
	char path[FILENAME_MAX];		/* this part needs to be kept on stack */
	long hFind;						/* pointer to findfirst/next data */
	int ret;

	nNames = 0;
	if (*name == '\0') {			/* sanity check */
		fprintf(errout, "ERROR: Called %s with blank file name\n",
			   "CountFileNames");
		return -1;
	}
	if (traceflag)
		printf("COUNTING: `%s' (%s)\n", name, subdirflag ? "DIR" : "Files");
	strcpy(infile, name);
/*	if (strcmp(name, ".") == 0) strcpy(infile, "*.*"); */
	if (strcmp(name, ".") == 0) strcat(infile, "\\*.*");
	strcpy(path, name); 
	stripfilename(path);			/* should have filename at end */
/*	if (strcmp(path, ".") == 0) *path = '\0'; */
/*	if (traceflag) {
		printf("Remembered path is `%s'\n", path);
		if (useasterisks) printf("Remembered pattern is `*.*'\n");
		else if (usepattern) printf("Remembered pattern is `%s'\n", pattern);
	} */
	if (subdirflag) c_file.attrib = _A_NORMAL | _A_SUBDIR;
	else c_file.attrib =  _A_NORMAL;
	hFind = _findfirst (infile, &c_file);
	if (hFind <= 0) {
		if (traceflag) {
			printf("No files match: `%s'\n", infile);
			printf("\n");	/* no files match this pattern */
		}
	}
	else {					/* found at least one files */
		for (;;) {			/* findfirst / findnext loop */
/*			if (traceflag) {
				if (*path == '\0')
					printf("Found   %s %x\n",  c_file.name, c_file.attrib);
				else printf("Found   %s %x (in %s)\n",
							c_file.name, c_file.attrib, path);
			} */
/*			count files or count sub-directories ? */
/*			ignore . and .. and correct case already */
			if (subdirflag == 0 ||
				(c_file.attrib & _A_SUBDIR) != 0) {
				if (strcmp(c_file.name, ".") != 0 &&
					strcmp(c_file.name, "..") != 0)
					if (subdirflag || iswrongcase(c_file.name)) {
						if (traceflag) {
							if (*path == '\0')
								printf("Found   %s %x ",  c_file.name, c_file.attrib);
							else printf("Found   %s %x (in %s) ",
										c_file.name, c_file.attrib, path);
							decodeattribute(c_file.attrib);
							printf("\n");
						}
						nNames++;
					}
			}
			ret = _findnext (hFind, &c_file);
			if (ret < 0) {
				if (traceflag) {
					if (*path == '\0')
						printf("FINDNEXT: found %d %s\n", nNames,
							  subdirflag ? "DIR" : "Files");
					else printf("FINDNEXT: found %d %s in `%s'\n", nNames,
						  subdirflag ? "DIR" : "Files", path);
					printf("\n");
				}
				break;	/* break out of loop if no more */
			}
		} /*end of _findfirst/_findnext for(;;) loop */
	} /* end of _findfirst found something */
/*	if (verboseflag)
		printf("Found %d %s listed in `%s'\n", nNames,
			   subdirflag ? "DIR" : "Files", name); */
	return nNames;
}

/* This creates a list of files at this level of the directory */

int CreateFileList (char *name, int nNames, int subdirflag) {
	char path[FILENAME_MAX];		/* this part needs to be kept on stack */
	long hFind;						/* pointer to findfirst/next data */
	int ret;
	int nIndex=0;

	if (*name == '\0') {			/* sanity check */
		fprintf(errout, "ERROR: Called %s with blank file name\n",
			   "CreateFileList");
		return -1;
	}
	if (traceflag) printf("LISTING: `%s'\n", name);
	strcpy(infile, name);
/*	if (strcmp(name, ".") == 0) strcpy(infile, "*.*"); */
	if (strcmp(name, ".") == 0) strcat(infile, "\\*.*");
	strcpy(path, name);
	stripfilename(path);			/* should have filename at end */
/*	if (strcmp(path, ".") == 0) *path = '\0'; */
/*	if (traceflag) {
		printf("Remembered path is `%s'\n", path);
		if (useasterisks) printf("Remembered pattern is `*.*'\n");
		else if (usepattern) printf("Remembered pattern is `%s'\n", pattern);
	} */
	if (subdirflag) c_file.attrib =  _A_NORMAL | _A_SUBDIR;
	else c_file.attrib =  _A_NORMAL;
	hFind = _findfirst (infile, &c_file);
	if (hFind <= 0) {
		if(traceflag) {
/*			printf("FINDFIRST: failed on `%s'\n", infile); */
			printf("No files match: `%s'\n", infile);
			printf("\n");	/* no files match this pattern */
		}
	}
	else {					/* found at least one file */
		for (;;) {			/* findfirst / findnext loop */
/*			if (traceflag) {
				if (*path == '\0')
					printf("Found   %s %x\n",  c_file.name, c_file.attrib);
				else printf("Found   %s %x (in %s)\n",
							c_file.name, c_file.attrib, path);
			} */
			strcpy(infile, path);
			if (*path != '\0') strcat(infile, "\\");
			strcat(infile, c_file.name);
			if (outcurrent) strcpy(outfile, c_file.name); 
			else strcpy(outfile, infile);
/*			remember files or sub-directories ? */
			if ((subdirflag == 0) ||
				(c_file.attrib & _A_SUBDIR) != 0) {
				if (strcmp(c_file.name, ".") != 0 &&
					strcmp(c_file.name, "..") != 0) {
					if (subdirflag || iswrongcase(c_file.name)) {
						if (traceflag) {
							if (*path == '\0')
								printf("Found   %s %x ",  c_file.name, c_file.attrib);
							else printf("Found   %s %x (in %s) ",
										c_file.name, c_file.attrib, path);
							decodeattribute(c_file.attrib);
							printf("\n");
						}
						if (nIndex >= nNames) {
							fprintf(errout, "Exceeded allocation of %d file names\n", nNames);
							break;
						}
/*						szFileList[nIndex++] = xstrdup(infile); */
						szFileList[nIndex++] = xstrdup(c_file.name);
					}
				}
			}
			ret = _findnext (hFind, &c_file);
			if (ret < 0) {
				if (traceflag) {
					if (*path == '\0')
						printf("FINDNEXT: found %d %s\n", nIndex,
									  subdirflag ? "DIR" : "Files");
					else printf("FINDNEXT: found %d %s in `%s'\n", nIndex,
								subdirflag ? "DIR" : "Files", path);
					printf("\n");
				}
				break;	/* break out of loop if no more */
			}
		} /*end of _findfirst/_findnext for(;;) loop */
	} /* end of _findfirst found something */
	if (nIndex != nNames) {
		printf("INCONSISTENCY: nIndex %d versus nNames %d\n", nIndex, nNames);
		nNames = nIndex;
	}
/*	if (verboseflag)
		printf("Found %d %s listed in `%s'\n", nNames,
			   subdirflag ? "DIR" : "Files", name); */
	return nIndex;
}

void ShowFileList (int nNames, int subdirflag) {	/* debugging */
	int k;
	printf("FILE LIST %d %s \n", nNames, subdirflag ? "DIR" : "Files");
	for (k = 0; k < nNames; k++)
		printf("%3d\t%s\n", k, szFileList[k]);
	printf("\n");
}

/* new version, build list of files at each level first, before processing */

int renamefiles (char *name) {		/* this may call itself recursively */
	char path[FILENAME_MAX];		/* this part needs to be kept on stack */
	char subdir[FILENAME_MAX];
	int k;

	strcpy(path, name);
	stripfilename(path);			/* should have filename at end */
/*	if (strcmp(path, ".") == 0) *path = '\0'; */
	if (traceflag) {
		printf("Entering RenameFiles with `%s'\n", name);
		printf("Remembered path is `%s'\n", path);
		if (useasterisks) printf("Pattern is `*.*'\n");
		else if (usepattern) printf("Pattern is `%s'\n", pattern);
	}
/*	do files and subdirectpry names */
	nNames = CountFileNames(name, 0);
	if (nNames > 0) {
		(void) AllocateFileList(nNames);
		(void) CreateFileList (name, nNames, 0);
/*		ShowFileList(nNames, 0); */
		if (verboseflag) printf("Going to rename %d files\n", nNames);
		for (k = 0; k < nNames; k++) {
			strcpy(infile, path);
			if (*path != '\0') strcat(infile, "\\");
			strcat(infile, szFileList[k]);
			if (verboseflag) printf("Trying %s   %s\n", "file", infile);
			RenameOneFile(infile);
			if (traceflag) printf("\n");
		} 
		FreeFileList();
	}

	if (recurseflag ==0) return 0;	/* skip out if not recursive */

/*	now recurse into subdirectories */
	nNames = CountFileNames(name, 1);
	if (nNames > 0) {
		(void) AllocateFileList(nNames);
		(void) CreateFileList (name, nNames, 1);
/*		ShowFileList(nNames, 1); */
		if (verboseflag)
			printf("Going to recurse into %d subdirs from `%s'\n",
				   nNames, name);
		for (k = 0; k < nNames; k++) {
			strcpy(subdir, path);
			if (*path != '\0') strcat(subdir, "\\");
			strcat(subdir, szFileList[k]);
			strcat(subdir, "\\");
			if (useasterisks) strcat(subdir, "*.*");
			else if (usepattern) strcat(subdir, pattern);
			else strcat(subdir, filename);
			if (verboseflag) printf("Trying %s %s\n", "subdirectory", subdir);
			PushFileList();
			renamefiles(subdir);	/* recursive call */
/*			if (verboseflag) printf("Return to subdirs of `%s'\n", name); */
			PopFileList();
			if (traceflag) printf("\n");
		}
		FreeFileList();
		if (verboseflag) printf("Return from subdirs %s\n", name);
	}
	return 0;
}

#ifdef IGNORED
int renamefiles (char *name) {		/* this may call itself recursively */
	char path[FILENAME_MAX];		/* this part needs to be kept on stack */
	long hFind;						/* pointer to findfirst/next data */
	int ret;

	if (*name == '\0') {			/* sanity check */
		fprintf(errout, "ERROR: Called renamefiles with blank file name\n");
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

			if (verboseflag) printf("Trying %s => %s\n", infile, outfile);

			RenameOneFile(infile);

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
#endif

void showusage (char *argzero) {
	printf("Useage: %s [-v][-t] ...  [files]\n", argzero);
	printf("\t? showusage\n");
	printf("\tv verbose mode\n");
	printf("\tt trace mode (show more verbage)\n");
	printf("\td debug mode (don't actually rename)\n");
	printf("\tu uppercase names (instead of lowercase)\n");
/*	printf("\tc put output in current directory\n"); */
	printf("\tr recurse subdirectories\n");
	printf("\ta use *.* in subdirectories\n"); 
	printf("\tp use specified pattern in subdirectories\n"); 
	printf("\tc include files already in correct case\n");
	printf("\t  wild card arguments welcome\n");
	printf("\t  \n");
	printf("\t  %s c:\\chomp\\foo.bar\n", argzero);
	printf("\t  %s c:\\chomp\\*.txt\n", argzero);
	printf("\t  %s -r -a c:\\foo\\bar\n", argzero);
	printf("\t  %s -r -a .\n", argzero);
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
		if (strchr(argv[k], 'v') != NULL) verboseflag = 1;
		if (strchr(argv[k], 't') != NULL) traceflag = 1;
		if (strchr(argv[k], 'u') != NULL) uppercaseflag = 1;
		if (strchr(argv[k], 'l') != NULL) uppercaseflag = 0;
		if (strchr(argv[k], 'd') != NULL) debugflag = 1;
		if (strchr(argv[k], 'r') != NULL) recurseflag = 1;
		if (strchr(argv[k], 'a') != NULL) useasterisks = 1;
/*		if (strchr(argv[k], 'c') != NULL) outcurrent = !outcurrent; */
		if (strchr(argv[k], 'c') != NULL) dontcheckcase = 1;
		k++;
	}
	return k;
}

int main (int argc, char *argv[]) {
	int m, firstarg=1;

	noneed = failed = renamed = subdirs = 0;
	if (argc < 2) showusage(argv[0]);
	firstarg = commandline (argc, argv);
	if (argc < firstarg + 1) showusage(argv[0]);
#ifdef _WIN32
	printf("%s --- Make file names all %s case (32)\n", argv[0],
		   uppercaseflag ? "upper" : "lower");
#else
	printf("%s --- Make file names all %s case\n", argv[0],
		  		  uppercaseflag ? "upper" : "lower");
#endif
	if (traceflag) printf ("%d (%d) command line arguments\n",
						   argc - firstarg, argc);
	if (traceflag)
		for (m = 0; m < argc; m++) printf("%d\t%s\n", m, argv[m]);
	szFileList=NULL;
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
