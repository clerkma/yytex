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
#include <direct.h>

#define MAXFILENAME 128

/* char DefPath[MAXFILENAME]=""; */

/* expand ..\ and .\ notation in file names, allow for nested occurence */

int expanddots(char *s) {
	char *t, *u;
	int n;
	char DefPath[MAXFILENAME]="";			/* use global version ? */

	if (*s != '.') return -1;				/* nothing to do */
#ifdef WIN32
	GetCurrentDirectory(sizeof(DefPath), DefPath);
#else
	_getcwd(DefPath, sizeof(DefPath));
#endif
	printf("Current Directory is %s\n", DefPath);
	t = DefPath + strlen(DefPath) - 1;
	if (*t != '\\' && *t != '/') strcat(DefPath, "\\");
	printf("Current Directory is %s\n", DefPath);
	n = strlen(DefPath);
	printf("Length %d\n", n);
	t = s + n;							/* where string will get moved to */
	printf("Before memmove we have %s\n", s);
	memmove (t, s, strlen(s)+1);		/* make space for current directory */
/*	printf("Before splice we have %s\n", s); */
	strncpy (s, DefPath, n);			/* splice it in */
	printf("After splice we have %s\n", s);
	while (*t == '.') {					/* loop until all . and .. removed */
		if (*(t+1) != '.')	{			/* single dot case */
			if (*(t+1) != '\\' && *(t+1) != '/') {
				strcpy(s, t);			/* restore (partial) bad string */
				return -1;				/* garbage format */
			}
			strcpy(t, t+2);			/* flush ".\" */
		}
		else {							/* double dot */
			if (*(t+2) != '\\' && *(t+2) != '/') {
				strcpy(s, t);			/* restore (partial) bad string */
				return -1;				/* garbage format */
			}
			u = t-2;					/* assuming ..\ form ... */
			if (u < s) u = s;			/* sanity check avoid error */
/* search back for separator or colon or start of string ... */
			while (u > s && *u != '\\' && *u != '/' && *u != ':') u--;
			if (*u == ':') u++;			/* leave in separator after : */
			strcpy(u+1, t+3);			/* flush "..\" */
			t = u+1;
		}
		printf("After removal of dots we have %s\n", s);
	}
/*	if (deslash) unixify(s); */
	return 0;
}

int main(int argc, char *argv[]) {
	char name[MAXFILENAME];
	if (argc < 2) exit(1);
	strcpy(name, argv[1]);
	printf("We have been given %s\n", name);
	if (expanddots (name) != 0) printf("Expand Dots Failed\n");
	printf("%s => %s\n", argv[1], name);
	return 0;
}
