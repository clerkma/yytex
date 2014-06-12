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

/* sec 0440 */
void scan_int (void)
{
  boolean negative;
  integer m;
  small_number d;
  boolean vacuous;
  boolean OKsofar;

  radix = 0;
  OKsofar = true;
  negative = false;

  do
    {
      do 
        {
          get_x_token();
        }
      while (!(cur_cmd != spacer));

      if (cur_tok == other_token + '-')
      {
        negative = !negative;
        cur_tok = other_token + '+';
      }
    }
  while (!(cur_tok != other_token + '+'));

  if (cur_tok == alpha_token)
  {
    get_token();

    if (cur_tok < cs_token_flag)
    {
      cur_val = cur_chr;

      if (cur_cmd <= right_brace)
        if (cur_cmd == right_brace)
          incr(align_state);
        else
          decr(align_state);
    }
    else if (cur_tok < cs_token_flag + single_base)
      cur_val = cur_tok - cs_token_flag - active_base;
    else
      cur_val = cur_tok - cs_token_flag - single_base;

    if (cur_val > 255)
    {
      print_err("Improper alphabetic constant");
      help2("A one-character control sequence belongs after a ` mark.",
        "So I'm essentially inserting \\0 here.");
      cur_val = '0';
      back_error();
    }
    else
    {
      get_x_token();

      if (cur_cmd != spacer)
        back_input();
    }
  }
  else if ((cur_cmd >= min_internal) && (cur_cmd <= max_internal))
  {
    scan_something_internal(int_val, false);
  }
  else
  {
    radix = 10;
    m = 214748364L;   /* 7FFFFFFF hex */

    if (cur_tok == octal_token)
    {
      radix = 8;
      m = 268435456L;   /* 2^28 */
      get_x_token();
    }
    else if (cur_tok == hex_token)
    {
      radix = 16;
      m = 134217728L;   /* 2^27 8000000 hex */
      get_x_token();
    }

    vacuous = true;
    cur_val = 0;

    while (true)
    {
      if ((cur_tok < zero_token + radix) && (cur_tok >= zero_token) && (cur_tok <= zero_token + 9))
        d = cur_tok - zero_token;
      else if (radix == 16)
        if ((cur_tok <= A_token + 5) && (cur_tok >= A_token))
          d = cur_tok - A_token + 10;
        else if ((cur_tok <= other_A_token + 5) && (cur_tok >= other_A_token))
          d = cur_tok - other_A_token;
        else
          goto lab30;
      else
        goto lab30;

      vacuous = false;

      if ((cur_val >= m) && ((cur_val > m) || (d > 7) || (radix != 10)))
      {
        if (OKsofar)
        {
          print_err("Number too big");
          help2("I can only go up to 2147483647='17777777777=\"7FFFFFFF,",
            "so I'm using that number instead of yours.");
          error();
          cur_val = 2147483647L;    /* 7FFFFFFF hex */
          OKsofar = false;
        }
      }
      else
        cur_val = cur_val * radix + d;
      get_x_token();
    }
lab30:;

    if (vacuous)
    {
      print_err("Missing number, treated as zero");
      help3("A number should have been here; I inserted `0'.",
        "(If you can't figure out why I needed to see a number,",
        "look up `weird error' in the index to The TeXbook.)");
      back_error();
    } 
    else if (cur_cmd != spacer)
      back_input();
  }

  if (negative)
    cur_val = - (integer) cur_val;
}
/* sec 0448 */
void scan_dimen_(boolean mu, boolean inf, boolean shortcut)
{
  boolean negative;
  integer f;
  integer num, denom;
  small_number k, kk;
  halfword p, q;
  scaled v;
  integer savecurval;

  f = 0;
  arith_error = false;
  cur_order = 0;
  negative = false;

  if (!shortcut)
  {
    negative = false;

    do
      {
        do
          {
            get_x_token();
          }
        while (!(cur_cmd != spacer));

        if (cur_tok == other_token + '-')
        {
          negative = ! negative;
          cur_tok = other_token + '+';
        }
      }
    while (!(cur_tok != other_token + '+'));

    if ((cur_cmd >= min_internal) && (cur_cmd <= max_internal))
    {
      if (mu)
      {
        scan_something_internal(mu_val, false);

        if (cur_val_level >= glue_val)
        {
          v = width(cur_val);
          delete_glue_ref(cur_val);
          cur_val = v;
        }

        if (cur_val_level == mu_val)
          goto lab89;

        if (cur_val_level != int_val)
          mu_error();
      }
      else
      {
        scan_something_internal(dimen_val, false);

        if (cur_val_level == dimen_val)
          goto lab89;
      }
    }
    else
    {
      back_input();

      if (cur_tok == continental_point_token)
        cur_tok = point_token;

      if (cur_tok != point_token)
      {
        scan_int();
      }
      else
      {
        radix = 10;
        cur_val = 0;
      }

      if (cur_tok == continental_point_token)
        cur_tok = point_token;

      if ((radix == 10) && (cur_tok == point_token))
      {
        k = 0;
        p = 0;      /* p:=null l.8883 */
        get_token();

        while (true)
        {
          get_x_token();

          if ((cur_tok > zero_token + 9) || (cur_tok < zero_token))
            goto lab31;

          if (k < 17)
          {
            q = get_avail();
            link(q) = p;
            info(q) = cur_tok - zero_token;
            p = q;
            incr(k);
          }
        }
lab31:
        for (kk = k; kk >= 1; kk--)
        {
          dig[kk - 1] = info(p);
          q = p;
          p = link(p);
          free_avail(q);
        }

        f = round_decimals(k);

        if (cur_cmd != spacer)
          back_input();
        }
      }
  }

  if (cur_val < 0)
  {
    negative = !negative;
    cur_val = - (integer) cur_val;
  }

  if (inf)
  {
    if (scan_keyword("fil"))
    {
      cur_order = fil;

      while (scan_keyword("l"))
      {
        if (cur_order == filll)
        {
          print_err("Illegal unit of measure (");
          print_string("replaced by filll)");
          help1("I dddon't go any higher than filll.");
          error();
        }
        else
          incr(cur_order);
      }
      goto lab88;
    }
  }

  savecurval = cur_val;

  do
    {
      get_x_token();
    }
  while (!(cur_cmd != spacer));

  if ((cur_cmd < min_internal) || (cur_cmd > max_internal))
    back_input();
  else
  {
    if (mu)
    {
      scan_something_internal(mu_val, false);

      if (cur_val_level >= glue_val)
      {
        v = width(cur_val);
        delete_glue_ref(cur_val);
        cur_val = v;
      }

      if (cur_val_level != mu_val)
      {
        mu_error();
      }
    }
    else
    {
      scan_something_internal(dimen_val, false);
    }

    v = cur_val;
    goto lab40;
  }

  if (mu)
    goto lab45;

  if (scan_keyword("em"))
    v = quad(cur_font);
  else if (scan_keyword("ex"))
    v = x_height(cur_font);
  else
    goto lab45;

  {
    get_x_token();

    if (cur_cmd != spacer)
      back_input();
  }
lab40:
  cur_val = mult_and_add(savecurval, v, xn_over_d(v, f, 65536L), 1073741823L);   /* 2^30 - 1 */
  goto lab89;
lab45:
  if (mu)
  {
    if (scan_keyword("mu"))
      goto lab88;
    else
    {
      print_err("Illegal unit of measure (");
      print_string("mu inserted)");
      help4("The unit of measurement in math glue must be mu.",
          "To recover gracefully from this error, it's best to",
          "delete the erroneous units; e.g., type `2' to delete",
          "two letters. (See Chapter 27 of The TeXbook.)");
      error();
      goto lab88;
    }
  }

  if (scan_keyword("true"))
  {
    prepare_mag();

    if (mag != 1000)
    {
      cur_val = xn_over_d(cur_val, 1000, mag);
      f = (1000 * f + 65536L * tex_remainder) / mag;
      cur_val = cur_val + (f / 65536L);
      f = f % 65536L;
    }
  }

  if (scan_keyword("pt"))
    goto lab88;

  if (scan_keyword("in"))
    set_conversion(7227, 100);
  else if (scan_keyword("pc"))
    set_conversion(12, 1);
  else if (scan_keyword("cm"))
    set_conversion(7227, 254);
  else if (scan_keyword("mm"))
    set_conversion(7227, 2540);
  else if (scan_keyword("bp"))
    set_conversion(7227, 7200);
  else if (scan_keyword("dd"))
    set_conversion(1238, 1157);
  else if (scan_keyword("cc"))
    set_conversion(14856, 1157);
  else if (scan_keyword("Q"))
    set_conversion(7227, 10160);
  else if (scan_keyword("H"))
    set_conversion(7227, 10160);
  else if (scan_keyword("sp"))
    goto lab30;
  else
  {
    print_err("Illegal unit of measure (");
    print_string("pt inserted)");
    help6("Dimensions can be in units of em, ex, in, pt, pc,",
      "cm, mm, dd, cc, bp, or sp; but yours is a new one!",
      "I'll assume that you meant to say pt, for printer's points.",
      "To recover gracefully from this error, it's best to",
      "delete the erroneous units; e.g., type `2' to delete",
      "two letters. (See Chapter 27 of The TeXbook.)");
    error();
    goto lab32;
  }

  cur_val = xn_over_d(cur_val, num, denom);
  f = (num * f + 65536L * tex_remainder) / denom;
  cur_val = cur_val +(f / 65536L);
  f = f % 65536L;
lab32:;
lab88:
  if (cur_val >= 16384)     /* 2^14 */
    arith_error = true;
  else
    cur_val = cur_val * 65536L + f;
lab30:;
  {
    get_x_token();

    if (cur_cmd != spacer)
      back_input();
  }
lab89:
  if (arith_error || (abs(cur_val) >= 1073741824L)) /* 2^30 */
  {
    print_err("Dimension too large");
    help2("I can't work with sizes bigger than about 19 feet.",
        "Continue and I'll use the largest value I can.");
    error();
    cur_val = 1073741823L;  /* 2^30 - 1 */
    arith_error = false;
  }

  if (negative)
    cur_val = - (integer) cur_val;
}
/* sec 0461 */
void scan_glue_(small_number level)
{
  boolean negative;
  halfword q;
  boolean mu;

  mu = (level == mu_val);
  negative = false;

  do
    {
      do
        {
          get_x_token();
        }
      while (!(cur_cmd != spacer));

      if (cur_tok == other_token + '-')
      {
        negative = !negative;
        cur_tok = other_token + '+';
      }
    }
  while (!(cur_tok != other_token + '+'));

  if ((cur_cmd >= min_internal) && (cur_cmd <= max_internal))
  {
    scan_something_internal(level, negative);

    if (cur_val_level >= glue_val)
    {
      if (cur_val_level != level)
      {
        mu_error();
      }
      return;
    }

    if (cur_val_level == int_val)
    {
      scan_dimen(mu, false, true);
    }
    else if (level == mu_val)
    {
      mu_error();
    }
  }
  else
  {
    back_input();
    scan_dimen(mu, false, false);

    if (negative)
      cur_val = - (integer) cur_val;
  }
  q = new_spec(zero_glue);
  width(q) = cur_val;

  if (scan_keyword("plus"))
  {
    scan_dimen(mu, true, false);
    stretch(q) = cur_val;
    stretch_order(q) = cur_order;
  }

  if (scan_keyword("minus"))
  {
    scan_dimen(mu, true, false);
    shrink(q) = cur_val;
    shrink_order(q) = cur_order;
  }

  cur_val = q;
}
/* sec 0463 */
halfword scan_rule_spec (void)
{
  halfword q;

  q = new_rule();

  if (cur_cmd == vrule)
    width(q) = default_rule;
  else
  {
    height(q) = default_rule;
    depth(q) = 0;
  }

lab21:

  if (scan_keyword("width"))
  {
    scan_dimen(false, false, false);
    width(q) = cur_val;
    goto lab21;
  }

  if (scan_keyword("height"))
  {
    scan_dimen(false, false, false);
    height(q) = cur_val;
    goto lab21;
  }

  if (scan_keyword("depth"))
  {
    scan_dimen(false, false, false);
    depth(q) = cur_val;
    goto lab21;
  }

  return q;
}
/* sec 0464 */
halfword str_toks_(pool_pointer b)
{
  halfword p;
  halfword q;
  halfword t;
  pool_pointer k;

  str_room(1);
  p = temp_head;
  link(p) = 0;
  k = b;

  while (k < pool_ptr)
  {
    t = str_pool[k];

    if (t == ' ')
      t = space_token;
    else
      t = other_token + t;

    fast_store_new_token(t);
    incr(k);
  }

  pool_ptr = b;

  return p;
}
/* sec 0465 */
halfword the_toks (void)
{
  register halfword Result;
  char old_setting;
  halfword p, q, r;
  pool_pointer b;

  get_x_token();
  scan_something_internal(tok_val, false);

  if (cur_val_level >= ident_val)
  {
    p = temp_head;
    link(p) = 0;

    if (cur_val_level == ident_val)
      store_new_token(cs_token_flag + cur_val);
    else if (cur_val != 0)
    {
      r = link(cur_val);

      while (r != 0)
      {
        fast_store_new_token(info(r));
        r = link(r);
      }
    }

    Result = p;
  }
  else
  {
    old_setting = selector;
    selector = new_string;
    b = pool_ptr;

    switch (cur_val_level)
    {
      case int_val:
        print_int(cur_val);
        break;
      case dimen_val:
        {
          print_scaled(cur_val);
          print_string("pt");
        }
        break;
      case glue_val:
        {
          print_spec(cur_val, "pt");
          delete_glue_ref(cur_val);
        }
        break;
      case mu_val:
        {
          print_spec(cur_val, "mu");
          delete_glue_ref(cur_val);
        }
        break;
    }
    selector = old_setting;
    Result = str_toks(b);
  }

  return Result;
}
/* sec 0467 */
void ins_the_toks (void) 
{ 
  link(garbage) = the_toks();
  begin_token_list(link(temp_head), 4);
}
/* sec 0470 */
void conv_toks (void)
{
  char old_setting;
  char c;
  small_number save_scanner_status;
  pool_pointer b;

  c = cur_chr;

  switch (c)
  {
    case number_code:
    case roman_numeral_code:
      scan_int();
      break;

    case string_code:
    case meaning_code:
      save_scanner_status = scanner_status;
      scanner_status = 0;
      get_token();
      scanner_status = save_scanner_status;
      break;

    case font_name_code:
      scan_font_ident();
      break;

    case job_name_code:
      if (job_name == 0)
        open_log_file();
      break;
  }

  old_setting = selector;
  selector = new_string;
  b = pool_ptr;

  switch (c)
  {
    case number_code:
      print_int(cur_val);
      break;

    case roman_numeral_code:
      print_roman_int(cur_val);
      break;

    case string_code:
      if (cur_cs != 0)
        sprint_cs(cur_cs);
      else
        print_char(cur_chr);
      break;

    case meaning_code:
      print_meaning();
      break;

    case font_name_code:
      print(font_name[cur_val]);

      if (font_size[cur_val] != font_dsize[cur_val])
      {
        print_string(" at ");
        print_scaled(font_size[cur_val]);
        print_string("pt");
      }
      break;

    case job_name_code:
      print(job_name);
      break;
  }

  selector = old_setting;
  link(garbage) = str_toks(b);
  begin_token_list(link(temp_head), 4);
}
/* sec 0473 */
halfword scan_toks_(boolean macrodef, boolean xpand)
{
  register halfword Result;
  halfword t;
  halfword s;
  halfword p;
  halfword q;
  halfword unbalance;
  halfword hashbrace;

  if (macrodef)
    scanner_status = defining;
  else
    scanner_status = absorbing;

  warning_index = cur_cs;
  def_ref = get_avail();
  token_ref_count(def_ref) = 0;
  p = def_ref;
  hashbrace = 0;
  t = zero_token;

  if (macrodef)
  {
    while (true)
    {
      get_token();

      if (cur_tok < right_brace_limit)
        goto lab31;

      if (cur_cmd == mac_param)
      {
        s = match_token + cur_chr;
        get_token();

        if (cur_cmd == left_brace)
        {
          hashbrace = cur_tok;
          store_new_token(cur_tok);
          store_new_token(end_match_token);
          goto lab30;
        }

        if (t == zero_token + 9)
        {
          print_err("You already have nine parameters");
          help1("I'm going to ignore the # sign you just used.");
          error();
        }
        else
        {
          incr(t);

          if (cur_tok != t)
          {
            print_err("Parameters must be numbered consecutively");
            help2("I've inserted the digit you should have used after the #.",
                "Type `1' to delete what you did use.");
            back_error();
          }
          cur_tok = s;
        }
      }

      store_new_token(cur_tok);
    }

lab31:
    store_new_token(end_match_token);

    if (cur_cmd == right_brace)
    {
      print_err("Missing { inserted");
      incr(align_state);
      help2("Where was the left brace? You said something like `\\def\\a}',",
          "which I'm going to interpret as `\\def\\a{}'.");
      error();
      goto lab40;
    }
lab30:;
  }
  else
  {
    scan_left_brace();
  }

  unbalance = 1;

  while (true)
  {
    if (xpand)
    {
      while (true)
      {
        get_next();

        if (cur_cmd <= max_command)
          goto lab32;

        if (cur_cmd != the)
        {
          expand();
        }
        else
        {
          q = the_toks();

          if (link(temp_head) != 0)
          {
            link(p) = link(temp_head);
            p = q;
          }
        }
      }
lab32:
      x_token();
    }
    else
      get_token();

    if (cur_tok < right_brace_limit)
      if (cur_cmd < right_brace)
        incr(unbalance);
      else
      {
        decr(unbalance);

        if (unbalance == 0)
          goto lab40;
      }
    else if (cur_cmd == mac_param)
      if (macrodef)
      {
        s = cur_tok;

        if (xpand)
          get_x_token();
        else
          get_token();

        if (cur_cmd != mac_param)
          if ((cur_tok <= zero_token) || (cur_tok > t))
          {
            print_err("Illegal parameter number in definition of ");
            sprint_cs(warning_index);
            help3("You meant to type ## instead of #, right?",
                "Or maybe a } was forgotten somewhere earlier, and things",
                "are all screwed up? I'm going to assume that you meant ##.");
            back_error();
            cur_tok = s;
          }
          else
            cur_tok = out_param_token - '0' + cur_chr;
      }

    store_new_token(cur_tok);
  }
lab40:
  scanner_status = 0;

  if (hashbrace != 0)
    store_new_token(hashbrace);

  Result = p;
  return Result;
}
/* used only in ITEX.C */
/* sec 0482 */
void read_toks_(integer n, halfword r)
{
  halfword p;
  halfword q;
  integer s;
/*  small_number m;  */
  int m; /* 95/Jan/7 */

  scanner_status = defining;
  warning_index = r;
  def_ref = get_avail();
  token_ref_count(def_ref) = 0;
  p = def_ref;
  store_new_token(end_match_token);

  if ((n < 0) || (n > 15))
    m = 16;
  else
    m = n;

  s = align_state;
  align_state = 1000000L;

  do
    {
      begin_file_reading();
      cur_input.name_field = m + 1;

      if (read_open[m] == closed)
        if (interaction > nonstop_mode)
          if (n < 0)
            prompt_input("");
          else
          {
            print_ln();
            sprint_cs(r);
            prompt_input("=");
            n = -1;
          }
        else
        {
          fatal_error("*** (cannot \\read from terminal in nonstop modes)");
          return;     // abort_flag set
        }
      else if (read_open[m] == 1)
        if (input_ln(read_file[m], false))
          read_open[m] = 0;
        else
        {
          (void) a_close(read_file[m]);
          read_open[m] = 2;
        }
      else
      {
        if (!input_ln(read_file[m], true))
        {
          (void) a_close(read_file[m]);
          read_open[m] = 2;

          if (align_state != 1000000L)
          {
            runaway();
            print_err("File ended within ");
            print_esc("read");
            help1("This \\read has unbalanced braces.");
            align_state = 1000000L;
            error();
          }
        }
      }

      limit = last;

      if (end_line_char_inactive())
        decr(limit);
      else
        buffer[limit] = end_line_char;

      first = limit + 1;
      loc = start;
      state = new_line;

      while (true)
      {
        get_token();

        if (cur_tok == 0)
          goto lab30;

        if (align_state < 1000000L)
        {
          do
            {
              get_token();
            }
          while (!(cur_tok == 0));

          align_state = 1000000L;
          goto lab30;
        }

        store_new_token(cur_tok);
      }
lab30:
      end_file_reading();
    }
  while (!(align_state == 1000000L));

  cur_val = def_ref;
  scanner_status = normal;
  align_state = s;
}
/* sec 0494 */
void pass_text (void)
{
  integer l;
  small_number save_scanner_status;

  save_scanner_status = scanner_status;
  scanner_status = skipping;
  l = 0;
  skip_line = line;

  while (true)
  {
    get_next();

    if (cur_cmd == fi_or_else)
    {
      if (l == 0)
        goto lab30;

      if (cur_chr == 2)
        decr(l);
    }
    else if (cur_cmd == if_test)
      incr(l);
  }
lab30:
  scanner_status = save_scanner_status;
}
/* sec 0497 */
void change_if_limit_(small_number l, halfword p)
{
  halfword q;

  if (p == cond_ptr)
    if_limit = l;
  else
  {
    q = cond_ptr;

    while (true)
    {
      if (q == 0)
      {
        confusion("if");
        return;       // abort_flag set
      }

      if (link(q) == p)
      {
        type(p) = l;
        return;
      }

      q = link(q);
    }
  }
}
/* called from tex2.c */
/* sec 0498 */
void conditional (void)
{
  boolean b;
  char r;
  integer m, n;
  halfword p, q;
  small_number save_scanner_status;
  halfword savecondptr;
  small_number thisif;

  {
    p = get_node(if_node_size);
    link(p) = cond_ptr;
    type(p) = if_limit;
    subtype(p) = cur_if;
    if_line_field(p) = if_line;
    cond_ptr = p;
    cur_if = cur_chr;
    if_limit = if_code;
    if_line = line;
  }

  savecondptr = cond_ptr;
  thisif = cur_chr;

  switch (thisif)
  {
    case if_char_code:
    case if_cat_code:
      {
        {
          get_x_token();

          if (cur_cmd == relax)
            if (cur_chr == no_expand_flag)
            {
              cur_cmd = active_char;
              cur_chr = cur_tok - cs_token_flag - active_base;
            }
        }

        if ((cur_cmd > active_char) || (cur_chr > 255))
        {
          m = relax;
          n = 256;
        }
        else
        {
          m = cur_cmd;
          n = cur_chr;
        }
        {
          get_x_token();

          if (cur_cmd == relax)
            if (cur_chr == no_expand_flag)
            {
              cur_cmd = active_char;
              cur_chr = cur_tok - cs_token_flag - active_base;
            }
        }

        if ((cur_cmd > active_char) || (cur_chr > 255))
        {
          cur_cmd = relax;
          cur_chr = 256;
        }

        if (thisif == if_char_code)
          b = (n == cur_chr); 
        else
          b = (m == cur_cmd);
      }
      break;

    case if_int_code:
    case if_dim_code:
      {
        if (thisif == if_int_code)
          scan_int();
        else
          scan_dimen(false, false, false);

        n = cur_val;
        
        do
          {
            get_x_token();
          }
        while (!(cur_cmd != spacer));

        if ((cur_tok >= other_token + '<') && (cur_tok <= other_token + '>'))
          r = cur_tok - other_token;
        else
        {
          print_err("Missing = inserted for ");
          print_cmd_chr(if_test, thisif);
          help1("I was expecting to see `<', `=', or `>'. Didn't.");
          back_error();
          r = '=';
        }

        if (thisif == if_int_code)
          scan_int();
        else 
          scan_dimen(false, false, false);

        switch (r)
        {
          case '<':
            b = (n < cur_val);
            break;

          case '=':
            b = (n == cur_val);
            break;

          case '>':
            b = (n > cur_val);
            break;
        }
      }
      break;

    case if_odd_code:
      scan_int();
      b = odd(cur_val);
      break;

    case if_vmode_code:
      b = (abs(mode) == 1);
      break;

    case if_hmode_code:
      b = (abs(mode) == 102);
      break;

    case if_mmode_code:
      b = (abs(mode) == 203);
      break;

    case if_inner_code:
      b = (mode < 0);
      break;

    case if_void_code:
    case if_hbox_code:
    case if_vbox_code:
      {
        scan_eight_bit_int();
        p = box(cur_val);

        if (thisif == if_void_code)
          b = (p == 0);
        else if (p == 0)
          b = false;
        else if (thisif == if_hbox_code)
          b = (type(p) == hlist_node);
        else
          b = (type(p) == vlist_node);
      }
      break;

    case ifx_code:
      {
        save_scanner_status = scanner_status;
        scanner_status = 0;
        get_next();
        n = cur_cs;
        p = cur_cmd;
        q = cur_chr;
        get_next();

        if (cur_cmd != p)
          b = false;
        else if (cur_cmd < call)
          b = (cur_chr == q);
        else
        {
          p = link(cur_chr);
          q = link(equiv(n));

          if (p == q)
            b = true;
          else
          {
            while ((p != 0) && (q != 0))
              if (info(p) != info(q))
                p = 0;
              else
              {
                p = link(p);
                q = link(q);
              }

            b = ((p == 0) && (q == 0));
          }
        }

        scanner_status = save_scanner_status;
      }
      break;

    case if_eof_code:
      {
        scan_four_bit_int();
        b = (read_open[cur_val] == closed);
      }
      break;

    case if_true_code:
      b = true;
      break;

    case if_false_code:
      b = false;
      break;

    case if_case_code:
      {
        scan_int();
        n = cur_val;

        if (tracing_commands > 1)
        {
          begin_diagnostic();
          print_string("{case ");
          print_int(n); 
          print_char('}');
          end_diagnostic(false);
        }

        while (n != 0)
        {
          pass_text();

          if (cond_ptr == savecondptr)
            if (cur_chr == or_code)
              decr(n);
            else 
              goto lab50;
          else if (cur_chr == fi_code)
          {
            p = cond_ptr;
            if_line = if_line_field(p);
            cur_if = subtype(p);
            if_limit = type(p);
            cond_ptr = link(p);
            free_node(p, if_node_size);
          }
        }

        change_if_limit(or_code, savecondptr);
        return;
      }
      break;
  }

  if (tracing_commands > 1)
  {
    begin_diagnostic();

    if (b)
      print_string("{true}");
    else
      print_string("{false}");

    end_diagnostic(false);
  }

  if (b)     /* b may be used without ... */
  {
    change_if_limit(else_code, savecondptr);
    return;
  }

  while (true)
  {
    pass_text();

    if (cond_ptr == savecondptr)
    {
      if (cur_chr != or_code)
        goto lab50;

      print_err("Extra ");
      print_esc("or");
      help1("I'm ignoring this; it doesn't match any \\if.");
      error();
    }
    else if (cur_chr == fi_code)
    {
      p = cond_ptr;
      if_line = if_line_field(p);
      cur_if = subtype(p);
      if_limit = type(p);
      cond_ptr = link(p);
      free_node(p, if_node_size);
    }
  }

lab50:
  if (cur_chr == fi_code)
  {
    p = cond_ptr;
    if_line = if_line_field(p);
    cur_if = subtype(p);
    if_limit = type(p);
    cond_ptr = link(p);
    free_node(p, if_node_size);
  }
  else
    if_limit = fi_code;
}
/* sec 0515 */
void begin_name (void)
{
  area_delimiter = 0;
  ext_delimiter = 0;
}
/* This gathers up a file name and makes a string of it */
/* Also tries to break it into `file area' `file name' and `file extension' */
/* Used by scan_file_name and prompt_file_name */
/* We assume tilde has been converted to pseudo_tilde and space to pseudo_space */
/* returns false if it is given a space character - end of file name */
/* sec 0516 */
boolean more_name_(ASCII_code c)
{
  if (quoted_file_name == 0 && c == ' ')
    return false;
  else if (quoted_file_name != 0 && c == '"')
  {
    quoted_file_name = 0; /* catch next space character */
    return true;    /* accept ending quote, but throw away */
  }
  else
  {   
    str_room(1);
    append_char(c);
    //  for DOS/Windows
    if ((c == '/' || c == '\\' || c == ':')) 
    {
      area_delimiter = cur_length;
      ext_delimiter = 0;
    } 
    else if (c == '.')
      ext_delimiter = cur_length;

    return true;
  }
}

/* sec 0517 */
void end_name (void) 
{
#ifdef ALLOCATESTRING
  if (str_ptr + 3 > current_max_strings)
    str_start = realloc_str_start(increment_max_strings + 3);

  if (str_ptr + 3 > current_max_strings)
  {
    overflow("number of strings", current_max_strings - init_str_ptr);
    return;
  }
#else
  if (str_ptr + 3 > max_strings)
  {
    overflow("number of strings", max_strings - init_str_ptr);
    return;
  }
#endif

  if (area_delimiter == 0)   // no area delimiter ':' '/' or '\' found
    cur_area = 335;     // "" default area 
  else
  {
    cur_area = str_ptr;
    str_start[str_ptr + 1] = str_start[str_ptr] + area_delimiter;
    incr(str_ptr);
  }

  if (ext_delimiter == 0) // no extension delimiter '.' found
  {
    cur_ext = 335;        // "" default extension 
    cur_name = make_string();
  } 
  else            // did find an extension
  {
    cur_name = str_ptr;
    str_start[str_ptr + 1] = str_start[str_ptr]+ ext_delimiter - area_delimiter - 1;
    incr(str_ptr);
    cur_ext = make_string();
  }
}

/* n current name, a current area, e current extension */
/* result in name_of_file[] */
/* sec 0519 */
void pack_file_name_(str_number n, str_number a, str_number e)
{
  integer k;
  ASCII_code c;
  pool_pointer j;

  k = 0;

  for (j = str_start[a]; j <= str_start[a + 1] - 1; j++)
  {
    c = str_pool[j];
    incr(k);

    if (k <= file_name_size)
      name_of_file[k] = xchr[c];
  }

  for (j = str_start[n]; j <= str_start[n + 1] - 1; j++)
  {
    c = str_pool[j];
    incr(k);

    if (k <= file_name_size)
      name_of_file[k] = xchr[c];
  }

  for (j = str_start[e]; j <= str_start[e + 1] - 1; j++)
  {
    c = str_pool[j];
    incr(k);

    if (k <= file_name_size)
      name_of_file[k] = xchr[c];
  }

  if (k < file_name_size)
    name_length = k;
  else
    name_length = file_name_size - 1;

/*  pad it out with spaces ... what for ? in case we modify and forget  ? */
  for (k = name_length + 1; k <= file_name_size; k++)
    name_of_file[k] = ' ';

  name_of_file[file_name_size] = '\0';    /* paranoia 94/Mar/24 */

  {
    name_of_file [name_length+1] = '\0';

    if (trace_flag)
    {
      sprintf(log_line, " pack_file_name `%s' (%d) ", name_of_file + 1, name_length); /* debugging */
      show_line(log_line, 0);
    }

    name_of_file [name_length + 1] = ' ';
  }
}
/* Called only from two places tex9.c for format name - specified and default */
/* for specified format name args are 0, a, b name in buffer[a] --- buffer[b] */
/* for default args are format_default_length-4, 1, 0 */
/* sec 0523 */
void pack_buffered_name_(small_number n, integer a, integer b)
{
  integer k;
  ASCII_code c;
  integer j;

  if (n + b - a + 5 > file_name_size)
    b = a + file_name_size - n - 5;

  k = 0;

/*  This loop kicks in when we want the default format name */
  for (j = 1; j <= n; j++)
  {
    c = xord[TEX_format_default[j]];
    incr(k);

    if (k <= file_name_size)
      name_of_file[k] = xchr[c];
  }
/*  This loop kicks in when we want a specififed format name */
  for (j = a; j <= b; j++)
  {
    c = buffer[j];
    incr(k);

    if (k <= file_name_size)
      name_of_file[k] = xchr[c];
  }

/*  This adds the extension from the default format name */
  for (j = format_default_length - 3; j <= format_default_length; j++)
  {
    c = xord[TEX_format_default[j]];
    incr(k);

    if (k <= file_name_size)
      name_of_file[k] = xchr[c];
  }

  if (k < file_name_size)
    name_length = k;
  else
    name_length = file_name_size - 1;

 /*  pad it out with spaces ... what for ? */
  for (k = name_length + 1; k <= file_name_size; k++)
    name_of_file[k]= ' ';

  name_of_file[file_name_size] = '\0';    /* paranoia 94/Mar/24 */
}
/* sec 0525 */
str_number make_name_string (void)
{
  integer k;

#ifdef ALLOCATESTRING
  if (pool_ptr + name_length > current_pool_size)
    str_pool = realloc_str_pool(increment_pool_size + name_length);

  if (str_ptr == current_max_strings)
    str_start = realloc_str_start(increment_max_strings);

  if ((pool_ptr + name_length > current_pool_size) || (str_ptr == current_max_strings) || (cur_length > 0))
#else
  if ((pool_ptr + name_length > pool_size) || (str_ptr == max_strings) || (cur_length > 0))
#endif
  {
    return '?';
  }
  else
  {
    for (k = 1; k <= name_length; k++)
      append_char(xord[name_of_file[k]]);

    return make_string();
  }
}
/* sec 0525 */
//str_number a_make_name_string_(alpha_file * f)
str_number a_make_name_string_(void)
{
  return make_name_string();
}
/* sec 0525 */
//str_number b_make_name_string_(byte_file * f)
str_number b_make_name_string_(void)
{
  return make_name_string(); 
}
/* sec 0525 */
//str_number w_make_name_string_(word_file * f)
str_number w_make_name_string_(void)
{
  return make_name_string();
}

/* Used by start_input to scan file name on command line */
/* Also in tex8.c new_font_, open_or_close_in, and do_extension */
/* sec 0526 */
void scan_file_name (void)
{
  name_in_progress = true;
  begin_name();

  do
    {
      get_x_token(); 
    }
  while (!(cur_cmd != spacer));

  quoted_file_name = false;

  if (allow_quoted_names)
  {
    if (cur_chr == '"')
    {
      quoted_file_name = 1;
      get_x_token();
    }
  }

  while (true)
  {
    if ((cur_cmd > other_char) || (cur_chr > 255)) 
    {
      back_input();
      goto lab30; 
    } 

    if (!more_name(cur_chr))    /* up to next white space */
      goto lab30;

    get_x_token();
  }

lab30:
  end_name();
  name_in_progress = false;
}
/* argument is string .fmt, .log, or .dvi */
/* sec 0529 */
void pack_job_name_(str_number s)
{
  cur_area = 335;       /* "" */
  cur_ext  = s;
  cur_name = job_name;
  pack_file_name(cur_name, cur_area, cur_ext);
}

/**********************************************************************/
/* sec 0530 */
/* s - what can't be found, e - default */
void prompt_file_name_(char * s, str_number e) 
{
  integer k;

  if (interaction == scroll_mode);

  if (!strcmp("input file name", s))
    print_err("I can't find file `");
  else
    print_err("I can't write on file `");

  print_file_name(cur_name, cur_area, cur_ext);
  print_string("'.");

  if (e == 785)    /* .tex */
    show_context();

  print_nl("Please type another ");
  print_string(s); 

  if (interaction < 2)
  {
    fatal_error("*** (job aborted, file error in nonstop mode)");
    return;     // abort_flag set
  }

  if (!knuth_flag)
#ifdef _WINDOWS
    show_line(" (or ^z to exit)", 0);
#else
    show_line(" (or Ctrl-Z to exit)", 0);
#endif
  prompt_input(": ");

/*  should we deal with tilde and space in file name here ??? */
  {
    begin_name();
    k = first;

    while ((buffer[k] == ' ') && (k < last))
      incr(k);
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
    quoted_file_name = 0;         /* 98/March/15 */

    if (allow_quoted_names && k < last) /* check whether quoted name */
    {
      if (buffer[k]== '"')
      {
        quoted_file_name = 1;
        incr(k);
      }
    }

    while (true)
    {
      if (k == last)
        goto lab30;
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/*  convert tilde '~' to pseudo tilde */
      if (pseudo_tilde != 0 && buffer[k]== '~')
        buffer[k] = pseudo_tilde;
/*  convert space ' ' to pseudo space */
      if (pseudo_space != 0 && buffer[k]== ' ')
        buffer[k] = pseudo_space;
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
      if (!more_name(buffer[k]))
        goto lab30;

      incr(k);
    }
lab30:
    end_name();
  }

  if (cur_ext == 335)    /* "" */
    cur_ext = e;      /* use default extension */

  pack_file_name(cur_name, cur_area, cur_ext);
}
/* sec 0534 */
void open_log_file (void)
{
  char old_setting;
  integer k;
  integer l;
  char * months;

  old_setting = selector;

  if (job_name == 0)
    job_name = 790;

  pack_job_name(".log");

  while (!a_open_out(log_file))
  {
    selector = term_only;
    prompt_file_name("transcript file name", ".log");
  }

  texmf_log_name = a_make_name_string(log_file);
  selector = log_only;
  log_opened = true;

  {
    if (want_version)
    {
      stamp_it(log_line);
      strcat(log_line, "\n");
      (void) fputs(log_line, log_file);
      stampcopy(log_line);
      strcat(log_line, "\n");
      (void) fputs(log_line, log_file);
    }
    
    (void) fputs(tex_version, log_file); 
    (void) fprintf(log_file, " (%s %s)", application, yandyversion);

    if (format_ident > 0)
      slow_print(format_ident);

    print_string("  ");

    if (civilize_flag)
      print_int(year);
    else
      print_int(day);

    print_char(' ');
    months = " JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC";

    for (k = 3 * month - 2; k <= 3 * month; k++)
      (void) putc(months[k],  log_file);

    print_char(' ');

    if (civilize_flag)
      print_int(day);
    else
      print_int(year);

    print_char(' ');
    print_two(tex_time / 60);
    print_char(':');
    print_two(tex_time % 60);
  }

  input_stack[input_ptr] = cur_input;
  print_nl("**");
  l = input_stack[0].limit_field;

  if (buffer[l] == end_line_char)
    decr(l);

  for (k = 1; k <= l; k++)
    print(buffer[k]);

  print_ln(); 

  if (show_fmt_flag)
  {
    if (format_file != NULL)
    {
      fprintf(log_file, "(%s)\n", format_file);
      free(format_file);
      format_file = NULL;
    }
  }

  selector = old_setting + 2;
}

// Attempt to deal with foo.bar.tex given as foo.bar on command line
// Makes copy of job_name with extension

void more_name_copy(ASCII_code c)
{
#ifdef ALLOCATESTRING
  if (pool_ptr + 1 > current_pool_size)
    str_pool = realloc_str_pool (increment_pool_size);

  if (pool_ptr + 1 > current_pool_size)
  {
    overflow("pool size", current_pool_size - init_pool_ptr);
    return;
  }
#else
  if (pool_ptr + 1 > pool_size)
  {
    overflow("pool size", pool_size - init_pool_ptr);
    return;
  }
#endif

  str_pool[pool_ptr] = c; 
  incr(pool_ptr);
}

int end_name_copy(void)
{
#ifdef ALLOCATESTRING
  if (str_ptr + 1 > current_max_strings)
    str_start = realloc_str_start(increment_max_strings + 1);

  if (str_ptr + 1 > current_max_strings)
  {
    overflow("number of strings", current_max_strings - init_str_ptr);
    return 0;
  }
#else
  if (str_ptr + 1 > max_strings)
  {
    overflow("number of strings", max_strings - init_str_ptr);
    return 0;
  }
#endif

  return make_string();
}

void job_name_append (void)
{ 
  int k, n;

  k = str_start[job_name];
  n = str_start[job_name + 1];

  while (k < n)
    more_name_copy(str_pool[k++]);

  k = str_start[cur_ext];
  n = str_start[cur_ext + 1];

  while (k < n)
    more_name_copy(str_pool[k++]);

  job_name = end_name_copy();
}

/* sec 0537 */
void start_input(void)
{
  scan_file_name();
  pack_file_name(cur_name, cur_area, cur_ext); 

  while (true)
  {
    begin_file_reading();
    
    if (a_open_in(cur_file, TEXINPUTPATH))
      goto lab30;

    end_file_reading();
    prompt_file_name("input file name", ".tex");
  }

lab30: 
  cur_input.name_field = a_make_name_string(cur_file);

  if (job_name == 0)
  {
    job_name = cur_name;
    open_log_file();
  }

  if (term_offset + length(cur_input.name_field) > max_print_line - 2)
    print_ln();
  else if ((term_offset > 0) || (file_offset > 0))
    print_char(' ');

  print_char('(');
  incr(open_parens);

  if (open_parens > max_open_parens)
    max_open_parens = open_parens;

  slow_print(cur_input.name_field);

#ifndef _WINDOWS
  fflush(stdout);
#endif

  state = new_line;

  {
    line = 1;

    if (input_ln(cur_file, false));

    firm_up_the_line();

    if (end_line_char_inactive())
      decr(limit);
    else
      buffer[limit] = end_line_char;

    first = limit + 1;
    loc = start;
  }
}
/* sec 0560 */
internal_font_number read_font_info_(halfword u, str_number nom, str_number aire, scaled s)
{
  font_index k;
  boolean file_opened;
  halfword lf, lh, nw, nh, nd, ni, nl, nk, ne, np;
  int bc, ec;
  internal_font_number f;
  internal_font_number g;
  eight_bits a, b, c, d;
  four_quarters qw;
  scaled sw;
  integer bch_label;
  short bchar;
  scaled z;
  integer alpha;
  char beta;

  g = 0;
  file_opened = false;
  pack_file_name(nom, aire, 805); /* .tfm */

  if (!b_open_in(tfm_file))
  {
    goto lab11;
  } 

  file_opened = true; 

  {
    read_sixteen(lf);
    tfm_temp = getc(tfm_file);
    read_sixteen(lh);
    tfm_temp = getc(tfm_file);
    read_sixteen(bc);
    tfm_temp = getc(tfm_file);
    read_sixteen(ec);

    if ((bc > ec + 1) || (ec > 255))
      goto lab11;

    if (bc > 255)
    {
      bc = 1;
      ec = 0;
    }

    tfm_temp = getc(tfm_file);
    read_sixteen(nw);
    tfm_temp = getc(tfm_file);
    read_sixteen(nh);
    tfm_temp = getc(tfm_file);
    read_sixteen(nd);
    tfm_temp = getc(tfm_file);
    read_sixteen(ni);
    tfm_temp = getc(tfm_file);
    read_sixteen(nl);
    tfm_temp = getc(tfm_file);
    read_sixteen(nk);
    tfm_temp = getc(tfm_file);
    read_sixteen(ne);
    tfm_temp = getc(tfm_file);
    read_sixteen(np);

    if (lf != 6 + lh + (ec - bc + 1) + nw + nh + nd + ni + nl + nk + ne + np)
      goto lab11;

    if ((nw == 0) || (nh == 0) || (nd == 0) || (ni == 0))
      goto lab11;
  }

  lf = lf - 6 - lh;

  if (np < 7)
    lf = lf + 7 - np;

#ifdef ALLOCATEFONT
  if ((fmem_ptr + lf > current_font_mem_size))
    font_info = realloc_font_info (increment_font_mem_size + lf);

  if ((font_ptr == font_max) || (fmem_ptr + lf > current_font_mem_size))
#else
  if ((font_ptr == font_max) || (fmem_ptr + lf > font_mem_size))
#endif
  {
    if (trace_flag)
    {
      sprintf(log_line, "font_ptr %d font_max %d fmem_ptr %d lf %d font_mem_size %d\n",
          font_ptr, font_max, fmem_ptr, lf, font_mem_size);
      show_line(log_line, 0);
    }

    print_err("Font ");
    sprint_cs(u);
    print_char('=');
    print_file_name(nom, aire, 335); /* "" */

    if (s >= 0)
    {
      print_string(" at ");
      print_scaled(s);
      print_string("pt");
    }
    else if (s != -1000)
    {
      print_string(" scaled ");
      print_int(- (integer) s);
    }

    print_string(" not loaded: Not enough room left");
    help4("I'm afraid I won't be able to make use of this font,",
        "because my memory for character-size data is too small.",
        "If you're really stuck, ask a wizard to enlarge me.",
        "Or maybe try `I\\font<same font id>=<name of loaded font>'.");
    error();
    goto lab30;
  }

  f = font_ptr + 1;
  char_base[f] = fmem_ptr - bc;
  width_base[f] = char_base[f] + ec + 1;
  height_base[f] = width_base[f] + nw;
  depth_base[f] = height_base[f] + nh;
  italic_base[f] = depth_base[f] + nd;
  lig_kern_base[f] = italic_base[f] + ni;
  kern_base[f] = lig_kern_base[f] + nl - 256 * (128);
  exten_base[f] = kern_base[f] + 256 * (128) + nk;
  param_base[f] = exten_base[f] + ne;

  {
    if (lh < 2)
      goto lab11;
    
    store_four_quarters(font_check[f]);
    tfm_temp = getc(tfm_file);
    read_sixteen(z);
    tfm_temp = getc(tfm_file);
    z = z * 256 + tfm_temp;
    tfm_temp = getc(tfm_file);
    z =(z * 16) + (tfm_temp / 16);

    if (z < 65536L)
      goto lab11; 

    while (lh > 2)
    {
      tfm_temp = getc(tfm_file);
      tfm_temp = getc(tfm_file);
      tfm_temp = getc(tfm_file);
      tfm_temp = getc(tfm_file);
      decr(lh);
    }

    font_dsize[f] = z;

    if (s != -1000)
      if (s >= 0)
        z = s;
      else
        z = xn_over_d(z, - (integer) s, 1000);

    font_size[f] = z;
  }

  for (k = fmem_ptr; k <= width_base[f] - 1; k++)
  {
    store_four_quarters(font_info[k].qqqq);

    if ((a >= nw) || (b / 16 >= nh) || (b % 16 >= nd) || (c / 4 >= ni))
      goto lab11;

    switch (c % 4)
    {
      case lig_tag:
        if (d >= nl)
          goto lab11;
        break;

      case ext_tag:
        if (d >= ne)
          goto lab11;
        break;

      case list_tag:
        {
          {
            if ((d < bc) || (d > ec))
              goto lab11;
          }

          while (d < k + bc - fmem_ptr)
          {
            qw = char_info(f, d);
 
            if (char_tag(qw) != list_tag)
              goto lab45;

            d = rem_byte(qw);
          }

          if (d == k + bc - fmem_ptr)
            goto lab11;
lab45:; 
        }
        break;

      default:
        break;
    }
  }

  {
    {
      alpha = 16;

      while (z >= 8388608L)   /* 2^23 */
      {
        z = z / 2;
        alpha = alpha + alpha;
      }

      beta = (char) (256 / alpha);
      alpha = alpha * z;
    }

    for (k = width_base[f]; k <= lig_kern_base[f] - 1; k++)
    {
      tfm_temp = getc(tfm_file);
      a = tfm_temp;
      tfm_temp = getc(tfm_file);
      b = tfm_temp;
      tfm_temp = getc(tfm_file);
      c = tfm_temp;
      tfm_temp = getc(tfm_file);
      d = tfm_temp;
      sw = (((((d * z) / 256) + (c * z)) / 256) + (b * z)) / beta;

      if (a == 0)
        font_info[k].cint = sw;
      else if (a == 255)
        font_info[k].cint = sw - alpha;
      else
        goto lab11;
    }

    if (font_info[width_base[f]].cint != 0)
      goto lab11;

    if (font_info[height_base[f]].cint != 0)
      goto lab11;

    if (font_info[depth_base[f]].cint != 0)
      goto lab11;

    if (font_info[italic_base[f]].cint != 0)
      goto lab11;
  }

  bch_label = 32767;     /* '77777 */
  bchar = 256;

  if (nl > 0)
  {
    for (k = lig_kern_base[f]; k <= kern_base[f] + 256 * (128) - 1; k++)
    {
      store_four_quarters(font_info[k].qqqq);

      if (a > 128)
      {
        if (256 * c + d >= nl)
          goto lab11;       /* error in TFM, abort */

        if (a == 255)
          if (k == lig_kern_base[f])
            bchar = b;
      }
      else
      {
        if (b != bchar)
        {
          {
            if ((b < bc) || (b > ec))  /* check-existence(b) */
              goto lab11;         /* error in TFM, abort */
          }

          qw = font_info[char_base[f] + b].qqqq;

          if (!(qw.b0 > 0))
            goto lab11;         /* error in TFM, abort */
        }

        if (c < 128)
        {
          {
            if ((d < bc) || (d > ec))  /* check-existence(d) */
              goto lab11;         /* error in TFM, abort */
          }

          qw = font_info[char_base[f] + d].qqqq;

          if (!(qw.b0 > 0))
            goto lab11;         /* error in TFM, abort */
        }
        else if (256 * (c - 128) + d >= nk)
          goto lab11;           /* error in TFM, abort */

        if (a < 128)
          if (k - lig_kern_base[f] + a + 1 >= nl)
            goto lab11;         /* error in TFM, abort */
      }
    }

    if (a == 255)
      bch_label = 256 * c + d;
  }

  for (k = kern_base[f] + 256 * (128); k <= exten_base[f] - 1; k++)
  {
    tfm_temp = getc(tfm_file);
    a = tfm_temp;
    tfm_temp = getc(tfm_file);
    b = tfm_temp;
    tfm_temp = getc(tfm_file);
    c = tfm_temp;
    tfm_temp = getc(tfm_file);
    d = tfm_temp;
    sw = (((((d * z) / 256) + (c * z)) / 256) + (b * z)) / beta;

    if (a == 0)
      font_info[k].cint = sw;
    else if (a == 255)
      font_info[k].cint = sw - alpha;
    else goto lab11;
  }

  /*  read extensible character recipes */
  for (k = exten_base[f]; k <= param_base[f] - 1; k++)
  {
    store_four_quarters(font_info[k].qqqq);

    if (a != 0)
    {
      {
        if ((a < bc) || (a > ec))
          goto lab11;
      }

      qw = font_info[char_base[f] + a].qqqq;

      if (!(qw.b0 > 0))
        goto lab11;
    }

    if (b != 0)
    {
      {
        if ((b < bc) || (b > ec))
          goto lab11;
      }

      qw = font_info[char_base[f] + b].qqqq;

      if (!(qw.b0 > 0))
        goto lab11;
    }

    if (c != 0)
    {
      {
        if ((c < bc) || (c > ec))
          goto lab11;
      }

      qw = font_info[char_base[f] + c].qqqq;

      if (!(qw.b0 > 0))
        goto lab11;
    }

    {
      {
        if ((d < bc) || (d > ec))
          goto lab11;
      }

      qw = font_info[char_base[f] + d].qqqq;

      if (!(qw.b0 > 0))
        goto lab11;
    }
  }

  {
    for (k = 1; k <= np; k++)
      if (k == 1)
      {
        tfm_temp = getc(tfm_file);
        sw = tfm_temp;

        if (sw > 127)
          sw = sw - 256;

        tfm_temp = getc(tfm_file);
        sw = sw * 256 + tfm_temp;
        tfm_temp = getc(tfm_file);
        sw = sw * 256 + tfm_temp;
        tfm_temp = getc(tfm_file);
        font_info[param_base[f]].cint = (sw * 16) + (tfm_temp / 16);
      }
      else
      {
        tfm_temp = getc(tfm_file);
        a = tfm_temp;
        tfm_temp = getc(tfm_file);
        b = tfm_temp;
        tfm_temp = getc(tfm_file);
        c = tfm_temp;
        tfm_temp = getc(tfm_file);
        d = tfm_temp;
        sw = (((((d * z) / 256) + (c * z)) / 256) + (b * z)) / beta;

        if (a == 0)
          font_info[param_base[f] + k - 1].cint = sw;
        else if (a == 255)
          font_info[param_base[f] + k - 1].cint = sw - alpha;
        else goto lab11;
      }

    if (feof(tfm_file))
      goto lab11;

    for (k = np + 1; k <= 7; k++)
      font_info[param_base[f] + k - 1].cint = 0;
  }

  if (np >= 7)
    font_params[f] = np;
  else
    font_params[f] = 7;

  hyphen_char[f] = default_hyphen_char;
  skew_char[f] = default_skew_char;

  if (bch_label < nl)
    bchar_label[f] = bch_label + lig_kern_base[f];
  else
    bchar_label[f] = non_address;

  font_bchar[f] = bchar;
  font_false_bchar[f] = bchar;

  if (bchar <= ec)
    if (bchar >= bc)
    {
      qw = font_info[char_base[f] + bchar].qqqq;

      if ((qw.b0 > 0))
        font_false_bchar[f] = 256;
    }

  font_name[f] = nom;
  font_area[f] = aire;
  font_bc[f] = bc;
  font_ec[f] = ec;
  font_glue[f] = 0;
  char_base[f] = char_base[f];
  width_base[f] = width_base[f];
  lig_kern_base[f] = lig_kern_base[f];
  kern_base[f] = kern_base[f];
  exten_base[f] = exten_base[f];
  decr(param_base[f]);
  fmem_ptr = fmem_ptr + lf;
  font_ptr = f;
  g = f;
  goto lab30;

lab11:
  print_err("Font ");
  sprint_cs(u); 
  print_char('=');
  print_file_name(nom, aire, 335);

  if (s >= 0)
  {
    print_string(" at ");
    print_scaled(s);
    print_string("pt");
  }
  else if (s != -1000)
  {
    print_string("scaled");
    print_int(- (integer) s);
  } 

  if (file_opened)
    print_string(" not loadable: Bad metric (TFM) file");
  else
    print_string(" not loadable: Metric (TFM) file not found");

  help5("I wasn't able to read the size data for this font,",
      "so I will ignore the font specification.",
      "[Wizards can fix TFM files using TFtoPL/PLtoTF.]",
      "You might try inserting a different font spec;",
      "e.g., type `I\\font<same font id>=<substitute font name>'.");
  error();

lab30:
  if (file_opened)
    b_close(tfm_file);

  return g;
}