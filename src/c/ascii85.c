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
#include <malloc.h>

/* Crude code for expanding ASCII85 code in Adobe Acrobat PDF files */

/* Very Crude Code for expanding LZW code in Adobe Acrobat PDF files */

#define MAXFILENAME 128

/* #define MAXLINE 128 */
#define MAXLINE 256				/* for /CharSet lines */

int verboseflag = 0;
int traceflag = 0;
int usefgets = 0;				/* use fgets (assumes \n is terminator) */
int psfile = 0;					/* process <~ ... ~> in PS file */
int convertreturn = 1;			/* convert return to newline */
int reversefree = 0;			/* free memory in reverse order */
int useown = 1;					/* use our own memory allocation */
int sfntflag = 0;				/* strings have extra zero byte padding */
int showEOD = 0;				/* show end <~ abd ~> in output file */

int ascii85flag = 0;			/* ASCII85 filter specified for stream */
int lzwflag = 0;				/* LZW filter specified for stream */
int dctflag = 0;				/* DCT filter specified for stream */
int ccittflag = 0;				/* CCITTFax filter specified for stream */

unsigned long total=0;			/* total space allocated */

char InLine[MAXLINE];			/* input buffer */

unsigned char OutLine[MAXLINE];		/* output from ASCII85 decoding */

FILE *errout = stdout;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXTABLE 4096

#define MAXSTRING 2048

#define MAXCHR 256

#define CLEAR 256

#define EOD 257

#define FIRST 258

char *StringTable[MAXTABLE];	/* pointers to char strings */

int TableIndex=FIRST;

char SingleByte [MAXCHR][2];	/* single byte strings */

int CodeLength=9;

int OmegaIndex=0;				/* pointer into `prefix' string */

char Omega[MAXSTRING];			/* `prefix' string */

/* Typically need about 20000 bytes of string memory */

#define MAXMEMORY 44000L

int FreeIndex=0;				/* next available byte in following */

char Memory[MAXMEMORY];			/* our own memory allocation */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Our own version of fgets - responds to ^M and ^J as line terminators */
/* returns NULL upon end of file */

char *GetLine(char *buffer, int nmax, FILE *input) {
	char *s=buffer;
	int c, n=1;

	if (usefgets) return fgets(buffer, nmax, input);

	while ((c = getc(input)) != EOF && c != '\n' && c != '\r') {
		*s++ = (char) c;
		if (n++ >= nmax-1) {
			*s = '\0';
			fprintf(errout, "LONG LINE: %s", buffer);
			while ((c = getc(input)) != EOF && c != '\n' && c != '\r') {
				putc(c, errout);
			}
			return buffer;
/*			break; */
		}
	}
/*	if (c != EOF) *s++ = (char) c; */
	if (c == '\r') {
		c = getc(input);					/* is what follows \r a \n ? */
		if (c == '\n') *s++ = (char) '\r';	/* if so, include that in line */
		else {								/* if not, do not use next char */
			ungetc(c, input);
			if (convertreturn) c = '\n';
			else c = '\r';
		}
	}
	if (c != EOF) *s++ = (char) c;
	*s = '\0';
	if (c == EOF) return NULL;
	else return buffer;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

void InitializeStringTable (void) {
	int k;

	total = 0;
	for (k = 0; k < MAXCHR; k++) {
		SingleByte[k][0] = (char) k;
		SingleByte[k][1] = '\0';
	}
	for (k = 0; k < MAXCHR; k++) StringTable[k] = SingleByte[k];
	for (k = MAXCHR; k < MAXTABLE; k++) StringTable[k] = NULL;
	TableIndex = FIRST;
	CodeLength = 9;
}

void ResetStringTable (int quietflag) {
	int k;

	if (!quietflag) {
		if (verboseflag) printf("CLEAR ");
		if (traceflag) printf("\n");
	}
	if (total > 0) {
/*		if (verboseflag) printf("Freeing %lu bytes ", total); */
		if (traceflag) printf("Freeing %lu bytes ", total);
		else if (verboseflag) printf("%lu ", total);
/*		total = 0; */
	}

/*	for (k = 0; k < MAXCHR; k++) {
		SingleByte[k][0] = (char) k;
		SingleByte[k][1] = '\0';
	} */
/*	for (k = 0; k < MAXCHR; k++) StringTable[k] = SingleByte[k]; */
/*	for (k = MAXCHR; k < MAXTABLE; k++) { */
	if (reversefree) {
		for (k = TableIndex-1; k >= FIRST; k--) {
			total -= strlen(StringTable[k]) + 1;
			if (!useown)
/*				if (StringTable[k] != NULL) _ffree(StringTable[k]); */
				if (StringTable[k] != NULL) free(StringTable[k]); 
			StringTable[k] = NULL;
		}
	}
	else {
		for (k = FIRST; k < TableIndex; k++) {
			total -= strlen(StringTable[k]) + 1;
			if (!useown)
/*				if (StringTable[k] != NULL) _ffree(StringTable[k]); */
				if (StringTable[k] != NULL) free(StringTable[k]); 
			StringTable[k] = NULL;
		}
	}
	if (total != 0) {
		fprintf(errout, "%lu bytes left! ", total);
		total = 0;
	}
	if (useown) FreeIndex = 0;		/* reset pointer */
	TableIndex = FIRST;
	CodeLength = 9;
}

void AddTableEntry(char *new) {
	char *s;
	int len;
	if (traceflag) printf("ADD %d `%s' ", TableIndex, new);
/*	if ((TableIndex & (TableIndex - 1)) == 0) {
		if (verboseflag) printf("LENGTH++ %d ", TableIndex);
		CodeLength++;
	}  */
/*	s = _strdup(new); */
	len = strlen(new)+1;
	if (useown) {
		if (FreeIndex + len > MAXMEMORY) s = NULL;
		else {
			s = Memory + FreeIndex;
			FreeIndex += len;
		}
	}
	else s = (char *) malloc(len);
	if (s == NULL) fprintf(errout, "M %lu (%d) ", total, len);
	else {
		strcpy(s, new);
		total += len;
	}

	StringTable[TableIndex++] = s;
/*	if ((TableIndex & (TableIndex - 1)) == 0) { */
/*	if (TableIndex == 512 || TableIndex == 1024 || TableIndex == 2048) {  */
/*	if ((TableIndex & (TableIndex + 1)) == 0) {  */
	if (TableIndex == 511 || TableIndex == 1023 || TableIndex == 2047) {  
		CodeLength++;
		if (traceflag)
			printf("LENGTH %d (%d) ", TableIndex, CodeLength);
	} 
/*	if (TableIndex == 4096) { */
	if (TableIndex > 4096) {
		fprintf(errout, "Table overflow\n");
		exit(1); 
	}
} 

int IsInTable(int Code) {
	return (Code >= 0 && Code < TableIndex);
}

int CodeFromString(char *test) {
	int k;
	for (k = 0; k < TableIndex; k++) {
		if (strcmp(StringTable[k], test) == 0) return k; /* in table */
	}
	return -1;	/* not in table */
}

int StringInTable(char *test) {
	int k;
	for (k = 0; k < TableIndex; k++) {
		if (strcmp(StringTable[k], test) == 0) return 1; /* in table */
	}
	return 0;	/* not in table */
}

char *StringFromCode (int k) {
	return StringTable[k];
}

void ConcatOneChar (char *Base, char *Exten) {
	char Temp[2];
	Temp[0] = Exten[0];
	Temp[1] = '\0';
	strcat (Base, Temp);
}

void WriteCode (int c, FILE *output) {
	putc (c, output);
}

/* Assumes strings do not contain null bytes ! */

void WriteString (char *s, FILE *output) {
	fputs (s, output);
}

int DecodeLine (unsigned char *outline, char *inline);

int ByteIndex = 0, BytesLeft=0;

/* gets next byte from ASCII85 encoded input */

/* unsigned int GetNextByte (FILE *input) { */
int GetNextByte (FILE *input) {
	int k;
	if (BytesLeft-- > 0) return OutLine[ByteIndex++];
	if (GetLine(InLine, MAXLINE, input) != NULL) {
		if (strlen(InLine) >= MAXLINE - 1)
			fprintf(errout, "Line too long\n");
		if (traceflag) fputs(InLine, stdout);
		if (strncmp(InLine, "endstream", 9) == 0) {
			fprintf(errout, "Hit ENDSTREAM \n");
			return -1;				/* emergency exit */
		}
	}
	else {
		fprintf(errout, "Unexpected EOF (GetNextByte)\n");
		exit(1);
	}
	BytesLeft = DecodeLine(OutLine, InLine);
	if (traceflag) printf("Decoded %d bytes\n", BytesLeft);
	if (traceflag) {
		for (k = 0; k < BytesLeft; k++) printf("%02X", OutLine[k]);
	}
	ByteIndex = 0;
	if (BytesLeft-- > 0) return OutLine[ByteIndex++];
	fprintf(errout, "No ASCII85 data\n");
	exit(1);
}

int BitsLeft=0;
unsigned long OldByte=0;

/* int lastline; */		/* debugging, on when ~> has been seen */

int GetNextCode (FILE *input) {
	int bits;
/*	unsigned int c; */
	int c;
	unsigned long k;

	bits = BitsLeft;					/* how many bits do we have */
	k = OldByte;						/* start with old bits */
	
	while (bits < CodeLength) {
/*		if ((c = getc(input)) == EOF) { 
			fprintf(errout, "Unexpected EOF (GetNextCode)\n");
			exit(1);
		} */
		c = GetNextByte(input);
		if (c == -1) {
			fprintf(errout, "Emergency Escape\n");
			return -1;		/* emergency escape */
		}
		k = (k << 8) | c;
		bits += 8;
	}
	OldByte = k;
	BitsLeft = bits - CodeLength;		/* extra bits not used */
/*	OldByte = OldByte & ((1 << BitsLeft) - 1); */ /* redundant */
	k = k >> BitsLeft;					/* shift out extra bits */
	k = k & ((1 << CodeLength) - 1);	/* mask out high order */
	if (traceflag) printf("CODE %d ", k);
/*	else if (lastline && verboseflag) printf("CODE %d ", k); */
	return (int) k;
}

void LZWcompress(FILE *output, FILE *input) {
	int c;

/*	InitializeStringTable(); */
	ResetStringTable(1);
	WriteCode(CLEAR, output);
	OmegaIndex = 0;
	Omega[0] = '\0';
	while ((c = getc(input)) != EOF) {
		Omega[OmegaIndex++] = (char) c;
		Omega[OmegaIndex] = '\0';
		if (StringInTable(Omega) >= 0) {
			Omega[OmegaIndex-1] = '\0';			/* strip new char again */
			WriteCode(CodeFromString(Omega), output);
			Omega[OmegaIndex-1] = (char) c;		/* concat new char again */
			AddTableEntry(Omega);
			if (TableIndex == 4093) {
				WriteCode (CLEAR, output);
				ResetStringTable(0);
			}
			OmegaIndex = 0;
			Omega[OmegaIndex++] = (char) c;
			Omega[OmegaIndex] = '\0';
		}
	}
	WriteCode (CodeFromString(Omega), output);
	WriteCode (EOD, output);
}

void LZWdecompress (FILE *output, FILE *input) {
	int Code, OldCode;
	
	OldCode = 0;						/* to keep compiler happy */
/*	InitializeStringTable(); */
/*	ResetStringTable(1); */
	while ((Code = GetNextCode (input)) != EOD) {
		if (Code == -1) {
			fprintf(errout, "Premature end of ZLW\n");
			return;
		}
		if (traceflag) printf("%d ", Code);			/* debugging */
		if (Code == CLEAR) {
			ResetStringTable(0);
			Code = GetNextCode (input);
			if (Code == -1) {
				fprintf(errout, "Premature end of ZLW (after CLEAR)\n");
				return;
			}
			if (Code == EOD) break;
			WriteString (StringFromCode(Code), output);
			OldCode = Code;
		}								/* end of CLEAR case */
		else {
			if (IsInTable(Code)) {
				WriteString(StringFromCode(Code), output);
/*				AddTableEntry(StringTable(OldCode)
					+ FirstChar(StringFromCode(Code);)); */
				strcpy(Omega, StringFromCode(OldCode));
				ConcatOneChar(Omega, StringFromCode(Code));
				AddTableEntry(Omega);
				OldCode = Code;
			} /* end of Code in Table case */
			else { /* Code is *not* in table */
/*				OutString = StringFromCode (OldCode) +
					+ FirstChar(StringFromCode(Code);));  */
				if (Code > TableIndex) {
					fprintf(errout, "Code (%d) > TableIndex (%d) ",
						Code, TableIndex);
				}
				strcpy(Omega, StringFromCode(OldCode));
				ConcatOneChar(Omega, StringFromCode(OldCode));
				WriteString(Omega, output);
				AddTableEntry(Omega);
				OldCode = Code;
			} /* end of Code *not* in Table case */
		} /* end of *not* CLEAR case */
	} /* end of not EOD loop */
	if (verboseflag) printf("EOD ");
/*	reset table for next one */
	ResetStringTable(1);
/*	flush out last line if not already at `endstream' */
	if (strncmp(InLine, "endstream", 9) != 0)
		GetLine(InLine, MAXLINE, input);
}

void DecodeLZW (FILE *output, FILE *input) {
	OldByte = 0;	BitsLeft = 0;
	ByteIndex = 0;	BytesLeft = 0;
	LZWdecompress (output, input);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* make unsigned long integer from five ASCII characters */

int asciitolong (unsigned long *k, int a, int b, int c, int d, int e) {
	if (a < 33 || a > 117 || b < 33 || b > 117 || c < 33 || c > 117 ||
		d < 33 || d > 117 || e < 33 || e > 117) {
		fprintf(errout, "ERROR: %d %d %d %d %d\n", a, b, c, d, e);
		return -1;
	}
	*k = ((((((((unsigned long) (a-33) * 85) + (b-33)) * 85) + (c-33)) * 85)
		+  (d-33)) * 85) + (e-33);
	return 0;
}

/* make four bytes from one unsigned long */

void longtobin (unsigned long code, int *u, int *v, int *w, int *x) {
	*x = (int) (code & 255);
	code = code >> 8;
	*w = (int) (code & 255);
	code = code >> 8;
	*v = (int) (code & 255);
	code = code >> 8;
	*u = (int) (code & 255);
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* Decodes one line of ASCII85 code - returns number of bytes produced */

int DecodeLine (unsigned char *outline, char *inline) {
	char *s=inline;
	unsigned char *t=outline;
	unsigned long k;
	int a, b, c, d, e;
	int u, v, w, x;
	int n;

	while (*s >= ' ') {
/*	Note  'z' should *not* occur in middle of group of 5 characters */
		if (*s == 'z') {
/*			if (verboseflag) printf("Hit a 'z' (four zeros)\n"); */
			if (verboseflag) printf("0 ");
			*t++ = 0; *t++ = 0; *t++ = 0; *t++ = 0;
/*			putc (0, output); putc (0, output); */
/*			putc (0, output); putc (0, output); */
		}
		else {
/*			a = 33; b = 33; c = 33; d = 33; e = 33; */
/*			background values in case incomplete group of 5 chars */
			a = 117; b = 117; c = 117; d = 117; e = 117;

/*			a = *s++; b = *s++; c = *s++;  d = *s++;  e = *s++; */
/*			if ((a = *s++) == '~') {
				n = 0; a = 33;
			}
			else if ((b = *s++) == '~') {
				n = 1; b = 33;
			}
			else if ((c = *s++) == '~') {
				n = 2; c = 33;
			}
			else if ((d = *s++) == '~') {
				n = 3; d = 33;
			}
			else if ((e = *s++) == '~') {
				n = 4; e = 33;
			}
			else n = 5;
*/

			if (*s == '~') {
				if (traceflag) printf("START ~> "); 
				break;				/* *no* new group of 5 chars */
			}

			n = 0;
			if (*s < '~')  {
				a = *s++; n++;
			}
			if (*s < '~')  {
				b = *s++; n++;
			}
			if (*s < '~')  {
				c = *s++; n++;
			}
			if (*s < '~')  {
				d = *s++; n++;
			}
			if (*s < '~')  {
				e = *s++; n++;
			}
			if (*s == '~') {
				if (verboseflag) printf ("~> ");
/*				lastline = 1; */
			}
/*			else lastline = 0;	*/
			if (asciitolong (&k, a, b, c, d, e) != 0) {
				fprintf(errout, "Don't understand: %s", InLine);
/*				putc('\n', output); */
				return 0;
			}
			longtobin(k, &u, &v, &w, &x);
/*			putc (u, output);				putc (v, output); */
/*			putc (w, output);				putc (x, output); */
/*			if (n > 1) putc (u, output); */
/*			if (n > 1) *t++ = (char) u;
			if (n > 2) *t++ = (char) v;
			if (n > 3) *t++ = (char) w;
			if (n > 4) *t++ = (char) x;
			if (n < 5) break; */
			n--;
			if (n-- > 0) *t++ = (char) u;
			if (n-- > 0) *t++ = (char) v;
			if (n-- > 0) *t++ = (char) w;
			if (n-- > 0) *t++ = (char) x;
			if (n < 0) break;
		}	/* end of *not* 'z' case */
	} /* end of while loop over one line of input */
	return (t - outline);
}

void showhex (unsigned char *s, int n, FILE *output) {
	int i;
	unsigned  int c, d;

	for (i = 0; i < n; i++) {
		c = *s++;
		d = c & 15;
		c = (c >> 4) & 15;
		if (c > 9) c = c + 'A' - 10;
		else c = c + '0';
		putc(c, output);
		if (d > 9) d = d + 'A' - 10;
		else d = d + '0';
		putc(d, output);
	}
/*	putc('\n', output); */
	putc('\r', output);
}

void showbin (unsigned char *s, int n, FILE *output) {
	int i;
	for (i = 0; i < n; i++) putc(*s++, output);
}

/* Assumes each line of input has a multiple of 5 ASCII characters */
/* except for last line which stops short and ends with ~> */

int decode85(FILE *output, FILE *input) {
	int n;
/*	char *t; */
	unsigned char *t;
	int eexecflag = 0;
	
	while (GetLine(InLine, MAXLINE, input) != NULL) {
		if (strlen(InLine) >= MAXLINE - 1) fprintf(errout, "Line too long\n");
		if (traceflag) fputs(InLine, stdout);
		if (strncmp(InLine, "endstream", 9) == 0) {
			putc('\n', output);
			return 0;
		}
/*		s = InLine; t = OutLine; */
		n = DecodeLine(OutLine, InLine);
/*		if (eexecflag && strncmp(OutLine, "00000000", 8) == 0) { */
		if (eexecflag &&
			(t = (unsigned char *) strstr((char *) OutLine, "00000000")) != NULL) {
			showhex(OutLine, t - OutLine, output);
			if (verboseflag) printf("end EEXEC ");
			eexecflag = 0;
			showbin(t, OutLine + n - t, output);
/*			putc('\n', output); */
		}
/*		t = OutLine; */
		else if (!eexecflag &&
				(t = (unsigned char *) strstr((char *) OutLine, "eexec")) != NULL) {
			while (*t > ' ') t++;	/* skip to white space */
			while (*t <= ' ') t++;	/* skip over white space */
			showbin(OutLine, t - OutLine, output);
/*			putc('\n', output);  */
			if (verboseflag) printf("start EEXEC ");
			eexecflag = 1;
			showhex(t, OutLine + n - t, output);
		}
		else if (eexecflag) showhex(OutLine, n, output);
		else showbin(OutLine, n, output);
	
	} /* end of while loop over input file GetLine */
	fprintf(errout, "Unexpected EOF (decode85)\n");
	return EOF;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

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

char *filename(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

int DecodeFile (FILE *output, FILE *input) {
	ascii85flag=0; lzwflag=0; dctflag=0; ccittflag=0;
	while (GetLine(InLine, MAXLINE, input) != NULL) {
		if (strlen(InLine) >= MAXLINE - 1)
			fprintf(errout, "Line too long\n");
/*		if (strncmp(InLine, "/Filter", 7) == 0) { */
		if (strstr(InLine, "/Filter") != NULL) {
/*			if (traceflag) fputs(InLine, stdout); */
			if (verboseflag) fputs(InLine, stdout);
			if (strstr(InLine, "ASCII85Decode") != NULL) ascii85flag=1;
			else ascii85flag=0;
			if (strstr(InLine, "LZWDecode") != NULL) lzwflag=1;
			else lzwflag=0;
			if (strstr(InLine, "DCTDecode") != NULL) dctflag=1;
			else dctflag=0;
			if (strstr(InLine, "CCITFaxDecode") != NULL) ccittflag=1;
			else ccittflag=0;
		}
		if (strncmp(InLine, "stream", 6) == 0) {
			if (verboseflag) printf("Start stream - ");
			fputs(InLine, output);
			if (dctflag)
				fprintf(errout, "WARNING: Don't know how to DCT decode\n");
			if (ccittflag)
				fprintf(errout, "WARNING: Don't know how to CCITTFax decode\n");
			if (lzwflag) {
				if (verboseflag) printf("ASCII85 + LZW  - ");
				DecodeLZW(output, input);
/*				if (debugflag != 0) break; */
			}
			else if (ascii85flag) {
				if (verboseflag) printf("ASCII85 - ");
				if (decode85(output, input) == EOF) break;
			}
			else {
				 fprintf(errout, "No Filter specified?\n");
				 continue;
			}
			if (verboseflag) printf("End stream\n");
		}
		if (fputs(InLine, output) == EOF) {
			perror ("Output File");
			return -1;
		}
	}
	return 0;
}

#ifdef IGNORED
/* assumes <~ and ~> are on separate lines and lines are short */

int DoStrings (FILE *output, FILE *input) {
	int k, nlen;
	unsigned char *s;

	while (GetLine(InLine, MAXLINE, input) != NULL) {
		if (strncmp(InLine, "<~", 2) == 0) {
			fputs(InLine, output);				/* copy <~ to output */
			fputs(InLine, stdout);				/* copy <~ to output */
			while (GetLine(InLine, MAXLINE, input) != NULL) {
				if (strncmp(InLine, "~>", 2) == 0) {
					fputs(InLine, output);		/* copy ~> to output */
					fputs(InLine, stdout);		/* copy ~> to output */
					break;
				}
				fputs(InLine, stdout);			/* show input */
				nlen = DecodeLine (OutLine, InLine);
				printf("Converted %d bytes\n", nlen);	/* debugging */
				s = OutLine;
				for (k = 0; k < nlen; k++) putc(*s++, output);
			}
		}
		else fputs(InLine, output);
	}
	return 0;
}
#endif

#ifdef IGNORED
/* assumes <~ and ~> are on separate lines and lines are short */

int DoStrings (FILE *output, FILE *input) {
	int k, nlen;
	unsigned char *s;

	while (GetLine(InLine, MAXLINE, input) != NULL) {
		if (strncmp(InLine, "<~", 2) == 0) {
			fputs(InLine, output);				/* copy <~ to output */
			fputs(InLine, stdout);				/* copy <~ to output */
			while (GetLine(InLine, MAXLINE, input) != NULL) {
				if (strncmp(InLine, "~>", 2) == 0) {
					fputs(InLine, output);		/* copy ~> to output */
					fputs(InLine, stdout);		/* copy ~> to output */
					break;
				}
				fputs(InLine, stdout);			/* show input */
				nlen = DecodeLine (OutLine, InLine);
				printf("Converted %d bytes\n", nlen);	/* debugging */
				s = OutLine;
				for (k = 0; k < nlen; k++) putc(*s++, output);
			}
		}
		else fputs(InLine, output);
	}
	return 0;
}
#endif

int DoStrings (FILE *output, FILE *input) {
	int a, b, c, d, e;
	int m, n=-1;
	int u, v, w, x;
	unsigned long bin;
	char *s;
	unsigned total;
	unsigned long grand=0;
	char szSFNT[128];

	sfntflag = 0;
	for(;;) {
/*		Scan for start of an ASCII85 string */
		while ((s = fgets(InLine, MAXLINE, input)) != NULL) {
			if (strncmp(InLine, "<~", 2) == 0) break;
			if (sfntflag != 0 && showEOD == 0) {
				if (strncmp(InLine, "] def", 5) == 0)
					fprintf(output, "\n");
			}
			fputs(InLine, output);
/*			Note start of sfnt definition */
			if (*InLine == '/' &&
				strchr(InLine, '[') != NULL &&
				strstr(InLine, "sfnt") != NULL) {
				printf(InLine);
				sscanf (InLine, "/%s", &szSFNT);
				if ((s = strchr(szSFNT, '[')) != NULL) *s = '\0';
				sfntflag=1;
				grand = 0;				/* grand total */
			}
			if (*InLine == ']' &&
				strstr(InLine, "def") != NULL &&
				sfntflag != 0) {
				printf(InLine);
				sfntflag = 0;
				printf("Grand total for %s %lu bytes\n", szSFNT, grand);
			}
		}
		if (s == NULL) {
			printf("EOF\n");
			break;
		}
		if (verboseflag) printf(InLine);		/* show the starting <~ */
		total = 0;					/* reset total byte count */
		if (showEOD) fputs(InLine, output);

		while ((m = getc(input)) != EOF) {
			if (m <= ' ') continue;		/* ignore white space */
			if (m == '~') {
				if (verboseflag) printf("~"); /* trailing ~> */
				break;		/* hit the end presumably */
			}
			if (m == 'z') {				/* 5 zero special case */
				if (traceflag) printf("0");
				u = v = w = x = 0;
				n = 5;
			}
			else {
				b = c = d = e = 'u';		/* 117 = 33 + 84 */
				a = m;
				n = 1;
				while ((b = getc(input)) != EOF && b <= ' ') ;
				if (b == '~') {
					if (verboseflag) printf("%c%c\n", a, b);
					ungetc(b, input);
					b = 'u';
				}
				else {
					n++;
					while ((c = getc(input)) != EOF && c <= ' ') ;
					if (c == '~') {
						if (verboseflag) printf("%c%c%c\n", a, b, c);
						ungetc(c, input);
						c = 'u';
					}
					else {
						n++;
						while ((d = getc(input)) != EOF && d <= ' ') ;
						if (d == '~') {
							if (verboseflag) printf("%c%c%c%c\n", a, b, c, d);
							ungetc(d, input);
							d = 'u';
						}
						else {
							n++;
							while ((e = getc(input)) != EOF && e <= ' ') ;
							if (e == '~') {
								if (verboseflag) printf("%c%c%c%c%c\n", a, b, c, d, e);
								ungetc(e, input);
								e = 'u';
							}
							else n++;
						}
					}
				}
				if (traceflag)
					printf("%c%c%c%c%c\n", a, b, c, d, e);
				if (e == EOF || d == EOF || c == EOF || b == EOF) {
					fprintf(stderr, "Premature EOF\n");
					return -1;
				}
				if (asciitolong (&bin, a, b, c, d, e) != 0) {
					printf("ERROR: %c%c%c%c%c\n", a, b, c, d, e);
					break;							/* error */
				}
				if (n != 5) {
					if (verboseflag)
					printf("Not multiple of four bytes (%d)\n", n-1);
				}
				longtobin(bin, &u, &v, &w, &x);
			}
			if (n < 5) {
				printf("n %d u %d v %d w %d x %d sfnt %d\n",
					   n, u, v, w, x, sfntflag);
				if (sfntflag) { /* Throw away last zero byte for sfnt */
					/* check that last one really is zero ? */
					n--;
				}
			}
			if (n > 1) {
				putc(u, output);
				total++;
				if (n > 2) {
					putc(v, output);
					total++;
					if (n > 3) {
						putc(w, output);
						total++;
						if (n > 4) {
							putc(x, output);
							total++;
						}
					}
				}
			}
		}
		if (m == EOF) {
			fprintf(stderr, "Premature EOF\n");
			return -1;
		}
		if (m == '~') {
			m = getc(input);
			if (verboseflag) putc(m, stdout); /* trailing ~> */
			if (m != '>') {
				fprintf(stderr, " ERROR: Bad End");
				return -1;
			}
			if (verboseflag) putc('\n', stdout);			
		}
		printf("LAST n %d\n", n);
		if (showEOD) fprintf(output, "~>\n");
		printf("%u bytes\n", total);
		grand += total;
		while ((m = getc(input)) != EOF && m <= ' ') ;	/* white space */
		ungetc (m, input);
	}
	return 0;			/* can't get here ? */
}


int main (int argc, char *argv[]) {
	char infilename[MAXFILENAME], outfilename[MAXFILENAME];
	FILE *input, *output;
	int m, flag, firstarg = 1;

/*	if (argc < firstarg+1) exit(1); */
	if (firstarg >= argc) {
		printf("\tv verbose mode\n");
		printf("\tt trace mode\n");
		printf("\tp process PS file\n");
		printf("\te showEOD\n");
		printf("\tf use fgets\n");
		exit(1);
	}

	while (firstarg < argc && argv[firstarg][0] == '-') {
		if (argv[firstarg][1] == 'v') verboseflag = 1;
		else if (argv[firstarg][1] == 't') traceflag = 1;
		else if (argv[firstarg][1] == 'f') usefgets = 1;
		else if (argv[firstarg][1] == 'p') psfile = 1;
		else if (argv[firstarg][1] == 'e') showEOD = 1;
		firstarg++;
	}
	
	InitializeStringTable ();

	for (m = firstarg; m < argc; m++) {
		strcpy(infilename, argv[m]);
		if (psfile) extension(infilename, "ps");
		else extension(infilename, "pdf");
		if ((input = fopen(infilename, "rb")) == NULL) {
			perror(infilename); exit(1);
		}
		strcpy(outfilename, filename(infilename));
		if (psfile) forceexten(outfilename, "psx");
		else forceexten(outfilename, "pdx");
		if ((output = fopen(outfilename, "wb")) == NULL) {
			perror(outfilename); exit(1);
		}

		if (psfile) flag = DoStrings (output, input);
		else flag = DecodeFile (output, input);

		fclose(output);
		fclose(input);
		if (flag != 0)  break;
	}
	return 0;
}

/* Lacking DCT filter */

/* Lacking CCITTFax filter */

/* Assumes /Filter info all on one line */

/* Assumes `eexec' not split across lines */

/* Assumes `00000000' leads in end of encrypted section */

/* May run out of memory for LZW StringTable */

/* for PS files with <~...~> use -p on command line */

/* sfnt in PS files Type 42 fonts: */
/* may be multiple strings that get concatenated */
/* each individual string is limited to 65535 bytes */
/* strings must begin at TrueType table boundary or glyph boundary */
/* withing the glyf table */
/* TT requires tables start on 4 byte boundary */
/* TT requires individual glyf descriptions begin on 2 byte boundary */
/* Therefore each string must contain an even number of bytes */
/* Each string must have a padding byte of 00 appended */
/* So the length of each string is odd, the last byte is not part of TT */

/*	/Foo-sfnts[
	<~...~> <~...~> <~...~> <~...~> <~...~>
	] def
*/
