/* Copyright 1992 Karl Berry
   Copyright 2007 TeX Users Group

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

#ifdef _WINDOWS
#define NOCOMM
#define NOSOUND
#define NODRIVERS
#define STRICT
#pragma warning(disable:4115)	// kill rpcasync.h complaint
#include <windows.h>
#endif

#ifdef _WINDOWS
// We must define MYLIBAPI as __declspec(dllexport) before including
// texwin.h, then texwin.h will see that we have already
// defined MYLIBAPI and will not (re)define it as __declspec(dllimport)
#define MYLIBAPI __declspec(dllexport)
// #include "texwin.h"
#endif

#include "texwin.h"

#pragma warning(disable:4131)	// old style declarator
#pragma warning(disable:4135)	// conversion between different integral types 
#pragma warning(disable:4127)	// conditional expression is constant

#include <setjmp.h>

#pragma hdrstop

#include <direct.h>						/* for _getcwd() */

#define EXTERN extern

#include "texd.h"

#define PATH_SEP '/'
#define PATH_SEP_STRING "/"

/* openinout.c: open input and output files.  These routines used by
   TeX, Metafont, and BibTeX.  */

/* #include <sys\stat.h> */				/* debugging 94/Mar/2 */
/* #include <sys\types.h> */			/* debugging 94/Mar/2 */
/* #include <conio.h>	*/				/* for getch */

// #include "config.h"
// #include "c-namemx.h"
// #include "c-pathch.h"

#define BUILDNAMEDIRECT					/* avoid malloc for string concat */

/* int _cdecl _access(const char *pathname, int mode); */	/* debugging */
/* #define access _access */

booleane testreadaccess (unsigned char *, int); 	/* in ourpaths.c - bkph */
/* booleane testreadaccess (char *, int, int); */	/* in ourpaths.c - bkph */

extern unsigned char *unixify (unsigned char *);			/* in pathsrch.c bkph */

extern void tryandopen (char *);		/* in local.c bkph */

extern int shortenfilename;				/* in local.c bkph */

#ifdef FUNNY_CORE_DUMP
/* This is defined in ./texmf.c.  */
extern void funny_core_dump ();
#endif /* FUNNY_CORE_DUMP */



#ifdef MSDOS

#ifdef BUILDNAMEDIRECT
/* similar to concat, but AVOIDS using malloc, pass in place to put result */
char *xconcat (char *buffer, char *s1, char *s2) {
	int n1 = strlen(s1);
	int n2 = strlen(s2);
	if (buffer == s2) {			/* treat special case of overlap */
		memmove (buffer + n1, buffer, n2 + 1); /* trailing null ! */
		strncpy (buffer, s1, n1);
	}
	else {
		strcpy(buffer, s1);
		strcat(buffer + n1, s2);
	}
	return buffer;
}

/* similar to concat3, but avoids using malloc, pass in place to put result */
char *xconcat3 (char *buffer, char *s1, char *s2, char *s3) {
	int n1 = strlen(s1);
	int n2 = strlen(s2);
	int n3 = strlen(s3);
	if (buffer == s3) {			/* treat special case of overlap */
		memmove (buffer + n1 + n2, buffer, n3 + 1); /* trailing null ! */
		strncpy (buffer, s1, n1);
		strncpy (buffer + n1, s2, n2);
	}
	else {
		strcpy(buffer, s1);
		strcat(buffer + n1, s2);
		strcat(buffer + n1 + n2, s3);
	}
	return buffer;
}
#endif /* end of BUILDNAMEDIRECT  */

#endif  /* end of ifdef MSDOS ??? */

#ifdef MSDOS
/* separated out 1996/Jan/20 to make easier to read */
/* assumes path does not end in PATH_SEP */
void patch_in_path (unsigned char *buffer, unsigned char *name, unsigned char *path) {
#ifdef BUILDNAMEDIRECT
	if (*path == '\0') strcpy(buffer, name);
	else xconcat3(buffer, path, PATH_SEP_STRING, name);
#else
	string temp_name;
	temp_name = concat3 (path, PATH_SEP_STRING, name);
	strcpy (buffer, temp_name);
	free (temp_name);
#endif
}

int qualified (unsigned char *name) {
	if (strchr(name, PATH_SEP) != NULL ||
		strchr(name, '\\') != NULL ||
		strchr(name, ':') != NULL) return 1;
	else return 0;
}

/* patch path if (i) path not empty (ii) name not qualified (iii) ext match */

int prepend_path_if (unsigned char *buffer, unsigned char *name, char *ext, unsigned char *path) {
	if (path == NULL) return 0;
	if (*path == '\0') return 0;
	if (qualified(name)) return 0;
	if (strstr(name, ext) == NULL) return 0;
	patch_in_path(buffer, name, path);
	return 1;
}
#endif			/* end of MSDOS */

/*  Following works on null-terminated strings */

/* void checkshortname(void) { */				/* 1995/Feb/20 */
void checkshortname (unsigned char *s) {					/* 1995/Sep/26 */
	unsigned char *star, *sdot;
	int n;

/*	if ((star = strrchr(nameoffile+1, '\\')) != NULL) star++;
	else if ((star = strrchr(nameoffile+1, '/')) != NULL) star++;
	else if ((star = strchr(nameoffile+1, ':')) != NULL) star++;
	else star = nameoffile+1; */				/* 1995/Sep/26 */
	if ((star = strrchr(s, '\\')) != NULL) star++;
	else if ((star = strrchr(s, '/')) != NULL) star++;
	else if ((star = strchr(s, ':')) != NULL) star++;
	else star = s;
	if ((sdot = strchr(star, '.')) != NULL) n = sdot - star;
	else n = strlen(star);
	if (n > 8) strcpy(star+8, star+n);
	if ((sdot = strchr(star, '.')) != NULL) {
		star = sdot+1;
		n = strlen(star);
		if (n > 3) *(star+3) = '\0';
	}
}

/* Following works on both null-terminated names */
/* reconvert 254 to '~' in file name 95/Sep/26 */
/* reconvert 255 to ' ' in file name 95/Sep/26 */
/* we do this in tex3.c startinput() -> scanfilename() now 95/Sep/26 */

void retwiddle (unsigned char *s) {	/* assumes null terminated - 97/June/5 */
/*	while (*s != '\0' && *s != ' ') { */
	while (*s != '\0') {
		if (*s == (unsigned char) pseudotilde) *s = '~';
		else if (*s == (unsigned char) pseudospace) *s = ' ';
		s++;
	}
} 

/* #endif */ /* ??? */

/* Open an input file F, using the path PATHSPEC and passing
   FOPEN_MODE to fopen.  The filename is in `nameoffile', as a Pascal
   string. We return whether or not the open succeeded.  If it did, we
   also set `namelength' to the length of the full pathname that we
   opened.  */

booleane open_input (FILE **f, path_constant_type path_index, char *fopen_mode) {
	booleane openable = false;

#if defined (FUNNY_CORE_DUMP) && !defined (BibTeX)
/*	This only applies if a preloaded TeX (or Metafont) is being made;
	it allows for automatic creation of the core dump (typing ^\
	requires manual intervention).  */
	if (path_index == TEXINPUTPATH
      && strncmp (nameoffile+1, "HackyInputFileNameForCoreDump.tex", 33) == 0)
		funny_core_dump ();
#endif /* FUNNY_CORE_DUMP and not BibTeX */

#ifdef MSDOS
	if (returnflag) {
		if (strcmp(fopen_mode, "r") == 0)
			fopen_mode = "rb";		/* so can catch `return' bkph */
	}
#endif /* MSDOS */

/*	null_terminate (nameoffile + 1);  */	/* moved here 97/June/5 */
	nameoffile[namelength+1] = '\0';	/* null terminate */

/* reinsert '~' and ' ' in file names -  95/June/5 */
/* done late to prevent problems with  null_terminate / space_terminate */  
	if (pseudotilde != 0 || pseudospace != 0) {
		retwiddle(nameoffile + 1);
	}

#ifdef MSDOS
	if (shortenfilename) {	/* 8 + 3 file names on Windows NT 95/Feb/20 */
/*		null_terminate (nameoffile + 1); */
		checkshortname(nameoffile + 1);						/* 95/Sep/26 */
/*		space_terminate (nameoffile + 1); */
	}
#endif	/* MSDOS */

#ifdef BibTeX
	if (path_index == NO_FILE_PATH) {
		unsigned temp_length;

/*      null_terminate (nameoffile + 1); */
/*  if shareflag is non-zero and we are opening for reading use fsopen */
/*	but we can assume here that we are opening for *input* */
/*      *f = fopen (nameoffile + 1, fopen_mode); */
		if (shareflag == 0)  *f = fopen (nameoffile + 1, fopen_mode);
		else *f = _fsopen (nameoffile + 1, fopen_mode, shareflag);
		temp_length = strlen (nameoffile + 1);
/*      space_terminate (nameoffile + 1); */

		if (*f != NULL)	{
			namelength = temp_length;
			openable = true;
		}
	}

	else
#endif /* BibTeX */
  
	if (opentraceflag) {
/*      null_terminate (nameoffile + 1); */
		sprintf(logline, " Open `%s' for input ", nameoffile+1);	/* Pascal */
		showline(logline, 0);
/*      space_terminate (nameoffile + 1); */
	}		// debugging only

	if (testreadaccess(nameoffile+1, path_index)) { 
/*	if (testreadaccess(nameoffile, namelength, path_index)) */

/*		We can assume `nameoffile' is openable, */
/*		since `testreadaccess' just returned true.  */
/*		*f = xfopen_pas (nameoffile, fopen_mode); */
		*f = xfopen(nameoffile+1, fopen_mode);

//		should we check *f == NULL ??? (should be OK because of	testreadaccess)

/*		If we found the file in the current directory, don't leave the
        `./' at the beginning of `nameoffile', since it looks dumb when
        TeX says `(./foo.tex ... )', and analogously for Metafont.  */
#ifdef MSDOS
		if (nameoffile[1] == '.' &&					/* 1994/Mar/1 */
			(nameoffile[2] == PATH_SEP || nameoffile[2] == '\\'))
#else
		if (nameoffile[1] == '.' && nameoffile[2] == PATH_SEP) 
#endif
		{
			unsigned i = 1;
/*				while (nameoffile[i + 2] != ' ') */
			while (nameoffile[i + 2] != '\0')
			{
				nameoffile[i] = nameoffile[i + 2];
				i++;
			}
/*			nameoffile[i] = ' '; */
			nameoffile[i] = '\0';
			namelength = i - 1;
		}
		else
/*			namelength = strchr(nameoffile + 1, ' ') - (nameoffile + 1); */
			namelength = strlen(nameoffile + 1);
      
#ifdef TeX
/*		If we just opened a TFM file, we have to read the first byte,
        since TeX wants to look at it.  What a kludge.  */
		if (path_index == TFMFILEPATH)
		{ /* See comments in ctex.ch for why we need this.  */
/*          extern integer tfmtemp; */	/* see texd.h for definition */
			tfmtemp = getc (*f);
		}
#endif /* TeX */  

#ifdef MSDOS
/*		code added 94/June/21 to show 'fmt' file opening in log */
/*		if (showfmtflag && strstr(nameoffile+1, ".fmt") != NULL) { */
		if (strstr(nameoffile+1, ".fmt") != NULL) {
			if (formatfile == NULL) {
/*				null_terminate (nameoffile + 1); */
				formatfile = xstrdup(nameoffile + 1);
/*				space_terminate (nameoffile + 1); */
			}
		}	/* remember full format file name with path */
/*		if (showpoolflag && strstr(nameoffile+1, ".poo") != NULL) { */
		else if (strstr(nameoffile+1, ".poo") != NULL) {
			if (stringfile == NULL) {
/*				null_terminate (nameoffile + 1); */
				stringfile = xstrdup(nameoffile + 1);
/*				space_terminate (nameoffile + 1); */
			}
		}	/* remember full pool file name with path */
/*		else if (showtfmflag && strstr(nameoffile+1, ".tfm") != NULL) { */
		else if (strstr(nameoffile+1, ".tfm") != NULL) {
			if (showtfmflag && logopened) {
#ifdef WRAPLINES
				int oldsetting = selector;
				char *s = nameoffile + 1;
				selector = 18;			/* log file only */
				printchar (' ');
				printchar (40);			/* ( */
/*				while (*s > ' ') printchar (*s++); */
				while (*s != '\0') printchar (*s++);
				printchar (41);			/* ) */
				selector = oldsetting;
#else
				int n; 
/*				null_terminate (nameoffile + 1); */
				n = strlen(nameoffile+1);
				if (fileoffset + n > maxprintline) {
					putc('\n', logfile);
					fileoffset = 0;
				}	/* somewhat risky ? */
				else putc(' ', logfile);
				fprintf(logfile, "(%s)", nameoffile+1);
				fileoffset += n+3;
/*				space_terminate (nameoffile + 1);  */
#endif	/*  end of WRAPLINES */
			}
		}
/*		code added 98/Sep/29 to catch first file input */
/*		is there a problem if this file bombs ? */
		else if (sourcedirect == NULL) {			/* 98/Sep/29 */
			char *s;
/*			null_terminate (nameoffile + 1); */
			sourcedirect = xstrdup(nameoffile + 1);
/*			space_terminate (nameoffile + 1); */
			if (traceflag) {
				sprintf(logline, "Methinks the source %s is `%s'\n", "file", sourcedirect);
				showline(logline, 0);
			}
			if ((s = strrchr(sourcedirect, '/')) == NULL) *sourcedirect='\0';
			else *(s+1) = '\0';
			if (traceflag) {
				sprintf(logline, "Methinks the source %s is `%s'\n", "directory", sourcedirect);
				showline(logline, 0);
			}
		}

#endif	/* end of MSDOS */
		openable = true;
	}
/*	space_terminate (nameoffile + 1); */
	{
		unsigned temp_length = strlen(nameoffile + 1);
		nameoffile[temp_length+1] = ' ';	/* space terminate */
/*		set up namelength ??? */
	}
	return openable;
}

/* Call the external program PROGRAM, passing it `nameoffile'.  */
/* This nonsense probably only works for Unix anyway. bkph */
/* For one thing, MakeTeXTFM etc is more than 8 characters ! */

#ifdef MSDOS
#define NO_MAKETEX
#endif

/* the string program is unreferenced in DOS NO_MAKETEX */

static booleane
make_tex_file (program)
    string program;
{
#ifdef NO_MAKETEX
  return 0;
#else
  char cmd[NAME_MAX + 1 + PATH_MAX + 1];
  unsigned cmd_len;
  int ret;
  unsigned i = 1; /* For copying from `nameoffile'.  */

  /* Wrap another sh around the invocation of the MakeTeX program, so we
     can avoid `sh: MakeTeXTFM: not found' errors confusing the user.
     We don't use fork/exec ourselves, since we'd have to call sh anyway
     to interpret the script.  */
#ifdef MSDOS
  strcpy (cmd, "command.com ");
#else
  strcpy (cmd, "sh -c ");
#endif
  
/*  strcat (cmd, program); */	/* shrouded 93/Nov/20 */
  strcat (cmd, "Make");
#ifndef MSDOS
  strcat (cmd, "TeX");
#endif
  strcat (cmd, program);
  cmd_len = strlen (cmd);
  cmd[cmd_len++] = ' ';

  while (nameoffile[i] != ' ')
    cmd[cmd_len++] = nameoffile[i++];

  /* Add terminating null.  */
  cmd[cmd_len] = 0;

  /* Don't show any output.  */
#ifdef MSDOS
  strcat (cmd, "> nul");	/* ? 93/Nov/20 */
#else
  strcat (cmd, ">/dev/null 2>&1");
#endif

/* Run the command, and return whether or not it succeeded.  */
  ret = system (cmd);
  return ret == EXIT_SUCCESS_CODE;
#endif /* not NO_MAKE_TEX */
}

#define TEXONLY

/* This is called by TeX if an \input resp. TFM file can't be opened.  */

booleane maketextex ()					/* called in tex3.c and tex8.c */
{
/*  return make_tex_file ("MakeTeXTeX"); */
  return make_tex_file ("TeX"); 
}

booleane maketextfm ()					/* called in tex3.c */
{
/*  return make_tex_file ("MakeTeXTFM"); */
  return make_tex_file ("TFM");
}

#ifndef TEXONLY
booleane
maketexmf ()
{
/*  return make_tex_file ("MakeTeXMF"); */
  return make_tex_file ("MF");
}
#endif /* ifndef TEXONLY */


char *getenvshroud (char *);		/* defined in texmf.c */

/* char outputdirectory[PATH_MAX]; */				/* defined in local.c */

extern char *dvidirectory;				/* defined in local.c */
extern char *logdirectory;				/* defined in local.c */
extern char *auxdirectory;				/* defined in local.c */

#ifdef IGNORED
/* Try and figure out if can write to current directory */
booleane isitsafe (char *name) {
/*  struct stat statbuf; */					/* debugging 94/Mar/2 */
/*	Can't test access on file, since fails if not exist */
/*	Can't test access on `nul', since always fails */ 
/*	Can   test access on `.', but its true on locked diskette! */
/*	stat on directory always says read an write permission */
	return true;				/* for now */
}
#endif

/* open_output moved to end to avoid pragma problems 96/Sep/15 */


/* used only in startinput in tex3.c, and in openorclosein in tex8.c */
/* modified 97/June/5 to take null terminated (C) string */

#ifdef IGNORED
booleane extensionirrelevantaux (char *base, char *suffix)  { 
  booleane ret;
/*  make_c_string (&base);  */
/*  base[nlen+1] = '\0'; */			/* null terminate */
#ifdef MSDOS
/*	In DOS, an extension is irrelevant if there already is an extension ! */
/*	MAY NEED TO REVISE IN WIN32 where we can have foo.bar.chomp.tex ??? */
  {								/* simplification 1996/Jan/20 ??? */
	  char *s, *t;
	  if ((s = strrchr (base, '.')) == NULL) ret = 0;	/* no extension */
	  else {
		  if ((t = strrchr (base, PATH_SEP)) != NULL ||
			  (t = strrchr (base, '\\')) != NULL ||
			  (t = strrchr (base, ':')) != NULL) {
			  if (t > s) ret = 0;	/* last dot occurs in path - no extension */
			  else ret = 1;			/* last dot occurs in file name itself */
		  }
		  else ret = 1;				/* name not qualified and has dot */
	  }
  }
#else	/*  not MSDOS */
  {
	  char temp[PATH_MAX];
	  strcpy (temp, base);
	  strcat (temp, ".");
	  strcat (temp, suffix);
	  ret = same_file_p (base, temp);
  }
#endif /* end of not MSDOS */
/*  make_pascal_string (&base); */
/*  base[nlen+1] = ' '; */			/* space terminate */
  return ret;
}
#endif /* IGNORED */

/* Test if the Pascal string BASE concatenated with the extension
   `.SUFFIX' is the same file as just BASE.  SUFFIX is a C string.  */

/* used in `startinput' (tex3.c) and openorclosein (tex8.c) */
/* used to always return true, since in DOS can have only one extension */
/* modified 98/Feb/7 to always return false */

booleane extensionirrelevantp (unsigned char *base, int nlen, char *suffix)  { 
#ifdef IGNORED
  booleane ret;
  base[nlen+1] = '\0';			/* null terminate */
  ret = extensionirrelevantaux(base+1, suffix);
  base[nlen+1] = ' ';			/* space terminate */
  return ret;
#endif
  return false;
}


/* #define aclose(f) if (f) { if (ferror (f)) {perror(""); exit(1);} } if (f) (void) fclose (f) */
/* #define aclose(f)	if (f) (void) checkfclose (f) */

/* At least check for I/O error (such as disk full) when closing */
/* Would be better to check while writing - but this is better than nothing */
/* This is used for both input and output files, but never mind ... */

/* now aclose returns -1 on error --- which could be used by caller */
/* probably want to ignore on input files ... */

void perrormod (char *s);				/* in local.c */

// checkfclose not used by anything

int checkfclose (FILE *f) {				/* 1993/Nov/20 - bkph */
	if (f == NULL) return 0;			// sanity check
	if (ferror(f) || fclose (f)) {
		perrormod("\n! I/O Error");
		uexit (1);		// ???
	}
	return 0;
}

/* open_output moved down here to avoid potential pragma problem */

/* #pragma optimize ("g", off) *//* try and avoid compiler bug here */

/* Open an output file F either in the current directory or in
   $TEXMFOUTPUT/F, if the environment variable `TEXMFOUTPUT' exists.
   (Actually, this applies to the BibTeX output files, also, but
   `TEXMFBIBOUTPUT' was just too long.)  The filename is in the global
   `nameoffile', as a Pascal string.  We return whether or not the open
   succeeded.  If it did, the global `namelength' is set to the length
   of the actual filename.  */

booleane open_output (FILE **f, char *fopen_mode) {
	unsigned temp_length;

/*	null_terminate (nameoffile + 1);	*/	/* moved here 95/Sep/26  */
	nameoffile[namelength+1] = '\0';	/* null terminate */

/* reinsert '~' and ' ' in file names -  95/June/5 */
/* done late to prevent problems with  null_terminate / space_terminate */  
	if (pseudotilde != 0 || pseudospace !=  0) {
		retwiddle(nameoffile + 1);
	}

#ifdef MSDOS
	if (shortenfilename) {	/* 8 + 3 file names on Windows NT 95/Feb/20 */
/*		null_terminate (nameoffile + 1);  */
/*		checkshortname(); */
		checkshortname(nameoffile + 1);					/* 95/Sep/26 */
/*		space_terminate (nameoffile + 1); */
	}
#endif

/*  null_terminate (nameoffile + 1); */ /* Make the filename into a C string.  */

/*  if (debugflag) tryandopen(nameoffile+1); */	/* debugging 94/Mar/20 */

#ifdef MSDOS
/* write into user specified output directory if given on command line */
/* following code added 1993/Dec/12 */ /* separated 1996/Jan/20 */
	if (prepend_path_if(nameoffile+1, nameoffile+1, ".dvi", dvidirectory) ||
		prepend_path_if(nameoffile+1, nameoffile+1, ".log", logdirectory) ||
		prepend_path_if(nameoffile+1, nameoffile+1, ".aux", auxdirectory)) {
		if (opentraceflag) {
			sprintf(logline, "After prepend %s\n", nameoffile+1);
			showline(logline, 0);
		}
	}
#endif

/* namelength recomputed below so don't need to do it yet */

	if (opentraceflag) {
/*      null_terminate (nameoffile + 1); */
		sprintf(logline, " Open `%s' for output ", nameoffile+1); /* C string */
		showline(logline, 0);
/*      space_terminate (nameoffile + 1); */
	}		// debugging only

/* Is the filename openable as given?  */

/*  if shareflag is non-zero and we are opening for reading use fsopen */
/*	but we can assume this is opening here for *output* */
	*f = fopen(nameoffile + 1, fopen_mode);

	if (*f == NULL)    { /* Can't open as given.  Try the envvar.  */
/*    string temp_dir = getenv ("TEXMFOUTPUT"); */	/* 93/Nov/20 */
/*    string temp_dir = getenv ("TEXMFOUT"); */	/* 93/Nov/20 */
/*      string temp_dir = getenvshroud ("UFYNGPVUQVU"); */
		string temp_dir = getenvshroud ("UFYNGPVU");

/*		if (deslash) unixify(temp_dir); */		/* deslashify 93/Dec/28 */

		if (temp_dir != NULL) {
#ifdef BUILDNAMEDIRECT
			unsigned char temp_name[PATH_MAX];
			xconcat3(temp_name, temp_dir, PATH_SEP_STRING, nameoffile + 1);
#else
/*          string temp_name = concat3 (temp_dir, "/", nameoffile + 1); */
			string temp_name = concat3 (temp_dir, PATH_SEP_STRING, nameoffile + 1);
#endif
			if (deslash) unixify(temp_name); 		/* deslashify 93/Dec/28 */
/*	If shareflag is non-zero and we are opening for reading use fsopen */
/*	but we can assume this is opening here for *output* */
			*f = fopen(temp_name, fopen_mode);
/*	If this succeeded, change nameoffile accordingly.  */
			if (*f) strcpy(nameoffile + 1, temp_name);
#ifndef BUILDNAMEDIRECT
			free (temp_name);
#endif
		}
	}

//	showline(nameoffile+1, 1);		// debugging only
//	New code to remember complete dvifile name and logfilename
//	To remember for output at the end 2000 June 18
	if (strstr(nameoffile + 1, ".dvi") != NULL) {
		if (qualified(nameoffile+1)) *logline = '\0';
		else {
			(void) _getcwd(logline, sizeof(logline));
			strcat(logline, PATH_SEP_STRING);
		}
		strcat(logline, nameoffile+1);
		unixify(logline);
		dvifilename = xstrdup(logline);
//		showline(dvifilename, 1);	// debugging only
	}
	else if (strstr(nameoffile + 1, ".log") != NULL) {
		if (qualified(nameoffile+1)) *logline = '\0';
		else {
			(void) _getcwd(logline, sizeof(logline));
			strcat(logline, PATH_SEP_STRING);
		}
		strcat(logline, nameoffile+1);
		unixify(logline);
		logfilename = xstrdup(logline);
//		showline(logfilename, 1);	// debugging only
	}
/* Back into a Pascal string, but first get its length.  */
	temp_length = strlen (nameoffile + 1);
/*	space_terminate (nameoffile + 1); */
	nameoffile[temp_length+1] = ' ';	/* space terminate */

/* Only set `namelength' if we succeeded.  I'm not sure why.  */
	if (*f) 							/* TEST ? 94/MAR/2 */
		namelength = temp_length;
  
	return *f != NULL;
}


/* #pragma optimize ("g", ) */ 	/* try and avoid compiler bug here */
/* #pragma optimize ("", on) */ 	/* try and avoid compiler bug here */

