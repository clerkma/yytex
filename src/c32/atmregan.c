/* Show ATM proprietary data in atmreg.atm */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* #define FILENAME_MAX 256 */

/* #define PATHNAME_MAX 256 */

/* #define _MAX_PATH 260 */ /* max. length of full pathname */

// #define MAXDIRS 64		/* #define MAXDIRS 32 */

int verboseflag=1;
int detailflag=1;
int showflags=0;
int traceflag=1;
int debugflag=0;

int bATM41=0;			/* whether UNICODE strings or not */

int dirflags=1;			/* show directories in file names */

int dirindex;

// char *DirPaths[MAXDIRS];

char **DirPaths=NULL;

FILE *errout=stdout;

/*****************************************************************************/

void perrormod (char *s) {
//	perror (s);
	fprintf(errout, "%s: %s\n", s, strerror(errno));
}

/*****************************************************************************/

unsigned int ureadtwo (FILE *input) {
	unsigned int c, d;
	unsigned int n;
	
	c = getc(input);
	d = getc(input);
	n = (d << 8) | c;
	return n;
}

unsigned long ureadfour (FILE *input) {
	unsigned int a, b, c, d;
	unsigned long n;
	
	a = getc(input);
	b = getc(input);
	c = getc(input);
	d = getc(input);
	n = (d << 8) | c;
	n = (n << 16) | (b << 8) | a;
	return n;
}

/*****************************************************************************/

/* APP: 4.1 Build 243  May 24 2000 15:32:52|DUW410R2100296-131<<<<<<<<<<<<<<< */

unsigned long startfontlist, endfontlist;
unsigned long startdirlist, enddirlist;
unsigned long startsetlist, endsetlist;
int nDirs, nFonts, nSets;

void ReadPointers (FILE *input) {
	long current;
	int c;
	
	(void) fseek(input, 6, SEEK_SET);
	nDirs = ureadtwo(input);			/* 6 number of directory paths */
	nFonts = ureadtwo(input);			/* 8 number of font entries */
	nSets = ureadtwo(input);			/* 10 number of font sets (?) */
	printf("%d Directory paths, %d Font entries, %d Font sets\n",
		  nDirs, nFonts, nSets);
	putc('\n', stdout);
//	(void) fseek(input, 12, SEEK_SET);
	enddirlist = ureadfour(input);		/* 12 endlist */
	(void) ureadfour(input);			/* 16 mystery ??? */
	startfontlist = ureadfour(input);	/* 20 startfontlist */
	startdirlist = ureadfour(input);	/* 24 startdirlist */
	endfontlist = ureadfour(input);		/* 28 endfontlist */
	startsetlist = endfontlist;
	endsetlist = ureadfour(input);		/* 32 endsetlist */
	if (verboseflag) {
		printf("Directory List %lu - %lu; Font List %lu - %lu; Font Set List %lu - %lu\n",
			   startdirlist, enddirlist, startfontlist, endfontlist,
				   endfontlist, endsetlist);
		putc('\n', stdout);
	}
	current = ftell(input);
	fseek(input, endsetlist, SEEK_SET);		/* now show build string */
	(void) getc(input);						/* Check whether build string is UNICODE */
	if (getc(input) == 0) bATM41 = 1;
	else bATM41 = 0;
	printf("Assuming %s character strings\n\n", bATM41 ? "wide" : "narrow");

	fseek(input, endsetlist, SEEK_SET);		/* now show build string */
	for (;;) {
		c = getc(input);
		if (bATM41) (void) getc(input);
		if (c <= 0) break;
		if (c == '<') break;
		putc(c, stdout);
	}
	putc('\n', stdout);
	putc('\n', stdout);
	fseek (input, current, SEEK_SET);
}

int ShowDirectories (FILE *input, int noiseflag) {
	int c, k, noff, nlen;
	int npath=0;
	unsigned long noffset=36;
	char pathname[ _MAX_PATH];
	char *s;

	if (verboseflag) printf("Master Directory List:\n\n");

/*	if (fseek(input, 24, SEEK_SET) >= 0) noffset = ureadfour(input); */

	noffset = startdirlist;			// 36

	for (;;) {
		if (noffset >= enddirlist) break;	/* normal exit from this */
		if (fseek(input, noffset, SEEK_SET) < 0) {
		    if (traceflag) printf("Seek to %ld failed\n", noffset);
			break;	/* 		    return -1; */
		}
		noff = ureadtwo(input);
		if (noff != 8) {
			if (traceflag) printf("noff != 8\n");
/*			break; */	/* new sanity check */
		}
		nlen = ureadtwo(input);
		if (nlen == 0) {
			if (traceflag) printf("nlen == 0\n");
			break;		/* sanity check */
		}
		if (nlen > _MAX_PATH) {
			if (traceflag) printf("nlen > %d\n", _MAX_PATH);
			break; 		/* new sanity check */
		}
		noffset = ureadfour(input);
		if (noffset == 0) {
			if (traceflag) printf("noffset == 0\n");
			break;	/* sanity check */
		}
		s = pathname;
		for (k = 0; k < nlen; k++) {
			c = getc(input);
			if (bATM41) (void) getc(input);
			if (c == EOF) {
				if (traceflag) printf("Unexpected EOF\n");
				return -1;
			}
			*s++ = (char) c;
			if (c == 0) break;
		}
		npath++;
		if (noiseflag) printf("%d\t%s\n", npath, pathname);
//		if (dirindex >= MAXDIRS) 
//		if (dirindex >= nDirs) {
		if (dirindex > nDirs) {
			if (traceflag) printf("Too many paths (> %d)\n", dirindex);
		    return -1;
		}
		DirPaths[dirindex] = _strdup(pathname);
		dirindex++;
	}
	if (noiseflag) putc('\n', stdout);
	return 0;
}

/* Skip over dir paths. Returns number of paths seen in list */
/* Returns -1 if it failed for some reason (like fseek error) */

#ifdef IGNORED
int skipdirectories (FILE *input) {		/* not used anymore */
	int n, nlen;
	int npath=0;				/* how many paths seen */
	unsigned long noffset=36;	/* first directory entry */

	noffset = startdirlist;
	for (;;) {
		if (noffset >= enddirlist) break;
		if (fseek(input, noffset, SEEK_SET) < 0) return -1;
		n = ureadtwo(input);
/*		if (n != 8) break; */			/* check offset flag */
		nlen = ureadtwo(input);			/* length of this entry */
		if (nlen == 0) break;
		noffset = ureadfour(input);		/* start of next entry */
		if (noffset == 0) break;
		npath++;
	}
/*	fseek(input, enddirlist, SEEK_SET); */ /* simpler! do this instead */
	return npath;
}
#endif

int ReadString (FILE *input, char *name, int nlen) {
	int c;
	int n=0;
	char *s=name;

	*s = '\0';				/* in case we pop out early */
/*	c = getc(input); */		/* always read first byte ... */
/*	*s++ = (char) c; */
/*	n++; */					/* changed 1996/June/4 */
	for (;;) {				/* read string up to zero byte */
		c = getc(input);
		if (bATM41) (void) getc(input);
		if (c == EOF) {
			if (traceflag) printf("Hit EOF in ReadString\n");
			*s++ = '\0';
			return -1;
		}
		*s++ = (char) c;
		if (c == 0) break;
		n++;
		if (n >= nlen) {	/* too long */
			*(name+nlen-1) = '\0';
		    if (traceflag)
				printf("Exceeded string space %d (%s)\n", nlen, name);
			*name = '\0';
		    return -1;
		}
	}
	return 0;
}

/***************************************************************************/

void showhex (unsigned int n) {
	int c, d;
	c = n >> 4;
	d = n & 15;
	if (c > 9) putc(c + 'A' - 10, stdout);
	else putc(c + '0', stdout);
	if (d > 9) putc(d + 'A' - 10, stdout);
	else putc(d + '0', stdout);
}

#define MAXFACENAME 32
#define MAXSTYLENAME 32
#define MAXFULLNAME 64
#define MAXFONTNAME 64
/* #define MAXPFMNAME 16 */
#define MAXPFMNAME 32
/* #define MAXPFBNAME 16 */
#define MAXPFBNAME 32

void showtheflags (unsigned int flag[]) {				/* show flags in hex */
	int k;
	for (k = 0; k < 16; k++) {
		if (flag[k] <= 9) putc(flag[k] + '0', stdout);
		else if (flag[k] <= 15) putc(flag[k] + 'A' - 10, stdout);
		else {
			showhex(flag[k]);
			putc(' ', stdout); 
		}
	}
}

int showallfonts (FILE *input) {
	unsigned long next;
	unsigned int stroffset, nlen;
	int c, k;
	int bold, italic;				/* style bits */
	int boldx, italicx;				/* from TT StyleName */
	int ps, ttf, mmm, mmi, gen;		/* font type bits */
	int pscount=0;					/* number of T1 fonts */
	int ttfcount=0;					/* number of TT fonts */
	int mmmcount=0;					/* number of MM font masters */
	int mmicount=0;					/* number of MM font instances */
	int gencount=0;					/* number of generic MM fonts */
	int total=0;
	int nMMM, nPFB, nPFM;
	unsigned int flag[16];			/* 16 bytes of flags */
	char FaceName[MAXFACENAME+1];	/* Windows Face Name */
	char StyleName[MAXSTYLENAME+1];	/* Style Name for TT font */
	char FullName[MAXFULLNAME+1];	/* Full Name */
	char FontName[MAXFONTNAME+1];	/* Font Name */
	char MMMName[MAXPFMNAME+1];		/* MMM file or TTF file or PFM file */
	char PFBName[MAXPFBNAME+1];		/* PFB file or PSS file */
	char PFMName[MAXPFMNAME+1];		/* PFM name of MMM font */

	fseek(input, startfontlist, SEEK_SET);
	if (verboseflag) printf("Master Font List:\n");
/*	if (findfontstart(input) < 0) return -1; */
	putc('\n', stdout);
	if (detailflag) ;
	else 
		printf("Flags TTF/PS `FaceName' (BI) `StyleName' `FullName' `FontName'\n");
	putc('\n', stdout);
	for (;;) {
		stroffset = ureadtwo(input);	/* offset to first string == 44 */
		if (stroffset != 44) {			/* sanity check */
			if (traceflag) printf("WARNING: stroffset != 44\n");
		}
		nlen = ureadtwo(input);			/* length of this record in bytes */
		next = ureadfour(input);		/* pointer to next record */
		if (debugflag)
			printf("stroff %d nlen %d next %ld\n", stroffset, nlen, next);
		for (k = 0; k < (28 - 8); k++) (void) getc(input);
		for (k = 0; k < 16; k++) flag[k] = getc(input); /* read flags */
/*		showtheflags(flag); */
/*		if (flag[0] < 0) break; */
/*		putc(' ', stdout); */
/*		if (flag[0] != 144 && flag[0] != 188) break; */
		c = flag[0];
/*		if (c != 144 && c != 188 && c != 44 && c != 88) break; */
		bold = flag[1];
/*		conceivably, 0 could be light and 3 could be heavy */
		if (bold == 0 || bold > 2) {
			if (traceflag)
printf("ERROR: ILLEGAL VALUE OF %s FLAG %x (should be %d or %d) ***********\n",
	   "BOLD", bold, 1, 2);
			if (bold > 2) bold = 1;	/* pretends its OK;
/*			break; */				/* removed 97/Sep/14 */
		}
		else bold = bold - 1;
/*		NOTE: Times New Roman MT Extra Bold has 3 and style `Regular' ! */
/*		Some TCI fonts have 44 in BoldItalic style ! */
		italic = flag[2];
		if (italic > 1) {
			if (traceflag)
printf("ERROR: ILLEGAL VALUE OF %s FLAG %x (should be %d or %d) **********\n",
	   italic, "ITALIC", 0, 1);
			italic = 1;
/*			break; */				/* removed 97/Sep/14 */
		}
		if (detailflag) putc('\n', stdout);
		if (showflags) {
			showtheflags(flag);
			putc(' ', stdout);
		}
/*		ttf = flag[5]; */
		ttf = ps = mmm = mmi = gen = 0;
		if (flag[4] == 0) ttf = 1;
		else if (flag[4] == 1) ps = 1;
		else if (flag[4] == 2) mmm = 1;		
		else if (flag[4] == 4) mmi = 1;		
		if (flag[6] == 10) {
			gen = 1;			/* generic MM master */
			mmm = 0;			/* not count as normal MM master */
		}
		nMMM = flag[8] | (flag[9] << 8);
		nPFB = flag[10] | (flag[11] << 8);
		nPFM = flag[12] | (flag[13] << 8);
/*		if (flag[13] || flag[15]) */
		if (flag[13])
printf("******************************************************************\n");
/*		if (flag[7] || flag[9] || flag[11]) */
		if (flag[9] || flag[11])
printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
		if (flag[3])
printf("##################################################################\n");
/*		flag[4] == 0 for TTF, 1 for PS, 2 for MM */
		if (ttf) ttfcount++;
		else if (gen) gencount++;
		else if (mmm) mmmcount++;
		else if (mmi) mmicount++;
		else pscount++;
		if (ttf) printf("TTF ");
		else if (gen) printf("GEN ");
		else if (mmm) printf("MMM ");
//		else if (mmi) printf("PSS ");
		else if (mmi) printf("MMI ");
		else printf("T1  ");
/*	Windows Face Name */
	    if (ReadString (input, FaceName, sizeof(FaceName)) < 0) goto donext;
/*	Style Name (will be empty string for PS SM or MM font) */
	    if (ReadString (input, StyleName, sizeof(StyleName)) < 0) goto donext;
/*	FullName will be empty string for a T1 SM or MM font */
	    if (ReadString (input, FullName, sizeof(FullName)) < 0) goto donext;
/*	FontName may be an empty string if PFB not yet inspected ? */
	    if (ReadString (input, FontName, sizeof(FontName)) < 0) goto donext;
/*	Name of MMM file or PFM file or TTF file */ 
	    if (ReadString (input, MMMName, sizeof(MMMName)) < 0) goto donext;
/*	Name of PFB file or PSS file */ 
	    if (ReadString (input, PFBName, sizeof(PFBName)) < 0) goto donext;
/*	Name of PFM file in case of MMM font */ 
	    if (ReadString (input, PFMName, sizeof(PFMName)) < 0) goto donext;
		if (detailflag) {
			printf("FaceName: `%s' STYLE: %s%s StyleName: `%s' FullName: `%s' FontName: `%s'\n",
			   FaceName, bold ? "BOLD" : "", italic ? "ITALIC" : "", 
			   StyleName, FullName, FontName);
//			printf("MMMName: `%s' PFBName: `%s' PFMName: `%s' %s\n",
//			   MMMName, PFBName, PFMName,  ttf ? "(TT)" : "");
			printf("MMMName: `%s%s' ", DirPaths[nMMM], MMMName);
			printf("PFBName: `%s%s' ", DirPaths[nPFB], PFBName);
			printf("PFMName: `%s%s' ", DirPaths[nPFM], PFMName);			
			putc('\n', stdout);
		}
		else {
			printf("`%s' (%s%s%s)", FaceName, (!bold && !italic) ? "R" : "",
		    bold ? "B" : "", italic ? "I" : "");
/* For Type 1 fonts, Style Name and FullName will be empty */
/* For Type 1 fonts, FontName may be empty also if not yet filled in */		
/*		printf(" `%s' `%s' `%s'\n", StyleName, FullName, FontName); */
			printf(" `%s' `%s' `%s'\n", StyleName, FullName, FontName);
			if (dirflags == 0) {
				if (strcmp(MMMName, "") != 0) printf("\t`%s'", MMMName);
				if (strcmp(PFBName, "") != 0) printf("\t`%s'", PFBName);
				if (strcmp(PFMName, "") != 0) printf("\t`%s'", PFMName);
				putc('\n', stdout);
			}
			else {
				if (strcmp(MMMName, "") != 0)
					printf("\t`%s%s'", DirPaths[nMMM], MMMName);
				if (strcmp(PFBName, "") != 0)
					printf("\t`%s%s'", DirPaths[nPFB], PFBName);
				if (strcmp(PFMName, "") != 0)
					printf("\t`%s%s'", DirPaths[nPFM], PFMName);			
				putc('\n', stdout);
			}
		}

		if (ttf) {	/* check style bits */
			boldx=0; italicx=0;
			if (strstr(StyleName, "Regular") != NULL) ;
			if (strstr(StyleName, "Roman") != NULL) ;
			if (strstr(StyleName, "Medium") != NULL) ;
			if (strstr(StyleName, "Italic") != NULL) italicx = 1;
			if (strstr(StyleName, "Oblique") != NULL) italicx = 1;
			if (strstr(StyleName, "Kursiv") != NULL) italicx = 1;
			if (strstr(StyleName, "Bold") != NULL) boldx = 1;
			if (strstr(StyleName, "Demibold") != NULL) boldx = 1;
			if (strstr(StyleName, "Semibold") != NULL) boldx = 1;
			if (strstr(StyleName, "Fett") != NULL) boldx = 1;
			if (strstr(StyleName, "Halbfett") != NULL) boldx = 1;
			if (italicx != italic || boldx != bold)
printf("ERROR: MISMATCH STYLE BITS (bold %d italic %d) - TRUETYPE STYLENAME (bold %d italic %d) *********\n",
	  bold, italic, boldx, italicx);
		}
donext:
/*		if (findfontstart(input) < 0) break; */
		if (next >= endfontlist) {
/*			if (traceflag) printf("next >= endfontlist\n"); */
			break;						/* normal exit */
		}
		if (fseek(input, next, SEEK_SET) < 0) break;
	}
	total =  ttfcount + pscount + mmmcount + gencount + mmicount + total; 
	putc('\n', stdout);
	putc('\n', stdout);
 printf("ATMREG: %d TTF + %d T1 + %d MMM + %d GEN + %d MMI = %d total fonts\n",
		  ttfcount, pscount, mmmcount, gencount, mmicount, total); 
	putc('\n', stdout);
	return 0;
}

/* T1 fonts: Face_Name, FontName,  PFM_File, PFB_File */
/* TT fonts: Face_Name, StyleName, FullName, PS Name, TTF_File_name */

/* From ATM.INI in Windows directory */
/* [Settings] */
/* ACPBase=C:\WINDOWS.000\ATMREG.ATM */

char atmreg[FILENAME_MAX]; 

FILE *openatmreg(char *regname) {
	FILE *input;
/*	char atmreg[FILENAME_MAX]; */
	char *windir;

	*atmreg = '\0';
/*	Easy to find Windows directory if Windows runs */
/*	Or if user kindly set WINDIR environment variable */
/*	Or if running in Windows NT */	
	if ((windir = getenv("windir")) != NULL ||
		(windir = getenv("WINDIR")) != NULL ||
		(windir = getenv("winbootdir")) != NULL ||
		(windir = getenv("SystemRoot")) != NULL ||
		(windir = getenv("SYSTEMROOT")) != NULL) {
		strcpy(atmreg, windir);
		strcat(atmreg, "\\");
		strcat(atmreg, regname);
		if ((input = fopen(atmreg, "rb")) != NULL) return input;
		else fprintf(errout, "Could not find %s in %s\n", regname, windir);
	}
	else fprintf(errout, "Could not find windows directory\n");

	*atmreg = '\0';
	_searchenv (regname, "PATH", atmreg);
	if (*atmreg != '\0') {
		if ((input = fopen(atmreg, "rb")) != NULL) return input;
	}

	strcpy(atmreg, "c:\\windows\\");
	strcat(atmreg, regname);
	if ((input = fopen(atmreg, "rb")) != NULL) return input;
	strcpy(atmreg, "c:\\winnt\\");
	strcat(atmreg, regname);
	if ((input = fopen(atmreg, "rb")) != NULL) return input;
	strcpy(atmreg, "c:\\windows.000\\");
	strcat(atmreg, regname);
	if ((input = fopen(atmreg, "rb")) != NULL) return input;
	strcpy(atmreg, "c:\\psfonts\\");
	strcat(atmreg, regname);
	if ((input = fopen(atmreg, "rb")) != NULL) return input;

	strcpy(atmreg, "c:\\temp\\");
	strcat(atmreg, regname);
	if ((input = fopen(atmreg, "rb")) != NULL) return input;
	atmreg[0] = 'd';
	if ((input = fopen(atmreg, "rb")) != NULL) return input;

	perrormod(regname);
	return NULL;
}

/* void __cdecl _searchenv(const char *, const char *,	char *); */

// void presetdirs (void) {
//     int k;
//	DirPaths[0] = _strdup("");
//	for (k = 1; k <= nDirs; k++) DirPaths[k] = NULL;
//	dirindex=1;
// }

void AllocDirs (int nDirs) {
	int k, nlen;
	nlen = (nDirs + 1) * sizeof(char *);
	DirPaths = (char **) malloc (nlen);
	if (DirPaths == NULL) {
		printf("ERROR: unable to allocate %d bytes for %d dir entries\n",
			   nlen, nDirs);
		exit(1);
	}
	DirPaths[0] = _strdup("");
	for (k = 1; k <= nDirs; k++) DirPaths[k] = NULL;
	dirindex=1;
}

void FreeDirs (void) {
    int k;
	for (k = 0; k <= nDirs; k++) {
		if (DirPaths[k] != NULL) free(DirPaths[k]);
		DirPaths[k] = NULL;
    }
    dirindex=1;
	free(DirPaths);
	DirPaths = NULL;
}

int main (int argc, char *argv[]) {
	FILE *input;
	int firstarg=1;

//	presetdirs();
//	printf("ATMREG.ATM analyzer 1.2 Copyright (C) 1996--1999 Y&Y, Inc.\n");
	printf("ATMREG.ATM analyzer 1.3 (C) 1996--2000 Y&Y, Inc. http://www.YandY.com\n");

	if (argc > firstarg) {
		strcpy(atmreg, argv[firstarg]);
		if ((input = fopen(atmreg, "rb")) == NULL) {
			perrormod(atmreg);
			exit(1);
		}
	}
	else {
		if ((input = openatmreg("atmreg.atm")) == NULL) exit(1);
	}

	printf("Analyzing: %s\n\n", atmreg);
	ReadPointers(input);
	AllocDirs(nDirs);
	if (dirflags) ShowDirectories(input, 1);
	showallfonts(input);
	fclose (input);
	FreeDirs();
	return 0;
}

#ifdef IGNORE
flag[0]	    144	or 188 (or 44 or 88) or ...
flag[1]	    1 (medium)  or 2 (bold)
flag[2]	    0 (upright) or 1 (italic)
flag[3]		0
flag[4]		0 (TTF) or 1 (PS) or 2 (MMM) or 4 (MMI)
flag[5]	    0 (PS / MMM / MMI) or 1 (TTF)
flag[6]	    0 (TTF) or 8 (PS / MMM / MMI) or 10 (generic MM)
flag[7]	    0 or 1
flag[8]	    nMMM directory index for MMM, or TTF or PFM file
flag[9]	    ..
flag[10]    nPFB directory index for PFB, or PSS file
flag[11]    ..
flag[12]    nPFM directory index for PFM if MMM font
flag[13]    ..
flag[14]    0 or 111 or 63 ?
flag[15]    0 or 1 ?
#endif

#ifdef IGNORED
adr 12 (4)	ptr start of X (end of directory list)
adr 16 (4)  ptr start of Y (end of X)
adr 20 (4)  ptr start of font list (end of Y)
adr 24 (4)  ptr start of directory list
adr 28 (4)  ptr start of fontset list (end of font list)
adr 32 (4)  ptr exe version signature (end of fontset list)
adr 36      start of directory list
#endif

