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

/* process c:\cserve\support\addrbook.dat to make email list */

#define MAXFILENAME 128

int main (int argc, char *argv[]) {
	char infile[MAXFILENAME], outfile[MAXFILENAME];
	FILE *input, *output;
	int c, d, k, n, nitem, ntotal;
	char *s;

	if (argc < 2) 
		strcpy(infile, "c:\\cserve\\support\\addrbook.dat");
	else strcpy(infile, argv[1]);
	if ((input = fopen(infile, "rb")) == NULL) {
		perror(infile);
		exit(1);
	}
	if ((s = strrchr(infile, '\\')) != 0) s++;
	else if ((s = strrchr(infile, '/')) != 0) s++;
	else if ((s = strrchr(infile, ':')) != 0) s++;
	else s = infile;

	strcpy(outfile, s);
	if (strrchr(outfile, '.') == NULL) strcat (outfile, ".");
	s = strrchr(outfile, '.');
	strcpy (s+1, "txt");
	if ((output = fopen(outfile, "w")) == NULL) {
		perror(outfile);
		exit(1);
	}

	(void) getc(input);			/*  2 */
	c = getc(input);			/* 41 */
	d = getc(input);			/*  1 */
	ntotal = (d << 8) | c;
	printf("There are %d entries\n", ntotal);

	for (;;) {
		c = getc(input);
		d = getc(input);
		if (c == EOF || d == EOF) break;
		nitem = (d << 8) | c;			/* number of this entry */
		fprintf(output, "%d\t", nitem);	
		n = getc(input);			/* length of name field */
		for (k = 0; k < n; k++)	putc(getc(input), output); /* copy field */
		putc('\t', output);			/* tab */
		n = getc(input);			/* length of address field */
		for (k = 0; k < n; k++)	putc(getc(input), output); /* copy field */		
		n = getc(input);
		if (n > 0) {
			putc('\t', output);			/* tab */
			for (k = 0; k < n; k++)	putc(getc(input), output); /* copy field */		
			fprintf(stderr, "Non-zero third field %d in entry %d?\n",
				n, nitem);
		}
		putc('\n', output);			/* newline */
		if (n == EOF) break;
	}	

	fclose(output);
	fclose(input);
	return 0;
}
