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

/* eresbox.c */

#include <graph.h>
#include <stdio.h>
#include <conio.h>

int main(int argc, char *argv[]) {
	printf("ERESBOX\n");
	_getch();
	if (_setvideomode( _ERESCOLOR ) ) { /* EGA 640 x 350 mode */
		_rectangle(_GBORDER, 10, 10, 110, 110);		/* draw */
		_getch();	/* wait */
		_setvideomode(_DEFAULTMODE);	/* retrun to default */
	}
	else puts("Can't enter _ERESCOLOR graphics mode.");
	return 0;
}
