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

/* Y&Y TeX's DVIPDFMX backend. */

#define EXTERN extern

#include "texd.h"

double dev_origin_x = 72.0;
double dev_origin_y = 770.0;

extern void pdf_init_fontmaps (void);

void dpx_ship_out(pointer p)
{
//read_config_file(DPX_CONFIG_FILE);
//pdf_font_set_dpi(font_dpi);
//dpx_delete_old_cache(image_cache_life);
//pdf_files_init();
//pdf_doc_begin_page(1, dev_origin_x, dev_origin_y);
//pdf_doc_end_page();
}

void dpx_hlist_out (void)
{
}

void dpx_vlist_out (void)
{
}
