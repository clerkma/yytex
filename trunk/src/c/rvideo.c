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

/* RVIDEO.C --- 16 bit real mode */  /* poke video display memory */

/* Have to compile with -Ze, NOT -Za in order to enable _asm */

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

#define MAX 1000000L

unsigned short GetVidSeg(void);

int main (int argc, char *argv[]) {
	unsigned short VidSeg;
	unsigned short _far *pVid; 
/*	unsigned short __far *pVid; */
	long i;

/* get real mode paragraph adr of video memory */

	VidSeg = GetVidSeg();
	if (VidSeg == 0) return 1;
	_FP_SEG(pVid) = VidSeg;
	_FP_OFF(pVid) = 0;

/* poke video memory MAX times */
	
	for (i = MAX; i--;)
		*(pVid + (i %2000)) = (unsigned short) i;

	return 0;
}

unsigned short GetVidSeg(void) {
	char VidMode;
	char ActivePage;

/* Get Video Mode */

	_asm { 
/*	__asm { */
		mov		ah,0Fh
		int		10h
		mov		VidMode,al
		mov		ActivePage,bh
	}

/* only supports video page 0, 80 x 25 text mode */

	if (ActivePage != 0) {
		printf("Only video page 0 is supported\n");
		return 0;
	}
	if (VidMode == 7) return 0xB000;	/* mono */
	else if (VidMode == 2 || VidMode == 3) return 0xB800;
	else {
		printf("Only 80 x 25 text mode supported\n");
		return 0;
	}
}

