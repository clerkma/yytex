/* Copyright 1995 Y&Y, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dos.h>		/* for dos_find... */
#include <malloc.h>		/* for free... */

#define PC_OS	 		/* following is for ttfname extraction */

#include <io.h> 
#include <fcntl.h> 
#include <errno.h> 

#include "fscdefs.h" 
#include "sfnt.h"
/* #include "sfnt_en.h" */ /* included by the above */

/* #define FNAMELEN 80 */

#define MAXLINE 256

int detailflag=0;
int verboseflag=0;
int traceflag=0;
int debugflag=0;
int showflag=0;
int showconflict=0;
int dotflag=1;
int shortenname=1;
int wantquote=1;
int wantttf=0;			/* process TTF instead of FOT files */
int ttsection=0;		/* use  [TTFonts] instead of [Fonts] */

char *szFontExt="FOT";		/* or "TTF" --- set in main */

char *szSection="[Fonts]";	/* or [TTFonts] --- set in main */

int saveflag=1;			/* build table of names */
int discardflag=1;		/* discard duplicates */
int prescanwin=1;		/* scan win.ini first */
int modwinini=0;		/* modify win.ini at end */
int listfull=0;			/* list face names, full names, style names */
int cleanfirst=0;		/* empty out [TTFonts] section first 97/Apr/12 */
int skipatmfolder=0;	/* do not look in ATMFolder */

int windirflag=0;		/* next arg is windir (no \\ at end) */
int systemdirflag=0;	/* next arg is systemdir (no \\ at end) */
int registerflag=0;		/* next arg is registry file name */

char *windir="";		/* Windows directory WIN.INI */
char *systemdir="";		/* Windows system/font directory *.FOT and *.TTF */
						/* \system or \fonts */
char *registryfile="";	/* name of Registry Editor ASCII file */

#define MAXNAMES 1024

char *FullNames[MAXNAMES];	/* Windows Full Names */
char *FileNames[MAXNAMES];	/* FOT or TTF File Names */
int WinTag[MAXNAMES];		/* flag that is set if entry from WIN.INI */

int nameindex=0;			/* index into the above */

int winindex=0;				/* how many from WIN.INI */

/* char *version="SetupTTF version 1.2\n"; */		/* 1996/Nov/30 */
/* char *version="SetupTTF version 1.3\n"; */			/* 1997/Apr/12 */
char *version="SetupTTF version 1.4\n";				/* 1997/Jul/25 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* searches binary file for given pattern */
/* returns zero and positions at first char after pattern if found */
/* starts at current positions returns -1 if pattern not found */

int findpattern (FILE *input, char *pattern) {
	int c, k=0;
	while ((c = getc(input)) != EOF) {
		if (c == pattern[k]) {
/*			putc('0'+k, stdout); */		/* debugging */
			k++;
			if (pattern[k] == '\0') return 0;
		}
		else k = 0;
	}
	return -1;
}

char *xstrdup(char *str) {
	char *s = _strdup(str);
	if (s == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		exit(1);
	}
	return s;
}

/* Pick what appears to be the better name of the two */
/* Better means shorter, or with lower version number at end */
/* lbtr is better than lbtr____, lbtr___0 is better than lbtr___2 */

int BetterName (char *aname, char *bname) {
	int c, d;

	if (strlen(aname) < strlen(bname)) return 1;
	if (strlen(aname) > strlen(bname)) return 0;
	if (strlen(aname) == 8 && strlen(bname) == 8) {
		c = *(aname+7); d = *(bname+7);
		if (c >= '0' && c <= '9') c = c - '0';
		else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
		if (d >= '0' && d <= '9') d = d - '0';
		else if (d >= 'A' && d <= 'F') d = d - 'A' + 10;
		if (c < 16 && d > 32) return 0;	/* aname has number, bname does not */
		if (d < 16 && c > 32) return 1;	/* bname has number, aname does not */
		if (c < 16 && d < 16) {
			if (c < d) return 1;
			else return 0;
		}
/*		if (c > '0' && c <= '9') {
			if (d > '0' && d <= '9') {
			}
			else return 0;
		} */
	}
	if (strcmp(aname, bname) < 0) return 1;
}

/* Insert new name in data base */ /* discard if duplicate Face Name */
/* last arg is zero if name from file, non-zero if from WIN.INI */

int insertname(char *fullname, char *filename, int winini) {
	int k, m, n, flag;

	if (nameindex >= MAXNAMES) {
		return -1;
	}
	if (traceflag) printf("CONSIDERING %s (TrueType)=%s.%s\n",
		fullname, filename, szFontExt);
	if (shortenname) {
/*	if (strstr(filename, "c:\\windows\\system\\") != NULL ||
		strstr(filename, "C:\\WINDOWS\\SYSTEM\\") != NULL) { */
/*	if (_strnicmp(filename, "C:\\WINDOWS\\SYSTEM\\", 18) == 0) { */
		n = strlen(systemdir);
		if (_strnicmp(filename, systemdir, n) == 0) {
			filename = filename + (n+1);	/* extra for '\\' */
		}
		if (traceflag) 	printf("CONSIDERING %s (TrueType)=%s.%s\n",
							  fullname, filename, szFontExt);
	}
	flag = 1;
/*	Insertion sort, figure out where it goes */
	for (k = 0; k < nameindex; k++) {
		if (discardflag && strcmp(FullNames[k], fullname) == 0) {
/*			if (BetterName (fullname, FileNames[k]) ) { */
			if (_stricmp(filename, FileNames[k]) == 0) {
/*	Nothing to do, `new' entry is already in table */
			}
			else if (BetterName (filename, FileNames[k]) ) {
				if (showconflict)
				printf("DISCARDING %-8s  KEEPING %-8s (%s)\n",
					   FileNames[k], filename, fullname);
				free(FullNames[k]);
/*	replace name that is now in table */
				FullNames[k] = xstrdup(fullname);
				free(FileNames[k]);
				FileNames[k] = xstrdup(filename);
			}
			else {
/*	discard the new name instead */
				if (showconflict)
				printf("DISCARDING %-8s  KEEPING %-8s (%s)\n",
					   filename, FileNames[k], fullname);
			}
			flag = 0;
			break;
		}	 /* end of FullNames[k] == fullname case */
		if (strcmp(FullNames[k], fullname) > 0) {
/*			for (m = k; m < nameindex; m++)  */
/*	 shift everything up by one to make space */
			for (m = nameindex; m > k; m--) {
				FullNames[m] =  FullNames[m-1];
				FileNames[m] =  FileNames[m-1];
				WinTag[m] = WinTag[m-1];
			}
/*			printf("INSERTING %s\n", filename);
			FullNames[k] = xstrdup(fullname);
			FileNames[k] = xstrdup(filename);
			nameindex++; */
			flag = 1;
			break;
		} /* end of FullNames[k] > fullname case */
	} /* end of loop over k */
	if (flag) {
		if (traceflag) printf("INSERTING %s\n", filename);
		FullNames[k] = xstrdup(fullname);
		FileNames[k] = xstrdup(filename);
		WinTag[k] = winini;
		nameindex++;
	}
	return 0;
}

/* dump out names from table */
/* those from WIN.INI if flag is 1 */
/* those from *.FOT or *.TTF if flag is 0 */

int dumpnames (FILE *output, int winini, char *ext) {
	int k, count=0;
	
	if (traceflag) {  
		putc('\n', stdout);
		printf("Dumping Names --- winini = %d nameindex = %d\n",
			winini, nameindex);
		putc('\n', stdout);
	} 

	for (k = 0; k < nameindex; k++) {
		if (winini != 0 && WinTag[k] == 0) continue;
		if (winini == 0 && WinTag[k] != 0) continue;
		if (traceflag) printf ("%d\t", k); 
/*		if (verboseflag) printf("%d\t", k); */
		fprintf(output, "%s (TrueType)=%s.%s\n",
			FullNames[k], FileNames[k], ext);
		count++;
	}
	return count;
}

void clearnames (void) {	/* clear out the tables again */
	int k;
	for (k = 0; k < nameindex; k++) {
		free (FullNames[k]);
		free (FileNames[k]);		
	}
	nameindex = 0;
}

/* Try and extract information from *.FOT */
/* sets up FileName, TTFName, FullName */
/* and does insertname (FullName, FileName, 0); */

int readfot (FILE *input) {
	int c, k;
	char *s;
/*	char filename[16], ttfname[16], fullname[34]; */
	char filename[16], ttfname[16], fullname[64];
	
	if (findpattern (input, "FONTDIR") != 0) {
		if (traceflag) printf ("No FONTDIR. ");
		return -1;
	}
	c = getc(input);				/* length of next field (8) */
	for (k = 0; k < 8; k++) filename[k] = (char) getc(input);
	filename[8] = '\0';
	while ((c = getc(input)) < ' ') ;	/* skip the 5 blanks */
	ungetc (c, input);
	for (k = 0; k < 12; k++) ttfname[k] = (char) getc(input);
	ttfname[12] = '\0';
/*	for (k = 0; k < 5; k++) getc(input); */	/* skip the blanks */
/*	c = getc(input); */ 				/* length of next field */
	if (findpattern (input, "FONTRES:") != 0) {
		if (traceflag) printf ("No FONTRES. ");
		return -1;
	}
/*	for (k = 0; k < 33; k++) fullname[k] = (char) getc(input); */
	s = fullname;
	while ((c = getc(input)) > 0 && s < fullname + sizeof(fullname))
		*s++ = (char) c;
/*	fullname[33] = '\0'; */
	*s = '\0';
	if (saveflag) insertname(fullname, filename, 0);
	if (traceflag) {
		printf("%s (TrueType)=%s.%s", fullname, filename, szFontExt);
		if (traceflag) printf(" ; %s", ttfname);
		putc('\n', stdout);
	}
	else if (traceflag) {
		printf("%s\t%s\t%s", filename, ttfname, fullname);
		putc('\n', stdout);
	}
	else if (dotflag && showconflict == 0) putc('.', stdout);
	return 0;
}

int checkhead (FILE *input, char *fname, char *ext) {
	int c, d;

	(void) fseek (input, 0, SEEK_END);			/* end of file */
	if (ftell(input) == 0) {
		printf("%s zero length file!\n", fname);
		return -1;
	}

	(void) fseek (input, 0, SEEK_SET);			/* rewind */
	c = getc(input);
	d = getc(input);
	if (c == EOF || d == EOF) {
		printf("%s premature EOF in header\n", fname);
		return -1;
	}
	if (c == 0 && d == 1) {
		if (_stricmp(ext, "FOT") == 0) {
			printf("%s is a TTF file, not FOT!\n", fname);
			return -1;
		}
	}
	else if (c == 'M' && d == 'Z') {
		if (_stricmp(ext, "TTF") == 0) {
			printf("%s is a FOT file, not TTF!\n", fname);
			return -1;
		}
	}
	else if (c == 0 && d == 0) {
		while ((c = getc(input)) == 0) ;
		if (c == EOF)
			printf("%s contains nothing but null bytes (0)!\n", fname);
		else printf("%s is not a valid FOT or TTF file (%d %d)!\n",
					fname, c, d);
		return -1;
	}
	else {
/*		printf("%s is not a valid FOT or TTF file!\n"); */
		printf("%s is not a valid FOT or TTF file (%d %d)!\n", fname, c, d);
		return -1;
	}
	fseek (input, 0, SEEK_SET);
	return 0;
}

int readttf (char *);

/* Deal with an *.FOT or *.TTF file */

int dofontfile(char *fotname) {
 	FILE *input;
	int flag;
	
/*	if (verboseflag) printf("Processing %s: ", fotname); */
/*	if (traceflag) printf("Processing %s\n", fotname); */
	if ((input = fopen(fotname, "rb")) == NULL) {
		perror (fotname);
/*		continue; */
		return -1;
	}
	if (checkhead(input, fotname, szFontExt) != 0) {
		fclose(input);
		return -1;
	}
/*	if (wantttf) { */
	if (_strcmpi(szFontExt, "TTF") == 0) {
		fclose(input);
		flag = readttf (fotname);
	}
	else {
		flag = readfot (input);
		fclose(input);
	}
	if (flag != 0)
		fprintf(stderr, "Not a valid %s file: %s\n", szFontExt, fotname);
/*		fclose (input); */
	return flag;
}

int parseiniline (char *line, int winini) {
	char *s, *t, *u;

/*	Deal with quotedbl in lines from registry file 95/Aug/2 */
/*	while ((s = strchr(line, '"')) != NULL) strcpy(s, s+1); */
	if ((s = strstr(line, "(TrueType)")) == NULL) return 0;
	if ((t = strchr(s, '=')) == NULL) return 0;
	s--;
	while (*s <= ' ') s--;
	*(s+1) = '\0';
	t++;
	while (*t <= ' ') t++;
	if ((u = strstr(t, ".FOT")) != NULL) *u = '\0';
	if ((u = strstr(t, ".fot")) != NULL) *u = '\0';
	if ((u = strstr(t, ".TTF")) != NULL) *u = '\0';
	if ((u = strstr(t, ".ttf")) != NULL) *u = '\0';
	insertname (line, t, winini);
	return 1;
}

/* Scan WIN.INI and built table of TrueType fonts */

int scanwinini (FILE *input, char *szSection, int wininiflag) {
	char line[MAXLINE];
/*	char fullname[64]; */
/*	char filename[128]; */
	int flag=0;
/*	char *s, *t, *u; */


	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == ';') continue;
		if (*line == '[') {
			if (_strnicmp(line, szSection, strlen(szSection)) == 0) break;
		}
	}

	if (feof(input) != 0) return 0;

	if (cleanfirst) return 0;				/* 97/Apr/12 */
	
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '[') break;
		if (*line == ';') continue;
		parseiniline (line, wininiflag);
		flag++;		
/*		if (sscanf(line, "%s (TrueType)=%s.FOT", fullname, filename) == 2) {
			if ((s = strstr(filename, ".FOT")) != NULL) *s = '\0';
			if ((s = strstr(filename, ".fot")) != NULL) *s = '\0';
			insertname (fullname, filename, 1);
			flag++;
		} */
	}	
	return flag;
}

/* update WIN.INI by adding new TrueType font entries */

int updatewinini (FILE *output, FILE *input, char *infilename) {
	char line[MAXLINE];
	int flag;
/*	char *s, *t, *u; */

/*	printf("\n"); */
/*	printf("Updating WIN.INI\n"); */
/*	scan up to [Fonts] section */
	while (fgets(line, sizeof(line), input) != NULL) {
		fputs(line, output);
		if (*line == ';') continue;
		if (*line == '[') {
			if (_strnicmp(line, szSection, strlen(szSection)) == 0) break;
		}
	}
	if (feof(input) != 0) {
		printf("Adding %s section to %s\n", szSection, infilename);
		putc('\n', output);
		fprintf(output, "%s\n", szSection);
/*		return 0; */		/* bad EOF no [Fonts] */
	}
/*	fputs(line, output); */
/*	scan over existing font entries */
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line < ' ') break;
		if (*line == '[') break;
/*		fputs(line, output); */
		if (cleanfirst == 0) fputs(line, output);	/* 97/Apr/12 */
		if (*line == ';') continue;
	}	
/*	if (feof(input) != 0) return -1; */
/*	printf("Adding new names\n"); */			/* use FOT in WIN.INI ??? */
/*	printf("Adding new names to %s section in %s\n", szSection, infilename); */
	printf("Adding new names to %s section\n", szSection);
	flag = dumpnames(output, 0, "FOT");		/* dump only new names */
	if (feof(input) != 0) return flag;
	fputs(line, output);
/*	copy rest of file */
	while (fgets(line, sizeof(line), input) != NULL) fputs(line, output);
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showusage(char *s) {
	fprintf (stderr,
 "Usage:\n%s -{v}{f}{n}{m}{c}{s}{x}{z} [-W=<windir>] [-S=<sysdir>] [<file1> ...]\n", s);
/*	if (detailflag == 0) exit(1); */
	putc('\n', stderr);
	fprintf(stderr, "\tv verbose\n");
	fprintf(stderr, "\tf look for TTF files instead of FOT files\n");
	fprintf(stderr, "\tn use [TTFonts] instead of [Fonts] section\n");
	fprintf(stderr, "\tm add new entries to fonts section in WIN.INI\n");
	fprintf(stderr, "\tx ignore old entries in [TTFonts] section\n");
	fprintf(stderr, "\ta do not look in ATMFolder (subfolder or font folder)\n");
	fprintf(stderr, "\tc show conflicts (font files with same Face Name)\n");
	fprintf(stderr, "\ts show (i) fonts section of WIN.INI and (ii) font files\n");
	fprintf(stderr, "\tz list File Name, Face Name, Full Name, Style Name (FOT files)\n");
	fprintf(stderr, "\tW specify windows directory (default c:\\winnt or c:\\windows)\n");
	fprintf(stderr, "\tS specify system directory (default c:\\winnt\\system or c:\\windows\\system)\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\tCan omit *.ttf (or *.fot) if files in system or font directory\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\tIf no command line args given, assume -vfnmx \n");
	fprintf(stderr, "\totherwise must use one of the flags m, s, or z\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "\tR scan specified ASCII registry editor export file");
	exit(1);
}

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0; 
		case 'v': verboseflag = ~verboseflag; return 0;
		case 't': traceflag = ~traceflag; return 0; 
		case 's': showflag = ~showflag; return 0; 
		case 'd': dotflag = ~dotflag; return 0;
		case 'c': showconflict = ~showconflict; return 0;
		case 'm': modwinini = ~modwinini; return 0;
		case 'f': wantttf = ~wantttf; return 0;
		case 'n': ttsection = ~ttsection; return 0;
		case 'x': cleanfirst = ~cleanfirst; return 0;
		case 'z': listfull = ~listfull; return 0; /* ??? */
		case 'a': skipatmfolder = ~skipatmfolder; return 0;
/* following are of dubious value ... */
		case 'T': debugflag = ~debugflag; return 0; /* ??? */
		case 'D': discardflag = ~discardflag; return 0; /* ??? */
		case 'Y': saveflag = ~saveflag; return 0; /* ??? */
		case 'X': shortenname = ~shortenname; return 0; /* ??? */
		case 'P': prescanwin = ~prescanwin; return 0; /* ??? */
/*	following take arguments */
		case 'W': windirflag = 1; return -1; 
		case 'S': systemdirflag = 1; return -1; 
		case 'R': registerflag = 1; return -1;
		default: {
				fprintf(stderr, "Invalid command line flag '%c'", c);
				exit(7);
		}
	}
	return 0;		/* ??? */
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

/*	if (argc < 2) showusage(argv[0]); */
	if (argc < 2) {					/* pretend -vfnm given */
		printf("No arguments given:  assuming -vfnmx command line flags\n");
		printf("(Use setupttf -? to see all command line options)\n");
		verboseflag = 1;
		wantttf = 1;
		ttsection = 1;
		modwinini = 1;
		cleanfirst = 1;				/* 1997/Apr/12 */
		return firstarg;			/* 1996/Oct/8 */
	}

	while (firstarg < argc && argv[firstarg][0] == '-') { /* command flags */
		for(i=1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break;
			else if (decodeflag(c) != 0) { /* flag takes argument */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (windirflag != 0) {
					windir = s; 
					printf("WINDIR=%s\n", windir); /* debugging */
					windirflag = 0;
				} 
				if (systemdirflag != 0) {
					systemdir = s; 
					printf("SYSTEMDIR=%s\n", systemdir); /* debugging */
					systemdirflag = 0;
				} 
				if (registerflag != 0) {
					registryfile = s; 
					printf("REGISTRYFILE=%s\n", registryfile); /* debugging */
					registerflag = 0;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

int showstring (FILE *input, FILE *output) {
	int c, k=0;
	if (wantquote) putc(96, output);
	while ((c = getc(input)) > 0) {
		if (k++ > 128) {
			printf(" String too long\n");
			return -1;				/* sanity check */
		}
		if (c < 32 || c > 128) {
			printf(" non ASCII character in string\n");
			return -1;		/* sanity check */
		}
		putc(c, output);
	}
	if (wantquote) putc(39, output);
	return 0;
}

/* At offset 1270 in the FOT file are in sequence: */
/* null-terminated strings for: Face Name, Full Name, Style Name */
/* Typically Full Name is Face Name with Style Name appended */
/* (unless its `Regular') */

/* The Face Name is what is listed in font menus */
/* The Full Name is what appears in WIN.INI */

int showfieldsinFOT (char *fotname) {
	FILE *input;
	char *s;
/*	int c, d; */
	int n;

	if ((input = fopen(fotname, "rb")) == NULL) {
		perror(fotname);
/*		continue; */
		return -1;
	}
/*	if (_strnicmp(fotname, "c:\\windows\\system\\", 18) == 0) */
	n = strlen(systemdir);
	if (_strnicmp(fotname, systemdir, n) == 0) {
		s = fotname + (n+1);				/* extra for '\\' */
	}
	else s = fotname;
	fputs(s, stdout);
	putc('\t', stdout);
	if (checkhead(input, "", szFontExt) != 0) {
		fclose(input);
		return -1;
	}
	if (fseek (input, 1270, SEEK_SET) != 0) {
		printf("Failure to seek to font name information!\n");
		fclose(input);
		return -1;
	}
	if (showstring(input, stdout)) {
		fclose(input);
		return -1;
	}
	putc('\t', stdout);
	if (showstring(input, stdout)) {
		fclose(input);
		return -1;
	}
	putc('\t', stdout);
	if (showstring(input, stdout)) {
		fclose(input);
		return -1;
	}
	putc('\n', stdout);
	fclose (input);
	return 0;
}

/*	unsigned int attrib = _A_NORMAL | _A_RDONLY; */
unsigned int attrib = _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM;

/* NOTE:	showfields works only for *.FOT files */

/*void showfields (int firstarg, int argc, char *argv[], char *windir) {*/
void showfields (int firstarg, int argc, char *argv[]) {
	char infilename[FILENAME_MAX];
	struct _find_t fileinfo;
	unsigned flag;
	int m;
	int count=0;

	putc('\n', stdout);
	fputs("File Name\t`Face Name'\t`Full Name'\t`Style Name'\n\n", stdout); 
	if (argc < firstarg+1) {
		strcpy(infilename, systemdir);
		strcat(infilename, "\\");
		strcat(infilename, "*.fot");
/*		Here we should ONLY try *.fot files ... */
		flag=_dos_findfirst(infilename, attrib, &fileinfo);
/*		show error message 1995/Aug/27 */
		if (flag != 0) {
			printf("Found no files named %s\n", infilename);
#ifdef IGNORED
			strcpy(infilename, systemdir);
			strcat(infilename, "\\");
			strcat(infilename, "*.fot");	/* try this second 96/Oct/6 */
			flag=_dos_findfirst(infilename, attrib, &fileinfo);
/*			show error message 1995/Aug/27 */
			if (flag != 0) printf("Found no files named %s\n", infilename);
#endif
		}  
		else if (verboseflag != 0) printf("Found %s\n", infilename);
		while (flag == 0) {
			if (debugflag) printf("File: %s Attrib %0x Size %ld\n",
				  fileinfo.name, fileinfo.attrib, fileinfo.size);
			strcpy(infilename, systemdir);
			strcat(infilename, "\\");
			strcat(infilename, fileinfo.name);
			showfieldsinFOT(infilename);
			count++;
			flag = _dos_findnext(&fileinfo);
		}
	}
/*	If FOT files are given on command line */
	else {
/*	fputs("File Name\t`Face Name'\t`Full Name'\t`Style Name'\n\n", stdout); */
		for (m = firstarg; m < argc; m++) showfieldsinFOT(argv[m]);
	}
}


int scanfontsaux (char *infilename, int errorflag) {
	int count=0;
	unsigned int flag;
	struct _find_t fileinfo;

	strcpy(infilename, systemdir);
	strcat(infilename, "\\*.");
	strcat(infilename, szFontExt);
	flag = _dos_findfirst(infilename, attrib, &fileinfo);
	if (flag != 0) {		/*		show error message 1995/Aug/27 */
		if (errorflag) printf("Found no files named %s\n", infilename);
		if (_strcmpi(szFontExt, "FOT") == 0) szFontExt = "TTF";
		else szFontExt = "FOT";		/*		Try the other extension */
		strcpy(infilename, systemdir);
		strcat(infilename, "\\*.");
		strcat(infilename, szFontExt);
		flag=_dos_findfirst(infilename, attrib, &fileinfo);
		if (flag != 0) {	/*		show error message 1995/Aug/27 */
			if (errorflag) printf("Found no files named %s\n", infilename);
			return count;
		}
	}
	if (verboseflag != 0) printf("Found %s\n", infilename);
	while (flag == 0) {
		if (debugflag) printf("File: %s Attrib %0x Size %ld\n",
							  fileinfo.name, fileinfo.attrib, fileinfo.size);
		strcpy(infilename, systemdir);
		strcat(infilename, "\\");
		strcat(infilename, fileinfo.name);
		dofontfile(infilename);
		count++;
		flag = _dos_findnext(&fileinfo);
	}
	return count;
}

int scanfonts (int firstarg, int argc, char *argv[]) {
	char infilename[FILENAME_MAX];
	int m, n;
	int count=0;
	char *s;
	
/*	if (argc < firstarg+1) { */
	if (argc >= firstarg+1) {	/*	If FOT files are given on command line */
		for (m = firstarg; m < argc; m++) {
			if (debugflag) printf("File: %s\n", argv[m]);
			dofontfile(argv[m]);
			count++;
		}
	}
	else {
		count = 0;
		if (traceflag) printf("Trying in %s\n", systemdir);
		count += scanfontsaux(infilename, 1);
		if (skipatmfolder) return count;
/*		Now add \\ATMFolder to systemdir */
		n = strlen(systemdir);
		s = (char *) malloc(n+10+1);
		if (s == NULL) {
			fprintf(stderr, "Memory allocation error\n");
			exit(1);
		}
		strcpy(s, systemdir);
#ifdef _WIN32
		strcat(s, "\\ATMFolder");	/* Try again in ATMFolder 32 bit */
#else
		strcat(s, "\\atmfol~1");	/* Try again in ATMFolder 16 bit */
#endif
		systemdir = s;
		putc('\n', stdout);
		if (traceflag) printf("Trying in %s\n", systemdir);
		count += scanfontsaux(infilename, 0); /* but don't complain if missing */
	}
	return count;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* REGEDIT4 */

char *registrysection1 =
"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts]";

char *registrysection2 =
"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts]";

int scanregistryfile (FILE *input) {
	int hitfonts=0;
	char *s;
	int n1, n2, count=0;
	char line[MAXLINE];
	
	n1 = strlen(registrysection1);
	n2 = strlen(registrysection2);
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line != '[') continue;
		if (strncmp(line, registrysection1, n1) == 0) {
			hitfonts++;
			break;
		}
		if (strncmp(line, registrysection2, n2) == 0) {
			hitfonts++;
			break;
		}
	}
	if (hitfonts == 0) {
		fprintf(stderr, "Did not find fonts in registry file\n");
		return 0;
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line != '[') break;			/* hit another section */
		if (*line == ';') continue;			/* unlikely ... */
/*		if (strstr(line, "(TrueType)") == NULL) continue; */
/*		"Arial Bold Italic (TrueType)"="ARIALBI.TTF" */
/*		Deal with quotedbl in lines from registry file 95/Aug/2 */
		while ((s = strchr(line, '"')) != NULL)	strcpy(s, s+1);
/*		Arial Bold Italic (TrueType)=ARIALBI.TTF */
/*		now treat like line read from WIN.INI - except don't mark that way */
		if (parseiniline (line, 0) != 0) count++;
	}
	return count;
}

int scanregistry (char *rname) {
	FILE *input;
	char filename[FILENAME_MAX];
	int count;

	if (strchr(rname, '\\') != NULL || strchr(rname, ':') != NULL) 
		strcpy (filename, rname);
	else {
		strcpy (filename, windir);
		strcat (filename, "\\");
		strcat (filename, rname);
	}
	if ((input = fopen(filename, "r")) == NULL) {
		perror(filename);
		return 0;
	}
	count = scanregistryfile(input);
	fclose(input);
	return count;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

void stripfilename(char *fname) {
	char *s;
/*	if ((s = strrchr(fname, '\\')) != NULL) *(s+1) = '\0'; */
	if ((s = strrchr(fname, '\\')) != NULL) *s = '\0';
	else if ((s = strrchr(fname, '/')) != NULL) *s = '\0';
	else if ((s = strrchr(fname, ':')) != NULL) *(s+1) = '\0';
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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* This tries to open WIN.INI in given directory - full path name infilename */

FILE *TryWinDir(char *windir, char *infilename) {
	FILE *input=NULL;
	strcpy(infilename, windir);
	strcat(infilename, "\\");
	strcat(infilename, "WIN.INI");
	input = fopen(infilename, "r");
	return input;
}

FILE *getwindir (char *infilename) {		/* Try and find win.ini */
	char *s;
	FILE *input=NULL;
	if ((s = getenv("windir")) != NULL ||
		(s = getenv("WINDIR")) != NULL ||
		(s = getenv("winbootdir")) != NULL ||
		(s = getenv("SystemRoot")) != NULL ||
		(s = getenv("SYSTEMROOT")) != NULL) {
		windir = s;
		printf("Windows directory is %s\n", windir);
	}
	input = TryWinDir(windir, infilename);
/*	it's very unlikely we haven't found it at this point ... */
	if (input == NULL) {		/* Windows NT 4.0 */
		perror (infilename);
		windir = "c:\\winnt";			/* ugh try default */
		input = TryWinDir(windir, infilename);
	}							/* This option enabled 96/Oct/6 */
	if (input == NULL) {		/* Windows 95 and Windows NT 3.51 */
		perror (infilename);
		windir = "c:\\windows";			/* ugh try default */
		input = TryWinDir(windir, infilename);
	}
/*	now we are clutching at straws ... */
	if (input == NULL) {
		perror (infilename);
		_searchenv("WIN.INI", "PATH", infilename);
		if (strcmp(infilename, "") != 0) {
			windir = xstrdup(infilename);
/*				if ((s = strrchr(windir, '\\')) != NULL) *s='\0'; */
			stripfilename(windir);
			printf("Windows directory is %s\n", windir);
			input = fopen(infilename, "r");
		}
	}
	return input;
}

int getsysdir(char *outfilename, char *infilename) {
	char *s;
	int flag;
	struct _find_t fileinfo;
	
	strcpy(outfilename, infilename);
	stripfilename(outfilename);
	strcat(outfilename, "\\");
	strcat(outfilename, "FONTS");		/* reversed order 96/Oct/6 */
	systemdir = xstrdup(outfilename);
	strcat(outfilename, "\\*.TTF");				/* 1995/Aug/27 */
/* do a _dos_findfirst on *.ttf at this point */ /* UNFINISHED */
/* if it fails (returns non-zero) then try c:\windows\system\ instead */
	flag = _dos_findfirst(outfilename, attrib, &fileinfo);
	if (flag != 0) {							/* 1995/Aug/27 */
/*		Unlikely to find *.FOT files in windows\fonts, but what the hell... */
		if ((s = strrchr(outfilename, '\\')) != NULL)
			strcat(s, "\\*.FOT");				/* Try other extension */
		flag = _dos_findfirst(outfilename, attrib, &fileinfo);
	}
/*		Fonts may be in windows\system instead */
	if (flag != 0) {
		free(systemdir);
		strcpy(outfilename, infilename);
		stripfilename(outfilename);
			strcat(outfilename, "\\");
		strcat(outfilename, "SYSTEM");		/* reversed order 96/Oct/6 */
		systemdir = xstrdup(outfilename);
		strcat(outfilename, "\\*.TTF");			/* 1995/Aug/27 */
		flag = _dos_findfirst(outfilename, attrib, &fileinfo);
		if (flag != 0) {						/* 1995/Aug/27 */
				if ((s = strrchr(outfilename, '\\')) != NULL)
				strcat(s, "\\*.FOT");			/* Try other extension */
			flag = _dos_findfirst(outfilename, attrib, &fileinfo);
		}
		if (flag != 0) systemdir="";
	}
	return !strcmp(systemdir, "");
}

int main(int argc, char *argv[]) {
	int firstarg=1;
	int count, new;
	char infilename[FILENAME_MAX];
	char outfilename[FILENAME_MAX];
	char bakfilename[FILENAME_MAX];
	int foundwinini=0;
	int foundsystem=0;
	FILE *input=NULL;
	FILE *output;
/*	struct _find_t fileinfo; */
/*	unsigned flag; */
/*	char *s; */

	nameindex=0;			/* index into FullName/FileName array */
	winindex=0;				/* how many of these are from WIN.INI */
	count=0;

	printf(version);							/* 96/Nov/30 */

	firstarg = commandline(argc, argv, 1);
	if (detailflag) showusage(argv[0]);

	if (wantttf) szFontExt = "TTF";				/* initial default */
	else szFontExt = "FOT";						/* may be changed later */

	if (ttsection) szSection = "[TTFonts]";
	else szSection = "[Fonts]";

	if (argc < firstarg+1 && listfull == 0 && 
		modwinini == 0 && showflag == 0) showusage(argv[0]); 

/*	Try and figure out where the Windows directory and win.ini are */

	if (strcmp(windir, "") != 0) {		/* user specified Windows dir */
		input = TryWinDir(windir, infilename);
		if (input == NULL) 	perror (infilename);
	}
	else input = getwindir(infilename);

/*	it's very unlikely we haven't founnd it at this point ... */
	if (input == NULL) {
		strcpy(infilename, "WIN.INI");
		foundwinini = 0;
		printf("WARNING: Unable to find WIN.INI\n");
		exit(1);	/* ? */
	}
	printf("Found %s\n", infilename);
	foundwinini = 1;
	fclose(input);

/*	Full path name for win.ini is now in `infilename' at this point */

/*	Now try and figure out where the Windows system/font directory is */
/*	Search first in windows\fonts in case *both* have *.ttf files */

	
	if (strcmp(systemdir, "") != 0) {
	}
	else getsysdir(outfilename, infilename);	/* Get font directory */

	if (strcmp(systemdir, "") != 0) printf("Fonts in %s\n", systemdir);
	else {
		printf("WARNING: Unable to find System/Font directory\n");
/*		exit(1); */
	}

	if (listfull) {
		showfields (firstarg, argc, argv);
		exit(1);
	}


	if (prescanwin) {
		if ((input = fopen(infilename, "r")) == NULL) {
			perror (infilename);
		}
		else {
			scanwinini(input, szSection, 1);
			fclose(input);
			if (showflag) {
				putc('\n', stdout);
				printf("Contents of %s in WIN.INI\n", szSection);
				putc('\n', stdout);
				dumpnames(stdout, 1, "FOT");/* dump fonts in WIN.INI already */
				putc('\n', stdout);
			}
			winindex = nameindex;
		}
	}

	if (strcmp(registryfile, "") != 0) {
		count = scanregistry(registryfile);
	}
	else {
		count = scanfonts (firstarg, argc, argv);
	}

	if (traceflag)
		printf("Returned %d from scanfonts (winindex: %d nameindex: %d)\n",
		   count, winindex, nameindex);
	
	new = (nameindex - winindex);
	if (saveflag) {
		if (dotflag) putc('\n', stdout);
		if (nameindex == MAXNAMES)
			printf("There were more than %d names\n", MAXNAMES);
		else if (verboseflag) {
			printf("There were %d font files with %d Face Names",
				   count, nameindex);
			if (cleanfirst == 0)				/* 97/Apr/12 */
				printf(" (%d new ones)", (nameindex - winindex));
			putc('\n', stdout);
		}
		putc('\n', stdout);
		if (showflag)					/* dump new fonts not in WIN.INI */
			dumpnames(stdout, 0, szFontExt); 
	}

/*	Now modify win.ini if asked for */
	if (modwinini) {
		if (new > 0) {
/*		strcpy(infilename, windir);
		strcat(infilename, "\\");
		strcat(infilename, "WIN.INI"); */
		if ((input = fopen(infilename, "r")) == NULL) {
			perror (infilename);
		}
		else {
/*			strcpy(outfilename, windir);
			strcat(outfilename, "\\");
			strcat(outfilename, "win.mod"); */
			strcpy(outfilename, infilename);
			forceexten(outfilename, "mod");
			if ((output = fopen(outfilename, "w")) == NULL) {
				perror (outfilename);
			}
			else {
				printf("\n");
				printf("Updating %s\n", infilename);
				count = updatewinini(output, input, infilename);
				fclose (output);
			}
			fclose (input);
			if (count > 0) {
				/* delete win.bak */
/*				strcpy (bakfilename, windir);	
				strcat (bakfilename, "\\");
				strcat (bakfilename, "win.bak"); */
				strcpy(bakfilename, infilename);
				forceexten(bakfilename, "bak");
				remove (bakfilename);
				/*rename win.ini to win.bak */
				rename (infilename, bakfilename);
				/* rename win.mod to win.ini */
				rename (outfilename, infilename);
			}
/*			else printf("No need to update WIN.INI\n"); */
			else printf("No need to update %s\n", infilename);
		}
		}
/*		else printf("No need to update WIN.INI\n");	*/
		else printf("No need to update %s\n", infilename);		
	}
	clearnames();
	return 0;
}

/* [fonts] in win.ini */
/* Arial (TrueType)=C:\WINDOWS\SYSTEM\ARIAL.FOT */
/* Arial Bold (TrueType)=C:\WINDOWS\SYSTEM\ARIALBD.FOT */
/* Arial Bold Italic (TrueType)=C:\WINDOWS\SYSTEM\ARIALBI.FOT */
/* Arial Italic (TrueType)=C:\WINDOWS\SYSTEM\ARIALI.FOT */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Code for dumping out name table in TTF file */

char *PlatFormNames[] = {
    "Unicode",    "Macintosh",    "ISO",    "MS", ""
};

char *FontNames[] = {
    "Copyright",    "Family/Face",    "Subfamily/Style",    "UniqueName",
    "FullName",    "Version",    "Postscript/FontName",	 "Notice", ""
};

int fullpathflag=0;
int writeiniflag=0;

/*
void ShowNameField (int fp, FILE *output, char * pszFile,
					sfnt_NameRecord NameRecord,
					sfnt_NamingTable NamingTable,
					sfnt_DirectoryEntry Table, char *szNameType) {
	long		      curseek; 
	char		      namebuf[255];
	unsigned int m, nLen;

	curseek = _tell(fp);
	nLen = SWAPW(NameRecord.length);
	_lseek (fp, SWAPW (NameRecord.offset) + 
		SWAPW(NamingTable.stringOffset) + 
		SWAPL(Table.offset), SEEK_SET);
	_read (fp, &namebuf, SWAPW(NameRecord.length));
	namebuf[SWAPW(NameRecord.length)] = '\0';

	if (writeiniflag) {
		fputs(namebuf, output);
		fputs(" (TrueType)=", output);
		if (fullpathflag) fputs(pszFile, output);
		else fputs(extractfilename(pszFile), output);
		putc('\n', output);
	}
	else {
		if (!fullpathflag) fputs(extractfilename(pszFile), output);
		else fputs(pszFile, output);
		fputs(": ", output);
		fputs(szNameType, output);
		fputs(" = ", output);
		if (*namebuf != '\0') fputs(namebuf, output);
		else {	
			for (m = 0; m < nLen; m += 2) {
				putc(namebuf[m+1], output);
			}
		}
		putc('\n', output);
	}
	_lseek (fp, curseek, SEEK_SET);
} */

void stripextension (char *fname) {
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL) return;
	if ((t = strrchr(fname, '\\')) == NULL) *s = '\0';	
	if (s > t) *s = '\0';
}

void DeUnicode (char *s, int n) {
	char *send = s+n;
	char *t=s;

	while (s < send) {
		s++;
		*t++ = *s++;
	}
	*t = '\0';
}

/* find and seek to the naming table */
/* read full name and insertname() */

int DumpNameTable (char *pszFile) {
	int flag=0;
	unsigned            i; 
	char				namebuf[255]; 
/*	char				fname[128]; */
	int                 fp; 
/*  FILE                *fp;  */
	unsigned short      numNames; 
	long				curseek; 
	unsigned            cTables;
	sfnt_OffsetTable    OffsetTable; 
	sfnt_DirectoryEntry Table; 
	sfnt_NamingTable    NamingTable; 
	sfnt_NameRecord     NameRecord;
	int	nPlatForm, nNameID, nLanguage;
	int nHitFullName=0;
	int nLen;
  
	if (traceflag) printf("Opening %s\n", pszFile);
/*  if ((fp = _open (pszFile, O_RDONLY | O_BINARY)) == -1)  { */
	if ((fp = _open (pszFile, _O_RDONLY | _O_BINARY)) == -1) {
/*  if ((fp = fopen (pszFile, "rb")) == NULL) { */
		perror(pszFile);
		return -1;
	}

 /*	First off, read the initial directory header on the TTF.  We're only
   * interested in the "numOffsets" variable to tell us how many tables
   * are present in this file.  
   *
   * Remember to always convert from Motorola format (Big Endian to 
   * Little Endian).
   */

	_read (fp, &OffsetTable,
		sizeof (OffsetTable) - sizeof (sfnt_DirectoryEntry));
	cTables = (int) SWAPW (OffsetTable.numOffsets);
	if (traceflag)	printf("cTables %d\n", cTables); 	/* debugging */

	for ( i = 0; i < cTables && i < 40; i++)  {
		if ((_read (fp, &Table, sizeof (Table))) != sizeof(Table)) {
			printf("Read failed on table #%d\n", i);
			exit(-1);
		}
/*		printf("Table tag #%02d %lx (%lx)\n",
			i, Table.tag, tag_NamingTable); */ /* debugging */
		if (Table.tag == tag_NamingTable) {	/* defined in sfnt_en.h */
/*			printf("ENTERING NAMING TABLE\n"); */ /* debugging */

	/* Now that we've found the entry for the name table, seek to that
	 * position in the file and read in the initial header for this
	 * particular table.  See "True Type Font Files" for information
	 * on this record layout.
	 */
			_lseek (fp, SWAPL (Table.offset), SEEK_SET);
			_read (fp, &NamingTable, sizeof (NamingTable));
			numNames = SWAPW(NamingTable.count);
			if (traceflag) printf("%d Names\n", numNames); /* debugging */
			while (numNames--) {
				_read (fp, &NameRecord, sizeof (NameRecord));
				curseek = _tell(fp);

/* Undefine this next section if you'd like a little bit more info 
 * during the parsing of this particular name table.
 */
				nPlatForm = SWAPW(NameRecord.platformID);
				nNameID = SWAPW(NameRecord.nameID);
				nLanguage = SWAPW(NameRecord.languageID);

				if (traceflag)
					printf("PlatForm %d (%s) NameID %d (%s) Language %d\n",
						   nPlatForm, PlatFormNames[nPlatForm],
						   nNameID, FontNames[nNameID], nLanguage);
/*				or test nPlatForm == 1 */
				if (traceflag && nLanguage == 0) {
printf("(%ld) platform=%u (%s), specific=%u, lang=%x, name=%u (%s) (%u, %u)\n",
					curseek,
					SWAPW(NameRecord.platformID),
					PlatFormNames[nPlatForm],
					SWAPW(NameRecord.specificID),
					SWAPW(NameRecord.languageID),
					SWAPW(NameRecord.nameID),
					FontNames[nNameID],
					SWAPW(NameRecord.offset),
					SWAPW(NameRecord.length));
				}
/* look for (platformID == 1) && (nameID == 4) */
/*				printf("PlatForm %d NamedID %d\n", nPlatForm, nNameID); */
/*				if (nPlatForm == 1 && nNameID == 4) { */
				if (nNameID == 4) {
					if (nPlatForm == 1 || (nPlatForm == 3 && !nHitFullName)) {
/*					printf("PlatForm %d NamedID %d\n", nPlatForm, nNameID); */
/*	ShowNameField (fp, output, pszFile, NameRecord, NamingTable, Table,
						FontNames[nNameID]); */
/*						"Full Name"); */
						_lseek (fp, SWAPW (NameRecord.offset) + 
								SWAPW(NamingTable.stringOffset) + 
								SWAPL(Table.offset), SEEK_SET);
						nLen = SWAPW(NameRecord.length);
						_read (fp, &namebuf, nLen);
/*						namebuf[SWAPW(NameRecord.length)] = '\0'; */
						namebuf[nLen] = '\0';
						if (nPlatForm == 3)	DeUnicode(namebuf, nLen);
/*					printf("%s: FullFontName = %s\n", pszFile, namebuf); */
						if (traceflag)
							printf("%s (TrueType)=%s\n", namebuf, pszFile);
						if (saveflag) {
							stripextension(pszFile);
							insertname(namebuf, pszFile, 0);
							nHitFullName++;
						}
						_lseek (fp, curseek, SEEK_SET);
					}
					else if (verboseflag && nLanguage == 0) {
/*	ShowNameField (fp, output, pszFile, NameRecord, NamingTable, Table,
				FontNames[nNameID]); */
					}
				}
			} /* end of while --numNames loop */
			if (nHitFullName == 0) {
				flag = -1;
				printf("%s: ** No font full name found **\n", pszFile);
			}
			goto cleanup;		/* we did find name tables */
		} /* end of if name table tag is name table tag */
	} /* end of looping over the tables */
	flag = -1;
	printf("%s: ** No name table found **\n", pszFile);

cleanup:
	_close (fp); 
/*  fclose (fp); */
	return flag;
}

int readttf (char *name) {
	int flag;
	flag = DumpNameTable (name);
	if (dotflag) putc('.', stdout);
	return flag;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* This now tries to dynamically determine whether to look in */
/* c:\winnt\fonts or c:\windows\system */

/* This now tries to dynamically determine whether to look for */
/* *.ttf or *.fot files */

/* Changed to ignore/delete old entries in [TTFonts] section */

/* Changed to also look in ATMFolder 97/Jul/25 */
