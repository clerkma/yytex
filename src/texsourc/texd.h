/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#define ALLOCATEINI        /* allocate iniTeX (550 k) trie_c, trie_o, trie_l, trie_r, trie_hash, trie_taken */
#define ALLOCATEMAIN       /* allocate main memory for TeX (2 Meg) zmem = zzzaa */
#define ALLOCATEFONT       /* allocate font_info (800 k) (dynamically now) */
#define ALLOCATETRIES      /* allocate hyphenation trie stuff (270 k) trie_trl, trie_tro, trie_trc */
#define ALLOCATEHYPHEN     /* allocate hyphenation exception tables */
#define VARIABLETRIESIZE   /* allow trie_size to be variable */
#define ALLOCATESTRING     /* allocate strings and string pointers (184 k)str_pool & str_start */
#define ALLOCATESAVESTACK  /* experiment to dynamically deal with save_stack 99/Jan/4 */
#define ALLOCATEINPUTSTACK /* experiment to dynamically deal with input_stack 99/Jan/21 */
#define ALLOCATENESTSTACK  /* experiment to dynamically deal with nest_stack 99/Jan/21 */
#define ALLOCATEPARAMSTACK /* experiment to dynamically deal with param_stack 99/Jan/21 */
#define ALLOCATEBUFFER     /* experiment to dynamically deal with input buffer 99/Jan/22 */
#define INCREASEFIXED
/* increase number of fonts - quarterword 16 bit - max_quarterword limit */
/* there may still be some bugs with this however ... also may slow down */
/* also: should split use of quarterword for (i) font from (ii) char */
/* for example, xeq_level ? hyphenation trie_trc ? */
#define INCREASEFONTS
#define SHORTFONTINFO
#define INCREASETRIEOP
#define COMPACTFORMAT

#define STAT
#include "yandytex.h"

enum {
  out_dvi_flag = (1 << 0),
  out_pdf_flag = (1 << 1),
  out_xdv_flag = (1 << 2),
  out_dpx_flag = (1 << 3),
};

#define INLINE inline

#define dump_file  fmt_file
#define out_file   dvi_file

/* Read a line of input as quickly as possible.  */
extern boolean input_line (FILE *);
#define input_ln(stream, flag) input_line(stream)

/* `b_open_in' (and out) is used only for reading (and writing) .tfm
   files; `w_open_in' (and out) only for dump files.  The filenames are
   passed in as a global variable, `name_of_file'.  */
   
#define b_open_in(f)  open_input  (&(f), TFMFILEPATH, FOPEN_RBIN_MODE)
#define w_open_in(f)  open_input  (&(f), TEXFORMATPATH, FOPEN_RBIN_MODE)
#define b_open_out(f) open_output (&(f), FOPEN_WBIN_MODE)
#define w_open_out    b_open_out
#define b_close       a_close
#define w_close       a_close
#define gz_w_close    gzclose

/* sec 0241 */
#define fix_date_and_time() get_date_and_time (&(tex_time), &(day), &(month), &(year))

/* If we're running under Unix, use system calls instead of standard I/O
   to read and write the output files; also, be able to make a core dump. */ 
#ifndef unix
  #define dumpcore() exit(1)
#else /* unix */
  #define dumpcore abort
#endif

#define write_dvi(a, b)                                           \
  if ((size_t) fwrite ((char *) &dvi_buf[a], sizeof (dvi_buf[a]), \
         (size_t) ((size_t)(b) - (size_t)(a) + 1), dvi_file)      \
         != (size_t) ((size_t)(b) - (size_t)(a) + 1))             \
     FATAL_PERROR ("\n! dvi file")

extern int do_dump (char *, int, int, FILE *);
extern int do_undump (char *, int, int, FILE *);

/* Reading and writing the dump files.  `(un)dumpthings' is called from
   the change file.*/
#define dumpthings(base, len)           \
  do_dump ((char *) &(base), sizeof (base), (int) (len), dump_file)

#define undumpthings(base, len)           \
  do_undump ((char *) &(base), sizeof (base), (int) (len), dump_file)

/* Use the above for all the other dumping and undumping.  */
#define generic_dump(x)   dumpthings (x, 1)
#define generic_undump(x) undumpthings (x, 1)

#define dump_wd     generic_dump
#define undump_wd   generic_undump
#define dump_hh     generic_dump
#define undump_hh   generic_undump
#define dump_qqqq   generic_dump
#define undump_qqqq generic_undump

/* `dump_int' is called with constant integers, so we put them into a
   variable first.  */
#define dump_int(x)         \
  do                        \
    {                       \
      integer x_val = (x);  \
      generic_dump (x_val); \
    }                       \
  while (0)

/* web2c/regfix puts variables in the format file loading into
   registers.  Some compilers aren't willing to take addresses of such
   variables.  So we must kludge.  */
#ifdef REGFIX
#define undump_int(x)         \
  do                          \
    {                         \
      integer x_val;          \
      generic_undump (x_val); \
      x = x_val;              \
    }                         \
  while (0)
#else
#define undump_int  generic_undump
#endif


/* If we're running on an ASCII system, there is no need to use the
   `xchr' array to convert characters to the external encoding.  */

#define Xchr(x) xchr[x]

/* following added from new texmf.c file 1996/Jan/12 */
/* these, of course are useless definitions since parameters not given */

/* Declare routines in texmf.c.  */
extern void t_open_in();
#include "yandy_macros.h"

// #define max_halfword 65535L  /* for 32 bit memory word */
#define min_halfword -2147483647L /* for 64 bit memory word (signed) */
#define max_halfword  2147483647L /* for 64 bit memory word (signed) */

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
  #define max_mem_size (max_halfword / sizeof(memory_word) - 1)
#else
  #define mem_top 262140L
#endif

#ifdef ALLOCATEMAIN
  EXTERN integer mem_max;
#else
  #define mem_max mem_top
#endif

#ifdef ALLOCATEMAIN
  EXTERN integer mem_min;
#else
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
  #define font_max 1023
#else
  #define font_max 255
#endif

#ifdef ALLOCATEFONT
  #define font_mem_size (max_halfword / sizeof(memory_word) - 1)
#else
  #define font_mem_size 100000L
#endif

#ifdef ALLOCATEFONT
  #define initial_font_mem_size   20000
  #define increment_font_mem_size 40000
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

#ifdef ALLOCATEDVIBUF
  #define default_dvi_buf_size 16384
  EXTERN int dvi_buf_size;
#else
  #define dvi_buf_size 16384
#endif

#define hash_size 32768 // 9500 25000
#define hash_extra (255 - font_max)
#define hash_prime 27197

#if (hash_extra != 255 - font_max)
  #error ERROR: hash_extra not equal to (255 - font_max)
#endif

/* sec 0113 */
#ifdef INCREASEFONTS
  typedef unsigned short quarterword;
#else
  typedef unsigned char quarterword;
#endif
/* possible alternative ? */
/* min_halfword = 0 and double max_halfword ? */
/* typedef unsigned long halfword; NO NO: since mem_min may be < 0 */
/* sec 0113 */
typedef long halfword;
typedef halfword pointer;
typedef char two_choices;
typedef char four_choices;
/* sec 0113 */
/*
  meaning      structure                      TeX                 Y&Y TeX
               ----------------------------------------------------------------------
  integer      |            int            || 4: long           | 8: long long      |   min_quarterword 0
               ---------------------------------------------------------------------- max_quarterword FFFF
  scaled       |            sc             || 4: long           | 8: long long      |   min_halfword
               ----------------------------------------------------------------------
  glue_ratio   |            gr             || 4: float          | 8: double         |
               ----------------------------------------------------------------------
  halfword     |     lh      |     rh      || 2: unsigned short | 4: unsigned long  |
               ----------------------------------------------------------------------
  half+quarter |  b0  |  b1  |     rh      ||                                       |
               ----------------------------------------------------------------------
  quarter      |  b0  |  b1  |  b2  |  b3  || 1: unsigned char  | 2: unsigned short |
               ----------------------------------------------------------------------
*/

typedef struct
{
#ifdef WORDS_BIGENDIAN
  halfword rh;

  union
  {
    halfword lh;

    struct
    {
      quarterword b0, b1;
    };
  };
#endif
} two_halves;

typedef struct
{
#ifdef WORDS_BIGENDIAN
  quarterword b0, b1, b2, b3;
#else
  quarterword b3, b2, b1, b0;
#endif
} four_quarters;

typedef union
{
  glue_ratio gr;
  two_halves hh;
  integer cint;
  four_quarters qqqq;
} memory_word;

#ifndef WORDS_BIGENDIAN
  #define cint u.CINT
  #define qqqq v.QQQQ
#endif
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
EXTERN unsigned char name_of_file[PATH_MAX + 4];
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
  EXTERN memory_word 
  #define zmem (zzzaa - (int)(mem_bot))
  zzzaa[mem_max - mem_bot + 1];
#endif

EXTERN pointer lo_mem_max;
EXTERN pointer hi_mem_min;
EXTERN integer var_used, dyn_used;
/* sec 0118 */
EXTERN pointer avail;
EXTERN pointer mem_end;
EXTERN halfword mem_start; // for yandytex
/* sec 0124 */
EXTERN halfword rover;
/* sec 0165 */
/* NOTE: the following really also need to be dynamically allocated */
#ifdef DEBUG
  #ifdef ALLOCATEMAIN
    EXTERN char * zzzab;
  #else

  EXTERN char
/* #define freearr (zzzab - (int)(mem_min)) */
/*  zzzab[mem_max - mem_min + 1];  */
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

EXTERN halfword was_mem_end, was_lo_max, was_hi_min;
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

/* Probably require eqtb_extra to be zero, so hash_extra = 255 - font_max */
#if (eqtb_extra != 0)
  #error ERROR: eqtb_extra is not zero (need hash_extra equal 255 - font_max)
#endif

#ifdef ALLOCATEZEQTB
  EXTERN memory_word * eqtb;
#else
  #ifdef INCREASEFONTS
    EXTERN memory_word eqtb[eqtb_size + 1];
  #else
    EXTERN memory_word eqtb[eqtb_size + 1];
  #endif
#endif

#ifdef INCREASEFONTS
  #define xeq_level (zzzad - (int_base + eqtb_extra))
#else
  #define xeq_level (zzzad - (int_base))
#endif

EXTERN quarterword zzzad[844];
/* region 5 & 6 int_base to eqtb_size: 13507 - 12663 */

#ifdef ALLOCATEHASH
  #ifdef SHORTHASH
    EXTERN htwo_halves *zzzae;
  #else
    EXTERN two_halves *zzzae;
  #endif

  #define hash (zzzae - 514)
#else
  #ifdef SHORTHASH
    EXTERN htwo_halves 
  #else
    EXTERN two_halves 
  #endif

  #define hash (zzzae - 514)

  #ifdef INCREASEFONTS
    zzzae[hash_size + 267 + eqtb_extra];
  #else
    zzzae[hash_size + 267];
  #endif
#endif

EXTERN halfword hash_used;
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
EXTERN int cur_chr;
EXTERN halfword cur_cs;
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
  EXTERN halfword * param_stack;
#else
  #define param_size 500
EXTERN halfword param_stack[param_size + 1];
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
EXTERN halfword warning_index;
EXTERN halfword def_ref;
EXTERN integer align_state;
EXTERN integer base_ptr;
EXTERN halfword par_loc;
EXTERN halfword par_token;
EXTERN boolean force_eof;
EXTERN halfword cur_mark[6];
EXTERN int long_state;
EXTERN halfword pstack[10];
EXTERN int cur_val;
EXTERN int cur_val_level;
EXTERN int radix;
EXTERN int cur_order;
EXTERN alpha_file read_file[16];  /* hard wired limit in TeX */
EXTERN char read_open[20];
EXTERN halfword cond_ptr;
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
EXTERN str_number output_file_name;
EXTERN str_number texmf_log_name;
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

EXTERN eight_bits font_dir[font_max + 1];
EXTERN integer font_num_ext[font_max + 1];
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
EXTERN halfword font_glue[font_max + 1];
EXTERN boolean font_used[font_max + 1];
EXTERN integer hyphen_char[font_max + 1];
EXTERN integer skew_char[font_max + 1];
EXTERN font_index bchar_label[font_max + 1];
EXTERN short font_bchar[font_max + 1];
EXTERN short font_false_bchar[font_max + 1];
EXTERN integer char_base[font_max + 1];
EXTERN integer ctype_base[font_max + 1];
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
EXTERN int c, f;
EXTERN scaled rule_ht, rule_dp, rule_wd;
EXTERN halfword g;
EXTERN integer lq, lr;
EXTERN eight_bits dvi_buf[dvi_buf_size + 4];
EXTERN dvi_index half_buf;
EXTERN dvi_index dvi_limit;
EXTERN dvi_index dvi_ptr;
EXTERN integer dvi_offset;
EXTERN integer pdf_offset;
EXTERN integer dvi_gone;
EXTERN halfword down_ptr, right_ptr;
EXTERN scaled dvi_h, dvi_v;
EXTERN scaled pdf_h, pdf_v;
EXTERN scaled pdf_x, pdf_y;
EXTERN scaled pdf_delta_h, pdf_delta_v;
EXTERN scaled cur_h, cur_v;
EXTERN internal_font_number dvi_f;
EXTERN internal_font_number pdf_f;
EXTERN integer cur_s; /* sec 616 */
EXTERN scaled total_stretch[4], total_shrink[4];
EXTERN integer last_badness;
EXTERN halfword adjust_tail;
EXTERN integer pack_begin_line;
EXTERN two_halves empty_field;
EXTERN four_quarters null_delimiter;
EXTERN halfword cur_mlist;
EXTERN int cur_style;
EXTERN int cur_size;
EXTERN scaled cur_mu;
EXTERN boolean mlist_penalties;
EXTERN internal_font_number cur_f;
EXTERN int cur_c;
EXTERN four_quarters cur_i;
EXTERN integer magic_offset;
EXTERN halfword cur_align;
EXTERN halfword cur_span;
EXTERN halfword cur_loop;
EXTERN halfword align_ptr;
EXTERN halfword cur_head, cur_tail;
EXTERN halfword just_box;
EXTERN halfword passive;
EXTERN halfword printed_node;
EXTERN halfword pass_number;
EXTERN scaled active_width[8];
EXTERN scaled cur_active_width[8];
EXTERN scaled background[8];
EXTERN scaled break_width[8];
EXTERN boolean no_shrink_error_yet;
EXTERN halfword cur_p;
EXTERN boolean second_pass;
EXTERN boolean final_pass;
EXTERN integer threshold;
EXTERN integer minimal_demerits[4];
EXTERN integer minimum_demerits;
EXTERN halfword best_place[4];
EXTERN halfword best_pl_line[4];
EXTERN scaled disc_width;
EXTERN halfword easyline;
EXTERN halfword last_special_line;
EXTERN scaled first_width;
EXTERN scaled second_width;
EXTERN scaled first_indent;
EXTERN scaled second_indent;
EXTERN halfword best_bet;
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
EXTERN integer lhyf, rhyf;
EXTERN integer init_l_hyf, init_r_hyf;
EXTERN halfword hyfbchar;
EXTERN char hyf[68];
EXTERN halfword init_list;
EXTERN boolean init_lig;
EXTERN boolean init_lft;
EXTERN int hyphen_passed;
EXTERN int cur_l, cur_r;
EXTERN halfword cur_q;
EXTERN halfword lig_stack;
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

/* if ALLOCATEHYPHEN is true, then hyphen_prime is a variable */
/* otherwise it is a pre-processor defined constant */
#ifdef ALLOCATEHYPHEN
  #define default_hyphen_prime 1009
  EXTERN str_number * hyph_word;
  EXTERN halfword * hyph_list;
  EXTERN integer hyphen_prime;
#else
  #define hyphen_prime 607
  EXTERN str_number hyph_word[hyphen_prime + 1];
  EXTERN halfword hyph_list[hyphen_prime + 1];
#endif

EXTERN int hyph_count;

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
    EXTERN packed_ASCII_code *trie_c; /* characters to match */
    EXTERN trie_op_code *trie_o;      /* operations to perform */
    EXTERN trie_pointer *trie_l;      /* left subtrie links */
    EXTERN trie_pointer *trie_r;      /* right subtrie links */
    EXTERN trie_pointer *trie_hash;   /* used to identify equivlent subtries */
  #else /* end ALLOCATEINI */
    EXTERN packed_ASCII_code trie_c[trie_size + 1];
    EXTERN trie_op_code trie_o[trie_size + 1];
    EXTERN trie_pointer trie_l[trie_size + 1];
    EXTERN trie_pointer trie_r[trie_size + 1];
    EXTERN trie_pointer trie_hash[trie_size + 1];
  #endif /* end not ALLOCATEINI */
  EXTERN trie_pointer trie_ptr;
#endif /* INITEX */

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
EXTERN halfword page_tail;
EXTERN int page_contents;

/* (cannot catch everything here, since some is now dynamic) */

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
EXTERN halfword best_page_break;
EXTERN integer least_page_cost;
EXTERN scaled best_size;
EXTERN scaled page_so_far[8];
EXTERN halfword last_glue;
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
EXTERN int tfm_temp;

/* new stuff defined in local.c - bkph */
EXTERN boolean is_initex;
EXTERN boolean verbose_flag;
EXTERN boolean trace_flag;
EXTERN boolean debug_flag;
EXTERN boolean open_trace_flag;
EXTERN boolean knuth_flag;
EXTERN boolean no_interrupts;
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
EXTERN boolean want_version;
EXTERN boolean civilize_flag;
EXTERN boolean show_numeric;
EXTERN boolean restrict_to_ascii;
EXTERN boolean show_missing;
EXTERN boolean full_file_name_flag;
EXTERN boolean save_strings_flag;
EXTERN int mem_initex;
EXTERN int mem_extra_high;
EXTERN int mem_extra_low;
EXTERN int new_hyphen_prime;
EXTERN int missing_characters;
EXTERN int show_in_hex;
EXTERN int show_in_dos;
EXTERN int show_fmt_flag;
EXTERN int show_tfm_flag;
EXTERN boolean show_texinput_flag;
EXTERN boolean truncate_long_lines;
EXTERN boolean show_cs_names;
EXTERN int tab_step;
EXTERN int pseudo_tilde;
EXTERN int pseudo_space;
EXTERN int allow_quoted_names;
EXTERN int default_rule;
EXTERN char * format_file;
EXTERN char * source_direct;
EXTERN char * format_name;
EXTERN char * encoding_name;
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
EXTERN int font_dimen_zero;
EXTERN int ignore_frozen;
EXTERN boolean suppress_f_ligs;
EXTERN int abort_flag;
EXTERN int err_level;
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
extern char *tex_version;
extern char *application;
extern char *yandyversion;
extern unsigned char wintodos[128];
extern char log_line[MAXLINE];
extern char *texpath;

memory_word * allocate_main_memory (int);
memory_word * realloc_main (int, int);
packed_ASCII_code * realloc_str_pool (int);
pool_pointer * realloc_str_start (int);
memory_word * realloc_save_stack (int);
list_state_record * realloc_nest_stack (int);
in_state_record * realloc_input_stack (int);
halfword * realloc_param_stack (int);
ASCII_code * realloc_buffer (int);
memory_word * realloc_font_info (int);

int realloc_hyphen (int);
int allocate_tries (int);
void check_eqtb (char *);
void probe_memory (void);
void print_cs_names (FILE *, int);
void perrormod(char *);
char *grabenv(char *);
void stamp_it (char *);
void stampcopy (char *);
boolean prime (int);
int endit (int);

void uexit (int unix_code);
void t_open_in (void);


void call_edit (ASCII_code *filename, pool_pointer fnstart,
                integer fnlength, integer linenumber);

void add_variable_space(int);

void get_date_and_time (integer *, integer *, integer *, integer *);

//void get_date_and_time (integer *minutes, integer *day,
//                        integer *month, integer *year);

char *unixify (char *);

//#include "yandy_macros.h"
#include "coerce.h"

/* sec 79 */
extern INLINE void prompt_input(const char *s);
extern INLINE void synch_h(void);
extern INLINE void synch_v(void);
extern INLINE void set_cur_lang(void);
extern char * md5_file(FILE * in_file);
extern INLINE void str_room_ (int val);
#define str_room(a) str_room_((int) a)
extern INLINE void tail_append_ (pointer val);
#define tail_append(a) tail_append_((pointer) a)
extern INLINE void tex_help (unsigned int n, ...);
extern INLINE void append_char(ASCII_code c);
extern INLINE void append_lc_hex(ASCII_code c);
extern INLINE void succumb(void);
extern INLINE void dvi_out_ (ASCII_code op);
#define dvi_out(op) dvi_out_((ASCII_code) (op))
extern INLINE void free_avail_(halfword p);
#define free_avail(p) free_avail_((halfword) (p))
extern INLINE void flush_string (void);
extern str_number load_pool_strings (integer spare_size);
extern str_number make_string_pool (const char *s);
#define help0()     tex_help(0)
#define help1(...)  tex_help(1, __VA_ARGS__)
#define help2(...)  tex_help(2, __VA_ARGS__)
#define help3(...)  tex_help(3, __VA_ARGS__)
#define help4(...)  tex_help(4, __VA_ARGS__)
#define help5(...)  tex_help(5, __VA_ARGS__)
#define help6(...)  tex_help(6, __VA_ARGS__)

/********BINDING WITH LIBHARU*********/
typedef struct _mapping_table mapping_table;
typedef struct _mapping_entry mapping_entry;
EXTERN HPDF_Doc  yandy_pdf;
EXTERN HPDF_Page yandy_page;
EXTERN HPDF_Font yandy_font[1024];
EXTERN boolean pdf_doing_string;
EXTERN boolean pdf_doing_text;
EXTERN integer scaled_out;
EXTERN boolean shipout_flag;
EXTERN mapping_table * gentbl;
EXTERN mapping_table * font_name_hash_init (void);
EXTERN void font_name_hash_free (mapping_table * tbl);
EXTERN void pdf_ship_out(pointer p);
EXTERN void pdf_vlist_out (void);
EXTERN void pdf_hlist_out (void);
EXTERN void pdf_begin_text(void);
EXTERN void pdf_font_def(internal_font_number f);
EXTERN void pdf_error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void * user_data);
/********BINDING WITH LIBHARU*********/