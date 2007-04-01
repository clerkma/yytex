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

#define MAXLINE 512

#define MAXCHARNAME 128

#define MAXFILENAME 128

#define MAXVECTORNAME 64

#define MAXCHRS 256

int verboseflag=1;
int traceflag=0;
int approximate=0;

char *outext="tra";			/* extension for output file */

char *vecpath=NULL;

char line[MAXLINE];

int remap[MAXCHRS];

int badhit[MAXCHRS];

char *encodingin[MAXCHRS];

char *encodingout[MAXCHRS];

#ifdef MACENCODING
char *macencoding[MAXCHRS]={
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle", 
"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", 
"zero", "one", "two", "three", "four", "five", "six", "seven", 
"eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question", 
"at", "A", "B", "C", "D", "E", "F", "G", 
"H", "I", "J", "K", "L", "M", "N", "O", 
"P", "Q", "R", "S", "T", "U", "V", "W", 
"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", 
"grave", "a", "b", "c", "d", "e", "f", "g", 
"h", "i", "j", "k", "l", "m", "n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w", 
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "del", 
"Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis", "aacute", 
"agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla", "eacute", "egrave", 
"ecircumflex", "edieresis", "iacute", "igrave", "icircumflex", "idieresis", "ntilde", "oacute", 
"ograve", "ocircumflex", "odieresis", "otilde", "uacute", "ugrave", "ucircumflex", "udieresis", 
"dagger", "degree", "cent", "sterling", "section", "bullet", "paragraph", "germandbls", 
"registered", "copyright", "trademark", "acute", "dieresis", "notequal", "AE", "Oslash", 
"infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu", "partialdiff", "summation", 
"product", "pi", "integral", "ordfeminine", "ordmasculine", "Omega", "ae", "oslash", 
"questiondown", "exclamdown", "logicalnot", "radical", "florin", "approxequal", "Delta", "guillemotleft", 
"guillemotright", "ellipsis", "nbspace", "Agrave", "Atilde", "Otilde", "OE", "oe", 
"endash", "emdash", "quotedblleft", "quotedblright", "quoteleft", "quoteright", "divide", "lozenge", 
"ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft", "guilsinglright", "fi", "fl", 
"daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase", "perthousand", "Acircumflex", "Ecircumflex", "Aacute", 
"Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex", 
"apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde", 
"macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron"
};
#endif

#ifdef IBMENCODING
char *IBMencoding[MAXCHRS]={
"notdef", "smileface", "invsmileface", "heart", "diamond", "club", "spade", "bullet", 
"invbullet", "circle", "invcircle", "male", "female", "musicalnote", "musicalnotedbl", "sun", 
"triagrt", "triaglf", "arrowupdn", "exclamdbl", "paragraph", "section", "filledrect", "arrowupdnbse", 
"arrowup", "arrowdown", "arrowright", "arrowleft", "orthogonal", "arrowboth", "triagup", "triagdn", 
"space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quoteright", 
"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", 
"zero", "one", "two", "three", "four", "five", "six", "seven", 
"eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question", 
"at", "A", "B", "C", "D", "E", "F", "G", 
"H", "I", "J", "K", "L", "M", "N", "O", 
"P", "Q", "R", "S", "T", "U", "V", "W", 
"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", 
"quoteleft", "a", "b", "c", "d", "e", "f", "g", 
"h", "i", "j", "k", "l", "m", "n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w", 
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "house", 
"Ccedilla", "udieresis", "eacute", "acircumflex", "adieresis", "agrave", "aring", "ccedilla", 
"ecircumflex", "edieresis", "egrave", "idieresis", "icircumflex", "igrave", "Adieresis", "Aring", 
"Eacute", "ae", "AE", "ocircumflex", "odieresis", "ograve", "ucircumflex", "ugrave", 
"ydieresis", "Odieresis", "Udieresis", "cent", "sterling", "yen", "peseta", "florin", 
"aacute", "iacute", "oacute", "uacute", "ntilde", "Ntilde", "ordfeminine", "ordmasculine", 
"questiondown", "revlogicalnot", "logicalnot", "onehalf", "onequarter", "exclamdown", "guillemotleft", "guillemotright", 
"ltshade", "shade", "dkshade", "2502", "2524", "2561", "2562", "2556", 
"2555", "2563", "2551", "2557", "255d", "255c", "255b", "2510", 
"2514", "2534", "252c", "251c", "2500", "253c", "255e", "255f", 
"255a", "2554", "2569", "2566", "2560", "2550", "256c", "2567", 
"2568", "2564", "2565", "2559", "2558", "2552", "2553", "256b", 
"256a", "2518", "250c", "block", "dnblock", "lfblock", "rtblock", "upblock", 
"alpha", "germandbls", "Gamma", "pi", "summation", "sigma", "mu", "tau", 
"Phi", "Theta", "Omega", "delta", "infinity", "phi", "epsilon", "intersection", 
"equivalence", "plusminus", "greaterequal", "lessequal", "integraltp", "integralbt", "divide", "approxequal", 
"degree", "middot", "bulletsml", "radical", "nsuperior", "twosuperior", "filledbox", "nbspace"
};
#endif

#ifdef ANSIENCODING
char ansiencoding[MAXCHRS] = {
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"", "", "", "", "", "", "", "", 
"space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle", 
"parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", 
"zero", "one", "two", "three", "four", "five", "six", "seven", 
"eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question", 
"at", "A", "B", "C", "D", "E", "F", "G", 
"H", "I", "J", "K", "L", "M", "N", "O", 
"P", "Q", "R", "S", "T", "U", "V", "W", 
"X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", 
"grave", "a", "b", "c", "d", "e", "f", "g", 
"h", "i", "j", "k", "l", "m", "n", "o", 
"p", "q", "r", "s", "t", "u", "v", "w", 
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "notdef", 
"notdef", "notdef", "quotesinglbase", "florin", "quotedblbase", "ellipsis", "dagger", "daggerdbl", 
"circumflex", "perthousand", "Scaron", "guilsinglleft", "OE", "notdef", "notdef", "notdef", 
"notdef", "quoteleft", "quoteright", "quotedblleft", "quotedblright", "bullet", "endash", "emdash", 
"tilde", "trademark", "scaron", "guilsinglright", "oe", "notdef", "notdef", "Ydieresis", 
"nbspace", "exclamdown", "cent", "sterling", "currency", "yen", "brokenbar", "section", 
"dieresis", "copyright", "ordfeminine", "guillemotleft", "logicalnot", "hyphen", "registered", "macron", 
"degree", "plusminus", "twosuperior", "threesuperior", "acute", "mu", "paragraph", "periodcentered", 
"cedilla", "onesuperior", "ordmasculine", "guillemotright", "onequarter", "onehalf", "threequarters", "questiondown", 
"Agrave", "Aacute", "Acircumflex", "Atilde", "Adieresis", "Aring", "AE", "Ccedilla", 
"Egrave", "Eacute", "Ecircumflex", "Edieresis", "Igrave", "Iacute", "Icircumflex", "Idieresis", 
"Eth", "Ntilde", "Ograve", "Oacute", "Ocircumflex", "Otilde", "Odieresis", "multiply", 
"Oslash", "Ugrave", "Uacute", "Ucircumflex", "Udieresis", "Yacute", "Thorn", "germandbls", 
"agrave", "aacute", "acircumflex", "atilde", "adieresis", "aring", "ae", "ccedilla", 
"egrave", "eacute", "ecircumflex", "edieresis", "igrave", "iacute", "icircumflex", "idieresis", 
"eth", "ntilde", "ograve", "oacute", "ocircumflex", "otilde", "odieresis", "divide", 
"oslash", "ugrave", "uacute", "ucircumflex", "udieresis", "yacute", "thorn", "ydieresis",
};
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); 
			strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

void removeexten(char *fname) {  /* remove extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) != NULL) {
		if ((t = strrchr(fname, '\\')) == NULL || s > t) *s = '\0';
	}
} 

char *stripname(char *filename) {
	char *s;
	if ((s = strrchr(filename, '\\')) != NULL) return (s+1);
	if ((s = strrchr(filename, '/')) != NULL) return (s+1);
	if ((s = strrchr(filename, ':')) != NULL) return (s+1);
	return filename;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int readencodingfile (FILE *input, char *encoding[]) {
	int k, count=0;
	char charname[MAXCHARNAME];

	for (k = 0; k < MAXCHRS; k++) encoding[k] = "";
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == ';' || *line == '%' || *line == '\n') continue;
		if (sscanf(line, "%d %s", &k, charname) == 2) {
			if (k >= 0 && k < MAXCHRS) {
/*				encoding[k] = strdup(charname); */
				encoding[k] = _strdup(charname);
				count++;
			}
/*			else fputs(line, stderr); */
			else fprintf(stderr, "k %d line %s", k, line);
		}
/*		else fputs(line, stderr); */
		else fprintf(stderr, "Can't read line %s", line);
	}
	return count;
}

FILE *getvectorfile (char *vector) {
	FILE *input;
	char vectorfile[MAXFILENAME];
	char *s;
	
	strcpy(vectorfile, vector);				/* try as is */
	extension(vectorfile, "vec");
	if ((input = fopen(vectorfile, "r")) != NULL) return input;

/*	vecpath = getenv("VECPATH");  */
	if (vecpath != NULL) {
		strcpy(vectorfile, vecpath);
		s = vectorfile + strlen(vectorfile) - 1;
		if (*s != '\\' && *s != '/') strcat(s, "\\");
		strcat(vectorfile, vector);	
		extension(vectorfile, "vec");
		if ((input = fopen(vectorfile, "r")) != NULL) return input;
		strcpy(vectorfile, vecpath);
		s = vectorfile + strlen(vectorfile) - 1;
		if (*s != '\\' && *s != '/') strcat(s, "\\");
		strcat(vectorfile, stripname(vector));	
		extension(vectorfile, "vec");
		if ((input = fopen(vectorfile, "r")) != NULL) return input;
	}
	return NULL;
}

int removeduplicates(char *encoding[]) {
	int i, j, count=0;
	for (i = 0; i < MAXCHRS; i++) {
		if (strcmp(encoding[i], "") == 0) continue;
		for (j = i+1; j < MAXCHRS; j++) {
			if (strcmp(encoding[j], "") == 0) continue;
			if (strcmp(encoding[i], encoding[j]) == 0) {
				fprintf(stderr, "WARNING: repeat encoding for `%s' (%d %d)\n",
					   encoding[i], i, j);
				encoding[j] = "";
				count++;
			}
		}
	}
	return count;
}

int readencoding (char *vector, char *encoding[]) {
	FILE *input;
	int n;
	if ((input = getvectorfile(vector)) == NULL) return 0;
	if (traceflag) printf("Have a file handle\n");
	n = readencodingfile (input, encoding);
	fclose(input);
	removeduplicates(encoding);		/* 95/Mar/20 */
	if (n == 0) return 0;			/* failed */
	else return 1;					/* success */
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int lookup (char *name, char *encoding[]) {
	int k;
	if (strcmp(name, "") == 0) return -1;
	for (k = 0; k < MAXCHRS; k++) 
		if (strcmp(name, encoding[k]) == 0) return k;
	return -1;
}

char *synonyms[][2] = {
{"nbspace", "space"},
{"ring", "degree"},
{"degree", "ring"},
{"minus", "hyphen"},
{"hyphen", "minus"},
{"endash", "hyphen"},
{"emdash", "hyphen"},
{"brokenbar", "bar"},
{"bar", "brokenbar"},
{"asciicircum", "circumflex"},
{"circumflex", "asciicircum"},
{"asciitilde", "tilde"},
{"tilde", "asciitilde"},
{"grave", "quoteleft"},
{"acute", "quoteright"},
{"quotedblleft", "quotedbl"},
{"quotedblright", "quotedbl"},
{"quotedblbase", "quotedbl"},
{"quotesingle", "quoteleft"},
{"quotesinglbase", "quoteleft"},
{"guilsinglleft", "guillemotleft"},
{"guilsinglright", "guillemotright"},
{"", ""}
};

void mapremap (int identity) {	/* flag non-zero => fill in gaps */
	int k, n;
	char charname[MAXCHARNAME];
	
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(encodingin[k], "") == 0) remap[k] = k;
		else {
			remap[k] = lookup (encodingin[k], encodingout);
			if (remap[k] < 0 && approximate)  {
				strcpy(charname, "");
				for (n=0; n < 128; n++) {
					if (strcmp(synonyms[n][0], "") == 0) break;
					if (strcmp(synonyms[n][0], encodingin[k]) == 0) {
						strcpy(charname, synonyms[n][1]); break;
					}
				}
				remap[k] = lookup (charname, encodingout);
			}
			if (remap[k] < 0 && identity) remap[k] = k;
		}
	}
}

int remapfile (FILE *output, FILE *input) {
	int c, d, k,count=0;
	for (k = 0; k < MAXCHRS; k++) badhit[k] = 0;
	while ((c = getc(input)) != EOF) {
		d = remap[c];
		if (d < 0) {
			if (traceflag) printf("char %d %s ", c, encodingin[c]);
			badhit[c]++;
			d = c;
		} 
		putc(d, output);
	}
	for (k = 0; k < MAXCHRS; k++)
		if (badhit[k] != 0) count += badhit[k];
	return count;
}

void showbadhits(FILE *output) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (badhit[k] != 0) {
			fprintf(output, "char %d (%d hit%s)   \t%s\n",
				k, badhit[k], (badhit[k] > 1) ? "s" : "",
					encodingin[k]);
		}
	}
}

void showremap(FILE *output) {	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (remap[k] != k ||
			(strcmp(encodingin[k], "") != 0 &&
				strcmp(encodingin[k], encodingout[remap[k]]) != 0)
			) {
			fprintf(output, "%d\t%d\t%% %s",
				k, remap[k], encodingin[k]);
			if (remap[k] >= 0 &&
				strcmp(encodingin[k], encodingout[remap[k]]) != 0)
					fprintf(output, " => %s", encodingout[remap[k]]);
			putc('\n', output);
		}
	}
}

void showusage (char *argv[]) {
	printf("%s [-{v}{a}] <in-vector> <out-vector> [<in-filename>]\n", argv[0]);
	printf("\t-v  verbose mode\n");
	printf("\t-a  allow approximate match\n");
	printf("\n\toutput file in current directory, extension `tra'\n");
	printf("\n\twithout file argument shows translation table\n");
	exit(1);

}

/* *** *** *** *** *** *** *** NEW APPROACH TO `ENV VARS' *** *** *** *** */

/* grab `env variable' from `dviwindo.ini' or from DOS environment 94/May/19 */

/*	if (usedviwindo)  setupdviwindo(); 	*/ /* need to do this before use */

int usedviwindo = 1;		/* use [Environment] section in `dviwindo.ini' */
							/* reset if setup of dviwindo.ini file fails */

char *dviwindo = "";			/* full file name for dviwindo.ini with path */

/* #define FNAMELEN 80 */

/* set up full file name for ini file and check for specified section */
/* e.g. dviwindo = setupinifile ("dviwindo.ini", "[Environment]") */

char *setupinifile (char *ininame, char *section) {	
	char fullfilename[FILENAME_MAX];
	FILE *input;
	char *windir;
	char line[MAXLINE];
	int m;

/*	Easy to find Windows directory if Windows runs */
/*	Or if user kindly set WINDIR environment variable */
/*	Or if running in Windows NT */	
	if ((windir = getenv("windir")) != NULL ||	/* 1994/Jan/22 */
		(windir = getenv("WINDIR")) != NULL ||
		(windir = getenv("winbootdir")) != NULL ||
		(windir = getenv("SystemRoot")) != NULL ||
		(windir = getenv("SYSTEMROOT")) != NULL) { /* 1995/Jun/23 */
		strcpy(fullfilename, windir);
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);
	}
	else _searchenv (ininame, "PATH", fullfilename);

	if (strcmp(fullfilename, "") == 0) {		/* ugh, try standard place */
		strcpy(fullfilename, "c:\\windows");
		strcat(fullfilename, "\\");
		strcat(fullfilename, ininame);		
	}

/*	if (*fullfilename != '\0') { */
/*	check whether ini file actually has required section */
		if ((input = fopen(fullfilename, "r")) != NULL) {
			m = strlen(section);
			while (fgets (line, sizeof(line), input) != NULL) {
				if (*line == ';') continue;
/*				if (strncmp(line, section, m) == 0) { */
				if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
					fclose(input);
					return _strdup(fullfilename);
				}
			}					
			fclose(input);
		}
/*	} */
	return "";							/* failed, for one reason or another */
}

int setupdviwindo (void) {
	if (usedviwindo == 0) return 0;		/* already tried and failed */
	if (*dviwindo != '\0') return 1;	/* already tried and succeeded */
/*	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");  */
	dviwindo = setupinifile ("dviwindo.ini", "[Environment]");
	if (*dviwindo == '\0') usedviwindo = 0; /* failed, don't try this again */
	return (*dviwindo != '\0');
}

char *grabenvvar (char *varname, char *inifile, char *section, int useini) {
	FILE *input;
	char line[MAXLINE];
	char *s;
	int m, n;

	if (useini == 0 || *inifile == '\0')
		return getenv(varname);	/* get from environment */
	if ((input = fopen(inifile, "r")) != NULL) {
		m = strlen(section);
/* search for [...] section */
		while (fgets (line, sizeof(line), input) != NULL) {
			if (*line == ';') continue;
/*			if (strncmp(line, section, m) == 0) { */
			if (_strnicmp(line, section, m) == 0) {	/* 95/June/23 */
/* search for varname=... */
				n = strlen(varname);
				while (fgets (line, sizeof(line), input) != NULL) {
					if (*line == ';') continue;
					if (*line == '[') break;
/*					if (*line == '\n') break; */	/* ??? */
					if (*line <= ' ') continue;		/* 95/June/23 */
/*					if (strncmp(line, varname, n) == 0 && */
					if (_strnicmp(line, varname, n) == 0 &&
						*(line+n) == '=') {	/* found it ? */
							fclose (input);
							/* flush trailing white space */
							s = line + strlen(line) - 1;
							while (*s <= ' ' && s > line) *s-- = '\0';
							if (traceflag)  /* DEBUGGING ONLY */
								printf("%s=%s\n", varname, line+n+1);
							return _strdup(line+n+1);
					}							
				}	/* end of while fgets */
			}	/* end of search for [Environment] section */
		}	/* end of while fgets */
		fclose (input);
	}	/* end of if fopen */
/*	useini = 0; */				/* so won't try this again ! need & then */
	return getenv(varname);	/* failed, so try and get from environment */
}							/* this will return NULL if not found anywhere */

char *grabenv (char *varname) {	/* get from [Environment] in dviwindo.ini */
	return grabenvvar (varname, dviwindo, "[Environment]", usedviwindo);
}

#ifdef NEEDATMINI
/* grab setting from `atm.ini' 94/June/15 */

/*	if (useatmini)  setupatmini(); 	*/ /* need to do this before use */

int useatmini = 1;			/* use [Setup] section in `atm.ini' */
							/* reset if setup of atm.ini file fails */

char *atmininame = "atm.ini";		/* name of ini file we are looking for */

char *atmsection = "[Setup]";		/* ATM.INI section */

char *atmini = "";				/* full file name for atm.ini with path */

int setupatmini (void) {
	if (useatmini == 0) return 0;		/* already tried and failed */
	if (*atmini != '\0') return 1;		/* already tried and succeeded */
/*	atmini = setupinifile ("atm.ini", "[Setup]");  */
	atmini = setupinifile (atmininame, atmsection);
	if (*atmini == '\0') useatmini = 0;	/* failed, don't try this again */
	return (*atmini != '\0');
}
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int main(int argc, char *argv[]) {
	char infile[MAXFILENAME], outfile[MAXFILENAME];
	FILE *input, *output;
	char *s;
	int count, firstarg=1;

	if (argc < 2) showusage(argv);

/*	while (*argv[firstarg] == '-') { */
	while (firstarg < argc && *argv[firstarg] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag = 1;
		else if (strcmp(argv[firstarg], "-t") == 0) traceflag = 1;
		else if (strcmp(argv[firstarg], "-a") == 0) approximate = 1;
		firstarg++;
	}

	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

/*	vecpath = getenv("VECPATH");  */
	if ((s = getenv("VECPATH")) != NULL) vecpath = s;
/*	else if (usedviwindo) { */
	if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("VECPATH")) != NULL) vecpath = s;
	}

	if (argc < firstarg+2) showusage(argv);

	if (traceflag) printf("Now reading input encoding %s\n", argv[firstarg]);
	if (readencoding(argv[firstarg], encodingin) == 0) exit(1);
	if (traceflag) printf("Now reading output encoding %s\n", argv[firstarg+1]);
	if (readencoding(argv[firstarg+1], encodingout) == 0) exit(1);

	if (traceflag) printf("Now making up remapping array\n");
	mapremap(0);

	if (argc < firstarg+3) {
		showremap(stdout);
		return 0;
	}

	strcpy(infile, argv[firstarg+2]);
	if (traceflag) printf("Now preparing input files %s\n", infile);
	if ((input = fopen(infile, "rb")) == NULL) {
		perror(infile); exit(1);
	}
	strcpy(outfile, stripname(argv[firstarg+2]));
	if ((s = strrchr(outfile, '.')) == NULL) strcat(outfile, ".");
	if ((s = strrchr(outfile, '.')) == NULL) exit(1);
	strcpy (s+1, outext);
	if (strcmp(infile, outfile) == 0) exit(1);
	if (traceflag) printf("Now preparing output files %s\n", outfile);
	if ((output = fopen(outfile, "wb")) == NULL) {
		perror(outfile); exit(1);
	}

	if (traceflag) printf("Now translating file\n");
	count = remapfile(output, input);
	fclose(output);
	fclose(input);
	if (traceflag) printf("Now showing bad hits\n");
	if (count > 0) showbadhits(stdout);
	return 0;
}

/* do something special about 10 and 13 ? */

/* problem when character appears more than once (e.g. space or hyphen) */
