/* fontmap.h: declarations for reading a file to define additional font names.

   Copyright 1993 Karl Berry
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

/* isn't used by anything as far can be determined */

#ifndef FONTMAP_H
#define FONTMAP_H

typedef struct map_element_struct
{
  char *key;
  char *value;
  struct map_element_struct *next;
} map_element_type;

typedef map_element_type **map_type;

extern map_type map_create (string *dir_list);

extern char *map_lookup (map_type map, char *key);

#endif /* not FONTMAP_H */
