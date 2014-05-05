//#include "hpdf.h"
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef MSDOS
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
  /* NOT *//* allow increasing high area of main memory */ /* FLUSH */
  #undef ALLOCATEHIGH
  /* NOT *//* allow increasing low area of main memory */ /* FLUSH */
  #undef ALLOCATELOW
  /* NOT */ /* allocate hash table (zzzae) (78k) */
  #undef ALLOCATEHASH
  /* NOT */ /* allocate dvi_buf (16k) */ /* changed in 1.3 1996/Jan/18 */
  /* #define ALLOCATEDVIBUF */
  #undef ALLOCATEDVIBUF
  /* increase fixed allocations */
  #define INCREASEFIXED
  /* increase number of fonts - quarterword 16 bit - max_quarterword limit */
  /* there may still be some bugs with this however ... also may slow down */
  /* also: should split use of quarterword for (i) font from (ii) char */
  /* for example, xeq_level ? hyphenation trie_trc ? */
  #define INCREASEFONTS
  /* NOT NOT *//* allocate eqtb (108k) */ /* changed in 1.3 1996/Jan/18 */
  #undef ALLOCATEZEQTB
  /* make font_info array fmemoryword == 32 bit instead of memory_word = 64 bit */
  #define SHORTFONTINFO
  /* make hash table htwohalves == 32 bit instead of twohalves == 64 bit */
  // #define SHORTHASH  --- changed 2000/Feb/22 increase max_strings from 64K to 512M
  #undef SHORTHASH
  /* increase trie_op_size from 751 to 3001 96/Oct/12 */
  #define INCREASETRIEOP
#endif

/* With old PharLap linker it was important to avoid large fixed allocation */
/* Now may be better to use fixed arrays rather than allocate them */
/* hence ALLOCATEZQTB, ALLOCATEDVIBUF and ALLOCATEMINOR are NOT defined */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#undef  TRIP
#undef  TRAP
#define STAT
#undef  DEBUG
#include "texmf.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXLINE 256         // for log_line buffer

/* #define max_halfword 65535L  */  /* for 32 bit memory word */
/* #define max_halfword 262143L */  /* for 36 bit memory word */
#define min_halfword -2147483647L   /* for 64 bit memory word (signed) */
#define max_halfword  2147483647L   /* for 64 bit memory word (signed) */

#define block_size 1000 /* block_size for variable length node alloc */

/* min_quarterword assumed 0 -- i.e. we use unsigned types for quarterword */
#define min_quarterword 0
#ifdef INCREASEFONTS
  #define max_quarterword 65535L
#else
  #define max_quarterword 255
#endif

/* #define default_mem_top 262140L */ /* usual big TeX allocation 2 Meg bytes */
/* #define default_mem_top 131070L */ /* usual big TeX allocation 1 Meg bytes */
#define default_mem_top 65534L        /* usual small TeX allocation 0.5 Meg   */

/* mem_bot smallest index in mem array dumped by iniTeX mem_top >= mem_min */
#define mem_bot 0
/* mem_top largest index in mem array dumped by iniTeX mem_top <= mem_max */
#ifdef ALLOCATEMAIN
  EXTERN integer mem_top;
  #define max_mem_size (max_halfword / sizeof(memory_word) - 1)
#else
  #define mem_top 262140L
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* mem_max == mem_top in iniTeX, otherwise mem_max >= mem_top */
/* if ALLOCATEMAIN is true, then mem_max is a variable */
/* otherwise it is a pre-processor defined constant */
#ifdef ALLOCATEMAIN
  EXTERN integer mem_max;
#else
/* #define mem_max 262140L */
  #define mem_max mem_top
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* if ALLOCATEMAIN is true, then mem_min is a variable */
/* otherwise it is a pre-processor defined constant */
/* mem_min == mem_bot in iniTeX, otherwise mem_min <= mem_bot */
#ifdef ALLOCATEMAIN
  EXTERN integer mem_min;
#else
  #define mem_min 0
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* buf_size is max size of input line and max size of csname               */
/* make sure its multiple of four bytes long                               */
/* want to increase this so it can handle whole paragraph imported from WP */
#ifdef INCREASEFIXED
/* #define buf_size 8192 */
/* #define buf_size 12000 */    /* 1996/Jan/17 */
/* #define buf_size 16384 */    /* 1998/June/30 */
/* #define buf_size 20000 */    /* 1999/Jan/7 */
#else
/* #define buf_size 3000  */
#endif

#ifdef ALLOCATEBUFFER
  #define initial_buf_size   1000
  #define increment_buf_size 2000
  #define buf_size           2000000L /* arbitrary limit <= max_halfword */
  EXTERN ASCII_code *        buffer;
#else
  #define buf_size           20000 /* 1999/Jan/7 */
  EXTERN ASCII_code          buffer[buf_size + 4]; /* padded out to ...  + 4 */
#endif
EXTERN integer first; 
EXTERN integer last; 
EXTERN integer max_buf_stack; 

#define error_line      79
#define half_error_line 50
#define max_print_line  79

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef INCREASEFIXED
/* maximum number of simultaneous input sources (?) */
/* #define stack_size 600 */    /* expanded again 1999/Jan/21 */
/* #define stack_size 800 */
/* maximum number of input files and insertions that can be going on */
/* #define max_in_open 15 */    /* for Y&Y TeX 1.2 */
/* #define max_in_open 31 */    /* 1996/Jan/17 */
/* #define max_in_open 63 */    /* 1996/Jan/18 */
  #define max_in_open 127       /* 1996/Jan/20 - really ? */
/* save_size space for saving values outside of current group */
/* #define save_size 6000 */
/* #define save_size 8000 */    /* 1999/Jan/6 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* #define stack_size 300 */    /* Unix C version default */
  #define max_in_open 15
/* #define save_size 1000 */    /* 3.14159 C version */
/* #define save_size 4000 */    /* 3.14159 C version */
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* maximum internal font number - cannot be greated than max_quarter_word! */
#ifdef INCREASEFONTS
  #define font_max 1023     /* 1996/Jan/17 */
#else
  #define font_max 255
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* free the limit on font memory ! */ /* (2^32 - 1) / sizeof(memory_word) */
#ifdef ALLOCATEFONT
/* #define font_mem_size 262140L */
  #define font_mem_size (max_halfword / sizeof(memory_word) -1)
#else
  #define font_mem_size 100000L
#endif

/* our real font_mem_size is 268435456 --- ridiculously large, of course */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* below is new dynamic allocation - bkph 93/Nov/28 */  /* enough for lplain */
#ifdef ALLOCATEFONT
  #define initial_font_mem_size   20000
  #define increment_font_mem_size 40000
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* max_strings max number of strings */ /* (2^32 - 1) / sizeof (integer) */
#ifdef ALLOCATESTRING
/* #define max_strings 262140L */
// #define max_strings (max_halfword / sizeof(integer) -1)
  #define max_strings (max_halfword / sizeof(pool_pointer) - 1)
/* #define pool_size 4032000L */
  #define pool_size (max_halfword - 1)
/* #define stringmargin 32768L */
  #define stringmargin 10000
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* #define max_strings 15000 */
#define max_strings 16384
#define pool_size 124000L
#endif
#define string_vacancies 100000L

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* #if defined (ALLOCATEINITRIE) && defined (ALLOCATEHYPHEN) */
#ifdef VARIABLETRIESIZE
  EXTERN integer trie_size;
  #define default_trie_size 60000
#else
  #define trie_size 30000     /* 3.14159 C version */
#endif

/* increase trieop to accomadate more hyphenation patterns 96/OCt/12 */

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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* dvi_buf_size must be multiple of 8 - half is written out at a time */
#ifdef ALLOCATEDVIBUF
  #define default_dvi_buf_size 16384 
/* #define default_dvi_buf_size 32768 */    /* ? */
  EXTERN int dvi_buf_size;
#else
  #define dvi_buf_size 16384      /* 3.14159 C version */
/* #define dvi_buf_size 32768 */        /* ? */
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* WARNING: increasing hash table for cs names is not trivial */
/* size of hash table for control sequence name  < (mem_max - mem_min) / 10 */
/* #define hash_size 9500 */
/* #define hash_size 25000 */   /* 96/Jan/10 */
#define hash_size 32768       /* 96/Jan/17 */
/* trick to try and get around eqtb_extra problem */
/* 1024 fonts = font_max + 2 */
/* #define hash_extra -256 */
#define hash_extra (255 - font_max)
/* hash prime about 85% of hash_size (+ hash_extra) */
/* #define hashprime 7919  */
/* #define hash_prime 21247 */    /* 96/Jan/10 */
#define hash_prime 27197      /* 96/Jan/17 */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* CONSTRAINT: reconcile increase font use by stealing from hash table ... */

/* Probably require eqtb_extra to be zero, so hash_extra = 255 - font_max */
#if (hash_extra != 255 - font_max)
  #error ERROR: hash_extra not equal to (255 - font_max)
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* some sanity check when using narrow hash table ... */

/* using SHORTHASH limits hash_size to be less than 65536 */
/* using SHORTHASH limits max_strings to be less than 65536 */
/* if you ever need more string pointers, then #undef SHORTHASH --- */
/* you'll use more memory (about 130k byte) and format file larger (8k) */

#ifdef SHORTHASH
/* can only do this if INCREASEFONTS defined up above ... */
#if (max_quarterword < 65535L)
#error ERROR: max_quarterword < 65535L
#endif
/* with narrowed hash table can only have 65535 string pointers */
/* #if (max_strings > max_quarterword) */ /* this test does not work */
#undef max_strings
#define max_strings max_quarterword
/* #endif */
/* with narrowed hash table can only have 65535 hash table entries */
#if (hash_size > max_quarterword)
#undef hash_size
#define hash_size max_quarterword
#endif
#endif /* end of if SHORTHASH */

/* NOTE: if you define/undefine SHORTFONT have to redo formats */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef INCREASEFONTS
  typedef unsigned short quarterword;
#else
  typedef unsigned char quarterword;
#endif

/* possible alternative ? */ /* min_halfword = 0 and double max_halfword ? */
/* typedef unsigned long halfword; NO NO: since mem_min may be < 0 */
typedef integer halfword;
typedef char twochoices;
typedef char fourchoices;

#include "texmfmem.h"

typedef char glue_ord; 

typedef struct
{
/*  short mode_field;  */
  int mode_field;
  halfword head_field, tail_field;
  integer pg_field, ml_field;
  memory_word aux_field;
} list_state_record;

typedef char group_code;

typedef struct
{
  quarterword state_field, index_field; 
  halfword start_field, loc_field, limit_field, name_field;
} in_state_record; 

typedef integer internal_font_number; 
typedef integer font_index; 
typedef integer dvi_index; 
typedef integer trie_op_code; 
typedef integer trie_pointer; 

/* typedef short hyph_pointer; */   /* increased 1996/Oct/20 ??? */
typedef integer hyph_pointer; 

EXTERN integer bad;
EXTERN ASCII_code xord[256];
EXTERN ASCII_code xchr[256];
EXTERN unsigned char name_of_file[PATHMAX + 4]; // fix 2000 June 18
EXTERN integer name_length;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
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
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

EXTERN pool_pointer pool_ptr;
EXTERN str_number   str_ptr;
EXTERN pool_pointer init_pool_ptr;
EXTERN str_number   init_str_ptr;

#ifdef INITEX
  EXTERN alpha_file pool_file; 
#endif

EXTERN alpha_file log_file; 
/* EXTERN char selector; */
/* EXTERN integer selector; */ /* padded out */
EXTERN int selector;      /* padded out */ /* 95/Jan/7 */
/* EXTERN char dig[23]; */
EXTERN char dig[23 + 1];  /* padded out */
EXTERN integer tally; 
EXTERN integer term_offset; 
EXTERN integer file_offset; 
EXTERN ASCII_code trick_buf[error_line + 1]; /* already padded 79 + 1 */
EXTERN integer trick_count; 
EXTERN integer first_count; 
/* EXTERN char interaction;  */
/* EXTERN integer interaction; */ /* padded out */
EXTERN int interaction; /* padded out */      /* 95/Jan/7 */
EXTERN bool deletions_allowed; 
EXTERN bool set_box_allowed; 
/* EXTERN char history; */
/* EXTERN integer history; */ /* padded out */
EXTERN int history; /* padded out */        /* 95/Jan/7 */
/* EXTERN schar error_count;  */
/* EXTERN integer error_count; */ /* padded out */
EXTERN int error_count; /* padded out */      /* 95/Jan/7 */
/* EXTERN str_number help_line[6]; */
EXTERN char * help_line[6];
/* EXTERN char help_ptr; */
/* EXTERN integer help_ptr; */ /* padded out */
EXTERN int help_ptr; /* padded out */       /* 95/Jan/7 */
EXTERN bool use_err_help; 
/* EXTERN integer interrupt;  */
EXTERN volatile integer interrupt;  /* bkph - do avoid compiler optimization */
EXTERN bool OK_to_interrupt; 
EXTERN bool arith_error; 
EXTERN scaled tex_remainder; 
EXTERN halfword temp_ptr; 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEMAIN
  EXTERN memory_word * mainmemory;   /* remembered so can be free() later */
  EXTERN memory_word * zzzaa;
  #define zmem zzzaa
#else
  /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  EXTERN memory_word 
  /* #define zmem (zzzaa - (int)(mem_min)) */
  /*  zzzaa[mem_max - mem_min + 1]; */
  #define zmem (zzzaa - (int)(mem_bot))
  zzzaa[mem_max - mem_bot + 1];
#endif

EXTERN halfword lo_mem_max;
EXTERN halfword hi_mem_min;
EXTERN integer var_used, dyn_used;
EXTERN halfword avail; 
EXTERN halfword mem_end; 
EXTERN halfword mem_start;      /* new in 3.14159 ??? */
EXTERN halfword rover; 

/* NOTE: the following really also need to be dynamically allocated */
#ifdef DEBUG
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEMAIN
  EXTERN char * zzzab;
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* EXTERN bool  */    /* save (4 - 1) * mem_max - mem_min */
EXTERN char
/* #define freearr (zzzab - (int)(mem_min)) */
/*  zzzab[mem_max - mem_min + 1];  */
#define freearr (zzzab - (int)(mem_bot))
  zzzab[mem_max - mem_bot + 1]; 
#endif

#ifdef ALLOCATEMAIN
  EXTERN char *zzzac;
#else
/* EXTERN bool */   /* save (4 - 1) * mem_max - mem_min */
EXTERN char
/* #define wasfree (zzzac - (int)(mem_min)) */
#define wasfree (zzzac - (int)(mem_bot))
/*  zzzac[mem_max - mem_min + 1];  */
  zzzac[mem_max - mem_bot + 1]; 
#endif

EXTERN halfword was_mem_end, was_lo_max, was_hi_min; 
EXTERN bool panicking; 
#endif /* DEBUG */

EXTERN integer font_in_short_display; 
EXTERN integer depth_threshold; 
EXTERN integer breadth_max; 
/* EXTERN list_state_record nest[nest_size + 1];  */
/* EXTERN short shown_mode; */
/* EXTERN integer shown_mode; */ /* padded out */
EXTERN int shown_mode; /* padded out */   /* 95/Jan/7 */
/* EXTERN char old_setting; */
/* EXTERN integer old_setting; */ /* padded out */
EXTERN int old_setting; /* padded out */    /* 95/Jan/7 */

/* eqtn_extra is no longer used --- it should be 0 96/Jan/10 */
#ifdef INCREASEFONTS
  #define eqtb_extra (font_max - 255 + hash_extra) 
#else
  #define eqtb_extra 0
#endif

/* Probably require eqtb_extra to be zero, so hash_extra = 255 - font_max */
#if (eqtb_extra != 0)
  #error ERROR: eqtb_extra is not zero (need hash_extra equal 255 - font_max)
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEZEQTB
EXTERN memory_word * zeqtb;
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#else
#ifdef INCREASEFONTS
EXTERN memory_word eqtb[(hash_size + 4007) + eqtb_extra]; 
#else
EXTERN memory_word zeqtb[(hash_size + 4007)]; 
#endif
#endif

#ifdef INCREASEFONTS
  #define xeq_level (zzzad - (int_base + eqtb_extra))
#else
  #define xeq_level (zzzad - (int_base))
#endif
/* zzzad[844]; */
EXTERN quarterword zzzad[844]; /* ??? attempt to fix 99/Jan/5 */
/* region 5 & 6 int_base to eqtb_size: 13507 - 12663 */

#ifdef ALLOCATEHASH
#ifdef SHORTHASH
EXTERN htwohalves *zzzae;
#else
EXTERN twohalves *zzzae;
#endif
#define hash (zzzae - 514)
#else /* not allocating hash table */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef SHORTHASH
EXTERN htwohalves 
#else
EXTERN twohalves 
#endif
#define hash (zzzae - 514)

#ifdef INCREASEFONTS
  zzzae[hash_size + 267 + eqtb_extra]; 
#else
  zzzae[hash_size + 267]; 
#endif
#endif

EXTERN halfword hash_used; 
EXTERN bool no_new_control_sequence; 
EXTERN integer cs_count; 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* using allocated save stack slows it down 1% to 2% */
/* despite reallocation, we still limit it to something finite */
/* to avoid soaking up all of machine memory in case of infinite loop */
#ifdef ALLOCATESAVESTACK
  #define save_size           65536     /* arbitrary - ridiculously large */
  #define initial_save_size   1000
  #define increment_save_size 2000
  EXTERN memory_word *save_stack; 
#else
  #define save_size 8000        /* 1999/Jan/6 enough for even CRC */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  EXTERN memory_word save_stack[save_size + 1]; 
#endif

EXTERN integer save_ptr; 
EXTERN integer max_save_stack; 

/* The following could really be char instead of quarterword ... */
/* EXTERN quarterword cur_level;  */
/* EXTERN integer cur_level; */ /* padded out */
EXTERN int cur_level; /* padded out */    /* 95/Jan/7 */

/* EXTERN group_code cur_group;  */
/* EXTERN integer cur_group;  */ /* padded out */
EXTERN int cur_group; /* padded out */      /* 95/Jan/7 */

EXTERN integer cur_boundary; 
EXTERN integer mag_set; 

/* EXTERN eight_bits cur_cmd;  */
/* EXTERN integer cur_cmd;  */ /* padded out */
EXTERN int cur_cmd; /* padded out */      /* 95/Jan/7 */

/* EXTERN halfword cur_chr;  */ /* itex, tex0, tex, tex3, tex5, tex6, tex7, tex8 */
EXTERN int cur_chr;             /* 95/Jan/7 */

EXTERN halfword cur_cs; 
EXTERN halfword cur_tok; 

#ifdef ALLOCATENESTSTACK
  #define nest_size           65536         /* arbitrary - ridiculously large */
  #define initial_nest_size   100
  #define increment_nest_size 200
  EXTERN list_state_record * nest; 
#else
  #define nest_size 200       /* 1999/Jan/7 */
  EXTERN list_state_record nest[nest_size + 1]; 
#endif
EXTERN integer nest_ptr; 
EXTERN integer max_nest_stack; 
EXTERN list_state_record cur_list; 

#ifdef ALLOCATEPARAMSTACK
  #define param_size 65536        /* arbitrary - ridiculously large */
  #define initial_param_size 100
  #define increment_param_size 200
  EXTERN halfword * param_stack; 
#else
  #define param_size 500        /* 1997/Jan/17 */
EXTERN halfword param_stack[param_size + 1]; 
#endif
EXTERN integer param_ptr; 
EXTERN integer max_param_stack; 

#ifdef ALLOCATEINPUTSTACK
  #define stack_size           65536          /* arbitrary - ridiculously large */
  #define initial_stack_size   100
  #define increment_stack_size 200
  EXTERN in_state_record * input_stack; 
#else
  #define stack_size 800
  EXTERN in_state_record input_stack[stack_size + 1]; 
#endif
EXTERN integer input_ptr; 
EXTERN integer max_in_stack; 

EXTERN integer high_in_open;      /* 1997/Jan/17 */
EXTERN in_state_record cur_input; 

/* EXTERN integer in_open;  */      /* used in itex, tex2, texmf */
EXTERN int in_open;         /* 95/Jan/7 */

EXTERN integer open_parens;
EXTERN integer max_open_parens; 
EXTERN alpha_file input_file[max_in_open + 1]; 
EXTERN integer line; 
EXTERN integer line_stack[max_in_open + 1]; 

/* EXTERN char scanner_status;  */
/* EXTERN integer scanner_status;  */ /* padded out */
EXTERN int scanner_status; /* padded out */ /* 95/Jan/7 */

EXTERN halfword warning_index; 
EXTERN halfword def_ref; 

EXTERN integer align_state; 
EXTERN integer base_ptr; 
EXTERN halfword par_loc; 
EXTERN halfword par_token; 
EXTERN bool force_eof; 
/* EXTERN halfword cur_mark[5];  */
EXTERN halfword cur_mark[6]; 

/* EXTERN char long_state; */
/* EXTERN integer long_state; */ /* padded out */
EXTERN int long_state; /* padded out */ /* 95/Jan/7 */

/* EXTERN halfword pstack[9];  */
EXTERN halfword pstack[10]; 

/* EXTERN integer cur_val; */
EXTERN int cur_val;           /* 95/Jan/7 */

/* EXTERN char cur_val_level;  */
/* EXTERN integer cur_val_level; */ /* padded out */
EXTERN int cur_val_level; /* padded out */ /* 95/Jan/7 */

/* EXTERN small_number radix;  */
/* EXTERN integer radix;  */ /* padded out */
EXTERN int radix; /* padded out */    /* 95/Jan/7 */

/* EXTERN glue_ord cur_order;  */
/* EXTERN integer cur_order;  */ /* padded out */
EXTERN int cur_order; /* padded out */    /* 95/Jan/7 */

EXTERN alpha_file read_file[16];  /* hard wired limit in TeX */
/* EXTERN char read_open[17];  */
EXTERN char read_open[20]; /* padded out */

EXTERN halfword cond_ptr; 

/* EXTERN char if_limit; */
/* EXTERN integer if_limit; */ /* padded out */
EXTERN int if_limit; /* padded out */   /* 95/Jan/7 */

/* EXTERN small_number cur_if; */
/* EXTERN integer cur_if; */ /* padded out */
EXTERN int cur_if; /* padded out */   /* 95/Jan/7 */

EXTERN integer if_line; 
EXTERN integer skip_line; 
EXTERN str_number cur_name; 
EXTERN str_number cur_area; 
EXTERN str_number cur_ext; 
EXTERN pool_pointer area_delimiter; 
EXTERN pool_pointer ext_delimiter; 
EXTERN integer format_default_length; 
EXTERN char * TEX_format_default; 
EXTERN bool name_in_progress; 
EXTERN bool log_opened; 
EXTERN bool quoted_file_name;
EXTERN str_number job_name; 
EXTERN str_number output_file_name;   // DVI file
EXTERN str_number texmf_log_name;   // LOG file
EXTERN byte_file dvi_file; 
EXTERN byte_file tfm_file;
EXTERN byte_file pdf_file;
EXTERN char * dvi_file_name;
EXTERN char * pdf_file_name;
EXTERN char * log_file_name;

/*******************************************************************/

/* SHORTFONTINFO halves the memory word used to store font info */
/* if it is not defined we use ordinary TeX memory words */

#ifdef SHORTFONTINFO
/* keep definition of fmemoryword in texmfmem.h */
/* keep definition of ffourquarters in texfmem.h */
/* keep definition of fquarterword in texfmem.h */
#else
/* go back to original definition as TeX memory word */
#undef fmemoryword
#define fmemoryword memory_word
/* go back to original definition as fourquarters of a TeX word */
#undef ffourquarters
#define ffourquarters fourquarters
/* go back to original definition as quaterword */
#undef fquarterword
#define fquarterword quarterword
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEFONT
  EXTERN fmemoryword * font_info; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#else
/* EXTERN memory_word font_info[font_mem_size + 1];  */
  EXTERN fmemoryword font_info[font_mem_size + 1]; 
#endif

EXTERN font_index fmem_ptr; 
EXTERN internal_font_number font_ptr;
EXTERN internal_font_number frozenfontptr;        /* 99/Mar/26 */
/* There are about 24 integer size items per font, or about 100 bytes */
EXTERN ffourquarters font_check[font_max + 1]; 

EXTERN scaled font_size[font_max + 1]; 
EXTERN scaled font_dsize[font_max + 1]; 
EXTERN font_index font_params[font_max + 1];    /* in 3.14159 */
EXTERN str_number font_name[font_max + 1]; 
EXTERN str_number font_area[font_max + 1]; 
EXTERN eight_bits font_bc[font_max + 1]; /* already padded 511 + 1 = 512 */
EXTERN eight_bits font_ec[font_max + 1]; /* already padded 511 + 1 = 512 */
EXTERN halfword font_glue[font_max + 1]; 
/* use char instead of bool to save space, but is it worth slow down ? */
/* EXTERN char font_used[font_max + 1];  */
EXTERN bool font_used[font_max + 1]; 

/* might want to make some of following only one character wide also ? */
/* except some use -1 as special case value */
/* well, at least make them short instead of integer */
EXTERN integer hyphen_char[font_max + 1]; 
EXTERN integer skew_char[font_max + 1]; 
EXTERN font_index bchar_label[font_max + 1]; 
EXTERN short font_bchar[font_max + 1];  /* already padded 1023 + 1 = 1024 */
/* don't change above to int or format file will be incompatible */
EXTERN short font_false_bchar[font_max + 1];  /* already padded 1023 + 1 = 1024 */
/* don't change above to int or format file will be incompatible */
EXTERN integer char_base[font_max + 1]; 
EXTERN integer width_base[font_max + 1]; 
EXTERN integer height_base[font_max + 1]; 
EXTERN integer depth_base[font_max + 1]; 
EXTERN integer italic_base[font_max + 1]; 
EXTERN integer lig_kern_base[font_max + 1]; 
EXTERN integer kern_base[font_max + 1]; 
EXTERN integer exten_base[font_max + 1]; 
EXTERN integer param_base[font_max + 1]; 

EXTERN ffourquarters null_character; 

EXTERN integer total_pages; 
EXTERN scaled max_v; 
EXTERN scaled max_h; 
EXTERN integer max_push; 
EXTERN integer last_bop; 
EXTERN integer dead_cycles; 
EXTERN bool doing_leaders; 

/* EXTERN quarterword c, f;  */
/* EXTERN integer c, f */; /* padded out */
EXTERN int c, f; /* padded out */       /* 95/Jan/7 */

EXTERN scaled rule_ht, rule_dp, rule_wd; 
EXTERN halfword g; 
EXTERN integer lq, lr; 

#ifdef ALLOCATEDVIBUF
  EXTERN eight_bits *zdvibuf; 
#else
  /* EXTERN eight_bits dvi_buf[dvi_buf_size + 1]; */
  /* EXTERN eight_bits dvi_buf[dvi_buf_size + 4]; */ /* padded out  */
  EXTERN eight_bits zdvibuf[dvi_buf_size + 4];  /* padded out 1996/Jan/18 */
#endif

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
EXTERN integer cur_s; 
EXTERN scaled total_stretch[4], total_shrink[4]; /* padded already */
EXTERN integer last_badness; 
EXTERN halfword adjust_tail; 
EXTERN integer pack_begin_line; 
EXTERN twohalves empty_field; 
EXTERN fourquarters null_delimiter; 
EXTERN halfword cur_mlist; 

/* EXTERN small_number cur_style; */
/* EXTERN integer cur_style; */  /* padded out */ /* tex5.c, tex7.c */
EXTERN int cur_style;  /* padded out */     /* 95/Jan/7 */

/* EXTERN small_number cur_size; */
/* EXTERN integer cur_size;  */ /* padded out */
EXTERN int cur_size;  /* padded out */        /* 95/Jan/7 */

EXTERN scaled cur_mu; 
EXTERN bool mlist_penalties; 
EXTERN internal_font_number cur_f; 

/* EXTERN quarterword cur_c; */
/* EXTERN integer cur_c; */  /* padded out */
EXTERN int cur_c;  /* padded out */     /* 95/Jan/7 */

EXTERN ffourquarters cur_i;
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
EXTERN bool no_shrink_error_yet;
EXTERN halfword cur_p;
EXTERN bool second_pass;
EXTERN bool final_pass;
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
/* EXTERN short hc[64+2]; */  /* padded OK 66 * 2 = 132 which is divisible by 4 */
EXTERN int hc[66];  /* padded OK 66 * 2 = 132 which is divisible by 4 */

/* EXTERN small_number hn; */
/* EXTERN integer hn; */  /* padded out */
EXTERN int hn;  /* padded out */      /* 95/Jan/7 */

EXTERN halfword ha, hb;

/* EXTERN internal_font_number hf;  */
EXTERN int hf;              /* 95/Jan/7 */

/* EXTERN short hu[64+2]; */    /* padded OK */
EXTERN int hu[66];    /* padded OK */ 

/* EXTERN integer hyf_char;  */
EXTERN int hyf_char;            /* 95/Jan/7 */

/* init_cur_lang new in 3.14159 */
/* EXTERN ASCII_code cur_lang, init_cur_lang; */
/* EXTERN integer cur_lang, init_cur_lang; */ /* padded out */
EXTERN int cur_lang, init_cur_lang; /* padded out */    /* 95/Jan/7 */

EXTERN integer lhyf, rhyf;
/* EXTERN init_l_hyf, init_r_hyf; */ /* new in 3.14159 */
EXTERN integer init_l_hyf, init_r_hyf;  /* new in 3.14159 */

EXTERN halfword hyfbchar;
/* EXTERN char hyf[65];  */
EXTERN char hyf[68]; /* padded out */
EXTERN halfword init_list;
EXTERN bool init_lig;
EXTERN bool init_lft;

/* EXTERN small_number hyphen_passed; */
/* EXTERN integer hyphen_passed; */  /* padded out */
EXTERN int hyphen_passed;  /* padded out */     /* 95/Jan/7 */

/* EXTERN halfword cur_l, cur_r; */   /* make int's ? 95/Jan/7 */
EXTERN int cur_l, cur_r;    /* make int's ? 95/Jan/7 */

EXTERN halfword cur_q;
EXTERN halfword lig_stack;
EXTERN bool ligature_present;
EXTERN bool lft_hit, rt_hit;

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* could perhaps use packed_ASCII_code for trie_trc ? */
#ifdef ALLOCATETRIES
  EXTERN halfword * trie_trl;
  EXTERN halfword * trie_tro;
  EXTERN quarterword * trie_trc;
#else
  /* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
  EXTERN halfword trie_trl[trie_size + 1];
  EXTERN halfword trie_tro[trie_size + 1];
  EXTERN quarterword trie_trc[trie_size + 1];
#endif
EXTERN small_number hyf_distance[trie_op_size + 1]; /* already padded 751 + 1 */
EXTERN small_number hyf_num[trie_op_size + 1];    /* already padded 751 + 1 */
EXTERN trie_op_code hyf_next[trie_op_size + 1];
EXTERN integer op_start[256];

/* if ALLOCATEHYPHEN is true, then hyphen_prime is a variable */
/* otherwise it is a pre-processor defined constant */
#ifdef ALLOCATEHYPHEN
  #define default_hyphen_prime 1009
  EXTERN str_number * hyph_word;
  EXTERN halfword * hyph_list;
  /* EXTERN hyphen_prime; */
  EXTERN integer hyphen_prime;
#else
  #define hyphen_prime 607
  /* EXTERN str_number hyph_word[608];  */
  EXTERN str_number hyph_word[hyphen_prime + 1]; 
  /* EXTERN halfword hyph_list[608];  */
  EXTERN halfword hyph_list[hyphen_prime + 1]; 
#endif

/* EXTERN hyph_pointer hyph_count;  */
/* EXTERN integer hyph_count; */  /* padded out */
EXTERN int hyph_count;  /* padded out */    /* 95/Jan/7 */

#ifdef INITEX
EXTERN integer trie_op_hash_C[trie_op_size - neg_trie_op_size + 1];
#define trie_op_hash (trie_op_hash_C - (int)(neg_trie_op_size)) 
EXTERN trie_op_code trie_used[256]; 
EXTERN ASCII_code trie_op_lang[trie_op_size + 1];   /* already padded 751 + 1 */
EXTERN trie_op_code trie_op_val[trie_op_size + 1]; 
EXTERN integer trie_op_ptr; 
#endif /* INITEX */

EXTERN trie_op_code max_op_used; 
EXTERN bool smallop; 

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
    /* EXTERN bool *trie_taken; */ /* save (4 - 1) * trie_size = 90,000 byte */
    EXTERN char *trie_taken; 
  #else
    EXTERN bool trie_taken[trie_size + 1]; 
  #endif

  EXTERN trie_pointer trie_min[256]; 
  EXTERN trie_pointer trie_max; 
  EXTERN bool trie_not_ready; 
#endif /* INITEX */
EXTERN scaled best_height_plus_depth; 
EXTERN halfword page_tail; 

/* EXTERN char page_contents; */
/* EXTERN integer page_contents; */ /* padded out */
EXTERN int page_contents; /* padded out */      /* 95/Jan/7 */

/* ********************************************************************* */

/* do *some* sanity checking here --- rather than in TeX later 96/Jan/18 */
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

/* ********************************************************************* */

EXTERN scaled page_max_depth; 
EXTERN halfword best_page_break; 
EXTERN integer least_page_cost; 
EXTERN scaled best_size; 
EXTERN scaled page_so_far[8]; 
EXTERN halfword last_glue; 
EXTERN integer last_penalty; 
EXTERN scaled last_kern; 
EXTERN integer insert_penalties; 
EXTERN bool output_active; 
EXTERN internal_font_number main_f; 

EXTERN ffourquarters main_i; 
EXTERN ffourquarters main_j; 

EXTERN font_index main_k; 
EXTERN halfword main_p; 
EXTERN integer main_s; 
EXTERN halfword bchar; 
EXTERN halfword false_bchar; 
EXTERN bool cancel_boundary; 
EXTERN bool ins_disc; 
EXTERN halfword cur_box; 
EXTERN halfword after_token; 
EXTERN bool long_help_seen; 
EXTERN str_number format_ident; 
EXTERN word_file fmt_file; 
EXTERN integer ready_already; 

EXTERN alpha_file write_file[16]; /* hard wired limit in TeX */
EXTERN bool write_open[18]; 

EXTERN halfword write_loc; 
EXTERN pool_pointer edit_name_start; 
/* EXTERN integer edit_name_length, edit_line, tfm_temp;  */
EXTERN integer edit_name_length, edit_line; 
/* EXTERN integer tfm_temp; */    /* only used in tex3.c */
EXTERN int tfm_temp;        /* only used in tex3.c 95/Jan/7 */

/* new stuff defined in local.c - bkph */

EXTERN bool is_initex;
EXTERN bool verbose_flag;
EXTERN bool trace_flag;
EXTERN bool debug_flag;
EXTERN bool heap_flag;
EXTERN bool open_trace_flag;
EXTERN bool cache_file_flag;
EXTERN bool knuth_flag;
EXTERN bool no_interrupts;
EXTERN bool c_style_flag;
EXTERN bool non_ascii;
EXTERN bool key_replace;
EXTERN bool deslash;
EXTERN bool trimeof;
EXTERN bool allow_patterns;
EXTERN bool show_fonts_used;
EXTERN bool reset_exceptions;
EXTERN bool show_current;
EXTERN bool current_flag;
EXTERN bool current_tfm;
EXTERN bool return_flag;
EXTERN bool want_version;
EXTERN bool civilize_flag;
EXTERN bool show_numeric;
EXTERN bool restrict_to_ascii;
EXTERN bool show_missing;
EXTERN bool full_file_name_flag;
EXTERN bool save_strings_flag;
EXTERN int mem_initex;
EXTERN int mem_extra_high;
EXTERN int mem_extra_low;
EXTERN int new_hyphen_prime;
EXTERN int missing_characters;
EXTERN int show_in_hex;
EXTERN int show_in_dos;
EXTERN int test_dir_access;
EXTERN int dir_method;
EXTERN int file_method;
/* EXTERN int waitflush; */
EXTERN int show_fmt_flag;
EXTERN int show_tfm_flag;
EXTERN bool show_texinput_flag;  /* 1998/Jan/28 */
EXTERN bool truncate_long_lines; /* 1998/Feb/2 */
EXTERN bool show_cs_names;       /* 1998/Mar/31 */
EXTERN int tab_step;
EXTERN int pseudo_tilde;
EXTERN int pseudo_space;
EXTERN int allow_quoted_names;
EXTERN int default_rule;
EXTERN char * format_file;
EXTERN char * source_direct;     /* 1998/Sep/29 */
EXTERN char * string_file;
EXTERN int share_flag;
EXTERN char * format_name;
EXTERN char * encoding_name;
EXTERN bool format_specific;
EXTERN bool encoding_specific;
EXTERN bool show_line_break_stats;  /* 1996/Feb/9 */
EXTERN int first_pass_count;        /* 1996/Feb/9 */
EXTERN int second_pass_count;       /* 1996/Feb/9 */
EXTERN int final_pass_count;        /* 1996/Feb/9 */
EXTERN int underfull_hbox;          /* 1996/Feb/9 */
EXTERN int overfull_hbox;           /* 1996/Feb/9 */
EXTERN int underfull_vbox;          /* 1996/Feb/9 */
EXTERN int overfull_vbox;           /* 1996/Feb/9 */
EXTERN int paragraph_failed;        /* 1996/Feb/9 */
EXTERN int single_line;             /* 1996/Feb/15 */
EXTERN FILE * errout;
EXTERN int font_dimen_zero;   /* 1998/Oct/5 */
EXTERN int ignore_frozen;     /* 1998/Oct/5 */
EXTERN bool suppress_f_ligs;  /* 1999/Jan/5 */
EXTERN int abort_flag;      // not yet hooked up ???
EXTERN int err_level;     // not yet hooked up ???
EXTERN int jump_used;       /* 1999/Nov/28 */
EXTERN jmp_buf jumpbuffer;  /* 1999/Nov/7 */
extern int current_pool_size;        /* in local.c - bkph */
extern int current_max_strings;      /* in local.c - bkph */
extern int current_mem_size;         /* in local.c - bkph */
extern int current_font_mem_size;    /* in local.c - bkph */
extern int current_save_size;        /* in local.c - bkph */
extern int current_stack_size;       /* in local.c - bkph */
extern int current_nest_size;        /* in local.c - bkph */
extern int current_param_size;       /* in local.c - bkph */
extern int current_buf_size;         /* in local.c - bkph */
extern char *tex_version;            /* in local.c - bkph */
extern char *application;            /* in local.c - bkph */
extern char *yandyversion;           /* in local.c - bkph */
extern unsigned char wintodos[128];  /* in local.c - bkph */
extern char log_line[MAXLINE];       /* in local.c */
extern char *texpath;           /* in local.c */

memory_word * allocate_main_memory (int);     /* in local.c - bkph */
memory_word * realloc_main (int, int);        /* in local.c - bkph */
packed_ASCII_code * realloc_str_pool (int);   /* in local.c - bkph */
pool_pointer * realloc_str_start (int);       /* in local.c - bkph */
memory_word * realloc_save_stack (int);       /* in local.c - bkph */
list_state_record * realloc_nest_stack (int); /* in local.c - bkph */
in_state_record * realloc_input_stack (int);  /* in local.c - bkph */
halfword * realloc_param_stack (int);         /* in local.c - bkph */
ASCII_code * realloc_buffer (int);            /* in local.c - bkph */
fmemoryword * realloc_font_info (int);        /* in local.c - bkph */

int realloc_hyphen (int);         /* in local.c - bkph */
int allocate_tries (int);         /* in local.c - bkph */
void check_eqtb (char *);          /* in local.c - bkph */
void probe_memory (void);          /* in local.c - bkph */
void print_cs_names (FILE *, int); /* in local.c - bkph */
void perrormod(char *);            /* in local.c */
char *grabenv(char *);             /* in local.c - bkph */
void stamp_it (char *);            /* in local.c - bkph */
void stampcopy (char *);           /* in local.c - bkph */
bool prime (int);                  /* in local.c - bkph */
int endit (int);                   /* in local.c - bkph */

void uexit (int unix_code);     /* in lib/uexit.c - bkph */
void t_open_in (void);          /* in lib/texmf.c - bkph */


void call_edit (ASCII_code *filename, pool_pointer fnstart,
                integer fnlength, integer linenumber); /* from lib/texmf.c - bkph */

void add_variable_space(int);       /* in itex.c - bkph */

void get_date_and_time (integer *minutes, integer *day,
                        integer *month, integer *year);   /* in lib/texmf.c - bkph */

char *unixify (char *);       /* in pathsrch.c bkph */

/****************************************************************************/

#include "coerce.h"

/****************************************************************************/
#define font_base 0
/* sec 0022 */
#define null_code       0     // 0
#define carriage_return 015   // 13
#define invalid_code    0177  // 127
/* sec 0040 */
#define length(s) (str_start[(s) + 1] - str_start[(s)])
/* sec 0041 */
#define cur_length (pool_ptr - str_start[str_ptr])
/* sec 0054 */
#define no_print     16
#define term_only    17
#define log_only     18
#define term_and_log 19
#define pseudo       20
#define new_string   21
#define max_selector 21
/* sec 0073 */
#define batch_mode      0
#define nonstop_mode    1
#define scroll_mode     2
#define error_stop_mode 3
/* sec 0076 */
#define spotless             0
#define warning_issued       1
#define error_message_issued 2
#define fatal_error_stop     3
/* sec 0105 */
#define nx_plux_y(...)   mult_and_add(..., 07777777777L)
#define mult_integers(a) mult_and_add(a,0,017777777777L)
/* sec 0108 */
#define inf_bad 10000L
/* sec 0109 */
#define set_glue_ratio_zero(a) (a) = 0.0
#define set_glue_ratio_one(a)  (a) = 1.0
#define tex_float(a)           (a)
#define unfloat(a)             (a)
#define float_constant(a)      (float) (a)
/* sec 0115*/
#define pointer halfword
#define null    min_halfword
/* sec 0118 */
#define link(p) mem[(p)].hh.v.RH
#define info(p) mem[(p)].hh.v.LH
/* sec 0124 */
#define empty_flag  max_halfword
#define is_empty(a) (link(a) = empty_flag)
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
#define is_running(a)  (a = null_flag)
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
#define toks_register 71
#define assign_toks 72
#define assign_int 73
#define assign_dimen 74
#define assign_glue 75
#define assign_mu_glue 76
#define assign_font_dimen 77
#define assign_font_int 78
#define set_aux 79
#define set_prev_graf 80
#define set_page_dimen 81
#define set_page_int 82
#define set_box_dimen 83
#define set_shape 84
#define def_code 85
#define def_family 86
#define set_font 87
#define def_font 88
#define tex_register 89
#define max_internal 89
#define advance 90
#define multiply 91
#define divide 92
#define prefix 93
#define let 94
#define shorthand_def 95
#define read_to_cs 96
#define def 97
#define set_box 98
#define hyph_data 99
#define set_interaction 100
#define max_command 100
/* sec 0210 */
#define undefined_cs    (max_command + 1)
#define expand_after    (max_command + 2)
#define no_expand       (max_command + 3)
#define input           (max_command + 4)
#define if_test         (max_command + 5)
#define fi_or_else      (max_command + 6)
#define cs_name         (max_command + 7)
#define convert         (max_command + 8)
#define the             (max_command + 9)
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
#define space_factor    aux.hh.v.LH
#define clang           aux.hh.v.RH
#define incompleat_noad aux.cint
#define prev_graf       cur_list.pg_field
#define mode_line       cur_list.ml_field
/* sec 0221 */
#define eq_level_field(a) a.hh.b1
#define eq_type_field(a)  a.hh.b0
#define equiv_field(a)    a.hh.v.RH
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
#define undefined_control_sequence    (frozen_null_font + 1025)            // (hash_size + hash_extra + 781) = font_max + 2
                                                                           // (hash_size + (255 - 1023) + 1025 + 524)
                                                                           // (hash_size + 781)
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
#define text(a)         hash[a].v.RH
#define next(a)         hash[a].v.LH
#define hash_is_full    (hash_used == hash_base)
#define font_id_text(a) text(font_id_base + a)
/* sec 0268 */
#define save_type(a)      save_stack[a].hh.b0
#define save_level(a)     save_stack[a].hh.b1
#define save_index(a)     save_stack[a].hh.v.RH
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
/* sec 0303 */
#define mid_line    1
#define skip_blanks 2 + max_char_code // 17
#define new_line    3 + max_char_code + max_char_code // 33
/* sec 0305 */
#define skipping  1
#define defining  2
#define matching  3
#define aligning  4
#define absorbing 5
/* sec 0307 */
#define token_list         0
#define token_type         cur_input.index_field
#define param_start        cur_input.limit_field
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
/* sec 0358 */
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
#define zero_token    (other_token + '0')  // 3120
#define A_token       (letter_token + 'A') // 2881
#define other_A_token (other_token + 'A')  // 3137
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
/* sec 0564 */
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
//#define dvi_push 141
//#define dvi_pop  142
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
/* sec 0644 */
#define exactly    0
#define additional 1
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
/* sec 0725 */
#define new_hlist(a) mem[nucleus(a)].cint
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
#define do_all_six(a) a(1); a(2); a(3); a(4); a(5); a(6)
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
/* sec 0877 */
#define next_break prev_break
/* sec 0908 */
#define append_charnode_to_t(a) \
  {                             \
    link(t) = get_avail();      \
    t = link(t);                \
    font(t) = hf;               \
    character(t) = (a);         \
  }
#define set_cur_r()      \
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
  }
/* sec 0910 */
#define wrap_lig(a)                           \
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
  }
#define pop_lig_stack()                       \
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
  }
/* sec 0914 */
#define advance_major_tail()       \
  {                                \
    major_tail = link(major_tail); \
    incr(r_count);                 \
  }
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
}
/* sec 1035 */
/* -> false */
#define wrapup(a)                                         \
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
}
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
/* sec 79 */
extern void synch_h(void);
extern void synch_v(void);
extern void set_cur_lang(void);
extern str_number make_string_pool (char *s);
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
extern int load_pool_strings (integer spare_size);
#define help0()     tex_help(0)
#define help1(...)  tex_help(1, __VA_ARGS__)
#define help2(...)  tex_help(2, __VA_ARGS__)
#define help3(...)  tex_help(3, __VA_ARGS__)
#define help4(...)  tex_help(4, __VA_ARGS__)
#define help5(...)  tex_help(5, __VA_ARGS__)
#define help6(...)  tex_help(6, __VA_ARGS__)

/********BINDING WITH LIBHARU*********/

EXTERN HPDF_Doc  yandy_pdf;
EXTERN HPDF_Page yandy_page;
EXTERN HPDF_Font yandy_font[1024];
bool pdf_doing_string;
bool pdf_doing_text;
bool pdf_output_flag;
EXTERN tree *avl_tree;
EXTERN void init_tfm_map(void);
EXTERN void free_tfm_map(void);
EXTERN void pdf_ship_out(pointer p);
EXTERN void pdf_vlist_out (void);
EXTERN void pdf_hlist_out (void);
EXTERN void pdf_begin_text(void);
EXTERN void pdf_font_def(internal_font_number f);
EXTERN void pdf_error_handler (HPDF_STATUS error_no, HPDF_STATUS detail_no, void * user_data);
/********BINDING WITH LIBHARU*********/