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

/* rebox_ used to be in tex4.c */
/* sec 0715 */
halfword rebox_(halfword b, scaled w)
{
  halfword p;
  internal_font_number f;
  scaled v;

  if ((width(b) != w) && (list_ptr(b) != 0))
  {
    if (type(b) == vlist_node)
      b = hpack(b, 0, 1);

    p = list_ptr(b);

    if (((p >= hi_mem_min)) && (link(p) == 0))
    {
      f = font(p);
      v = char_width(f, char_info(f, character(p)));

      if (v != width(b))
        link(p) = new_kern(width(b) - v);
    }

    free_node(b, box_node_size);
    b = new_glue(ss_glue);
    link(b) = p;

    while (link(p) != 0)
      p = link(p);

    link(p) = new_glue(ss_glue);
    return hpack(b, w, exactly);
  }
  else
  {
    width(b) = w;
    return b;
  }
}
/* This is to be the start of tex5.c */
/* sec 0716 */
halfword math_glue_(halfword g, scaled m)
{
  halfword p;
  integer n;
  scaled f;

  n = x_over_n(m, 65536L);
  f = tex_remainder;

  if (f < 0)
  {
    decr(n);
    f = f + 65536L;
  }

  p = get_node(glue_spec_size);
  width(p) = mult_and_add(n, width(g), xn_over_d(width(g), f, 65536L), 1073741823L);  /* 2^30 - 1 */
  stretch_order(p) = stretch_order(g);

  if (stretch_order(p) == normal)
    stretch(p) = mult_and_add(n, stretch(g), xn_over_d(stretch(g), f, 65536L), 1073741823L);  /* 2^30 - 1 */
  else
    stretch(p) = stretch(g);

  shrink_order(p) = shrink_order(g);

  if (shrink_order(p) == normal)
    shrink(p) = mult_and_add(n, shrink(g), xn_over_d(shrink(g), f, 65536L), 1073741823L);  /* 2^30 - 1 */
  else
    shrink(p) = shrink(g);

  return p;
}
/* sec 0717 */
void math_kern_ (halfword p, scaled m)
{
  integer n;
  scaled f;

  if (subtype(p) == mu_glue)
  {
    n = x_over_n(m, 65536L);
    f = tex_remainder;

    if (f < 0)
    {
      decr(n);
      f = f + 65536L;
    }

    width(p) = mult_and_add(n, width(p), xn_over_d(width(p), f, 65536L), 1073741823L);  /* 2^30 - 1 */
    subtype(p) = explicit;
  }
}
/* sec 0718 */
void flush_math (void)
{
  flush_node_list(link(head));
  flush_node_list(incompleat_noad);
  link(head) = 0;
  tail = head;
  incompleat_noad = 0;
}
/* sec 0720 */
halfword clean_box_(halfword p, small_number s)
{
  halfword q;
  small_number save_style;
  halfword x;
  halfword r;

  switch(math_type(p))
  {
    case math_char:
      {
        cur_mlist = new_noad();
        mem[nucleus(cur_mlist)] = mem[p];
      }
      break;

    case sub_box:
      {
        q = info(p);
        goto lab40;
      }
      break;

    case sub_mlist:
      cur_mlist = info(p);
      break;

    default:
      {
        q = new_null_box();
        goto lab40;
      }
    break;
  }

  save_style = cur_style;
  cur_style = s;
  mlist_penalties = false;
  mlist_to_hlist();
  q = link(temp_head);
  cur_style = save_style;

  {
    if (cur_style < script_style)
      cur_size = text_size;
    else
      cur_size = 16 * ((cur_style - text_style) / 2);

    cur_mu = x_over_n(math_quad(cur_size), 18);
  }
lab40:
  if ((q >= hi_mem_min) || (q == 0))
    x = hpack(q, 0, 1);
  else if ((link(q) == 0) && (type(q) <= vlist_node) && (shift_amount(q) == 0))
    x = q;
  else
    x = hpack(q, 0, 1);

  q = list_ptr(x);

  if ((q >= hi_mem_min))
  {
    r = link(q);

    if (r != 0)
      if (link(r) == 0)
        if (!(r >= hi_mem_min))
          if (type(r) == kern_node)
          {
            free_node(r, small_node_size);
            link(q) = 0;
          }
  }

  return x;
}
/* sec 0722 */
void fetch_(halfword a)
{
  cur_c = character(a);
  cur_f = fam_fnt(fam(a) + cur_size);

  if (cur_f == null_font)
  {
    print_err("");
    print_size(cur_size);
    print_char(' ');
    print_int(fam(a));
    print_string(" is undefined (character ");
    print(cur_c);
    print_char(')');
    help4("Somewhere in the math formula just ended, you used the",
        "stated character from an undefined font family. For example,",
        "plain TeX doesn't allow \\it or \\sl in subscripts. Proceed,",
        "and I'll try to forget that I needed that character.");
    error();
    cur_i = null_character;
    math_type(a) = 0;
  }
  else
  {
    if ((cur_c >= font_bc[cur_f]) && (cur_c <= font_ec[cur_f]))
      cur_i = char_info(cur_f, cur_c);
    else
      cur_i = null_character;

    if (!((cur_i.b0 > 0)))
    {
      char_warning(cur_f, cur_c);
      math_type(a) = 0;
    }
  }
}
/* sec 0734 */
void make_over_(halfword q)
{
  info(nucleus(q)) = overbar(clean_box(nucleus(q), 2 * (cur_style / 2) + 1),
      3 * default_rule_thickness, default_rule_thickness);
  math_type(nucleus(q)) = sub_box;
}
/* sec 0735 */
void make_under_(halfword q)
{
  halfword p, x, y;
  scaled delta;

  x = clean_box(nucleus(q), cur_style);
  p = new_kern(3 * default_rule_thickness);
  link(x) = p;
  link(p) = fraction_rule(default_rule_thickness);
  y = vpackage(x, 0, 1, 1073741823L); /* 2^30 - 1 */
  delta = height(y) + depth(y) + default_rule_thickness;
  height(y) = height(x);
  depth(y) = delta - height(y);
  info(nucleus(q)) = y;
  math_type(nucleus(q)) = sub_box;
}
/* sec 0736 */
void make_vcenter_(halfword q)
{ 
  halfword v;
  scaled delta;

  v = info(nucleus(q));

  if (type(v) != vlist_node)
  {
    confusion("vcenter");
    return;         // abort_flag set
  }
  delta = height(v) + depth(v);
  height(v) = axis_height(cur_size) + half(delta);
  depth(v) = delta - height(v);
}
/* sec 0737 */
void make_radical_(halfword q)
{
  halfword x, y;
  scaled delta, clr;

  x = clean_box(nucleus(q), 2 * (cur_style / 2) + 1);

  if (cur_style < text_style)
    clr = default_rule_thickness + (abs(math_x_height(cur_size)) / 4);
  else
  {
    clr = default_rule_thickness;
    clr = clr + (abs(clr) / 4);
  }

  y = var_delimiter(left_delimiter(q), cur_size, height(x) + depth(x) + clr + default_rule_thickness);
  delta = depth(y) -(height(x) + depth(x) + clr);

  if (delta > 0)
    clr = clr + half(delta);

  shift_amount(y) = - (integer) (height(x) + clr);
  link(y) = overbar(x, clr, height(y));
  info(nucleus(q)) = hpack(y, 0, 1);
  math_type(nucleus(q)) = sub_box;
}
/* sec 0738 */
void make_math_accent_(halfword q)
{
  halfword p, x, y;
  integer a;
  quarterword c;
  internal_font_number f;
  ffourquarters i;
  scaled s;
  scaled h;
  scaled delta;
  scaled w;

  fetch(accent_chr(q));

  if ((cur_i.b0 > 0))
  {
    i = cur_i;
    c = cur_c;
    f = cur_f;
    s = 0;

    if (math_type(nucleus(q)) == math_char)
    {
      fetch(nucleus(q));

      if (char_tag(cur_i) == lig_tag)
      {
        a = lig_kern_start(cur_f, cur_i);
        cur_i = font_info[a].qqqq;

        if (skip_byte(cur_i) > stop_flag)
        {
          a = lig_kern_restart(cur_f, cur_i);
          cur_i = font_info[a].qqqq;
        }

        while (true)
        {
          if (next_char(cur_i) == skew_char[cur_f])
          {
            if (op_byte(cur_i) >= kern_flag)
              if (skip_byte(cur_i) <= stop_flag)
                s = char_kern(cur_f, cur_i);
            goto lab31;
          }

          if (skip_byte(cur_i) >= stop_flag)
            goto lab31;

          a = a + skip_byte(cur_i) + 1;
          cur_i = font_info[a].qqqq;
        }
      }
    }
lab31:;
    x = clean_box(nucleus(q), cramped_style(cur_style));
    w = width(x);
    h = height(x);

    while (true)
    {
      if (char_tag(i) != list_tag)
        goto lab30;

      y = rem_byte(i);
      i = char_info(f, y);

      if (!(i.b0 > 0))
        goto lab30;

      if (char_width(f, i) > w)
        goto lab30;

      c = y;
    }
lab30:;
    if (h < x_height(f))
      delta = h;
    else
      delta = x_height(f);

    if ((math_type(supscr(q)) != 0) || (math_type(subscr(q)) != 0))
      if (math_type(nucleus(q)) == math_char)
      {
        flush_node_list(x);
        x = new_noad();
        mem[nucleus(x)]= mem[nucleus(q)];
        mem[supscr(x)]= mem[supscr(q)];
        mem[subscr(x)]= mem[subscr(q)];
        mem[supscr(q)].hh = empty_field;
        mem[subscr(q)].hh = empty_field;
        math_type(nucleus(q)) = sub_mlist;
        info(nucleus(q)) = x;
        x = clean_box(nucleus(q), cur_style);
        delta = delta + height(x) - h;
        h = height(x);
      }

    y = char_box(f, c);
    shift_amount(y) = s + half(w - width(y));
    width(y) = 0;
    p = new_kern(- (integer) delta);
    link(p) = x;
    link(y) = p;
    y = vpackage(y, 0, 1, 1073741823L);  /* 2^30 - 1 */
    width(y) = width(x);

    if (height(y) < h)
    {
      p = new_kern(h - height(y));
      link(p) = list_ptr(y);
      list_ptr(y) = p;
      height(y) = h;
    }

    info(nucleus(q)) = y;
    math_type(nucleus(q)) = sub_box;
  }
}
/* sec 0743 */
void make_fraction_(halfword q)
{
  halfword p, v, x, y, z;
  scaled delta, delta1, delta2, shift_up, shift_down, clr;
  
  if (thickness(q) == default_code) /* 2^30 */
    thickness(q) = default_rule_thickness;

  x = clean_box(numerator(q), num_style(cur_style));
  z = clean_box(denominator(q), denom_style(cur_style));

  if (width(x) < width(z))
    x = rebox(x, width(z));
  else
    z = rebox(z, width(x));

  if (cur_style < text_style)
  {
    shift_up = num1(cur_size);
    shift_down = denom1(cur_size);
  }
  else
  {
    shift_down = denom2(cur_size);

    if (thickness(q) != 0)
      shift_up = num2(cur_size);
    else
      shift_up = num3(cur_size);
  }

  if (thickness(q) == 0)
  {
    if (cur_style < text_style)
      clr = 7 * default_rule_thickness;
    else
      clr = 3 * default_rule_thickness;

    delta = half(clr - ((shift_up - depth(x)) - (height(z) - shift_down)));

    if (delta > 0)
    {
      shift_up = shift_up + delta;
      shift_down = shift_down + delta;
    }
  }
  else
  {
    if (cur_style < text_style)
      clr = 3 * thickness(q);
    else
      clr = thickness(q);

    delta = half(thickness(q));
    delta1 = clr - ((shift_up - depth(x)) - (axis_height(cur_size) + delta));
    delta2 = clr -((axis_height(cur_size) - delta) - (height(z) - shift_down));

    if (delta1 > 0)
      shift_up = shift_up + delta1;

    if (delta2 > 0)
      shift_down = shift_down + delta2;
  }

  v = new_null_box();
  type(v) = vlist_node;
  height(v) = shift_up + height(x);
  depth(v) = depth(z) + shift_down;
  width(v) = width(x);

  if (thickness(q) == 0)
  {
    p = new_kern((shift_up - depth(x)) - (height(z) - shift_down));
    link(p) = z;
  }
  else
  {
    y = fraction_rule(thickness(q));
    p = new_kern((axis_height(cur_size) - delta) - (height(z) - shift_down));
    link(y) = p;
    link(p) = z;
    p = new_kern((shift_up - depth(x)) - (axis_height(cur_size) + delta));
    link(p) = y;
  }

  link(x) = p;
  list_ptr(v) = x;

  if (cur_style < text_style)
    delta = delim1(cur_size);
  else
    delta = delim2(cur_size);

  x = var_delimiter(left_delimiter(q), cur_size, delta);
  link(x) = v;
  z = var_delimiter(right_delimiter(q), cur_size, delta);
  link(v) = z;
  new_hlist(q) = hpack(x, 0, 1);
}
/***************************************************************************/
scaled make_op_ (halfword);
/***************************************************************************/
/* sec 0752 */
void make_ord_(halfword q)
{
  integer a;
  halfword p, r;

lab20:
  if (math_type(subscr(q)) == 0)
    if (math_type(supscr(q)) == 0)
      if (math_type(nucleus(q)) == math_char)
      {
        p = link(q);

        if (p != 0)
          if ((type(p) >= ord_noad) && (type(p) <= punct_noad))
            if (math_type(nucleus(p)) == math_char)
              if (fam(nucleus(p)) == fam(nucleus(q)))
              {
                math_type(nucleus(q)) = math_text_char;
                fetch(nucleus(q));

                if (char_tag(cur_i) == lig_tag)
                {
                  a = lig_kern_start(cur_f, cur_i);
                  cur_c = character(nucleus(p));
                  cur_i = font_info[a].qqqq;

                  if (skip_byte(cur_i) > stop_flag)
                  {
                    a = lig_kern_restart(cur_f, cur_i);
                    cur_i = font_info[a].qqqq;
                  }

                  while (true)
                  {
                    if (next_char(cur_i) == cur_c)
                      if (skip_byte(cur_i) <= stop_flag)
                        if (op_byte(cur_i) >= kern_flag)
                        {
                          p = new_kern(char_kern(cur_f, cur_i));
                          link(p) = link(q);
                          link(q) = p;
                          return;
                        }
                        else
                        {
                          {
                            if (interrupt != 0)
                            {
                              pause_for_instructions();
                            }
                          }

                          switch (op_byte(cur_i))
                          {
                            case 1:
                            case 5:
                              character(nucleus(q)) = rem_byte(cur_i);
                              break;
                            case 2:
                            case 6:
                              character(nucleus(p)) = rem_byte(cur_i);
                              break;
                            case 3:
                            case 7:
                            case 11:
                              {
                                r = new_noad();
                                character(nucleus(r)) = rem_byte(cur_i);
                                fam(nucleus(r)) = fam(nucleus(q));
                                link(q) = r;
                                link(r) = p;

                                if (op_byte(cur_i) < 11)
                                  math_type(nucleus(r)) = math_char;
                                else
                                  math_type(nucleus(r)) = math_text_char;
                              }
                              break;

                            default:
                              {
                                link(q) = link(p);
                                character(nucleus(q)) = rem_byte(cur_i);
                                mem[subscr(q)] = mem[subscr(p)];
                                mem[supscr(q)] = mem[supscr(p)];
                                free_node(p, noad_size);
                              }
                              break;
                          }

                          if (op_byte(cur_i) > 3)
                            return;

                          math_type(nucleus(q)) = math_char;
                          goto lab20;
                        }

                    if (skip_byte(cur_i) >= stop_flag)
                      return;

                    a = a + skip_byte(cur_i) + 1;
                    cur_i = font_info[a].qqqq;
                  }
                }
              }
      }
}
/***************************************************************************/
void make_scripts_ (halfword, scaled);
/***************************************************************************/
/* sec 0762 */
small_number make_left_right_(halfword q, small_number style, scaled max_d, scaled max_h)
{
  scaled delta, delta1, delta2;

  if (style < script_style)
    cur_size = text_size;
  else
    cur_size = 16 * ((style - text_style) / 2);

  delta2 = max_d + axis_height(cur_size);
  delta1 = max_h + max_d - delta2;

  if (delta2 > delta1)
    delta1 = delta2;

  delta = (delta1 / 500) * delimiter_factor;
  delta2 = delta1 + delta1 - delimiter_shortfall;

  if (delta < delta2)
    delta = delta2;

  new_hlist(q) = var_delimiter(delimiter(q), cur_size, delta);
  return type(q) - (left_noad - open_noad);
}
/* sec 0726 */
void mlist_to_hlist (void)
{
  halfword mlist;
  bool penalties;
  small_number style;
  small_number save_style;
  halfword q;
  halfword r;
/*  small_number r_type;  */
  int r_type;            /* 95/Jan/7 */
/*  small_number t; */
  int t;              /* 95/Jan/7 */
  halfword p, x, y, z;
  integer pen;
  small_number s;
  scaled max_h, max_d;
  scaled delta;

  mlist = cur_mlist;
  penalties = mlist_penalties;
  style = cur_style;
  q = mlist;
  r = 0;
  r_type = op_noad;
  max_h = 0;
  max_d = 0;

  {
    if (cur_style < script_style)
      cur_size = text_size;
    else
      cur_size = 16 * ((cur_style - text_style) / 2);

    cur_mu = x_over_n(math_quad(cur_size), 18);
  }

  while (q != 0)
  {
lab21:
    delta = 0;

    switch (type(q))
    {
      case bin_noad:
        switch (r_type)
        {
          case bin_noad:
          case op_noad:
          case rel_noad:
          case open_noad:
          case punct_noad:
          case left_noad:
            {
              type(q) = ord_noad;
              goto lab21;
            }
            break;

          default:
            ;
            break;
        }
        break;

      case rel_noad:
      case close_noad:
      case punct_noad:
      case right_noad:
        {
          if (r_type == bin_noad)
            type(r) = ord_noad;

          if (type(q) == right_noad)
            goto lab80;
        }
        break;

      case left_noad:
        goto lab80;
        break;

      case fraction_noad:
        {
          make_fraction(q);
          goto lab82;
        }
        break;

      case op_noad:
        {
          delta = make_op(q);

          if (subtype(q) == limits)
            goto lab82;
        }
        break;

      case ord_noad:
        make_ord(q);
        break;

      case open_noad:
      case inner_noad:
        ;
        break;

      case radical_noad:
        make_radical(q);
        break;

      case over_noad:
        make_over(q);
        break;

      case under_noad:
        make_under(q);
        break;

      case accent_noad:
        make_math_accent(q);
        break;

      case vcenter_noad:
        make_vcenter(q);
        break;

      case style_node:
        {
          cur_style = subtype(q);

          {
            if (cur_style < script_style)
              cur_size = text_size;
            else
              cur_size = 16 * ((cur_style - text_style) / 2);

            cur_mu = x_over_n(math_quad(cur_size), 18);
          }

          goto lab81;
        }
        break;

      case choice_node:
        {
          switch (cur_style / 2)
          {
            case 0:
              {
                p = display_mlist(q);
                display_mlist(q) = 0;
              }
              break;

            case 1:
              {
                p = text_mlist(q);
                text_mlist(q) = 0;
              }
              break;

            case 2:
              {
                p = script_mlist(q);
                script_mlist(q) = 0;
              }
              break;

            case 3:
              {
                p = script_script_mlist(q);
                script_script_mlist(q) = 0;
              }
              break;
          }

          flush_node_list(display_mlist(q));
          flush_node_list(text_mlist(q));
          flush_node_list(script_mlist(q));
          flush_node_list(script_script_mlist(q));
          type(q) = style_node;
          subtype(q) = cur_style;
          width(q) = 0;
          depth(q) = 0;

          if (p != 0)
          {
            z = link(q);
            link(q) = p;

            while (link(p) != 0)
              p = link(p);

            link(p) = z;
          }
          goto lab81;
        }
        break;

      case ins_node:
      case mark_node:
      case adjust_node:
      case whatsit_node:
      case penalty_node:
      case disc_node:
        goto lab81;
        break;

      case rule_node:
        {
          if (height(q) > max_h)
            max_h = height(q);

          if (depth(q) > max_d)
            max_d = depth(q);

          goto lab81;
        }
        break;

      case glue_node:
        {
          if (subtype(q) == mu_glue)
          {
            x = glue_ptr(q);
            y = math_glue(x, cur_mu);
            delete_glue_ref(x);
            glue_ptr(q) = y;
            subtype(q) = normal;
          }
          else if ((cur_size != text_size) && (subtype(q) == cond_math_glue))
          {
            p = link(q);

            if (p != 0)
              if ((type(q) == glue_node) || (type(p) == kern_node))
              {
                link(q) = link(p);
                link(p) = 0;
                flush_node_list(p);
              }
          }
          goto lab81;
        }
        break;

      case kern_node:
        {
          math_kern(q, cur_mu);
          goto lab81;
        }
        break;

      default:
        {
          confusion("mlist1");
          return;       // abort_flag set
        }
        break;
    }

    switch (math_type(nucleus(q)))
    {
      case math_char:
      case math_text_char:
        {
          fetch(nucleus(q));

          if ((cur_i.b0 > 0))
          {
            delta = char_italic(cur_f, cur_i);
            p = new_character(cur_f, cur_c);

            if ((math_type(nucleus(q)) == math_text_char) && (space(cur_f) != 0))
              delta = 0;

            if ((math_type(subscr(q)) == 0) && (delta != 0))
            {
              link(p) = new_kern(delta);
              delta = 0;
            }
          }
          else
            p = 0;
        }
        break;

      case 0:
        p = 0;
        break;

      case sub_box:
        p = info(nucleus(q));
        break;

      case sub_mlist:
        {
          cur_mlist = info(nucleus(q));
          save_style = cur_style;
          mlist_penalties = false;
          mlist_to_hlist();
          cur_style = save_style;

          {
            if (cur_style < script_style)
              cur_size = text_size;
            else
              cur_size = 16 * ((cur_style - text_style) / 2);

            cur_mu = x_over_n(math_quad(cur_size), 18);
          }

          p = hpack(link(temp_head), 0, 1);
        }
        break;

      default:
        {
          confusion("mlist2");
          return;       // abort_flag set
        }
        break;
    }
  
    new_hlist(q) = p;

    if ((math_type(subscr(q)) == 0) && (math_type(supscr(q)) == 0))
      goto lab82;

    make_scripts(q, delta);
lab82:
    z = hpack(new_hlist(q), 0, 1);

    if (height(z) > max_h)
      max_h = height(z);

    if (depth(z) > max_d)
      max_d = depth(z);

    free_node(z, box_node_size);
lab80:
    r = q;
    r_type = type(r);
lab81:
    q = link(q);
  }

  if (r_type == bin_noad)
    type(r) = ord_noad;

  p = temp_head;
  link(p) = 0;
  q = mlist;
  r_type = 0;
  cur_style = style;

  {
    if (cur_style < script_style)
      cur_size = text_size;
    else
      cur_size = 16 *((cur_style - text_style) / 2);

    cur_mu = x_over_n(math_quad(cur_size), 18);
  }

  while (q != 0)
  {
    t = ord_noad;
    s = noad_size;
    pen = inf_penalty;

    switch (type(q))
    {
      case op_noad:
      case open_noad:
      case close_noad:
      case punct_noad:
      case inner_noad:
        t = type(q);
        break;

      case bin_noad:
        {
          t = bin_noad;
          pen = bin_op_penalty;
        }
        break;

      case rel_noad:
        {
          t = rel_noad;
          pen = rel_penalty;
        }
        break;

      case ord_noad:
      case vcenter_noad:
      case over_noad:
      case under_noad:
        ;
        break;

      case radical_noad:
        s = radical_noad_size;
        break;

      case accent_noad:
        s = accent_noad_size;
        break;

      case fraction_noad:
        {
          t = inner_noad;
          s = fraction_noad_size;
        }
        break;

      case left_noad:
      case right_noad:
        t = make_left_right(q, style, max_d, max_h);
        break;

      case style_node:
        {
          cur_style = subtype(q);
          s = style_node_size;

          {
            if (cur_style < script_style)
              cur_size = text_size;
            else
              cur_size = 16 *((cur_style - text_style) / 2);

            cur_mu = x_over_n(math_quad(cur_size), 18);
          }

          goto lab83;
        }
        break;

      case whatsit_node:
      case penalty_node:
      case rule_node:
      case disc_node:
      case adjust_node:
      case ins_node:
      case mark_node:
      case glue_node:
      case kern_node:
        {
          link(p) = q;
          p = q;
          q = link(q);
          link(p) = 0;
          goto lab30;
        }
        break;

      default:
        {
          confusion("mlist3");
          return;       // abort_flag set
        }
        break;
    }

    if (r_type > 0)
    {
      switch (str_pool[r_type * 8 + t + magic_offset])
      {
        case '0':
          x = 0;
          break;

        case '1':
          if (cur_style < script_style)
            x = thin_mu_skip_code;
          else
            x = 0;
          break;

        case '2':
          x = thin_mu_skip_code;
          break;

        case '3':
          if (cur_style < script_style)
            x = med_mu_skip_code;
          else
            x = 0;
          break;

        case '4':
          if (cur_style < script_style)
            x = thick_mu_skip_code;
          else
            x = 0;
          break;

        default:
          {
            confusion("mlist4");
            return;       // abort_flag set
          }
          break;
      }

      if (x != 0)
      {
        y = math_glue(glue_par(x), cur_mu);
        z = new_glue(y);
        glue_ref_count(y) = 0;
        link(p) = z;
        p = z;
        subtype(z) = x + 1;
      }
    }

    if (new_hlist(q) != 0)
    {
      link(p) = new_hlist(q);

      do
        {
          p = link(p);
        }
      while (!(link(p) == 0));
    }

    if (penalties)
      if (link(q) != 0)
        if (pen < inf_penalty)
        {
          r_type = type(link(q));

          if (r_type != penalty_node)
            if (r_type != rel_noad)
            {
              z = new_penalty(pen);
              link(p) = z;
              p = z;
            }
        }

    r_type = t;
lab83:
    r = q;
    q = link(q);
    free_node(r, s);
lab30:;
  }
}
/* sec 0772 */
void push_alignment (void)
{
  halfword p;

  p = get_node(align_stack_node_size);
  link(p) = align_ptr;
  info(p) = cur_align;
  llink(p) = preamble;
  rlink(p) = cur_span;
  mem[p + 2].cint = cur_loop;
  mem[p + 3].cint = align_state;
  info(p + 4) = cur_head;
  link(p + 4) = cur_tail;
  align_ptr = p;
  cur_head = get_avail();
}
/* sec 0772 */
void pop_alignment (void)
{
  halfword p;

  free_avail(cur_head);
  p = align_ptr;
  cur_tail = link(p + 4);
  cur_head = info(p + 4);
  align_state = mem[p + 3].cint;
  cur_loop = mem[p + 2].cint;
  cur_span = rlink(p);
  preamble = llink(p);
  cur_align = info(p);
  align_ptr = link(p);
  free_node(p, align_stack_node_size);
}
/* sec 0782 */
void get_preamble_token (void)
{
lab20:
  get_token();

  while ((cur_chr == span_code) && (cur_cmd == tab_mark))
  {
    get_token();

    if (cur_cmd > max_command)
    {
      expand();
      get_token();
    }
  }

  if (cur_cmd == endv)
  {
    fatal_error("(interwoven alignment preambles are not allowed)");
    return;     // abort_flag set
  }

  if ((cur_cmd == assign_glue) && (cur_chr == glue_base + tab_skip_code))
  {
    scan_optional_equals();
    scan_glue(glue_val);

    if (global_defs > 0)
      geq_define(glue_base + tab_skip_code, glue_ref, cur_val);
    else
      eq_define(glue_base + tab_skip_code, glue_ref, cur_val);

    goto lab20;
  }
}
/* sec 0774 */
void init_align (void)
{
  halfword save_cs_ptr;
  halfword p;

  save_cs_ptr = cur_cs;
  push_alignment();
  align_state = -1000000L;

  if ((mode == mmode) && ((tail != cur_list.head_field) || (incompleat_noad != 0)))
  {
    print_err("Improper ");
    print_esc("halign");
    print_string(" inside $$'s");
    help3("Displays can use special alignments (like \\eqalignno)",
        "only if nothing but the alignment itself is between $$'s.",
        "So I've deleted the formulas that preceded this alignment.");
    error();
    flush_math();
  }

  push_nest();

  if (mode == mmode)
  {
    mode = -vmode;
    prev_depth = nest[nest_ptr - 2].aux_field.cint;
  }
  else if (mode > 0)
    mode = - (integer) mode;

  scan_spec(align_group, false);
  preamble = 0;
  cur_align = align_head;
  cur_loop = 0;
  scanner_status = aligning;
  warning_index = save_cs_ptr;
  align_state = -1000000L;

  while (true)
  {
    link(cur_align) = new_param_glue(tab_skip_code);
    cur_align = link(cur_align);

    if (cur_cmd == car_ret)
      goto lab30;

    p = hold_head;
    link(p) = 0;

    while (true)
    {
      get_preamble_token();

      if (cur_cmd == mac_param)
        goto lab31;

      if ((cur_cmd <= car_ret) && (cur_cmd >= tab_mark) && (align_state == -1000000L))
        if ((p == hold_head) && (cur_loop == 0) && (cur_cmd == tab_mark))
          cur_loop = cur_align;
        else
        {
          print_err("Missing # inserted in alignment preamble");
          help3("There should be exactly one # between &'s, when an",
              "\\halign or \\valign is being set up. In this case you had",
              "none, so I've put one in; maybe that will work.");
          back_error();
          goto lab31;
        }
      else if ((cur_cmd != spacer) || (p != hold_head))
      {
        link(p) = get_avail();
        p = link(p);
        info(p) = cur_tok;
      }
    }

lab31:
    link(cur_align) = new_null_box();
    cur_align = link(cur_align);
    info(cur_align) = end_span;
    width(cur_align) = null_flag;  /* - 2^30 */
    u_part(cur_align) = link(hold_head);
    p = hold_head;
    link(p) = 0;

    while (true)
    {
lab22:
      get_preamble_token();

      if ((cur_cmd <= car_ret) && (cur_cmd >= tab_mark) && (align_state == -1000000L))
        goto lab32;

      if (cur_cmd == mac_param)
      {
        print_err("Only one # is allowed per tab");
        help3("There should be exactly one # between &'s, when an",
            "\\halign or \\valign is being set up. In this case you had",
            "more than one, so I'm ignoring all but the first.");
        error();
        goto lab22;
      }

      link(p) = get_avail();
      p = link(p);
      info(p) = cur_tok;
    }

lab32:
    link(p) = get_avail();
    p = link(p);
    info(p) = end_template_token;
    v_part(cur_align) = link(hold_head);
  }

lab30:
  scanner_status = 0;
  new_save_level(align_group);

  if (every_cr != 0)
    begin_token_list(every_cr, every_cr_text);
  align_peek();
}
/* sec 0787 */
void init_span_ (halfword p)
{
  push_nest();

  if (mode == -hmode)
    space_factor = 1000;
  else
  {
    cur_list.aux_field.cint = ignore_depth;
    normal_paragraph();
  }

  cur_span = p;
}
/* sec 0786 */
void init_row (void)
{
  push_nest();

  mode = (-hmode - vmode) - mode;

  if (mode == -hmode)
    space_factor = 0;
  else
    cur_list.aux_field.cint = 0;

  tail_append(new_glue(glue_ptr(preamble)));
  subtype(tail) = tab_skip_code + 1;
  cur_align = link(preamble);
  cur_tail = cur_head;
  init_span(cur_align);
}
/* sec 0788 */
void init_col (void)
{
  extra_info(cur_align) = cur_cmd;

  if (cur_cmd == omit)
    align_state = 0;
  else
  {
    back_input();
    begin_token_list(u_part(cur_align), u_template);
  }
}
/* sec 0799 */
void fin_row (void)
{
  halfword p;

  if (mode == -hmode)
  {
    p = hpack(link(head), 0, 1);
    pop_nest();
    append_to_vlist(p);

    if (cur_head != cur_tail)
    {
      link(tail) = link(cur_head);
      tail = cur_tail;
    }
  }
  else
  {
    p = vpackage(link(head), 0, 1, 1073741823L);   /* 2^30 - 1 */
    pop_nest();
    link(tail) = p;
    tail = p;
    space_factor = 1000;
  }

  type(p) = unset_node;
  glue_stretch(p) = 0;

  if (every_cr != 0)
    begin_token_list(every_cr, every_cr_text);

  align_peek();
}
/* sec 0800 */
void fin_align (void)
{
  halfword p, q, r, s, u, v;
  scaled t, w;
  scaled o;
  halfword n;
  scaled rule_save;
  memory_word aux_save;

  if (cur_group != align_group)
  {
    confusion("align1");
    return;       // abort_flag set
  }

  unsave();

  if (cur_group != align_group)
  {
    confusion("align0");
    return;       // abort_flag set
  }

  unsave();

  if (nest[nest_ptr - 1].mode_field == mmode)
    o = display_indent;
  else
    o = 0;

  q = link(preamble);

  do
    {
      flush_list(u_part(q));
      flush_list(v_part(q));
      p = link(link(q));

      if (width(q) == null_flag) /* - 2^30 */
      {
        width(q) = 0;
        r = link(q);
        s = glue_ptr(r);

        if (s != zero_glue)
        {
          add_glue_ref(zero_glue);
          delete_glue_ref(s);
          glue_ptr(c) = zero_glue;
        }
      }

      if (info(q) != end_span)
      {
        t = width(q) + width(glue_ptr(link(q)));
        r = info(q);
        s = end_span;
        info(s) = p;
        n = min_quarterword + 1;

        do
          {
            width(r) = width(r) - t;
            u = info(r);

            while (link(r) > n)
            {
              s = info(s);
              n = link(info(s)) + 1;
            }

            if (link(r) < n)
            {
              info(r) = info(s);
              info(s) = r;
              decr(link(r));
              s = r;
            }
            else
            {
              if (width(r) > width(info(s)))
                width(info(s)) = width(r);

              free_node(r, span_node_size);
            }

            r = u;
          }
        while (!(r == end_span));
      }

      type(q) = unset_node;
      span_count(q) = min_quarterword;
      height(q) = 0;
      depth(q) = 0;
      glue_order(q) = normal;
      glue_sign(q) = normal;
      glue_stretch(q) = 0;
      glue_shrink(q) = 0;
      q = p;
    }
  while (!(q == 0));

  save_ptr = save_ptr - 2;
  pack_begin_line = - (integer) mode_line;

  if (mode == -vmode)
  {
    rule_save = overfull_rule;
    overfull_rule = 0;
    p = hpack(preamble, saved(1), saved(0));
    overfull_rule = rule_save;
  }
  else
  {
    q = link(preamble);

    do
      {
        height(q) = width(q);
        width(q) = 0;
        q = link(link(q));
      }
    while (!(q == 0));

    p = vpackage(preamble, saved(1), saved(0), 1073741823L);  /* 2^30 - 1 */
    q = link(preamble);

    do
      {
        width(q) = height(q);
        height(q) = 0;
        q = link(link(q));
      }
    while (!(q == 0));
  }

  pack_begin_line = 0;
  q = link(head);
  s = head;

  while (q != 0)
  {
    if (!(q >= hi_mem_min))
      if (type(q) == unset_node)
      {
        if (mode == -vmode)
        {
          type(q) = hlist_node;
          width(q) = width(p);
        }
        else
        {
          type(q) = vlist_node;
          height(q) = height(p);
        }

        glue_order(q) = glue_order(p);
        glue_sign(q) = glue_sign(p);
        glue_set(q) = glue_set(p);
        shift_amount(q) = o;
        r = link(list_ptr(q));
        s = link(list_ptr(p));

        do
          {
            n = span_count(r);
            t = width(s);
            w = t;
            u = hold_head;

            while (n > min_quarterword)
            {
              decr(n);
              s = link(s);
              v = glue_ptr(s);
              link(u) = new_glue(v);
              u = link(u);
              subtype(u) = tab_skip_code + 1;
              t = t + width(v);

              if (glue_sign(p) == stretching)
              {
                if (stretch_order(v) == glue_order(p))
                  t = t + round(glue_set(p) * stretch(v));
              }
              else if (glue_sign(p) == shrinking)
              {
                if (shrink_order(v) == glue_order(p))
                  t = t - round(glue_set(p) * shrink(v));
              }

              s = link(s);
              link(u) = new_null_box();
              u = link(u);
              t = t + width(s);

              if (mode == -vmode)
                width(u) = width(s);
              else
              {
                type(u) = vlist_node;
                height(u) = width(s);
              }
            }
            

            if (mode == -vmode)
            {
              height(r) = height(q);
              depth(r) = depth(q);

              if (t == width(r))
              {
                glue_sign(r) = normal;
                glue_order(r) = normal;
                glue_set(r) = 0.0;
              }
              else if (t > width(r))
              {
                glue_sign(r) = stretching;

                if (glue_stretch(r) == 0)
                  glue_set(r) = 0.0;
                else
                  glue_set(r) = (t - width(r)) / ((double) glue_stretch(r));
              }
              else
              {
                glue_order(r) = glue_sign(r);
                glue_sign(r) = shrinking;

                if (glue_shrink(r) == 0)
                  glue_set(r) = 0.0;
                else if ((glue_order(r) == normal) && (width(r) - t > glue_shrink(r)))
                  glue_set(r) = 1.0;
                else
                  glue_set(r) = (width(r) - t)/ ((double) glue_shrink(r));
              }

              width(r) = w;
              type(r) = hlist_node;
            }
            else
            {
              width(r) = width(q);

              if (t == height(r))
              {
                glue_sign(r) = normal;
                glue_order(r) = normal;
                glue_set(r) = 0.0;
              }
              else if (t > height(r))
              {
                glue_sign(r) = stretching;

                if (glue_stretch(r) == 0)
                  glue_set(r) = 0.0;
                else
                  glue_set(r) = (t - height(r)) / ((double) glue_stretch(r));
              }
              else
              {
                glue_order(r) = glue_sign(r);
                glue_sign(r) = shrinking;

                if (glue_shrink(r) == 0)
                  glue_set(r) = 0.0;
                else if ((glue_order(r) == normal) && (height(r) - t > glue_shrink(r)))
                  glue_set(r) = 1.0;
                else
                  glue_set(r) = (height(r) - t) / ((double) glue_shrink(r));
              }

              height(r) = w;
              type(r) = vlist_node;
            }

            shift_amount(r) = 0;

            if (u != hold_head)
            {
              link(u) = link(r);
              link(r) = link(hold_head);
              r = u;
            }

            r = link(link(r));
            s = link(link(s));
          }
        while(!(r == 0));
      }
      else if (type(q) == rule_node)
      {
        if ((width(q) == -1073741824L))  /* 2^30  */
          width(q) = width(p);

        if ((height(q) == -1073741824L))  /* 2^30  */
          height(q) = height(p);

        if ((depth(q) == -1073741824L))  /* 2^30  */
          depth(q) = depth(p);

        if (o != 0)
        {
          r = link(q);
          link(q) = 0;
          q = hpack(q, 0, 1);
          shift_amount(q) = o;
          link(q) = r;
          link(s) = q;
        }
      }
    s = q;
    q = link(q);
  }

  flush_node_list(p);
  pop_alignment();
  aux_save = cur_list.aux_field;
  p = link(head);
  q = tail;
  pop_nest();

  if (mode == mmode)
  {
    do_assignments();

    if (cur_cmd != math_shift)
    {
      print_err("Missing $$ inserted");
      help2("Displays can use special alignments (like \\eqalignno)",
          "only if nothing but the alignment itself is between $$'s.");
      back_error();
    }
    else
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

    pop_nest();
    tail_append(new_penalty(pre_display_penalty));
    tail_append(new_param_glue(above_display_skip_code));
    link(tail) = p;

    if (p != 0)
      tail = q;

    tail_append(new_penalty(post_display_penalty));
    tail_append(new_param_glue(below_display_skip_code));
    cur_list.aux_field.cint = aux_save.cint;
    resume_after_display();
  }
  else
  {
    cur_list.aux_field = aux_save;
    link(tail) = p;

    if (p != 0)
      tail = q;

    if (mode == vmode)
    {
      build_page();
    }
  }
}
/* used to be align_peek, zfintieshrink, etc in old tex5.c */
/************************************************************************/
/* moved down here to avoid questions about pragma optimize */
/* #pragma optimize("g", off) */
/* for MC VS compiler */
/* Moved down here 96/Oct/12 in response to problem with texerror.tex */
/* pragma optimize("a", off) not strong enough - this may slow things */
/* sec 0791 */
bool fin_col (void)
{
  halfword p;
  halfword q, r;
  halfword s;
  halfword u;
  scaled w;
  glue_ord o;
  halfword n;

  if (cur_align == 0)
  {
    confusion("endv");
    return 0;       // abort_flag set
  }

  q = link(cur_align);

  if (q == 0)
  {
    confusion("endv");
    return 0;       // abort_flag set
  }

  if (align_state < 500000L)
  {
    fatal_error("(interwoven alignment preambles are not allowed)");
    return 0;     // abort_flag set
  }

  p = link(q);

  if ((p == 0) && (extra_info(cur_align) < cr_code))
    if (cur_loop != 0)
    {
/*  potential problem here if new_null_box causes memory reallocation ??? */
/*  compiler optimization does not refresh `mem' loaded in registers ? */
      link(q) = new_null_box();
      p = link(q);
      info(p) = end_span;
      width(p) = null_flag;
      cur_loop = link(cur_loop);
      q = hold_head;
      r = u_part(cur_loop);

      while (r != 0)
      {
        link(q) = get_avail();
        q = link(q);
        info(q) = info(r);
        r = link(r);
      }

      link(q) = 0;
      u_part(p) = link(hold_head);
      q = hold_head;
      r = v_part(cur_loop);

      while (r != 0)
      {
        link(q) = get_avail();
        q = link(q);
        info(q) = info(r);
        r = link(r);
      }

      link(q) = 0;
      v_part(p) = link(hold_head);
      cur_loop = link(cur_loop);
      link(p) = new_glue(glue_ptr(cur_loop));
    }
    else
    {
      print_err("Extra alignment tab has been changed to ");
      print_esc("cr");
      help3("You have given more \\span or & marks than there were",
          "in the preamble to the \\halign or \\valign now in progress.",
          "So I'll assume that you meant to type \\cr instead.");
      extra_info(cur_align) = cr_code;
      error();
    }

  if (extra_info(cur_align) != span_code)
  {
    unsave();

    new_save_level(align_group);

    {
      if (mode == -hmode)
      {
        adjust_tail = cur_tail;
        u = hpack(link(head), 0, 1);
        w = width(u);
        cur_tail = adjust_tail;
        adjust_tail = 0;
      }
      else
      {
        u = vpackage(link(head), 0, 1, 0);
        w = height(u);
      }

      n = min_quarterword;

      if (cur_span != cur_align)
      {
        q = cur_span;

        do
          {
            incr(n);
            q = link(link(q));
          }
        while (!(q == cur_align));

        if (n > max_quarterword)
        {
          confusion("256 spans");   /* 256 spans --- message wrong now, but ... */
          return 0;       // abort_flag set
        }

        q = cur_span;

        while (link(info(q)) < n)
          q = info(q);

        if (link(info(q)) > n)
        {
          s = get_node(span_node_size);
          info(s) = info(q);
          link(s) = n;
          info(q) = s;
          width(s) = w;
        }
        else if (width(info(q)) < w)
          width(info(q)) = w;
      }
      else if (w > width(cur_align))
        width(cur_align) = w;

      type(u) = unset_node;
      span_count(u) = n;

      if (total_stretch[filll] != 0)
        o = filll;
      else if (total_stretch[fill] != 0)
        o = fill;
      else if (total_stretch[fil] != 0)
        o = fil;
      else
        o = normal;

      glue_order(u) = o;
      glue_stretch(u) = total_stretch[o];

      if (total_shrink[filll] != 0)
        o = filll;
      else if (total_shrink[fill] != 0)
        o = fill;
      else if (total_shrink[fil] != 0)
        o = fil;
      else
        o = normal;

      glue_sign(u) = o;
      glue_shrink(u) = total_shrink[o];
      pop_nest();
      link(tail) = u;
      tail = u;
    }

    tail_append(new_glue(glue_ptr(link(cur_align))));
    subtype(tail) = tab_skip_code + 1;

    if (extra_info(cur_align) >= cr_code)
    {
      return true;
    }

    init_span(p);
  }

  align_state = 1000000L;

  do
    {
      get_x_token();
    }
  while (!(cur_cmd != spacer));

  cur_align = p;
  init_col();

  return false;
}
/* sec 0749 */
scaled make_op_(halfword q)
{
  scaled delta;
  halfword p, v, x, y, z;
  quarterword c;
  ffourquarters i;
  scaled shift_up, shift_down;

  if ((subtype(q) == normal) && (cur_style < text_style))
    subtype(q) = limits;

  if (math_type(nucleus(q)) == math_char)
  {
    fetch(nucleus(q));

    if ((cur_style < text_style) && (char_tag(cur_i) == list_tag))
    {
      c = rem_byte(cur_i);
      i = char_info(cur_f, c);

      if ((i.b0 > 0))
      {
        cur_c = c;
        cur_i = i;
        character(nucleus(q)) = c;
      }
    }

    delta = char_italic(cur_f, cur_i);
    x = clean_box(nucleus(q), cur_style);

    if ((math_type(subscr(q)) != 0) && (subtype(q) != limits))
      width(x) = width(x) - delta;

    shift_amount(x) = half(height(x) - depth(x)) - axis_height(cur_size);
    math_type(nucleus(q)) = sub_box;
    info(nucleus(q)) = x;
  }
  else
    delta = 0;

  if (subtype(q) == limits)
  {
    x = clean_box(supscr(q), sup_style(cur_style));
    y = clean_box(nucleus(q), cur_style);
    z = clean_box(subscr(q), sub_style(cur_style));
    v = new_null_box();
    type(v) = vlist_node;
    width(v) = width(y);

    if (width(x) > width(v))
      width(v) = width(x);

    if (width(z) > width(v))
      width(v) = width(z);

    x = rebox(x, width(v));
    y = rebox(y, width(v));
    z = rebox(z, width(v));
    shift_amount(x) = half(delta);
    shift_amount(z) = - (integer) shift_amount(x);
    height(v) = height(y);
    depth(v) = depth(y);

    if (math_type(supscr(q)) == 0)
    {
      free_node(x, box_node_size);
      list_ptr(v) = y;
    }
    else
    {
      shift_up = big_op_spacing3 - depth(x);

      if (shift_up < big_op_spacing1)
        shift_up = big_op_spacing1;

      p = new_kern(shift_up);
      link(p) = y;
      link(x) = p;
      p = new_kern(big_op_spacing5);
      link(p) = x;
      list_ptr(v) = p;
      height(v) = height(v) + big_op_spacing5 + height(x) + depth(x) + shift_up;
    }

    if (math_type(subscr(q)) == 0)
      free_node(z, box_node_size);
    else
    {
      shift_down = big_op_spacing4 - height(z);

      if (shift_down < big_op_spacing2)
        shift_down = big_op_spacing2;

      p = new_kern(shift_down);
      link(y) = p;
      link(p) = z;
      p = new_kern(big_op_spacing5);
      link(z) = p;
      depth(v) = depth(v) + big_op_spacing5 + height(z) + depth(z) + shift_down;
    }

    new_hlist(q) = v;
  }

  return delta;
}
/* sec 0756 */
void make_scripts_(halfword q, scaled delta)
{
  halfword p, x, y, z;
  scaled shift_up, shift_down, clr;
  small_number t;

  p = new_hlist(q);

  if ((p >= hi_mem_min))
  {
    shift_up = 0;
    shift_down = 0;
  }
  else
  {
    z = hpack(p, 0, 1);

    if (cur_style < script_style)
      t = script_size;
    else
      t = script_script_size;

    shift_up = height(z) - sup_drop(t);
    shift_down = depth(z) + sub_drop(t);
    free_node(z, box_node_size);
  }

  if (math_type(supscr(q)) == 0)
  {
    x = clean_box(subscr(q), sub_style(cur_style));
    width(x) = width(x) + script_space;

    if (shift_down < sub1(cur_size))
      shift_down = sub1(cur_size);

    clr = height(x) -(abs(math_x_height(cur_size) * 4) / 5);

    if (shift_down < clr)
      shift_down = clr;

    shift_amount(x) = shift_down;
  }
  else
  {
    {
      x = clean_box(supscr(q), sup_style(cur_style));
      width(x) = width(x) + script_space;

      if (odd(cur_style))
        clr = sup3(cur_size);
      else if (cur_style < text_style)
        clr = sup1(cur_size);
      else
        clr = sup2(cur_size);

      if (shift_up < clr)
        shift_up = clr;

      clr = depth(x) +(abs(math_x_height(cur_size)) / 4);

      if (shift_up < clr)
        shift_up = clr;
    }

    if (math_type(subscr(q)) == 0)
      shift_amount(x) = - (integer) shift_up;
    else
    {
      y = clean_box(subscr(q), sub_style(cur_style));
      width(y) = width(y) + script_space;

      if (shift_down < sub2(cur_size))
        shift_down = sub2(cur_size);

      clr = 4 * default_rule_thickness - ((shift_up - depth(x)) - (height(y) - shift_down));

      if (clr > 0)
      {
        shift_down = shift_down + clr;

        clr = (abs(math_x_height(cur_size) * 4) / 5) - (shift_up - depth(x));

        if (clr > 0)
        {
          shift_up = shift_up + clr;
          shift_down = shift_down - clr;
        }
      }

      shift_amount(x) = delta;
      p = new_kern((shift_up - depth(x)) - (height(y) - shift_down));
      link(x) = p;
      link(p) = y;
      x = vpackage(x, 0, 1, 1073741823L);  /* 2^30 - 1 */
      shift_amount(x) = shift_down;
    }
  }

  if (new_hlist(q) == 0)
    new_hlist(q) = x;
  else
  {
    p = new_hlist(q);

    while (link(p) != 0)
      p = link(p);

    link(p) = x;
  }
}