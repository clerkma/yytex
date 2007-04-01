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

/********************************************************************/
/*  Voyager Image Decompression Program - C Version for PC, VAX,    */
/*  and UNIX systems.                                               */
/*                                                                  */
/*  Decompresses images using Kris Becker's subroutine DECOMP.C     */
/*  which is included in this program in a shortened version.       */
/*                                                                  */
/*  Reads a variable length compressed VOYAGER image and outputs a  */
/*  fixed length uncompressed image file in PDS format with         */
/*  labels, image histogram, engineering table and 800 lines of     */
/*  836 bytes (800 samples, 36 engineering bytes); or an 800 by     */
/*  800 array with FITS, VICAR or no labels.  If used on a non-     */
/*  byte-swapped machine the image histogram is un-swapped.         */
/*                                                                  */
/*  Use the following commands to compile and link to produce an    */
/*  executable file:                                                */
/*                                                                  */
/*  On an IBM PC (using Microsoft C Version 5.x)                    */
/*                                                                  */
/*    cl /c cdcomp.c                                                */
/*    link  cdcomp/stack:10000;                                     */
/*                                                                  */
/*  On a VAX:                                                       */
/*                                                                  */
/*    cc   cdcomp                                                   */
/*    link cdcomp                                                   */
/*                                                                  */
/*  On a Unix host (Sun, Masscomp)                                  */
/*                                                                   */
/*    cc -o cdcomp cdcomp.c                                         */
/*                                                                  */
/* LIMS                                                             */
/*  This program has been tested on a VAX 780 (VMS 4.6), SUN        */
/*  Workstation (UNIX 4.2, release 3.4), and an IBM PC              */
/*  (MICROSOFT 5.1 compiler).  When converting to other             */
/*  systems, check for portability conflicts.                       */
/*                                                                  */
/* HIST                                                             */
/*  JUL88 C driver to decompress standard Voyager Compressed images */
/*  by Mike Martin 1988/07/30                                       */
/*                                                                  */
/*  Inputs   - Input file to be decompressed.                       */
/*                                                                  */
/*  Outputs  - Output file containing decompressed image.           */
/*                                                                  */
/********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE                  1
#define FALSE                 0
#define RECORD_BYTES        836

                                    /* pc i/o defines ?             */
#define O_RDONLY         0x0000     /* open for reading only        */
#define O_BINARY         0x8000     /* file mode is binary          */

                                    /* vax i/o defines              */
#define RECORD_TYPE      "rfm=fix"  /* VAX fixed length output      */
#define CTX              "ctx=bin"  /* no translation of \n         */
#define FOP          "fop=cif,sup"  /* file processing ops          */

#define HIST_LEN 511 
#define BUF_LEN 2048
#define LINE_LEN 80

typedef struct leaf
  {
   struct leaf *right;
   short int dn;
   struct leaf *left;
  } NODE;

/************************************************************************
 Declare the tree pointer. This pointer will hold the root of the tree
 once the tree is created by the accompanying routine huff_tree.
**************************************************************************/

  NODE *tree;

/* subroutine definitions                                           */

void               pds_labels(int); 
void               fits_labels(int);
void               vicar_labels(int);
void               no_labels(int);
int                check_host(void);
int                get_files(int, int);
long swap_long(long);
void decompress(char *, char *, long int *, long int *);
void decmpinit(long int*);

int read_var(char *, int, int);	

/* global variables                                                 */

int verboseflag=0;
int traceflag=0;
int showflag=0;

/* int                infile; */
FILE		       *infile, *outfile;

int                output_format=4;			/* default */

/* new goodies that were made global */

char          ibuf[BUF_LEN], obuf[BUF_LEN];

long          hist[HIST_LEN];

char    inname[LINE_LEN], outname[LINE_LEN];

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* return pointer to file name - minus path - returns pointer to filename */

char *stripname(char *pathname) {
	char *s;
	
	if ((s=strrchr(pathname, '\\')) != NULL) s++;
	else if ((s=strrchr(pathname, '/')) != NULL) s++;
	else if ((s=strrchr(pathname, ':')) != NULL) s++;
	else s = pathname;
	return s;
}

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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* main(); */

int main(int argc, char *argv[]) {

/* char          ibuf[BUF_LEN], obuf[BUF_LEN]; */
	
/* long          hist[HIST_LEN]; */

unsigned char blank=32;
/* short         host,length,total_bytes,line,i; */
int           host, length, total_bytes, i;
short         line;
long          long_length;
/* int           out_bytes = RECORD_BYTES; */
long          out_bytes = RECORD_BYTES;
int           count;
long compress_length, expand_length;
int flag=0;
int firstarg=1;
char *s;

/*********************************************************************/
/*                                                                   */
/* get host information and input and output files                   */
/*                                                                   */
/*********************************************************************/

   host = check_host();
/*   if (verboseflag != 0) printf("\nHost Type %d", host);*/

/*   while (argv[firstarg][0] == '-') { */
   while (firstarg < argc && argv[firstarg][0] == '-') {
	   if (strchr(argv[firstarg], 'v') != 0) verboseflag++;
	   if (strchr(argv[firstarg], 't') != 0) traceflag++;
	   if (strchr(argv[firstarg], 's') != 0) showflag++;
	   firstarg++;
   }

   if (argc > firstarg) {
	   strcpy(inname, argv[firstarg]);
	   extension(inname, "imq");
/*	   output_format = 4; */	/* default */
	   s = stripname(argv[firstarg]);
	   strcpy(outname, s);
	   forceexten(outname, "img");
	   flag = 1;
   }
   else flag = 0;
   host = get_files(host, flag);			/* may change host if VAX */
/*   if (verboseflag != 0) printf("\nOutput Type %d", output_format); */

/*********************************************************************/
/*                                                                   */
/* read and edit compressed file labels                              */
/*                                                                   */
/*********************************************************************/

   if (verboseflag != 0) printf("\nReading compressed file labels...");
   
   switch (output_format)
     {
       case 1: pds_labels(host);
               break;
       case 2: fits_labels(host);
               break;
       case 3: vicar_labels(host);
                break;
       case 4: no_labels(host);
     }
/*********************************************************************/
/*                                                                   */
/* process the image histogram                                       */
/*                                                                   */
/*********************************************************************/

if (verboseflag != 0) printf("\nProcessing image histogram...");

/* need to know record_bytes,hist_count,hist_item_type,item_count.*/
   total_bytes = 0;
   length = read_var((char *)hist, host, HIST_LEN * 4);
/*   total_bytes = total_bytes + length; */
   total_bytes += length;
   length = read_var((char *)hist+836, host, HIST_LEN * 4 - 836);
/*   total_bytes = total_bytes + length; */
   total_bytes += length;

   if (host == 2 || host == 5)             /* If non-byte swapped    */
     for (i=0;i<256;i++)                   /* host, swap bytes in    */
       hist[i] = swap_long(hist[i]);       /* the output histogram   */

   if (traceflag != 0) {
	   putc('\n', stdout);
	   for(i=0;i<256;i++) printf("%d\t%ld\n", i, hist[i]);
   }

   if (output_format == 1)     {
      fwrite((char *)hist,    836,1,outfile);
      fwrite((char *)hist+836,length,1,outfile);

      /*  pad out the histogram to a multiple of RECORD_BYTES */
      for (i=total_bytes;i<RECORD_BYTES*2;i++) 
		  fputc(blank,outfile);
     }

	 if (feof(infile) != 0) {
		 fprintf(stderr, "\nPremature EOF");
		 exit(3);
	 }

/*********************************************************************/
/*                                                                   */
/* process the encoding histogram                                    */
/* don't have to byte-swap because DECOMP.C does it for us           */
/*                                                                   */
/*********************************************************************/

if (verboseflag != 0) printf("\nProcessing encoding histogram...");

   length = read_var((char *)hist, host, HIST_LEN * 4);
   length = read_var((char *)hist+836, host, HIST_LEN * 4 - 836);
   length = read_var((char *)hist+1672, host, HIST_LEN * 4 - 1672);

   if (traceflag != 0) {
	   putc('\n', stdout);
	   for(i=0;i<511;i++) printf("%d\t%ld\n", i-256, hist[i]);
   }

	 if (feof(infile) != 0) {
		 fprintf(stderr, "\nPremature EOF");
		 exit(3);
	 }

/*********************************************************************/
/*                                                                   */
/* process the engineering summary                                   */
/*                                                                   */
/*********************************************************************/

   if (verboseflag != 0) printf("\nProcessing the engineering summary...");
   total_bytes = 0;
   length = read_var(ibuf, host, BUF_LEN);

   if (output_format == 1)     {
      fwrite(ibuf,length,1,outfile);
/*      total_bytes = total_bytes + length; */
      total_bytes += length;

      /*  pad out engineering to multiple of 836 */
      for (i=total_bytes;i<RECORD_BYTES;i++) 
		  fputc(blank,outfile);
     }

	 if (feof(infile) != 0) {
		 fprintf(stderr, "\nPremature EOF");
		 exit(3);
	 }

/*********************************************************************/
/*                                                                   */
/* initialize the decompression                                      */
/*                                                                   */
/*********************************************************************/

	  if (verboseflag != 0) printf("\nInitializing decompression routine...");
	  decmpinit(hist);

/*********************************************************************/
/*                                                                   */
/* decompress the image                                              */
/*                                                                   */
/*********************************************************************/

	  if (verboseflag != 0) printf("\nDecompressing data...");
    line=0;
	compress_length = 0;
	expand_length = 0;
    do      {
       length = read_var(ibuf, host, BUF_LEN);
       if (length <= 0) break;
       long_length = (long)length;
	   compress_length += long_length;			/* new */
       line += 1;
       decompress(ibuf, obuf, &long_length, &out_bytes);
	   expand_length += out_bytes;				/* new */
       if (output_format == 1)         {
          count = fwrite(obuf,RECORD_BYTES,1,outfile);
          if (count != 1)
            {
             printf("\nError writing output file.  Aborting program.");
             printf("\nCheck disk space or for duplicate file name on VAX.");
             exit(1);
            }
         }
       else fwrite(obuf,800,1,outfile);

       if (line % 100 == 0 && verboseflag != 0) printf("\n line %d",line);
      } while (length > 0 && line < 800);

 /*  pad out FITS file to a multiple of 2880 */
 if (output_format == 2)
   for (i=0;i<2240;i++) fputc(blank,outfile);

/* printf("\n"); */
/* close(infile); */					/* ??? */
 fclose(infile);
 if (ferror(outfile) != 0) {
	 perror("output file error");
 }
 fclose(outfile);
 if (verboseflag != 0) printf("\nCompression Ratio %lg", 
	 (double) expand_length / (double) compress_length);

 return 0;
}

/*********************************************************************/
/*                                                                   */
/* subroutine get_files - get input filenames and open               */
/*                                                                   */
/*********************************************************************/

/* int get_files(host) */
int get_files(int host, int flag) {
/* int host; */

/* char    inname[LINE_LEN], outname[LINE_LEN]; */
short   shortint;

if (flag == 0) {
  printf("\nEnter name of file to be decompressed: ");
  gets (inname);
}
  if (host == 1)    {
/*   if ((infile = open(inname,O_RDONLY | O_BINARY)) <= 0)	*//* ??? */
	 if ((infile = fopen(inname, "rb")) == NULL)  {
		perror(inname);
/*        printf("\ncan't open input file: %s\n",inname); */
        exit(1);
       }
	}
  else if (host == 2 | host == 3 | host == 5)    {
/*     if ((infile = open(inname,O_RDONLY)) <= 0)	*/			/* ??? */
     if ((infile = fopen(inname, "rb")) == NULL) {
		perror(inname);
/*        printf("\ncan't open input file: %s\n",inname); */
        exit(1);
       }

  /****************************************************************/
  /* If we are on a vax see if the file is in var length format.  */
  /* This logic is in here in case the vax file has been stored   */
  /* in fixed or undefined format.  This might be necessary since */
  /* vax variable length files can't be moved to other computer   */
  /* systems with standard comm programs (kermit, for example).   */
  /****************************************************************/

     if (host == 3)       {
/*        read(infile,&shortint,2); */					/* ??? */
          fread(&shortint, sizeof(shortint), 1, infile);
        if (shortint > 0 && shortint < 80)
          {
           host = 4;              /* change host to 4                */
           printf("This is not a VAX variable length file.");
          }
        else printf("This is a VAX variable length file.");
/*        lseek(infile,0,0); */        /* reposition to beginning of file */ 
		fseek(infile, 0, SEEK_SET);		/* rewind(infile); */
       }
    }
		
	if (flag == 0) {
  output_format = 0;
  do    {
     printf("\nEnter a number for the output format desired:\n");
     printf("\n  1.  SFDU/PDS format.");     printf("\n  2.  FITS format.");
     printf("\n  3.  VICAR format.");
     printf("\n  4.  Unlabelled binary array.\n");
     printf("\n  Enter format number:");
     gets(inname);
     output_format = atoi(inname);
    } while (output_format < 1 || output_format > 4);

	}
	if (flag == 0) {
  printf("\nEnter name of uncompressed output file: ");
  gets (outname);
	}

  if (host == 1 || host == 2 || host == 5)    {
     if ((outfile = fopen(outname,"wb"))==NULL)   {
		 perror(outname);
/*        printf("\ncan't open output file: %s\n",outname); */
        exit(1);
       }
    }
#ifdef NOT_PC
  else if (host == 3 || host == 4)
    {
     if (output_format == 1) {     /* write PDS format blocks */
        if ((outfile=fopen(outname,"w",
                           "mrs=836",FOP,CTX,RECORD_TYPE))==NULL)     {
          printf("\ncan't open output file: %s\n",outname);
          exit(1);
         }
       }
     else if (output_format == 2) { /* write FITS format blocks */
        if ((outfile=fopen(outname,"w",
                           "mrs=2880",FOP,CTX,RECORD_TYPE))==NULL)        {
          printf("\ncan't open output file: %s\n",outname);
          exit(1);
         }
       }
     else {                        /* write 800 byte records */
        if ((outfile=fopen(outname,"w",
                           "mrs=800",FOP,CTX,RECORD_TYPE))==NULL)   {
          printf("\ncan't open output file: %s\n",outname);
          exit(1);
         }
       }
	}
#endif			/* exclude above if compiling on PC */

  return(host);  /* In case its been updated */
}

/*********************************************************************/
/*                                                                   */
/* subroutine pds_labels - edit PDS labels and write to output file */
/*                                                                   */
/*********************************************************************/

/* void pds_labels(host) */
void pds_labels(int host) {
/* int host; */

char          outstring[LINE_LEN];

/* char	      ibuf[BUF_LEN]; */ /* use global buffer */

unsigned char cr=13,lf=10;
/* unsigned char blank=332;		*/					/* ??? */
unsigned char blank=32;
short         total_bytes;
/* short         i; */
int			i, length;
/* short         len,line; */				/* unreferenced */

total_bytes = 0;
do  {
   length = read_var(ibuf, host, BUF_LEN);
/*   ibuf[length]=NULL; */
   ibuf[length]='\0';						/* FIX */

  /******************************************************************/
  /*  edit labels which need to be changed                          */
  /******************************************************************/

   if      ((i = strncmp(ibuf,"NJPL1I00PDS1",12)) == 0)
   /*****************************************************************/
   /* add the output file length to the sfdu label                  */
   /*****************************************************************/
     {
      strcpy(outstring,ibuf);
      strcpy(outstring+12,"00673816");
      strcpy(outstring+20,ibuf+20);
      fwrite(outstring,length,1,outfile);
      fprintf(outfile,"%c%c",cr,lf);
/*      total_bytes = total_bytes + length + 2; */
      total_bytes += (short) (length + 2);
     }
   else if ((i = strncmp(ibuf,"RECORD_TYPE",11)) == 0)
   /*****************************************************************/
   /* change the record_type value from variable to fixed           */
   /*****************************************************************/
     {
      strcpy(ibuf+35,"FIXED_LENGTH");
/*      length = length - 3; */
      length -= 3;
      fwrite(ibuf,length,1,outfile);
      fprintf(outfile,"%c%c",cr,lf);
/*      total_bytes = total_bytes + length + 2; */
      total_bytes += (short) (length + 2);
     }
   else if ((i = strncmp(ibuf,"FILE_RECORDS",12)) == 0)
   /*****************************************************************/
   /* change the file_records count to 806                          */
   /*****************************************************************/
     {
      strcpy(ibuf+35,"806");
      fwrite(ibuf,length,1,outfile);
      fprintf(outfile,"%c%c",cr,lf);
/*      total_bytes = total_bytes + length + 2; */
      total_bytes += (short) (length + 2);
     }
   else if ((i = strncmp(ibuf,"LABEL_RECORDS",13)) == 0)
   /*****************************************************************/
  /* change the label_records count from 56 to 3                    */
   /****************************************************************/
     {
      strcpy(ibuf+35,"3");
      length -= 1;
      fwrite(ibuf,length,1,outfile);
      fprintf(outfile,"%c%c",cr,lf);
/*      total_bytes = total_bytes + length + 2; */
      total_bytes += (short) (length + 2);
     }
   else if ((i = strncmp(ibuf,"^IMAGE_HISTOGRAM",16)) == 0)
   /*****************************************************************/
   /* change the location pointer of image_histogram to record 4    */
   /*****************************************************************/
     {
      strcpy(ibuf+35,"4");
      length -= 1;
      fwrite(ibuf,length,1,outfile);
      fprintf(outfile,"%c%c",cr,lf);
/*      total_bytes = total_bytes + length + 2; */
      total_bytes += (short) (length + 2);
     }
   else if ((i = strncmp(ibuf,"^ENCODING_HISTOGRAM)",19)) == 0);
   /*****************************************************************/
   /* delete the encoding_histogram location pointer                */
   /*****************************************************************/
   else if ((i = strncmp(ibuf,"^ENGINEERING_TABLE",18)) == 0)
   /*****************************************************************/
   /* change the location pointer of engineering_summary to record 6*/
   /*****************************************************************/
     {
      strcpy(ibuf+35,"6");
      length -= 1;
      fwrite(ibuf,length,1,outfile);
      fprintf(outfile,"%c%c",cr,lf);
/*      total_bytes = total_bytes + length + 2; */
      total_bytes += (short) (length + 2);
     }
   else if ((i = strncmp(ibuf,"^IMAGE",6)) == 0)
   /*****************************************************************/
   /* change the location pointer of image to record 7              */
   /*****************************************************************/
     {
      strcpy(ibuf+35,"7");
/*      length = length -1; */
      length -= 1;
      fwrite(ibuf,length,1,outfile);
      fprintf(outfile,"%c%c",cr,lf);
/*      total_bytes = total_bytes + length + 2; */
      total_bytes += (short) (length + 2);
     }
   else if ((i = strncmp(ibuf,
             "OBJECT                           = ENCODING",43)) == 0)
   /******************************************************************/
   /* delete the 4 encoding histogram labels                        */
   /*****************************************************************/
     {
      for (i=0;i<4;i++)  {  /* ignore these labels */
         length = read_var(ibuf, host, BUF_LEN);
        }
     }
   else if ((i = strncmp(ibuf," ENCODING",9)) == 0);
   /*****************************************************************/
   /* delete the encoding type label in the image object            */
   /*****************************************************************/
   else if ((host == 2 || host == 5) && (i = strncmp(ibuf,
             " ITEM_TYPE                       = VAX_INTEGER",46)) == 0)
   /*****************************************************************/
   /* change the record_type value from variable to fixed           */
   /*****************************************************************/
     {
      strcpy(ibuf+35,"INTEGER");
/*      length = length - 4; */
      length -= 4;
      fwrite(ibuf,length,1,outfile);
      fprintf(outfile,"%c%c",cr,lf);
/*      total_bytes = total_bytes + length + 2; */
      total_bytes += (short) (length + 2);
     }



   /*****************************************************************/
   /* if none of above write out the label to the output file       */
   /*****************************************************************/
   else
     {
      fwrite(ibuf,length,1,outfile);
      fprintf(outfile,"%c%c",cr,lf);
/*      total_bytes = total_bytes + length + 2; */
      total_bytes += (short) (length + 2);
     }
   /*****************************************************************/
   /* test for the end of the PDS labels                            */
   /*****************************************************************/
   if ((i = strncmp(ibuf,"END",3)) == 0 && length == 3) break;
  } while (length > 0);

/* pad out the labels with blanks to multiple of RECORD_BYTES */
   for (i=total_bytes;i<RECORD_BYTES*3;i++) fputc(blank,outfile);
}

/*********************************************************************/
/*                                                                  */
/* subroutine fits_labels - write FITS header to output file */
/*                                                                   */
/*********************************************************************/

/* void fits_labels(host) */
void fits_labels(int host) {
/* int host; */

/* char          ibuf[BUF_LEN]; */ /* use global buffer */

char          outstring[LINE_LEN];

unsigned char cr=13,lf=10,blank=32;
/* short         length,total_bytes; */
/* short         i; */
int         i, length,total_bytes; 
/* short         nlen, line; */

do  {
   length = read_var(ibuf, host, BUF_LEN);
   /*****************************************************************/
   /* read to the end of the PDS labels                             */
   /*****************************************************************/
   if ((i = strncmp(ibuf,"END",3)) == 0 && length == 3) break;
  } while (length > 0);

total_bytes = 0;

strcpy(outstring,
"SIMPLE  =                    T                                                ");
fwrite(outstring,78,1,outfile);
fprintf(outfile,"%c%c",cr,lf);
/* total_bytes = total_bytes + 80; */
total_bytes += 80;

strcpy(outstring,
"BITPIX  =                    8                                                ");
fwrite(outstring,78,1,outfile);
fprintf(outfile,"%c%c",cr,lf);
/* total_bytes = total_bytes + 80; */
total_bytes += 80;

strcpy(outstring,
"NAXIS   =                    2                                                ");
fwrite(outstring,78,1,outfile);
fprintf(outfile,"%c%c",cr,lf);
/* total_bytes = total_bytes + 80; */
total_bytes += 80;

strcpy(outstring,
"NAXIS1  =                  800                                                ");
fwrite(outstring,78,1,outfile);
fprintf(outfile,"%c%c",cr,lf);
/* total_bytes = total_bytes + 80; */
total_bytes += 80;

strcpy(outstring,
"NAXIS2  =                  800                                                ");
fwrite(outstring,78,1,outfile);
fprintf(outfile,"%c%c",cr,lf);
/* total_bytes = total_bytes + 80; */
total_bytes += 80;

strcpy(outstring,
"END                                                                           ");
fwrite(outstring,78,1,outfile);
fprintf(outfile,"%c%c",cr,lf);
/* total_bytes = total_bytes + 80; */
total_bytes += 80;

/* pad out the labels with blanks to multiple of RECORD__BYTES */
   for (i=total_bytes;i<2880;i++) fputc(blank,outfile);
}

/*********************************************************************/
/*                                                                   */
/* subroutine vicar_labels - write vicar labels to output file       */
/*                                                                   */
/*********************************************************************/

/* void vicar_labels(host) */
void vicar_labels(int host) {
/* int host; */

/* char          ibuf[BUF_LEN]; */ /* use global buffer */

char		  outstring[LINE_LEN];

unsigned char cr=13,lf=10,blank=32;
/* short         length,total_bytes; */
/* short         i; */
int         i, length,total_bytes; 
/* short         nlen,line; */

do  {
   length = read_var(ibuf, host, BUF_LEN);
   /*****************************************************************/
   /* read to the end of the PDS labels                             */
   /*****************************************************************/
   if ((i = strncmp(ibuf,"END",3)) == 0 && length == 3) break;
  } while (length > 0);

total_bytes = 0;

strcpy(outstring,
"LBLSIZE=800             FORMAT='BYTE'  TYPE='IMAGE'  BUFSIZ=800  DIM=2  ");
fwrite(outstring,72,1,outfile);
/* total_bytes = total_bytes + 72; */
total_bytes += 72;
strcpy(outstring,
"EOL=0  RECSIZE=800  ORG='BSQ'  NL=800  NS=800  NB=1  N1=0  N2=0  N3=0  ");
/* total_bytes = total_bytes + 71; */
total_bytes += 71;
fwrite(outstring,71,1,outfile);
strcpy(outstring,
"N4=0  NBB=0  NLB=0");
fwrite(outstring,18,1,outfile);
fprintf(outfile,"%c%c",cr,lf);
/* total_bytes = total_bytes + 20; */
total_bytes += 20;

/* pad out the labels with blanks to multiple of RECORD_BYTES */
   for (i=total_bytes;i<800;i++) fputc(blank,outfile);
}

/*********************************************************************/
/*                                                                   */
/* subroutine no_labels - read past the pds labels                   */
/*                                                                   */
/*********************************************************************/

/* void no_labels(host) */
void no_labels(int host) {
/* int host; */

/* char          ibuf[BUF_LEN]; */ /* use global buffer */

/* char          outstring[80]; */

unsigned char cr=13,lf=10,blan=32;
/* short         length; */
/* short         i; */
int         i, length;
/* short         nlen,line,total_bytes; */

do  {
   length = read_var(ibuf, host, BUF_LEN);
   if (showflag != 0) {
	   ibuf[length] = '\0';
	   printf("\n%s", ibuf); 
/*	   putc('\n', stdout);	   printf(ibuf); */
   }
   /*****************************************************************/
   /* read to the end of the PDS labels                             */
   /*****************************************************************/
   if ((i = strncmp(ibuf,"END",3)) == 0 && length == 3) break;
  } while (length > 0);
}


/*********************************************************************/
/*                                                                   */
/* subroutine read_var - read variable length records from input file*/
/*                                                                   */
/*********************************************************************/

/* read_var(ibuf,host) */
/* short read_var(char *ibuf, int host) { */
int read_var(char *ibuf, int host, int maxlen) {
/* char  *ibuf; */
/* int   host; */

int   length, result, nlen;
char  temp;
/* int k; */

union /* this union is used to swap 16 and 32 bit integers          */
  {
   char  ichar[4];
   short slen;
   long  llen;
  } onion;

  switch (host)
    {
     case 1: /*******************************************************/
             /* IBM PC host                                         */
             /*******************************************************/
       length = 0;
/*       result = read(infile,&length,2); */
       result = fread(&length, sizeof(length), 1, infile);
/*       nlen =   read(infile,ibuf,length+(1*length%2)); */
	   if (length > maxlen) {
		   fprintf(stderr, "\nUnexpectedly long record %d > %d", 
			   length, maxlen);
/*		   for (k=0; k < length +(1*length%2); k++) getc(infile); */
		   exit(3);
	   }
	   else {
		   nlen = fread(ibuf, 1, length+(1*length%2), infile); 
		   if (traceflag != 0) 
		   printf("\nresult %d length %d nlen %d", result, length, nlen);
		   return (length);
	   }
	   if (ferror(infile) != 0) {
		   perror("input file error");
		   exit(3);
	   }
       break;

     case 2: /*******************************************************/
             /* non byte swapped 16 bit host                        */
             /*******************************************************/
       length = 0;
/*       result = read(infile,onion.ichar,2); */
       result = fread(onion.ichar, 1, 2, infile);
       /*     byte swap the length field                            */
       temp   = onion.ichar[0];
       onion.ichar[0]=onion.ichar[1];
       onion.ichar[1]=temp;
       /* printf("length=%04x,result=%d\n",length,result);          */
/*       nlen =   read(infile,ibuf,length+(1*length%2)); */
       nlen =   fread(ibuf, 1, length+(1*length%2), infile);
	   if (traceflag != 0) printf("result %d length %d nlen %d\n");
       return (length);
       break;

/*   case 33: */
     case 3: /*******************************************************/
             /* VAX host with variable length support               */
             /*******************************************************/
/*       length = read(infile,ibuf,RECORD_BYTES); */
       length = fread(ibuf, 1, RECORD_BYTES, infile);
       return (length);

     case 4: /*******************************************************/
             /* VAX host, but not a variable length file            */
             /*******************************************************/
       length = 0;
/*       result = read(infile,&length,2); */
       result = fread(&length, sizeof(length), 1, infile);
/*       nlen =   read(infile,ibuf,length+(1*length%2)); */
       nlen =   fread(ibuf, 1, length+(1*length%2), infile);

       /* check to see if we crossed a vax record boundary          */
       while (nlen < length)
/*         nlen += read(infile,ibuf+nlen,length+(1*length%2)-nlen); */
         nlen += fread(ibuf+nlen, 1, length+(1*length%2)-nlen, infile);
       return (length);
       break;

     case 5: /*******************************************************/
             /* non byte swapped 32 bit host without var support    */
             /*******************************************************/
       length = 0;
/*       result = read(infile,onion.ichar,2); */
       result = fread(onion.ichar, 1, 2, infile);
       /*     byte swap the length field                            */
       temp   = onion.ichar[0];
       onion.ichar[0]=onion.ichar[1];
       onion.ichar[1]=temp;
       length = onion.slen;
       /* printf("length=%04x,result=%d\n",length,result); */
/*       nlen =   read(infile,ibuf,length+(1*length%2)); */
       nlen =   fread(ibuf, 1, length+(1*length%2), infile);
       return (length);
       break;
    }
}

/*********************************************************************/
/*                                                                   */
/* subroutine check_host - find out what kind of machine we are on   */
/*                                                                   */
/*********************************************************************/

/* int check_host() { */
int check_host(void) {
/*  This subroutine checks the attributes of the host computer and
    returns a host code number. */
char hostname[LINE_LEN];

int swap,host,bits;
/* int var; */

union
  {
   char  ichar[2];	
   short ilen;
  } onion;

/* if (sizeof(var) == 4) bits = 32; */
if (sizeof(swap) == 4) bits = 32;
                 else bits = 16;

onion.ichar[0] = 1;
onion.ichar[1] = 0;

if (onion.ilen == 1) swap = TRUE;
else                 swap = FALSE;

if (bits == 16 && swap == TRUE)  {
   host = 1; /* IBM PC host  */
   strcpy(hostname,
          "Type 1 - 16 bit integers with swapping, no var len support.");
  }

if (bits == 16 && swap == FALSE)  {
   host = 2; /* Non byte swapped 16 bit host  */
   strcpy(hostname,
          "Type 2 - 16 bit integers without swapping, no var len support.");
  }

if (bits == 32 && swap == TRUE) { 
	host = 3; /* VAX host with var length support */
   strcpy(hostname,
          "Type 3,4 - 32 bit integers with swapping.");
 }

if (bits == 32 && swap == FALSE)  {
   host = 5; /* OTHER 32-bit host  */
   strcpy(hostname,
          "Type 5 - 32 bit integers without swapping, no var len support.");
  }

/* printf("%s\n",hostname); */
  if (verboseflag != 0) printf("\n%s",hostname);
return(host);
}

/* long swap_long(inval) */  /* swap 4 byte integer                       */
long swap_long(long inval) {  /* swap 4 byte integer                       */
/* long inval; */

union /* this union is used to swap 16 and 32 bit integers          */
  {
   char  ichar[4];
   short slen;
   long  llen;
  } onion;
  char   temp;

  /* byte swap the input field                                      */
  onion.llen   = inval;
  temp   = onion.ichar[0];
  onion.ichar[0]=onion.ichar[3];
  onion.ichar[3]=temp;
  temp   = onion.ichar[1];
  onion.ichar[1]=onion.ichar[2];
  onion.ichar[2]=temp;
  return (onion.llen);
}

/*  void decompress(ibuf,obuf,nin,nout) */
void decompress(char *ibuf, char *obuf, long int *nin, long int *nout) {
/****************************************************************************
*_TITLE decompress - decompresses image lines stored in compressed format   *
*_ARGS  TYPE       NAME      I/O        DESCRIPTION                         *
        char       *ibuf;   * I         Compressed data buffer              *
        char       *obuf;   * O         Decompressed image line             *
        long int   *nin;    * I         Number of bytes on input buffer     *
        long int   *nout;   * I         Number of bytes in output buffer    */

 /* The external root pointer to tree */
    extern NODE *tree;

 /* Declare functions called from this routine */
    void dcmprs();

/*************************************************************************
  This routine is fairly simple as it's only function is to call the
  routine dcmprs.
**************************************************************************/

    dcmprs(ibuf,obuf,nin,nout,tree);

    return;
  }

/* void decmpinit(hist) */
void decmpinit(long int *hist) {
/***************************************************************************
*_TITLE decmpinit - initializes the Huffman tree                           *
*_ARGS  TYPE       NAME      I/O        DESCRIPTION                        *
        long int   *hist;   * I         First-difference histogram.        */

  extern NODE *tree;          /* Huffman tree root pointer */

  /* Specify the calling function to initialize the tree */
  NODE *huff_tree();

/****************************************************************************
  Simply call the huff_tree routine and return.
*****************************************************************************/

  tree = huff_tree(hist);

  return;
 }

/* NODE *huff_tree(hist) */
NODE *huff_tree(long int *hist) {
/****************************************************************************
*_TITLE huff_tree - constructs the Huffman tree; returns pointer to root    *
*_ARGS  TYPE          NAME        I/O   DESCRIPTION                         *
        long int     *hist;      * I    First difference histogram          */

  /*  Local variables used */
    long int freq_list[512];      /* Histogram frequency list */
    NODE **node_list;             /* DN pointer array list */

    register long int *fp;        /* Frequency list pointer */
    register NODE **np;           /* Node list pointer */

    register long int num_freq;   /* Number non-zero frequencies in histogram */
/*    long int sum; */                /* Sum of all frequencies */

    register short int num_nodes; /* Counter for DN initialization */
    register short int cnt;       /* Miscellaneous counter */

    short int znull = -1;         /* Null node value */

    register NODE *temp;          /* Temporary node pointer */

  /* Functions called */
    void sort_freq();
    NODE *new_node();
/*  char *malloc(); */ /* void * __cdecl malloc(size_t); */

/***************************************************************************
  Allocate the array of nodes from memory and initialize these with numbers
  corresponding with the frequency list.  There are only 511 possible
  permutations of first difference histograms.  There are 512 allocated
  here to adhere to the FORTRAN version.
****************************************************************************/

   fp = freq_list;
   node_list = (NODE **) malloc(sizeof(temp)*512);
   if (node_list == NULL)
    {
      printf("\nOut of memory in huff_tree!\n");
      exit(1);
    }
   np = node_list;

   for (num_nodes=1, cnt=512 ; cnt-- ; num_nodes++)
     {
/**************************************************************************
    The following code has been added to standardize the VAX byte order
    for the "long int" type.  This code is intended to make the routine
    as machine independant as possible.
***************************************************************************/
        unsigned char *cp = (unsigned char *) hist++;
        unsigned long int j;
        short int i;
        for (i=4 ; --i >= 0 ; j = (j << 8) | *(cp+i));

/* Now make the assignment */
        *fp++ = j;
        temp = new_node(num_nodes);
        *np++ = temp;
     }

     (*--fp) = 0;         /* Ensure the last element is zeroed out.  */

/***************************************************************************
  Now, sort the frequency list and eliminate all frequencies of zero.
****************************************************************************/

  num_freq = 512;
  sort_freq(freq_list,node_list,num_freq);

  fp = freq_list;
  np = node_list;

  for (num_freq=512 ; (*fp) == 0 && (num_freq) ; fp++, np++, num_freq--);


/***************************************************************************
  Now create the tree.  Note that if there is only one difference value,
  it is returned as the root.  On each interation, a new node is created
  and the least frequently occurring difference is assigned to the right
  pointer and the next least frequency to the left pointer.  The node
  assigned to the left pointer now becomes the combination of the two
  nodes and it's frequency is the sum of the two combining nodes.
****************************************************************************/

  for (temp=(*np) ; (num_freq--) > 1 ; )
    {
        temp = new_node(znull);
        temp->right = (*np++);
        temp->left = (*np);
        *np = temp;
        *(fp+1) = *(fp+1) + *fp;
        *fp++ = 0;
        sort_freq(fp,np,num_freq);
    }

  return temp;
 }

/* NODE *new_node(value) */
NODE *new_node(short int value) {
/****************************************************************************
*_TITLE new_node - allocates a NODE structure and returns a pointer to it   *
*_ARGS  TYPE        NAME        I/O     DESCRIPTION                         *
        short int   value;     * I      Value to assign to DN field         */

    NODE *temp;         /* Pointer to the memory block */

/***************************************************************************
  Allocate the memory and intialize the fields.
****************************************************************************/

  temp = (NODE *) malloc(sizeof(NODE));

  if (temp != NULL)
    {
      temp->right = NULL;
      temp->dn = value;
      temp->left = NULL;
    }
  else
    {
       printf("\nOut of memory in new_node!\n");
       exit(1);
    }

   return temp;
  }

/*  void sort_freq(freq_list,node_list,num_freq) */
void sort_freq(long int *freq_list, NODE **node_list, long int num_freq) {
/***************************************************************************
*_TITLE sort_freq - sorts frequency and node lists in increasing freq. order*
*_ARGS  TYPE       NAME            I/O  DESCRIPTION                         *
        long int   *freq_list;    * I   Pointer to frequency list           *
        NODE       **node_list;   * I   Pointer to array of node pointers   *
        long int   num_freq;      * I   Number of values in freq list       */

    /* Local Variables */
    register long int *i;       /* primary pointer into freq_list */
    register long int *j;       /* secondary pointer into freq_list */

    register NODE **k;          /* primary pointer to node_list */
    register NODE **l;          /* secondary pointer into node_list */

    long int temp1;             /* temporary storage for freq_list */
    NODE *temp2;                /* temporary storage for node_list */

    register long int cnt;      /* count of list elements */


/************************************************************************
  Save the current element - starting with the second - in temporary
  storage.  Compare with all elements in first part of list moving
  each up one element until the element is larger.  Insert current
  element at this point in list.
*************************************************************************/

   if (num_freq <= 0) return;      /* If no elements or invalid, return */

   for (i=freq_list, k=node_list, cnt=num_freq ; --cnt ; *j=temp1, *l=temp2)
     {
        temp1 = *(++i);
        temp2 = *(++k);

        for (j = i, l = k ;  *(j-1) > temp1 ; )
          {
            *j = *(j-1);
            *l = *(l-1);
            j--;
            l--;
            if ( j <= freq_list) break;
		  }

	 }
	return;
}

/*  void dcmprs(ibuf,obuf,nin,nout,root) */
void dcmprs(char *ibuf, char *obuf, long int *nin, long int *nout, NODE *root) {
/***************************************************************************
*_TITLE dcmprs - decompresses Huffman coded compressed image lines         *
*_ARGS  TYPE       NAME       I/O       DESCRIPTION                        *
        char       *ibuf;   * I        Compressed data buffer              *
        char       *obuf;   * O        Decompressed image line             *
        long int   *nin;    * I        Number of bytes on input buffer     *
        long int   *nout;   * I        Number of bytes in output buffer    *
        NODE       *root;   * I        Huffman coded tree                  */

    /* Local Variables */
    register NODE *ptr = root;        /* pointer to position in tree */
    register unsigned char test;      /* test byte for bit set */
    register unsigned char idn;       /* input compressed byte */

    register char odn;                /* last dn value decompressed */

    char *ilim = ibuf + *nin;         /* end of compressed bytes */
    char *olim = obuf + *nout;        /* end of output buffer */

/**************************************************************************
  Check for valid input values for nin, nout and make initial assignments.
***************************************************************************/

    if (ilim > ibuf && olim > obuf)
       odn = *obuf++ = *ibuf++;
    else
       {
           printf("\nInvalid byte count in dcmprs!\n");
           exit(1);
       }

/**************************************************************************
  Decompress the input buffer.  Assign the first byte to the working
  variable, idn.  An arithmatic and (&) is performed using the variable
  'test' that is bit shifted to the right.  If the result is 0, then
  go to right else go to left.
***************************************************************************/

    for (idn=(*ibuf) ; ibuf < ilim  ; idn =(*++ibuf))
     {
        for (test=0x80 ; test ; test >>= 1)
           {
            ptr = (test & idn) ? ptr->left : ptr->right;

            if (ptr->dn != -1)
              {
                if (obuf >= olim) return;
                odn -= ptr->dn + 256;
                *obuf++ = odn;
                ptr = root;
              }
          }
     }
   return;
  }
