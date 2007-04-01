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

/* Utility for extracting ordinary *.PLN font file from Windows output */
/* Print to file with PostScript printer driver and select TrueType font */

/* #define FNAMELEN 80 */

#define MAXLINE 512

char line[MAXLINE];

char fontname[MAXLINE]="";

int verboseflag=1;

int traceflag=0;

char *stripname(char *fname) {
	char *s;
	if ((s = strrchr(fname, '\\')) != NULL) return s+1;
	else if ((s = strrchr(fname, '/')) != NULL) return s+1;
	else if ((s = strrchr(fname, ':')) != NULL) return s+1;
	else return fname;
}

void forceexten (char *fname, char *ext) {
	char *s;
	if ((s = strrchr(fname, '.')) != NULL && s > stripname(fname))
		*s = '\0';				/* first cut off old extension */
	strcat(fname, ".");
	strcat(fname, ext);
}

void extension (char *fname, char *ext) {
	char *s;
	if ((s = strrchr(fname, '.')) != NULL && s > stripname(fname)) return;
	strcat(fname, ".");
	strcat(fname, ext);
}

int main (int argc, char *argv[]) {
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	FILE *input, *output;
	int firstarg=1;
	int k, nbeg, nend;
	char *s;

	if (argc - firstarg < 1) exit(1);	/* need one arg at least */

	strcpy(infilename, argv[firstarg]);
	extension(infilename, "pln");
	if ((input = fopen(infilename, "r")) == NULL) {
		perror(infilename); exit(1);
	}
	strcpy(outfilename, stripname(infilename));
	forceexten(outfilename, "plx");
	if ((output = fopen(outfilename, "w")) == NULL) {
		perror(outfilename); exit(1);
	}
/*	if (firstarg +1 < argc)	fontname = argv[firstarg+1]; */
	if (firstarg +1 < argc)	strcpy(fontname, argv[firstarg+1]);

/*	search for first BeginResource: font (or the particular one we want) */
	while (fgets(line, MAXLINE, input) != NULL) {
		if (strncmp(line, "%%BeginResource:", 16) != 0) continue;
		if ((s = strstr(line, "font")) == NULL) continue;
/* if fontname not specified pick the first one we meet */
/* if fontname specified check that this is the one we want */
		if (strcmp(fontname, "") == 0) {
			sscanf (s+5, "%s", fontname);
			break;
		}
		else if (strstr(s, fontname) != NULL) break;
	}

/*	now copy all %%BeginResource to %%EndResource for same font */
	for (;;) {
		if (traceflag) fputs(line, stdout);
		fputs(line, output);		/* copy up to EndResource */
		while ((s = fgets(line, MAXLINE, input)) != NULL) {
			if (strncmp(line, "%%EndResource", 13) == 0) break;
			if (strstr(line, " FE") != NULL &&
				sscanf(line, "%d %d FE", &nbeg, &nend) == 2) {
					for (k = nbeg; k <= nend; k++) 
/*						fprintf(output, "dup %d /G%d put\n", k, k); */
						fprintf(output, "dup %d /G%2X put\n", k, k);
			}
			else fputs(line, output);
		}
		if (s == NULL) break;		/* EOF ? */
		if (traceflag) fputs(line, stdout);
		fputs(line, output);
/*	now search for next BeginResource: font that has same font name */
		while ((s = fgets(line, MAXLINE, input)) != NULL) {
			if (strncmp(line, "%%BeginResource:", 16) != 0) continue;
			if ((s = strstr(line, "font")) == NULL) continue;
			if (traceflag) printf ("%s: %s", fontname, line);
			if (strstr(s, fontname) != NULL) break;
		}
		if (s == NULL) break;
	}

	fclose (output);
	fclose (input);
	return 0;
}

/* first run PS file through makepln */

/*	30 255 FE
	/FEbuf 2 string def
	/FEglyph(G  )def
	/FE{1 exch{dup 16 FEbuf cvrs FEglyph exch 1 exch
		putinterval 1 index exch FEglyph cvn put}for}bd */

/* /FontMatrix [1 2048 div 0 0 1 2048 div 0 0] def */

/* rescale all paths ? */  /* is scale dependent on font size ? */

/* Print to PS file from Windows at large point size */
/* rename foo.ps to foo.pfa */
/* makepln foo */
/* trutopln foo */
/* extroutl foo.plx */
/* convert foo */
/* showchar foo */
