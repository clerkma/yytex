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

#define BEGINFMTCHECKSUM 367403084L
#define ENDFMTCHECKSUM   69069L

extern clock_t start_time, main_time, finish_time; /* in local.c */

#ifdef INITEX
  void do_initex (void);
#endif

/* sec 0004 */
void initialize (void)
{
  integer i; 
  integer k; 
  integer flag; /* bkph */

#ifndef ALLOCATEHYPHEN
  hyph_pointer z;
#endif

  if (!non_ascii)
  {
    for (i = 0; i <= 255; i++)
      xchr[i] = (char) i;

#ifdef JOKE
    xchr[32] = ' ';  xchr[33] = '!';  xchr[34] = '"';  xchr[35] = '#';
    xchr[36] = '$';  xchr[37] = '%';  xchr[38] = '&';  xchr[39] = '\'';
    xchr[40] = '(';  xchr[41] = ')';  xchr[42] = '*';  xchr[43] = '+';
    xchr[44] = ',';  xchr[45] = '-';  xchr[46] = '.';  xchr[47] = '/';
    xchr[48] = '0';  xchr[49] = '1';  xchr[50] = '2';  xchr[51] = '3';
    xchr[52] = '4';  xchr[53] = '5';  xchr[54] = '6';  xchr[55] = '7';
    xchr[56] = '8';  xchr[57] = '9';  xchr[58] = ':';  xchr[59] = ';';
    xchr[60] = '<';  xchr[61] = '=';  xchr[62] = '>';  xchr[63] = '?';
    xchr[64] = '@';  xchr[65] = 'A';  xchr[66] = 'B';  xchr[67] = 'C';
    xchr[68] = 'D';  xchr[69] = 'E';  xchr[70] = 'F';  xchr[71] = 'G';
    xchr[72] = 'H';  xchr[73] = 'I';  xchr[74] = 'J';  xchr[75] = 'K';
    xchr[76] = 'L';  xchr[77] = 'M';  xchr[78] = 'N';  xchr[79] = 'O';
    xchr[80] = 'P';  xchr[81] = 'Q';  xchr[82] = 'R';  xchr[83] = 'S';
    xchr[84] = 'T';  xchr[85] = 'U';  xchr[86] = 'V';  xchr[87] = 'W';
    xchr[88] = 'X';  xchr[89] = 'Y';  xchr[90] = 'Z';  xchr[91] = '[';
    xchr[92] = '\\'; xchr[93] = ']';  xchr[94] = '^';  xchr[95] = '_';
    xchr[96] = '`';  xchr[97] = 'a';  xchr[98] = 'b';  xchr[99] = 'c';
    xchr[100] = 'd'; xchr[101] = 'e'; xchr[102] = 'f'; xchr[103] = 'g';
    xchr[104] = 'h'; xchr[105] = 'i'; xchr[106] = 'j'; xchr[107] = 'k';
    xchr[108] = 'l'; xchr[109] = 'm'; xchr[110] = 'n'; xchr[111] = 'o';
    xchr[112] = 'p'; xchr[113] = 'q'; xchr[114] = 'r'; xchr[115] = 's';
    xchr[116] = 't'; xchr[117] = 'u'; xchr[118] = 'v'; xchr[119] = 'w';
    xchr[120] = 'x'; xchr[121] = 'y'; xchr[122] = 'z'; xchr[123] = '{';
    xchr[124] = '|'; xchr[125] = '}'; xchr[126] = '~';

    for (i = 0; i <= 31; i++)
      xchr[i] = chr(i);

    for (i = 127; i <= 255; i++)
      xchr[i]= chr(i);
#endif
  }

  for (i = 0; i <= 255; i++)
    xord[chr(i)] = 127;

#ifdef JOKE
  for (i = 128; i <= 255 ; i++)
    xord[xchr[i]] = i;

  for (i = 0; i <= 126; i++)
    xord[xchr[i]] = i;
#endif

  for (i = 0; i <= 255; i++)
    xord[xchr[i]] = (char) i;

  xord[127] = 127;

  flag = 0;

  if (trace_flag != 0)
  {
    for (k = 0; k < 256; k++)
      if (xord[k] != k)
      {
        flag = 1;
        break;
      }

    if (flag)
    {
      show_line("Inverted mapping xord[] pairs:\n", 0);

      for (k = 0; k < 256; k++)
      {
        if (xord[k] != 127)
        {
          sprintf(log_line, "%d => %d\n", k, xord[k]);
          show_line(log_line, 0);
        }
      }
    }
  }

  if (interaction < batch_mode)
    interaction = error_stop_mode;

  deletions_allowed = true;
  set_box_allowed = true;
  error_count = 0;
  help_ptr = 0;
  use_err_help = false;
  interrupt = 0;
  OK_to_interrupt = true;

#ifdef DEBUG
  was_mem_end = mem_min;
/*  was_lo_max = mem_min; */
  was_lo_max = mem_bot;
/*  was_hi_min = mem_max; */
  was_hi_min = mem_top;
  panicking = false;
#endif

  nest_ptr = 0;
  max_nest_stack = 0;
  mode = 1;
  head = contrib_head;
  tail = contrib_head;
  cur_list.aux_field.cint = ignore_depth;
  mode_line = 0;
  prev_graf = 0;
  shown_mode = 0;
  page_contents = 0;
  page_tail = page_head;

#ifdef ALLOCATEMAIN
  if (is_initex)
#endif
    link(page_head) = 0;

  last_glue = empty_flag;
  last_penalty = 0;
  last_kern = 0;
  page_so_far[7] = 0;
  page_max_depth = 0;

  for (k = int_base; k <= eqtb_size; k++)
    xeq_level[k] = level_one;

  no_new_control_sequence = true;
  next(hash_base) = 0;
  text(hash_base) = 0;

  for (k = hash_base + 1; k <= undefined_control_sequence - 1; k++)
    hash[k] = hash[hash_base];

  save_ptr = 0;
  cur_level = 1;
  cur_group = 0;
  cur_boundary = 0;
  max_save_stack = 0;
  mag_set = 0;
  cur_mark[0] = 0;
  cur_mark[1] = 0;
  cur_mark[2] = 0;
  cur_mark[3] = 0;
  cur_mark[4] = 0;
  cur_val = 0;
  cur_val_level = 0;
  radix = 0;
  cur_order = 0;

  for (k = 0; k <= 16; k++)
    read_open[k] = 2;

  cond_ptr = 0;
  if_limit = 0;
  cur_if = 0;
  if_line = 0;

  for (k = 0; k <= font_max; k++)
    font_used[k] = false;

  null_character.b0 = 0;
  null_character.b1 = 0;
  null_character.b2 = 0;
  null_character.b3 = 0;
  total_pages = 0;
  max_v = 0;
  max_h = 0;
  max_push = 0;
  last_bop = -1;
  doing_leaders = false;
  dead_cycles = 0;
  cur_s = -1;
  half_buf = dvi_buf_size / 2;
  dvi_limit = dvi_buf_size;
  dvi_ptr = 0;
  dvi_offset = 0;
  dvi_gone = 0;
  down_ptr = 0;
  right_ptr = 0;
  adjust_tail = 0;
  last_badness = 0;
  pack_begin_line = 0;
  empty_field.v.RH = 0;
  empty_field.v.LH = 0;
  null_delimiter.b0 = 0;
  null_delimiter.b1 = 0;
  null_delimiter.b2 = 0;
  null_delimiter.b3 = 0;
  align_ptr = 0;
  cur_align = 0;
  cur_span = 0;
  cur_loop = 0;
  cur_head = 0;
  cur_tail = 0;


/*  *not* OK with ALLOCATEHYPHEN, since may not be allocated yet */
#ifndef ALLOCATEHYPHEN
  for (z = 0; z <= hyphen_prime; z++)
  {
    hyph_word[z] = 0;
    hyph_list[z] = 0;
  }
#endif

  hyph_count = 0;
  output_active = false;
  insert_penalties = 0;
  ligature_present = false;
  cancel_boundary = false;
  lft_hit = false;
  rt_hit = false;
  ins_disc = false;
  after_token = 0;
  long_help_seen = false;
  format_ident = 0;

  for (k = 0; k <= 17; k++)
    write_open[k] = false;

  edit_name_start = 0;

#ifdef INITEX
  if (is_initex)
  {
    do_initex();
  }
#endif
}

/* do the part of initialize() that requires mem_top, mem_max or mem[] */
/* do this part after allocating main memory */

#ifdef ALLOCATEMAIN
void initialize_aux (void)
{
#ifdef DEBUG
  was_mem_end = mem_min;
/*  was_lo_max = mem_min; */
  was_lo_max = mem_bot;
/*  was_hi_min = mem_max; */
  was_hi_min = mem_top;
  panicking = false;
#endif /* DEBUG */
/*  nest_ptr = 0; */
/*  max_nest_stack = 0; */
  mode = 1;
  head = contrib_head;
  tail = contrib_head;
  cur_list.aux_field.cint = ignore_depth;
  mode_line = 0;
  prev_graf = 0;
/*  shown_mode = 0; */
/*  page_contents = 0; */
  page_tail = page_head;
  link(page_head) = 0;
}
#endif  // end of ifdef ALLOCATEMAIN
/* sec 0815 */
void line_break_ (integer final_widow_penalty)
{
  bool auto_breaking;
  halfword prevp;
  halfword q, r, s, prevs;
  internal_font_number f;
/*  small_number j;  */
  int j;                /* 95/Jan/7 */
/*  unsigned char c;  */
  unsigned int c;           /* 95/Jan/7 */
/*  savedbadness = 0; */    /* 96/Feb/9 */

  pack_begin_line = mode_line;
  link(temp_head) = link(head);

  if ((tail >= hi_mem_min))
  {
    tail_append(new_penalty(inf_penalty));
  } 
  else if (type(tail) != glue_node)
  {
    tail_append(new_penalty(inf_penalty));
  }
  else
  {
    type(tail) = penalty_node;
    delete_glue_ref(glue_ptr(tail));
    flush_node_list(leader_ptr(tail));
    penalty(tail) = inf_penalty;
  }

  link(tail) = new_param_glue(par_fill_skip_code);
  init_cur_lang = prev_graf % 65536L;
  init_l_hyf = prev_graf / 4194304L; /* 2^22 */
  init_r_hyf = (prev_graf / 65536L) % 64;
  pop_nest();
  no_shrink_error_yet = true;

  if ((shrink_order(left_skip) != normal) && (shrink(left_skip) != 0))
  {
    left_skip = finite_shrink(left_skip);
  }

  if ((shrink_order(right_skip) != normal) && (shrink(right_skip) != 0))
  {
    right_skip = finite_shrink(right_skip);
  }

  q = left_skip;
  r = right_skip;
  background[1] = width(q) + width(r);
  background[2] = 0;
  background[3] = 0;
  background[4] = 0;
  background[5] = 0;
  background[2 + stretch_order(q)] = stretch(q);
  background[2 + stretch_order(r)] = background[2 + stretch_order(r)] + stretch(r);
  background[6] = shrink(q) + shrink(r);

  minimum_demerits = 1073741823L; /* 2^30 - 1 *//* 40000000 hex - 1 */
  minimal_demerits[tight_fit] = 1073741823L;
  minimal_demerits[decent_fit] = 1073741823L;
  minimal_demerits[loose_fit] = 1073741823L;
  minimal_demerits[very_loose_fit] = 1073741823L;

  if (par_shape_ptr == 0)
    if (hang_indent == 0)
    {
      last_special_line = 0;
      second_width = hsize;
      second_indent = 0;
    }
    else
    {
      last_special_line = abs(hang_after);

      if (hang_after < 0)
      {
        first_width = hsize - abs(hang_indent);

        if (hang_indent >= 0)
          first_indent = hang_indent;
        else
          first_indent = 0;

        second_width = hsize;
        second_indent = 0;
      }
      else
      {
        first_width = hsize;
        first_indent = 0;
        second_width = hsize - abs(hang_indent);

        if (hang_indent >= 0)
          second_indent = hang_indent;
        else
          second_indent = 0;
      }
    }
  else
  {
    last_special_line = info(par_shape_ptr) - 1;
    second_width = mem[par_shape_ptr + 2 * (last_special_line + 1)].cint;
    second_indent = mem[par_shape_ptr + 2 * last_special_line + 1].cint;
  }

  if (looseness == 0)
    easyline = last_special_line;
  else
    easyline = empty_flag;

  threshold = pretolerance;

  if (threshold >= 0)
  {
#ifdef STAT
    if (tracing_paragraphs > 0)
    {
      begin_diagnostic();
      print_nl("@firstpass");
    }
#endif /* STAT */
    second_pass = false;
    final_pass = false;
    first_pass_count++; /* 96 Feb 9 */
  }
  else
  {
    threshold = tolerance;
    second_pass = true;
    final_pass = (emergency_stretch <= 0);
#ifdef STAT
    if (tracing_paragraphs > 0)
      begin_diagnostic();
#endif /* STAT */
  }

  while (true)
  {
    if (threshold > inf_bad)
      threshold = inf_bad;

    if (second_pass)
    {
#ifdef INITEX
      if (is_initex)
      {
        if (trie_not_ready)
          init_trie();
      } /* bkph */
#endif /* INITEX */

      cur_lang = init_cur_lang;
      lhyf = init_l_hyf;
      rhyf = init_r_hyf;
    }

    q = get_node(3);
    type(q) = unhyphenated;
    fitness(q) = decent_fit;
    link(q) = active;
    break_node(q) = 0;
    line_number(q) = prev_graf + 1;
    total_demerits(q) = 0;
    link(active) = q;
    active_width[1] = background[1];
    active_width[2] = background[2];
    active_width[3] = background[3];
    active_width[4] = background[4];
    active_width[5] = background[5];
    active_width[6] = background[6];

    passive = 0;
    printed_node = temp_head;
    pass_number = 0;
    font_in_short_display = 0;

    cur_p = link(temp_head);
    auto_breaking = true;
    prevp = cur_p;

    while((cur_p != 0) && (link(active) != active))
    {
      if ((cur_p >= hi_mem_min))
      {
        prevp = cur_p;

        do
          {
            f = font(cur_p);
            active_width[1] = active_width[1] + char_width(f, char_info(f, character(cur_p)));
            cur_p = link(cur_p);
          }
        while(!(!(cur_p >= hi_mem_min)));
      }

      switch(type(cur_p))
      {
        case hlist_node:
        case vlist_node:
        case rule_node:
          active_width[1] = active_width[1] + width(cur_p);
          break;
        case whatsit_node:
          if (subtype(cur_p) == language_node)
          {
            cur_lang = what_lang(cur_p);
            lhyf = what_lhm(cur_p);
            rhyf = what_rhm(cur_p);
          }
          break;
        case glue_node:
          {
            if (auto_breaking)
            {
              if ((prevp >= hi_mem_min))
                try_break(0, unhyphenated);
              else if ((mem[prevp].hh.b0 < 9))
                try_break(0, unhyphenated);
              else if ((type(prevp) == kern_node) && (subtype(prevp) != explicit))
                try_break(0, unhyphenated);
            }

            if ((mem[glue_ptr(cur_p)].hh.b1 != 0) && (mem[glue_ptr(cur_p) + 3].cint != 0))
            {
              glue_ptr(cur_p) = finite_shrink(glue_ptr(cur_p));
            }

            q = glue_ptr(cur_p);
            active_width[1] = active_width[1]+ width(q);
            active_width[2 + stretch_order(q)] = active_width[2 + stretch_order(q)] + stretch(q);
            active_width[6] = active_width[6] + shrink(q);

            if (second_pass && auto_breaking)
            {
              prevs = cur_p;
              s = link(prevs);

              if (s != 0)
              {
                while (true)
                {
                  if ((s >= hi_mem_min))
                  {
                    c = character(s);
                    hf = font(s);
                  }
                  else if (type(s) == ligature_node)
                    if (lig_ptr(s) == 0)
                      goto lab22;
                    else
                    {
                      q = lig_ptr(s);
                      c = character(q);
                      hf = font(q);
                    }
                  else if ((type(s) == kern_node) && (subtype(s) == normal))
                    goto lab22;
                  else if (type(s) == whatsit_node)
                  {
                    if (subtype(s) == language_node)
                    {
                      cur_lang = what_lang(s);
                      lhyf = what_lhm(s);
                      rhyf = what_rhm(s);
                    }
                    goto lab22;
                  }
                  else
                    goto lab31;

                  if (lc_code(c) != 0)
                    if ((lc_code(c) == (halfword) c) || (uc_hyph > 0)) /* fixed signed tyoe */
                      goto lab32;
                    else
                      goto lab31;
lab22:
                  prevs = s;
                  s = link(prevs);
                }
lab32:
                hyf_char = hyphen_char[hf];

                if (hyf_char < 0)
                  goto lab31; 

                if (hyf_char > 255)
                  goto lab31; /* ? */

                ha = prevs;

                if (lhyf + rhyf > 63)
                  goto lab31;

                hn = 0;

                while (true)
                {
                  if ((s >= hi_mem_min))
                  {
                    if (font(s) != hf)
                      goto lab33;

                    hyfbchar = character(s);

                    c = hyfbchar;     /*  unsigned char c;  */

                    if (lc_code(c) == 0)
                      goto lab33;

                    if (hn == 63)
                      goto lab33;

                    hb = s;
                    incr(hn);
                    hu[hn] = c;
                    hc[hn]= lc_code(c);
                    hyfbchar = non_char;
                  }
                  else if (type(s) == ligature_node)
                  {
                    if (font(lig_char(s)) != hf)
                      goto lab33;

                    j = hn;
                    q = lig_ptr(s);

                    if (q != 0) /* 94/Mar/23 BUG FIX */
                      hyfbchar = character(q);

                    while (q != 0) /* 94/Mar/23 BUG FIX */
                    {
                      c = character(q);

                      if (lc_code(c) == 0)
                        goto lab33;

                      if (j == 63)
                        goto lab33;

                      incr(j);
                      hu[j] = c;
                      hc[j] = lc_code(c);
                      q = link(q);
                    }

                    hb = s;
                    hn = j;

                    if (odd(subtype(s)))
                      hyfbchar = font_bchar[hf];
                    else
                      hyfbchar = non_char;
                  }
                  else if ((type(s) == kern_node) && (subtype(s) == normal))
                  {
                    hb = s;
                    hyfbchar = font_bchar[hf];
                  }
                  else
                    goto lab33;

                  s = link(s);
                }
lab33:;
                if (hn < lhyf + rhyf)
                  goto lab31;

                while (true)
                {
                  if (!((s >= hi_mem_min)))
                    switch(type(s))
                    {
                      case ligature_node:
                        break;
                      case kern_node:
                        if (subtype(s) != normal)
                          goto lab34;
                        break;
                      case whatsit_node:
                      case glue_node:
                      case penalty_node:
                      case ins_node:
                      case adjust_node:
                      case mark_node:
                        goto lab34;
                        break;
                      default:
                        goto lab31;
                        break;
                    }
                  s = link(s);
                }
lab34:;
                hyphenate();
              }
lab31:;
            }
          }
          break;
        case kern_node:
          if (subtype(cur_p) == explicit)
          {
            if (!(link(cur_p) >= hi_mem_min) && auto_breaking)
              if (type(link(cur_p)) == glue_node)
                try_break(0, unhyphenated);

            active_width[1] = active_width[1] + width(cur_p);
          }
          else
            active_width[1] = active_width[1] + width(cur_p);
          break;
        case ligature_node:
          {
            f = font(lig_char(cur_p));
            active_width[1] = active_width[1] + char_width(f, char_info(f, character(lig_char(cur_p))));
          }
          break;
        case disc_node:
          {
            s = pre_break(cur_p);
            disc_width = 0;

            if (s == 0)
              try_break(ex_hyphen_penalty, hyphenated);
            else
            {
              do
              {
                if ((s >= hi_mem_min))
                {
                  f = font(s);
                  disc_width = disc_width + char_width(f, char_info(f, character(s)));
                }
                else switch(type(s))
                {
                  case ligature_node:
                    {
                      f = font(lig_char(s));
                      disc_width = disc_width + font_info[width_base[f] + font_info[char_base[f]+ mem[s + 1].hh.b1].qqqq.b0].cint;
                    }
                    break;
                  case hlist_node:
                  case vlist_node:
                  case rule_node:
                  case kern_node:
                    disc_width = disc_width + width(s);
                    break;
                  default:
                    {
                      confusion("disc3");
                      return;       // abort_flag set
                    }
                    break;
                }
                s = link(s);
              }
              while(!(s == 0));

              active_width[1] = active_width[1] + disc_width;
              try_break(hyphen_penalty, hyphenated);
              active_width[1] = active_width[1] - disc_width;
            }
            r = replace_count(cur_p);
            s = link(cur_p);

            while (r > 0)
            {
              if ((s >= hi_mem_min))
              {
                f = font(s);
                active_width[1] = active_width[1] + char_width(f, char_info(f, character(s)));
              }
              else switch(type(s))
              {
                case ligature_node:
                  {
                    f = font(lig_char(s));
                    active_width[1] = active_width[1] + char_width(f, char_info(f, character(lig_char(s))));
                  }
                  break;
                case hlist_node:
                case vlist_node:
                case rule_node:
                case kern_node:
                  active_width[1] = active_width[1] + width(s);
                  break;
                default:
                  {
                    confusion("disc4");
                    return;       // abort_flag set
                  }
                  break;
              }
              decr(r);
              s = link(s);
            }
            prevp = cur_p;
            cur_p = s;
            goto lab35;
          }
          break;
        case math_node:
          {
            auto_breaking = (subtype(cur_p) == 1);
            {
              if (!(link(cur_p) >= hi_mem_min) && auto_breaking)
                if (type(link(cur_p)) == glue_node)
                  try_break(0, unhyphenated);

              active_width[1] = active_width[1] + width(cur_p);
            }
          }
          break;
        case penalty_node:
          try_break(penalty(cur_p), unhyphenated);
          break;
        case mark_node:
        case ins_node:
        case adjust_node:
          break;
        default:
          {
            confusion("paragraph");
            return;       // abort_flag set
          }
          break;
      }
      prevp = cur_p;
      cur_p = link(cur_p);
lab35:;
    }

    if (cur_p == 0)
    {
      try_break(eject_penalty, hyphenated);

      if (link(active) != active)
      {
        r = link(active);
        fewest_demerits = 1073741823L; /* 2^30 - 1 */

        do
        {
          if (type(r) != 2)
            if (total_demerits(r) < fewest_demerits)
            {
              fewest_demerits = total_demerits(r);
              best_bet = r;
            }
          r = link(r);
        }
        while (!(r == active));
        best_line = line_number(best_bet);

        if (looseness == 0)
        {
          goto lab30;           /* normal exit */
        }

        {
          r = link(active);
          actual_looseness = 0;
          do
          {
            if (type(r) != 2)
            {
              line_diff = toint(line_number(r)) - toint(best_line);

              if (((line_diff < actual_looseness) && (looseness <= line_diff)) ||
                  ((line_diff > actual_looseness) && (looseness >= line_diff)))
              {
                best_bet = r;
                actual_looseness = line_diff;
                fewest_demerits = total_demerits(r);
              }
              else if ((line_diff == actual_looseness) && (total_demerits(r) < fewest_demerits))
              {
                best_bet = r;
                fewest_demerits = total_demerits(r);
              }
            }
            r = link(r);
          }
          while (!(r == active));
          best_line = line_number(best_bet);
        }

        if ((actual_looseness == looseness) || final_pass)
        {
          goto lab30;
        }
      }
    }

    q = link(active);

    while (q != active)
    {
      cur_p = link(q);

      if (type(q) == 2)
        free_node(q, 7);
      else
        free_node(q, 3);

      q = cur_p;
    }

    q = passive;

    while (q != 0)
    {
      cur_p = link(q);
      free_node(q, 2);
      q = cur_p;
    }

    if (! second_pass)
    {
#ifdef STAT
      if (tracing_paragraphs > 0)
        print_nl("@secondpass");
#endif /* STAT */
      threshold = tolerance;
      second_pass = true;
      second_pass_count++;          /* 96 Feb 9 */
      final_pass = (emergency_stretch <= 0);
    }
    else
    {
#ifdef STAT
      if (tracing_paragraphs > 0)
        print_nl("@emergencypass");
#endif /* STAT */
/*     can only get here is \emergencystretch has been set positive */
      background[2] = background[2] + emergency_stretch;
      final_pass = true;
      ++final_pass_count;         /* 96 Feb 9 */
    } /* end of if second_pass */
  } /* end of while(true)do */
/* cannot drop through from above loop */
lab30:                /* common exit point */
  if (best_line == 2)
    single_line++;

#ifdef STAT
  if (tracing_paragraphs > 0)
  {
    end_diagnostic(true);
    normalize_selector();
  }
#endif /* STAT */

  post_line_break(final_widow_penalty);
  q = link(active);

  while (q != active)
  {
    cur_p = link(q);

    if (type(q) == 2)
      free_node(q, 7);
    else
      free_node(q, 3);

    q = cur_p;
  }

  q = passive;

  while (q != 0)
  {
    cur_p = link(q);
    free_node(q, 2);
    q = cur_p;
  }

  pack_begin_line = 0;
}
/* sec 1211 */
void prefixed_command (void)
{
  small_number a;
  internal_font_number f;
  halfword j;
  font_index k;
  halfword p, q;
  integer n;
  bool e;

  a = 0;

  while (cur_cmd == prefix)
  {
    if (!odd(a / cur_chr))
      a = a + cur_chr;

    do
      {
        get_x_token();
      }
    while(!((cur_cmd != spacer) && (cur_cmd != relax)));

    if (cur_cmd <= max_non_prefixed_command)
    {
      print_err("You can't use a prefix with `");
      print_cmd_chr(cur_cmd, cur_chr);
      print_char('\'');
      help1("I'll pretend you didn't say \\long or \\outer or \\global.");
      back_error();
      return;
    }
  }

  if ((cur_cmd != def) && (a % 4 != 0))
  {
    print_err("You can't use `");
    print_esc("long");
    print_string("' or `");
    print_esc("outer");
    print_string("' with `");
    print_cmd_chr(cur_cmd, cur_chr); 
    print_char('\'');
    help1("I'll pretend you didn't say \\long or \\outer here.");
    error();
  }

  if (global_defs != 0)
    if (global_defs < 0)
    {
      if ((a >= 4))
        a = a - 4;
    }
    else
    {
      if (!(a >= 4))
        a = a + 4;
    }

  switch (cur_cmd)
  {
    case set_font:
      if ((a >= 4))
        geq_define(cur_font_loc, data, cur_chr);
      else
        eq_define(cur_font_loc, data, cur_chr);
      break;

    case def:
      {
        if (odd(cur_chr) && !(a >= 4) && (global_defs >= 0))
          a = a + 4;

        e = (cur_chr >= 2);
        get_r_token();
        p = cur_cs;
        q = scan_toks(true, e);

        if ((a >= 4))
          geq_define(p, call + (a % 4), def_ref);
        else
          eq_define(p, call + (a % 4), def_ref);
      }
      break;

    case let:
      {
        n = cur_chr;
        get_r_token();
        p = cur_cs;

        if (n == 0)
        {
          do
            {
              get_token();
            }
          while (!(cur_cmd != spacer));

          if (cur_tok == other_token + '=')
          {
            get_token();

            if (cur_cmd == spacer)
              get_token();
          }
        }
        else
        {
          get_token();
          q = cur_tok;
          get_token();
          back_input();
          cur_tok = q;
          back_input();
        }

        if (cur_cmd >= call)
          incr(mem[cur_chr].hh.v.LH);

        if ((a >= 4))
          geq_define(p, cur_cmd, cur_chr);
        else
          eq_define(p, cur_cmd, cur_chr);
      }
      break;

    case shorthand_def:
      {
        n = cur_chr;
        get_r_token();
        p = cur_cs;

        if ((a >= 4))
          geq_define(p, relax, 256);
        else
          eq_define(p, relax, 256);

        scan_optional_equals();

        switch (n)
        {
          case char_def_code:
            {
              scan_char_num();

              if ((a >= 4))
                geq_define(p, char_given, cur_val);
              else
                eq_define(p, char_given, cur_val);
            }
            break;

          case math_char_def_code:
            {
              scan_fifteen_bit_int();

              if ((a >= 4))
                geq_define(p, math_given, cur_val);
              else
                eq_define(p, math_given, cur_val);
            }
            break;

          default:
            {
              scan_eight_bit_int();

              switch (n)
              {
                case count_def_code:
                  if ((a >= 4))
                    geq_define(p, assign_int, count_base + cur_val);
                  else
                    eq_define(p, assign_int, count_base + cur_val);
                  break;

                case dimen_def_code:
                  if ((a >= 4))
                    geq_define(p, assign_dimen, scaled_base + cur_val);
                  else
                    eq_define(p, assign_dimen, scaled_base + cur_val);
                  break;

                case skip_def_code:
                  if ((a >= 4)) 
                    geq_define(p, assign_glue, skip_base + cur_val);
                  else
                    eq_define(p, assign_glue, skip_base + cur_val);
                  break;

                case mu_skip_def_code:
                  if ((a >= 4))
                    geq_define(p, assign_mu_glue, mu_skip_base + cur_val);
                  else
                    eq_define(p, assign_mu_glue, mu_skip_base + cur_val);
                  break;

                case toks_def_code:
                  if ((a >= 4))
                    geq_define(p, assign_toks, toks_base + cur_val);
                  else
                    eq_define(p, assign_toks, toks_base + cur_val);
                  break;
              }
            }
            break;
        }
      }
      break;

    case read_to_cs:
      {
        scan_int();
        n = cur_val;

        if (! scan_keyword("to"))
        {
          print_err("Missing `to' inserted");
          help2("You should have said `\\read<number> to \\cs'.",
              "I'm going to look for the \\cs now.");
          error();
        }

        get_r_token();
        p = cur_cs;
        read_toks(n, p);

        if ((a >= 4))
          geq_define(p, call, cur_val);
        else
          eq_define(p, call, cur_val);
      }
      break;

    case toks_register:
    case assign_toks:
      {
        q = cur_cs;

        if (cur_cmd == toks_register)
        {
          scan_eight_bit_int();
          p = toks_base + cur_val;
        }
        else
          p = cur_chr;

        scan_optional_equals();

        do
          {
            get_x_token();
          }
        while(!((cur_cmd != spacer) && (cur_cmd != relax)));

        if (cur_cmd != left_brace)
        {
          if (cur_cmd == toks_register)
          {
            scan_eight_bit_int();
            cur_cmd = assign_toks;
            cur_chr = toks_base + cur_val;
          }

          if (cur_cmd == assign_toks)
          {
            q = equiv(cur_chr);

            if (q == 0)
              if ((a >= 4))
                geq_define(p, undefined_cs, 0);
              else
                eq_define(p, undefined_cs, 0);
            else
            {
              incr(mem[q].hh.v.LH);

              if ((a >= 4))
                geq_define(p, call, q);
              else
                eq_define(p, call, q);
            }
            goto lab30;
          }
        }

        back_input();
        cur_cs = q;
        q = scan_toks(false, false);

        if (link(def_ref) == 0)
        {
          if ((a >= 4))
            geq_define(p, undefined_cs, 0);
          else
            eq_define(p, undefined_cs, 0);

          free_avail(def_ref);
        }
        else
        {
          if (p == output_routine_loc)
          {
            link(q) = get_avail();
            q = link(q);
            info(q) = right_brace_token + '}';
            q = get_avail();
            info(q) = left_brace_token + '{';
            link(q) = link(def_ref);
            link(def_ref) = q;
          }

          if ((a >= 4))
            geq_define(p, call, def_ref);
          else
            eq_define(p, call, def_ref);
        }
      }
      break;

    case assign_int:
      {
        p = cur_chr;
        scan_optional_equals();
        scan_int();

        if ((a >= 4))
          geq_word_define(p, cur_val);
        else
          eq_word_define(p, cur_val);
      }
      break;

    case assign_dimen:
      {
        p = cur_chr;
        scan_optional_equals();
        scan_dimen(false, false, false);

        if ((a >= 4))
          geq_word_define(p, cur_val);
        else
          eq_word_define(p, cur_val);
      }
      break;

    case assign_glue:
    case assign_mu_glue:
      {
        p = cur_chr;
        n = cur_cmd;
        scan_optional_equals();

        if (n == assign_mu_glue)
          scan_glue(mu_val);
        else
          scan_glue(glue_val);

        trap_zero_glue();

        if ((a >= 4))
          geq_define(p, glue_ref, cur_val);
        else
          eq_define(p, glue_ref, cur_val);
      }
      break;

    case def_code:
      {
        if (cur_chr == cat_code_base)
          n = max_char_code;
        else if (cur_chr == math_code_base)
          n = 32768L;                 /* 2^15 */
        else if (cur_chr == sf_code_base)
          n = 32767;                /* 2^15 - 1*/
        else if (cur_chr == del_code_base)
          n = 16777215L;              /* 2^24 - 1 */
        else
          n = 255;             /* 2^8 - 1 */

        p = cur_chr;
        scan_char_num();
        p = p + cur_val;
        scan_optional_equals();
        scan_int();

        if (((cur_val < 0) && (p < del_code_base)) ||(cur_val > n))
        {
          print_err("Invalid code(");
          print_int(cur_val);

          if (p < del_code_base)
            print_string("), should be in the range 0..");
          else
            print_string("), should be at most ");

          print_int(n);
          help1("I'm going to use 0 instead of that illegal code value.");
          error();
          cur_val = 0;
        }

        if (p < math_code_base)
          if ((a >= 4))
            geq_define(p, data, cur_val);
          else
            eq_define(p, data, cur_val);
        else if (p < del_code_base)
          if ((a >= 4))
            geq_define(p, data, cur_val);
          else
            eq_define(p, data, cur_val);
        else 
          if ((a >= 4))
            geq_word_define(p, cur_val);
          else
            eq_word_define(p, cur_val);
      }
      break;

    case def_family:
      {
        p = cur_chr;
        scan_four_bit_int();
        p = p + cur_val;
        scan_optional_equals();
        scan_font_ident();

        if ((a >= 4))
          geq_define(p, data, cur_val);
        else
          eq_define(p, data, cur_val);
      }
      break;

    case tex_register:
    case advance:
    case multiply:
    case divide:
      do_register_command(a);
      break;

    case set_box:
      {
        scan_eight_bit_int();

        if ((a >= 4))
          n = 256 + cur_val;
        else
          n = cur_val;

        scan_optional_equals();

        if (set_box_allowed)
        {
          scan_box(box_flag + n);
        }
        else
        {
          print_err("Improper ");
          print_esc("setbox");
          help2("Sorry, \\setbox is not allowed after \\halign in a display,",
              "or between \\accent and an accented character.");
          error();
        }
      }
      break;

    case set_aux:
      alter_aux();
      break;

    case set_prev_graf:
      alter_prev_graf();
      break;

    case set_page_dimen:
      alter_page_so_far();
      break;

    case set_page_int:
      alter_integer();
      break;

    case set_box_dimen:
      alter_box_dimen();
      break;

    case set_shape:
      {
        scan_optional_equals();
        scan_int();
        n = cur_val;

        if (n <= 0)
          p = 0;
        else
        {
          p = get_node(2 * n + 1);
          info(p) = n;

          for (j = 1; j <= n; j++)
          {
            scan_dimen(false, false, false);
            mem[p + 2 * j - 1].cint = cur_val;
            scan_dimen(false, false, false);
            mem[p + 2 * j].cint = cur_val;
          }
        }

        if ((a >= 4))
          geq_define(par_shape_loc, shape_ref, p);
        else
          eq_define(par_shape_loc, shape_ref, p);
      }
      break;

    case hyph_data:
      if (cur_chr == 1)
      {
#ifdef INITEX
        if (is_initex)
        {
          new_patterns();
          goto lab30;
        }
#endif /* INITEX */
        print_err("Patterns can be loaded only by INITEX");
        help_ptr = 0;
        error();

        do
          {
            get_token();
          }
        while(!(cur_cmd == right_brace));

        return;
      }
      else
      {
        new_hyph_exceptions();
        goto lab30;
      }
      break;

    case assign_font_dimen:
      {
        find_font_dimen(true);
        k = cur_val;
        scan_optional_equals();
        scan_dimen(false, false, false);
        font_info[k].cint = cur_val;
      }
      break;

    case assign_font_int:
      {
        n = cur_chr;
        scan_font_ident();
        f = cur_val;
        scan_optional_equals();
        scan_int();

        if (n == 0)
          hyphen_char[f] = cur_val;
        else
          skew_char[f] = cur_val;
      }
      break;

    case def_font:
      new_font(a);
      break;

    case set_interaction:
      new_interaction();
      break;

    default:
      {
        confusion("prefix");
        return;       // abort_flag set
      }
      break;
  }

lab30:
  if (after_token != 0)
  {
    cur_tok = after_token;
    back_input();
    after_token = 0;
  }
}

void bad_formator_pool (char *name, char *defaultname, char *envvar)
{
  if (name == NULL)
    name = defaultname;

  sprintf(log_line, "(Perhaps %s is for an older version of TeX)\n", name);
  show_line(log_line, 0);
  name_of_file[name_length + 1] = '\0';
  sprintf(log_line, "(Alternatively, %s may have been corrupted)\n", name_of_file + 1);
  show_line(log_line, 0);
  name_of_file[name_length + 1] = ' ';
  sprintf(log_line, "(Perhaps your %s environment variable is not set correctly)\n", envvar);
  show_line(log_line, 0);

  {
    char *s;

    if ((s = grabenv(envvar)) != NULL)
    {
      sprintf(log_line, "(%s=%s)\n", envvar, s);
      show_line(log_line, 0);
    }
    else
    {
      sprintf(log_line, "%s environment variable not set\n", envvar);
      show_line(log_line, 0);
    }
  }

#ifndef _WINDOWS
  fflush(stdout);
#endif
}
/* sec 1303 */
bool load_fmt_file (void)
{
  integer j, k;
  halfword p, q;
  integer x;

  undump_int(x);

  if (x != BEGINFMTCHECKSUM) /* magic FMT file start 4C 20 E6 15 hex */
    goto lab6666;

  undump_int(x); /* mem_bot */

  if (x != mem_bot)
    goto lab6666;

  undump_int(x); /* mem_top */

#ifdef ALLOCATEMAIN
/* we already read this once earlier to grab mem_top */
  if (trace_flag)
  {
    sprintf(log_line, "Read from fmt file mem_top = %d TeX words\n", x);
    show_line(log_line, 0);
  }

  mem = allocate_main_memory(x); /* allocate main memory at this point */

  if (mem == NULL)
    exit(1);                     /* redundant sanity test ! */

  initialize_aux();              /* do `mem' part of initialize */
/*  mem = zmem; */               /* update pointer to main memory */
#endif

  if (x != mem_top)
    goto lab6666;

  undump_int(x); /* eqtbsize */

  if (x != (eqtb_size))
    goto lab6666;

  undump_int(x); /* hash_prime */

  if (x != hash_prime)
    goto lab6666;

  undump_int(x); /* hyphen_prime */

#ifdef ALLOCATEHYPHEN
/* allow format files dumped with arbitrary (prime) hyphenation exceptions */
  realloc_hyphen(x); /* reset_hyphen(); */
  hyphen_prime = x;
#endif

  if (x != hyphen_prime)
    goto lab6666;

  {
    undump_int(x); /* pool_size */

    if (x < 0)
      goto lab6666; 

#ifdef ALLOCATESTRING
    if (x > current_pool_size)
    {
      if (trace_flag)
      {
        sprintf(log_line, "undump string pool reallocation (%d > %d)\n", x, current_pool_size);
        show_line(log_line, 0);
      }
      str_pool = realloc_str_pool (x - current_pool_size + increment_pool_size);
    }
    if (x > current_pool_size)   /* 94/Jan/24 */
#else
    if (x > pool_size)
#endif
    {
      sprintf(log_line, "%s%s\n",  "---! Must increase the ", "string pool size");
      show_line(log_line, 0);
      goto lab6666;
    }
    else
      pool_ptr = x;
  }
  {
    undump_int(x);  /* max_strings */
    if (x < 0)
      goto lab6666;

#ifdef ALLOCATESTRING
    if (x > current_max_strings)
    {
      if (trace_flag)
      {
        sprintf(log_line, "undump string pointer reallocation (%d > %d)\n", x, current_max_strings);
        show_line(log_line, 0);
      }
      str_start = realloc_str_start(x - current_max_strings + increment_max_strings);
    }
    if (x > current_max_strings) /* 94/Jan/24 */
#else
    if (x > max_strings)
#endif
    {
      sprintf(log_line,  "%s%s\n",  "---! Must increase the ", "max strings");
      show_line(log_line, 0);
      goto lab6666;
    }
    else
      str_ptr = x;
  }

  if (undumpthings(str_start[0], str_ptr + 1)) /* undump string ptrs */
    return -1;
  if (undumpthings(str_pool[0], pool_ptr)) /*  undump string pool */
    return -1;

  init_str_ptr = str_ptr;
  init_pool_ptr = pool_ptr;
/*  undump the dynamic memory - paragraph 1312 in the book */
  {
    undump_int(x);
    if ((x < lo_mem_stat_max + 1000) || (x > hi_mem_stat_min - 1))
      goto lab6666;
    else
      lo_mem_max = x;
  }
  {
    undump_int(x);
    if ((x < lo_mem_stat_max + 1) || (x > lo_mem_max))
      goto lab6666;
    else
      rover = x;
  }
  p = 0;                  /* mem_bot */
  q = rover;

  do
  {
    if (undumpthings(mem[p], q + 2 - p))
      return -1;

    p = q + node_size(q);

    if ((p > lo_mem_max) || ((q >= rlink(q)) && (rlink(q) != rover)))
      goto lab6666;

    q = rlink(q);
  } while (!(q == rover));

  if (undumpthings(mem[p], lo_mem_max + 1 - p))
    return -1;

  if (mem_min < mem_bot - 2)          /*  ? splice in block below */
  {
/*  or call add_variable_space(mem_bot - (mem_min + 1)) */
    if (trace_flag)
      show_line("Splicing in mem_min space in undump!\n", 0);

    p = llink(rover);
    q = mem_min + 1;
    link(mem_min) = 0;       /* null */
    info(mem_min) = 0;       /* null */
    rlink(p) = q;
    llink(rover) = q;
    rlink(q) = rover;
    llink(q) = p;
    link(q) = empty_flag;
    node_size(q) = mem_bot - q;     /* ? size of block  */
  }
  {
    undump_int(x);
    if ((x < lo_mem_max + 1) || (x > hi_mem_stat_min))
      goto lab6666;
    else
      hi_mem_min = x;
  }
  {
    undump_int(x);

    if ((x < mem_bot) || (x > mem_top))
      goto lab6666;
    else
      avail = x;
  }
  mem_end = mem_top;

  if (undumpthings(mem[hi_mem_min], mem_end + 1 - hi_mem_min))
    return -1;

  undump_int(var_used);
  undump_int(dyn_used);

  k = active_base;
  do {
    undump_int(x);
    if ((x < 1) || (k + x > (eqtb_size + 1)))
      goto lab6666;

    if (undumpthings(eqtb[k], x))
      return -1;

    k = k + x;
    undump_int(x);

    if ((x < 0) || (k + x > (eqtb_size + 1)))
      goto lab6666;

    for (j = k; j <= k + x - 1; j++)
    {
      eqtb[j] = eqtb[k - 1];
    }
    k = k + x;
  } while(!(k > (eqtb_size)));
  {
    undump_int(x);
    if ((x < hash_base) || (x > (frozen_control_sequence)))  /*96/Jan/10*/
      goto lab6666;
    else
      par_loc = x;
  }
  par_token = 4095 + par_loc;
  {
    undump_int(x);
    if ((x < hash_base) || (x > (frozen_control_sequence))) /*96/Jan/10*/
      goto lab6666;
    else
      write_loc = x;
  }
  {
    undump_int(x);
    if ((x < hash_base) || (x > (frozen_control_sequence))) /*96/Jan/10*/
      goto lab6666;
    else
      hash_used = x;
  }
  p = hash_base - 1;
  do {
    {
      undump_int(x);
      if ((x < p + 1) || (x > hash_used))
        goto lab6666;
      else
        p = x;
    }
    undump_hh(hash[p]);
  } while (!(p == hash_used));

  if (undumpthings(hash[hash_used + 1], (undefined_control_sequence - 1) - hash_used))
    return -1;

  undump_int(cs_count);     /* cs_count */
  if (trace_flag)
  {
    sprintf(log_line, "itex undump cs_count %d ", cs_count); /* debugging */
    show_line(log_line, 0);
  }
  {
    undump_int(x);        /* font_mem_size */
    if (x < 7)
      goto lab6666;
#ifdef ALLOCATEFONT
    if (trace_flag)
    {
      sprintf(log_line, "Read from fmt fmem_ptr = %d\n", x);
      show_line(log_line, 0);
    }

    if (x > current_font_mem_size) /* 93/Nov/28 dynamic allocate font_info */
    {
      if (trace_flag)
      {
        sprintf(log_line, "Undump realloc font_info (%d > %d)\n", x, current_font_mem_size);
        show_line(log_line, 0);
      }
      font_info = realloc_font_info (x - current_font_mem_size + increment_font_mem_size);
    }
    if (x > current_font_mem_size)  /* in case allocation failed 94/Jan/24 */
#else
    if (x > font_mem_size)
#endif
    {
      sprintf(log_line, "%s%s\n",  "---! Must increase the ", "font mem size");
      show_line(log_line, 0);
      goto lab6666;
    }
    else
      fmem_ptr = x;
  }
  {
    if (undumpthings(font_info[0], fmem_ptr))
      return -1;
    {
      undump_int(x); /* font_max */
      if (x < 0)
        goto lab6666;
      if (x > font_max)
      {
        sprintf(log_line, "%s%s\n",  "---! Must increase the ", "font max"); 
        show_line(log_line, 0);
        goto lab6666;
      }
      else
        font_ptr = x;
    }
    frozen_font_ptr = font_ptr; /* remember number of fonts frozen into format */
    if (undumpthings(font_check[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_size[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_dsize[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_params[0], font_ptr + 1))
      return -1;
    if (undumpthings(hyphen_char[0], font_ptr + 1))
      return -1;
    if (undumpthings(skew_char[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_name[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_area[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_bc[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_ec[0], font_ptr + 1))
      return -1;
    if (undumpthings(char_base[0], font_ptr + 1))
      return -1;
    if (undumpthings(width_base[0], font_ptr + 1))
      return -1;
    if (undumpthings(height_base[0], font_ptr + 1))
      return -1;
    if (undumpthings(depth_base[0], font_ptr + 1))
      return -1;
    if (undumpthings(italic_base[0], font_ptr + 1))
      return -1;
    if (undumpthings(lig_kern_base[0], font_ptr + 1))
      return -1;
    if (undumpthings(kern_base[0], font_ptr + 1))
      return -1;
    if (undumpthings(exten_base[0], font_ptr + 1))
      return -1;
    if (undumpthings(param_base[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_glue[0], font_ptr + 1))
      return -1;
    if (undumpthings(bchar_label[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_bchar[0], font_ptr + 1))
      return -1;
    if (undumpthings(font_false_bchar[0], font_ptr + 1))
      return -1;
  }

/* log not opened yet, so can't show fonts frozen into format */
/* May be able to avoid the following since we switched to */
/* non_address from font_mem_size to 0 96/Jan/15 ??? */

#ifdef ALLOCATEFONT
/* deal with fmt files dumped with *different* font_mem_size 93/Nov/29 */
 {
   int count = 0, oldfont_mem_size = 0;

   for (x = 0; x <= font_ptr; x++)
   {
     if (bchar_label[x] > oldfont_mem_size)
       oldfont_mem_size = bchar_label[x];
   }

/* somewhat arbitrary sanity check ... */
/* if (oldfont_mem_size != font_mem_size && oldfont_mem_size > font_max) { */
   if (oldfont_mem_size != non_address && oldfont_mem_size > font_max) /* 96/Jan/16 */
   {
     for (x = 0; x <= font_ptr; x++)
     {
       if (bchar_label[x] == oldfont_mem_size)
       {
         /* bchar_label[x] = font_mem_size; */
         bchar_label[x] = non_address;  /* 96/Jan/16 */
         count++;
       }
     }
     if (trace_flag)
     {
       sprintf(log_line, "oldfont_mem_size is %d --- hit %d times. Using non_address %d\n",
           oldfont_mem_size, count, non_address);
       show_line(log_line, 0);
     }
   }
 }
#endif
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* undump(0)(hyph_size)(hyph_count); */
  {
    undump_int(x);

    if ((x < 0) || (x > hyphen_prime))
      goto lab6666;
    else
      hyph_count = x;
  }
/* undump hypenation exception tables p.1325 */
  for (k = 1; k <= hyph_count; k++)
  {
    {
      undump_int(x);
      if ((x < 0) || (x > hyphen_prime))
        goto lab6666;
      else
        j = x;
    }
    {
      undump_int(x);
      if ((x < 0) || (x > str_ptr))
        goto lab6666;
      else
        hyph_word[j] = x;
    }
    {
      undump_int(x);
      if ((x < 0) || (x > max_halfword)) /* mem_top ? no p.1325 */
        goto lab6666;
      else
        hyph_list[j] = x;
    }
  }

#ifdef ALLOCATEHYPHEN
/* if user specified new hyphen prime - flush existing exception patterns ! */
/* but, we can reclaim the string storage wasted ... */
  if (is_initex)
  {
    if (new_hyphen_prime != 0)
    {
      realloc_hyphen(new_hyphen_prime); /* reset_hyphen(); */
      hyphen_prime = new_hyphen_prime;
    }
  }
#endif

  {
    undump_int(x);
    if (x < 0)
      goto lab6666;

#ifdef ALLOCATETRIES
    if (!is_initex)
    {
      allocate_tries(x); /* allocate only as much as is needed */
/* trie_size = x; */ /* ??? */
    }
#endif

    if (x > trie_size)
    {
      sprintf(log_line, "%s%s\n",  "---! Must increase the ", "trie size");
      show_line(log_line, 0);
      goto lab6666;
    }
    else
      j = x;
  }
#ifdef INITEX
  if (is_initex) /* bkph */
    trie_max = j;
#endif /* INITEX */
  if (undumpthings(trie_trl[0], j + 1))
    return -1;
  if (undumpthings(trie_tro[0], j + 1))
    return -1;
  if (undumpthings(trie_trc[0], j + 1))
    return -1;
  {
    undump_int(x);
    if (x < 0)
      goto lab6666;
    if (x > trie_op_size)
    {
      sprintf(log_line, "%s%s\n",  "---! Must increase the ", "trie op size");
      show_line(log_line, 0);
      goto lab6666;
    }
    else
      j = x;
  }

#ifdef INITEX
  if (is_initex)   /* bkph */
    trie_op_ptr = j;
#endif /* INITEX */
  
  if (undumpthings(hyf_distance[1], j))
    return -1;
  if (undumpthings(hyf_num[1], j))
    return -1;
  if (undumpthings(hyf_next[1], j))
    return -1;

#ifdef INITEX
  if (is_initex)    /* bkph */
  {
    for (k = 0; k <= 255; k++)
    {
        trie_used[k] = 0;
    }
  }
#endif /* INITEX */
  k = 256;
  while (j > 0) {
    {
      undump_int(x);
      if ((x < 0) || (x > k - 1))
        goto lab6666;
      else
        k = x;
    }
    {
      undump_int(x);
      if ((x < 1) || (x > j))
        goto lab6666;
      else
        x = x;
    }
#ifdef INITEX
    if (is_initex)          /* bkph */
      trie_used[k] = x;
#endif /* INITEX */
/*   j:=j-x; op_start[k]:=qo(j); */
    j = j - x;
    op_start[k] = j;
  }
#ifdef INITEX
  if (is_initex)          /* bkph */
    trie_not_ready = false;
#endif /* INITEX */
  {
    undump_int(x);
    if ((x < batch_mode) || (x > error_stop_mode))
      goto lab6666;
/*    else interaction = x;  */
    if (interaction < batch_mode)    /* may now set in local.c bkph 94/Jan/8 */
      interaction = x;
  }
  {
    undump_int(x);
    if ((x < 0) || (x > str_ptr))
      goto lab6666;
    else
      format_ident = x;
  }
  undump_int(x);
  
  if ((x != ENDFMTCHECKSUM) || feof(fmt_file))
    goto lab6666;

  return true;

lab6666:;
  sprintf(log_line, "(Fatal format file error; I'm stymied)\n");
  show_line(log_line, 1);

  if (!knuth_flag)
    bad_formator_pool(format_file, "the format file", "TEXFORMATS");

  return false;
}
/* sec 1335 */
void final_cleanup (void)
{
  small_number c;

  c = cur_chr;

  if (job_name == 0)
    open_log_file();

  while (input_ptr > 0)
  {
    if (cur_input.state_field == 0)
    {
      end_token_list();
    }
    else
      end_file_reading();
  }

  while (open_parens > 0)
  {
    print_string(" )");
    decr(open_parens);
  }

  if (cur_level > level_one)
  {
    print_nl("(");
    print_esc("end occurred ");
    print_string("inside a group at level ");
    print_int(cur_level - 1);
    print_char(')');
  }

  while (cond_ptr != 0)
  {
    print_nl("(");
    print_esc("end occurred ");
    print_string("when ");
    print_cmd_chr('i', cur_if);

    if (if_line != 0)
    {
      print_string("on line ");
      print_int(if_line);
    }

    print_string(" was incomplete)");
    if_line = mem[cond_ptr + 1].cint;
    cur_if = mem[cond_ptr].hh.b1;
    temp_ptr = cond_ptr;
    cond_ptr = mem[cond_ptr].hh.v.RH;
    free_node(temp_ptr, 2);
  }

  if (history != spotless)
    if (((history == warning_issued) || (interaction < error_stop_mode)))
      if (selector == term_and_log)
      {
        selector = term_only;
        print_nl("(see the transcript file for additional information)");
        selector = term_and_log;
      }

  if (c == 1)
  {
#ifdef INITEX
    if (is_initex)
    {
      for (c = 0; c <= 4; c++)
      {
        if (cur_mark[c] != 0)
          delete_token_ref(cur_mark[c]);
      }

      if (last_glue != max_halfword)
        delete_glue_ref(last_glue);
      store_fmt_file(); // returns a value ?
    }
#endif

    if (!is_initex)
      print_nl("(\\dump is performed only by INITEX)");
  }
}

void show_frozen (void)
{
  int i, j, n;

  fprintf(log_file, "\n");
  fprintf(log_file, "(%d fonts frozen in format file:\n", font_ptr);

  for (i = 1; i <= font_ptr; i++)
  {
    if (i > 1)
      fprintf(log_file, ", ");

    if ((i % 8) == 0)
      fprintf(log_file, "\n");

    n = str_start[font_name[i] + 1] - str_start[font_name[i]];

    for (j = 0; j < n; j++)
    {
      putc(str_pool[str_start[font_name[i]] + j], log_file);
    }
  }

  fprintf(log_file, ") ");
}

int texbody (void)
{
  history = fatal_error_stop;

  if (ready_already == 314159L)
    goto lab1;

  bad = 0;

  if ((half_error_line < 30) || (half_error_line > error_line - 15))
    bad = 1;

  if (max_print_line < 60)
    bad = 2;

  if (dvi_buf_size % 8 != 0)
    bad = 3;

  if (mem_bot + 1100 > mem_top)
    bad = 4;

  if (hash_prime > (hash_size + hash_extra))
    bad = 5;

  if (max_in_open >= 128)
    bad = 6;

  if (mem_top < 256 + 11)
    bad = 7;

#ifdef INITEX
  if (is_initex)
  {
    if ((mem_min != 0) || (mem_max != mem_top))
      bad = 10;
  }
#endif

  if ((mem_min > mem_bot) || (mem_max < mem_top))
    bad = 10;

  if ((min_quarterword > 0) || (max_quarterword < 255))
    bad = 11;

  if ((min_halfword > 0) || (max_halfword < 32767))
    bad = 12;

  if ((min_quarterword < min_halfword) || (max_quarterword > max_halfword))
    bad = 13;

  if ((mem_min < min_halfword) || (mem_max >= max_halfword) || (mem_bot - mem_min >= max_halfword))
    bad = 14;

  if (mem_max > mem_top + mem_extra_high)
    bad = 14;

  if ((0 < min_quarterword) || (font_max > max_quarterword))
    bad = 15;

#ifdef INCREASEFONTS
  if (font_max > 1024)
#else
  if (font_max > 256)
#endif
    bad = 16;

  if ((save_size > max_halfword) || (max_strings > max_halfword))
    bad = 17;

  if (buf_size > max_halfword)
    bad = 18;

  if (max_quarterword - min_quarterword < 255)
    bad = 19;

  if (cs_token_flag + undefined_control_sequence > max_halfword)
    bad = 21;

  if (format_default_length > PATHMAX)
    bad = 31;

  if (max_halfword < (mem_top - mem_min) / 2)
    bad = 41;

  if (bad > 0)
  {
    sprintf(log_line, "%s%s%ld\n", "Ouch---my internal constants have been clobbered!",
        "---case ", (long) bad);
    show_line(log_line, 1);

    if (!knuth_flag)
      bad_formator_pool(format_file, "the format file", "TEXFORMATS");

    goto lab9999;
  }

  initialize();

#ifdef INITEX
  if (is_initex)
  {
    if (!get_strings_started())
      goto lab9999;

    init_prim();
    init_str_ptr = str_ptr;
    init_pool_ptr = pool_ptr;
    fix_date_and_time();
  }
#endif

  ready_already = 314159L;

lab1:
  selector = term_only;
  tally = 0;
  term_offset = 0;
  file_offset = 0;
  show_line(tex_version, 0);

  {
#ifdef _WIN32
  #ifdef _WIN64
    sprintf(log_line, " (%s %s/Windows 64bit)", application, yandyversion);
  #else
    sprintf(log_line, " (%s %s/Windows 32bit)", application, yandyversion);
  #endif
#else
    sprintf(log_line, " (%s %s/Linux)", application, yandyversion);
#endif
  }

  show_line(log_line, 0);

  if (format_ident > 0)
    slow_print(format_ident);

  print_ln();

#ifndef _WINDOWS
  fflush(stdout);
#endif

  job_name = 0;
  name_in_progress = false;
  log_opened = false;
  output_file_name = 0;

  {
    {
      input_ptr = 0;
      max_in_stack = 0;
      in_open = 0;
      high_in_open = 0;
      open_parens = 0;
      max_open_parens = 0;
      max_buf_stack = 0;
      param_ptr = 0;
      max_param_stack = 0;

#ifdef ALLOCATEBUFFER
      memset (buffer, 0, current_buf_size);
#else
      memset (buffer, 0, buf_size);
#endif

      first = 0;
      scanner_status = 0;
      warning_index = 0;
      first = 1;
      cur_input.state_field = 33;
      cur_input.start_field = 1;
      cur_input.index_field = 0;
      line = 0;
      cur_input.name_field = 0;
      force_eof = false;
      align_state = 1000000L;

      if (!init_terminal())
        goto lab9999;

      cur_input.limit_field = last;
      first = last + 1;
    }
    
    if ((format_ident == 0) ||
        (buffer[cur_input.loc_field] == '&') ||
        (buffer[cur_input.loc_field] == '+'))
    {
      if (format_ident != 0)
        initialize();

      if (!open_fmt_file ())
        goto lab9999;

      if (!load_fmt_file ())
      {
        w_close(fmt_file);
        goto lab9999;
      }

      w_close(fmt_file);

      while ((cur_input.loc_field < cur_input.limit_field) &&
          (buffer[cur_input.loc_field] == ' '))
        incr(cur_input.loc_field);
    }

    if ((end_line_char < 0) || (end_line_char > 255))
      decr(cur_input.limit_field);
    else
      buffer[cur_input.limit_field] = end_line_char;

    fix_date_and_time();
    magic_offset = str_start[886] - 9 * ord_noad;

    if (interaction == batch_mode)
      selector = no_print;
    else
      selector = term_only;

    if ((cur_input.loc_field < cur_input.limit_field) &&
      (cat_code(buffer[cur_input.loc_field]) != escape))
      start_input();
  }

  if (show_tfm_flag && log_opened && font_ptr > 0)
    show_frozen();

  main_time = clock();
  history = 0;

  if (show_cs_names)
    print_cs_names(stdout, 0);

  main_control();

  if (show_cs_names)
    print_cs_names(stdout, 1);

  final_cleanup();

  close_files_and_terminate();

lab9999:
  {
    int code;

#ifndef _WINDOWS
    fflush(stdout);
#endif

    ready_already = 0;

    if ((history != 0) && (history != 1))
      code = 1;
    else
      code = 0;

    return code;
  }
}

#ifdef ALLOCATEMAIN
/* add a block of variable size node space below mem_bot */
void add_variable_space(int size)
{
  halfword p;
  halfword q;
  integer t;

  if (mem_min == 0)
    t = mem_min;
  else
    t = mem_min + 1;

  mem_min = t - (size + 1);     /* first word in new block - 1 */

  if (mem_min < mem_start)      /* sanity test */
  {
    if (trace_flag)
      show_line("WARNING: mem_min < mem_start!\n", 0);

    mem_min = mem_start;
  }

  p = llink(rover);
  q = mem_min + 1;
  link(mem_min) = 0; /* insert blank word below ??? */
  info(mem_min) = 0; /* insert blank word below ??? */
  rlink(p) = q;
  llink(rover) = q;
  rlink(q) = rover;
  llink(q) = p;
  link(q) = empty_flag;
  info(q) = t - q; /* block size */
  rover = q;
}
#endif

/**************************************************************************/

#ifdef INITEX
/* split out to allow sharing of code from do_initex and newpattern */
void reset_trie (void)
{
  integer k;

  for (k = - (integer) trie_op_size; k <= trie_op_size; k++)
    trie_op_hash[k] = 0;

  for (k = 0; k <= 255; k++)
    trie_used[k] = min_trie_op;

  max_op_used = min_trie_op;
  trie_op_ptr = 0;
  trie_not_ready = true;
  trie_l[0] = 0;
  trie_c[0] = 0;
  trie_ptr = 0;
  trie_not_ready = true;
}
/* borrowed code from initialize() */
void reset_hyphen (void)
{
  hyph_pointer z;

  for (z = 0; z <= hyphen_prime; z++)
  {
    hyph_word[z] = 0;
    hyph_list[z] = 0;
  }

  hyph_count = 0;
}
/* split out to allow optimize for space, not time */
void do_initex (void)
{
/*  integer i; */
  integer k;
/*  hyph_pointer z; */

  for (k = mem_bot + 1; k <= lo_mem_stat_max; k++)
    mem[k].cint = 0;

  k = mem_bot;

  while (k <= lo_mem_stat_max)
  {
    glue_ref_count(k) = 1;
    stretch_order(k) = normal;
    shrink_order(k) = normal;
    k = k + glue_spec_size;
  }

  stretch(fil_glue) = 65535L;
  stretch_order(fil_glue) = fil;
  stretch(fill_glue) = 65535L;
  stretch_order(fill_glue) = fill;
  stretch(ss_glue) = 65535L;
  stretch_order(ss_glue) = fil;
  shrink(ss_glue) = 65535L;
  shrink_order(ss_glue) = fil;
  stretch(fil_neg_glue) = -65536L;
  stretch_order(fil_neg_glue) = fil;
  rover = lo_mem_stat_max + 1;
  link(rover) = empty_flag;
  node_size(rover) = block_size;
  llink(rover) = rover;
  rlink(rover) = rover;
  lo_mem_max = rover + block_size;
  link(lo_mem_max) = 0;
  info(lo_mem_max) = 0;

  for (k = hi_mem_stat_min; k <= mem_top; k++)
    mem[k] = mem[lo_mem_max];

  info(omit_template) = end_template_token;
  link(end_span) = max_quarterword + 1;
  info(end_span) = 0;
  type(last_active) = hyphenated;
  line_number(last_active) = max_halfword;
  subtype(last_active) = 0;
  subtype(page_ins_head) = 255;
  type(page_ins_head) = split_up;
  link(mem_top) = page_ins_head;
  type(page_head) = glue_node;
  subtype(page_head) = normal;
  avail = 0;                   /* avail <- null p.164 */
  mem_end = mem_top;
  hi_mem_min = hi_mem_stat_min;
  var_used = lo_mem_stat_max + 1 - mem_bot;
  dyn_used = hi_mem_stat_usage;
  eq_type(undefined_control_sequence) = undefined_cs;
  equiv(undefined_control_sequence) = 0;
  eq_level(undefined_control_sequence) = level_zero;

  for (k = active_base; k <= undefined_control_sequence - 1; k++)
    eqtb[k] = eqtb[undefined_control_sequence];

  equiv(glue_base) = zero_glue;
  eq_level(glue_base) = level_one;
  eq_type(glue_base) = glue_ref;

  for (k = glue_base + 1; k <= local_base - 1; k++)
    eqtb[k] = eqtb[glue_base];

  glue_ref_count(zero_glue) = glue_ref_count(zero_glue) + local_base - glue_base;

  par_shape_ptr = 0;
  eq_type(par_shape_loc) = shape_ref;
  eq_level(par_shape_loc) = level_one;

  for (k = output_routine_loc; k <= toks_base + 255; k++)
    eqtb[k] = eqtb[undefined_control_sequence];

  box(0) = 0;
  eq_type(box_base) = box_ref;
  eq_level(box_base) = level_one;

  for (k = box_base + 1; k <= box_base + 255; k++)
    eqtb[k] = eqtb[box_base];

  cur_font = null_font;
  eq_type(cur_font_loc) = data;
  eq_level(cur_font_loc) = level_one;

  for (k = math_font_base; k <= math_font_base + 47; k++)
    eqtb[k] = eqtb[cur_font_loc];

  equiv(cat_code_base) = 0;
  eq_type(cat_code_base) = data;
  eq_level(cat_code_base) = level_one;

  for (k = cat_code_base; k <= int_base - 1; k++)
    eqtb[k] = eqtb[cat_code_base];

  for (k = 0; k <= 255; k++)
  {
    cat_code(k) = other_char;
    math_code(k) = k;
    sf_code(k) = 1000;
  }

  cat_code(carriage_return) = car_ret;
  cat_code(' ') = spacer;
  cat_code('\\') = escape;
  cat_code('%') = comment;
  cat_code(invalid_code) = invalid_char;
  cat_code(null_code) = ignore;

  for (k = '0'; k <= '9'; k++)
    math_code(k) = k + var_code;

  for (k = 'A'; k <= 'Z'; k++)
  {
    cat_code(k) = letter;
    cat_code(k + 'a' - 'A') = letter;
    math_code(k) = k + var_code + 0x100;
    math_code(k + 'a' - 'A') = k + 'a' - 'A' + var_code + 0x100;
    lc_code(k) = k + 'a' - 'A';
    lc_code(k + 'a' - 'A') = k + 'a' - 'A';
    uc_code(k) = k;
    uc_code(k + 'a' - 'A') = k;
    sf_code(k) = 999;
  }

  for (k = int_base; k <= del_code_base - 1; k++)
    eqtb[k].cint = 0;

  mag = 1000;
  tolerance = 10000;
  hang_after = 1;
  max_dead_cycles = 25;
  escape_char = '\\';
  end_line_char = carriage_return;

  for (k = 0; k <= 255; k++)
    del_code(k) = -1;

  del_code('.') = 0;

  for (k = dimen_base; k <= eqtb_size; k++)
    eqtb[k].cint = 0;

  hash_used = frozen_control_sequence;
  cs_count = 0;

  if (trace_flag)
    show_line("initex cs_count = 0 ", 0);

  eq_type(frozen_dont_expand) = dont_expand;
  text(frozen_dont_expand) = 499;  /* notexpanded */

  font_ptr                    = null_font;
  fmem_ptr                    = 7;
  font_name[null_font]        = 795; /* nullfont */
  font_area[null_font]        = 335; /* "" */
  hyphen_char[null_font]      = '-';
  skew_char[null_font]        = -1; 
  bchar_label[null_font]      = non_address;
  font_bchar[null_font]       = non_char;
  font_false_bchar[null_font] = non_char;
  font_bc[null_font]          = 1;
  font_ec[null_font]          = 0;
  font_size[null_font]        = 0;
  font_dsize[null_font]       = 0;
  char_base[null_font]        = 0;
  width_base[null_font]       = 0;
  height_base[null_font]      = 0;
  depth_base[null_font]       = 0;
  italic_base[null_font]      = 0;
  lig_kern_base[null_font]    = 0;
  kern_base[null_font]        = 0;
  exten_base[null_font]       = 0;
  font_glue[null_font]        = 0;
  font_params[null_font]      = 7;
  param_base[null_font]       = -1;
  reset_trie();
  text(frozen_protection) = 1184; /* "inaccessible" */
  format_ident = 1251;
  text(end_write) = 1290;
  eq_level(end_write) = level_one;
  eq_type(end_write) = outer_call;
  equiv(end_write) = 0;
}
#endif

#ifdef INITEX
/* sec 0047 */
bool get_strings_started (void)
{
  integer k;
  str_number g;

  pool_ptr = 0;
  str_ptr = 0;
  str_start[0] = 0;

  for (k = 0; k <= 255; k++)
  {
    if (((k < ' ') || (k > '~')))
    {
      append_char('^');
      append_char('^');

      if (k < 64)
        append_char(k + 64);
      else
        if (k < 128)
          append_char(k - 64);
        else
        {
          append_lc_hex(k / 16);
          append_lc_hex(k % 16);
        }
    }
    else
      append_char(k);

    g = make_string();
  }

  g = load_pool_strings(pool_size - string_vacancies);

  if (g == 0)
  {
    fprintf(stdout , "%s\n",  "! You have to increase POOLSIZE." );
    return false;
  }

  return true;
}
#endif /* INITEX */

#ifdef INITEX
/* sec 0131 */
void sort_avail (void)
{
  halfword p, q, r;
  halfword old_rover;

  p = get_node(1073741824L); /* 2^30 merge adjacent free nodes */
  p = rlink(rover);
  rlink(rover) = empty_flag;
  old_rover = rover;

  while (p != old_rover)
  {
    if (p < rover)
    {
      q = p;
      p = rlink(q);
      rlink(q) = rover;
      rover = q;
    }
    else
    {
      q = rover;

      while (rlink(q) < p)
        q = rlink(q);

      r = rlink(p);
      rlink(p) = rlink(q);
      rlink(q) = p;
      p = r;
    }
  }

  p = rover;

  while (rlink(p) != empty_flag)
  {
    llink(rlink(p)) = p;
    p = rlink(p);
  }

  rlink(p) = rover;
  llink(rover) = p;
}
#endif /* INITEX */

#ifdef INITEX
str_number make_string_pool (char *s)
{
  int slen = strlen(s);

  if (slen == 1)
  {
    return ((str_number)s[0]);
  }
  else
  {
    memcpy(str_pool + pool_ptr, s, slen);
    pool_ptr += slen;
    return (make_string());
  }
}
/* sec 0264 */
void primitive_ (str_number s, quarterword c, halfword o)
{ 
  pool_pointer k;
  small_number j;
/*  small_number l;  */
  int l; /* 95/Jan/7 */

  if (s < 256)
    cur_val = s + single_base;
  else
  {
    k = str_start[s];
    l = str_start[s + 1] - k; /* small_number l */

    for (j = 0; j <= l - 1; j++)
      buffer[j] = str_pool[k + j];

    cur_val = id_lookup(0, l);
    flush_string();

#ifdef SHORTHASH
    if (s > 65535L)
      show_line("ERROR: hash entry too large\n", 1);
#endif

    text(cur_val) = s;
  }

  eq_level(cur_val) = level_one;
  eq_type(cur_val) = c;
  equiv(cur_val) = o;
}
#endif

#ifdef INITEX
/* sec 0944 */
trie_op_code new_trie_op_ (small_number d, small_number n, trie_op_code v)
{
  integer h;
  trie_op_code u;
  integer l;

  h = abs(n + 313 * d + 361 * v + 1009 * cur_lang) % (trie_op_size + trie_op_size) + neg_trie_op_size;

  while (true)
  {
    l = trie_op_hash[h];

    if (l == 0)
    {
      if (trie_op_ptr == trie_op_size)
      {
        overflow("pattern memory ops", trie_op_size);
        return 0;     // abort_flag set
      }

      u = trie_used[cur_lang];

      if (u == max_trie_op)
      {
        overflow("pattern memory ops per language", max_trie_op - min_trie_op);
        return 0;     // abort_flag set
      }

      incr(trie_op_ptr);
      incr(u);
      trie_used[cur_lang] = u;

      if (u > max_op_used)
        max_op_used = u;

      hyf_distance[trie_op_ptr] = d;
      hyf_num[trie_op_ptr] = n;
      hyf_next[trie_op_ptr] = v;
      trie_op_lang[trie_op_ptr] = cur_lang;
      trie_op_hash[h] = trie_op_ptr;
      trie_op_val[trie_op_ptr] = u;
      return u;
    }

    if ((hyf_distance[l]== d) && (hyf_num[l]== n) && (hyf_next[l]== v) && (trie_op_lang[l]== cur_lang))
    {
      return trie_op_val[l];
    }

    if (h > - (integer) trie_op_size)
      decr(h);
    else
      h = trie_op_size;
  }
}
/* sec 0948 */
trie_pointer trie_node_ (trie_pointer p)
{
  trie_pointer h;
  trie_pointer q;

  /* the 1009, 2718, 3142 are hard-wired constants here (not hyphen_prime) */
  /* compute hash value */
  h = abs(trie_c[p] + 1009 * trie_o[p] + 2718 * trie_l[p] + 3142 * trie_r[p]) % trie_size;

  while (true)
  {
    q = trie_hash[h];

    if (q == 0)
    {
      trie_hash[h] = p;
      return p;
    }

    if ((trie_c[q] == trie_c[p]) && (trie_o[q] == trie_o[p]) &&
      (trie_l[q] == trie_l[p]) && (trie_r[q] == trie_r[p]))
    {
      return q;
    }

    if (h > 0)
      decr(h);
    else
      h = trie_size;
  }
}
/* sec 0949 */
trie_pointer compress_trie_ (trie_pointer p)
{
  if (p == 0)
    return 0;
  else
  {
    trie_l[p] = compress_trie(trie_l[p]);
    trie_r[p] = compress_trie(trie_r[p]);
    return trie_node(p);
  }
}
/* sec 0953 */
void first_fit_ (trie_pointer p)
{
  trie_pointer h;
  trie_pointer z;
  trie_pointer q;
  ASCII_code c;
  trie_pointer l, r;
  short ll;

  c = trie_c[p];
  z = trie_min[c];

  while (true)
  {
    h = z - c;

    if (trie_max < h + 256)
    {
      if (trie_size <= h + 256)
      {
        overflow("pattern memory", trie_size);
/*      not dynamic ---- but can be set -h=... from command line in ini-TeX */
        return;     // abort_flag set
      }

      do
        {
          incr(trie_max);
          trie_taken[trie_max] = false;
          trie_trl[trie_max] = trie_max + 1;
          trie_tro[trie_max] = trie_max - 1;
        }
      while (!(trie_max == h + 256));
    }

    if (trie_taken[h])
      goto lab45;

    q = trie_r[p];

    while (q > 0)
    {
      if (trie_trl[h + trie_c[q]] == 0)
        goto lab45;

      q = trie_r[q];
    }

    goto lab40;
lab45:
    z = trie_trl[z];
  }
lab40:
  trie_taken[h] = true; /* h may be used without ... */
  trie_hash[p] = h;
  q = p;

  do {
    z = h + trie_c[q];
    l = trie_tro[z];
    r = trie_trl[z];
    trie_tro[r] = l;
    trie_trl[l] = r;
    trie_trl[z] = 0;

    if (l < 256)
    {
      if (z < 256)
        ll = z;         /* short ll */
      else
        ll = 256;

      do {
        trie_min[l] = r;
        incr(l);
      } while (!(l == ll));
    }
    q = trie_r[q];
  } while (!(q == 0));
}
/* sec 0957 */
void trie_pack_ (trie_pointer p)
{
  trie_pointer q;

  do
    {
      q = trie_l[p];

      if ((q > 0) && (trie_hash[q]== 0))
      {
        first_fit(q);
        trie_pack(q);
      }

      p = trie_r[p];
    }
  while(!(p == 0));
}
/* sec 0959 */
void trie_fix_ (trie_pointer p)
{
  trie_pointer q;
  ASCII_code c;
  trie_pointer z;

  z = trie_hash[p];

  do
    {
      q = trie_l[p];
      c = trie_c[p];
      trie_trl[z + c] = trie_hash[q];
      trie_trc[z + c] = c;
      trie_tro[z + c] = trie_o[p];

      if (q > 0)
        trie_fix(q);

      p = trie_r[p];
    }
  while(!(p == 0));
}
/* sec 0960 */
void new_patterns (void)
{
/* ******************************************************************* */
/*  was small_number k, l;  in 3.141 */
  char k, l;
/* ******************************************************************* */
  bool digitsensed;
  trie_op_code v;
  trie_pointer p, q;
  bool firstchild;
/*  ASCII_code c;  */
  int c;                /* 95/Jan/7 */

  if (!trie_not_ready) /* new stuff */
  {
    if (allow_patterns)
    {
      if (trace_flag)
        show_line("Resetting patterns\n", 0);

      reset_trie();         /* RESET PATTERNS -  93/Nov/26 */

      if (reset_exceptions)
      {
        if (trace_flag)
          show_line("Resetting exceptions\n", 0);

        reset_hyphen();     /* RESET HYPHENEXCEPTIONS -  93/Nov/26 */
      }
    }
  }
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  if (trie_not_ready)
  {
    set_cur_lang();
    scan_left_brace();
    k = 0;
    hyf[0] = 0;
    digitsensed = false;

    while (true)
    {
      get_x_token();

      switch (cur_cmd)
      {
        case letter:
        case other_char:
          if (digitsensed || (cur_chr < '0') || (cur_chr > '9'))
          {
            if (cur_chr == '.')
              cur_chr = 0;
            else
            {
              cur_chr = lc_code(cur_chr);

              if (cur_chr == 0)
              {
                print_err("Nonletter");
                help1("(See Appendix H.)");
                error();
              }
            }

            if (k < 63)
            {
              incr(k);
              hc[k] = cur_chr;
              hyf[k] = 0;
              digitsensed = false;
            }
          }
          else if (k < 63)
          {
            hyf[k] = cur_chr - '0';
            digitsensed = true;
          }
          break;
        case spacer:
        case right_brace:
          {
            if (k > 0)
            {
              if (hc[1] == 0)
                hyf[0] = 0;

              if (hc[k] == 0)
                hyf[k] = 0;

              l = k;
              v = min_trie_op;

              while (true) {
                if (hyf[l]!= 0)
                  v = new_trie_op(k - l, hyf[l], v);

                if (l > 0)
                  decr(l);
                else
                  goto lab31;
              }
lab31:
              q = 0;
              hc[0] = cur_lang;

              while (l <= k) {
                c = hc[l];
                incr(l);
                p = trie_l[q];
                firstchild = true;

                while ((p > 0) && (c > trie_c[p])) {
                  q = p;
                  p = trie_r[q];
                  firstchild = false;
                }

                if ((p == 0) || (c < trie_c[p]))
                {
                  if (trie_ptr == trie_size)
                  {
                    overflow("pattern memory", trie_size);  /* pattern memory - NOT DYNAMIC */
/*      not dynamic ---- but can be set -h=... from command line in ini-TeX */
                    return;     // abort_flag set
                  }

                  incr(trie_ptr);
                  trie_r[trie_ptr] = p;
                  p = trie_ptr;
                  trie_l[p] = 0;

                  if (firstchild)
                    trie_l[q]= p;
                  else
                    trie_r[q]= p;

                  trie_c[p] = c;
                  trie_o[p] = min_trie_op;
                }
                q = p;
              }

              if (trie_o[q]!= min_trie_op)
              {
                print_err("Duplicate pattern");
                help1("(See Appendix H.)");
                error();
              }

              trie_o[q]= v;
            }

            if (cur_cmd == right_brace)
              goto lab30;

            k = 0;
            hyf[0] = 0;
            digitsensed = false;
          }
          break;
        default:
          {
            print_err("Bad ");
            print_esc("patterns");
            help1("(See Appendix H.)");
            error();
          }
          break;
      }
    }
lab30:;
  }
  else
  {
    print_err("Too late for ");
    print_esc("patterns");
    help1("All patterns must be given before typesetting begins.");
    error();
    link(garbage) = scan_toks(false, false);
    flush_list(def_ref);
  }
}
/* sec 0966 */
void init_trie (void)
{
  trie_pointer p;
/*  integer j, k, t;  */
  integer j, k;
  int t;                  /* 95/Jan/7 */
  trie_pointer r, s;

  op_start[0] = - (integer) min_trie_op;

  for (j = 1; j <= 255; j++)
  {
    op_start[j] = op_start[j - 1] + trie_used[j - 1];
  }

  for (j = 1; j <= trie_op_ptr; j++)
  {
    trie_op_hash[j] = op_start[trie_op_lang[j]] + trie_op_val[j];
  }

  for (j = 1; j <= trie_op_ptr; j++)
  {
    while (trie_op_hash[j] > j)
    {
      k = trie_op_hash[j];
      t = hyf_distance[k];
      hyf_distance[k] = hyf_distance[j];
      hyf_distance[j] = t;
      t = hyf_num[k];
      hyf_num[k] = hyf_num[j];
      hyf_num[j] = t;
      t = hyf_next[k];
      hyf_next[k] = hyf_next[j];
      hyf_next[j]= t;
      trie_op_hash[j] = trie_op_hash[k];
      trie_op_hash[k] = k;
    }
  }

  for (p = 0; p <= trie_size; p++)
  {
    trie_hash[p] = 0;
  }

  trie_l[0] = compress_trie(trie_l[0]);

  for (p = 0; p <= trie_ptr; p++)
  {
    trie_hash[p]= 0;
  }

  for (p = 0; p <= 255; p++)
  {
    trie_min[p] = p + 1;
  }

  trie_trl[0] = 1;
  trie_max = 0;

  if (trie_l[0] != 0)
  {
    first_fit(trie_l[0]);
    trie_pack(trie_l[0]);
  }

  if (trie_l[0] == 0)
  {
    for (r = 0; r <= 256; r++)
    {
      trie_trl[r] = 0;
      trie_tro[r] = min_trie_op;
      trie_trc[r] = 0;
    }
    trie_max = 256;
  }
  else
  {
    trie_fix(trie_l[0]);
    r = 0;

    do
      {
        s = trie_trl[r];
        {
          trie_trl[r] = 0;
          trie_tro[r] = min_trie_op;
          trie_trc[r] = 0;
        }

        r = s;
      }
    while(!(r > trie_max));
  }
  trie_trc[0] = 63;
  trie_not_ready = false;
}
#endif /* INITEX */

#ifdef INITEX
/* sec 1302 */
void store_fmt_file (void)
{
  integer j, k, l;
  halfword p, q;
  integer x;

  if (!is_initex)   /* redundant check 94/Feb/14 */
  {
    show_line("! \\dump is performed only by INITEX\n", 1);

    if (! knuth_flag)
      show_line("  (Use -i on the command line)\n", 0);

    abort_flag++;
    return;
  }

  if (save_ptr != 0)
  {
    print_err("You can't dump inside a group");
    help1("`{...\\dump}' is a no-no.");
    succumb();
  }

  selector = new_string;
  print_string(" (format=");
  print(job_name);
  print_char(' ');
  print_int(year);
  print_char('.');
  print_int(month);
  print_char('.');
  print_int(day);
  print_char(')');

  if (interaction == 0)
    selector = log_only;
  else
    selector = term_and_log;

  str_room(1);
  format_ident = make_string();
  pack_job_name(".fmt");

  while(!w_open_out(fmt_file))
  {
    prompt_file_name("format file name", ".fmt");
  }

  print_nl("Beginning to dump on file ");
  slow_print(w_make_name_string(fmt_file));
  flush_string();
  print_nl("");
  slow_print(format_ident);

  dump_int(BEGINFMTCHECKSUM); /* magic FMT file start 4C 20 E6 15 hex */

  dump_int(mem_bot);
  dump_int(mem_top);
  dump_int(eqtb_size);
  dump_int(hash_prime);
  dump_int(hyphen_prime);   /* bkph */

  dump_int(pool_ptr);
  dump_int(str_ptr);

  if (dumpthings(str_start[0], str_ptr + 1))
    return;

  if (dumpthings(str_pool[0], pool_ptr))
    return;

  print_ln();
  print_int(str_ptr);
  print_string(" strings of total length ");
  print_int(pool_ptr);

  sort_avail();
  var_used = 0;
  dump_int(lo_mem_max);
  dump_int(rover);
  p = 0;
  q = rover;
  x = 0;

  do
    {
      if (dumpthings(mem[p], q + 2 - p))
        return;

      x = x + q + 2 - p;
      var_used = var_used + q - p;
      p = q + mem[q].hh.v.LH;
      q = mem[q + 1].hh.v.RH;
    }
  while (!(q == rover));

  var_used = var_used + lo_mem_max - p;
  dyn_used = mem_end + 1 - hi_mem_min;

  if (dumpthings(mem[p], lo_mem_max + 1 - p))
    return;

  x = x + lo_mem_max + 1 - p;
  dump_int(hi_mem_min);
  dump_int(avail);

  if (dumpthings(mem[hi_mem_min], mem_end + 1 - hi_mem_min))
    return;

  x = x + mem_end + 1 - hi_mem_min;
  p = avail;

  while (p != 0)
  {
    decr(dyn_used);
    p = mem[p].hh.v.RH;
  }

  dump_int(var_used);
  dump_int(dyn_used);
  print_ln();
  print_int(x);
  print_string(" memory locations dumped; current usage is ");
  print_int(var_used);
  print_char('&');
  print_int(dyn_used);

  k = active_base;
  do
    {
      j = k;

      while (j < (int_base - 1))
      {
        if ((equiv(j) == equiv(j + 1)) &&
          (eq_type(j) == eq_type(j + 1)) &&
          (eq_level(j) == eq_level(j + 1)))
          goto lab41;
        incr(j);
      }

      l = (int_base);
      goto lab31;
lab41:
      incr(j);
      l = j;
      while (j < (int_base - 1))
      {
        if ((equiv(j) != equiv(j + 1)) ||
          (eq_type(j) != eq_type(j + 1)) ||
          (eq_level(j) != eq_level(j + 1)))
          goto lab31;
        incr(j);
      }
lab31:
      dump_int(l - k);

      if (dumpthings(eqtb[k], l - k))
        return;

      k = j + 1;
      dump_int(k - l);
    }
  while (!(k == (int_base)));

  do {
    j = k;
    while (j < (eqtb_size)) {
      if (eqtb[j].cint == eqtb[j + 1].cint)
        goto lab42;
      incr(j);
    }
    l = (eqtb_size + 1);
    goto lab32;
lab42:
    incr(j);
    l = j;
    while (j < (eqtb_size)) {
      if (eqtb[j].cint != eqtb[j + 1].cint)
        goto lab32;
      incr(j);
    }
lab32:
    dump_int(l - k);
    if (dumpthings(eqtb[k], l - k))
      return;
    k = j + 1;
    dump_int(k - l);
  } while (!(k > (eqtb_size)));

  dump_int(par_loc);
  dump_int(write_loc);

  dump_int(hash_used);
  cs_count = frozen_control_sequence - 1 - hash_used;

  if (trace_flag)
  {
    sprintf(log_line, "itex cs_count %d hash_size %d hash_extra %d hash_used %d",
        cs_count, hash_size, hash_extra, hash_used);
    show_line(log_line, 0);
  }

  for (p = hash_base; p <= hash_used; p++)
  {
    if (hash[p].v.RH != 0)
    {
      dump_int(p);
      dump_hh(hash[p]);
      incr(cs_count);

      if (trace_flag)
      {
        sprintf(log_line, "itex cs_count++ ");
        show_line(log_line, 0); /* debugging */
      }

    }
  }
/*  ??? */
/* for p <- hash_used+1 to undefined_control_sequence-1 do dump_hh(hash[p]) */
  if (dumpthings(hash[hash_used + 1], (hash_size + 780) - hash_used))
    return;

  dump_int(cs_count);
  print_ln();
  print_int(cs_count);
  print_string(" multiletter control sequences");

  dump_int(fmem_ptr);
  {
    if (dumpthings(font_info[0], fmem_ptr))
      return;

    dump_int(font_ptr);

    if (dumpthings(font_check[0], font_ptr + 1))
      return;

    if (dumpthings(font_size[0], font_ptr + 1))
      return;

    if (dumpthings(font_dsize[0], font_ptr + 1))
      return;

    if (dumpthings(font_params[0], font_ptr + 1))
      return;

    if (dumpthings(hyphen_char[0], font_ptr + 1))
      return;

    if (dumpthings(skew_char[0], font_ptr + 1))
      return;

    if (dumpthings(font_name[0], font_ptr + 1))
      return;

    if (dumpthings(font_area[0], font_ptr + 1))
      return;

    if (dumpthings(font_bc[0], font_ptr + 1))
      return;

    if (dumpthings(font_ec[0], font_ptr + 1))
      return;

    if (dumpthings(char_base[0], font_ptr + 1))
      return;

    if (dumpthings(width_base[0], font_ptr + 1))
      return;

    if (dumpthings(height_base[0], font_ptr + 1))
      return;

    if (dumpthings(depth_base[0], font_ptr + 1))
      return;

    if (dumpthings(italic_base[0], font_ptr + 1))
      return;

    if (dumpthings(lig_kern_base[0], font_ptr + 1))
      return;

    if (dumpthings(kern_base[0], font_ptr + 1))
      return;

    if (dumpthings(exten_base[0], font_ptr + 1))
      return;

    if (dumpthings(param_base[0], font_ptr + 1))
      return;

    if (dumpthings(font_glue[0], font_ptr + 1))
      return;

    if (dumpthings(bchar_label[0], font_ptr + 1))
      return;

    if (dumpthings(font_bchar[0], font_ptr + 1))
      return;

    if (dumpthings(font_false_bchar[0], font_ptr + 1))
      return;

    for (k = 0; k <= font_ptr; k++)
    {
      print_nl("\\font");
/*  print_esc(hash[(hash_size + 524) + k].v.RH);  */
/*  print_esc(hash[(hash_size + hash_extra + 524) + k].v.RH); */
      print_esc("");print(hash[(hash_size + hash_extra + 524) + k].v.RH);
      print_char('=');
      print_file_name(font_name[k], font_area[k], 335);

      if (font_size[k] != font_dsize[k])
      {
        print_string(" at ");
        print_scaled(font_size[k]);
        print_string("pt");
      }
    }
  }

  print_ln();
  print_int(fmem_ptr - 7);
  print_string(" words of font info for ");
  print_int(font_ptr - 0);
  print_string(" preloaded font");

  if (font_ptr != 1)
    print_char('s');

  dump_int(hyph_count);

  for (k = 0; k <= hyphen_prime; k++)
  {
    if (hyph_word[k]!= 0)
    {
      dump_int(k);
      dump_int(hyph_word[k]);
      dump_int(hyph_list[k]);
    }
  }

  print_ln();
  print_int(hyph_count);
  print_string(" hyphenation exception");

  if (hyph_count != 1)
    print_char('s');

  if (trie_not_ready)
    init_trie();

  dump_int(trie_max);

  if (dumpthings(trie_trl[0], trie_max + 1))
    return;

  if (dumpthings(trie_tro[0], trie_max + 1))
    return;

  if (dumpthings(trie_trc[0], trie_max + 1))
    return;

  dump_int(trie_op_ptr);

  if (dumpthings(hyf_distance[1], trie_op_ptr))
    return;

  if (dumpthings(hyf_num[1], trie_op_ptr))
    return;

  if (dumpthings(hyf_next[1], trie_op_ptr))
    return;

  print_nl("Hyphenation trie of length ");
  print_int(trie_max);
  print_string(" has ");
  print_int(trie_op_ptr);
  print_string(" op");

  if (trie_op_ptr != 1)
    print_char('s');

  print_string(" out of ");
  print_int(trie_op_size);

  for (k = 255; k >= 0; k--)
  {
    if (trie_used[k] > 0)
    {
      print_nl("  ");
      print_int(trie_used[k]);
      print_string(" for language ");
      print_int(k);
      dump_int(k);
      dump_int(trie_used[k]);
    }
  }
  dump_int(interaction);
  dump_int(format_ident);
  dump_int(ENDFMTCHECKSUM); /* magic checksum end of FMT file --- change ??? */ 
  tracing_stats = 0;

  w_close(fmt_file);
}
#endif /* INITEX */

#ifdef INITEX
/* sec 01336 */
void init_prim (void)
{
  no_new_control_sequence = false;
  primitive("lineskip", assign_glue, glue_base + line_skip_code);
  primitive("baselineskip", assign_glue, glue_base + baseline_skip_code);
  primitive("parskip", assign_glue, glue_base + par_skip_code);
  primitive("abovedisplayskip", assign_glue, glue_base + above_display_skip_code);
  primitive("belowdisplayskip", assign_glue, glue_base + below_display_skip_code);
  primitive("abovedisplayshortskip", assign_glue, glue_base + above_display_short_skip_code);
  primitive("belowdisplayshortskip", assign_glue, glue_base + below_display_short_skip_code);
  primitive("leftskip", assign_glue, glue_base + left_skip_code);
  primitive("rightskip", assign_glue, glue_base + right_skip_code);
  primitive("topskip", assign_glue, glue_base + top_skip_code);
  primitive("splittopskip", assign_glue, glue_base + split_top_skip_code);
  primitive("tabskip", assign_glue, glue_base + tab_skip_code);
  primitive("spaceskip", assign_glue, glue_base + space_skip_code);
  primitive("xspaceskip", assign_glue, glue_base + xspace_skip_code);
  primitive("parfillskip", assign_glue, glue_base + par_fill_skip_code);
  primitive("thinmuskip", assign_mu_glue, glue_base + thin_mu_skip_code);
  primitive("medmuskip", assign_mu_glue, glue_base + med_mu_skip_code);
  primitive("thickmuskip", assign_mu_glue, glue_base + thick_mu_skip_code);
  primitive("output", assign_toks, output_routine_loc);
  primitive("everypar", assign_toks, every_par_loc);
  primitive("everymath", assign_toks, every_math_loc);
  primitive("everydisplay", assign_toks, every_display_loc);
  primitive("everyhbox", assign_toks, every_hbox_loc);
  primitive("everyvbox", assign_toks, every_vbox_loc);
  primitive("everyjob", assign_toks, every_job_loc);
  primitive("everycr", assign_toks, every_cr_loc);
  primitive("errhelp", assign_toks, err_help_loc);
  primitive("pretolerance", assign_int, int_base + pretolerance_code);
  primitive("tolerance", assign_int, int_base + tolerance_code);
  primitive("linepenalty", assign_int, int_base + line_penalty_code);
  primitive("hyphenpenalty", assign_int, int_base + hyphen_penalty_code);
  primitive("exhyphenpenalty", assign_int, int_base + ex_hyphen_penalty_code);
  primitive("clubpenalty", assign_int, int_base + club_penalty_code);
  primitive("widowpenalty", assign_int, int_base + widow_penalty_code);
  primitive("displaywidowpenalty", assign_int, int_base + display_widow_penalty_code);
  primitive("brokenpenalty", assign_int, int_base + broken_penalty_code);
  primitive("binoppenalty", assign_int, int_base + bin_op_penalty_code);
  primitive("relpenalty", assign_int, int_base + rel_penalty_code);
  primitive("predisplaypenalty", assign_int, int_base + pre_display_penalty_code);
  primitive("postdisplaypenalty", assign_int, int_base + post_display_penalty_code);
  primitive("interlinepenalty", assign_int, int_base + inter_line_penalty_code);
  primitive("doublehyphendemerits", assign_int, int_base + double_hyphen_demerits_code);
  primitive("finalhyphendemerits", assign_int, int_base + final_hyphen_demerits_code);
  primitive("adjdemerits", assign_int, int_base + adj_demerits_code);
  primitive("mag", assign_int, int_base + mag_code);
  primitive("delimiterfactor", assign_int, int_base + delimiter_factor_code);
  primitive("looseness", assign_int, int_base + looseness_code);
  primitive("time", assign_int, int_base + time_code);
  primitive("day", assign_int, int_base + day_code);
  primitive("month", assign_int, int_base + month_code);
  primitive("year", assign_int, int_base + year_code);
  primitive("showboxbreadth", assign_int, int_base + show_box_breadth_code);
  primitive("showboxdepth", assign_int, int_base + show_box_depth_code);
  primitive("hbadness", assign_int, int_base + hbadness_code);
  primitive("vbadness", assign_int, int_base + vbadness_code);
  primitive("pausing", assign_int, int_base + pausing_code);
  primitive("tracingonline", assign_int, int_base + tracing_online_code);
  primitive("tracingmacros", assign_int, int_base + tracing_macros_code);
  primitive("tracingstats", assign_int, int_base + tracing_stats_code);
  primitive("tracingparagraphs", assign_int, int_base + tracing_paragraphs_code);
  primitive("tracingpages", assign_int, int_base + tracing_pages_code);
  primitive("tracingoutput", assign_int, int_base + tracing_output_code);
  primitive("tracinglostchars", assign_int, int_base + tracing_lost_chars_code);
  primitive("tracingcommands", assign_int, int_base + tracing_commands_code);
  primitive("tracingrestores", assign_int, int_base + tracing_restores_code);
  primitive("uchyph", assign_int, int_base + uc_hyph_code);
  primitive("outputpenalty", assign_int, int_base + output_penalty_code);
  primitive("maxdeadcycles", assign_int, int_base + max_dead_cycles_code);
  primitive("hangafter", assign_int, int_base + hang_after_code);
  primitive("floatingpenalty", assign_int, int_base + floating_penalty_code);
  primitive("globaldefs", assign_int, int_base + global_defs_code);
  primitive("fam", assign_int, int_base + cur_fam_code);
  primitive("escapechar", assign_int, int_base + escape_char_code);
  primitive("defaulthyphenchar", assign_int, int_base + default_hyphen_char_code);
  primitive("defaultskewchar", assign_int, int_base + default_skew_char_code);
  primitive("endlinechar", assign_int, int_base + end_line_char_code);
  primitive("newlinechar", assign_int, int_base + new_line_char_code);
  primitive("language", assign_int, int_base + language_code);
  primitive("lefthyphenmin", assign_int, int_base + left_hyphen_min_code);
  primitive("righthyphenmin", assign_int, int_base + right_hyphen_min_code);
  primitive("holdinginserts", assign_int, int_base + holding_inserts_code);
  primitive("errorcontextlines", assign_int, int_base + error_context_lines_code);
  primitive("parindent", assign_dimen, dimen_base + par_indent_code);
  primitive("mathsurround", assign_dimen, dimen_base + math_surround_code);
  primitive("lineskiplimit", assign_dimen, dimen_base + line_skip_limit_code);
  primitive("hsize", assign_dimen, dimen_base + hsize_code);
  primitive("vsize", assign_dimen, dimen_base + vsize_code);
  primitive("maxdepth", assign_dimen, dimen_base + max_depth_code);
  primitive("splitmaxdepth", assign_dimen, dimen_base + split_max_depth_code);
  primitive("boxmaxdepth", assign_dimen, dimen_base + box_max_depth_code);
  primitive("hfuzz", assign_dimen, dimen_base + hfuzz_code);
  primitive("vfuzz", assign_dimen, dimen_base + vfuzz_code);
  primitive("delimitershortfall", assign_dimen, dimen_base + delimiter_shortfall_code);
  primitive("nulldelimiterspace", assign_dimen, dimen_base + null_delimiter_space_code);
  primitive("scriptspace", assign_dimen, dimen_base + script_space_code);
  primitive("predisplaysize", assign_dimen, dimen_base + pre_display_size_code);
  primitive("displaywidth", assign_dimen, dimen_base + display_width_code);
  primitive("displayindent", assign_dimen, dimen_base + display_indent_code);
  primitive("overfullrule", assign_dimen, dimen_base + overfull_rule_code);
  primitive("hangindent", assign_dimen, dimen_base + hang_indent_code);
  primitive("hoffset", assign_dimen, dimen_base + h_offset_code);
  primitive("voffset", assign_dimen, dimen_base + v_offset_code);
  primitive("emergencystretch", assign_dimen, dimen_base + emergency_stretch_code);
  primitive(" ", ex_space, 0);
  primitive("/", ital_corr, 0);
  primitive("accent", accent, 0);
  primitive("advance", advance, 0);
  primitive("afterassignment", after_assignment, 0);
  primitive("aftergroup", after_group, 0);
  primitive("begingroup", begin_group, 0);
  primitive("char", char_num, 0);
  primitive("csname", cs_name, 0);
  primitive("delimiter", delim_num, 0);
  primitive("divide", divide, 0);
  primitive("endcsname", end_cs_name, 0);
  primitive("endgroup", end_group, 0);
  text(frozen_end_group) = make_string_pool("endgroup");
  eqtb[frozen_end_group] = eqtb[cur_val]; 
  primitive("expandafter", expand_after, 0);
  primitive("font", def_font, 0);
  primitive("fontdimen", assign_font_dimen, 0);
  primitive("halign", halign, 0);
  primitive("hrule", hrule, 0);
  primitive("ignorespaces", ignore_spaces, 0);
  primitive("insert", insert, 0);
  primitive("mark", mark, 0);
  primitive("mathaccent", math_accent, 0);
  primitive("mathchar", math_char_num, 0);
  primitive("mathchoice", math_choice, 0);
  primitive("multiply", multiply, 0);
  primitive("noalign", no_align, 0);
  primitive("noboundary", no_boundary, 0);
  primitive("noexpand", no_expand, 0);
  primitive("nonscript", non_script, 0);
  primitive("omit", omit, 0);
  primitive("parshape", set_shape, 0);
  primitive("penalty", break_penalty, 0);
  primitive("prevgraf", set_prev_graf, 0);
  primitive("radical", radical, 0);
  primitive("read", read_to_cs, 0);
  primitive("relax", relax, 256);
  text(frozen_relax) = make_string_pool("relax");
  eqtb[frozen_relax] = eqtb[cur_val];
  primitive("setbox", set_box, 0);
  primitive("the", the, 0);
  primitive("toks", toks_register, 0);
  primitive("vadjust", vadjust, 0);
  primitive("valign", valign, 0);
  primitive("vcenter", vcenter, 0);
  primitive("vrule", vrule, 0);
  primitive("par", par_end, 256);
  par_loc = cur_val; 
  par_token = cs_token_flag + par_loc;
  primitive("input", input, 0);
  primitive("endinput", input, 1);
  primitive("topmark", top_bot_mark, 0);
  primitive("firstmark", top_bot_mark, 1);
  primitive("botmark", top_bot_mark, 2);
  primitive("splitfirstmark", top_bot_mark, 3);
  primitive("splitbotmark", top_bot_mark, 4);
  primitive("count", tex_register, 0);
  primitive("dimen", tex_register, 1);
  primitive("skip", tex_register, 2);
  primitive("muskip", tex_register, 3);
  primitive("spacefactor", set_aux, 102);
  primitive("prevdepth", set_aux, 1);
  primitive("deadcycles", set_page_int, 0);
  primitive("insertpenalties", set_page_int, 1);
  primitive("wd", set_box_dimen, 1);
  primitive("ht", set_box_dimen, 3);
  primitive("dp", set_box_dimen, 2);
  primitive("lastpenalty", last_item, 0);
  primitive("lastkern", last_item, 1);
  primitive("lastskip", last_item, 2);
  primitive("inputlineno", last_item, 3);
  primitive("badness", last_item, 4);
  primitive("number", convert, 0);
  primitive("romannumeral", convert, 1);
  primitive("string", convert, 2);
  primitive("meaning", convert, 3);
  primitive("fontname", convert, 4);
  primitive("jobname", convert, 5);
  primitive("if", if_test, 0);
  primitive("ifcat", if_test, 1);
  primitive("ifnum", if_test, 2);
  primitive("ifdim", if_test, 3);
  primitive("ifodd", if_test, 4);
  primitive("ifvmode", if_test, 5);
  primitive("ifhmode", if_test, 6);
  primitive("ifmmode", if_test, 7);
  primitive("ifinner", if_test, 8);
  primitive("ifvoid", if_test, 9);
  primitive("ifhbox", if_test, 10);
  primitive("ifvbox", if_test, 11);
  primitive("ifx", if_test, 12);
  primitive("ifeof", if_test, 13);
  primitive("iftrue", if_test, 14);
  primitive("iffalse", if_test, 15);
  primitive("ifcase", if_test, 16);
  primitive("fi", fi_or_else, 2);
  text(frozen_fi) = make_string_pool("fi");
  eqtb[frozen_fi] = eqtb[cur_val];
  primitive("or", fi_or_else, 4);
  primitive("else", fi_or_else, 3);
  primitive("nullfont", set_font, 0);
  text(frozen_null_font) = 795;
  eqtb[frozen_null_font] = eqtb[cur_val];
  primitive("span", tab_mark, 256);
  primitive("cr", car_ret, 257);
  text(frozen_cr) = make_string_pool("cr");
  eqtb[frozen_cr] = eqtb[cur_val];
  primitive("crcr", car_ret, 258);
  text(frozen_end_template) = make_string_pool("endtemplate");
  text(frozen_endv) = make_string_pool("endtemplate");
  eq_type(frozen_endv) = endv;
  equiv(frozen_endv) = null_list; 
  eq_level(frozen_endv) = level_one; 
  eqtb[frozen_end_template] = eqtb[frozen_endv]; 
  eq_type(frozen_end_template) = end_template;
  primitive("pagegoal", set_page_dimen, 0);
  primitive("pagetotal", set_page_dimen, 1);
  primitive("pagestretch", set_page_dimen, 2);
  primitive("pagefilstretch", set_page_dimen, 3);
  primitive("pagefillstretch", set_page_dimen, 4);
  primitive("pagefilllstretch", set_page_dimen, 5);
  primitive("pageshrink", set_page_dimen, 6);
  primitive("pagedepth", set_page_dimen, 7);
  primitive("end", end_match, 0);
  primitive("dump", stop, 1);
  primitive("hskip", hskip, 4);
  primitive("hfil", hskip, 0);
  primitive("hfill", hskip, 1);
  primitive("hss", hskip, 2);
  primitive("hfilneg", hskip, 3);
  primitive("vskip", vskip, 4);
  primitive("vfil", vskip, 0);
  primitive("vfill", vskip, 1);
  primitive("vss", vskip, 2);
  primitive("vfilneg", vskip, 3);
  primitive("mskip", mskip, 5);
  primitive("kern", kern, 1);
  primitive("mkern", mkern, 99);
  primitive("moveleft", hmove, 1);
  primitive("moveright", hmove, 0);
  primitive("raise", vmove, 1);
  primitive("lower", vmove, 0);
  primitive("box", make_box, 0);
  primitive("copy", make_box, 1);
  primitive("lastbox", make_box, 2);
  primitive("vsplit", make_box, 3);
  primitive("vtop", make_box, 4);
  primitive("vbox", make_box, 5);
  primitive("hbox", make_box, 106);
  primitive("shipout", leader_ship, 99);
  primitive("leaders", leader_ship, 100);
  primitive("cleaders", leader_ship, 101);
  primitive("xleaders", leader_ship, 102);
  primitive("indent", start_par, 1);
  primitive("noindent", start_par, 0);
  primitive("unpenalty", remove_item, 12);
  primitive("unkern", remove_item, 11);
  primitive("unskip", remove_item, 10);
  primitive("unhbox", un_hbox, 0);
  primitive("unhcopy", un_hbox, 1);
  primitive("unvbox", un_vbox, 0);
  primitive("unvcopy", un_vbox, 1);
  primitive("-", discretionary, 1);
  primitive("discretionary", discretionary, 0);
  primitive("eqno", eq_no, 0);
  primitive("leqno", eq_no, 1);
  primitive("mathord", math_comp, 16);
  primitive("mathop", math_comp, 17);
  primitive("mathbin", math_comp, 18);
  primitive("mathrel", math_comp, 19);
  primitive("mathopen", math_comp, 20);
  primitive("mathclose", math_comp, 21);
  primitive("mathpunct", math_comp, 22);
  primitive("mathinner", math_comp, 23);
  primitive("underline", math_comp, 26);
  primitive("overline", math_comp, 27);
  primitive("displaylimits", limit_switch, 0);
  primitive("limits", limit_switch, 1);
  primitive("nolimits", limit_switch, 2);
  primitive("displaystyle", math_style, 0);
  primitive("textstyle", math_style, 2);
  primitive("scriptstyle", math_style, 4);
  primitive("scriptscriptstyle", math_style, 6);
  primitive("above", above, 0);
  primitive("over", above, 1);
  primitive("atop", above, 2);
  primitive("abovewithdelims", above, 3);
  primitive("overwithdelims", above, 4);
  primitive("atopwithdelims", above, 5);
  primitive("left", left_right, 30);
  primitive("right", left_right, 31);
  text(frozen_right) = make_string_pool("right");
  eqtb[frozen_right] = eqtb[cur_val]; 
  primitive("long", prefix, 1);
  primitive("outer", prefix, 2);
  primitive("global", prefix, 4);
  primitive("def", def, 0);
  primitive("gdef", def, 1);
  primitive("edef", def, 2);
  primitive("xdef", def, 3);
  primitive("let", let, 0);
  primitive("futurelet", let, 1);
  primitive("chardef", shorthand_def, 0);
  primitive("mathchardef", shorthand_def, 1);
  primitive("countdef", shorthand_def, 2);
  primitive("dimendef", shorthand_def, 3);
  primitive("skipdef", shorthand_def, 4);
  primitive("muskipdef", shorthand_def, 5);
  primitive("toksdef", shorthand_def, 6);
  primitive("catcode", def_code, cat_code_base);
  primitive("mathcode", def_code, math_code_base);
  primitive("lccode", def_code, lc_code_base);
  primitive("uccode", def_code, uc_code_base);
  primitive("sfcode", def_code, sf_code_base);
  primitive("delcode", def_code, (hash_size + 3474));
  primitive("textfont", def_family, (hash_size + 1835));
  primitive("scriptfont", def_family, (hash_size + 1851));
  primitive("scriptscriptfont", def_family, (hash_size + 1867));
  primitive("hyphenation", hyph_data, 0);
  primitive("patterns", hyph_data, 1);
  primitive("hyphenchar", assign_font_int, 0);
  primitive("skewchar", assign_font_int, 1);
  primitive("batchmode", set_interaction, 0);
  primitive("nonstopmode", set_interaction, 1);
  primitive("scrollmode", set_interaction, 2);
  primitive("errorstopmode", set_interaction, 3);
  primitive("openin", in_stream, 1);
  primitive("closein", in_stream, 0);
  primitive("message", message, 0);
  primitive("errmessage", message, 1);
  primitive("lowercase", case_shift, (hash_size + 2139));
  primitive("uppercase", case_shift, (hash_size + 2395));
  primitive("show", xray, 0);
  primitive("showbox", xray, 1);
  primitive("showthe", xray, 2);
  primitive("showlists", xray, 3);
  primitive("openout", extension, 0);
  primitive("write", extension, 1);
  write_loc = cur_val;
  primitive("closeout", extension, 2);
  primitive("special", extension, 3);
  primitive("immediate", extension, 4);
  primitive("setlanguage", extension, 5);
  no_new_control_sequence = true; 
}
#endif