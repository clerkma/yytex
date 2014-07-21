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

#include "yandytex.h"

void dpx_ship_out_(pointer p)
{
  integer page_loc;
  char j, k;
  pool_pointer s;
  char old_setting;

  if (tracing_output > 0)
  {
    print_nl("");
    print_ln();
    prints("Completed box being shipped out");
  }

  if (term_offset > max_print_line - 9)
    print_ln();
  else if ((term_offset > 0) || (file_offset > 0))
    print_char(' ');

  print_char('[');
  j = 9;

  while ((count(j) == 0) && (j > 0))
    decr(j);

  for (k = 0; k <= j; k++)
  {
    print_int(count(k));

    if (k < j)
      print_char('.');
  }
  
  update_terminal();

  if (tracing_output > 0)
  {
    print_char(']');
    begin_diagnostic();
    show_box(p);
    end_diagnostic(true);
  }

  if ((height(p) > max_dimen) || (depth(p) > max_dimen) ||
      (height(p) + depth(p) + v_offset > max_dimen) ||
      (width(p) + h_offset > max_dimen))
  {
    print_err("Huge page cannot be shipped out");
    help2("The page just created is more than 18 feet tall or",
        "more than 18 feet wide, so I suspect something went wrong.");
    error();

    if (tracing_output <= 0)
    {
      begin_diagnostic();
      print_nl("The following box has been deleted:");
      show_box(p);
      end_diagnostic(true);
    }

    goto done;
  }

  if (height(p) + depth(p) + v_offset > max_v)
    max_v = height(p) + depth(p) + v_offset;

  if (width(p) + h_offset > max_h)
    max_h = width(p) + h_offset;

  dvi_h = 0;
  dvi_v = 0;
  cur_h = h_offset;
  dvi_f = null_font;
  ensure_dvi_open();

  if (total_pages == 0)
  {
    pdf_set_version(5);
    pdf_doc_set_creator("Y&Y TeX 2.3.0");
  }

  page_loc = dvi_offset + dvi_ptr;
  pdf_doc_begin_page(1000, 0, 0); // bop
  cur_v = height(p) + v_offset;
  pdf_doc_end_page(); // eop
  incr(total_pages);
  cur_s = -1;

done:
  if (tracing_output <= 0)
    print_char(']');

  dead_cycles = 0;
  update_terminal();

#ifdef STAT
  if (tracing_stats > 1)
  {
    print_nl("Memory usage before: ");
    print_int(var_used);
    print_char('&');
    print_int(dyn_used);
    print_char(';');
  }
#endif

  flush_node_list(p);

#ifdef STAT
  if (tracing_stats > 1)
  {
    prints(" after: ");
    print_int(var_used);
    print_char('&');
    print_int(dyn_used);
    prints("; still utouched: ");
    print_int(hi_mem_min - lo_mem_max - 1);
    print_ln();
  }
#endif
}


void dpx_hlist_out (void)
{
}

void dpx_vlist_out (void)
{
}
