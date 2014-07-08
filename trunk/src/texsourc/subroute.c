/* Copyright 2007 TeX Users Group
   Copyright 2014 Clerk Ma
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

#define EXTERN extern

#include "texd.h"

#define PATH_SEP '/'

// texk/web2c/lib/uexit.c
void uexit (int unix_code)
{
  int final_code;

  update_terminal();

  if (unix_code == 0)
    final_code = EXIT_SUCCESS;
  else if (unix_code == 1)
    final_code = EXIT_FAILURE;
  else
    final_code = unix_code;

  if (jump_used)
  {
    printf("Jump Buffer already used\n");
    exit(1);
  }

  jump_used++;
  exit(final_code);
}
// texk/web2c/lib/zround.c
integer web2c_round (double r)
{
  integer i;

  if (r > 2147483647.0)
    i = 2147483647;
  else if (r < -2147483647.0)
    i = -2147483647;
  else if (r >= 0.0)
    i = (integer) (r + 0.5);
  else
    i = (integer) (r - 0.5);

  return i;
}
// Unixify filename and path (turn \ into /)
// --- assumes null terminated
char * unixify (char * t)
{
  char * s = t;

  if (s == NULL)
    return s;

  if (t != '\0')
  {
    while (*s != '\0')
    {
      if (*s == '\\')
        *s = PATH_SEP;

      s++;
    }
  }

  if (trace_flag)
    printf("Unixified name: %s\n", t);

  return t;
}

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