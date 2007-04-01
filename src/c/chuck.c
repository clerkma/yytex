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

/* Code for renaming characters in BDF files */
/* Typical use: chuck -c=isolati5 -f fontogr.bdf */
/* Typical use: chuck -c=c:\lucidexp\lscyrbdf -f fontogr.bdf */
/* Typical use: chuck -c=c:\lucidexp\lscybbdf -f fontogr.bdf */
/* Typical use: chuck -c=c:\lucidexp\lsl2rbdf -f lsl2r12.bdf */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include <malloc.h> */

#define MAXLINE 256

#define MAXNAME 32

#define MAXCHRS 256

#define MAXMAPPINGS 256

#define MAXFILENAME 128

int useencoding=0;			/* use encoding number */

int useremapping=0;			/* use remapping pairs */

int verboseflag=0;
int traceflag=0;

char line[MAXLINE];

char *encoding[MAXCHRS];

char *vecpath=NULL;

/* Following are the aliases that Fontographer uses for character codes */

char *fontographer[] = {	/* Fontographer encoding */
"NUL", "Eth", "eth", "Lslash", "lslash", "Scaron", "scaron", "Yacute",
"yacute", "HT", "LF", "Thorn", "thorn", "CR", "Zcaron", "zcaron",
"DLE", "DC1", "DC2", "DC3", "DC4", "onehalf", "onequarter", "onesuperior",
"threequarters", "threesuperior", "twosuperior", "brokenbar", "minus", "multiply", "RS", "US",
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
"x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "DEL",
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
"daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase", "perthousand",
"Acircumflex", "Ecircumflex", "Aacute",
"Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex",
"apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde",
"macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron"
};

int index=0;				/* index into oldname/newname */

char *oldname[MAXMAPPINGS];

char *newname[MAXMAPPINGS];

char *translate(char *name) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(oldname[k], "") == 0) continue;
		if (strcmp(oldname[k], name) == 0) {
			if (strcmp(newname[k], "") == 0) {
				fprintf(stderr, "WARNING: no new name for %s (%d)\n", name, k);
				return name;
			}
			else return newname[k];
		}
	}
	fprintf(stderr, "WARNING: %s not found in translation table\n", name);
	return name;
}

void showtranslate (void) {
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		if (strcmp(oldname[k], "") != 0 ||
			strcmp(newname[k], "") != 0) {
			printf("%s\t=> %s\n", oldname[k], newname[k]);
		}
	}	
}

void setupfontocrock(void) { /* assume BDF file uses Fontographer encode */
	int k;
	for (k = 0; k < MAXCHRS; k++) {
		oldname[k] = fontographer[k];
		newname[k] = encoding[k];
	}
	if (verboseflag != 0) showtranslate();
	useencoding = 0;
	useremapping = 1;
}

void renamech (FILE *output, FILE *input) {
/*	char name[MAXNAME]; */
	char *name;
	int chr;

/*	ignore everything up to STARTCHAR line */
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '\n') continue;	/* ignore blank lines */
		if (strncmp(line, "STARTFONT", strlen("STARTFONT")) == 0) {
			fputs(line, output);
			break;
		}
	}

	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '\n') continue;	/* ignore blank lines */
		if (strncmp(line, "STARTPROPERTIES", strlen("STARTPROPERTIES")) == 0) {
			while (fgets(line, MAXLINE, input) != NULL) {
				if (strncmp(line, "ENDPROPERTIES", strlen("ENDPROPERTIES"))
					==  0) {	/* ignore from STARTPROP.. to ENDPROP.. */
					break;
				}
			}
			continue;
		}
		if (strncmp(line, "ENDFONT", strlen("ENDFONT")) == 0) {
			fputs(line, output);
			break;
		}
		if (strncmp(line, "STARTCHAR", strlen("STARTCHAR")) == 0) {
			if ((name = strtok(line + strlen("STARTCHAR") + 1, " \t\n\r")) ==
				NULL) {
				fprintf(stderr, line);
				name = "missing";
			}
			if (useencoding) {			/* using encoding vector ? */
				fgets(line, MAXLINE, input);
				if (strncmp(line, "ENCODING", strlen("ENCODING")) != 0) {
					fprintf(stderr, line);
				}
				chr = atoi(line + strlen("ENCODING") + 1); 
				if (chr < 0 || chr >= MAXCHRS) fprintf(stderr, line);
				else fprintf(output, "STARTCHAR %s\n", encoding[chr]);
			}
			else if (useremapping)
				 sprintf(line, "STARTCHAR %s\n", translate(name));
		}
		else if (strncmp(line, "ENCODING", strlen("ENCODING")) == 0) {
			chr = atoi(line + strlen("ENCODING") + 1); 
/*			if (sscanf (line, "ENCODING %d", &chr) < 1) {
				fprintf(stderr, line);
			} */
			if (chr >= MAXCHRS) chr = chr - MAXCHRS;
			sprintf(line + strlen("ENCODING") + 1, "%d\n", chr);
		}
		fputs(line, output);
	}
}

void readmapping(FILE *input) {
	char *s, *t;

	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '\n' || *line == '%' || *line == ';') continue;
		if ((s = strtok(line, " \t\n\r")) == NULL) {
			fprintf(stderr, line);
		}
		else {
			if ((t = strtok(NULL, " \t\n\r")) == NULL) {
				fprintf(stderr, line);
			}
			else {
				if (index >= MAXMAPPINGS) {
					fprintf(stderr, "Too many renaming pairs\n");
					break;
				}
				oldname[index] = _strdup(s);
				newname[index] = _strdup(t);
				index++;
			}
		}
	}
}

void readencoding (FILE *input) {
	char *s, *t;
	int chr;
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '\n' || *line == '%' || *line == ';') continue;
		if ((s = strtok(line, " \t\n\r")) == NULL) {
			fprintf(stderr, line);
		}
		else {
			if ((t = strtok(NULL, " \t\n\r")) == NULL) {
				fprintf(stderr, line);
			}
			else {
				chr = atoi(s);
				if (chr < 0 || chr >= MAXCHRS) {
					fprintf(stderr, line);
				}
				else encoding[chr] = _strdup(t);
			}
		}
	}
}

void showencoding (void) {
	int k;
	for (k = 0; k < MAXCHRS; k++) 
		if (strcmp(encoding[k], "") != 0) 
			printf("%d\t%s\n", k, encoding[k]);
}

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

int getencoding(char *name) {
	FILE *input;
	char infilename[MAXFILENAME];
/*	char *s; */

	strcpy (infilename, name);
	extension (infilename, "vec");
	if ((input = fopen(infilename, "r")) == NULL) {
		if (vecpath != NULL) {
			strcpy(infilename, vecpath);
			strcat(infilename, "\\");
		}
		else strcpy(infilename, "");
		strcat(infilename, name);
		extension (infilename, "vec");			
		if ((input = fopen(infilename, "r")) == NULL) {
			perror(infilename); return -1;
		}
	}
	readencoding (input);
	fclose (input);
	if (verboseflag != 0) showencoding();
	return 0;
}

int getremapping(char *name) {
	FILE *input;
	char infilename[MAXFILENAME];

	strcpy (infilename, name);
	extension (infilename, "ren");
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename); return -1;
	}
	readmapping (input);
	fclose (input);
	if (verboseflag != 0) showtranslate();
	return 0;
}

int remapfile(char *name) {
	FILE *input, *output;
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	strcpy (infilename, name);
	extension (infilename, "bdf");
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename); return -1;
	}
	strcpy (outfilename, infilename);
	forceexten (outfilename, "new");
	if ((output = fopen(outfilename, "w")) == NULL) {
		fclose(input);
		perror(outfilename); return -1;
	}
	renamech (output, input);
	fclose (output);
	fclose (input);
	return 0;
}

void showusage (char *s) {
	fprintf(stderr, "chuck -c=c:\\lucidexp\\lscyrbdf -f lscyr10.bdf\n");
	fprintf(stderr, "chuck -c=c:\\lucidexp\\lscybbdf -f lscyb10.bdf\n");
	fprintf(stderr, "chuck -c=c:\\lucidexp\\lsl2rbdf -f lsl2r12.bdf\n");
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

int main (int argc, char *argv[]) {
	int k, firstarg=1;
	char *s;

	if (firstarg + 1 > argc) {
		showusage(argv[0]);
		exit(1);
	}
/*	while (argv[firstarg][0] == '-') */
	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (argv[firstarg][1] == 'v') {
			verboseflag = 1;
		}
		if (argv[firstarg][1] == 'c') {
			getencoding(argv[firstarg]+3);
			useencoding = 1;
		}
		if (argv[firstarg][1] == 'r') {
			getremapping(argv[firstarg]+3);
			useremapping = 1;
		}
		if (argv[firstarg][1] == 'f') {
			setupfontocrock();
		}
		firstarg++;
	}

	if ((s = getenv("USEDVIWINDOINI")) != NULL) 
		sscanf (s, "%d", &usedviwindo);				/* 1994/June/14 */

	if ((s = getenv("VECPATH")) != NULL) vecpath = s;
/*	else if (usedviwindo) { */
	if (usedviwindo) {
		setupdviwindo(); 	 /* need to do this before use */
		if ((s = grabenv("VECPATH")) != NULL) vecpath = s;
	}

	if (!useencoding && !useremapping) {
		fprintf(stderr, "Must specify encoding or remapping\n");
		exit(1);
	}
	if (useencoding != 0 || useremapping == 0) {
		fprintf(stderr, "WARNING: non-standard use...\n");
	}
	if (firstarg + 1 > argc) {
		showusage(argv[0]);
		exit(1);
	}
	for (k = firstarg; k < argc; k++) {
		printf("Now processing %s\n", argv[k]);
		remapfile(argv[k]);
	}
	return 0;
}
