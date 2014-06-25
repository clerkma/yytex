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

#define EXTERN extern

#include "texd.h"

/* end of the old tex8.c */
/* sec 1284 */
void give_err_help (void)
{
  token_show(err_help);
}
/* sec 0524 */
boolean open_fmt_file (void)
{
  integer j;

  j = loc;

  if (buffer[loc] == '&' || buffer[loc] == '+')
  {
    incr(loc);
    j = loc;
    buffer[last] = ' ';

    while (buffer[j] != ' ')
      incr(j);

    pack_buffered_name(0, loc, j - 1);

    if (w_open_in(fmt_file))
      goto found;
  
    if (knuth_flag)
    {
      sprintf(log_line, "%s;%s\n", "Sorry, I can't find that format",
        " will try the default.");
      show_line(log_line, 1);
    }
    else
    {
      char *s = log_line;

      name_of_file[name_length + 1] = '\0';
      sprintf(s, "%s (%s);%s\n", "Sorry, I can't find that format",
        name_of_file + 1, " will try the default."); 
      name_of_file[name_length + 1] = ' ';
      s += strlen(s);
      sprintf(s, "(Perhaps your %s environment variable is not set correctly)\n",
        "TEXFORMATS");
      s += strlen(s);
      show_line(log_line, 1);
    }

    update_terminal();
  }

  pack_buffered_name(format_default_length - 4, 1, 0);

  if (!w_open_in(fmt_file))
  {
    if (knuth_flag)
    {
      sprintf(log_line, "%s!\n", "I can't find the default format file");
      show_line(log_line, 1);
    }
    else
    {
      char *s = log_line;

      name_of_file[name_length + 1] = '\0';
      sprintf(s, "%s (%s)!\n", "I can't find the default format file", name_of_file + 1);
      name_of_file[name_length + 1] = ' ';
      s += strlen(s);
      sprintf(s, "(Perhaps your %s environment variable is not set correctly)\n", "TEXFORMATS");
      s += strlen(s);
      show_line(log_line, 1);
    }

    return false;
  }

found:
  loc = j;

  return true;
}

void show_font_info (void); // now in local.c
extern int closed_already;  // make sure we don't try this more than once
/* sec 1333 */
void close_files_and_terminate (void)
{
  integer k; 

  if (closed_already++)
  {
    puts("close_files_and_terminated already ");
    return;     // sanity check
  }

  if (trace_flag)
    puts("\nclose_files_and_terminate ");

  for (k = 0; k <= 15; k++)
    if (write_open[k])
    {
      (void) a_close(write_file[k]);
    }

#ifdef STAT
  if (tracing_stats > 0 || verbose_flag != 0)
    if (log_opened)
    {
      fprintf(log_file, "%c\n", ' ');
      fprintf(log_file, "\n");
      fprintf(log_file, "%s%s\n", "Here is how much of TeX's memory", " you used:");
      fprintf(log_file, "%c%lld%s", ' ', (integer)(str_ptr - init_str_ptr), " string");

      if (str_ptr != init_str_ptr + 1)
        putc('s',  log_file);

#ifdef ALLOCATESTRING
      if (show_current)
        fprintf(log_file, "%s%d\n", " out of ", (int)(current_max_strings - init_str_ptr));
      else
#endif
        fprintf(log_file, "%s%d\n", " out of ", (int)(max_strings - init_str_ptr));

#ifdef ALLOCATESTRING
      if (show_current)
        fprintf(log_file, "%c%lld%s%lld\n", ' ', (integer)(pool_ptr - init_pool_ptr), " string characters out of ", current_pool_size - init_pool_ptr);
      else
#endif
        fprintf(log_file, "%c%lld%s%lld\n", ' ', (integer)(pool_ptr - init_pool_ptr), " string characters out of ", pool_size - init_pool_ptr);

#ifdef ALLOCATEMAIN
      if (show_current)
        fprintf(log_file, "%c%lld%s%d\n", ' ', (integer)(lo_mem_max - mem_min + mem_end - hi_mem_min + 2), " words of memory out of ", current_mem_size);
      else
#endif
        fprintf(log_file, "%c%lld%s%lld\n", ' ', (integer)(lo_mem_max - mem_min + mem_end - hi_mem_min + 2), " words of memory out of ", mem_end + 1 - mem_min);

      fprintf(log_file, "%c%lld%s%d\n", ' ', (cs_count), " multiletter control sequences out of ", (hash_size + hash_extra));
      fprintf(log_file, "%c%lld%s%lld%s", ' ', (fmem_ptr), " words of font info for ", (font_ptr - font_base), " font");

      if (font_ptr != 1)
        putc('s',  log_file);

#ifdef ALLOCATEFONT
      if (show_current)
        fprintf(log_file, "%s%d%s%d\n", ", out of ", current_font_mem_size, " for ", font_max - font_base);
      else
#endif
        fprintf(log_file, "%s%d%s%d\n", ", out of ", font_mem_size, " for ", font_max - font_base);

      fprintf(log_file, "%c%ld%s", ' ', hyph_count, " hyphenation exception");

      if (hyph_count != 1)
        putc('s',  log_file);

      fprintf(log_file, "%s%ld\n",  " out of ", hyphen_prime);
      fprintf(log_file, " ");
      fprintf(log_file, "%ld%s", (int)max_in_stack, "i,");
      fprintf(log_file, "%ld%s", (int)max_nest_stack, "n,");
      fprintf(log_file, "%ld%s", (int)max_param_stack, "p,");
      fprintf(log_file, "%ld%s", (int)max_buf_stack + 1, "b,");
      fprintf(log_file, "%ld%s", (int)max_save_stack + 6, "s");
      fprintf(log_file, " stack positions out of ");

#ifdef ALLOCATESAVESTACK
      if (show_current)
        fprintf(log_file, "%ld%s", current_stack_size, "i,");
      else
#endif
        fprintf(log_file, "%ld%s", stack_size, "i,");

#ifdef ALLOCATENESTSTACK
      if (show_current)
        fprintf(log_file, "%ld%s", current_nest_size, "n,");
      else
#endif
        fprintf(log_file, "%ld%s", nest_size, "n,");

#ifdef ALLOCATEPARAMSTACK
      if (show_current)
        fprintf(log_file, "%ld%s", current_param_size, "p,");
      else
#endif
        fprintf(log_file, "%ld%s", param_size, "p,");

#ifdef ALLOCATEBUFFER
      if (show_current)
        fprintf(log_file, "%ld%s", current_buf_size, "b,");
      else
#endif
        fprintf(log_file, "%ld%s", buf_size, "b,");

#ifdef ALLOCATESAVESTACK
      if (show_current)
        fprintf(log_file, "%ld%s", current_save_size, "s");
      else
#endif
        fprintf(log_file, "%ld%s", save_size, "s");

      fprintf(log_file, "\n");

      if (!knuth_flag)
        fprintf(log_file, " (i = in_stack, n = nest_stack, p = param_stack, b = buf_stack, s = save_stack)\n");

      if (!knuth_flag)
        fprintf(log_file, " %lld inputs open max out of %d\n", high_in_open, max_in_open);

      if (show_line_break_stats && first_pass_count > 0)
      {
        int first_count, second_count, third_count;

        fprintf(log_file, "\nSuccess at breaking %d paragraph%s:", first_pass_count, (first_pass_count == 1) ? "" : "s");

        if (single_line > 0)
          fprintf(log_file, "\n %d single line `paragraph%s'", single_line, (single_line == 1) ? "" : "s");

        first_count = first_pass_count - single_line - second_pass_count;

        if (first_count < 0)
          first_count = 0;

        second_count = second_pass_count - final_pass_count;
        third_count = final_pass_count - paragraph_failed;

        if (first_pass_count > 0)
          fprintf(log_file, "\n %d first pass (\\pretolerance = %lld)", first_pass_count, pretolerance);

        if (second_pass_count > 0)
          fprintf(log_file, "\n %d second pass (\\tolerance = %lld)", second_pass_count, tolerance);

        if (final_pass_count > 0 || emergency_stretch > 0)
        {
          fprintf(log_file, "\n %d third pass (\\emergencystretch = %lgpt)", final_pass_count, (double) emergency_stretch / 65536.0);
        }

        if (paragraph_failed > 0)
          fprintf(log_file, "\n %d failed", paragraph_failed);

        putc('\n', log_file);

        if (overfull_hbox > 0)
          fprintf(log_file, "\n %d overfull \\hbox%s", overfull_hbox, (overfull_hbox > 1) ? "es" : "");

        if (underfull_hbox > 0)
          fprintf(log_file, "\n %d underfull \\hbox%s", underfull_hbox, (underfull_hbox > 1) ? "es" : "");

        if (overfull_vbox > 0)
          fprintf(log_file, "\n %d overfull \\vbox%s", overfull_vbox, (overfull_vbox > 1) ? "es" : "");

        if (underfull_vbox > 0)
          fprintf(log_file, "\n %d underfull \\vbox%s", underfull_vbox, (underfull_vbox > 1) ? "es" : "");

        if (overfull_hbox || underfull_hbox || overfull_vbox || underfull_vbox)
          putc('\n', log_file);
      }
  }
#endif

  switch (shipout_flag)
  {
    case out_dvi_flag:
    case out_xdv_flag:
      {
        while (cur_s > -1)
        {
          if (cur_s > 0) 
            dvi_out(pop);
          else
          {
            dvi_out(eop);
            incr(total_pages);
          }

          decr(cur_s);
        }

        if (total_pages == 0)
          print_nl("No pages of output.");
        else
        {
          dvi_out(post);
          dvi_four(last_bop);
          last_bop = dvi_offset + dvi_ptr - 5;
          dvi_four(25400000L);
          dvi_four(473628672L);
          prepare_mag();
          dvi_four(mag);
          dvi_four(max_v);
          dvi_four(max_h);
          dvi_out(max_push / 256);
          dvi_out(max_push % 256);

          if (total_pages >= 65536)
          {
            sprintf(log_line, "\nWARNING: page count (dvi_t) in DVI file will be %lld not %lld\n",
              (total_pages % 65536), total_pages);

            if (log_opened)
              fputs(log_line, log_file);

            show_line(log_line, 1);
          }

          dvi_out((total_pages / 256) % 256);
          dvi_out(total_pages % 256);

          if (show_fonts_used && log_opened)
            show_font_info();

          while (font_ptr > 0)
          {
            if (font_used[font_ptr])
              dvi_font_def(font_ptr);

            decr(font_ptr);
          }

          dvi_out(post_post);
          dvi_four(last_bop);
          dvi_out(id_byte);
          k = 4 + ((dvi_buf_size - dvi_ptr) % 4);

          while (k > 0)
          {
            dvi_out(223);
            decr(k);
          }

          if (trace_flag)
          {
            sprintf(log_line, "\ndviwrite %lld", dvi_gone);
            show_line(log_line, 0);
          }

          if (dvi_limit == half_buf)
            write_dvi(half_buf, dvi_buf_size - 1);

          if (dvi_ptr > 0)
            write_dvi(0, dvi_ptr - 1); 

          print_nl("Output written on ");

          if (full_file_name_flag && dvi_file_name != NULL)
            prints(dvi_file_name);
          else
            slow_print(output_file_name);

          prints(" (");
          print_int(total_pages);
          prints(" page");

          if (total_pages != 1)
            print_char('s');

          prints(", ");
          print_int(dvi_offset + dvi_ptr);
          prints(" bytes).");
          b_close(dvi_file);
        }
      }
      break;
  }

  if (log_opened)
  {
    putc('\n', log_file);
    a_close(log_file);
    selector = selector - 2;

    if (selector == term_only)
    {
      print_nl("Transcript written on ");

      if (full_file_name_flag && log_file_name != NULL)
        prints(log_file_name);
      else
        slow_print(texmf_log_name);

      print_char('.');
    }
  }

  print_ln();

  if ((edit_name_start != 0) && (interaction > 0))
    call_edit(str_pool, edit_name_start, edit_name_length, edit_line);
}
#ifdef DEBUG
/* sec 1338 */
void debug_help (void) 
{
  integer k, l, m, n;

  while (true)
  { 
    print_nl(" debug # (-1 to exit):");

#ifndef _WINDOWS
    fflush(stdout); 
#endif

    read(stdin, m);  // ???

    if (m < 0)
      return;
    else if (m == 0)
      dumpcore();
    else
    {
      read(stdin, n);

      switch(m)
      {
        case 1:
          print_word(mem[n]);
          break;

        case 2:
          print_int(mem[n].hh.lh);
          break;
          
        case 3:
          print_int(mem[n].hh.rh);
          break;
        
        case 4:
          print_word(eqtb[n]);
          break;

        case 5:
#ifdef SHORTFONTINFO
          print_scaled(font_info[n].sc);
          print_char(' ');
          print_int(font_info[n].b0);
          print_char(':');
          print_int(font_info[n].b1);
          print_char(':');
          print_int(font_info[n].b2);
          print_char(':');
          print_int(font_info[n].b3);
#else
          print_word(font_info[n]); 
#endif
          break;
        
        case 6:
          print_word(save_stack[n]);
          break;
          
        case 7:
          show_box(n);
          break;
        
        case 8:
          {
            breadth_max = 10000;
#ifdef ALLOCATESTRING
            if (pool_ptr + 32000 > current_pool_size)
              str_pool = realloc_str_pool (increment_pool_size);
#endif
#ifdef ALLOCATESTRING
            depth_threshold = current_pool_size - pool_ptr - 10;
#else
            depth_threshold = pool_size - pool_ptr - 10;
#endif
            show_node_list(n);
          }
          break;
        
        case 9:
          show_token_list(n, 0, 1000);
          break;
        
        case 10:
          slow_print(n);
          break;
        
        case 11:
          check_mem(n > 0);
          break;
        
        case 12:
          search_mem(n);
          break;
        
        case 13:
          {
            read(stdin, l);
            print_cmd_chr(n, l);
          }
          break;
        
        case 14:
          {
            for (k = 0; k <= n; k++)
              print(buffer[k]);
          }
          break;
        
        case 15:
          {
            font_in_short_display = 0;
            short_display(n);
          }
          break;
        
        case 16:
          panicking = !panicking;
          break;
        
        default:
          print('?');
          break;
      }
    }
  }
}
#endif /* DEBUG */
