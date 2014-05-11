/* Copyright 2007 TeX Users Group
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

/* #define USEREGISTERS tells compiler use registers for mem and eqtb bkph */
/* this may make executable slightly smaller and slightly faster ... */
/* HOWEVER, won't work anymore for mem with dynamic memory allocation ... */
/* so can use registers now *only* for eqtb ... NOT for mem ... */
/* It is OK for eqtb, because, even though we may allocate eqtb, */
/* we won't ever reallocate it ... */

void print_err (const char * s);
void initialize (void);
void print_ln (void);
void print_char_(ASCII_code);
#define print_char(s) print_char_((ASCII_code) (s))
void print_(integer);
#define print(s) print_((integer) (s))
void print_string_(unsigned char * s);
#define print_string(s) print_string_((unsigned char *) s)
void slow_print_(integer);
#define slow_print(s) slow_print_((integer) (s))
void print_nl_(const char *);
#define print_nl(s) print_nl_((const char *) (s))
void print_esc_(const char *);
#define print_esc(s) print_esc_((const char *) (s))
void print_the_digs_(eight_bits);
#define print_the_digs(k) print_the_digs_((eight_bits) (k))
void print_int_(integer);
#define print_int(n) print_int_((integer) (n))
void print_cs_(integer);
#define print_cs(p) print_cs_((integer) (p))
void sprint_cs_(halfword);
#define sprint_cs(p) sprint_cs_((halfword) (p))
void print_file_name_(integer, integer, integer);
#define print_file_name(n, a, e) print_file_name_((integer) (n), (integer) (a), (integer) (e))
void print_size_(integer);
#define print_size(s) print_size_((integer) (s))
void print_write_whatsit_(str_number, halfword);
#define print_write_whatsit(s, p) print_write_whatsit_((str_number) (s), (halfword) (p))
void jump_out(void);
void error(void);
void fatal_error_(char *);
#define fatal_error(s) fatal_error_((char *) (s))
void overflow_(char *, integer);
#define overflow(s, n) overflow_((char *) (s), (integer) (n))
void confusion_(char *);
#define confusion(s) confusion_((char *) (s))
bool init_terminal(void);
str_number make_string(void);
bool str_eq_buf_(str_number, integer);
#define str_eq_buf(s, k) str_eq_buf_((str_number) (s), (integer) (k))
bool str_eq_str_(str_number, str_number);
#define str_eq_str(s, t) str_eq_str_((str_number) (s), (str_number) (t))
bool get_strings_started(void);
void print_two_(integer);
#define print_two(n) print_two_((integer) (n))
void print_hex_(integer);
#define print_hex(n) print_hex_((integer) (n))
void print_roman_int_(integer);
#define print_roman_int(n) print_roman_int_((integer) (n))
void print_current_string(void);
void term_input(char *, int);
void int_error_(integer);
#define int_error(n) int_error_((integer) (n))
void normalize_selector(void);
void pause_for_instructions(void);
integer half_(integer);
#define half(x) half_((integer) (x))
scaled round_decimals_(small_number);
#define round_decimals(k) round_decimals_((small_number) (k))
void print_scaled_(scaled);
#define print_scaled(s) print_scaled_((scaled) (s))
scaled mult_and_add_(integer, scaled, scaled, scaled);
#define mult_and_add(n, x, y, maxanswer) mult_and_add_((integer) (n), (scaled) (x), (scaled) (y), (scaled) (maxanswer))
scaled x_over_n_(scaled, integer);
#define x_over_n(x, n) x_over_n_((scaled) (x), (integer) (n))
scaled xn_over_d_(scaled, integer, integer);
#define xn_over_d(x, n, d) xn_over_d_((scaled) (x), (integer) (n), (integer) (d))
halfword badness_(scaled, scaled);
#define badness(t, s) badness_((scaled) (t), (scaled) (s))
void print_word_(memory_word);
#define print_word(w) print_word_((memory_word) (w))
void show_token_list_(integer, integer, integer);
#define show_token_list(p, q, l) show_token_list_((integer) (p), (integer) (q), (integer) (l))
void runaway(void);
halfword get_avail(void);
void flush_list_(halfword);
#define flush_list(p) flush_list_((halfword) (p))
halfword get_node_(integer);
#define get_node(s) get_node_((integer) (s))
void free_node_(halfword, halfword);
#define free_node(p, s) free_node_((halfword) (p), (halfword) (s))
void sort_avail(void);
halfword new_null_box(void);
halfword new_rule(void);
halfword new_ligature_(quarterword, quarterword, halfword);
#define new_ligature(f, c, q) new_ligature_((quarterword) (f), (quarterword) (c), (halfword) (q))
halfword new_lig_item_(quarterword);
#define new_lig_item(c) new_lig_item_((quarterword) (c))
halfword new_disc(void);
halfword new_math_(scaled, small_number);
#define new_math(w, s) new_math_((scaled) (w), (small_number) (s))
halfword new_spec_(halfword);
#define new_spec(p) new_spec_((halfword) (p))
halfword new_param_glue_(small_number);
#define new_param_glue(n) new_param_glue_((small_number) (n))
halfword new_glue_(halfword);
#define new_glue(q) new_glue_((halfword) (q))
halfword new_skip_param_(small_number);
#define new_skip_param(n) new_skip_param_((small_number) (n))
halfword new_kern_(scaled);
#define new_kern(w) new_kern_((scaled) (w))
halfword new_penalty_(integer);
#define new_penalty(m) new_penalty_((integer) (m))
void check_mem_(bool);
#define check_mem(printlocs) check_mem_((bool) (printlocs))
void search_mem_(halfword);
#define search_mem(p) search_mem_((halfword) (p))
void short_display_(integer);
#define short_display(p) short_display_((integer) (p))
void print_font_and_char_(integer);
#define print_font_and_char(p) print_font_and_char_((integer) (p))
void print_mark_(integer);
#define print_mark(p) print_mark_((integer) (p))
void print_rule_dimen_(scaled);
#define print_rule_dimen(d) print_rule_dimen_((scaled) (d))
void print_glue_(scaled, integer, char *);
#define print_glue(d, order, s) print_glue_((scaled) (d), (integer) (order), (char *) (s))
void print_spec_(integer, char *);
#define print_spec(p, s) print_spec_((integer) (p), (char *) (s))
void print_fam_and_char_(halfword);
#define print_fam_and_char(p) print_fam_and_char_((halfword) (p))
void print_delimiter_(halfword);
#define print_delimiter(p) print_delimiter_((halfword) (p))
void print_subsidiary_data_(halfword, ASCII_code);
#define print_subsidiary_data(p, c) print_subsidiary_data_((halfword) (p), (ASCII_code) (c))
void print_style_(integer);
#define print_style(c) print_style_((integer) (c))
void print_skip_param_(integer);
#define print_skip_param(n) print_skip_param_((integer) (n))
void show_node_list_(integer);
#define show_node_list(p) show_node_list_((integer) (p))
void show_box_(halfword);
#define show_box(p) show_box_((halfword) (p))
void delete_token_ref_(halfword);
#define delete_token_ref(p) delete_token_ref_((halfword) (p))
void delete_glue_ref_(halfword);
#define delete_glue_ref(p) delete_glue_ref_((halfword) (p))
void flush_node_list_(halfword);
#define flush_node_list(p) flush_node_list_((halfword) (p))
halfword copy_node_list_(halfword);
#define copy_node_list(p) copy_node_list_((halfword) (p))
void print_mode_(integer);
#define print_mode(m) print_mode_((integer) (m))
void push_nest(void);
void pop_nest(void);
void show_activities(void);
void print_param_(integer);
#define print_param(n) print_param_((integer) (n))
void begin_diagnostic(void);
void end_diagnostic_(bool);
#define end_diagnostic(blankline) end_diagnostic_((bool) (blankline))
void print_length_param_(integer);
#define print_length_param(n) print_length_param_((integer) (n))
void print_cmd_chr_(quarterword, halfword);
#define print_cmd_chr(cmd, chr_code) print_cmd_chr_((quarterword) (cmd), (halfword) (chr_code))
void show_eqtb_(halfword);
#define show_eqtb(n) show_eqtb_((halfword) (n))
halfword id_lookup_(integer, integer);
#define id_lookup(j, l) id_lookup_((integer) (j), (integer) (l))
str_number make_string_pool (char *s);
void primitive_s (char * s, quarterword c, halfword o);
void primitive_(str_number, quarterword, halfword);
#define primitive(s, c, o) primitive_(make_string_pool((char *) s), (quarterword) (c), (halfword) (o))
void new_save_level_(group_code);
#define new_save_level(c) new_save_level_((group_code) (c))
void eq_destroy_(memory_word);
#define eq_destroy(w) eq_destroy_((memory_word) (w))
void eq_save_(halfword, quarterword);
#define eq_save(p, l) eq_save_((halfword) (p), (quarterword) (l))
void eq_define_(halfword, quarterword, halfword);
#define eq_define(p, t, e) eq_define_((halfword) (p), (quarterword) (t), (halfword) (e))
void eq_word_define_(halfword, integer);
#define eq_word_define(p, w) eq_word_define_((halfword) (p), (integer) (w))
void geq_define_(halfword, quarterword, halfword);
#define geq_define(p, t, e) geq_define_((halfword) (p), (quarterword) (t), (halfword) (e))
void geq_word_define_(halfword, integer);
#define geq_word_define(p, w) geq_word_define_((halfword) (p), (integer) (w))
void save_for_after_(halfword);
#define save_for_after(t) save_for_after_((halfword) (t))
void restore_trace_(halfword, char *);
#define restore_trace(p, s) restore_trace_((halfword) (p), (char *) (s))
void unsave(void);
void prepare_mag(void);
void token_show_(halfword);
#define token_show(p) token_show_((halfword) (p))
void print_meaning(void);
void show_cur_cmd_chr(void);
void show_context(void);
void begin_token_list_(halfword, quarterword);
#define begin_token_list(p, t) begin_token_list_((halfword) (p), (quarterword) (t))
void end_token_list(void);
void back_input(void);
void back_error(void);
void ins_error(void);
void begin_file_reading(void);
void end_file_reading(void);
void clear_for_error_prompt(void);
void check_outer_validity(void);
void get_next(void);
void firm_up_the_line(void);
void get_token(void);
void macro_call(void);
void insert_relax(void);
void expand(void);
void get_x_token(void);
void x_token(void);
void scan_left_brace(void);
void scan_optional_equals(void);
bool scan_keyword_(char *);
#define scan_keyword(s) scan_keyword_((char *) (s))
void mu_error(void);
void scan_eight_bit_int(void);
void scan_char_num(void);
void scan_four_bit_int(void);
void scan_fifteen_bit_int(void);
void scan_twenty_seven_bit_int(void);
void scan_font_ident(void);
void find_font_dimen_(bool);
#define find_font_dimen(writing) find_font_dimen_((bool) (writing))
void scan_something_internal_(small_number, bool);
#define scan_something_internal(level, negative) scan_something_internal_((small_number) (level), (bool) (negative))
void scan_int(void);
void scan_dimen_(bool, bool, bool);
#define scan_dimen(mu, inf, shortcut) scan_dimen_((bool) (mu), (bool) (inf), (bool) (shortcut))
void scan_glue_(small_number);
#define scan_glue(level) scan_glue_((small_number) (level))
halfword scan_rule_spec(void);
halfword str_toks_(pool_pointer);
#define str_toks(b) str_toks_((pool_pointer) (b))
halfword the_toks(void);
void ins_the_toks(void);
void conv_toks(void);
halfword scan_toks_(bool, bool);
#define scan_toks(macrodef, xpand) scan_toks_((bool) (macrodef), (bool) (xpand))
void read_toks_(integer, halfword);
#define read_toks(n, r) read_toks_((integer) (n), (halfword) (r))
void pass_text(void);
void change_if_limit_(small_number, halfword);
#define change_if_limit(l, p) change_if_limit_((small_number) (l), (halfword) (p))
void conditional(void);
void begin_name(void);
bool more_name_(ASCII_code);
#define more_name(c) more_name_((ASCII_code) (c))
void end_name(void);
void pack_file_name_(str_number, str_number, str_number);
#define pack_file_name(n, a, e) pack_file_name_((str_number) (n), (str_number) (a), (str_number) (e))
void pack_buffered_name_(small_number, integer, integer);
#define pack_buffered_name(n, a, b) pack_buffered_name_((small_number) (n), (integer) (a), (integer) (b))
str_number make_name_string(void);
str_number a_make_name_string_(alpha_file *);
#define a_make_name_string(f) a_make_name_string_((alpha_file *) &(f))
str_number b_make_name_string_(byte_file *);
#define b_make_name_string(f) b_make_name_string_((byte_file *) &(f))
str_number w_make_name_string_(word_file *);
#define w_make_name_string(f) w_make_name_string_((word_file *) &(f))
void scan_file_name(void);
void pack_job_name_(str_number);
#define pack_job_name(s) pack_job_name_(make_string_pool((char*)s))
void prompt_file_name_(char *, str_number);
#define prompt_file_name(s, e) prompt_file_name_((char *) s, make_string_pool((char*)e))
void open_log_file(void);
void start_input(void);
internal_font_number read_font_info_(halfword, str_number, str_number, scaled);
#define read_font_info(u, nom, aire, s) read_font_info_((halfword) (u), (str_number) (nom), (str_number) (aire), (scaled) (s))
void char_warning_(internal_font_number, eight_bits);
#define char_warning(f, c) char_warning_((internal_font_number) (f), (eight_bits) (c))
halfword new_character_(internal_font_number, eight_bits);
#define new_character(f, c) new_character_((internal_font_number) (f), (eight_bits) (c))
#ifdef ALLOCATEDVIBUF
  void dvi_swap(void);
  void dvi_four_(integer);
  #define dvi_four(x) dvi_four_((integer) (x))
  void zdvipop(integer);
  #define dvi_pop(l) zdvipop((integer) (l))
  void dvi_font_def_(internal_font_number);
  #define dvi_font_def(f) dvi_font_def_((internal_font_number) (f))
  void zmovement(scaled, eight_bits);
  #define movement(w, o) zmovement((scaled) (w), (eight_bits) (o))
  void special_out_(halfword);
  #define special_out(p) special_out_((halfword) (p))
  void hlist_out(void);
  void vlist_out(void);
  void ship_out_(halfword);
  #define ship_out(p) ship_out_((halfword) (p))
#else /* not ALLOCATEDVIBUF */
  void dvi_swap(void);
  void dvi_four_(integer);
  #define dvi_four(x) dvi_four_((integer) (x))
  void zdvipop(integer);
  #define dvi_pop(l) zdvipop((integer) (l))
  void dvi_font_def_(internal_font_number);
  #define dvi_font_def(f) dvi_font_def_((internal_font_number) (f))
  void zmovement(scaled, eight_bits);
  #define movement(w, o) zmovement((scaled) (w), (eight_bits) (o))
  void special_out_(halfword);
  #define special_out(p) special_out_((halfword) (p))
  void hlist_out(void);
  void vlist_out(void);
  void ship_out_(halfword);
  #define ship_out(p) ship_out_((halfword) (p))
#endif
void prune_movements_(integer);
#define prune_movements(l) prune_movements_((integer) (l))
void write_out_(halfword);
#define write_out(p) write_out_((halfword) (p))
void out_what_(halfword);
#define out_what(p) out_what_((halfword) (p))
void scan_spec_(group_code, bool);
#define scan_spec(c, threecodes) scan_spec_((group_code) (c), (bool) (threecodes))
halfword hpack_(halfword, scaled, small_number);
#define hpack(p, w, m) hpack_((halfword) (p), (scaled) (w), (small_number) (m))
halfword vpackage_(halfword, scaled, small_number, scaled);
#define vpackage(p, h, m, l) vpackage_((halfword) (p), (scaled) (h), (small_number) (m), (scaled) (l))
void append_to_vlist_(halfword);
#define append_to_vlist(b) append_to_vlist_((halfword) (b))
halfword new_noad(void);
halfword new_style_(small_number);
#define new_style(s) new_style_((small_number) (s))
halfword new_choice(void);
void show_info(void);
halfword fraction_rule_(scaled);
#define fraction_rule(t) fraction_rule_((scaled) (t))
halfword overbar_(halfword, scaled, scaled);
#define overbar(b, k, t) overbar_((halfword) (b), (scaled) (k), (scaled) (t))
halfword char_box_(internal_font_number, quarterword);
#define char_box(f, c) char_box_((internal_font_number) (f), (quarterword) (c))
void stack_into_box_(halfword, internal_font_number, quarterword);
#define stack_into_box(b, f, c) stack_into_box_((halfword) (b), (internal_font_number) (f), (quarterword) (c))
scaled height_plus_depth_(internal_font_number, fquarterword); 
#define height_plus_depth(f, c) height_plus_depth_((internal_font_number) (f), (fquarterword) (c))
halfword var_delimiter_(halfword, small_number, scaled);
#define var_delimiter(d, s, v) var_delimiter_((halfword) (d), (small_number) (s), (scaled) (v))
halfword rebox_(halfword, scaled);
#define rebox(b, w) rebox_((halfword) (b), (scaled) (w))
halfword math_glue_(halfword, scaled);
#define math_glue(g, m) math_glue_((halfword) (g), (scaled) (m))
void math_kern_(halfword, scaled);
#define math_kern(p, m) math_kern_((halfword) (p), (scaled) (m))
void flush_math(void);
halfword clean_box_(halfword, small_number);
#define clean_box(p, s) clean_box_((halfword) (p), (small_number) (s))
void fetch_(halfword);
#define fetch(a) fetch_((halfword) (a))
void make_over_(halfword);
#define make_over(q) make_over_((halfword) (q))
void make_under_(halfword);
#define make_under(q) make_under_((halfword) (q))
void make_vcenter_(halfword);
#define make_vcenter(q) make_vcenter_((halfword) (q))
void make_radical_(halfword);
#define make_radical(q) make_radical_((halfword) (q))
void make_math_accent_(halfword);
#define make_math_accent(q) make_math_accent_((halfword) (q))
void make_fraction_(halfword);
#define make_fraction(q) make_fraction_((halfword) (q))
scaled make_op_(halfword);
#define make_op(q) make_op_((halfword) (q))
void make_ord_(halfword);
#define make_ord(q) make_ord_((halfword) (q))
void make_scripts_(halfword, scaled);
#define make_scripts(q, delta) make_scripts_((halfword) (q), (scaled) (delta))
small_number make_left_right_(halfword, small_number, scaled, scaled);
#define make_left_right(q, style, maxd, max_h) make_left_right_((halfword) (q), (small_number) (style), (scaled) (maxd), (scaled) (max_h))
void mlist_to_hlist(void);
void push_alignment(void);
void pop_alignment(void);
void get_preamble_token(void);
void init_align(void);
void init_span_(halfword);
#define init_span(p) init_span_((halfword) (p))
void init_row(void);
void init_col(void);
bool fin_col(void);
void fin_row(void);
void fin_align(void);
void align_peek(void);
halfword finite_shrink_(halfword);
#define finite_shrink(p) finite_shrink_((halfword) (p))
void try_break_(integer, small_number);
#define try_break(pi, breaktype) try_break_((integer) (pi), (small_number) (breaktype))
void post_line_break_(integer);
#define post_line_break(final_widow_penalty) post_line_break_((integer) (final_widow_penalty))
small_number reconstitute_(small_number, small_number, halfword, halfword); 
#define reconstitute(j, n, bchar, hchar) reconstitute_((small_number) (j), (small_number) (n), (halfword) (bchar), (halfword) (hchar))
void hyphenate(void);
trie_op_code new_trie_op_(small_number, small_number, trie_op_code);
#define new_trie_op(d, n, v) new_trie_op_((small_number) (d), (small_number) (n), (trie_op_code) (v))
trie_pointer trie_node_(trie_pointer);
#define trie_node(p) trie_node_((trie_pointer) (p))
trie_pointer compress_trie_(trie_pointer);
#define compress_trie(p) compress_trie_((trie_pointer) (p))
void first_fit_(trie_pointer);
#define first_fit(p) first_fit_((trie_pointer) (p))
void trie_pack_(trie_pointer);
#define trie_pack(p) trie_pack_((trie_pointer) (p))
void trie_fix_(trie_pointer);
#define trie_fix(p) trie_fix_((trie_pointer) (p))
void new_patterns(void);
void init_trie(void);
void line_break_(integer);
#define line_break(final_widow_penalty) line_break_((integer) (final_widow_penalty))
void new_hyph_exceptions(void);
halfword prune_page_top_(halfword);
#define prune_page_top(p) prune_page_top_((halfword) (p))
halfword vert_break_(halfword, scaled, scaled);
#define vert_break(p, h, d) vert_break_((halfword) (p), (scaled) (h), (scaled) (d))
halfword vsplit_(eight_bits, scaled);
#define vsplit(n, h) vsplit_((eight_bits) (n), (scaled) (h))
void print_totals(void);
void freeze_page_specs_(small_number);
#define freeze_page_specs(s) freeze_page_specs_((small_number) (s))
void box_error_(eight_bits);
#define box_error(n) box_error_((eight_bits) (n))
void ensure_vbox_(eight_bits);
#define ensure_vbox(n) ensure_vbox_((eight_bits) (n))
void fire_up_(halfword);
#define fire_up(c) fire_up_((halfword) (c))
void build_page(void);
void app_space(void);
void insert_dollar_sign(void);
void you_cant(void);
void report_illegal_case(void);
bool privileged(void);
bool its_all_over(void);
void append_glue(void);
void append_kern(void);
void off_save(void);
void extra_right_brace(void);
void normal_paragraph(void);
void box_end_(integer);
#define box_end(boxcontext) box_end_((integer) (boxcontext))
void begin_box_(integer);
#define begin_box(boxcontext) begin_box_((integer) (boxcontext))
void scan_box_(integer);
#define scan_box(boxcontext) scan_box_((integer) (boxcontext))
void package_(small_number);
#define package(c) package_((small_number) (c))
small_number norm_min_(integer);
#define norm_min(h) norm_min_((integer) (h))
void new_graf_(bool);
#define new_graf(indented) new_graf_((bool) (indented))
void indent_in_hmode(void);
void head_for_vmode(void);
void end_graf(void);
void begin_insert_or_adjust(void);
void make_mark(void);
void append_penalty(void);
void delete_last(void);
void unpackage(void);
void append_italic_correction(void);
void append_discretionary(void);
void build_discretionary(void);
void make_accent(void);
void align_error(void);
void noalign_error(void);
void omit_error(void);
void do_endv(void);
void cs_error(void);
void push_math_(group_code);
#define push_math(c) push_math_((group_code) (c))
void init_math(void);
void start_eq_no(void);
void scan_math_(halfword);
#define scan_math(p) scan_math_((halfword) (p))
void set_math_char_(integer);
#define set_math_char(c) set_math_char_((integer) (c))
void math_limit_switch(void);
void scan_delimiter_(halfword, bool);
#define scan_delimiter(p, r) scan_delimiter_((halfword) (p), (bool) (r))
void math_radical(void);
void math_ac(void);
void append_choices(void);
halfword fin_mlist_(halfword);
#define fin_mlist(p) fin_mlist_((halfword) (p))
void build_choices(void);
void sub_sup(void);
void math_fraction(void);
void math_left_right(void);
void after_math(void);
void resume_after_display(void);
void get_r_token(void);
void trap_zero_glue(void);
void do_register_command_(small_number);
#define do_register_command(a) do_register_command_((small_number) (a))
void alter_aux(void);
void alter_prev_graf(void);
void alter_page_so_far(void);
void alter_integer(void);
void alter_box_dimen(void);
void new_font_(small_number);
#define new_font(a) new_font_((small_number) (a))
void new_interaction(void);
void prefixed_command(void);
void do_assignments(void);
void open_or_close_in(void);
void issue_message(void);
void shift_case(void);
void show_whatever(void);
void store_fmt_file(void);
void new_whatsit_(small_number, small_number);
#define new_whatsit(s, w) new_whatsit_((small_number) (s), (small_number) (w))
void new_write_whatsit_(small_number);
#define new_write_whatsit(w) new_write_whatsit_((small_number) (w))
void do_extension(void);
void fix_language(void);
void handle_right_brace(void);
void main_control(void);
void give_err_help(void);
bool open_fmt_file(void);
bool load_fmt_file(void);
void close_files_and_terminate(void);
void final_cleanup(void);
void init_prim(void);
void debug_help(void);
int texbody(void);          /* 1993/Dec/16 bkph */

/* may want to consider copying other addresses to local registers ... */

/*
 * The C compiler ignores most unnecessary casts (i.e., casts of something
 * to its own type).  However, for structures, it doesn't.  Therefore,
 * we have to redefine these two macros so that they don't try to cast
 * the argument (a memory_word) as a memory_word.
 */
#undef  eq_destroy
#define eq_destroy(x)  eq_destroy_(x)
#undef  print_word
#define print_word(x)  print_word_(x)
