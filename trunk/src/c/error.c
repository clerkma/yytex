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

#include <stdarg.h>

/* a Windows error routine that allows format specification ala printf */
/* page 84 in Windos/DOS Developer's Journal August 1993 */

BOOL error (HWND hMainWnd, char *format, ...) {
	va_list ap;
	char buf[256];
	
	va_start(ap, format);
	wvsprintf(buf, format, ap);
	va_end(ap);

	if (MessageBox (hMainWnd, buf, "ERROR",
	MB_ICONEXCLAMATION | MB_RETRYCANCEL) == IDCANCEL) {
		DestroyWindow (hMainWnd);
		exit(1);
	}
	return TRUE;
}

/* allowed format specifiers %c %d %x %i -- %ld %li %lx -- %s */
