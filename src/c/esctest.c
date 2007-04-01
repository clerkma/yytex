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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char test1[] = "/Gamma 23 RD\n";
char test2[] = " /Gamma 23 RD\n";
char test3[] = "\t/Gamma 23 RD\n";
char test4[] = "\n/Gamma 23 RD\n";
char test5[] = "\r/Gamma 23 RD\n";
char test6[] = "\f/Gamma 23 RD\n";

int teststring(char *s) {
	char charname[32];
	int nbin, count;
	count = sscanf(s, "/%s %d", &charname, &nbin);
	printf("%s yields %d items\n", s, count);
	return 0;
}


int main(int argc, char *argv[]) {
	teststring(test1);
	teststring(test2);	
	teststring(test3);		
	teststring(test4);
	teststring(test5);
	teststring(test6);
	return 0;
}
