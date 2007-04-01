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

/* Attempt to extract composite char information from OUT file */
/* Assumes base paths are followed by accent paths (not mixed up) */
/* Assumes paths are not reversed */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXCHRS 1024

#define MAXBUF 256

#define MAXCHARNAME 32

int charindex=0;			/* total number of characters */

long charpos[MAXCHRS];		/* positions within file */

long charlen[MAXCHRS];		/* length of character program (minus comments) */

char *charnames[MAXCHRS];	/* names of characters */

int charcodes[MAXCHRS];		/* character codes */

int widths[MAXCHRS];		/* width of character */

int startx[MAXCHRS], starty[MAXCHRS];	/* starting x and y of char */

char buffer1[MAXBUF];

char buffer2[MAXBUF];

int xoffset, yoffset;		/* offset accent - base */

int xinit, yinit;			/* start of accent in composite */

long composte;				/* position in composite char */

long lengths;				/* length of input file */

int verboseflag=0;			/* verbosity */

int traceflag=0;			/* verbosity */

int debugflag=0;			/* verbosity */

int allowall=1;				/* allow all to be base and accent */

int allowpartial=0;			/* allow partial match of accent */

int showdistance=1;			/* show max separation between curves */

int usenumeric=0;			/* give numeric code rather than glyph name */

int epsilon=100;			/* max allowed coordinate error */

/* int linethres=12; */		/* assume match after this many lines */

int baseflag;				/* one while looking for base character */

int count;					/* how many components found */

int distance;				/* maximum distance between displaced curves */

int errorcode=0;			/* last error code */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *stripname (char *filename) {
	char *s;
	if ((s = strrchr(filename, '\\')) != NULL) return s+1;
	if ((s = strrchr(filename, '/')) != NULL) return s+1;
	if ((s = strrchr(filename, ':')) != NULL) return s+1;
	return filename;
}

void extension (char *filename, char *ext) {
	char *s;
	if ((s = strrchr(filename, '.')) != NULL &&
		s > stripname(filename)) strcpy(s+1, ext);
	else {
		strcat(filename, ".");
		strcat(filename, ext);
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

char *nonchars[] = {
	"i", "j", "hyphen", "sfthypen", "space", "nbspace",
	"period", "comma", "colon", "semicolon", ""
};

char *accents[] = {
	"grave", "acute", "circumflex", "tilde", "breve", "dotaccent",
	"dieresis", "ring", 
	"hungarumlaut", "caron", "macron",
	"cedilla", "ogonek",			/* usually overlap/attached */
	"commaaccent",					/* not in ASE */
	"quoteright", "quoteleft", "quotesingle",
	""
};
			
char *nonaccents[] = {
	"hyphen", "sfthyphen", "underscore", "overscore", "degree", "period", ""
};

char *nonbase[] = {
	"i", "j", "hyphen", "sfthypen", "period", "comma",
	"grave", "acute", "dieresis", "cedilla", ""
};

int basechar (char *name) {		/* could this be a base character ? */
	int k;
	for (k = 0; k < 32; k++) {
		if (strcmp(nonbase[k], "") == 0) break;
		if (strcmp(nonbase[k], name) == 0) return 0; 
	}
	if (allowall) return 1;		/* allow all to be base */
	if (strlen(name) == 1) return 1;
	if (strcmp(name, "dotlessi") == 0) return 1;
	if (strcmp(name, "dotlessj") == 0) return 1;
	if (strcmp(name, "AE") == 0) return 1;
	if (strcmp(name, "ae") == 0) return 1;	
	return 0;					/* probably not a base character */
}

int accentchar (char *name) {	/* could this be an accent ? */
	int k;
	for (k = 0; k < 32; k++) {
		if (strcmp(nonaccents[k], "") == 0) break;
		if (strcmp(nonaccents[k], name) == 0) return 0; 
	}
	if (allowall) return 1;		/* allow all to be accent */
	for (k = 0; k < 32; k++) {
//		if (strcmp(accents[k], "") == 0) return 0;
		if (strcmp(accents[k], "") == 0) break;
		if (strcmp(accents[k], name) == 0) return 1;
	}
	return 0;					/* probably not an accent */
}

char *getrealline (char *buffer, int nlen, FILE *input) {	/* 1996/May/11 */
	char *s;
	for (;;) {
		s = fgets(buffer, nlen, input);				/* prime the pump */
		if (s == NULL) return s;
		if (*s == '%' || *s == ';' || *s == '\n') continue;
		else return s;
	}
}

/* gets character names, positions in file, and starting (x, y) */

int getcharpos (FILE *input) {
	char *s;
	long present;
	int chr, width;
	char charname[MAXCHARNAME];
	long charlength;
	int x, y;
	
/*	present = ftell(input); */
/*	s = fgets(buffer1, MAXBUF, input); */			/* prime the pump */
	s = getrealline(buffer1, MAXBUF, input);		/* 1996/May/11 */
	charindex=0;
/*	if (s == NULL) return 0; */						/* ugh */
	charlength = 0;
	for (;;) {
/*		search for start of next character at ] */
		for (;;) {
/*			if (*buffer1 == ']') break; */
			if (s == NULL || *buffer1 == ']') break;
/*			already ignores comment lines and blank lines... */
/*			if ((s = fgets(buffer1, MAXBUF, input)) == NULL) break; */
/*			if ((s = getrealline(buffer1, MAXBUF, input)) == NULL) break; */
			s = getrealline(buffer1, MAXBUF, input);
/*			accumulate only non-comment length of character program */
			charlength += strlen(buffer1);
		}
/*		time to record length of previous character program */
		if (charindex > 0) charlen[charindex-1] = charlength;
		if (s == NULL) {				/* hit EOF ? */
			charpos[charindex] = lengths;
			return charindex;			/* EOF */
		}
		present = ftell(input);			/* remember start of line after ] */
		charlength = 0;
		x = 0; y = 0;
/*		look for character number, width and character name */
/*		while ((s = fgets(buffer1, MAXBUF, input)) != NULL) { */
		while ((s = getrealline(buffer1, MAXBUF, input)) != NULL) {
			if (*buffer1 == ';' || *buffer1 == '%') continue;
			if (*buffer1 == ']') break;				/* hit next char */
/*			length += strlen(buffer1); */			/* increment ??? */
			if (sscanf(buffer1, "%d %d %% %s", &chr, &width, &charname) == 3) {
/*				if (chr >= 0 && chr < MAXCHRS) */
				if (charindex >= MAXCHRS) {
					fprintf(stderr, "ERROR: too many characters\n");
					return charindex;
				}
				widths[charindex] = width;
				charnames[charindex] = _strdup(charname);
				charpos[charindex] = present;
				charcodes[charindex] = chr;
/*				now look for initial (x, y) */
/*				while ((s = fgets(buffer1, MAXBUF, input)) != NULL) { */
				while ((s = getrealline(buffer1, MAXBUF, input)) != NULL) {
					if (*buffer1 == ';' || *buffer1 == '%') continue;
					if (*buffer1 == ']') break;
					charlength += strlen(buffer1);				/* increment */
					if (strchr(buffer1, '%') != NULL) continue;
					if (strchr(buffer1, 'm') == NULL) continue;					
					if (sscanf (buffer1, "%d %d m", &x, &y) == 2) {
						startx[charindex] = x; 
						starty[charindex] = y; 
						break;
					}
				}
				charindex++;
				break;
			}
		}
		if (s == NULL) {					/* hit EOF while searching ? */
			charpos[charindex] = lengths;
			if (charindex > 0) charlen[charindex] = charlength;
			return charindex;				/* EOF */
		}
	}
}

void showcharpos (void) {
	int k;
	for (k = 0; k < charindex; k++) {
		printf("%d %s %d (%d %d) %ld (%ld)\n",
			k, charnames[k], widths[k], startx[k], starty[k],
/*				charpos[k], charpos[k+1] - charpos[k]); */
				charpos[k], charlen[k]);
	}
}

/* compare two char component descriptions - with assumed xoffset / yoffset */

/* returns  0 if match up to end of accent (i.e. ran out on input2) */
/* returns  0 if match up to end of composite (i.e. ran out on input1) */
/* returns -1 if hit end of file or next character first */
/* returns -1 if mismatching char path codes */
/* returns -1 if distance between coordinates too large */
/* returns -1 if empty character (like space) */
/* returns +1 if reached end of composite */

/* both inputs are positioned *after* the initial moveto */
/* this should somehow make sure we match *all* of second char */

/* return  0 success, looking for base and ran out on input2 */
/* return  0 success, looking for accent and ran out on input2 *and* input1 */
/* return -1 if empty character */
/* return -1 if looking for base and width does not match or non-zero offset */
/* return -1 if outline code does not match */
/* return -1 if distance between outline coordinates too large */
/* return -1 if program length of base or accent longer than composite */
/* return +1 if looking for base and ran out of input1 first */
/* return +1 if looking for accent and ran out of input2 first */

int compareinputs (FILE *input1, FILE *input2) {
	char *s;
	int code1, code2;
	int x1, y1, xa1, ya1, xb1, yb1, xc1, yc1;
	int x2, y2, xa2, ya2, xb2, yb2, xc2, yc2;
	int lines=0;	/* how many lines compared so far */
	int delta;
/*	int distance=0; */

	distance=0; 
	for (;;) {
/*		if (fgets (buffer2, MAXBUF, input2) == NULL) return -1; */
/*		if (getrealline (buffer2, MAXBUF, input2) == NULL) return -1; */
		s = getrealline (buffer2, MAXBUF, input2);
/*		remember to skip out if s == NULL ... */
		if (*buffer2 == '%' || *buffer2 == '\n')
			continue;	/* redundant ? 96/Apr/11 ??? */
/*		hit end of input2 character description */
/*		if (*buffer2 == ']') { */
		if (s == NULL || *buffer2 == ']') {			/* 96/May/11 */
			if (traceflag) printf("Matched (reached ] on 2)!\n");
			if (lines == 0) {
				errorcode = 1;
				return -1;		/* Failed (empty char) */
			}
			else {
				if (baseflag) return 0;		/* base match success */
				else {
/* check here - if its not base - whether we also ran out on input1 ? */
					s = getrealline (buffer1, MAXBUF, input1);
					while (*buffer1 == '%' || *buffer1 == '\n') {
						s = getrealline (buffer1, MAXBUF, input1);
						if (s == NULL) break;
					}
					if (s == NULL || *buffer1 == ']') {
						if (traceflag) printf("Reached end of composite!\n");
						return 0;		/* success match of accent */
					}
					else {
						errorcode = 2;
						return +1;		/* partial match only */
					}
				}
			}
		}
		if (strchr (buffer2, 'm') != 0) code2 = 'm';
		else if (strchr (buffer2, 'l') != 0) code2 = 'l';
		else if (strchr (buffer2, 'c') != 0) code2 = 'c';
		else if (strchr (buffer2, 'h') != 0) code2 = 'h';
		else code2 = -1;						/* error */

/*		if (fgets (buffer1, MAXBUF, input1) == NULL) return -1; */
/*		if (getrealline (buffer1, MAXBUF, input1) == NULL) return -1; */
		s = getrealline (buffer1, MAXBUF, input1);
/*		remember to skip out if s == NULL */
		while (*buffer1 == '%' || *buffer1 == '\n') {	/* 96/Apr/11 ??? */
/*			if (fgets (buffer1, MAXBUF, input1) == NULL) return -1; */
			s = getrealline (buffer1, MAXBUF, input1);
			if (s == NULL) break;
		}
/*		reached end of composite character *before* end of input2 */
/*		if (*buffer1 == ']') { */
		if (s == NULL || *buffer1 == ']') {
			if (traceflag) printf("Reached end of composite!\n");
/* check here - if its accent - whether we reached end of accent ? */
			if (baseflag) {
				errorcode = 3;
				return -1;				/* fail if looking for base */
			}
			else {
				if (allowpartial) return 0;		/* treat as success */
				else {
					errorcode = 4;
					return 1;			/* hit end of accent first */
				}
			}
/*			if (allowpartial) return 1; */	/* success, be permissive */
/*			else return 0; */		/* fail, an experiment 96/June/27 */
		}
		if (strchr (buffer1, 'm') != 0) code1 = 'm';
		else if (strchr (buffer1, 'l') != 0) code1 = 'l';
		else if (strchr (buffer1, 'c') != 0) code1 = 'c';
		else if (strchr (buffer1, 'h') != 0) code1 = 'h';
		else code1 = -1;						/* error */

		if (code1 != code2) {
			if (debugflag) printf("Failed %c != %c\n", code1, code2); 
/*			errorcode = 5; */
			return -1;			/* unequal codes - bad luck */
		}
/*		if (code1 == 'h') return 0;	*/			/* success (we think) */
		if (code1 == 'm') {						/* should be impossible ? */
			sscanf(buffer1, "%d %d m", &x1, &y1);
			sscanf(buffer2, "%d %d m", &x2, &y2);
			delta = abs(x2 + xoffset - x1);
			if (delta > distance) distance = delta;
			delta = abs(y2 + yoffset - y1);
			if (delta > distance) distance = delta;
			if (distance > epsilon) {
				if (debugflag) printf("%s%s", buffer1, buffer2);
				errorcode = 6;
				return -1;
			}
		}
		else if (code1 == 'l') {				/* lineto */
			sscanf(buffer1, "%d %d l", &x1, &y1);
			sscanf(buffer2, "%d %d l", &x2, &y2);
			delta = abs(x2 + xoffset - x1);
			if (delta > distance) distance = delta;
			delta = abs(y2 + yoffset - y1);
			if (delta > distance) distance = delta;
			if (distance > epsilon) {
				if (debugflag) printf("%s%s", buffer1, buffer2);
/*				errorcode = 7; */
				return -1;
			}
		}
		else if (code1 == 'c') {				/* curveto */
			sscanf(buffer1, "%d %d %d %d %d %d c",
				&xa1, &ya1, &xb1, &yb1, &xc1, &yc1);
			sscanf(buffer2, "%d %d %d %d %d %d c",
				&xa2, &ya2, &xb2, &yb2, &xc2, &yc2);
			delta = abs(xa2 + xoffset - xa1);
			if (delta > distance) distance = delta;
			delta = abs(ya2 + yoffset - ya1);
			if (delta > distance) distance = delta;
			delta = abs(xb2 + xoffset - xb1);
			if (delta > distance) distance = delta;
			delta = abs(yb2 + yoffset - yb1);
			if (delta > distance) distance = delta;
			delta = abs(xc2 + xoffset - xc1);
			if (delta > distance) distance = delta;
			delta = abs(yc2 + yoffset - yc1);
			if (delta > distance) distance = delta;
			if (distance > epsilon) {
				if (debugflag) printf("%s%s", buffer1, buffer2);
				if (debugflag) {
					printf("%d %d %d %d %d %d c (OFFSETTED)\n",
						xa2 + xoffset, ya2 + yoffset,
						xb2 + xoffset, yb2 + yoffset,
						xc2 + xoffset, yc2 + yoffset);
				}
/*				errorcode = 8; */
				return -1;
			}
		}
		else if (code1 == 'h') {			/* should be impossible ? */
/*			return 0;			 */
		}
		lines++;				/* count lines compared */
	}
}

/* search all chars for match to component of particular char */

int searchformatch (FILE *input1, FILE *input2, int current, int old) {
	int k, n;
	int x, y;
/*	int xinit, yinit; */
/*	int chr, width; */
/*	char charname[MAXCHARNAME]; */
/*	int flag; */
 	long lengthcomposite, lengthaccent;
	
	for (k = 0; k < charindex; k++) {
		if (k == current) continue;		/* don't match against self! */
		if (k == old) continue;			/* don't match against tried already */
		if (baseflag) {					/* base has to match with zero offset*/
			if (widths[k] != widths[current] ||
				startx[k] != startx[current] ||
				starty[k] != starty[current])
				continue;				/* not a possible match for base */
		}
/*		lengthaccent = charpos[k+1] - charpos[k]; */
		lengthaccent = charlen[k];				/* bytes in description */
/*		lengthcomposite = charpos[current+1] - charpos[current]; */
		lengthcomposite = charlen[current];		/* bytes in description */
		if (lengthaccent > lengthcomposite) {
			if (debugflag) {	/* 1996/May/11 */
				printf("%s (%ld) > %s (%ld)\n",  charnames[k], lengthaccent,
					   charnames[current], lengthcomposite);
			}
/*			take out, mismatch could be due to comment lines 96/May/11 ? */
			continue;			/* too long --- can't be base or accent  */
		}
		if (baseflag) {
//			if (! allowall && basechar(charnames[k]) == 0) continue;
			if (basechar(charnames[k]) == 0) continue;
		}
		else {
//			if (! allowall && accentchar(charnames[k]) == 0) continue;
			if (accentchar(charnames[k]) == 0) continue;
		}
		if (debugflag)
			printf("Comparing %s (%ld) %s (%ld)\n", 
				charnames[current], lengthcomposite,
					charnames[k], lengthaccent);
		if (baseflag) {				
			xoffset = startx[k] - startx[current];
			yoffset = starty[k] - starty[current];
			if (xoffset != 0 || yoffset != 0) printf("CANT HAPPEN!\n");
		}
		fseek(input1, composte, SEEK_SET);
		fseek(input2, charpos[k], SEEK_SET);
/*		fgets(buffer2, MAXBUF, input2); */		/* step over chr width line */
		getrealline(buffer2, MAXBUF, input2);	/* step over chr width line */
/*		while (fgets(buffer2, MAXBUF, input2) != NULL) { */
		while (getrealline(buffer2, MAXBUF, input2) != NULL) {
			if (*buffer2 == '%' || *buffer2 == '\n') continue; /* 96/Apr/11 */
			if (*buffer2 == ']') break;
/*			looking for first moveto */
			if (strchr(buffer2, '%') != NULL) continue;
			if (strchr(buffer2, 'm') == NULL) continue;
			if (sscanf (buffer2, "%d %d m", &x, &y) == 2) {
				if (baseflag == 0) {
					xoffset = xinit - x;
					yoffset = yinit - y;
					if (debugflag) 
						printf("Composite %d %d Accent %d %d Offset %d %d\n",
							xinit, yinit, x, y, xoffset, yoffset);
				}
				if (baseflag) {
					if (xoffset != 0 || yoffset != 0) {
						if (traceflag)
							printf("Failed xo %d yo %d\n", xoffset, yoffset);
						errorcode = 9;
						return -1;			/* not allowed */
					}
				}
				n = compareinputs(input1, input2);
/* returns 0 if successful, -1 if msimatch, +1 if incomplete match */
				if (n < 0) break;		/* failed for one reason or another */
				if (n > 0) {			/* ran to end of composite */
					if (baseflag) {
			printf("Characters %s and %s matched completely **********\n",
							charnames[current], charnames[k]);
						errorcode = 10;
						return -1;		/* nothing more to do ...*/
					}
					else {
						if (traceflag) printf("Failed on accent %d\n", k);
						if (allowpartial == 0) {
							errorcode = 11;
							break;			/* failed on accent */
						}
					}
				}
				if (traceflag)
					printf("MATCH %d %s\n", k, charnames[k]);
				return k;				/* found a match */
			}
		}
	}
/*	errorcode = 12; */
	return -1;							/* failed to find match */
}

/* check each characters to see whether it may be composite */

void lookformatch (FILE *input1, FILE *input2) {
	int k, i, n;
	int x, y;
/*	int chr, width; */
/*	char charname[MAXCHARNAME]; */
	long present;
	int flag;
	int old = -1;
	
	for (k = 0; k < charindex; k++) {
		old = -1;
		errorcode = 0;
		flag = 0;
		for (i = 0; i < 32; i++) {
			if (strcmp(nonchars[i], "") == 0) break;
			if (strcmp(nonchars[i], charnames[k]) == 0) {
//				printf("%s == %s\n", nonchars[i], charnames[k]);
				flag = 1;
			}
		}
		if (flag) continue;
		for (i = 0; i < 32; i++) {
			if (strcmp(accents[i], "") == 0) break;
			if (strcmp(accents[i], charnames[k]) == 0) {
//				printf("%s == %s\n", accents[i], charnames[k]);
				flag = 1;
			}
		}
		if (flag) continue;
retry:
		if (verboseflag)
			printf("Checking char %s for decomposition\n", charnames[k]);
		baseflag = 1;					/* base character at first */
		count = 0;						/* number of components */
		fseek(input1, charpos[k], SEEK_SET);
/*		fgets(buffer1, MAXBUF, input1); */		/* step over chr width line */
		getrealline(buffer1, MAXBUF, input1);	/* step over chr width line */
/*		while (fgets(buffer1, MAXBUF, input1) != NULL) { */
		while (getrealline(buffer1, MAXBUF, input1) != NULL) {
			if (flag) break;
			if (*buffer1 == '%' || *buffer1 == '\n') continue; /* 96/Apr/11 */
			if (*buffer1 == ']') {				/* hit end of char */
				if (count == 1) printf("ERROR %d", errorcode);
				if (count > 0) printf("\n");
				break;
			}
/*			looking for initial moveto */
			if (strchr(buffer1, '%') != NULL) continue;
			if (strchr(buffer1, 'm') == NULL) continue;
			if (sscanf (buffer1, "%d %d m", &x, &y) == 2) {
				xinit = x; yinit = y;		/* coordinates at start of path */
				composte = ftell(input1);	/* right after initial moveto */
/*				if ((n = searchformatch (input1, input2, k)) < 0) { */
				if ((n = searchformatch (input1, input2, k, old)) < 0) {
					if (count == 1) printf("ERROR %d", errorcode);
					if (count > 0) printf("\n");
					break;
				}
				if (baseflag) {			/* first time */
/*					printf("%s = ", charnames[k]); */
					if (usenumeric) printf("CC a%d 2 ; ", charcodes[k]);
					else printf("CC %s 2 ; ", charnames[k]);
					baseflag = 0;
				}
/*				printf ("%s (%d %d)", charnames[n], xoffset, yoffset); */
/*				printf ("PCC %s %d %d ; ", charnames[n], xoffset, yoffset); */
				if (usenumeric)
					printf ("PCC a%d %d %d ;", charcodes[n], xoffset, yoffset);
				else printf ("PCC %s %d %d ;", charnames[n], xoffset, yoffset);
				count++;
				if (count >= 2) {
					if (showdistance) {		/* not a complete match ? */
						if (distance > 1) {
							printf(" %% deviation %d ?\n", distance);
							if (old < 0) {	/* try again ??? 1996/May/11 */
								old = n;
								goto retry;	/* can only happen once ! */
							}
						}
						else printf("\n");
					}
					else printf("\n");
					if (traceflag) printf("Found two components\n");
					break;		/* two components max break out of search */
				}
				printf (" ");
/*				printf (" + ");	*/			/* more to come */
				if (traceflag != 0) printf("\n");
/* leave positioned where search finished ... */				
/*				fseek(input1, present, SEEK_SET); */
				present = ftell(input1);
/*				fgets(buffer2, MAXBUF, input1); */
				getrealline(buffer2, MAXBUF, input1);
				if (debugflag) printf("NEXT LINE %s", buffer2);
				fseek (input1, present, SEEK_SET);
			} /* if (sscanf (buffer1, "%d %d m", &x, &y) == 2) */
/*			composte = ftell(input1); */		/* remember previous line */
/*			if (baseflag == 0) printf("\n"); */		/* ha */
		} /* while (getrealline(buffer1, MAXBUF, input1) != NULL) */
	} /* 	for (k = 0; k < charindex; k++)  */
}

int readcommand (int argc, char *argv[]) {
	int firstarg=1;
/*	while (*argv[firstarg] == '-') */
	while (firstarg < argc && *argv[firstarg] == '-') {
		if (strcmp(argv[firstarg], "-d") == 0) {
			debugflag = !debugflag;
			firstarg++;
		}
		else if (strcmp(argv[firstarg], "-t") == 0) {
			traceflag = !traceflag;
			firstarg++;
		}
		else if (strcmp(argv[firstarg], "-v") == 0) {
			verboseflag = !verboseflag;
			firstarg++;
		}
		else if (strcmp(argv[firstarg], "-r") == 0) {
			allowall = !allowall;
			firstarg++;
		}
		else if (strcmp(argv[firstarg], "-p") == 0) {
			allowpartial = !allowpartial;
			firstarg++;
		}
		else if (strcmp(argv[firstarg], "-n") == 0) {
			usenumeric = !usenumeric;
			firstarg++;
		}
		else {
			fprintf(stderr, "Don't understand argument\n");
			firstarg++;
			exit(1);
		}
	}
	return firstarg;
}

void showusage (char *argv[]) {
		fprintf(stderr, "%s [-v][-t][-d][-r][-p][-n] <out-file>\n", argv[0]);
		fprintf(stderr, "\n\
\tIt is assumed that composite character outlines have *all* of the\n\
\tbase character outline *before* the accent outline.\n\n\
\tIt is assumed that none of the outline components are reversed.\n\n\
\tIt is assumed that the outline components in the composite start in\n\
\tthe same place as they do in the individual character programs.\n\n\
\tIt is assumed that the last outline is followed by a ]\n\n\
\t-v verbose mode\n\
\t-t trace mode\n\
\t-d debug mode\n\
\t-r restrict base and accent possibilities to normal base and accent chars\n\
\t-p allow match with partial accent\n\
\t-n show numeric code instead of glyph name\n\
");
		exit(1);
}

int main (int argc, char *argv[]) {
	int k;
	int firstarg=1;
/*	FILE *input, *output;  */
	FILE *input1, *input2; 
/*	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX]; */
	char infilename1[FILENAME_MAX], infilename2[FILENAME_MAX];

	if (firstarg+1 > argc) showusage(argv);

	firstarg = readcommand (argc, argv);

/*	if (firstarg+1 > argc) exit(1); */
	if (firstarg+1 > argc) showusage(argv);

	for (k = 0; k < MAXCHRS; k++) charpos[k] = -1;
	for (k = 0; k < MAXCHRS; k++) charlen[k] = -1;
	for (k = 0; k < MAXCHRS; k++) charcodes[k] = -1;
//	for (k = 0; k < MAXCHRS; k++) charnames[k] = "";
	for (k = 0; k < MAXCHRS; k++) charnames[k] = NULL;
	
	strcpy(infilename1, argv[firstarg]);
	extension (infilename1, "out");
/*	if ((input1 = fopen(infilename1, "r")) == NULL) { */
	if ((input1 = fopen(infilename1, "rb")) == NULL) {	/* 96/Apr/5 */
		perror(infilename1);
		exit(1);
	}
	fseek(input1, 0, SEEK_END);
	lengths = ftell(input1);
	if (traceflag) printf("File length is %ld bytes\n", lengths);
	fseek(input1, 0, SEEK_SET);
	strcpy(infilename2, argv[firstarg]);
	extension (infilename2, "out");
/*	if ((input2 = fopen(infilename2, "r")) == NULL) { */
	if ((input2 = fopen(infilename2, "rb")) == NULL) {
		perror(infilename2);
		exit(1);
	}

	if (traceflag) printf("Gathering character positions\n");

	getcharpos (input1);

	if (debugflag) showcharpos();

	if (traceflag) {
		putc('\n', stdout);
		printf("Looking for matches ***********************\n");
	}

	lookformatch(input1, input2);

	fclose (input2);
	fclose (input1);
	return 0;
}

/* fsopen */

/* need some way to limit which characters considered for composite */

/* need some way to limit which characters considered for base and accent */

/* don't allow now for comments and blank lines */

/* need to check that base has offset 0 0 ? */	/* limit search */

/* need to check that only two components ? */	/* limit search */

/* check base has same width as composite */

