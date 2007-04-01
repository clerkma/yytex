/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#ifdef MSDOS
/* allocate iniTeX (550 k) triec, trieo, triel, trier, triehash, trietaken */
#define ALLOCATEINI
/* allocate main memory for TeX (2 Meg) zmem = zzzaa */
#define ALLOCATEMAIN
/* NOT *//* allow increasing high area of main memory */ /* FLUSH */
#undef ALLOCATEHIGH  
/* NOT *//* allow increasing low area of main memory */ /* FLUSH */
#undef ALLOCATELOW  
/* allocate fontinfo (800 k) (dynamically now) */
#define ALLOCATEFONT 
/* allocate hyphenation trie stuff (270 k) trietrl, trietro, trietrc */
#define ALLOCATETRIES
/* allocate hyphenation exception tables */
#define ALLOCATEHYPHEN 
/* allow triesize to be variable */
#define VARIABLETRIESIZE
/* allocate strings and string pointers (184 k ) strpool & strstart */
#define ALLOCATESTRING 
/* NOT */ /* allocate hash table (zzzae) (78k) */
#undef ALLOCATEHASH
/* NOT */ /* allocate dvibuf (16k) */ /* changed in 1.3 1996/Jan/18 */
/* #define ALLOCATEDVIBUF */
#undef ALLOCATEDVIBUF 
/* experiment to dynamically deal with savestack 99/Jan/4 */
#define ALLOCATESAVESTACK
/* experiment to dynamically deal with inputstack 99/Jan/21 */
#define ALLOCATEINPUTSTACK
/* experiment to dynamically deal with neststack 99/Jan/21 */
#define ALLOCATENESTSTACK
/* experiment to dynamically deal with paramstack 99/Jan/21 */
#define ALLOCATEPARAMSTACK
/* experiment to dynamically deal with input buffer 99/Jan/22 */
#define ALLOCATEBUFFER
/* increase fixed allocations */
#define INCREASEFIXED
/* increase number of fonts - quarterword 16 bit - maxquarterword limit */
/* there may still be some bugs with this however ... also may slow down */
/* also: should split use of quarterword for (i) font from (ii) char */
/* for example, xeqlevel ? hyphenation trietrc ? */
#define INCREASEFONTS 
/* NOT NOT *//* allocate eqtb (108k) */ /* changed in 1.3 1996/Jan/18 */
#undef ALLOCATEZEQTB
/* make fontinfo array fmemoryword == 32 bit instead of memoryword = 64 bit */
#define SHORTFONTINFO
/* make hash table htwohalves == 32 bit instead of twohalves == 64 bit */
// #define SHORTHASH	--- changed 2000/Feb/22 increase maxstrings from 64K to 512M
#undef SHORTHASH
/* increase trieopsize from 751 to 3001 96/Oct/12 */
#define INCREASETRIEOP
#endif

/* With old PharLap linker it was important to avoid large fixed allocation */
/* Now may be better to use fixed arrays rather than allocate them */
/* hence ALLOCATEZQTB, ALLOCATEDVIBUF and ALLOCATEMINOR are NOT defined */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#undef	TRIP
#undef	TRAP
#define	STAT
#undef	DEBUG
#include "texmf.h"

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define MAXLINE 256					// for logline buffer

/* #define maxhalfword 65535L */	/* for 32 bit memory word */
/* #define maxhalfword 262143L */	/* for 36 bit memory word */
#define minhalfword -2147483647L 	/* for 64 bit memory word (signed) */
#define maxhalfword 2147483647L 	/* for 64 bit memory word (signed) */
#define emptyflag maxhalfword

#define blocksize 1000			/* blocksize for variable length node alloc */

/* minquarterword assumed 0 -- i.e. we use unsigned types for quarterword */
#define minquarterword 0
#ifdef INCREASEFONTS
#define maxquarterword 65535L
#else
#define maxquarterword 255
#endif

/* #define defaultmemtop 262140L */	/* usual big TeX allocation 2 Meg bytes */
/* #define defaultmemtop 131070L */ /*							1 Meg bytes */
#define defaultmemtop 65534L 		/* usual small TeX allocation 0.5 Meg   */

/* membot smallest index in mem array dumped by iniTeX memtop >= memmin */
#define membot 0
/* memtop largest index in mem array dumped by iniTeX memtop <= memmax */
#ifdef ALLOCATEMAIN
EXTERN integer memtop ;
#define maxmemsize (maxhalfword / sizeof(memoryword) -1)
#else
#define memtop 262140L 
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* memmax == memtop in iniTeX, otherwise memmax >= memtop */
/* if ALLOCATEMAIN is true, then memmax is a variable */
/* otherwise it is a pre-processor defined constant */
#ifdef ALLOCATEMAIN
EXTERN integer memmax ;
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* #define memmax 262140L */
#define memmax memtop
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* if ALLOCATEMAIN is true, then memmin is a variable */
/* otherwise it is a pre-processor defined constant */
/* memmin == membot in iniTeX, otherwise memmin <= membot */
#ifdef ALLOCATEMAIN
EXTERN integer memmin ;
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#define memmin 0
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

#define poolname TEXPOOLNAME 

/* #define memtop 262140L */

/* type definitions follow */

typedef unsigned char ASCIIcode  ; 
typedef unsigned char eightbits  ; 
typedef integer poolpointer  ; 
typedef integer strnumber  ; 
typedef unsigned char packedASCIIcode  ; 
typedef integer scaled  ; 
typedef integer nonnegativeinteger  ; 
typedef char smallnumber  ; 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* bufsize is max size of input line and max size of csname */
/* make sure its multiple of four bytes long */
/* want to increase this so it can handle whole paragraph imported from WP */
#ifdef INCREASEFIXED
/* #define bufsize 8192 */
/* #define bufsize 12000 */ 		/* 1996/Jan/17 */
/* #define bufsize 16384 */			/* 1998/June/30 */
/* #define bufsize 20000 */			/* 1999/Jan/7 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* #define bufsize 3000  */
#endif

#ifdef ALLOCATEBUFFER
#define initialbufsize 1000
#define incrementbufsize 2000
#define bufsize 2000000L				/* arbitrary limit <= maxhalfword */
EXTERN ASCIIcode *buffer  ; 
#else
#define bufsize 20000					/* 1999/Jan/7 */
EXTERN ASCIIcode buffer[bufsize + 4]  ; /* padded out to ...  + 4 */
#endif
EXTERN integer first  ; 
EXTERN integer last  ; 
EXTERN integer maxbufstack  ; 

#define errorline 79 
#define halferrorline 50 
#define maxprintline 79 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef INCREASEFIXED
/* maximum number of simultaneous input sources (?) */
/* #define stacksize 600 */		/* expanded again 1999/Jan/21 */
/* #define stacksize 800 */
/* maximum number of input files and insertions that can be going on */
/* #define maxinopen 15 */		/* for Y&Y TeX 1.2 */
/* #define maxinopen 31 */		/* 1996/Jan/17 */
/* #define maxinopen 63 */		/* 1996/Jan/18 */
#define maxinopen 127			/* 1996/Jan/20 - really ? */
/* savesize space for saving values outside of current group */
/* #define savesize 6000 */
/* #define savesize 8000 */ 			/* 1999/Jan/6 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* #define stacksize 300 */		/* Unix C version default */
#define maxinopen 15
/* #define savesize 1000 */		/* 3.14159 C version */
/* #define savesize 4000 */		/* 3.14159 C version */
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* maximum internal font number - cannot be greated than max_quarter_word ! */
#ifdef INCREASEFONTS
/* #define fontmax 511 */
#define fontmax 1023			/* 1996/Jan/17 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#define fontmax 255 
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* free the limit on font memory ! */ /* (2^32 - 1) / sizeof(memoryword) */
#ifdef ALLOCATEFONT
/* #define fontmemsize 262140L */
#define fontmemsize (maxhalfword / sizeof(memoryword) -1)
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#define fontmemsize 100000L
#endif

/* our real fontmemsize is 268435456 --- ridiculously large, of course */

/* #define non_address fontmemsize */		/* 3.141 */
#define non_address 0						/* 3.14159 96/Jan/16 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* below is new dynamic allocation - bkph 93/Nov/28 */	/* enough for lplain */
#ifdef ALLOCATEFONT
#define initialfontmemsize 20000
#define incrementfontmemsize 40000
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* paramsize maximum number of simultaneous macro parameters */
/* nestsize  maximum number of semantic levels simultaneously active */
#ifdef INCREASEFIXED
/* #define paramsize 60 */			/* 3.14159 C version */
/* #define paramsize 120 */
/* #define paramsize 200 */			/* 1994/Oct/11 */
/* #define paramsize 300 */			/* 1995/May/15 */
/* #define paramsize 500 */			/* 1997/Jan/17 */
/* #define nestsize 40 */			/* 3.14159 C version */
/* #define nestsize 80 */
/* #define nestsize 100	*/			/* 1994/Oct/11 */
/* #define nestsize 120 */			/* 1995/May/15 */
/* #define nestsize 200 */			/* 1999/Jan/7 */
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* #define paramsize 60 */			/* Unix C default */
/* #define nestsize 40 */			/* Unix C default */
/* #define nestsize 100 */			/* Unix C default */
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* maxstrings max number of strings */ /* (2^32 - 1) / sizeof (integer) */
#ifdef ALLOCATESTRING
/* #define maxstrings 262140L */
// #define maxstrings (maxhalfword / sizeof(integer) -1)
#define maxstrings (maxhalfword / sizeof(poolpointer) -1)
/* #define poolsize 4032000L */
#define poolsize (maxhalfword - 1)
/* #define stringmargin 32768L */
#define stringmargin 10000
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* #define maxstrings 15000 */
#define maxstrings 16384
#define poolsize 124000L 
#endif
#define stringvacancies 100000L 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* #if defined (ALLOCATEINITRIE) && defined (ALLOCATEHYPHEN) */
#ifdef VARIABLETRIESIZE
EXTERN integer triesize ;
#define defaulttriesize 60000
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#else
#define triesize 30000			/* 3.14159 C version */
#endif

/* increase trieop to accomadate more hyphenation patterns 96/OCt/12 */

#ifdef INCREASETRIEOP
#define trieopsize 3001 
#define negtrieopsize -3001
#define mintrieop 0 
#define maxtrieop 1000
#else
#define trieopsize 751 
#define negtrieopsize -751 
#define mintrieop 0 
#define maxtrieop 500
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* dvibufsize must be multiple of 8 - half is written out at a time */
#ifdef ALLOCATEDVIBUF
#define defaultdvibufsize 16384 
/* #define defaultdvibufsize 32768 */ 		/* ? */
EXTERN int dvibufsize ;
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#define dvibufsize 16384  		/* 3.14159 C version */
/* #define dvibufsize 32768 */				/* ? */
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* WARNING: increasing hash table for cs names is not trivial */
/* size of hash table for control sequence name  < (memmax - memmin) / 10 */
/* #define hash_size 9500 */
/* #define hash_size 25000 */		/* 96/Jan/10 */
#define hash_size 32768				/* 96/Jan/17 */
/* trick to try and get around eqtb_extra problem */
/* #define hash_extra -256 */
#define hash_extra (255 - fontmax) 
/* hash prime about 85% of hash_size (+ hash_extra) */
/* #define hashprime 7919  */
/* #define hash_prime 21247 */		/* 96/Jan/10 */
#define hash_prime 27197			/* 96/Jan/17 */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* CONSTRAINT: reconcile increase font use by stealing from hash table ... */

/* Probably require eqtb_extra to be zero, so hash_extra = 255 - fontmax */
#if (hash_extra != 255 - fontmax)
#error ERROR: hash_extra not equal to (255 - fontmax)
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* some sanity check when using narrow hash table ... */

/* using SHORTHASH limits hash_size to be less than 65536 */
/* using SHORTHASH limits maxstrings to be less than 65536 */
/* if you ever need more string pointers, then #undef SHORTHASH --- */
/* you'll use more memory (about 130k byte) and format file larger (8k) */

#ifdef SHORTHASH
/* can only do this if INCREASEFONTS defined up above ... */
#if (maxquarterword < 65535L)
#error ERROR: maxquarterword < 65535L
#endif
/* with narrowed hash table can only have 65535 string pointers */
/* #if (maxstrings > maxquarterword) */ /* this test does not work */
#undef maxstrings
#define maxstrings maxquarterword
/* #endif */
/* with narrowed hash table can only have 65535 hash table entries */
#if (hash_size > maxquarterword)
#undef hash_size
#define hash_size maxquarterword
#endif
#endif /* end of if SHORTHASH */

/* NOTE: if you define/undefine SHORTFONT have to redo formats */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef INCREASEFONTS 
typedef unsigned short quarterword  ; 
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
typedef unsigned char quarterword  ; 
#endif

/* possible alternative ? */ /* minhalfword = 0 and double maxhalfword ? */
/* typedef unsigned long halfword  ; NO NO: since mem_min may be < 0 */
typedef integer halfword  ; 
typedef char twochoices  ; 
typedef char fourchoices  ; 

#include "texmfmem.h"

typedef char glueord  ; 

typedef struct {
/*  short modefield ;  */
  int modefield ; 
  halfword headfield, tailfield ; 
  integer pgfield, mlfield ; 
  memoryword auxfield ; 
/*  quarterword lhmfield, rhmfield ; */	/* was in TeX 3.141 ... */
} liststaterecord  ; 

typedef char groupcode  ; 

typedef struct {
  quarterword statefield, indexfield ; 
  halfword startfield, locfield, limitfield, namefield ; 
} instaterecord  ; 

typedef integer internalfontnumber  ; 

typedef integer fontindex  ; 

typedef integer dviindex  ; 

typedef integer trieopcode  ; 

typedef integer triepointer  ; 

/* typedef short hyphpointer  ; */		/* increased 1996/Oct/20 ??? */
typedef integer hyphpointer  ; 

EXTERN integer bad  ; 
EXTERN ASCIIcode xord[256]  ; 
EXTERN ASCIIcode xchr[256]  ; 
/* EXTERN char nameoffile[PATHMAX + 1]  ; */
// EXTERN char nameoffile[PATHMAX + 4]  ;	/* padded out 512 + 4 */
EXTERN unsigned char nameoffile[PATHMAX + 4]  ;	// fix 2000 June 18 
EXTERN integer namelength  ; 

/* EXTERN ASCIIcode buffer[bufsize + 1]  ; */
/* EXTERN ASCIIcode buffer[bufsize + 4]  ; */ /* padded out to ...  + 4 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATESTRING
#define initialpoolsize 40000
#define incrementpoolsize 80000
EXTERN packedASCIIcode *strpool ; 
#define initialmaxstrings 5000
#define incrementmaxstrings 10000	
EXTERN poolpointer *strstart  ; 
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
EXTERN packedASCIIcode strpool[poolsize + 1]  ; 
EXTERN poolpointer strstart[maxstrings + 1]  ; 
#endif

EXTERN poolpointer poolptr  ; 
EXTERN strnumber strptr  ; 
EXTERN poolpointer initpoolptr  ; 
EXTERN strnumber initstrptr  ; 

#ifdef INITEX
EXTERN alphafile poolfile  ; 
#endif /* INITEX */

EXTERN alphafile logfile  ; 
/* EXTERN char selector  ; */
/* EXTERN integer selector  ; */ /* padded out */
EXTERN int selector  ;			/* padded out */ /* 95/Jan/7 */
/* EXTERN char dig[23]  ; */
EXTERN char dig[23 + 1]  ;	/* padded out */
EXTERN integer tally  ; 
EXTERN integer termoffset  ; 
EXTERN integer fileoffset  ; 
EXTERN ASCIIcode trickbuf[errorline + 1]  ; /* already padded 79 + 1 */
EXTERN integer trickcount  ; 
EXTERN integer firstcount  ; 
/* EXTERN char interaction  ;  */
/* EXTERN integer interaction  ; */ /* padded out */
EXTERN int interaction  ; /* padded out */			/* 95/Jan/7 */
EXTERN booleane deletionsallowed  ; 
EXTERN booleane setboxallowed  ; 
/* EXTERN char history  ; */
/* EXTERN integer history  ; */ /* padded out */
EXTERN int history  ; /* padded out */				/* 95/Jan/7 */
/* EXTERN schar errorcount  ;  */
/* EXTERN integer errorcount  ; */ /* padded out */
EXTERN int errorcount  ; /* padded out */			/* 95/Jan/7 */
EXTERN strnumber helpline[6]  ; 
/* EXTERN char helpptr  ; */
/* EXTERN integer helpptr  ; */ /* padded out */
EXTERN int helpptr  ; /* padded out */				/* 95/Jan/7 */
EXTERN booleane useerrhelp  ; 
/* EXTERN integer interrupt  ;  */
EXTERN volatile integer interrupt ;	/* bkph - do avoid compiler optimization */
EXTERN booleane OKtointerrupt  ; 
EXTERN booleane aritherror  ; 
EXTERN scaled texremainder  ; 
EXTERN halfword tempptr  ; 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEMAIN
EXTERN memoryword *mainmemory;		/* remembered so can be free() later */
EXTERN memoryword *zzzaa ;
#define zmem zzzaa
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
EXTERN memoryword 
/* #define zmem (zzzaa - (int)(memmin)) */
/*  zzzaa[memmax - memmin + 1]  ; */
#define zmem (zzzaa - (int)(membot))
  zzzaa[memmax - membot + 1]  ;
#endif

EXTERN halfword lomemmax  ; 
EXTERN halfword himemmin  ; 
EXTERN integer varused, dynused  ; 
EXTERN halfword avail  ; 
EXTERN halfword memend  ; 
EXTERN halfword memstart  ;			/* new in 3.14159 ??? */
EXTERN halfword rover  ; 

/* NOTE: the following really also need to be dynamically allocated */
#ifdef DEBUG
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEMAIN
EXTERN char *zzzab ;
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* EXTERN booleane  */		/* save (4 - 1) * memmax - memmin */
EXTERN char
/* #define freearr (zzzab - (int)(memmin)) */
/*  zzzab[memmax - memmin + 1]  ;  */
#define freearr (zzzab - (int)(membot))
  zzzab[memmax - membot + 1]  ; 
#endif

#ifdef ALLOCATEMAIN
EXTERN char *zzzac ;
#else
/* EXTERN booleane */		/* save (4 - 1) * memmax - memmin */
EXTERN char
/* #define wasfree (zzzac - (int)(memmin)) */
#define wasfree (zzzac - (int)(membot))
/*  zzzac[memmax - memmin + 1]  ;  */
  zzzac[memmax - membot + 1]  ; 
#endif

EXTERN halfword wasmemend, waslomax, washimin  ; 
EXTERN booleane panicking  ; 
#endif /* DEBUG */

EXTERN integer fontinshortdisplay  ; 
EXTERN integer depththreshold  ; 
EXTERN integer breadthmax  ; 
/* EXTERN liststaterecord nest[nestsize + 1]  ;  */
/* EXTERN short shownmode  ; */
/* EXTERN integer shownmode  ; */ /* padded out */
EXTERN int shownmode  ; /* padded out */		/* 95/Jan/7 */
/* EXTERN char oldsetting  ; */
/* EXTERN integer oldsetting  ; */ /* padded out */
EXTERN int oldsetting  ; /* padded out */		/* 95/Jan/7 */

/* eqtn_extra is no longer used --- it should be 0 96/Jan/10 */
#ifdef INCREASEFONTS
#define eqtb_extra (fontmax - 255 + hash_extra) 
#else
#define eqtb_extra 0
#endif

/* Probably require eqtb_extra to be zero, so hash_extra = 255 - fontmax */
#if (eqtb_extra != 0)
#error ERROR: eqtb_extra is not zero (need hash_extra equal 255 - fontmax)
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEZEQTB
EXTERN memoryword *zeqtb  ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#else
#ifdef INCREASEFONTS
/* EXTERN memoryword zeqtb[13507 + eqtb_extra]  ;  */
EXTERN memoryword zeqtb[(hash_size + 4007) + eqtb_extra]  ; 
#else
/* EXTERN memoryword zeqtb[13507]  ;  */ /* hash_size =  9500 */
/* EXTERN memoryword zeqtb[29007]  ;  */ /* hash_size = 25000 */
EXTERN memoryword zeqtb[(hash_size + 4007)]  ; 
#endif
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* EXTERN quarterword 
#define xeqlevel (zzzad -12663)		hash_size =  9500
#define xeqlevel (zzzad -28163)		hash_size = 25000
  zzzad[844]  ; */

#ifdef INCREASEFONTS
/* #define xeqlevel (zzzad - (12663 + eqtb_extra))  */
#define xeqlevel (zzzad - ((hash_size + 3163) + eqtb_extra))  
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#else
/* #define xeqlevel (zzzad -12663) */
#define xeqlevel (zzzad - (hash_size + 3163)) 
#endif
/* zzzad[844]  ; */
EXTERN quarterword zzzad[844]  ; /* ??? attempt to fix 99/Jan/5 */
/* region 5 & 6 int_base to eqtb_size: 13507 - 12663 */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* EXTERN twohalves 
#define hash (zzzae - 514)
    zzzae[9767]  ;  hash_size =  9500
    zzzae[25267]  ; hash_size = 25000 */

#ifdef ALLOCATEHASH
#ifdef SHORTHASH
EXTERN htwohalves *zzzae ;
#else
EXTERN twohalves *zzzae ;
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
/*  zzzae[9767]  ;  */
#ifdef INCREASEFONTS
  zzzae[hash_size + 267 + eqtb_extra]  ; 
#else
  zzzae[hash_size + 267]  ; 
#endif
#endif

EXTERN halfword hashused  ; 
EXTERN booleane nonewcontrolsequence  ; 
EXTERN integer cscount  ; 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* using allocated save stack slows it down 1% to 2% */
/* despite reallocation, we still limit it to something finite */
/* to avoid soaking up all of machine memory in case of infinite loop */
#ifdef ALLOCATESAVESTACK
#define savesize 65536			/* arbitrary - ridiculously large */
#define initialsavesize 1000
#define incrementsavesize 2000
EXTERN memoryword *savestack  ; 
#else
#define savesize 8000  			/* 1999/Jan/6 enough for even CRC */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
EXTERN memoryword savestack[savesize + 1]  ; 
#endif

EXTERN integer saveptr  ; 
EXTERN integer maxsavestack  ; 

/* The following could really be char instead of quarterword ... */
/* EXTERN quarterword curlevel  ;  */
/* EXTERN integer curlevel  ; */	/* padded out */
EXTERN int curlevel  ;	/* padded out */		/* 95/Jan/7 */

/* EXTERN groupcode curgroup  ;  */
/* EXTERN integer curgroup  ;  */ /* padded out */
EXTERN int curgroup  ; /* padded out */			/* 95/Jan/7 */

EXTERN integer curboundary  ; 
EXTERN integer magset  ; 

/* EXTERN eightbits curcmd  ;  */
/* EXTERN integer curcmd  ;  */ /* padded out */
EXTERN int curcmd  ; /* padded out */			/* 95/Jan/7 */

/* EXTERN halfword curchr  ;  */ /* itex, tex0, tex, tex3, tex5, tex6, tex7, tex8 */
EXTERN int curchr  ;							/* 95/Jan/7 */

EXTERN halfword curcs  ; 
EXTERN halfword curtok  ; 

#ifdef ALLOCATENESTSTACK
#define nestsize 65536					/* arbitrary - ridiculously large */
#define initialnestsize 100
#define incrementnestsize 200
EXTERN liststaterecord *nest ; 
#else
#define nestsize 200				/* 1999/Jan/7 */
EXTERN liststaterecord nest[nestsize + 1]  ; 
#endif
EXTERN integer nestptr  ; 
EXTERN integer maxneststack  ; 
EXTERN liststaterecord curlist  ; 

#ifdef ALLOCATEPARAMSTACK
#define paramsize 65536				/* arbitrary - ridiculously large */
#define initialparamsize 100
#define incrementparamsize 200
EXTERN halfword *paramstack  ; 
#else
#define paramsize 500				/* 1997/Jan/17 */
EXTERN halfword paramstack[paramsize + 1]  ; 
#endif
EXTERN integer paramptr  ; 
EXTERN integer maxparamstack  ; 

#ifdef ALLOCATEINPUTSTACK
#define stacksize 65536					/* arbitrary - ridiculously large */
#define initialstacksize 100
#define incrementstacksize 200
EXTERN instaterecord *inputstack  ; 
#else
#define stacksize 800
EXTERN instaterecord inputstack[stacksize + 1]  ; 
#endif
EXTERN integer inputptr  ; 
EXTERN integer maxinstack  ; 

EXTERN integer highinopen  ;			/* 1997/Jan/17 */
EXTERN instaterecord curinput  ; 

/* EXTERN integer inopen  ;  */			/* used in itex, tex2, texmf */
EXTERN int inopen  ;					/* 95/Jan/7 */

EXTERN integer openparens  ;
EXTERN integer maxopenparens  ; 
EXTERN alphafile inputfile[maxinopen + 1]  ; 
EXTERN integer line  ; 
EXTERN integer linestack[maxinopen + 1]  ; 

/* EXTERN char scannerstatus  ;  */
/* EXTERN integer scannerstatus  ;  */ /* padded out */
EXTERN int scannerstatus  ; /* padded out */ /* 95/Jan/7 */

EXTERN halfword warningindex  ; 
EXTERN halfword defref  ; 

EXTERN integer alignstate  ; 
EXTERN integer baseptr  ; 
EXTERN halfword parloc  ; 
EXTERN halfword partoken  ; 
EXTERN booleane forceeof  ; 
/* EXTERN halfword curmark[5]  ;  */
EXTERN halfword curmark[6]  ; 

/* EXTERN char longstate  ; */
/* EXTERN integer longstate  ; */ /* padded out */
EXTERN int longstate  ; /* padded out */	/* 95/Jan/7 */

/* EXTERN halfword pstack[9]  ;  */
EXTERN halfword pstack[10]  ; 

/* EXTERN integer curval  ; */
EXTERN int curval  ;						/* 95/Jan/7 */

/* EXTERN char curvallevel  ;  */
/* EXTERN integer curvallevel  ; */ /* padded out */
EXTERN int curvallevel  ; /* padded out */ /* 95/Jan/7 */

/* EXTERN smallnumber radix  ;  */
/* EXTERN integer radix  ;  */ /* padded out */
EXTERN int radix  ; /* padded out */		/* 95/Jan/7 */

/* EXTERN glueord curorder  ;  */
/* EXTERN integer curorder  ;  */ /* padded out */
EXTERN int curorder  ; /* padded out */		/* 95/Jan/7 */

EXTERN alphafile readfile[16]  ; 	/* hard wired limit in TeX */
/* EXTERN char readopen[17]  ;  */
EXTERN char readopen[20]  ; /* padded out */

EXTERN halfword condptr  ; 

/* EXTERN char iflimit  ; */
/* EXTERN integer iflimit  ; */ /* padded out */
EXTERN int iflimit  ; /* padded out */		/* 95/Jan/7 */

/* EXTERN smallnumber curif  ; */
/* EXTERN integer curif  ; */ /* padded out */
EXTERN int curif  ; /* padded out */		/* 95/Jan/7 */

EXTERN integer ifline  ; 
EXTERN integer skipline  ; 
EXTERN strnumber curname  ; 
EXTERN strnumber curarea  ; 
EXTERN strnumber curext  ; 
EXTERN poolpointer areadelimiter  ; 
EXTERN poolpointer extdelimiter  ; 
EXTERN integer formatdefaultlength  ; 
EXTERN ccharpointer TEXformatdefault  ; 
EXTERN booleane nameinprogress  ; 
EXTERN booleane logopened  ; 
EXTERN booleane quotedfilename ;
EXTERN strnumber jobname  ; 
EXTERN strnumber outputfilename  ;		// DVI file
EXTERN strnumber texmflogname  ;		// LOG file
EXTERN bytefile dvifile  ; 
EXTERN bytefile tfmfile  ;
EXTERN char *dvifilename ;
EXTERN char *logfilename ;

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
#define fmemoryword memoryword
/* go back to original definition as fourquarters of a TeX word */
#undef ffourquarters
#define ffourquarters fourquarters
/* go back to original definition as quaterword */
#undef fquarterword
#define fquarterword quarterword
#endif

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#ifdef ALLOCATEFONT
EXTERN fmemoryword *fontinfo ; 
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
#else
/* EXTERN memoryword fontinfo[fontmemsize + 1]  ;  */
EXTERN fmemoryword fontinfo[fontmemsize + 1]  ; 
#endif

EXTERN fontindex fmemptr  ; 
EXTERN internalfontnumber fontptr  ;
EXTERN internalfontnumber frozenfontptr  ;				/* 99/Mar/26 */
/* There are about 24 integer size items per font, or about 100 bytes */
EXTERN ffourquarters fontcheck[fontmax + 1]  ; 

EXTERN scaled fontsize[fontmax + 1]  ; 
EXTERN scaled fontdsize[fontmax + 1]  ; 
/* EXTERN halfword fontparams[fontmax + 1]  ; */
EXTERN fontindex fontparams[fontmax + 1]  ;		/* in 3.14159 */
EXTERN strnumber fontname[fontmax + 1]  ; 
EXTERN strnumber fontarea[fontmax + 1]  ; 
EXTERN eightbits fontbc[fontmax + 1]  ; /* already padded 511 + 1 = 512 */
EXTERN eightbits fontec[fontmax + 1]  ; /* already padded 511 + 1 = 512 */
EXTERN halfword fontglue[fontmax + 1]  ; 
/* use char instead of booleane to save space, but is it worth slow down ? */
/* EXTERN char fontused[fontmax + 1]  ;  */
EXTERN booleane fontused[fontmax + 1]  ; 

/* might want to make some of following only one character wide also ? */
/* except some use -1 as special case value */
/* well, at least make them short instead of integer */
EXTERN integer hyphenchar[fontmax + 1]  ; 
EXTERN integer skewchar[fontmax + 1]  ; 
EXTERN fontindex bcharlabel[fontmax + 1]  ; 
EXTERN short fontbchar[fontmax + 1]  ; 	/* already padded 1023 + 1 = 1024 */
/* don't change above to int or format file will be incompatible */
EXTERN short fontfalsebchar[fontmax + 1]  ;  /* already padded 1023 + 1 = 1024 */
/* don't change above to int or format file will be incompatible */
EXTERN integer charbase[fontmax + 1]  ; 
EXTERN integer widthbase[fontmax + 1]  ; 
EXTERN integer heightbase[fontmax + 1]  ; 
EXTERN integer depthbase[fontmax + 1]  ; 
EXTERN integer italicbase[fontmax + 1]  ; 
EXTERN integer ligkernbase[fontmax + 1]  ; 
EXTERN integer kernbase[fontmax + 1]  ; 
EXTERN integer extenbase[fontmax + 1]  ; 
EXTERN integer parambase[fontmax + 1]  ; 

EXTERN ffourquarters nullcharacter  ; 

EXTERN integer totalpages  ; 
EXTERN scaled maxv  ; 
EXTERN scaled maxh  ; 
EXTERN integer maxpush  ; 
EXTERN integer lastbop  ; 
EXTERN integer deadcycles  ; 
EXTERN booleane doingleaders  ; 

/* EXTERN quarterword c, f  ;  */
/* EXTERN integer c, f */  ; /* padded out */
EXTERN int c, f  ; /* padded out */				/* 95/Jan/7 */

EXTERN scaled ruleht, ruledp, rulewd  ; 
EXTERN halfword g  ; 
EXTERN integer lq, lr  ; 

#ifdef ALLOCATEDVIBUF
EXTERN eightbits *zdvibuf  ; 
#else
/* EXTERN eightbits dvibuf[dvibufsize + 1]  ; */
/* EXTERN eightbits dvibuf[dvibufsize + 4]  ; */ /* padded out  */
EXTERN eightbits zdvibuf[dvibufsize + 4]  ;  /* padded out 1996/Jan/18 */
#endif

EXTERN dviindex halfbuf  ; 
EXTERN dviindex dvilimit  ; 
EXTERN dviindex dviptr  ; 
EXTERN integer dvioffset  ; 
EXTERN integer dvigone  ; 
EXTERN halfword downptr, rightptr  ; 
EXTERN scaled dvih, dviv  ; 
EXTERN scaled curh, curv  ; 
EXTERN internalfontnumber dvif  ; 
EXTERN integer curs  ; 
EXTERN scaled totalstretch[4], totalshrink[4]  ; /* padded already */
EXTERN integer lastbadness  ; 
EXTERN halfword adjusttail  ; 
EXTERN integer packbeginline  ; 
EXTERN twohalves emptyfield  ; 
EXTERN fourquarters nulldelimiter  ; 
EXTERN halfword curmlist  ; 

/* EXTERN smallnumber curstyle  ; */
/* EXTERN integer curstyle  ; */  /* padded out */	/* tex5.c, tex7.c */
EXTERN int curstyle  ;  /* padded out */			/* 95/Jan/7 */

/* EXTERN smallnumber cursize  ; */
/* EXTERN integer cursize  ;  */ /* padded out */
EXTERN int cursize  ;  /* padded out */				/* 95/Jan/7 */

EXTERN scaled curmu  ; 
EXTERN booleane mlistpenalties  ; 
EXTERN internalfontnumber curf  ; 

/* EXTERN quarterword curc  ; */
/* EXTERN integer curc  ; */  /* padded out */
EXTERN int curc  ;  /* padded out */			/* 95/Jan/7 */

EXTERN ffourquarters curi  ; 

EXTERN integer magicoffset  ; 
EXTERN halfword curalign  ; 
EXTERN halfword curspan  ; 
EXTERN halfword curloop  ; 
EXTERN halfword alignptr  ; 
EXTERN halfword curhead, curtail  ; 
EXTERN halfword justbox  ; 
EXTERN halfword passive  ; 
EXTERN halfword printednode  ; 
EXTERN halfword passnumber  ; 
/* EXTERN scaled activewidth[7]  ; */
EXTERN scaled activewidth[8]  ; 
/* EXTERN scaled curactivewidth[7]  ; */
EXTERN scaled curactivewidth[8]  ; 
/* EXTERN scaled background[7]  ; */
EXTERN scaled background[8]  ; 
/* EXTERN scaled breakwidth[7]  ; */
EXTERN scaled breakwidth[8]  ; 
EXTERN booleane noshrinkerroryet  ; 
EXTERN halfword curp  ; 
EXTERN booleane secondpass  ; 
EXTERN booleane finalpass  ; 
EXTERN integer threshold  ; 
EXTERN integer minimaldemerits[4]  ; 
EXTERN integer minimumdemerits  ; 
EXTERN halfword bestplace[4]  ; 
EXTERN halfword bestplline[4]  ; 
EXTERN scaled discwidth  ; 
EXTERN halfword easyline  ; 
EXTERN halfword lastspecialline  ; 
EXTERN scaled firstwidth  ; 
EXTERN scaled secondwidth  ; 
EXTERN scaled firstindent  ; 
EXTERN scaled secondindent  ; 
EXTERN halfword bestbet  ; 
EXTERN integer fewestdemerits  ; 
EXTERN halfword bestline  ; 
EXTERN integer actuallooseness  ; 
EXTERN integer linediff  ; 
/* EXTERN short hc[64+2]  ; */	/* padded OK 66 * 2 = 132 which is divisible by 4 */
EXTERN int hc[66]  ;	/* padded OK 66 * 2 = 132 which is divisible by 4 */

/* EXTERN smallnumber hn  ; */
/* EXTERN integer hn  ; */  /* padded out */
EXTERN int hn  ;  /* padded out */			/* 95/Jan/7 */

EXTERN halfword ha, hb  ; 

/* EXTERN internalfontnumber hf  ;  */
EXTERN int hf  ;							/* 95/Jan/7 */

/* EXTERN short hu[64+2]  ; */		/* padded OK */
EXTERN int hu[66]  ;		/* padded OK */ 

/* EXTERN integer hyfchar  ;  */
EXTERN int hyfchar  ;						/* 95/Jan/7 */

/* initcurlang new in 3.14159 */
/* EXTERN ASCIIcode curlang, initcurlang  ; */
/* EXTERN integer curlang, initcurlang  ; */ /* padded out */
EXTERN int curlang, initcurlang  ; /* padded out */		/* 95/Jan/7 */

EXTERN integer lhyf, rhyf  ;
/* EXTERN initlhyf, initrhyf  ;	*/ /* new in 3.14159 */
EXTERN integer initlhyf, initrhyf  ;	/* new in 3.14159 */

EXTERN halfword hyfbchar  ; 
/* EXTERN char hyf[65]  ;  */
EXTERN char hyf[68]  ; /* padded out */
EXTERN halfword initlist  ; 
EXTERN booleane initlig  ; 
EXTERN booleane initlft  ; 

/* EXTERN smallnumber hyphenpassed  ; */
/* EXTERN integer hyphenpassed  ; */  /* padded out */
EXTERN int hyphenpassed  ;  /* padded out */			/* 95/Jan/7 */

/* EXTERN halfword curl, curr  ; */		/* make int's ? 95/Jan/7 */
EXTERN int curl, curr  ;		/* make int's ? 95/Jan/7 */

EXTERN halfword curq  ; 
EXTERN halfword ligstack  ; 
EXTERN booleane ligaturepresent  ; 
EXTERN booleane lfthit, rthit  ; 

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* could perhaps use packedASCIIcode for trietrc ? */
#ifdef ALLOCATETRIES
EXTERN halfword *trietrl  ; 
EXTERN halfword *trietro  ; 
EXTERN quarterword *trietrc  ; 
#else
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
EXTERN halfword trietrl[triesize + 1]  ; 
EXTERN halfword trietro[triesize + 1]  ; 
EXTERN quarterword trietrc[triesize + 1]  ; 
#endif
EXTERN smallnumber hyfdistance[trieopsize + 1]  ; /* already padded 751 + 1 */
EXTERN smallnumber hyfnum[trieopsize + 1]  ;	  /* already padded 751 + 1 */
EXTERN trieopcode hyfnext[trieopsize + 1]  ; 
EXTERN integer opstart[256]  ; 

/* if ALLOCATEHYPHEN is true, then hyphen_prime is a variable */
/* otherwise it is a pre-processor defined constant */
#ifdef ALLOCATEHYPHEN
#define defaulthyphenprime 1009
EXTERN strnumber *hyphword  ;
EXTERN halfword *hyphlist  ; 
/* EXTERN hyphen_prime ; */
EXTERN integer hyphen_prime ;
#else
#define hyphen_prime 607
/* EXTERN strnumber hyphword[608]  ;  */
EXTERN strnumber hyphword[hyphen_prime + 1]  ; 
/* EXTERN halfword hyphlist[608]  ;  */
EXTERN halfword hyphlist[hyphen_prime + 1]  ; 
#endif

/* EXTERN hyphpointer hyphcount  ;  */
/* EXTERN integer hyphcount  ; */  /* padded out */
EXTERN int hyphcount  ;  /* padded out */		/* 95/Jan/7 */

#ifdef INITEX
EXTERN integer 
#define trieophash (zzzaf - (int)(negtrieopsize))
  zzzaf[trieopsize - negtrieopsize + 1]  ; 
EXTERN trieopcode trieused[256]  ; 
EXTERN ASCIIcode trieoplang[trieopsize + 1]  ;   /* already padded 751 + 1 */
EXTERN trieopcode trieopval[trieopsize + 1]  ; 
EXTERN integer trieopptr  ; 
#endif /* INITEX */

EXTERN trieopcode maxopused  ; 
EXTERN booleane smallop  ; 

#ifdef INITEX
#ifdef ALLOCATEINI
EXTERN packedASCIIcode *triec  ;  
EXTERN trieopcode *trieo  ; 
EXTERN triepointer *triel  ; 
EXTERN triepointer *trier  ; 
EXTERN triepointer *triehash  ; 
#else /* end ALLOCATEINI */
EXTERN packedASCIIcode triec[triesize + 1]  ; 
EXTERN trieopcode trieo[triesize + 1]  ; 
EXTERN triepointer triel[triesize + 1]  ; 
EXTERN triepointer trier[triesize + 1]  ; 
EXTERN triepointer triehash[triesize + 1]  ; 
#endif /* end not ALLOCATEINI */
EXTERN triepointer trieptr  ; 
#endif /* INITEX */

#ifdef INITEX
#ifdef ALLOCATEINI
/* EXTERN booleane *trietaken  ; */	/* save (4 - 1) * triesize = 90,000 byte */
EXTERN char *trietaken  ; 
#else
EXTERN booleane trietaken[triesize + 1]  ; 
#endif

EXTERN triepointer triemin[256]  ; 
EXTERN triepointer triemax  ; 
EXTERN booleane trienotready  ; 
#endif /* INITEX */
EXTERN scaled bestheightplusdepth  ; 
EXTERN halfword pagetail  ; 

/* EXTERN char pagecontents  ; */
/* EXTERN integer pagecontents  ; */ /* padded out */
EXTERN int pagecontents  ; /* padded out */			/* 95/Jan/7 */

#define cstokenflag 4095		/* if we should want to use this ... */

/* ********************************************************************* */

/* do *some* sanity checking here --- rather than in TeX later 96/Jan/18 */
/* (cannot catch everything here, since some is now dynamic) */

#if (halferrorline < 30) || (halferrorline > errorline - 15)
#error ERROR: (halferrorline < 30) || (halferrorline > errorline - 15) BAD 1
#endif

#if (maxprintline < 60)
#error ERROR: (maxprintline < 60) BAD 2
#endif

#if (hash_prime > hash_size)
#error ERROR: (hash_prime > hash_size) BAD 5
#endif

#if (maxinopen > 127)
#error ERROR: (maxinopen > 127) BAD 6
#endif

#if (minquarterword > 0) || (maxquarterword < 127)
#error ERROR: (minquarterword > 0) || (maxquarterword < 127) BAD 11
#endif

#if (minhalfword > 0) || (maxhalfword < 32767)
#error ERROR:  (minhalfword > 0) || (maxhalfword < 32767) BAD 12
#endif

#if (minquarterword < minhalfword) || (maxquarterword > maxhalfword)
#error ERROR: (minquarterword < minhalfword) || (maxquarterword > maxhalfword) BAD 13
#endif

#if (fontmax > maxquarterword)
#error ERROR: (fontmax > maxquarterword) BAD 15
#endif

#if (savesize > maxhalfword)
#error ERROR: (savesize > maxhalfword) BAD 17
#endif

#if (bufsize > maxhalfword)
#error ERROR:  (bufsize > maxhalfword) BAD 18
#endif

#if (maxquarterword - minquarterword) < 255
#error (maxquarterword - minquarterword) < 255 BAD 19
#endif

/* ********************************************************************* */

EXTERN scaled pagemaxdepth  ; 
EXTERN halfword bestpagebreak  ; 
EXTERN integer leastpagecost  ; 
EXTERN scaled bestsize  ; 
EXTERN scaled pagesofar[8]  ; 
EXTERN halfword lastglue  ; 
EXTERN integer lastpenalty  ; 
EXTERN scaled lastkern  ; 
EXTERN integer insertpenalties  ; 
EXTERN booleane outputactive  ; 
EXTERN internalfontnumber mainf  ; 

EXTERN ffourquarters maini  ; 
EXTERN ffourquarters mainj  ; 

EXTERN fontindex maink  ; 
EXTERN halfword mainp  ; 
EXTERN integer mains  ; 
EXTERN halfword bchar  ; 
EXTERN halfword falsebchar  ; 
EXTERN booleane cancelboundary  ; 
EXTERN booleane insdisc  ; 
EXTERN halfword curbox  ; 
EXTERN halfword aftertoken  ; 
EXTERN booleane longhelpseen  ; 
EXTERN strnumber formatident  ; 
EXTERN wordfile fmtfile  ; 
EXTERN integer readyalready  ; 

EXTERN alphafile writefile[16]  ;	/* hard wired limit in TeX */
EXTERN booleane writeopen[18]  ; 

EXTERN halfword writeloc  ; 
EXTERN poolpointer editnamestart  ; 
/* EXTERN integer editnamelength, editline, tfmtemp  ;  */
EXTERN integer editnamelength, editline  ; 
/* EXTERN integer tfmtemp  ; */		/* only used in tex3.c */
EXTERN int tfmtemp  ; 				/* only used in tex3.c 95/Jan/7 */

/* new stuff defined in local.c - bkph */

#ifdef MSDOS
EXTERN booleane is_initex;
EXTERN booleane verboseflag;
EXTERN booleane traceflag;
EXTERN booleane debugflag;
EXTERN booleane heapflag;
EXTERN booleane opentraceflag;
EXTERN booleane cachefileflag;
EXTERN booleane knuthflag;
EXTERN booleane nointerrupts;
EXTERN booleane cstyleflag;
EXTERN booleane nonascii;
EXTERN booleane keyreplace;
EXTERN booleane deslash;
EXTERN booleane trimeof;
EXTERN booleane allowpatterns;
EXTERN booleane showfontsused;
EXTERN booleane resetexceptions;
EXTERN booleane showcurrent;
EXTERN booleane currentflag;
EXTERN booleane currenttfm;
EXTERN booleane returnflag;
EXTERN booleane want_version;
EXTERN booleane civilizeflag;
EXTERN booleane shownumeric;
EXTERN booleane restricttoascii;
EXTERN booleane showmissing;
EXTERN booleane fullfilenameflag;
EXTERN booleane savestringsflag;
EXTERN int meminitex;
EXTERN int memextrahigh;
EXTERN int memextralow;
EXTERN int newhyphenprime;
EXTERN int missingcharacters;
EXTERN int showinhex;
EXTERN int showindos;
EXTERN int testdiraccess;
EXTERN int dirmethod;
EXTERN int filemethod;
/* EXTERN int waitflush; */
EXTERN int showfmtflag;
EXTERN int showtfmflag;
EXTERN booleane showtexinputflag;	/* 1998/Jan/28 */
EXTERN booleane truncatelonglines;	/* 1998/Feb/2 */
EXTERN booleane showcsnames;			/* 1998/Mar/31 */
EXTERN int tabstep;
EXTERN int pseudotilde;
EXTERN int pseudospace;
EXTERN int allowquotednames;
EXTERN int defaultrule;
EXTERN char *formatfile;
EXTERN char *sourcedirect;			/* 1998/Sep/29 */
EXTERN char *stringfile;
EXTERN int shareflag;
EXTERN char *formatname;
EXTERN char *encodingname;
EXTERN booleane formatspecific;
EXTERN booleane encodingspecific;
EXTERN booleane showlinebreakstats;	/* 1996/Feb/9 */
EXTERN int firstpasscount;			/* 1996/Feb/9 */
EXTERN int secondpasscount;			/* 1996/Feb/9 */
EXTERN int finalpasscount;			/* 1996/Feb/9 */
EXTERN int underfullhbox;			/* 1996/Feb/9 */
EXTERN int overfullhbox;			/* 1996/Feb/9 */
EXTERN int underfullvbox;			/* 1996/Feb/9 */
EXTERN int overfullvbox;			/* 1996/Feb/9 */
EXTERN int paragraphfailed;			/* 1996/Feb/9 */
EXTERN int singleline;				/* 1996/Feb/15 */
EXTERN FILE *errout;
EXTERN int fontdimenzero;			/* 1998/Oct/5 */
EXTERN int ignorefrozen;			/* 1998/Oct/5 */
EXTERN booleane suppressfligs;		/* 1999/Jan/5 */
EXTERN int abortflag;			// not yet hooked up ???
EXTERN int errlevel;			// not yet hooked up ???
EXTERN int jumpused;				/* 1999/Nov/28 */
EXTERN jmp_buf jumpbuffer;			/* 1999/Nov/7 */
#endif /* DOS */

#ifdef MSDOS
extern int currentpoolsize;				/* in local.c - bkph */
extern int currentmaxstrings;			/* in local.c - bkph */
extern int currentmemsize;				/* in local.c - bkph */
extern int currentfontmemsize;			/* in local.c - bkph */
extern int currentsavesize;				/* in local.c - bkph */
extern int currentstacksize;			/* in local.c - bkph */
extern int currentnestsize;				/* in local.c - bkph */
extern int currentparamsize;			/* in local.c - bkph */
extern int currentbufsize;				/* in local.c - bkph */

extern char *texversion;				/* in local.c - bkph */
extern char *application;				/* in local.c - bkph */
extern char *yandyversion;				/* in local.c - bkph */

unsigned char wintodos[128];				/* in local.c - bkph */

extern char last_filename[PATH_MAX];		/* in ourpaths.c */

extern char logline[MAXLINE];				/* in local.c */

extern char *texpath;						/* in local.c */

memoryword *allocatemainmemory (int);		/* in local.c - bkph */
memoryword *reallocmain (int, int);			/* in local.c - bkph */
packedASCIIcode *reallocstrpool (int);		/* in local.c - bkph */
poolpointer *reallocstrstart (int);			/* in local.c - bkph */
memoryword *reallocsavestack (int);			/* in local.c - bkph */
liststaterecord *reallocneststack (int);	/* in local.c - bkph */
instaterecord *reallocinputstack (int);		/* in local.c - bkph */
halfword *reallocparamstack (int);			/* in local.c - bkph */
ASCIIcode *reallocbuffer (int);				/* in local.c - bkph */
fmemoryword *reallocfontinfo (int);			/* in local.c - bkph */

// void reallochyphen (int);				/* in local.c - bkph */
int reallochyphen (int);				/* in local.c - bkph */
// void allocatetries (int);				/* in local.c - bkph */
int allocatetries (int);				/* in local.c - bkph */

void tryandopen (char *);				/* in local.c - bkph */
void checkeqtb (char *);				/* in local.c - bkph */
void probememory (void);				/* in local.c - bkph */
// void showmaximums (FILE *);			/* in local.c - bkph */
void printcsnames (FILE *, int);		/* in local.c - bkph */
void perrormod(char *);					/* in local.c */

char *grabenv(char *);			/* in local.c - bkph */
// void showversion (FILE *);			/* in local.c - bkph */
void stampit (char *);					/* in local.c - bkph */
void stampcopy (char *);				/* in local.c - bkph */
booleane prime (int);					/* in local.c - bkph */
int endit (int);						/* in local.c - bkph */

// void uexit (int unix_code);			/* in lib/uexit.c - bkph */
int uexit (int unix_code);				/* in lib/uexit.c - bkph */
void topenin (void);					/* in lib/texmf.c - bkph */

booleane extensionirrelevantp (unsigned char *base, int nlen, char *suffix);

void calledit (ASCIIcode *filename, poolpointer fnstart,
			   integer fnlength, integer linenumber); /* from lib/texmf.c - bkph */

void addvariablespace(int);				/* in itex.c - bkph */

void get_date_and_time (integer *minutes, integer *day,
						integer *month, integer *year);		/* in lib/texmf.c - bkph */

booleane maketextex (void);				/* in openinou.c */
booleane maketextfm (void);				/* in openinou.c */

unsigned char *unixify (unsigned char *);				/* in pathsrch.c bkph */

#ifdef _WINDOWS
void showline (char *, int);			/* in local.c */
void showchar (int);					/* in local.c */
int main(int, char *[]);				/* in lib\texmf.c */
#endif

#ifdef CHECKPOOL
int checkpool (char *);					/* in itex.c - debugging */
#endif

#endif /* ifdef MSDOS */

/****************************************************************************/

#include "coerce.h"

/****************************************************************************/