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

bool pdf_doing_string;
bool pdf_doing_text;
static integer ten_pow[10] =
{
  1,
  10,
  100,
  1000,
  10000,
  100000,
  1000000,
  10000000,
  100000000,
  1000000000
};
integer scaled_out;
HPDF_Doc  yandy_pdf;
HPDF_Page yandy_page;
HPDF_Font yandy_font[1024];
tree *avl_tree = NULL;

struct tfm_map
{
  char * tfm_name;
  unsigned int key;
};

int tfm_cmp(void * a, void * b)
{
  char * aa = ((struct tfm_map *) (a))->tfm_name;
  char * bb = ((struct tfm_map *) (b))->tfm_name;

  return (strcmp(aa, bb)) ;
}

void tfm_print(void *d)
{
  struct tfm_map * dd = (struct tfm_map *) d;

  if (dd)
    printf("{ %s => %d }\n", dd->tfm_name, dd->key);
}

void tfm_delete(void *d)
{
  struct tfm_map *dd = (struct tfm_map *) d;

  if (dd)
  {
    free(dd);
  }
}

void tfm_copy(void *src, void *dst)
{
  struct tfm_map *s = (struct tfm_map *) src;
  struct tfm_map *d = (struct tfm_map *) dst;
  d->tfm_name = s->tfm_name;
  d->key = s->key;
}

void init_tfm_map(void)
{
  avl_tree = init_dictionnary(tfm_cmp, tfm_print, tfm_delete, tfm_copy);
}

void free_tfm_map(void)
{
  delete_tree(avl_tree);
}

int insert_font_index(char * name)
{
  struct tfm_map nn;
  nn.tfm_name = name;
  nn.key = avl_tree->count + 1;

  return insert_elmt(avl_tree, &nn, sizeof(struct tfm_map));
}

int get_font_index(char * name)
{
  struct tfm_map nn;

  nn.tfm_name = name;
  nn.key = -1;

  if (is_present(avl_tree, &nn))
  {
    if (get_data(avl_tree, &nn, sizeof(struct tfm_map)))
      return nn.key;
    else
      return 0;
  }

  return 0;
}
// report error.
void pdf_error(const char * t, const char * p)
{
  normalize_selector();
  print_err("Y&Y TeX error");

  if (t != NULL)
  {
    print('(');
    print_string(t);
    print(')');
  }

  print_string(": ");
  print_string(p);
  succumb();
}
// char to string.
char * pdf_char_to_string(unsigned char i)
{
  char * str = (char *) malloc(2);
  str[0] = i;
  str[1] = 0;
  return str;
}
// output one char: normal.
void pdf_out(unsigned char i)
{
  HPDF_PageAttr attr = (HPDF_PageAttr) yandy_page->attr;
  HPDF_Stream_WriteChar(attr->stream, i);
}
// octal number: 032, 009 etc.
void pdf_print_octal(integer s)
{
  char buf[HPDF_INT_LEN + 1];
  HPDF_PageAttr attr = (HPDF_PageAttr) yandy_page->attr;
  sprintf(buf, "%03o", s);
  HPDF_Stream_Write(attr->stream, (HPDF_BYTE *)buf, strlen(buf));
}
// output one char: normal or octal.
void pdf_print_char(unsigned char s)
{
  // 32 = '', 92 = '\', 40 = '(', 41 = ')'
  if ((s <= 32) || (s == 92) || (s == 40) || (s == 41) || (s > 127))
  {
    pdf_out(92);
    pdf_print_octal(s);
  }
  else
  {
    pdf_out(s);
  }
}
// equivlent: pdf_print in pdfTeX.
void pdf_print_string(char * s)
{
  HPDF_PageAttr attr = (HPDF_PageAttr) yandy_page->attr;
  HPDF_Stream_WriteStr(attr->stream, s);
}
// print one integer value: signed or unsigned.
void pdf_print_int(int s)
{
  char buf[HPDF_INT_LEN + 1];
  HPDF_PageAttr attr = (HPDF_PageAttr) yandy_page->attr;
  char* p = HPDF_IToA(buf, s, buf + HPDF_INT_LEN);
  HPDF_Stream_Write(attr->stream, (HPDF_BYTE *)buf, (HPDF_UINT)(p - buf));
}
// translate sp to bp.
HPDF_REAL pdf_sp_to_bp(scaled s)
{
  // 1 bp = 65781.76 sp
  return (HPDF_REAL) s / 65781.76;
}
// divides scaled s by scaled m.
scaled divide_scaled(scaled s, scaled m, integer dd)
{
  scaled q, r;
  integer sign, i;

  sign = 1;

  if (s < 0)
  {
    sign = -sign;
    s = -s;
  }

  if (m < 0)
  {
    sign = -sign;
    m = -m;
  }

  if (m == 0)
    pdf_error("arithmetic", "divided by zero");
  else if (m >= 0x7FFFFFFF / 10)
    pdf_error("arithmetic", "number too big");

  q = s / m;
  r = s % m;

  for (i = 1; i <= dd; i++)
  {
    q = 10 * q + (10 * r) / m;
    r = (10 * r) % m;
  }

  if (2 * r >= m)
  {
    incr(q);
    r = r - m;
  }

  scaled_out = sign * (s - (r / ten_pow[dd]));
  return sign * q;
}
scaled round_xn_over_d(scaled x, integer n, integer d)
{
  bool positive;
  nonnegative_integer t, u, v;

  if (x >= 0)
    positive = true; 
  else
  {
    x = - (integer) x;
    positive = false;
  }

  t = (x % 32767L) * n;
  u = (x / 32768L) * n + (t / 32768L);
  v = (u % d) * 32768L + (t % 32768L); 

  if (u / d >= 32768L)
    arith_error = true; 
  else
    u = 32768L * (u / d) + (v / d);

  v = v % d;

  if (2 * v >= d)
    incr(u);

  if (positive)
    return u;
  else
    return -u;
}
// advance char width
void adv_char_width(internal_font_number f, eight_bits c)
{
  scaled w, s_out;
  integer s;

  w = char_width(f, char_info(f, c));
  divide_scaled(w, font_size[f], 4);
  pdf_delta_h = pdf_delta_h + scaled_out;
}
// print real value
void pdf_print(integer m, integer d)
{
  if (m < 0)
  {
    pdf_out('-');
    m = -m;
  }

  pdf_print_int(m / ten_pow[d]);
  m = m % ten_pow[d];

  if (m > 0)
  {
    pdf_out('.');
    decr(d);

    while (m < ten_pow[d])
    {
      pdf_out('0');
      decr(d);
    }

    while (m % 10 == 0)
      m = m / 10;

    pdf_print_int(m);
  }
}
// end the current string
void pdf_end_string(void)
{
  if (pdf_doing_string)
  {
    pdf_print_string(")] TJ\012");
    pdf_doing_string = false;
  }
}
// begin to draw a string
void pdf_begin_string(internal_font_number f)
{
  scaled s_out, v, v_out;
  integer s;

  if (!pdf_doing_text)
//    pdf_begin_text();

  if (f != dvi_f)
  {
    pdf_end_string();
    pdf_font_def(f);
  }

  s = divide_scaled(cur_h - pdf_delta_h, font_size[f], 3);
  s_out = scaled_out;

  if (abs(s) < 0100000)
  {
    s_out = divide_scaled(round_xn_over_d(cur_h - pdf_delta_h, 1000, 1000),
      font_size[f], 3);

    if (s < 0)
      s_out = -s_out;
  }

//  if (cur_v - pdf_v >= )
}
// end a text section.
void pdf_end_text()
{
  if (pdf_doing_text)
  {
    pdf_end_string();
    HPDF_Page_EndText(yandy_page);
    pdf_doing_text = false;
  }
}
// draw a rule.
void pdf_set_rule(scaled x, scaled y, scaled w, scaled h)
{
}
void pdf_error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void * user_data)
{
  printf ("Y&Y TeX error: error_no=%04X, detail_no=%u\n",
    (HPDF_UINT)error_no,
    (HPDF_UINT)detail_no);
  longjmp(jumpbuffer, 1);
}

void pdf_font_def(internal_font_number f)
{
  int k;
  const char * fnt_name;
  char * afm_name;
  char * pfb_name;
  char buffer[256];
  
  memcpy(buffer, (const char *) str_pool + str_start[font_name[f]], length(font_name[f]));
  buffer[length(font_name[f])] = '\0';

  k = get_font_index(buffer);
  printf("DEF: %s--%d.\n", buffer, k);

  if (k == 0)
  {
    afm_name = kpse_find_file(strcat(strdup(buffer), ".afm"), kpse_afm_format, 1);
    printf("path: %s.\n", afm_name);
    pfb_name = kpse_find_file(strcat(strdup(buffer), ".pfb"), kpse_type1_format, 1);

    if (afm_name != NULL && pfb_name != NULL)
    {
      k = insert_font_index(buffer);
      fnt_name = HPDF_LoadType1FontFromFile (yandy_pdf, afm_name, pfb_name);
      yandy_font[k] = HPDF_GetFont(yandy_pdf, fnt_name, NULL);
    }
  }

  HPDF_Page_SetFontAndSize(yandy_page, yandy_font[k], (font_size[f] / 65535));
}

void pdf_ship_out(halfword p)
{
  char j, k;

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
    init_tfm_map();
    yandy_pdf = HPDF_New (pdf_error_handler, NULL);
    HPDF_SetCompressionMode (yandy_pdf, HPDF_COMP_ALL);
    yandy_pdf -> pdf_version = HPDF_VER_17;
    HPDF_SetInfoAttr(yandy_pdf, HPDF_INFO_PRODUCER, "Y&Y TeX");
    yandy_font[0] = HPDF_GetFont (yandy_pdf, "Times-Roman", NULL);
  }

  yandy_page = HPDF_AddPage (yandy_pdf);
  HPDF_Page_SetSize (yandy_page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

  cur_v = height(p) + v_offset;
  temp_ptr = p;

  if (type(p) == vlist_node)
    pdf_vlist_out();
  else
    pdf_hlist_out();

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
  //integer save_loc;
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

  base_line = cur_v;
  left_edge = cur_h;

  while (p != 0)
lab21:
    if (is_char_node(p))
    {
      do
        {
          f = font(p);
          c = character(p);

          if (f != dvi_f)
          {
            pdf_font_def(f);

            if (!font_used[f])
            {
              font_used[f] = true;
            }

            dvi_f = f;
          }
          
          HPDF_Page_SetFontAndSize (yandy_page, yandy_font[dvi_f], 10);
          HPDF_Page_BeginText(yandy_page);
          HPDF_Page_MoveTextPos(yandy_page, pdf_sp_to_bp(cur_h) + 72, (841.89 - (pdf_sp_to_bp(cur_v) + 72)));
          HPDF_Page_ShowText(yandy_page, pdf_char_to_string(c));
          HPDF_Page_EndText(yandy_page);
          cur_h = cur_h + char_width(f, char_info(f, c));
          p = link(p);
        }
      while(((p >= hi_mem_min)));

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
        //out_what(p);
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
                //synch_v();
                dvi_v = cur_v;
                save_v = dvi_v;
                //synch_h();
                dvi_h = cur_h;
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
      cur_v = base_line + rule_dp;
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
  //integer save_loc;
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
            //synch_v();
            dvi_v = cur_v;
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
          //out_what(p);
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
                  //synch_h();
                  dvi_h = cur_h;
                  save_h = dvi_h;
                  cur_v = cur_v + height(leader_box);
                  //synch_v();
                  dvi_v = cur_v;
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
        HPDF_Page_SetLineWidth(yandy_page, rule_ht / 65535);
        HPDF_Page_MoveTo(yandy_page, (cur_h / 65535 + 72), (841.89 - cur_v / 65535 - 72));
        HPDF_Page_LineTo(yandy_page, (cur_h / 65535 + 72 + rule_wd / 65535), (841.89 - cur_v / 65535 - 72));
        HPDF_Page_Stroke(yandy_page);
      }

      goto lab15;
lab13:
      cur_v = cur_v + rule_ht;
    }
lab15:
    p = link(p);
  }

  decr(cur_s);
}
