/* Copyright 1991,1992,1993,1994,1995,1996,1997,1998,1999,2000 Y&Y, Inc.
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

/* Program to look at TIFF file innards */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIFF60

/* #define FNAMELEN 128 */

/* #define PTSPERINCH 72.27 */

#define PTSPERINCH 72

char *copyright = "\
";

char *progversion="TIFFTAGS 1.2.2   Copyright (C) 1991--2000 Y&Y, Inc., Copyright 2007 TeX Users Group";

int verboseflag = 0;				/* 1996/Mar/18 */
/* int verboseflag = 1; */			/* 1995/July/1 */
int detailflag = 1;					/* 1996/Mar/18 */
int traceflag = 0;					/* not used at all */
int splitflag = 0;					/* split EPSF file into EPS and TIFF */

int showasciiflag = 1;
int showratioflag = 1;
int showlongflag = 1;
int showshortflag = 1;

unsigned int compression=0;				/* remember compression scheme */
unsigned int photointerpretation=0;		/* remember photointerpretation */
unsigned int resolutionunit = 0;		/* remember resolutionunit */

unsigned long xresnum, xresden, yresnum, yresden;	/* 95/July/1 */

unsigned int imagewidth, imagelength;				/* 95/July/1 */

#define MAXSEEKERROR 8

int seekerrors=0;

unsigned int leastfirst=1;	/* non-zero on PC, zero on Mac */
unsigned int version=42;
unsigned int ifdcount;

unsigned long ifdposition, filelength;

unsigned long tiffoffset, tifflength;
unsigned long psoffset, pslength;
unsigned long metaoffset, metalength;

FILE *errout=stdout;

int xseek(FILE *input, unsigned long pos) {
	int flag;
	if (pos > filelength) {
		fprintf(errout, "Attempt to seek past end of file %ld > %ld\n",
				pos, filelength);
		return -1;
	}
	flag = fseek(input, pos, SEEK_SET);
	if (flag != 0) {
		fprintf(errout, "SEEK ERROR %lu\n", pos);
		if (seekerrors++ > MAXSEEKERROR) exit(1);
	}
	return flag;
}

unsigned int ureadtwo(FILE *input) {
	unsigned int c, d;
	c = (unsigned int) getc(input);
	d = (unsigned int) getc(input);	
	if (c == EOF || d == EOF) {
		if (seekerrors++ > MAXSEEKERROR) exit(1);
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
		if (seekerrors++ > MAXSEEKERROR) exit(1);
		return 0;
	}
	if (leastfirst != 0) return ((((((f << 8) | e) << 8) | d) << 8) | c);
	else return ((((((c << 8) | d) << 8) | e) << 8) | f);
}

/* TYPES for TAGS, mostly only 1, 2, 3, 4, 5 supported */

#define TYPE_BYTE 1			/* An 8-bit unsigned integer */
#define TYPE_ASCII 2		/* ASCII 8-bit byte that contains a 7-bit ASCII code */
#define TYPE_SHORT 3		/* SHORT 16-bit (2-byte) unsigned integer */
#define TYPE_LONG 4			/* LONG 32-bit (4-byte) unsigned integer */
#define TYPE_RATIO 5		/* RATIONAL Two LONGs: numerator and denominator */

#ifdef TIFF60
#define TYPE_SBYTE 6		/* An 8-bit signed (twos-complement) integer */
#define TYPE_UNDEFINED 7	/* An 8-bit byte that may contain anything */
#define TYPE_SSHORT 8		/* A 16-bit (2-byte) signed (twos-complement) integer */
#define TYPE_SLONG 9		/* A 32-bit (4-byte) signed (twos-complement) integer */
#define TYPE_SRATIONAL 10	/* Two SLONG’s: numerator and denominator */
#define TYPE_FLOAT 11		/* Single precision (4-byte) IEEE format */
#define TYPE_DOUBLE 12		/* Double precision (8-byte) IEEE format */
#endif

char *typename[] = {"", "BYTE ", "ASCII", "SHORT", "LONG ", "RATIO",
"SBYTE", "UNDEFINED", "SSHORT", "SLONG", "SRATIONAL", "FLOAT", "DOUBLE"};

unsigned int MaxType = sizeof(typename) / sizeof(typename[0]);

/* int typesize[6] = {0, 1, 1, 2, 4, 8}; */	/* units of length/count */

struct tagname {
	unsigned int tag; char *name;
};

#define MAXTAGNAME 25			/* mac number of characters in tag name */

/* struct tagname tagnames[MAXTAGS] = { */
struct tagname tagnames[] = {
{254, "NewSubfileType"},
{255, "SubfileType"},
{256, "ImageWidth"},
{257, "ImageLength"},
{258, "BitsPerSample"},
{259, "Compression"},
{262, "PhotometricInterpretation"},
{263, "Threshholding"},
{264, "CellWidth"},
{265, "CellLength"},
{266, "FillOrder"},
{269, "DocumentName"},
{270, "ImageDescription"},
{271, "Make"},
{272, "Model"},
{273, "StripOffsets"},
{274, "Orientation"},
{277, "SamplesPerPixel"},
{278, "RowsPerStrip"},
{279, "StripByteCounts"},
{280, "MinSampleValue"},
{281, "MaxSampleValue"},
{282, "XResolution"},
{283, "YResolution"},
{284, "PlanarConfiguration"},
{285, "PageName"},
{286, "XPosition"},
{287, "YPosition"},
{288, "FreeOffsets"},
{289, "FreeByteCounts"},
{290, "GrayResponseUnit"},
{291, "GrayResponseCurve"},
{292, "T4Options"}, /* "Group3Options" */
{293, "T6Options"}, /* "Group4Options" */
{296, "ResolutionUnit"},
{297, "PageNumber"},
{301, "TransferFunction"}, /* "ColorResponseCurves" */
{305, "Software"},
{306, "DateTime"},
{315, "Artist"},
{316, "HostComputer"},
{317, "Predictor"},
{318, "WhitePoint"},
{319, "PrimaryChromaticities"}, /* "ColorList" */
{320, "ColorMap"},
#ifdef TIFF60
{321, "HalftoneHints"},
{322, "TileWidth"},
{323, "TileLength"},
{324, "TileOffsets"},
{325, "TileByteCounts"},
/* {326, "BadFaxLines"}, */
/* {327, "CleanFaxData"}, */
/* {328, "ConsecutiveBadFaxLines"}, */
{332, "InkSets"},
{333, "InkNames"},
{334, "NumberOfInks"},
{336, "DotRange"},
{337, "TargetPrinter"},
{338, "ExtraSamples"},		/* important ... */
{339, "SampleFormat"},
{340, "SMinSampleValue"},
{341, "SMaxSampleValue"},
{342, "TransferRange"},
{512, "JPEGProc"},
{513, "JPEGInterchangeFormat"},
{514, "JPEGInterchangeFormatLngth"},
{515, "JPEGRestartInterval"},
{517, "JPEGLosslessPredictors"},
{518, "JPEGPointTransforms"},
{519, "JPEGQTables"},
{520, "JPEGDCTables"},
{521, "JPEGACTables"},
{529, "YCbCrCoefficients"},
{530, "YCbCrSubSampling"},
{531, "YCbCrPositioning"},
{532, "ReferenceBlack"},
{33432, "Copyright"},
#endif
{0, "Unknown"}
};

/* char *compstring[] = {
	"Unknown", "No", "TIFF CCITT", "CCITT Group 3", "CCITT Group 4", "LZW",
	"PackBits" 
}; */

char *compstring[] = {
	"Unknown", "Uncompressed", "CCITT 1D", "Group 3 Fax",
	"Group 4 Fax", "LZW", "JPEG", "PackBits"	/* 32773 */
};

/* char *interpretationstring[] = {
	"Inverted Gray", "Normal Gray", "RGB Color", "Palette Color", "Unknown"
}; */

char *interpretationstring[] = {
	"WhiteIsZero", "BlackIsZero", "RGB Color", "RGB Palette",
	"Transparency Mask", "CMYK", "YCbCr", "Unknown",
	"CIELab", "Unknown"
}; 

char *resolutionstring[] = {		/* default is 2 inch */
	"unknown", "not absolute", "inch", "centimeter"
};

char *lookupname (unsigned int tag) {
	unsigned int n, k;
	n = sizeof(tagnames);
	for (k = 0; k < n; k++) {	
		if (tagnames[k].tag == 0) return tagnames[k].name;
		if (tagnames[k].tag == tag) return tagnames[k].name;
	}
	return "Unknown";
}

void showfields(FILE *input, unsigned long ifdpos) {
	unsigned int j, k, n, tag, type;
	unsigned long length, offset;
	char *s;

	if (ifdpos + tiffoffset > filelength) {
		if (verboseflag) fprintf(errout, "\n");  /* ??? */
		fprintf(errout,
			"Giving up: supposed IFD position past end of file at %lu\n",
				filelength);
		return;
	}

	if (fseek(input, (long) (ifdpos + tiffoffset), SEEK_SET) != 0) {
		if (verboseflag) fprintf(errout, "\n");  /* ??? */
		fprintf(errout, "SEEK ERROR %lu\n", ifdpos);
		if (seekerrors++ > MAXSEEKERROR) exit(1);
		return;
	}
	ifdcount = ureadtwo(input);
	if (ifdcount == 0) {
		if (verboseflag) fprintf(errout, "\n"); /* ??? */
		fprintf(errout, "Giving up: IFD count == 0\n");
		return;
	}

	if (verboseflag) printf(" and has %d tags\n\n", ifdcount);
	
	for (k = 0; k < ifdcount; k++) {
		tag = ureadtwo(input);
		type = ureadtwo(input);
		if ((tag == 0 && type == 0)) {
			if (seekerrors++ > MAXSEEKERROR) exit(1);
			if (verboseflag) fprintf(errout, "\n");  /* ??? */
			fprintf(errout, "WARNING: Tag: %d Type: %d\n", tag, type);
			break;
		}
		length = ureadfour(input);	/* count */
/*		if (type == 0 || type > MAX_KNOWN_TYPE)  */
		if (type == 0 || type >= MaxType) {
			if (verboseflag) fprintf(errout, "\n");  /* ??? */
			fprintf(errout, "WARNING: Tag: %d Type: %d\n", tag, type);
			(void) ureadfour(input);	/* value */
			continue;
		}
		if (length == 1) {
			if (type == TYPE_LONG) offset = ureadfour(input);
			else if (type  == TYPE_SHORT) {
				offset = ureadtwo(input);	
				(void) ureadtwo(input);		/* should be zero */
			}
			else if (type == TYPE_BYTE) {
				offset = getc(input);
				(void) getc(input);(void) getc(input);(void) getc(input);
			}
			else offset = ureadfour(input);	/* for ratio e.g. */
		}
		else offset = ureadfour(input);	/* value */

		if (verboseflag) printf("TAG:%5u ", tag);
		s = lookupname(tag);
		if (verboseflag) printf ("(%s) ", s);
		n = strlen(s);
		if (verboseflag)  
			for (j = n; j < MAXTAGNAME; j++) putc(' ', stdout);
		if (verboseflag) printf ("TYPE:%2u ", type);
/*		if (type <= MAX_KNOWN_TYPE )  */
		if (type < MaxType) {
			if (verboseflag) printf("(%s) ", typename[type]);
		}
		else {
			if (verboseflag) printf("(%s) ", "UNKNOWN");
		}
		if (verboseflag) {
			printf("LENGTH:%3lu ", length);
			printf("OFFSET:%6lu", offset);
			printf("\n");
		}
/*		remember some noteworthy tags */
		if (tag == 259) compression = (unsigned int) offset;
		else if (tag == 262) photointerpretation = (unsigned int) offset;
		else if (tag == 296) resolutionunit = (unsigned int) offset;
		else if (tag == 256) imagewidth = (unsigned int) offset;
		else if (tag == 257) imagelength = (unsigned int) offset;
	}
}

void showascii(FILE *input, unsigned long ifdpos) {
	unsigned int j, k, n, tag, type;
	unsigned long length, offset;
	long present;
	int c;
	char *s;

	if(fseek(input, (long) (ifdpos + tiffoffset), SEEK_SET) != 0) {
		if (verboseflag) fprintf(errout, "\n");  /* ??? */
		fprintf(errout, "SEEK ERROR %lu\n", ifdpos);
		if (seekerrors++ > MAXSEEKERROR) exit(1);
		return;
	}

	ifdcount = ureadtwo(input);
	
	for (k = 0; k < ifdcount; k++) {
		tag = ureadtwo(input);
		type = ureadtwo(input);
		if (tag == 0 && type == 0) {
			if (seekerrors++ > MAXSEEKERROR) exit(1);
			break;
		}
		length = ureadfour(input);	/* count */
		offset = ureadfour(input);	/* value */
/*		if (type == 0 || type > MAX_KNOWN_TYPE ) */
		if (type == 0 || type >= MaxType) {
			continue;
		}
		if (type == TYPE_ASCII) {
			if (verboseflag) printf("TAG:%5u ", tag);
			s = lookupname(tag);
			if (verboseflag) printf ("(%s) ", s);
			n = strlen(s);
			if (verboseflag) 
				for (j = n; j < MAXTAGNAME; j++) putc(' ', stdout);
			if (offset != 0) {
				present = ftell(input);
				if (fseek(input, (long) (offset + tiffoffset), SEEK_SET) != 0) {
					if (verboseflag) fprintf(errout, "\n");  /* ??? */
					fprintf(errout, "SEEK ERROR %lu\n", offset);
					if (seekerrors++ > MAXSEEKERROR) exit(1);
					return;
				}
				for (j = 0; j < (unsigned int) length; j++)  {
					if ((c = getc(input)) == 0) break;				
					if (verboseflag) putc(c, stdout);
				}
				fseek(input, present, SEEK_SET);
			}
			else {
				if (verboseflag) printf("NULL");
			}
			if (verboseflag) printf("\n");
		}
	}
}

void showratio(FILE *input, unsigned long ifdpos) {
	unsigned int j, k, n, tag, type;
	unsigned long length, offset, num, den;
	long present;
	char *s;
	
	if (fseek(input, (long) (ifdpos + tiffoffset), SEEK_SET) != 0) {
		if (verboseflag) fprintf(errout, "\n");  /* ??? */
		fprintf(errout, "SEEK ERROR %lu\n", ifdpos);
		if (seekerrors++ > MAXSEEKERROR) exit(1);
		return;
	}

	ifdcount = ureadtwo(input);
	
	for (k = 0; k < ifdcount; k++) {
		tag = ureadtwo(input);
		type = ureadtwo(input);
		if (tag == 0 && type == 0) {
			if (seekerrors++ > MAXSEEKERROR) exit(1);
			break;
		}
		length = ureadfour(input);	/* count */
		offset = ureadfour(input);	/* value */
		if (type == 0 || type >= MaxType ) {
			continue;
		}

		if (type == TYPE_RATIO) {
			if (verboseflag) printf("TAG:%5u ", tag);
			s = lookupname(tag);
			if (verboseflag) printf ("(%s) ", s);
			n = strlen(s);
			if (verboseflag) 
				for (j = n; j < MAXTAGNAME; j++) putc(' ', stdout);
			present = ftell(input);
			if (fseek(input, (long) (offset + tiffoffset), SEEK_SET) != 0) {
				if (verboseflag) fprintf(errout, "\n");  /* ??? */
				fprintf(errout, "SEEK ERROR %lu\n", offset);
				if (seekerrors++ > MAXSEEKERROR) exit(1);
				return;
			}
			num = ureadfour(input);
			den = ureadfour(input);
			if (verboseflag) printf("( %lu / %lu )\n", num, den);
			if (tag == 282) {				/* XResolution */
				xresnum = num; xresden = den;
			}
			if (tag == 283) {				/* YResolution */
				yresnum = num; yresden = den;
			}
			fseek(input, present, SEEK_SET);
		}
	}
}

void showlong(FILE *input, unsigned long ifdpos) {
	unsigned int j, k, n, tag, type;
	unsigned long length, offset, num;
	long present;
	char *s;
	
	if(fseek(input, (long) (ifdpos + tiffoffset), SEEK_SET) != 0) {
		if (verboseflag) fprintf(errout, "\n");  /* ??? */
		fprintf(errout, "SEEK ERROR %lu\n", ifdpos);
		if (seekerrors++ > MAXSEEKERROR) exit(1);
		return;
	}
	ifdcount = ureadtwo(input);
	
	for (k = 0; k < ifdcount; k++) {
		tag = ureadtwo(input);
		type = ureadtwo(input);
		if (tag == 0 && type == 0) {
			if (seekerrors++ > MAXSEEKERROR) exit(1);
			break;
		}
		length = ureadfour(input);	/* count */
		offset = ureadfour(input);	/* value */
		if (type == 0 || type >= MaxType ) {
			continue;
		}

/* #define TYPE_BYTE 1 */
/* #define TYPE_ASCII 2 */
/* #define TYPE_SHORT 3 */
/* #define TYPE_LONG 4 */
/* #define TYPE_RATIO 5 */

		if (type == TYPE_LONG && length > 1) {
			if (verboseflag) printf("TAG:%5u ", tag);
			s = lookupname(tag);
			if (verboseflag) printf ("(%s) ", s);
			n = strlen(s);
			if (verboseflag) 
				for (j = n; j < MAXTAGNAME; j++) putc(' ', stdout);
			present = ftell(input);
			if (fseek(input, (long) (offset + tiffoffset), SEEK_SET) != 0) {
				if (verboseflag) fprintf(errout, "\n");  /* ??? */
				fprintf(errout, "SEEK ERROR %lu\n", offset);
				if (seekerrors++ > MAXSEEKERROR) exit(1);
				return;
			}
			for (j = 0; j < (unsigned int) length; j++) {
				num = ureadfour(input);
				if (verboseflag) printf("%lu ", num);
			}
			if (verboseflag) printf("\n");
			fseek(input, present, SEEK_SET);
		}
	}
}

void showshort(FILE *input, unsigned long ifdpos) {
	unsigned int j, k, n, tag, type, num;
	unsigned long length, offset;
	long present;
	char *s;
	
	if (fseek(input, (long) (ifdpos + tiffoffset), SEEK_SET) != 0) {
		if (verboseflag) fprintf(errout, "\n");  /* ??? */
		fprintf(errout, "SEEK ERROR %lu\n", ifdpos);
		if (seekerrors++ > MAXSEEKERROR) exit(1);
		return;
	}
	ifdcount = ureadtwo(input);
	
	for (k = 0; k < ifdcount; k++) {
		tag = ureadtwo(input);
		type = ureadtwo(input);
		if (tag == 0 && type == 0) {
			if (seekerrors++ > MAXSEEKERROR) exit(1);
			break;
		}
		length = ureadfour(input);	/* count */
		offset = ureadfour(input);	/* value */
		if (type == 0 || type >= MaxType ) {
			continue;
		}

/* #define TYPE_BYTE 1 */
/* #define TYPE_ASCII 2 */
/* #define TYPE_SHORT 3 */
/* #define TYPE_LONG 4 */
/* #define TYPE_RATIO 5 */

		if (type == TYPE_SHORT && length > 2) {
			if (verboseflag) printf("TAG:%5u ", tag);
			s = lookupname(tag);
			if (verboseflag) printf ("(%s) ", s);
			n = strlen(s);
			if (verboseflag) 
				for (j = n; j < MAXTAGNAME; j++) putc(' ', stdout);
			present = ftell(input);
			if (fseek(input, (long) (offset + tiffoffset), SEEK_SET) != 0) {
				if (verboseflag) fprintf(errout, "\n");  /* ??? */
				fprintf(errout, "SEEK ERROR %lu\n", offset);
				if (seekerrors++ > MAXSEEKERROR) exit(1);
				return;
			}
			for (j = 0; j < (unsigned int) length; j++) {
				num = ureadtwo(input);
				if (verboseflag) printf("%u ", num);
			}
			if (verboseflag) printf("\n");
			fseek(input, present, SEEK_SET);
		}
	}
}

void extension(char *fname, char *ext) { /* supply extension if none */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s, *t;
    if ((s = strrchr(fname, '.')) == NULL ||
		((t = strrchr(fname, '\\')) != NULL && s < t)) {
			strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}

/* return pointer to file name - minus path - returns pointer to filename */
char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

void analyzetiff(FILE *input, int c, int d) {
	int nifd;
	if (c == 'I' && d == 'I') {
		leastfirst = 1;
		if (verboseflag)
			printf("II form (least significant -> most significant)  ");
	}
	else if (c == 'M' && d == 'M') {
		if (verboseflag)
			printf("MM form (most significant -> least significant)  ");
		leastfirst = 0;
	}
	else {
/*		if (verboseflag) fprintf(errout, "\n"); */
		fprintf(errout,
			"ERROR: TIFF file starts with `%c%c' instead of `II' or `MM'\n", 
				c, d);
			exit(1);
	}
	version = ureadtwo(input);
	if (version != 42) {
/*		if (verboseflag) fprintf(errout, "\n"); */
		fprintf(errout,
			"ERROR: The TIFF `version number' is %u instead of 42\n", 
				version);
			exit(1);
	}
	if (verboseflag) printf("TIFF `version' %u\n", version);
	
	ifdposition = ureadfour(input); nifd = 1;
	
	for(;;) {
		
		if (verboseflag) printf("\n");	/* ??? */
		
/*			protect from missing zeros after last IFD ... NEW */
		
		if (fseek(input, (long) (ifdposition + tiffoffset), SEEK_SET) != 0) {
			fprintf(errout, "Seek to IFD failed\n");
			break;
		}
		
		if (tiffoffset == 0)  {
			if (verboseflag)
				printf("Image File Directory (IFD) # %d starts at %lu",
				nifd, ifdposition);
		}
		else {
			if (verboseflag)
				printf("Image File Directory (IFD) # %d starts at %lu (+ %ld) ",
				nifd, ifdposition, tiffoffset);
		}
		showfields(input, ifdposition);
		if (verboseflag) printf("\n");
				
		if (showasciiflag != 0) {
			showascii(input, ifdposition); /* if (verboseflag) printf("\n"); */
		}
		
		if (showratioflag != 0) {
			showratio(input, ifdposition); /* if (verboseflag) printf("\n"); */
		}
		
		if (showlongflag != 0) {
			showlong(input, ifdposition); /* if (verboseflag) printf("\n"); */
		}		
		if (showshortflag != 0) {
			showshort(input, ifdposition); /* if (verboseflag) printf("\n"); */
		}				
		
		ifdposition = ureadfour(input);
		if (ifdposition == 0) break;		/* NO MORE IFDs */
		if (ifdposition > filelength) {
			if (verboseflag) fprintf(errout, "\n");  /* ??? */
			fprintf(errout,
				"Giving up: supposed next IFD position (%lu) past end of file at (%lu)\n",
					ifdposition, filelength);
			break;	/* bugus IFD - bad file termination */
		}
		nifd++;
	}
}

void showusage (char *argv[]) {
	printf("Usage: %s [-v] <tiff-file-1> <tiff-file-2> ...\n", 
		argv[0]);
	printf("\n");
	printf("       -v verbose mode (show all TIFF TAGS)\n");
	printf("\n");
	printf("       default is to show only file name and image size\n");
	exit(1);
}

int main (int argc, char *argv[]) {
	FILE *input, *output;
	char infilename[FILENAME_MAX], outfilename[FILENAME_MAX];
	int c, d, e, f, m;
	/*	int k; */
	int firstarg=1;
	/*	int nifd; */
	int ignoretiff, epsffile;
	/*	unsigned long offset, length; */
	unsigned long k;
	unsigned int checksum;
	double width, height;
	double widthpt, heightpt;
	
	if (argc < 2) showusage(argv);
	
/*	while (argv[firstarg][0] == '-') { */
	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (strcmp(argv[firstarg], "-v") == 0) {
			verboseflag = 1;
		}
		if (strcmp(argv[firstarg], "-q") == 0) {
			verboseflag = 0;
		}
		if (strcmp(argv[firstarg], "-d") == 0) {
			detailflag = 0;
		}
		if (strcmp(argv[firstarg], "-t") == 0) {
			traceflag = 1;
		}
		if (strcmp(argv[firstarg], "-s") == 0) {
			splitflag = 1;
		}
		if (strcmp(argv[firstarg], "-?") == 0) {
			showusage(argv);
		}
		firstarg++;
	}

/*	printf("v %d d %d t %d\n", verboseflag, detailflag, traceflag); */

	if (argc < firstarg+1) showusage(argv);

	if (verboseflag) printf("%s\n\n", progversion);

	for (m = firstarg; m < argc; m++) {
		strcpy(infilename, argv[m]);
		extension(infilename, "tif");
	
		input = fopen(infilename, "rb");
		if (input == NULL) {
			strcpy(infilename, argv[m]);
			extension(infilename, "tiff");
			input = fopen(infilename, "rb");
			if (input == NULL) {
/*				perror(infilename); */
				perror(argv[m]);
/*				exit(1); */
				continue;
			}
		}
		fseek (input, 0, SEEK_END);			/* go to end */
		filelength = ftell(input);			/* read length */
		fseek (input, 0, SEEK_SET);			/* rewind */
		
		printf ("Analysis of file `%s' (%lu bytes)\n",
			infilename, filelength);
		if (verboseflag) printf("\n");
		tiffoffset = 0;
		ignoretiff = 0;
		epsffile = 0;
		c = getc(input); d = getc(input);
		if ((c == 'I' && d == 'I') || (c == 'M' && d == 'M')) {
/*			OK it is a plain old TIFF file - go on with it */
		}
		else {
			e = getc(input); f = getc(input);
			if (c == 'E'+128 && d == 'P'+128 && e == 'S'+128 && f == 'F'+128) {
/*				read over PS-start, PS-length, MF-start, MF-end */
/*				if (verboseflag) printf ("EPSF file with TIFF header:\n"); */
				if (verboseflag)
					printf ("EPSF file with TIFF or WMF preview:\n");
				epsffile = 1;
				leastfirst = 1;
/* skip over PS length, offset, MetaFile length, offset flushed 94/Mar/7 */
/*				for (k = 0; k < 4; k++) (void) ureadfour(input); */
				psoffset = ureadfour(input);
				pslength = ureadfour(input);
				if (pslength > 0) {
					if (verboseflag)
						printf("PS subfile at %ld of length %ld\n", 
						psoffset, pslength);
				}
				metaoffset = ureadfour(input);
				metalength = ureadfour(input);
				if (metalength > 0) {
					if (verboseflag)
						printf("MetaFile subfile at %ld of length %ld\n", 
						metaoffset, metalength);
				}
				tiffoffset = ureadfour(input);	/* read TIFF start offset */
				tifflength = ureadfour(input);	/* read TIFF length */
/*				if (tifflength > 0) */
				if (verboseflag) {
					if (tiffoffset != 0 || tifflength != 0)
						printf("TIFF subfile at %ld of length %ld\n", 
						   tiffoffset, tifflength);
				}
				checksum = ureadtwo(input);			/* should be 255, 255 */
													/* or checksum */
				if (checksum != (unsigned int) -1) {
					if (verboseflag) printf("CheckSum %0X\n", checksum);
				}
/*				Is there a TIFF subfile ? */
				if (tiffoffset == 0 || tifflength == 0) {
/*					fclose(input); */
					fprintf(errout, 
						"TIFF offset or length in EPSF file are zero\n");
					ignoretiff = 1;
/*					exit(1); */
				}
				else {
					if (fseek(input, (long) tiffoffset, SEEK_SET) != 0)  {
						fclose(input);
						fprintf(errout, "EPSF file seek error\n");
/*						exit(1); */
						continue;	 /* go on to next file ? */
					}
					c = getc(input); d = getc(input);
				}
			}
			else {
				fclose(input); 
				fprintf(errout, "Not a valid TIFF or EPSF file\n");
				fprintf(errout, "\n");
/*				exit(1); */
				continue;					 /* go on to next file ? */
			}
		}
	
		if (!ignoretiff) {
			compression = 0;
			photointerpretation = 0;
			resolutionunit = 2;					/* default */
			imagelength = 0; imagewidth = 0;	/* 95/July/1 */
			xresnum = 0; xresden = 0;			/* 95/July/1 */
			yresnum = 0; yresden = 0;			/* 95/July/1 */
/*			xresnum = 72; xresden = 1; */		/* default */
/*			yresnum = 72; yresden = 1; */		/* default */
/*			resolutionunit = 2;	*/				/* default */

			analyzetiff (input, c, d);
			if (detailflag) {
				if (verboseflag) putc('\n', stdout);
/*				if (compression == 32773) compression = 6; */
				if (compression == 32773) compression = 7;
				if (compression != 1) {
					if (compression < 7)  {
						if (verboseflag)
							printf("Compression:\t%s\n", compstring[compression]);
					}
					else {
						printf("Compression:\tUnknown %d\n", compression);
					}
				}
/*				0 WhiteIsZero -- for bilevel and gray scale */
/*					normal for compression=2 (CCITT 1D) */
/*				1 BlackIsZero -- for bilevel and gray scale */
/*					if specified for compression=2, show reversed */
/*				if (photointerpretation > 3) photointerpretation = 4; */
				if (photointerpretation > 8) photointerpretation = 9;
				if (photointerpretation != 1) {
					if (photointerpretation < 9) {
						if (verboseflag) printf("Photometrics:\t%s\n",
						   interpretationstring[photointerpretation]);
					}
					else {
						if (verboseflag)
							printf("Photometrics:\tUnknown %d\n",
								photointerpretation);
					}
				}
				if (imagelength != 0 && imagewidth != 0) {
				if (verboseflag) printf("\n");
/*				if (verboseflag)  */
				printf("Imagewidth (columns): %u\tImageheight (rows): %u\n",
						   imagewidth, imagelength);
				}
				if (resolutionunit > 3) resolutionunit = 0;
/* if resolution not specified, stick in defaults 1996/Mar/18 */
				if (xresnum == 0 && yresnum == 0 &&
					xresden == 0 && yresden == 0) {
/*					resolutionunit == 0  */
					xresnum = 72; xresden = 1;
					yresnum = 72; yresden = 1;
					resolutionunit = 2;
				}
/* do following only if resolution is specified */
				if (xresnum != 0 && yresnum != 0 &&
					xresden != 0 && yresden != 0) {
/*					if (verboseflag) printf("ResolutionUnit:\t%s\n",
						   resolutionstring[resolutionunit]); */
					if (imagewidth != 0 && imagelength != 0) {
						width = (double) imagewidth / xresnum * xresden;
						height = (double) imagelength / yresnum * yresden;
						if (resolutionunit == 2) {
							widthpt = width * PTSPERINCH;
							heightpt = height * PTSPERINCH;
						}
						else if (resolutionunit == 3) {
							widthpt = width / 2.54 * PTSPERINCH;
							heightpt = height / 2.54 * PTSPERINCH;;
						}
						else {
							widthpt = heightpt = 0;
						}
/*						putc('\n', stdout); */
						printf("Width: %lg %s (%lg pt)\t",
						   width, resolutionstring[resolutionunit], widthpt);
						printf("Height: %lg %s (%lg pt)\n",
						   height, resolutionstring[resolutionunit], heightpt);
/*						putc('\n', stdout);  */
					}
/*					putc('\n', stdout);  */
				}
				putc('\n', stdout); 
			}
		}

		if (splitflag && epsffile) {
			rewind(input);
			if (pslength > 0 && psoffset > 0) {
				if (verboseflag) printf("PS subfile at %ld of length %ld\n", 
					psoffset, pslength);
				strcpy (outfilename, stripname(infilename));
				forceexten(outfilename, "ps");
				if ((output = fopen(outfilename, "w")) != NULL) {
					fseek (input, psoffset, SEEK_SET);
					for (k = 0; k < pslength; k++) {
						c = getc(input);
	/*					printf("%d ", c); */
						if (c == EOF) break;
						putc(c, output);
					}
					fclose(output);
				}
				else perror (outfilename);
			}
			rewind(input);
			if (tifflength > 0 && tiffoffset > 0) {
				if (verboseflag) printf("TIFF subfile at %ld of length %ld\n", 
					tiffoffset, tifflength);
				strcpy (outfilename, stripname(infilename));
				forceexten(outfilename, "tif");
				if ((output = fopen(outfilename, "w")) != NULL) {
					fseek (input, tiffoffset, SEEK_SET);
					for (k = 0; k < tifflength; k++) {
					c = getc(input);
					if (c == EOF) break;
					putc(c, output);
					}
					fclose(output);
				}
				else perror (outfilename);
			}
		}
		fclose(input);
		if (verboseflag) putc('\n', stdout);
	}
	return 0;
}

/* NOTE: if offset is used directly for value */
/* then value is packed left justified */
/* so above is only `correct' if type is `II' - not if type is `MM' */

/* allow for 128 MacBinary header maybe ? */

/* Show `ColorMap' info in special form ? */

/* show `RowsPerStrip' in special form when (unsigned) -1 ? */
