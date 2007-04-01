/* Copyright 1990, 1991, 1992 Y&Y, Inc.
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

/* This is Adobe's packPS.c - totally brain-damaged (Glenn Reid) ! */

/* WARNING: this doesn't do at all what manual page says it does */
/* --- for example: command flags are NOT allowed */
/* --- and there is an enormous amount of unused garbage... */

/* usage: PACKPS <file-in>  > <file-out> */

#include <stdio.h>
#include <string.h>

/* #include "version.h" */

/* #define EDIT 10 */ /* Thu Aug  2 12:21:36 EDT 1990 */
/* #define VERSION "2.0" */  /* Sat Jul 18 20:34:03 1987 */

/*
 * (C) 1986 by Adobe Systems Incorporated. All rights reserved.
 *
 * This file may be freely copied and redistributed as long as:
 *   1) This entire notice continues to be included in the file, 
 *   2) If the file has been modified in any way, a notice of such
 *      modification is conspicuously indicated.
 *
 * PostScript, Display PostScript, and Adobe are registered trademarks of
 * Adobe Systems Incorporated.
 * 
 * ************************************************************************
 * THE INFORMATION BELOW IS FURNISHED AS IS, IS SUBJECT TO CHANGE WITHOUT
 * NOTICE, AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY ADOBE SYSTEMS
 * INCORPORATED. ADOBE SYSTEMS INCORPORATED ASSUMES NO RESPONSIBILITY OR 
 * LIABILITY FOR ANY ERRORS OR INACCURACIES, MAKES NO WARRANTY OF ANY 
 * KIND (EXPRESS, IMPLIED OR STATUTORY) WITH RESPECT TO THIS INFORMATION, 
 * AND EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR PARTICULAR PURPOSES AND NONINFINGEMENT OF THIRD PARTY RIGHTS.
 * ************************************************************************
 */
 
/*************************************************************************
 **									**
 **	packPS							**
 **									**
 **	filter to compress PostScript source files into minimal space.	**
 **									**
 **									**
Edit History:
	Glenn Reid Wed Tue Sep  3 14:10:17 1986 created
	greid Thu Oct 23 12:38:13 1986 fixed bug:  packPS was
		inserting spaces ahead of / inside strings.
	greid Fri May  8 14:18:32 1987 break lines at 256 maximum
 **									**
 *************************************************************************/


/* constants */

#define LEFTPAREN '('		/* delimeter for PasteUp operators */
#define RIGHTPAREN ')'		/* delimeter for PasteUp operators */

/* your basic constants */
#define TRUE 1
#define FALSE 0
/* #define MAXCOUNT 128 */		/* max characters on one line */

#define WHITE_CHAR '\n'		/* for separating tokens */

/* #define SPACE ' ' */		/* another WHITE character */
/* #define FILENAME_MAX 256 */	/* max characters in pathname */
/* #define STRING_MAX 256 */		/* max for input strings */
/* #define TEXT 4 */			/* verbatim text, unparsed */

/* boolean switches */
/* short	messages;	*/ /* print messages or not? (boolean) */

/* other Globals */
/*  int	sp;	*/	/* for indenting the messages */
int	whitecount;
/* int	charcount;	*/ /* for 'condense' mode */
/* char  pstring[STRING_MAX]; */

/** --- beginning of procedure definitions --- **/

void InitGlobals(void) {
/*    int     i; */

/*    printf ("%%! Adobe packPS Version %s (%d)\n", VERSION, EDIT ); */
    printf ("%%! Adobe packPS (Glenn Reid)\n"); 

/*    messages = FALSE; */
    whitecount = 0; 	/* that's it ! */
/*    charcount = 0; */
}				/* InitGlobals */


FILE *openfile (char *filename) {  /* int newtype */
/*
 * This is a separate function for better portability, since this is
 * the most machine-dependent part of the system.  Returns
 */
    FILE *newfile;

    if ( 0 >= strlen(filename) ) {
	newfile = (FILE *)-1;
    }
    newfile = fopen(filename, "r" );
    if ( newfile <= 0 ) {
	printf ("%%%% CAN'T FIND: %s\n", filename );
	fprintf ( stderr, "CAN'T FIND: %s\n", filename );
	newfile = (FILE *)-1;
    }
    return ( newfile );
} /* openfile */


char *mainfilter (FILE *stream ) /* FILE *stream; */
/*
 * copies current input file to current output file char by char
 * removing unnecessary white space and comments.
 */
{
/*    char   *curr_token; */
    register int ch;
/*    register int lastch; */
    int	    parenlevel;			/* parentheses counter */
/*    long    spacecount; */			/* for breaking long lines */
    short   backslash;			/* boolean */
/*    short   spaceFLAG; */
    short   happy;
/*    short   allowspace; */

/*    int    readcount, writecount, in_fd, out_fd; */

    happy = FALSE;
    parenlevel = 0;
    backslash = FALSE;
/*    spacecount = 0; */
/*    spaceFLAG = FALSE; */

 /* copy text to standard out, looking for comments or EOF.  Collapse
    consecutive white space characters into one.
  */
    while ( !happy ) {
	while (((ch = fgetc (stream)) != EOF) && ch != '%' ) {
	    if ( parenlevel<=0 && (ch==' ' || ch=='\t' || ch=='\n') ) {
		whitecount++;
	    } /* whitespace */
	    else if ( parenlevel ) {
		putchar(ch);
/*		charcount++; */
	    }
	    else {
		if ( whitecount ) {
		    putchar ( WHITE_CHAR );
/*		    charcount = 0; */
		}		/* if whitecount */
		putchar(ch);
/*		lastch = ch; */
/* 		charcount++; */
		whitecount = 0;
	    } /* regular char */
	    if ( ch==LEFTPAREN && !backslash ) parenlevel++;
	    else if ( ch==RIGHTPAREN && !backslash ) parenlevel--;
	    if ( (ch=='\\') && !backslash )	/* allow '\\' */
	        backslash = TRUE;
	    else
		backslash = FALSE;
	} /* while */

       /* handle COMMENTS here */
	if ( ch == '%' ) {
	    if ( parenlevel > 0 ) putchar ( ch );
	    else {	/* read through end of line */
		while ((ch = fgetc (stream)) != '\n' && ch != EOF ) {
		    ;
		} /* while comment text */
		if ( ch == EOF ) putchar ( WHITE_CHAR );
		if ( !whitecount ) whitecount++;
/* 		charcount = 0; */
	    }		/* else all other cases */
	    backslash = FALSE;
	}			/* if this is a comment line */

       /* handle END-OF-FILE here */
	else if (ch == EOF) {
	    putchar (WHITE_CHAR);
	    return ((char *) EOF);
	 } /* if EOF */
    } /* while not happy */
} /* mainfilter */

/* mainfilter returns value sometimes, sometimes not - value ignored */    


/************************************************************************
**
**	main program
**
*************************************************************************/

int main(int argc, char *argv[]) { /* int argc; char *argv[]; */
    FILE *inputfd;
/*    char initfilename[FILENAME_MAX], *operator; */
/*    int filetype; */
    int argument;

/*    short flags;	 */		/* boolean */
    short  keyboard,			/* read from stdin? */
	  morefiles;

    InitGlobals();

    argument = 1;
    if ( argc <= 1 )  {
		keyboard = TRUE;
    } else {
		keyboard = FALSE;
    }
    morefiles = TRUE;
    while ( morefiles ) {
	if ( keyboard ) {
	    keyboard = FALSE;
	    inputfd = stdin;
	}
	else {
	    inputfd = openfile (argv[argument]); /* TEXT */
	}

     /* here is where the main processing goes on */

	if ( inputfd > 0 ) {
	    mainfilter ( inputfd );
	    fclose (inputfd);
	}

	argument++;
	if (argument >= argc && !keyboard )
	    morefiles = FALSE;
    }				/* while morefiles */
	return 0;
} /* main */
