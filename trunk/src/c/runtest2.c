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

int debugflag=1;

/* Do run encoding per row (rather than image) --- easier, if not optimal */

/* This basically has two states: */
/* (1) accumulating non-run (runflag == 0) */
/* (2) accumulating run (runflag != 0) */
/* Here (1) is the starting state */
/* Dumps out when (i) state changes (ii) 128 bytes seen (iii) end input */

void dumprun(FILE *output, int nlen, int previous) {
	if (nlen == 0) return;
	if (nlen < 3) printf("ERROR: run %d < 3\n", nlen);
	if (nlen > 128) printf("ERROR: run %d > 128", nlen);
	if (debugflag) printf("Run %d ", nlen);
	else putc(257 - nlen, output);
	putc(previous, output);	
	if (debugflag) putc(' ', output);
}

void dumpnonrun(FILE *output, int nlen, unsigned char *buffer) {
	int i;
	if (nlen == 0) return;
	if (nlen > 128) printf("ERROR: nonrun %d > 128", nlen);
	if (debugflag) printf("NonRun %d ", nlen);
	else putc(nlen - 1, output);
	for (i = 0; i < nlen; i++) putc(buffer[i], output);
	if (debugflag) putc(' ', output);
}

void runencode (FILE *output, unsigned char *s, unsigned long width) {
	int runflag;			/* non-zero if accumulating run */
	int previous;			/* character that appears to be repeating */
	int repeat;				/* how many times we have seen previous */
	int bcount;				/* count of bytes buffered up (s-new) */
	unsigned char *new;		/* points to part of string not yet dumped */
	int n, k, c;

	n = (int) width;
	k = 0;
	bcount = 0;
	new = s+1;
	repeat = 0;	
	previous = -1;
	runflag = 0;			/* start in non run accumulating state */

	while (k < n) {
		c = *s;
		if (runflag) {					/* accumulating a run ? */
			if (c == previous) {		/* continue the run ? */
				repeat++;
				if (repeat >= 128) {	/* run too long ? */
					dumprun(output, repeat, previous);
					runflag = 0;
					previous = -1;
					repeat = 0;
					bcount = 0;
					new = s+1;
				}
			}
			else {						/* c != previous --- end of run */
				dumprun(output, repeat, previous);
				runflag = 0;
				previous = c;
				repeat = 1;
				bcount = 0;
				new = s;
				bcount++;
			}
		}
		else {						/* runflag == 0 accumulating a non run */
			if (c == previous) {
				repeat++;
				if (repeat >= 4) {	/* end of non-run */
					dumpnonrun(output, bcount-repeat+1, new);
					bcount = 0;
					new = s+1;
					runflag = 1;	/* switch to accumulating run state */
				}
			}
			if (runflag == 0) {		/* runflag still 0 ? */
				bcount++;
				if (bcount >= 128) {	/* accumulated too much ? */
					dumpnonrun(output, bcount, new);
					bcount = 0;			/* and stay in non run mode */
					new = s+1;
					repeat = 0;
					previous = -1;
				}
				else {
					previous = c;
					if (repeat == 0) repeat = 1;
				}
			}
		}
		s++;
		k++;
	}	/* end of while loop */
	if (runflag) {
		dumprun(output, repeat, previous);
	}
	else {
		dumpnonrun(output, bcount, new);
	}
	if (debugflag) putc('\n', output);
}

char *test1="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
char *test2="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
char *test3="abcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyz";
char *test4="aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkk";

char *test11="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
char *test12="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
char *test13="abcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyz";
char *test14="aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkkaaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkk";

char *test21="aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
char *test22="abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
char *test23="abcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzkkkkkabcdefghijklmnopqrstuvwxyz";
char *test24="aaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkkaaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkkaaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkkaaaaabbbbbcccccdddddeeeeefffffggggghhhhhiiiiijjjjjkkkkk";

char *test31="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"; 
char *test32="abababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababab";
char *test33="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxabcedfghijkl"; 
char *test34="ababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababababxxxxxxxxxxxx";


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
	printf("test21\n");
	runencodetest (test21);
	printf("test22\n");
	runencodetest (test22);
	printf("test23\n");
	runencodetest (test23);
	printf("test24\n");
	runencodetest (test24);
	printf("test31\n");
	runencodetest (test31);
	printf("test32\n");
	runencodetest (test32);
	printf("test33\n");
	runencodetest (test33);
	printf("test34\n");
	runencodetest (test34);
	return 0;

}	
