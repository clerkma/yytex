/* Copyright 2007 TeX Users Group

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

/* #define USEREGISTERS */
#undef USEREGISTERS 

/* WARNING: with dynamic allocation cannot use memoryword *mem=zmem */

#ifdef USEREGISTERS
void initialize(void);
#define initialize_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void println(void);
#define println_regmem
void zprintchar(ASCIIcode);
#define printchar(s) zprintchar((ASCIIcode) (s))
#define printchar_regmem register memoryword *eqtb=zeqtb;
void zprint(integer);
#define print(s) zprint((integer) (s))
#define print_regmem register memoryword *eqtb=zeqtb;
void zslowprint(integer); 
#define slowprint(s) zslowprint((integer) (s))
#define slowprint_regmem
void zprintnl(strnumber); 
#define printnl(s) zprintnl((strnumber) (s))
#define printnl_regmem
void zprintesc(strnumber); 
#define printesc(s) zprintesc((strnumber) (s))
#define printesc_regmem register memoryword *eqtb=zeqtb;
void zprintthedigs(eightbits); 
#define printthedigs(k) zprintthedigs((eightbits) (k))
#define printthedigs_regmem
void zprintint(integer); 
#define printint(n) zprintint((integer) (n))
#define printint_regmem
void zprintcs(integer); 
#define printcs(p) zprintcs((integer) (p))
#define printcs_regmem register memoryword *eqtb=zeqtb;
void zsprintcs(halfword); 
#define sprintcs(p) zsprintcs((halfword) (p))
#define sprintcs_regmem
void zprintfilename(integer, integer, integer); 
#define printfilename(n, a, e) zprintfilename((integer) (n), (integer) (a), (integer) (e))
#define printfilename_regmem
void zprintsize(integer); 
#define printsize(s) zprintsize((integer) (s))
#define printsize_regmem
void zprintwritewhatsit(strnumber, halfword); 
#define printwritewhatsit(s, p) zprintwritewhatsit((strnumber) (s), (halfword) (p))
#define printwritewhatsit_regmem register memoryword *mem=zmem;
// void jumpout(void);
int jumpout(void); 
#define jumpout_regmem
// void error(void);
int error(void); 
#define error_regmem
// void zfatalerror(strnumber);
int zfatalerror(strnumber); 
#define fatalerror(s) zfatalerror((strnumber) (s))
#define fatalerror_regmem
// void zoverflow(strnumber, integer);
int zoverflow(strnumber, integer); 
#define overflow(s, n) zoverflow((strnumber) (s), (integer) (n))
#define overflow_regmem
// void zconfusion(strnumber);
int zconfusion(strnumber); 
#define confusion(s) zconfusion((strnumber) (s))
#define confusion_regmem
booleane initterminal(void); 
#define initterminal_regmem
strnumber makestring(void); 
#define makestring_regmem
booleane zstreqbuf(strnumber, integer); 
#define streqbuf(s, k) zstreqbuf((strnumber) (s), (integer) (k))
#define streqbuf_regmem
booleane zstreqstr(strnumber, strnumber); 
#define streqstr(s, t) zstreqstr((strnumber) (s), (strnumber) (t))
#define streqstr_regmem
booleane getstringsstarted(void); 
#define getstringsstarted_regmem
void zprinttwo(integer); 
#define printtwo(n) zprinttwo((integer) (n))
#define printtwo_regmem
void zprinthex(integer); 
#define printhex(n) zprinthex((integer) (n))
#define printhex_regmem
void zprintromanint(integer);
#define printromanint(n) zprintromanint((integer) (n))
#define printromanint_regmem
void printcurrentstring(void); 
#define printcurrentstring_regmem
// void terminput(void); 
void terminput(int, int); 
#define terminput_regmem
void zinterror(integer); 
#define interror(n) zinterror((integer) (n))
#define interror_regmem
void normalizeselector(void); 
#define normalizeselector_regmem
void pauseforinstructions(void); 
#define pauseforinstructions_regmem
integer zhalf(integer); 
#define half(x) zhalf((integer) (x))
#define half_regmem
scaled zrounddecimals(smallnumber); 
#define rounddecimals(k) zrounddecimals((smallnumber) (k))
#define rounddecimals_regmem
void zprintscaled(scaled); 
#define printscaled(s) zprintscaled((scaled) (s))
#define printscaled_regmem
scaled zmultandadd(integer, scaled, scaled, scaled); 
#define multandadd(n, x, y, maxanswer) zmultandadd((integer) (n), (scaled) (x), (scaled) (y), (scaled) (maxanswer))
#define multandadd_regmem
scaled zxovern(scaled, integer);	/* scaled zxovern(); */
#define xovern(x, n) zxovern((scaled) (x), (integer) (n))
#define xovern_regmem
scaled zxnoverd(scaled, integer, integer); 
#define xnoverd(x, n, d) zxnoverd((scaled) (x), (integer) (n), (integer) (d))
#define xnoverd_regmem
halfword zbadness(scaled, scaled); 
#define badness(t, s) zbadness((scaled) (t), (scaled) (s))
#define badness_regmem
void zprintword(memoryword); 
#define printword(w) zprintword((memoryword) (w))
#define printword_regmem
void zshowtokenlist(integer, integer, integer); 
#define showtokenlist(p, q, l) zshowtokenlist((integer) (p), (integer) (q), (integer) (l))
#define showtokenlist_regmem register memoryword *mem=zmem;
void runaway(void); 
#define runaway_regmem register memoryword *mem=zmem;
halfword getavail(void); 
#define getavail_regmem register memoryword *mem=zmem;
void zflushlist(halfword); 
#define flushlist(p) zflushlist((halfword) (p))
#define flushlist_regmem register memoryword *mem=zmem;
halfword zgetnode(integer); 
#define getnode(s) zgetnode((integer) (s))
#define getnode_regmem register memoryword *mem=zmem;
void zfreenode(halfword, halfword); 
#define freenode(p, s) zfreenode((halfword) (p), (halfword) (s))
#define freenode_regmem register memoryword *mem=zmem;
void sortavail(void); 
#define sortavail_regmem register memoryword *mem=zmem;
halfword newnullbox(void); 
#define newnullbox_regmem register memoryword *mem=zmem;
halfword newrule(void); 
#define newrule_regmem register memoryword *mem=zmem;
halfword znewligature(quarterword, quarterword, halfword); 
#define newligature(f, c, q) znewligature((quarterword) (f), (quarterword) (c), (halfword) (q))
#define newligature_regmem register memoryword *mem=zmem;
halfword znewligitem(quarterword); 
#define newligitem(c) znewligitem((quarterword) (c))
#define newligitem_regmem register memoryword *mem=zmem;
halfword newdisc(void); 
#define newdisc_regmem register memoryword *mem=zmem;
halfword znewmath(scaled, smallnumber); 
#define newmath(w, s) znewmath((scaled) (w), (smallnumber) (s))
#define newmath_regmem register memoryword *mem=zmem;
halfword znewspec(halfword); 
#define newspec(p) znewspec((halfword) (p))
#define newspec_regmem register memoryword *mem=zmem;
halfword znewparamglue(smallnumber); 
#define newparamglue(n) znewparamglue((smallnumber) (n))
#define newparamglue_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
halfword znewglue(halfword); 
#define newglue(q) znewglue((halfword) (q))
#define newglue_regmem register memoryword *mem=zmem;
halfword znewskipparam(smallnumber); 
#define newskipparam(n) znewskipparam((smallnumber) (n))
#define newskipparam_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
halfword znewkern(scaled); 
#define newkern(w) znewkern((scaled) (w))
#define newkern_regmem register memoryword *mem=zmem;
halfword znewpenalty(integer); 
#define newpenalty(m) znewpenalty((integer) (m))
#define newpenalty_regmem register memoryword *mem=zmem;
void zcheckmem(booleane); 
#define checkmem(printlocs) zcheckmem((booleane) (printlocs))
#define checkmem_regmem register memoryword *mem=zmem;
void zsearchmem(halfword); 
#define searchmem(p) zsearchmem((halfword) (p))
#define searchmem_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zshortdisplay(integer); 
#define shortdisplay(p) zshortdisplay((integer) (p))
#define shortdisplay_regmem register memoryword *mem=zmem;
void zprintfontandchar(integer); 
#define printfontandchar(p) zprintfontandchar((integer) (p))
#define printfontandchar_regmem register memoryword *mem=zmem;
void zprintmark(integer); 
#define printmark(p) zprintmark((integer) (p))
#define printmark_regmem register memoryword *mem=zmem;
void zprintruledimen(scaled); 
#define printruledimen(d) zprintruledimen((scaled) (d))
#define printruledimen_regmem
void zprintglue(scaled, integer, strnumber); 
#define printglue(d, order, s) zprintglue((scaled) (d), (integer) (order), (strnumber) (s))
#define printglue_regmem
void zprintspec(integer, strnumber); 
#define printspec(p, s) zprintspec((integer) (p), (strnumber) (s))
#define printspec_regmem register memoryword *mem=zmem;
void zprintfamandchar(halfword); 
#define printfamandchar(p) zprintfamandchar((halfword) (p))
#define printfamandchar_regmem register memoryword *mem=zmem;
void zprintdelimiter(halfword); 
#define printdelimiter(p) zprintdelimiter((halfword) (p))
#define printdelimiter_regmem register memoryword *mem=zmem;
void zprintsubsidiarydata(halfword, ASCIIcode); 
#define printsubsidiarydata(p, c) zprintsubsidiarydata((halfword) (p), (ASCIIcode) (c))
#define printsubsidiarydata_regmem register memoryword *mem=zmem;
void zprintstyle(integer); 
#define printstyle(c) zprintstyle((integer) (c))
#define printstyle_regmem
void zprintskipparam(integer); /* void zprintskipparam(); */
#define printskipparam(n) zprintskipparam((integer) (n))
#define printskipparam_regmem
void zshownodelist(integer); /* void zshownodelist(); */
#define shownodelist(p) zshownodelist((integer) (p))
#define shownodelist_regmem register memoryword *mem=zmem;
void zshowbox(halfword); /* void zshowbox(); */
#define showbox(p) zshowbox((halfword) (p))
#define showbox_regmem register memoryword *eqtb=zeqtb;
void zdeletetokenref(halfword); /* void zdeletetokenref(); */
#define deletetokenref(p) zdeletetokenref((halfword) (p))
#define deletetokenref_regmem register memoryword *mem=zmem;
void zdeleteglueref(halfword); /* void zdeleteglueref(); */
#define deleteglueref(p) zdeleteglueref((halfword) (p))
#define deleteglueref_regmem register memoryword *mem=zmem;
void zflushnodelist(halfword); /* void zflushnodelist(); */
#define flushnodelist(p) zflushnodelist((halfword) (p))
#define flushnodelist_regmem register memoryword *mem=zmem;
halfword zcopynodelist(halfword); /* halfword zcopynodelist(); */
#define copynodelist(p) zcopynodelist((halfword) (p))
#define copynodelist_regmem register memoryword *mem=zmem;
void zprintmode(integer); /* void zprintmode(); */
#define printmode(m) zprintmode((integer) (m))
#define printmode_regmem
void pushnest(void); /* void pushnest(); */
#define pushnest_regmem
void popnest(void); /* void popnest(); */
#define popnest_regmem register memoryword *mem=zmem;
void showactivities(void); /* void showactivities(); */
#define showactivities_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zprintparam(integer); /* void zprintparam(); */
#define printparam(n) zprintparam((integer) (n))
#define printparam_regmem
void begindiagnostic(void); /* void begindiagnostic(); */
#define begindiagnostic_regmem register memoryword *eqtb=zeqtb;
void zenddiagnostic(booleane); /* void zenddiagnostic(); */
#define enddiagnostic(blankline) zenddiagnostic((booleane) (blankline))
#define enddiagnostic_regmem
void zprintlengthparam(integer); /* void zprintlengthparam(); */
#define printlengthparam(n) zprintlengthparam((integer) (n))
#define printlengthparam_regmem
void zprintcmdchr(quarterword, halfword); /* void zprintcmdchr(); */
#define printcmdchr(cmd, chrcode) zprintcmdchr((quarterword) (cmd), (halfword) (chrcode))
#define printcmdchr_regmem
void zshoweqtb(halfword); /* void zshoweqtb(); */
#define showeqtb(n) zshoweqtb((halfword) (n))
#define showeqtb_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
halfword zidlookup(integer, integer); /* halfword zidlookup(); */
#define idlookup(j, l) zidlookup((integer) (j), (integer) (l))
#define idlookup_regmem
void zprimitive(strnumber, quarterword, halfword); /* void zprimitive(); */
#define primitive(s, c, o) zprimitive((strnumber) (s), (quarterword) (c), (halfword) (o))
#define primitive_regmem register memoryword *eqtb=zeqtb;
void znewsavelevel(groupcode); /* void znewsavelevel(); */
#define newsavelevel(c) znewsavelevel((groupcode) (c))
#define newsavelevel_regmem
void zeqdestroy(memoryword); /* void zeqdestroy(); */
#define eqdestroy(w) zeqdestroy((memoryword) (w))
#define eqdestroy_regmem register memoryword *mem=zmem;
void zeqsave(halfword, quarterword); /* void zeqsave(); */
#define eqsave(p, l) zeqsave((halfword) (p), (quarterword) (l))
#define eqsave_regmem register memoryword *eqtb=zeqtb;
void zeqdefine(halfword, quarterword, halfword); /* void zeqdefine(); */
#define eqdefine(p, t, e) zeqdefine((halfword) (p), (quarterword) (t), (halfword) (e))
#define eqdefine_regmem register memoryword *eqtb=zeqtb;
void zeqworddefine(halfword, integer); /* void zeqworddefine(); */
#define eqworddefine(p, w) zeqworddefine((halfword) (p), (integer) (w))
#define eqworddefine_regmem register memoryword *eqtb=zeqtb;
void zgeqdefine(halfword, quarterword, halfword); /* void zgeqdefine(); */
#define geqdefine(p, t, e) zgeqdefine((halfword) (p), (quarterword) (t), (halfword) (e))
#define geqdefine_regmem register memoryword *eqtb=zeqtb;
void zgeqworddefine(halfword, integer); /* void zgeqworddefine(); */
#define geqworddefine(p, w) zgeqworddefine((halfword) (p), (integer) (w))
#define geqworddefine_regmem register memoryword *eqtb=zeqtb;
void zsaveforafter(halfword); /* void zsaveforafter(); */
#define saveforafter(t) zsaveforafter((halfword) (t))
#define saveforafter_regmem
void zrestoretrace(halfword, strnumber); /* void zrestoretrace(); */
#define restoretrace(p, s) zrestoretrace((halfword) (p), (strnumber) (s))
#define restoretrace_regmem
void unsave(void); /* void unsave(); */
#define unsave_regmem register memoryword *eqtb=zeqtb;
void preparemag(void); /* void preparemag(); */
#define preparemag_regmem register memoryword *eqtb=zeqtb;
void ztokenshow(halfword); /* void ztokenshow(); */
#define tokenshow(p) ztokenshow((halfword) (p))
#define tokenshow_regmem register memoryword *mem=zmem;
void printmeaning(void); /* void printmeaning(); */
#define printmeaning_regmem
void showcurcmdchr(void); /* void showcurcmdchr(); */
#define showcurcmdchr_regmem
void showcontext(void); /* void showcontext(); */
#define showcontext_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zbegintokenlist(halfword, quarterword); /* void zbegintokenlist(); */
#define begintokenlist(p, t) zbegintokenlist((halfword) (p), (quarterword) (t))
#define begintokenlist_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void endtokenlist(void); /* void endtokenlist(); */
#define endtokenlist_regmem
void backinput(void); /* void backinput(); */
#define backinput_regmem register memoryword *mem=zmem;
void backerror(void); /* void backerror(); */
#define backerror_regmem
void inserror(void); /* void inserror(); */
#define inserror_regmem
void beginfilereading(void); /* void beginfilereading(); */
#define beginfilereading_regmem
void endfilereading(void); /* void endfilereading(); */
#define endfilereading_regmem
void clearforerrorprompt(void); /* void clearforerrorprompt(); */
#define clearforerrorprompt_regmem
void checkoutervalidity(void); /* void checkoutervalidity(); */
#define checkoutervalidity_regmem register memoryword *mem=zmem;
void getnext(void); /* void getnext(); */
#define getnext_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void firmuptheline(void); /* void firmuptheline(); */
#define firmuptheline_regmem register memoryword *eqtb=zeqtb;
void gettoken(void); /* void gettoken(); */
#define gettoken_regmem
void macrocall(void); /* void macrocall(); */
#define macrocall_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void insertrelax(void); /* void insertrelax(); */
#define insertrelax_regmem
void expand(void); /* void expand(); */
#define expand_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void getxtoken(void); /* void getxtoken(); */
#define getxtoken_regmem
void xtoken(void); /* void xtoken(); */
#define xtoken_regmem
void scanleftbrace(void); /* void scanleftbrace(); */
#define scanleftbrace_regmem
void scanoptionalequals(void); /* void scanoptionalequals(); */
#define scanoptionalequals_regmem
booleane zscankeyword(strnumber); /* booleane zscankeyword(); */
#define scankeyword(s) zscankeyword((strnumber) (s))
#define scankeyword_regmem register memoryword *mem=zmem;
void muerror(void); /* void muerror(); */
#define muerror_regmem
void scaneightbitint(void); /* void scaneightbitint(); */
#define scaneightbitint_regmem
void scancharnum(void); /* void scancharnum(); */
#define scancharnum_regmem
void scanfourbitint(void); /* void scanfourbitint(); */
#define scanfourbitint_regmem
void scanfifteenbitint(void); /* void scanfifteenbitint(); */
#define scanfifteenbitint_regmem
void scantwentysevenbitint(void); /* void scantwentysevenbitint(); */
#define scantwentysevenbitint_regmem
void scanfontident(void); /* void scanfontident(); */
#define scanfontident_regmem register memoryword *eqtb=zeqtb;
void zfindfontdimen(booleane); /* void zfindfontdimen(); */
#define findfontdimen(writing) zfindfontdimen((booleane) (writing))
#define findfontdimen_regmem
void zscansomethinginternal(smallnumber, booleane);
/* void zscansomethinginternal(); */
#define scansomethinginternal(level, negative) zscansomethinginternal((smallnumber) (level), (booleane) (negative))
#define scansomethinginternal_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void scanint(void); /* void scanint(); */
#define scanint_regmem
void zscandimen(booleane, booleane, booleane); /* void zscandimen(); */
#define scandimen(mu, inf, shortcut) zscandimen((booleane) (mu), (booleane) (inf), (booleane) (shortcut))
#define scandimen_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zscanglue(smallnumber); /* void zscanglue(); */
#define scanglue(level) zscanglue((smallnumber) (level))
#define scanglue_regmem register memoryword *mem=zmem;
halfword scanrulespec(void); /* halfword scanrulespec(); */
#define scanrulespec_regmem register memoryword *mem=zmem;
halfword zstrtoks(poolpointer); /* halfword zstrtoks(); */
#define strtoks(b) zstrtoks((poolpointer) (b))
#define strtoks_regmem register memoryword *mem=zmem;
halfword thetoks(void); /* halfword thetoks(); */
#define thetoks_regmem register memoryword *mem=zmem;
void insthetoks(void); /* void insthetoks(); */
#define insthetoks_regmem register memoryword *mem=zmem;
void convtoks(void); /* void convtoks(); */
#define convtoks_regmem register memoryword *mem=zmem;
halfword zscantoks(booleane, booleane); /* halfword zscantoks(); */
#define scantoks(macrodef, xpand) zscantoks((booleane) (macrodef), (booleane) (xpand))
#define scantoks_regmem register memoryword *mem=zmem;
void zreadtoks(integer, halfword); /* void zreadtoks(); */
#define readtoks(n, r) zreadtoks((integer) (n), (halfword) (r))
#define readtoks_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void passtext(void); /* void passtext(); */
#define passtext_regmem
void zchangeiflimit(smallnumber, halfword); /* void zchangeiflimit(); */
#define changeiflimit(l, p) zchangeiflimit((smallnumber) (l), (halfword) (p))
#define changeiflimit_regmem register memoryword *mem=zmem;
void conditional(void); /* void conditional(); */
#define conditional_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void beginname(void); /* void beginname(); */
#define beginname_regmem
booleane zmorename(ASCIIcode); /* booleane zmorename(); */
#define morename(c) zmorename((ASCIIcode) (c))
#define morename_regmem
void endname(void); /* void endname(); */
#define endname_regmem
void zpackfilename(strnumber, strnumber, strnumber); /* void zpackfilename(); */
#define packfilename(n, a, e) zpackfilename((strnumber) (n), (strnumber) (a), (strnumber) (e))
#define packfilename_regmem
void zpackbufferedname(smallnumber, integer, integer); /* void zpackbufferedname(); */
#define packbufferedname(n, a, b) zpackbufferedname((smallnumber) (n), (integer) (a), (integer) (b))
#define packbufferedname_regmem
strnumber makenamestring(void); /* strnumber makenamestring(); */
#define makenamestring_regmem
strnumber zamakenamestring(alphafile *); /* strnumber zamakenamestring(); */
#define amakenamestring(f) zamakenamestring((alphafile *) &(f))
#define amakenamestring_regmem
strnumber zbmakenamestring(bytefile *); /* strnumber zbmakenamestring(); */
#define bmakenamestring(f) zbmakenamestring((bytefile *) &(f))
#define bmakenamestring_regmem
strnumber zwmakenamestring(wordfile *); /* strnumber zwmakenamestring(); */
#define wmakenamestring(f) zwmakenamestring((wordfile *) &(f))
#define wmakenamestring_regmem
void scanfilename(void); /* void scanfilename(); */
#define scanfilename_regmem
void zpackjobname(strnumber); /* void zpackjobname(); */
#define packjobname(s) zpackjobname((strnumber) (s))
#define packjobname_regmem
void zpromptfilename(strnumber, strnumber); /* void zpromptfilename(); */
#define promptfilename(s, e) zpromptfilename((strnumber) (s), (strnumber) (e))
#define promptfilename_regmem
void openlogfile(void); /* void openlogfile(); */
#define openlogfile_regmem register memoryword *eqtb=zeqtb;
void startinput(void); /* void startinput(); */
#define startinput_regmem register memoryword *eqtb=zeqtb;
internalfontnumber zreadfontinfo(halfword, strnumber, strnumber, scaled);
/* internalfontnumber zreadfontinfo(); */
#define readfontinfo(u, nom, aire, s) zreadfontinfo((halfword) (u), (strnumber) (nom), (strnumber) (aire), (scaled) (s))
#define readfontinfo_regmem register memoryword *eqtb=zeqtb;
void zcharwarning(internalfontnumber, eightbits); /* void zcharwarning(); */
#define charwarning(f, c) zcharwarning((internalfontnumber) (f), (eightbits) (c))
#define charwarning_regmem register memoryword *eqtb=zeqtb;
halfword znewcharacter(internalfontnumber, eightbits); /* halfword znewcharacter(); */
#define newcharacter(f, c) znewcharacter((internalfontnumber) (f), (eightbits) (c))
#define newcharacter_regmem register memoryword *mem=zmem;
void dviswap(void); /* void dviswap(); */
#define dviswap_regmem
void zdvifour(integer); /* void zdvifour(); */
#define dvifour(x) zdvifour((integer) (x))
#define dvifour_regmem
void zdvipop(integer); /* void zdvipop(); */
#define dvipop(l) zdvipop((integer) (l))
#define dvipop_regmem
void zdvifontdef(internalfontnumber); /* void zdvifontdef(); */
#define dvifontdef(f) zdvifontdef((internalfontnumber) (f))
#define dvifontdef_regmem
void zmovement(scaled, eightbits); /* void zmovement(); */
#define movement(w, o) zmovement((scaled) (w), (eightbits) (o))
#define movement_regmem register memoryword *mem=zmem; 
void zprunemovements(integer); /* void zprunemovements(); */
#define prunemovements(l) zprunemovements((integer) (l))
#define prunemovements_regmem register memoryword *mem=zmem;
void zspecialout(halfword); /* void zspecialout(); */
#define specialout(p) zspecialout((halfword) (p))
#define specialout_regmem register memoryword *mem=zmem;
void zwriteout(halfword); /* void zwriteout(); */
#define writeout(p) zwriteout((halfword) (p))
#define writeout_regmem register memoryword *mem=zmem;
void zoutwhat(halfword); /* void zoutwhat(); */
#define outwhat(p) zoutwhat((halfword) (p))
#define outwhat_regmem register memoryword *mem=zmem;
void hlistout(void); /* void hlistout(); */
#define hlistout_regmem register memoryword *mem=zmem;
void vlistout(void); /* void vlistout(); */
#define vlistout_regmem register memoryword *mem=zmem;
void zshipout(halfword); /* void zshipout(); */
#define shipout(p) zshipout((halfword) (p))
#define shipout_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zscanspec(groupcode, booleane); /* void zscanspec(); */
#define scanspec(c, threecodes) zscanspec((groupcode) (c), (booleane) (threecodes))
#define scanspec_regmem
halfword zhpack(halfword, scaled, smallnumber); /* halfword zhpack(); */
#define hpack(p, w, m) zhpack((halfword) (p), (scaled) (w), (smallnumber) (m))
#define hpack_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
halfword zvpackage(halfword, scaled, smallnumber, scaled); /* halfword zvpackage(); */
#define vpackage(p, h, m, l) zvpackage((halfword) (p), (scaled) (h), (smallnumber) (m), (scaled) (l))
#define vpackage_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zappendtovlist(halfword); /* void zappendtovlist(); */
#define appendtovlist(b) zappendtovlist((halfword) (b))
#define appendtovlist_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
halfword newnoad(void); /* halfword newnoad(); */
#define newnoad_regmem register memoryword *mem=zmem;
halfword znewstyle(smallnumber); /* halfword znewstyle(); */
#define newstyle(s) znewstyle((smallnumber) (s))
#define newstyle_regmem register memoryword *mem=zmem;
halfword newchoice(void); /* halfword newchoice(); */
#define newchoice_regmem register memoryword *mem=zmem;
void showinfo(void); /* void showinfo(); */
#define showinfo_regmem register memoryword *mem=zmem;
halfword zfractionrule(scaled); /* halfword zfractionrule(); */
#define fractionrule(t) zfractionrule((scaled) (t))
#define fractionrule_regmem register memoryword *mem=zmem;
halfword zoverbar(halfword, scaled, scaled); /* halfword zoverbar(); */
#define overbar(b, k, t) zoverbar((halfword) (b), (scaled) (k), (scaled) (t))
#define overbar_regmem register memoryword *mem=zmem;
halfword zcharbox(internalfontnumber, quarterword); /* halfword zcharbox(); */
#define charbox(f, c) zcharbox((internalfontnumber) (f), (quarterword) (c))
#define charbox_regmem register memoryword *mem=zmem;
void zstackintobox(halfword, internalfontnumber, quarterword);
/* void zstackintobox(); */
#define stackintobox(b, f, c) zstackintobox((halfword) (b), (internalfontnumber) (f), (quarterword) (c))
#define stackintobox_regmem register memoryword *mem=zmem;

scaled zheightplusdepth(internalfontnumber, fquarterword);
#define heightplusdepth(f, c) zheightplusdepth((internalfontnumber) (f), (fquarterword) (c))

#define heightplusdepth_regmem
halfword zvardelimiter(halfword, smallnumber, scaled); /* halfword zvardelimiter(); */
#define vardelimiter(d, s, v) zvardelimiter((halfword) (d), (smallnumber) (s), (scaled) (v))
#define vardelimiter_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
halfword zrebox(halfword, scaled); /* halfword zrebox(); */
#define rebox(b, w) zrebox((halfword) (b), (scaled) (w))
#define rebox_regmem register memoryword *mem=zmem;
halfword zmathglue(halfword, scaled); /* halfword zmathglue(); */
#define mathglue(g, m) zmathglue((halfword) (g), (scaled) (m))
#define mathglue_regmem register memoryword *mem=zmem;
void zmathkern(halfword, scaled); /* void zmathkern(); */
#define mathkern(p, m) zmathkern((halfword) (p), (scaled) (m))
#define mathkern_regmem register memoryword *mem=zmem;
void flushmath(void); /* void flushmath(); */
#define flushmath_regmem register memoryword *mem=zmem;
halfword zcleanbox(halfword, smallnumber); /* halfword zcleanbox(); */
#define cleanbox(p, s) zcleanbox((halfword) (p), (smallnumber) (s))
#define cleanbox_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zfetch(halfword); /* void zfetch(); */
#define fetch(a) zfetch((halfword) (a))
#define fetch_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zmakeover(halfword); /* void zmakeover(); */
#define makeover(q) zmakeover((halfword) (q))
#define makeover_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zmakeunder(halfword); /* void zmakeunder(); */
#define makeunder(q) zmakeunder((halfword) (q))
#define makeunder_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zmakevcenter(halfword); /* void zmakevcenter(); */
#define makevcenter(q) zmakevcenter((halfword) (q))
#define makevcenter_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zmakeradical(halfword); /* void zmakeradical(); */
#define makeradical(q) zmakeradical((halfword) (q))
#define makeradical_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zmakemathaccent(halfword); /* void zmakemathaccent(); */
#define makemathaccent(q) zmakemathaccent((halfword) (q))
#define makemathaccent_regmem register memoryword *mem=zmem;
void zmakefraction(halfword); /* void zmakefraction(); */
#define makefraction(q) zmakefraction((halfword) (q))
#define makefraction_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
scaled zmakeop(halfword); /* scaled zmakeop(); */
#define makeop(q) zmakeop((halfword) (q))
#define makeop_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zmakeord(halfword); /* void zmakeord(); */
#define makeord(q) zmakeord((halfword) (q))
#define makeord_regmem register memoryword *mem=zmem;
void zmakescripts(halfword, scaled); /* void zmakescripts(); */
#define makescripts(q, delta) zmakescripts((halfword) (q), (scaled) (delta))
#define makescripts_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
smallnumber zmakeleftright(halfword, smallnumber, scaled, scaled);
/* smallnumber zmakeleftright(); */
#define makeleftright(q, style, maxd, maxh) zmakeleftright((halfword) (q), (smallnumber) (style), (scaled) (maxd), (scaled) (maxh))
#define makeleftright_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void mlisttohlist(void); /* void mlisttohlist(); */
#define mlisttohlist_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void pushalignment(void); /* void pushalignment(); */
#define pushalignment_regmem register memoryword *mem=zmem;
void popalignment(void); /* void popalignment(); */
#define popalignment_regmem register memoryword *mem=zmem;
void getpreambletoken(void); /* void getpreambletoken(); */
#define getpreambletoken_regmem register memoryword *eqtb=zeqtb;
void initalign(void); /* void initalign(); */
#define initalign_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zinitspan(halfword); /* void zinitspan(); */
#define initspan(p) zinitspan((halfword) (p))
#define initspan_regmem
void initrow(void); /* void initrow(); */
#define initrow_regmem register memoryword *mem=zmem;
void initcol(void); /* void initcol(); */
#define initcol_regmem register memoryword *mem=zmem;
booleane fincol(void); /* booleane fincol(); */
#define fincol_regmem register memoryword *mem=zmem;
void finrow(void); /* void finrow(); */
#define finrow_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void finalign(void); /* void finalign(); */
#define finalign_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void alignpeek(void); /* void alignpeek(); */
#define alignpeek_regmem
halfword zfiniteshrink(halfword); /* halfword zfiniteshrink(); */
#define finiteshrink(p) zfiniteshrink((halfword) (p))
#define finiteshrink_regmem register memoryword *mem=zmem;
void ztrybreak(integer, smallnumber); /* void ztrybreak(); */
#define trybreak(pi, breaktype) ztrybreak((integer) (pi), (smallnumber) (breaktype))
#define trybreak_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zpostlinebreak(integer); /* void zpostlinebreak(); */
#define postlinebreak(finalwidowpenalty) zpostlinebreak((integer) (finalwidowpenalty))
#define postlinebreak_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
smallnumber zreconstitute(smallnumber, smallnumber, halfword, halfword); 
/* smallnumber zreconstitute(); */
#define reconstitute(j, n, bchar, hchar) zreconstitute((smallnumber) (j), (smallnumber) (n), (halfword) (bchar), (halfword) (hchar))
#define reconstitute_regmem register memoryword *mem=zmem;
void hyphenate(void); /* void hyphenate(); */
#define hyphenate_regmem register memoryword *mem=zmem;
trieopcode znewtrieop(smallnumber, smallnumber, trieopcode);
/* trieopcode znewtrieop(); */
#define newtrieop(d, n, v) znewtrieop((smallnumber) (d), (smallnumber) (n), (trieopcode) (v))
#define newtrieop_regmem
triepointer ztrienode(triepointer); /* triepointer ztrienode(); */
#define trienode(p) ztrienode((triepointer) (p))
#define trienode_regmem
triepointer zcompresstrie(triepointer); /* triepointer zcompresstrie(); */
#define compresstrie(p) zcompresstrie((triepointer) (p))
#define compresstrie_regmem
void zfirstfit(triepointer); /* void zfirstfit(); */
#define firstfit(p) zfirstfit((triepointer) (p))
#define firstfit_regmem
void ztriepack(triepointer); /* void ztriepack(); */
#define triepack(p) ztriepack((triepointer) (p))
#define triepack_regmem
void ztriefix(triepointer); /* void ztriefix(); */
#define triefix(p) ztriefix((triepointer) (p))
#define triefix_regmem
void newpatterns(void); /* void newpatterns(); */
#define newpatterns_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void inittrie(void); /* void inittrie(); */
#define inittrie_regmem
void zlinebreak(integer); /* void zlinebreak(); */
#define linebreak(finalwidowpenalty) zlinebreak((integer) (finalwidowpenalty))
#define linebreak_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void newhyphexceptions(void); /* void newhyphexceptions(); */
#define newhyphexceptions_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
halfword zprunepagetop(halfword); /* halfword zprunepagetop(); */
#define prunepagetop(p) zprunepagetop((halfword) (p))
#define prunepagetop_regmem register memoryword *mem=zmem;
halfword zvertbreak(halfword, scaled, scaled); /* halfword zvertbreak(); */
#define vertbreak(p, h, d) zvertbreak((halfword) (p), (scaled) (h), (scaled) (d))
#define vertbreak_regmem register memoryword *mem=zmem;
halfword zvsplit(eightbits, scaled); /* halfword zvsplit(); */
#define vsplit(n, h) zvsplit((eightbits) (n), (scaled) (h))
#define vsplit_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void printtotals(void); /* void printtotals(); */
#define printtotals_regmem
void zfreezepagespecs(smallnumber); /* void zfreezepagespecs(); */
#define freezepagespecs(s) zfreezepagespecs((smallnumber) (s))
#define freezepagespecs_regmem register memoryword *eqtb=zeqtb;
void zboxerror(eightbits); /* void zboxerror(); */
#define boxerror(n) zboxerror((eightbits) (n))
#define boxerror_regmem register memoryword *eqtb=zeqtb;
void zensurevbox(eightbits); /* void zensurevbox(); */
#define ensurevbox(n) zensurevbox((eightbits) (n))
#define ensurevbox_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zfireup(halfword); /* void zfireup(); */
#define fireup(c) zfireup((halfword) (c))
#define fireup_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void buildpage(void); /* void buildpage(); */
#define buildpage_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void appspace(void); /* void appspace(); */
#define appspace_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void insertdollarsign(void); /* void insertdollarsign(); */
#define insertdollarsign_regmem
void youcant(void); /* void youcant(); */
#define youcant_regmem
void reportillegalcase(void); /* void reportillegalcase(); */
#define reportillegalcase_regmem
booleane privileged(void); /* booleane privileged(); */
#define privileged_regmem
booleane itsallover(void); /* booleane itsallover(); */
#define itsallover_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void appendglue(void); /* void appendglue(); */
#define appendglue_regmem register memoryword *mem=zmem;
void appendkern(void); /* void appendkern(); */
#define appendkern_regmem register memoryword *mem=zmem;
void offsave(void); /* void offsave(); */
#define offsave_regmem register memoryword *mem=zmem;
void extrarightbrace(void); /* void extrarightbrace(); */
#define extrarightbrace_regmem
void normalparagraph(void); /* void normalparagraph(); */
#define normalparagraph_regmem register memoryword *eqtb=zeqtb;
void zboxend(integer); /* void zboxend(); */
#define boxend(boxcontext) zboxend((integer) (boxcontext))
#define boxend_regmem register memoryword *mem=zmem;
void zbeginbox(integer); /* void zbeginbox(); */
#define beginbox(boxcontext) zbeginbox((integer) (boxcontext))
#define beginbox_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zscanbox(integer); /* void zscanbox(); */
#define scanbox(boxcontext) zscanbox((integer) (boxcontext))
#define scanbox_regmem
void zpackage(smallnumber); /* void zpackage(); */
#define package(c) zpackage((smallnumber) (c))
#define package_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
smallnumber znormmin(integer); /* smallnumber znormmin(); */
#define normmin(h) znormmin((integer) (h))
#define normmin_regmem
void znewgraf(booleane); /* void znewgraf(); */
#define newgraf(indented) znewgraf((booleane) (indented))
#define newgraf_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void indentinhmode(void); /* void indentinhmode(); */
#define indentinhmode_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void headforvmode(void); /* void headforvmode(); */
#define headforvmode_regmem
void endgraf(void); /* void endgraf(); */
#define endgraf_regmem register memoryword *eqtb=zeqtb;
void begininsertoradjust(void); /* void begininsertoradjust(); */
#define begininsertoradjust_regmem
void makemark(void); /* void makemark(); */
#define makemark_regmem register memoryword *mem=zmem;
void appendpenalty(void); /* void appendpenalty(); */
#define appendpenalty_regmem register memoryword *mem=zmem;
void deletelast(void); /* void deletelast(); */
#define deletelast_regmem register memoryword *mem=zmem;
void unpackage(void); /* void unpackage(); */
#define unpackage_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void appenditaliccorrection(void); /* void appenditaliccorrection(); */
#define appenditaliccorrection_regmem register memoryword *mem=zmem;
void appenddiscretionary(void); /* void appenddiscretionary(); */
#define appenddiscretionary_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void builddiscretionary(void); /* void builddiscretionary(); */
#define builddiscretionary_regmem register memoryword *mem=zmem;
void makeaccent(void); /* void makeaccent(); */
#define makeaccent_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void alignerror(void); /* void alignerror(); */
#define alignerror_regmem
void noalignerror(void); /* void noalignerror(); */
#define noalignerror_regmem
void omiterror(void); /* void omiterror(); */
#define omiterror_regmem
void doendv(void); /* void doendv(); */
#define doendv_regmem
void cserror(void); /* void cserror(); */
#define cserror_regmem
void zpushmath(groupcode); /* void zpushmath(); */
#define pushmath(c) zpushmath((groupcode) (c))
#define pushmath_regmem
void initmath(void); /* void initmath(); */
#define initmath_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void starteqno(void); /* void starteqno(); */
#define starteqno_regmem register memoryword *eqtb=zeqtb;
void zscanmath(halfword); /* void zscanmath(); */
#define scanmath(p) zscanmath((halfword) (p))
#define scanmath_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void zsetmathchar(integer); /* void zsetmathchar(); */
#define setmathchar(c) zsetmathchar((integer) (c))
#define setmathchar_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void mathlimitswitch(void); /* void mathlimitswitch(); */
#define mathlimitswitch_regmem register memoryword *mem=zmem;
void zscandelimiter(halfword, booleane); /* void zscandelimiter(); */
#define scandelimiter(p, r) zscandelimiter((halfword) (p), (booleane) (r))
#define scandelimiter_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void mathradical(void); /* void mathradical(); */
#define mathradical_regmem register memoryword *mem=zmem;
void mathac(void); /* void mathac(); */
#define mathac_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void appendchoices(void); /* void appendchoices(); */
#define appendchoices_regmem register memoryword *mem=zmem;
halfword zfinmlist(halfword); /* halfword zfinmlist(); */
#define finmlist(p) zfinmlist((halfword) (p))
#define finmlist_regmem register memoryword *mem=zmem;
void buildchoices(void); /* void buildchoices(); */
#define buildchoices_regmem register memoryword *mem=zmem;
void subsup(void); /* void subsup(); */
#define subsup_regmem register memoryword *mem=zmem;
void mathfraction(void); /* void mathfraction(); */
#define mathfraction_regmem register memoryword *mem=zmem;
void mathleftright(void); /* void mathleftright(); */
#define mathleftright_regmem register memoryword *mem=zmem;
void aftermath(void); /* void aftermath(); */
#define aftermath_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void resumeafterdisplay(void); /* void resumeafterdisplay(); */
#define resumeafterdisplay_regmem
void getrtoken(void); /* void getrtoken(); */
#define getrtoken_regmem
void trapzeroglue(void); /* void trapzeroglue(); */
#define trapzeroglue_regmem register memoryword *mem=zmem;
void zdoregistercommand(smallnumber); /* void zdoregistercommand(); */
#define doregistercommand(a) zdoregistercommand((smallnumber) (a))
#define doregistercommand_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void alteraux(void); /* void alteraux(); */
#define alteraux_regmem
void alterprevgraf(void); /* void alterprevgraf(); */
#define alterprevgraf_regmem
void alterpagesofar(void); /* void alterpagesofar(); */
#define alterpagesofar_regmem
void alterinteger(void); /* void alterinteger(); */
#define alterinteger_regmem
void alterboxdimen(void); /* void alterboxdimen(); */
#define alterboxdimen_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void znewfont(smallnumber); /* void znewfont(); */
#define newfont(a) znewfont((smallnumber) (a))
#define newfont_regmem register memoryword *eqtb=zeqtb;
void newinteraction(void); /* void newinteraction(); */
#define newinteraction_regmem
void prefixedcommand(void); /* void prefixedcommand(); */
#define prefixedcommand_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void doassignments(void); /* void doassignments(); */
#define doassignments_regmem
void openorclosein(void); /* void openorclosein(); */
#define openorclosein_regmem
void issuemessage(void); /* void issuemessage(); */
#define issuemessage_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void shiftcase(void); /* void shiftcase(); */
#define shiftcase_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void showwhatever(void); /* void showwhatever(); */
#define showwhatever_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
// void storefmtfile(void);
int storefmtfile(void);
#define storefmtfile_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void znewwhatsit(smallnumber, smallnumber); /* void znewwhatsit(); */
#define newwhatsit(s, w) znewwhatsit((smallnumber) (s), (smallnumber) (w))
#define newwhatsit_regmem register memoryword *mem=zmem;
void znewwritewhatsit(smallnumber); /* void znewwritewhatsit(); */
#define newwritewhatsit(w) znewwritewhatsit((smallnumber) (w))
#define newwritewhatsit_regmem register memoryword *mem=zmem;
void doextension(void); /* void doextension(); */
#define doextension_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void fixlanguage(void); /* void fixlanguage(); */
#define fixlanguage_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void handlerightbrace(void); /* void handlerightbrace(); */
#define handlerightbrace_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
// void maincontrol(void); /* void maincontrol(); */
int maincontrol(void); /* void maincontrol(); */
#define maincontrol_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void giveerrhelp(void); /* void giveerrhelp(); */
#define giveerrhelp_regmem register memoryword *eqtb=zeqtb;
booleane openfmtfile(void); /* booleane openfmtfile(); */
#define openfmtfile_regmem
booleane loadfmtfile(void); /* booleane loadfmtfile(); */
#define loadfmtfile_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
void closefilesandterminate(void); /* void closefilesandterminate(); */
#define closefilesandterminate_regmem register memoryword *eqtb=zeqtb;
// void finalcleanup(void); /* void finalcleanup(); */
int finalcleanup(void); /* void finalcleanup(); */
#define finalcleanup_regmem register memoryword *mem=zmem;
void initprim(void); /* void initprim(); */
#define initprim_regmem register memoryword *eqtb=zeqtb;
void debughelp(void);
#define debughelp_regmem register memoryword *mem=zmem, *eqtb=zeqtb;
/* void texbody(void); */
int texbody(void);					/* 1993/Dec/16 bkph */
#define texbody_regmem register memoryword *eqtb=zeqtb;

#else /* end of ifdef USEREGISTERS */

/* in this case, just let mem be alias for zmem */
/* BUT: allow continued use of register for eqtb at least ! */
#define mem zmem
#ifndef ALLOCATEZEQTB
/* if eqtb is NOT allocated, can just let eqtb be an alias for zeqtb */
#define eqtb zeqtb				/* EXPERIMENT 1996/JAN/18 */
#else
/* uncomment `register memoryword *eqtb=zeqtb' */
/* but with present compiler its faster to not do it this way 99/Jan/18 */
#endif

#ifndef ALLOCATEDVIBUF
/* if dvibuf is NOT allocated, can just let dvibuf be an alias for zdvibuf */
#define dvibuf zdvibuf			/* EXPERIMENT 1996/JAN/18 */
#else
/* uncomment `register memoryword *dvibuf=zdvibuf' */
/* but with present compiler its faster to not do it this way 99/Jan/18 */
#endif

/* this basically removes all use for these stupid #define foo_regmem s */

void initialize(void); /* void initialize(); */
#define initialize_regmem  /* register memoryword *eqtb=zeqtb; */
void println(void); /* void println(); */
#define println_regmem
void zprintchar(ASCIIcode); /* void zprintchar(); */
#define printchar(s) zprintchar((ASCIIcode) (s))
#define printchar_regmem  /* register memoryword *eqtb=zeqtb; */
void zprint(integer); /* void zprint(); */
#define print(s) zprint((integer) (s))
#define print_regmem  /* register memoryword *eqtb=zeqtb; */
void zslowprint(integer); /* void zslowprint(); */
#define slowprint(s) zslowprint((integer) (s))
#define slowprint_regmem
void zprintnl(strnumber); /* void zprintnl(); */
#define printnl(s) zprintnl((strnumber) (s))
#define printnl_regmem
void zprintesc(strnumber); /* void zprintesc(); */
#define printesc(s) zprintesc((strnumber) (s))
#define printesc_regmem  /* register memoryword *eqtb=zeqtb; */
void zprintthedigs(eightbits); /* void zprintthedigs(); */
#define printthedigs(k) zprintthedigs((eightbits) (k))
#define printthedigs_regmem
void zprintint(integer); /* void zprintint(); */
#define printint(n) zprintint((integer) (n))
#define printint_regmem
void zprintcs(integer); /* void zprintcs(); */
#define printcs(p) zprintcs((integer) (p))
#define printcs_regmem  /* register memoryword *eqtb=zeqtb; */
void zsprintcs(halfword); /* void zsprintcs(); */
#define sprintcs(p) zsprintcs((halfword) (p))
#define sprintcs_regmem
void zprintfilename(integer, integer, integer); /* void zprintfilename(); */
#define printfilename(n, a, e) zprintfilename((integer) (n), (integer) (a), (integer) (e))
#define printfilename_regmem
void zprintsize(integer); /* void zprintsize(); */
#define printsize(s) zprintsize((integer) (s))
#define printsize_regmem
void zprintwritewhatsit(strnumber, halfword); /* void zprintwritewhatsit(); */
#define printwritewhatsit(s, p) zprintwritewhatsit((strnumber) (s), (halfword) (p))
#define printwritewhatsit_regmem 
void jumpout(void); /* void jumpout(); */
// int jumpout(void); /* void jumpout(); */
#define jumpout_regmem
void error(void); /* void error(); */
// int  error(void); /* void error(); */
#define error_regmem
void zfatalerror(strnumber); /* void zfatalerror(); */
// int zfatalerror(strnumber); /* void zfatalerror(); */
#define fatalerror(s) zfatalerror((strnumber) (s))
#define fatalerror_regmem
void zoverflow(strnumber, integer); /* void zoverflow(); */
// int zoverflow(strnumber, integer); /* void zoverflow(); */
#define overflow(s, n) zoverflow((strnumber) (s), (integer) (n))
#define overflow_regmem
void zconfusion(strnumber); /* void zconfusion(); */
// int zconfusion(strnumber); /* void zconfusion(); */
#define confusion(s) zconfusion((strnumber) (s))
#define confusion_regmem
booleane initterminal(void); /* booleane initterminal(); */
#define initterminal_regmem
strnumber makestring(void); /* strnumber makestring(); */
#define makestring_regmem
booleane zstreqbuf(strnumber, integer); /* booleane zstreqbuf(); */
#define streqbuf(s, k) zstreqbuf((strnumber) (s), (integer) (k))
#define streqbuf_regmem
booleane zstreqstr(strnumber, strnumber); /* booleane zstreqstr(); */
#define streqstr(s, t) zstreqstr((strnumber) (s), (strnumber) (t))
#define streqstr_regmem
booleane getstringsstarted(void); /* booleane getstringsstarted(); */
#define getstringsstarted_regmem
void zprinttwo(integer); /* void zprinttwo(); */
#define printtwo(n) zprinttwo((integer) (n))
#define printtwo_regmem
void zprinthex(integer); /* void zprinthex(); */
#define printhex(n) zprinthex((integer) (n))
#define printhex_regmem
void zprintromanint(integer);	/* void zprintromanint(); */
#define printromanint(n) zprintromanint((integer) (n))
#define printromanint_regmem
void printcurrentstring(void); /* void printcurrentstring(); */
#define printcurrentstring_regmem
// void terminput(void); /* void terminput(); */
void terminput(int, int); /* void terminput(); */
#define terminput_regmem
void zinterror(integer); /* void zinterror(); */
#define interror(n) zinterror((integer) (n))
#define interror_regmem
void normalizeselector(void); /* void normalizeselector(); */
#define normalizeselector_regmem
void pauseforinstructions(void); /* void pauseforinstructions(); */
#define pauseforinstructions_regmem
integer zhalf(integer); /* integer zhalf(); */
#define half(x) zhalf((integer) (x))
#define half_regmem
scaled zrounddecimals(smallnumber); /* scaled zrounddecimals(); */
#define rounddecimals(k) zrounddecimals((smallnumber) (k))
#define rounddecimals_regmem
void zprintscaled(scaled); /* void zprintscaled(); */
#define printscaled(s) zprintscaled((scaled) (s))
#define printscaled_regmem
scaled zmultandadd(integer, scaled, scaled, scaled); /* scaled zmultandadd(); */
#define multandadd(n, x, y, maxanswer) zmultandadd((integer) (n), (scaled) (x), (scaled) (y), (scaled) (maxanswer))
#define multandadd_regmem
scaled zxovern(scaled, integer);	/* scaled zxovern(); */
#define xovern(x, n) zxovern((scaled) (x), (integer) (n))
#define xovern_regmem
scaled zxnoverd(scaled, integer, integer); /* scaled zxnoverd(); */
#define xnoverd(x, n, d) zxnoverd((scaled) (x), (integer) (n), (integer) (d))
#define xnoverd_regmem
halfword zbadness(scaled, scaled); /* halfword zbadness(); */
#define badness(t, s) zbadness((scaled) (t), (scaled) (s))
#define badness_regmem
void zprintword(memoryword); /* void zprintword(); */
#define printword(w) zprintword((memoryword) (w))
#define printword_regmem
void zshowtokenlist(integer, integer, integer); /* void zshowtokenlist(); */
#define showtokenlist(p, q, l) zshowtokenlist((integer) (p), (integer) (q), (integer) (l))
#define showtokenlist_regmem 
void runaway(void); /* void runaway(); */
#define runaway_regmem 
halfword getavail(void); /* halfword getavail(); */
#define getavail_regmem 
void zflushlist(halfword); /* void zflushlist(); */
#define flushlist(p) zflushlist((halfword) (p))
#define flushlist_regmem 
halfword zgetnode(integer); /* halfword zgetnode(); */
#define getnode(s) zgetnode((integer) (s))
#define getnode_regmem 
void zfreenode(halfword, halfword); /* void zfreenode(); */
#define freenode(p, s) zfreenode((halfword) (p), (halfword) (s))
#define freenode_regmem 
void sortavail(void); /* void sortavail(); */
#define sortavail_regmem 
halfword newnullbox(void); /* halfword newnullbox(); */
#define newnullbox_regmem 
halfword newrule(void); /* halfword newrule(); */
#define newrule_regmem 
halfword znewligature(quarterword, quarterword, halfword); /* halfword znewligature(); */
#define newligature(f, c, q) znewligature((quarterword) (f), (quarterword) (c), (halfword) (q))
#define newligature_regmem 
halfword znewligitem(quarterword); /* halfword znewligitem(); */
#define newligitem(c) znewligitem((quarterword) (c))
#define newligitem_regmem 
halfword newdisc(void); /* halfword newdisc(); */
#define newdisc_regmem 
halfword znewmath(scaled, smallnumber); /* halfword znewmath(); */
#define newmath(w, s) znewmath((scaled) (w), (smallnumber) (s))
#define newmath_regmem 
halfword znewspec(halfword); /* halfword znewspec(); */
#define newspec(p) znewspec((halfword) (p))
#define newspec_regmem 
halfword znewparamglue(smallnumber); /* halfword znewparamglue(); */
#define newparamglue(n) znewparamglue((smallnumber) (n))
#define newparamglue_regmem  /* register memoryword *eqtb=zeqtb; */
halfword znewglue(halfword); /* halfword znewglue(); */
#define newglue(q) znewglue((halfword) (q))
#define newglue_regmem 
halfword znewskipparam(smallnumber); /* halfword znewskipparam(); */
#define newskipparam(n) znewskipparam((smallnumber) (n))
#define newskipparam_regmem  /* register memoryword *eqtb=zeqtb; */
halfword znewkern(scaled); /* halfword znewkern(); */
#define newkern(w) znewkern((scaled) (w))
#define newkern_regmem 
halfword znewpenalty(integer); /* halfword znewpenalty(); */
#define newpenalty(m) znewpenalty((integer) (m))
#define newpenalty_regmem 
void zcheckmem(booleane); /* void zcheckmem(); */
#define checkmem(printlocs) zcheckmem((booleane) (printlocs))
#define checkmem_regmem 
void zsearchmem(halfword); /* void zsearchmem(); */
#define searchmem(p) zsearchmem((halfword) (p))
#define searchmem_regmem  /* register memoryword *eqtb=zeqtb; */
void zshortdisplay(integer); /* void zshortdisplay(); */
#define shortdisplay(p) zshortdisplay((integer) (p))
#define shortdisplay_regmem 
void zprintfontandchar(integer); /* void zprintfontandchar(); */
#define printfontandchar(p) zprintfontandchar((integer) (p))
#define printfontandchar_regmem 
void zprintmark(integer); /* void zprintmark(); */
#define printmark(p) zprintmark((integer) (p))
#define printmark_regmem 
void zprintruledimen(scaled); /* void zprintruledimen(); */
#define printruledimen(d) zprintruledimen((scaled) (d))
#define printruledimen_regmem
void zprintglue(scaled, integer, strnumber); /* void zprintglue(); */
#define printglue(d, order, s) zprintglue((scaled) (d), (integer) (order), (strnumber) (s))
#define printglue_regmem
void zprintspec(integer, strnumber); /* void zprintspec(); */
#define printspec(p, s) zprintspec((integer) (p), (strnumber) (s))
#define printspec_regmem 
void zprintfamandchar(halfword); /* void zprintfamandchar(); */
#define printfamandchar(p) zprintfamandchar((halfword) (p))
#define printfamandchar_regmem 
void zprintdelimiter(halfword); /* void zprintdelimiter(); */
#define printdelimiter(p) zprintdelimiter((halfword) (p))
#define printdelimiter_regmem 
void zprintsubsidiarydata(halfword, ASCIIcode); /* void zprintsubsidiarydata(); */
#define printsubsidiarydata(p, c) zprintsubsidiarydata((halfword) (p), (ASCIIcode) (c))
#define printsubsidiarydata_regmem 
void zprintstyle(integer); /* void zprintstyle(); */
#define printstyle(c) zprintstyle((integer) (c))
#define printstyle_regmem
void zprintskipparam(integer); /* void zprintskipparam(); */
#define printskipparam(n) zprintskipparam((integer) (n))
#define printskipparam_regmem
void zshownodelist(integer); /* void zshownodelist(); */
#define shownodelist(p) zshownodelist((integer) (p))
#define shownodelist_regmem 
void zshowbox(halfword); /* void zshowbox(); */
#define showbox(p) zshowbox((halfword) (p))
#define showbox_regmem  /* register memoryword *eqtb=zeqtb; */
void zdeletetokenref(halfword); /* void zdeletetokenref(); */
#define deletetokenref(p) zdeletetokenref((halfword) (p))
#define deletetokenref_regmem 
void zdeleteglueref(halfword); /* void zdeleteglueref(); */
#define deleteglueref(p) zdeleteglueref((halfword) (p))
#define deleteglueref_regmem 
void zflushnodelist(halfword); /* void zflushnodelist(); */
#define flushnodelist(p) zflushnodelist((halfword) (p))
#define flushnodelist_regmem 
halfword zcopynodelist(halfword); /* halfword zcopynodelist(); */
#define copynodelist(p) zcopynodelist((halfword) (p))
#define copynodelist_regmem 
void zprintmode(integer); /* void zprintmode(); */
#define printmode(m) zprintmode((integer) (m))
#define printmode_regmem
void pushnest(void); /* void pushnest(); */
#define pushnest_regmem
void popnest(void); /* void popnest(); */
#define popnest_regmem 
void showactivities(void); /* void showactivities(); */
#define showactivities_regmem  /* register memoryword *eqtb=zeqtb; */
void zprintparam(integer); /* void zprintparam(); */
#define printparam(n) zprintparam((integer) (n))
#define printparam_regmem
void begindiagnostic(void); /* void begindiagnostic(); */
#define begindiagnostic_regmem  /* register memoryword *eqtb=zeqtb; */
void zenddiagnostic(booleane); /* void zenddiagnostic(); */
#define enddiagnostic(blankline) zenddiagnostic((booleane) (blankline))
#define enddiagnostic_regmem
void zprintlengthparam(integer); /* void zprintlengthparam(); */
#define printlengthparam(n) zprintlengthparam((integer) (n))
#define printlengthparam_regmem
void zprintcmdchr(quarterword, halfword); /* void zprintcmdchr(); */
#define printcmdchr(cmd, chrcode) zprintcmdchr((quarterword) (cmd), (halfword) (chrcode))
#define printcmdchr_regmem
void zshoweqtb(halfword); /* void zshoweqtb(); */
#define showeqtb(n) zshoweqtb((halfword) (n))
#define showeqtb_regmem  /* register memoryword *eqtb=zeqtb; */
halfword zidlookup(integer, integer); /* halfword zidlookup(); */
#define idlookup(j, l) zidlookup((integer) (j), (integer) (l))
#define idlookup_regmem
void zprimitive(strnumber, quarterword, halfword); /* void zprimitive(); */
#define primitive(s, c, o) zprimitive((strnumber) (s), (quarterword) (c), (halfword) (o))
#define primitive_regmem  /* register memoryword *eqtb=zeqtb; */
void znewsavelevel(groupcode); /* void znewsavelevel(); */
#define newsavelevel(c) znewsavelevel((groupcode) (c))
#define newsavelevel_regmem
void zeqdestroy(memoryword); /* void zeqdestroy(); */
#define eqdestroy(w) zeqdestroy((memoryword) (w))
#define eqdestroy_regmem 
void zeqsave(halfword, quarterword); /* void zeqsave(); */
#define eqsave(p, l) zeqsave((halfword) (p), (quarterword) (l))
#define eqsave_regmem  /* register memoryword *eqtb=zeqtb; */
void zeqdefine(halfword, quarterword, halfword); /* void zeqdefine(); */
#define eqdefine(p, t, e) zeqdefine((halfword) (p), (quarterword) (t), (halfword) (e))
#define eqdefine_regmem  /* register memoryword *eqtb=zeqtb; */
void zeqworddefine(halfword, integer); /* void zeqworddefine(); */
#define eqworddefine(p, w) zeqworddefine((halfword) (p), (integer) (w))
#define eqworddefine_regmem  /* register memoryword *eqtb=zeqtb; */
void zgeqdefine(halfword, quarterword, halfword); /* void zgeqdefine(); */
#define geqdefine(p, t, e) zgeqdefine((halfword) (p), (quarterword) (t), (halfword) (e))
#define geqdefine_regmem  /* register memoryword *eqtb=zeqtb; */
void zgeqworddefine(halfword, integer); /* void zgeqworddefine(); */
#define geqworddefine(p, w) zgeqworddefine((halfword) (p), (integer) (w))
#define geqworddefine_regmem  /* register memoryword *eqtb=zeqtb; */
void zsaveforafter(halfword); /* void zsaveforafter(); */
#define saveforafter(t) zsaveforafter((halfword) (t))
#define saveforafter_regmem
void zrestoretrace(halfword, strnumber); /* void zrestoretrace(); */
#define restoretrace(p, s) zrestoretrace((halfword) (p), (strnumber) (s))
#define restoretrace_regmem
void unsave(void); /* void unsave(); */
#define unsave_regmem  /* register memoryword *eqtb=zeqtb; */
void preparemag(void); /* void preparemag(); */
#define preparemag_regmem  /* register memoryword *eqtb=zeqtb; */
void ztokenshow(halfword); /* void ztokenshow(); */
#define tokenshow(p) ztokenshow((halfword) (p))
#define tokenshow_regmem 
void printmeaning(void); /* void printmeaning(); */
#define printmeaning_regmem
void showcurcmdchr(void); /* void showcurcmdchr(); */
#define showcurcmdchr_regmem
void showcontext(void); /* void showcontext(); */
#define showcontext_regmem  /* register memoryword *eqtb=zeqtb; */
void zbegintokenlist(halfword, quarterword); /* void zbegintokenlist(); */
#define begintokenlist(p, t) zbegintokenlist((halfword) (p), (quarterword) (t))
#define begintokenlist_regmem  /* register memoryword *eqtb=zeqtb; */
void endtokenlist(void); /* void endtokenlist(); */
#define endtokenlist_regmem
void backinput(void); /* void backinput(); */
#define backinput_regmem 
void backerror(void); /* void backerror(); */
#define backerror_regmem
void inserror(void); /* void inserror(); */
#define inserror_regmem
void beginfilereading(void); /* void beginfilereading(); */
#define beginfilereading_regmem
void endfilereading(void); /* void endfilereading(); */
#define endfilereading_regmem
void clearforerrorprompt(void); /* void clearforerrorprompt(); */
#define clearforerrorprompt_regmem
void checkoutervalidity(void); /* void checkoutervalidity(); */
#define checkoutervalidity_regmem 
void getnext(void); /* void getnext(); */
#define getnext_regmem  /* register memoryword *eqtb=zeqtb; */
void firmuptheline(void); /* void firmuptheline(); */
#define firmuptheline_regmem  /* register memoryword *eqtb=zeqtb; */
void gettoken(void); /* void gettoken(); */
#define gettoken_regmem
void macrocall(void); /* void macrocall(); */
#define macrocall_regmem  /* register memoryword *eqtb=zeqtb; */
void insertrelax(void); /* void insertrelax(); */
#define insertrelax_regmem
void expand(void); /* void expand(); */
#define expand_regmem  /* register memoryword *eqtb=zeqtb; */
void getxtoken(void); /* void getxtoken(); */
#define getxtoken_regmem
void xtoken(void); /* void xtoken(); */
#define xtoken_regmem
void scanleftbrace(void); /* void scanleftbrace(); */
#define scanleftbrace_regmem
void scanoptionalequals(void); /* void scanoptionalequals(); */
#define scanoptionalequals_regmem
booleane zscankeyword(strnumber); /* booleane zscankeyword(); */
#define scankeyword(s) zscankeyword((strnumber) (s))
#define scankeyword_regmem 
void muerror(void); /* void muerror(); */
#define muerror_regmem
void scaneightbitint(void); /* void scaneightbitint(); */
#define scaneightbitint_regmem
void scancharnum(void); /* void scancharnum(); */
#define scancharnum_regmem
void scanfourbitint(void); /* void scanfourbitint(); */
#define scanfourbitint_regmem
void scanfifteenbitint(void); /* void scanfifteenbitint(); */
#define scanfifteenbitint_regmem
void scantwentysevenbitint(void); /* void scantwentysevenbitint(); */
#define scantwentysevenbitint_regmem
void scanfontident(void); /* void scanfontident(); */
#define scanfontident_regmem  /* register memoryword *eqtb=zeqtb; */
void zfindfontdimen(booleane); /* void zfindfontdimen(); */
#define findfontdimen(writing) zfindfontdimen((booleane) (writing))
#define findfontdimen_regmem
void zscansomethinginternal(smallnumber, booleane);
/* void zscansomethinginternal(); */
#define scansomethinginternal(level, negative) zscansomethinginternal((smallnumber) (level), (booleane) (negative))
#define scansomethinginternal_regmem  /* register memoryword *eqtb=zeqtb; */
void scanint(void); /* void scanint(); */
#define scanint_regmem
void zscandimen(booleane, booleane, booleane); /* void zscandimen(); */
#define scandimen(mu, inf, shortcut) zscandimen((booleane) (mu), (booleane) (inf), (booleane) (shortcut))
#define scandimen_regmem  /* register memoryword *eqtb=zeqtb; */
void zscanglue(smallnumber); /* void zscanglue(); */
#define scanglue(level) zscanglue((smallnumber) (level))
#define scanglue_regmem 
halfword scanrulespec(void); /* halfword scanrulespec(); */
#define scanrulespec_regmem 
halfword zstrtoks(poolpointer); /* halfword zstrtoks(); */
#define strtoks(b) zstrtoks((poolpointer) (b))
#define strtoks_regmem 
halfword thetoks(void); /* halfword thetoks(); */
#define thetoks_regmem 
void insthetoks(void); /* void insthetoks(); */
#define insthetoks_regmem 
void convtoks(void); /* void convtoks(); */
#define convtoks_regmem 
halfword zscantoks(booleane, booleane); /* halfword zscantoks(); */
#define scantoks(macrodef, xpand) zscantoks((booleane) (macrodef), (booleane) (xpand))
#define scantoks_regmem 
void zreadtoks(integer, halfword); /* void zreadtoks(); */
#define readtoks(n, r) zreadtoks((integer) (n), (halfword) (r))
#define readtoks_regmem  /* register memoryword *eqtb=zeqtb; */
void passtext(void); /* void passtext(); */
#define passtext_regmem
void zchangeiflimit(smallnumber, halfword); /* void zchangeiflimit(); */
#define changeiflimit(l, p) zchangeiflimit((smallnumber) (l), (halfword) (p))
#define changeiflimit_regmem 
void conditional(void); /* void conditional(); */
#define conditional_regmem  /* register memoryword *eqtb=zeqtb; */
void beginname(void); /* void beginname(); */
#define beginname_regmem
booleane zmorename(ASCIIcode); /* booleane zmorename(); */
#define morename(c) zmorename((ASCIIcode) (c))
#define morename_regmem
void endname(void); /* void endname(); */
#define endname_regmem
void zpackfilename(strnumber, strnumber, strnumber); /* void zpackfilename(); */
#define packfilename(n, a, e) zpackfilename((strnumber) (n), (strnumber) (a), (strnumber) (e))
#define packfilename_regmem
void zpackbufferedname(smallnumber, integer, integer); /* void zpackbufferedname(); */
#define packbufferedname(n, a, b) zpackbufferedname((smallnumber) (n), (integer) (a), (integer) (b))
#define packbufferedname_regmem
strnumber makenamestring(void); /* strnumber makenamestring(); */
#define makenamestring_regmem
strnumber zamakenamestring(alphafile *); /* strnumber zamakenamestring(); */
#define amakenamestring(f) zamakenamestring((alphafile *) &(f))
#define amakenamestring_regmem
strnumber zbmakenamestring(bytefile *); /* strnumber zbmakenamestring(); */
#define bmakenamestring(f) zbmakenamestring((bytefile *) &(f))
#define bmakenamestring_regmem
strnumber zwmakenamestring(wordfile *); /* strnumber zwmakenamestring(); */
#define wmakenamestring(f) zwmakenamestring((wordfile *) &(f))
#define wmakenamestring_regmem
void scanfilename(void); /* void scanfilename(); */
#define scanfilename_regmem
void zpackjobname(strnumber); /* void zpackjobname(); */
#define packjobname(s) zpackjobname((strnumber) (s))
#define packjobname_regmem
void zpromptfilename(strnumber, strnumber); /* void zpromptfilename(); */
#define promptfilename(s, e) zpromptfilename((strnumber) (s), (strnumber) (e))
#define promptfilename_regmem
void openlogfile(void); /* void openlogfile(); */
#define openlogfile_regmem  /* register memoryword *eqtb=zeqtb; */
void startinput(void); /* void startinput(); */
#define startinput_regmem  /* register memoryword *eqtb=zeqtb; */
internalfontnumber zreadfontinfo(halfword, strnumber, strnumber, scaled);
/* internalfontnumber zreadfontinfo(); */
#define readfontinfo(u, nom, aire, s) zreadfontinfo((halfword) (u), (strnumber) (nom), (strnumber) (aire), (scaled) (s))
#define readfontinfo_regmem  /* register memoryword *eqtb=zeqtb; */
void zcharwarning(internalfontnumber, eightbits); /* void zcharwarning(); */
#define charwarning(f, c) zcharwarning((internalfontnumber) (f), (eightbits) (c))
#define charwarning_regmem  /* register memoryword *eqtb=zeqtb; */
halfword znewcharacter(internalfontnumber, eightbits); /* halfword znewcharacter(); */
#define newcharacter(f, c) znewcharacter((internalfontnumber) (f), (eightbits) (c))
#define newcharacter_regmem 
#ifdef ALLOCATEDVIBUF
void dviswap(void); /* void dviswap(); */
#define dviswap_regmem  register eightbits *dvibuf=zdvibuf;
void zdvifour(integer); /* void zdvifour(); */
#define dvifour(x) zdvifour((integer) (x))
#define dvifour_regmem  register eightbits *dvibuf=zdvibuf;
void zdvipop(integer); /* void zdvipop(); */
#define dvipop(l) zdvipop((integer) (l))
#define dvipop_regmem  register eightbits *dvibuf=zdvibuf;
void zdvifontdef(internalfontnumber); /* void zdvifontdef(); */
#define dvifontdef(f) zdvifontdef((internalfontnumber) (f))
#define dvifontdef_regmem  register eightbits *dvibuf=zdvibuf;
void zmovement(scaled, eightbits); /* void zmovement(); */
#define movement(w, o) zmovement((scaled) (w), (eightbits) (o))
#define movement_regmem  register eightbits *dvibuf=zdvibuf;
void zspecialout(halfword); /* void zspecialout(); */
#define specialout(p) zspecialout((halfword) (p))
#define specialout_regmem  register eightbits *dvibuf=zdvibuf;
void hlistout(void); /* void hlistout(); */
#define hlistout_regmem  register eightbits *dvibuf=zdvibuf;
void vlistout(void); /* void vlistout(); */
#define vlistout_regmem  register eightbits *dvibuf=zdvibuf;
void zshipout(halfword); /* void zshipout(); */
#define shipout(p) zshipout((halfword) (p))
#define shipout_regmem  register eightbits *dvibuf=zdvibuf; /* register memoryword *eqtb=zeqtb; */ 

#else	/* not ALLOCATEDVIBUF */

void dviswap(void); /* void dviswap(); */
#define dviswap_regmem
void zdvifour(integer); /* void zdvifour(); */
#define dvifour(x) zdvifour((integer) (x))
#define dvifour_regmem
void zdvipop(integer); /* void zdvipop(); */
#define dvipop(l) zdvipop((integer) (l))
#define dvipop_regmem 
void zdvifontdef(internalfontnumber); /* void zdvifontdef(); */
#define dvifontdef(f) zdvifontdef((internalfontnumber) (f))
#define dvifontdef_regmem
void zmovement(scaled, eightbits); /* void zmovement(); */
#define movement(w, o) zmovement((scaled) (w), (eightbits) (o))
#define movement_regmem 
void zspecialout(halfword); /* void zspecialout(); */
#define specialout(p) zspecialout((halfword) (p))
#define specialout_regmem
void hlistout(void); /* void hlistout(); */
#define hlistout_regmem 
void vlistout(void); /* void vlistout(); */
#define vlistout_regmem 
void zshipout(halfword); /* void zshipout(); */
// int zshipout(halfword); /* void zshipout(); */
#define shipout(p) zshipout((halfword) (p))
#define shipout_regmem  /* register memoryword *eqtb=zeqtb; */
#endif
void zprunemovements(integer); /* void zprunemovements(); */
#define prunemovements(l) zprunemovements((integer) (l))
#define prunemovements_regmem 
void zwriteout(halfword); /* void zwriteout(); */
// int zwriteout(halfword); /* void zwriteout(); */
#define writeout(p) zwriteout((halfword) (p))
#define writeout_regmem 
void zoutwhat(halfword); /* void zoutwhat(); */
// int zoutwhat(halfword); /* void zoutwhat(); */
#define outwhat(p) zoutwhat((halfword) (p))
#define outwhat_regmem 
void zscanspec(groupcode, booleane); /* void zscanspec(); */
#define scanspec(c, threecodes) zscanspec((groupcode) (c), (booleane) (threecodes))
#define scanspec_regmem
halfword zhpack(halfword, scaled, smallnumber); /* halfword zhpack(); */
#define hpack(p, w, m) zhpack((halfword) (p), (scaled) (w), (smallnumber) (m))
#define hpack_regmem  /* register memoryword *eqtb=zeqtb; */
halfword zvpackage(halfword, scaled, smallnumber, scaled); /* halfword zvpackage(); */
#define vpackage(p, h, m, l) zvpackage((halfword) (p), (scaled) (h), (smallnumber) (m), (scaled) (l))
#define vpackage_regmem  /* register memoryword *eqtb=zeqtb; */
void zappendtovlist(halfword); /* void zappendtovlist(); */
#define appendtovlist(b) zappendtovlist((halfword) (b))
#define appendtovlist_regmem  /* register memoryword *eqtb=zeqtb; */
halfword newnoad(void); /* halfword newnoad(); */
#define newnoad_regmem 
halfword znewstyle(smallnumber); /* halfword znewstyle(); */
#define newstyle(s) znewstyle((smallnumber) (s))
#define newstyle_regmem 
halfword newchoice(void); /* halfword newchoice(); */
#define newchoice_regmem 
void showinfo(void); /* void showinfo(); */
#define showinfo_regmem 
halfword zfractionrule(scaled); /* halfword zfractionrule(); */
#define fractionrule(t) zfractionrule((scaled) (t))
#define fractionrule_regmem 
halfword zoverbar(halfword, scaled, scaled); /* halfword zoverbar(); */
#define overbar(b, k, t) zoverbar((halfword) (b), (scaled) (k), (scaled) (t))
#define overbar_regmem 
halfword zcharbox(internalfontnumber, quarterword); /* halfword zcharbox(); */
#define charbox(f, c) zcharbox((internalfontnumber) (f), (quarterword) (c))
#define charbox_regmem 
void zstackintobox(halfword, internalfontnumber, quarterword);
/* void zstackintobox(); */
#define stackintobox(b, f, c) zstackintobox((halfword) (b), (internalfontnumber) (f), (quarterword) (c))
#define stackintobox_regmem 

scaled zheightplusdepth(internalfontnumber, fquarterword); 
#define heightplusdepth(f, c) zheightplusdepth((internalfontnumber) (f), (fquarterword) (c))

#define heightplusdepth_regmem
halfword zvardelimiter(halfword, smallnumber, scaled); /* halfword zvardelimiter(); */
#define vardelimiter(d, s, v) zvardelimiter((halfword) (d), (smallnumber) (s), (scaled) (v))
#define vardelimiter_regmem  /* register memoryword *eqtb=zeqtb; */
halfword zrebox(halfword, scaled); /* halfword zrebox(); */
#define rebox(b, w) zrebox((halfword) (b), (scaled) (w))
#define rebox_regmem 
halfword zmathglue(halfword, scaled); /* halfword zmathglue(); */
#define mathglue(g, m) zmathglue((halfword) (g), (scaled) (m))
#define mathglue_regmem 
void zmathkern(halfword, scaled); /* void zmathkern(); */
#define mathkern(p, m) zmathkern((halfword) (p), (scaled) (m))
#define mathkern_regmem 
void flushmath(void); /* void flushmath(); */
#define flushmath_regmem 
halfword zcleanbox(halfword, smallnumber); /* halfword zcleanbox(); */
#define cleanbox(p, s) zcleanbox((halfword) (p), (smallnumber) (s))
#define cleanbox_regmem  /* register memoryword *eqtb=zeqtb; */
void zfetch(halfword); /* void zfetch(); */
#define fetch(a) zfetch((halfword) (a))
#define fetch_regmem  /* register memoryword *eqtb=zeqtb; */
void zmakeover(halfword); /* void zmakeover(); */
#define makeover(q) zmakeover((halfword) (q))
#define makeover_regmem  /* register memoryword *eqtb=zeqtb; */
void zmakeunder(halfword); /* void zmakeunder(); */
#define makeunder(q) zmakeunder((halfword) (q))
#define makeunder_regmem  /* register memoryword *eqtb=zeqtb; */
void zmakevcenter(halfword); /* void zmakevcenter(); */
#define makevcenter(q) zmakevcenter((halfword) (q))
#define makevcenter_regmem  /* register memoryword *eqtb=zeqtb; */
void zmakeradical(halfword); /* void zmakeradical(); */
#define makeradical(q) zmakeradical((halfword) (q))
#define makeradical_regmem  /* register memoryword *eqtb=zeqtb; */
void zmakemathaccent(halfword); /* void zmakemathaccent(); */
#define makemathaccent(q) zmakemathaccent((halfword) (q))
#define makemathaccent_regmem 
void zmakefraction(halfword); /* void zmakefraction(); */
#define makefraction(q) zmakefraction((halfword) (q))
#define makefraction_regmem  /* register memoryword *eqtb=zeqtb; */
scaled zmakeop(halfword); /* scaled zmakeop(); */
#define makeop(q) zmakeop((halfword) (q))
#define makeop_regmem  /* register memoryword *eqtb=zeqtb; */
void zmakeord(halfword); /* void zmakeord(); */
#define makeord(q) zmakeord((halfword) (q))
#define makeord_regmem 
void zmakescripts(halfword, scaled); /* void zmakescripts(); */
#define makescripts(q, delta) zmakescripts((halfword) (q), (scaled) (delta))
#define makescripts_regmem  /* register memoryword *eqtb=zeqtb; */
smallnumber zmakeleftright(halfword, smallnumber, scaled, scaled);
/* smallnumber zmakeleftright(); */
#define makeleftright(q, style, maxd, maxh) zmakeleftright((halfword) (q), (smallnumber) (style), (scaled) (maxd), (scaled) (maxh))
#define makeleftright_regmem  /* register memoryword *eqtb=zeqtb; */
void mlisttohlist(void); /* void mlisttohlist(); */
#define mlisttohlist_regmem  /* register memoryword *eqtb=zeqtb; */
void pushalignment(void); /* void pushalignment(); */
#define pushalignment_regmem 
void popalignment(void); /* void popalignment(); */
#define popalignment_regmem 
void getpreambletoken(void); /* void getpreambletoken(); */
#define getpreambletoken_regmem  /* register memoryword *eqtb=zeqtb; */
void initalign(void); /* void initalign(); */
#define initalign_regmem  /* register memoryword *eqtb=zeqtb; */
void zinitspan(halfword); /* void zinitspan(); */
#define initspan(p) zinitspan((halfword) (p))
#define initspan_regmem
void initrow(void); /* void initrow(); */
#define initrow_regmem 
void initcol(void); /* void initcol(); */
#define initcol_regmem 
booleane fincol(void); /* booleane fincol(); */
#define fincol_regmem 
void finrow(void); /* void finrow(); */
#define finrow_regmem  /* register memoryword *eqtb=zeqtb; */
void finalign(void); /* void finalign(); */
#define finalign_regmem  /* register memoryword *eqtb=zeqtb; */
void alignpeek(void); /* void alignpeek(); */
#define alignpeek_regmem
halfword zfiniteshrink(halfword); /* halfword zfiniteshrink(); */
#define finiteshrink(p) zfiniteshrink((halfword) (p))
#define finiteshrink_regmem 
void ztrybreak(integer, smallnumber); /* void ztrybreak(); */
#define trybreak(pi, breaktype) ztrybreak((integer) (pi), (smallnumber) (breaktype))
#define trybreak_regmem  /* register memoryword *eqtb=zeqtb; */
void zpostlinebreak(integer); /* void zpostlinebreak(); */
#define postlinebreak(finalwidowpenalty) zpostlinebreak((integer) (finalwidowpenalty))
#define postlinebreak_regmem  /* register memoryword *eqtb=zeqtb; */
smallnumber zreconstitute(smallnumber, smallnumber, halfword, halfword); 
/* smallnumber zreconstitute(); */
#define reconstitute(j, n, bchar, hchar) zreconstitute((smallnumber) (j), (smallnumber) (n), (halfword) (bchar), (halfword) (hchar))
#define reconstitute_regmem 
void hyphenate(void); /* void hyphenate(); */
#define hyphenate_regmem 
trieopcode znewtrieop(smallnumber, smallnumber, trieopcode);
/* trieopcode znewtrieop(); */
#define newtrieop(d, n, v) znewtrieop((smallnumber) (d), (smallnumber) (n), (trieopcode) (v))
#define newtrieop_regmem
triepointer ztrienode(triepointer); /* triepointer ztrienode(); */
#define trienode(p) ztrienode((triepointer) (p))
#define trienode_regmem
triepointer zcompresstrie(triepointer); /* triepointer zcompresstrie(); */
#define compresstrie(p) zcompresstrie((triepointer) (p))
#define compresstrie_regmem
void zfirstfit(triepointer); /* void zfirstfit(); */
#define firstfit(p) zfirstfit((triepointer) (p))
#define firstfit_regmem
void ztriepack(triepointer); /* void ztriepack(); */
#define triepack(p) ztriepack((triepointer) (p))
#define triepack_regmem
void ztriefix(triepointer); /* void ztriefix(); */
#define triefix(p) ztriefix((triepointer) (p))
#define triefix_regmem
void newpatterns(void); /* void newpatterns(); */
#define newpatterns_regmem  /* register memoryword *eqtb=zeqtb; */
void inittrie(void); /* void inittrie(); */
#define inittrie_regmem
void zlinebreak(integer); /* void zlinebreak(); */
#define linebreak(finalwidowpenalty) zlinebreak((integer) (finalwidowpenalty))
#define linebreak_regmem  /* register memoryword *eqtb=zeqtb; */
void newhyphexceptions(void); /* void newhyphexceptions(); */
#define newhyphexceptions_regmem  /* register memoryword *eqtb=zeqtb; */
halfword zprunepagetop(halfword); /* halfword zprunepagetop(); */
#define prunepagetop(p) zprunepagetop((halfword) (p))
#define prunepagetop_regmem 
halfword zvertbreak(halfword, scaled, scaled); /* halfword zvertbreak(); */
#define vertbreak(p, h, d) zvertbreak((halfword) (p), (scaled) (h), (scaled) (d))
#define vertbreak_regmem 
halfword zvsplit(eightbits, scaled); /* halfword zvsplit(); */
#define vsplit(n, h) zvsplit((eightbits) (n), (scaled) (h))
#define vsplit_regmem  /* register memoryword *eqtb=zeqtb; */
void printtotals(void); /* void printtotals(); */
#define printtotals_regmem
void zfreezepagespecs(smallnumber); /* void zfreezepagespecs(); */
#define freezepagespecs(s) zfreezepagespecs((smallnumber) (s))
#define freezepagespecs_regmem  /* register memoryword *eqtb=zeqtb; */
void zboxerror(eightbits); /* void zboxerror(); */
#define boxerror(n) zboxerror((eightbits) (n))
#define boxerror_regmem  /* register memoryword *eqtb=zeqtb; */
void zensurevbox(eightbits); /* void zensurevbox(); */
#define ensurevbox(n) zensurevbox((eightbits) (n))
#define ensurevbox_regmem  /* register memoryword *eqtb=zeqtb; */
void zfireup(halfword); /* void zfireup(); */
#define fireup(c) zfireup((halfword) (c))
#define fireup_regmem  /* register memoryword *eqtb=zeqtb; */
void buildpage(void); /* void buildpage(); */
#define buildpage_regmem  /* register memoryword *eqtb=zeqtb; */
void appspace(void); /* void appspace(); */
#define appspace_regmem  /* register memoryword *eqtb=zeqtb; */
void insertdollarsign(void); /* void insertdollarsign(); */
#define insertdollarsign_regmem
void youcant(void); /* void youcant(); */
#define youcant_regmem
void reportillegalcase(void); /* void reportillegalcase(); */
#define reportillegalcase_regmem
booleane privileged(void); /* booleane privileged(); */
#define privileged_regmem
booleane itsallover(void); /* booleane itsallover(); */
#define itsallover_regmem  /* register memoryword *eqtb=zeqtb; */
void appendglue(void); /* void appendglue(); */
#define appendglue_regmem 
void appendkern(void); /* void appendkern(); */
#define appendkern_regmem 
void offsave(void); /* void offsave(); */
#define offsave_regmem 
void extrarightbrace(void); /* void extrarightbrace(); */
#define extrarightbrace_regmem
void normalparagraph(void); /* void normalparagraph(); */
#define normalparagraph_regmem  /* register memoryword *eqtb=zeqtb; */
void zboxend(integer); /* void zboxend(); */
#define boxend(boxcontext) zboxend((integer) (boxcontext))
#define boxend_regmem 
void zbeginbox(integer); /* void zbeginbox(); */
#define beginbox(boxcontext) zbeginbox((integer) (boxcontext))
#define beginbox_regmem  /* register memoryword *eqtb=zeqtb; */
void zscanbox(integer); /* void zscanbox(); */
#define scanbox(boxcontext) zscanbox((integer) (boxcontext))
#define scanbox_regmem
void zpackage(smallnumber); /* void zpackage(); */
#define package(c) zpackage((smallnumber) (c))
#define package_regmem  /* register memoryword *eqtb=zeqtb; */
smallnumber znormmin(integer); /* smallnumber znormmin(); */
#define normmin(h) znormmin((integer) (h))
#define normmin_regmem
void znewgraf(booleane); /* void znewgraf(); */
#define newgraf(indented) znewgraf((booleane) (indented))
#define newgraf_regmem  /* register memoryword *eqtb=zeqtb; */
void indentinhmode(void); /* void indentinhmode(); */
#define indentinhmode_regmem  /* register memoryword *eqtb=zeqtb; */
void headforvmode(void); /* void headforvmode(); */
#define headforvmode_regmem
void endgraf(void); /* void endgraf(); */
#define endgraf_regmem  /* register memoryword *eqtb=zeqtb; */
void begininsertoradjust(void); /* void begininsertoradjust(); */
#define begininsertoradjust_regmem
void makemark(void); /* void makemark(); */
#define makemark_regmem 
void appendpenalty(void); /* void appendpenalty(); */
#define appendpenalty_regmem 
void deletelast(void); /* void deletelast(); */
#define deletelast_regmem 
void unpackage(void); /* void unpackage(); */
#define unpackage_regmem  /* register memoryword *eqtb=zeqtb; */
void appenditaliccorrection(void); /* void appenditaliccorrection(); */
#define appenditaliccorrection_regmem 
void appenddiscretionary(void); /* void appenddiscretionary(); */
#define appenddiscretionary_regmem  /* register memoryword *eqtb=zeqtb; */
void builddiscretionary(void); /* void builddiscretionary(); */
#define builddiscretionary_regmem 
void makeaccent(void); /* void makeaccent(); */
#define makeaccent_regmem  /* register memoryword *eqtb=zeqtb; */
void alignerror(void); /* void alignerror(); */
#define alignerror_regmem
void noalignerror(void); /* void noalignerror(); */
#define noalignerror_regmem
void omiterror(void); /* void omiterror(); */
#define omiterror_regmem
void doendv(void); /* void doendv(); */
#define doendv_regmem
void cserror(void); /* void cserror(); */
#define cserror_regmem
void zpushmath(groupcode); /* void zpushmath(); */
#define pushmath(c) zpushmath((groupcode) (c))
#define pushmath_regmem
void initmath(void); /* void initmath(); */
#define initmath_regmem  /* register memoryword *eqtb=zeqtb; */
void starteqno(void); /* void starteqno(); */
#define starteqno_regmem  /* register memoryword *eqtb=zeqtb; */
void zscanmath(halfword); /* void zscanmath(); */
#define scanmath(p) zscanmath((halfword) (p))
#define scanmath_regmem  /* register memoryword *eqtb=zeqtb; */
void zsetmathchar(integer); /* void zsetmathchar(); */
#define setmathchar(c) zsetmathchar((integer) (c))
#define setmathchar_regmem  /* register memoryword *eqtb=zeqtb; */
void mathlimitswitch(void); /* void mathlimitswitch(); */
#define mathlimitswitch_regmem 
void zscandelimiter(halfword, booleane); /* void zscandelimiter(); */
#define scandelimiter(p, r) zscandelimiter((halfword) (p), (booleane) (r))
#define scandelimiter_regmem  /* register memoryword *eqtb=zeqtb; */
void mathradical(void); /* void mathradical(); */
#define mathradical_regmem 
void mathac(void); /* void mathac(); */
#define mathac_regmem  /* register memoryword *eqtb=zeqtb; */
void appendchoices(void); /* void appendchoices(); */
#define appendchoices_regmem 
halfword zfinmlist(halfword); /* halfword zfinmlist(); */
#define finmlist(p) zfinmlist((halfword) (p))
#define finmlist_regmem 
void buildchoices(void); /* void buildchoices(); */
#define buildchoices_regmem 
void subsup(void); /* void subsup(); */
#define subsup_regmem 
void mathfraction(void); /* void mathfraction(); */
#define mathfraction_regmem 
void mathleftright(void); /* void mathleftright(); */
#define mathleftright_regmem 
void aftermath(void); /* void aftermath(); */
#define aftermath_regmem  /* register memoryword *eqtb=zeqtb; */
void resumeafterdisplay(void); /* void resumeafterdisplay(); */
/* #define resumeafterdisplay_regmem */	/* changed 1996/Jan/10 */
#define resumeafterdisplay_regmem  /* register memoryword *eqtb=zeqtb; */
void getrtoken(void); /* void getrtoken(); */
#define getrtoken_regmem
void trapzeroglue(void); /* void trapzeroglue(); */
#define trapzeroglue_regmem 
void zdoregistercommand(smallnumber); /* void zdoregistercommand(); */
#define doregistercommand(a) zdoregistercommand((smallnumber) (a))
#define doregistercommand_regmem  /* register memoryword *eqtb=zeqtb; */
void alteraux(void); /* void alteraux(); */
#define alteraux_regmem
void alterprevgraf(void); /* void alterprevgraf(); */
#define alterprevgraf_regmem
void alterpagesofar(void); /* void alterpagesofar(); */
#define alterpagesofar_regmem
void alterinteger(void); /* void alterinteger(); */
#define alterinteger_regmem
void alterboxdimen(void); /* void alterboxdimen(); */
#define alterboxdimen_regmem  /* register memoryword *eqtb=zeqtb; */
void znewfont(smallnumber); /* void znewfont(); */
#define newfont(a) znewfont((smallnumber) (a))
#define newfont_regmem  /* register memoryword *eqtb=zeqtb; */
void newinteraction(void); /* void newinteraction(); */
#define newinteraction_regmem
void prefixedcommand(void); /* void prefixedcommand(); */
#define prefixedcommand_regmem  /* register memoryword *eqtb=zeqtb; */
void doassignments(void); /* void doassignments(); */
#define doassignments_regmem
void openorclosein(void); /* void openorclosein(); */
#define openorclosein_regmem
void issuemessage(void); /* void issuemessage(); */
#define issuemessage_regmem  /* register memoryword *eqtb=zeqtb; */
void shiftcase(void); /* void shiftcase(); */
#define shiftcase_regmem  /* register memoryword *eqtb=zeqtb; */
void showwhatever(void); /* void showwhatever(); */
#define showwhatever_regmem  /* register memoryword *eqtb=zeqtb; */
void storefmtfile(void);
// int storefmtfile(void);
#define storefmtfile_regmem  /* register memoryword *eqtb=zeqtb; */
void znewwhatsit(smallnumber, smallnumber); /* void znewwhatsit(); */
#define newwhatsit(s, w) znewwhatsit((smallnumber) (s), (smallnumber) (w))
#define newwhatsit_regmem 
void znewwritewhatsit(smallnumber); /* void znewwritewhatsit(); */
#define newwritewhatsit(w) znewwritewhatsit((smallnumber) (w))
#define newwritewhatsit_regmem 
void doextension(void); /* void doextension(); */
#define doextension_regmem  /* register memoryword *eqtb=zeqtb; */
void fixlanguage(void); /* void fixlanguage(); */
#define fixlanguage_regmem  /* register memoryword *eqtb=zeqtb; */
void handlerightbrace(void); /* void handlerightbrace(); */
#define handlerightbrace_regmem  /* register memoryword *eqtb=zeqtb; */
void maincontrol(void); /* void maincontrol(); */
// int maincontrol(void); /* void maincontrol(); */
#define maincontrol_regmem  /* register memoryword *eqtb=zeqtb; */
void giveerrhelp(void); /* void giveerrhelp(); */
#define giveerrhelp_regmem  /* register memoryword *eqtb=zeqtb; */
booleane openfmtfile(void); /* booleane openfmtfile(); */
#define openfmtfile_regmem
booleane loadfmtfile(void); /* booleane loadfmtfile(); */
#define loadfmtfile_regmem  /* register memoryword *eqtb=zeqtb; */
void closefilesandterminate(void); /* void closefilesandterminate(); */
#ifdef ALLOCATEDVIBUF
#define closefilesandterminate_regmem  register eightbits *dvibuf=zdvibuf; /* register memoryword *eqtb=zeqtb; */
#else
#define closefilesandterminate_regmem  /* register memoryword *eqtb=zeqtb; */
#endif
void finalcleanup(void); /* void finalcleanup(); */
// int finalcleanup(void); /* void finalcleanup(); */
#define finalcleanup_regmem 
void initprim(void); /* void initprim(); */
#define initprim_regmem  /* register memoryword *eqtb=zeqtb; */
void debughelp(void);
#define debughelp_regmem  /* register memoryword *eqtb=zeqtb; */
/* void texbody(void); */
int texbody(void);					/* 1993/Dec/16 bkph */
#define texbody_regmem  /* register memoryword *eqtb=zeqtb; */

#endif /* end of NOT ifdef USEREGISTERS */

/* may want to consider copying other addresses to local registers ... */

/*
 * The C compiler ignores most unnecessary casts (i.e., casts of something
 * to its own type).  However, for structures, it doesn't.  Therefore,
 * we have to redefine these two macros so that they don't try to cast
 * the argument (a memoryword) as a memoryword.
 */
#undef	eqdestroy
#define	eqdestroy(x)	zeqdestroy(x)
#undef	printword
#define	printword(x)	zprintword(x)
