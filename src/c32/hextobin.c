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

/* From BinHex format to MacBinary 98/Jun/23 */
/* This allocates memory to hold the file ... */
/* It rearranges the file header as it reads it */
/* and does the `decompression' and unpacking of bits on the writing */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ESC 144

/* ESC is used for run length compression */

#define MACBINHEAD 128

#define MAXFILE 64

#define MAXFORK 16000000

/* #define INPUTMODE "r" */
#define INPUTMODE "rb"

#define OUTPUTMODE "wb"

#define MAXLINE 512

#define BADCODE 255

int verboseflag=1;
int traceflag=0;
int debugflag=1;
int dumpflag=0;
int dataforkonly=1;		/* If no resource fork, write data fork only */
int translatetext=1;	/* convert ^M to ^M^J in output if data only */

int dataonly=0;			/* data fork only */

char *inexten="hqx";
char *outexten="bin";

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

int nName;			/* length of file name */

int nVersion=0;

int Flags=0;

int bitsused;		/* how many bits used in last byte of outline */

char bhcode[64]=	/* BinHeX 6 bit code */
	 "!\"#$%&'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";

/* omits 7 (2), O (0), ] ([), g (9), n (m), o (0) */

char inline[MAXLINE];

unsigned char outline[MAXLINE];

unsigned char *sout;

unsigned char bhdecode[256]="";

char *comment="(This file must be converted with BinHex 4.0)"; /* Yves Lempereur */

char *programversion="HEXtoBIN for MacBinary 1.0";

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

void makebhdecode(void) {				/* initialize bhdecode */
	int c, k;
	for (c = 0; c < 255; c++) bhdecode[c] = BADCODE;
	for (k = 0; k < 64; k++) {
		c = bhcode[k];
		bhdecode[c] = (unsigned char) k;
	}
}

void dumphead(unsigned char *file) {
	int k;
	printf("HEADER:\n");
	for (k = 0; k < 128; k++)
		if (file[k] != 0)
			printf("%d\t%02X\t%u\t(%c)\n", k, file[k], file[k], file[k]);
}

int emitsixbits (int c) {
	if (c < 0 || c > 63) {
		printf("ERROR: %d\n", c);
		return 0;
	}
	switch (bitsused) {
		case 0:
			*sout = (unsigned char) (c << 2);
			bitsused = 6;
			break;
		case 6:
			*sout  = (unsigned char) (*sout | (c >> 4));
			sout++;
			*sout = (unsigned char) ((c << 4) & 255);
			bitsused = 4;
			break;
		case 4:
			*sout = (unsigned char) (*sout | (c >> 2));
			sout++;
			*sout = (unsigned char) ((c << 6) & 255);
			bitsused = 2;
			break;
		case 2:
			*sout  = (unsigned char) (*sout | c);
			sout++;
			bitsused = 0;
			break;
		default:
			fprintf(errout, "Bit packing error!"); 
			break;
	}
	return 0;
}


int unpackaline (char *line) {
	char *s = line;
	int c, cdec;
	c = *s++;
	while (c > 0) {
		if (c != ' ' && c != '\n' && c != '\r' && c != ':') {
			cdec = bhdecode[c];
			if (cdec == BADCODE) 
				printf("BAD BinHex code: %d (%c)\n", c, c);
			else emitsixbits(cdec);
/*			if (traceflag) printf("(%c => %d) ", c, bhdecode[c]); */
		}
		c = *s++;
		if (c == ':') {
/*			if (traceflag) printf ("Found colon in %s", line);  */
			return 1;
		}
	}
	return 0;
}

/* fgets that works with any line termination ^M, ^J, or ^M^J */

char *fgetx(char *buffer, int nlen, FILE *input) {
	int c;
	char *s=buffer;
	while ((c = getc(input)) != EOF) {
		*s++ = (char) c;
		if (c == '\n' || c == '\r') break;
		if ((s - buffer) >= nlen-2) break;
	}
	if (c == EOF && s == buffer) return NULL;
	if (c == '\r') {
		c = getc(input);
		if (c == '\n') *s++ = (char) c;
		else ungetc(c, input);
	}
	*s++ = '\0';
	return buffer;
}

unsigned long readhexfile(FILE *input) {
	int c, flag;
	sout = infile;
	bitsused = 0;
	while (fgetx(inline, sizeof(inline), input) != NULL) {
		if (strstr(inline, "converted") != NULL &&
			strstr(inline, "BinHex") != NULL) break;
/*		actually, expecting the string in comment */
	}
	while ((c = getc(input)) != ':' && c != EOF) ;
	ungetc((char) c, input);
	while (fgetx(inline, sizeof(inline), input) != NULL) {
/*		if (sout >= infile + InLen - 64) { */
		if (sout >= infile + nInBuf - 64) {
			fprintf(stderr, " Overun input file buffer %u\n", nInBuf);
			exit(1);
		}
/*		if (traceflag) printf("IN:  %s", inline); */
		flag = unpackaline(inline);
		if (flag) break;
	}
	InLen = sout - infile;			/* change to actual space used */
	if (verboseflag) printf("InLen %u\n", InLen);
	if (traceflag) dumphead(infile);
	return InLen;
}

/* can do this only after expansion of ESC */

int extracthead(void) {
	int k;
	unsigned int CRC, CRCD;
	unsigned long n;

	nName = outfile[0];
	if (nName == 0) {
		dumphead(outfile);
		printf("Zero length Mac file name\n");
		return -1;
	}
	if (nName > MAXFILE) {
		dumphead(outfile);
		printf("Mac file name too long %d\n", nName);
		return -1;
	}
	for (k = 0; k < nName; k++) FileName[k] = outfile[k+1];
	FileName[nName] = '\0';
	for (k = 0; k < 4; k++) Type[k] = outfile[k+nName+2];
	Type[4] = '\0';
	for (k = 0; k < 4; k++) Creator[k] = outfile[k+nName+6];
	Creator[4] = '\0';
	Flags = (outfile[nName+10] << 8) | outfile[nName+11];	/* ??? */
	n = 0;
	for (k = 0; k < 4; k++) n = (n << 8) | outfile[k+nName+12];
	nData = n;
	if (traceflag)
		for (k = 0; k < 4; k++) n = printf("%d ", outfile[k+nName+12]);
	n = 0;
	for (k = 0; k < 4; k++) n = (n << 8) | outfile[k+nName+16];
	nResource = n;
	if (traceflag)
		for (k = 0; k < 4; k++) n = printf("%d ", outfile[k+nName+16]);
	OffData = nName+22;
	OffResource = nName+22+nData+2;
	OffEnd = nName+22+nData+2+nResource+2;
	if (verboseflag) {
		printf("Name: %s Type: %s Creator: %s\n", FileName, Type, Creator);
		printf("Data Fork at %u length %u  Resource Fork at %u length %u\n",
			   OffData, nData, OffResource, nResource);
		printf("Actual file length %u Decompressed file length %u\n",
			   InLen, OffEnd);
	}
	if (dumpflag || traceflag) dumphead(outfile);
	if (nData > MAXFORK || nResource > MAXFORK) {
		printf("Apparently inconsistent data or resource length\n");
		return -1;
	}
	CRC = (outfile[nName+20] << 8) | outfile[nName+21];
	CRCD = macbinary_crc(outfile, nName+20, 0);
	if (CRC != CRCD) 
		printf("CRC %s mismatch %04X %04X\n", "header", CRC, CRCD);
	CRC = (outfile[OffData+nData] << 8) | outfile[OffData+nData+1];
	CRCD = macbinary_crc(outfile+OffData, nData, 0);
	if (CRC != CRCD) 
		printf("CRC %s mismatch %04X %04X\n", "data fork", CRC, CRCD);
	CRC = (outfile[OffResource+nResource] << 8) | outfile[OffResource+nResource+1];
	CRCD = macbinary_crc(outfile+OffResource, nResource, 0);
	if (CRC != CRCD) 
		printf("CRC %s mismatch %04X %04X\n", "resource fork", CRC, CRCD);
	return 0;
}

int nocontrolchars(void) {	/* returns 1 if there are no control chars */
	unsigned char *s;
	int c;
	s = outfile + OffData;
	while (s < outfile + OffData + nData) {
		if ((c = *s++) < 32 && c != '\t' && c != '\r' && c != '\n') {
			printf("Control character %d C-%c found\n", c, c+64);
			return 0;
		}
	}
	return 1;
}

int howmanyescaped(void) {
	unsigned long k, count = 0;
	for (k = 0; k < InLen; k++) 
		if (infile[k] == ESC) count += infile[k+1];
	return count;
}

unsigned long decompress(void) {
	unsigned char *s, *t;
	int k, n, c, cold;
	
/*	OutLen = InLen + howmanyescaped(); */
	nOutBuf = InLen + howmanyescaped();
	outfile = malloc(nOutBuf);
	if (outfile == NULL) {
		fprintf(stderr, "Unable to allocate %u bytes\n", nOutBuf);
		exit(1);
	}
	if (debugflag) printf("Allocated %u bytes to outfile buffer\n", nOutBuf);
	s = infile;
	t = outfile;
	cold = -1;
	if (*s == ESC) {
		printf("ERROR: First byte of data is repeat escape code\n");
		return 0;
	}
	while (s < infile + InLen) {
/*		if (t >= outfile + OutLen) { */
		if (t >= outfile + nOutBuf) {
			fprintf(stderr, "Buffer Overrun %u\n", nOutBuf);
			exit(1);
		}
		c = *s++;
		if (c != ESC) *t++ = (unsigned char) c;
		else {
			n = *s++;		/* 3 <= n <= 255 or 0 */
			if (traceflag) printf("%d ESC %d ", *(t-1), n);
			if (n == 0) *t++ = (unsigned char) c;				
			else {
				if (n < 3) printf("WARNING: invalid repeat code %d\n", n);
				cold = *(t-1);
				for (k = 1; k < n; k++) *t++ = (unsigned char) cold;
			}
		}
	}
	OutLen = t - outfile;	/* actual space needed for output in buffer */
	return OutLen;
}

void writebinhead(FILE *output) {
	int k, n;
	unsigned long len;
	n = strlen((char *) FileName);
	putc(0, output);
	putc(n, output);
	for (k = 0; k < n; k++) putc(FileName[k], output);
	for (k = n+1; k < 64; k++) putc(0, output);
/*	should now be at byte 65 */
	if (ftell(output) != 65) printf("%d <> %d\n", ftell(output), 65);
	for (k = 0; k < 4; k++) putc(Type[k], output);
	for (k = 0; k < 4; k++) putc(Creator[k], output);
	putc(Flags, output);		/* ??? */
	putc(0, output);			/* zero */
	for (k = 0; k < 6; k++) putc(0, output); /* desktop location */
	putc(0, output);			/* protected flag */
	putc(0, output);			/* zero */
/*	should now be at byte 83 */
	if (ftell(output) != 83) printf("%d <> %d\n", ftell(output), 83);
	len = nData;
	for (k = 0; k < 4; k++) {
		putc(len >> 24, output);
		len = (len << 8);
	}
	len = nResource;
	for (k = 0; k < 4; k++) {
		putc(len >> 24, output);
		len = (len << 8);
	}
/*	should now be at byte 91 */
	if (ftell(output) != 91) printf("%d <> %d\n", ftell(output), 91);
	for (k = 0; k < 4; k++) putc(0, output); /* creation date */
	for (k = 0; k < 4; k++) putc(0, output); /* modification date */
/*	should now be at byte 99 */
	if (ftell(output) != 99) printf("%d <> %d\n", ftell(output), 99);
	for (k = 99; k < 128; k++) putc(0, output); /* padding */
	if (ftell(output) != 128) printf("%d <> %d\n", ftell(output), 128);
}

void writebinfile(FILE *output) {
	unsigned long n;
	if (dataforkonly == 0 || nResource != 0) writebinhead (output);
	n = fwrite(outfile + OffData, 1, nData, output);
	if (n != nData) {
		fprintf(errout, "Failed to write %u bytes\n", nData);
	}
	n = fwrite(outfile + OffResource, 1, nResource, output);
	if (n != nResource) {
		fprintf(errout, "Failed to write %u bytes\n", nResource);
	}
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
	printf("\tT write even data fork only files in MacBinary format\n");
	printf("\t  default: write data fork only if resource fork empty\n");
	printf("\tM do not convert line terminations in non MacBinary files\n");
	printf("\t  default: if data only, convert line terminations to IBM PC form\n");
	printf("\t  Output appears in current directory, extension %s\n", outexten);
	exit(1);
}

int commandline(int argc, char *argv[]) {
	int k=1;
	while (k < argc && argv[k][0] == '-') {
		if (strcmp(argv[k], "-v") == 0) verboseflag = ! verboseflag;
		if (strcmp(argv[k], "-t") == 0) traceflag = ! traceflag;
		if (strcmp(argv[k], "-d") == 0) debugflag = ! debugflag;
		if (strcmp(argv[k], "-D") == 0) dumpflag = ! dumpflag;
		if (strcmp(argv[k], "-M") == 0) translatetext = ! translatetext;
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
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX]="";
	FILE *input, *output;
	int firstarg=1;
	char *outmode="";
	int m;
	
	if (argc < firstarg+1) showusage(argc, argv);
	printf("%s - %s\n", programversion, copyright);
	if (checkcopyright(copyright) != 0) {
		exit(1);	/* check for tampering */
	}
	firstarg = commandline(argc, argv);
	if (argc < firstarg+1) showusage(argc, argv);
	makebhdecode();

	for (m = firstarg; m < argc; m++) {
		strcpy(infilename, argv[m]);
		extension(infilename, inexten);
		if ((input = fopen(infilename, INPUTMODE)) == NULL) {
			perror(infilename);
			continue;
		}
		if (verboseflag) printf("Working on %s\n", infilename);
		fseek(input, 0, SEEK_END);
		InLen = ftell(input);
		fseek(input, 0, SEEK_SET);
/*		InLen = InLen * 6 / 8 + 64; */		/* upper bound on space needed */
		nInBuf = InLen * 6 / 8 + 64;		/* upper bound on space needed */
		infile =  malloc(nInBuf);
		if (infile == NULL) {
			fprintf(stderr, "Unable to allocate %u bytes\n", nInBuf);
			exit(1);
		}
		if (debugflag) printf("Allocated %u bytes to infile buffer\n", nInBuf);
		readhexfile(input);
		if (ferror(input)) perror(infilename);
		fclose(input);
		decompress();
		if (extracthead()) {
			printf("Apparently bad hex file\n");
			continue;
		}
		
		dataonly = 0;
		if (nResource == 0 && dataforkonly) {
			if (verboseflag) printf("Resource fork empty\n");
			if (nocontrolchars()) {
				printf("Will translate line terminations\n");
				dataonly = 1;
			}
			else {
				if (verboseflag) printf("Appears not to be plain ASCII\n");
			}
		}

		strcpy(outfilename, extractfilename(argv[m]));
		forceexten(outfilename, outexten);

		if (dataonly) outmode = "w";
		else outmode = OUTPUTMODE;
		if ((output = fopen(outfilename, outmode)) == NULL) {
			perror(outfilename);
			exit(1);
		}

		if (verboseflag) printf ("%s => %s\n", infilename, outfilename);
		writebinfile(output);
		if (ferror(output)) perror(outfilename);
		fclose(output);
		if (outfile != NULL) {
			outfile = NULL;
			free(outfile);
		}
		if (infile != NULL) {
			free(infile);
			infile = NULL;
		}
		if (verboseflag) printf("\n");
	}
	return 0;
}

/****************************************************************************/

#ifdef IGNORED
void dumpaline (FILE *output, unsigned char *outline) {
	int c, k, rep, i;
/*	int nn = (sout - outline) / 3; */
	int nn = (sout - outline-1) / 3;
	int n = nn * 3;
	if (traceflag) printf("# %d -> %d -> %d #",
						  sout-outline, nn, n);
	while (n > 0 && outline[n-1] == ESC) {
		if (debugflag) printf("ESC at end %d ", n);
		n = n-3;
	}
	for (k = 0; k < n; k++) {
		if (outline[k] == ESC) {	/* $90 */
			k++;
			rep = outline[k];		/* Repeat count */
			if (rep > 0) {
				for (i = 1; i < rep; i++) putc(cold, output);
			}
			else putc(ESC, output);
			if (debugflag) {
				if (rep > 0) printf("ESC rep %d (%c) x %d ", cold, cold, rep);
				else printf("ESC %d ", ESC);
			}
		}
		else {
			c = outline[k];
			putc(c, output);
			cold = c;				/* in case ESC follows */
		}
	}
/*	memmove(outline, outline+n, sout - (outline + n) + 1); */
	memcpy(outline, outline+n, sout - (outline + n) + 1);
	sout = sout - n;		/*	and keep bitsused same */
	if (traceflag) printf("* %d %d *", bitsused, sout - outline);
/*	if (debugflag) putc('\n', output); */
}
#endif

#ifdef IGNORED
void dumpend (FILE *output, unsigned char *outline) {
	int k;
	int n = (sout - outline);
	if (bitsused > 0) n++; 
	for (k = 0; k < n; k++) putc(outline[k], output);
	sout = outline;
	bitsused = 0;
}
#endif

#ifdef IGNORED
void spitline(unsigned char *outline, int n) {
	int c, k;
	for (k = 0; k < n; k++) {
		c = outline[k];
		if (c == 0) putc('@', stdout);
		else putc(c, stdout);
/*		if (c < 32) printf("C-%c", c+64);
		else if (c < 128) printf("%c", c);
		else if (c < 160) printf("M-C-%c", c-128+64);
		else printf("M-%c", c-128); */
	}
}
#endif

#ifdef IGNORED
void unpackafile(FILE *output, FILE *input) {
	int c, flag;
	sout = outline;
	bitsused = 0;
	while ((c = getc(input)) != ':' && c != EOF) ;
	ungetc(c, input);
	while (fgetx(inline, sizeof(inline), input) != NULL) {
		if (traceflag) printf("IN:  %s", inline);
		flag = unpackaline(inline);
		if (flag) break;
		if (traceflag) spitline(outline, sout - outline);
		dumpaline(output, outline);
	}
	dumpend(output, outline);
}
#endif

/****************************************************************************/

/* 0X90 is special repeat marker */
/* A 0x90 n => A A A ... A n times */
/* 0x90 0x00 => 0x90 */

/* 0x90 0x90 0x90 ... 0x90 -> 0x90 0x00 0x90 n-1 */

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

/* does data + resource fork length have to be padded to multiple of 512 ? */

/* should it be padded out ? */
