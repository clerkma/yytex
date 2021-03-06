#ifndef _YANDYTEX_H
#define _YANDYTEX_H

#define ALLOCATEINI        /* allocate iniTeX (550 k) trie_c, trie_o, trie_l, trie_r, trie_hash, trie_taken */
#define ALLOCATEMAIN       /* allocate main memory for TeX (2 Meg) */
#define ALLOCATEFONT       /* allocate font_info (800 k) (dynamically now) */
#define ALLOCATETRIES      /* allocate hyphenation trie stuff (270 k) trie_trl, trie_tro, trie_trc */
#define ALLOCATEHYPHEN     /* allocate hyphenation exception tables */
#define VARIABLETRIESIZE   /* allow trie_size to be variable */
#define ALLOCATESTRING     /* allocate strings and string pointers (184 k) str_pool & str_start */
#define ALLOCATESAVESTACK  /* experiment to dynamically deal with save_stack   */
#define ALLOCATEINPUTSTACK /* experiment to dynamically deal with input_stack  */
#define ALLOCATENESTSTACK  /* experiment to dynamically deal with nest_stack   */
#define ALLOCATEPARAMSTACK /* experiment to dynamically deal with param_stack  */
#define ALLOCATEBUFFER     /* experiment to dynamically deal with input buffer */
#define INCREASEFIXED      /* max_in_open */
#define INCREASEFONTS      /* 65536 fonts */
#define INCREASETRIEOP     /* tire_* */
#define COMPACTFORMAT      /* .fmt file with zlib */
#define STAT               /* TeX's statistics (tex82) */
#define INITEX             /* invoke initex */
#define WORDS_BIGENDIAN 0  /* about format file */

#include "texd.h"

#define file_name_size PATH_MAX

#define min_halfword -2147483647L /* LONG_MIN, for 64 bit memory word (signed) */
#define max_halfword  2147483647L /* LONG_MAX, for 64 bit memory word (signed) */

#define block_size 1000 /* block_size for variable length node alloc */

#define min_quarterword 0
#ifdef INCREASEFONTS
  #define max_quarterword 65535L
#else
  #define max_quarterword 255
#endif

#define default_mem_top 262140L  /* usual big TeX allocation 2 Meg bytes */
/* #define default_mem_top 131070L */ /* usual big TeX allocation 1 Meg bytes */
/* #define default_mem_top 65534L  */ /* usual small TeX allocation 0.5 Meg   */

#define mem_bot 0

#ifdef ALLOCATEMAIN
  EXTERN integer mem_top;
  EXTERN integer mem_max;
  EXTERN integer mem_min;
  #define max_mem_size (max_halfword / sizeof(memory_word) - 1)
#else
  #define mem_top 262140L
  #define mem_max mem_top
  #define mem_min 0
#endif

#ifdef ALLOCATEBUFFER
  #define initial_buf_size   1000
  #define increment_buf_size 2000
  #define buf_size           2000000L
  EXTERN ASCII_code *        buffer;
#else
  #define buf_size           20000
  EXTERN ASCII_code          buffer[buf_size + 4];
#endif

EXTERN integer first; 
EXTERN integer last; 
EXTERN integer max_buf_stack; 

#define error_line      79
#define half_error_line 50
#define max_print_line  79

#ifdef INCREASEFIXED
  #define max_in_open 127
#else
  #define max_in_open 15
#endif

#ifdef INCREASEFONTS
  #define font_max 65535
#else
  #define font_max 255
#endif

#ifdef ALLOCATEFONT
  #define font_mem_size (max_halfword / sizeof(memory_word) - 1)
  #define initial_font_mem_size   20000
  #define increment_font_mem_size 40000
#else
  #define font_mem_size 100000L
#endif

#ifdef ALLOCATESTRING
  #define max_strings (max_halfword / sizeof(pool_pointer) - 1)
  #define pool_size (max_halfword - 1)
#else
  #define max_strings 16384
  #define pool_size 124000L
#endif

#define string_vacancies 100000L

#ifdef VARIABLETRIESIZE
  EXTERN integer trie_size;
  #define default_trie_size 60000
#else
  #define trie_size 30000
#endif

#ifdef INCREASETRIEOP
  #define trie_op_size      3001
  #define neg_trie_op_size -3001
  #define min_trie_op       0
  #define max_trie_op       1000
#else
  #define trie_op_size      751
  #define neg_trie_op_size -751
  #define min_trie_op       0
  #define max_trie_op       500
#endif


#define dvi_buf_size 16384

#define hash_extra (255 - font_max)
#define hash_prime 27197 // (prime ~ 85% * (hash_size + hash_extra))
#define hash_size  97280 // 32768 9500 25000

#if (hash_extra != 255 - font_max)
  #error ERROR: hash_extra not equal to (255 - font_max)
#endif

/* sec 0113 */
#ifdef INCREASEFONTS
  typedef unsigned short quarterword;
#else
  typedef unsigned char quarterword;
#endif

/* typedef unsigned long halfword; NO NO: since mem_min may be < 0 */
/* sec 0113 */
typedef int32_t halfword;
typedef halfword pointer;
typedef char two_choices;
typedef char four_choices;
/* sec 0113 */
#include "memory.h"
#include "macros.h"
/* sec 0150 */
typedef char glue_ord; 
/* sec 0212 */
typedef struct
{
  int mode_field;
  halfword head_field, tail_field;
  integer pg_field, ml_field;
  memory_word aux_field;
} list_state_record;
/* sec 0269 */
typedef char group_code;
/* sec 0300 */
typedef struct
{
  quarterword state_field, index_field; 
  halfword start_field, loc_field, limit_field, name_field;
} in_state_record; 
/* sec 0548 */
typedef integer internal_font_number;
typedef integer font_index;
/* sec 0594 */
typedef integer dvi_index;
/* sec 0920 */
typedef integer trie_op_code;
/* sec 0925 */
typedef integer trie_pointer;
typedef integer hyph_pointer;

EXTERN integer bad;
EXTERN ASCII_code xord[256];
EXTERN ASCII_code xchr[256];
EXTERN ASCII_code name_of_file[file_name_size + 4];
EXTERN integer name_length;

#ifdef ALLOCATESTRING
  #define initial_pool_size     40000
  #define increment_pool_size   80000
  EXTERN packed_ASCII_code *    str_pool;
  #define initial_max_strings   5000
  #define increment_max_strings 10000
  EXTERN pool_pointer *         str_start;
#else
  EXTERN packed_ASCII_code      str_pool[pool_size + 1]; 
  EXTERN pool_pointer           str_start[max_strings + 1]; 
#endif

EXTERN pool_pointer pool_ptr;
EXTERN str_number   str_ptr;
EXTERN pool_pointer init_pool_ptr;
EXTERN str_number   init_str_ptr;

#ifdef INITEX
  EXTERN alpha_file pool_file; 
#endif

EXTERN alpha_file log_file; 
EXTERN int selector;
EXTERN char dig[23 + 1];
EXTERN integer tally;
EXTERN integer term_offset;
EXTERN integer file_offset;
EXTERN ASCII_code trick_buf[error_line + 1];
EXTERN integer trick_count;
EXTERN integer first_count;
EXTERN int interaction;
EXTERN boolean deletions_allowed;
EXTERN boolean set_box_allowed;
EXTERN int history;
EXTERN int error_count;
EXTERN char * help_line[6];
EXTERN int help_ptr;
EXTERN boolean use_err_help;
EXTERN volatile integer interrupt;
EXTERN boolean OK_to_interrupt;
EXTERN boolean arith_error;
EXTERN scaled tex_remainder;
EXTERN halfword temp_ptr;

/* sec 0116 */
#ifdef ALLOCATEMAIN
  EXTERN memory_word * main_memory;
  EXTERN memory_word * mem;
#else
  EXTERN memory_word zzzaa[mem_max - mem_bot + 1];
  #define zmem (zzzaa - (int)(mem_bot))
#endif

EXTERN pointer lo_mem_max;
EXTERN pointer hi_mem_min;
EXTERN integer var_used, dyn_used;
/* sec 0118 */
EXTERN pointer avail;
EXTERN pointer mem_end;
EXTERN pointer mem_start;
/* sec 0124 */
EXTERN pointer rover;
/* sec 0165 */
/* NOTE: the following really also need to be dynamically allocated */
#ifdef DEBUG
  #ifdef ALLOCATEMAIN
    EXTERN char * zzzab;
  #else

  EXTERN char
#define freearr (zzzab - (int)(mem_bot))
  zzzab[mem_max - mem_bot + 1]; 
#endif

#ifdef ALLOCATEMAIN
  EXTERN char *zzzac;
#else
/* EXTERN boolean */   /* save (4 - 1) * mem_max - mem_min */
EXTERN char
/* #define wasfree (zzzac - (int)(mem_min)) */
#define wasfree (zzzac - (int)(mem_bot))
/*  zzzac[mem_max - mem_min + 1];  */
  zzzac[mem_max - mem_bot + 1]; 
#endif

EXTERN pointer was_mem_end, was_lo_max, was_hi_min;
EXTERN boolean panicking;
#endif /* DEBUG */

EXTERN integer font_in_short_display;
EXTERN integer depth_threshold;
EXTERN integer breadth_max;
EXTERN int shown_mode;
EXTERN int old_setting;

#ifdef INCREASEFONTS
  #define eqtb_extra (font_max - 255 + hash_extra)
#else
  #define eqtb_extra 0
#endif

#if (eqtb_extra != 0)
  #error ERROR: eqtb_extra is not zero (need hash_extra equal 255 - font_max)
#endif

#ifdef INCREASEFONTS
  EXTERN memory_word eqtb[eqtb_size + 1 + eqtb_extra];
#else
  EXTERN memory_word eqtb[eqtb_size + 1];
#endif

#ifdef INCREASEFONTS
  #define xeq_level (zzzad - (int_base + eqtb_extra))
#else
  #define xeq_level (zzzad - (int_base))
#endif

EXTERN quarterword zzzad[eqtb_size - int_base + 1];

#ifdef ALLOCATEHASH
  EXTERN two_halves *zzzae;
  #define hash (zzzae - hash_base)
#else
  #ifdef INCREASEFONTS
    EXTERN two_halves zzzae[undefined_control_sequence - hash_base + eqtb_extra];
  #else
    EXTERN two_halves zzzae[undefined_control_sequence - hash_base];
  #endif

  #define hash (zzzae - hash_base)
#endif

EXTERN pointer hash_used;
EXTERN boolean no_new_control_sequence;
EXTERN integer cs_count;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* using allocated save stack slows it down 1% to 2%                       */
/* despite reallocation, we still limit it to something finite             */
/* to avoid soaking up all of machine memory in case of infinite loop      */
#ifdef ALLOCATESAVESTACK
  #define save_size           65536
  #define initial_save_size   1000
  #define increment_save_size 2000
  EXTERN memory_word * save_stack;
#else
  #define save_size 8000
  EXTERN memory_word save_stack[save_size + 1];
#endif

EXTERN integer save_ptr;
EXTERN integer max_save_stack;
EXTERN int cur_level;
EXTERN int cur_group;
EXTERN integer cur_boundary;
EXTERN integer mag_set;
EXTERN int cur_cmd;
EXTERN halfword cur_chr;
EXTERN pointer cur_cs;
EXTERN halfword cur_tok;

#ifdef ALLOCATENESTSTACK
  #define nest_size           65536
  #define initial_nest_size   100
  #define increment_nest_size 200
  EXTERN list_state_record * nest;
#else
  #define nest_size 200
  EXTERN list_state_record nest[nest_size + 1];
#endif

EXTERN integer nest_ptr;
EXTERN integer max_nest_stack;
EXTERN list_state_record cur_list;

#ifdef ALLOCATEPARAMSTACK
  #define param_size           65536
  #define initial_param_size   100
  #define increment_param_size 200
  EXTERN pointer * param_stack;
#else
  #define param_size 500
  EXTERN pointer param_stack[param_size + 1];
#endif

EXTERN integer param_ptr;
EXTERN integer max_param_stack;

#ifdef ALLOCATEINPUTSTACK
  #define stack_size           65536
  #define initial_stack_size   100
  #define increment_stack_size 200
  EXTERN in_state_record * input_stack;
#else
  #define stack_size 800
  EXTERN in_state_record input_stack[stack_size + 1];
#endif

EXTERN integer input_ptr;
EXTERN integer max_in_stack;
EXTERN integer high_in_open;
EXTERN in_state_record cur_input;
EXTERN int in_open;
EXTERN integer open_parens;
EXTERN integer max_open_parens;
EXTERN alpha_file input_file[max_in_open + 1];
EXTERN integer line;
EXTERN integer line_stack[max_in_open + 1];
EXTERN int scanner_status;
EXTERN pointer warning_index;
EXTERN pointer def_ref;
EXTERN integer align_state;
EXTERN integer base_ptr;
EXTERN pointer par_loc;
EXTERN halfword par_token;
EXTERN boolean force_eof;
EXTERN pointer cur_mark[6];
EXTERN int long_state;
EXTERN pointer pstack[10];
EXTERN integer cur_val;
EXTERN int cur_val_level;
EXTERN int radix;
EXTERN int cur_order;
EXTERN alpha_file read_file[16];
EXTERN char read_open[20];
EXTERN pointer cond_ptr;
EXTERN int if_limit;
EXTERN int cur_if;
EXTERN integer if_line;
EXTERN integer skip_line;
EXTERN str_number cur_name;
EXTERN str_number cur_area;
EXTERN str_number cur_ext;
EXTERN pool_pointer area_delimiter;
EXTERN pool_pointer ext_delimiter;
EXTERN integer format_default_length;
EXTERN char * TEX_format_default;
EXTERN boolean name_in_progress;
EXTERN boolean log_opened;
EXTERN boolean quoted_file_name;
EXTERN str_number job_name;
EXTERN const char * c_job_name;
EXTERN str_number output_file_name;
EXTERN str_number log_name;
EXTERN byte_file dvi_file;
EXTERN byte_file tfm_file;
EXTERN byte_file pdf_file;
EXTERN char * dvi_file_name;
EXTERN char * pdf_file_name;
EXTERN char * log_file_name;

#ifdef ALLOCATEFONT
  EXTERN memory_word * font_info;
#else
  EXTERN memory_word font_info[font_mem_size + 1];
#endif

EXTERN font_index fmem_ptr;
EXTERN internal_font_number font_ptr;
EXTERN internal_font_number frozen_font_ptr;
EXTERN four_quarters font_check[font_max + 1];
EXTERN scaled font_size[font_max + 1];
EXTERN scaled font_dsize[font_max + 1];
EXTERN font_index font_params[font_max + 1];
EXTERN str_number font_name[font_max + 1];
EXTERN str_number font_area[font_max + 1];
EXTERN eight_bits font_bc[font_max + 1];
EXTERN eight_bits font_ec[font_max + 1];
EXTERN pointer font_glue[font_max + 1];
EXTERN boolean font_used[font_max + 1];
EXTERN integer hyphen_char[font_max + 1];
EXTERN integer skew_char[font_max + 1];
EXTERN font_index bchar_label[font_max + 1];
EXTERN short font_bchar[font_max + 1];
EXTERN short font_false_bchar[font_max + 1];
EXTERN integer char_base[font_max + 1];
EXTERN integer width_base[font_max + 1];
EXTERN integer height_base[font_max + 1];
EXTERN integer depth_base[font_max + 1];
EXTERN integer italic_base[font_max + 1];
EXTERN integer lig_kern_base[font_max + 1];
EXTERN integer kern_base[font_max + 1];
EXTERN integer exten_base[font_max + 1];
EXTERN integer param_base[font_max + 1];
EXTERN four_quarters null_character;
EXTERN integer total_pages;
EXTERN scaled max_v;
EXTERN scaled max_h;
EXTERN integer max_push;
EXTERN integer last_bop;
EXTERN integer dead_cycles;
EXTERN boolean doing_leaders;
EXTERN quarterword c, f;
EXTERN scaled rule_ht, rule_dp, rule_wd;
EXTERN pointer g;
EXTERN integer lq, lr;
EXTERN eight_bits dvi_buf[dvi_buf_size + 4];
EXTERN dvi_index half_buf;
EXTERN dvi_index dvi_limit;
EXTERN dvi_index dvi_ptr;
EXTERN integer dvi_offset;
EXTERN integer pdf_offset;
EXTERN integer dvi_gone;
EXTERN pointer down_ptr, right_ptr;
EXTERN scaled dvi_h, dvi_v;
EXTERN scaled pdf_h, pdf_v;
EXTERN scaled pdf_x, pdf_y;
EXTERN scaled pdf_delta_h, pdf_delta_v;
EXTERN scaled cur_h, cur_v;
EXTERN internal_font_number dvi_f;
EXTERN internal_font_number pdf_f;
EXTERN integer cur_s;
EXTERN scaled total_stretch[4], total_shrink[4];
EXTERN integer last_badness;
EXTERN pointer adjust_tail;
EXTERN integer pack_begin_line;
EXTERN two_halves empty_field;
EXTERN four_quarters null_delimiter;
EXTERN pointer cur_mlist;
EXTERN small_number cur_style;
EXTERN small_number cur_size;
EXTERN scaled cur_mu;
EXTERN boolean mlist_penalties;
EXTERN internal_font_number cur_f;
EXTERN quarterword cur_c;
EXTERN four_quarters cur_i;
EXTERN integer magic_offset;
EXTERN pointer cur_align;
EXTERN pointer cur_span;
EXTERN pointer cur_loop;
EXTERN pointer align_ptr;
EXTERN pointer cur_head, cur_tail;
EXTERN pointer just_box;
EXTERN pointer passive;
EXTERN pointer printed_node;
EXTERN halfword pass_number;
EXTERN scaled active_width[8];
EXTERN scaled cur_active_width[8];
EXTERN scaled background[8];
EXTERN scaled break_width[8];
EXTERN boolean no_shrink_error_yet;
EXTERN pointer cur_p;
EXTERN boolean second_pass;
EXTERN boolean final_pass;
EXTERN integer threshold;
EXTERN integer minimal_demerits[4];
EXTERN integer minimum_demerits;
EXTERN pointer best_place[4];
EXTERN halfword best_pl_line[4];
EXTERN scaled disc_width;
EXTERN halfword easy_line;
EXTERN halfword last_special_line;
EXTERN scaled first_width;
EXTERN scaled second_width;
EXTERN scaled first_indent;
EXTERN scaled second_indent;
EXTERN pointer best_bet;
EXTERN integer fewest_demerits;
EXTERN halfword best_line;
EXTERN integer actual_looseness;
EXTERN integer line_diff;
EXTERN int hc[66];
EXTERN int hn;
EXTERN halfword ha, hb;
EXTERN int hf;
EXTERN int hu[66];
EXTERN int hyf_char;
EXTERN int cur_lang, init_cur_lang;
EXTERN integer l_hyf, r_hyf;
EXTERN integer init_l_hyf, init_r_hyf;
EXTERN halfword hyf_bchar;
EXTERN char hyf[68];
EXTERN pointer init_list;
EXTERN boolean init_lig;
EXTERN boolean init_lft;
EXTERN int hyphen_passed;
EXTERN halfword cur_l, cur_r;
EXTERN pointer cur_q;
EXTERN pointer lig_stack;
EXTERN boolean ligature_present;
EXTERN boolean lft_hit, rt_hit;

#ifdef ALLOCATETRIES
  EXTERN halfword * trie_trl;
  EXTERN halfword * trie_tro;
  EXTERN quarterword * trie_trc;
#else
  EXTERN halfword trie_trl[trie_size + 1];
  EXTERN halfword trie_tro[trie_size + 1];
  EXTERN quarterword trie_trc[trie_size + 1];
#endif

EXTERN small_number hyf_distance[trie_op_size + 1];
EXTERN small_number hyf_num[trie_op_size + 1];
EXTERN trie_op_code hyf_next[trie_op_size + 1];
EXTERN integer op_start[256];

#ifdef ALLOCATEHYPHEN
  #define default_hyphen_prime 1009
  EXTERN str_number * hyph_word;
  EXTERN pointer * hyph_list;
  EXTERN integer hyphen_prime;
#else
  #define hyphen_prime 607
  EXTERN str_number hyph_word[hyphen_prime + 1];
  EXTERN pointer hyph_list[hyphen_prime + 1];
#endif

EXTERN hyph_pointer hyph_count;

#ifdef INITEX
  EXTERN integer trie_op_hash_C[trie_op_size - neg_trie_op_size + 1];
  #define trie_op_hash (trie_op_hash_C - (int)(neg_trie_op_size)) 
  EXTERN trie_op_code trie_used[256];
  EXTERN ASCII_code trie_op_lang[trie_op_size + 1];
  EXTERN trie_op_code trie_op_val[trie_op_size + 1];
  EXTERN integer trie_op_ptr;
#endif

EXTERN trie_op_code max_op_used;

#ifdef INITEX
  #ifdef ALLOCATEINI
    EXTERN packed_ASCII_code * trie_c; /* characters to match */
    EXTERN trie_op_code * trie_o;      /* operations to perform */
    EXTERN trie_pointer * trie_l;      /* left subtrie links */
    EXTERN trie_pointer * trie_r;      /* right subtrie links */
    EXTERN trie_pointer * trie_hash;   /* used to identify equivlent subtries */
  #else
    EXTERN packed_ASCII_code trie_c[trie_size + 1];
    EXTERN trie_op_code trie_o[trie_size + 1];
    EXTERN trie_pointer trie_l[trie_size + 1];
    EXTERN trie_pointer trie_r[trie_size + 1];
    EXTERN trie_pointer trie_hash[trie_size + 1];
  #endif

  EXTERN trie_pointer trie_ptr;
#endif

#ifdef INITEX
  #ifdef ALLOCATEINI
    EXTERN char * trie_taken;
  #else
    EXTERN boolean trie_taken[trie_size + 1];
  #endif

  EXTERN trie_pointer trie_min[256];
  EXTERN trie_pointer trie_max;
  EXTERN boolean trie_not_ready;
#endif

EXTERN scaled best_height_plus_depth;
EXTERN pointer page_tail;
EXTERN int page_contents;

#if (half_error_line < 30) || (half_error_line > error_line - 15)
  #error ERROR: (half_error_line < 30) || (half_error_line > error_line - 15) BAD 1
#endif

#if (max_print_line < 60)
  #error ERROR: (max_print_line < 60) BAD 2
#endif

#if (hash_prime > hash_size)
  #error ERROR: (hash_prime > hash_size) BAD 5
#endif

#if (max_in_open > 127)
  #error ERROR: (max_in_open > 127) BAD 6
#endif

#if (min_quarterword > 0) || (max_quarterword < 127)
  #error ERROR: (min_quarterword > 0) || (max_quarterword < 127) BAD 11
#endif

#if (min_halfword > 0) || (max_halfword < 32767)
  #error ERROR:  (min_halfword > 0) || (max_halfword < 32767) BAD 12
#endif

#if (min_quarterword < min_halfword) || (max_quarterword > max_halfword)
  #error ERROR: (min_quarterword < min_halfword) || (max_quarterword > max_halfword) BAD 13
#endif

#if (font_max > max_quarterword)
  #error ERROR: (font_max > max_quarterword) BAD 15
#endif

#if (save_size > max_halfword)
  #error ERROR: (save_size > max_halfword) BAD 17
#endif

#if (buf_size > max_halfword)
  #error ERROR:  (buf_size > max_halfword) BAD 18
#endif

#if (max_quarterword - min_quarterword) < 255
  #error (max_quarterword - min_quarterword) < 255 BAD 19
#endif

EXTERN scaled page_max_depth;
EXTERN pointer best_page_break;
EXTERN integer least_page_cost;
EXTERN scaled best_size;
EXTERN scaled page_so_far[8];
EXTERN pointer last_glue;
EXTERN integer last_penalty;
EXTERN scaled last_kern;
EXTERN integer insert_penalties;
EXTERN boolean output_active;
/* sec 1032 */
EXTERN internal_font_number main_f;
EXTERN four_quarters main_i;
EXTERN four_quarters main_j;
EXTERN font_index main_k;
EXTERN pointer main_p;
EXTERN integer main_s;
EXTERN halfword bchar;
EXTERN halfword false_bchar;
EXTERN boolean cancel_boundary;
EXTERN boolean ins_disc;
/* sec 1074 */
EXTERN pointer cur_box;
EXTERN halfword after_token;
EXTERN boolean long_help_seen;
EXTERN str_number format_ident;
EXTERN word_file fmt_file;
EXTERN gzFile gz_fmt_file;
/* sec 1331 */
EXTERN integer ready_already;
/* sec 1342 */
EXTERN alpha_file write_file[16];
EXTERN boolean write_open[18];
/* sec 1345 */
EXTERN pointer write_loc;
EXTERN pool_pointer edit_name_start;
EXTERN integer edit_name_length, edit_line;
EXTERN int fbyte;
/* new variables defined in local.c */
EXTERN boolean is_initex;
EXTERN boolean verbose_flag;
EXTERN boolean trace_flag;
EXTERN boolean open_trace_flag;
EXTERN boolean knuth_flag;
EXTERN boolean c_style_flag;
EXTERN boolean non_ascii;
EXTERN boolean key_replace;
EXTERN boolean deslash;
EXTERN boolean trimeof;
EXTERN boolean allow_patterns;
EXTERN boolean show_fonts_used;
EXTERN boolean reset_exceptions;
EXTERN boolean show_current;
EXTERN boolean return_flag;
EXTERN boolean civilize_flag;
EXTERN boolean show_numeric;
EXTERN boolean restrict_to_ascii;
EXTERN boolean show_missing;
EXTERN boolean full_file_name_flag;
EXTERN int mem_initex;
EXTERN int mem_extra_high;
EXTERN int mem_extra_low;
EXTERN int new_hyphen_prime;
EXTERN int missing_characters;
EXTERN boolean show_in_hex;
EXTERN boolean show_in_dos;
EXTERN boolean show_fmt_flag;
EXTERN boolean show_tfm_flag;
EXTERN boolean truncate_long_lines;
EXTERN boolean show_cs_names;
EXTERN int tab_step;
EXTERN int pseudo_tilde;
EXTERN int pseudo_space;
EXTERN boolean allow_quoted_names;
EXTERN int default_rule;
EXTERN char * format_file;
EXTERN char * source_direct;
EXTERN char * format_name;
EXTERN boolean show_line_break_stats;
EXTERN int first_pass_count;
EXTERN int second_pass_count;
EXTERN int final_pass_count;
EXTERN int underfull_hbox;
EXTERN int overfull_hbox;
EXTERN int underfull_vbox;
EXTERN int overfull_vbox;
EXTERN int paragraph_failed;
EXTERN int single_line;
EXTERN FILE * errout;
EXTERN int ignore_frozen;
EXTERN boolean suppress_f_ligs;
EXTERN int jump_used;
EXTERN jmp_buf jumpbuffer;
extern int current_pool_size;
extern int current_max_strings;
extern int current_mem_size;
extern int current_font_mem_size;
extern int current_save_size;
extern int current_stack_size;
extern int current_nest_size;
extern int current_param_size;
extern int current_buf_size;
extern const char * banner;
extern const char * application;
extern const char * yandyversion;
extern unsigned char wintodos[128];
extern char log_line[256];
extern char * dvi_directory;
extern char * log_directory;
extern char * aux_directory;
extern char * fmt_directory;
extern char * pdf_directory;
extern clock_t start_time, main_time, finish_time;

extern memory_word * allocate_main_memory (int size);
extern memory_word * realloc_main (int lo_size, int hi_size);
extern packed_ASCII_code * realloc_str_pool (int size);
extern pool_pointer * realloc_str_start (int size);
extern memory_word * realloc_save_stack (int size);
extern list_state_record * realloc_nest_stack (int size);
extern in_state_record * realloc_input_stack (int size);
extern halfword * realloc_param_stack (int size);
extern ASCII_code * realloc_buffer (int size);
extern memory_word * realloc_font_info (int size);
extern int realloc_hyphen (int hyphen_prime);
extern int allocate_tries (int trie_max);
extern void probe_memory (void);
extern void print_cs_names (FILE * output, int pass);
extern void perrormod (const char * s);
extern char * grabenv (const char * varname);
extern void flush_trailing_slash (char * directory);
extern boolean prime (int x);
extern int endit (int flag);
extern void uexit (int unix_code);
extern void t_open_in (void);
extern void call_edit (ASCII_code * filename, pool_pointer fnstart,
  integer fnlength, integer linenumber);
extern void add_variable_space (int size);
extern char * unixify (char * t);

#include "coerce.h"

/* sec 79 */
extern void node_list_display (integer p);
extern void do_nothing (void);
extern void update_terminal (void);
extern void check_full_save_stack (void);
extern void push_input (void);
extern void pop_input (void);
extern void print_err (const char * s);
extern void ensure_dvi_open (void);
extern void write_dvi (size_t a, size_t b);
extern void prompt_input (const char * s);
extern void synch_h (void);
extern void synch_v (void);
extern void set_cur_lang (void);
extern char * md5_file (FILE * in_file);
extern void str_room (int val);
extern void tail_append_ (pointer val);
#define tail_append(a) tail_append_((pointer) a)
extern void tex_help (unsigned int n, ...);
extern void append_char (ASCII_code c);
extern void append_lc_hex (ASCII_code c);
extern void succumb (void);
extern void dvi_out_ (ASCII_code op);
#define dvi_out(op) dvi_out_((ASCII_code) (op))
extern void free_avail_ (halfword p);
#define free_avail(p) free_avail_((halfword) (p))
extern void flush_string (void);
extern str_number load_pool_strings (integer spare_size);
extern str_number make_string_pool (const char * s);
extern void print_plus (int i, const char * s);
#define help0()     tex_help(0)
#define help1(...)  tex_help(1, __VA_ARGS__)
#define help2(...)  tex_help(2, __VA_ARGS__)
#define help3(...)  tex_help(3, __VA_ARGS__)
#define help4(...)  tex_help(4, __VA_ARGS__)
#define help5(...)  tex_help(5, __VA_ARGS__)
#define help6(...)  tex_help(6, __VA_ARGS__)
extern char * md5_file_name(const char * file_name);
extern void fget (void);
extern str_number get_job_name (str_number job);
extern void show_font_info (void);

EXTERN int shipout_flag;
#endif
