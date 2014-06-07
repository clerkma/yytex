/* Copyright 1992 Karl Berry
   Copyright 2007 TeX Users Group
   Copyright 2014 Clerk Ma

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

boolean check_kanji(integer c)
{
  return is_char_kanji(c);
}

boolean is_char_ascii(integer c)
{
  return (0 <= c && c < 0x100);
}

boolean is_char_kanji(integer c)
{
  return (iskanji1(Hi(c)) && iskanji2(Lo(c)));
}

boolean ismultiprn(integer c)
{
  if (iskanji1(c) || iskanji2(c))
    return true;
  
  return false;
}

integer calc_pos(integer c)
{
  unsigned char c1, c2;
  
  if(c>=0 && c<=255)
    return(c);
  
  c1 = (c >> 8) & 0xff;
  c2 = c & 0xff;
  
  if(iskanji1(c1))
  {
    if (is_internalSJIS())
    {
      c1 = ((c1 - 0x81) % 4) * 64;  /* c1 = 0, 64, 128, 192 */
      c2 = c2 % 64;                 /* c2 = 0..63 */
    }
    else
    {
      c1 = ((c1 - 0xa1) % 4) * 64;  /* c1 = 0, 64, 128, 192 */
      c2 = c2 % 64;                 /* c2 = 0..63 */
    }
    
    return(c1 + c2);              /* ret = 0..255 */
  } else
    return(c2);
}

integer kcatcodekey(integer c)
{
  return Hi(toDVI(c));
}

void init_default_kanji (const_string file_str, const_string internal_str)
{
  char *p;
  
  enable_UPTEX (false); /* disable */
  
  if (!set_enc_string (file_str, internal_str))
  {
    fprintf (stderr, "Bad kanji encoding \"%s\" or \"%s\".\n",
      file_str ? file_str  : "NULL",
      internal_str ? internal_str : "NULL");
    uexit(1);
  }

  p = getenv ("PTEX_KANJI_ENC");
  
  if (p)
  {
    if (!set_enc_string (p, NULL))
      fprintf (stderr, "Ignoring bad kanji encoding \"%s\".\n", p);
  }

#ifdef WIN32
  p = kpse_var_value ("guess_input_kanji_encoding");
  
  if (p)
  {
    if (*p == '1' || *p == 'y' || *p == 't')
      infile_enc_auto = 1;
    
    free(p);
  }
#endif
}
/* Routines needed by pTeX */
/*
void print_kanji(KANJI_code s)
{
  if (s > 255)
  {
    print_char(Hi(s));
    print_char(Lo(s));
  }
  else
    print_char(s);
}

void print_kansuji(integer n)
{
  int k;
  KANJI_code cx;

  k = 0;
  
  if (n < 0)
    return;

  do
    {
      dig[k] = n % 10;
      n = n / 10;
      incr(k);
    }
  while (!(n = 0));

  while (k > 0)
  {
    decr(k);
    cx = kansuji_char(dig[k]);
    print_kanji(fromDVI(cx));
  }
}

void print_dir(eight_bits dir)
{
  if (dir == dir_yoko)
    print_char('Y');
  else if (dir == dir_tate)
    print_char('T');
  else if (dir == dir_dtou)
    print_char('D');
}

void print_direction(integer d)
{
  switch (d)
  {
    case dir_yoko:
      print_string("yoko");
      break;

    case dir_tate:
      print_string("tate");
      break;

    case dir_dtou:
      print_string("dtou");
      break;
  }

  if (d < 0)
    print_string("(math)");
  else
    print_string(" direction");
}

eight_bits get_jfm_pos(KANJI_code kcode, internal_font_number f)
{
  KANJI_code jc;
  pointer sp, mp, ep;

  if (f == null_font)
  {
    return kchar_type(null_font, 0);
  }

  jc = toDVI(kcode);
  sp = 1;
  ep = font_num_ext[f] - 1;

  if (ep >= 1 && kchar_code(f, sp) <= jc && jc <= kchar_code(f, ep))
  {
    while (sp <= ep)
    {
      mp = sp + (ep - sp) / 2;

      if (jc < kchar_code(f, mp))
        ep = mp - 1;
      else if (jc > kchar_code(f, mp))
        sp = mp + 1;
      else
      {
        return kchar_typ(f, mp);
      }
    }
  }

  return kchar_type(f, 0);
}

pointer get_inhibit_pos(KANJI_code c, small_number n)
{
  pointer p, s;

  s = calc_pos(c);
  p = s;

  if (n == new_pos)
  {
    do
      {
        if (inhibit_xsp_code(p) == 0 || inhibit_xsp_code(p) == c)
          goto done;

        incr(p);

        if (p > 255)
          p = 0;
      }
    while (!(s == p));

    p = no_entry;
  }
  else
  {
    do
      {
        if (inhibit_xsp_code(p) == 0)
          goto done1;

        if (inhibit_xsp_code(p) == c)
          goto done;

        incr(p);

        if (p > 255)
          p = 0;
      }
    while (!(s == p));

done1:
    p = no_entry;
  }

done:
  return p;
}

void set_math_kchar(integer c)
{
  pointer p;

  p = new_noad();
  math_type(nucleus(p)) = math_jchar;
  inhibit_glue_flag = false;
  character(nucleus(p)) = 0;
  math_kcode(p) = c;
  fam(nucleus) = cur_jfam;

  if (font_dir[fam_fnt(fam(nucleus(p)) + cur_size)] == dir_default)
  {
    print_err("Not two-byte family");
    help1("IGNORE.");
    error();
  }

  type(p) = ord_noad;
  link(tail) = p;
  tail = p;
}

void synch_dir(void)
{
  scaled tmp;

  switch (cur_dir_hv)
  {
    case dir_yoko:
      if (dvi_dir != cur_dir_hv)
      {
        synch_h();
        synch_v();
        dvi_out(dir_chg);
        dvi_out(dvi_yoko);
        dir_used = true;

        switch ()
        {
          case dir_tate:
            tmp = cur_h;
            cur_h = -cur_v;
            cur_v = tmp;
            break;

          case dir_dtou:
            tmp = cur_h;
            cur_h = cur_v;
            cur_v = -tmp;
            break
        }

        dvi_h = cur_h;
        dvi_v = cur_v;
        dvi_dir = cur_dir_hv;
      }
      break;

    case dir_tate:
      if (dvi_dir != cur_dir_hv)
      {
        synch_h();
        synch_v();
        dvi_out(dirchg);
        dvi_out(dvi_tate);
        dir_used = true;

        switch (dvi_dir)
        {
          case dir_yoko:
            tmp = cur_h;
            cur_h = cur_v;
            cur_v = -tmp;
            break;

          case dir_dtou:
            cur_v = -cur_v;
            cur_h = -cur_h;
            break;
        }

        dvi_h = cur_h;
        dvi_v = cur_v;
        dvi_dir = cur_dir_hv;
      }
      break;

    case dir_dtou:
      if (dvi_dir != cur_dir_hv)
      {
        synch_h();
        synch_v();
        dvi_out(dir_chg);
        dvi_out(dvi_dtou);
        dir_used = true;

        switch (dvi_dir)
        {
          case dir_yoko:
            tmp = cur_h;
            cur_h = -cur_v;
            cur_v = tmp;
            break;

          case dir_tate:
            cur_v = -cur_v;
            cur_h = -cur_h;
            break;
        }

        dvi_h = cur_h;
        dvi_v = cur_v;
        dvi_dir = cur_dir_hv;
      }
      break;

    default:
      confusion("synch_dir");
      break;
  }
}

void dir_out(void)
{
  pointer this_box;

  this_box = temp_ptr;
  temp_ptr = list_ptr(this_box);

  if (type(temp_ptr) != hlist_node && type(temp_ptr) != vlist_out)
    confusion("dir_out");

  switch (box_dir(this_box))
  {
    case dir_yoko:
      switch (box_dir(temp_ptr))
      {
        case dir_tate:
          cur_v = cur_v - height(this_box);
          cur_h = cur_h + depth(temp_ptr);
          break;

        case dir_dtou:
          cur_v = cur_v + depth(this_box);
          cur_h = cur_h + height(temp_ptr);
          break;
      }
      break;

    case dir_tate:
      switch (box_dir(temp_ptr))
      {
        case dir_yoko:
          cur_v = cur_v + depth(this_box);
          cur_h = cur_h + height(temp_ptr);
          break;

        case dir_dtou:
          cur_v = cur_v + depth(this_box) - height(temp_ptr);
          cur_h = cur_h + width(temp_ptr);
          break;
      }
      break;

    case dir_dtou:
      switch (box_dir(temp_ptr))
      {
        case dir_yoko:
          cur_v = cur_v - height(this_box);
          cur_h = cur_h + depth(temp_ptr);
          break;

        case dir_tate:
          cur_v = cur_v + depth(this_box) - height(temp_ptr);
          cur_h = cur_h + width(temp_ptr);
          break;
      }
      break;
  }

  cur_dir_hv = box_dir(temp_ptr);

  if (type(temp_ptr) == vlist_node)
    vlist_out();
  else
    hlist_out();
}
*/