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

#ifdef _WINDOWS
  #define NOCOMM
  #define NOSOUND
  #define NODRIVERS
  #define STRICT
  #pragma warning(disable:4115) // kill rpcasync.h complaint
  #include <windows.h>
  #define MYLIBAPI __declspec(dllexport)
#endif

#pragma warning(disable:4996)
#include <kpathsea/kpathsea.h>
#pragma warning(disable:4131) // old style declarator
#pragma warning(disable:4135) // conversion between different integral types
#pragma warning(disable:4127) // conditional expression is constant

#include <setjmp.h>

#define EXTERN extern

#include "texd.h"

int dot_string = 0;
HPDF_Doc  yandy_pdf;
HPDF_Page yandy_page;
HPDF_Font yandy_font;

char * pdf_char_to_string(unsigned char i)
{
  char * str = malloc(2);
  str[0] = i;
  str[1] = 0;
  return str;
}

void pdf_out(unsigned char i)
{
  char temp[2] = {i, '\0'};
  HPDF_PageAttr attr = (HPDF_PageAttr) yandy_page->attr;
  HPDF_Stream_WriteStr(attr->stream, temp);
}

void pdf_print_octal(integer n)
{
  unsigned int k = 0;

  do
    {
      dig[k] = n % 8;
      n = n / 8;
      incr(k);
    }
  while (n != 0);

  if (k == 1)
  {
    pdf_out('0');
    pdf_out('0');
  }

  if (k == 2)
  {
    pdf_out('0');
  }

  while (k > 0)
  {
    decr(k);
    pdf_out('0' + dig[k]);
  }
}

void pdf_print_int(integer n)
{
  integer k;
  integer m;

  if (n < 0)
  {
    pdf_out('-');
    if (n > -100000000)
      n = -n;
    else
    {
      m = -1 - n;
      n = m / 10;
      m = m % 10 + 1;
      k = 1;
      if (m < 10)
        dig[0] = (char) m;
      else
      {
        dig[0] = 0;
        incr(n);
      }
    }
  }
  do
    {
      dig[k] = n % 10;
      n = n / 10;
      incr(k);
    }
  while (n != 0);

  while (k > 0)
  {
    decr(k);
    pdf_out('0' + dig[k]);
  }
}

void pdf_print_char(unsigned char c)
{
  if ((c < 32) || (c == 92) || (c == 40) || (c == 41) || (c > 127))
  {
    pdf_out(92);
    pdf_print_octal(c);
  }
  else
  {
    pdf_out(c);
  }
}

void pdf_print_string(char * s)
{
  HPDF_PageAttr attr = (HPDF_PageAttr) yandy_page->attr;
  HPDF_Stream_WriteStr(attr->stream, s);
}

void pdf_end_string(void)
{
  if (pdf_doing_string)
  {
    pdf_print_string(")] TJ\012");
    pdf_doing_string = false;
  }
}

void pdf_begin_string(void)
{
  scaled s;

//  if (pdf_doing_text)
//    pdf_begin_text();

  if (f != pdf_f)
  {
    pdf_end_string();
    pdf_font_def(f);
  }

  s = cur_h - pdf_delta_h;

  if (!pdf_doing_string)
  {
    pdf_print_string(" [");
    if (s == 0)
      pdf_out('(');
  }

  if (s != 0)
  {
    if (pdf_doing_string)
      pdf_out(')');
    pdf_print_int(-s);
    pdf_out('(');
    pdf_delta_h = cur_h;
  }

  pdf_doing_string = true;
}

void pdf_error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void * user_data)
{
  printf ("YANDYTEX ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no, (HPDF_UINT)detail_no);
  longjmp(jumpbuffer, 1);
}

void pdf_font_def(internal_font_number f)
{
  int k;
  //char temp[256];
  const char * font_name = NULL;
  char * afm_name = NULL;
  char * pfb_name = NULL;
  
  //for (k = str_start[font_name[f]]; k <= str_start[font_name[f] + 1] - 1; k++)
  //  printf("%d\n", font_name[f]);
  //memmove(temp, str_pool + str_start[font_name[f]], length(font_name[f]));
  //memcpy(temp, (const char *)(str_pool + str_start[font_name[f]]), length(font_name[f]));
  //printf("ZZZ: %s.\n", temp);
  //for (k = str_start[font_area[f]]; k <= str_start[font_area[f] + 1] - 1; k++)
  //temp[] = (str_pool[k]);
  //afm_name = kpse_find_file(strcat(temp, ".afm"), kpse_afm_format, 0);
  //pfb_name = kpse_find_file(strcat(temp, ".pfb"), kpse_type1_format, 0);

  //if (afm_name != NULL && pfb_name != NULL)
  {
  //  font_name = HPDF_LoadType1FontFromFile (yandy_pdf, afm_name, pfb_name);
  //  font_name = HPDF_LoadType1FontFromFile(yandy_pdf, "cmr10.afm", "cmr10.pfb");
  //  yandy_font[f] = HPDF_GetFont(yandy_pdf, font_name, NULL);
  }
  //else
  //{
  //  yandy_font = HPDF_GetFont(yandy_pdf, "Times-Roman", NULL);
  //}

  HPDF_Page_SetFontAndSize(yandy_page, yandy_font, (font_size[f] / 65535));
}

void pdf_ship_out(halfword p)
{
  integer page_loc;
  char j, k;
  pool_pointer s;
  char old_setting;

  if (tracing_output > 0)
  {
    print_nl("");
    print_ln();
    print_string("Completed box being shipped out");
  }

  if (term_offset > max_print_line - 9)
    print_ln();
  else if ((term_offset > 0) || (file_offset > 0))
    print_char(' ');

  print_char('[');
  j = 9;

  while((count(j) == 0) && (j > 0))
    decr(j);

  for (k = 0; k <= j; k++)
  {
    print_int(count(k));

    if (k < j)
      print_char('.');
  }

#ifndef _WINDOWS
  fflush(stdout);
#endif

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

    goto lab30;
  }

  if (height(p) + depth(p) + v_offset > max_v)
    max_v = height(p) + depth(p) + v_offset;

  if (width(p) + h_offset > max_h)
    max_h = width(p) + h_offset;

  dvi_h = 0;
  dvi_v = 0;
  cur_h = h_offset;
  dvi_f = null_font;

  if (output_file_name == 0)
  {
    if (job_name == 0)
      open_log_file();

    pack_job_name(".pdf");

    while(!b_open_out(pdf_file))
    {
      prompt_file_name("file name for output", ".pdf");
    }

    output_file_name = b_make_name_string(pdf_file);
  }

  if (total_pages == 0)
  {
    yandy_pdf = HPDF_New(pdf_error_handler, NULL);
    yandy_pdf->pdf_version = HPDF_VER_17;
    HPDF_SetCompressionMode(yandy_pdf, HPDF_COMP_ALL);
    HPDF_SetInfoAttr(yandy_pdf, HPDF_INFO_PRODUCER, "Y&YTeX 2.2.3");
    yandy_font = HPDF_GetFont(yandy_pdf, HPDF_LoadType1FontFromFile(yandy_pdf, "cmr10.afm", "cmr10.pfb"), NULL);
  }

  //page_loc = dvi_offset + dvi_ptr;
  //dvi_out(bop);
  yandy_page = HPDF_AddPage (yandy_pdf);
  HPDF_Page_SetSize(yandy_page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
  //yandy_font = HPDF_GetFont(yandy_pdf, HPDF_LoadType1FontFromFile(yandy_pdf, "cmr10.afm", "cmr10.pfb"), NULL);
  //HPDF_Page_SetFontAndSize(yandy_page, yandy_font, (10));

  //for (k = 0; k <= 9; k++)
  //  dvi_four(count(k));
  //
  //dvi_four(last_bop);
  //last_bop = page_loc;
  cur_v = height(p) + v_offset;
  temp_ptr = p;

  if (type(p) == vlist_node)
    pdf_vlist_out();
  else
    pdf_hlist_out();

  //dvi_out(eop);
  incr(total_pages);
  cur_s = -1;
lab30:;
  if (tracing_output <= 0)
    print_char(']');

  dead_cycles = 0;

#ifndef _WINDOWS
  fflush(stdout);
#endif

  flush_node_list(p);
}
/*
void pdf_ship_out(pointer p)
{
  integer i, j, k;

  if (tracing_output > 0)
  {
    print_nl("");
    print_ln();
    print_string("Completed box being shipped out");
  }

  if (term_offset > max_print_line - 9)
    print_ln();
  else if ((term_offset > 0) || (file_offset > 0))
    print_char(' ');

  print_char('[');
  j = 9;

  while((count(j) == 0) && (j > 0))
    decr(j);

  for (k = 0; k <= j; k++)
  {
    print_int(count(k));

    if (k < j)
      print_char('.');
  }

#ifndef _WINDOWS
  fflush(stdout);
#endif

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

    goto lab30;
  }

  if (height(p) + depth(p) + v_offset > max_v)
    max_v = height(p) + depth(p) + v_offset;

  if (width(p) + h_offset > max_h)
    max_h = width(p) + h_offset;

  pdf_h = 0;
  pdf_v = 0;
  cur_h = h_offset;
  pdf_f = null_font;

 if (output_file_name == 0)
  {
    if (job_name == 0)
      open_log_file();

    pack_job_name(".pdf");

    while(!b_open_out(dvi_file))
    {
      prompt_file_name("file name for output", ".pdf");
    }

    output_file_name = b_make_name_string(dvi_file);
  }

  if (total_pages == 0)
  {
    yandy_pdf = HPDF_New(pdf_error_handler, NULL);
    yandy_pdf->pdf_version = HPDF_VER_17;
    HPDF_SetCompressionMode(yandy_pdf, HPDF_COMP_ALL);
  }

  yandy_page = HPDF_AddPage (yandy_pdf);
  HPDF_Page_SetWidth (yandy_page, (HPDF_REAL)hsize / 65536);
  HPDF_Page_SetHeight (yandy_page, (HPDF_REAL)vsize / 65536);

  cur_v = height(p) + v_offset;
  temp_ptr = p;

  incr(total_pages);
  cur_s = -1;
lab30:;
  if (tracing_output <= 0)
    print_char(']');

  dead_cycles = 0;

#ifndef _WINDOWS
  fflush(stdout);
#endif

  flush_node_list(p);
}
*/
void pdf_hlist_out (void)
{
  scaled base_line;
  scaled left_edge;
  scaled save_h, save_v;
  halfword this_box;
/*  glue_ord g_order;  */
  int g_order;           /* 95/Jan/7 */
/*  char g_sign;  */
  int g_sign;            /* 95/Jan/7 */
  halfword p;
  integer save_loc;
  halfword leader_box;
  scaled leader_wd;
  scaled lx;
  bool outer_doing_leaders;
  scaled edge;
  real glue_temp;
  real cur_glue;
  scaled cur_g;

  cur_g = 0;
  cur_glue = 0.0;
  this_box = temp_ptr;
  g_order = glue_order(this_box);
  g_sign = glue_sign(this_box);
  p = list_ptr(this_box);
  incr(cur_s);

  if (cur_s > 0)
    dvi_out(141);

  if (cur_s > max_push)
    max_push = cur_s;

  save_loc = dvi_offset + dvi_ptr;
  base_line = cur_v;
  left_edge = cur_h;

  while (p != 0)
lab21:
    if (is_char_node(p))
    {
      synch_h();
      synch_v();

      do
        {
          f = font(p);
          c = character(p);

          if (f != dvi_f)
          {
            if (!font_used[f])
            {
              dvi_font_def(f);
              //pdf_font_def(f);
              font_used[f] = true;
            }

            dvi_f = f;
          }

          //if (c >= 128)
          //  dvi_out(set1);
          //
          //dvi_out(c);
          pdf_font_def(f);
          HPDF_Page_BeginText(yandy_page);
          HPDF_Page_MoveTextPos(yandy_page, (cur_h/65536 + 72), (841.89 - (cur_v/65536 + 72)));
          HPDF_Page_ShowText(yandy_page, pdf_char_to_string(c));
          HPDF_Page_EndText(yandy_page);
          cur_h = cur_h + char_width(f, char_info(f, c));
          p = link(p);
        }
      while(!(!(p >= hi_mem_min)));

      dvi_h = cur_h;
  }
  else
  {
    switch (type(p))
    {
      case hlist_node:
      case vlist_node:
        if (list_ptr(p) == 0)
          cur_h = cur_h + width(p);
        else
        {
          save_h = dvi_h;
          save_v = dvi_v;
          cur_v = base_line + shift_amount(p);
          temp_ptr = p;
          edge = cur_h;

          if (type(p) == vlist_node)
            pdf_vlist_out();
          else
            pdf_hlist_out();

          dvi_h = save_h;
          dvi_v = save_v;
          cur_h = edge + width(p);
          cur_v = base_line;
        }
        break;

      case rule_node:
        {
          rule_ht = height(p);
          rule_dp = depth(p);
          rule_wd = width(p);
          goto lab14;
        }
        break;

      case whatsit_node:
        out_what(p);
        break;

      case glue_node:
        {
          g = glue_ptr(p);
          rule_wd = width(g) - cur_g;

          if (g_sign != normal)
          {
            if (g_sign == stretching)
            {
              if (stretch_order(g) == g_order)
              {
                cur_glue = cur_glue + stretch(g);
                glue_temp = glue_set(this_box) * cur_glue;

                if (glue_temp > 1000000000.0)
                  glue_temp = 1000000000.0;
                else if (glue_temp < -1000000000.0)
                  glue_temp = -1000000000.0;

                cur_g = round(glue_temp);
              }
            }
            else if (shrink_order(g) == g_order)
            {
              cur_glue = cur_glue - shrink(g);
              glue_temp = glue_set(this_box) * cur_glue;

              if (glue_temp > 1000000000.0)
                glue_temp = 1000000000.0;
              else if (glue_temp < -1000000000.0)
                glue_temp = -1000000000.0;

              cur_g = round(glue_temp);
            }
          }

          rule_wd = rule_wd + cur_g;

          if (subtype(p) >= a_leaders)
          {
            leader_box = leader_ptr(p);

            if (type(leader_box) == rule_node)
            {
              rule_ht = height(leader_box);
              rule_dp = depth(leader_box);
              goto lab14;
            }

            leader_wd = width(leader_box);

            if ((leader_wd > 0) && (rule_wd > 0))
            {
              rule_wd = rule_wd + 10;
              edge = cur_h + rule_wd;
              lx = 0;

              if (subtype(p) == a_leaders)
              {
                save_h = cur_h;
                cur_h = left_edge + leader_wd * ((cur_h - left_edge) / leader_wd);

                if (cur_h < save_h)
                  cur_h = cur_h + leader_wd;
              }
              else
              {
                lq = rule_wd / leader_wd;
                lr = rule_wd % leader_wd;

                if (subtype(p) == c_leaders)
                  cur_h = cur_h + (lr / 2);
                else
                {
                  lx =(2 * lr + lq + 1) / (2 * lq + 2);
                  cur_h = cur_h + ((lr - (lq - 1)* lx) / 2);
                }
              }

              while (cur_h + leader_wd <= edge)
              {
                cur_v = base_line + shift_amount(leader_box);
                synch_v();
                save_v = dvi_v;
                synch_h();
                save_h = dvi_h;
                temp_ptr = leader_box;
                outer_doing_leaders = doing_leaders;
                doing_leaders = true;

                if (type(leader_box) == vlist_node)
                  pdf_vlist_out();
                else
                  pdf_hlist_out();

                doing_leaders = outer_doing_leaders;
                dvi_v = save_v;
                dvi_h = save_h;
                cur_v = base_line;
                cur_h = save_h + leader_wd + lx;
              }

              cur_h = edge - 10;
              goto lab15;
            }
          }

          goto lab13;
        }
        break;

      case kern_node:
      case math_node:
        cur_h = cur_h + width(p);
        break;

      case ligature_node:
        {
          mem[lig_trick] = mem[lig_char(p)];
          link(lig_trick) = link(p);
          p = lig_trick;
          goto lab21;
        }
        break;

      default:
        break;
    }

    goto lab15;
lab14:
    if ((rule_ht == -1073741824L))  /* - 2^30 */
      rule_ht = height(this_box);

    if ((rule_dp == -1073741824L))     /* - 2^30 */
      rule_dp = depth(this_box);

    rule_ht = rule_ht + rule_dp;

    if ((rule_ht > 0) && (rule_wd > 0))
    {
      synch_h();
      cur_v = base_line + rule_dp;
      synch_v();
      //dvi_out(set_rule);
      //dvi_four(rule_ht);
      //dvi_four(rule_wd);
      HPDF_Page_SetLineWidth(yandy_page, rule_ht / 65535);
      HPDF_Page_MoveTo (yandy_page, (cur_h / 65535 + 72), (841.89 - cur_v / 65535 - 72));
      HPDF_Page_LineTo (yandy_page, (cur_h / 65535 + 72 + rule_wd / 65535), (841.89 - cur_v / 65535 - 72));
      HPDF_Page_Stroke (yandy_page);
      cur_v = base_line;
      dvi_h = dvi_h + rule_wd;
    }
lab13:
    cur_h = cur_h + rule_wd;
lab15:
    p = link(p);
  }

  prune_movements(save_loc);

  if (cur_s > 0)
    dvi_pop(save_loc);

  decr(cur_s);
}
/* following needs access to dvi_buf=zdvibuf see coerce.h */
/* sec 0629 */
void pdf_vlist_out (void)
{
  scaled left_edge;
  scaled top_edge;
  scaled save_h, save_v;
  halfword this_box;
/*  glue_ord g_order;  */
  int g_order;         /* 95/Jan/7 */
/*  char g_sign;  */
  int g_sign;          /* 95/Jan/7 */
  halfword p;
  integer save_loc;
  halfword leader_box;
  scaled leader_ht;
  scaled lx;
  bool outer_doing_leaders;
  scaled edge;
  real glue_temp;
  real cur_glue;
  scaled cur_g;

  cur_g = 0;
  cur_glue = 0.0;
  this_box = temp_ptr;
  g_order = glue_order(this_box);
  g_sign = glue_sign(this_box);
  p = list_ptr(this_box);
  incr(cur_s);

  if (cur_s > 0)
    dvi_out(141);

  if (cur_s > max_push)
    max_push = cur_s;

  save_loc = dvi_offset + dvi_ptr;
  left_edge = cur_h;
  cur_v = cur_v - height(this_box);
  top_edge = cur_v;

  while (p != 0)
  {
    if ((p >= hi_mem_min))
    {
      confusion("vlistout");
      return;       // abort_flag set
    }
    else
    {
      switch (type(p))
      {
        case hlist_node:
        case vlist_node:
          if (list_ptr(p) == 0)
            cur_v = cur_v + height(p) + depth(p);
          else
          {
            cur_v = cur_v + height(p);
            synch_v();
            save_h = dvi_h;
            save_v = dvi_v;
            cur_h = left_edge + shift_amount(p);
            temp_ptr = p;

            if (type(p) == vlist_node)
              pdf_vlist_out();
            else
              pdf_hlist_out();

            dvi_h = save_h;
            dvi_v = save_v;
            cur_v = save_v + depth(p);
            cur_h = left_edge;
          }
          break;

        case rule_node:
          {
            rule_ht = height(p);
            rule_dp = depth(p);
            rule_wd = width(p);
            goto lab14;
          }
          break;

        case whatsit_node:
          out_what(p);
          break;

        case glue_node:
          {
            g = glue_ptr(p);
            rule_ht = width(g) - cur_g;

            if (g_sign != normal)
            {
              if (g_sign == stretching)
              {
                if (stretch_order(g) == g_order)
                {
                  cur_glue = cur_glue + stretch(g);
                  glue_temp = glue_set(this_box) * cur_glue;

                  if (glue_temp > 1000000000.0)
                    glue_temp = 1000000000.0;
                  else if (glue_temp < -1000000000.0)
                    glue_temp = -1000000000.0;

                  cur_g = round(glue_temp);
                }
              }
              else if (shrink_order(g) == g_order)   /* BUG FIX !!! */
              {
                cur_glue = cur_glue - shrink(g);
                glue_temp = glue_set(this_box) * cur_glue;

                if (glue_temp > 1000000000.0)
                  glue_temp = 1000000000.0;
                else if (glue_temp < -1000000000.0)
                  glue_temp = -1000000000.0;

                cur_g = round(glue_temp);
              }
            }

            rule_ht = rule_ht + cur_g;

            if (subtype(p) >= a_leaders)
            {
              leader_box = leader_ptr(p);

              if (type(leader_box) == rule_node)
              {
                rule_wd = width(leader_box);
                rule_dp = 0;
                goto lab14;
              }

              leader_ht = height(leader_box) + depth(leader_box);

              if ((leader_ht > 0) && (rule_ht > 0))
              {
                rule_ht = rule_ht + 10;
                edge = cur_v + rule_ht;
                lx = 0;

                if (subtype(p) == a_leaders)
                {
                  save_v = cur_v;
                  cur_v = top_edge + leader_ht * ((cur_v - top_edge) / leader_ht);

                  if (cur_v < save_v)
                    cur_v = cur_v + leader_ht;
                }
                else
                {
                  lq = rule_ht / leader_ht;
                  lr = rule_ht % leader_ht;

                  if (subtype(p) == c_leaders)
                    cur_v = cur_v + (lr / 2);
                  else
                  {
                    lx = (2 * lr + lq + 1) / (2 * lq + 2);
                    cur_v = cur_v + ((lr - (lq - 1) * lx) / 2);
                  }
                }

                while (cur_v + leader_ht <= edge)
                {
                  cur_h = left_edge + shift_amount(leader_box);
                  synch_h();
                  save_h = dvi_h;
                  cur_v = cur_v + height(leader_box);
                  synch_v();
                  save_v = dvi_v;
                  temp_ptr = leader_box;
                  outer_doing_leaders = doing_leaders;
                  doing_leaders = true;

                  if (type(leader_box) == vlist_node)
                    pdf_vlist_out();
                  else
                    pdf_hlist_out();

                  doing_leaders = outer_doing_leaders;
                  dvi_v = save_v;
                  dvi_h = save_h;
                  cur_h = left_edge;
                  cur_v = save_v - height(leader_box) + leader_ht + lx;
                }

                cur_v = edge - 10;
                goto lab15;
              }
            }

            goto lab13;
          }
          break;

        case kern_node:
          cur_v = cur_v + width(p);
          break;

        default:
          break;
      }
      goto lab15;
lab14:
      if ((rule_wd == -1073741824L))    /* -2^30 */
        rule_wd = width(this_box);

      rule_ht = rule_ht + rule_dp;
      cur_v = cur_v + rule_ht;

      if ((rule_ht > 0) && (rule_wd > 0))
      {
        //synch_h();
        //synch_v();
        //dvi_out(put_rule);
        //dvi_four(rule_ht);
        //dvi_four(rule_wd);
        HPDF_Page_SetLineWidth(yandy_page, rule_ht / 65535);
        HPDF_Page_MoveTo (yandy_page, (cur_h / 65535 + 72), (841.89 - cur_v / 65535 - 72));
        HPDF_Page_LineTo (yandy_page, (cur_h / 65535 + 72 + rule_wd / 65535), (841.89 - cur_v / 65535 - 72));
        HPDF_Page_Stroke (yandy_page);
      }

      goto lab15;
lab13:
      cur_v = cur_v + rule_ht;
    }
lab15:
    p = link(p);
  }

  prune_movements(save_loc);

  if (cur_s > 0)
    dvi_pop(save_loc);

  decr(cur_s);
}