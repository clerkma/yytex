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

/* sec 0581 */
void char_warning_(internal_font_number f, eight_bits c)
{ 
  if (tracing_lost_chars > 0)
  {
    if (show_missing == 0)
      begin_diagnostic();

    if (show_missing) /* add ! before 94/June/10 */
    {
      print_nl("! ");
      prints("Missing character: there is no ");
    }
    else
      print_nl("Missing character: there is no ");

    print(c);

    if (show_numeric) /* bkph 93/Dec/21 */
    {
      print_char(' ');
      print_char('(');

      if (c / 100 > 0)
      {
        print_char('0' + c / 100);
        c = c - (c / 100) * 100;
        print_char('0' + c / 10);
      }
      else
      {
        c = c - (c / 100) * 100;
        if (c / 10 > 0)
          print_char('0' + c / 10);
      }
      print_char('0' + c % 10);
      print_char(')');
    }

    prints(" in font ");
    slow_print(font_name[f]);
    print_char('!');

    if (show_missing)
    {
      if (f != 0)
        show_context();     /* not if its the nullfont */
    }

    if (show_missing == 0)
      end_diagnostic(false);

    missing_characters++;           /* bkph 93/Dec/16 */
  }
}
/* sec 0582 */
halfword new_character_(internal_font_number f, eight_bits c)
{
  halfword p;

  if (font_bc[f] <= c)
    if (font_ec[f] >= c)
      if (char_exists(char_info(f, c)))
      {
        p = get_avail();
        font(p) = f;
        character(p) = c;
        return p;
      }

  char_warning(f, c);
  return 0;
}
/* sec 0598 */
void dvi_swap (void)
{ 
  if (trace_flag)
  {
    show_char('\n');
    sprintf(log_line, "dvi_swap() %lld", dvi_gone);
    show_line(log_line, 0);
  }

  if (dvi_limit == dvi_buf_size)
  {
    write_dvi(0, half_buf - 1);
    dvi_limit = half_buf;
    dvi_offset = dvi_offset + dvi_buf_size;
    dvi_ptr = 0;
  }
  else
  {
    write_dvi(half_buf, dvi_buf_size - 1);
    dvi_limit = dvi_buf_size;
  }

  dvi_gone = dvi_gone + half_buf;
}
/* sec 0600 */
void dvi_four_(integer x)
{ 
  if (x >= 0)
    dvi_out(x / 0100000000);
  else
  {
    x = x + 010000000000;
    x = x + 010000000000;
    dvi_out((x / 0100000000) + 128);
  }

  x = x % 0100000000;
  dvi_out(x / 0200000);
  x = x % 0200000;
  dvi_out(x / 0400);
  dvi_out(x % 0400);
}
/* sec 0601 */
void dvi_pop_(integer l)
{
  if ((l == dvi_offset + dvi_ptr) && (dvi_ptr > 0))
    decr(dvi_ptr);
  else
    dvi_out(pop);
}
/* sec 0602 */
void dvi_font_def_(internal_font_number f)
{
  pool_pointer k;

#ifdef INCREASEFONTS
  if (f <= 256)
  {
    dvi_out(fnt_def1);
    dvi_out(f - 1);
  }
  else
  {
    dvi_out(fnt_def2);
    dvi_out(((f - 1) >> 8));
    dvi_out(((f - 1) & 255));
  }
#else
  dvi_out(fnt_def1);
  dvi_out(f - 1);
#endif

  dvi_out(font_check[f].b0);
  dvi_out(font_check[f].b1);
  dvi_out(font_check[f].b2);
  dvi_out(font_check[f].b3);
  dvi_four(font_size[f]); 
  dvi_four(font_dsize[f]);
  dvi_out(length(font_area[f]));
  dvi_out(length(font_name[f]));

  for (k = str_start[font_area[f]]; k <= str_start[font_area[f] + 1] - 1; k++)
    dvi_out(str_pool[k]);

  for (k = str_start[font_name[f]]; k <= str_start[font_name[f] + 1] - 1; k++)
    dvi_out(str_pool[k]);
}
/* sec 0607 */
void zmovement(scaled w, eight_bits o)
{
  small_number mstate;
  halfword p, q;
  integer k;

  q = get_node(movement_node_size);
  width(q) = w;
  location(q) = dvi_offset + dvi_ptr;

  if (o == down1)
  {
    link(q) = down_ptr;
    down_ptr = q;
  }
  else
  {
    link(q) = right_ptr;
    right_ptr = q;
  }

  p = link(q);
  mstate = none_seen;

  while (p != 0)
  {
    if (width(p) == w)
      switch(mstate + info(p))
      {
        case none_seen + yz_OK:
        case none_seen + y_OK:
        case z_seen + yz_OK:
        case z_seen + y_OK:
          if (location(p) < dvi_gone)
            goto not_found;
          else
          {
            k = location(p) - dvi_offset;

            if (k < 0)
              k = k + dvi_buf_size;

            dvi_buf[k] = dvi_buf[k] + y1 - down1;
            info(p) = y_here;
            goto found;
          }
          break;

        case none_seen + z_OK:
        case y_seen + yz_OK:
        case y_seen + z_OK:
          if (location(p) < dvi_gone)
            goto not_found;
          else
          {
            k = location(p) - dvi_offset;

            if (k < 0)
              k = k + dvi_buf_size;

            dvi_buf[k] = dvi_buf[k] + z1 - down1;
            info(p) = z_here;
            goto found;
          }
          break;

        case none_seen + y_here:
        case none_seen + z_here:
        case y_seen + z_here:
        case z_seen + y_here:
          goto found;
          break;

        default:
          break;
      }
    else
      switch (mstate + info(p))
      {
        case none_seen + y_here:
          mstate = y_seen;
          break;

        case none_seen + z_here:
          mstate = z_seen;
          break;

        case y_seen + z_here:
        case z_seen + y_here:
          goto not_found;
          break;

        default:
          break;
      }

    p = link(p);
  }
not_found:

  info(q) = yz_OK;

  if (abs(w) >= 8388608L) /* 2^23 */
  {
    dvi_out(o + 3);
    dvi_four(w);
    return;
  }

  if (abs(w) >= 32768L)
  {
    dvi_out(o + 2);

    if (w < 0)
      w = w + 16777216L;  /* 2^24 */
    //dvi_out(w / 65536L);
    dvi_out((w >> 16));
/*    w = w % 65536L; */
    w = w & 65535L;
    goto lab2;
  }

  if (abs(w)>= 128)
  {
    dvi_out(o + 1);

    if (w < 0)
      w = w + 65536L;

    goto lab2;
  }

  dvi_out(o);

  if (w < 0)
    w = w + 256;

  goto lab1;

lab2:
  dvi_out(w / 256);

lab1:
  dvi_out(w % 256);
  return;

found:
  info(q) = info(p);

  if (info(q) == y_here)
  {
    dvi_out(o + y0 - down1);

    while (link(q) != p)
    {
      q = link(q);

      switch (info(q))
      {
        case yz_OK:
          info(q) = z_OK;
          break;

        case y_OK:
          info(q) = d_fixed;
          break;

        default:
          break;
      }
    }
  }
  else
  {
    dvi_out(o + z0 - down1);

    while (link(q) != p)
    {
      q = link(q);

      switch (info(q))
      {
        case yz_OK:
          info(q) = y_OK;
          break;

        case z_OK:
          info(q) = d_fixed;
          break;

        default:
          break;
      }
    }
  }
}
/* sec 0615 */
void prune_movements_(integer l)
{
  halfword p;

  while (down_ptr != 0)
  {
    if (location(down_ptr) < l)
      goto done;

    p = down_ptr;
    down_ptr = link(p);
    free_node(p, movement_node_size);
  }

done:
  while (right_ptr != 0)
  {
    if (location(right_ptr) < l)
      return;

    p = right_ptr;
    right_ptr = link(p);
    free_node(p, movement_node_size);
  }
}
/* sec 1368 */
void special_out_(pointer p)
{
  char old_setting;
  pool_pointer k;

  synch_h();
  synch_v();
  old_setting = selector;
  selector = new_string;

#ifdef ALLOCATESTRING
  if (pool_ptr + 32000 > current_pool_size)
    str_pool = realloc_str_pool (increment_pool_size);

  show_token_list(link(write_tokens(p)), 0, 10000000L);
#else
  show_token_list(link(write_tokens(p)), 0, pool_size - pool_ptr);
#endif

  selector = old_setting;
  str_room(1);

  if (cur_length < 256)
  {
    dvi_out(xxx1);
    dvi_out(cur_length);
  }
  else
  {
    dvi_out(xxx4);
    dvi_four(cur_length); 
  } 

  for (k = str_start[str_ptr]; k <= pool_ptr - 1; k++)
    dvi_out(str_pool[k]);

  pool_ptr = str_start[str_ptr];
}
/* sec 1370 */
void write_out_(pointer p)
{
  char old_setting;
/*  integer oldmode;  */
  int oldmode;          /* 1995/Jan/7 */
/*  small_number j;  */
  int j;              /* 1995/Jan/7 */
  halfword q, r;

  q = get_avail();
  info(q) = right_brace_token + '}';
  r = get_avail();
  link(q) = r;
  info(r) = end_write_token;
  ins_list(q);
  begin_token_list(write_tokens(p), write_text);
  q = get_avail();
  info(q) = left_brace_token + '{';
  ins_list(q);
  oldmode = mode;
  mode = 0;
  cur_cs = write_loc;
  q = scan_toks(false, true);
  get_token();

  if (cur_tok != end_write_token)
  {
    print_err("Unbalanced write command");
    help2("On this page there's a \\write with fewer real {'s than }'s.",
        "I can't handle that very well; good luck.");
    error();

    do
      {
        get_token();
      }
    while (!(cur_tok == end_write_token));
  }

  mode = oldmode;
  end_token_list();
  old_setting = selector;
  j = write_stream(p);

  if (write_open[j])
    selector = j;
  else
  {
    if ((j == 17) && (selector == term_and_log))
      selector = log_only;

    print_nl("");
  }

  token_show(def_ref);
  print_ln();
  flush_list(def_ref);
  selector = old_setting;
}
/* sec 1373 */
void out_what_(pointer p)
{
/*  small_number j;  */
  int j;            /* 1995/Jan/7 */

  switch (subtype(p))
  {
    case open_node:
    case write_node:
    case close_node:
      if (!doing_leaders)
      {
        j = write_stream(p);

        if (subtype(p) == write_node)
        {
          write_out(p);
        }
        else
        {
          if (write_open[j])
            a_close(write_file[j]); 

          if (subtype(p) == close_node)
            write_open[j]= false;
          else if (j < 16)
          {
            cur_name = open_name(p);
            cur_area = open_area(p);
            cur_ext = open_ext(p); 

            if (cur_ext == 335)  /* "" */
              cur_ext = 785;  /* => ".tex" */

            pack_file_name(cur_name, cur_area, cur_ext);

            while (!a_open_out(write_file[j]))
              prompt_file_name("output file name", ".tex");

            write_open[j] = true;
          }
        }
      }
      break; 

    case special_node:
      special_out(p); 
      break;

    case language_node:
      ;
      break;

    default:
      {
        confusion("ext4");
        return;
      }
      break;
  }
}
/* sec 0619 */
void hlist_out (void)
{
  scaled base_line;
  scaled left_edge;
  scaled save_h, save_v;
  halfword this_box;
/*  glue_ord g_order;  */
  int g_order;
/*  char g_sign;  */
  int g_sign;
  halfword p;
  integer save_loc;
  halfword leader_box;
  scaled leader_wd;
  scaled lx;
  boolean outer_doing_leaders;
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
    dvi_out(push);

  if (cur_s > max_push)
    max_push = cur_s;

  save_loc = dvi_offset + dvi_ptr;
  base_line = cur_v;
  left_edge = cur_h;

  while (p != 0)
reswitch:
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
              font_used[f] = true;
            }

            if (f <= 64 + font_base)
              dvi_out(f - font_base - 1 + fnt_num_0);
#ifdef INCREASEFONTS
            /* if we allow greater than 256 fonts */
            else if (f <= 256)
            {
              dvi_out(fnt1);
              dvi_out(f - 1);
            }
#else
            else
            {
              dvi_out(fnt1);
              dvi_out(f - 1);
            }
#endif

#ifdef INCREASEFONTS
            else
            {
              dvi_out(fnt2);
              dvi_out(((f - 1) >> 8));  /* top byte */
              dvi_out(((f - 1) & 255)); /* bottom byte */
            }
#endif

            dvi_f = f;
          }

          if (c >= 128)
            dvi_out(set1);

          dvi_out(c);
          cur_h = cur_h + char_width(f, char_info(f, c));
          p = link(p);
        }
      while (!(!is_char_node(p)));

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
            vlist_out();
          else
            hlist_out();

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
          goto fin_rule;
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
                vet_glue(glue_set(this_box) * cur_glue);
                cur_g = round(glue_temp);
              }
            }
            else if (shrink_order(g) == g_order)
            {
              cur_glue = cur_glue - shrink(g);
              vet_glue(glue_set(this_box) * cur_glue);
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
              goto fin_rule;
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
                  vlist_out();
                else
                  hlist_out();

                doing_leaders = outer_doing_leaders;
                dvi_v = save_v;
                dvi_h = save_h;
                cur_v = base_line;
                cur_h = save_h + leader_wd + lx;
              }

              cur_h = edge - 10;
              goto next_p;
            }
          }

          goto move_past;
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
          goto reswitch;
        }
        break;

      default:
        break;
    }

    goto next_p;

fin_rule:
    if (is_running(rule_ht))
      rule_ht = height(this_box);

    if (is_running(rule_dp))
      rule_dp = depth(this_box);

    rule_ht = rule_ht + rule_dp;

    if ((rule_ht > 0) && (rule_wd > 0))
    {
      synch_h();
      cur_v = base_line + rule_dp;
      synch_v();
      dvi_out(set_rule);
      dvi_four(rule_ht);
      dvi_four(rule_wd);
      cur_v = base_line;
      dvi_h = dvi_h + rule_wd;
    }

move_past:
    cur_h = cur_h + rule_wd;

next_p:
    p = link(p);
  }

  prune_movements(save_loc);

  if (cur_s > 0)
    dvi_pop(save_loc);

  decr(cur_s);
}
/* sec 0629 */
void vlist_out (void)
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
  boolean outer_doing_leaders;
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
    dvi_out(push);

  if (cur_s > max_push)
    max_push = cur_s;

  save_loc = dvi_offset + dvi_ptr;
  left_edge = cur_h;
  cur_v = cur_v - height(this_box);
  top_edge = cur_v;

  while (p != 0)
  {
    if (is_char_node(p))
    {
      confusion("vlistout");
      return;
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
              vlist_out();
            else
              hlist_out();

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
            goto fin_rule;
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
                  vet_glue(glue_set(this_box) * cur_glue);
                  cur_g = round(glue_temp);
                }
              }
              else if (shrink_order(g) == g_order)   /* BUG FIX !!! */
              {
                cur_glue = cur_glue - shrink(g);
                vet_glue(glue_set(this_box) * cur_glue);
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
                goto fin_rule;
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
                    vlist_out();
                  else
                    hlist_out();

                  doing_leaders = outer_doing_leaders;
                  dvi_v = save_v;
                  dvi_h = save_h;
                  cur_h = left_edge;
                  cur_v = save_v - height(leader_box) + leader_ht + lx;
                }

                cur_v = edge - 10;
                goto next_p;
              }
            }

            goto move_past;
          }
          break;

        case kern_node:
          cur_v = cur_v + width(p);
          break;

        default:
          break;
      }

      goto next_p;

fin_rule:
      if (is_running(rule_wd))
        rule_wd = width(this_box);

      rule_ht = rule_ht + rule_dp;
      cur_v = cur_v + rule_ht;

      if ((rule_ht > 0) && (rule_wd > 0))
      {
        synch_h();
        synch_v();
        dvi_out(put_rule);
        dvi_four(rule_ht);
        dvi_four(rule_wd);
      }

      goto next_p;

move_past:
      cur_v = cur_v + rule_ht;
    }

next_p:
    p = link(p);
  }

  prune_movements(save_loc);

  if (cur_s > 0)
    dvi_pop(save_loc);

  decr(cur_s);
}
/* sec 0638 */
void dvi_ship_out_(halfword p)
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
    dvi_out(pre);
    dvi_out(id_byte);
    dvi_four(25400000L);
    dvi_four(473628672L);
    prepare_mag();
    dvi_four(mag);
    old_setting = selector;
    selector = new_string;
    prints(" TeX output ");
    print_int(year);
    print_char('.');
    print_two(month);
    print_char('.');
    print_two(day);
    print_char(':');
    print_two(tex_time / 60);
    print_two(tex_time % 60);
    selector = old_setting;
    dvi_out(cur_length);

    for (s = str_start[str_ptr]; s <= pool_ptr - 1; s++)
      dvi_out(str_pool[s]);

    pool_ptr = str_start[str_ptr];
  }

  page_loc = dvi_offset + dvi_ptr;
  dvi_out(bop);

  for (k = 0; k <= 9; k++)
    dvi_four(count(k));

  dvi_four(last_bop);
  last_bop = page_loc;
  cur_v = height(p) + v_offset;
  temp_ptr = p;

  if (type(p) == vlist_node)
    vlist_out();
  else
    hlist_out();

  dvi_out(eop);
  incr(total_pages);
  cur_s = -1;

done:;
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
void ship_out_(halfword p)
{
  switch (shipout_flag)
  {
    case out_dvi_flag:
    case out_xdv_flag:
      dvi_ship_out_(p);
      break;
  }
}
/* sec 0645 */
void scan_spec_(group_code c, boolean three_codes)
{
  integer s;
  char spec_code;

  if (three_codes)
    s = saved(0);

  if (scan_keyword("to"))
    spec_code = exactly;
  else if (scan_keyword("spread"))
    spec_code = additional;
  else
  {
    spec_code = additional;
    cur_val = 0;
    goto found;
  }

  scan_dimen(false, false, false);

found:
  if (three_codes)
  {
    saved(0) = s;
    incr(save_ptr);
  }

  saved(0) = spec_code;
  saved(1) = cur_val;
  save_ptr = save_ptr + 2;
  new_save_level(c);
  scan_left_brace();
}
/* sec 0649 */
halfword hpack_(halfword p, scaled w, small_number m)
{
  halfword r;
  halfword q;
  scaled h, d, x;
  scaled s;
  halfword g;
/*  glue_ord o;  */
  int o;              /* 95/Jan/7 */
  internal_font_number f;
  four_quarters i;
  eight_bits hd;

  last_badness = 0;
  r = get_node(box_node_size);
  type(r) = hlist_node;
  subtype(r) = 0;
  shift_amount(r) = 0;
  q = r + list_offset;
  link(q) = p;
  h = 0;
  d = 0;
  x = 0;
  total_stretch[normal] = 0;
  total_shrink[normal] = 0;
  total_stretch[fil] = 0;
  total_shrink[fil] = 0;
  total_stretch[fill] = 0;
  total_shrink[fill] = 0;
  total_stretch[filll] = 0;
  total_shrink[filll] = 0;

  while (p != 0)
  {
reswitch:
    while (is_char_node(p))
    {
      f = font(p);
      i = char_info(f, character(p));
      hd = height_depth(i);
      x = x + char_width(f, i);
      s = char_height(f, hd);

      if (s > h)
        h = s;

      s = char_depth(f, hd);

      if (s > d)
        d = s;

      p = link(p);
    }

    if (p != 0)
    {
      switch (type(p))
      {
        case hlist_node:
        case vlist_node:
        case rule_node:
        case unset_node:
          {
            x = x + width(p);

            if (type(p) >= rule_node)
              s = 0;
            else
              s = shift_amount(p);

            if (height(p) - s > h)
              h = height(p) - s;

            if (depth(p) + s > d)
              d = depth(p) + s;
          }
          break;

        case ins_node:
        case mark_node:
        case adjust_node:
          if (adjust_tail != 0)
          {
            while (link(q) != p)
              q = link(q);

            if (type(p) == adjust_node)
            {
              link(adjust_tail) = adjust_ptr(p);

              while (link(adjust_tail) != 0)
                adjust_tail = link(adjust_tail);

              p = link(p);
              free_node(link(q), small_node_size);
            }
            else
            {
              link(adjust_tail) = p;
              adjust_tail = p;
              p = link(p);
            }

            link(q) = p;
            p = q;
          }
          break;

        case whatsit_node:
          break;

        case glue_node:
          {
            g = glue_ptr(p);
            x = x + width(g);
            o = stretch_order(g);
            total_stretch[o] = total_stretch[o] + stretch(g);
            o = shrink_order(g);
            total_shrink[o] = total_shrink[o] + shrink(g);

            if (subtype(p) >= a_leaders)
            {
              g = leader_ptr(p);

              if (height(g) > h)
                h = height(g);

              if (depth(g) > d)
                d = depth(g);
            }
          }
          break;

        case kern_node:
        case math_node:
          x = x + width(p);
          break;

        case ligature_node:
          {
            mem[lig_trick] = mem[lig_char(p)];
            link(lig_trick) = link(p);
            p = lig_trick;
            goto reswitch;
          }
          break;

        default:
          break;
      }
      p = link(p);
    }
  }

  if (adjust_tail != 0)
    link(adjust_tail) = 0;

  height(r) = h;
  depth(r) = d;

  if (m == additional)
    w = x + w;

  width(r) = w;
  x = w - x;

  if (x == 0)
  {
    glue_sign(r) = normal;
    glue_order(r) = normal;
    glue_set(r) = 0.0;
    goto exit;
  }
  else if (x > 0)
  {
    if (total_stretch[filll] != 0)
      o = filll;
    else if (total_stretch[fill] != 0)
      o = fill;
    else if (total_stretch[fil] != 0)
      o = fil;
    else
      o = normal;

    glue_order(r) = o;
    glue_sign(r) = stretching;

    if (total_stretch[o] != 0)
      glue_set(r) = x / ((double) total_stretch[o]);
    else
    {
      glue_sign(r) = normal;
      glue_set(r) = 0.0;
    }

    if (o == normal)
      if (list_ptr(r) != 0)
      {
        last_badness = badness(x, total_stretch[normal]);

        if (last_badness > hbadness)
        {
          print_ln();

          if (last_badness > 100)
            print_nl("Underfull");
          else
            print_nl("Loose");

          prints(" \\hbox (badness ");
          print_int(last_badness);

          if (last_badness > 100) /* Y&Y TeX */
            underfull_hbox++;

          goto common_ending;
        }
      }
      goto exit;
  }
  else
  {
    if (total_shrink[filll] != 0)
      o = filll;
    else if (total_shrink[fill] != 0)
      o = fill;
    else if (total_shrink[fil] != 0)
      o = fil;
    else
      o = normal;

    glue_order(r) = o;
    glue_sign(r) = shrinking;

    if (total_shrink[o] != 0)
      glue_set(r) = ((- (integer) x) / ((double) total_shrink[o]));
    else
    {
      glue_sign(r) = normal;
      glue_set(r) = 0.0;
    }

    if ((total_shrink[o] < - (integer) x) && (o == 0) && (list_ptr(r) != 0))
    {
      last_badness = 1000000L;
      glue_set(r) = 1.0;

      if ((- (integer) x - total_shrink[normal] > hfuzz) || (hbadness < 100))
      {
          if ((overfull_rule > 0) && (- (integer) x - total_shrink[0] > hfuzz))
          {
              while (link(q) != 0)
                q = link(q);
 
              link(q) = new_rule();
              width(link(q)) = overfull_rule;
          }

          print_ln();
          print_nl("Overfull \\hbox (");
          print_scaled(- (integer) x - total_shrink[normal]);
          prints("pt too wide");

          overfull_hbox++;

          goto common_ending;
      }
    }
    else if (o == normal)
      if (list_ptr(r) != 0)
      {
        last_badness = badness(- (integer) x, total_shrink[normal]);

        if (last_badness > hbadness)
        {
          print_ln();
          print_nl("Tight \\hbox (badness ");
          print_int(last_badness);
          goto common_ending;
        }
      }
      goto exit;
  }

common_ending:
  if (output_active)
    prints(") has occurred while \\output is active");
  else
  {
    if (pack_begin_line != 0)
    {
      if (pack_begin_line > 0)
        prints(") in paragraph at lines ");
      else
        prints(") in alignment at lines ");

      print_int(abs(pack_begin_line));
      prints("--");
    }
    else
      prints(") detected at line ");

    print_int(line);
  }

  print_ln();
  font_in_short_display = null_font;
  short_display(list_ptr(r));
  print_ln();
  begin_diagnostic();
  show_box(r);
  end_diagnostic(true);

exit:
  return r;
}
/* sec 0668 */
halfword vpackage_(halfword p, scaled h, small_number m, scaled l)
{
  halfword r;
  scaled w, d, x;
  scaled s;
  halfword g;
/*  glue_ord o;  */
  int o;              /* 95/Jan/7 */

  last_badness = 0;
  r = get_node(box_node_size);
  type(r) = vlist_node;
  subtype(r) = min_quarterword;
  shift_amount(r) = 0;
  list_ptr(r) = p;
  w = 0;
  d = 0;
  x = 0;
  total_stretch[normal] = 0;
  total_shrink[normal] = 0;
  total_stretch[fil] = 0;
  total_shrink[fil] = 0;
  total_stretch[fill] = 0;
  total_shrink[fill] = 0;
  total_stretch[filll] = 0;
  total_shrink[filll] = 0;

  while (p != 0)
  {
    if (is_char_node(p))
    {
      confusion("vpack");
      return 0;
    }
    else switch (type(p))
    {
      case hlist_node:
      case vlist_node:
      case rule_node:
      case unset_node:
        {
          x = x + d + height(p);
          d = depth(p);

          if (type(p) >= rule_node)
            s = 0;
          else
            s = shift_amount(p);

          if (width(p) + s > w)
            w = width(p) + s;
        }
        break;

      case whatsit_node:
        break;

      case glue_node:
        {
          x = x + d;
          d = 0;
          g = glue_ptr(p);
          x = x + width(g);
          o = stretch_order(g);
          total_stretch[o] = total_stretch[o] + stretch(g);
          o = shrink_order(g);
          total_shrink[o] = total_shrink[o] + shrink(g);

          if (subtype(p) >= a_leaders)
          {
            g = leader_ptr(p);

            if (width(g) > w)
              w = width(g);
          }
        }
        break;

      case kern_node:
        {
          x = x + d + width(p);
          d = 0;
        }
        break;

      default:
        break;
    }
    p = link(p);
  }

  width(r) = w;

  if (d > l)
  {
    x = x + d - l;
    depth(r) = l;
  }
  else
    depth(r) = d;

  if (m == additional)
    h = x + h;

  height(r) = h;
  x = h - x;

  if (x == 0)
  {
    glue_sign(r) = normal;
    glue_order(r) = normal;
    glue_set(r) = 0.0;
    goto exit;
  }
  else if (x > 0)
  {
    if (total_stretch[filll] != 0)
      o = filll;
    else if (total_stretch[fill] != 0)
      o = fill;
    else if (total_stretch[fil] != 0)
      o = fil;
    else
      o = normal;

    glue_order(r) = o;
    glue_sign(r) = stretching;

    if (total_stretch[o] != 0)
      glue_set(r) = x / ((double) total_stretch[o]);
    else
    {
      glue_sign(r) = normal;
      glue_set(r) = 0.0;
    }

    if (o == normal)
      if (list_ptr(r) != 0)
      {
        last_badness = badness(x, total_stretch[normal]);

        if (last_badness > vbadness)
        {
          print_ln();

          if (last_badness > 100)
            print_nl("Underfull");
          else
            print_nl("Loose");

          prints(" \\vbox (badness ");
          print_int(last_badness);

          if (last_badness > 100)
            underfull_vbox++; /* 1996/Feb/9 */

          goto common_ending;
        }
      }
      goto exit;
  }
  else
  {
    if (total_shrink[filll] != 0)
      o = filll;
    else if (total_shrink[fill] != 0)
      o = fill;
    else if (total_shrink[fil] != 0)
      o = fil;
    else
      o = normal;

    glue_order(r) = o;
    glue_sign(r) = shrinking;

    if (total_shrink[o] != 0)
      glue_set(r) =(- (integer) x)/ ((double) total_shrink[o]);
    else
    {
      glue_sign(r) = normal;
      glue_set(r) = 0.0;
    }

    if ((total_shrink[o] < - (integer) x) && (o == 0) && (list_ptr(r) != 0))
    {
      last_badness = 1000000L;
      glue_set(r) = 1.0;

      if ((- (integer) x - total_shrink[0] > vfuzz) || (vbadness < 100))
      {
        print_ln();
        print_nl("Overfull \\vbox (");
        print_scaled(- (integer) x - total_shrink[0]);
        prints("pt too high");

        overfull_vbox++;    /* 1996/Feb/9 */

        goto common_ending;
      }
    }
    else if (o == 0)
      if (list_ptr(r) != 0)
      {
        last_badness = badness(- (integer) x, total_shrink[normal]);
        if (last_badness > vbadness)
        {
          print_ln();
          print_nl("Tight \\vbox (badness ");
          print_int(last_badness);
          goto common_ending;
        }
      }
    goto exit;
  }

common_ending:
  if (output_active)
    prints(") has occurred while \\output is active");
  else
  {
    if (pack_begin_line != 0)
    {
      prints(") in alignment at lines ");
      print_int(abs(pack_begin_line));
      prints("--");
    }
    else
      prints(") detected at line ");

    print_int(line);
    print_ln();
  }

  begin_diagnostic();
  show_box(r);
  end_diagnostic(true);

exit:
  return r;
}
/* sec 0679 */
void append_to_vlist_(halfword b)
{
  scaled d;
  halfword p;

  if (prev_depth > ignore_depth)
  {
    d = width(baseline_skip) - prev_depth - height(b);

    if (d < line_skip_limit)
      p = new_param_glue(line_skip_code);
    else
    {
      p = new_skip_param(baseline_skip_code);
      width(temp_ptr) = d;
    }

    link(tail) = p;
    tail = p;
  }

  link(tail) = b;
  tail = b;
  prev_depth = depth(b);
}
/* sec 0686 */
halfword new_noad (void)
{
  halfword p;

  p = get_node(noad_size);
  type(p) = ord_noad;
  subtype(p) = normal;
  mem[nucleus(p)].hh = empty_field;
  mem[subscr(p)].hh = empty_field;
  mem[supscr(p)].hh = empty_field;

  return p;
}
/* sec 0688 */
halfword new_style_(small_number s)
{
  halfword p;

  p = get_node(style_node_size);
  type(p) = style_node;
  subtype(p) = s;
  width(p) = 0;
  depth(p) = 0;

  return p;
}
/* sec 0689 */
halfword new_choice (void)
{
  halfword p;

  p = get_node(style_node_size);
  type(p) = choice_node;
  subtype(p) = 0;
  display_mlist(p) = 0;
  text_mlist(p) = 0;
  script_mlist(p) = 0;
  script_script_mlist(p) = 0;

  return p;
}
/* sec 0693 */
void show_info (void)
{
  show_node_list(info(temp_ptr));
}
/* sec 0704 */
halfword fraction_rule_(scaled t)
{
  halfword p;

  p = new_rule();
  height(p) = t;
  depth(p) = 0;

  return p;
}
/* sec 0705 */
halfword overbar_(halfword b, scaled k, scaled t)
{
  halfword p, q;

  p = new_kern(k);
  link(p) = b;
  q = fraction_rule(t);
  link(q) = p;
  p = new_kern(t);
  link(p) = q;
  return vpackage(p, 0, 1, 1073741823L); /* 2^30 - 1 */
}
/* sec 0709 */
halfword char_box_(internal_font_number f, quarterword c)
{
  four_quarters q;
  eight_bits hd;
  halfword b, p;

  q = char_info(f, c);
  hd = height_depth(q);
  b = new_null_box();
  width(b) = char_width(f, q) + char_italic(f, q);
  height(b) = char_height(f, hd);
  depth(b) = char_depth(f, hd);
  p = get_avail();
  character(p) = c;
  font(p) = f;
  list_ptr(b) = p;

  return b;
}
/* sec 0711 */
void stack_into_box_(halfword b, internal_font_number f, quarterword c)
{
  halfword p;

  p = char_box(f, c);
  link(p) = list_ptr(b);
  list_ptr(b) = p;
  height(b) = height(p);
}
/* sec 0712 */
scaled height_plus_depth_(internal_font_number f, quarterword c)
{
  four_quarters q;
  eight_bits hd;

  q = char_info(f, c);
  hd = height_depth(q);
  return char_height(f, hd) + char_depth(f, hd);
}
/* sec 0706 */
halfword var_delimiter_(halfword d, small_number s, scaled v)
{
  halfword b;
  internal_font_number f, g;
  quarterword c, x, y;
  integer m, n;
  scaled u;
  scaled w;
  four_quarters q;
  four_quarters r;
  eight_bits hd;
/*  small_number z;  */
  int z;                  /* 95/Jan/7 */
/*  boolean large_attempt;  */
  int large_attempt;           /* 95/Jan/7 */

  f = null_font;
  w = 0;
  large_attempt = false;
  z = small_fam(d);
  x = small_char(d);

  while (true)
  {
    if ((z != 0) || (x != 0))
    {
      z = z + s + 16;

      do
        {
          z = z - 16;
          g = fam_fnt(z);

          if (g != null_font)
          {
            y = x;

            if ((y >= font_bc[g]) && (y <= font_ec[g]))
            {
continu:
              q = char_info(g, y);
              
              if ((q.b0 > 0))
              {
                if (char_tag(q) == ext_tag)
                {
                  f = g;
                  c = y;
                  goto found;
                }

                hd = height_depth(q);
                u = char_height(g, hd) + char_depth(g, hd);

                if (u > w)
                {
                  f = g;
                  c = y;
                  w = u;

                  if (u >= v)
                    goto found;
                }

                if (char_tag(q) == list_tag)
                {
                  y = rem_byte(q);
                  goto continu;
                }
              }
            }
          }
        }
      while (!(z < 16));
    }

    if (large_attempt)
      goto found;

    large_attempt = true;
    z = large_fam(d);
    x = large_char(d);
  }

found:
  if (f != null_font)
    if (char_tag(q) == ext_tag)
    {
      b = new_null_box();
      type(b) = vlist_node;
      r = font_info[exten_base[f] + rem_byte(q)].qqqq;
      c = ext_rep(r);
      u = height_plus_depth(f, c);
      w = 0;
      q = char_info(f, c);
      width(b) = char_width(f, q) + char_italic(f, q);
      c = ext_bot(r);

      if (c != min_quarterword)
        w = w + height_plus_depth(f, c);

      c = ext_mid(r);

      if (c != min_quarterword)
        w = w + height_plus_depth(f, c);

      c = ext_top(r);

      if (c != min_quarterword)
        w = w + height_plus_depth(f, c);

      n = 0;

      if (u > 0)
        while (w < v)
        {
          w = w + u;
          incr(n);

          if (ext_mid(r) != min_quarterword)
            w = w + u;
        }

      c = ext_bot(r);

      if (c != min_quarterword)
        stack_into_box(b, f, c);

      c = ext_rep(r);

      for (m = 1; m <= n; m++)
        stack_into_box(b, f, c);

      c = ext_mid(r);

      if (c != min_quarterword)
      {
        stack_into_box(b, f, c);
        c = ext_rep(r);

        for (m = 1; m <= n; m++)
          stack_into_box(b, f, c);
      }

      c = ext_top(r);

      if (c != 0)
        stack_into_box(b, f, c);
      
      depth(b) = w - height(b);
    }
    else
      b = char_box(f, c);
  else
  {
    b = new_null_box();
    width(b) = null_delimiter_space;
  }

  shift_amount(b) = half(height(b) - depth(b)) - axis_height(s);

  return b;
}
/* rebox_ etc used to follow here in tex4.c */