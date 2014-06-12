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

/* sec 1181 */
void math_fraction (void)
{
  small_number c;

  c = cur_chr;

  if (incompleat_noad != 0)
  {
    if (c >= delimited_code)
    {
      scan_delimiter(garbage, false);
      scan_delimiter(garbage, false);
    }

    if (c % delimited_code == 0)
      scan_dimen(false, false, false);

    print_err("Ambiguous; you need another { and }");
    help3("I'm ignoring this fraction specification, since I don't",
      "know whether a construction like `x \\over y \\over z'",
      "means `{x \\over y} \\over z' or `x \\over {y \\over z}'.");
    error();
  }
  else
  {
    incompleat_noad = get_node(fraction_noad_size);
    type(incompleat_noad) = fraction_noad;
    subtype(incompleat_noad) = normal;
    math_type(numerator(incompleat_noad)) = sub_mlist;
    info(numerator(incompleat_noad)) = link(head);
    mem[denominator(incompleat_noad)].hh = empty_field;
    mem[left_delimiter(incompleat_noad)].qqqq = null_delimiter;
    mem[right_delimiter(incompleat_noad)].qqqq = null_delimiter;
    link(head) = 0;
    tail = head;

    if (c >= delimited_code)
    {
      scan_delimiter(left_delimiter(incompleat_noad), false);
      scan_delimiter(right_delimiter(incompleat_noad), false);
    }

    switch (c % delimited_code)
    {
      case above_code:
        scan_dimen(false, false, false);
        thickness(incompleat_noad) = cur_val;
        break;

      case over_code:
        thickness(incompleat_noad) = default_code;
        break;

      case atop_code:
        thickness(incompleat_noad) = 0;
        break;
    }
  }
}
/* sec 1191 */
void math_left_right (void)
{
  small_number t;
  pointer p;

  t = cur_chr;

  if ((t == right_noad) && (cur_group != math_left_group))
  {
    if (cur_group == math_shift_group)
    {
      scan_delimiter(garbage, false);
      print_err("Extra ");
      print_esc("right");
      help1("I'm ignoring a \\right that had no matching \\left.");
      error();
    }
    else
      off_save();
  }
  else
  {
    p = new_noad();
    type(p) = t;
    scan_delimiter(delimiter(p), false);

    if (t == left_noad)
    {
      push_math(math_left_group);
      link(tail) = p;
      tail = p;
    }
    else
    {
      p = fin_mlist(p);
      unsave();
      tail_append(new_noad());
      type(tail) = inner_noad;
      math_type(nucleus(tail)) = sub_mlist;
      info(nucleus(tail)) = p;
    }
  }
}
/* sec 1194 */
void after_math (void)
{
  boolean l;
  boolean danger;
  integer m;
  pointer p;
  pointer a;
  pointer b;
  scaled w;
  scaled z;
  scaled e;
  scaled q;
  scaled d;
  scaled s;
  small_number g1, g2;
  pointer r;
  pointer t;

  danger = false;
  
  if ((font_params[fam_fnt(2 + text_size)] < total_mathsy_params) ||
    (font_params[fam_fnt(2 + script_size)] < total_mathsy_params) ||
    (font_params[fam_fnt(2 + script_script_size)] < total_mathsy_params))
  {
    print_err("Math formula deleted: Insufficient symbol fonts");
    help3("Sorry, but I can't typeset math unless \\textfont 2",
        "and \\scriptfont 2 and \\scriptscriptfont 2 have all",
        "the \\fontdimen values needed in math symbol fonts.");
    error();
    flush_math();
    danger = true;
  }
  else if ((font_params[fam_fnt(3 + text_size)] < total_mathex_params) ||
    (font_params[fam_fnt(3 + script_size)] < total_mathex_params) ||
    (font_params[fam_fnt(3 + script_script_size)] < total_mathex_params))
  {
    print_err("Math formula deleted: Insufficient extension fonts");
    help3("Sorry, but I can't typeset math unless \\textfont 3",
        "and \\scriptfont 3 and \\scriptscriptfont 3 have all",
        "the \\fontdimen values needed in math extension fonts.");
    error();
    flush_math();
    danger = true;
  }

  m = mode;
  l = false;
  p = fin_mlist(0);

  if (mode == -m)
  {
    {
      get_x_token();

      if (cur_cmd != math_shift)
      {
        print_err("Display math should end with $$");
        help2("The `$' that I just saw supposedly matches a previous `$$'.",
            "So I shall assume that you typed `$$' both times.");
        back_error();
      }
    }

    cur_mlist = p;
    cur_style = text_style;
    mlist_penalties = false;
    mlist_to_hlist();
    a = hpack(link(temp_head), 0, 1);
    unsave();
    decr(save_ptr);

    if (saved(0) == 1)
      l = true;

    danger = false;

    if ((font_params[fam_fnt(2 + text_size)] < total_mathsy_params) ||
      (font_params[fam_fnt(2 + script_size)] < total_mathsy_params) ||
      (font_params[fam_fnt(2 + script_script_size)] < total_mathsy_params))
    {
      print_err("Math formula deleted: Insufficient symbol fonts");
      help3("Sorry, but I can't typeset math unless \\textfont 2",
          "and \\scriptfont 2 and \\scriptscriptfont 2 have all",
          "the \\fontdimen values needed in math symbol fonts.");
      error();
      flush_math();
      danger = true;
    }
    else if ((font_params[fam_fnt(3 + text_size)] < total_mathex_params) ||
      (font_params[fam_fnt(3 + script_size)] < total_mathex_params) ||
      (font_params[fam_fnt(3 + script_script_size)] < total_mathex_params))
    {
      print_err("Math formula deleted: Insufficient extension fonts");
      help3("Sorry, but I can't typeset math unless \\textfont 3",
        "and \\scriptfont 3 and \\scriptscriptfont 3 have all",
        "the \\fontdimen values needed in math extension fonts.");
      error();
      flush_math();
      danger = true;
    }

    m = mode;
    p = fin_mlist(0);
  }
  else
    a = 0;

  if (m < 0)
  {
    tail_append(new_math(math_surround, 0));
    cur_mlist = p;
    cur_style = text_style;
    mlist_penalties = (mode > 0);
    mlist_to_hlist();
    link(tail) = link(temp_head);

    while (link(tail) != 0)
      tail = link(tail);

    tail_append(new_math(math_surround, 1));
    space_factor = 1000;
    unsave();
  }
  else
  {
    if (a == 0)
    {
      get_x_token();

      if (cur_cmd != math_shift)
      {
        print_err("Display math should end with $$");
        help2("The `$' that I just saw supposedly matches a previous `$$'.",
            "So I shall assume that you typed `$$' both times.");
        back_error();
      }
    }

    cur_mlist = p;
    cur_style = display_style;
    mlist_penalties = false;
    mlist_to_hlist();
    p = link(temp_head);
    adjust_tail = adjust_head;
    b = hpack(p, 0, 1);
    p = list_ptr(b);
    t = adjust_tail;
    adjust_tail = 0;
    w = width(b);
    z = display_width;
    s = display_indent;

    if ((a == 0) || danger)
    {
      e = 0;
      q = 0;
    }
    else
    {
      e = width(a);
      q = e + math_quad(text_size);
    }

    if (w + q > z)
    {
      if ((e != 0) && ((w - total_shrink[normal] + q <= z) || (total_shrink[fil] != 0) ||
        (total_shrink[fill] != 0) || (total_shrink[filll] != 0)))
      {
        free_node(b, box_node_size);
        b = hpack(p, z - q, 0);
      }
      else
      {
        e = 0;

        if (w > z)
        {
          free_node(b, box_node_size);
          b = hpack(p, z, 0);
        }
      }
      w = width(b);
    }

    d = half(z - w);

    if ((e > 0) && (d < 2 * e))
    {
      d = half(z - w - e);

      if (p != 0)
        if (!is_char_node(p))
          if (type(p) == glue_node)
            d = 0;
    }

    tail_append(new_penalty(pre_display_penalty));

    if ((d + s <= pre_display_size) || l)
    {
      g1 = above_display_skip_code;
      g2 = below_display_skip_code;
    }
    else
    {
      g1 = above_display_short_skip_code;
      g2 = below_display_short_skip_code;
    }
    if (l && (e == 0))
    {
      shift_amount(a) = s;
      append_to_vlist(a);
      tail_append(new_penalty(10000));
    }
    else
    {
      tail_append(new_param_glue(g1));
    }

    if (e != 0)
    {
      r = new_kern(z - w - e - d);

      if (l)
      {
        link(a) = r;
        link(r) = b;
        b = a;
        d = 0;
      }
      else
      {
        link(b) = r;
        link(r) = a;
      }
      b = hpack(b, 0, 1);
    }

    shift_amount(b) = s + d;
    append_to_vlist(b);

    if ((a != 0) && (e == 0) && !l)
    {
      tail_append(new_penalty(10000));
      shift_amount(a) = s + z - width(a);
      append_to_vlist(a);
      g2 = 0;
    }

    if (t != adjust_head)
    {
      link(tail) = link(adjust_head);
      tail = t;
    }

    tail_append(new_penalty(post_display_penalty));

    if (g2 > 0)
    {
      tail_append(new_param_glue(g2));
    }

    resume_after_display();
  }
}
/* sec 1200 */
void resume_after_display (void)
{
  if (cur_group != math_shift_group)
  {
    confusion("display");
    return;
  }

  unsave();
  prev_graf = prev_graf + 3;
  push_nest();
  mode = hmode;
  space_factor = 1000;
  set_cur_lang();
  clang = cur_lang;
  prev_graf =(norm_min(left_hyphen_min) * 64 + norm_min(right_hyphen_min)) * 65536L + cur_lang;

  {
    get_x_token();

    if (cur_cmd != spacer)
      back_input();
  }

  if (nest_ptr == 1)
    build_page();
}
/* sec 1215 */
void get_r_token (void)
{
lab20:
  do
    {
      get_token();
    }
  while (!(cur_tok != space_token));

  if ((cur_cs == 0) || (cur_cs > frozen_control_sequence))
  {
    print_err("Missing control sequence inserted");
    help5("Please don't say `\\def cs{...}', say `\\def\\cs{...}'.",
      "I've inserted an inaccessible control sequence so that your",
      "definition will be completed without mixing me up too badly.",
      "You can recover graciously from this error, if you're",
      "careful; see exercise 27.2 in The TeXbook.");

    if (cur_cs == 0)
      back_input();

    cur_tok = cs_token_flag + frozen_protection;
    ins_error();
    goto lab20;
  }
}
/* sec 1229 */
void trap_zero_glue (void)
{
  if ((width(cur_val) == 0) && (stretch(cur_val) == 0) && (shrink(cur_val) == 0))
  {
    add_glue_ref(zero_glue);
    delete_glue_ref(cur_val);
    cur_val = 0;
  }
}
/* sec 1236 */
void do_register_command_ (small_number a)
{
  pointer l, q, r, s;
  char p;

  q = cur_cmd;

  {
    if (q != tex_register)
    {
      get_x_token();

      if ((cur_cmd >= assign_int) && (cur_cmd <= assign_mu_glue))
      {
        l = cur_chr;
        p = cur_cmd - assign_int;
        goto lab40;
      }

      if (cur_cmd != tex_register)
      {
        print_err("You can't use `");
        print_cmd_chr(cur_cmd, cur_chr);
        print_string("' after ");
        print_cmd_chr(q, 0);
        help1("I'm forgetting what you said and not changing anything.");
        error();
        return;
      }
    }

    p = cur_chr;
    scan_eight_bit_int();

    switch (p)
    {
      case int_val:
        l = cur_val + count_base;
        break;

      case dimen_val:
        l = cur_val + scaled_base;
        break;

      case glue_val:
        l = cur_val + skip_base;
        break;

      case mu_val:
        l = cur_val + mu_skip_base;
        break;
    }
  }

lab40:
  if (q == tex_register)
    scan_optional_equals();
  else
    if (scan_keyword("by"));

  arith_error = false;

  if (q < multiply)
    if (p < glue_val)
    {
      if (p == int_val)
        scan_int();
      else
        scan_dimen(false, false, false);

      if (q == advance)
        cur_val = cur_val + eqtb[l].cint;
    }
    else
    {
      scan_glue(p);

      if (q == advance)
      {
        q = new_spec(cur_val);
        r = equiv(l);
        delete_glue_ref(cur_val);
        width(q) = width(q) + width(r);

        if (stretch(q) == 0)
          stretch_order(q) = normal;

        if (stretch_order(q) == stretch_order(r))
          stretch(q) = stretch(q) + stretch(r);
        else if ((stretch_order(q) < stretch_order(r)) && (stretch(r) != 0))
        {
          stretch(q) = stretch(r);
          stretch_order(q) = stretch_order(r);
        }

        if (shrink(q) == 0)
          shrink_order(q) = normal;

        if (shrink_order(q) == shrink_order(r))
          shrink(q) = shrink(q) + shrink(r);
        else if ((shrink_order(q) < shrink_order(r)) && (shrink(r) != 0))
        {
          shrink(q) = shrink(r);
          shrink_order(q) = shrink_order(r);
        }
        cur_val = q;
      }
    }
  else
  {
    scan_int();

    if (p < glue_val)
      if (q == multiply)
        if (p == int_val)
          cur_val = mult_and_add(eqtb[l].cint, cur_val, 0, 2147483647L); /*  2^31 - 1 */
        else
          cur_val = mult_and_add(eqtb[l].cint, cur_val, 0, 1073741823L); /*  2^30 - 1 */
      else
        cur_val = x_over_n(eqtb[l].cint, cur_val);
    else
    {
      s = equiv(l);
      r = new_spec(s);

      if (q == multiply)
      {
        width(r) = mult_and_add(width(s), cur_val, 0, 1073741823L);  /* 2^30 - 1 */
        stretch(r) = mult_and_add(stretch(s), cur_val, 0, 1073741823L);  /* 2^30 - 1 */
        shrink(r) = mult_and_add(shrink(s), cur_val, 0, 1073741823L);  /* 2^30 - 1 */
      }
      else
      {
        width(r) = x_over_n(width(s), cur_val);
        stretch(r) = x_over_n(stretch(s), cur_val);
        shrink(r) = x_over_n(shrink(s), cur_val);
      }
      cur_val = r;
    }
  }

  if (arith_error)
  {
    print_err("Arithmetic overflow");
    help2("I can't carry out that multiplication or division,",
        "since the result is out of range.");

    if (p >= glue_val)
      delete_glue_ref(cur_val);

    error();
    return;
  }

  if (p < glue_val)
    word_define(l, cur_val);
  else
  {
    trap_zero_glue();
    define(l, glue_ref, cur_val);
  }
}
/* sec 1243 */
void alter_aux (void)
{
  halfword c;

  if (cur_chr != abs(mode))
    report_illegal_case();
  else
  {
    c = cur_chr;
    scan_optional_equals();

    if (c == vmode)
    {
      scan_dimen(false, false, false);
      prev_depth = cur_val;
    }
    else
    {
      scan_int();

      if ((cur_val <= 0) || (cur_val > 32767))
      {
        print_err("Bad space factor");
        help1("I allow only values in the range 1..32767 here.");
        int_error(cur_val);
      }
      else
        space_factor = cur_val;
    }
  }
}
/* sec 1244 */
void alter_prev_graf (void)
{
  integer p;

  nest[nest_ptr] = cur_list;
  p = nest_ptr;

  while (abs(nest[p].mode_field) != vmode)
    decr(p);

  scan_optional_equals();
  scan_int();

  if (cur_val < 0)
  {
    print_err("Bad ");
    print_esc("prevgraf");
    help1("I allow only nonnegative values here.");
    int_error(cur_val);
  }
  else
  {
    nest[p].pg_field = cur_val;
    cur_list = nest[nest_ptr];
  }
}
/* sec 1245 */
void alter_page_so_far (void)
{
  char c;

  c = cur_chr;
  scan_optional_equals();
  scan_dimen(false, false, false);
  page_so_far[c] = cur_val;
}
/* sec 1246 */
void alter_integer (void)
{
  char c;

  c = cur_chr;
  scan_optional_equals();
  scan_int();

  if (c == 0)
    dead_cycles = cur_val;
  else
    insert_penalties = cur_val;
}
/* sec 1247 */
void alter_box_dimen (void)
{
  small_number c;
  eight_bits b;

  c = cur_chr;
  scan_eight_bit_int();
  b = cur_val;
  scan_optional_equals();
  scan_dimen(false, false, false);

  if (box(b) != 0)
    mem[box(b) + c].cint = cur_val;
}
/* sec 1257 */
void new_font_(small_number a)
{
  pointer u;
  scaled s;
  internal_font_number f;
  str_number t;
  char old_setting;
  str_number flushable_string;

  if (job_name == 0)
    open_log_file();

  get_r_token();
  u = cur_cs;

  if (u >= hash_base)
    t = text(u);
  else if (u >= single_base)
    if (u == null_cs)
      t = 1213;     /* FONT */
    else
      t = u - single_base;
  else
  {
    old_setting = selector;
    selector = new_string;
    print_string("FONT");
    print(u - active_base);
    selector = old_setting;
    str_room(1);
    t = make_string();
  }

  define(u, set_font, null_font);
  scan_optional_equals();
  scan_file_name();

  name_in_progress = true;

  if (scan_keyword("at"))
  {
    scan_dimen(false, false, false);
    s = cur_val; 

    if ((s <= 0) || (s >= 134217728L)) /* 2^27 */
    {
      print_err("Improper `at' size (");
      print_scaled(s);
      print_string("pt), replaced by 10pt");
      help2("I can only handle fonts at positive sizes that are",
        "less than 2048pt, so I've changed what you said to 10pt.");
      error();
      s = 10 * 65536L;
    }
  }
  else if (scan_keyword("scaled"))
  {
    scan_int();
    s = -cur_val;

    if ((cur_val <= 0) || (cur_val > 32768L))
    {
      print_err("Illegal magnification has been changed to 1000");
      help1("The magnification ratio must be between 1 and 32768.");
      int_error(cur_val);
      s = -1000;
    }
  }
  else
    s = -1000;

  name_in_progress = false;

  flushable_string = str_ptr - 1;

  if (trace_flag)
  {
    int i, k1, k2, l1, l2;
    char *sch = log_line;
    k1 = str_start[cur_area];
    k2 = str_start[cur_name];
    l1 = length(cur_area);
    l2 = length(cur_name);
    show_char('\n');
    puts("FONT ");

    for (i = 0; i < l1; i++)
    {
      *sch++ = str_pool[i + k1];
    }

    for (i = 0; i < l2; i++)
    {
      *sch++ = str_pool[i + k2];
    }

    *sch++ = ' ';
    *sch++ = '\0';
    show_line(log_line, 0);
  }

  for (f = font_base + 1; f < font_ptr; f++)
  {
    if (str_eq_str(font_name[f], cur_name) && str_eq_str(font_area[f], cur_area))
    {
      if (cur_name == flushable_string)
      {
        flush_string();
        cur_name = font_name[f];
      }

      if (s > 0)
      {
        if (s == font_size[f])
        {
          if (ignore_frozen == 0 || f > frozen_font_ptr)
          {
            if (trace_flag)
            {
              sprintf(log_line, "SKIPPING %ld ", s);
              show_line(log_line, 0);
            }
            goto lab50;
          }
        }
      }
      else if (font_size[f] == xn_over_d(font_dsize[f], - (integer) s, 1000))
      {
        if (ignore_frozen == 0 || f > frozen_font_ptr)
        {
          if (trace_flag)
          {
            sprintf(log_line, "SKIPPING %ld ", s);
            show_line(log_line, 0);
          }
          goto lab50;
        }
      }
    }
  }

  if (trace_flag)
    show_line("READING ", 0);

  f = read_font_info(u, cur_name, cur_area, s); 

lab50:
  if (trace_flag)
  {
    sprintf(log_line, "NEW FONT %d ", f);
    show_line(log_line, 0);
  }

  equiv(u) = f;
  eqtb[font_id_base + f] = eqtb[u];

#ifdef SHORTHASH
  if (t > 65535L)
  {
    sprintf(log_line, "ERROR: %s too large %d\n",  "hash_used", t);
    show_line(log_line, 1);
  }
#endif
  font_id_text(f) = t;
}
/* sec 1265 */
void new_interaction (void)
{
  print_ln();
  interaction = cur_chr;

  if (interaction == batch_mode)
    selector = no_print;
  else
    selector = term_only;

  if (log_opened)
    selector = selector + 2;
}
/* sec 1270 */
void do_assignments (void)
{
  while (true)
  {
    do
      {
        get_x_token();
      }
    while (!((cur_cmd != spacer) && (cur_cmd != relax)));

    if (cur_cmd <= max_non_prefixed_command)
      return;

    set_box_allowed = false;
    prefixed_command();
    set_box_allowed = true;
  }
}
/* sec 1275 */
void open_or_close_in (void)
{
  char c;
  char n;

  c = cur_chr;
  scan_four_bit_int();
  n = cur_val;

  if (read_open[n] != closed)
  {
    a_close(read_file[n]);
    read_open[n] = closed;
  }

  if (c != 0)
  {
    scan_optional_equals();
    scan_file_name();
    pack_file_name(cur_name, cur_area, cur_ext);

    if ((cur_ext != 335) && a_open_in(read_file[n], TEXINPUTPATH))
      read_open[n] = 1;
    else if ((cur_ext != 785) && (name_length + 5 < file_name_size))
    {
      strncpy((char *) name_of_file + name_length + 1, ".tex ", 5);
      name_length = name_length + 4;

      if (a_open_in(read_file[n], TEXINPUTPATH))
        read_open[n] = just_open;
      else
      {
        name_length = name_length - 4;
        name_of_file[name_length + 1] = ' ';

        if ((cur_ext == 335) && a_open_in(read_file[n], TEXINPUTPATH))
          read_open[n] = 1;
      }
    }
  }
}
/* sec 1279 */
void issue_message (void)
{
  char old_setting;
  char c;
  str_number s;

  c = cur_chr;
  link(garbage) = scan_toks(false, true);
  old_setting = selector;
  selector = new_string;
  token_show(def_ref);
  selector = old_setting;
  flush_list(def_ref);
  str_room(1);
  s = make_string();

  if (c == 0)
  {
    if (term_offset + length(s) > max_print_line - 2)
      print_ln();
    else if ((term_offset > 0) || (file_offset > 0))
      print_char(' ');

    slow_print(s);

#ifndef _WINDOWS
    fflush(stdout);
#endif
  }
  else
  {
    print_err("");
    slow_print(s);

    if (err_help != 0)
      use_err_help = true;
    else if (long_help_seen)
      help1("(That was another \\errmessage.)");
    else
    {
      if (interaction < error_stop_mode)
        long_help_seen = true;

      help4("This error message was generated by an \\errmessage",
        "command, so I can't give any explicit help.",
        "Pretend that you're Hercule Poirot: Examine all clues,",
        "and deduce the truth by order and method.");
    }

    error();
    use_err_help = false;
  }

  flush_string();
}
/* sec 1288 */
void shift_case (void)
{
  pointer b;
  pointer p;
  halfword t;
  eight_bits c;

  b = cur_chr;
  p = scan_toks(false, false);
  p = link(def_ref);

  while (p != 0)
  {
    t = info(p); 

    if (t < cs_token_flag + single_base)
    {
      c = t % 256;

      if (equiv(b + c) != 0)
        info(p) = t - c + equiv(b + c);
    }

    p = link(p);
  }

  begin_token_list(link(def_ref), 3);
  free_avail(def_ref);
}
/* sec 1293 */
void show_whatever (void)
{
  pointer p;

  switch (cur_chr)
  {
    case show_lists:
      {
        begin_diagnostic();
        show_activities();
      }
      break;

    case show_box_code:
      {
        scan_eight_bit_int();
        begin_diagnostic();
        print_nl("> \\box");
        print_int(cur_val);
        print_char('=');

        if (box(cur_val) == 0)
          print_string("void");
        else
          show_box(box(cur_val));
      }
      break;

    case show_code:
      {
        get_token();

        if (interaction == error_stop_mode)
          ;

        print_nl("> ");

        if (cur_cs != 0)
        {
          sprint_cs(cur_cs);
          print_char('=');
        }

        print_meaning();
        goto lab50;
      }
      break;

    default:
      {
        p = the_toks();

        if (interaction == error_stop_mode)
          ;

        print_nl("> ");
        token_show(temp_head);
        flush_list(link(temp_head));
        goto lab50;
      }
      break;
  }

  end_diagnostic(true);
  print_err("OK");

  if (selector == term_and_log)
    if (tracing_online <= 0)
    {
      selector = term_only;
      print_string(" (see the transcript file)");
      selector = term_and_log;
    }

lab50:

  if (interaction < error_stop_mode)
  {
    help_ptr = 0;
    decr(error_count);
  }
  else if (tracing_online > 0)
    help3("This isn't an error message; I'm just \\showing something.",
      "Type `I\\show...' to show more (e.g., \\show\\cs,",
      "\\showthe\\count10, \\showbox255, \\showlists).");
  else
    help5("This isn't an error message; I'm just \\showing something.",
      "Type `I\\show...' to show more (e.g., \\show\\cs,",
      "\\showthe\\count10, \\showbox255, \\showlists).",
      "And type `I\\tracingonline=1\\show...' to show boxes and",
      "lists on your terminal as well as in the transcript file.");

  error();
}
/* sec 1349 */
void new_whatsit_(small_number s, small_number w)
{
  pointer p;

  p = get_node(w);
  type(p) = whatsit_node;
  subtype(p) = s;
  link(tail) = p;
  tail = p;
}
/* sec 1350 */
void new_write_whatsit_(small_number w)
{
  new_whatsit(cur_chr, w);

  if (w != write_node_size)
  {
    scan_four_bit_int();
  }
  else
  {
    scan_int();

    if (cur_val < 0)
      cur_val = 17;
    else if (cur_val > 15)
      cur_val = 16;
  }

  write_stream(tail) = cur_val;
}
/* sec 1348 */
void do_extension (void)
{
  integer k;
  pointer p;

  switch(cur_chr)
  {
    case open_node:
      {
        new_write_whatsit(open_node_size);
        scan_optional_equals();
        scan_file_name();
        open_name(tail) = cur_name;
        open_area(tail) = cur_area;
        open_ext(tail) = cur_ext;
      }
      break;

    case write_node:
      {
        k = cur_cs;
        new_write_whatsit(write_node_size);
        cur_cs = k;
        p = scan_toks(false, false);
        write_tokens(tail) = def_ref;
      }
      break;

    case close_node:
      {
        new_write_whatsit(write_node_size);
        write_tokens(tail) = 0;
      }
      break;

    case special_node:
      {
        new_whatsit(special_node, write_node_size);
        write_stream(tail) = 0;
        p = scan_toks(false, true);
        write_tokens(tail) = def_ref;
      }
      break;

    case immediate_code:
      {
        get_x_token();

        if ((cur_cmd == extension) && (cur_chr <= close_node))
        {
          p = tail;
          do_extension();
          out_what(tail);
          flush_node_list(tail);
          tail = p;
          link(p) = 0;
        }
        else
          back_input();
      }
      break;

    case set_language_code:
      if (abs(mode) != hmode)
      {
        report_illegal_case();
      }
      else
      {
        new_whatsit(language_node, small_node_size);
        scan_int();

        if (cur_val <= 0)
          clang = 0;
        else if (cur_val > 255)
          clang = 0;
        else
          clang = cur_val;

        what_lang(tail) = clang;
        what_lhm(tail) = norm_min(left_hyphen_min);
        what_rhm(tail) = norm_min(right_hyphen_min);
      }
      break;

    default:
      {
        confusion("ext1");
        return;
      }
      break;
  }
}
/* sec 1376 */
void fix_language (void)
{
/*  ASCII_code l;  */
  int l;

  if (language <= 0)
    l = 0; 
  else if (language > 255)
    l = 0;
  else
    l = language;

  if (l != clang)
  {
    new_whatsit(language_node, small_node_size);
    what_lang(tail) = l;
    clang = l;
    what_lhm(tail) = norm_min(left_hyphen_min);
    what_rhm(tail) = norm_min(right_hyphen_min);
  }
}
/* sec 1068 */
void handle_right_brace (void)
{
  pointer p, q;
  scaled d;
  integer f;

  switch(cur_group)
  {
    case simple_group:
      unsave();
      break;

    case bottom_level:
      {
        print_err("Too many }'s");
        help2("You've closed more groups than you opened.",
          "Such booboos are generally harmless, so keep going.");
        error();
      }
      break;

    case semi_simple_group:
    case math_shift_group:
    case math_left_group:
      extra_right_brace();
      break;

    case hbox_group:
      package(0);
      break;

    case adjust_hbox_group:
      {
        adjust_tail = adjust_head;
        package(0);
      }
      break;

    case vbox_group:
      {
        end_graf();
        package(0);
      }
      break;

    case vtop_group:
      {
        end_graf();
        package(vtop_code);
      }
      break;

    case insert_group:
      {
        end_graf();
        q = split_top_skip;
        add_glue_ref(q);
        d = split_max_depth;
        f = floating_penalty;
        unsave();
        decr(save_ptr);
        p = vpackage(link(head), 0, 1, 1073741823L);  /* 2^30 - 1 */
        pop_nest();

        if (saved(0) < 255)
        {
          tail_append(get_node(ins_node_size));
          type(tail) = ins_node;
          subtype(tail) = saved(0);
          height(tail) = height(p) + depth(p);
          ins_ptr(tail) = list_ptr(p);
          split_top_ptr(tail) = q;
          depth(tail) = d;
          float_cost(tail) = f;
        }
        else
        {
          tail_append(get_node(small_node_size));
          type(tail) = adjust_node;
          subtype(tail) = 0;
          adjust_ptr(tail) = list_ptr(p);
          delete_glue_ref(q);
        }
        free_node(p, box_node_size);

        if (nest_ptr == 0)
          build_page();
      }
      break;

    case output_group:
      {
        if ((loc != 0) || ((token_type != output_text) && (token_type != backed_up)))
        {
          print_err("Unbalanced output routine");
          help2("Your sneaky output routine has problematic {'s and/or }'s.",
            "I can't handle that very well; good luck.");
          error();

          do
            {
              get_token();
            }
          while (!(loc == 0));
        }

        end_token_list();
        end_graf();
        unsave();
        output_active = false;
        insert_penalties = 0;

        if (box(255) != 0)
        {
          print_err("Output routine didn't use all of ");
          print_esc("box");
          print_int(255);
          help3("Your \\output commands should empty \\box255,",
            "e.g., by saying `\\shipout\\box255'.",
            "Proceed; I'll discard its present contents.");
          box_error(255);
        }

        if (tail != head)
        {
          link(page_tail) = link(head);
          page_tail = tail;
        }

        if (link(page_head) != 0)
        {
          if (link(contrib_head) == 0)
            nest[0].tail_field = page_tail;

          link(page_tail) = link(contrib_head);
          link(contrib_head) = link(page_head);
          link(page_head) = 0;
          page_tail = page_head;
        }

        pop_nest();
        build_page();
      }
      break;

    case disc_group:
      build_discretionary();
      break;

    case align_group:
      {
        back_input();
        cur_tok = cs_token_flag + frozen_cr;
        print_err("Missing ");
        print_esc("cr");
        print_string("inserted");
        help1("I'm guessing that you meant to end an alignment here.");
        ins_error();
      }
      break;

    case no_align_group:
      {
        end_graf();
        unsave();
        align_peek();
      }
      break;

    case vcenter_group:
      {
        end_graf();
        unsave();
        save_ptr = save_ptr - 2;
        p = vpackage(link(head), saved(1), saved(0), 1073741823L);   /* 2^30 - 1 */
        pop_nest();
        tail_append(new_noad());
        type(tail) = vcenter_noad;
        math_type(nucleus(tail)) = sub_box;
        info(nucleus(tail)) = p;
      }
      break;

    case math_choice_group:
      build_choices();
      break;

    case math_group:
      {
        unsave();
        decr(save_ptr);
        math_type(saved(0)) = sub_mlist;
        p = fin_mlist(0);
        info(saved(0)) = p;

        if (p != 0)
          if (link(p) == 0)
            if (type(p) == ord_noad)
            {
              if (math_type(subscr(p)) == 0)
                if (math_type(supscr(p)) == 0)
                {
                  mem[saved(0)].hh = mem[nucleus(p)].hh;
                  free_node(p, noad_size);
                }
            }
            else if (type(p) == accent_noad)
              if (saved(0) == nucleus(tail))
                if (type(tail) == ord_noad)
                {
                  q = head;

                  while (link(q) != tail)
                    q = link(q);

                  link(q) = p;
                  free_node(tail, noad_size);
                  tail = p;
                }
      }
      break;
    default:
      {
        confusion("rightbrace");
        return;
      }
      break;
  }
}
/* sec 1030 */
void main_control (void) 
{
  integer t;
  integer bSuppress; /* 199/Jan/5 */

  if (every_job != 0)
    begin_token_list(every_job, every_job_text);

big_switch:
  get_x_token();       /* big_switch */

reswitch:
  if (interrupt != 0)
    if (OK_to_interrupt)
    {
      back_input();
      check_interrupt();
      goto big_switch;
    }

#ifdef DEBUG
  if (panicking)
    check_mem(false);
#endif

  if (tracing_commands > 0)
    show_cur_cmd_chr();

  switch(abs(mode) + cur_cmd)
  {
    case hmode + letter:
    case hmode + other_char:
    case hmode + char_given:
      goto main_loop;
      break;

    case hmode + char_num:
      {
        scan_char_num();
        cur_chr = cur_val;
        goto main_loop;
      }
      break;

    case hmode + no_boundary:
      {
        get_x_token();

        if ((cur_cmd == letter) || (cur_cmd == other_char) ||
          (cur_cmd == char_given) || (cur_cmd == char_num))
          cancel_boundary = true;
        goto reswitch;
      }
      break;

    case hmode + spacer:
      if (space_factor == 1000)
        goto append_normal_space;
      else
        app_space();
      break;

    case hmode + ex_space:
    case mmode + ex_space:
      goto append_normal_space;
      break;

    case any_mode(relax):
    case vmode + spacer:
    case mmode + spacer:
    case mmode + no_boundary:
      ;
      break;

    case any_mode(ignore_spaces):
      {
        do
          {
            get_x_token();
          }
        while (!(cur_cmd != spacer));
        goto reswitch;
      }
      break;

    case vmode + stop:
      if (its_all_over())
        return;
      break;

    case vmode + vmove:
    case hmode + hmove:
    case mmode + hmove:
    case any_mode(last_item):
    case vmode + vadjust:
    case vmode + ital_corr:
    case non_math(eq_no):
    case any_mode(mac_param):
      report_illegal_case();
      break;

    case non_math(sup_mark):
    case non_math(sub_mark):
    case non_math(math_char_num):
    case non_math(math_given):
    case non_math(math_comp):
    case non_math(delim_num):
    case non_math(left_right):
    case non_math(above):
    case non_math(radical):
    case non_math(math_style):
    case non_math(math_choice):
    case non_math(vcenter):
    case non_math(non_script):
    case non_math(mkern):
    case non_math(limit_switch):
    case non_math(mskip):
    case non_math(math_accent):
    case mmode + endv:
    case mmode + par_end:
    case mmode + stop:
    case mmode + vskip:
    case mmode + un_vbox:
    case mmode + valign:
    case mmode + hrule:
      insert_dollar_sign();
      break;

    case vmode + hrule:
    case hmode + vrule:
    case mmode + vrule:
      {
        tail_append(scan_rule_spec());

        if (abs(mode) == vmode)
          prev_depth = ignore_depth;
        else if (abs(mode) == hmode)
          space_factor = 1000;
      }
      break;

    case vmode + vskip:
    case hmode + hskip:
    case mmode + hskip:
    case mmode + mskip:
      append_glue();
      break;

    case any_mode(kern):
    case mmode + mkern:
      append_kern();
      break;

    case non_math(left_brace):
      new_save_level(simple_group);
      break;

    case any_mode(begin_group):
      new_save_level(semi_simple_group);
      break;

    case any_mode(end_group):
      if (cur_group == semi_simple_group)
        unsave();
      else
        off_save();
      break;

    case any_mode(right_brace):
      handle_right_brace();
      break;

    case vmode + hmove:
    case hmode + vmove:
    case mmode + vmove:
      {
        t = cur_chr;
        scan_dimen(false, false, false);

        if (t == 0)
          scan_box(cur_val);
        else
          scan_box(- (integer) cur_val);
      }
      break;

    case any_mode(leader_ship):
      scan_box(leader_flag - a_leaders + cur_chr);
      break;

    case any_mode(make_box):
      begin_box(0);
      break;

    case vmode + start_par:
      new_graf(cur_chr > 0);
      break;

    case vmode + letter:
    case vmode + other_char:
    case vmode + char_num:
    case vmode + char_given:
    case vmode + math_shift:
    case vmode + un_hbox:
    case vmode + vrule:
    case vmode + accent:
    case vmode + discretionary:
    case vmode + hskip:
    case vmode + valign:
    case vmode + ex_space:
    case vmode + no_boundary:
      {
        back_input();
        new_graf(true);
      }
      break;

    case hmode + start_par:
    case mmode + start_par:
      indent_in_hmode();
      break;

    case vmode + par_end:
      {
        normal_paragraph();

        if (mode > 0)
          build_page();
      }
      break;

    case hmode + par_end:
      {
        if (align_state < 0)
          off_save();

        end_graf();

        if (mode == 1)
          build_page();
      }
      break;

    case hmode + stop:
    case hmode + vskip:
    case hmode + hrule:
    case hmode + un_vbox:
    case hmode + halign:
      head_for_vmode();
      break;

    case any_mode(insert):
    case hmode + vadjust:
    case mmode + vadjust:
      begin_insert_or_adjust();
      break;

    case any_mode(mark):
      make_mark();
      break;

    case any_mode(break_penalty):
      append_penalty();
      break;

    case any_mode(remove_item):
      delete_last();
      break;

    case vmode + un_vbox:
    case hmode + un_hbox:
    case mmode + un_hbox:
      unpackage();
      break;

    case hmode + ital_corr:
      append_italic_correction();
      break;

    case mmode + ital_corr:
      tail_append(new_kern(0));
      break;

    case hmode + discretionary:
    case mmode + discretionary:
      append_discretionary();
      break;

    case hmode + accent:
      make_accent();
      break;

    case any_mode(car_ret):
    case any_mode(tab_mark):
      align_error();
      break;

    case any_mode(no_align):
      noalign_error();
      break;

    case any_mode(omit):
      omit_error();
      break;

    case vmode + halign:
    case hmode + valign:
      init_align();
      break;

    case mmode + halign:
      if (privileged ())
        if (cur_group == math_shift_group)
          init_align();
        else
          off_save();
      break;

    case vmode + endv:
    case hmode + endv:
      do_endv();
      break;

    case any_mode(end_cs_name):
      cs_error();
      break;

    case hmode + math_shift:
      init_math();
      break;

    case mmode + eq_no:
      if (privileged ())
        if (cur_group == math_shift_group)
          start_eq_no();
        else
          off_save();
      break;

    case mmode + left_brace:
      {
        tail_append(new_noad());
        back_input();
        scan_math(nucleus(tail));
      }
      break;

    case mmode + letter:
    case mmode + other_char:
    case mmode + char_given:
      set_math_char(math_code(cur_chr));
      break;

    case mmode + char_num:
      {
        scan_char_num();
        cur_chr = cur_val;
        set_math_char(math_code(cur_chr));
      }
      break;

    case mmode + math_char_num:
      {
        scan_fifteen_bit_int();
        set_math_char(cur_val);
      }
      break;

    case mmode + math_given:
      set_math_char(cur_chr);
      break;

    case mmode + delim_num:
      {
        scan_twenty_seven_bit_int();
        set_math_char(cur_val / 4096);
      }
      break;

    case mmode + math_comp:
      {
        tail_append(new_noad());
        type(tail) = cur_chr;
        scan_math(nucleus(tail));
      }
      break;

    case mmode + limit_switch:
      math_limit_switch();
      break;

    case mmode + radical:
      math_radical();
      break;

    case mmode + accent:
    case mmode + math_accent:
      math_ac();
      break;

    case mmode + vcenter:
      {
        scan_spec(vcenter_group, false);
        normal_paragraph();
        push_nest();
        mode = -1;
        prev_depth = ignore_depth;

        if (every_vbox != 0)
          begin_token_list(every_vbox, every_vbox_text);
      }
      break;

    case mmode + math_style:
      tail_append(new_style(cur_chr));
      break;

    case mmode + non_script:
      {
        tail_append(new_glue(0));
        subtype(tail) = cond_math_glue;
      }
      break;

    case mmode + math_choice:
      append_choices();
      break;

    case mmode + sub_mark:
    case mmode + sup_mark:
      sub_sup();
      break;

    case mmode + above:
      math_fraction();
      break;

    case mmode + left_right:
      math_left_right();
      break;

    case mmode + math_shift:
      if (cur_group == math_shift_group)
        after_math();
      else
        off_save();
      break;

    case any_mode(toks_register):
    case any_mode(assign_toks):
    case any_mode(assign_int):
    case any_mode(assign_dimen):
    case any_mode(assign_glue):
    case any_mode(assign_mu_glue):
    case any_mode(assign_font_dimen):
    case any_mode(assign_font_int):
    case any_mode(set_aux):
    case any_mode(set_prev_graf):
    case any_mode(set_page_dimen):
    case any_mode(set_page_int):
    case any_mode(set_box_dimen):
    case any_mode(set_shape):
    case any_mode(def_code):
    case any_mode(def_family):
    case any_mode(set_font):
    case any_mode(def_font):
    case any_mode(tex_register):
    case any_mode(advance):
    case any_mode(multiply):
    case any_mode(divide):
    case any_mode(prefix):
    case any_mode(let):
    case any_mode(shorthand_def):
    case any_mode(read_to_cs):
    case any_mode(def):
    case any_mode(set_box):
    case any_mode(hyph_data):
    case any_mode(set_interaction):
      prefixed_command();
      break;

    case any_mode(after_assignment):
      {
        get_token();
        after_token = cur_tok;
      }
      break;

    case any_mode(after_group):
      {
        get_token();
        save_for_after(cur_tok);
      }
      break;

    case any_mode(in_stream):
      open_or_close_in();
      break;

    case any_mode(message):
      issue_message();
      break;

    case any_mode(case_shift):
      shift_case();
      break;

    case any_mode(xray):
      show_whatever();
      break;

    case any_mode(extension):
      do_extension();
      break;
  }

  goto big_switch;

main_loop:
  adjust_space_factor();
  main_f = cur_font;
  bchar = font_bchar[main_f];
  false_bchar = font_false_bchar[main_f];

  if (mode > 0)
    if (language != clang)
      fix_language();

  fast_get_avail(lig_stack);
  font(lig_stack) = main_f;
  cur_l = cur_chr;
  character(lig_stack) = cur_l;
  cur_q = tail;

  if (cancel_boundary)
  {
    cancel_boundary = false;
    main_k = non_address;
  }
  else
    main_k = bchar_label[main_f];

  if (main_k == non_address)
    goto main_loop_move_2;

  cur_r = cur_l;
  cur_l = non_char;
  goto main_lig_loop_1;

main_loop_wrapup: 
  wrapup(rt_hit);

main_loop_move:
  if (lig_stack == 0)
    goto reswitch;

  cur_q = tail;
  cur_l = character(lig_stack);

main_loop_move_1:
  if (!is_char_node(lig_stack))
    goto main_loop_move_lig;

main_loop_move_2:
  if ((cur_chr < font_bc[main_f]) || (cur_chr > font_ec[main_f]))
  {
    char_warning(main_f, cur_chr);
    free_avail(lig_stack);
    goto big_switch;
  }

  main_i = char_info(main_f, cur_l);

  if (!(main_i.b0 > 0))
  {
    char_warning(main_f, cur_chr);
    free_avail(lig_stack);
    goto big_switch; 
  }
  {
    link(tail) = lig_stack;
    tail = lig_stack;
  }

main_loop_lookahead:
  get_next();

  if (cur_cmd == letter)
    goto main_loop_lookahead_1;
  if (cur_cmd == other_char)
    goto main_loop_lookahead_1;
  if (cur_cmd == char_given)
    goto main_loop_lookahead_1;

  x_token();

  if (cur_cmd == letter)
    goto main_loop_lookahead_1;
  if (cur_cmd == other_char)
    goto main_loop_lookahead_1;
  if (cur_cmd == char_given)
    goto main_loop_lookahead_1;

  if (cur_cmd == char_num)
  {
    scan_char_num();
    cur_chr = cur_val;
    goto main_loop_lookahead_1;
  }

  if (cur_cmd == no_boundary)
    bchar = non_char;

  cur_r = bchar;
  lig_stack = 0;
  goto main_lig_loop;

main_loop_lookahead_1:
  adjust_space_factor();
  fast_get_avail(lig_stack);
  font(lig_stack) = main_f;
  cur_r = cur_chr;
  character(lig_stack) = cur_r;

  if (cur_r == false_bchar)
    cur_r = non_char;

main_lig_loop:
  if (char_tag(main_i) != lig_tag)
    goto main_loop_wrapup;

  if (cur_r == non_char)
    goto main_loop_wrapup;

  main_k = lig_kern_start(main_f, main_i);
  main_j = font_info[main_k].qqqq;

  if (skip_byte(main_j) <= stop_flag)
    goto main_lig_loop_2;

  main_k = lig_kern_restart(main_f, main_j);

main_lig_loop_1:
  main_j = font_info[main_k].qqqq;

main_lig_loop_2:
  bSuppress = 0;

  if (suppress_f_ligs && next_char(main_j) == cur_r && op_byte(main_j) == no_tag)
  {
    if (cur_l == 'f')
      bSuppress = 1;
  }

  if (next_char(main_j) == cur_r && bSuppress == 0)  /* 99/Jan/5 */
    if (skip_byte(main_j) <= stop_flag)
    {
      if (op_byte(main_j) >= kern_flag)
      {
        wrapup(rt_hit);
        tail_append(new_kern(char_kern(main_f, main_j)));
        goto main_loop_move;
      }

      if (cur_l == non_char)
        lft_hit = true;
      else if (lig_stack == 0)
        rt_hit = true;

      check_interrupt();

      switch (op_byte(main_j))
      {
        case 1:
        case 5:
          {
            cur_l = rem_byte(main_j);
            main_i = char_info(main_f, cur_l);
            ligature_present = true;
          }
          break;
        case 2:
        case 6:
          {
            cur_r = rem_byte(main_j);

            if (lig_stack == 0)
            {
              lig_stack = new_lig_item(cur_r);
              bchar = non_char;
            }
            else if (is_char_node(lig_stack))
            {
              main_p = lig_stack;
              lig_stack = new_lig_item(cur_r);
              lig_ptr(lig_stack) = main_p;
            }
            else
              character(lig_stack) = cur_r;
          }
          break;
        case 3:
          {
            cur_r = rem_byte(main_j);
            main_p = lig_stack;
            lig_stack = new_lig_item(cur_r);
            link(lig_stack) = main_p;
          }
          break;
        case 7:
        case 11:
          {
            wrapup(false);
            cur_q = tail;
            cur_l = rem_byte(main_j);
            main_i = char_info(main_f, cur_l);
            ligature_present = true;
          }
          break;
        default:
          {
            cur_l = rem_byte(main_j);
            ligature_present = true;
 
            if (lig_stack == 0)
              goto main_loop_wrapup;
            else
              goto main_loop_move_1;
          }
          break;
      }

      if (op_byte(main_j) > 4)
        if (op_byte(main_j) != 7)
          goto main_loop_wrapup;

      if (cur_l < non_char)
        goto main_lig_loop;

      main_k = bchar_label[main_f];
      goto main_lig_loop_1;
    }

    if (skip_byte(main_j) == 0)
      incr(main_k);
    else
    {
      if (skip_byte(main_j) >= stop_flag)
        goto main_loop_wrapup;

      main_k = main_k + skip_byte(main_j) + 1;
    }

    goto main_lig_loop_1;

main_loop_move_lig:
  main_p = lig_ptr(lig_stack);

  if (main_p != 0)     /* BUG FIX */
    tail_append(main_p);

  temp_ptr = lig_stack;
  lig_stack = link(temp_ptr);
  free_node(temp_ptr, small_node_size);
  main_i = char_info(main_f, cur_l);
  ligature_present = true;

  if (lig_stack == 0)
    if (main_p != 0)   /* BUG FIX */
      goto main_loop_lookahead;
    else
      cur_r = bchar;
  else
    cur_r = character(lig_stack);

  goto main_lig_loop;

append_normal_space:
  if (space_skip == 0)
  {
    {
      main_p = font_glue[cur_font];

      if (main_p == 0)
      {
        main_p = new_spec(zero_glue);
        main_k = param_base[cur_font] + space_code;
        width(main_p) = font_info[main_k].cint;
        stretch(main_p) = font_info[main_k + 1].cint;
        shrink(main_p) = font_info[main_k + 2].cint;
        font_glue[cur_font] = main_p;
      }
    }
    temp_ptr = new_glue(main_p);
  }
  else
    temp_ptr = new_param_glue(space_skip_code);

  link(tail) = temp_ptr;
  tail = temp_ptr;

  goto big_switch;
}