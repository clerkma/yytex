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

int showposition=0;

int beginpage=1;

int endpage=10;

void showcomment (FILE *input, FILE *output) {
    char line[512];
    int i;
    long k=0; 
    long present;
    while (fgets(line, sizeof(line), input) != NULL) {
	if (k++ < 1024) {
	    fputs(line, output);
	    continue;
	}
	if (*line == 12 || *line == '%') {
	    present = ftell(input);
		if (showposition) printf("BYTE %ld:\n", present);
	    fputs(line, output);
	}
	if (strncmp(line, "TB ", 3) == 0) {
	    present = ftell(input);
	    if (showposition) printf("BYTE %ld:\n", present);
	    for (i=0; i < 4; i++) {
		fputs(line, output);
		fgets(line, sizeof(line), input);
	    }
/*	    continue ;*/
	}
	if (strstr(line, "EOP") != NULL) fputs(line, output);
	if (strstr(line, "BOP") != NULL) fputs(line, output);
	if (strstr(line, "image") != NULL) fputs(line, output);
	if (strstr(line, "cleartomark") != NULL) fputs(line, output);
	if (strstr(line, "showpage") != NULL) fputs(line, output);
    }
}

void extractpages (FILE *input, FILE *output) {
    char line[512];
	char pagename[64];
	int pageno;
    long k=0; 
	int copyflag=1;

/*	copy everything up to first %%Page: */
    while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == 12) {
			putc(12, output);
			strcpy(line, line+1);
		}
		if (strncmp(line, "%%Page:", 7) == 0) {
			break;
		}
		fputs(line, output);
	}
/* now scan up to begin page */
	for (;;) {
		if (sscanf (line+2, "Page: %s %d", pagename, &pageno) == 2) {
			if (pageno >= beginpage && pageno <= endpage) copyflag=1;
			else copyflag = 0;
		}
		if (copyflag) fputs(line, output);
		if (fgets(line, sizeof(line), input) == NULL) break;
		if (*line == 12) {
			if (copyflag) putc(12, output);
			strcpy(line, line+1);
		}
		if (strncmp(line, "%EOJ", 4) == 0) break;
		if (strncmp(line, "%%Trailer", 9) == 0) break;
	}
	fputs(line, output);
    while (fgets(line, sizeof(line), input) != NULL) {
		fputs(line, output);
	}
}

int main(int  argc, char *argv[]) {
    FILE *input;
    if (argc < 2) exit(1);
    if ((input = fopen(argv[1], "r")) == NULL) {
		perror(argv[1]);
		exit(1);
    }
/*    showcomment(input, stdout);  */
    extractpages(input, stdout); 
    fclose(input);
    return 0;
}
