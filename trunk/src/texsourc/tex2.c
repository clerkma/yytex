#ifdef _WINDOWS
  #define NOCOMM
  #define NOSOUND
  #define NODRIVERS
  #define STRICT
  #pragma warning(disable:4115) // kill rpcasync.h complaint
  #include <windows.h>
  #define MYLIBAPI __declspec(dllexport)
#endif

#pragma warning(disable:4996)
#pragma warning(disable:4131) // old style declarator
#pragma warning(disable:4135) // conversion between different integral types 
#pragma warning(disable:4127) // conditional expression is constant

#include <setjmp.h>

#define EXTERN extern

#include "texd.h"

#pragma warning(disable:4244)       /* 96/Jan/10 */

/* following bit used to be end of tex1.c */

#ifdef STAT
/* sec 0284 */
void restore_trace_(halfword p, char * s)
{
  begin_diagnostic();
  print_char('{');
  print_string(s);
  print_char(' ');
  show_eqtb(p);
  print_char('}');
  end_diagnostic(false);
}
#endif /* STAT */
/* sec 0281 */
void unsave (void)
{
  halfword p;
  quarterword l;
  halfword t;

  if (cur_level > level_one)
  {
    decr(cur_level);
    while (true) {
      decr(save_ptr);

      if (save_type(save_ptr) == level_boundary)
        goto lab30;

      p = save_index(save_ptr);

      if (save_type(save_ptr) == insert_token)
      {
        t = cur_tok;
        cur_tok = p;
        back_input();
        cur_tok = t;
      }
      else
      {
        if (save_type(save_ptr) == restore_old_value)
        {
          l = save_level(save_ptr);
          decr(save_ptr);
        }
        else
          save_stack[save_ptr] = eqtb[undefined_control_sequence];
        
        if (p < int_base)
          if (eq_level(p) == level_one)
          {
            eq_destroy(save_stack[save_ptr]);
#ifdef STAT
            if (tracing_restores > 0)
              restore_trace(p, "retaining");
#endif /* STAT */
          }
          else
          {
            eq_destroy(eqtb[p]);
            eqtb[p] = save_stack[save_ptr];
#ifdef STAT
            if (tracing_restores > 0)
              restore_trace(p, "restoring");
#endif /* STAT */
          }
        else if (xeq_level[p] != level_one)
        {
          eqtb[p] = save_stack[save_ptr];
          xeq_level[p] = l;     /* l may be used without having been ... */
#ifdef STAT
          if (tracing_restores > 0)
            restore_trace(p, "restoring");
#endif /* STAT */
        }
        else
        {
#ifdef STAT
          if (tracing_restores > 0)
            restore_trace(p, "retaining");
#endif /* STAT */
        }
      }
    }
lab30:
    cur_group = save_level(save_ptr);
    cur_boundary = save_index(save_ptr);
  }
  else
  {
    confusion("curlevel");
    return;       // abort_flag set
  }
}
/* This is where the old tex2.c used to start */
/* sec 0288 */
void prepare_mag (void) 
{
  if ((mag_set > 0) && (mag != mag_set))
  {
    print_err("Incompatible magnification (");
    print_int(mag);
    print_string(");");
    print_nl(" the previous value will be retained");
    help2("I can handle only one magnification ratio per job. So I've",
        "reverted to the magnification you used earlier on this run.");
    int_error(mag_set);
    geq_word_define(int_base + mag_code, mag_set);
  }

  if ((mag <= 0) || (mag > 32768L))
  {
    print_err("Illegal magnification has been changed to 1000");
    help1("The magnification ratio must be between 1 and 32768.");
    int_error(mag);
    geq_word_define(int_base + mag_code, 1000);
  }
  mag_set = mag;
}
/* sec 0295 */
void token_show_ (halfword p)
{
  if (p != 0)
    show_token_list(link(p), 0, 10000000L);
}
/* sec 0296 */
void print_meaning (void) 
{
  print_cmd_chr(cur_cmd, cur_chr);

  if (cur_cmd >= call)
  {
    print_char(':');
    print_ln();
    token_show(cur_chr);
  }
  else if (cur_cmd == top_bot_mark)
  {
    print_char(':');
    print_ln();
    token_show(cur_mark[cur_chr]);
  }
}
/* sec 0299 */
void show_cur_cmd_chr (void)
{ 
  begin_diagnostic();
  print_nl("{");

  if (mode != shown_mode)
  {
    print_mode(mode);
    print_string(": ");
    shown_mode = mode;
  }

  print_cmd_chr(cur_cmd, cur_chr);
  print_char('}');
  end_diagnostic(false);
}
/* sec 0311 */
void show_context (void)
{
  char old_setting;
  integer nn;
  bool bottomline;
  integer i;
  integer j;
  integer l;
  integer m;
  integer n;
  integer p;
  integer q;

  base_ptr = input_ptr;
  input_stack[base_ptr] = cur_input;
  nn = -1;
  bottomline = false;

  while (true)
  {
    cur_input = input_stack[base_ptr];

    if ((cur_input.state_field != 0))
      if ((cur_input.name_field > 17) || (base_ptr == 0))
        bottomline = true;

    if ((base_ptr == input_ptr) || bottomline || (nn < error_context_lines))
    {
      if ((base_ptr == input_ptr) || (cur_input.state_field != token_list) ||
          (cur_input.index_field != backed_up) || (cur_input.loc_field != 0))
      {
        tally = 0;
        old_setting = selector;

        if (cur_input.state_field != 0)
        {
          if (cur_input.name_field <= 17)
            if ((cur_input.name_field == 0))
              if (base_ptr == 0)
                print_nl("<*>");
              else
                print_nl("<insert> ");
            else
            {
              print_nl("<read ");
              if (cur_input.name_field == 17)
                print_char('*');
              else
                print_int(cur_input.name_field - 1);
              print_char('>');
            }
          else
          {
            if (c_style_flag)
            {
              print_ln();
              /* show current input file name - ignore if from terminal */
              if (cur_input.name_field > 17)  /* redundant ? */
                print(cur_input.name_field);

              print_char('(');
              print_int(line); /* line number */
              print_char(')');
              print_char(' ');
              print_char(':');
            }
            else
            {
              print_nl("l.");        /* l. ? 573 ????? 98/Dec/8 check ! */
              print_int(line);      /* line number */
            }
          }

          print_char(' ');

          {
            l = tally;
            tally = 0;
            selector = pseudo;
            trick_count = 1000000L;
          }

          if (buffer[cur_input.limit_field] == end_line_char)
            j = cur_input.limit_field;
          else
            j = cur_input.limit_field + 1;

          if (j > 0)
            for (i = cur_input.start_field; i <= j - 1; i++)
            {
              if (i == cur_input.loc_field)
              {
                first_count = tally;
                trick_count = tally + 1 + error_line - half_error_line;

                if (trick_count < error_line)
                  trick_count = error_line;
              }
              print(buffer[i]);
            }
        }
        else
        {
          switch (cur_input.index_field)
          {
            case parameter:
              print_nl("<argument> ");
              break;

            case u_template:
            case v_template:
              print_nl("<template> ");
              break;

            case backed_up:
              if (cur_input.loc_field == 0)
                print_nl("<recently read> ");
              else
                print_nl("<to be read again> ");
              break;

            case inserted:
              print_nl("<inserted text> ");
              break;

            case macro:
              print_ln();
              print_cs(cur_input.name_field);
              break;

            case output_text:
              print_nl("<output> ");
              break;

            case every_par_text:
              print_nl("<everypar> ");
              break;

            case every_math_text:
              print_nl("<everymath> ");
              break;

            case every_display_text:
              print_nl("<everydisplay> ");
              break;

            case every_hbox_text:
              print_nl("<everyhbox> ");
              break;

            case every_vbox_text:
              print_nl("<everyvbox> ");
              break;

            case every_job_text:
              print_nl("<everyjob> ");
              break;

            case every_cr_text:
              print_nl("<everycr> ");
              break;

            case mark_text:
              print_nl("<mark> ");
              break;

            case write_text:
              print_nl("<write> ");
              break;

            default:
              print_nl("?");
              break;
          }

          {
            l = tally;
            tally = 0;
            selector = pseudo;
            trick_count = 1000000L;
          }

          if (cur_input.index_field < macro)
            show_token_list(cur_input.start_field, cur_input.loc_field, 100000L);
          else
            show_token_list(link(cur_input.start_field), cur_input.loc_field, 100000L);
        }

        selector = old_setting;

        if (trick_count == 1000000L)
        {
          first_count = tally;
          trick_count = tally + 1 + error_line - half_error_line;

          if (trick_count < error_line)
            trick_count = error_line;
        }
        
        if (tally < trick_count)
          m = tally - first_count;
        else
          m = trick_count - first_count;

        if (l + first_count <= half_error_line)
        {
          p = 0;
          n = l + first_count;
        }
        else
        {
          print_string("...");
          p = l + first_count - half_error_line + 3;
          n = half_error_line;
        }

        for (q = p; q <= first_count - 1; q++)
          print_char(trick_buf[q % error_line]);
        
        print_ln();

        for (q = 1; q <= n; q++)
          print_char(' ');

        if (m + n <= error_line)
          p = first_count + m;
        else
          p = first_count +(error_line - n - 3);

        for (q = first_count; q <= p - 1; q++)
          print_char(trick_buf[q % error_line]);

        if (m + n > error_line)
          print_string("...");
        incr(nn);
      }
    }
    else if (nn == error_context_lines)
    {
      print_nl("...");
      incr(nn); 
    }

    if (bottomline)
      goto lab30;

    decr(base_ptr);
  }
lab30:
  cur_input = input_stack[input_ptr];
}
#pragma optimize("g", off)          /* 98/Dec/10 experiment */
/* sec 0323 */
void begin_token_list_ (halfword p, quarterword t)
{
  {
    if (input_ptr > max_in_stack)
    {
      max_in_stack = input_ptr;

#ifdef ALLOCATEINPUTSTACK
      if (input_ptr == current_stack_size)
        input_stack = realloc_input_stack(increment_stack_size);

      if (input_ptr == current_stack_size) /* check again after allocation */
      {
        overflow("input stack size", current_stack_size);
        return;     // abort_flag set
      }
#else
      if (input_ptr == stack_size) /* input stack - not dynamic */
      {
        overflow("input stack size", stack_size);
        return;     // abort_flag set
      }
#endif
    }

    input_stack[input_ptr] = cur_input;
    incr(input_ptr);
  }

  cur_input.state_field = token_list;
  cur_input.start_field = p;
  cur_input.index_field = t;

  if (t >= macro)
  {
    add_token_ref(p);

    if (t == macro)
      cur_input.limit_field = param_ptr;
    else
    {
      cur_input.loc_field = link(p);

      if (tracing_macros > 1)
      {
        begin_diagnostic(); 
        print_nl("");

        switch (t)
        {
          case mark_text:
            print_esc("mark");
            break;

          case write_text:
            print_esc("write");
            break;

          default:
            print_cmd_chr(assign_toks, t + (hash_size + 1307));
            break;
        }

        print_string("->");
        token_show(p);
        end_diagnostic(false);
      }
    }
  }
  else
    cur_input.loc_field = p;
}
#pragma optimize("", on)          /* 98/Dec/10 experiment */
/* sec 0324 */
void end_token_list (void) 
{ 
  if (cur_input.index_field >= backed_up)
  {
    if (cur_input.index_field <= inserted)
      flush_list(cur_input.start_field); 
    else
    {
      delete_token_ref(cur_input.start_field);
      if (cur_input.index_field == macro)
        while (param_ptr > cur_input.limit_field)
        {
          decr(param_ptr);
          flush_list(param_stack[param_ptr]);
        }
    }
  }
  else if (cur_input.index_field == u_template)
    if (align_state > 500000L)
      align_state = 0;
    else
    {
      fatal_error("(interwoven alignment preambles are not allowed)");
      return;     // abort_flag set
    }

  {
    decr(input_ptr);
    cur_input = input_stack[input_ptr];
  }

  {
    if (interrupt != 0)
    {
      pause_for_instructions();
    }
  }
}
/* sec 0325 */
void back_input (void)
{
  halfword p;

  while ((cur_input.state_field == 0) && (cur_input.loc_field == 0) &&
      (cur_input.index_field != v_template))
  {
    end_token_list();
  }

  p = get_avail();
  info(p) = cur_tok;

  if (cur_tok < right_brace_limit)
    if (cur_tok < left_brace_limit)
      decr(align_state);
    else
      incr(align_state);

  {
    if (input_ptr > max_in_stack)
    {
      max_in_stack = input_ptr;
#ifdef ALLOCATEINPUTSTACK
      if (input_ptr == current_stack_size)
        input_stack = realloc_input_stack(increment_stack_size);

      if (input_ptr == current_stack_size) /* check again after allocation */
      {
        overflow("input stack size", current_stack_size);
        return;     // abort_flag set
      }
#else
      if (input_ptr == stack_size) /* stack size - not dynamic */
      {
        overflow("input stack size", stack_size);
        return;     // abort_flag set
      }
#endif
    }
    input_stack[input_ptr] = cur_input;
    incr(input_ptr);
  }

  cur_input.state_field = token_list;
  cur_input.start_field = p;
  cur_input.index_field = backed_up;
  cur_input.loc_field = p;
}
/* sec 0327 */
void back_error (void)
{
  OK_to_interrupt = false;
  back_input();
  OK_to_interrupt = true;
  error();
}
/* sec 0327 */
void ins_error (void) 
{
  OK_to_interrupt = false;
  back_input();
  cur_input.index_field = inserted;
  OK_to_interrupt = true;
  error();
}
/* sec 0328 */
void begin_file_reading (void)
{
  if (in_open == max_in_open)
  {
    overflow("text input levels", max_in_open); /* text input levels - NOT DYNAMIC */
    return;     // abort_flag set
  }
#ifdef ALLOCATEBUFFER
  if (first == current_buf_size)
    buffer = realloc_buffer(increment_buf_size);

  if (first == current_buf_size) /* check again after allocation */
  {
    overflow("buffer size", current_buf_size);
    return;     // abort_flag set
  }
#else
  if (first == buf_size)
  {
    overflow("buffer size", buf_size);  /* buffer size - not dynamic */
    return;     // abort_flag set
  }
#endif
  incr(in_open);
  if (in_open > high_in_open)     /* 1999 Jan 17 */
    high_in_open = in_open;
  {
    if (input_ptr > max_in_stack)
    {
      max_in_stack = input_ptr;
#ifdef ALLOCATEINPUTSTACK
      if (input_ptr == current_stack_size)
        input_stack = realloc_input_stack(increment_stack_size);
      if (input_ptr == current_stack_size)
      {
        overflow("input stack size", current_stack_size);  /* check again after allocation */
        return;     // abort_flag set
      }
#else
      if (input_ptr == stack_size)
      {
        overflow("input stack size", stack_size);    /* input stack - not dynamic */
        return;     // abort_flag set
      }
#endif
    }
    input_stack[input_ptr] = cur_input;
    incr(input_ptr);
  }
  cur_input.index_field = in_open;
  line_stack[cur_input.index_field] = line;
  cur_input.start_field = first;
  cur_input.state_field = 1;
  cur_input.name_field = 0;
}
/* sec 0329 */
void end_file_reading (void)
{
  first = cur_input.start_field;
  line = line_stack[cur_input.index_field];

  if (cur_input.name_field > 17)
    (void) a_close(input_file[cur_input.index_field]);

  {
    decr(input_ptr);
    cur_input = input_stack[input_ptr];
  }
  decr(in_open);
}
/* called only form tex0.c */
/* sec 0330 */
void clear_for_error_prompt (void) 
{
  while ((cur_input.state_field != 0) &&
      (cur_input.name_field == 0) && (input_ptr > 0) &&
      (cur_input.loc_field > cur_input.limit_field))
    end_file_reading();

  print_ln();
}
/* sec 0336 */
void check_outer_validity (void)
{
  halfword p;
  halfword q;

  if (scanner_status != 0)
  {
    deletions_allowed = false;

    if (cur_cs != 0)
    {
      if ((cur_input.state_field == 0) || (cur_input.name_field < 1) || (cur_input.name_field > 17))
      {
        p = get_avail();
        info(p) = cs_token_flag + cur_cs;
        back_list(p);
      }

      cur_cmd = spacer;
      cur_chr = ' ';
    }

    if (scanner_status > skipping)
    {
      runaway();

      if (cur_cs == 0)
        print_err("File ended");
      else
      {
        cur_cs = 0;
        print_err("Forbidden control sequence found");
      }

      print_string(" while scanning ");
      p = get_avail();

      switch (scanner_status)
      {
        case defining:
          print_string("definition");
          info(p) = right_brace_token + '}';
          break;

        case matching:
          print_string("use");
          info(p) = par_token;
          long_state = outer_call;
          break;

        case aligning:
          print_string("preamble");
          info(p) = right_brace_token + '}';
          q = p;
          p = get_avail();
          link(p) = q;
          info(p) = cs_token_flag + frozen_cr; /*96/Jan/10*/
          align_state = -1000000L;
          break;

        case absorbing:
          print_string("text");
          info(p) = right_brace_token + '}';
          break;
      }
      ins_list(p);
      print_string(" of ");
      sprint_cs(warning_index);
      help4("I suspect you have forgotten a `}', causing me",
          "to read past where you wanted me to stop.",
          "I'll try to recover; but if the error is serious,",
          "you'd better type `E' or `X' now and fix your file.");
      error();
    }
    else
    {
      print_err("Incomplete ");
      print_cmd_chr(if_test, cur_if);
      print_string("; all text was ignored after line ");
      print_int(skip_line);
      help3("A forbidden control sequence occurred in skipped text.",
          "This kind of error happens when you say `\\if...' and forget",
          "the matching `\\fi'. I've inserted a `\\fi'; this might work.");

      if (cur_cs != 0)
        cur_cs = 0; 
      else
        help_line[2] = "The file ended while I was skipping conditional text.";

      cur_tok = cs_token_flag + frozen_fi; /* 96/Jan/10 */
      ins_error();
  }
    deletions_allowed = true;
  }
}
/*****************************************************************************/
/* get_next() moved from here to end for pragma optimize reasons 96/Sep/12 */
void get_next(void);
/*****************************************************************************/
/* sec 0363 */
void firm_up_the_line (void)
{
  integer k;

  cur_input.limit_field = last;

  if (pausing > 0)
    if (interaction > nonstop_mode)
    {
      ;
      print_ln();

      if (cur_input.start_field < cur_input.limit_field)
        for (k = cur_input.start_field; k <= cur_input.limit_field - 1; k++)
          print(buffer[k]);

      first = cur_input.limit_field;

      {
        ;
        print_string("=>");
        term_input("=>", 0);
      }

      if (last > first)
      {
        for (k = first; k <= last - 1; k++)
          buffer[k + cur_input.start_field - first] = buffer[k];

        cur_input.limit_field = cur_input.start_field + last - first;
      }
    }
}
/* sec 0365 */
void get_token (void)
{ 
  no_new_control_sequence = false;
  get_next();
  no_new_control_sequence = true;

  if (cur_cs == 0)
    cur_tok = (cur_cmd * 256) + cur_chr;
  else
    cur_tok = cs_token_flag + cur_cs;
}
/* sec 0389 */
void macro_call (void)
{
  halfword r;
  halfword p;
  halfword q;
  halfword s;
  halfword t;
  halfword u, v;
  halfword rbraceptr;
  small_number n;
  halfword unbalance;
  halfword m;
  halfword refcount;
  small_number savescannerstatus;
  halfword savewarningindex;
  ASCII_code match_chr;

  savescannerstatus = scanner_status;
  savewarningindex = warning_index;
  warning_index = cur_cs;
  refcount = cur_chr;
  r = link(refcount);
  n = 0;

  if (tracing_macros > 0)
  {
    begin_diagnostic();
    print_ln();
    print_cs(warning_index);
    token_show(refcount);
    end_diagnostic(false);
  }

  if (info(r) != end_match_token)
  {
    scanner_status = matching;
    unbalance = 0;
    long_state = eq_type(cur_cs);

    if (long_state >= outer_call)
      long_state = long_state - 2;

    do
      {
        link(temp_head) = 0;

        if ((info(r) > match_token + 255) || (info(r) < match_token))
          s = 0;
        else
        {
          match_chr = info(r) - match_token;
          s = link(r);
          r = s;
          p = temp_head;
          m = 0;
        }
lab22:
        get_token();

        if (cur_tok == info(r))
        {
          r = link(r);

          if ((info(r) >= match_token) && (info(r) <= end_match_token))
          {
            if (cur_tok < left_brace_limit)
              decr(align_state);

            goto lab40;
          }
          else
            goto lab22;
        }

        if (s != r)
          if (s == 0)
          {
            print_err("Use of ");
            sprint_cs(warning_index);
            print_string(" doesn't match its definition");
            help4("If you say, e.g., `\\def\\a1{...}', then you must always",
              "put `1' after `\\a', since control sequence names are",
              "made up of letters only. The macro here has not been",
              "followed by the required stuff, so I'm ignoring it.");
            error();
            goto lab10;
          }
          else
          {
            t = s;
            do
              {
                {
                  q = get_avail();
                  mem[p].hh.v.RH = q;
                  mem[q].hh.v.LH = mem[t].hh.v.LH;
                  p = q;
                }

                incr(m);
                u = link(t);
                v = s;

                while (true)
                {
                  if (u == r)
                    if (cur_tok != info(v))
                      goto lab30;
                    else
                    {
                      r = link(v);
                      goto lab22;
                    }

                    if (info(u) != info(v))
                      goto lab30;

                    u = link(u);
                    v = link(v);
                }
lab30:
                t = link(t);
              }
            while(!(t == r));

            r = s;
          }

        if (cur_tok == par_token)
          if (long_state != long_call)
          {
            if (long_state == call)
            {
              runaway();
              print_err("Paragraph ended before ");
              sprint_cs(warning_index);
              print_string("was complete");
              help3("I suspect you've forgotten a `}', causing me to apply this",
                  "control sequence to too much text. How can we recover?",
                  "My plan is to forget the whole thing and hope for the best.");
              back_error();
            }

            pstack[n] = link(temp_head);
            align_state = align_state - unbalance;

            for (m = 0; m <= n; m++)
              flush_list(pstack[m]);

            goto lab10;
          }

        if (cur_tok < right_brace_limit)
          if (cur_tok < left_brace_limit)
          {
            unbalance = 1;

            while (true)
            {
              {
                {
                  q = avail;

                  if (q == 0)
                    q = get_avail();
                  else
                  {
                    avail = mem[q].hh.v.RH;
                    mem[q].hh.v.RH = 0;
#ifdef STAT
                    incr(dyn_used);
#endif /* STAT */
                  }
                }

                mem[p].hh.v.RH = q;
                mem[q].hh.v.LH = cur_tok;
                p = q;
              }

              get_token();

              if (cur_tok == par_token)
                if (long_state != long_call)
                {
                  if (long_state == call)
                  {
                    runaway();
                    print_err("Paragraph ended before ");
                    sprint_cs(warning_index);
                    print_string(" was complete");
                    help3("I suspect you've forgotten a `}', causing me to apply this",
                        "control sequence to too much text. How can we recover?",
                        "My plan is to forget the whole thing and hope for the best.");
                    back_error();
                  }

                  pstack[n] = link(temp_head);
                  align_state = align_state - unbalance;

                  for (m = 0; m <= n; m++)
                    flush_list(pstack[m]);
                  goto lab10;
                }

              if (cur_tok < right_brace_limit)
                if (cur_tok < left_brace_limit)
                  incr(unbalance);
                else
                {
                  decr(unbalance);

                  if (unbalance == 0)
                    goto lab31;
                }
            }
lab31:
            rbraceptr = p;

            {
              q = get_avail();
              mem[p].hh.v.RH = q;
              mem[q].hh.v.LH = cur_tok;
              p = q;
            }
          }
          else
          {
            back_input();
            print_err("Argument of ");
            sprint_cs(warning_index);
            print_string(" has an extra }");
            help6("I've run across a `}' that doesn't seem to match anything.",
                "For example, `\\def\\a#1{...}' and `\\a}' would produce",
                "this error. If you simply proceed now, the `\\par' that",
                "I've just inserted will cause me to report a runaway",
                "argument that might be the root of the problem. But if",
                "your `}' was spurious, just type `2' and it will go away.");
            incr(align_state);
            long_state = call;
            cur_tok = par_token;
            ins_error();
            goto lab22;
          }
        else
        {
          if (cur_tok == space_token)
            if (info(r) <= end_match_token)
              if (info(r) >= match_token)
                goto lab22;

          {
            q = get_avail();
            mem[p].hh.v.RH = q;   /* p may be used without having ... */
            mem[q].hh.v.LH = cur_tok;
            p = q;
          }
        }

        incr(m);          /* m may be used without having been ... */

        if (info(r) > end_match_token)
          goto lab22;

        if (info(r) < match_token)
          goto lab22;
lab40:
        if (s != 0)
        {
          if ((m == 1) && (info(p) < right_brace_limit) && (p != temp_head))
          {
            link(rbraceptr) = 0; /* rbraceptr may be used without ... */
            free_avail(p);
            p = link(temp_head);
            pstack[n] = link(p);
            free_avail(p);
          }
          else
            pstack[n] = link(temp_head);

          incr(n);

          if (tracing_macros > 0)
          {
            begin_diagnostic();
            //print_nl(match_chr); /* matchchar may be used without ... */
            print_nl(""); print(match_chr);
            print_int(n);
            print_string("<-");
            show_token_list(pstack[n - 1], 0, 1000);
            end_diagnostic(false);
          }
        }
      }
    while(!(info(r) == end_match_token));
  }

  while((cur_input.state_field == token_list) && (cur_input.loc_field == 0) &&
      (cur_input.index_field != v_template))
    end_token_list();

  begin_token_list(refcount, macro);
  cur_input.name_field = warning_index;
  cur_input.loc_field = link(r);

  if (n > 0)
  {
    if (param_ptr + n > max_param_stack)
    {
      max_param_stack = param_ptr + n;

#ifdef ALLOCATEPARAMSTACK
      if (max_param_stack > current_param_size)
        param_stack = realloc_param_stack(increment_param_size);

      if (max_param_stack > current_param_size) /* check again after allocation */
      {
        overflow("parameter stack size", current_param_size);
        return;     // abort_flag set
      }
#else
      if (max_param_stack > param_size)
      {
        overflow("parameter stack size", param_size); /* parameter stack - not dynamic */
        return;     // abort_flag set
      }
#endif
    }

    for (m = 0; m <= n - 1; m++)
      param_stack[param_ptr + m] = pstack[m];

    param_ptr = param_ptr + n;
  }
lab10:
  scanner_status = savescannerstatus;
  warning_index = savewarningindex;
}
/* sec 0379 */
void insert_relax (void)
{
  cur_tok = cs_token_flag + cur_cs;
  back_input();
  cur_tok = cs_token_flag + frozen_relax;  /* 96/Jan/10 */
  back_input();
  cur_input.index_field = inserted;
}
/* sec 0366 */
void expand (void)
{
  halfword t;
  halfword p, q, r;
  integer j;
  integer cvbackup;
  small_number cvlbackup, radixbackup, cobackup;
  halfword backupbackup;
  small_number savescannerstatus;

  cvbackup = cur_val;
  cvlbackup = cur_val_level;
  radixbackup = radix;
  cobackup = cur_order;
  backupbackup = link(backup_head);

  if (cur_cmd < call)
  {
    if (tracing_commands > 1)
      show_cur_cmd_chr();

    switch (cur_cmd)
    {
      case top_bot_mark:
        if (cur_mark[cur_chr] != 0)
          begin_token_list(cur_mark[cur_chr], mark_text);
        break;

      case expand_after:
        get_token();
        t = cur_tok;
        get_token();

        if (cur_cmd > max_command)
          expand();
        else
          back_input();

        cur_tok = t;
        back_input();
        break;

      case no_expand:
        savescannerstatus = scanner_status;
        scanner_status = normal;
        get_token();
        scanner_status = savescannerstatus;
        t = cur_tok;
        back_input();

        if (t >= cs_token_flag)
        {
          p = get_avail();
          info(p) = cs_token_flag + frozen_dont_expand; /*96/Jan/10*/
          link(p) = cur_input.loc_field;
          cur_input.start_field = p;
          cur_input.loc_field = p;
        }
        break;

      case cs_name:
        r = get_avail();
        p = r;
        do
          {
            get_x_token();
  
            if (cur_cs == 0)
            {
              q = get_avail();
              mem[p].hh.v.RH = q;
              mem[q].hh.v.LH = cur_tok;
              p = q;
            }
          }
        while(!(cur_cs != 0));

        if (cur_cmd != end_cs_name)
        {
          print_err("Missing ");
          print_esc("endcsname");
          print_string(" inserted");
          help2("The control sequence marked <to be read again> should",
              "not appear between \\csname and \\endcsname.");
          back_error();
        }

        j = first;
        p = link(r);

        while (p != 0)
        {
          if (j >= max_buf_stack)
          {
            max_buf_stack = j + 1;

#ifdef ALLOCATEBUFFER
            if (max_buf_stack == current_buf_size)
              buffer = realloc_buffer (increment_buf_size);

            if (max_buf_stack == current_buf_size) /* check again after allocation */
            {
              overflow("buffer size", current_buf_size);
              return;     // abort_flag set
            }
#else
            if (max_buf_stack == buf_size)
            {
              overflow("buffer size", buf_size); /* buffer size - not dynamic */
              return;     // abort_flag set
            }
#endif
          }

          buffer[j] = info(p) % 256;
          incr(j);
          p = link(p);
        }

        if (j > first + 1)
        {
          no_new_control_sequence = false;
          cur_cs = id_lookup(first, j - first);
          no_new_control_sequence = true;
        }
        else if (j == first)
          cur_cs = null_cs;
        else
          cur_cs = single_base + buffer[first];

        flush_list(r);

        if (eq_type(cur_cs) == undefined_cs)
        {
          eq_define(cur_cs, relax, 256);
        }

        cur_tok = cur_cs + cs_token_flag;
        back_input();
        break;

      case convert:
        conv_toks();
        break;

      case the:
        ins_the_toks();
        break;

      case if_test:
        conditional();
        break;

      case fi_or_else:
        if (cur_chr > if_limit)
          if (if_limit == 1)
            insert_relax();
          else
          {
            print_err("Extra ");
            print_cmd_chr(fi_or_else, cur_chr);
            help1("I'm ignoring this; it doesn't match any \\if.");
            error();
          }
        else
        {
          while(cur_chr != fi_code)
            pass_text();

          {
            p = cond_ptr;
            if_line = if_line_field(p);
            cur_if = subtype(p);
            if_limit = type(p);
            cond_ptr = link(p);
            free_node(p, if_node_size);
          }
        }
        break;

      case input:
        if (cur_chr > 0)
          force_eof = true;
        else if (name_in_progress)
          insert_relax();
        else
          start_input();
        break;

      default:
        print_err("Undefined control sequence");
        help5("The control sequence at the end of the top line",
            "of your error message was never \\def'ed. If you have",
            "misspelled it (e.g., `\\hobx'), type `I' and the correct",
            "spelling (e.g., `I\\hbox'). Otherwise just continue,",
            "and I'll forget about whatever was undefined.");
        error();
        break;
    }
  }
  else if (cur_cmd < end_template)
  {
    macro_call();
  }
  else
  {
    cur_tok = cs_token_flag + frozen_endv; /* 96/Jan/10 */
    back_input();
  }

  cur_val = cvbackup;
  cur_val_level = cvlbackup;
  radix = radixbackup;
  cur_order = cobackup;
  link(backup_head) = backupbackup;
}
/* sec 0380 */
void get_x_token (void)
{
lab20:
  get_next();

  if (cur_cmd <= max_command)
    goto lab30;
  if (cur_cmd >= call)
    if (cur_cmd < end_template)
      macro_call();
    else
    {
      cur_cs = frozen_endv;  /* 96/Jan/10 */
      cur_cmd = endv;
      goto lab30;
    }
  else
    expand();

  goto lab20;
lab30:
  if (cur_cs == 0)
    cur_tok = (cur_cmd * 256) + cur_chr;
  else
    cur_tok = cs_token_flag + cur_cs;
}
/* sec 0381 */
void x_token (void)
{
  while (cur_cmd > max_command)
  {
    expand();
    get_next();
  }

  if (cur_cs == 0)
    cur_tok = (cur_cmd * 256) + cur_chr;
  else
    cur_tok = cs_token_flag + cur_cs;
}
/* sec 0403 */
void scan_left_brace (void)
{
  do
    {
      get_x_token();
    }
  while(!((cur_cmd != spacer) && (cur_cmd != relax)));

  if (cur_cmd != left_brace)
  {
    print_err("Missing { inserted");
    help4("A left brace was mandatory here, so I've put one in.",
        "You might want to delete and/or insert some corrections",
        "so that I will find a matching right brace soon.",
        "(If you're confused by all this, try typing `I}' now.)");
    back_error();
    cur_tok = left_brace_token + '{';
    cur_cmd = left_brace;
    cur_chr = '{';
    incr(align_state);
  }
}
/* sec 0405 */
void scan_optional_equals (void)
{
  do
    {
      get_x_token();
    }
  while(!(cur_cmd != spacer));

  if (cur_tok != other_token + '=')
    back_input();
}
/* sec 0407 */
bool scan_keyword_(char * s)
{
  halfword p;
  halfword q;
  char * k;

  p = backup_head;
  link(p) = 0;
  k = s;

  while (*k)
  {
    get_x_token(); 

    if ((cur_cs == 0) && ((cur_chr == (*k)) || (cur_chr == (*k) - 'a' + 'A')))
    {
      {
        q = get_avail();
        mem[p].hh.v.RH = q;
        mem[q].hh.v.LH = cur_tok;
        p = q;
      }

      incr(k);
    }
    else if ((cur_cmd != spacer) || (p != backup_head))
    {
      back_input();

      if (p != backup_head)
        back_list(link(backup_head));

      return false;
    }
  }

  flush_list(link(backup_head));

  return true;
}
/* sec 0408 */
void mu_error (void)
{
  print_err("Incompatible glue units");
  help1("I'm going to assume that 1mu=1pt when they're mixed.");
  error();
}
/* sec 0433 */
void scan_eight_bit_int (void)
{
  scan_int();

  if ((cur_val < 0) || (cur_val > 255))
  {
    print_err("Bad register code");
    help2("A register number must be between 0 and 255.",
        "I changed this one to zero.");
    int_error(cur_val);
    cur_val = 0;
  }
}
/* sec 0434 */
void scan_char_num (void)
{
  scan_int();

  if ((cur_val < 0) || (cur_val > 255))
  {
    print_err("Bad character code");
    help2("A character number must be between 0 and 255.",
        "I changed this one to zero.");
    int_error(cur_val);
    cur_val = 0;
  }
}
/* sec 0435 */
void scan_four_bit_int (void)
{
  scan_int();

  if ((cur_val < 0) || (cur_val > 15))
  {
    print_err("Bad number");
    help2("Since I expected to read a number between 0 and 15,",
        "I changed this one to zero.");
    int_error(cur_val);
    cur_val = 0;
  }
}
/* sec 0436 */
void scan_fifteen_bit_int (void) 
{
  scan_int();
  if ((cur_val < 0) || (cur_val > 32767))
  {
    print_err("Bad mathchar");
    help2("A mathchar number must be between 0 and 32767.",
        "I changed this one to zero.");
    int_error(cur_val);
    cur_val = 0;
  }
}
/* sec 0437 */
void scan_twenty_seven_bit_int (void)
{
  scan_int();

  if ((cur_val < 0) || (cur_val > 134217727L)) /* 2^27 - 1 */
  {
    print_err("Bad delimiter code");
    help2("A numeric delimiter code must be between 0 and 2^{27}-1.",
        "I changed this one to zero.");
    int_error(cur_val);
    cur_val = 0;
  }
}
/* sec 0577 */
void scan_font_ident (void) 
{
  internal_font_number f;
  halfword m;

  do
    {
      get_x_token();
    }
  while (!(cur_cmd != spacer));

  if (cur_cmd == def_font)
    f = cur_font;
  else if (cur_cmd == set_font)
    f = cur_chr; 
  else if (cur_cmd == def_family)
  {
    m = cur_chr;
    scan_four_bit_int();
    f = equiv(m + cur_val);
  }
  else
  {
    print_err("Missing font identifier");
    help2("I was looking for a control sequence whose",
        "current meaning has been defined by \\font.");
    back_error();
    f = null_font;
  }

  cur_val = f;
}
/* sec 0578 */
void find_font_dimen_(bool writing)
{
  internal_font_number f;
  integer n;

  scan_int();
  n = cur_val;
  scan_font_ident();
  f = cur_val;

  if (n < 0 || (n == 0 && font_dimen_zero == 0)) /* change 98/Oct/5 */
    cur_val = fmem_ptr;
  else
  {
    if (writing && (n <= space_shrink_code) && (n >= space_code) && (font_glue[f] != 0)) 
    {
      delete_glue_ref(font_glue[f]);
      font_glue[f] = 0;
    }

    if (n > font_params[f])
      if (f < font_ptr)
        cur_val = fmem_ptr;
    else
    {
      do
        {
 #ifdef ALLOCATEFONT
          if (fmem_ptr == current_font_mem_size) /* 93/Nov/28 ??? */
          {
            font_info = realloc_font_info(increment_font_mem_size);
          }

          if (fmem_ptr == current_font_mem_size) /* 94/Jan/24 */
          {
            overflow("font memory", current_font_mem_size); /* font memory */
            return;     // abort_flag set
          }
#else
          if (fmem_ptr == font_mem_size)
          {
            overflow("font memory", font_mem_size); /* font memory */
            return;     // abort_flag set
          }
#endif
          font_info[fmem_ptr].cint = 0;
          incr(fmem_ptr);
          incr(font_params[f]);
        }
      while(!(n == font_params[f]));

      cur_val = fmem_ptr - 1;
    }
  else if (n > 0)
    cur_val = n + param_base[f];    /* 98/Oct/5 */
  else
    cur_val = (&font_check[f] - &font_info[0]); /* 98/Oct/5 */
/*  checksum =  (((font_check[f].b0) << 8 | font_check[f].b1) << 8 |
        font_check[f].b2) << 8 | font_check[f].b3; */
  }
/* compiler error: '-' : incompatible types - from 'union fmemoryword *' to 'struct fourunsignedchars *' */
  if (cur_val == fmem_ptr)
  {
    print_err("Font ");
/*    print_esc(hash[(hash_size + 524) + f].v.RH); */
    print_esc(""); print(font_id_text(f));
    print_string(" has only ");
    print_int(font_params[f]);
    print_string(" fontdimen parameters");
    help2("To increase the number of font parameters, you must",
      "use \\fontdimen immediately after the \\font is loaded.");
    error();
  }
}
/* NOTE: the above use of /fontdimen0 to access the checksum is a kludge */
/* In future would be better to do this by allocating one more slot for */
/* for parameters when a font is read rather than carry checksum separately */
/* The above gets the value byte order reversed ... 98/Oct/5 */
/* sec 0413 */
void scan_something_internal_(small_number level, bool negative)
{
  halfword m;
  integer p;

  m = cur_chr;

  switch (cur_cmd)
  {
    case def_code:
      {
        scan_char_num();

        if (m == math_code_base)
        {
          cur_val = math_code(cur_val);
          cur_val_level = int_val;
        }
        else if (m < math_code_base)
        {
          cur_val = equiv(m + cur_val);
          cur_val_level = int_val;
        }
        else
        {
          cur_val = eqtb[m + cur_val].cint;
          cur_val_level = int_val;
        }
      }
      break;

    case toks_register:
    case assign_toks:
    case def_family:
    case set_font:
    case def_font:
      if (level != tok_val)
      {
        print_err("Missing number, treated as zero");
        help3("A number should have been here; I inserted `0'.",
            "(If you can't figure out why I needed to see a number,",
            "look up `weird error' in the index to The TeXbook.)");
        back_error();
        {
          cur_val = 0;
          cur_val_level = dimen_val;
        }
      }
      else if (cur_cmd <= assign_toks)
      {
        if (cur_cmd < assign_toks)
        {
          scan_eight_bit_int();
          m = toks_base + cur_val;
        }

        {
          cur_val = eqtb[m].hh.v.RH;
          cur_val_level = tok_val;
        }
      }
      else
      {
        back_input();
        scan_font_ident();

        {
          cur_val = font_id_base + cur_val; /* 96/Jan/10 */
          cur_val_level = ident_val;
        }
      }
      break;

    case assign_int:
      {
        cur_val = eqtb[m].cint;
        cur_val_level = int_val;
      }
      break;

    case assign_dimen:
      {
        cur_val = eqtb[m].cint;
        cur_val_level = dimen_val;
      }
      break; 

    case assign_glue:
      {
        cur_val = eqtb[m].hh.v.RH;
        cur_val_level = glue_val;
      }
      break;

    case assign_mu_glue:
      {
        cur_val = eqtb[m].hh.v.RH;
        cur_val_level = mu_val;
      }
      break;

    case set_aux:
      if (abs(mode)!= m)
      {
        print_err("Improper ");
        print_cmd_chr(set_aux, m);
        help4("You can refer to \\spacefactor only in horizontal mode;",
            "you can refer to \\prevdepth only in vertical mode; and",
            "neither of these is meaningful inside \\write. So",
            "I'm forgetting what you said and using zero instead.");
        error();

        if (level != tok_val)
        {
          cur_val = 0;
          cur_val_level = dimen_val;
        }
        else
        {
          cur_val = 0;
          cur_val_level = int_val;
        }
      }
      else if (m == vmode)
      {
        cur_val = cur_list.aux_field.cint;
        cur_val_level = dimen_val;
      }
      else
      {
        cur_val = space_factor;
        cur_val_level = int_val;
      }
      break;

    case set_prev_graf:
      if (mode == 0)
      {
        cur_val = 0;
        cur_val_level = int_val;
      }
      else
      {
        nest[nest_ptr] = cur_list;
        p = nest_ptr;

        while (abs(nest[p].mode_field)!= vmode)
          decr(p);

        {
          cur_val = nest[p].pg_field;
          cur_val_level = int_val;
        }
      }
      break;

    case set_page_int:
      {
        if (m == 0)
          cur_val = dead_cycles; 
        else
          cur_val = insert_penalties;

        cur_val_level = 0;
      }
      break;

    case set_page_dimen:
      {
        if ((page_contents == 0) && (! output_active))
          if (m == 0)
            cur_val = 1073741823L;  /* 2^30 - 1 */
          else
            cur_val = 0;
        else
          cur_val = page_so_far[m];

        cur_val_level = dimen_val;
      }
      break;

    case set_shape:
      {
        if (par_shape_ptr == 0)
          cur_val = 0; 
        else
          cur_val = info(par_shape_ptr);

        cur_val_level = int_val;
      }
      break;

    case set_box_dimen:
      {
        scan_eight_bit_int();

        if (box(cur_val) == 0)
          cur_val = 0;
        else
          cur_val = mem[box(cur_val) + m].cint;

        cur_val_level = dimen_val;
      }
      break;

    case char_given:
    case math_given:
      {
        cur_val = cur_chr;
        cur_val_level = int_val;
      }
      break;

    case assign_font_dimen:
      {
        find_font_dimen(false);
        font_info[fmem_ptr].cint = 0;
        {
          cur_val = font_info[cur_val].cint;
          cur_val_level = dimen_val;
        }
      }
      break;

    case assign_font_int:
      {
        scan_font_ident();

        if (m == 0)
        {
          cur_val = hyphen_char[cur_val];
          cur_val_level = int_val;
        }
        else
        {
          cur_val = skew_char[cur_val];
          cur_val_level = int_val;
        }
      }
      break;

    case tex_register:
      {
        scan_eight_bit_int();

        switch(m)
        {
          case int_val:
            cur_val = count(cur_val);
            break;

          case dimen_val:
            cur_val = dimen(cur_val);
            break;

          case glue_val:
            cur_val = skip(cur_val);
            break;

          case mu_val:
            cur_val = mu_skip(cur_val);
            break;
        }
        
        cur_val_level = m;
      }
      break;

    case last_item:
      if (cur_chr > glue_val)
      {
        if (cur_chr == input_line_no_code)
          cur_val = line;
        else
          cur_val = last_badness;

        cur_val_level = int_val;
      }
      else
      {
        if (cur_chr == glue_val)
          cur_val = zero_glue;
        else
          cur_val = 0;

        cur_val_level = cur_chr;

        if (!(tail >= hi_mem_min) && (mode != 0))
          switch(cur_chr)
          {
            case int_val:
              if (type(tail) == penalty_node)
                cur_val = penalty(tail);
              break;

            case dimen_val:
              if (type(tail) == kern_node)
                cur_val = width(tail);
              break;

            case glue_val:
              if (type(tail) == glue_node)
              {
                cur_val = glue_ptr(tail);

                if (subtype(tail) == mu_glue)
                  cur_val_level = mu_val;
              }
              break;
          }
        else if ((mode == 1) && (tail == cur_list.head_field))
          switch (cur_chr)
          {
            case int_val:
              cur_val = last_penalty;
              break;

            case dimen_val:
              cur_val = last_kern;
              break;

            case glue_val:
              if (last_glue != empty_flag)
                cur_val = last_glue;
              break;
          }
      }
      break;

    default:
      {
        print_err("You can't use `");
        print_cmd_chr(cur_cmd, cur_chr);
        print_string("' after ");
        print_esc("the");
        help1("I'm forgetting what you said and using zero instead.");
        error();

        if (level != tok_val)
        {
          cur_val = 0;
          cur_val_level = dimen_val;
        }
        else
        {
          cur_val = 0;
          cur_val_level = int_val;
        }
      }
      break;
  }

  while (cur_val_level > level)
  {
    if (cur_val_level == glue_val)
      cur_val = width(cur_val);
    else if (cur_val_level == mu_val)
      mu_error();

    decr(cur_val_level);
  }

  if (negative)
    if (cur_val_level >= 2)
    {
      cur_val = new_spec(cur_val);

      {
        width(cur_val) = - (integer) width(cur_val);
        stretch(cur_val) = - (integer) stretch(cur_val);
        shrink(cur_val) = - (integer) shrink(cur_val);
      }
    }
    else
      cur_val = - (integer) cur_val;
  else if ((cur_val_level >= glue_val) && (cur_val_level <= mu_val))
    add_glue_ref(cur_val);
}
/*****************************************************************************/
/* Moved here to avoid question about pragma optimize 96/Sep/12 */
/* sec 0341 */
void get_next (void)
{
  integer k;
  halfword t;
/*  char cat; */    /* make this an int ? */
  int cat;      /* make this an int ? 95/Jan/7 */
  ASCII_code c, cc;
  char d;

lab20:
  cur_cs = 0;

  if (cur_input.state_field != token_list)
  {
lab25:
    if (cur_input.loc_field <= cur_input.limit_field)
    {
      cur_chr = buffer[cur_input.loc_field];
      incr(cur_input.loc_field);
lab21:
      cur_cmd = cat_code(cur_chr);

      switch (cur_input.state_field + cur_cmd)
      {
        case any_state_plus(ignore):
        case skip_blanks + spacer:
        case new_line + spacer:
          goto lab25;
          break;

        case any_state_plus(escape):
          {
            if (cur_input.loc_field > cur_input.limit_field)
              cur_cs = null_cs;
            else
            {
lab26:
              k = cur_input.loc_field;
              cur_chr = buffer[k];
              cat = cat_code(cur_chr);
              incr(k);

              if (cat == letter)
                cur_input.state_field = skip_blanks;
              else if (cat == spacer)
                cur_input.state_field = skip_blanks;
              else
                cur_input.state_field = mid_line;

              if ((cat == letter) && (k <= cur_input.limit_field))
              {
                do
                  {
                    cur_chr = buffer[k];
                    cat = cat_code(cur_chr);
                    incr(k);
                  }
                while(!((cat != letter) || (k > cur_input.limit_field)));

                {
                  if (buffer[k]== cur_chr)
                    if (cat == sup_mark)
                      if (k < cur_input.limit_field)
                      {
                        c = buffer[k + 1];

                        if (c < 128)
                        {
                          d = 2;
                          if ((((c >= 48) && (c <= 57)) || ((c >= 97) && (c <= 102))))
                            if (k + 2 <= cur_input.limit_field)
                            {
                              cc = buffer[k + 2];

                              if ((((cc >= 48) && (cc <= 57)) || ((cc >= 97) && (cc <= 102))))
                                incr(d);
                            }

                          if (d > 2)
                          {
                            if (c <= 57)
                              cur_chr = c - 48;
                            else
                              cur_chr = c - 87;

                            if (cc <= 57)
                              cur_chr = 16 * cur_chr + cc - 48;
                            else
                              cur_chr = 16 * cur_chr + cc - 87;

                            buffer[k - 1] = cur_chr;
                          }
                          else if (c < 64)
                            buffer[k - 1] = c + 64;
                          else
                            buffer[k - 1] = c - 64;

                          cur_input.limit_field = cur_input.limit_field - d;
                          first = first - d;

                          while (k <= cur_input.limit_field)
                          {
                            buffer[k] = buffer[k + d];
                            incr(k);
                          }

                          goto lab26;
                        }
                      }
                }

                if (cat != letter)
                  decr(k);

                if (k > cur_input.loc_field + 1)
                {
                  cur_cs = id_lookup(cur_input.loc_field, k - cur_input.loc_field);
                  cur_input.loc_field = k;
                  goto lab40;
                }
              }
              else
              {
                if (buffer[k] == cur_chr)
                  if (cat == sup_mark)
                    if (k < cur_input.limit_field)
                    {
                      c = buffer[k + 1];

                      if (c < 128)             /* ? */
                      {
                        d = 2;
                        if ((((c >= 48) && (c <= 57)) || ((c >= 97) && (c <= 102))))
                          if (k + 2 <= cur_input.limit_field)
                          {
                            cc = buffer[k + 2];

                            if ((((cc >= 48) && (cc <= 57)) || ((cc >= 97) && (cc <= 102))))
                              incr(d);
                          }

                        if (d > 2)
                        {
                          if (c <= 57)
                            cur_chr = c - 48;
                          else
                            cur_chr = c - 87;

                          if (cc <= 57)          /* cc may be used without ... */
                            cur_chr = 16 * cur_chr + cc - 48;
                          else
                            cur_chr = 16 * cur_chr + cc - 87;

                          buffer[k - 1] = cur_chr;
                        }
                        else if (c < 64)
                          buffer[k - 1] = c + 64;
                        else
                          buffer[k - 1] = c - 64;

                        cur_input.limit_field = cur_input.limit_field - d;
                        first = first - d;

                        while (k <= cur_input.limit_field)
                        {
                          buffer[k] = buffer[k + d];
                          incr(k);
                        }
                        goto lab26;
                      }
                    }
              }
              cur_cs = single_base + buffer[cur_input.loc_field];
              incr(cur_input.loc_field);
            }
lab40:
            cur_cmd = eq_type(cur_cs);
            cur_chr = equiv(cur_cs);
            
            if (cur_cmd >= outer_call)
              check_outer_validity();
          }
          break;

        case any_state_plus(active_char):
          {
            cur_cs = cur_chr + active_base;
            cur_cmd = eq_type(cur_cs);
            cur_chr = equiv(cur_cs);
            cur_input.state_field = mid_line;
            
            if (cur_cmd >= outer_call)
              check_outer_validity();
          }
          break;

        case any_state_plus(sup_mark):
          {
            if (cur_chr == buffer[cur_input.loc_field])
              if (cur_input.loc_field < cur_input.limit_field)
              {
                c = buffer[cur_input.loc_field + 1];

                if (c < 128)
                {
                  cur_input.loc_field = cur_input.loc_field + 2;

                  if ((((c >= 48) && (c <= 57)) || ((c >= 97) && (c <= 102))))
                    if (cur_input.loc_field <= cur_input.limit_field)
                    {
                      cc = buffer[cur_input.loc_field];

                      if ((((cc >= 48) && (cc <= 57)) || ((cc >= 97) && (cc <= 102))))
                      {
                        incr(cur_input.loc_field);

                        if (c <= 57)
                          cur_chr = c - 48;
                        else
                          cur_chr = c - 87;

                        if (cc <= 57)
                          cur_chr = 16 * cur_chr + cc - 48;
                        else
                          cur_chr = 16 * cur_chr + cc - 87;

                        goto lab21;
                      }
                    }

                  if (c < 64)
                    cur_chr = c + 64;
                  else
                    cur_chr = c - 64;

                  goto lab21;
                }
              }

              cur_input.state_field = mid_line;
          }
          break;

        case any_state_plus(invalid_char):
          {
            print_err("Text line contains an invalid character");
            help2("A funny symbol that I can't read has just been input.",
                "Continue, and I'll forget that it ever happened.");
            deletions_allowed = false;
            error();
            deletions_allowed = true;
            goto lab20;
          }
          break;

        case mid_line + spacer:
          {
            cur_input.state_field = skip_blanks;
            cur_chr = ' ';
          }
          break;

        case mid_line + car_ret:
          {
            cur_input.loc_field = cur_input.limit_field + 1;
            cur_cmd = spacer;
            cur_chr = ' ';
          }
          break;

        case skip_blanks + car_ret:
        case any_state_plus(comment):
          {
            cur_input.loc_field = cur_input.limit_field + 1;
            goto lab25;
          }
          break;

        case new_line + car_ret:
          {
            cur_input.loc_field = cur_input.limit_field + 1;
            cur_cs = par_loc;
            cur_cmd = eq_type(cur_cs);
            cur_chr = equiv(cur_cs);
            
            if (cur_cmd >= outer_call)
              check_outer_validity();
          }
          break;

        case mid_line + left_brace:
          incr(align_state);
          break;

        case skip_blanks + left_brace:
        case new_line + left_brace:
          {
            cur_input.state_field = mid_line;
            incr(align_state);
          }
          break;

        case mid_line + right_brace:
          decr(align_state);
          break;

        case skip_blanks + right_brace:
        case new_line + right_brace:
          {
            cur_input.state_field = 1;
            decr(align_state);
          }
          break;

        case add_delims_to(skip_blanks):
        case add_delims_to(new_line):
          cur_input.state_field = 1;
          break;

        default:
          break;
      }
    }
    else
    {
      cur_input.state_field = new_line;

      if (cur_input.name_field > 17)
      {
        incr(line);
        first = cur_input.start_field;

        if (!force_eof)
        {
          if (input_ln(input_file[cur_input.index_field], true))
            firm_up_the_line();
          else
            force_eof = true;
        }

        if (force_eof)
        {
          print_char(')');
          decr(open_parens);
#ifndef _WINDOWS
          fflush(stdout);
#endif
          force_eof = false;
          end_file_reading();
          check_outer_validity();
          goto lab20;
        }

        if ((end_line_char < 0) || (end_line_char > 255))
          decr(cur_input.limit_field);
        else
          buffer[cur_input.limit_field] = end_line_char;

        first = cur_input.limit_field + 1;
        cur_input.loc_field = cur_input.start_field;
      }
      else
      {
        if (!(cur_input.name_field == 0))
        {
          cur_cmd = 0;
          cur_chr = 0;
          return;
        }

        if (input_ptr > 0)
        {
          end_file_reading();
          goto lab20;
        }

        if (selector < log_only)
          open_log_file();

        if (interaction > nonstop_mode)
        {
          if ((end_line_char < 0) || (end_line_char > 255))
            incr(cur_input.limit_field);

          if (cur_input.limit_field == cur_input.start_field)
            print_nl("(Please type a command or say `\\end')");

          print_ln();
          first = cur_input.start_field;

          {
            ;
            print_string("*");
            term_input("*", 0);
          }

          cur_input.limit_field = last;

          if ((end_line_char < 0) || (end_line_char > 255))
            decr(cur_input.limit_field);
          else
            buffer[cur_input.limit_field]= end_line_char;

          first = cur_input.limit_field + 1;
          cur_input.loc_field = cur_input.start_field;
        }
        else
        {
          fatal_error("*** (job aborted, no legal \\end found)");
          return;     // abort_flag set
        }
      }

      {
        if (interrupt != 0)
        {
          pause_for_instructions();
        }
      }

      goto lab25;
    }
  }
  else if (cur_input.loc_field != 0)
  {
    t = info(cur_input.loc_field);
    cur_input.loc_field = link(cur_input.loc_field);

    if (t >= cs_token_flag)
    {
      cur_cs = t - cs_token_flag;
      cur_cmd = eq_type(cur_cs);
      cur_chr = equiv(cur_cs);

      if (cur_cmd >= outer_call)
        if (cur_cmd == dont_expand)
        {
          cur_cs = info(cur_input.loc_field) - cs_token_flag;
          cur_input.loc_field = 0;
          cur_cmd = eq_type(cur_cs);
          cur_chr = equiv(cur_cs);

          if (cur_cmd > max_command)
          {
            cur_cmd = relax;
            cur_chr = no_expand_flag;
          }
        }
        else
        {
          check_outer_validity();
        }
    }
    else
    {
      cur_cmd = t / 256;
      cur_chr = t % 256;

      switch (cur_cmd)
      {
        case left_brace:
          incr(align_state);
          break;

        case right_brace:
          decr(align_state);
          break;

        case out_param:
          {
            begin_token_list(param_stack[cur_input.limit_field + cur_chr - 1], parameter);
            goto lab20;
          }
          break;

        default:
          break;
      }
    }
  }
  else
  {
    end_token_list();
    goto lab20;
  }

  if (cur_cmd <= car_ret)
    if (cur_cmd >= tab_mark)
      if (align_state == 0)
      {
        if ((scanner_status == aligning) && (cur_align == 0))
        {
          fatal_error("(interwoven alignment preambles are not allowed)");
          return;     // abort_flag set
        }

        cur_cmd = extra_info(cur_align);
        extra_info(cur_align) = cur_chr;

        if (cur_cmd == omit)
          begin_token_list(omit_template, v_template);
        else
          begin_token_list(v_part(cur_align), v_template);

        align_state = 1000000L;
        goto lab20;
      }
}
#pragma optimize ("", on)             /* 96/Sep/12 */