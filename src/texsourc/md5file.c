/* Copyright 2014 Clerk Ma

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

#pragma warning(disable:4996)

#include <stdio.h>
#include "libmd5/md5.h"

char * md5_file_name(const char * file_name)
{
  md5_state_t md5_ship;
  md5_byte_t  md5_data[1024];
  md5_byte_t  md5_digest[16];
  static char md5_hex[33];
  int         md5_len;
  FILE * ship_file;
  int i;

  ship_file = fopen(file_name, "rb");

  md5_init(&md5_ship);

  while ((md5_len = fread(md5_data, 1, 1024, ship_file)) != 0)
    md5_append(&md5_ship, md5_data, md5_len);

  md5_finish(&md5_ship, md5_digest);

  fclose(ship_file);

  for (i = 0; i < 16; ++i)
    sprintf(md5_hex + i * 2, "%02X", md5_digest[i]);

  return md5_hex;
}

char * md5_file(FILE * in_file)
{
  md5_state_t md5_ship;
  md5_byte_t  md5_data[1024];
  md5_byte_t  md5_digest[16];
  static char md5_hex[33];
  int         md5_len;
  int i;

  md5_init(&md5_ship);

  while ((md5_len = fread(md5_data, 1, 1024, in_file)) != 0)
    md5_append(&md5_ship, md5_data, md5_len);

  md5_finish(&md5_ship, md5_digest);

  fclose(in_file);

  for (i = 0; i < 16; ++i)
    sprintf(md5_hex + i * 2, "%02X", md5_digest[i]);

  return md5_hex;
}

#ifdef MD5TEST
int main(void)
{
  printf("%s", md5_file("md5.c"));

  return 0;
}
#endif
