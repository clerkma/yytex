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

/* compile as 32 bit */

/* This shows the tables in T1INSTAL.DLL (Windows NT) */

/* Following specific to Windows NT 4.0 T1INSTAL.DLL dated 8-09-96 */

/* #define PROCESSFILE */

#define MAXBADPOINTER 64

#define MAXBUFFER 150000

int showmatches=0;
int fixmismatches=1;
int showreplace=1;

int baseaddress=0x74A40000;
int maxt1instal=117520;
int maxfull=355;
int fulloffset=7792;
int maxase=149;
int aseoffset=12056;
int maxansi=256;
int ansioffset=92168;
int maxaccept=255;
int acceptoffset=106032;

int ansinames=4494;				/* start of strings for ANSI names */
int sftnbspace=6468;
int othernames=6500;			/* start of strings for other names */
int hungerumlaut=14748;

/* NT 4.0 version */

int baseaddress1=0x74A40000;
int maxt1instal1=117520;
int maxfull1=355;
int fulloffset1=7792;
int maxase1=149;
int aseoffset1=12056;
int maxansi1=256;
int ansioffset1=92168;
int maxaccept1=255;
int acceptoffset1=106032;

int ansinames1=4494;			/* start of strings for ANSI names */
int sftnbspace1=6468;
int othernames1=6500;			/* start of strings for other names */
int hungerumlaut1=14748;

/* NT 3.51 version */

int baseaddress0=0x76391400;
int maxt1instal0=115008;
int maxfull0=355;
int fulloffset0=1520;
int maxase0=149;
int aseoffset0=5784;
int maxansi0=256;
int ansioffset0=85000;
int maxaccept0=255;
int acceptoffset0=102616;

int ansinames0=86058;			/* start of strings for ANSI names */
int sftnbspace0=87928;
int othernames0=87960;			/* start of strings for other names */
int hungerumlaut0=103636;

typedef struct entry {
  unsigned int name;
  unsigned short unicode;
  short ansicode;
  short standardcode;
  short maccode;
} ENTRY;

char t1instal1[MAXBUFFER];

#ifdef PROCESSFILE
char t1instal0[MAXBUFFER];
#endif

char infile1[FILENAME_MAX];

#ifdef PROCESSFILE
char infile0[FILENAME_MAX];
char *outfile="t1instal.mod";
#endif

int traceflag=0;

char *sysdir1="c:\\winnt\\system32\\";

char *sysdir0="c:\\windows\\system32\\";

char *dllname="t1instal.dll";

int badpointer=0;		/* count bad string pointers */

int winnt40=0;			/* Windows NT 4.0 */

/* pointer to char name string, Ucode, Acode, Scode, Mcode */
/* pointer is four bytes, rest are 2 bytes */
/* Ucode is UNICODE, Acode is ANSI code, Scode is Standard, Mcode is Mac */

/* Table `All' 7792 - 12052 = 4260 bytes 4260 / 12 = 355 glyphs */

/* Table `ASE' 12056 - 13844 = 1788 bytes 1788 / 12 = 149 glyphs see below */

/* used with fulloffset and aseoffset */

void showfulltable (void *ptr, int n) {
	int i;
	ENTRY *full = (ENTRY *) ptr;

	printf("INX  UNIC ANSI  ASE  MAC  glyphname\n");
	printf("\n");

	for (i = 0; i < n; i++) {
		if (full[i].unicode == 0 && full[i].ansicode == 0 && 
			full[i].standardcode == 0 && full[i].maccode == 0) continue; 
		if (traceflag)
			printf("%3d %6d\t", i, ((char *) &full[i] - (char *) t1instal1));
		if (full[i].name - baseaddress > maxt1instal ||
			full[i].name - baseaddress < 0) {
			printf("%03d  %04X  %3d  %3d  %3d  BAD POINTER %04X\n", 
			   i, full[i].unicode, full[i].ansicode, 
			   full[i].standardcode, full[i].maccode,
			   full[i].name);
			if (badpointer++ > MAXBADPOINTER) {
				printf("ERROR: too many bad string pointers\n");
				exit(1);
			}
		}
		else printf("%03d  %04X  %3d  %3d  %3d  %s\n", 
			   i, full[i].unicode, full[i].ansicode, 
			   full[i].standardcode, full[i].maccode, 
			   (char *) &t1instal1 + (full[i].name - baseaddress)
			  );
	}
}

/* used with ansioffset and acceptoffset */

void showacceptable (void *ptr, int n) {
	int *accept = (int *) ptr;
	int i;
	char *s;

	printf("INX  glyphname\n");
	printf("\n");

	for (i = 0; i < n; i++) {
		if (accept[i] - baseaddress > maxt1instal ||
			accept[i] - baseaddress < 0) {
			printf("%03d  BAD POINTER %04X\n", i, accept[i]);
			if (badpointer++ > MAXBADPOINTER) {
				printf("ERROR: too many bad string pointers\n");
				exit(1);
			}
		}
		else {
			s = (char *) &t1instal1 + (accept[i] - baseaddress);
			if (*s == '\0') continue;
			if (traceflag)
				printf("%3d %6d\t", i, ((char *)&accept[i] - (char *) t1instal1));
/*			printf("%03d  %s\n", i, t1instal[accept[i]]); */
			printf("%03d  %s\n", i, s);
		}
	}
}

void asterisks(int flag) {
	if (!flag) printf("\n");
	printf("*************************************************************\n");
	if (flag) printf("\n");
}

int readfile(char *t1instal, char *infile) {
	FILE *input;
	int n, nlen;

	printf("Opening file %s\n", infile);

	if ((input = fopen(infile, "rb")) == NULL) {
		perror(infile);
		return -1;
	}
	fseek(input, 0, SEEK_END);
	nlen = ftell(input);
	printf("File %s has %d bytes\n", infile, nlen);
	fseek(input, 0, SEEK_SET);	
/*	n = fread(&t1instal, 1, sizeof(t1instal), input); */
	n = fread(t1instal, 1, MAXBUFFER, input);
	if (n != maxt1instal1 && n != maxt1instal0) {
		printf ("Read %d bytes, not %d or %d\n", n, maxt1instal1, maxt1instal0);
/*		printf("Sorry, I don't understand this version of DLL\n"); */
		exit(1);
	}
	if (ferror(input)) {
		printf("Read Error\n");
		perror(infile);
	}
	if (feof(input)) printf("EOF\n");
	fclose(input);

	printf("Closing file %s\n", infile);
	return nlen;
}

int writefile(char *t1instal, char *outfile, int nlen) {
	FILE *output;
	int n;

	printf("Opening file %s\n", outfile);

	if ((output = fopen(outfile, "wb")) == NULL) {
		perror(outfile);
		return -1;
	}
	n = fwrite(t1instal, 1, nlen, output);
	if (n != nlen) {
		printf ("Wrote %d bytes, not %d\n", n, nlen);
		exit(1);
	}
	if (ferror(output)) {
		printf("Write Error\n");
		perror(outfile);
	}

	fclose(output);

	printf("Closing file %s\n", outfile);
	return nlen;
}

void setupoffsets (int nlen) {
	if (nlen == maxt1instal1) {	/* 117520 */
		winnt40=1;
		printf("Windows NT 4.0 version of DLL (V1.0d)\n");
		baseaddress=baseaddress1;	/* 0x74A40000; */
		maxt1instal=maxt1instal1;	/* 117520; */
		maxfull=maxfull1;	/* 355; */
		fulloffset=fulloffset1;	/* 7792; */
		maxase=maxase1;	/* 149; */
		aseoffset=aseoffset1;	/* 12056; */
		maxansi=maxansi1;	/* 256; */
		ansioffset=ansioffset1;	/* 92168; */
		maxaccept=maxaccept1;	/* 255; */
		acceptoffset=acceptoffset1;	/* 106032; */
		ansinames=ansinames1;	/* 4494 */
		othernames=othernames1;	/* 6468 */
		sftnbspace=sftnbspace1;
		hungerumlaut=hungerumlaut1;
	}
	else if (nlen == maxt1instal0) {	/* 115008 */
		winnt40=0;
		printf("Windows NT 3.51 version of DLL (V1.0d) \n");
		baseaddress=baseaddress0;	/* 0x74A40000; */
		maxt1instal=maxt1instal0;	/* 117520; */
		maxfull=maxfull0;	/* 355; */
		fulloffset=fulloffset0;	/* 7792; */
		maxase=maxase0;	/* 149; */
		aseoffset=aseoffset0;	/* 12056; */
		maxansi=maxansi0;	/* 256; */
		ansioffset=ansioffset0;	/* 92168; */
		maxaccept=maxaccept0;	/* 255; */
		acceptoffset=acceptoffset0;	/* 106032; */
		ansinames=ansinames0;	/* 86058 */
		othernames=othernames0;	/* 87928 */
		sftnbspace=sftnbspace0;
		hungerumlaut=hungerumlaut0;
	}
	else {
		printf("Sorry, I don't understand this version of DLL\n");
		exit(1);
	}
}

#ifdef PROCESSFILE
/* translate pointer from address in 4.0 DLL to that in 3.51 DLL */

int translateptr (int ptr) {
	int offsetold, offsetnew;
	offsetnew = (ptr - baseaddress1);
	if (offsetnew >= hungerumlaut1)
		offsetnew = offsetnew - hungerumlaut1 + hungerumlaut0;
	else if (offsetnew >= othernames1)
		offsetnew = offsetnew - othernames1 + othernames0;
	else if (offsetnew >= sftnbspace1)
		offsetnew = offsetnew - sftnbspace1 + sftnbspace0;
	else if (offsetnew >= ansinames1)
		offsetnew = offsetnew - ansinames1 + ansinames0;
	return (offsetnew + baseaddress0);
}
#endif

#ifdef PROCESSFILE
int comparedlls(void) {
	int k;
	ENTRY *full1 = (ENTRY *) ((char *) &t1instal1 + fulloffset1);
	ENTRY *full0 = (ENTRY *) ((char *) &t1instal0 + fulloffset0);
	ENTRY *ase1 = (ENTRY *) ((char *) &t1instal1 + aseoffset1);
	ENTRY *ase0 = (ENTRY *) ((char *) &t1instal0 + aseoffset0);
	int *ansi1 = (int *) ((char *) &t1instal1 + ansioffset1);
	int *ansi0 = (int *) ((char *) &t1instal0 + ansioffset0);
	int *accept1 = (int *) ((char *) &t1instal1 + acceptoffset1);
	int *accept0 = (int *) ((char *) &t1instal0 + acceptoffset0);
	int n1, n0;
	int matches, fixes, failures, smashed;
	int flag=0;
	int differences=0;
	char *s1, *s0;

	asterisks(0);
	printf("FULL TABLE\n");
	asterisks(1);
	matches=fixes=failures=smashed=0;
	for (k=0; k < maxfull; k++) {	/* look at full table first */
		n1 = (full1[k].name - baseaddress1);
		if (n1 > maxt1instal1 || n1 < 0) continue;
		s1 = (char *) &t1instal1 + n1;
		n0 = (full0[k].name - baseaddress0);
		if (n0 > maxt1instal0 || n0 < 0) continue;
		s0 = (char *) &t1instal0 + n0;
		if (strcmp(s0, s1) == 0) {
			if (showmatches) printf("MATCH:      %3d %s %s\n", k, s1, s0);
			matches++;
		}
		else {
			differences++;
			printf("DIFFERENCE:   %3d %s %s\n", k, s1, s0);
/*			attempt to fix this up */
			full0[k].name =	translateptr (full1[k].name);
			n0 = (full0[k].name - baseaddress0);
			if (n0 > maxt1instal0 || n0 < 0) {
				printf("FAILED TO ADJUST %3d ****************\n", k);
				failures++;
			}
			s0 = (char *) &t1instal0 + n0;
			if (strcmp(s0, s1) == 0) {
				printf("FIXED:      %3d %s %s\n", k, s1, s0);
				fixes++;
			}
			else {
				printf("STILL DIFFERENT: %3d %s %s *********\n", k, s1, s0);
				if (fixmismatches) {
					n1 = strlen(s1);
					n0 = strlen(s0);
					if (n1 <= n0 ||
						(n1 == n0+1 && *(s0+n1) == '\0')) {
/* deals with ffi on top of fi and triaglf on top of traglf */
						if (showreplace)
							printf("WRITING:      %3d %s over %s\n", k, s1, s0);
						strcpy(s0, s1);
						smashed++;
					}
					else {
						printf("WILL NOT FIT: %3d OLD %s NEW %s\n", k, s1, s0);
						failures++;
					}
				}
				else failures++;
			}
		}
		if (full0[k].unicode != full1[k].unicode ||
			full0[k].ansicode != full1[k].ansicode ||
			full0[k].standardcode != full1[k].standardcode ||
			full0[k].maccode != full1[k].maccode) {
			differences++;
			printf("CODE mismatch %3d %04X %3d %3d %3d\n",
				   k, full0[k].unicode, full0[k].ansicode, 
				   full0[k].standardcode, full0[k].maccode);
			printf("              %3d %04X %3d %3d %3d\n",				   
				   k, full1[k].unicode, full1[k].ansicode,
				   full1[k].standardcode, full1[k].maccode);
			full0[k].unicode = full1[k].unicode;
			full0[k].ansicode = full1[k].ansicode;
			full0[k].standardcode = full1[k].standardcode;
			full0[k].maccode = full1[k].maccode;
		}
	}
	printf("%d matched, %d fixed, %d smashed, %d failed\n",
		   matches, fixes, smashed, failures);
	if (failures > 0) {
		printf("%d FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", failures);
		flag++;
	}

	asterisks(0);
	printf("ASE TABLE\n");
	asterisks(1);
	matches=fixes=failures=0;
	for (k=0; k < maxase; k++) {	/* look at full table first */
		n1 = (ase1[k].name - baseaddress1);
		if (n1 > maxt1instal1 || n1 < 0) continue;
		s1 = (char *) &t1instal1 + n1;
		n0 = (ase0[k].name - baseaddress0);
		if (n0 > maxt1instal0 || n0 < 0) continue;
		s0 = (char *) &t1instal0 + n0;
		if (strcmp(s0, s1) == 0) {
			if (showmatches) printf("MATCH:      %d %s %s\n", k, s1, s0);
			matches++;
		}
		else {
			differences++;
			printf("DIFFERENCE: %d %s %s\n", k, s1, s0);
/*			attempt to fix this up */
			ase0[k].name =	translateptr (ase1[k].name);
			n0 = (ase0[k].name - baseaddress0);
			if (n0 > maxt1instal0 || n0 < 0) {
				printf("FAILED TO ADJUST %d ****************\n", k);
				failures++;
			}
			s0 = (char *) &t1instal0 + n0;
			if (strcmp(s0, s1) == 0) {
				printf("FIXED:      %d %s %s\n", k, s1, s0);
				fixes++;
			}
			else {
				printf("STILL DIFFERENT: %d %s %s *********\n", k, s1, s0);
				failures++;
			}
		}
		if (ase0[k].unicode != ase1[k].unicode ||
			ase0[k].ansicode != ase1[k].ansicode ||
			ase0[k].standardcode != ase1[k].standardcode ||
			ase0[k].maccode != ase1[k].maccode) {
			differences++;
			printf("CODE mismatch %3d %04X %3d %3d %3d\n",
				   k, ase0[k].unicode, ase0[k].ansicode, 
				   ase0[k].standardcode, ase0[k].maccode);
			printf("              %3d %04X %3d %3d %3d\n",				   
				   k, ase1[k].unicode, ase1[k].ansicode,
				   ase1[k].standardcode, ase1[k].maccode);
			ase0[k].unicode = ase1[k].unicode;
			ase0[k].ansicode = ase1[k].ansicode;
			ase0[k].standardcode = ase1[k].standardcode;
			ase0[k].maccode = ase1[k].maccode;
		}
	}
	printf("%d matched %d fixed %d failed\n", matches, fixes, failures);
	if (failures > 0) {
		printf("%d FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", failures);
		flag++;
	}

	asterisks(0);
	printf("ANSI TABLE\n");
	asterisks(1);
	matches=fixes=failures=0;
	for (k=0; k < maxansi; k++) {	/* look at full table first */
		n1 = (ansi1[k] - baseaddress1);
		if (n1 > maxt1instal1 || n1 < 0) continue;
		s1 = (char *) &t1instal1 + n1;
		n0 = (ansi0[k] - baseaddress0);
		if (n0 > maxt1instal0 || n0 < 0) continue;
		s0 = (char *) &t1instal0 + n0;
		if (strcmp(s0, s1) == 0) {
			if (showmatches) printf("MATCH:      %d %s %s\n", k, s1, s0);
			matches++;
		}
		else {
			differences++;
			printf("DIFFERENCE: %d %s %s\n", k, s1, s0);
/*			attempt to fix this up */
			ansi0[k] = translateptr (ansi1[k]);
			n0 = (ansi0[k] - baseaddress0);
			if (n0 > maxt1instal0 || n0 < 0) {
				printf("FAILED TO ADJUST %d ****************\n", k);
				failures++;
			}
			s0 = (char *) &t1instal0 + n0;
			if (strcmp(s0, s1) == 0) {
				printf("FIXED:      %d %s %s\n", k, s1, s0);
				fixes++;
			}
			else {
				printf("STILL DIFFERENT: %d %s %s *********\n", k, s1, s0);
				failures++;
			}
		}
	}
	printf("%d matched %d fixed %d failed\n", matches, fixes, failures);
	if (failures > 0) {
		printf("%d FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", failures);
		flag++;
	}

	asterisks(0);
	printf("ACCEPT TABLE\n");
	asterisks(1);
	matches=fixes=failures=0;
	for (k=0; k < maxaccept; k++) {	/* look at full table first */
		n1 = (accept1[k] - baseaddress1);
		if (n1 > maxt1instal1 || n1 < 0) continue;
		s1 = (char *) &t1instal1 + n1;
		n0 = (accept0[k] - baseaddress0);
		if (n0 > maxt1instal0 || n0 < 0) continue;
		s0 = (char *) &t1instal0 + n0;
		if (strcmp(s0, s1) == 0) {
			if (showmatches) printf("MATCH:      %d %s %s\n", k, s1, s0);
			matches++;
		}
		else {
			differences++;
			printf("DIFFERENCE: %d %s %s\n", k, s1, s0);
/*			attempt to fix this up */
			accept0[k] = translateptr (accept1[k]);
			n0 = (accept0[k] - baseaddress0);
			if (n0 > maxt1instal0 || n0 < 0) {
				printf("FAILED TO ADJUST %d ****************\n", k);
				failures++;
			}
			s0 = (char *) &t1instal0 + n0;
			if (strcmp(s0, s1) == 0) {
				printf("FIXED:      %d %s %s\n", k, s1, s0);
				fixes++;
			}
			else {
				printf("STILL DIFFERENT: %d %s %s *********\n", k, s1, s0);
				failures++;
			}
		}
	}
	printf("%d matched %d fixed %d failed\n", matches, fixes, failures);
	if (failures > 0) {
		printf("%d FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", failures);
		flag++;
	}
	if (differences == 0) {
		printf("FILES COMPARE OK - NO DIFFERENCES NOTED\n");
		flag++;		/* don't bother writing it then */
	}

	return flag;
}
#endif

void showusage(void) {
	printf("Usage: showtabl c:\\winnt\\system32\\t1instal.dll\n");
	printf("       showtabl c:\\windows\\system32\\t1instal.dll\n");
	printf("       showtabl (use default DLL location)\n");
	printf("       showtabl | more (pause screen full)\n");
	printf("       showtabl > showtabl.log (write to log file)\n");
	exit(1);
}

/* showtabl + NT-4.0-DLL NT-3.51-DLL to copy fixups from NT 4.0 version */

int main (int argc, char *argv[]) {
/*	FILE *input, *output; */
/*	int i, n; */
	int nlen;
	int modflag=0;
	int defaultflag=0;
	int firstarg=1;
	char *sysroot, *windir;
	char str[FILENAME_MAX];

/*	if (argc == 1) showusage(); */

	if (argc > firstarg) {
		if ((strcmp(argv[firstarg], "-?") == 0) ||
			(strcmp(argv[firstarg], "?") == 0))
			showusage();
		if (strcmp(argv[firstarg], "-t") == 0) {
			traceflag = 1;
			firstarg++;
		}
	}

#ifdef PROCESSFILE
/*	+ NT-4.0-DLL NT-3.51-DLL */
	if (strcmp(argv[firstarg], "+") == 0) {
		if (argc == 4) {
			modflag++;
			firstarg++;
		}
		else exit(1);
	}
#endif

	if (argc == firstarg) {
		defaultflag++;
		firstarg++;
	}
	if ((sysroot = getenv("SystemRoot")) != NULL) {
		printf("SystemRoot=%s\n", sysroot);
		sprintf(str, "%s\\%s\\", sysroot, "system32");
		sysdir1 = strdup(str);
	}
	else if ((windir = getenv("windir")) != NULL) {
		printf("windir=%s\n", windir);
		sprintf(str, "%s\\%s\\", windir, "system32");
		sysdir1 = strdup(str);
	}
	if (defaultflag) {
		strcpy(infile1, sysdir1);
		strcat(infile1, dllname);
	}
	else {
		strcpy(infile1, argv[firstarg]);
		firstarg++;
	}

	nlen = readfile (t1instal1, infile1);
	if (nlen < 0 && defaultflag) {
		strcpy(infile1, sysdir0);
		strcat(infile1, dllname);
		nlen = readfile (t1instal1, infile1);
	}
	if (nlen < 0) exit(1);
	setupoffsets(nlen);

#ifdef PROCESSFILE
	if (modflag) {
		if (nlen != maxt1instal1) {
			printf("The NT 4.0 version should be listed first\n");
			exit(1);
		}
		strcpy(infile0, argv[firstarg]);
		nlen = readfile(t1instal0, infile0);
		if (nlen < 0) exit(1);
		setupoffsets(nlen);
		if (nlen != maxt1instal0) {
			printf("The NT 3.51 version should be listed second\n");
			exit(1);
		}
		if (comparedlls()) {
			printf("Processing failed.  Not writing output\n");
			exit(1);
		}
		else {
			printf ("Processing completed. Writing %s\n", outfile);
			writefile(t1instal0, outfile, maxt1instal0);
			return 0;
		}
	}
#endif

	asterisks(0);
	printf(
   "Show FULL table (i.e. all glyphs recognized by DLL) alphabetical order\n");
	asterisks(1);

	showfulltable((char *)&t1instal1+fulloffset, maxfull);/* 92168 - 93192 ? */

	asterisks(0);
	printf(
	"Show ASE table (i.e.glyphs in Adobe Standard Encoding) alphabetical order\n");
	asterisks(1);

	showfulltable((char *)&t1instal1+aseoffset, maxase);/* 12056 - 13844 ? */

	asterisks(0);
	printf(
	"Show ANSI table (i.e. Windows ANSI encoding vector) numerical order\n");
	asterisks(1);

	showacceptable((char *)&t1instal1+ansioffset, maxansi);/* 92168 - 93192 ? */

	asterisks(0);
	printf(
"Show ACCEPT table (i.e. all glyphs translated by DLL) alphabetical order\n");
	asterisks(1);

	showacceptable((char *)&t1instal1+acceptoffset, maxaccept);/* 105904 - 106032 ? */

	if (badpointer > 0) {
		printf("ERROR: encountered bad string pointers\n");
		printf("ERROR: this program is not designed for this version of T1instal\n");
		exit(1);
	}
	return 0;
}

/* 130  quotesinglbase	201A */
/* 131  florin			0192 */
/* 132  quotedblbase	201E */
/* 133  ellipsis		2026 */
/* 134  dagger			2020 */
/* 135  daggerdbl		2021 */
/* 136  circumflex		02C6 */
/* 137  perthousand		2030 */
/* 138  Scaron			0160 */
/* 139  guilsinglleft	2039 */
/* 140  OE				0152 */

/* 145  quoteleft		2018 */
/* 146  quoteright		2019 */
/* 147  quotedblleft	201C */
/* 148  quotedblright	201D */
/* 149  bullet			2022 */
/* 150  endash			2013 */
/* 151  emdash			2014 */
/* 152  tilde			02DC */
/* 153  trademark		2122 */
/* 154  scaron			0161 */
/* 155  guilsinglright	203A */
/* 156  oe				0153 */

/* 159  Ydieresis		0178 */

/* ANSI table has some weirdnesses */
/* 160 (A0) should be nbspace (A0), not space (0020) */
/* 173 (AD) should be sfthypen (AD), not minus (2212) */
/* 183 (B7) should be periocentered / middot (B7) not bullet operator (2219) */


/* Also change IsFixedPitch to IsFaxPitch and /isFixedPitch to /isFaxedPitch */
/* To prevent fixed-pitch non-text fonts from losing FF_DECORATIVE */
