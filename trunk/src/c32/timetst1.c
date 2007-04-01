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
#include <time.h>

/* #define CLK_TCK 1000 */

void showtm(struct tm *tmt) {
	printf("isdst %d yday %d wday %d year %d mon %d day %d hour %d min %d sec %d\n",
		   tmt->tm_isdst, tmt->tm_yday, tmt->tm_wday, tmt->tm_year,
		   tmt->tm_mon, tmt->tm_mday,  tmt->tm_hour, tmt->tm_min, tmt->tm_sec);
}

int main(int argc, char *argv[]) {
	clock_t clockt;
	time_t timet;
	struct tm *ltmt;
	struct tm *gtmt;
	int m;

	(void) time(&timet);
	timet = ((unsigned long) 1 << 31) - 1;
/*	timet = 0; */
	for (m = 0; m < 16; m++) {
		printf ("time %ld time %0lX \n", timet, timet);
/*		This goes wrong whenever timet < 0 (i.e. > 2^31-1) */
		ltmt = localtime(&timet);
		if (ltmt == NULL) printf("localtime is NULL\n");
		else {
			showtm(ltmt);
			printf("asctime %s", asctime(ltmt));
		}
/*		This goes wrong whenever timet < 0 (i.e. > 2^31-1) */
		gtmt = gmtime(&timet);
		if (gtmt == NULL) printf("gmtime is NULL\n");
		else {
			showtm(gtmt);
			printf("gmtime %s", asctime(gtmt));
		}
		timet++; 
/*		timet--; */
		fflush(stdout);
		putc('\n', stdout);
	}
	clockt = clock();
	printf ("clock %ld CLK_TCK %d clock / CLOCK_TCK %lg \n",
			clockt, CLK_TCK,  (double) clockt / CLK_TCK);
	return 0;
}

/*          0 => gmtime Thu Jan 01 00:00:00 1970 */

/* 2147483647 => gmtime Tue Jan 19 03:14:07 2038 */
