/* Copyright 1993 Y&Y, Inc.
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

/* EPSFTERM.C change `return' into `newline' in EPS or EPSF file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #define FNAMELEN 128 */

int main (int argc, char *argv[]) {
	FILE *input;
	char infilename[FILENAME_MAX];
	int c, d, e, f, k, m;
	int firstarg=1;
	int nifd;

	if (argc < 2) exit(1);
/*	while (argv[firstarg][0] == '-') { */
	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) {
			verboseflag++;
			firstarg++;
		}
		if (strcmp(argv[firstarg], "-t") == 0) {
			traceflag++;
			firstarg++;
		}
		if (strcmp(argv[firstarg], "-?") == 0) {
			printf("Usage: %s <tiff-file-1> <tiff-file-2> ...\n", argv[0]);
			exit(1);
		}
	}
	if (argc < firstarg+1) exit(1);

	for (m = firstarg; m < argc; m++) {
		strcpy(infilename, argv[m]);
		extension(infilename, "tif");
	
		input = fopen(infilename, "rb");
		if (input == NULL) {
			perror(infilename);
			exit(2);
		}
		fseek (input, 0, SEEK_END);			/* go to end */
		filelength = ftell(input);			/* read length */
		fseek (input, 0, SEEK_SET);			/* rewind */
		
		printf("Analysis of file %s (%lu bytes)\n\n", infilename, filelength);
		tiffoffset = 0;
		c = getc(input); d = getc(input);
		if ((c == 'I' && d == 'I') || (c == 'M' && d == 'M')) ;
		else {
			e = getc(input); f = getc(input);
			if (c == 'E'+128 && d == 'P'+128 && e == 'S'+128 && f == 'F'+128) {
/* read over PS-start, PS-length, MF-start, MF-end */
				leastfirst = 1;
				for (k = 0; k < 4; k++) (void) ureadfour(input);
				tiffoffset = ureadfour(input);	/* read TIFF start offset */
				tifflength = ureadfour(input);	/* read TIFF length */
				printf("TIFF subfile at %ld of length %ld\n", 
					tiffoffset, tifflength);
				(void) ureadtwo(input);				/* should be 255, 255 */
				if (tiffoffset == 0 || tifflength == 0) {
					fclose(input);
					fprintf(stderr, 
						"TIFF offset or length in EPSF file are zero\n");
					exit(7);
				}
				if(fseek(input,	(long) tiffoffset, SEEK_SET) != 0)  {
					fclose(input);
					fprintf(stderr, "EPSF file seek error\n");
					exit(8);
				}
				c = getc(input); d = getc(input);
			}
			else {
				fprintf(stderr, "Not a valid TIFF or EPSF file\n");
				exit(13);
			}
		}

		if (c == 'I' && d == 'I') {
			leastfirst = 1;
			printf("II form (least significant -> most significant)  ");
		}
		else if (c == 'M' && d == 'M') {
			printf("MM form (most significant -> least significant)  ");
			leastfirst = 0;
		}
		else {
			fprintf(stderr, "File starts with `%c%c' instead of `II' or `MM'\n", 
				c, d);
			exit(3);
		}
		version = ureadtwo(input);
		if (version != 42) {
			fprintf(stderr, "The `version number' is %u instead of 42\n", 
				version);
			exit(4);
		}
		printf("TIFF `version' %u\n", version);
	
		ifdposition = ureadfour(input); nifd = 1;
	
		for(;;) {

			printf("\n"); 

/*			protect from missing zeros after last IFD ... NEW */

			if(fseek(input, (long) (ifdposition + tiffoffset), SEEK_SET) != 0) {
				fprintf(stderr, "Seek to IFD failed\n");
				break;
			}

			if (tiffoffset == 0) 
				printf("Image File Directory (IFD) # %d starts at %lu",
					nifd, ifdposition);
			else 
				printf("Image File Directory (IFD) # %d starts at %lu (+ %ld) ",
					nifd, ifdposition, tiffoffset);

			showfields(input, ifdposition);	printf("\n");

			if (showasciiflag != 0) {
				showascii(input, ifdposition); /* printf("\n"); */
			}

			if (showratioflag != 0) {
				showratio(input, ifdposition); /* printf("\n"); */
			}

			if (showlongflag != 0) {
				showlong(input, ifdposition); /* printf("\n"); */
			}		
			if (showshortflag != 0) {
				showshort(input, ifdposition); /* printf("\n"); */
			}				

			ifdposition = ureadfour(input);
			if (ifdposition == 0) break;		/* NO MORE IFDs */
			if (ifdposition > filelength) {
				fprintf(stderr,
	"\nGiving up: supposed next IFD position (%lu) past end of file at (%lu)\n",
				ifdposition, filelength);
				break;	/* bugus IFD - bad file termination */
			}
			nifd++;
		}
		fclose(input);
		putc('\n', stdout);
	}
	return 0;
}

/* NOTE: if offset is used directly for value */
/* then value is packed left justified */
/* so above is only `correct' if type is `II' - not if type is `MM' */

/* allow for 128 MacBinary header maybe ? */

/* Show `ColorMap' info in special form ? */

/* show `RowsPerStrip' in special form when (unsigned) -1 ? */
