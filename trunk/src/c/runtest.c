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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Do run encoding per row --- easier, even if not optimal */

/* This basically has two states: (1) is the starting state */
/* (1) accumulating non-run (runindex > 0) */
/* (2) accumulating run (repeat >= 4) */
/* It dumps out when (i) state changes (ii) 128 bytes seen (iii) end input */

void runencode (FILE *output, unsigned char *s, unsigned long width) {
	unsigned char runbuffer[128];	/* buffer of non-run bytes */
	int runindex;		/* index into the above */
	int previous;		/* character that appears to be repeating */
	int repeat;			/* how many times we have seen previous */
	int nonrun;			/* part of buffer that is not in current run */
	int skipflag;
	int i, n, k, c;

	n = (int) width;
	k = 0;
	runindex = 0;
	repeat = 0;	
	previous = -1;
	skipflag = 0;
	while (k < n) {
		c = *s++;
		k++;
		if (c == previous) {
			repeat++;
/*			if we found 4 copies, switch to accumulate run mode */
			if (repeat >= 4 && runindex > 0) {		/* spit out buffer */
/*				dump everything but the trailing repetitions */
				nonrun = runindex - repeat + 1;
				if (nonrun > 0) {					/* only if there is some */
/*					putc (nonrun-1, output); */
					printf ("NonRun (A) %d ", nonrun);	/* debugging */
					for (i = 0; i < nonrun; i++) putc(runbuffer[i], output);
					putc(' ', output);				/* debugging */
				}
				runindex = 0;						/* clear non run buffer */
				skipflag = 1;						/* don't put in non-run */
			}
/*			need to dump out if 128 byte repetition */
			if (repeat >= 128) {
/*				putc(257 - repeat, output); */
				printf ("Run (A) %d ", repeat);		/* debugging */
				putc(previous, output);
				putc(' ', output);				/* debugging */
				repeat = 0;
				previous = -1;
			}
		}
		else {									/* c != previous */
/*			if (repeat > 1) { */
			if (repeat >= 4) {			
/*				putc(257 - repeat, output); */
				printf ("Run (B) %d ", repeat);
				putc(previous, output);
				putc(' ', output);				/* debugging */
				repeat = 0;
				previous = -1;
			}
			else {								/* start new run */
				repeat = 1;
				previous = c;
			}
		}	 /* experiment */
		if (skipflag != 0) skipflag = 0;
		else {
		if (repeat < 4) {						/* accumulate non run state */
			runbuffer[runindex++] = (unsigned char) c;
/*			need to dump out if 128 bytes accumulated */
			if (runindex >= 128) {
/*				putc (runindex-1, output); */
				printf ("NonRun (B) %d ", runindex);/* debugging */
				for (i = 0; i < runindex; i++) putc(runbuffer[i], output);
				putc(' ', output);				/* debugging */
				runindex = 0;
				repeat = 0;
				previous = -1;
			}
		} 	/* experiment */
		}
	} /* while k < n */
/*	clean up remains at end of input */
	if (runindex > 0) {					/* accumulating non run state */
/*		putc (runindex-1, output); */
		printf ("NonRun (C) %d ", runindex);
		for (i = 0; i < runindex; i++) putc(runbuffer[i], output);
		putc(' ', output);				/* debugging */
		runindex = 0;
		repeat = 0;
		previous = -1;
	}
/*	if (repeat > 1) { */
	if (repeat >= 4) {					/* accumulating run state */
/*		putc(257 - repeat, output); */
		printf ("Run (C) %d ", repeat);		/* debugging */
		putc(previous, output);
		putc(' ', output);				/* debugging */
		repeat = 0;
		previous = -1;
	}
	putc('\n', stdout);					/* debugging */
}

char *test1="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
char *test2="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
char *test3="abcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyz";
char *test4="aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkk";

char *test11="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
char *test12="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
char *test13="abcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyz";
char *test14="aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkkaaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkk";

void runencodetest (char *test) {
	runencode (stdout, (unsigned char *) test, strlen(test));
}

int main (int argc, char *argv[]) {
	printf("test1\n");
	runencodetest (test1);
	printf("test2\n");
	runencodetest (test2);
	printf("test3\n");
	runencodetest (test3);
	printf("test4\n");
	runencodetest (test4);
	printf("test11\n");
	runencodetest (test11);
	printf("test12\n");
	runencodetest (test12);
	printf("test13\n");
	runencodetest (test13);
	printf("test14\n");
	runencodetest (test14);
	return 0;
}
