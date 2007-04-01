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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* utility for combining two *.OUT files */

/* presently set up for combining based on numeric character code */

/* code for using char name instead installed, but not tested */

/* combined c:\bsr\cmr10.out cmr10.out */
/* convert -v combined */
/* showchar -y combined */

#define MAXCHRS 256
#define MAXLINE 256
/* #define FNAMELEN 80 */
#define STARTCHAR ']'

long charpos1[MAXCHRS], charpos2[MAXCHRS];

char *charname1[MAXCHRS], *charname2[MAXCHRS];

int charuse1[MAXCHRS], charuse2[MAXCHRS]; 

int verboseflag=0;
int traceflag=0;
int dotsflag=1;

int pctexbug=0;			/* deal with bad scaling in PC TeX fonts */

int usecharnames=1;		/* use character names instead of char codes */

int remapflag1=0;		/* for T1 derived from TT fonts */
int remapflag2=0;		/* for T1 derived from TT fonts */

double xscale1=1.0;
double xscale2=1.0;

double slant1=0.0;		/* 97/July/12 */
double slant2=0.0;		/* 97/July/12 */

int xoffset1=0;			/* 97/July/12 */
int xoffset2=0;			/* 97/July/12 */

double ptsize1=10.0;
double ptsize2=10.0;

double scalef1=1.0;
double scalef2=1.0;

int offset1, offset2;

int notfirstscale=0, notfirstoffset=0, notfirstslant=0;

void showusage (char *s) {
	printf("%s [-v] [-t] [-n] <font1.out> <font2.out>\n", s);
	printf("\t-v\tVerbose\n");
	printf("\t-t\tTrace\n");
	printf("\t-n\tUse numbers instead of char names when merging\n");
	printf("\t-s=...\tScale font by given factor\n");
	printf("\t-o=...\tOffset font by given factor\n");
	printf("\t-w=...\tSlant font by given factor\n");
	exit(1);
}

int lookup (char *charname[], char *name) {
	int k;
	if (name == NULL) return -1;
	if (*name == '\0') return -1;
	for (k = 0; k < MAXCHRS; k++) {
		if (charname[k] == NULL) continue;
		if (*charname[k] == '\0') continue;
		if (strcmp(charname[k], name) == 0) return k;
	}
	return -1;
}

void prescan (FILE *input, long charpos[], char *charname[], int filenum) {
	char line[MAXLINE];
	int bracketflag=0;
	int n, chrs, width;
	long present = ftell (input);
	char name[MAXLINE];
	char *s;

	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == STARTCHAR) break;
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (filenum == 1) {
			sscanf (line, "%lg %lg", &ptsize1, &scalef1);
			fputs (line, stdout);
			if (pctexbug && scalef1 == 2.048) scalef1 = 2.005;
/*			if (xscale1 != 1.0) scalef1 = scalef1 / xscale1; */
		}
		if (filenum == 2) {
			sscanf (line, "%lg %lg", &ptsize2, &scalef2);
			fputs (line, stdout);
			if (pctexbug && scalef2 == 2.048) scalef2 = 2.005;
/*			if (xscale2 != 1.0) scalef2 = scalef2 / xscale2; */
		}
		break;
	}

	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == '%' || *line == ';' || *line == '\n') continue;
		if (*line == STARTCHAR) {
			if (strncmp(line, "] end", 5) == 0) {
				printf("Early termination\n");
				break;
			}
			present = ftell(input);
			bracketflag++;
		}
		if (bracketflag && sscanf(line, "%d %d%n", &chrs, &width, &n) == 2) {
			if (chrs >= 0 && chrs < MAXCHRS) {
				charpos[chrs] = present;
				if ((s = strchr(line+n, '%')) != NULL) {
					if (sscanf(s+1, "%s", &name) == 1)
						charname[chrs] = _strdup(name); /* get char name */
				}
			}
			else fputs(line, stderr);
			bracketflag = 0;
		}
	}

	if (traceflag) {
		int k;
		for (k = 0; k < MAXCHRS; k++) {
			if (charpos[k] >= 0)
				printf("%d\t%ld\t%s\n", k, charpos[k], charname[k]);
		}
		putc('\n', stdout);
	}
}

#ifdef IGNORED

int rescalenum (int num, int filenum, int xflag) {
	double scale;
	if (filenum == 1) {
		scale=scalef1;
		if (xflag % 2) scale = scale / xscale1;
	}
	else {
		scale=scalef2;
		if (xflag % 2) scale = scale / xscale2;
	}

	if (num > 0) return (int) ((double) num / scale + 0.5);
	else return - (int) (- (double) num / scale + 0.5);
}

#endif

int round (double x) {
	if (x >= 0) return (int) (x + 0.5);
	else return - (int) (-x + 0.5);
}

int rescalenumx (int xnum, int ynum, int filenum) {
	double scale, slant, x;
	if (filenum == 1) {
		scale=scalef1;
		scale = scale / xscale1;
		slant = slant1;
	}
	else {
		scale=scalef2;
		scale = scale / xscale2;
		slant = slant2;
	}

/*	if (xnum > 0) return (int) ((double) xnum / scale + 0.5);
	else return - (int) (- (double) xnum / scale + 0.5); */
	if (slant != 0)	x = (double) xnum + slant * (double) ynum; 
	else x = (double) xnum;
	x = x / scale;
/*	x = (double) xnum / scale; */
/*	if (slant != 0)	x = x + slant * (double) ynum; */
	return round(x);
}

int rescalenumy (int xnum, int ynum, int filenum) {
	double scale, y;
	if (filenum == 1) {
		scale=scalef1;
/*		scale = scale / yscale1; */
	}
	else {
		scale=scalef2;
/*		scale = scale / yscale2; */
	}

/*	if (ynum > 0) return (int) ((double) ynum / scale + 0.5);
	else return - (int) (- (double) ynum / scale + 0.5); */
	y = (double) ynum;
	y = y / scale;
	return round(y);
}

void rescaleline (char *line, int filenum) {
	char buffer[MAXLINE];
	char *s=line, *t=buffer;
/*	int num, n; */
	int xnum, ynum, n, m;
/*	int xflag=1; */

	*t = '\0';
/*	while (sscanf(s, "%d%n", &num, &n) == 1) {
		sprintf(t, "%d ", rescalenum(num, filenum, xflag++));
		s += n;
		t += strlen(t);
	} */
	while ((m = sscanf(s, "%d %d%n", &xnum, &ynum, &n)) == 2) {
		sprintf(t, "%d %d ",
				rescalenumx(xnum, ynum, filenum),
				rescalenumy(xnum, ynum, filenum));
		s += n;
		t += strlen(t);
	}
	if (m == 1) printf("BAD: %s", line);
	strcat(t, s);
	strcpy(line, buffer);
}

int copychar (FILE *output, FILE *input, int flag, int filenum) {
	char line[MAXLINE];
	int offsetflag = 0;
	char *s;

/*	scan over comments and blank lines up to char code and width */
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == STARTCHAR) return offsetflag;
		if (*line == '%' || *line == ';' || *line == '\n')
			fputs(line, output);
		else {
			if (flag) fputs("% ", output);	/* comment out */
			fputs(line, output);
			break;
		}
	}
/*	scan over comments and blank lines up to start of character program */
/*	if (fgets(line, MAXLINE, input) == NULL) return offsetflag; */
	*line = '\0';
	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == STARTCHAR) return offsetflag;
		if (*line == '%' || *line == ';' || *line == '\n') continue; /* ??? */
		break;
	}
	if (*line == '\0') return offsetflag;		/* ??? */
	if ((filenum == 1 && (scalef1 != 1.0 || xscale1 != 1.0 || slant1 != 0.0)) ||
		(filenum == 2 && (scalef2 != 1.0 || xscale2 != 1.0 || slant2 != 0.0)))
			rescaleline (line, filenum);
/*	if (*line == STARTCHAR) return offsetflag; */
	if (strstr(line, " % offset") != NULL) offsetflag++;
	else if (filenum == 1 && xoffset1 != 0) {
		s = line + strlen(line);	/* trim off line termination */
		while (*(s-1) < ' ') s--;
		*s = '\0';
		sprintf(s,  " % offset %d 0\n", xoffset1 + offset1);
/*		strcat(line, " % offset %d 0\n", xoffset1 + offset1); */
	}
	else if (filenum == 2 && xoffset2 != 0) {
		s = line + strlen(line);	/* trim off line termination */
		while (*(s-1) < ' ') s--;
		*s = '\0';
		sprintf(s, " % offset %d 0\n", xoffset2 + offset2);
/*		strcat(line, " % offset %d 0\n", xoffset2 + offset2); */
	}
	else if (filenum == 2 && offset1 != 0) {
		s = line + strlen(line);	/* trim off line termination */
		while (*(s-1) < ' ') s--;
		*s = '\0';
		strcat(line, " % offset 0 0\n");
	}
	fputs(line, output);

/*	printf("PUTTING OUT: %s", line); */		/* debugging */

	while (fgets(line, MAXLINE, input) != NULL) {
		if (*line == STARTCHAR) return offsetflag;
		if (*line == '%' || *line == ';' || *line == '\n') {
		}
		else {
			if (strstr(line, " % offset") != NULL) offsetflag++;
			if ((filenum == 1 && (scalef1 != 1.0 || xscale1 != 1.0 ||
								  slant1 != 0.0)) ||
				(filenum == 2 && (scalef2 != 1.0 || xscale2 != 1.0 ||
								  slant2 != 0.0)))
					rescaleline (line, filenum);
		}
		fputs(line, output);
	}
	return offsetflag;
}

void combined (FILE *output, FILE *input1, long charpos1[],
					FILE *input2, long charpos2[]) {
	char line[MAXLINE];
	int k, m;
	int flag;
/*	int offset1, offset2; */
	
/*  start by copying header from file 1 */
	while (fgets(line, MAXLINE, input1) != NULL) {
		if (*line == STARTCHAR) break;
		fputs(line, output);
	}
	for (k = 0 ; k < MAXCHRS; k++) {
		if (usecharnames == 0) 	{			/* using char code numbers */
			m = k;
			if (charpos1[k] < 0) {
				if (charpos2[m] < 0) continue; /* no outline */
			}
		}
		else {								/* using char names */
			if (charname1[k] == NULL) continue;					/* no name */
			if (*charname1[k] == '\0') continue;
			else m = lookup (charname2, charname1[k]);
			if (charpos1[k] < 0) {
				if (m < 0 || charpos2[m] < 0) continue; /* no outline */
			}
		}

		if (dotsflag) putc('.', stdout);
		fputs("]\n", output);
		flag = 0;
		if (charpos1[k] >= 0) {
			if (traceflag) printf("Using %s (char %d) at %ld from font 1\n",
				charname1[k], k, charpos1[k]);
			charuse1[k] = 1;
			fputs("% from font 1\n", output);
			fseek (input1, charpos1[k], SEEK_SET);
			offset1 = copychar (output, input1, flag, 1);
			flag++;
		}
/*		if (usecharnames) {		
			if (charname1[k] == NULL) m = -1;
			if (*charname1[k] == '\0') m = -1;
			else m = lookup (charname2, charname1[k]);
		}
		else m = k; */					/* else match on char number */
		if (m >= 0 && charpos2[m] >= 0) {
			if (traceflag) printf("Using %s (char %d) at %ld from font 2\n",
				charname2[m], m, charpos2[m]);
			charuse2[m] = 1;
			fputs("% from font 2\n", output);
			fseek (input2, charpos2[m], SEEK_SET);
			offset2 = copychar (output, input2, flag, 2);
			flag++;
		}
		else if (verboseflag)
			printf("%s not found in second font\n", charname1[k]);
	}
	if (dotsflag) putc('\n', stdout);
}

void interchange (long charpos[], int m, int n) {
	long temp;
	temp = charpos[n];
	charpos[n] = charpos[m];
	charpos[m] = temp;		/* this will usually be -1 */
}

void remapencode (long charpos[]) {
	int k;
	int flag = 0;
/*	sanity check first */
	for (k = 0; k < 32; k++) if (charpos[k] >= 0) flag++;
	if (flag) {
		fprintf(stderr, "ERROR: Remapping font that already uses 0 -- 31\n");
		return;
	}
	for (k = 0; k < 10; k++) interchange (charpos, k, k+161);
	for (k = 10; k < 32; k++) interchange (charpos, k, k+163);
	if (charpos[32] < 0) interchange (charpos, 32, 195);
	if (charpos[127] < 0) interchange (charpos, 127, 196);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void showmismatch(void) {
	int k, flag, count;
	putc('\n', stdout);
	flag = count = 0;
	for (k=0; k < MAXCHRS; k++) {
		if (charpos1[k] < 0) continue;
		count++;
		if (charuse1[k]) continue;
/*		if (strcmp(charname1[k], "") == 0) continue; */
		flag++; 
	}
	if (flag) {
		printf("Not used from font 1: (%d out of %d)\n", flag, count);
		for (k=0; k < MAXCHRS; k++) {
			if (charpos1[k] < 0) continue;
			if (charuse1[k]) continue;
			if (strcmp(charname1[k], "") == 0) printf("a%d, ", k);
			else printf("%s, ", charname1[k]);
		}
		putc('\n', stdout);
	}
	else printf("Used all %d from font 1\n", count);
	flag = count = 0;
	for (k=0; k < MAXCHRS; k++) {
		if (charpos2[k] < 0) continue;
		count++;
		if (charuse2[k]) continue;
/*		if (strcmp(charname2[k], "") == 0) continue; */
		flag++; 
	}	
	if (flag) {
		printf("Not used from font 1: (%d out of %d)\n", flag, count);
		for (k=0; k < MAXCHRS; k++) {
			if (charpos2[k] < 0) continue;
			if (charuse2[k]) continue;
			if (strcmp(charname2[k], "") == 0) printf("a%d, ", k);
			else printf("%s, ", charname2[k]);
		}
		putc('\n', stdout);
	}
	else printf("Used all %d from font 2\n", count);
	for (k = 0; k < MAXCHRS; k++) {
		if (charpos1[k] < 0 && charpos2[k] >= 0) {
			if (strcmp(charname2[k], "") == 0)
				printf("a%d in font 2 has no match in font 1\n", k);
			else printf("%s (%d) in font 2 has no match in font 1\n",
						charname2[k], k);
		}
		if (charpos1[k] >= 0 && charpos2[k] < 0) {
			if (strcmp(charname1[k], "") == 0)
				printf("a%d in font 1 has no match in font 2\n", k);
			else printf("%s (%d) in font 1 has no match in font 2\n",
						charname1[k], k);
		}
	}
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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

/* remove file name - keep only path - inserts '\0' to terminate */

char *stripname(char *filename) {
	char *s;
	if ((s=strrchr(filename, '\\')) != NULL) return s+1;
	else if ((s=strrchr(filename, '/')) != NULL) return s+1;
	else if ((s=strrchr(filename, ':')) != NULL) return s+1;
	else return filename;
}

int main(int argc, char *argv[]) {
	int k;
	int firstarg=1;
	char infilename1[FILENAME_MAX], infilename2[FILENAME_MAX], outfilename[FILENAME_MAX];
	FILE *input1, *input2, *output;
	char *s;

	if (argc < firstarg + 1) showusage(argv[0]);

/*	while (*argv[firstarg] == '-') { */
	while (firstarg < argc && *argv[firstarg] == '-') {
		s = argv[firstarg];
		if (strcmp(s, "-n") == 0) usecharnames=!usecharnames;
		else if (strcmp(s, "-v") == 0) verboseflag=!verboseflag;
		else if (strcmp(s, "-t") == 0) traceflag=!traceflag;
		else if (strncmp(s, "-s=", 3) == 0) {
			 if (notfirstscale++ == 0) sscanf(s+3, "%lg", &xscale1);
			 else sscanf(s+3, "%lg", &xscale2);
		}
		else if (strncmp(s, "-o=", 3) == 0) {
			if (notfirstoffset++ == 0) sscanf(s+3, "%d", &xoffset1);
			else sscanf(s+3, "%d", &xoffset2);
		}
		else if (strncmp(s, "-w=", 3) == 0) {
			if (notfirstslant++ == 0) sscanf(s+3, "%lg", &slant1);
			else sscanf(s+3, "%lg", &slant2);
		}
		else if (strcmp(s, "-?") == 0) showusage(argv[0]);
		else printf("%s?\n", s);
		firstarg++;
	}
	if (verboseflag) {
		printf("Verbose %d Trace %d CharNames %d\n",
			   verboseflag, traceflag, usecharnames);
	}
	for (k = 0; k < MAXCHRS; k++) {
		charpos1[k] = -1;  charpos2[k] = -1;
		charname1[k] = ""; charname2[k] = "";
		charuse1[k] = 0; charuse2[k] = 0;
	}
	strcpy(infilename1, argv[firstarg]);
	extension(infilename1, "out");
	if (argc < 3) strcpy(infilename2, stripname(infilename1));
	else strcpy(infilename2, argv[firstarg+1]);
	extension(infilename2, "out");

	strcpy(outfilename, "combined.out");
	if ((input1 = fopen(infilename1, "rb")) == NULL) {
		perror(infilename1); exit(1);
	}
	if ((input2 = fopen(infilename2, "rb")) == NULL) {
		perror(infilename2); exit(1);
	}
	prescan (input1, charpos1, charname1, 1);
	rewind (input1);
	if (remapflag1) remapencode (charpos1);
	prescan (input2, charpos2, charname2, 2);
	rewind (input2);
	if (remapflag2) remapencode (charpos2);
	if ((output = fopen(outfilename, "wb")) == NULL) {
		perror(outfilename); exit(1);
	}
	combined (output, input1, charpos1, input2, charpos2);
	fclose(output);
	fclose(input2);
	fclose(input1);
	if (verboseflag) showmismatch();
	return 0;
}

/* this version is based on character number, not character name ... */

/* assumes second font is to be remapped and rescaled ... */
