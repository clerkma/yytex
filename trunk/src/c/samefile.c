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

/* #include "types.h" */
#include <stdio.h> 
#include <stdlib.h> 
#include <sys\types.h>
#include <sys/stat.h>

/* this works in Unix, but apparently not in DOS ... */

/* typedef unsigned short _ino_t; */		/* i-node number (not used on DOS) */

int same_file_p (char *filename1,  char *filename2) {

    struct stat sb1, sb2;
    int r1, r2;

    r1 = stat (filename1, &sb1);
    r2 = stat (filename2, &sb2);

	if (r1 != 0) printf("%s does not exist\n", filename1);
	if (r2 != 0) printf("%s does not exist\n", filename2);

	printf("st_ino %u %u st_dev %u %u\n",
		sb1.st_ino, sb2.st_ino, sb2.st_dev, sb2.st_dev);

    if (r1 == 0 && r2 == 0)
      return sb1.st_ino == sb2.st_ino && sb2.st_dev == sb2.st_dev;
    else
      return 0;
}

int main (int argc, char *argv[]) {
	if (argc < 3) exit(1);
	if (same_file_p (argv[1], argv[2]))
		printf("%s and %s are the SAME file\n", argv[1], argv[2]);
	else printf("%s and %s are NOT the same file\n", argv[1], argv[2]);
	return 0;
}
