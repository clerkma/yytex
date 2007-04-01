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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *string1="this is string one\n";

#define MAXLINE 256

int main(int argc, char *argv[]) {
	char buffer[MAXLINE];

/*	printf("First 5 characters of `foobarfoobar' are %*s\n",
		5, "foobarfoobar"); */

	sscanf(string1, "%s", &buffer);
	printf("percent s on string1 yields `%s'\n", buffer);	
	sscanf(string1, "%s ", &buffer);
	printf("percent s space on string1 yields `%s'\n", buffer);	
	sscanf(string1, "%s\n", &buffer);
	printf("percent s return on string1 yields `%s'\n", buffer);	
	return 0;
}
