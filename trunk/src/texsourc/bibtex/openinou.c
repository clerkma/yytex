/* openinout.c: open input and output files.  These routines used by
   TeX, Metafont, and BibTeX.  */

#include "config.h"
#include <kpathsea/c-namemx.h>
#include <kpathsea/c-pathch.h>
#include <kpathsea/tex-make.h>

#ifdef BibTeX
/* See comments in bibtex.ch for why we need these.  */
FILE *standardinput = stdin;
FILE *standardoutput = stdout;

/* Because we don't generate a .h file with all the global definitions
   for BibTeX, as we do with TeX and Metafont, we must declare these
   variables.  */
extern char nameoffile[];
extern integer namelength;

#else /* not BibTeX */

#define EXTERN extern /* Don't instantiate data here.  */

#ifdef TeX
#include "texd.h"
#else /* Metafont */
#include "mfd.h"
#endif

#ifdef FUNNY_CORE_DUMP
/* This is defined in texmf.c.  */
extern void funny_core_dump ();
#endif /* FUNNY_CORE_DUMP */

#endif /* not BibTeX */

/* Open an input file F, using the path PATHSPEC and passing
   FOPEN_MODE to fopen.  The filename is in `nameoffile', as a Pascal
   string. We return whether or not the open succeeded.  If it did, we
   also set `namelength' to the length of the full pathname that we
   opened.  */

boolean
open_input (f, path_index, fopen_mode)
    FILE **f;
    path_constant_type path_index;
    char *fopen_mode;
{
  boolean openable = false;

#if defined (FUNNY_CORE_DUMP) && !defined (BibTeX)
  /* This only applies if a preloaded TeX/Metafont is being made;
     it allows automatic creation of the core dump (typing ^\ loses
     since it requires manual intervention).  */
  if ((path_index == TEXINPUTPATH || path_index == MFINPUTPATH)
      && strncmp (nameoffile+1, "HackyInputFileNameForCoreDump.tex", 33) == 0)
    funny_core_dump ();
#endif /* FUNNY_CORE_DUMP and not BibTeX */

#ifdef BibTeX
  if (path_index == NO_FILE_PATH)
    {
      unsigned temp_length;

      null_terminate (nameoffile + 1);
      *f = fopen (nameoffile + 1, fopen_mode);
      temp_length = strlen (nameoffile + 1);
      space_terminate (nameoffile + 1);
      if (*f != NULL)
        {
          namelength = temp_length;
          openable = true;
        }
    }

  else
#endif /* BibTeX */
  
  if (testreadaccess (nameoffile, path_index))
    {
      /* We can assume `nameoffile' is openable, since
         `testreadaccess' just returned true.  */
      *f = xfopen_pas (nameoffile, fopen_mode);
      
      /* If we found the file in the current directory, don't leave the
         `./' at the beginning of `nameoffile', since it looks dumb when
         TeX says `(./foo.tex ... )', and analogously for Metafont.  */
      if (nameoffile[1] == '.' && IS_DIR_SEP (nameoffile[2]))
        {
          unsigned i = 1;
          while (nameoffile[i + 2] != ' ')
            {
              nameoffile[i] = nameoffile[i + 2];
              i++;
            }
          nameoffile[i] = ' ';
          namelength = i - 1;
        }
      else
        namelength = strchr (nameoffile + 1, ' ') - nameoffile - 1;
      
#ifdef TeX
      /* If we just opened a TFM file, we have to read the first byte,
         since TeX wants to look at it.  What a kludge.  */
      if (path_index == TFMFILEPATH)
        { /* See comments in ctex.ch for why we need this.  */
          extern integer tfmtemp;
          tfmtemp = getc (*f);
        }
#endif /* TeX */  

      openable = true;
    }

  return openable;
}


/* Call the external program for FORMAT, passing it `nameoffile'.  We'd
   like to pass back the filename the script returns, but I'm not yet
   sure how to do that in Pascal.  */

static boolean
make_tex_file (format)
    kpse_file_format_type format;
{
  string found;
  /* Since & is a no-op when applied to an array, we must put the
     address of the filename in a variable.  */
  string name = nameoffile;
  
  make_c_string (&name);
  found = kpse_make_tex (format, name);
  make_pascal_string (&name);

  return found != NULL;
}


/* These are called by TeX or MF if an input or TFM file can't be opened.  */

boolean
maketextex ()
{
  return make_tex_file (kpse_tex_format);
}

boolean
maketexmf ()
{
  return make_tex_file (kpse_mf_format);
}

boolean
maketextfm ()
{
  return make_tex_file (kpse_tfm_format);
}

/* Open an output file F either in the current directory or in
   $TEXMFOUTPUT/F, if the environment variable `TEXMFOUTPUT' exists.
   (Actually, this applies to the BibTeX output files, also, but
   `TEXMFBIBOUTPUT' was just too long.)  The filename is in the global
   `nameoffile', as a Pascal string.  We return whether or not the open
   succeeded.  If it did, the global `namelength' is set to the length
   of the actual filename.  */

boolean
open_output (f, fopen_mode)
    FILE **f;
    char *fopen_mode;
{
  unsigned temp_length;

  /* Make the filename into a C string.  */
  null_terminate (nameoffile + 1);
  
  /* Is the filename openable as given?  */
  *f = fopen (nameoffile + 1, fopen_mode);

  if (*f == NULL)
    { /* Can't open as given.  Try the envvar.  */
      string temp_dir = getenv ("TEXMFOUTPUT");

      if (temp_dir != NULL)
        {
          string temp_name = concat3 (temp_dir, "/", nameoffile + 1);
          *f = fopen (temp_name, fopen_mode);
          
          /* If this succeeded, change nameoffile accordingly.  */
          if (*f)
            strcpy (nameoffile + 1, temp_name);
          
          free (temp_name);
        }
    }

  /* Back into a Pascal string, but first get its length.  */
  temp_length = strlen (nameoffile + 1);
  space_terminate (nameoffile + 1);

  /* Only set `namelength' if we succeeded.  I'm not sure why.  */
  if (*f)
    namelength = temp_length;
  
  return *f != NULL;
}

/* Test if the Pascal string BASE concatenated with the extension
   `.SUFFIX' is the same file as just BASE.  SUFFIX is a C string.  */

boolean
extensionirrelevantp (base, suffix)
    char *base;
    char *suffix;
{
  boolean ret;
  char temp[PATH_MAX];
  
  make_c_string (&base);
  strcpy (temp, base);
  strcat (temp, ".");
  strcat (temp, suffix);
  
  ret = same_file_p (base, temp);
  make_pascal_string (&base);
  
  return ret;
}
