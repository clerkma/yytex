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

  j = cur_input.loc_field;

/* For Windows NT, lets allow + instead of & for format specification */
/*  User specified a format name on the command line                  */
  if (buffer[cur_input.loc_field] == '&' || buffer[cur_input.loc_field] == '+')
  {
    incr(cur_input.loc_field);
    j = cur_input.loc_field;
    buffer[last] = ' ';

    while (buffer[j] != ' ')
      incr(j);

    pack_buffered_name(0, cur_input.loc_field, j - 1);

    if (w_open_in(fmt_file))
      goto lab40;
  
    if (knuth_flag)
    {
      (void) sprintf(log_line, "%s;%s\n", "Sorry, I can't find that format",
        " will try the default.");
      show_line(log_line, 1);
    }
    else
    {
      char *s = log_line;

      name_of_file[name_length + 1] = '\0';
      (void) sprintf(s, "%s (%s);%s\n", "Sorry, I can't find that format",
        name_of_file + 1, " will try the default."); 
      name_of_file[name_length + 1] = ' ';
      s += strlen(s);
      (void) sprintf(s, "(Perhaps your %s environment variable is not set correctly)\n",
        "TEXFORMATS");
      s += strlen(s);
      {
        char *t;            /* extra info 97/June/13 */

        if ((t = grabenv("TEXFORMATS")) != NULL)
        {
          sprintf(s, "(%s=%s)\n", "TEXFORMATS", t);
        }
        else
        {
          sprintf(s, "%s environment variable not set\n", "TEXFORMATS");
        }
      }
      show_line(log_line, 1); // show all three lines at once
    }

#ifndef _WINDOWS
    fflush(stdout);
#endif
  }

/* Try the default format (either because no format specified or failed) */
  pack_buffered_name(format_default_length - 4, 1, 0);

  if (!w_open_in(fmt_file))
  {
    if (knuth_flag)
    {
      (void) sprintf(log_line, "%s!\n", "I can't find the default format file");
      show_line(log_line, 1);
    }
    else
    {
      char *s = log_line;

      name_of_file[name_length + 1] = '\0';
      (void) sprintf(s, "%s (%s)!\n", "I can't find the default format file", name_of_file + 1);
      name_of_file[name_length + 1] = ' ';
      s += strlen(s);
      (void) sprintf(s, "(Perhaps your %s environment variable is not set correctly)\n", "TEXFORMATS");
      s += strlen(s);
      {
        char *t;            /* extra info 97/June/13 */

        if ((t = grabenv("TEXFORMATS")) != NULL)
        {
          sprintf(s, "(%s=%s)\n", "TEXFORMATS", t);
        }
        else
        {
          sprintf(s, "%s environment variable not set\n", "TEXFORMATS");
        }
      }
      show_line(log_line, 1);   // show all three lines at once
    }

    return false;
  }

lab40:
  cur_input.loc_field = j;

  return true;
}
/**************************************************************************/
void print_char_string (unsigned char *s)
{
  while (*s > 0)
    print_char(*s++);
}
void show_font_info (void);   // now in local.c
extern int closed_already;     // make sure we don't try this more than once
/* The following needs access to zdvibuf of ALLOCATEDVIBUF 94/Mar/24 */
/* done in closefilesandterminate_regmem  in coerce.h */
/* sec 1333 */
void close_files_and_terminate (void)
{
  integer k; 

  if (closed_already++)
  {
    show_line("close_files_and_terminated already ", 0);
    return;     // sanity check
  }

  if (trace_flag)
    show_line("\nclose_files_and_terminate ", 0);

/* close all open files */
  for (k = 0; k <= 15; k++)
    if (write_open[k])
    {
      (void) a_close(write_file[k]);
    }

#ifdef STAT
  if (tracing_stats > 0 || verbose_flag != 0)  /* 93/Nov/30 - bkph */
    if (log_opened)
    {
/* used to output paragraph breaking statistics here */
      (void) fprintf(log_file, "%c\n", ' ');
      (void) fprintf(log_file, "\n");
      (void) fprintf(log_file, "%s%s\n", "Here is how much of TeX's memory", " you used:");
      (void) fprintf(log_file, "%c%ld%s", ' ', (long)str_ptr - init_str_ptr, " string");
      if (str_ptr != init_str_ptr + 1)
        (void) putc('s',  log_file);
#ifdef ALLOCATESTRING
      if (show_current)
        (void) fprintf(log_file, "%s%ld\n", " out of ", (long) current_max_strings - init_str_ptr);
      else
#endif
        (void) fprintf(log_file, "%s%ld\n", " out of ", (long) max_strings - init_str_ptr);

#ifdef ALLOCATESTRING
      if (show_current)
        (void) fprintf(log_file, "%c%ld%s%ld\n", ' ', (long) pool_ptr - init_pool_ptr, " string characters out of ", (long) current_pool_size - init_pool_ptr);
      else
#endif
        (void) fprintf(log_file, "%c%ld%s%ld\n", ' ', (long) pool_ptr - init_pool_ptr, " string characters out of ", (long) pool_size - init_pool_ptr);

#ifdef ALLOCATEMAIN
      if (show_current)
        (void) fprintf(log_file, "%c%ld%s%ld\n", ' ', (long) lo_mem_max - mem_min + mem_end - hi_mem_min + 2, " words of memory out of ", (long)current_mem_size);
      else
#endif
        (void) fprintf(log_file, "%c%ld%s%ld\n", ' ', (long) lo_mem_max - mem_min + mem_end - hi_mem_min + 2, " words of memory out of ", (long)mem_end + 1 - mem_min);
      (void) fprintf(log_file, "%c%ld%s%ld\n", ' ', (long) cs_count, " multiletter control sequences out of ", (long)(hash_size + hash_extra));
      (void) fprintf(log_file, "%c%ld%s%ld%s", ' ', (long) fmem_ptr, " words of font info for ", (long) font_ptr - 0, " font");
      if (font_ptr != 1)
        (void) putc('s',  log_file);

#ifdef ALLOCATEFONT
      if (show_current)
        (void) fprintf(log_file, "%s%ld%s%ld\n", ", out of ", (long)current_font_mem_size, " for ", (long)font_max - 0);
      else
#endif
        (void) fprintf(log_file, "%s%ld%s%ld\n", ", out of ", (long)font_mem_size, " for ", (long)font_max - 0);
      (void) fprintf(log_file, "%c%ld%s", ' ', (long)hyph_count, " hyphenation exception");
      if (hyph_count != 1)
        (void) putc('s',  log_file);

      (void) fprintf(log_file, "%s%ld\n",  " out of ", (long) hyphen_prime);
      (void) fprintf(log_file, " ");
      (void) fprintf(log_file, "%ld%s", (long)max_in_stack, "i,");
      (void) fprintf(log_file, "%ld%s", (long)max_nest_stack, "n,");
      (void) fprintf(log_file, "%ld%s", (long)max_param_stack, "p,");
      (void) fprintf(log_file, "%ld%s", (long)max_buf_stack + 1, "b,");
      (void) fprintf(log_file, "%ld%s", (long)max_save_stack + 6, "s");
      (void) fprintf(log_file, " stack positions out of ");

#ifdef ALLOCATESAVESTACK
      if (show_current)
        (void) fprintf(log_file, "%ld%s", (long)current_stack_size, "i,");
      else
#endif
        (void) fprintf(log_file, "%ld%s", (long)stack_size, "i,");

#ifdef ALLOCATENESTSTACK
      if (show_current)
        (void) fprintf(log_file, "%ld%s", (long)current_nest_size, "n,");
      else
#endif
        (void) fprintf(log_file, "%ld%s", (long)nest_size, "n,");

#ifdef ALLOCATEPARAMSTACK
      if (show_current)
        (void) fprintf(log_file, "%ld%s", (long)current_param_size, "p,");
      else
#endif
        (void) fprintf(log_file, "%ld%s", (long)param_size, "p,");

#ifdef ALLOCATEBUFFER
      if (show_current)
        (void) fprintf(log_file, "%ld%s", (long)current_buf_size, "b,");
      else
#endif
        (void) fprintf(log_file, "%ld%s", (long)buf_size, "b,");

#ifdef ALLOCATESAVESTACK
      if (show_current)
        (void) fprintf(log_file, "%ld%s", (long)current_save_size, "s");
      else
#endif
        (void) fprintf(log_file, "%ld%s", (long)save_size, "s");
      (void) fprintf(log_file, "\n");

      if (!knuth_flag)
        fprintf(log_file, " (i = in_stack, n = nest_stack, p = param_stack, b = buf_stack, s = save_stack)\n");

      if (!knuth_flag)
        fprintf(log_file, " %d inputs open max out of %d\n", high_in_open, max_in_open);

/*  Modified 98/Jan/14 to leave out lines with zero counts */
      if (show_line_break_stats && first_pass_count > 0) /* 96/Feb/8 */
      {
        int first_count, secondcount, thirdcount;

        (void) fprintf(log_file, "\nSuccess at breaking %d paragraph%s:", first_pass_count, (first_pass_count == 1) ? "" : "s");

        if (single_line > 0)
          (void) fprintf(log_file, "\n %d single line `paragraph%s'", single_line, (single_line == 1) ? "" : "s");  /* 96/Apr/23 */

        first_count = first_pass_count - single_line - second_pass_count;

        if (first_count < 0)
          first_count = 0;

        secondcount = second_pass_count - final_pass_count;
        thirdcount = final_pass_count - paragraph_failed;

        if (first_pass_count > 0)
          (void) fprintf(log_file, "\n %d first pass (\\pretolerance = %d)", first_pass_count, pretolerance);

        if (second_pass_count > 0)
          (void) fprintf(log_file, "\n %d second pass (\\tolerance = %d)", second_pass_count, tolerance);

        if (final_pass_count > 0 || emergency_stretch > 0)
        {
          (void) fprintf(log_file, "\n %d third pass (\\emergencystretch = %lgpt)", final_pass_count, (double) emergency_stretch / 65536.0);
        }

        if (paragraph_failed > 0)
          (void) fprintf(log_file, "\n %d failed", paragraph_failed);

        (void) putc('\n', log_file);

        if (overfull_hbox > 0)
          (void) fprintf(log_file, "\n %d overfull \\hbox%s", overfull_hbox, (overfull_hbox > 1) ? "es" : "");

        if (underfull_hbox > 0)
          (void) fprintf(log_file, "\n %d underfull \\hbox%s", underfull_hbox, (underfull_hbox > 1) ? "es" : "");

        if (overfull_vbox > 0)
          (void) fprintf(log_file, "\n %d overfull \\vbox%s", overfull_vbox, (overfull_vbox > 1) ? "es" : "");

        if (underfull_vbox > 0)
          (void) fprintf(log_file, "\n %d underfull \\vbox%s", underfull_vbox, (underfull_vbox > 1) ? "es" : "");

        if (overfull_hbox || underfull_hbox || overfull_vbox || underfull_vbox)
          (void) putc('\n', log_file);
      }
  } /* end of if (log_opened) */ 
#endif /* STAT */

  switch (pdf_output_flag)
  {
    case out_pdf_flag:
      {
        if (total_pages == 0)
        {
          print_nl("No pages of output.");
        }
        else
        {
          HPDF_SaveToFile(yandy_pdf, pdf_file_name);
      
          print_nl("Output written on ");

          if (full_file_name_flag && pdf_file_name != NULL)
            print_char_string((unsigned char *) pdf_file_name);
          else
            slow_print(output_file_name);

          print_string(" (");
          print_int(total_pages);
          print_string(" page");

          if (total_pages != 1)
            print_char('s');
          
          print_string(").");
        }

        HPDF_Free(yandy_pdf);
        font_name_hash_free(gentbl);
      }
      break;
    case out_dvi_flag:
    case out_xdv_flag:
      {
        while (cur_s > -1)
        {
          if (cur_s > 0) 
            dvi_out(142);
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

          if (total_pages >= 65536)    // 99/Oct/10 dvi_t 16 bit problem
          {
            sprintf(log_line, "\nWARNING: page count (dvi_t) in DVI file will be %ld not %ld\n",
              (total_pages % 65536), total_pages);

            if (log_opened)
              (void) fputs (log_line, log_file);

            show_line(log_line, 1);
          }

          dvi_out((total_pages / 256) % 256);
          dvi_out(total_pages % 256);

          if (show_fonts_used && log_opened)     /* 97/Dec/24 */
            show_font_info();           // now in local.c

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

          if (trace_flag) /* 93/Dec/28 - bkph */
          {
            sprintf(log_line, "\ndviwrite %d", dvi_gone);
            show_line(log_line, 0);
          }

          if (dvi_limit == half_buf)
            write_dvi(half_buf, dvi_buf_size - 1);

          if (dvi_ptr > 0)
            write_dvi(0, dvi_ptr - 1); 

          print_nl("Output written on ");

          if (full_file_name_flag && dvi_file_name != NULL)
            print_char_string((unsigned char *) dvi_file_name);
          else
            slow_print(output_file_name);

          print_string(" (");
          print_int(total_pages);
          print_string(" page");

          if (total_pages != 1)
            print_char('s');

          print_string(", ");
          print_int(dvi_offset + dvi_ptr);
          print_string(" bytes).");
          b_close(dvi_file);
        }
      }
      break;
  }

  if (log_opened)
  {
    (void) putc('\n', log_file);
    (void) a_close(log_file);
    selector = selector - 2;

    if (selector == term_only)
    {
      print_nl("Transcript written on ");

      if (full_file_name_flag && log_file_name != NULL)
        print_char_string((unsigned char *) log_file_name);
      else
        slow_print(texmf_log_name);

      print_char('.');
    }
  }

  print_ln();

  if ((edit_name_start != 0) && (interaction > 0))
  {
    call_edit(str_pool, edit_name_start, edit_name_length, edit_line);
  }
}
#ifdef DEBUG
/* sec 1338 */
void debug_help (void) 
{/* 888 10 */ 
  integer k, l, m, n; 
  while (true) {
 ; 
    print_nl(" debug # (-1 to exit):");
#ifndef _WINDOWS
    fflush(stdout); 
#endif
    read(stdin, m);  // ???
    if (m < 0)return; 
    else if (m == 0)
    dumpcore(); 
    else {
      read(stdin, n);  // ???
      switch(m)
      {case 1 : 
  print_word(mem[n]); 
  break; 
      case 2 : 
  print_int(mem[n].hh.lh); 
  break; 
      case 3 : 
  print_int(mem[n].hh.rh); 
  break; 
      case 4 : 
  print_word(eqtb[n]); 
  break; 
      case 5 : 
#ifdef SHORTFONTINFO
  print_scaled(font_info[n].sc);  print_char(' ');
  print_int(font_info[n].qqq.b0);  print_char(':');
  print_int(font_info[n].qqq.b1);  print_char(':');
  print_int(font_info[n].qqq.b2);  print_char(':');
  print_int(font_info[n].qqq.b3);  
#else
  print_word(font_info[n]); 
#endif
  break; 
      case 6 : 
  print_word(save_stack[n]); 
  break; 
      case 7 : 
  show_box(n); 
  break; 
      case 8 : 
  {
    breadth_max = 10000; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
/* About to output node list make some space in string pool 97/Mar/9 */
  if (pool_ptr + 32000 > current_pool_size)
    str_pool = realloc_str_pool (increment_pool_size);
/* We don't bother to check whether this worked */
#endif
#ifdef ALLOCATESTRING
    depth_threshold = current_pool_size - pool_ptr - 10; 
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    depth_threshold = pool_size - pool_ptr - 10; 
#endif
    show_node_list(n); 
  } 
  break; 
      case 9 : 
  show_token_list(n, 0, 1000); 
  break; 
      case 10 : 
  slow_print(n); 
  break; 
      case 11 : 
  check_mem(n > 0); 
  break; 
      case 12 : 
  search_mem(n); 
  break; 
      case 13 : 
  {
    read(stdin, l);  // ???
    print_cmd_chr(n, l); 
  } 
  break; 
      case 14 : 
  {
    register integer for_end; 
    k = 0; 
    for_end = n; 
    if (k <= for_end) 
      do print(buffer[k]); 
    while(k++ < for_end);
  } 
  break; 
      case 15 : 
  {
    font_in_short_display = 0; 
    short_display(n); 
  } 
  break; 
      case 16 : 
  panicking = !panicking; 
  break; 
  default: 
  print(63);    /* ? */
  break; 
      } 
    } 
  } 
} 
#endif /* DEBUG */
