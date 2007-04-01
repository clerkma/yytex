/* Copyright 1990, 1991, 1992, 1994 Y&Y, Inc.
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

/* MODEX - program to interrogate and change COM<i>: registers
*
*  with no arguments displays state of all COM<i>:'s
*  with one argument displays state of specified port (COM<i>:)
*  with two or more arguments, changes state of specified port -
*  arguments then are port, baud, parity, data-bits, stop-bits, retry
*  (not all arguments need to be specified - rest default to old values)
*
*  port should be COM<x>:, baud should be 115200/n for integer n
*  parity should be n (none), o (odd), e (even), s (zero) or m (one), 
*  data-bits should be 5, 6, 7 or 8, while stop-bits should be 1 or 2.
*  retry should be e (error) b (busy) p (busy) or r (ready) -
*  arguments should be separated by spaces, not commmas.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>		/* for call to errno */
#include <conio.h>		/* for _inp, _inpw, and _outp, _outpw */
#include <process.h>	/* for system call */

#define DEBUG 0 	/* non-zero means extra debugging output provided */
#define MAXPORTS 4	/* maximum number of installed ports */
#define HONOR 0 	/* non-zero means trust info on presence of ports */
					/* which means cannot access redirected ports, however */
#define MAXDOSBAUD 19200L	/* maximum baud rate allowed DOS MODE command */
#define SUPPRESSMODE 1	/* suppress output from DOS mode command */

#if HONOR != 0
/* space for addresses of COM<i>: - if found */
int basecom[MAXPORTS]={0, 0, 0, 0};
#else
/* Use default base I/O register addresses for COM1: and COM2: */
int basecom[MAXPORTS]={0x3f8, 0x2f8, 0, 0};
#endif

char commessage[6];		/* COM1: */
char modemessage[32];	/* 19200,n,8,1,b  or  MODE COM1: 19200,n,8,1,b */

int debugflag=0;		/* 1994/July/14 */

char *copyright="\
Copyright (C) 1990-1994  Y&Y, Inc.  (978) 371-3286  http://www.YandY.com\
";

/* Copyright (C) 1990 - 1994  Y&Y, Inc. All rights reserved. (508) 371-3286\ */

/* char *programversion = "MODEX version 0.7"; */
char *programversion = "MODEX version 1.0";

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

/* global variables to communicate between routines */

int control=0;
int parity;
int datan;
int stopn;
long baud=0;
char retry;

/* Assembly language routines next - not used, sinc _inp and _outp work */

/* read I/O register at given address 
unsigned char inreg(int adrs) { 
  _asm {
    mov dx,adrs
	xor ax,ax
    in al,dx
    }
} */

/* write I/O register at address 
void outreg(int adrs, unsigned char num) { 
  _asm {
    mov dx,adrs
    mov al,num
    out dx,al
    }
} */

/* get address at absolute location 
unsigned int getpoint(int adrs) { 
	_asm {
		push ds
		xor ax,ax
		mov ds,ax
		mov bx,adrs
		mov ax,[bx]
		pop ds
	}
} */


void getstate(int port, int flag) { /* get status of given port */
	int base;
/*	int divisor; */
	long divisor;							/* 1994/July/14 */
/*	unsigned char inchar, control; */
/*	int control; */
	int inchar;					/* 1994/July/14 */

	base = basecom[port];	/* get base address of I/O registers */
	if (base == 0) {
		fprintf(stderr, "Port COM%d does not exist\n", port+1);
		exit(3);
	}
	inchar = _inp(base); 	/* clear input character - just for fun */
	control = _inp(base + 3);
#if DEBUG
	printf("control = %x ", (unsigned int) control);
#endif
	_outp(base + 3, (unsigned char) (control | 0x80));
	divisor = _inpw(base);
	_outp(base + 3, control);
/*	baud = 115200 / divisor; */
	if (divisor == 0) baud = 0;			/* 1995/Feb/15 */
	else baud = 115200 / divisor;
	if (control == 0 && flag != 0) {
		fprintf(stderr, "Port COM%d inaccessible\n", port+1);
	}
	datan = (control & 03) + 5;
	stopn = ((control >> 2) & 01) + 1;
	parity = ((control >> 3) & 07);
}

char paritychar(int parity) {
	if (parity % 2 == 0) return 'n';
	else {
		parity = parity >> 1;
		if (parity == 0) return 'o';
		else if (parity == 1) return 'e';
		else if (parity == 2) return 'm';
		else if (parity == 3) return 's';
	}
}

void showstate(int port) { /* get state of port and show it */
	getstate(port, 1);
	if (control == 0 && baud == 0) return;
	printf("COM%d: ", port+1);
	if (baud != 0) printf("%ld", baud);
	if (control != 0) printf(",%c,%d,%d", paritychar(parity), datan, stopn);
	putc('\n', stdout);
}

void setstate(int port, long baud, int parity, int datan, int stopn,
		int enable) { /* set state of given port */
	int base, modem;
/*	int divisor; */
	long divisor;							/* 1994/July/14 */
/*	unsigned char control; */
	int control;							/* 1994/July/14 */

	base = basecom[port];
	if (base == 0) {
		fprintf(stderr, "Port COM%d does not exist", port+1);
		exit(3);
	}

	control = _inp(base + 3);
	_outp(base + 3, (unsigned char) (control | 0x80));
	if (baud > 115200) {
		printf("WARNING: Invalid baud rate %ld\n", baud);
		baud = 115200;
	}
	else if (baud < 150) {
		printf("WARNING: Invalid baud rate %ld\n", baud);
		baud = 150;
	}
	if (115200 % baud != 0) {
		printf("WARNING: Invalid baud rate %ld ", baud);
/*		if (baud == 0) baud == 1; */
		divisor = (2 * 115200 + baud) / (2 * baud); 
/*		if (divisor == 0) divisor = 1; */
		baud = 115200 / divisor; 
		printf("rounded to %ld baud\n", baud);
	}
	divisor = 115200 / baud;
/*	_outpw(base, divisor); */
	_outpw(base, (int) divisor);			/* 1994/July/14 */
	if (datan < 5 || datan > 8) {
		fprintf(stderr, "WARNING: Invalid number of data bits %d\n", datan);
		datan = 9; /*	exit(1); */
	}
	if (stopn < 1 || stopn > 2) {
		fprintf(stderr, "WARNING: Invalid number of stop bits %d\n", stopn);
		stopn = 1; /*	exit(1); */
	}
	if (parity < 0 || parity > 7) {
		fprintf(stderr, "WARNING: Invalid parity specification %d\n", parity);
		parity=0;
	}
	control = (datan - 5) | ((stopn - 1) << 2) | (parity << 3);
	_outp(base+3, control);
	if (enable != 0) {
		modem = _inp(base + 4);
		if (enable > 0)
			_outp(base+4, (unsigned char) (modem | 03)); /* set RTS DTR */
		else _outp(base+4, (unsigned char) (modem & 0374)); /* reset */
	}
}

int getport(char *portname) { /* figure out the number of the port specified */
	int port;
		
	if (strncmp(portname, "COM", 3) != 0 && strncmp(portname, "com", 3) != 0) {
		fprintf(stderr, "Invalid port specification %s", portname);
		exit(1);
	}
	port = portname[3] - '1';
	if (port < 0 || port >= MAXPORTS) {
/*		fprintf(stderr, "Invalid port number %d", port); */
		fprintf(stderr, "Invalid port number %d", port+1);
		exit(1);
	}
	return port;
}

/* is there some way easy way to tell whether port is installed ? */
/* address at 0x400 + 2 * i is zero -- BUT this may also be redirection */

void getportadrs(void) {
	int i, point;
	for(i = 0; i < MAXPORTS; i++) { /* get actual base addresses */
/*		point = getpoint(0x400 + 2 * i);   */
		point = *((unsigned __far *) (0x400L + 2 * i));
#if DEBUG != 0 
		printf("%x ", point);
#endif
#if HONOR != 0
		basecom[i] = point;
#else
		if (point != 0) basecom[i] = point;
#endif 
	}
}

/* int paritycode(char c) { */ /* encode parity specification */
int paritycode(int c) { /* encode parity specification */
#ifdef DEBUGFLAG
	printf("Parity %c\n", c);
#endif
	switch(c) {
		case 'n': case 'N': return 0; break;
		case 'o': case 'O': return 1; break;
		case 'e': case 'E': return 3; break;
		case 'm': case 'M': return 5; break;
		case 's': case 'S': return 7; break;
		default: {
			fprintf(stderr, "WARNING: Invalid parity code %c\n", c);
			return 0;	/*	exit(1);	*/
		}
	}
}

/* determine enable or disable code 
int enablecode(char d) { 
	if (d == 'e') return 1;
	else if (d == 'd') return -1;
	else {
		fprintf(stderr, "WARNING: Invalid enable code %c\n", d);
		exit(5);
	}
} */

/* call DOS with MODE command */

int dosmode(int port, long baud, int parity, int data, int stop, char retry) {
	sprintf(modemessage, "MODE COM%d: %ld,%c,%d,%d,%c",
		port+1, baud, paritychar(parity), data, stop, retry);
#if SUPPRESSMODE != 0
	strcat(modemessage, " > NUL");
#endif
	if (system(modemessage) < 0) {
		perror("Call to DOS MODE command failed ");
		return -1;
	}
	return 0;
} 

/* more elaborate using `spawn' - including error level return 
int dosmode(int port, long baud, int parity, int data, int stop, char retry) {
	sprintf(commessage, "COM%d:", port+1);
	sprintf(modemessage, "%ld,%c,%d,%d,%c",
		baud, paritychar(parity), data, stop, retry);
	return spawnlp(P_WAIT, "MODE", "MODE", commessage, modemessage, NULL); 
} */

void showusage (char *name) {
	printf("%s <port>:<baud>,<parity>,<data>,<stop>,<retry>\n", name);
	printf("\n");
	printf("e.g. %s COM1: 57600,n,8,1\n", name);	
	printf("\n");
	printf("     Arguments are mostly as for DOS MODE command\n");
	printf("\n");
	printf("     <baud>   = 115200 / n for some integer n\n");
	printf("     <parity> = n (none), o (odd), e (even), s (zero), m (one)\n");
	printf("     <data>   = 5, 6, 7 or 8 data bits\n");
	printf("     <stop>   = 1 or 2 stop bits\n");
	printf("     <retry>  = r (ready), b (busy), p (busy), or e (error)\n");
	printf("\n");
	printf("     Omitted trailing arguments default to old settings\n");
	printf("\n");
	printf("     With single arguments, MODEX reports state of specified port\n");
	printf("     Without any arguments, MODEX reports state of all installed ports\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	int i, n, port;
	int enable=0;		/* means don't change state of DSR/DTR */
	int parc=255;
	int nextarg=1;
	int argseen=0;
	char *s;

	getportadrs();			/* get addresses of installed ports */

	retry = '\0';			/* reset retry string */

	if (argc > 7) {
		fprintf(stderr, "WARNING: Too many arguments %d", argc);
/*		exit(1); */
	}

	if (argc == 1) {		/* no arguments - just show state */
		for (i = 0; i < MAXPORTS; i++) {
			if (basecom[i] != 0) {
				showstate(i); 
#if DEBUG
				printf("\n");
#endif
			}
		}
		return 0;					/* that is all */
	}

	if (strchr(argv[nextarg], '?') != NULL) showusage(argv[0]);

	port = getport(argv[nextarg]);			/* which port ? */
	argseen++;								/* seen port argseen = 1 */

/*	Try and deal with various ways for args to get split up here */
	
	getstate(port, 0);						/* get present state as default */
#ifdef DEBUGFLAG
		printf (
"nextarg %d, argseen %d, baud %ld, parc %c, datan %d, stopn %d, retry %c\n",
			nextarg, argseen, baud, parc, datan, stopn, retry);
#endif
	if (strlen(argv[nextarg]) > 4) {		/* maybe COM3:57600, ... */
		n = 0;
		s = argv[nextarg]+4;
#ifdef DEBUGFLAG
		printf("rest[%d] %s\n", nextarg, s);
#endif
		if (*s == ':') s++;
		if (*s == ',') s++;
		if (*s == ' ') s++;
		if (*s != '\0') {
#ifdef DEBUGFLAG
			printf("rest[%d] %s\n", nextarg, s);
#endif
			n = sscanf(s, "%ld,%c,%d,%d,%c", &baud, &parc, &datan,
				&stopn, &retry);
#ifdef DEBUGFLAG
			printf("n %d\n", n);
#endif
			if (n > 0) argseen += n;
		}
	}
	nextarg++;

	while (nextarg < argc && argseen < 6) {
#ifdef DEBUGFLAG
			printf (
"nextarg %d, argseen %d, baud %ld, parc %c, datan %d, stopn %d, retry %c\n",
				nextarg, argseen, baud, parc, datan, stopn, retry);
#endif
		n = 0;
#ifdef DEBUGFLAG
		printf("argv[%d] = %s\n", nextarg, argv[nextarg]);
#endif
		if (argseen == 1) 
			n = sscanf(argv[nextarg], "%ld,%c,%d,%d,%c", &baud, &parc, &datan,
				&stopn, &retry);
		else if (argseen == 2) 
			n = sscanf(argv[nextarg], "%c,%d,%d,%c", &parc, &datan, &stopn,
				&retry);
		else if (argseen == 3) 
			n = sscanf(argv[nextarg], "%d,%d,%c", &datan, &stopn, &retry);
		else if (argseen == 4) 
			n = sscanf(argv[nextarg], "%d,%c", &stopn, &retry);
		else if (argseen == 5) 
			n = sscanf(argv[nextarg], "%c", &retry);
#ifdef DEBUGFLAG
		printf("n %d\n", n);
#endif
		if (n > 0) argseen += n;
		nextarg++;
	}

#ifdef DEBUGFLAG
		printf (
"nextarg %d, argseen %d, baud %ld, parc %c, datan %d, stopn %d, retry %c\n",
		nextarg, argseen, baud, parc, datan, stopn, retry);
#endif

/*	if (argc == 2) showstate(port);	*/		/* show state if only arg */
	if (argseen == 1) {
		showstate(port);				/* show state if only port spec */
		return 0;
	}
#ifdef DEBUGFLAG
	printf("Parc %c %d\n", parc, parc);
#endif
	if (parc != 255) parity = paritycode(parc);
	if (retry != '\0') {		 /* retry  specified - call DOS MODE first */
		if (baud > MAXDOSBAUD)
			dosmode(port, MAXDOSBAUD, parity, datan, stopn, retry);
		else 
			dosmode(port, baud, parity, datan, stopn, retry);
	}
	setstate(port, baud, parity, datan, stopn, enable);
	showstate(port);
	return 0;
}

/* use COMPILET.BAT on this file */

/* c:\c600\bin\cl -AT -W2 -Od  c:\c\modex.c */

