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

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
void synch_h(void)
{
  if (cur_h != dvi_h)
  {
    movement(cur_h - dvi_h, right1);
    dvi_h = cur_h;
  }
}
void synch_v(void)
{
  if (cur_v != dvi_v)
  {
    movement(cur_v - dvi_v, down1);
    dvi_v = cur_v;
  }
}
void set_cur_lang(void)
{
  if (language <= 0)
    cur_lang = 0;
  else if (language > 255)
    cur_lang = 0;
  else
    cur_lang = language;
}
INLINE void free_avail_(halfword p)
{
  link(p) = avail;
  avail = p;
#ifdef STAT
  decr(dyn_used);
#endif /* STAT */
}
INLINE void dvi_out_(ASCII_code op)
{
  dvi_buf[dvi_ptr] = op;
  incr(dvi_ptr);
  if (dvi_ptr == dvi_limit)
    dvi_swap();
}
INLINE void succumb (void)
{
  if (interaction == error_stop_mode)
    interaction = scroll_mode;

  if (log_opened)
  {
    error();
  }
#ifdef DEBUG
  if (interaction > 0)
    debug_help();
#endif
  history = 3;
  jump_out();
}
INLINE void flush_string (void)
{
  decr(str_ptr);
  pool_ptr = str_start[str_ptr];
}
INLINE void append_char (ASCII_code c)
{
  str_pool[pool_ptr] = c;
  incr(pool_ptr);
}
INLINE void append_lc_hex (ASCII_code c)
{
  if (c < 10)
    append_char(c + '0');
  else
    append_char(c - 10 + 'a');
}
INLINE void print_err (const char * s)
{
  if (interaction == error_stop_mode);
    print_nl("! ");
  print_string(s);
}
INLINE void tex_help (unsigned int n, ...)
{
  int i;
  va_list help_arg;

  if (n > 6)
    n = 6;

  help_ptr = n;
  va_start(help_arg, n);

  for (i = n - 1; i > -1; --i)
    help_line[i] = va_arg(help_arg, char *);

  va_end(help_arg);
}
INLINE void str_room_ (int val)
{
#ifdef ALLOCATESTRING
  if (pool_ptr + val > current_pool_size)
    str_pool = realloc_str_pool(increment_pool_size);

  if (pool_ptr + val > current_pool_size)
  {
    overflow("pool size", current_pool_size - init_pool_ptr);
  }
#else
  if (pool_ptr + val > pool_size)
  {
    overflow("pool size", pool_size - init_pool_ptr);
  }
#endif
}
INLINE void tail_append_ (pointer val)
{
  link(tail) = val;
  tail = link(tail);
}
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* sec 0058 */
void print_ln (void)
{
  switch (selector)
  {
    case term_and_log:
      show_char('\n');
      term_offset = 0;
      (void) putc ('\n', log_file);
      file_offset = 0;
      break;

    case log_only:
      (void) putc ('\n',  log_file);
      file_offset = 0;
      break;

    case term_only:
      show_char('\n');
      term_offset = 0;
      break;

    case no_print:
    case pseudo:
    case new_string:
      break;

    default:
      (void) putc ('\n', write_file[selector]);
      break;
  }
}
/* sec 0058 */
void print_char_ (ASCII_code s)
{
  if (s == new_line_char)
  {
    if (selector < pseudo)
    {
      print_ln();
      return;
    }
  }

  switch (selector)
  {
    case term_and_log:
      (void) show_char(Xchr(s));
      incr(term_offset);
      (void) putc(Xchr(s), log_file);
      incr(file_offset);

      if (term_offset == max_print_line)
      {
        show_char('\n');
        term_offset = 0;
      }
      
      if (file_offset == max_print_line)
      {
        (void) putc ('\n', log_file);
        file_offset = 0;
      }

      break;

    case log_only:
      (void) putc(Xchr(s), log_file);
      incr(file_offset);

      if (file_offset == max_print_line)
        print_ln();

      break;

    case term_only:
      (void) show_char(Xchr(s));
      incr(term_offset);

      if (term_offset == max_print_line)
        print_ln();

      break;

    case no_print:
      break;

    case pseudo:
      if (tally < trick_count)
        trick_buf[tally % error_line] = s;

      break;

    case new_string:
#ifdef ALLOCATESTRING
      if (pool_ptr + 1 > current_pool_size)
      {
        str_pool = realloc_str_pool (increment_pool_size);
      }
      
      if (pool_ptr < current_pool_size)
      {
        str_pool[pool_ptr]= s;
        incr(pool_ptr);
      }
#else
      if (pool_ptr < pool_size)
      {
        str_pool[pool_ptr]= s;
        incr(pool_ptr);
      }
#endif
      break;

    default:
      (void) putc(Xchr(s), write_file[selector]);
      break;
  }

  incr(tally);
}
/* sec 0059 */
void print_ (integer s)
{
  pool_pointer j;
  integer nl;

  if (s >= str_ptr)
    s = 259; /* ??? */
  else 
  {
    if (s < 256)
    {
      if (s < 0)
        s = 259; /* ??? */
      else
      {
        if (selector > pseudo)
        {
          print_char(s);
          return;
        }

        if ((s == new_line_char))
          if (selector < pseudo)
          {
            print_ln();
            return;
          }
          
          nl = new_line_char;
          new_line_char = -1;
/* translate ansi to dos 850 */
          if (!show_in_hex && s < 256 && s >= 32)
          {
            if (show_in_dos && s > 127)
            {
              if (wintodos[s - 128] > 0)
              {
                print_char (wintodos[s - 128]);
              }
              else
              {
                j = str_start[s];

                while (j < str_start[s + 1])
                {
                  print_char(str_pool[j]);
                  incr(j);
                }
              }
            }
            else
            {
              print_char(s);       /* don't translate to hex */
            }
          }
          else
          {                       /* not just a character */
            j = str_start[s];

            while (j < str_start[s + 1])
            {
              print_char(str_pool[j]);
              incr(j);
            }
          }
          new_line_char = nl; /* restore eol */
          return;
      }
    }
  }
/*  we get here with s > 256 - i.e. not a single character */
  j = str_start[s];

  while (j < str_start[s + 1])
  {
    print_char(str_pool[j]);
    incr(j);
  }
}
/* string version print. */
void print_string_ (unsigned char *s)
{
  while (*s > 0)
    print_char(*s++);
}
/* sec 0060 */
// print string number s from string pool by calling print_
void slow_print_ (integer s)
{
  pool_pointer j;

  if ((s >= str_ptr) || (s < 256))
  {
    print(s);
  }
  else
  {
    j = str_start[s];

    while (j < str_start[s + 1])
    {
      print(str_pool[j]);
      incr(j);
    }
  }
}
/* sec 0062 */
// print newline followed by string number s (unless at start of line)
void print_nl_ (const char * s)
{
  if (((term_offset > 0) && (odd(selector))) ||
      ((file_offset > 0) && (selector >= log_only)))
    print_ln();

  print_string(s);
}
/* sec 0063 */
// print string number s preceded by escape character
void print_esc_ (const char * s)
{
  integer c;

  c = escape_char;

  if (c >= 0)
    if (c < 256)
      print(c);

  print_string(s);
}
/* sec 0064 */
void print_the_digs_ (eight_bits k)
{
  while (k > 0)
  {
    decr(k);

    if (dig[k] < 10)
      print_char('0' + dig[k]);
    else
      print_char('A' + dig[k]);
  }
}
/* sec 0065 */
void print_int_ (integer n)
{
  char k;
  integer m;

  k = 0;

  if (n < 0)
  {
    print_char('-');

    if (n > -100000000L)
      n = - (integer) n;
    else
    {
      m = -1 - n;
      n = m / 10;
      m = (m % 10) + 1;
      k = 1;

      if (m < 10)
        dig[0] = (char) m;
      else
      {
        dig[0] = 0;
        incr(n);
      }
    }
  }

  do
    {
      dig[k] = (char) (n % 10);
      n = n / 10;
      incr(k);
    }
  while (!(n == 0));

  print_the_digs(k);
}
/* sec 0262 */
void print_cs_ (integer p)
{
  if (p < hash_base)
    if (p >= single_base)
      if (p == null_cs)
      {
        print_esc("csname");
        print_esc("endcsname");
        print_char(' ');
      }
      else
      {
        print_esc(""); print(p - single_base);

        if (cat_code(p - single_base) == letter)
          print_char(' ');
      }
    else if (p < active_base)
      print_esc("IMPOSSIBLE.");
    else print(p - active_base);
  else if (p >= undefined_control_sequence)
    print_esc("IMPOSSIBLE.");
  else if ((text(p) >= str_ptr))
    print_esc("NONEXISTENT.");
  else
  {
    print_esc(""); print(text(p));
    print_char(' ');
  }
}
/* sec 0263 */
void sprint_cs_(halfword p)
{ 
  if (p < hash_base)
    if (p < single_base)
      print(p - active_base);
    else if (p < null_cs)
    {
      print_esc(""); print(p - single_base);
    }
    else
    {
      print_esc("csname");
      print_esc("endcsname");
    }
  else
  {
    print_esc(""); print(text(p));
  }
}
/* sec 0518 */
void print_file_name_(integer n, integer a, integer e)
{
  slow_print(a);
  slow_print(n);
  slow_print(e);
}
/* sec 0699 */
void print_size_ (integer s)
{ 
  if (s == 0)
    print_esc("textfont");
  else if (s == 16)
    print_esc("scriptfont");
  else
    print_esc("scriptscriptfont");
} 
/* sec 1355 */
void print_write_whatsit_(str_number s, halfword p)
{
  print_esc(""); print(s);
  if (write_stream(p) < 16)
    print_int(write_stream(p)); 
  else if (write_stream(p) == 16)
    print_char('*');
  else print_char('-');
}
/* sec 0081 */
void jump_out (void) 
{
  close_files_and_terminate();

  {
    int code;

#ifndef _WINDOWS
    fflush(stdout); 
#endif

    ready_already = 0;

    if (trace_flag)
      show_line("EXITING at JUMPOUT\n", 0);

    if ((history != 0) && (history != 1))
      code = 1;
    else
      code = 0;

    uexit(code);
  }
}
/* sec 0082 */
// deal with error by asking for user response 0-9, D, E, H, I, X, Q, R, S
// NOTE: this may JUMPOUT either via X, or because of too many errors
void error (void)
{
  ASCII_code c;
  integer s1, s2, s3, s4;

  if (history < error_message_issued)
    history = error_message_issued;

  print_char('.');
  show_context();

  if (interaction == error_stop_mode)
    while (true)
    {
lab22:
      clear_for_error_prompt();

      { /* prompt_input */
        print_string("? ");
        term_input("? ", help_ptr);
      }

      if (last == first)
        return; // no input

      c = buffer[first];   // analyze first letter typed

      if (c >= 'a')         // uppercase letter first
        c = (unsigned char) (c + 'A' - 'a'); 

      switch (c)
      {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          if (deletions_allowed)
          {
            s1 = cur_tok;
            s2 = cur_cmd;
            s3 = cur_chr;
            s4 = align_state;
            align_state = 1000000L;
            OK_to_interrupt = false;

            if ((last > first + 1) && (buffer[first + 1] >= '0') && (buffer[first + 1] <= '9'))
              c = (unsigned char) (c * 10 + buffer[first + 1] - '0' * 11);
            else
              c = (unsigned char) (c - 48);
            
            while (c > 0)
            {
              get_token();
              decr(c);
            }

            cur_tok = s1;
            cur_cmd = s2;
            cur_chr = s3;
            align_state = s4;
            OK_to_interrupt = true;
            help2("I have just deleted some text, as you asked.",
                "You can now delete more, or insert, or whatever.");
            show_context();
            goto lab22;     /* loop again */
          }
          break;

#ifdef DEBUG
        case 'D':
          {
            debug_help();
            goto lab22;       /* loop again */
          }
          break;
#endif /* DEBUG */
        case 'E':
          if (base_ptr > 0)
          {
            edit_name_start = str_start[input_stack[base_ptr].name_field];
            edit_name_length = length(input_stack[base_ptr].name_field);
            edit_line = line;
            jump_out();
          }
          break;

        case 'H':
          {
            if (use_err_help)
            {
              give_err_help();
              use_err_help = false;
            }
            else
            {
              if (help_ptr == 0)
                help2("Sorry, I don't know how to help in this situation.",
                    "Maybe you should try asking a human?");
              do {
                decr(help_ptr);
                print_string(help_line[help_ptr]);
                print_ln();
              } while (!(help_ptr == 0));
            }

            help4("Sorry, I already gave what help I could...",
                "Maybe you should try asking a human?",
                "An error might have occurred before I noticed any problems.",
                "``If all else fails, read the instructions.''");
            goto lab22; /* loop again */
          }
          break;

        case 'I':
          {
            begin_file_reading();

            if (last > first + 1)
            {
              cur_input.loc_field = first + 1;
              buffer[first] = 32;
            }
            else
            {
              { /* prompt_input */
                print_string("insert>");
                term_input("insert>", 0);
              }
              cur_input.loc_field = first;
            }
            first = last;
            cur_input.limit_field = last - 1;
            return;
          }
          break;

        case 'Q':
        case 'R':
        case 'S':
          {
            error_count = 0; 
            interaction = 0 + c - 81; /* Q = 0, R = 1, S = 2, T = 3 */
            print_string("OK, entering ");

            switch (c)
            {
              case 'Q':
                print_esc("batchmode");
                decr(selector);
                break;
              case 'R':
                print_esc("nonstopmode");
                break;
              case 'S':
                print_esc("scrollmode");
                break;
            }

            print_string("...");
            print_ln();
#ifndef _WINDOWS
            fflush(stdout);
#endif
            return;
          }
          break;

        case 'X':
          {
            interaction = 2;
            jump_out();
          }
          break;

        default:
          break;
      }           /* end of switch analysing response character */

      {
        print_string("Type <return> to proceed, S to scroll future error messages,");
        print_nl("R to run without stopping, Q to run quietly,");
        print_nl("I to insert something, ");

        if (base_ptr > 0)
          print_string("E to edit your file,");

        if (deletions_allowed)
          print_nl("1 or ... or 9 to ignore the next 1 to 9 tokens of input,");

        print_nl("H for help, X to quit.");
      }
    } /* end of while(true) loop */

  incr(error_count);

  if (error_count == 100)
  {
    print_nl("(That makes 100 errors; please try again.)");
    history = 3;
    jump_out();
  }

  if (interaction > batch_mode)
    decr(selector);

  if (use_err_help)
  {
    print_ln();
    give_err_help();
  }
  else while(help_ptr > 0)
  {
    decr(help_ptr);
    print_nl(help_line[help_ptr] == NULL ? "" : help_line[help_ptr]);
  }

  print_ln();

  if (interaction > batch_mode)
    incr(selector);
  
  print_ln();
}
/* sec 0093 */
void fatal_error_(char * s)
{
  normalize_selector();
  print_err("Emergency stop");
  help1(s);
  succumb();
}
/* sec 0094 */
void overflow_(char * s, integer n)
{
  normalize_selector();
  print_err("TeX capacity exceeded, sorry [");
  print_string(s);
  print_char('=');
  print_int(n);
  print_char(']');
  help2("If you really absolutely need more capacity,",
      "you can ask a wizard to enlarge me.");

  if (!knuth_flag)
  {
    if (!strcmp(s, "pattern memory") && n == trie_size)
    {
      sprintf(log_line, "\n  (Maybe use -h=... on command line in ini-TeX)\n");
      show_line(log_line, 0);
    }
    else if (!strcmp(s, "exception dictionary") && n == hyphen_prime)
    {
      sprintf(log_line, "\n  (Maybe use -e=... on command line in ini-TeX)\n");
      show_line(log_line, 0);
    }
  }

  succumb();
}
/* sec 0095 */
void confusion_(char * s)
{
  normalize_selector();

  if (history < error_message_issued)
  {
    print_err("This can't happen (");
    print_string(s);
    print_char(')');
    help1("I'm broken. Please show this to someone who can fix can fix");
  }
  else
  {
    print_err("I can't go on meeting you like this");
    help2("One of your faux pas seems to have wounded me deeply...",
        "in fact, I'm barely conscious. Please fix it and try again.");
  }

  succumb();
}
/* sec 0037 */
bool init_terminal (void)
{
  int flag;

  t_open_in();

  if (last > first)
  {
    cur_input.loc_field = first;

    while((cur_input.loc_field < last) && (buffer[cur_input.loc_field]== ' '))
      incr(cur_input.loc_field);    // step over initial white space

    if (cur_input.loc_field < last)
      return true;
  }

// failed to find input file name
  while (true)
  {
#ifdef _WINDOWS
    flag = ConsoleInput("**", "Please type a file name or a control sequence\r\n(or ^z to exit)", (char *) &buffer[first]);
    last = first + strlen((char *) &buffer[first]); /* -1 ? */
//    may need to be more elaborate see input_line in texmf.c
#else
    (void) fputs("**", stdout);
    fflush(stdout);
    flag = input_ln(stdin, true);
#endif

    if (!flag)
    {
      show_char('\n');
      show_line("! End of file on the terminal... why?\n", 1);
      return false;
    }

    cur_input.loc_field = first;

    while ((cur_input.loc_field < last) && (buffer[cur_input.loc_field]== ' '))
      incr(cur_input.loc_field);    // step over intial white space

    if (cur_input.loc_field < last)
      return true;

    sprintf(log_line, "%s\n", "Please type the name of your input file.");
    show_line(log_line, 1);
  }
}
/* sec 0043 */
// Make string from str_start[str_ptr] to pool_ptr
str_number make_string (void)
{
#ifdef ALLOCATESTRING
  if (str_ptr == current_max_strings)
    str_start = realloc_str_start(increment_max_strings);

  if (str_ptr == current_max_strings)
  {
    overflow("number of strings", current_max_strings - init_str_ptr); /* 97/Mar/9 */
    return 0;     // abort_flag set
  }
#else
  if (str_ptr == max_strings)
  {
    overflow("number of strings", max_strings - init_str_ptr);
    return 0;     // abort_flag set
  }
#endif
  incr(str_ptr);
  str_start[str_ptr] = pool_ptr;

  return (str_ptr - 1);
}
/* sec 0044 */
bool str_eq_buf_ (str_number s, integer k)
{
  register bool Result;
  pool_pointer j;
  bool result;

  j = str_start[s];

  while (j < str_start[s + 1])
  {
    if (str_pool[j] != buffer[k])
    {
      result = false;
      goto lab45;
    }

    incr(j);
    incr(k);
  }
  result = true;
lab45:
  Result = result;
  return Result;
}
/* sec 0045 */
bool str_eq_str_ (str_number s, str_number t)
{
  register bool Result;
  pool_pointer j, k;
  bool result;

  result = false;

  if (length(s) != length(t))
    goto lab45;

  j = str_start[s];
  k = str_start[t];

  while (j < str_start[s + 1])
  {
    if (str_pool[j] != str_pool[k])
      goto lab45;

    incr(j);
    incr(k);
  }
  result = true;
lab45:
  Result = result;
  return Result;
}
/* sec 0066 */
void print_two_(integer n)
{ 
  n = abs(n) % 100;
  print_char('0' + (n / 10));
  print_char('0' + (n % 10));
} 
/* sec 0067 */
void print_hex_(integer n)
{
  char k;

  k = 0;
  print_char('"');

  do
    {
      dig[k] = (unsigned char) (n % 16);
      n = n / 16;
      incr(k);
    }
  while (!(n == 0));

  print_the_digs(k);
}
/* sec 0069 */
void print_roman_int_(integer n)
{
  pool_pointer j, k;
  nonnegative_integer u, v;

  j = str_start[260]; /*  m2d5c2l5x2v5i */
  v = 1000;

  while (true)
  {
    while (n >= v)
    {
      print_char(str_pool[j]);
      n = n - v;
    }

    if (n <= 0)
      return;

    k = j + 2;
    u = v / (str_pool[k - 1] - '0');

    if (str_pool[k - 1] == 50)
    {
      k = k + 2;
      u = u / (str_pool[k - 1] - '0');
    }

    if (n + u >= v)
    {
      print_char(str_pool[k]);
      n = n + u;
    }
    else
    {
      j = j + 2;
      v = v / (str_pool[j - 1] - '0');
    }
  }
}
/* sec 0070 */
void print_current_string (void)
{
  pool_pointer j;

  j = str_start[str_ptr];

  while (j < pool_ptr)
  {
    print_char(str_pool[j]);
    incr(j);
  }
}

int stringlength (int str_ptr)
{
  return (str_start[str_ptr + 1] - str_start[str_ptr]) + 2;
}

char * add_string (char *s, char * str_string)
{
  int n;

  n = strlen(str_string);
  memcpy(s, &str_string, n);
  s += n;
  strcpy(s, "\r\n");
  s += 2;

  return s;
}

int addextrahelp = 1;

// make one long \r\n separated string out of help lines 
// str_pool is packed_ASCII_code *

char * make_up_help_string (int nhelplines)
{
  char * helpstring, *s;
  int k, nlen = 0;
  
//  get length of help for this specific message
  for (k = nhelplines - 1; k >= 0; k--)
  {
    nlen += strlen(help_line[k]);
  }

  nlen += 2; // for blank line separator: "\r\n"

  if (addextrahelp)
  {
    nlen += stringlength(265);
    nlen += stringlength(266);
    nlen += stringlength(267);

    if (base_ptr > 0)
      nlen += stringlength(268);

    if (deletions_allowed)
      nlen += stringlength(269);

    nlen += stringlength(270);
  }

  helpstring = (char *) malloc(nlen + 1); // +1 = '\0'
  s = helpstring;

  for (k = nhelplines-1; k >= 0; k--)
  {
    s = add_string(s, help_line[k]);
  }

  if (addextrahelp)
  {
    strcpy(s, "\r\n");
    s += 2;
    s = add_string(s, "Type <return> to proceed, S to scroll future error messages,");
    s = add_string(s, "R to run without stopping, Q to run quietly,");
    s = add_string(s, "I to insert something, ");

    if (base_ptr > 0)
      s = add_string(s, "E to edit your file, ");

    if (deletions_allowed)
      s = add_string(s, "1 or ... or 9 to ignore the next 1 to 9 tokens of input,");

    s = add_string(s, "H for help, X to quit.");
  }

  return helpstring;
}

char * make_up_query_string (int promptstr)
{
  char *querystr;
  int nstart, nnext, n;
  char *s;

  nstart = str_start[ promptstr];
  nnext = str_start[ promptstr + 1];
  n = nnext - nstart;
  querystr = (char *) malloc(n + 1);
  s = querystr;
  memcpy(s, &str_pool[nstart], n);  
  s += n;
  *s = '\0';

  return querystr;
}

// abort_flag set if input_line / ConsoleInput returns non-zero
// should set interrupt instead ???
// called from tex0.c, tex2.c, tex3.c
/* sec 0071 */
// void term_input(void)
void term_input (char * term_str, int term_help_lines)
{ 
  integer k;
  int flag;
  char * helpstring = NULL;
#ifdef _WINDOWS
  char * querystring = NULL;
#endif
//  if (nhelplines != 0) {
//    helpstring = make_up_help_string (nhelplines);
//    printf(helpstring);
//    free(helpstring);
//  }
  show_line("\n", 0);    // force it to show what may be buffered up ???
  helpstring = NULL;  

#ifdef _WINDOWS
  if (term_str != NULL)
    querystring = term_str;

  if (term_help_lines != NULL)
    helpstring = make_up_help_string(term_help_lines);

  if (helpstring == NULL && querystring != NULL)
  {
    if (strcmp(querystring, ": ") == 0)
      helpstring = xstrdup("Please type another file name (or ^z to exit):");
    else if (strcmp(querystring, "=>") == 0)    // from firm_up_the_line
      helpstring = xstrdup("Please type <enter> to accept this line\r\nor type a replacement line");
    else if (strcmp(querystring, "insert>") == 0) // from error() after "I"
      helpstring = xstrdup("Please type something to insert here");
    else if (strcmp(querystring, "") == 0)      // from read_toks
      helpstring = xstrdup("Please type a control sequence");
    else if (strcmp(querystring, "= ") == 0)    // from read_toks
      helpstring = xstrdup("Please type a token");
    else if (strcmp(querystring, "*") == 0)   // get_next
      helpstring = xstrdup("Please type a control sequence\r\n(or ^z to exit)");
  }

  flag = ConsoleInput(querystring, helpstring, (char *) &buffer[first]);  // ???
//  flag == 0 means trouble --- EOF on terminal
  if (querystring != NULL)
    free(querystring);

  if (helpstring != NULL)
    free(helpstring);

  helpstring = querystring = NULL;

  last = first + strlen((char *) &buffer[first]); /* -1 ? */
//  flag = (last > first);
//  may need to be more elaborate see input_line in texmf.c ???
//  sprintf(log_line, "first %d last %d flag %d - %s",
//      first, last, flag, (char *) &buffer[first]);
//  winerror(log_line);
#else
  fflush(stdout);
  flag = input_ln(stdin, true);
#endif
  if (!flag)
  {
    fatal_error("End of file on the terminal!");
    return;         // abort_flag set
  }
  term_offset = 0;
#ifdef _WINDOWS
// echo what was typed into Console buffer also
  if (last != first)
    for (k = first; k <= last - 1; k++)
      print(buffer[k]);
  print_ln();
#else
  decr(selector);     // shut off echo

  if (last != first)
    for (k = first; k <= last - 1; k++)
      print(buffer[k]);

  print_ln();
  incr(selector);     // reset selector again
#endif
}
/* sec 0091 */
void int_error_ (integer n)
{
  print_string(" (");
  print_int(n);
  print_char(')');
  error();
}
/* sec 0092 */
void normalize_selector (void)
{
  if (log_opened)
    selector = term_and_log;
  else
    selector = term_only;

  if (job_name == 0)
    open_log_file();

  if (interaction == batch_mode)
    decr(selector);
}
/* sec 0098 */
void pause_for_instructions (void)
{
   if (OK_to_interrupt)
   {
    interaction = error_stop_mode;

    if ((selector == log_only) || (selector == no_print))
      incr(selector);

    print_err("Interruption");
    help3("You rang?",
        "Try to insert some instructions for me (e.g.,`I\\showlists'),",
        "unless you just want to quit by typing `X'.");
    deletions_allowed = false;
    error();
    deletions_allowed = true;
    interrupt = 0;
  }
}
/* sec 0100 */
integer half_(integer x)
{
  if (odd(x))
    return ((x + 1) / 2);
  else
    return (x / 2);
}
/* sec 0102 */
scaled round_decimals_(small_number k)
{
  integer a;

  a = 0;

  while (k > 0)
  {
    decr(k);
    a = (a + dig[k] * 131072L) / 10; /* 2^17 */
  }
  
  return ((a + 1) / 2);
}
/* sec 0103 */
/* This has some minor speedup changes - no real advantage probably ... */
void print_scaled_(scaled s)
{
  scaled delta;

  if (s < 0)
  {
    print_char('-');
    s = - (integer) s;
  }

  print_int(s / 65536L);
  print_char('.');
  s = 10 * (s % 65536L) + 5;
  delta = 10;

  do
    {
      if (delta > 65536L)
        s = s - 17232; /* 2^15 - 50000 - rounding */
      print_char('0' + (s / 65536L));
      s = 10 * (s % 65536L);
      delta = delta * 10;
    }
  while (!(s <= delta));
}
/* sec 0105 */
scaled mult_and_add_(integer n, scaled x, scaled y, scaled maxanswer)
{
  if (n < 0)
  {
    x = - (integer) x;
    n = - (integer) n;
  }

  if (n == 0)
    return y;
  else if (((x <= (maxanswer - y) / n) && (- (integer) x <= (maxanswer + y) / n)))
    return (n * x + y); 
  else
  {
    arith_error = true;
    return 0;
  }
}
/* sec 0106 */
scaled x_over_n_(scaled x, integer n)
{
  register scaled Result;
  bool negative;

  negative = false;

  if (n == 0)
  {
    arith_error = true;
    Result = 0;
    tex_remainder = x;
  }
  else
  {
    if (n < 0)
    {
      x = - (integer) x;
      n = - (integer) n;
      negative = true;
    }

    if (x >= 0)
    {
      Result = x / n;
      tex_remainder = x % n;
    }
    else
    {
      Result = - (integer) ((- (integer) x)/ n);
      tex_remainder = - (integer) ((- (integer) x)% n);
    }
  }

  if (negative)
    tex_remainder = - (integer) tex_remainder;

  return Result;
}
/* sec 0107 */
scaled xn_over_d_(scaled x, integer n, integer d)
{
  register scaled Result;
  bool positive;
  nonnegative_integer t, u, v;

  if (x >= 0)
    positive = true; 
  else
  {
    x = - (integer) x;
    positive = false;
  }

  t = (x % 32767L) * n;
  u = (x / 32768L) * n + (t / 32768L);
  v = (u % d) * 32768L + (t % 32768L); 

  if (u / d >= 32768L)
    arith_error = true; 
  else
    u = 32768L * (u / d) + (v / d);

  if (positive)
  {
    Result = u;
    tex_remainder = v % d;
  }
  else
  {
    Result = - (integer) u;
    tex_remainder = - (integer)(v % d);
  }

  return Result;
}
/* sec 0108 */
halfword badness_(scaled t, scaled s)
{
  integer r;

  if (t == 0)
    return 0;
  else if (s <= 0)
    return 10000;
  else
  {
    if (t <= 7230584L)
      r = (t * 297) / s;
    else if (s >= 1663497L)
      r = t / (s / 297);
    else
      r = t;

    if (r > 1290)
      return 10000; 
    else
      return (r * r * r + 131072L) / 262144L;  /* 2^17 */
  }
}
/* sec 0114 */
#ifdef DEBUG
void print_word_(memory_word w)
{ 
  print_int(w.cint); 
  print_char(' ');
  print_scaled(w.cint); 
  print_char(' ');
  print_scaled(round(65536L * w.gr));
  print_ln();
  print_int(w.hh.v.LH);
  print_char('=');
  print_int(w.hh.b0);
  print_char(':');
  print_int(w.hh.b1);
  print_char(';');
  print_int(w.hh.v.RH);
  print_char(' ');
  print_int(w.qqqq.b0); 
  print_char(':');
  print_int(w.qqqq.b1); 
  print_char(':');
  print_int(w.qqqq.b2); 
  print_char(':');
  print_int(w.qqqq.b3);
} 
/* need this version only if SHORTFONTINFO defined */
void zprintfword(fmemoryword w)
{
  print_int(w.cint);
  print_char(' ');
  print_scaled(w.cint);
  print_char(' ');
  print_scaled(round(65536L * w.gr));
  print_ln();
  print_int(w.hh.v.LH);
  print_char('=');
  print_int(w.hh.b0);
  print_char(':');
  print_int(w .hh.b1);
  print_char(';');
  print_int(w.hh.v.RH);
  print_char(' ');
  print_int(w.qqqq.b0);
  print_char(':');
  print_int(w.qqqq.b1);
  print_char(':');
  print_int(w.qqqq.b2);
  print_char(':');
  print_int(w.qqqq.b3);
}
#endif
/* sec 0292 */
void show_token_list_(integer p, integer q, integer l)
{
  integer m, c;
  ASCII_code match_chr;
  ASCII_code n;

  match_chr = '#';
  n = '0';
  tally = 0;

  while ((p != 0) && (tally < l))
  {
    if (p == q)
    {
      first_count = tally;
      trick_count = tally + 1 + error_line - half_error_line;

      if (trick_count < error_line)
        trick_count = error_line;
    }

    if ((p < hi_mem_min) || (p > mem_end))
    {
      print_esc("CLOBBERED.");
      return;
    }

    if (info(p) >= cs_token_flag)
      print_cs(info(p) - cs_token_flag);
    else
    {
      m = info(p) / 256;
      c = info(p) % 256;

      if (info(p) < 0)
        print_esc("BAD.");
      else
        switch (m)
        {
          case left_brace:
          case right_brace:
          case math_shift:
          case tab_mark:
          case sup_mark:
          case sub_mark:
          case spacer:
          case letter:
          case other_char:
            print(c);
            break;

          case mac_param:
            print(c);
            print(c);
            break;

          case out_param:
            print(match_chr);

            if (c <= 9)
              print_char(c + '0');
            else
            {
              print_char('!');
              return;
            }
            break;

          case match:
            match_chr = (ASCII_code) c;
            print(c);
            incr(n);
            print_char(n);

            if (n > '9')
              return;
            break;

          case end_match:
            print_string("->");
            break;

          default:
            print_esc("BAD.");
            break;
        }
    }
    p = link(p);
  }

  if (p != 0)
    print_esc("ETC.");
}
/* sec 0306 */
void runaway (void)
{
  halfword p;

  if (scanner_status > 1)
  {
    print_nl("Runaway ");

    switch (scanner_status)
    {
      case defining:
        print_string("definition");
        p = def_ref;
        break;

      case matching:
        print_string("argument");
        p = temp_head;
        break;

      case aligning:
        print_string("preamble");
        p = hold_head;
        break;

      case absorbing:
        print_string("text");
        p = def_ref;
        break;
    }

    print_char('?');
    print_ln();
    show_token_list(link(p), 0, error_line - 10); 
  }
}
/* sec 0120 */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* first try list of available nodes (avail != NULL)                   */
/* then see if can go upwards (mem_end < mem_max)                      */
/* then see if can go downwards (hi_mem_min > lo_mem_max)              */
/* if not, extend memory at the top and grab from there --- new        */
/* else fail ! paragraph 120                                           */
/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
halfword get_avail (void)
{
  halfword p;

  p = avail;

  if (p != 0)
    avail = link(avail);
  else if (mem_end < mem_max)
  {
    incr(mem_end);
    p = mem_end;
  }
  else
  {
    decr(hi_mem_min);
    p = hi_mem_min;

    if (hi_mem_min <= lo_mem_max) /* have we run out in middle ? */
    {
      incr(hi_mem_min);
      mem = realloc_main (0, mem_top / 2);  /* zzzaa = zmem = mem */

      if (mem == NULL)
        return 0;

      if (mem_end >= mem_max)
      {
        runaway();
        overflow("main memory size", mem_max + 1 - mem_min);
        return 0;           // abort_flag set
      }

      incr(mem_end);        /* then grab from new area */
      p = mem_end;          /* 1993/Dec/14 */
    }
  }

  link(p) = 0;       /* link(p) = null !!! */

#ifdef STAT
  incr(dyn_used); 
#endif /* STAT */

  return p; 
} 
/* sec 0123 */
void flush_list_(halfword p)          /* paragraph 123 */
{ 
  halfword q, r;

  if (p != 0)              /* null !!! */
  {
    r = p;

    do
      {
        q = r;
        r = link(r);
#ifdef STAT
        decr(dyn_used);
#endif /* STAT */
      }
    while (!(r == 0));     /* r != null */

    link(q) = avail;
    avail = p;
  }
}
/* sec 0125 */
halfword get_node_(integer s)
{
  register halfword Result;
  halfword p;
  halfword q;
  integer r;
  integer t;
lab20:

  p = rover;

  do
    {
      q = p + node_size(p);

      while ((mem[q].hh.v.RH == empty_flag))
      {
        t = rlink(q);

        if (q == rover)
          rover = t;

        llink(t) = llink(q);
        rlink(llink(q)) = t;
        q = q + node_size(q);
      }

      r = q - s;

      if (r > toint(p + 1)) 
      {
        node_size(p) = r - p;
        rover = p;
        goto lab40;
      }

      if (r == p)
        if (rlink(p) != p)
        {
          rover = rlink(p);
          t = llink(p);
          llink(rover) = t;
          rlink(t) = rover;
          goto lab40;
        }

      node_size(p) = q - p;
      p = rlink(p);
    }
  while (!(p == rover));

  if (s == 1073741824L)    /* 2^30 - special case - merge adjacent */
  {
    Result = max_halfword;

    if (trace_flag)
      show_line("Merged adjacent multi-word nodes\n", 0);

    return Result;
  }

/*  maybe try downward epxansion first instead ? */
  if (lo_mem_max + 2 < hi_mem_min)
    if (lo_mem_max + 2 <= mem_bot + max_halfword)  /* silly ? flush 93/Dec/16 */
    {
      /* if (hi_mem_min - lo_mem_max >= 1998) */
      if (hi_mem_min - lo_mem_max >= (block_size + block_size - 2))
        /* t = lo_mem_max + 1000; */
        t = lo_mem_max + block_size;
      else
        t = lo_mem_max + 1 + (hi_mem_min - lo_mem_max) / 2;

      p = llink(rover);
      q = lo_mem_max;
      rlink(p) = q;
      llink(rover) = q;

      if (t > mem_bot + max_halfword)
        t = mem_bot + max_halfword;     /* silly ? flush 93/Dec/16 */

      rlink(q) = rover;
      llink(q) = p;
      link(q) = empty_flag;
      node_size(q) = t - lo_mem_max; /* block size */
      lo_mem_max = t;
      link(lo_mem_max) = 0;
      info(lo_mem_max) = 0;
      rover = q;
      goto lab20;
    }

/* *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** */
/* we've run out of space in the middle for variable length blocks */
/* try and add new block from below mem_bot *//* first check if space ! */
  if (mem_min - (block_size + 1) <= mem_start) /* extend lower memory downwards */
  {
    mem = realloc_main (mem_top/2 + block_size, 0);  /* zzzaa = zmem = mem */

    if (mem == NULL)
    {
      return 0;
    }
  }

  if (mem_min - (block_size + 1) <= mem_start) /* check again */
  {
    if (trace_flag)
    {
      sprintf(log_line, "mem_min %d, mem_start %d, block_size %d\n", mem_min, mem_start, block_size);
      show_line(log_line, 0);
    }

    overflow("main memory size", mem_max + 1 - mem_min); /* darn: allocation failed ! */
    return 0;     // abort_flag set
  }
/* avoid function call in following ? */
  add_variable_space (block_size); /* now to be found in itex.c */
  goto lab20;         /* go try get_node again */

lab40:
  link(r) = 0;

#ifdef STAT
  var_used = var_used + s; 
#endif /* STAT */

  Result = r; 
  return Result; 
} 
/* sec 0130 */
void free_node_(halfword p, halfword s)
{ 
  halfword q;

  node_size(p) = s;
  link(p) = empty_flag;
  q = llink(rover);
  llink(p) = q;
  rlink(p) = rover;
  llink(rover) = p;
  rlink(q) = p;
#ifdef STAT
  var_used = var_used - s; 
#endif /* STAT */
}
/* sec 0136 */
halfword new_null_box (void) 
{
  halfword p;

  p = get_node(box_node_size);
  type(p) = hlist_node;
  subtype(p) = min_quarterword;
  width(p) = 0;
  depth(p) = 0;
  height(p) = 0;
  shift_amount(p) = 0;
  list_ptr(p) = 0;
  glue_sign(p) = normal;
  glue_order(p) = normal;
  glue_set(p) = 0.0;

  return p;
}
/* sec 0139 */
halfword new_rule (void) 
{
  halfword p;

  p = get_node(rule_node_size);
  type(p) = rule_node;
  subtype(p) = 0;
  width(p) = null_flag;
  depth(p) = null_flag;
  height(p) = null_flag;

  return p;
}
/* sec 0144 */
halfword new_ligature_(quarterword f, quarterword c, halfword q)
{
  halfword p;

  p = get_node(small_node_size);
  type(p) = ligature_node;
  font(lig_char(p)) = f;
  character(lig_char(p)) = c;
  lig_ptr(p) = q;
  subtype(p) = 0;

  return p;
}
/* sec 0144 */
halfword new_lig_item_(quarterword c)
{
  halfword p;

  p = get_node(small_node_size);
  character(p) = c;
  lig_ptr(p) = 0;

  return p;
}
/* sec 0145 */
halfword new_disc (void) 
{
  halfword p;

  p = get_node(small_node_size);
  type(p) = disc_node;
  replace_count(p) = 0;
  pre_break(p) = 0;
  post_break(p) = 0;

  return p;
}
/* sec 0147 */
halfword new_math_(scaled w, small_number s)
{
  halfword p;

  p = get_node(small_node_size);
  type(p) = math_node;
  subtype(p) = s;
  width(p) = w;

  return p;
}
/* sec 0151 */
halfword new_spec_(halfword p)
{
  halfword q;

  q = get_node(glue_spec_size);
  mem[q] = mem[p];
  glue_ref_count(q) = 0;
  width(q) = width(p);
  stretch(q) = stretch(p);
  shrink(q) = shrink(p);

  return q;
}
/* se 0152 */
halfword new_param_glue_(small_number n)
{
  halfword p;
  halfword q;

  p = get_node(small_node_size);
  type(p) = glue_node;
  subtype(p) = n + 1;
  leader_ptr(p) = 0;
  q = glue_par(n);
  glue_ptr(p) = q;
  incr(glue_ref_count(q));

  return p;
}
/* sec 0153 */
halfword new_glue_(halfword q)
{
  halfword p;

  p = get_node(small_node_size);
  type(p) = glue_node;
  subtype(p) = normal;
  leader_ptr(p) = 0; 
  glue_ptr(p) = q;
  incr(glue_ref_count(q));

  return p;
}
/* sec 0154 */
halfword new_skip_param_(small_number n)
{
  halfword p;

  temp_ptr = new_spec(glue_par(n));
  p = new_glue(temp_ptr); 
  glue_ref_count(temp_ptr) = 0;
  subtype(p) = n + 1;

  return p;
}
/* sec 0155 */
halfword new_kern_(scaled w)
{
  halfword p;

  p = get_node(small_node_size);
  type(p) = kern_node;
  subtype(p) = normal;
  width(p) = w;

  return p;
}
/* sec 0158 */
halfword new_penalty_(integer m)
{
  halfword p;

  p = get_node(small_node_size);
  type(p) = penalty_node;
  subtype(p) = 0;
  penalty(p) = m;

  return p;
}

#ifdef DEBUG
/* sec 0167 */
void check_mem_(bool printlocs)
{
  halfword p, q;
  bool clobbered;

  for (p = mem_min; p <= lo_mem_max; p++) freearr[p] = false;
  for (p = hi_mem_min; p <= mem_end; p++) freearr[p] = false;
  p = avail;
  q = 0;
  clobbered = false;
  while (p != 0) {
    if ((p > mem_end) || (p < hi_mem_min))
      clobbered = true;
    else if (freearr[p])
      clobbered = true;

    if (clobbered)
    {
      print_nl("AVAIL list clobbered at ");
      print_int(q);
      goto lab31;
    }
    freearr[p] = true;
    q = p;
    p = link(q);
  }
lab31:;
  p = rover;
  q = 0;
  clobbered = false;
  do {
      if ((p >= lo_mem_max) || (p < mem_min))
        clobbered = true;
      else if ((rlink(p) >= lo_mem_max) || (rlink(p) < mem_min))
        clobbered = true;
      else if (!(is_empty(p)) || (node_size(p) < 2) ||
          (p + node_size(p) > lo_mem_max) || (llink(rlink(p)) != p))
        clobbered = true;
      
      if (clobbered)
      {
        print_nl("Double-AVAIL list clobbered at ");
        print_int(q);
        goto lab32;
      }

      for (q = p; q <= p + node_size(p) - 1; q++)
      {
        if (freearr[q])
        {
          print_nl("Doubly free location at ");
          print_int(q);
          goto lab32;
        }
        freearr[q]= true;
      }
      q = p;
      p = rlink(p);
  } while (!(p == rover));
lab32:;
  p = mem_min;
  while (p <= lo_mem_max) {
    if (is_empty(p))
    {
      print_nl("Bad flag at ");
      print_int(p);
    }
    while ((p <= lo_mem_max) && !freearr[p]) incr(p);
    while ((p <= lo_mem_max) && freearr[p]) incr(p);
  }

  if (printlocs)
  {
    print_nl("New busy locs:");

    for (p = mem_min; p <= lo_mem_max; p++)
      if (!freearr[p] && ((p > was_lo_max) || wasfree[p]))
      {
        print_char(' ');
        print_int(p);
      }

    for (p = hi_mem_min; p <= mem_end; p++)
      if (!freearr[p] && ((p < was_hi_min) || (p > was_mem_end) || wasfree[p]))
      {
        print_char(' ');
        print_int(p);
      }
  }

  for (p = mem_min; p <= lo_mem_max; p++) wasfree[p] = freearr[p];
  for (p = hi_mem_min; p <= mem_end; p++) wasfree[p] = freearr[p];

  was_mem_end = mem_end;
  was_lo_max = lo_mem_max;
  was_hi_min = hi_mem_min;
}
#endif /* DEBUG */

#ifdef DEBUG
/* sec 0172 */
void search_mem_(halfword p)
{
  integer q;

  for (q = mem_min; q <= lo_mem_max; q++)
  {
    if (link(q) == p)
    {
      print_nl("LINK(");
      print_int(q);
      print_char(')');
    }
    if (info(q) == p)
    {
      print_nl("INFO(");
      print_int(q);
      print_char(')');
    }
  }

  for (q = hi_mem_min; q <= mem_end; q++)
  {
    if (link(q) == p)
    {
      print_nl("LINK(");
      print_int(q);
      print_char(')');
    }
    if (info(q) == p)
    {
      print_nl("INFO(");
      print_int(q);
      print_char(')');
    }
  }

  for (q = active_base; q <= box_base + 255; q++)
    if (equiv(q) == p)
    {
      print_nl("EQUIV(");
      print_int(q);
      print_char(')');
    }

  if (save_ptr > 0)
    for (q = 0; q <= save_ptr - 1; q++)
    {
      if (equiv_field(save_stack[q]) == p)
      {
        print_nl("SAVE(");
        print_int(q);
        print_char(')');
      }
    }

  for (q = 0; q <= hyphen_prime; q++)
    if (hyph_list[q] == p)
    {
      print_nl("HYPH(");
      print_int(q);
      print_char(')');
    }
}
#endif /* DEBUG */
/* sec 0174 */
void short_display_(integer p)
{
  integer n; 

  while (p != 0) {      /* want p != null here bkph 93/Dec/15 !!! NOTE: still not fixed in 3.14159 ! */
     if (is_char_node(p))
     {
       if (p <= mem_end)
       {
         if (font(p) != font_in_short_display)
         {
           if ((font(p) > font_max))
             print_char('*');
           else
           {
             print_esc("");
             print(font_id_text(font(p)));
           }
           print_char(' ');
           font_in_short_display = font(p);
         }
         print(character(p));
       }
     }
     else switch (mem[p].hh.b0)
     {
      case hlist_node:
      case vlist_node:
      case ins_node:
      case whatsit_node:
      case mark_node:
      case adjust_node:
      case unset_node:
        print_string("[]");
        break;
      case rule_node:
        print_char('|');
        break;
      case glue_node:
        if (glue_ptr(p) != 0)
          print_char(' ');
        break;
      case math_node:
        print_char('$');
        break;
      case ligature_node:
        short_display(lig_ptr(p));
        break;
      case disc_node:
        short_display(pre_break(p));
        short_display(post_break(p));
        n = replace_count(p);

        while (n > 0) {
          if (link(p) != 0) /* if link(p)<>null then */
            p = link(p);
          decr(n);
        }
        break;
      default:
        break;
    }
    p = link(p);
  }
}
/* sec 0176 */
void print_font_and_char_ (integer p)
{
  if (p > mem_end)
    print_esc("CLOBBERED.");
  else
  {
    if ((font(p) > font_max))
      print_char('*');
    else
    {
      print_esc("");
      print(font_id_text(font(p)));
    }

    print_char(' ');
    print(character(p));
  }
}
/* sec 0176 */
void print_mark_ (integer p)
{ 
  print_char('{');

  if ((p < hi_mem_min)||(p > mem_end))
    print_esc("CLOBBERED.");
  else
    show_token_list(link(p), 0, max_print_line - 10);

  print_char('}');
}
/* sec 0176 */
void print_rule_dimen_ (scaled d)
{
  if ((d == -1073741824L)) /* - 2^30 */
    print_char('*');
  else
    print_scaled(d);
}
/* sec 0177 */
void print_glue_(scaled d, integer order, char * s)
{
  print_scaled(d); 

  if ((order < normal) || (order > filll))
    print_string("foul");
  else if (order > 0)
  {
    print_string("fil");

    while (order > 1) {
      print_char('l');
      decr(order);
    }
  } else if (*s != '\0')
    print_string(s);
}
/* sec 0178 */
void print_spec_(integer p, char * s)
{
  if ((p < mem_min)||(p >= lo_mem_max)) 
    print_char('*');
  else
  {
    print_scaled(width(p));

    if (*s != '\0')
      print_string(s);

    if (stretch(p) != 0)
    {
      print_string("plus");
      print_glue(stretch(p), stretch_order(p), s);
    }

    if (shrink(p) != 0)
    {
      print_string("minus");
      print_glue(shrink(p), shrink_order(p), s);
    }
  }
}
/* sec 0691 */
void print_fam_and_char_(halfword p)
{
  print_esc("fam");
  print_int(fam(p));
  print_char(' ');
  print(character(p));
}
/* sec 0691 */
void print_delimiter_(halfword p)
{
  integer a;

  a = small_fam(p) * 256 + small_char(p);
  a = a * 4096 + large_fam(p) * 256 + large_char(p);

  if (a < 0)
    print_int(a);
  else
    print_hex(a);
}
/* sec 0692 */
void print_subsidiary_data_(halfword p, ASCII_code c)
{
  if ((pool_ptr - str_start[str_ptr]) >= depth_threshold)
  {
    if (math_type(p) != 0)
      print_string(" []");
  }
  else
  {
    append_char(c);
    temp_ptr = p;

    switch (math_type(p))
    {
      case math_char:
        print_ln();
        print_current_string();
        print_fam_and_char(p);
        break;

      case sub_box:
        show_info();
        break;

      case sub_mlist:
        if (info(p) == 0)
        {
          print_ln();
          print_current_string();
          print_string("{}");
        }
        else
          show_info();
        break;

      default:
        break;
    }

    decr(pool_ptr);
  }
}
/* sec 0694 */
void print_style_(integer c)
{
  switch (c / 2)
  {
    case 0:
      print_esc("displaystyle");
      break;
    case 1:
      print_esc("textstyle");
      break;
    case 2:
      print_esc("scriptstyle");
      break;
    case 3:
      print_esc("scriptscriptstyle");
      break;
    default:
      print_string("Unknown style!");
      break;
  }
}
/* sec 0225 */
void print_skip_param_(integer n)
{
  switch(n)
  {
    case line_skip_code:
      print_esc("lineskip");
      break;

    case baseline_skip_code:
      print_esc("baselineskip");
      break; 

    case par_skip_code:
      print_esc("parskip");
      break;

    case above_display_skip_code:
      print_esc("abovedisplayskip");
      break;

    case below_display_skip_code:
      print_esc("belowdisplayskip");
      break;

    case above_display_short_skip_code:
      print_esc("abovedisplayshortskip");
      break;

    case below_display_short_skip_code:
      print_esc("belowdisplayshortskip");
      break;

    case left_skip_code:
      print_esc("leftskip");
      break;

    case right_skip_code:
      print_esc("rightskip");
      break;

    case top_skip_code:
      print_esc("topskip");
      break;

    case split_top_skip_code:
      print_esc("splittopskip");
      break;

    case tab_skip_code:
      print_esc("tabskip");
      break;

    case space_skip_code:
      print_esc("spaceskip");
      break;

    case xspace_skip_code:
      print_esc("xspaceskip");
      break;

    case par_fill_skip_code:
      print_esc("parfillskip");
      break;

    case thin_mu_skip_code:
      print_esc("thinmuskip");
      break;

    case med_mu_skip_code:
      print_esc("medmuskip");
      break; 

    case thick_mu_skip_code:
      print_esc("thickmuskip");
      break;

    default:
      print_string("[unknown glue parameter!]");
      break;
  }
}
/* sec 0182 */
void show_node_list_(integer p)
{
  integer n;
  real g;

  if (cur_length > depth_threshold)
  {
/*  if (p > 0) */  /* was p>null !!! line 3662 in tex.web */
    if (p != 0)    /* fixed 94/Mar/23 BUG FIX NOTE: still not fixed in 3.14159 ! */
    print_string(" []");
    return; 
  }

  n = 0; 

  while (p != 0) {      /* want p != null - bkph 93/Dec/15 NOTE: still not fixed in 3.14159 ! */
    print_ln(); 
    print_current_string(); 

    if (p > mem_end)
    {
      print_string("Bad link, display aborted.");
      return;
    }

    incr(n);

    if (n > breadth_max)
    {
      print_string("etc.");
      return;
    }

    if ((p >= hi_mem_min))
      print_font_and_char(p);
    else switch (type(p))
    {
      case hlist_node:
      case vlist_node:
      case unset_node:
        {
          if (type(p) == hlist_node)
            print_esc("h");
          else if (type(p) == vlist_node)
            print_esc("v");
          else print_esc("unset");

          print_string("box(");
          print_scaled(height(p));
          print_char('+');
          print_scaled(depth(p));
          print_string(")x");
          print_scaled(width(p));

          if (type(p) == unset_node)
          {
            if (span_count(p) != 0)
            {
              print_string(" (");
              print_int(span_count(p) + 1);
              print_string(" columns)");
            }

            if (glue_stretch(p) != 0)
            {
              print_string(", stretch ");
              print_glue(glue_stretch(p), glue_order(p), "");
            }

            if (glue_shrink(p) != 0)
            {
              print_string(", shrink ");
              print_glue(glue_shrink(p), glue_sign(p), "");
            }
          }
          else
          {
            g = glue_set(p);

            if ((g != 0.0) && (glue_sign(p) != 0))
            {
              print_string(", glue set ");

              if (glue_sign(p) == shrinking)
                print_string("- ");

              if (fabs(g)> 20000.0)
              {
                if (g > 0.0)
                  print_char('>');
                else
                  print_string("< -");

                print_glue(20000 * 65536L, glue_order(p), "");
              }
              else
                print_glue(round(65536L * g), glue_order(p), "");
            }

            if (shift_amount(p) != 0)
            {
              print_string(", shifted ");
              print_scaled(shift_amount(p));
            }
          }

          {
            {
              str_pool[pool_ptr] = 46;
              incr(pool_ptr);
            }
            show_node_list(mem[p + 5].hh.v.RH);
            decr(pool_ptr);
          }
        }
        break;

      case rule_node:
        {
          print_esc("rule(");
          print_rule_dimen(height(p));
          print_char('+');
          print_rule_dimen(depth(p));
          print_string(")x");
          print_rule_dimen(width(p));
        }
        break;

      case ins_node:
        {
          print_esc("insert");
          print_int(subtype(p));
          print_string(", natural size ");
          print_scaled(height(p));
          print_string("; split(");
          print_spec(split_top_ptr(p), "");
          print_char(',');
          print_scaled(depth(p));
          print_string("); float cost ");
          print_int(float_cost(p));
          {
            {
              str_pool[pool_ptr] = 46;
              incr(pool_ptr);
            }
            show_node_list(mem[p + 4].hh.v.LH);
            decr(pool_ptr);
          }
        }
        break;
      case 8:
        switch (subtype(p))
        {
          case open_node:
            {
              print_write_whatsit(1279, p);   /* debug # (-1 to exit): */
              print_char('=');
              print_file_name(open_name(p), open_area(p), open_ext(p));
            }
            break;

          case write_node:
            {
              print_write_whatsit(591, p);  /* write */
              print_mark(write_tokens(p));
            }
            break;

          case close_node:
            print_write_whatsit(1280, p); /* closeout */
            break;

          case special_node:
            {
              print_esc("special");
              print_mark(write_tokens(p));
            }
            break;

          case language_node:
            {
              print_esc("setlanguage");
              print_int(what_lang(p));
              print_string(" (hyphenmin ");
              print_int(what_lhm(p));
              print_char(',');
              print_int(what_rhm(p));
              print_char(')');
            }
            break;

          default:
            print_string("whatsit?");
            break;
        }
        break;

      case glue_node:
        if (subtype(p) >= a_leaders)
        {
          print_esc("");

          if (subtype(p) == c_leaders)
            print_char('c');
          else if (subtype(p) == x_leaders)
            print_char('x');

          print_string("leaders ");

          print_spec(glue_ptr(p), "");
          {
            {
              str_pool[pool_ptr] = 46;
              incr(pool_ptr);
            }
            show_node_list(mem[p + 1].hh.v.RH);
            decr(pool_ptr);
          }
        }
        else
        {
          print_esc("glue");

          if (subtype(p) != normal)
          {
            print_char('(');

            if (subtype(p) < cond_math_glue)
              print_skip_param(subtype(p) - 1);
            else if (subtype(p) == cond_math_glue)
              print_esc("nonscript");
            else print_esc("mskip");

            print_char(')');
          }

          if (subtype(p) != cond_math_glue)
          {
            print_char(' ');

            if (subtype(p) < cond_math_glue)
              print_spec(glue_ptr(p), "");
            else
              print_spec(glue_ptr(p), "mu");
          }
        }
        break;

      case kern_node:
        if (subtype(p) != mu_glue)
        {
          print_esc("kern");

          if (subtype(p) != normal)
            print_char(' ');

          print_scaled(width(p));

          if (subtype(p) == acc_kern)
            print_string(" (for accent)");
        }
        else
        {
          print_esc("mkern");
          print_scaled(width(p));
          print_string("mu");
        }
        break;

      case math_node:
        {
          print_esc("math");

          if (subtype(p) == before)
            print_string("on");
          else
            print_string("off");

          if (width(p) != 0)
          {
            print_string(", surrounded ");
            print_scaled(width(p));
          }
        }
        break;

      case ligature_node:
        {
          print_font_and_char(lig_char(p));
          print_string("(ligature ");

          if (subtype(p) > 1)
            print_char('|');

          font_in_short_display = font(lig_char(p)); 
          short_display(lig_ptr(p));

          if (odd(subtype(p)))
            print_char('|');

          print_char(')');
        }
        break;

      case penalty_node:
        {
          print_esc("penalty ");
          print_int(penalty(p));
        }
        break;

      case disc_node:
        {
          print_esc("discretionary");

          if (replace_count(p) > 0)
          {
            print_string(" replacing ");
            print_int(replace_count(p));
          }

          {
            {
              str_pool[pool_ptr] = 46;
              incr(pool_ptr);
            }
            show_node_list(mem[p + 1].hh.v.LH);
            decr(pool_ptr);
          }
          {
            str_pool[pool_ptr]= 124;
            incr(pool_ptr);
          }
          show_node_list(mem[p + 1].hh.v.RH);
          decr(pool_ptr);
        }
        break;

      case mark_node:
        {
          print_esc("mark");
          print_mark(mark_ptr(p));
        }
        break;

      case adjust_node:
        {
          print_esc("vadjust");
          {
            {
              str_pool[pool_ptr] = 46;
              incr(pool_ptr);
            }
            show_node_list(mem[p + 1].cint);
            decr(pool_ptr);
          }
        }
        break;

      case style_node:
        print_style(subtype(p));
        break;

      case choice_node:
        {
          print_esc("mathchoice");
          append_char('D');
          show_node_list(display_mlist(p));
          decr(pool_ptr);
          append_char('T');
          show_node_list(text_mlist(p));
          decr(pool_ptr);
          append_char('S');
          show_node_list(script_mlist(p));
          decr(pool_ptr);
          append_char('s');
          show_node_list(script_script_mlist(p)); 
          decr(pool_ptr); 
        } 
        break;

      case ord_noad:
      case op_noad:
      case bin_noad:
      case rel_noad:
      case open_noad:
      case close_noad:
      case punct_noad:
      case inner_noad:
      case radical_noad:
      case over_noad:
      case under_noad:
      case vcenter_noad:
      case accent_noad:
      case left_noad:
      case right_noad:
        {
          switch (type(p))
          {
            case ord_noad:
              print_esc("mathord");
              break;

            case op_noad:
              print_esc("mathop");
              break;

            case bin_noad:
              print_esc("mathbin");
              break;

            case rel_noad:
              print_esc("mathrel");
              break;

            case open_noad:
              print_esc("mathopen");
              break;

            case close_noad:
              print_esc("mathclose");
              break;

            case punct_noad:
              print_esc("mathpunct");
              break;

            case inner_noad:
              print_esc("mathinner");
              break;

            case over_noad:
              print_esc("overline");
              break;

            case under_noad:
              print_esc("underline");
              break;

            case vcenter_noad:
              print_esc("vcenter");
              break;

            case radical_noad:
              {
                print_esc("radical");
                print_delimiter(left_delimiter(p));
              }
              break;

            case accent_noad:
              {
                print_esc("accent");
                print_fam_and_char(accent_chr(p));
              }
              break;

            case left_noad:
              {
                print_esc("left");
                print_delimiter(delimiter(p));
              }
              break;

            case right_noad:
              {
                print_esc("right");
                print_delimiter(delimiter(p));
              }
              break;
          }

          if (subtype(p) != normal)
            if (subtype(p) == limits)
              print_esc("limits");
            else
              print_esc("nolimits");

          if (type(p) < left_noad)
            print_subsidiary_data(nucleus(p), '.');

          print_subsidiary_data(supscr(p), '^');
          print_subsidiary_data(subscr(p), '_');
        }
        break;

      case fraction_noad:
        {
          print_esc("fraction, thickness ");

          if (thickness(p) == 1073741824L)  /* 2^30 */
            print_string("= default");
          else
            print_scaled(thickness(p));

          if ((small_fam(left_delimiter(p)) != 0) || (small_char(left_delimiter(p)) != 0) ||
              (large_fam(left_delimiter(p)) != 0) || (large_char(left_delimiter(p)) != 0))
          {
            print_string(", left-delimiter ");
            print_delimiter(left_delimiter(p));
          }

          if ((small_fam(right_delimiter(p)) != 0) || (small_char(right_delimiter(p)) != 0) ||
              (large_fam(right_delimiter(p)) != 0)||(large_char(right_delimiter(p)) != 0))
          {
            print_string(", right-delimiter ");
            print_delimiter(right_delimiter(p));
          }

          print_subsidiary_data(numerator(p), '\\');
          print_subsidiary_data(denominator(p), '/');
        }
        break;

      default:
        print_string("Unknown node type!");
        break;
    }
    p = link(p);
  }
}
