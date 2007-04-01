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

/* Program SERIAL - output files to serial ports */
/* serial.c 1991 Berthold K.P. Horn, Y&Y --- All rights reserved */
/* supports both software (XON/XOFF) and hardware (DTR/DSR) flow control */

#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>

/* following two constants must be power of two */

#define	INBUFSIZE  16384
#define	OUTBUFSIZE 16384

#define MAXPORTS 4			/* maximum number of serial ports */

/* #define HONOR */	/* non-zero means trust info on presence of ports */
					/* which means cannot access redirected ports, however */

#define CONTROLC 3
#define CONTROLD 4

/* int waittime=30; */	/* max wait (in sec) for control-D acknowledgement */
int waittime=60;	/* max wait (in sec) for control-D acknowledgement */

#ifdef HONOR
/* space for addresses of COM<i>: - if found */
int basecom[MAXPORTS]={0, 0, 0, 0};
#else
/* Use default base I/O register addresses for COM1: and COM2: */
/* int basecom[MAXPORTS]={0x3f8, 0x2f8, 0x3e8, 0x2e8}; */
int basecom[MAXPORTS]={0x3f8, 0x2f8, 0, 0};
#endif

int verboseflag=0;
int traceflag=0;			/* show extra information */
int showflag=0;				/* show data send to printer as well */

int detailflag=0;			/* details in showusage */
int outputflag=0;			/* non-zero => next argument is destination */
int waitingflag=0;			/* non-zero => next argument is wait time */
int irqflag=0;				/* non-zero => next argument is IRQ */
int baudflag=0;				/* non-zero => next argument is baud rate */

int getportflag=1;			/* non-zero => get port address */
int honorflag=0;			/* honor given address even if zero */

volatile int bAbort=0;				/* set by user typing control-C */

/* communications parameters */

int setcommun=0;			/* mess up communications parameters */

unsigned long baud=0;		/* (115200 / n) for integer n */
unsigned int databits=8;	/* 5, 6, 7, or 8 */
unsigned int stopbits=1;	/* 1 or 2 */
unsigned int parity=0;		/* 0 = none, 1= odd, 3 = even */

unsigned long oldbaud;		/* read from port */

char *destination="COM1";	/* char *destination="AUX"; */

unsigned int base=0x3F8; 	/* 0x3F8 = COM1, 0x2F8 = COM2 */
unsigned int irq=4;			/* 4 = COM1, 3 = COM2 */

unsigned int userirq=0;		/* if user specifies irq */

char *programversion = "SERIAL version 0.9.9";

char *compilefile = __FILE__;
char *compiledate = __DATE__;
char *compiletime = __TIME__;

char *copyright = "\
Copyright (C) 1990--1993 Y&Y, Inc. All rights reserved. (978) 371-3286\
";

/* #define HASHCODE 6317301 */	/* out of date */
#define HASHCODE 8309556

/* Copyright (C) 1990, 1991 Berthold K.P. Horn, Y&Y. All rights reserved.\ */

/* buffer for characters coming from printer */

static char inputchars[INBUFSIZE];
volatile int inputcount=0;
volatile int inputindex=0;
volatile int inputoverflow=0;

/* buffer for characters going to the printer */

static char outputchars[OUTBUFSIZE];
volatile int outputcount=-1;
volatile int outputindex=0;
volatile int outputoverflow=0;

volatile int outputflow=0;	/* set by receive C-S - reset by receive C-Q */
						/* non-zero means don't send any more */
						/* actually not used anymore ... */

volatile int dsrstate=0;	/* set by interrupt handler */
						/* zero means don't send anymore */
						/* actually not used anymore ... */

/* following just for tracing and debugging */

int cscount=0;			/* count C-S from printer */
int cqcount=0;			/* count C-Q from printer */
int dsrdown=0;			/* DSR 1 -> 0 */
int dsrup=0;			/* DSR 0 -> 1 */
int dsrsame=0;			/* should always be zero ! */

int stopflag=0;			/* on when end of input file is seen */

/* saved interrupt vector - MUST restore ! */

/* void (__interrupt __far *oldintvector)();  */	/* 1992/Nov/24 */
void (__cdecl __interrupt __far *oldintvector)(); 

unsigned int intvectornum;	/* interrupt vector number - computed from IRQ */

int inputchar, outputchar;	/* current input and output characters */

/* char *logname="printer.log";	*/	/* default log file name */

char logname[FILENAME_MAX]="printer.log";	/* log file name */

char logbak[FILENAME_MAX];			/* file name for backup file renaming */

int wantlog=1;				/* do we want logging ? */
int logging=0;				/* are we actually logging ? */
int logcount=0;				/* number of characters in logging file */
							/* delete file at end if zero */

int wantcontrold=-1;		/* send control-D at end */
int waitforeoj=-1;			/* wait for end of job from printer */
int endofjob=0;				/* set when control-D is received */

int outbuffering=-1;		/* want output to be buffered */

int wantcpyrght=1;

int control;				/* control word from serial port */

static char fn_in[FILENAME_MAX];

FILE *input;					/* current input file - if any */
FILE *logfile;					/* output log file - if any */

/* void __interrupt __far interrupthandler(void) { */ /* the interrupt handler */
void __cdecl __interrupt __far interrupthandler(void) { /* the interrupt handler */
	int interruptid, linestatus, modemstatus, oldstate, ch;
	int wakeup = 0, gosleep = 0, loopcount=0;

	for(;;) {
		interruptid = _inp(base+2) & 7;
		linestatus = _inp(base+5);	/* also clears error condition	... */
		if (interruptid == 0) {			/* change in modem status */
/*			probably because DSR has changed - read it out */
			modemstatus = _inp(base+6);
			if (modemstatus & 2 != 0) { /* only if DSR has changed */
				oldstate = dsrstate;
				if ((modemstatus & 0x20) != 0) {
					dsrstate = 1;
					wakeup++;
					dsrup++;
				}
				else {
					dsrstate = 0;
					gosleep++;
					dsrdown++;
				}
				if (dsrstate == oldstate) dsrsame++;
			}
		}
		if ((interruptid == 4)) {
/*		if ((interruptid == 4) || ((linestatus & 1) != 0)) { */
/* a character has been received - or */
/* a character has been received (RXRDY) */
/*			so go read a character  */
			ch = _inp(base);		/* OK - read the character */
			if (ch == 19) {		/* control S - muffle output */
				outputflow=1;
				gosleep++;
				cscount++;
			}
			else if (ch == 17) {  /* control Q - restart output */
				outputflow = 0;
				wakeup++;
				cqcount++;
			}
			else {				/* not XON or XOFF - normal character */
				if (inputcount < INBUFSIZE) {
					inputchars[inputindex] = (char) ch;
					inputindex = (inputindex+1) & (INBUFSIZE-1);
					inputcount = inputcount+1;
				}
				else inputoverflow = 1;
/*				if (inputcount == (INBUFSIZE * 3) / 4) */
				if (inputcount == (INBUFSIZE / 4) * 3)
					_outp(base+4, 8); /* turn off DSR and RTS */
			} 
		}
		if ((interruptid == 2)) {
/*		if ((interruptid == 2) || ((linestatus & 0x20) != 0)) {   */
/* ready to send a character - or */
/* ready to send a character (TXRDY) - or */
/*			see if need to send next output character ? */
			if (outputcount > 0) {
				_outp(base, outputchars[outputindex]);
				outputindex = (outputindex+1) & (OUTBUFSIZE-1);
				outputcount = outputcount-1;
			}
			else outputcount = -1;
		}
		loopcount++;
		if (((interruptid & 1) == 1) || (loopcount == 4)) break; 
/*		if (((interruptid & 1) == 1) || (loopcount == 3)) break; */
/*		no interrupts pending - or - gone around too often ! */
	}
	if (gosleep != 0) _outp(base+1, 8 + 1); /* disable ready to send */
	if (wakeup != 0) _outp(base+1, 8 + 2 + 1); /* reenable ready to send */
	if (irq<8) _outp(0x20, 0x20); /* reset PIC */
/*  what if irq > 8 ? */
}

void showkey(int key) {
	if (key >= 512) {
		putchar('N'); putchar('-'); showkey(key - 512);
	}
 	else if (key >= 256) {
		putchar('F'); 	putchar('-'); showkey(key - 256);
	}
 	else if (key >= 128) {
		putchar('M'); 	putchar('-'); showkey(key - 128);
	}
 	else if (key < 32) {
		if (key == '\n') putchar('\n');
		else if (key == '\r') putchar('\r');
		else {
			putchar('C'); 	putchar('-'); showkey(key + 64);	
		}
	}
	else putchar(key);
}

int getkey(void) {					/* try and get a key from keyboard */
	int k, key;

/*	if (traceflag != 0) printf("*"); */
	key = 0;					/* return 0 if nothing there */
	if (_kbhit() != 0) {			/* see if character available */
		k = _getch();
		if (k == 0) key = 256 + _getch();			/* compound character */
		else if (k == 0xE0) key = 512 + _getch();	/* compound character */
		showkey(k);
		key = k;
	}
	return key;
}

#ifdef IGNORE
void extension(char *fname, char *ext) { /* supply extension if none */
    if (strchr(fname, '.') == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
}

void forceexten(char *fname, char *ext) { /* change extension if present */
	char *s;
    if ((s = strchr(fname, '.')) == NULL) {
		strcat(fname, "."); strcat(fname, ext);
	}
	else strcpy(s+1, ext);		/* give it default extension */
}
#endif

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

void restoreint(void) {
	int flags;
	int k;

	for (k = 0; k < 16; k++) {
		_outp(base+4, 0);			/* disable interrupts, clear RTS and DTR */
		flags = _inp(base+4);
		if (flags == 0) break;
	}
	if (flags != 0)
		printf("Tried to turn off interrupts, but failed %d\n", flags);
	else if (k == 0) {
		if (traceflag != 0)	printf("Turned off interrupts after one try\n");
	}
	else printf("Turned off interrupts after %d tries\n", k+1);

	_dos_setvect(intvectornum, oldintvector);	/* restore interrupt vector */
	if (irq<8)									/* restore PIC */
		_outp(0x21, _inp(0x21) | (1 << irq));
	else
		_outp(0xA1, _inp(0xA1) | (1 << (irq-8)));
	if (traceflag != 0) printf("Restored old interrupt vector\n");

	for (k = 0; k < 16; k++) {
		flags = _inp(base+4);
		if (flags == 0) break;
		_outp(base+4, 0);			/* disable interrupts, clear RTS and DTR */
		if (traceflag != 0) printf("Disabling interrupts again!\n");
	}
}

void statistics(void) {
	int modemstatus;

	if (traceflag != 0) {
		modemstatus = _inp(base+6);
		printf("modemstatus: %X\n", modemstatus);
	}
	if (logcount > 0)
		printf("Log File characters: %d ", logcount);
	if(cscount > 0 || cqcount > 0)
		printf("XON/XOFF: C-S %d C-Q %d ", cscount, cqcount);
	if (dsrup > 0 || dsrdown > 0)
		printf("DTR/DSR: DOWN %d UP %d ", dsrdown, dsrup);
	if (dsrsame > 0)
		printf("SAME %d ", dsrsame);
	putchar('\n');
}

void cleanup(void) {
	if ((logging != 0) && (logfile != NULL)) {
		fclose(logfile);		/* close output */
		if (logcount == 0) {
			remove(logname); 
			rename(logbak, logname); /* try and rename possible backup */
		}
	}
	restoreint();				/* restore interrupt vector */
}

void sweepup(void) {
	FILE *output;
/*	totally kludgy ! */
/*	reset endofjob in case set earlier */
	if (wantcontrold != 0) {		
		if ((output = fopen(destination, "wb")) != NULL) {
			endofjob = 0;		
/*			putc(CONTROLC, output); */
			putc(CONTROLD, output);
			fclose(output);
		} 
	} 
	if (verboseflag != 0) fprintf(stderr, "\nUser Interrupt\n"); 
	if (verboseflag != 0) statistics(); 
}

void abortjob (void) {
	cleanup();
	sweepup();
	exit(3);
}

void ctrlbreak(int err) {
	(void) signal(SIGINT, SIG_IGN);			/* disallow control-C */
	if (bAbort++ >= 7) exit(3);				/* emergency exit */
	(void) signal(SIGINT, ctrlbreak); 
}

void sendfile(FILE *input, FILE *logfile, int waitflag) {
	clock_t finishtime=0;

	outputchar = 0; stopflag = 0; endofjob = 0;

	if (traceflag != 0) printf("ENTERING sendfile\n"); 
	for(;;) {

/*		if (traceflag != 0 && outputflow != 0) printf("@"); */
		if (outputchar == 0 && stopflag == 0 
/*			&& outputflow == 0 && dsrstate != 0 */
			)  { 
			if (input == stdin) {
				if ((outputchar = getkey()) == 26) {
					stopflag=1;					/* cntrl-Z */
					finishtime = clock();
					if (waitflag != 0 && wantcontrold != 0) {
						endofjob = 0;	/* reset in case set earlier */
						outputchar = CONTROLD;	/* C-D */
					}
					else outputchar = 0;
				}
			}
			else {
				if ((outputchar = getc(input)) == EOF) {
					stopflag=1;					/* EOF */
					finishtime = clock();
					if (waitflag != 0 && wantcontrold != 0)  {
						endofjob = 0;	/* reset in case set earlier */
						outputchar = CONTROLD;	/* C-D */
					}
					else outputchar = 0;
				}
			}
		}

		if (outputchar > 0 && outputchar <= 127) {

			if (outbuffering != 0) {
/*				_disable(); */
/*				if (outputcount == OUTBUFSIZE)  outputoverflow = 1; */
				if (outputcount < OUTBUFSIZE) {
/*				else { */
					_disable(); 					
					if (outputcount == -1) _outp(base, outputchar);
					else 
					  outputchars[(outputindex+outputcount) & (OUTBUFSIZE-1)] 
							= (char) outputchar;
						outputcount = outputcount+1;
/*				} */
				_enable();
				outputchar = 0;
				}
			}
			else if ((_inp(base+5) & 0x20) != 0) {
				_outp(base, outputchar);
				outputchar = 0;
			}
		}
/*		with InputBuffer do */
		if (inputcount > 0) {
			_disable();
			inputchar = inputchars[(inputindex-inputcount) & (INBUFSIZE-1)];
			inputcount = inputcount-1;
			if (inputcount == INBUFSIZE/2) 
				_outp(base+4, 8+2+1); /*	turn DTR and RTS back on */
			_enable();
			if (inputchar != 0) {
				if (inputchar == CONTROLD) {
					endofjob = -1;	/* matching C-D seen */
					if (verboseflag != 0) 
						printf("End-of-Job (control-D) received from printer\n");
				}
				else {
					if (inputchar < 32 && 
						inputchar != '\n' && inputchar != '\r') {
						putchar('C'); putchar('-'); putchar(inputchar+64);
					}
					else putchar(inputchar);
					if (logging != 0) {
						putc(inputchar, logfile);
						logcount++;
					}
				}
			}
		}

/*		if ((input == stdin && outputchar == 29) ||  
			outputchar == EOF) break; */
		if (stopflag != 0 && outputcount < 0 && inputcount == 0) {
			if (waitflag == 0 || endofjob != 0) break; 
			if (waitforeoj == 0) break;
			else {		/* waittime = 0 => wait forever */
				if(waittime > 0 && 
/*					(clock() - finishtime)/CLK_TCK > waittime) { */
					(clock() - finishtime)/CLOCKS_PER_SEC > waittime) {
					if (verboseflag != 0)
						printf("\nTimed out waiting after %s seconds\n", 
							waittime);
					break;
				}
			}
		}
		if (bAbort > 0) abortjob();			/* 1992/Nov/24 */
	}
}

/* Sep 27 1990 => 1990 Sep 27 */

void scivilize (char *date) {
	int k;
	char year[6];

	strcpy (year, date + 7);
	for (k = 5; k >= 0; k--) date[k+5] = date[k];
/*	date[11] = '\0'; */
	for (k = 0; k < 4; k++) date[k] = year[k];
	date[4] = ' ';
	return;
}

void stampit(FILE *output) {
	char date[11 + 1];
	strcpy(date, compiledate);
	scivilize(date);	 
	fprintf(output, "%s (%s %s)\n", 
		programversion, date, compiletime);
}

long checkcopyright(char *s) {
	int c;
	unsigned long hash=0;
	while ((c = *s++) != '\0') {
		hash = (hash * 53 + c) & 16777215;
	}
	if (hash == HASHCODE) return 0; /*  change if copyright changed */
	fprintf(stderr, "HASHED %ld\n", hash);	_getch(); 
	return hash;
}

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***  */

int decodeflag (int c) {
	switch(c) {
		case '?': detailflag = 1; return 0;
		case 'v': if (verboseflag != 0) traceflag = 1; else verboseflag = 1; 
			return 0; break; 
/*		case 'v': verboseflag = 1; return 0; */ /* never used */
/*		case 't': traceflag = 1; return 0;  */
		case 'l': wantlog = 0; return 0;
		case 'e': wantcontrold = 0; waitforeoj = 0; return 0;
		case 'w': waitforeoj = 0; return 0; 
		case 'p': getportflag = 0; return 0;	/* 1993/Aug/5 */
		case 'h': honorflag = 1; return 0;		/* 1993/Aug/5 */
		case 's': showflag = 1; return 0;		/* 1993/Aug/5 */
		case 't': waitingflag = 1; break;
		case 'd': outputflag = 1; break;
		case 'q': irqflag = 1; break;			/* 1993/Aug/5 */
		case 'b': baudflag = 1; break;			/* 1993/Aug/5 */
		default: {
				fprintf(stderr, "Sorry: Invalid command line flag '%c'\n", c);
				exit(7);
		}
	}
	return -1;		/* need argument */
}

void showusage(char *s) {
	fprintf (stderr, /* "Correct usage is:\n\ */
"%s [-{v}{e}{w}{l}{p}] [-d=<destination>] [-t=<wait-time>]\n\
\t[-q=<irq>] <ps-file-1> <ps-file-2> ...\n\
",	s);
	if (detailflag == 0) exit(7);
	fprintf (stderr, "\
\tv: verbose mode\n\
\te: do not send end-of-job (control-D) at end (implies w)\n\
\tw: do not wait for end-of-job (control-D) acknowledgement\n\
\tl: do not write log file\n\
\tp: do not get port addresses from machine (use standard addresses)\n");
/* \tl: do not write log file (`%s')\n", logname); */
	fprintf(stderr, "\
\tt: time to wait for EOJ acknowledgement - in seconds (default `%d')\n",
	waittime);
	fprintf(stderr, "\
\t   (zero means wait indefinitely)\n");
	fprintf(stderr, "\
\tq: assume specified IRQ (rather than default for port)\n");
	fprintf (stderr, "\
\td: destination (AUX, COM1 ...) (default `%s')\n", destination);
	exit(7);
}

/* deal with command line flags */
int commandline(int argc, char *argv[], int firstarg) {
	int c;
	unsigned int i;
	char *s;

	if (argc < 2) showusage(argv[0]);

	while (firstarg < argc && argv[firstarg][0] == '-') { /* check for command line flags */
		for(i = 1; i < strlen(argv[firstarg]); i++) {
			if ((c = argv[firstarg][i]) == '\0') break; 
			else if (decodeflag(c) != 0) { /* flag requires argument ? */
				if ((s = strchr(argv[firstarg], '=')) == NULL) {
					firstarg++; s = argv[firstarg];
				}
				else s++;
				if (outputflag != 0) {
					destination = s;		outputflag = 0;
				}
				if (waitingflag != 0) {
					if (sscanf(s, "%d", &waittime) < 1) {
						fprintf(stderr, "Don't understand wait time\n");
					}
					waitingflag = 0;
				}
				if (irqflag != 0) {
					if (sscanf(s, "%d", &userirq) < 1) {
						fprintf(stderr, "Don't understand IRQ\n");
					}
					irqflag = 0;
				}
				if (baudflag != 0) {
					if (sscanf(s, "%d", &baud) < 1) {
						fprintf(stderr, "Don't understand baud rate\n");
					}
					baudflag = 0;
				}
				break;
			}
		}
		firstarg++;
	}
	return firstarg;
}

/* is there some way easy way to tell whether port is installed ? */
/* address at 0x400 + 2 * i is zero -- BUT this may also be redirection */

void getportadrs(void) {
	int i, point;
	for(i = 0; i < MAXPORTS; i++) { /* get actual base addresses */
/*		point = getpoint(0x400 + 2 * i); */
/*		point = *((unsigned _far *) (0x400L + 2 * i)); */
		point = *((unsigned __far *) (0x400L + 2 * i));
		if (traceflag != 0) printf("COM%d: at %X\n", i+1, point);
		if (point!= 0 || honorflag != 0) basecom[i] = point;
	}
}

int main(int argc, char *argv[]) {
	int m, firstarg=1;
	int modemstatus=0; 
	char *s;
	int flags;
/*	unsigned int baudrega, baudregb; */
	
/*	inputcount = 0;	inputindex = 0;	inputoverflow = 0; */
/*	outputcount = -1; outputindex = 0; outputoverflow = 0; */

/*	if (argc < 2) showusage(argv[0]); */

	firstarg = commandline(argc, argv, 1);

	if (firstarg > argc || detailflag != 0) showusage(argv[0]); 

/*	scivilize(compiledate);	 */
	stampit(stdout);
	if (wantcpyrght != 0) printf("%s\n", copyright);

	if (checkcopyright(copyright) != 0) exit(1);

	signal(SIGINT, ctrlbreak);			/* enable user interrupt */

	if (getportflag != 0) getportadrs();		 /* ??? */

	if (strncmp(destination, "COM1", 4) == 0 ||
		strncmp(destination, "com1", 4) == 0 ||
		strncmp(destination, "AUX", 3) == 0 ||
		strncmp(destination, "aux", 3) == 0) {			
		base = basecom[0];						/* base = 0x3F8; */
		irq = 4;
	}
	else if (strncmp(destination, "COM2", 4) == 0 ||
		     strncmp(destination, "com2", 4) == 0) {
			 base = basecom[1];					/*	base = 0x2F8; */
			 irq = 3;
	}
	else if (strncmp(destination, "COM3", 4) == 0 ||
		     strncmp(destination, "com3", 4) == 0) {
			 base = basecom[2];					/* base = 0x3E8 ? */
			 irq = 4;	/* ??? */
	}
	else if (strncmp(destination, "COM4", 4) == 0 ||
		     strncmp(destination, "com4", 4) == 0) {
			 base = basecom[3];					/* base = 0x2E8 ? */
			 irq = 3;	/* ??? */
	}
	else {
		fprintf(stderr, 
			"Sorry: Destination must be serial port (COM1: or COM2:)\n");
		exit(1);
	}
	if (base == 0) {
		fprintf(stderr, "Sorry: Specified port (%s) not installed.\n", 
			destination);
		exit(2);
	}
	if (userirq > 0) irq = userirq;			/* 1993/Aug/5 */

	if (wantlog != 0) {
		if (firstarg == argc - 1) {		/* just one file to send ? NEW */
			if ((s = strrchr(argv[firstarg], '\\')) == NULL) {
				if ((s = strrchr(argv[firstarg], ':')) == NULL) 
					s = argv[firstarg];
				else s++;
			} 
			else s++;
			strcpy(logname, s);			/* construct name for log */
			forceexten(logname, "log");
			if (traceflag != 0) printf("Constructed name %s\n", logname);
		}
		if((logfile = fopen(logname, "r")) != NULL) {
			strcpy(logbak, logname);
			forceexten(logbak, "bak");
			remove(logbak); 		 /* in case backup already exists */
			rename(logname, logbak); /* ignore error in renaming */
			fclose(logfile);
		}
		if((logfile = fopen(logname, "w")) != NULL) {
			if (traceflag != 0) printf("Log file is %s\n", logname);
			logcount = 0;	logging = 1;
		}
		else {
			fprintf(stderr, "Sorry: Unable to open log file\n");
			logging = 0;
		}
	}
	else logging = 0;

	flags = _inp(base+4);					/* 1993/Aug/5 */
	if (traceflag != 0) printf("Initial interrupt state %d\n", flags);
	if (flags != 0) _outp(base+4, 0);	/* turn interrupts off if on */

	if (irq<8) intvectornum = irq+8;
	else intvectornum = irq+112;
	oldintvector = _dos_getvect(intvectornum);	
	_dos_setvect (
		intvectornum, 
			interrupthandler); 
	_disable();

	control = _inp(base+3);
	_outp(base+3, control | 0x80);		/* select baud rate registers */

	oldbaud = (115200 / ((_inp(base+1) << 8) | _inp(base)));
	if (traceflag != 0) printf("Baud rate is %d\n", oldbaud);

	if (baud != 0) {					/* if user specified baud rate */
		_outp(base, (int) ((115200 / baud) & 0xFF));
		_outp(base+1, (int) ((115200 / baud) >> 8));
		_outp(base+3, (int) ((parity << 3) | ((stopbits-1) << 2) | (databits-5)));
	}
	control = _inp(base+3);
	_outp(base+3, control & 0x7F);		/* deselect baud rate registers */

	_outp(base+4, 8+2+1);	/* enable interrupts, set DTR & RTS */
	_outp(base+1, 8+2+1);	/* define interrupt conditions */
	/* namely, modem status change, ready to send next, received character */
	outputchar = _inp(base);	/* clean out Received DataRegister */

	if (irq<8)				/* enable interrupt via PIC */
		_outp(0x21, _inp(0x21) & (0xFF - (1 << irq)));
	else
		_outp(0xA1, _inp(0xA1) & (0xFF - (1 << (irq-8))));
	_enable();
	outputchar = 0;

	modemstatus = _inp(base+6);	
	if (traceflag != 0) printf("modemstatus: %X\n", modemstatus);
	if ((modemstatus & 0x20) != 0) dsrstate = 1;
	else dsrstate = 0; 

	if (verboseflag != 0) {
		if (dsrstate != 0) 	printf("DSR from printer is ON\n");
		else printf("DSR from printer is OFF\n");
	}
	dsrstate = 1;			/* stop output if ever reset */

	if (argc == firstarg) sendfile(stdin, logfile, 1);
	else {
		for(m = firstarg; m < argc; m++) {
			strcpy(fn_in, argv[m]);
			extension(fn_in, "ps");
			if ((input = fopen(fn_in, "r")) == NULL) {
				fprintf(stderr, "Sorry: Cannot open input file ");
				perror(fn_in); continue;
			}
			if (verboseflag != 0) printf("Sending %s\n", fn_in);
			if (m == argc - 1) sendfile(input, logfile, 1);	/* last	one */
			else sendfile(input, logfile, 0);
			fclose(input);
			if (bAbort > 0) abortjob();			/* 1992/Nov/24 */
		}
	}

/*	while (outputcount >= 0);	*/ /* wait for it to empty out */
	cleanup();
/*	if (logging != 0 && logfile != NULL) {
		fclose(logfile);
		if (logcount == 0) {
			remove(logname);
			rename(logbak, logname); 
		}
	}
	restoreint(); */
	if (verboseflag != 0) statistics();
	if (outputoverflow > 0) printf("WARNING: output overflow\n");
	if (inputoverflow > 0) printf("WARNING: input overflow\n");
	if (verboseflag != 0 && argc > firstarg + 1)
		printf("Sent %d files\n", argc - firstarg);
	return 0;
}

/* have option to add control-D at end - OK */
/* have option to wait for matching control-D - OK */
/* add a time-out in case matching C-D never arrives - OK */
/* add a command line flag to ignore dsr ? OK */

/* add stuff to watch for %%BeginFonts and %%Page ? */
/* output [font] and [page] ? */

/* what are IRQ's really for COM3 and COM4 ? */

/* send output to input filename, but extension "log" ? */

/* state that time out occured when it does */

/* #define COM1		0x03F8		*/ /* COM1 port base address      */
/* #define COM2		0x02F8		*/ /* COM2 port base address      */

/* #define COMBASE          COM1	   */ /* Define as COM1 or COM2      */
/* #define TH_REG      (COMBASE+0)     */ /* Transmit Holding Register   */
/* #define RB_REG      (COMBASE+0)     */ /* Receive Buffer Register     */
/* #define DL_LSB      (COMBASE+0)     */ /* Divisor Latch ... LS byte   */
/* #define DL_MSB      (COMBASE+1)     */ /* Divisor Latch ... MS byte   */
/* #define IE_REG      (COMBASE+1)     */ /* Interrupt Enable Register   */
/* #define ID_REG      (COMBASE+2)     */ /* Interrupt I.D. Register     */
/* #define LC_REG      (COMBASE+3)     */ /* Line Control Register       */
/* #define MC_REG      (COMBASE+4)     */ /* Modem Control Register      */
/* #define LS_REG      (COMBASE+5)     */ /* Line Status Register        */
/* #define MS_REG      (COMBASE+6)     */ /* Modem Status Register       */


