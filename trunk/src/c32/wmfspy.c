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

/* #define FNAMELEN 80 */

typedef int				BOOL;
typedef unsigned char	BYTE;
// typedef unsigned int	WORD;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

/* Predefined Clipboard Formats */
#define CF_TEXT				1
#define CF_BITMAP			2
#define CF_METAFILEPICT		3
#define CF_SYLK				4
#define CF_DIF				5
#define CF_TIFF				6
#define CF_OEMTEXT			7
#define CF_DIB				8
#define CF_PALETTE			9

/* Mapping Modes */
#define MM_TEXT			1
#define MM_LOMETRIC		2
#define MM_HIMETRIC		3
#define MM_LOENGLISH	4
#define MM_HIENGLISH	5
#define MM_TWIPS		6
#define MM_ISOTROPIC	7
#define MM_ANISOTROPIC	8

/* typedef struct tagMETAHEADER
{
	WORD	mtType;
	WORD	mtHeaderSize;
	WORD	mtVersion;
	DWORD	mtSize;
	WORD	mtNoObjects;
	DWORD	mtMaxRecord;
	WORD	mtNoParameters;
} METAHEADER; */

/* typedef struct tagMETAFILEPICT
  {
	LONG	mm;
	LONG	xExt;
	LONG	yExt;
	HANDLE	hMF; 
  } METAFILEPICT;
typedef METAFILEPICT; */

char *cbtext[] =
{"CF_UNKNOWN", "CF_TEXT", "CF_BITMAP", "CF_METAFILEPICT",
"CF_SYLK", "CF_DIF", "CF_TIFF", "CF_OEMTEXT",
"CF_DIB", "CF_PALETTE", ""};

char *mmtext[] = {
"MM_UNKNOWN", "MM_TEXT", "MM_LOMETRIC", "MM_HIMETRIC",
"MM_LOENGLISH", "MM_HIENGLISH", "MM_TWIPS", 
"MM_ISOTROPIC", "MM_ANISOTROPIC", ""};

int verboseflag=0;
int traceflag=0;

long nlen;				/* file length */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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

int placableanalyze (FILE *input) {
	DWORD key;
	WORD hmfzero;
//	int left, top, right, bottom;
	short left, top, right, bottom;
	WORD inch;
	DWORD reserved;
	WORD checksum;
	int flag=0;

	fread(&key, sizeof(key), 1, input);			/* 9ac6cdd7u */
	fread(&hmfzero, sizeof(hmfzero), 1, input);
	fread(&left, sizeof(left), 1, input);
	fread(&top, sizeof(top), 1, input);
	fread(&right, sizeof(right), 1, input);
	fread(&bottom, sizeof(bottom), 1, input);
	fread(&inch, sizeof(inch), 1, input);
	fread(&reserved, sizeof(reserved), 1, input);
	fread(&checksum, sizeof(checksum), 1, input);
	if (key != 0x9ac6cdd7u) {
/*		if (verboseflag) */
			printf("WARNING: Key %0lxu != %0lxu\n", key, 0x9ac6cdd7u);
		flag++;
	}
	if (hmfzero != 0) {
/*		if (verboseflag) */
			printf("WARNING: HMF ZERO %d != 0\n", hmfzero);
		flag++;
	}
	if (flag) return flag;
/*	if (verboseflag) printf("Key %0lx hmf %d\n", key, hmfzero); */
	if (traceflag) printf("Key %0lx hmf %d\n", key, hmfzero); 
	printf("Left %d Top %d Right %d Bottom %d (%d inch)\n",
		left, top, right, bottom, inch);
/* typical values inch == 1440 for measurements in inches */
/* typical values inch == 2540 for measurements in mm */
	if (inch == 2540) printf("Width %lg mm Height %lg mm\n",
		   (double) (right - left) / 100.0,
		   (double) (bottom - top) / 100.0);
	else printf("Width %lg inch Height %lg inch\n",
		   (double) (right - left) / inch,
		   (double) (bottom - top) / inch);
/*if (verboseflag) printf("Reserved %ld check %0xu\n", reserved, checksum);*/
	if (traceflag) printf("Reserved %ld check %0xu\n", reserved, checksum);
	return 0;
}

/* typedef struct tagMETAHEADER
{
	WORD	mtType;
	WORD	mtHeaderSize;
	WORD	mtVersion;
	DWORD	mtSize;
	WORD	mtNoObjects;
	DWORD	mtMaxRecord;
	WORD	mtNoParameters;
} METAHEADER; */

void wmfanalyze (FILE *input) {
	WORD mtType;
	WORD mtHeaderSize;
	WORD mtVersion;
	DWORD mtSize;
	WORD mtNoObjects;
	DWORD mtMaxRecord;
	WORD mtNoParameters;
	int nVer, dVer;

//	if (traceflag) printf("WMFANALYZE %ld\n", ftell(input));
	fread(&mtType, sizeof(mtType), 1, input);				/* 1 */
	fread(&mtHeaderSize, sizeof(mtHeaderSize), 1, input);	/* 9 */
	fread(&mtVersion, sizeof(mtVersion), 1, input);			/* 0x100 0x300 */
	fread(&mtSize, sizeof(mtSize), 1, input);		/* size of file in words */
	fread(&mtNoObjects, sizeof(mtNoObjects), 1, input);	/* max no of objects */
	fread(&mtMaxRecord, sizeof(mtMaxRecord), 1, input);	/* max record size */
	fread(&mtNoParameters, sizeof(mtNoParameters), 1, input);	/* 0 */

	nVer = mtVersion >> 8;
	dVer = mtVersion & 255;
/*	printf("Type %u HSize %u Vers %0x Size %lu NoObj %u MaxRec %lu NoPar %d\n",
		mtType, mtHeaderSize, mtVersion, mtSize, mtNoObjects, mtMaxRecord,
			mtNoParameters); */
	if (verboseflag) {
		printf(
	"MetaFile Type: %d (%s), Version: %x%02x (%s), Header Size: %u (words)\n",
			   mtType, (mtType == 1) ? "Memory" : "File",
			   nVer, dVer, (nVer == 1) ? "no DIB" : "supports DIB",
			   mtHeaderSize);
		printf(
	"Size: %lu (words), Max No. Objects: %u, Max Record Size: %lu (words)\n",
				mtSize,  mtNoObjects, mtMaxRecord, mtNoParameters);
	}
	else
printf("Type %u HSize %u Vers %x.%02x Size %lu NoObj %u MaxRec %lu NoPar %d\n",
		mtType, mtHeaderSize, nVer, dVer, mtSize, mtNoObjects, mtMaxRecord,
			mtNoParameters);
/* Type 1 metafile is in memory, Type 2 metafile is in disk file */
/* Hsize is in words */
/* mtVersion is 0x0100, or 0x0300 (support device-independent bitmaps) */
/* mtSize is in words */
/* mtNoObjects maximum number of objects that exists in MF at same time */
/* mtMaxRecord in words, size of largest record	in MF */
/* mtNoParameters is reserved */
}

/* typedef struct tagMETAFILEPICT
  {
	LONG	mm;
	LONG	xExt;
	LONG	yExt;
	HANDLE	hMF; 
  } METAFILEPICT;
typedef METAFILEPICT; */

void metafilepictanalyze(FILE *input) {
	int mm, id;
	int xExt, yExt;
	WORD hMF;

	fread(&mm, sizeof(mm), 1, input);
	fread(&xExt, sizeof(xExt), 1, input);
	fread(&yExt, sizeof(yExt), 1, input);
	fread(&hMF, sizeof(hMF), 1, input);
	if (mm < 0 || mm > 9) id = 0;
	else id = mm;
	printf("Mapping Mode: %d %s xExt %d yExt %d hMF %d\n",
		mm, mmtext[id], xExt, yExt, hMF);
}

/* mm mapping mode */

/* xExt size of picture in modes except MM_ISOTROPIC or MM_ANISOTROPIC */
/* xExt width of rectangle within which picture is drawn */
/* Coordinates in units that correspond to mampping mode */

/* yExt size of picture in modes except MM_ISOTROPIC or MM_ANISOTROPIC */
/* yExt height of rectangle within which picture is drawn */
/* Coordinates in units that correspond to mampping mode */

/* MM_ISOTROPIC and MM_ANISOTROPIC --- can be scaled */
/* xExt & yExt contain optional suggested size in MM_HIMETRIC units */

/* MM_ANISOTROPIC xExt and yExt can be zero if no suggested size supplied */

/* MM_ISOTROPIC aspect ratio must be supplied even when no suggested size */
/* To give aspect ratio without implying size, set xExt & yExt negative */
/* Then magnitude is ignored only ratio is used for aspect ratio */

/* MM_ISOTROPIC	7 */
/* MM_ANISOTROPIC 8 */

void clpanalyze (FILE *input) {
	WORD FileIdentifier;
	WORD FormatCount;
	WORD FormatID;
	DWORD LenData;
	DWORD OffData;
	char Name[79];
	int m, k, id;
	long present;

	fread(&FileIdentifier, sizeof(FileIdentifier), 1, input); /* 50000 */
	fread(&FormatCount, sizeof(FormatCount), 1, input);
	printf("FileIdentifier %u FormatCount %u\n",
		FileIdentifier, FormatCount);
	for (m = 0; m < (int) FormatCount; m++) {
		fread(&FormatID, sizeof(FormatID), 1, input);
		fread(&LenData, sizeof(LenData), 1, input);
		fread(&OffData, sizeof(OffData), 1, input);
		fread(Name, sizeof(Name), 1, input);		
		if (FormatID > 9) id = 0;
		else id = FormatID;
/*		printf("%d\t%u\tFormatID %s LenData %lu OffData %lu (%s)\n", */
		printf("%d\t%u\t%s LenData %lu OffData %lu (%s)\n",
			m, FormatID, cbtext[id], LenData, OffData, Name);
		present = ftell(input);							/* remember where */
		if (fseek (input, OffData, SEEK_SET) != 0) {
			fprintf(stderr, "CLP file seek error\n");
			if ((long) OffData > nlen)
				fprintf(stderr, "Format past end of file (%ld > %ld)\n",
					OffData, nlen);
			continue;
		}
		if (FormatID == CF_METAFILEPICT) {
			metafilepictanalyze(input);
			wmfanalyze(input);
		}
		else if (FormatID == CF_TEXT || FormatID == CF_OEMTEXT) {
			for (k = 0; k < (int) LenData; k++)
				putc(getc(input), stdout);
			putc('\n', stdout);
		}
		if (fseek (input, present, SEEK_SET) != 0) {	/* return */
			fprintf(stderr, "CLP file seek error\n");
		}
	}
}

unsigned long tiffoffset, tifflength;
unsigned long psoffset, pslength;
unsigned long metaoffset, metalength;

int seekerrors=0;

unsigned int leastfirst=1;	/* non-zero on PC, zero on Mac */

unsigned int ureadtwo(FILE *input) {
	unsigned int c, d;
	c = (unsigned int) getc(input);
	d = (unsigned int) getc(input);	
	if (c == EOF || d == EOF) {
		if (seekerrors++ > 16) exit(1);
		return 0;
	}
	if (leastfirst != 0) return ((d << 8) | c);
	else return ((c << 8) | d);
}

unsigned long ureadfour(FILE *input) {
	unsigned long c, d, e, f;
	c = (unsigned long) getc(input);
	d = (unsigned long) getc(input);
	e = (unsigned long) getc(input);
	f = (unsigned long) getc(input);	
	if (c == EOF || d == EOF || e == EOF || f == EOF) {
		if (seekerrors++ > 16) exit(1);
		return 0;
	}
	if (leastfirst != 0) return ((((((f << 8) | e) << 8) | d) << 8) | c);
	else return ((((((c << 8) | d) << 8) | e) << 8) | f);
}

int psanalyze (FILE *input) {
	char buffer[256];
	printf("Appears to be a raw PostScript file (i.e. no preview):\n");
	fgets(buffer, sizeof(buffer), input);
	fputs(buffer, stdout);
	return 0;
}

int epsanalyze (FILE *input) {
	int c, d, e, f;
	unsigned int checksum;

	c = getc(input); d = getc(input);
	e = getc(input); f = getc(input);
	if (c == 'E'+128 && d == 'P'+128 && e == 'S'+128 && f == 'F'+128) {
		printf ("EPSF file with MetaFile or TIFF header:\n");
	}
	else return 0;
/* read over PS-start, PS-length, MF-start, MF-end */
	psoffset = ureadfour(input);
	pslength = ureadfour(input);
	if (pslength > 0)
		printf("PS subfile at %ld of length %ld\n", 
	psoffset, pslength);
	metaoffset = ureadfour(input);
	metalength = ureadfour(input);
	if (metalength > 0)
		printf("MetaFile subfile at %ld of length %ld\n", 
			metaoffset, metalength);
	tiffoffset = ureadfour(input);	/* read TIFF start offset */
	tifflength = ureadfour(input);	/* read TIFF length */
	if (tifflength > 0)
		printf("TIFF subfile at %ld of length %ld\n", 
			tiffoffset, tifflength);
	checksum = ureadtwo(input);			/* should be 255, 255 */
	if (checksum != (unsigned int) -1)
		printf("CheckSum %0X\n", checksum);
	if (metaoffset == 0 || metalength == 0) {
		fprintf(stderr, "META offset or length in EPSF file are zero\n");
		return 0;
	}
	if (fseek(input, (long) metaoffset, SEEK_SET) != 0)  {
		fprintf(stderr, "EPSF file seek error\n");
		if ((long) metaoffset > nlen)
			fprintf(stderr, "WMF past end of file (%ld > %ld)\n",
					metaoffset, nlen);
		return 0;
	}
	return 1;
}

void showusage (char *argv[], int argc) {
	printf("%s [-v] [-t] <wmf-file1> <wmf-file2> ...\n", argv[0]);
	printf("\tv verbose mode\n");
	printf("\tt trace mode\n");
	exit(1);
}

/* FormatID 1	&Text */
/* FormatID 2	&Bitmap */
/* FormatID 3	&Picture (MetaFile?) */
/* FormatID 7	&OEM Text */
/* FormatID 9	Pal&ette */

int main(int argc, char *argv[]) {
	FILE *input;
	char filename[FILENAME_MAX];
	int c, m, flag;
	int firstarg=1;
	
	if (argc < 2) showusage(argv, argc);

	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) verboseflag++;
		else if (strcmp(argv[firstarg], "-t") == 0) traceflag++;
		else if (strcmp(argv[firstarg], "-?") == 0) showusage(argv, argc);
		firstarg++;
	}
	for (m = firstarg; m < argc; m++) {
		strcpy(filename, argv[m]);
		extension(filename, "wmf");
		if ((input = fopen(filename, "rb")) == NULL) {
			forceexten(filename, "clp");
			if ((input = fopen(filename, "rb")) == NULL) {
				forceexten(filename, "eps");
				if ((input = fopen(filename, "rb")) == NULL) {
/*					perror(filename); */
					perror(argv[m]);
					exit(1);
				}
			}
		}
		(void) fseek(input, 0, SEEK_END);
		nlen = ftell(input);
		(void) fseek(input, 0, SEEK_SET);		
		printf("FILE: %s (length %ld)\n", filename, nlen);
		c = getc(input);
		ungetc(c, input);
		if (c == 'P') clpanalyze(input);		/* P M-C */
		else if (c == '%') psanalyze(input);	/* %!PS-Adobe ... */
		else if (c == 197) {
			if (epsanalyze(input))	/* M-E M-P M-S M-F */
				wmfanalyze(input);
		}
		else {
			if (c == 1) {
				printf("NOT a placable WMF\n");
				wmfanalyze(input);
			}
			else if (c != 1) {
				flag = placableanalyze(input);
				if (flag) printf("Seems not to be a MetaFile file\n");
				else {
					printf("Placable WMF\n");
					wmfanalyze(input);
				}
			}
		}
		fclose(input);
		putc('\n', stdout);
	}
	return 0;
}

