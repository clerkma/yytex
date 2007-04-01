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

#define MAXSTACK 256

#define MAXLINE 512

#define MAXCHARNAME 64

#define MAXCHRS 256

int pstack[MAXSTACK];

char line[MAXLINE];

int stackindex=0;

int stackflag=0;

int debugflag=0;

int showbinary=0;

int showtoken=0;

int showencoding=0;

int showname = 0;

int absolute=1;

int xold, yold;

int chr=0;

char charname[MAXCHARNAME]="";

char *encoding[MAXCHRS];

/*************************************************************************/


/* simple PS emulator */

void push(int n) {
	if (stackflag) printf("push (%d) ", n);
	if (stackindex >= MAXSTACK) {
		printf("STACK OVERFLOW\n");
		exit(1);
	}
	pstack[stackindex++] = n;
}

int pop(void) {
	int ret;
	if (stackindex <= 0) {
		printf("STACK UNDERFLOW\n");
		exit(1);
	}
	ret= pstack[--stackindex];
	if (stackflag) printf("pop (%d) ", ret);
	return ret;
}

int top(void) {
	if (stackindex <= 0) {
		printf("STACK UNDERFLOW\n");
		exit(1);
	}
	return pstack[stackindex-1];
}

void resetstack(void) {
	stackindex=0;
}

void checkstack(void) {
	if (stackindex != 0) printf("STACK NOT EMPTY %d\n", stackindex);
}

void exch(void) {
	int a, b;
	b = pop();
	a = pop();
	push(b);
	push(a);
}

void dup(void) {
	int a;
	a = pop();
	push(a);
	push(a);
}

int eq(void) {
	int a, b;
	b = pop();
	a = pop();
	return (a == b);
}

int lt(void) {
	int a, b;
	b = pop();
	a = pop();
	return (a < b);
}

int gt(void) {
	int a, b;
	b = pop();
	a = pop();
	return (a > b);
}

void add(void) {
	int a, b;
	b = pop();
	a = pop();
	push(a+b);
}

void sub(void) {
	int a, b;
	b = pop();
	a = pop();
	push(a-b);
}

void mul(void) {
	int a, b;
	b = pop();
	a = pop();
	push(a*b);
}


void neg(void) {
	int a;
	a = pop();
	push(-a);
}

void moveto(void) {
	int x, y;
	y = pop();
	x = pop();
	printf("%d %d m\n", x, y);
	xold = x; yold = y;
}

void rmoveto(void) {
	int x, y;
	y = pop();
	x = pop();
	if (absolute) printf("%d %d m\n", xold+x, yold+y);
	else printf("%d %d rmoveto\n", x, y);
	xold += x; yold += y;
}

void rlineto(void) {
	int x, y;
	y = pop();
	x = pop();
	if (absolute) printf("%d %d l\n", xold+x, yold+y);
	else printf("%d %d rlineto\n", x, y);
	xold += x; yold += y;
}

void rcurveto(void) {
	int x1, y1, x2, y2, x3, y3;
	y3 = pop();
	x3 = pop();
	y2 = pop();
	x2 = pop();
	y1 = pop();
	x1 = pop();
	if (absolute)
		printf("%d %d %d %d %d %d c\n", xold+x1, yold+y1,
			   xold+x2, yold+y2, xold+x3, yold+y3);
	else printf("%d %d %d %d %d %d rcurveto\n", x1, y1, x2, y2, x3, y3);
	xold += x3; yold += y3;
}

void closepath(void) {
	if (absolute) printf("h\n");
	else printf("closepath\n");
}

void nop(void) {
	printf("NOP?\n");
}

void concat(void) {
	printf("CONCAT?\n");
}

void setcachedevice(void) {
	int wx, wy, xll, yll, xur, yur;
	yur = pop();
	xur = pop();
	yll = pop();
	xll = pop();
	wy  = pop();
	wx  = pop();
	if (absolute) printf("%d %d %% %s\n", chr, wx, charname);
	else printf("%d %d %d %d %d %d setcachedevice\n", wx, wy, xll, yll, xur, yur);
	   
}

void gsave(void) {
	printf("GSAVE?\n");
}

void setlinewidth(void) {
	int w;
	w = pop();
	printf("%d setlinewidth\n", w);
}

void fill(void) {
	printf("fill\n");
}

void eofill(void) {
	printf("eofill\n");
}

void stroke(void) {
	printf("stroke\n");
}

void setlinecap(void) {
	int cap;
	cap = pop();
	printf("%d setlinecap\n", cap);
}

void setlinejoin(void) {
	int join;
	join = pop();
	printf("%d setlinejoin\n", join);
}

/*************************************************************************/

void UVec(void) {
	int c=pop();
	if (debugflag) printf("%d UVec\n", c);
	switch(c) {
		case 0:					/*  233 */
			rmoveto();
			break;
		case 1:					/* 234 */
			rlineto();
			break;
		case 2:					/*  235 */
			rcurveto();
			break;
		case 3:					/*  236 */
			nop();
			break;
		case 4:					/*  237 */
			concat();			/* ]concat */
			push(0);
			push(0);
			moveto();			/* 0 0 moveto */
			break;
		case 5:					/*  238 */
			setcachedevice();	/* setcachedevice */
			push(0);
			push(0);
			moveto();			/* 0 0 moveto */
			break;
		case 6:					/*  239 */
			push(100);
			mul();
			add();				/* 100 mul add */
			break;
		case 7:					/* 240 */
		case 8:					/* 241 */
		case 9:					/* 242 */
		case 10:				/* 243 */
		case 11:				/* 244 */
		case 12:				/* 245 */
		case 13:				/* 246 */
			nop();
			break;
		case 14:				/* 247 */
			closepath();
			break;
		case 15:				/* 248 */
			gsave();			/* gsave[ */
			break;
		case 16:				/* 249 */
			setlinewidth();
			break;
		case 17:				/* 250 */
			fill();				/* {{fill}} */
			break;
		case 18:				/* 251 */
			eofill();			/* {{eofill}} */
			break;
		case 19:				/* 252 */
			stroke();			/* {{fill}} */
			break;
		case 20:				/* 253 */
			push(0);
			push(0);
			moveto();
			break;
		case 21:				/* 254 */
			setlinecap();
			break;
		case 22:				/* 255 */
			setlinejoin();
			break;
		default:
			printf("INVALID CODE %d in UVec\n", c);
			break;		
	}
}

void SubProcs () {
	int c = pop();
	if (debugflag) printf("%d SubProcs ", c);
	switch(c) {
		case 0:
			push(100);
			sub();
			push(0);
			break;
		case 1:
			push(233);
			sub();
			UVec();
			push(0);
			break;
		case 2:
			(void) pop();
			push(3939);
			push(6);
			break;
		case 3:
			(void) pop();
			push(4194);
			push(8);
			break;
		case 4:
			push(215);
			sub();
			neg();
			push(256);
			mul();
			push(100);
			add();
			push(10);
			break;
		case 5:
			push(216);
			sub();
			push(256);
			mul();
			push(99);
			add();
			push(11);
			break;
		case 6:
			push(256);
			mul();
			add();
			push(7);
			break;
		case 7:
			add();
			neg();
			push(0);
			break;
		case 8:
			push(256);
			mul();
			add();
			push(9);
			break;
		case 9:
			add();
			push(0);
			break;
		case 10:
			add();
			neg();
			push(0);
			break;
		case 11:
			add();
			push(0);
			break;
		default:
			printf("INVALID CODE %d in SubProcs\n", c);
			break;
	}
}

void UCS(void) {
	exch();
	dup();
	push(0);
	if (eq()) {
		(void) pop();
		dup();
		push(200);
		if (lt()) push(0);				/* < 200 ==> SubProcs 0 */
		else {
			dup();
			push(232);
			if (gt()) push(1);			/* > 232 ==> SubProcs 1 ! */
			else {
				dup();
				push(200);
				if (eq()) push(2);		/* = 200 ==> SubProcs 2 */
				else {
					dup();
					push(232);
					if (eq()) push(3);	/* = 232 ==> SubProcs 3 */
					else {
						dup();
						push(216);
						if (lt()) push(4);	/* < 216 ==> SubProcs 4 */
						else push(5);		/* else  ==> SubProcs 5 */
					}
				}
			}
		}
	}
	else if (debugflag) printf("NOT ZERO in UVec %d\n", top());
	SubProcs();
}

/*****************************************************************************/

int gettoken(char *token, int nlen, FILE *input) {
	char *s = token;
	int c;

	*s = '\0';
	while ((c = getc(input)) != EOF && c <= ' ') ;
	if (c == EOF) return -1;
	ungetc(c, input);
	while ((c = getc(input)) != EOF && c > ' ') {
		if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
			(c >= '0' && c <= '9') || c == '.' || c == '/') *s++ = (char) c;
		else {
			if (showtoken) printf("BREAK ON %c %d\n", c, c);
			break;
		}
	}
	*s++ = '\0';
	if (c == EOF) return -1;
	if (showtoken) printf("TOKEN: %s\n", token);
	return (s - token);
}

int lookup (char *name) {
	int k;
	for (k = 0; k < MAXCHRS; k++)
		if (strcmp(encoding[k], name) == 0) return k;
	return -1;
}


/*************************************************************************/

void readencoding(FILE *input) {
	int k;
	for (k = 0; k < MAXCHRS; k++) encoding[k] = "";

	while (fgets(line, sizeof(line), input) != NULL) {
		if (strncmp(line, "Encoding", 8) ==0) break;
	}
	while (fgets(line, sizeof(line), input) != NULL) {
		if (*line == '%' || *line == '\n') continue;
		if (strncmp(line, "pop", 3) == 0) break;
		if (strncmp(line, "CharacterDefs", 13) == 0) break;
		if (strncmp(line, "dup", 3) ==0) {
			if (sscanf(line, "dup %d /%s put", &chr, charname) == 2) {
				if (chr >= 0 && chr < MAXCHRS)
					encoding[chr] = strdup(charname);
			}
			else printf("BAD ENCODING: %s", line);
		}
	}
	if (showencoding) {
		for (k = 0; k < MAXCHRS; k++)
			if (strcmp(encoding[k], "") != 0)
				printf("%d\t%s\n", k, encoding[k]);
	}
}

int scantonextchar(FILE *input) {
	for (;;) {
		if (gettoken(line, sizeof(line), input) < 0) return -1;
		if (strcmp(line, "dup") == 0) break;
	}
	if (gettoken(line, sizeof(line), input) < 0) return -1;
	if (*line != '/') {
		printf("BAD CHAR NAME %s\n", line);
		return -1;
	}
	strcpy(charname, line+1);
	chr = lookup(charname);
	if (showname) printf("chr %d name %s\n", chr, charname);
	return chr;
}

int getbinary(FILE *input) {
	int c, d, n;
	while ((c = getc(input)) != EOF && c <= ' ') ;
	if (c == '>') return -1;
	if (c >= '0' && c <= '9') c = c - '0';
	else if (c >= 'A' && c <= 'F') c = c - 'A' + 10;
	else if (c >= 'a' && c <= 'f') c = c - 'a' + 10;
	else printf("BAD CHARACTER %d\n", c);
	while ((d = getc(input)) != EOF && d <= ' ') ;
	if (d == '>') {
		printf("ERROR > in odd place\n");
		return -1;
	}
	if (d >= '0' && d <= '9') d = d - '0';
	else if (d >= 'A' && d <= 'F') d = d - 'A' + 10;
	else if (d >= 'a' && d <= 'f') d = d - 'a' + 10;
	else printf("BAD CHARACTER %d\n", d);
	n = (c << 4) | d;
	if (showbinary) printf("BINARY %02d %02X\n", n, n);
	return n;
}

/*****************************************************************************/

void processfile(FILE *input) {
	int c, n;
	readencoding(input);
	for (;;) {
		if (scantonextchar(input) < 0) {
			if (strcmp(charname, ".notdef") == 0) {
				if (scantonextchar(input) < 0) break;
			}
			else break;
		}
		if (showname) printf("WORKING on %d %s\n", chr, charname);
		while ((c = getc(input)) != EOF && c != '<') ;
		if (c == EOF) break;
		putc('\n', stdout);
		putc(c, stdout);
		putc('\n', stdout);
		if (debugflag) printf("\n");
		resetstack();
		push(0);
		for (;;) {
			n = getbinary(input);
			if (n < 0) break;
			push(n);
			if (debugflag) putc('\n', stdout);
			if (debugflag) printf("%d UCS ", n);
			UCS();
		}
		pop();
		checkstack();
		if (c == '>') putc(c, stdout);
		putc('\n', stdout);
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

int main(int argc, char *argv[]) {
	FILE *input, *output;
	char infile[FILENAME_MAX];
	char outfile[FILENAME_MAX];
	if (argc < 2) strcpy(infile, "f24200.pfa");
	else strcpy(infile, argv[1]);
	extension(infile, "pfa");
	if ((input = fopen(infile, "r")) == NULL) {
		perror(infile);
		exit(1);
	}
	printf("Processing %s\n", infile);
	processfile(input);
	fclose(input);
	return 0;
}
