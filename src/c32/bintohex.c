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

/* From MacBinary to real BinHex format 98/Jun/23 */
/* This allocates memory to hold the file ... */
/* It rearranges the file header as it reads it */
/* and does the `compression' and packing of bits on the writing */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ESC 144

/* ESC is used for run length compression */

#define MACBINHEAD 128

#define MAXFILE 64

#define MAXFORK 16000000

#define INPUTMODE "rb"

/* #define OUTPUTMODE "w" */
#define OUTPUTMODE "wb"

#define DEFTYPE "TEXT"

#define DEFCREATOR "ttxt"

int verboseflag=0;
int traceflag=0;
int debugflag=0;
int dumpflag=0;
int mactermination=0;	/* use ^M instead of ^M^J in BinHex file */
int dataforkonly=1;		/* If not MacBinary form, write data fork only */
int translatetext=1;	/* convert ^M^J to ^M in input if data only */

int dataonly=0;			/* no MacBinary header found in input */

unsigned char *infile=NULL;
unsigned char *outfile=NULL;

unsigned long InLen;		/* length of input file */
unsigned long OutLen;		/* length of output file */
unsigned long nInBuf;		/* length of input buffer allocated */
unsigned long nOutBuf;		/* length of output buffer allocated */
unsigned long nData, nResource;
unsigned long OffData, OffResource, OffEnd;	/* in input file */
unsigned long OffDataX, OffResourceX, OffEndX;	/* in buffer */

unsigned char MacBinHeader[MACBINHEAD];
unsigned char FileName[MAXFILE];
unsigned char Type[4+1], Creator[4+1];

char *inexten="mac";
char *outexten="hqx";

int nName;			/* length of file name */

int nVersion=0;

int Flags=0;

int bitsused;		/* how many bits used in last byte of outline */

unsigned char *bufptr;

char bhcode[64]=	/* BinHeX 6 bit code */
	"!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";

char *comment="(This file must be converted with BinHex 4.0)"; /* Yves Lempereur */

char *programversion="BINtoHEX for MacBinary 1.0";

char *copyright="Copyright (C) 1998 Y&Y, Inc. http://www.YandY.com";

#define COPYHASH 12855365

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

FILE *errout=stderr;

/******************************************************************************/

/* Array useful for CRC calculations that use 0x1021 as the "multiplier" (?): */

unsigned int crcmagic[] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

/* macbinary_crc DATA, SEED */

/* Compute the MacBinary-II-style CRC for the given DATA, with the CRC
 * seeded to SEED.  Normally, you start with a SEED of 0, and you pump in
 * the previous CRC as the SEED if you're handling a lot of data one chunk
 * at a time.  That is: */

/*    $crc = 0;
 *    while (<STDIN>) {
 *        $crc = macbinary_crc($_, $crc);
 *    } */

/* I<Note:> Extracted from the I<mcvert> utility (Doug Moore, April '87),
 * using a "magic array" algorithm by Jim Van Verth for efficiency.
 * Converted to Perl5 by Eryq.  B<Untested.> */

unsigned int macbinary_crc (unsigned char *data, long len, unsigned int seed) {
/*	int len = strlen(data); */
	long i;
	unsigned int crc = seed;

	for (i = 0; i < len; i++) {
/*		($crc ^= (vec($_[0], $i, 8) << 8)) &= 0xFFFF; */
/*		crc = (crc ^ (data[i] << 8)) & 0xFFFF; */
		crc = crc ^ (data[i] << 8);
/*		$crc = ($crc << 8) ^ $MAGIC[$crc >> 8]; */
/*		crc = (crc << 8) ^ crcmagic[crc >> 8]; */
		crc = ((crc << 8) & 0xFFFF) ^ crcmagic[crc >> 8];
	}
	return crc;
}

/* ------------------------------------------------------------ */

/* binhex_crc DATA, SEED */

/* Compute the HQX-style CRC for the given DATA, with the CRC seeded to SEED.
 * Normally, you start with a SEED of 0, and you pump in the previous CRC as
 * the SEED if you're handling a lot of data one chunk at a time.  That is: */

/*    $crc = 0;
 *    while (<STDIN>) {
 *        $crc = binhex_crc($_, $crc);
 *    } */

/* I<Note:> Extracted from the I<mcvert> utility (Doug Moore, April '87),
 * using a "magic array" algorithm by Jim Van Verth for efficiency.
 * Converted to Perl5 by Eryq. */


unsigned int binhex_crc (unsigned char *data, long len, unsigned int seed) {
/*	len = strlen(data); */
	long i;
	unsigned int crc = seed;

	for (i = 0; i < len; i++) {
/*		ocrc = crc; */
		crc = ((((crc & 0xFF) << 8) | data[i])
			   ^ crcmagic[crc >> 8]) & 0xFFFF;
/*	 printf("CRCin = %04x, char = %02x (%c), CRCout = %04x\n",
 *        ocrc, data[i], data[i], crc); */
	}
	return crc;
}

/******************************************************************************/

void dumphead(unsigned char *file) {
	int k;
	printf("DUMP:\n");
	for (k = 0; k < 128; k++)
		if (file[k] != 0)
			printf("%d\t%02X\t%u\t(%c)\n", k, file[k], file[k], file[k]);
}

/* read the file into infile, while rearranging header */
/* return -1 if does not appear to be a MacBinary format file */

int readbinhead(FILE *input) {
	int k;
	unsigned long n;

	n = fread(MacBinHeader, 1, MACBINHEAD, input);
	if (n != MACBINHEAD) {
		printf("Read only %u out of %d MacBinary header\n", n, MACBINHEAD);
		return -1;
	}
	k = MacBinHeader[0];
	if (k != 0) {
		printf("Not a MacBinary file %s %d\n", "first byte", k);
		return -1;
	}
	nName = MacBinHeader[1];
	if (nName < 1 || nName > 63) {
		printf("Not a MacBinary file %s %d\n", "second byte", nName);
		return -1;
	}
	for (k = 0; k < nName; k++) FileName[k] = MacBinHeader[k+2];	/* Name */	
	FileName[nName] = '\0';
	for (k = 0; k < 4; k++) Type[k] = MacBinHeader[k+65];		/* Type */
	Type[4] = '\0';
	for (k = 0; k < 4; k++) Creator[k] = MacBinHeader[k+69];	/* Creator */
	Creator[4] = '\0';
	n = 0;
	for (k = 0; k < 4; k++) n = (n << 8) | MacBinHeader[83+k];	/* nData */
	nData = n;			/* data fork size */
	n = 0;
	for (k = 0; k < 4; k++) n = (n << 8) | MacBinHeader[87+k];	/* nResource */
	nResource = n;		/* resource fork size */
	OffData=MACBINHEAD;
	OffResource=OffData+nData;
	OffEnd=	OffResource+nResource;
	if (verboseflag) {
		printf("Name: %s Type: %s Creator: %s\n", FileName, Type, Creator);
		printf("Data Fork at %u length %u  Resource Fork at %u length %u\n",
			   OffData, nData, OffResource, nResource);
		printf("Overall length %u bytes (file size %u)\n", OffEnd, InLen);
	}
	if (OffEnd > InLen) {
		fprintf(errout, "Not a MacBinary file %s %u\n",
				"length mismatch", MACBINHEAD+nData+nResource);
		return -1;
	}
	Flags = MacBinHeader[73] << 8;					/* ??? */
	if (traceflag) dumphead(MacBinHeader);
	return 0;
}

/* construct (smaller) BinHex header from MacBinary header */
/* then reads data and resource fork and puts them after header */

int readbinfile(FILE *input) {
	int k;
	unsigned int CRC;
	unsigned long n;

/*	allocate memory to hold the raw binary file */
/*	infile = malloc(InLen+128); */ 			/* a bit more than needed */
	nInBuf = nName + 20 + 2 + nData + 2+ nResource +2;
	infile = malloc(nInBuf);				/* ??? */
	if (traceflag) printf("Allocating %u bytes for input file\n", nInBuf);
	if (infile == NULL) {
/*		fprintf(stderr, "Unable to allocate %u bytes\n", InLen+128); */
		fprintf(stderr, "Unable to allocate %u bytes\n", nInBuf);
		exit(1);
	}
	if (traceflag) 	printf("nName %d OffEnd %u\n", nName, nInBuf);
/*	Now assemble BinHeX style data in infile buffer */
	infile[0] = (unsigned char) nName;
	strcpy((char *) (infile+1), (char *) FileName);			/* n bytes + 0 */
	infile[nName+1] = (unsigned char) nVersion;
	strcpy((char *) (infile+nName+2), (char *) Type);		/* 4 bytes + 0 */
	strcpy((char *) (infile+nName+6), (char *) Creator);	/* 4 bytes + 0 */
	infile[nName+10] = (char) ((Flags >> 8) & 255);
	infile[nName+11] = (char) (Flags & 255);
	n = nData;			/* length of data fork */
	for (k = 0; k < 4; k++) {
		infile[nName+12+3-k] = (char) (n & 255);
		n = (n >> 8);
	}
	n = nResource;		/* length of resource fork */
	for (k = 0; k < 4; k++) {
		infile[nName+16+3-k] = (char) (n & 255);
		n = (n >> 8);
	}
	CRC = macbinary_crc (infile, nName+20 , 0);
	infile[nName+20] = (unsigned char) (CRC >> 8);
	infile[nName+21] = (unsigned char) (CRC & 0xFF);
	if (dumpflag || traceflag) dumphead(infile);
	OffDataX = nName+22;
	OffResourceX = OffDataX + nData + 2;
	OffEndX = OffResourceX + nResource + 2;
	if (traceflag)
		printf("nName %d, OffDataX %u, OffResourceX %u, OffEndX %u\n",
			   nName, OffDataX, OffResourceX, OffEndX);
	fseek(input, OffData, SEEK_SET);
	n = fread(infile+OffDataX, 1, nData, input);
	if (n != nData) {
		fprintf(errout, "Read only %u bytes out of %u\n", n, nData);
		return -1;
	}
	CRC = macbinary_crc (infile+OffDataX, nData, 0);
	infile[OffDataX+nData] = (unsigned char) (CRC >> 8);
	infile[OffDataX+nData+1] = (unsigned char) (CRC & 0xFF);
	fseek(input, OffResource, SEEK_SET);
	n = fread(infile+OffResourceX, 1, nResource, input);
	if (n != nResource) {
		fprintf(errout, "Read only %u bytes out of %u\n", n, nResource);
		return -1;
	}
	CRC = macbinary_crc (infile+OffResourceX, nResource , 0);
	infile[OffResourceX+nResource] = (unsigned char) (CRC >> 8);
	infile[OffResourceX+nResource+1] = (unsigned char) (CRC & 0xFF);
	return 0;
}

int nocontrolchars(void) {
	unsigned char *s;
	int c;
	s = infile + OffDataX;
	while (s < infile + OffDataX + nData) {
		if ((c = *s++) < 32 && c != '\t' && c != '\r' && c != '\n') {
			if (verboseflag) 
				printf("Control character %d C-%c found\n", c, c+64);
			return 0;
		}
	}
	return 1;
}

/* only used if input has no MacBinary header */

unsigned long convertlines(void) {
	unsigned char *s, *t;
	int c;
	s = t = infile + OffDataX;
	while (s < infile + OffDataX + nData) {
		if ((c = *s++) == '\r') {	/* is it return ? */
			if (*s == '\n') s++;	/* step over newline */
		}
		*t++ = (unsigned char) c;
	}
	nData =  t - (infile + OffDataX);
	OffResource = OffData + nData;
	OffEnd = OffResource + nResource;
	return OffEnd;
}

/* count how many 0x90 bytes that will have to be `escaped' */

/* ambiguity about what to do with sequence of 0x90's ??? */
/* shouldn't count header? doesn't matter ... */

int howmanyescapes (void) {
	unsigned long k, n;
	long count=0;
	if (infile == NULL) {
		fprintf(stderr, "infile %d is NULL\n", infile);
		exit(1);
	}
/*	n = InLen; */
	n = OffEndX;			/* computed in readbinfile */
	for (k = 0; k < n; k++)
		if (infile[k] == ESC) count++;
	if (verboseflag)
		printf("Found %u ESC code bytes in %u of input\n", count, n);
	return count;
}

/* This is where we run the ESC sequence `compression' */
/* can't do in place since it may *expand* instead of contract! */
/* count should be 3->255 */

int compressfile(void) {
	unsigned char *s;
	unsigned char *t;
	int c, cold, count;
	int n;

	n = howmanyescapes();
/*	OutLen = InLen+n; */		/* temporary until we know better */
	nOutBuf = OffEndX+n;			/* temporary until we know better */
	if (traceflag) printf("Allocating %u bytes for ouput file\n", nOutBuf);
	outfile = malloc(nOutBuf);
	if (outfile == NULL) {
/*		fprintf(stderr, "Unable to allocate %u bytes\n", InLen+n); */
		fprintf(stderr, "Unable to allocate %u bytes\n", nOutBuf); 
		exit(1);
	}
	s = infile;
	t = outfile;
	if (s == NULL || t == NULL) {
		fprintf(stderr, "infile %u or outfile %u are NULL\n",
				infile, outfile);
		exit(1);
	}
	cold = -1;					/* previous byte */
	count = 1;					/* how many of this byte seen */
/*	for (k = 0; k < InLen; k++) { */
/*	for (k = 0; k < OffEndX; k++) { */
	while (s < infile + OffEndX) {
		c = *s;
		if (traceflag) {
			printf("%d\t%02X\t%u\t(%c)\n", s-infile, c, c, c);
		}
		if (c == cold && count < 255) {
			count++;	/* just count repeat */
			s++;
		}
		else if (count == 1) {					/* `normal' case */
			*t++ = (unsigned char) c;
			if (c == ESC) *t++ = 0;		/* quote escape code */
			cold = c;
			s++;
		}
		else if (count > 2) {	/* end of repeating sequence */
/*			*t++ = (unsigned char) cold; */ /* ??? */
			*t++ = (unsigned char) ESC;
			*t++ = (unsigned char) count;
			if (traceflag) printf("%d ESC %d\n", cold, count);
			cold = -1;			/* or keep ??? */
			count = 1;
/*			s--; */
		}
		else if (count > 1) {	/* only two in sequence */
/*			*t++ = (unsigned char) cold; */
			*t++ = (unsigned char) cold;
			if (cold == ESC) *t++ = 0;		/* quote escape code */
			if (traceflag) printf("%d %d\n", cold, cold);
			cold = -1;
			count = 1;
/*			s--; */
		}
	}
	OutLen = t - outfile;
	if (traceflag) printf("Output file %u bytes\n", OutLen);
	return OutLen;
}

int writeoutfile(FILE *output) {
	unsigned long n;
	if (output == NULL) return -1;
/*	Turn into hex data on the way out !!! */
	n = fwrite(outfile, 1, OutLen, output);
	if (n != OutLen) {
		fprintf(stderr, "Wrote only %u out of %u bytes\n", n, OutLen);
		exit(1);
	}
}

int column;

int getnibble(void) {
	int c, d;
	if (bufptr >= outfile + OutLen) {
		fprintf(stderr, "Input buffer exhausted\n");
		exit(1);
	}
	switch(bitsused) {
		case 0:
			c = (*bufptr) >> 2;
			bitsused = 6;
			break;
		case 2:
			c = (*bufptr) & 63;
/*			bitsused = 8; */
			bufptr++;
			bitsused = 0;
			break;
		case 4:
			c = (*bufptr) & 15;
			bufptr++;
			d = (*bufptr) >> 6;
			c = (c << 2) | d;
			bitsused = 2;
			break;
		case 6:
			c = (*bufptr) & 3;			
			bufptr++;
			d = (*bufptr) >> 4;
			c = (c << 4) | d;
			bitsused = 4;
			break;
		case 8:
			bufptr++;
			c = (*bufptr) >> 2;
			bitsused = 6;
			break;
		default:
			fprintf(errout, "Bit unpacking error!"); 
			break;
	}
	return c;
}

void lineterminate(FILE *output) {
	putc('\r', output);
	if (!mactermination) putc('\n', output);
}

int writefile(FILE *output) {
	unsigned char *s=outfile;
	int c;
	fputs(comment, output);
	lineterminate(output);
	bufptr = outfile;
	bitsused = 0;
	putc(':', output);
	column = 1;
	bitsused = 0;
	while (bufptr < outfile + OutLen) {
		c = getnibble();	/* get next six bytes */
		putc(bhcode[c], output);
		column++;
		if (column == 64) {
			lineterminate(output);
			column = 0;
		}
	}
	putc(':', output);
	lineterminate(output);
	return 0;
}


void extension(char *fname, char *str) { /* supply extension if none */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);
	}
}

void forceexten(char *fname, char *str) { /* change extension if present */
	char *s, *t;
	if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
		strcat(fname, "."); strcat(fname, str);	
	}
	else strcpy(s+1, str);		/* give it default extension */
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

void showusage(int argc, char *argv[]) {
	printf("%s [-v][-t][-m][-M][-T]\n", argv[0]);
	printf("\tv verbose mode\n");
	printf("\tt trace mode (more verbosity)\n");
	printf("\tD show MacBinary header (debugging)\n");
	printf("\tm use Mac line termination in hqx output file\n");
	printf("\tT process only files that are in MacBinary format\n");
	printf("\t  default: if input not MacBinary file, treat as data fork only\n");
	printf("\tM do not convert line terminations in non MacBinary files\n");
	printf("\t  default: if data only, convert line terminations to Mac form\n");
	printf("\t  Output appears in current directory, extension %s\n", outexten);
	exit(1);
}

int commandline(int argc, char *argv[]) {
	int k=1;
	while (k < argc && argv[k][0] == '-') {
		if (strcmp(argv[k], "-v") == 0) verboseflag = ! verboseflag;
		if (strcmp(argv[k], "-t") == 0) traceflag = ! traceflag;
		if (strcmp(argv[k], "-d") == 0) debugflag = ! debugflag;
		if (strcmp(argv[k], "-m") == 0) mactermination = ! mactermination;
		if (strcmp(argv[k], "-M") == 0) translatetext = ! translatetext;
		if (strcmp(argv[k], "-D") == 0) dumpflag = ! dumpflag;
		if (strcmp(argv[k], "-T") == 0) dataforkonly = ! dataforkonly;
		if (strcmp(argv[k], "-?") == 0) showusage(argc, argv);
		k++;
	}
	return k;
}

unsigned long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == COPYHASH) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	
	return hash;
}

int main (int argc, char *argv[]) {
	int m, firstarg=1;   
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	FILE *input, *output;

	if (argc < firstarg+1) showusage(argc, argv);
	printf("%s - %s\n", programversion, copyright);
	if (checkcopyright(copyright) != 0) {
		exit(1);	/* check for tampering */
	}
	firstarg = commandline(argc, argv);
	if (argc < firstarg+1) showusage(argc, argv);
	for (m = firstarg; m < argc; m++) {
		strcpy(infilename, argv[m]);
		extension(infilename, inexten);
		if ((input = fopen(infilename, INPUTMODE)) == NULL) {
			perror(infilename);
			continue;
		}
		if (verboseflag) printf("Working on %s\n", infilename);
		fseek(input, 0, SEEK_END);	/* go to end */
		InLen = ftell(input);		/* get overall file size */
		fseek(input, 0, SEEK_SET);	/* rewind */
		if (traceflag) printf("Input file %u bytes\n", InLen);
		dataonly = 0;
		if (readbinhead(input) < 0) {
			printf("%s is not a file in MacBinary format\n", infilename);
			if (dataforkonly == 0) {
				fclose(input);
				continue;
			}
			strcpy((char *) FileName, extractfilename((char *) infilename));
			nName = strlen(infilename);
			strcpy((char *) Type, (char *) DEFTYPE);			/* default */
			strcpy((char *) Creator, (char *) DEFCREATOR);	/* default */
			nData = InLen;
			nResource = 0;
			OffData = 0;
			OffResource = OffData + nData;
			OffEnd = OffResource + nResource;
			printf("Name: %s Type: %s Creator: %s\n", FileName, Type, Creator);
			printf("Data Fork at %u length %u  Resource Fork at %u length %u\n",
				   OffData, nData, OffResource, nResource);
			printf("Overall length %u bytes\n", OffEnd);
			dataonly=1;
		}
		fflush(stdout);
		readbinfile(input);
		dataonly = 0;
		if (nResource == 0 && dataforkonly) {
			if (nocontrolchars()) {
				printf("Will translate line terminations\n");
				convertlines();
				dataonly = 1;
			}
			else if (verboseflag) printf("Appears not to be plain ASCII\n");
		}
		fflush(stdout);
		if (ferror(input)) perror(infilename);
		fclose (input);
		compressfile();
		strcpy(outfilename, extractfilename(argv[m]));
		forceexten(outfilename, outexten);
		if ((output = fopen(outfilename, OUTPUTMODE)) == NULL) {
			perror(outfilename);
			continue;
		}
		if (verboseflag) printf ("%s => %s\n", infilename, outfilename);
		writefile(output);
		if (ferror(output)) perror(outfilename);
		fclose (output);
		if (outfile != NULL) {
			free(outfile);
			outfile = NULL;
		}
		if (infile != NULL) {
			free(infile);
			infile = NULL;
		}
		if (verboseflag) printf("\n");
	}
	return 0;
}


/* Macbinary header format 128 bytes */

/*	0	 1 BYTE		Zero	*/
/*	1	64 BYTES	File Name -  Pascal string format */
/*	65	 4 BYTES 	File Type	 (no length) */
/*	69	 4 BYTES	File Creator (no length) */
/*	73	 1 BYTE		File Flags */
/*	74	 1 BYTE		Zero */
/*	75	 6 BYTES	DeskTop Location */
/*  81	 1 BYTE		Protected Flag */
/*	82	 1 BYTE		Zero */
/*	83	 LONG		Data Fork Length	*/
/*	87	 LONG		Resource Fork Length	*/
/*	91	 LONG		Creation Date	*/
/*	95	 LONG		Modification Date	*/

/* BinHex header format 26 + nName ... + nData + nResource */

/* 0		1 BYTE		n, length of Name */
/* 1		n BYTES		Name */
/* n+1		1 BYTE		Version */
/* n+2		4 BYTES		Type */
/* n+6		4 BYTES		Creator */
/* n+10		2 BYTES		Flags (and $F800) */
/* n+12		4 BYTES		nData, length of Data Fork */
/* n+16		4 BYTES		nResource, length of Resource Fork */
/* n+20		2 BYTES		CRC */
/* n+22			nData		Data Fork */
/* n+22+nData				2 BYTES	CRC */
/* n+22+nData+2	nResource	Resource Fork */
/* n+22+nData+2+nResource	2 BYTES CRC */
/* n+22+nData+2+nResource+2		past end */

/* provide option to use Mac line termination on output ? */

/* provide option to translate Mac line termination in data before ? */

/* Eudora for Windows drops MacBinary header when decoding this in Windows */
