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

#include "yandytex.h"
#undef name

#define PATH_SEP        '/'
#define PATH_SEP_STRING "/"

#define BUILDNAMEDIRECT

extern int shorten_file_name;

#ifdef FUNNY_CORE_DUMP
  extern void funny_core_dump(void);
#endif

#ifdef BUILDNAMEDIRECT
char * xconcat (char * buffer, char * s1, char * s2)
{
  int n1 = strlen(s1);
  int n2 = strlen(s2);

  if (buffer == s2)
  {
    memmove(buffer + n1, buffer, n2 + 1);
    strncpy(buffer, s1, n1);
  }
  else
  {
    strcpy(buffer, s1);
    strcat(buffer + n1, s2);
  }

  return buffer;
}
char * xconcat3 (char *buffer, char *s1, char *s2, char *s3)
{
  int n1 = strlen(s1);
  int n2 = strlen(s2);
  int n3 = strlen(s3);

  if (buffer == s3)
  {
    memmove(buffer + n1 + n2, buffer, n3 + 1);
    strncpy(buffer, s1, n1);
    strncpy(buffer + n1, s2, n2);
  }
  else
  {
    strcpy(buffer, s1);
    strcat(buffer + n1, s2);
    strcat(buffer + n1 + n2, s3);
  }

  return buffer;
}
#endif

void patch_in_path (ASCII_code * buffer, ASCII_code *name, ASCII_code * path)
{
  if (*path == '\0')
    strcpy((char *) buffer, (char *) name);
  else
    xconcat3((char *) buffer, (char *) path, "/", (char *) name);
}

int qualified (ASCII_code * name)
{
  if (strchr((char *) name, '/') != NULL ||
      strchr((char *) name, '\\') != NULL ||
      strchr((char *) name, ':') != NULL)
    return 1;
  else
    return 0;
}
/* patch path if 
    (i)   path not empty
    (ii)  name not qualified
    (iii) ext match
*/
int prepend_path_if(ASCII_code * buffer, ASCII_code * name, const char * ext, char *path)
{
  if (path == NULL)
    return 0;

  if (*path == '\0')
    return 0;

  if (qualified(name))
    return 0;

  if (strstr((char *) name, ext) == NULL)
    return 0;

  patch_in_path(buffer, name, (ASCII_code *)path);

  return 1;
}

//  Following works on null-terminated strings
void check_short_name (unsigned char *s)
{
  unsigned char *star, *sdot;
  int n;

  if ((star = (unsigned char *) strrchr((char *) s, '\\')) != NULL)
    star++;
  else if ((star = (unsigned char *) strrchr((char *) s, '/')) != NULL)
    star++;
  else if ((star = (unsigned char *) strchr((char *) s, ':')) != NULL)
    star++;
  else
    star = s;

  if ((sdot = (unsigned char *) strchr((char *) star, '.')) != NULL)
    n = sdot - star;
  else
    n = strlen((char *) star);

  if (n > 8)
    strcpy((char *) star + 8, (char *) star + n);

  if ((sdot = (unsigned char *) strchr((char *) star, '.')) != NULL)
  {
    star = sdot + 1;

    n = strlen((char *) star);

    if (n > 3)
      *(star + 3) = '\0';
  }
}

/* Following works on both null-terminated names */

void retwiddle (unsigned char *s)
{
  while (*s != '\0')
  {
    if (*s == (unsigned char) pseudo_tilde)
      *s = '~';
    else if (*s == (unsigned char) pseudo_space)
      *s = ' ';

    s++;
  }
}

boolean open_input (FILE ** f, kpse_file_format_type file_fmt, const char * fopen_mode)
{
  boolean openable = false;
  char * file_name = NULL;

#if defined (FUNNY_CORE_DUMP) && !defined (BibTeX)
  if (file_fmt == kpse_tex_format &&
    strncmp(name_of_file + 1, "HackyInputFileNameForCoreDump.tex", 33) == 0)
    funny_core_dump();
#endif

  if (return_flag)
  {
    if (strcmp(fopen_mode, "r") == 0)
      fopen_mode = "rb";
  }

  name_of_file[name_length + 1] = '\0';

  /* reinsert '~' and ' ' in file names */  
  if (pseudo_tilde != 0 || pseudo_space != 0)
    retwiddle(name_of_file + 1);

  if (shorten_file_name)
    check_short_name(name_of_file + 1);
  
  if (open_trace_flag)
    printf(" Open `%s' for input ", name_of_file + 1);

  file_name = kpse_find_file((const_string) name_of_file + 1, file_fmt, false);

  if (file_name != NULL)
  {
    strcpy ((char *) name_of_file + 1, file_name);
    *f = xfopen((char *) file_name, fopen_mode);

#ifdef _WIN32
    if (name_of_file[1] == '.' && (name_of_file[2] == PATH_SEP || name_of_file[2] == '\\'))
#else
    if (name_of_file[1] == '.' && name_of_file[2] == PATH_SEP)
#endif
    {
      unsigned i = 1;

      while (name_of_file[i + 2] != '\0')
      {
        name_of_file[i] = name_of_file[i + 2];
        i++;
      }

      name_of_file[i] = '\0';
      name_length = i - 1;
    }
    else
      name_length = strlen((char *) name_of_file + 1);
      
    if (file_fmt == kpse_tfm_format)
    {
      fbyte = getc(*f);
    } 

    if (strstr((char *) name_of_file + 1, ".fmt") != NULL)
    {
      if (format_file == NULL)
        format_file = xstrdup((char *) name_of_file + 1);

#ifdef COMPACTFORMAT
      gz_fmt_file = gzdopen(fileno(*f), "rb9");
#endif
    }
    else if (strstr((char *) name_of_file + 1, ".tfm") != NULL)
    {
      if (show_tfm_flag && log_opened)
      {
        int n; 
        n = strlen((char *) name_of_file + 1);

        if (file_offset + n > max_print_line)
        {
          (void) putc('\n', log_file);
          file_offset = 0;
        }
        else
          (void) putc(' ', log_file);

        fprintf(log_file, "(%s)", name_of_file + 1);
        file_offset += n + 3;
      }
    }
    else if (source_direct == NULL)
    {
      char *s;

      source_direct = xstrdup((char *) name_of_file + 1);

      if (trace_flag)
        printf("Methinks the source %s is `%s'\n", "file", source_direct);

      if ((s = strrchr(source_direct, '/')) == NULL)
        *source_direct = '\0';
      else
        *(s + 1) = '\0';

      if (trace_flag)
        printf("Methinks the source %s is `%s'\n", "directory", source_direct);
    }

    openable = true;
  }

  {
    unsigned temp_length = strlen((char *) name_of_file + 1);
    name_of_file[temp_length + 1] = ' ';
  }

  return openable;
}

/* At least check for I/O error (such as disk full) when closing */
/* Would be better to check while writing - but this is better than nothing */
/* This is used for both input and output files, but never mind ... */

/* now a_close returns -1 on error --- which could be used by caller */
/* probably want to ignore on input files ... */

// check_fclose not used by anything
int check_fclose (FILE * f)
{
  if (f == NULL)
    return 0;

  if (ferror(f) || fclose (f))
  {
    perrormod("\n! I/O Error");
    uexit(EXIT_FAILURE);
  }

  return 0;
}

char * xstrdup_name (void)
{
  if (qualified(name_of_file + 1))
    *log_line = '\0';
  else
  {
    (void) getcwd(log_line, sizeof(log_line));
    strcat(log_line, PATH_SEP_STRING);
  }

  strcat(log_line, (char *) name_of_file + 1);
  unixify(log_line);
  return xstrdup(log_line);
}

boolean open_output (FILE ** f, const char * fopen_mode)
{
  unsigned temp_length;

  name_of_file[name_length + 1] = '\0';

  if (pseudo_tilde != 0 || pseudo_space !=  0)
    retwiddle(name_of_file + 1);

  /* 8 + 3 file names on Windows NT 95/Feb/20 */
  if (shorten_file_name)
    check_short_name(name_of_file + 1);

  if (prepend_path_if(name_of_file + 1, name_of_file + 1, ".dvi", dvi_directory) ||
      prepend_path_if(name_of_file + 1, name_of_file + 1, ".log", log_directory) ||
      prepend_path_if(name_of_file + 1, name_of_file + 1, ".aux", aux_directory) ||
      prepend_path_if(name_of_file + 1, name_of_file + 1, ".fmt", fmt_directory) ||
      prepend_path_if(name_of_file + 1, name_of_file + 1, ".pdf", pdf_directory))
  {
    if (open_trace_flag)
      printf("After prepend %s\n", name_of_file + 1);
  }

  if (open_trace_flag)
    printf(" Open `%s' for output ", name_of_file + 1);

  *f = fopen((char *) name_of_file + 1, fopen_mode);

  if (*f == NULL)
  {
    string temp_dir = kpse_var_value("TEXMFOUTPUT");

    if (temp_dir != NULL)
    {
#ifdef BUILDNAMEDIRECT
      unsigned char temp_name[file_name_size];
      xconcat3((char *) temp_name, temp_dir, PATH_SEP_STRING, (char *) name_of_file + 1);
#else
      string temp_name = concat3(temp_dir, PATH_SEP_STRING, name_of_file + 1);
#endif

      if (deslash)
        unixify((char *) temp_name);
      
      /* but we can assume this is opening here for *output* */
      *f = fopen((char *) temp_name, fopen_mode);
      /* If this succeeded, change name_of_file accordingly. */
      if (*f)
        strcpy((char *) name_of_file + 1, (char *) temp_name);

#ifndef BUILDNAMEDIRECT
      free (temp_name);
#endif
    }
  }

#ifdef COMPACTFORMAT
  if (strstr((char *) name_of_file + 1, ".fmt") != NULL)
    gz_fmt_file = gzdopen(fileno(*f), "wb9");
#endif

  if (strstr((char *) name_of_file + 1, ".dvi") != NULL)
    dvi_file_name = xstrdup_name();
  else if (strstr((char *) name_of_file + 1, ".pdf") != NULL)
    pdf_file_name = xstrdup_name();
  else if (strstr((char *) name_of_file + 1, ".log") != NULL)
    log_file_name = xstrdup_name();

  temp_length = strlen((char *) name_of_file + 1);
  name_of_file[temp_length + 1] = ' ';

  if (*f)
    name_length = temp_length;
  
  return (*f != NULL);
}