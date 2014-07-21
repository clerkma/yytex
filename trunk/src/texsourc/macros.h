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
#ifndef _YANDYTEX_MACROS_H
#define _YANDYTEX_MACROS_H

#define font_base 0
/* sec 0022 */
#define null_code       0     // 0
#define carriage_return 015   // 13
#define invalid_code    0177  // 127
/* sec 0036 */
#define loc cur_input.loc_field
/* sec 0040 */
#define length(s) (str_start[(s) + 1] - str_start[(s)])
/* sec 0041 */
#define cur_length (pool_ptr - str_start[str_ptr])
/* sec 0054 */
enum 
{
  no_print     = 16,
  term_only    = 17,
  log_only     = 18,
  term_and_log = 19,
  pseudo       = 20,
  new_string   = 21,
  max_selector = 21,
};
/* sec 0073 */
enum
{
  batch_mode      = 0,
  nonstop_mode    = 1,
  scroll_mode     = 2,
  error_stop_mode = 3,
};
/* sec 0076 */
enum
{
  spotless             = 0,
  warning_issued       = 1,
  error_message_issued = 2,
  fatal_error_stop     = 3,
};
/* sec 0096 */
#define check_interrupt()     \
do                            \
{                             \
  if (interrupt != 0)         \
    pause_for_instructions(); \
}                             \
while (0)
/* sec 0101 */
#define unity 0200000
#define two   0400000
/* sec 0105 */
#define nx_plus_y(a, b, c)  mult_and_add(a, b, c, 07777777777L)
#define mult_integers(a, b) mult_and_add(a, b, 0, 017777777777L)
/* sec 0108 */
#define inf_bad 10000L
/* sec 0109 */
#define set_glue_ratio_zero(a) (a) = 0.0
#define set_glue_ratio_one(a)  (a) = 1.0
#define tex_float(a)           (a)
#define unfloat(a)             (a)
#define float_constant(a)      (float) (a)
/* sec 0115 */
//#define pointer halfword
#define null    min_halfword
/* sec 0118 */
#define link(p) mem[p].hh.rh
#define info(p) mem[p].hh.lh
/* sec 0122 */
#ifdef STAT
#define fast_get_avail(a) \
  do                      \
    {                     \
      a = avail;          \
                          \
      if (a == 0)         \
        a = get_avail();  \
      else                \
      {                   \
        avail = link(a);  \
        link(a) = 0;      \
        incr(dyn_used);   \
      }                   \
    }                     \
  while (0)
#else
#define fast_get_avail(a) \
  do                      \
    {                     \
      a = avail;          \
                          \
      if (a == 0)         \
        a = get_avail();  \
      else                \
      {                   \
        avail = link(a);  \
        link(a) = 0;      \
      }                   \
    }                     \
  while (0)
#endif
/* sec 0124 */
#define empty_flag  max_halfword
#define is_empty(a) (link(a) == empty_flag)
#define node_size   info
#define llink(a)    info(a+1)
#define rlink(a)    link(a+1)
/* sec 0133 */
#define type(a)    mem[a].hh.b0
#define subtype(a) mem[a].hh.b1
/* sec 0134 */
#define is_char_node(a) (a >= hi_mem_min)
#define font            type
#define character       subtype
/* sec 0135 */
#define hlist_node      0
#define box_node_size   7
#define width_offset    1
#define depth_offset    2
#define height_offset   3
#define width(a)        mem[a + width_offset].cint
#define depth(a)        mem[a + depth_offset].cint
#define height(a)       mem[a + height_offset].cint
#define shift_amount(a) mem[a + 4].cint
#define list_offset     5
#define list_ptr(a)     link(a + list_offset)
#define glue_order(a)   subtype(a + list_offset)
#define glue_sign(a)    type(a + list_offset)
#define normal          0
#define stretching      1
#define shrinking       2
#define glue_offset     6
#define glue_set(a)     mem[a + glue_offset].gr
/* sec 0137 */
#define vlist_node 1
/* sec 0138 */
#define rule_node      2
#define rule_node_size 4
#define null_flag      -010000000000L
#define is_running(a)  (a == null_flag)
/* sec 0140 */
#define ins_node         3
#define ins_node_size    5
#define float_cost(a)    mem[a + 1].cint
#define ins_ptr(a)       info(a + 4)
#define split_top_ptr(a) link(a + 4)
/* sec 0141 */
#define mark_node       4
#define small_node_size 2
#define mark_ptr(a)     mem[a + 1].cint
/* sec 0142 */
#define adjust_node 5
#define adjust_ptr  mark_ptr
/* sec 0143 */
#define ligature_node 6
#define lig_char(a)   (a + 1)
#define lig_ptr(a)    link(lig_char(a))
/* sec 0145 */
#define disc_node     7
#define replace_count subtype
#define pre_break     llink
#define post_break    rlink
/* sec 0146 */
#define whatsit_node 8
/* sec 0147 */
#define math_node 9
#define before    0
#define after     1
/* sec 0148 */
#define precedes_break(a)  (type(a) < math_node)
#define non_discardable(a) (type(a) < math_node)
/* sec 0149 */
#define glue_node      10
#define cond_math_glue 98
#define mu_glue        99
#define a_leaders      100
#define c_leaders      101
#define x_leaders      102
#define glue_ptr       llink
#define leader_ptr     rlink
/* sec 0150 */
#define glue_spec_size    4
#define glue_ref_count(a) link(a)
#define stretch(a)        mem[a + 2].cint
#define shrink(a)         mem[a + 3].cint
#define stretch_order     type
#define shrink_order      subtype
#define fil               1
#define fill              2
#define filll             3
/* sec 0155 */
#define kern_node 11
#define explicit  1
#define acc_kern  2
/* sec 0157 */
#define penalty_node  12
#define inf_penalty   inf_bad
#define eject_penalty -inf_bad
#define penalty(a)    mem[a + 1].cint
/* sec 0159 */
#define unset_node      13
#define glue_stretch(a) mem[a + glue_offset].cint
#define glue_shrink     shift_amount
#define span_count      subtype
/* sec 0162 */
#define zero_glue         mem_bot // 0
#define fil_glue          (zero_glue + glue_spec_size) // 4
#define fill_glue         (fil_glue + glue_spec_size) // 8
#define ss_glue           (fill_glue + glue_spec_size) // 12
#define fil_neg_glue      (ss_glue + glue_spec_size) // 16
#define lo_mem_stat_max   (fil_neg_glue + glue_spec_size - 1) // 19
#define page_ins_head     mem_top
#define contrib_head      (mem_top - 1)
#define page_head         (mem_top - 2)
#define temp_head         (mem_top - 3)
#define hold_head         (mem_top - 4)
#define adjust_head       (mem_top - 5)
#define active            (mem_top - 7)
#define align_head        (mem_top - 8)
#define end_span          (mem_top - 9)
#define omit_template     (mem_top - 10)
#define null_list         (mem_top - 11)
#define lig_trick         (mem_top - 12)
#define garbage           (mem_top - 12)
#define backup_head       (mem_top - 13)
#define hi_mem_stat_min   (mem_top - 13)
#define hi_mem_stat_usage 14
/* sec 0200 */
#define token_ref_count(a) info(a)
/* sec 0203 */
#define add_token_ref(a) incr(token_ref_count(a))
#define add_glue_ref(a)  incr(glue_ref_count(a))
/* sec 0207 */
#define escape        0
#define relax         0
#define left_brace    1
#define right_brace   2
#define math_shift    3
#define tab_mark      4
#define car_ret       5
#define out_param     5
#define mac_param     6
#define sup_mark      7
#define sub_mark      8
#define ignore        9
#define endv          9
#define spacer        10
#define letter        11
#define other_char    12
#define active_char   13
#define par_end       13
#define match         13
#define comment       14
#define end_match     14
#define stop          14
#define invalid_char  15
#define delim_num     15
#define max_char_code 15
/* sec 0208 */
#define char_num      16
#define math_char_num 17
#define mark          18
#define xray          19
#define make_box      20
#define hmove         21
#define vmove         22
#define un_hbox       23
#define un_vbox       24
#define remove_item   25
#define hskip         26
#define vskip         27
#define mskip         28
#define kern          29
#define mkern         30
#define leader_ship   31
#define halign        32
#define valign        33
#define no_align      34
#define vrule         35
#define hrule         36
#define insert        37
#define vadjust       38
#define ignore_spaces 39
#define after_assignment 40
#define after_group      41
#define break_penalty    42
#define start_par        43
#define ital_corr        44
#define accent           45
#define math_accent      46
#define discretionary    47
#define eq_no            48
#define left_right       49
#define math_comp        50
#define limit_switch     51
#define above            52
#define math_style       53
#define math_choice      54
#define non_script       55
#define vcenter          56
#define case_shift       57
#define message          58
#define extension        59
#define in_stream        60
#define begin_group      61
#define end_group        62
#define omit             63
#define ex_space         64
#define no_boundary      65
#define radical          66
#define end_cs_name      67
#define min_internal     68
#define char_given       68
#define math_given       69
#define last_item        70
#define max_non_prefixed_command 70
/* sec 0209 */
#define toks_register     71
#define assign_toks       72
#define assign_int        73
#define assign_dimen      74
#define assign_glue       75
#define assign_mu_glue    76
#define assign_font_dimen 77
#define assign_font_int   78
#define set_aux           79
#define set_prev_graf     80
#define set_page_dimen    81
#define set_page_int      82
#define set_box_dimen     83
#define set_shape         84
#define def_code          85
#define def_family        86
#define set_font          87
#define def_font          88
#define tex_register      89
#define max_internal      89
#define advance           90
#define multiply          91
#define divide            92
#define prefix            93
#define let               94
#define shorthand_def     95
#define read_to_cs        96
#define def               97
#define set_box           98
#define hyph_data         99
#define set_interaction   100
#define max_command       100
/* sec 0210 */
#define undefined_cs    (max_command + 1 )
#define expand_after    (max_command + 2 )
#define no_expand       (max_command + 3 )
#define input           (max_command + 4 )
#define if_test         (max_command + 5 )
#define fi_or_else      (max_command + 6 )
#define cs_name         (max_command + 7 )
#define convert         (max_command + 8 )
#define the             (max_command + 9 )
#define top_bot_mark    (max_command + 10)
#define call            (max_command + 11)
#define long_call       (max_command + 12)
#define outer_call      (max_command + 13)
#define long_outer_call (max_command + 14)
#define end_template    (max_command + 15)
#define dont_expand     (max_command + 16)
#define glue_ref        (max_command + 17)
#define shape_ref       (max_command + 18)
#define box_ref         (max_command + 19) 
#define data            (max_command + 20)
/* sec 0211 */
#define vmode 1
#define hmode (vmode + max_command + 1)
#define mmode (hmode + max_command + 1)
/* sec 0212 */
#define ignore_depth -65536000L
/* sec 0213 */
#define mode            cur_list.mode_field
#define head            cur_list.head_field
#define tail            cur_list.tail_field
#define aux             cur_list.aux_field
#define prev_depth      aux.cint
#define space_factor    aux.hh.lh
#define clang           aux.hh.rh
#define incompleat_noad aux.cint
#define prev_graf       cur_list.pg_field
#define mode_line       cur_list.ml_field
/* sec 0221 */
#define eq_level_field(a) a.hh.b1
#define eq_type_field(a)  a.hh.b0
#define equiv_field(a)    a.hh.rh
#define eq_level(a)       eq_level_field(eqtb[a])
#define eq_type(a)        eq_type_field(eqtb[a])
#define equiv(a)          equiv_field(eqtb[a])
#define level_zero        min_quarterword
#define level_one         level_zero + 1
/* sec 0222 */
#define active_base                   1                                    // 1
#define single_base                   (active_base + 256)                  // 257
#define null_cs                       (single_base + 256)                  // 513
#define hash_base                     (null_cs + 1)                        // 514
#define frozen_control_sequence       (hash_base + hash_size + hash_extra) // (hash_size + hash_extra + 514)
#define frozen_protection             frozen_control_sequence              // (hash_size + hash_extra + 514)
#define frozen_cr                     (frozen_control_sequence + 1)        // (hash_size + hash_extra + 515)
#define frozen_end_group              (frozen_control_sequence + 2)        // (hash_size + hash_extra + 516)
#define frozen_right                  (frozen_control_sequence + 3)        // (hash_size + hash_extra + 517)
#define frozen_fi                     (frozen_control_sequence + 4)        // (hash_size + hash_extra + 518)
#define frozen_end_template           (frozen_control_sequence + 5)        // (hash_size + hash_extra + 519)
#define frozen_endv                   (frozen_control_sequence + 6)        // (hash_size + hash_extra + 520)
#define frozen_relax                  (frozen_control_sequence + 7)        // (hash_size + hash_extra + 521)
#define end_write                     (frozen_control_sequence + 8)        // (hash_size + hash_extra + 522)
#define frozen_dont_expand            (frozen_control_sequence + 9)        // (hash_size + hash_extra + 523)
#define frozen_null_font              (frozen_control_sequence + 10)       // (hash_size + hash_extra + 524)
#define font_id_base                  (frozen_null_font - font_base)       // (hash_size + hash_extra + 524)
#define undefined_control_sequence    (frozen_null_font + font_max + 2)    // (hash_size + 781)
#define glue_base                     (undefined_control_sequence + 1)     // (hash_size + 782)
/* sec 0224 */
#define line_skip_code                0  // 782
#define baseline_skip_code            1  // 783
#define par_skip_code                 2  // 784
#define above_display_skip_code       3  // 785
#define below_display_skip_code       4  // 786
#define above_display_short_skip_code 5  // 787
#define below_display_short_skip_code 6  // 788
#define left_skip_code                7  // 789
#define right_skip_code               8  // 790
#define top_skip_code                 9  // 791
#define split_top_skip_code           10 // 792
#define tab_skip_code                 11 // 793
#define space_skip_code               12 // 794
#define xspace_skip_code              13 // 795
#define par_fill_skip_code            14 // 796
#define thin_mu_skip_code             15 // 797
#define med_mu_skip_code              16 // 798
#define thick_mu_skip_code            17 // 799
#define glue_pars                     18 // 800
#define skip_base                     (glue_base + glue_pars) // 800
#define mu_skip_base                  (skip_base + 256) // 1056
#define local_base                    (mu_skip_base + 256) // 1312
// #
#define skip(a)                       equiv(skip_base + a)
#define mu_skip(a)                    equiv(mu_skip_base + a)
#define glue_par(a)                   equiv(glue_base + a)
#define line_skip                     glue_par(line_skip_code)
#define baseline_skip                 glue_par(baseline_skip_code)
#define par_skip                      glue_par(par_skip_code)
#define above_display_skip            glue_par(above_display_skip_code)
#define below_display_skip            glue_par(below_display_skip_code)
#define above_display_short_skip      glue_par(above_display_short_skip_code)
#define below_display_short_skip      glue_par(below_display_short_skip_code)
#define left_skip                     glue_par(left_skip_code)
#define right_skip                    glue_par(right_skip_code)
#define top_skip                      glue_par(top_skip_code)
#define split_top_skip                glue_par(split_top_skip_code)
#define tab_skip                      glue_par(tab_skip_code)
#define space_skip                    glue_par(space_skip_code)
#define xspace_skip                   glue_par(xspace_skip_code)
#define par_fill_skip                 glue_par(par_fill_skip_code)
#define thin_mu_skip                  glue_par(thin_mu_skip_code)
#define med_mu_skip                   glue_par(med_mu_skip_code)
#define thick_mu_skip                 glue_par(thick_mu_skip_code)
/* sec 0230 */
#define par_shape_loc                 local_base             // 1312
#define output_routine_loc            (local_base + 1)       // 1313
#define every_par_loc                 (local_base + 2)       // 1314
#define every_math_loc                (local_base + 3)       // 1315
#define every_display_loc             (local_base + 4)       // 1316
#define every_hbox_loc                (local_base + 5)       // 1317
#define every_vbox_loc                (local_base + 6)       // 1318
#define every_job_loc                 (local_base + 7)       // 1319
#define every_cr_loc                  (local_base + 8)       // 1320
#define err_help_loc                  (local_base + 9)       // 1321
#define toks_base                     (local_base + 10)      // 1322
#define box_base                      (toks_base + 256)      // 1578
#define cur_font_loc                  (box_base + 256)       // 1834
#define math_font_base                (cur_font_loc + 1)     // 1835
#define cat_code_base                 (math_font_base + 48)  // 1883
#define lc_code_base                  (cat_code_base + 256)  // 2139
#define uc_code_base                  (lc_code_base + 256)   // 2395
#define sf_code_base                  (uc_code_base + 256)   // 2651
#define math_code_base                (sf_code_base + 256)   // 2907
#define int_base                      (math_code_base + 256) // 3163
// #
#define par_shape_ptr                 equiv(par_shape_loc)
#define output_routine                equiv(output_routine_loc)
#define every_par                     equiv(every_par_loc)
#define every_math                    equiv(every_math_loc)
#define every_display                 equiv(every_display_loc)
#define every_hbox                    equiv(every_hbox_loc)
#define every_vbox                    equiv(every_vbox_loc)
#define every_job                     equiv(every_job_loc)
#define every_cr                      equiv(every_cr_loc)
#define err_help                      equiv(err_help_loc)
#define toks(a)                       equiv(toks_base + a)
#define box(a)                        equiv(box_base + a)
#define cur_font                      equiv(cur_font_loc)
#define fam_fnt(a)                    equiv(math_font_base + a)
#define cat_code(a)                   equiv(cat_code_base + a)
#define lc_code(a)                    equiv(lc_code_base + a)
#define uc_code(a)                    equiv(uc_code_base + a)
#define sf_code(a)                    equiv(sf_code_base + a)
#define math_code(a)                  equiv(math_code_base + a)
/* sec 0232 */
#define null_font font_base
#define var_code 070000
/* sec 0236 */
#define pretolerance_code             0  // 3163
#define tolerance_code                1  // 3164
#define line_penalty_code             2  // 3165
#define hyphen_penalty_code           3  // 3166
#define ex_hyphen_penalty_code        4  // 3167
#define club_penalty_code             5  // 3168
#define widow_penalty_code            6  // 3169
#define display_widow_penalty_code    7  // 3170
#define broken_penalty_code           8  // 3171
#define bin_op_penalty_code           9  // 3172
#define rel_penalty_code              10 // 3173
#define pre_display_penalty_code      11 // 3174
#define post_display_penalty_code     12 // 3175
#define inter_line_penalty_code       13 // 3176
#define double_hyphen_demerits_code   14 // 3177
#define final_hyphen_demerits_code    15 // 3178
#define adj_demerits_code             16 // 3179
#define mag_code                      17 // 3180
#define delimiter_factor_code         18 // 3181
#define looseness_code                19 // 3182
#define time_code                     20 // 3183
#define day_code                      21 // 3184
#define month_code                    22 // 3185
#define year_code                     23 // 3186
#define show_box_breadth_code         24 // 3187
#define show_box_depth_code           25 // 3188
#define hbadness_code                 26 // 3189
#define vbadness_code                 27 // 3190
#define pausing_code                  28 // 3191
#define tracing_online_code           29 // 3192
#define tracing_macros_code           30 // 3193
#define tracing_stats_code            31 // 3194
#define tracing_paragraphs_code       32 // 3195
#define tracing_pages_code            33 // 3196
#define tracing_output_code           34 // 3197
#define tracing_lost_chars_code       35 // 3198
#define tracing_commands_code         36 // 3199 
#define tracing_restores_code         37 // 3200
#define uc_hyph_code                  38 // 3201
#define output_penalty_code           39 // 3202
#define max_dead_cycles_code          40 // 3203
#define hang_after_code               41 // 3204
#define floating_penalty_code         42 // 3205
#define global_defs_code              43 // 3206
#define cur_fam_code                  44 // 3207
#define escape_char_code              45 // 3208
#define default_hyphen_char_code      46 // 3209
#define default_skew_char_code        47 // 3210
#define end_line_char_code            48 // 3211
#define new_line_char_code            49 // 3212
#define language_code                 50 // 3213
#define left_hyphen_min_code          51 // 3214
#define right_hyphen_min_code         52 // 3215
#define holding_inserts_code          53 // 3216
#define error_context_lines_code      54 // 3217
#define int_pars                      55
#define count_base                    (int_base + int_pars) // 3218
#define del_code_base                 (count_base + 256)    // 3474
#define dimen_base                    (del_code_base + 256) // 3730
// #
#define del_code(a)                   eqtb[del_code_base + a].cint
#define count(a)                      eqtb[count_base + a].cint
#define int_par(a)                    eqtb[int_base + a].cint
#define pretolerance                  int_par(pretolerance_code)
#define tolerance                     int_par(tolerance_code)
#define line_penalty                  int_par(line_penalty_code)
#define hyphen_penalty                int_par(hyphen_penalty_code)
#define ex_hyphen_penalty             int_par(ex_hyphen_penalty_code)
#define club_penalty                  int_par(club_penalty_code)
#define widow_penalty                 int_par(widow_penalty_code)
#define display_widow_penalty         int_par(display_widow_penalty_code)
#define broken_penalty                int_par(broken_penalty_code)
#define bin_op_penalty                int_par(bin_op_penalty_code)
#define rel_penalty                   int_par(rel_penalty_code)
#define pre_display_penalty           int_par(pre_display_penalty_code)
#define post_display_penalty          int_par(post_display_penalty_code)
#define inter_line_penalty            int_par(inter_line_penalty_code)
#define double_hyphen_demerits        int_par(double_hyphen_demerits_code)
#define final_hyphen_demerits         int_par(final_hyphen_demerits_code)
#define adj_demerits                  int_par(adj_demerits_code)
#define mag                           int_par(mag_code)
#define delimiter_factor              int_par(delimiter_factor_code)
#define looseness                     int_par(looseness_code)
#define tex_time                      int_par(time_code)
#define day                           int_par(day_code)
#define month                         int_par(month_code)
#define year                          int_par(year_code)
#define show_box_breadth              int_par(show_box_breadth_code)
#define show_box_depth                int_par(show_box_depth_code)
#define hbadness                      int_par(hbadness_code)
#define vbadness                      int_par(vbadness_code)
#define pausing                       int_par(pausing_code)
#define tracing_online                int_par(tracing_online_code)
#define tracing_macros                int_par(tracing_macros_code)
#define tracing_stats                 int_par(tracing_stats_code)
#define tracing_paragraphs            int_par(tracing_paragraphs_code)
#define tracing_pages                 int_par(tracing_pages_code)
#define tracing_output                int_par(tracing_output_code)
#define tracing_lost_chars            int_par(tracing_lost_chars_code)
#define tracing_commands              int_par(tracing_commands_code)
#define tracing_restores              int_par(tracing_restores_code)
#define uc_hyph                       int_par(uc_hyph_code)
#define output_penalty                int_par(output_penalty_code)
#define max_dead_cycles               int_par(max_dead_cycles_code)
#define hang_after                    int_par(hang_after_code)
#define floating_penalty              int_par(floating_penalty_code)
#define global_defs                   int_par(global_defs_code)
#define cur_fam                       int_par(cur_fam_code)
#define escape_char                   int_par(escape_char_code)
#define default_hyphen_char           int_par(default_hyphen_char_code)
#define default_skew_char             int_par(default_skew_char_code)
#define end_line_char                 int_par(end_line_char_code)
#define new_line_char                 int_par(new_line_char_code)
#define language                      int_par(language_code)
#define left_hyphen_min               int_par(left_hyphen_min_code)
#define right_hyphen_min              int_par(right_hyphen_min_code)
#define holding_inserts               int_par(holding_inserts_code)
#define error_context_lines           int_par(error_context_lines_code)
/* sec 0247 */
#define par_indent_code               0  // 3730
#define math_surround_code            1  // 3731
#define line_skip_limit_code          2  // 3732
#define hsize_code                    3  // 3733
#define vsize_code                    4  // 3734
#define max_depth_code                5  // 3735
#define split_max_depth_code          6  // 3736
#define box_max_depth_code            7  // 3737
#define hfuzz_code                    8  // 3738
#define vfuzz_code                    9  // 3739
#define delimiter_shortfall_code      10 // 3740
#define null_delimiter_space_code     11 // 3741
#define script_space_code             12 // 3742
#define pre_display_size_code         13 // 3743
#define display_width_code            14 // 3744
#define display_indent_code           15 // 3745
#define overfull_rule_code            16 // 3746
#define hang_indent_code              17 // 3747
#define h_offset_code                 18 // 3748
#define v_offset_code                 19 // 3749
#define emergency_stretch_code        20 // 3750
#define dimen_pars                    21
#define scaled_base                   (dimen_base + dimen_pars) // 3751
#define eqtb_size                     (scaled_base + 255) // 4006
// #
#define dimen(a)                      eqtb[scaled_base + a].cint
#define dimen_par(a)                  eqtb[dimen_base + a].cint
#define par_indent                    dimen_par(par_indent_code)
#define math_surround                 dimen_par(math_surround_code)
#define line_skip_limit               dimen_par(line_skip_limit_code)
#define hsize                         dimen_par(hsize_code)
#define vsize                         dimen_par(vsize_code)
#define max_depth                     dimen_par(max_depth_code)
#define split_max_depth               dimen_par(split_max_depth_code)
#define box_max_depth                 dimen_par(box_max_depth_code)
#define hfuzz                         dimen_par(hfuzz_code)
#define vfuzz                         dimen_par(vfuzz_code)
#define delimiter_shortfall           dimen_par(delimiter_shortfall_code)
#define null_delimiter_space          dimen_par(null_delimiter_space_code)
#define script_space                  dimen_par(script_space_code)
#define pre_display_size              dimen_par(pre_display_size_code)
#define display_width                 dimen_par(display_width_code)
#define display_indent                dimen_par(display_indent_code)
#define overfull_rule                 dimen_par(overfull_rule_code)
#define hang_indent                   dimen_par(hang_indent_code)
#define h_offset                      dimen_par(h_offset_code)
#define v_offset                      dimen_par(v_offset_code)
#define emergency_stretch             dimen_par(emergency_stretch_code)
/* sec 0256 */
#define text(a)         hash[a].rh
#define next(a)         hash[a].lh
#define hash_is_full    (hash_used == hash_base)
#define font_id_text(a) text(font_id_base + a)
/* sec 0268 */
#define save_type(a)      save_stack[a].hh.b0
#define save_level(a)     save_stack[a].hh.b1
#define save_index(a)     save_stack[a].hh.rh
#define restore_old_value 0
#define restore_zero      1
#define insert_token      2
#define level_boundary    3
/* sec 0269 */
#define bottom_level      0
#define simple_group      1
#define hbox_group        2
#define adjust_hbox_group 3
#define vbox_group        4
#define vtop_group        5
#define align_group       6
#define no_align_group    7
#define output_group      8
#define math_group        9
#define disc_group        10
#define insert_group      11
#define vcenter_group     12
#define math_choice_group 13
#define semi_simple_group 14
#define math_shift_group  15
#define math_left_group   16
#define max_group_code    16
/* sec 0274 */
#define saved(a) save_stack[save_ptr + (a)].cint
/* sec 0289 */
#define cs_token_flag     07777 // 4095
#define left_brace_token  0400  // 256  = 2^8 * left_brace
#define left_brace_limit  01000 // 512  = 2^8 * (left_brace + 1)
#define right_brace_token 01000 // 512  = 2^8 * right_brace
#define right_brace_limit 01400 // 768  = 2^8 * (right_brace + 1)
#define math_shift_token  01400 // 768  = 2^8 * math_shift
#define tab_token         02000 // 1024 = 2^8 * tab_mark
#define out_param_token   02400 // 1280 = 2^8 * out_param
#define space_token       05040 // 2592 = 2^8 * spacer + ' '
#define letter_token      05400 // 2816 = 2^8 * letter
#define other_token       06000 // 3072 = 2^8 * other_char
#define match_token       06400 // 3328 = 2^8 * match
#define end_match_token   07000 // 3584 = 2^8 * end_match
/* sec 0298 */
#define chr_cmd(s)  \
do                  \
{                   \
  prints(s);        \
  print(chr_code);  \
}                   \
while (0)
/* sec 0302 */
#define state cur_input.state_field
#define index cur_input.index_field
#define start cur_input.start_field
#define limit cur_input.limit_field
#define name  cur_input.name_field
/* sec 0303 */
#define mid_line    1
#define skip_blanks 2 + max_char_code // 17
#define new_line    3 + max_char_code + max_char_code // 33
/* sec 0304 */
#define cur_file input_file[index]
/* sec 0305 */
#define skipping  1
#define defining  2
#define matching  3
#define aligning  4
#define absorbing 5
/* sec 0307 */
#define token_list         0
#define token_type         index
#define param_start        limit
#define parameter          0
#define u_template         1
#define v_template         2
#define backed_up          3
#define inserted           4
#define macro              5
#define output_text        6
#define every_par_text     7
#define every_math_text    8
#define every_display_text 9
#define every_hbox_text    10
#define every_vbox_text    11
#define every_job_text     12
#define every_cr_text      13
#define mark_text          14
#define write_text         15
/* sec 0316 */
#define begin_pseudoprint() \
do                          \
{                           \
  l = tally;                \
  tally = 0;                \
  selector = pseudo;        \
  trick_count = 1000000L;   \
}                           \
while (0)
#define set_trick_count()                                 \
do                                                        \
{                                                         \
  first_count = tally;                                    \
  trick_count = tally + 1 + error_line - half_error_line; \
                                                          \
  if (trick_count < error_line)                           \
    trick_count = error_line;                             \
}                                                         \
while (0)
/* sec 0322 */
/* sec 0323 */
#define back_list(a) begin_token_list(a, backed_up)
#define ins_list(a)  begin_token_list(a, inserted)
/* sec 0344 */
#define any_state_plus(a) mid_line + (a): case skip_blanks + (a): case new_line + (a)
/* sec 0347 */
#define add_delims_to(a) \
  (a) + math_shift:      \
  case (a) + tab_mark:   \
  case (a) + mac_param:  \
  case (a) + sub_mark:   \
  case (a) + letter:     \
  case (a) + other_char
/* sec 0352 */
#define is_hex(a) \
  (((a >= '0') && (a <= '9')) || ((a >= 'a') && (a <= 'f')))
#define hex_to_cur_chr()                    \
do                                          \
{                                           \
  if (c <= '9')                             \
    cur_chr = c - '0';                      \
  else                                      \
    cur_chr = c - 'a' + 10;                 \
                                            \
  if (cc <= '9')                            \
    cur_chr = 16 * cur_chr + cc - '0';      \
  else                                      \
    cur_chr = 16 * cur_chr + cc - 'a' + 10; \
}                                           \
while (0)
/* sec 0360 */
#define end_line_char_inactive()  \
  (end_line_char < 0) || (end_line_char > 255)
/* sec 0358 */
/* sec 0371 */
#define store_new_token(a)  \
  do                        \
    {                       \
      q = get_avail();      \
      link(p) = q;          \
      info(q) = a;          \
      p = q;                \
    }                       \
  while (0)
#define fast_store_new_token(a) \
  do                            \
    {                           \
      fast_get_avail(q);        \
      link(p) = q;              \
      info(q) = a;              \
      p = q;                    \
    }                           \
  while (0)
#define no_expand_flag 257
/* sec 0382 */
#define top_mark_code         0
#define first_mark_code       1
#define bot_mark_code         2
#define split_first_mark_code 3
#define split_bot_mark_code   4
#define top_mark              cur_mark[top_mark_code]
#define first_mark            cur_mark[first_mark_code]
#define bot_mark              cur_mark[bot_mark_code]
#define split_first_mark      cur_mark[split_first_mark_code]
#define split_bot_mark        cur_mark[split_bot_mark_code]
/* sec 0400 */
#define int_val   0
#define dimen_val 1
#define glue_val  2
#define mu_val    3
#define ident_val 4
#define tok_val   5
/* sec 0413 */
#define scanned_result(va, vb) \
do                            \
  {                           \
    cur_val = va;             \
    cur_val_level = vb;       \
  }                           \
while (0)
/* sec 0416 */
#define input_line_no_code (glue_val + 1)
#define badness_code       (glue_val + 2)
/* sec 0421 */
#define max_dimen 07777777777
/* sec 0438 */
#define octal_token             (other_token + '\'') // 3111
#define hex_token               (other_token + '"' ) // 3106
#define alpha_token             (other_token + '`' ) // 3168
#define point_token             (other_token + '.' ) // 3118
#define continental_point_token (other_token + ',' ) // 3116
/* sec 0445 */
#define zero_token    (other_token  + '0') // 3120
#define A_token       (letter_token + 'A') // 2881
#define other_A_token (other_token  + 'A') // 3137
/* sec 0458 */
#define set_conversion(a, b)  \
  do                          \
  {                           \
    num = a;                  \
    denom = b;                \
  } while (0)
/* sec 0468 */
#define number_code        0
#define roman_numeral_code 1
#define string_code        2
#define meaning_code       3
#define font_name_code     4
#define job_name_code      5
/* sec 0480 */
#define closed    2
#define just_open 1
/* sec 0487 */
#define if_char_code   0
#define if_cat_code    1
#define if_int_code    2
#define if_dim_code    3
#define if_odd_code    4
#define if_vmode_code  5
#define if_hmode_code  6
#define if_mmode_code  7
#define if_inner_code  8
#define if_void_code   9
#define if_hbox_code   10
#define if_vbox_code   11
#define ifx_code       12
#define if_eof_code    13
#define if_true_code   14
#define if_false_code  15
#define if_case_code   16
/* sec 0489 */
#define if_node_size     2
#define if_line_field(a) mem[(a) + 1].cint
#define if_code          1
#define fi_code          2
#define else_code        3
#define or_code          4
/* sec 0506 */
#define get_x_token_or_active_char()                    \
do                                                      \
{                                                       \
  get_x_token();                                        \
                                                        \
  if (cur_cmd == relax)                                 \
    if (cur_chr == no_expand_flag)                      \
    {                                                   \
      cur_cmd = active_char;                            \
      cur_chr = cur_tok - cs_token_flag - active_base;  \
    }                                                   \
}                                                       \
while (0)
/* sec 0519 */
#define append_to_name(a)       \
do                              \
{                               \
  c = a; incr(k);               \
                                \
  if (k < file_name_size)       \
    name_of_file[k] = xchr[c];  \
}                               \
while (0)
/* sec 0544 */
#define no_tag   0
#define lig_tag  1
#define list_tag 2
#define ext_tag  3
/* sec 0545 */
#define stop_flag    128
#define kern_flag    128
#define skip_byte(a) a.b0
#define next_char(a) a.b1
#define op_byte(a)   a.b2
#define rem_byte(a)  a.b3
/* sec 0546 */
#define ext_top(a) a.b0
#define ext_mid(a) a.b1
#define ext_bot(a) a.b2
#define ext_rep(a) a.b3
/* sec 0547 */
#define slant_code         1
#define space_code         2
#define space_stretch_code 3
#define space_shrink_code  4
#define x_height_code      5
#define quad_code          6
#define extra_space_code   7
/* sec 0549 */
#define non_char    256
#define non_address 0
/* sec 0554 */
#define char_info(a, b)   font_info[char_base[a] + b].qqqq
#define char_width(a, b)  font_info[width_base[a] + b.b0].cint
#define char_exists(a)    (a.b0 > min_quarterword)
#define char_italic(a, b) font_info[italic_base[a] + (b.b2) / 4].cint
#define height_depth(a)   (a.b1)
#define char_height(a, b) font_info[height_base[a] + (b) / 16].cint
#define char_depth(a, b)  font_info[depth_base[a] + (b) % 16].cint
#define char_tag(a)       (a.b2 % 4)
/* sec 0557 */
#define char_kern(a, b)        font_info[kern_base[a] + 256 * op_byte(b) + rem_byte(b)].cint
#define kern_base_offset       (256 * (128 + min_quarterword))
#define lig_kern_start(a, b)   lig_kern_base[a] + rem_byte(b)
#define lig_kern_restart(a, b) lig_kern_base[a] + 256 * op_byte(b) + rem_byte(b) + 32768 - kern_base_offset
/* sec 0558 */
#define param(a, b)      font_info[a + param_base[b]].cint
#define slant(f)         param(slant_code, f)
#define space(f)         param(space_code, f)
#define space_stretch(f) param(space_stretch_code, f)
#define space_shrink(f)  param(space_shrink_code, f)
#define x_height(f)      param(x_height_code, f)
#define quad(f)          param(quad_code, f)
#define extra_space(f)   param(extra_space_code, f)
/* sec 0561 */
#define start_font_error_message()  \
do                                  \
{                                   \
  print_err("Font ");               \
  sprint_cs(u);                     \
  print_char('=');                  \
  print_file_name(nom, aire, 335);  \
                                    \
  if (s >= 0)                       \
  {                                 \
    prints(" at ");                 \
    print_scaled(s);                \
    prints("pt");                   \
  }                                 \
  else if (s != -1000)              \
  {                                 \
    prints(" scaled ");             \
    print_int(-(integer)s);         \
  }                                 \
}                                   \
while (0)
/* sec 0564 */
#define read_sixteen(a) \
do                      \
  {                     \
    a = fbyte;          \
                        \
    if (a > 127)        \
      goto bad_tfm;     \
                        \
    fget();             \
    a = a * 256 + fbyte;\
  }                     \
while (0)
#define store_four_quarters(val)  \
do                                \
  {                               \
    fget(); a = fbyte; qw.b0 = a; \
    fget(); b = fbyte; qw.b1 = b; \
    fget(); c = fbyte; qw.b2 = c; \
    fget(); d = fbyte; qw.b3 = d; \
    val = qw;                     \
  }                               \
while (0)
/* sec 0570 */
#define check_byte_range(a) \
do                          \
{                           \
  if ((a < bc) || (a > ec)) \
    goto bad_tfm;           \
}                           \
  while (0)
/* sec 0571 */
#define store_scaled(a)                                       \
do                                                            \
{                                                             \
  fget(); a = fbyte; fget(); b = fbyte;                       \
  fget(); c = fbyte; fget(); d = fbyte;                       \
  sw = (((((d * z) / 256) + (c * z)) / 256) + (b * z)) / beta;\
                                                              \
  if (a == 0)                                                 \
    a = sw;                                                   \
  else if (a == 255)                                          \
    a = sw - alpha;                                           \
  else                                                        \
    goto bad_tfm;                                             \
}                                                             \
while (0)
/* sec 0573 */
#define check_existence(a)  \
do                          \
{                           \
  check_byte_range(a);      \
                            \
  qw = char_info(f, a);     \
                            \
  if (!char_exists(qw))     \
    goto bad_tfm;           \
}                           \
while (0)
/* sec 0576 */
#define adjust(a) a[f] = a[f]
/* sec 0585 */
#define set1      128 // c[1]
#define set2      129 // c[2]
#define set3      130 // c[3]
#define set4      131 // c[4]
#define set_rule  132 // a[4] b[4]
#define put1      133 // c[1]
#define put2      134 // c[2]
#define put3      135 // c[3]
#define put4      136 // c[4]
#define put_rule  137 // a[4] b[4]
#define nop       138 // NULL
#define bop       139 // c0[4] c1[4] ... c9[4] p[4]
#define eop       140 // NULL
#define push      141
#define pop       142
#define right1    143 // b[1]
#define right2    144 // b[2]
#define right3    145 // b[3]
#define right4    146 // b[4]
#define w0        147 //
#define w1        148 // b[1]
#define w2        149 // b[2]
#define w3        150 // b[3]
#define w4        151 // b[4]
#define x0        152 //
#define x1        153 // b[1]
#define x2        154 // b[2]
#define x3        155 // b[3]
#define x4        156 // b[4]
#define down1     157 // a[1]
#define down2     158 // a[2]
#define down3     159 // a[3]
#define down4     160 // a[4]
#define y0        161 //
#define y1        162 // a[1]
#define y2        163 // a[2]
#define y3        164 // a[3]
#define y4        165 // a[4]
#define z0        166 //
#define z1        167 // a[1]
#define z2        168 // a[2]
#define z3        169 // a[3]
#define z4        170 // a[4]
#define fnt_num_0 171 //
#define fnt1      235 // k[1]
#define fnt2      236 // k[2]
#define fnt3      237 // k[3]
#define fnt4      238 // k[4]
#define xxx1      239 // k[1] x[k]
#define xxx2      240 // k[2] x[k]
#define xxx3      241 // k[3] x[k]
#define xxx4      242 // k[4] x[k]
#define fnt_def1  243 // k[1] c[4] s[4] d[4] a[1] l[1] n[a + l]
#define fnt_def2  244 // k[2] c[4] s[4] d[4] a[1] l[1] n[a + l]
#define fnt_def3  245 // k[3] c[4] s[4] d[4] a[1] l[1] n[a + l]
#define fnt_def4  246 // k[4] c[4] s[4] d[4] a[1] l[1] n[a + l]
#define pre       247 // i[1] num[4] den[4] mag[4] k[1] x[k]
#define post      248 //
#define post_post 249 //
/* sec 0587 */
#define id_byte 2
/* sec 0605 */
#define movement_node_size 3
#define location(a) mem[a + 2].cint
/* sec 0608 */
#define y_here  1
#define z_here  2
#define yz_OK   3
#define y_OK    4
#define z_OK    5
#define d_fixed 6
/* sec 0611 */
#define none_seen 0
#define y_seen    6
#define z_seen    12
/* sec 0625 */
#define billion 1000000000.0
#define vet_glue(a)             \
do                              \
{                               \
  glue_temp = a;                \
  if (glue_temp > billion)      \
    glue_temp = billion;        \
  else if (glue_temp < -billion)\
    glue_temp = -billion;       \
}                               \
while (0)
/* sec 0644 */
#define exactly    0
#define additional 1
#define natural 0, additional
/* sec 0769 */
#define u_part(a)     mem[(a) + height_offset].cint
#define v_part(a)     mem[(a) + depth_offset].cint
#define extra_info(a) info((a) + list_offset)
/* sec 0681 */
#define noad_size      4
#define nucleus(a)     ((a) + 1)
#define supscr(a)      ((a) + 2)
#define subscr(a)      ((a) + 3)
#define math_type      link
#define fam            font
#define math_char      1
#define sub_box        2
#define sub_mlist      3
#define math_text_char 4
/* sec 0682 */
#define ord_noad   (unset_node + 3) // 16
#define op_noad    (ord_noad + 1  ) // 17
#define bin_noad   (ord_noad + 2  ) // 18
#define rel_noad   (ord_noad + 3  ) // 19
#define open_noad  (ord_noad + 4  ) // 20
#define close_noad (ord_noad + 5  ) // 21
#define punct_noad (ord_noad + 6  ) // 22
#define inner_noad (ord_noad + 7  ) // 23
#define limits    1
#define no_limits 2
/* sec 0683 */
#define left_delimiter(a)  ((a) + 4)
#define right_delimiter(a) ((a) + 5)
#define radical_noad       (inner_noad + 1) // 24
#define radical_noad_size  5
#define fraction_noad      (radical_noad + 1) // 25
#define fraction_noad_size 6
#define small_fam(a)       mem[(a)].qqqq.b0
#define small_char(a)      mem[(a)].qqqq.b1
#define large_fam(a)       mem[(a)].qqqq.b2
#define large_char(a)      mem[(a)].qqqq.b3
#define thickness          width
#define default_code       010000000000L
#define numerator          supscr
#define denominator        subscr
/* sec 0687 */
#define under_noad        (fraction_noad + 1) // 26
#define over_noad         (under_noad + 1   ) // 27
#define accent_noad       (over_noad + 1    ) // 28
#define accent_noad_size  5
#define accent_chr(a)     (a) + 4
#define vcenter_noad      (accent_noad + 1  ) // 29
#define left_noad         (vcenter_noad + 1 ) // 30
#define right_noad        (left_noad + 1    ) // 31
#define delimiter         nucleus
#define script_allowed(a) ((type(a) >= ord_noad) && (type(a) < left_noad))
/* sec 0688 */
#define style_node          (unset_node + 1)
#define style_node_size     3
#define display_style       0
#define text_style          2
#define script_style        4
#define script_script_style 6
#define cramped             1
/* sec 0689 */
#define choice_node            (unset_node + 2)
#define display_mlist(a)       info(a + 1)
#define text_mlist(a)          link(a + 1)
#define script_mlist(a)        info(a + 2)
#define script_script_mlist(a) link(a + 2)
/* sec 0699 */
#define text_size          0
#define script_size        16
#define script_script_size 32
/* sec 0700 */
#define mathsy(a, b)        font_info[a + param_base[fam_fnt(2 + b)]].cint
#define math_x_height(a)    mathsy(5, a)
#define math_quad(a)        mathsy(6, a)
#define num1(a)             mathsy(8, a)
#define num2(a)             mathsy(9, a)
#define num3(a)             mathsy(10, a)
#define denom1(a)           mathsy(11, a)
#define denom2(a)           mathsy(12, a)
#define sup1(a)             mathsy(13, a)
#define sup2(a)             mathsy(14, a)
#define sup3(a)             mathsy(15, a)
#define sub1(a)             mathsy(16, a)
#define sub2(a)             mathsy(17, a)
#define sup_drop(a)         mathsy(18, a)
#define sub_drop(a)         mathsy(19, a)
#define delim1(a)           mathsy(20, a)
#define delim2(a)           mathsy(21, a)
#define axis_height(a)      mathsy(22, a)
#define total_mathsy_params 22
/* sec 0701 */
#define mathex(a)              font_info[(a) + param_base[fam_fnt(3 + cur_size)]].cint
#define default_rule_thickness mathex(8)
#define big_op_spacing1        mathex(9)
#define big_op_spacing2        mathex(10)
#define big_op_spacing3        mathex(11)
#define big_op_spacing4        mathex(12)
#define big_op_spacing5        mathex(13)
#define total_mathex_params    13
/* sec 0702 */
#define cramped_style(a) (2 * ((a) / 2) + cramped)
#define sub_style(a)     (2 * ((a) / 4) + script_style + cramped)
#define sup_style(a)     (2 * ((a) / 4) + script_style + ((a) % 2))
#define num_style(a)     ((a) + 2 - 2 * ((a) / 6))
#define denom_style(a)   (2 * ((a) / 2) + cramped + 2 - 2 * ((a) / 6))
/* sec 0716 */
#define mu_mult(a) nx_plus_y(n, a, xn_over_d(a, f, 0200000))
/* sec 0725 */
#define new_hlist(a) mem[nucleus(a)].cint
/* sec 0731 */
#define choose_mlist(a) \
do                      \
{                       \
  p = a(q);             \
  a(q) = 0;             \
}                       \
while (0)
/* sec 0770 */
#define preamble              link(align_head)
#define align_stack_node_size 5
/* sec 0780 */
#define span_code          256
#define cr_code            257
#define cr_cr_code         (cr_code + 1)
#define end_template_token (cs_token_flag + frozen_end_template)
/* sec 0797 */
#define span_node_size 2
/* sec 0817 */
#define tight_fit      3
#define loose_fit      1
#define very_loose_fit 0
#define decent_fit     2
/* sec 0819 */
#define active_node_size  3
#define fitness           subtype
#define break_node        rlink
#define line_number       llink
#define total_demerits(a) mem[a + 2].cint
#define unhyphenated      0
#define hyphenated        1
#define last_active       active
/* sec 0821 */
#define passive_node_size 2
#define cur_break         rlink
#define prev_break        llink
#define serial            info
/* sec 0822 */
#define delta_node_size 7
#define delta_node      2
/* sec 0823 */
#define do_all_six(a) \
do                    \
{                     \
  a(1); a(2); a(3);   \
  a(4); a(5); a(6);   \
}                     \
while (0)
/* sec 0825 */
#define check_shrinkage(s)                            \
do                                                    \
{                                                     \
  if ((shrink_order(s) != normal) && (shrink(s) != 0))\
    s = finite_shrink(s);                             \
}                                                     \
while (0)
/* sec 0829 */
#define copy_to_cur_active(a) cur_active_width[a] = active_width[a]
/* sec 0832 */
#define update_width(a) cur_active_width[a] = cur_active_width[a] + mem[r + (a)].cint
/* sec 0833 */
#define awful_bad 07777777777
/* sec 0837 */
#define set_break_width_to_background(a) break_width[a] = background[a]
/* sec 0843 */
#define convert_to_break_width(a)   mem[prev_r + (a)].cint = mem[prev_r + (a)].cint - cur_active_width[a] + break_width[a]
#define store_break_width(a)        active_width[a] = break_width[a]
#define new_delta_to_break_width(a) mem[q + (a)].cint = break_width[(a)] - cur_active_width[(a)]
/* sec 0844 */
#define new_delta_from_break_width(a) mem[q + (a)].cint = cur_active_width[(a)] - break_width[(a)]
/* sec 0860 */
#define combine_two_deltas(a) mem[prev_r + (a)].cint = mem[prev_r + (a)].cint + mem[r + (a)].cint
#define downdate_width(a)     cur_active_width[(a)] = cur_active_width[(a)] - mem[prev_r + (a)].cint
/* sec 0861 */
#define update_active(a) active_width[(a)] = active_width[(a)] + mem[r + (a)].cint
/* sec 0864 */
#define store_background(a) active_width[a] = background[a]
/* sec 0866 */
#define act_width active_width[1]
#define kern_break()                                \
do                                                  \
{                                                   \
  if (!is_char_node(link(cur_p)) && auto_breaking)  \
    if (type(link(cur_p)) == glue_node)             \
      try_break(0, unhyphenated);                   \
                                                    \
  act_width = act_width + width(cur_p);             \
}                                                   \
while (0)
/* sec 0877 */
#define next_break prev_break
/* sec 0908 */
#define append_charnode_to_t(a) \
do                              \
  {                             \
    link(t) = get_avail();      \
    t = link(t);                \
    font(t) = hf;               \
    character(t) = (a);         \
  }                             \
while (0)
#define set_cur_r()      \
do                       \
  {                      \
    if (j < n)           \
      cur_r = hu[j + 1]; \
    else                 \
      cur_r = bchar;     \
                         \
    if (odd(hyf[j]))     \
      cur_rh = hchar;    \
    else                 \
      cur_rh = non_char; \
  }                      \
while (0)
/* sec 0910 */
#define wrap_lig(a)                           \
do                                            \
{                                             \
  if (ligature_present)                       \
  {                                           \
    p = new_ligature(hf, cur_l, link(cur_q)); \
                                              \
    if (lft_hit)                              \
    {                                         \
      subtype(p) = 2;                         \
      lft_hit = false;                        \
    }                                         \
                                              \
    if ((a))                                  \
      if (lig_stack == 0)                     \
      {                                       \
        incr(subtype(p));                     \
        rt_hit = false;                       \
      }                                       \
                                              \
    link(cur_q) = p;                          \
    t = p;                                    \
    ligature_present = false;                 \
  }                                           \
}                                             \
while (0)
#define pop_lig_stack()                       \
do                                            \
  {                                           \
    if (lig_ptr(lig_stack) != 0)              \
    {                                         \
      link(t) = lig_ptr(lig_stack);           \
      t = link(t);                            \
      incr(j);                                \
    }                                         \
                                              \
    p = lig_stack;                            \
    lig_stack = link(p);                      \
    free_node(p, small_node_size);            \
                                              \
    if (lig_stack == 0)                       \
    {                                         \
      set_cur_r();                            \
    }                                         \
    else                                      \
      cur_r = character(lig_stack);           \
  }                                           \
while (0)
/* sec 0914 */
#define advance_major_tail()       \
do                                 \
  {                                \
    major_tail = link(major_tail); \
    incr(r_count);                 \
  }                                \
while (0)
/* sec 0970 */
#define active_height      active_width
#define cur_height         active_height[1]
#define set_height_zero(a) active_width[(a)] = 0
/* sec 0974 */
#define deplorable 100000L
/* sec 0980 */
#define inserts_only 1
#define box_there    2
/* sec 0981 */
#define page_ins_node_size 4
#define inserting          0
#define split_up           1
#define broken_ptr(a)      link(a + 1)
#define broken_ins(a)      info(a + 1)
#define last_ins_ptr(a)    link(a + 2)
#define best_ins_ptr(a)    info(a + 2)
/* sec 0982 */
#define page_goal   page_so_far[0]
#define page_total  page_so_far[1]
#define page_shrink page_so_far[6]
#define page_depth  page_so_far[7]
/* sec 0987 */
#define set_page_so_far_zero(a) page_so_far[(a)] = 0
/* sec 1034 */
#define adjust_space_factor()   \
do                              \
{                               \
  main_s = sf_code(cur_chr);    \
  if (main_s == 1000)           \
    space_factor = 1000;        \
  else if (main_s < 1000)       \
  {                             \
    if (main_s > 0)             \
      space_factor = main_s;    \
  }                             \
  else if (space_factor < 1000) \
    space_factor = 1000;        \
  else                          \
    space_factor = main_s;      \
}                               \
while (0)
/* sec 1035 */
/* -> false */
#define wrapup(a)                                         \
do                                                        \
{                                                         \
  if (cur_l < non_char)                                   \
  {                                                       \
    if (link(cur_q) != 0)                                 \
      if (character(tail) == hyphen_char[main_f])         \
        ins_disc = true;                                  \
                                                          \
    if (ligature_present)                                 \
    {                                                     \
      main_p = new_ligature(main_f, cur_l, link(cur_q));  \
                                                          \
      if (lft_hit)                                        \
      {                                                   \
        subtype(main_p) = 2;                              \
        lft_hit = false;                                  \
      }                                                   \
                                                          \
      if (a)                                              \
        if (lig_stack == 0)                               \
        {                                                 \
          incr(subtype(main_p));                          \
          rt_hit = false;                                 \
        }                                                 \
                                                          \
      link(cur_q) = main_p;                               \
      tail = main_p;                                      \
      ligature_present = false;                           \
    }                                                     \
                                                          \
    if (ins_disc)                                         \
    {                                                     \
      ins_disc = false;                                   \
                                                          \
      if (mode > 0)                                       \
      {                                                   \
        tail_append(new_disc());                          \
      }                                                   \
    }                                                     \
  }                                                       \
}                                                         \
while (0)
/* sec 1045 */
#define any_mode(a) vmode + a: case hmode + a: case mmode + a
/* sec 1046 */
#define non_math(a) vmode + a: case hmode + a
/* sec 1058 */
#define fil_code     0
#define fill_code    1
#define ss_code      2
#define fil_neg_code 3
#define skip_code    4
#define mskip_code   5
/* sec 1071 */
#define box_flag      010000000000
#define ship_out_flag (box_flag + 512)
#define leader_flag   (box_flag + 513)
#define box_code      0
#define copy_code     1
#define last_box_code 2
#define vsplit_code   3
#define vtop_code     4
/* sec 1178 */
#define above_code     0
#define over_code      1
#define atop_code      2
#define delimited_code 3
/* sec 1214 */
#define define(p, t, e) \
do                      \
{                       \
  if (a >= 4)           \
    geq_define(p, t, e);\
  else                  \
    eq_define(p, t, e); \
}                       \
while (0)
#define word_define(p, w) \
do                        \
{                         \
  if (a >= 4)             \
    geq_word_define(p, w);\
  else                    \
    eq_word_define(p, w); \
}                         \
while (0)
/* sec 1222 */
#define char_def_code      0
#define math_char_def_code 1
#define count_def_code     2
#define dimen_def_code     3
#define skip_def_code      4
#define mu_skip_def_code   5
#define toks_def_code      6
/* sec 1290 */
#define show_code     0
#define show_box_code 1
#define show_the_code 2
#define show_lists    3
/* sec 1306 */
#define undump(va, vb, vc)        \
do                                \
  {                               \
    undump_int(x);                \
                                  \
    if ((x < (va)) || (x > (vb))) \
      goto bad_fmt;               \
    else                          \
      vc = x;                     \
  }                               \
while (0)
/* sec 1342 */
#define write_node_size 2
#define open_node_size  3
#define open_node       0
#define write_node      1
#define close_node      2
#define special_node    3
#define language_node   4
#define what_lang(s)    link(s+1)
#define what_lhm(s)     type(s+1)
#define what_rhm(s)     subtype(s+1)
#define write_tokens(s) link(s+1)
#define write_stream(s) info(s+1)
#define open_name(s)    link(s+1)
#define open_area(s)    info(s+2)
#define open_ext(s)     link(s+2)
/* sec 1344 */
#define immediate_code    4
#define set_language_code 5
/* sec 1371 */
#define end_write_token (cs_token_flag + end_write)

#endif