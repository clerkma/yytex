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

#define EXTERN extern

#include "texd.h"

#include <io.h>					// needed for _finddata_t 
#include <ctype.h>				// needed for isascii and isalpha

//////////////////////////////////////////////////////////////////////////////////

#define NAME_MAX 255			// max size of name component

#define ISALPHA(c) (isascii (c) && isalpha(c))

#define PATH_SEP '/'
#define PATH_SEP_STRING "/"
#define PATH_DELIMITER ';'
#define PATH_DELIMITER_STRING ";"

// default paths to look for things

#define TEXPATH "c:/yandy/yandytex/"

#define TEXFORMATS TEXPATH "fmt"
#define TEXPOOL TEXPATH "fmt"
#define TEXFONTS TEXPATH "tfm"
#define TEXINPUTS TEXPATH "tex//;" "c:/tex;" "c:/texinput"

// structure used by fontmap

typedef struct map_element_struct {
	char *key;
	char *value;
	struct map_element_struct *next;
} map_element_type;

typedef map_element_type **map_type;

extern booleane usesourcedirectory;			/* in local.c */

extern booleane workingdirectory;			/* in local.c */

booleane absolute_p (string filename);
string readable (string name);
string truncate_pathname (string name);
char *file_p(string fn);
booleane dir_p(string fn);
string *find_dir_list (string path);
void add_directory (string **dir_list_ptr, unsigned *dir_count_ptr, string dir);
void expand_subdir (string **dir_list_ptr, unsigned *dir_count_ptr, string dirname,
					struct _finddata_t findt, integer recurseflag);
void save_dir_list (string path,  string *dir_list);
string *initialize_path_list (string env_name,  string default_path);
int xfind_path_filename (string buffer, string filename,  string * dir_list);
string expand_default (string env_path, string default_path);
map_type map_create (string *dir_list);
char *map_lookup (map_type map, char *key);

// the following do *not* use MALLOC

extern char *xconcat (char *, char *, char *);				/* openinou.c */
extern char *xconcat3 (char *, char *, char *, char *);		/* openinou.c */

/////////////////////////////////////////////////////////////////////////

// used only in jumpout in tex0.c, and in texbody in itex.c
// and main in texmf.c and a few other abort situations in texmf.c

int uexit (int unix_code) {
	int code;

#ifndef _WINDOWS
	fflush(stdout);
#endif
	if (unix_code == 0)	code = EXIT_SUCCESS_CODE;
	else if (unix_code == 1) code = ! EXIT_SUCCESS_CODE;
	else code = unix_code;
	if (jumpused) {
		showline("Jump Buffer already used\n", 1);
		exit(1);
	}
	jumpused++;
	longjmp(jumpbuffer, code+1);		// 1999/Nov/7
}

// round a double being careful about very large and very small values

// used only in tex0.c

integer zround (double r) {
	integer i;

/*	R can be outside the range of an integer if glue is stretching or
	shrinking a lot.  We can't do any better than returning the largest
	or smallest integer possible in that case.  It doesn't seem to make
	any practical difference. */

	if (r > LONG_MAX) i = LONG_MAX;
	else if (r < LONG_MIN) i = LONG_MIN;
	else if (r >= 0.0) i = (int) (r + 0.5);
	else i = (int) (r - 0.5);
	return i;
}

/****************************************************************************/

// malloc with error checking and error message

address xmalloc (unsigned size) {
	address new_mem = (address) malloc (size);
	if (new_mem == NULL) {
		sprintf(logline, "malloc: Unable to honor request for %u bytes.\n", size);
		showline(logline, 1);
		abort ();					// ???
	}
#ifdef MYDEBUG
	if (traceflag) {
		sprintf(logline, "XMALLOC %d\n", size);		/* 1996/Jan/20 */
		showline(logline, 0);
	}
#endif
	return new_mem;
}

// calloc with error checking - used by map_create

address xcalloc (unsigned nelem, unsigned elsize) {
	address new_mem = (address) calloc (nelem, elsize);
	if (new_mem == NULL) {
		sprintf(logline,
				"Unable to honor request for %u elements of size %u.\n",
				nelem, elsize);
		showline(logline, 1);
		abort ();
	}
	return new_mem;
}

/* Return a copy of s in new storage.  */ /* xmalloc does error checking */

string xstrdup (string s) {
	string new_string = (string) xmalloc (strlen (s) + 1);
#ifdef MYDEBUG
	if (traceflag) {
		sprintf(logline, "XSTRDUP %d %s\n", strlen(s)+1, s);
		showline(logline, 0);
	}
#endif
	return strcpy (new_string, s);
}

/* only used by line.c (which in turn is only used by  fontmap.c) */

address xrealloc (address old_ptr, unsigned size) {
	address new_mem;

	if (old_ptr == NULL) new_mem = xmalloc (size);
	else {
		new_mem = (address) realloc (old_ptr, size);
		if (new_mem == NULL) {
			sprintf(logline, "Unable to honor request for %u bytes.\n", size);
			showline(logline, 1);
			abort ();
		}
	}
	return new_mem;
}

// returns newly allocated string

string concat (string s1, string s2) {
	string answer;
#ifdef MYDEBUG
	if (traceflag) {
		sprintf(logline, "CONCAT %s and %s ", s1, s2);
		showline(logline, 0);
	}
#endif
	answer = (string) xmalloc (strlen (s1) + strlen (s2) + 1);
	strcpy (answer, s1);
	strcat (answer, s2);
#ifdef MYDEBUG
	if (traceflag) {
		sprintf(logline, "=> %s\n", answer);
		showline(logline, 0);
	}
#endif
	return answer;
}

// returns newly allocated string

string concat3 (string s1, string s2, string s3) {
	string answer;
#ifdef MYDEBUG
	if (traceflag) {
		sprintf(logline, "CONCAT3 %s, %s, and %s ", s1, s2, s3);
		showline(logline, 0);
	}
#endif
	answer = (string) xmalloc (strlen (s1) + strlen (s2) + strlen (s3) + 1);
	strcpy (answer, s1);
	strcat (answer, s2);
	strcat (answer, s3);
#ifdef MYDEBUG
	if (traceflag) {
		sprintf(logline, "=> %s\n", answer);
		showline(logline, 0);
	}
#endif
	return answer;
}

/***********************************************************************/

// following used only in itex.c on pool file

/* Return true if we're at the end of FILE, else false.  This implements
   Pascal's `eof' builtin.  */
/* It differs from C feof in that the latter is not true at end of file
   unless an attempt has actually been made to read past EOF */

booleane test_eof (FILE *file) {
	int c;
/* Maybe we're already at the end?  */
	if (feof (file)) return true;
	if ((c = getc (file)) == EOF) return true;
/* We weren't at the end.  Back up.  */
	(void) ungetc (c, file);
	return false;
}

/* Return true on end-of-line in FILE or at the end of FILE, else false.  */

booleane eoln (FILE *file) {
	int c;
	if (feof (file)) return true;
	c = getc (file);
	if (c != EOF) (void) ungetc (c, file);
//	return c == '\n' || c == EOF;
	return c == '\n' || c == '\r' || c == EOF; // ???
/* Type mismatch (return) (int/enum) ? */
}

/***********************************************************************/

// following used only by fontmap.c and openinou.c

// #define FATAL_PERROR(s) do { perrormod (s); exit (errno); } while (0)

// perrormod puts error message on stdout instead of stderr

/* These routines just check the return status from standard library
   routines and abort if an error happens.  */

// xfopen used in open_input in openinou.c

FILE *xfopen (char *filename, char *mode) {
	FILE *f;
/*  if shareflag is non-zero and we are opening for reading use fsopen */
/*  f = fopen (filename, mode); */
	if (shareflag == 0 || *mode != 'r') f = fopen (filename, mode);
	else f = _fsopen (filename, mode, shareflag);
	if (f == NULL) {
//		FATAL_PERROR (filename);
		perrormod(filename);
		uexit(1);		// ???
	}
	return f;
}

// xfclose not used ...

int xfclose (FILE *f, char *filename) {
	if (ferror(f) != 0 || fclose(f) != 0) {
//		FATAL_PERROR (filename);
		perrormod(filename);
		uexit(1);		// ???
	}
	return 0;
}

/********************************************************************************/

// following used only in map_lookup

// return pointer to start of extension --- or NULL if there isn't one

string find_suffix (string name) {
	string dot_pos; 
	string slash_pos; 

	dot_pos = strrchr (name, '.');
#ifdef MSDOS
	if ((slash_pos = strrchr (name, PATH_SEP)) != NULL) ;
	else if ((slash_pos = strrchr (name, '\\')) != NULL) ;  
	else if ((slash_pos = strrchr (name, ':')) != NULL) ;
	else slash_pos = name;
#else
	slash_pos = strrchr (name, PATH_SEP);
#endif

/* If the name is `foo' or `/foo.bar/baz', we have no extension.  */
	return dot_pos == NULL || dot_pos < slash_pos ? NULL : dot_pos + 1;
}

// remove extension of file name - returns copy or NULL

string remove_suffix (string s) {
	string ret;
	string suffix = find_suffix (s);

	if (suffix)	{
		suffix--;	  /* Back up to before the dot.  */
		ret = (char *) xmalloc (suffix - s + 1);
		strncpy (ret, s, suffix - s);
		ret[suffix - s] = '\0';
	}
	else ret = NULL;

	return ret;
}

// add extension to file name unless it already has one
// returns copy or the old one (warning: danger when freeing)

string extend_filename (string name, string default_suffix) {
	string new_s;
	string suffix = find_suffix (name);

	new_s = suffix == NULL 
			? concat3 (name, ".", default_suffix) : name;
	return new_s;
}

/****************************************************************************************/

#ifdef MALLOCLINE

#define BLOCK_SIZE 64

// this returns newly allocated character string

char *read_line (FILE *f) {
	int c;
	unsigned limit = BLOCK_SIZE;
	unsigned loc = 0;
	char *line = (char *) xmalloc (limit);

/*  while ((c = getc (f)) != EOF && c != '\n')  */
	while ((c = getc (f)) != EOF && c != '\n' && c != '\r') {
		line[loc] = (char) c;
		loc++;

	  /* By testing after the assignment, we guarantee that we'll always
         have space for the null we append below.  We know we always
         have room for the first char, since we start with BLOCK_SIZE.  */
		if (loc == limit) {
			limit += BLOCK_SIZE;
			line = (char *) xrealloc (line, limit);
		}
	}

  /* If we read anything, return it.  This can't represent a last
     ``line'' which doesn't end in a newline, but so what.  */
  /* This is Tom Rokicki's mistake -- lets fix it ! 1994/March/18 */
	if (c != EOF)  {
	  /* Terminate the string.  We can't represent nulls in the file,
         either.  Again, it doesn't matter.  */
		line[loc] = 0;
	}
	else if (loc > 0) {	/* c == EOF, but line not empty 1994/March/18 */
		line[loc] = 0;
	}
	else { /* Real EOF --- at end of file.  */
		free (line);
		line = NULL;
	}

	return line;
}
#endif

/* Modified version 97/May/17 to avoid malloc for every line read ... */

char *read_a_line (FILE *f,  char *line, int limit) {
	int c;
	int loc = 0;

/*	while ((c = getc (f)) != EOF && c != '\n')  */
	while ((c = getc (f)) != EOF) {
		if (c == '\n' || c == '\r') {
			if (loc > 0) break;
			else continue;				/* ignore \r\n and blank lines */
		}
		line[loc] = (char) c;
		loc++;
		if (loc == limit-1) {			/* very unlikely */
			sprintf(logline, " ERROR: line too long\n");
			showline(logline, 1);
			showline(line, 0);
			showline("\n", 0);
			break;
		}
	}

	if (c != EOF || loc > 0) {			/* normal line or EOF at end of line */
		line[loc] = '\0';				/* terminate */
		return line;					/* and return */
	}
	else return(NULL);					/* true EOF */
}

/****************************************************************************************/

/* from ourpaths.c */

#define BUILDNAMEDIRECT		/* avoid malloc for string concat */

#define CACHEFILENAME		/* cache last full path/file name 96/Nov/16 */
							/* to speed up LaTeX 2e which opens files twice */

/* `path_dirs' is initialized in `setpaths', to a null-terminated array
   of directories to search for.  */

static string *path_dirs[LAST_PATH];

/* This sets up the paths, by either copying from an environment variable
   or using the default path, which is defined as a preprocessor symbol
   (with the same name as the environment variable) in `site.h'.  The
   parameter PATH_BITS is a logical or of the paths we need to set.  */

void setpaths (int path_bits) {
	int n;											/* 97/Apr/2 */
	char *s, *t, *u;								/* 94/Jan/6 */
	char buffer[PATH_MAX];

/*	eliminated lots of junk not needed for TeX itself 93/Nov/20 bkph */

/*	added code to look for pool file in format directory also */
/*	added code to look for fmt directory in tex directory also */
/*	added code to look for tfm directory in tex directory also */
/*	added code to look for some PC TeX flavour environment names 94/Jan/6 */
/*	which, in case of formats, could lead to I'm stymied errors ... */

	if (path_bits & TEXFORMATPATHBIT) {
		s = "TEXFORMATS";
		t = TEXFORMATS;
/*	  if (getenv(s) == NULL) {	*/	/* see if env var defined */
		if (grabenv(s) == NULL) {		/* see if env var defined 94/May/19*/
			strcpy(buffer, texpath);	/* not, see if texpath\fmt is directory */
			strcat(buffer, PATH_SEP_STRING); 
			strcat(buffer, "fmt");
			if (traceflag) {
				sprintf(logline, "Checking `%s' = %s %s %s\n",
						buffer, texpath, PATH_SEP_STRING, "fmt");	/* 95/Jan/25 */
				showline(logline, 0);
			}
/*			if (dir_p(buffer)) t = _strdup(buffer); */
			if (dir_p(buffer)) t = xstrdup(buffer);	/* 96/Jan/20 */
			else {
				s = "TEXFMTS";			/* added PC-TeX version 94/Jan/6 */
				if (getenv(s) == NULL)  s = "TEXFMT";	/* em-TeX ... */
			}
			if (traceflag) {
				sprintf(logline, "\nSetting up %s (default %s) ", "TEXFORMATS", t);
				showline(logline, 0);
			}
		}
/*		path_dirs[TEXFORMATPATH] = initialize_path_list ("TEXFORMATS", TEXFORMATS); */
/*		path_dirs[TEXFORMATPATH] = initialize_path_list (s, TEXFORMATS); */
		path_dirs[TEXFORMATPATH] = initialize_path_list (s, t);
/*		if (t != TEXFORMATS) free (t); */
	}

	if (path_bits & TEXPOOLPATHBIT) {
		s = "TEXPOOL";
		t = TEXPOOL;
/*		if (getenv(s) == NULL) { */
		if (grabenv(s) == NULL) {			/* 1994/May/19 */
			s = "TEXFORMATS";				/* try in format directory next */
/*			if (getenv(s) == NULL) { */	/* see if environment var defined */
			if (grabenv(s) == NULL) {		/* see if environment var defined */
				strcpy(buffer, texpath);	/* no, see if texpath\fmt is direct */
				strcat(buffer, PATH_SEP_STRING); 
				strcat(buffer, "fmt");
				if (traceflag) {
					sprintf(logline, "Checking `%s' = %s %s %s\n",
							buffer, texpath, PATH_SEP_STRING, "fmt");	/* 95/Jan/25 */
					showline(logline, 0);
				}
/*				if (dir_p(buffer)) t = _strdup(buffer); */
				if (dir_p(buffer)) t = xstrdup(buffer);		/* 96/Jan/20 */
				else {
					s = "TEXFMTS";			/* added PC-TeX version 94/Jan/6 */
					if (getenv(s) == NULL)  s = "TEXFMT";	/* em-TeX ... */
				}
				if (traceflag) {
					sprintf(logline, "\nSetting up %s (default %s) ", "TEXPOOL", t);
					showline(logline, 0);
				}
			}
		}
/*		path_dirs[TEXPOOLPATH] = initialize_path_list ("TEXPOOL", TEXPOOL); */
/*		path_dirs[TEXPOOLPATH] = initialize_path_list (s, TEXPOOL); */
		path_dirs[TEXPOOLPATH] = initialize_path_list (s, t);
/*		if (t != TEXPOOL) free (t); */
	}

	if (path_bits & TFMFILEPATHBIT) {
		s = "TEXFONTS";
/*		Introduce encoding specific TEXFONTS env variable 97/April/2 */
		if ((u = grabenv("ENCODING")) != NULL) {	/* get ENCODING=... */
			encodingname = u;				/* remember for error mess */
/*			sprintf(logline, "\nENCODING=%s\n", u); */
/*			ENCODING is defined, now see if matching env variable */
			if ((t = grabenv(u)) != NULL) {	/* get TEXNANSI=c:\yandy\tfm; ... */
/*				sprintf(loglein, "\nset %s=%s\n", u, t); */
/*				prevent problems with TEXNANSI=1 and such mistakes! */
/*				should have a drive letter and not be a number */
				if (strchr(t, ':') != NULL &&
					  sscanf(t, "%d", &n) == 0) {
					s = u;				/* look here instead of TEXFONTS=... */
/*				  sprintf(logline, "\nUSE %s\n", u); */
				}
			}
		}

		t = TEXFONTS;					/* #define TEXFONTS TEXPATH "tfm" */
/*		if (getenv(s) == NULL) { */
		if (grabenv(s) == NULL) {		/* 1994/May/19 */
			strcpy(buffer, texpath);	/* see if texpath\tfm is directory */
			strcat(buffer, PATH_SEP_STRING); 
			strcat(buffer, "tfm");
			if (traceflag) {
				sprintf(logline, "Checking `%s' = %s %s %s\n",
						buffer, texpath, PATH_SEP_STRING, "tfm");	/* 95/Jan/25 */
				showline(logline, 0);
			}
/*		  if (dir_p(buffer)) t = _strdup(buffer); */
			if (dir_p(buffer)) t = xstrdup(buffer);			/* 96/Jan/20 */
			else {
				s = "TEXTFMS";			/* added PC-TeX version 94/Jan/6 */
				if (getenv(s) == NULL) s = "TEXTFM"; /* em-TeX uses TEXTFM ... */
			}
			if (traceflag) {
				sprintf(logline, "\nSetting up %s (default %s) ", "TEXFONTS", t);
				showline(logline, 0);
			}
		}
/*		path_dirs[TFMFILEPATH] = initialize_path_list ("TEXFONTS", TEXFONTS); */
/*		path_dirs[TFMFILEPATH] = initialize_path_list (s, TEXFONTS); */
		path_dirs[TFMFILEPATH] = initialize_path_list (s, t);
/*		if (t != TEXFONTS) free (t); */
	}

	if (path_bits & TEXINPUTPATHBIT) {
		if (formatspecific) {								/* 1994/Oct/25 */
			s = formatname;								/* try specific */
			if (grabenv(s) == NULL) s = "TEXINPUTS";		/* no format specific */
		}
		else s = "TEXINPUTS";								/* normal case */
/*		if (getenv(s) == NULL) */
		if (grabenv(s) == NULL) {							/* 1994/May/19 */
			s = "TEXINPUT"; /* added PC-TeX vers 94/Jan/6 */
			if (traceflag) {
				sprintf(logline, "\nSetting up %s ", "TEXINPUTS");
				showline(logline, 0);
			}
		}
/*		path_dirs[TEXINPUTPATH] = initialize_path_list ("TEXINPUTS", TEXINPUTS); */
		path_dirs[TEXINPUTPATH]  = initialize_path_list (s, TEXINPUTS);
	}
}

#ifdef CACHEFILENAME
char last_filename[PATH_MAX]="";	/* last full path / file name found C */
char last_name[PATH_MAX]="";		/* last file name searched for C */
int last_path_index=-1;				/* last path_index */
#endif

/* Look for NAME, a C string (no longer Pascal), in the colon-separated list 
   of directories given by `path_dirs[PATH_INDEX]'.  If the search is
   successful, leave the full pathname in NAME (which therefore must
   have enough room for such a pathname), padded with blanks.
   Otherwise, or if NAME is an absolute or relative pathname, just leave
   it alone.  */

/*	changed to take C string 97/June/5 - used to take Pascal strings */
/*  now expects null terminated strings */

booleane testreadaccess (unsigned char *name, int path_index) { 
#ifdef BUILDNAMEDIRECT
	char buffer[PATH_MAX];			/* for constructing name 1996/Jan/20 */
	int foundflag;					/* true if path found */
#else
	string foundname;
#endif  

	if (opentraceflag) {
		sprintf(logline, "Test read access for `%s' ", name); 	/* C */
		showline(logline, 0);
	}

	if (*name == '\0') return FALSE;	/* sanity check */

#ifdef CACHEFILENAME
/*	If file name and path_index matches - and saved filename exists */
/*	then use cached full path / file name 96/Nov/16 */
	if (cachefileflag) {
		if (path_index == last_path_index &&
			  strcmp(name, last_name) == 0 && *last_filename != '\0') { 
			if (opentraceflag) {
				sprintf(logline, "\nFOUND `%s' (%d) IN CACHE: `%s' ",
						name, path_index, last_filename); 
/*					  name+1, path_index, last_filename); */
				showline(logline, 0);
			}
			strcpy(name, last_filename); 
			return TRUE;
		}
		last_path_index = path_index;
		strcpy(last_name, name);
		*last_filename = '\0';					/* in case not found */
	}
#endif

/* Look for it.  */ /* only call to find_path_filename in pathsrch.c */
#ifdef BUILDNAMEDIRECT
	foundflag = xfind_path_filename (buffer, name, path_dirs[path_index]);
#else
/*	this returns either a newly allocated string or name */
/*	will need to free it later again ... */
	foundname = find_path_filename (name, path_dirs[path_index]);
#endif
/*	If we didn't find it, and we're looking for a font, maybe it's
    an alias defined in a mapping file.  */	
/*  if (!foundname && path_index == TFMFILEPATH) */
#ifdef BUILDNAMEDIRECT
	if (foundflag == 0 && path_index == TFMFILEPATH)
#else
		if (foundname == NULL && path_index == TFMFILEPATH)
#endif
		{
			char *mapped_name;
			static map_type fontmap = NULL;		/* GLOBAL, so won't recreate */

/*			fault in the mapping if necessary.  */
			if (fontmap == NULL) {
				if (traceflag) {
					sprintf(logline, "Loading in texfonts.map file for %s\n", name);
					showline(logline, 0);
				}
				fontmap = map_create (path_dirs[path_index]);
			}

/*			Now look for our filename in the mapping.  */
			mapped_name = map_lookup (fontmap, name);
			if (mapped_name) {
/*			Found a possibility.  Look for the new name.  */
#ifdef BUILDNAMEDIRECT
				foundflag = xfind_path_filename (buffer, mapped_name, path_dirs[path_index]);
#else
				foundname = find_path_filename (mapped_name, path_dirs[path_index]);
#endif
/*	NOTE: mapped_name is NOT an allocated string to be freed ... */
			}
		}

	if (opentraceflag) {
		showline("\n", 0);	/* improve trace format out 94/Jan/8 */
	}

	if (opentraceflag) {
#ifdef BUILDNAMEDIRECT
		if (foundflag != 0) {
			sprintf(logline, "`%s' in test_read_access\n", buffer);
			showline(logline, 0);
		}
#else
		if (foundname != NULL) {
			sprintf(logline, "`%s' in test_read_access\n", foundname);
			showline(logline, 0);
		}
#endif
	}

/*	If we found it somewhere, save it.  */
#ifdef BUILDNAMEDIRECT
	if (foundflag != 0) {
		strcpy (name, buffer); 
#ifdef CACHEFILENAME
		if (cachefileflag) {
			strcpy(last_filename, buffer);	/* full path */
		}
#endif
	}
#else
	if (foundname != NULL) {
		strcpy (name, foundname);
#ifdef CACHEFILENAME
		if (cachefileflag) {
			strcpy(last_filename, foundname);	/* full path */
			last_namelength = strlen(buffer);
		}
#endif
	}
#endif

#ifdef BUILDNAMEDIRECT
	return foundflag;
#else
	if (foundname == NULL) return FALSE;
	else {
		if (foundname != name) free (foundname);  /* copied, now free ??? 96/Jan/10 */
		return TRUE;
	}
#endif
}

/********************************************************************/

#define STREQ(s1, s2) (strcmp (s1, s2) == 0)

/* from fontmap.c */

/* Fontname mapping.  We use a straightforward hash table.  */

#define MAP_SIZE 199

/* The hash function.  We go for simplicity here.  */

static unsigned map_hash (char *key) {
	unsigned n = 0;

/*	There are very few font names which are anagrams of each other 
    so no point in weighting the characters.  */
	while (*key != 0) n += *key++;
	n %= MAP_SIZE;
	return n;
}

/* Look up STR in MAP.  Return the corresponding `value' or NULL.  */

static char *map_lookup_str (map_type map, char *key) {
	map_element_type *p;
	unsigned n = map_hash (key);

	for (p = map[n]; p != NULL; p = p->next)
		if (STREQ (key, p->key)) return p->value;

	return NULL;
}


/* Look up KEY in MAP; if it's not found, remove any suffix from KEY and
   try again.  */

char *map_lookup (map_type map, char *key) {
	string suffix = find_suffix (key);
	string ret = map_lookup_str (map, key);

	if (! ret) {
/*		OK, the original KEY didn't work.  Let's check for the KEY without
		an extension -- perhaps they gave foobar.tfm, but the mapping only
		defines `foobar'.  */
		if (suffix)	{
			string base_key = remove_suffix (key);
			ret = map_lookup_str (map, base_key);
			free (base_key);	// it's safe to free copy
		}
	}

/*	Append the same suffix we took off, if necessary.  */
/*	what if suffix is NULL ??? */ /* what if we didn't take off suffix ??? */
/*  if (ret) */
	if (ret && suffix) {						/* 1994/March/18 */
		ret = extend_filename (ret, suffix);
//		the above creates a newly allocated string ... should free old ?
//		except extend_filename may return the old one ?
	}
	return ret;
}

/* If KEY is not already in MAP, insert it and VALUE.  */
/* This was very buggy (when hash codes collided) - rewritten 94/March/18 */

void map_insert (map_type map, char *key, char *value) {
	unsigned n = map_hash (key);
	map_element_type **ptr = &map[n];
/*  map_element_type ***trailer = &p; */

	while (*ptr != NULL && ! STREQ (key, (*ptr)->key)) {
/*       *p = (*p)->next; */
		ptr = &((*ptr)->next);
/*       trailer = &p; */
	}

	if (*ptr == NULL)    {
/*      **trailer = XTALLOC (MAP_SIZE, map_element_type); *//* 94/March/19 */
		*ptr = (map_element_type *) xmalloc (sizeof(map_element_type));
/*      (**trailer)->key = xstrdup (key); */
		(*ptr)->key = xstrdup (key);
/*      (**trailer)->value = xstrdup (value); */
		(*ptr)->value = xstrdup (value);
/*      (**trailer)->next = NULL; */
		(*ptr)->next = NULL;
	}
}

/* Open and read the mapping file FILENAME, putting its entries into
   MAP. Comments begin with % and continue to the end of the line.  Each
   line of the file defines an entry: the first word is the real
   filename (e.g., `ptmr'), the second word is the alias (e.g.,
   `Times-Roman'), and any subsequent words are ignored.  .tfm is added
   if either the filename or the alias have no extension.  This is the
   same order as in Dvips' psfonts.map; unfortunately, we can't have TeX
   read that same file, since most of the real filenames start with an
   `r', because of the virtual fonts Dvips uses.  */

/* Modified 97/May/17 to avoid malloc for each line read */

#ifndef MALLOCLINE
#define MAXLINE 256
#endif

int map_file_parse (map_type map, char *map_filename) {
	char *l;
	unsigned map_lineno = 0;
	FILE *f; 
#ifndef MALLOCLINE
	char line[MAXLINE];							/* 97/May/17 */
#endif  

	if (traceflag) {
		sprintf(logline, "Opening %s\n",  map_filename);	/* 97/May/17 */
		showline(logline, 0);
	}
//	f = xfopen (map_filename, FOPEN_R_MODE);
	f = fopen (map_filename, FOPEN_R_MODE);
	if (f == NULL) {
		perrormod(map_filename);	// should not happen, since we tested
		return -1;					// failed
	}
#ifdef MALLOCLINE
	while ((l = read_line (f)) != NULL) 
#else
		while ((l = read_a_line (f, line, sizeof(line))) != NULL)	/* 97/May/17 */
#endif
		{
			string filename;
			string comment_loc;

/*			comment_loc = strrchr (l, '%'); */
			comment_loc = strchr (l, '%');			/* 96/Nov/16 */
/*			if (comment_loc == NULL) comment_loc = strrchr (l, ';'); */
			if (comment_loc == NULL) comment_loc = strchr (l, ';'); /* fixed */

/*			Ignore anything after a % or ;  */
/*			if (comment_loc) *comment_loc = 0; */
			if (comment_loc != NULL) *comment_loc = '\0';

			map_lineno++;

/*			If we don't have any filename, that's ok, the line is blank.  */
			filename = strtok (l, " \t");
/*			if (filename)  */
			if (filename != NULL) {
				string alias = strtok (NULL, " \t");

/*				But if we have a filename and no alias, something's wrong.  */
				if (alias == NULL || *alias == 0) {
					sprintf(logline,
							" Have file name `%s', but no mapping (line %u in file %s).\n",
							filename, map_lineno, map_filename);
					showline(logline, 1);
				}
				else  {
/*					We've got everything.  Insert the new entry.  */
					map_insert (map, alias, filename);
				}
			}
#ifdef MALLOCLINE
			free (l);
#endif
		}
//	xfclose (f, map_filename);
	(void) fclose (f);		// we don't care about errors at this stage
	return 0;				// success
}

void unshroudstring (char *, char *, int);	/* in texmf.c */

/* Look for the file `texfonts.map' in each of the directories in
   DIR_LIST.  Entries in earlier files override later files.  */

/* This is probably quite silly - but what the hell lets leave it in */

map_type map_create (string *dir_list) {
	map_type map = (map_type) xcalloc (MAP_SIZE, sizeof (map_element_type *));

	while (*dir_list) {
		char filename[PATH_MAX];

/*		 We don't bother with the filename truncation that `readable' in
         `pathsrch.c' does, since we ourselves are giving the filename,
         and I don't think it's worth worrying about too-long
         intermediate directory names in the path.  */
		strcpy (filename, *dir_list);
/*      strcat (filename, "texfonts.map"); */		/* 1993/Nov/20 */
		unshroudstring (filename+strlen(filename),
						"ufygpout/nbq", PATH_MAX - strlen(filename));

/*		testing access first so xfopen won't fail... */
/*		maybe do this another way to avoid using `access' ? */
		if (filemethod) {
			if (file_p (filename) != NULL) {		/* use file_p the new way */
				(void)  map_file_parse (map, filename);
			}
		}
		else {	
/*			if (access (filename, R_OK) == 0) */	/* use access the old way */
			if (_access (filename, R_OK) == 0) {	/* 1999/Jan/8 ??? /*
/*			if (readable (filename) != NULL) */
				(void) map_file_parse (map, filename);
			}
		}
		dir_list++;
	}
	return map;
}

/**********************************************************************/

/* #pragma optimize ("g", off) *//* try and avoid compiler bug here _dos_find */

/* NOTE: _dos_find... prevents running under Windows NT ??? */
/* This is called if filemethod != 0 */ /* which is currently the default */

#ifdef MSDOS
/* see whether a file exists, is readable and is not a directory */
/* 1994/Feb/13 may be faster than `access' in `readable' */
/* returns NULL or the name filename passed in ??? */

char *file_p (string fn) {
	struct _finddata_t fi;
	long hFind;
	int ret;

	if (opentraceflag) {
		sprintf(logline, "Is `%s' a readable file? ", fn);
		showline(logline, 0);
	}

/*	allow for `normal' (_A_NORMAL) as well as `read-only' files */

	hFind = _findfirst (fn, &fi);
	if (hFind > 0) {
		ret = 0;
		_findclose (hFind);
	}
	else ret = -1;

/*	check whether found and whether *not* a sub-directory */
	if (ret == 0) {
		if ((fi.attrib & _A_SUBDIR) == 0) {
			if (opentraceflag) {
				sprintf(logline, "`%s' IS a readable file. ", fn);
				showline(logline, 0);
			}
			return fn;		/* true - its a file, not a dir */
		}
		else {
			if (opentraceflag) {
				sprintf(logline, "`%s' is a subdirectory. ", fn);
				showline(logline, 0);
			}
			return NULL;	/* false - directory */
		}
	}
	else {
		if (opentraceflag) {
			sprintf(logline, "`%s' is NOT a readable file. ", fn);
			showline(logline, 0);
		}
		return NULL;	/* false - not found or no read access */
	}
}

#endif /* DOS */

/* #pragma optimize ("g", )	*/	/* try and avoid compiler bug here _dos_find */
/* #pragma optimize ("g", ) */	/* try and avoid compiler bug here _dos_find */
// #pragma optimize ("", on)		/* 96/Sep/15 */


/**************************************************************************/

/* S_IFMT is file type mask 0170000 and S_IFDIR is directory 0040000 */

#pragma optimize ("g", off) 	/* try and avoid compiler bug here _dos_find */

/* NOTE: _dos_find... prevents running under Windows NT ??? */
/* and presently dirmethod = true so we do use this _dos_find_first */

booleane dir_p (string fn) {
	FILE *test;
	char *s;
	struct _finddata_t fi;
	long hFind;
	int ret;
	char tmpfn[FILENAME_MAX];				/* long enough ??? */

	strcpy (tmpfn, fn);						/* make copy so can modify */
	if (opentraceflag) {
		sprintf(logline, "Is `%s' a directory? ", tmpfn);
		showline(logline, 0);
	}

	s = tmpfn + strlen(tmpfn) - 1;
	if (*s == '\\' || *s == '/') *s = '\0';	/* get rid of trailing path sep */

/*	quick test for "." and ".." case - avoid confusion later */
	if (strcmp (tmpfn, ".") == 0 || strcmp(tmpfn, "..") == 0) return 1; 

	if (dirmethod) {			/* use _findfirst *first* if requested */
		hFind = _findfirst(tmpfn, &fi);
		if (hFind > 0) {
			ret = 0;
			_findclose(hFind);
		}
		else ret = -1;

		if (ret == 0) {
/*			_findfirst succeeded --- now test attributes of what was found */
			if (fi.attrib & _A_SUBDIR) {
				if (opentraceflag) {
					sprintf(logline, "Directory `%s' DOES exist ", fn);
					showline(logline, 0);
				}
				return 1;			/* true - it is a sub-directory */
			}
			else {
				if (opentraceflag) {
					sprintf(logline, "`%s' is a FILE, not a DIRECTORY ", fn);
					showline(logline, 0);
				}
				return 0;			/* false - its a file, not a dir */
			}
		}
		else {
/*			_findfirst failed --- possible causes: missing *or* top-level */
/*			crude check first top level directory ? - assume form `c:' */
			if (*(tmpfn+1) != ':' || *(tmpfn+2) != '\0') {
/*				it is *not* top level and _findfirst failed - give up */
				if (opentraceflag) {
					sprintf(logline, "Directory `%s' does NOT exist ", fn);
					showline(logline, 0);
				}
				return 0;			/* false - it is not a directory */
			}
/*			else drop through to old method */
/*			else { */
/*				top-level dir, so revert to the old method after all ... */
/*				return dir_p_1 (fn); */ 
/*				or try _findfirst after appending PATH_SEP and nul ? */
/*			} */ /* drop through */
		}
	}

/* either: dropped through (top-level of driver) or dirmethod is false */
/* use the old method --- fopen of nul in supposed directory */
/* NOTE: nul device exists in all dirs */ /* Possible OS/2 and NDOS problem */
	strcat (tmpfn, PATH_SEP_STRING "nul"); 
/*	if ((test = fopen (tmpfn, "r")) == NULL) */
	if (shareflag == 0) test = fopen (tmpfn, "r");
	else test = _fsopen(tmpfn, "r", shareflag);		/* 1994/July/12 */
	if (test == NULL) {
		if (opentraceflag) {
			sprintf(logline, "Directory `%s' does NOT exist ", tmpfn);
			showline(logline, 0);
		}
		return 0;			/* false */
	}
	else {
		(void) fclose(test);		/* have to remember to close it again */
		if (opentraceflag) {
			sprintf(logline, "Directory `%s' DOES exist ", tmpfn);
			showline(logline, 0);
		}
		return 1;			/* true */
	}
}

/* #pragma optimize ("g", ) */	/* try and avoid compiler bug here _dos_find */
/* #pragma optimize ("g") */	/* try and avoid compiler bug here _dos_find */
#pragma optimize ("", on)		/* 96/Sep/12 */

/* NOTE: calling _stat makes packed EXE file 3,400 bytes larger ! */

/* we don't want _fsopen instead of fopen here because only for dir\nul ??? */

/********************************************************************************/

/* pathsrch.c */

/* If FILENAME is absolute or explicitly relative (i.e., starts with
   `/', `./', or `../'), or if DIR_LIST is null, we return whether
   FILENAME is readable as-is.  Otherwise, we test if FILENAME is in any of
   the directories listed in DIR_LIST.  (The last entry of DIR_LIST must
   be null.)  We return the complete path if found, NULL else.
   
   In the interests of doing minimal work here, we assume that each
   element of DIR_LIST already ends with a `/'.
   
   DIR_LIST is most conveniently made by calling `initialize_path_list'.
   This is a separate routine because we allow recursive searching, and
   it may take some time to discover the list of directories.  
   We do not want to incur that overhead every time we want to look for
   a file.
   
   (Actually, `/' is not hardwired into this routine; we use PATH_SEP,
   defined above.)  */

/* xfind_path_filename is used now */

/* Called only from testreadaccess(...) in ourpaths.c */

#ifdef BUILDNAMEDIRECT

/* this string allocation / concatination is silly - use fixed buffer! */

int xfind_path_filename (string buffer, string filename,  string * dir_list) {
	string found_name = NULL;

	if (buffer == filename) {
		showline("buffer == filename\n", 1);
	}

	*buffer = '\0';				/* "" in case we fail */

	if (opentraceflag) {
		sprintf(logline, "Find path for `%s' ", filename);
		showline(logline, 0);
	}

/*  ignore current directory for TFM files ? */ /* 1994/Jan/24 */
	if (!currenttfm &&  strstr(filename, ".tfm") != NULL &&
		  strcmp(*dir_list, "./") == 0) {
		if (opentraceflag) {
			sprintf(logline, "Ignoring `.' for %s ", filename);
			showline(logline, 0);
		}
		dir_list++;						/* step over first entry in dir list */
	}

	if (traceflag && opentraceflag) {		/* debugging trace 1994/Jan/8 */
		char **pstrs;
		pstrs = dir_list;
		showline("\n", 0);
		sprintf(logline, "Find path for `%s' ", filename);
		showline(logline, 0);
		showline("- IN: ", 0);
		while (*pstrs != NULL) {
			sprintf(logline, "%s ", *pstrs);
			showline(logline, 0);
			pstrs++;
		}
		showline("\n", 0);
	}

/*	Do this before testing for absolute-ness, as a leading ~ will be an
	absolute pathname.  */	/* forget this for DOS ! */
#ifndef MSDOS
	filename = expand_tilde (filename);
#endif

#ifdef MSDOS
/*	is this always safe?  That is, is filename writable and its OK to modify */
/*	unixify( filename );   */ /* done `in place' */
	if (deslash)  unixify( filename );		/* made conditional 94/Feb/24 */
#endif

/*	following addded in attempt to catch `nul' */		/* 94/Jan/6 bkph */
/*	could also try and catch `nul.tex' first - but who cares about speed ? */
/*	needed to add this since `access' gets the wrong answer */
/*	that is, `nul' is a file that can be opened, but `access' says not so */
	if (strcmp(filename, "nul") == 0) strcpy(buffer, filename); 
/*	If FILENAME is absolute or explicitly relative, or if DIR_LIST is
		null, only check if FILENAME is readable.  */
/*	if (absolute_p (filename) || dir_list == NULL) */ /* 94/Jan/6 */
	else if (absolute_p (filename) || dir_list == NULL) {
		if (filemethod)  found_name = file_p (filename);
		else found_name = readable (filename);
		if (found_name != NULL) strcpy(buffer, found_name);
		else *buffer = '\0';
	}
	else { /* Test if FILENAME is in any of the directories in DIR_LIST.  */
		char *s;
		int sourceflag;
		int firsttime=1;
		while (*dir_list != NULL)  {
/* if item is current directory, look in source file directory first */
/* provided usesourcedirectory flag is set and workingdirectory in use */
			s = *dir_list;
			sourceflag = 0;
			if (strcmp(s, "./") == 0) {
				if (firsttime && usesourcedirectory && workingdirectory &&
					  sourcedirect != NULL && *sourcedirect != '\0') {
					s = sourcedirect;
					if (traceflag) {
						sprintf(logline, "Using %s dir %s %s\n", "source", s, "X");
						showline(logline, 0);
					}
					sourceflag = 1;			/* avoid increment of list below */
					firsttime = 0;			/* special stuff only first time */
				}
				else if (traceflag) {
					sprintf(logline, "Using %s dir %s %s\n", "current",  s, "X");
					showline(logline, 0);
				}
			}
			if (traceflag) {
				sprintf(logline, "XCONCAT %s %s in find_path_filename\n",
						s, filename);
				showline(logline, 0);
			}
/*			filename = concat (*dir_list, save_filename); */
			(void) xconcat (buffer, s, filename);
			if (filemethod) found_name = file_p (buffer);	/* new way */
			else found_name = readable (buffer);			/* slow old way */
			if (found_name == NULL) {
				*buffer = '\0';
				if (! sourceflag)		/* 98/Sep/29 repeat in current dir */
					dir_list++;				/* try next */
			}
			else {
				if (found_name != buffer)
					strcpy(buffer, found_name);				/* success */
				break;
			}
		}
	}
	return (*buffer != '\0');			/* true if not empty string */
}

#else

/* We are dealing with a C string here for filename presumably ... */

/* Also, this returns a string that was allocated --- */
/* unless it happens to equal the filename sent in ... */
/* this needs to be freed later - unless it happens to be ... */

string find_path_filename (string filename,  string * dir_list) {
	string found_name = NULL;
  
	if (opentraceflag) {
//		printf("Find path for `%s' ", filename);
		sprintf(logline, "Find path for `%s' ", filename);
		showline(logline, 0);
	}

/*  ignore current directory for TFM files ? */ /* 1994/Jan/24 */
	if (!currenttfm &&
			strstr(filename, ".tfm") != NULL &&
				strcmp(*dir_list, "./") == 0) {
		if (opentraceflag) {
			sprintf(logline, "Ignoring `.' for %s ", filename);
			showline(logline, 0);
		}
		dir_list++;						/* step over first entry in dir list */
	}

	if (traceflag && opentraceflag) {		/* debugging trace 1994/Jan/8 */
		char **pstrs;
		pstrs = dir_list;
		showline("\n", 0);
		sprintf(logline, "Find path for `%s' ", filename);
		showline(logline, 0);
		showline("- IN: ", 0);
		while (*pstrs != NULL) {
//			printf("%s ", *pstrs);
			sprintf(logline, "%s ", *pstrs);
			showline(logline, 0);
			pstrs++;
		}
		showline("\n", 0);
	}

/*	Do this before testing for absolute-ness, as a leading ~ will be an
    absolute pathname.  */	/* forget this for DOS ! */
#ifndef MSDOS
	filename = expand_tilde (filename);
#endif

#ifdef MSDOS
/*	is this always safe?  That is, is filename writable and its OK to modify */
/*	unixify( filename );   */ /* done `in place' */
	if (deslash)  unixify( filename );		/* made conditional 94/Feb/24 */
#endif
/*	following addded in attempt to catch `nul' */		/* 94/Jan/6 bkph */
/*	could also try and catch `nul.tex' first - but who cares about speed ? */
/*	needed to add this since `access' gets the wrong answer */
/*	that is, `nul' is a file that can be opened, but `access' says not so */
	if (strcmp(filename, "nul") == 0) found_name = filename; 
/*	If FILENAME is absolute or explicitly relative, or if DIR_LIST is
		null, only check if FILENAME is readable.  */
/*	if (absolute_p (filename) || dir_list == NULL) */ /* 94/Jan/6 */
	else if (absolute_p (filename) || dir_list == NULL) {
		if (filemethod)  found_name = file_p (filename);	/* new way 94/Feb/13 */
		else found_name = readable (filename);				/* slow old way */
	}
	else { /* Test if FILENAME is in any of the directories in DIR_LIST.  */
		string save_filename = filename;
		char *s;
		int sourceflag;
		int firsttime=1;
		while (*dir_list != NULL)  {
/* if item is current directory, look in source file directory first */
/* provided usesourcedirectory flag is set and workingdirectory in use */
			s = *dir_list;
			sourceflag = 0;
			if (strcmp(s, "./") == 0) {
				if (firsttime && usesourcedirectory && workingdirectory &&
					sourcedirect != NULL && *sourcedirect != '\0') {
					s = sourcedirect;
					if (traceflag) {
						sprintf(logline, "Using %s dir %s %s\n", "source", s, "F");
						showline(logline, 0);
					}
					sourceflag = 1;			/* avoid increment of list below */
					firsttime = 0;			/* special stuff only first time */
				}
				else if (traceflag) {
					sprintf(logline, "Using %s dir %s %s\n", "current", s, "F");
					showline(logline, 0);
				}
			}
			if (traceflag) {
				sprintf(logline, "CONCAT %s %s in find_path_filename\n",
								  s, save_filename); /* 1996/Jan/20 */
				showline(logline, 0);
			}
			filename = concat (s, save_filename);
/*          found_name = readable (filename); */
			if (filemethod) found_name = file_p (filename);	/* new way */
			else found_name = readable (filename);			/* slow old way */
			if (found_name == NULL)  {
				free (filename);		/* if this is not it, free it again */
				if (! sourceflag)		/* 98/Sep/29 repeat in current dir */
					dir_list++;
			}
			else {
				if (found_name != filename)
					free (filename);	/* don't free if is the one passed in */
				break;
			}
		}
	}
	return found_name;				/* return the allocated name - free later */
}
#endif

/* If NAME is readable, return it.  If the error is ENAMETOOLONG,
   truncate any too-long path components and return the result (unless
   there were no too-long components, i.e., a overall too-long name
   caused the error, in which case return NULL).  On any other error,
   return NULL.
   
   POSIX invented this brain-damage of not necessarily truncating
   pathname components; the system's behavior is defined by the value of
   the symbol _POSIX_NO_TRUNC, but you can't change it dynamically!  */

/* Using access (and dir_p)  is considerably slower than using dosfind */
/* NOTE: this is only called from find_path_file,
   and then ONLY if filemethod is false (-G) */

/* returns NULL or the file name passed in ??? */

/* static string readable (name) */
string readable (string name) {
	string ret;

	if (opentraceflag) {
		sprintf(logline, "is %s readable? ", name);
		showline(logline, 0);
	}

/*	Check first whether we have read access, then */
/*	need to test if directory, since access always says OK for directory */
/*	BUT: readable is called only from find_path_file */
/*	So we never call this with directory, so why waste time ? bkph */
/*	BUT: can be caught out by idiot with a directory called myfile.tex ? */
	
	if (_access (name, R_OK) == 0)  {
		if (testdiraccess) {		/* check only if we are asked to ... */
			if (dir_p (name)) {
				if (opentraceflag) {
					sprintf(logline, "tested read access of directory `%s' ", name);
					showline(logline, 0);
				}
				ret = NULL;
			}
			else ret = name;
		}
		else ret = name;
	}
/*  if (_access (name, R_OK) == 0 && !dir_p (name))   ret = name; */
#ifdef ENAMETOOLONG
	else if (errno == ENAMETOOLONG) { 
		ret = truncate_pathname (name);
/* Perhaps some other error will occur with the truncated name, so
         let's call access again.  */
		if (!(_access (ret, R_OK) == 0 && !dir_p (ret))) { /* Failed.  */
			free (ret);
			ret = NULL;
		}
	}
#endif
	else if (errno == EACCES) {
		if (traceflag) showline("Access denied!\n", 0);
		ret = NULL;
	}
	else if (errno == ENOENT) {
		if (traceflag) showline("File or path name not found!\n", 1);
		ret = NULL;
	}
	else {
		if (traceflag) {
			sprintf(logline, "Unknown access error %d!\n", errno);
			showline(logline, 0);
		}
		ret = NULL;
	}
	return ret;
}

#ifdef ENAMETOOLONG
/* Truncate any too-long path components in NAME, returning the result.  */

string truncate_pathname (string name) {
	unsigned c_len = 0;       /* Length of current component.  */
	unsigned ret_len = 0;		/* Length of constructed result.  */
	string ret = (string) xmalloc (PATH_MAX + 1);

	for (; *name; name++)
    {
      if (*name == PATH_SEP)			/* not in DOS */
        { /* At a directory delimiter, reset component length.  */
          c_len = 0;
        }
      else if (c_len > NAME_MAX)
        { /* If past the max for a component, ignore this character.  */
          continue;
        }

      /* If we've already copied the max, give up.  */
      if (ret_len == PATH_MAX)
        {
          free (ret);
          return NULL;
        }

      /* Copy this character.  */
      ret[ret_len++] = *name;
      c_len++;
    }
  ret[ret_len] = 0;

  return ret;
}
#endif	/* end of ifdef ENAMETOOLONG */


/* Return true if FILENAME is absolute or explicitly relative, else false.  */
/* Absolute: in DOS name starts with PATH_SEP, or with DRIVE letter and colon */
/* Explicitly relative: starts with ./ or ../ */

// static booleane absolute_p (string filename) {
booleane absolute_p (string filename) {
	booleane absolute;
	booleane explicit_relative;

#ifdef MSDOS
/*	absolute = (*filename == PATH_SEP) */			/* 1994/Mar/1 */
	absolute = (*filename == PATH_SEP || *filename == '\\')
                      || ((filename[1] == ':') && ISALPHA (*filename));
/*                      || ISALPHA (*filename) && filename[1] == ':'; */
#else
	absolute = (*filename == PATH_SEP);
#endif
	if (absolute) return true;			/* don't waste any more time */

#ifdef MSDOS
	explicit_relative = (*filename == '.'
		&& ((filename[1] == PATH_SEP || filename[1] == '\\')
           || (filename[1] == '.' &&
			   (filename[2] == PATH_SEP || filename[2] == '\\'))));
#else
	explicit_relative = (*filename == '.'  && (filename[1] == PATH_SEP
           || (filename[1] == '.' && filename[2] == PATH_SEP))); 
#endif

	return explicit_relative;
/*  return absolute || explicit_relative; */ /* slight rewrite 1994/Feb/13 */
}

#ifdef MSDOS
/*	note: this strips off trailing white space in actual environment var ... */
void striptrailing (string env_value, string env_name, string default_path) {
	char *s;
	if (env_name == NULL) {					/* 1994/Feb/24 */
		if (traceflag) {
			sprintf(logline, "WARNING: no env_name noted, using default %s\n",
				default_path);
			showline(logline, 0);
		}
		return;
	}
	if (env_value == NULL) {
		if (traceflag) {
			sprintf(logline, "WARNING: %s not defined in environment, using default %s\n",
				env_name, default_path);
			showline(logline, 0);
		}
		return;
	}
	if (strlen(env_value) == 0) return;
	s = env_value + strlen(env_value) - 1;
/*	while (*s <= ' ') *s-- = '\0'; */
	while (s >= env_value && *s <= ' ' ) *s-- = '\0';		/* 94/Feb/24 */
}
#endif

/* convert /! and /!! to / and // 97/Mar/22 */

#ifdef MSDOS
void convertexclam (string env_value) {	/* 97/Mar/22 */
	char *s;
	if (env_value == NULL) return;
	s = env_value;
	if (strchr(s, '!') == NULL) return;
	while ((s = strchr(s, '!')) != NULL) {
		if (*(s+1) == '!') {	/* double !! */
			if (*(s+2) == PATH_DELIMITER || *(s+2) == '\0') {
				if (s > env_value && *(s-1) == PATH_SEP) {
					*s = PATH_SEP;		/* convert the first ! */
					strcpy(s+1, s+2);	/* flush the second ! */
				}
			}
		}
		else {		/* single ! */	/* assume already unixified */
			if (*(s+1) == PATH_DELIMITER || *(s+1) == '\0') {
				if (s > env_value && *(s-1) == PATH_SEP)
					strcpy(s, s+1);	/* just flush the ! */
			}
		}
		s++;
	}
	if (traceflag) {
		sprintf(logline,"Now is %s\n", env_value);
		showline(logline, 0);
	}
}
#endif

/* Return a NULL-terminated array of directory names, each name ending
   with PATH_SEP, created by parsing the PATH_DELIMITER-separated list
   in the value of the environment variable ENV_NAME, or DEFAULT_PATH if
   the env var is not set.
   
   A leading or trailing PATH_DELIMITER in the value of ENV_NAME is replaced
   by DEFAULT_PATH.
   
   Any element of the path that ends with double PATH_SEP characters
   (e.g., `foo//') is replaced by all its subdirectories.

   If ENV_NAME is null, only parse DEFAULT_PATH.  If both are null, do
   nothing and return NULL.  */

string *initialize_path_list (string env_name,  string default_path) {
	string dir, path;
	string *dir_list;
	unsigned dir_count = 0;
	string env_value;
	string orig_path;
	struct _finddata_t findt;
/*  _finddata_t structure *can* be reused, unlike _find_t 95/Jan/31 */
/*  so save on stack space, by having one copy here, not one per expand_subdir*/

/*  env_value = env_name ?  getenv (env_name)  : NULL; */
	env_value = env_name ?  grabenv (env_name)  : NULL; /* 1994/May/19 */

/*  need to convert \ to / as soon as possible to avoid confusion */
/*	we may be modifying DOS environment variable here ... is it always safe ? */
#ifdef MSDOS
	if (deslash) unixify (env_value);		    /* 1994/Feb/24 */
#endif
	if (traceflag) {
		if (env_name) {			/* only if env_name is non-null 94/Feb/24 */
			sprintf(logline, "\nSet %s=", env_name);
			showline(logline, 0);
			if (env_value) {	/* only if env_name value is set */
				showline(env_value, 0);
			}
			showline("\n", 0);
		}
	}
#ifdef MSDOS
/*	strip off trailing white space which would confuse things - bkph  */
	striptrailing (env_value, env_name, default_path);
	convertexclam (env_value);							/* 97/Mar/22 */
#endif
	orig_path = expand_default (env_value, default_path);

	if (orig_path == NULL || *orig_path == 0)  return NULL;

/*	need to convert \ to / as soon as possible to avoid confusion */
#ifdef MSDOS
	if (deslash)  unixify (orig_path);	/* redundant ? */
#endif

/*	If we've already seen this PATH_DELIMITER-separated list, then just get
		it back instead of going back to the filesystem.  */
	dir_list = find_dir_list (orig_path);
	if (dir_list != NULL) return dir_list;
  
/*	Be sure `path' is in writable memory.  */ /* if not, copy it */
	path = (orig_path == env_value || orig_path == default_path
          ? xstrdup (orig_path) : orig_path); 

/*	Find each element in the path in turn.  */
#ifdef MSDOS
/*  if (!switchflag) */	
	if (currentflag) {		/* suppress adding current directory - debugging */
		if (traceflag) {
			sprintf(logline, "Adding directory `%s'\n", "."); /* 95/Jan/24 */
			showline(logline, 0);
		}
		add_directory( &dir_list, &dir_count, ".");
	}
#endif
	for (dir = strtok (path, PATH_DELIMITER_STRING); dir != NULL;
		dir = strtok (NULL, PATH_DELIMITER_STRING)) {
		int len;

// #ifdef MYDEBUG
	if (traceflag) {
		sprintf(logline, "dir %s\n", dir);
		showline(logline, 0);
	}
// #endif
      /* If the path starts with ~ or ~user, expand it.  Do this
         before calling `expand_subdir' or `add_directory', so that
         1) we don't expand the same ~ for every subdirectory; and 
         2) pathnames in `expand_subdir' don't have a `~' in them
            (since the system won't grok `~/foo' as a directory).  */
#ifndef MSDOS
		dir = expand_tilde (dir);
#endif
		len = strlen (dir);

      /* If `dir' is the empty string, ignore it.  */
		if (len == 0)  continue;

      /* If `dir' ends in double PATH_SEP, do subdirectories (and remove
         the second PATH_SEP, so the final pathnames we return don't look
         like foo//bar).  Because we obviously want to do subdirectories
         of `dir', we don't check if it is a leaf.  This means that if
         `dir' is `foo//', and `foo' contains only symlinks (so our leaf
         test below would be true), the symlinks are chased.  */

/* modified to treat single PATH_SEP as expand subdirs without recursion */
/* modified to treat double PATH_SEP as expand subdirs *with*  recursion */

#ifdef MSDOS
		if (len > 2 &&								/* 1994/Mar/1 */
			(dir[len - 1] == PATH_SEP || dir[len - 1] == '\\')
				&& (dir[len - 2] == PATH_SEP || dir[len - 2] == '\\'))
#else
		if (len > 2 && dir[len - 1] == PATH_SEP && dir[len - 2] == PATH_SEP) 
#endif
		{
			if (opentraceflag) {
				sprintf(logline, "Double backslash on `%s' ", dir);	/* bkph */
				showline(logline, 0);
			}

			dir[len - 1] = 0;
			if (dir_p (dir)) {
				if (traceflag) {
					sprintf(logline, "Adding directory `%s'\n", dir);
					showline(logline, 0);
				}
				add_directory (&dir_list, &dir_count, dir);
/* local variable 'findt' used without having been initialized ? &findt ? */
				expand_subdir (&dir_list, &dir_count, dir,
				    findt, 1); 	/* 95/Jan/31 */
			}
		}
/* following is new to find only directories to one level 1994/Jan/24 */
#ifdef MSDOS
		else if (len > 1 &&					/* 1994/Mar/1 */
			(dir[len - 1] == PATH_SEP || dir[len - 1] == '\\'))
#else
		else if (len > 1 && dir[len - 1] == PATH_SEP) 
#endif
        {
			if (opentraceflag) {
				sprintf(logline, "Single backslash on `%s' ", dir);	/* bkph */
				showline(logline, 0);
			}

/*			dir[len - 1] = 0; */
			if (dir_p (dir)) {
				if (traceflag) {
					sprintf(logline, "Adding directory `%s'\n", dir);
					showline(logline, 0);
				}
				add_directory (&dir_list, &dir_count, dir);
				expand_subdir (&dir_list, &dir_count, dir,
				    findt, 0); /* 95/Jan/31 */
			}
		}
      else { /* Don't bother to add the directory if it doesn't exist.  */
		  if (dir_p (dir)) {
			  if (traceflag) {
				  sprintf(logline, "Adding directory `%s'\n", dir);
				  showline(logline, 0);
			  }
			  add_directory (&dir_list, &dir_count, dir);
		  }
	  }
	}
  
// #ifdef MYDEBUG
	if (traceflag) {
		showline("Adding terminating null\n", 0);
	}
// #endif

/*	Add the terminating null entry to `dir_list'.  */
	dir_count++;
	XRETALLOC (dir_list, dir_count, string);
	dir_list[dir_count - 1] = NULL;
  
/*	Save the directory list we just found.  */
	save_dir_list (orig_path, dir_list);

	return dir_list;
}

/* Subroutines for `initialize_path_list'.  */

/* Add a newly-allocated copy of DIR to the end of the array pointed to
   by DIR_LIST_PTR. Increment DIR_COUNT_PTR.  Append a `/' to DIR if
   necessary.  We assume DIR is a directory, to avoid an unnecessary
   call to `stat'.  */

void add_directory (string **dir_list_ptr, unsigned *dir_count_ptr, string dir) {
	if (dir == NULL) return;				/* paranoia 1995/Jan/24 */
  /* If `dir' does not end with a `/', add it.  We can't just
     write it in place, since that would overwrite the null that
     strtok may have put in.  So we ALWAYS make a copy of DIR.  */
#ifdef MSDOS
	dir = (dir[strlen (dir) - 1] == PATH_SEP ||		/* 1994/Mar/1 */
			dir[strlen (dir) - 1] == '\\') ?
				xstrdup (dir) : concat (dir, PATH_SEP_STRING);
#else
	dir = (dir[strlen (dir) - 1] == PATH_SEP ? xstrdup (dir)
         : concat (dir, PATH_SEP_STRING)); 
#endif
#ifdef MSDOS
	if (deslash) unixify (dir);			/* redundant ? bkph */ 
#endif

// #ifdef MYDEBUG
	if (traceflag) {
		sprintf(logline, "Adding directory `%s'\n", dir);
		showline(logline, 0);
	}
// #else
//     if (opentraceflag) {
// 		sprintf(logline, "Adding directory `%s' ", dir);
// 		showline(logline, 0);
// 	}
// #endif

  /* Add `dir' to the list of the directories.  */
  (*dir_count_ptr)++;
  XRETALLOC (*dir_list_ptr, *dir_count_ptr, string);
  (*dir_list_ptr)[*dir_count_ptr - 1] = dir;
}

void lowercase (char *s) {							/* 1994/Jan/25 */
	while (*s) *s++ = (char) tolower(*s);
}


/* These routines, while not strictly needed to be exported, are
   plausibly useful to be called by outsiders.  */

/* Replace a leading or trailing PATH_DELIMITER in ENV_PATH with
   DEFAULT_PATH.  If neither is present, return ENV_PATH if that is 
   non-null, else DEFAULT_PATH.  */

string  expand_default (string env_path, string default_path) {
	string expansion;
  
	if (env_path == NULL) expansion = default_path;
	else if (*env_path == PATH_DELIMITER)
		expansion = concat (default_path, env_path);
	else if (env_path[strlen (env_path) - 1] == PATH_DELIMITER)
		expansion = concat (env_path, default_path);
	else expansion = env_path;
  
	if (traceflag) {								/* 1994/Jan/8 */
		if (env_path == NULL) {
			sprintf(logline, "Using the default %s\n", expansion);
			showline(logline, 0);
		}
		else if (expansion == env_path) {
			sprintf(logline, "Using %s (default was %s)\n", expansion, default_path);
			showline(logline, 0);
		}
		else {								/* expansion != env_path */
			sprintf(logline, "Expanded %s (default was %s) to %s\n",
				env_path, default_path, expansion);
			showline(logline, 0);
		}
	}
	return expansion;
}


#ifndef MSDOS
/* Expand a leading ~ or ~user, Unix-style, unless we are some weirdo
   operating system.  */

string expand_tilde (string name) {
#if defined (MSDOS) || defined (VMS) || defined (VMCMS)
  return name;
#else
  string expansion;
  string home;
  
  /* If no leading tilde, do nothing.  */
  if (*name != '~')
    expansion = name;
  
  /* If `~' or `~/', use $HOME if it exists, or `.' if it doesn't.  */
  else if (name[1] == PATH_SEP || name[1] == 0)			/* not in DOS */
    {
      home = getenv ("HOME");							/* not in DOS */
      if (home == NULL)
        home = ".";
        
      expansion
        = name[1] == 0 ? home : concat3 (home, PATH_SEP_STRING, name + 2);
    }
  
  /* If `~user' or `~user/', look up user in the passwd database.  */
  else
    {
      struct passwd *p;
      string user;
      unsigned c = 2;
      while (name[c] != PATH_SEP && name[c] != 0)	/* not in DOS */
        c++;
      
      user = (string) xmalloc (c);
      strncpy (user, name + 1, c - 1);
      user[c - 1] = 0;
      
      /* We only need the cast here for those (old deficient) systems
         which do not declare `getpwnam' in <pwd.h>.  */
      p = (struct passwd *) getpwnam (user);
      free (user);
      /* If no such user, just use `.'.  */
      home = p == NULL ? "." : p->pw_dir;
      
      expansion = name[c] == 0 ? home : concat (home, name + c);
    }
  
  return expansion;
#endif /* not (DOS or VMS or VM/CMS) */
}
#endif

// structure used for manipulation dir lists

typedef struct {
	string path;
	string *dir_list;
} saved_path_entry;

/* Routines to save and retrieve a directory list keyed by the original
   PATH_DELIMITER-separated path.  This is useful because 1) it can take a
   significant amount of time to discover all the subdirectories of a
   given directory, and 2) many paths all have the same basic default,
   and thus would recompute the directory list.  */

static saved_path_entry *saved_paths = NULL;
static unsigned saved_paths_length = 0;

/* We implement the data structure as a simple linear list, since it's
   unlikely to ever be more than a dozen or so elements long.  We don't
   bother to check here if PATH has already been saved; we always add it
   to our list.  */

void save_dir_list (string path,  string *dir_list) {
//	saved_paths_length++;
	XRETALLOC (saved_paths, saved_paths_length+1, saved_path_entry);
	saved_paths[saved_paths_length].path = path;
	saved_paths[saved_paths_length].dir_list = dir_list;
	saved_paths_length++;
}

/* When we retrieve, just check the list in order.  */

string *find_dir_list (string path) {
	unsigned p;
  
// #ifdef MYDEBUG
	if (traceflag) {
		sprintf(logline, "Find Dir List for path: %s\n", path);
		showline(logline, 0);
	}
// #endif

	for (p = 0; p < saved_paths_length; p++) {
		if (strcmp (saved_paths[p].path, path) == 0)
			return saved_paths[p].dir_list;
	}
	return NULL;
}

/* Unixify filename and path (turn \ into /) --- assumes null terminated */

char *unixify (char * t) {
	char * s = t;
	if (s == NULL) return s;		/* paranoia -- 1993/Apr/10 */
#ifdef MSDOS
	if (t != '\0') {
/*		while (*s) { */
/*		while (*s != 0 && *s != ' ') { */	/* paranoia -- 1994/Mar/22 */
		while (*s != '\0') {				/* paranoia -- 1997/Oct/23 */
/*			if (*s == '\\') *s = '/'; */
			if (*s == '\\') *s = PATH_SEP;
			s++;
		}				/* endwhile */
	}
// #ifdef MYDEBUG
	if (traceflag)	{
		sprintf(logline, "Unixified name: %s\n", t);
		showline(logline, 0);
	}
// #endif
#endif /* DOS */
	return t;
}

/****************************************************************************/

/* moved here to avoid problems with pragma */

/* struct _finddata_t findt; */	/* make global, can be reused unlike _find_t */
								/* avoids heavy stack usage in tree search */
								/* but ties up some global fixed space ... */

#pragma optimize ("g", off) 	/* try and avoid compiler bug here _dos_find */

/* Add DIRNAME to DIR_LIST and look for subdirectories, possibly recursively.
   We assume DIRNAME is the name of a directory.  */

/* NOTE: _dos_find... prevents running under Windows NT as console app ??? */
/* Yes, so lets flush it! use _findfirst, _findnext, _findclose instead */

/* called only from initialize_path_list  (and recursively) */

void expand_subdir (string **dir_list_ptr, unsigned *dir_count_ptr, string dirname,
					struct _finddata_t findt, integer recurseflag) {
#ifdef MSDOS
/*	struct _finddata_t findt; */
	long hFind;
	int ret;
	int len;
/*  char buffer[PATH_MAX]; */   /* pretty long? potential recursion problem? */
	char buffer[FILENAME_MAX];	/* this is DOS and Windows NT after all ... */
	char *potential;
#endif	/* DOS */

	if (traceflag) {
		sprintf(logline, "\nExpanding sub dir %s ", dirname);
		showline(logline, 0);
	}

#ifdef MSDOS
	strcpy(buffer, dirname);
	len = strlen(dirname);

#ifdef MSDOS
/*  if (buffer[len-1] == PATH_SEP )  strcat( buffer, "*.*" ); */
	if (buffer[len-1] == PATH_SEP || buffer[len-1] == '\\' )
		strcat( buffer, "*.*" );						/* 1994/Mar/1 */
	else strcat( buffer, PATH_SEP_STRING "*.*" );
#else
	if ( buffer[len-1] == PATH_SEP )  strcat( buffer, "*");
	else strcat( buffer, PATH_SEP_STRING "*");
#endif	/* MSDOS */

/*	Note: the _A_SUBDIR means we get ordinary files PLUS sub-directories */
	if (opentraceflag)  {
		sprintf(logline, "\nDIRNAME `%s' ", dirname);
		showline(logline, 0);
	}
/*	we'll need to step over `.' and `..' up front of directory list */
	hFind = _findfirst(buffer, &findt);
	if (hFind > 0) ret = 0;
	else ret = -1;
/*	_dos_findnext(  &findt  ); */
/* 	while( _dos_findnext( &findt ) == 0)  { */
	while (ret == 0)  {
/*		if (opentraceflag) */
		if (opentraceflag && traceflag) {
			sprintf(logline, "NEXT `%s' (%0x) ", findt.name, findt.attrib);
			showline(logline, 0);
		}
/*		if (strchr(findt.name, '.') != NULL) continue; *//* not needed */
		if (findt.name[0] != '.' &&		/* ignore "." and ".." */
			findt.attrib & _A_SUBDIR ) {	/* only look at SUBDIRs */
			if (opentraceflag)  {
				sprintf(logline, "\nDIRNAME `%s' ", dirname);
				showline(logline, 0);
			}
#ifdef MSDOS
			potential = concat3( dirname,
				(dirname[len-1] == PATH_SEP || dirname[len-1] == '\\')
					? "" : PATH_SEP_STRING, findt.name );
#else
			potential = concat3( dirname, dirname[len-1] == PATH_SEP 
				? "" : PATH_SEP_STRING, findt.name );
#endif	/* DOS */
			lowercase (potential);					/* make look nicer ? */
			if (opentraceflag) {
				sprintf(logline, "POTENTIAL `%s' ", potential);
				showline(logline, 0);
			}
			if (traceflag) {
				sprintf(logline, "Adding directory `%s'\n", potential); /* 95/Jan/24 */
				showline(logline, 0);
			}
			add_directory(dir_list_ptr, dir_count_ptr, potential);
			if (recurseflag) 
				expand_subdir(dir_list_ptr, dir_count_ptr,
				    potential, findt, 1);  /* 95/Jan/31 */
			free( potential );
		}			/* end of findt.attrib & _A_SUBDIR != 0 */
		ret = _findnext(hFind, &findt);
	}
#ifdef MSDOS
	if (hFind > 0) _findclose (hFind);
#endif

#ifndef MSDOS
	_dos_findclose( &findt );
#endif

#else  /* end of MSDOS (way up there) */

/*	This is how we do this if we are NOT using DOS */
  DIR *dir;
  struct dirent *e;
  unsigned length;
  char potential[PATH_MAX];
  struct _stat st;
  
   /* We will be looking at its contents.  */
  dir = opendir (dirname);
  if (dir == NULL)
    return;
  
  /* Compute the length of DIRNAME, since it's loop-invariant.  */
  length = strlen (dirname);

  /* Construct the part of the pathname that doesn't change.  */
  strcpy (potential, dirname);
  if (potential[length - 1] != PATH_SEP)	/* not in DOS */
    {
      potential[length] = PATH_SEP;
      potential[length + 1] = 0;
      length++;
    }
  
/* about to use _stat --- shouldn't get here when using MSDOS anyway */

  while ((e = readdir (dir)) != NULL)
    { /* If it's . or .., never mind.  */
      if (!(e->d_name[0] == '.'
            && (e->d_name[1] == 0
                || (e->d_name[1] == '.' && e->d_name[2] == 0))))
        { /* If it's not a directory, we will skip it on the
             recursive call.  */
          strcat (potential, e->d_name);

          /* If we can't _stat it, or if it isn't a directory, continue.  */
          if (_stat (potential, &st) == 0 && S_ISDIR (st.st_mode))
            { /* It's a subdirectory; add `potential' to the list.  */
				if (traceflag) {
					sprintf(logline, "Adding directory `%s'\n", potential); /* 95/Jan/24 */
					showline(logline, 0);
				}
              add_directory (dir_list_ptr, dir_count_ptr, potential);

              /* If it's not a leaf, quit.  Assume that leaf
                 directories have two links (one for . and one for ..).
                 This means that symbolic links to directories do not affect
                 the leaf-ness.  This is arguably wrong, but the only
                 alternative I know of is to _stat every entry in the
                 directory, and that is unacceptably slow.  */
              if (st.st_nlink > 2)
                { /* All criteria are met; find subdirectories.  */
                  expand_subdir (dir_list_ptr, dir_count_ptr, potential,
					  findt, 1);	/* 95/Jan/31 */
                }
            }

          /* ``Remove'' the directory entry name.  */
          potential[length] = 0;
        }
    }
  
	closedir (dir);
#endif	/* end of *not* DOS case */
}

// #pragma optimize ("", on)		/* 96/Sep/12 */

/************************************************************************/

#define ARGSEP '='

/* This version of `getopt' appears to the caller like standard Unix `getopt'
   but it behaves differently for the user, since it allows the user
   to intersperse the options with the other arguments.

   As `getopt' works, it permutes the elements of ARGV so that,
   when it is done, all the options precede everything else.  Thus
   all application programs are extended to handle flexible argument
   order. */

/* For communication from `getopt' to the caller.
   When `getopt' finds an option that takes an argument,
   the argument value is returned here.
   Also, when `ordering' is RETURN_IN_ORDER,
   each non-option ARGV-element is returned here.  */

char *optarg = 0;

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to `getopt'.

   On entry to `getopt', zero means this is the first call; initialize.

   When `getopt' returns EOF, this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise, `optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

int optind = 0;

/* The next char to be scanned in the option-element
   in which the last option character we returned was found.
   This allows us to pick up the scan where we left off.

   If this is zero, or a null string, it means resume the scan
   by advancing to the next ARGV-element.  */

static char *nextchar;

/* Callers store zero here to inhibit the error message
   for unrecognized options.  */

int opterr = 1;

/* Describe how to deal with options that follow non-option ARGV-elements.

   If the caller did not specify anything,
   the default is REQUIRE_ORDER if the environment variable
   POSIXLY_CORRECT is defined, PERMUTE otherwise.

   REQUIRE_ORDER means don't recognize them as options;
   stop option processing when the first non-option is seen.
   This is what Unix does.
   This mode of operation is selected by either setting the environment
   variable POSIXLY_CORRECT, or using `+' as the first character
   of the list of option characters.

   PERMUTE is the default.  We permute the contents of ARGV as we scan,
   so that eventually all the non-options are at the end.  This allows options
   to be given in any order, even with programs that were not written to
   expect this.

   RETURN_IN_ORDER is an option available to programs that were written
   to expect options and other ARGV-elements in any order and that care about
   the ordering of the two.  We describe each non-option ARGV-element
   as if it were the argument of an option with character code 1.
   Using `-' as the first character of the list of option characters
   selects this mode of operation.

   The special argument `--' forces an end of option-scanning regardless
   of the value of `ordering'.  In the case of RETURN_IN_ORDER, only
   `--' can cause `getopt' to return EOF with `optind' != ARGC.  */

static enum
{
  REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering;


#define	my_index strchr
#define	my_bcopy(src, dst, n)	memcpy ((dst), (src), (n))


/* Handle permutation of arguments.  */

/* Describe the part of ARGV that contains non-options that have
   been skipped.  `first_nonopt' is the index in ARGV of the first of them;
   `last_nonopt' is the index after the last of them.  */

static int first_nonopt;
static int last_nonopt;

/* Exchange two adjacent subsequences of ARGV.
   One subsequence is elements [first_nonopt,last_nonopt)
   which contains all the non-options that have been skipped so far.
   The other is elements [last_nonopt,optind), which contains all
   the options processed since those non-options were skipped.

   `first_nonopt' and `last_nonopt' are relocated so that they describe
   the new indices of the non-options in ARGV after they are moved.  */

static void exchange (char **argv) {
	int nonopts_size;					/* paranoia - bkph */
	char **temp;						/* paranoia - bkph */
/*  int nonopts_size = (last_nonopt - first_nonopt) * sizeof (char *); */
	nonopts_size = (last_nonopt - first_nonopt) * sizeof (char *);
/*  char **temp = (char **) _alloca (nonopts_size); */
	temp = (char **) _alloca (nonopts_size);

  /* Interchange the two blocks of data in ARGV.  */

	my_bcopy ((char *) &argv[first_nonopt], (char *) temp, nonopts_size);
	my_bcopy ((char *) &argv[last_nonopt], (char *) &argv[first_nonopt],
	    (optind - last_nonopt) * sizeof (char *));
	my_bcopy ((char *) temp,
	    (char *) &argv[first_nonopt + optind - last_nonopt],
	    nonopts_size);

  /* Update records for the slots the non-options now occupy.  */

  first_nonopt += (optind - last_nonopt);
  last_nonopt = optind;
}


char *getenvshroud (char *);		/* in texmf.c */

/* Scan elements of ARGV (whose length is ARGC) for option characters
   given in OPTSTRING.

   If an element of ARGV starts with '-', and is not exactly "-" or "--",
   then it is an option element.  The characters of this element
   (aside from the initial '-') are option characters.  If `getopt'
   is called repeatedly, it returns successively each of the option characters
   from each of the option elements.

   If `getopt' finds another option character, it returns that character,
   updating `optind' and `nextchar' so that the next call to `getopt' can
   resume the scan with the following option character or ARGV-element.

   If there are no more option characters, `getopt' returns `EOF'.
   Then `optind' is the index in ARGV of the first ARGV-element
   that is not an option.  (The ARGV-elements have been permuted
   so that those that are not options now come last.)

   OPTSTRING is a string containing the legitimate option characters.
   If an option character is seen that is not listed in OPTSTRING,
   return '?' after printing an error message.  If you set `opterr' to
   zero, the error message is suppressed but we still return '?'.

   If a char in OPTSTRING is followed by a colon, that means it wants an arg,
   so the following text in the same ARGV-element, or the text of the following
   ARGV-element, is returned in `optarg'.  Two colons mean an option that
   wants an optional arg; if there is text in the current ARGV-element,
   it is returned in `optarg', otherwise `optarg' is set to zero.

   If OPTSTRING starts with `-' or `+', it requests different methods of
   handling the non-option ARGV-elements.
   See the comments about RETURN_IN_ORDER and REQUIRE_ORDER, above.

   Long-named options begin with `--' instead of `-'.
   Their names may be abbreviated as long as the abbreviation is unique
   or is an exact match for some defined option.  If they have an
   argument, it follows the option name in the same ARGV-element, separated
   from the option name by a `=', or else the in next ARGV-element.
   When `getopt' finds a long-named option, it returns 0 if that option's
   `flag' field is nonzero, the value of the option's `val' field
   if the `flag' field is zero.

   The elements of ARGV aren't really const, because we permute them.
   But we pretend they're const in the prototype to be compatible
   with other systems.

   LONGOPTS is a vector of `struct option' terminated by an
   element containing a name which is zero.

   LONGIND returns the index in LONGOPT of the long-named option found.
   It is only valid when a long-named option has been found by the most
   recent call.

   If LONG_ONLY is nonzero, '-' as well as '--' can introduce
   long-named options.  */

int _getopt_internal (int argc, char *const *argv, const char *optstring,
					  const struct option *longopts, int *longind, int long_only) {
	int option_index;
	char *commandlineflag = "command line flag";

	optarg = 0;

  /* Initialize the internal data when the first call is made.
     Start processing options with ARGV-element 1 (since ARGV-element 0
     is the program name); the sequence of previously skipped
     non-option ARGV-elements is empty.  */

	if (optind == 0) {
		first_nonopt = last_nonopt = optind = 1;

		nextchar = NULL;

/*		 Determine how to handle the ordering of options and nonoptions.  */

		if (optstring[0] == '-') {
			ordering = RETURN_IN_ORDER;
			++optstring;
		}
		else if (optstring[0] == '+') {
			ordering = REQUIRE_ORDER;
			++optstring;
		}
/*      else if (getenv ("POSIXLY_CORRECT") != NULL) */
		else if (getenvshroud ("QPTJYMZ`DPSSFDU") != NULL)
			ordering = REQUIRE_ORDER;
		else
			ordering = PERMUTE;
	}

	if (nextchar == NULL || *nextchar == '\0') {
		if (ordering == PERMUTE) {
	  /* If we have just processed some options following some non-options,
	     exchange them so that the options come first.  */

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange ((char **) argv);
			else if (last_nonopt != optind)
				first_nonopt = optind;

	  /* Now skip any additional non-options
	     and extend the range of non-options previously skipped.  */

			while (optind < argc
				   && (argv[optind][0] != '-' || argv[optind][1] == '\0')
				  )
				optind++;
			last_nonopt = optind;
		}

/*	 Special ARGV-element `--' means premature end of options.
	 Skip it like a null option,
	 then exchange with previous non-options as if it were an option,
	 then skip everything else like a non-option.  */

		if (optind != argc && !strcmp (argv[optind], "--")) {
			optind++;

			if (first_nonopt != last_nonopt && last_nonopt != optind)
				exchange ((char **) argv);
			else if (first_nonopt == last_nonopt)
				first_nonopt = optind;
			last_nonopt = argc;

			optind = argc;
		}

/*	 If we have done all the ARGV-elements, stop the scan
	 and back over any non-options that we skipped and permuted.  */

		if (optind == argc) {
	  /* Set the next-arg-index to point at the non-options
	     that we previously skipped, so the caller will digest them.  */
			if (first_nonopt != last_nonopt)
				optind = first_nonopt;
			return EOF;
		}

/*   If we have come to a non-option and did not permute it,
	 either stop the scan or describe it to the caller and pass it by.  */

		if ((argv[optind][0] != '-' || argv[optind][1] == '\0')) {
			if (ordering == REQUIRE_ORDER)
				return EOF;
			optarg = argv[optind++];
			return 1;
		}

/*	 We have found another option-ARGV-element.
	 Start decoding its characters.  */ /* unusual use of booleane */

		nextchar = (argv[optind] + 1
					+ (longopts != NULL && argv[optind][1] == '-'));
	}

	if (longopts != NULL
		  && ((argv[optind][0] == '-'
			   && (argv[optind][1] == '-' || long_only)))) {
		const struct option *p;
		char *s = nextchar;
		int exact = 0;
		int ambig = 0;
		const struct option *pfound = NULL;
		int indfound=0;		/* keep compiler quiet */

		while (*s && *s != '=')
			s++;

/*		Test all options for either exact match or abbreviated matches.  */
		for (p = longopts, option_index = 0; p->name;
			 p++, option_index++)
			if (!strncmp (p->name, nextchar, s - nextchar))
			{
/*	    if (s - nextchar == strlen (p->name)) */
				if (s - nextchar == (int) strlen (p->name))	{ /* avoid warning bkph */
/*			Exact match found.  */
					pfound = p;
					indfound = option_index;
					exact = 1;
					break;
				}
				else if (pfound == NULL) {
		/* First nonexact match found.  */
					pfound = p;
					indfound = option_index;
				}
				else
		  /* Second nonexact match found.  */
					ambig = 1;
			}

		if (ambig && !exact) {
			if (opterr) {
				sprintf(logline,
						"%s `%s' is ambiguous\n", commandlineflag, argv[optind]);
				showline(logline, 1);
			}
			nextchar += strlen (nextchar);
			optind++;
			return '?';
		}

		if (pfound != NULL) {
			option_index = indfound;
			optind++;
			if (*s)	{
/*			Don't test has_arg with >, because some C compilers don't
			allow it to be used on enums.  */
				if (pfound->has_arg)
					optarg = s + 1;
				else {
					if (opterr) {
						if (argv[optind - 1][1] == '-') {		/* --option */
//				  fprintf (stderr,
							sprintf(logline,
									"%s `--%s' does not take an argument\n",
									commandlineflag,pfound->name);
							showline(logline, 1);
						}
						else {			/* +option or -option */
//				  fprintf (stderr,
							sprintf(logline,
									"%s `%c%s' does not take an argument\n",
									commandlineflag, argv[optind - 1][0], pfound->name);
							showline(logline, 1);
						}
					}
					nextchar += strlen (nextchar);
					return '?';
				}
			}
			else if (pfound->has_arg == 1) {
				if (optind < argc)
					optarg = argv[optind++];
				else  {
					if (opterr) {
//				fprintf (stderr, 
						sprintf(logline,
								"%s `%s' requires an argument\n",
								commandlineflag, argv[optind - 1]);
						showline(logline, 1);
					}
					nextchar += strlen (nextchar);
					return '?';
				}
			}
			nextchar += strlen (nextchar);
			if (longind != NULL)
				*longind = option_index;
			if (pfound->flag)
			{
				*(pfound->flag) = pfound->val;
				return 0;
			}
			return pfound->val;
		}
	  /* Can't find it as a long option.  If this is not getopt_long_only,
	 or the option starts with '--' or is not a valid short
	 option, then it's an error.
	 Otherwise interpret it as a short option.  */
		if (!long_only || argv[optind][1] == '-'
			  || my_index (optstring, *nextchar) == NULL) {
			if (opterr)	{
				if (argv[optind][1] == '-') {		/* --option */
					sprintf (logline,
							 "don't understand %s `--%s'\n",
							 commandlineflag, nextchar);
					showline(logline, 1);
				}
				else {		/* +option or -option */
					sprintf (logline,
							 "don't understand %s `%c%s'\n",
							 commandlineflag, argv[optind][0], nextchar);
					showline(logline, 1);
				}
			}
			nextchar = (char *) "";
			optind++;
			return '?';
		}
	}

  /* Look at and handle the next option-character.  */

	{
		char c = *nextchar++;
		char *temp = my_index (optstring, c);

	/* Increment `optind' when we start to process its last character.  */
		if (*nextchar == '\0')
			++optind;

/*    if (temp == NULL || c == ':') */
		if (temp == NULL || c == ARGSEP) {
			if (opterr) {
				if (c < 040 || c >= 0177) {
//	        fprintf (stderr,
					sprintf(logline,
							"Unrecognized %s (0%o)\n", commandlineflag, c);
					showline(logline, 1);
				}
				else {
//			fprintf (stderr,
					sprintf(logline,
							"Unrecognized %s `-%c'\n", commandlineflag, c);
					showline(logline, 1);
				}
			}
			return '?';
		}
/*		if (temp[1] == ':') */
		if (temp[1] == ARGSEP) {
/*			if (temp[2] == ':') */
			if (temp[2] == ARGSEP) {
/**				This is an option that accepts an argument optionally.  */
				if (*nextchar != '\0')	{
					optarg = nextchar;
					optind++;
				}
				else
					optarg = 0;
				nextchar = NULL;
			}
			else {
/*				This is an option that requires an argument.  */
				if (*nextchar != '\0') {
					optarg = nextchar;
/*			If we end this ARGV-element by taking the rest as an arg,
			we must advance to the next element now.  */
					optind++;
				}
				else if (optind == argc) {
					if (opterr) {
						sprintf(logline,
								"%s `-%c' requires an argument\n",
								commandlineflag, c);
						showline(logline, 1);
					}
					c = '?';
				}
				else
		  /* We already incremented `optind' once;
		 increment it again when taking next ARGV-elt as argument.  */
					optarg = argv[optind++];
				nextchar = NULL;
			}
		}
		return c;
	}
}

int getopt (int argc, char *const *argv, const char *optstring) {
  return _getopt_internal (argc, argv, optstring,
			   (const struct option *) 0,
			   (int *) 0,
			   0);
}

#pragma optimize ("", on)

/* this uses output to stderr quite a bit for errors on command line */
/* clean up command line option error output */
/* disallow all the weird combinations like -- */

//////////////////////////////////////////////////////////////////////
