/* Copyright 1990, 1991, 1992 Y&Y, Inc.
   Copyright 2007 TeX Users Group */

/*
 * (C) 1990 by Adobe Systems Incorporated. All rights reserved.
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

/*
 * PSlinewrap.c				greid Tue Jun  6 14:02:26 1989
 *
 *
 * (c) 1989 Adobe Systems Incorporated. All Rights Reserved.
 *
Edit History:
    Glenn Reid <ps-file-person@adobe.com> Tue Jun  6 14:02:26 1989  
        original version
	David Osborne <cczdao@uk.ac.nott.clan> Tue Oct 17 11:22:57 BST 1989
	    for vms, create new generation of output file
	    change second call to strncpy() to strcpy()

    SYNOPSIS:
	PSlinewrap [ -<longerthan> ] [ <filename> ]

    EXAMPLES:
	PSlinewrap myfile.ps
	PSlinewrap -255 myfile.ps > tmp
	 
    EXPLANATION:
    Wraps PostScript lines longer than the supplied limit by breaking
    the line.  The default line limit is 78, to conform to standard
    80-column terminals and the programs that line-wrap for them.
    The line breaks according to the following algorithm:

	* At a space, if there is one on the line somewhere.
	* with the \ notation, if it is in the middle of a string body.
	* at the cutoff point, if not in the middle of a string and
	  there are no spaces (hopefully it is a hex string)
    
 */

#include <stdio.h>

#define	DEFAULT 78
#define TRUE 1
#define FALSE 0

int	maxcolumn;	/* column to count lines longer than (good English!) */
long	counter;	/* byte counter */	
long	linecount;	/* line number */
long	level;		/* level of string paren nesting */
char	buff[1024];	/* char buffer */
char	tmpbuff[1024];	/* char buffer */
short	comment;	/* boolean (is it a PS comment? ) */

/*************************** main *************************/

main ( argc, argv ) int argc; char *argv[];
{
    FILE *inputfd;
    short quit;
    int ch, lastchar;
    int position;

    quit = FALSE;
    linecount = 0;
    counter = 0;
    level = 0;
    maxcolumn = DEFAULT;
    comment = FALSE;

  /* check command-line arguments */
    switch ( argc ) {
    case 0:
    case 1: {
	inputfd = stdin;
	break;
    }
    case 3: {
	if ( argv[1][0] == '-' ) {
	    sscanf ( argv[1], "-%d", &maxcolumn );
	    if ( maxcolumn < 0 ) maxcolumn = -maxcolumn;
	} else {
	    fprintf ( stderr, "Invalid switch: %s\n", argv[1] );
	    exit ( 1 );
	}
    }
    case 2:
    default: {
	inputfd = fopen ( argv[argc-1], "r" );
	if ( inputfd <= 0 ) {
	    fprintf ( stderr, "Cannot open %s.\n", argv[argc-1] );
	    exit ( 1 );
	}
#ifdef vms			/* ensure stdout goes to new generation of
				   input file  ---dao */
	if (freopen(argv[argc-1], "w", stdout) == NULL) {
	    fprintf ( stderr, "Cannot open new generation of %s for output.\n",
		argv[argc-1] );
	    exit ( 1 );
	}
#endif /*vms*/
	break;
    }
    } /* switch */

  /* main loop of program */
    fprintf (stderr,"Checking for lines longer than %d bytes...\n",maxcolumn);
    lastchar = ' ';
    ch = getc ( inputfd );
    while ( ch != EOF && !quit ) {
	if ( ch != '\n' ) {
	    buff[counter] = ch;
	    buff[counter+1] = 0;
	    counter++;
	    if ( lastchar != '\\' && ch == '(' ) level++;
	    if ( lastchar != '\\' && ch == ')' ) level--;
	    if ( ch == '%' && level == 0 ) comment = TRUE;
	    if ( counter >= maxcolumn ) {
		/* search backward and only break the line at a space */
		for ( position=counter; position >= 0; position-- ) {
		    if ( buff[position] == ' ' ) {
			strncpy ( tmpbuff, buff, position + 1 );
			tmpbuff [ position ] = 0;	/* nix the space */
			printf ( "%s", tmpbuff );
			if ( level != 0 ) printf ( "\\" );
			printf ( "\n" );
			if ( comment ) printf ( "%%%%+ " );
			break;
		    }
		}
		/* if no spaces are found, break the line and hope */
		if ( position <= 0 ) {
		    if ( level <= 0 ) {
			printf ( "%s\n", buff );
		    } else {
			printf ( "%s\\\n", buff );
		    }
		    counter = 0;
		    buff[counter] = 0;
		} else {
		    strcpy ( tmpbuff, (char *)(buff+position) ); /* was strncpy ---dao */
		    strcpy ( buff, tmpbuff );
		    counter = strlen ( buff );
		    buff[counter] = 0;
		}
		linecount++;
	    }
	} else {
	    comment = FALSE;
	    if ( counter ) {
		printf ( "%s\n", buff );
	    }
	    counter = 0;
	}
	if ( lastchar == '\\' && ch == '\\' ) ch = ' ';
	lastchar = ch;
	ch = getc ( inputfd );
    } /* while */
    if ( linecount ) {
	fprintf ( stderr, "PSlinewrap: %d lines wrapped.\n", linecount );
    }
    
} /* main */

