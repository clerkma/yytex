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

#include "stdlib.h"
#include "stdio.h"
#include "io.h"
#include "fcntl.h"

char *shortname =
"d:\\foo_bar_magic_long_name\\continuing_along_the_lines\\build_up_a_very_nasty\\problem_for_file_opening\\and_so_we_keep_on_adding\\more_and_more_stuff\\until_things_just_get_out_of_hand\\not_yet_perhaps_later\\we_must_persist\\even_yet_we_have_not_yet\\foo.txt";

char *longname =
"d:\\foo_bar_magic_long_name\\continuing_along_the_lines\\build_up_a_very_nasty\\problem_for_file_opening\\and_so_we_keep_on_adding\\more_and_more_stuff\\until_things_just_get_out_of_hand\\not_yet_perhaps_later\\we_must_persist\\even_yet_we_have_not_yet\\hello_world.text";

char *spacename = "d:\\foo bar chomp\\hello.txt"; 

void testname(char *name) {
	FILE *input;
	int hfile;
	int c;
	char buffer[256];
	
	printf("Trying to open\n%s\nLength %d\n", name, strlen(name));
	input = fopen(name, "r");
	if (input == NULL) {
		perror("FAILED");
	}
	else {
		while ((c = getc(input)) > 0) putc(c, stdout);
		putc('\n', stdout);
		fclose(input);
	}
	printf("Trying to open\n%s\nLength %d\n", name, strlen(name));
	hfile = _open(name, _O_RDONLY);
	if (hfile < 0) {
		perror("FAILED");
	}
	else {
		while (_read(hfile, buffer, 1) > 0)
			putc(*buffer, stdout);
		_close(hfile);
	}
}

int main(int argc, char *argv[]) {

	testname(longname);
	testname(shortname);
	testname(spacename);
	return 0;
}
