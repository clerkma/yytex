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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <dos.h>
/* #include <io.h> */				/* for _access ? */ 

int verboseflag=1;
int traceflag=0;

#define FONTMAP

#ifdef FONTMAP

#define MAXLINE 256

char *mapfile = "texfonts.map";			/* name aliasing file */

char *texfontsenv = "TEXFONTS";			/* environment variable */

int filemethod=1;
int dirmethod=1;

typedef struct map_element_struct {		/* list element */
	char *key;
	char *value;
	struct map_element_struct *next;
} map_element_type;

typedef map_element_type **map_type; 

/* **************************** auxiliary functions *********************** */

static void complain_mem (unsigned int size) {
	fprintf  (stderr, "Unable to honor request for %u bytes.\n", size);
	exit (1);
}

static void *xmalloc (unsigned int size) {
	void *new_mem = (void *) malloc (size);
	if (new_mem == NULL) complain_mem(size);
	return new_mem;
}

static void *xrealloc (void *old_ptr, unsigned int size) {
	void *new_mem;
	if (old_ptr == NULL)
		new_mem = xmalloc (size);	/* could just let realloc do this case? */
	else {
		new_mem = (void *) realloc (old_ptr, size);
		if (new_mem == NULL) complain_mem(size);
	}
	return new_mem;
}

static char *xstrdup (char *s) {
	char *new_string = (char *) xmalloc (strlen (s) + 1);
	return strcpy (new_string, s);
}

static char *concat3 (char *s1, char *s2, char *s3) {
	char *answer
		= (char *) xmalloc (strlen (s1) + strlen (s2) + strlen (s3) + 1);
	strcpy (answer, s1);
	strcat (answer, s2);
	strcat (answer, s3);
	return answer;
}

static void *xcalloc (unsigned int nelem, unsigned int elsize) {
	void *new_mem = (void *) calloc (nelem, elsize);
	if (new_mem == NULL) complain_mem (nelem * elsize);
	return new_mem;
}

static char *find_suffix (char *name) {
	char *dot_pos; 
	char *slash_pos; 
  
	dot_pos = strrchr (name, '.');
	if (dot_pos == NULL) return NULL;		/* short-cut */
	if ((slash_pos = strrchr (name, '/')) != NULL) ;
	else if ((slash_pos = strrchr (name, '\\')) != NULL) ;  
	else if ((slash_pos = strrchr (name, ':')) != NULL) ;
	else slash_pos = name;
	if (dot_pos < slash_pos) return NULL;
	return dot_pos + 1;
}

static char *extend_filename (char *name, char *default_suffix) {
  char *suffix = find_suffix (name);
  char *new_s;
  if (suffix != NULL) return name;	/* not duplicated ... */
  new_s = concat3 (name, ".", default_suffix);
  return new_s;						/* newly allocated */
}

static char *remove_suffix (char *s) {
	char *ret;
	char *suffix = find_suffix (s);
  
	if (suffix == NULL) return NULL;
	suffix--;						/* Back up to before the dot.  */
	ret = (char *) xmalloc (suffix - s + 1);
	strncpy (ret, s, suffix - s);
	ret[suffix - s] = 0;
	return ret;						/* newly allocated */
}

static FILE *xfopen (char *filename, char *mode) {
	FILE *f = fopen (filename, mode);

	if (f == NULL) {
	  perror (filename);
	  exit (errno);
	}
	return f;
}

static void xfclose (FILE *f, char *filename) {
	if (ferror(f) != 0 || fclose(f) == EOF) {
		perror (filename);
		exit (errno);
	}
}

/* only used by fontmap.c */ /* why not just use fgets ! */

#define BLOCK_SIZE 40

static char *read_line (FILE *f) {
	int c;
	unsigned int limit = BLOCK_SIZE;
	unsigned int loc = 0;
	char *line = (char *) xmalloc (limit);
  
	while ((c = getc (f)) != EOF && c != '\n') {
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
  
/*	If we read anything, return it.  This can't represent a last
     ``line'' which doesn't end in a newline, but so what.  */ /* BALLS! */
	if (c != EOF) { 
		line[loc] = 0;
	}
	else if (loc > 0) {	/* c == EOF, but line not empty */
		line[loc] = 0;
	}
	else { /* c == EOF and nothing on the line --- at end of file.  */
		free (line);
		line = NULL;
	}
	return line;
}

/* ************************************************************************* */

/* Fontname mapping.  We use a straightforward hash table.  */

#define MAP_SIZE 199

/* The hash function.  We go for simplicity here.  */

static unsigned int map_hash (char *key) {
	unsigned int n = 0;
	char *s = key;
/*	There are very few font names which are anagrams of each other
	so no point in weighting the characters.  */
	while (*s != 0) n += *s++;
	n %= MAP_SIZE;
	if (traceflag) printf("hash %u for %s\n", n, key);
	return n;
}

/* Look up STR in MAP.  Return the corresponding `value' or NULL.  */

static char *map_lookup_str (map_type map, char *key) {
	unsigned int n = map_hash (key);
	map_element_type *p;
  
	for (p = map[n]; p != NULL; p = p->next) {
		if (traceflag) printf("Trying %s against %s\n", key, p->key);
		if (strcmp (key, p->key) == 0) return p->value;
		if (traceflag) printf("p->next %p\n", p->next);
	}
	printf("failed to find %s\n", key);
	return NULL;					/* failed to find it */
}

static void map_show (map_type map) {
	map_element_type *p;
	unsigned int n;
  
	for (n = 0; n < MAP_SIZE; n++) {
		for (p = map[n]; p != NULL; p = p->next) {
			printf("n %u key %s next %p\n", n, p->key, p->next);
		}
	}
}

/*	Look up KEY in MAP; if it's not found, remove any suffix from KEY and
	try again.  Then paste key back into answer ... */

char *map_lookup (map_type map, char *key) {
	char *ret = map_lookup_str (map, key);
	char *suffix = find_suffix (key);
  
	if (!ret) {
      /* OK, the original KEY didn't work.  Let's check for the KEY without
         an extension -- perhaps they gave foobar.tfm, but the mapping only
         defines `foobar'.  */
		if (suffix) {
			char *base_key = remove_suffix (key);
			ret = map_lookup_str (map, base_key);
			free (base_key);
		}
	}

/* Append the same suffix we took off, if necessary.  */
/*	if (ret) ret = extend_filename (ret, suffix); */
	if (ret && suffix) ret = extend_filename (ret, suffix); 
	return ret;
}

/* If KEY is not already in MAP, insert it and VALUE.  Was a mess! */

static void map_insert (map_type map, char *key, char *value) {
	unsigned int n = map_hash (key);
	map_element_type **ptr = &map[n]; 

	while (*ptr != NULL && !(strcmp(key, (*ptr)->key) == 0))  
		ptr = &((*ptr)->next); 

	if (*ptr == NULL) {
		*ptr = (map_element_type *) xmalloc (sizeof(map_element_type));
		(*ptr)->key = xstrdup (key);
		(*ptr)->value = xstrdup (value);
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

/* What a load of bull! And who cares about DVIPS and stupid VF files !*/

static void map_file_parse (map_type map, char *map_filename) {
	char *l;
	unsigned int map_lineno = 0;
	unsigned int entries = 0;
	FILE *f = xfopen (map_filename, "r");
  
	while ((l = read_line (f)) != NULL) {
		char *filename;
		char *comment_loc;

		comment_loc = strrchr (l, '%');
		if (comment_loc == NULL) comment_loc = strrchr (l, ';');
      
/*	Ignore anything after a % or ;  */
		if (comment_loc)  *comment_loc = 0;
      
		map_lineno++;
      
/*	If we don't have any filename, that's ok, the line is blank.  */
		filename = strtok (l, " \t");
		if (filename) {
			char *alias = strtok (NULL, " \t");
          
			/* But if we have a filename and no alias, something's wrong.  */
			if (alias == NULL || *alias == 0)
				fprintf (stderr,
					" font name `%s', but no mapping (line %u in file %s).\n",
						filename, map_lineno, map_filename);
			else {       /* We've got everything.  Insert the new entry.  */
				map_insert (map, alias, filename); /* alias is the key */
				entries++;
			}
		}
		free (l);
	}
	xfclose (f, map_filename);
	if (traceflag) printf("%u entries\n", entries);
}

/* Look for the file `texfonts.map' in each of the directories in
   DIR_LIST.  Entries in earlier files override later files.  */

/* use _searchenv instead ? */

map_type map_create (char *envvar) {
	char filename[_MAX_PATH];
	map_type map;
      
	if (traceflag) printf("Creating alias table\n");
	_searchenv ("texfonts.map", envvar, filename);
	if (*filename == '\0') return NULL;

	map = (map_type) xcalloc (MAP_SIZE, sizeof (map_element_type *));
	map_file_parse (map, filename);
	if (traceflag) map_show(map);
	return map;
}

/* ************************************************************************* */

/*	if we didn't find the font, maybe its alias to be found in texfonts.map */
char *alias (char *name) {  
	static map_type fontmap = NULL;			/* keep around once set */
	char *mapped_name;
      
	/* Fault in the mapping if necessary.  */
	if (fontmap == NULL) fontmap = map_create ("TEXFONTS");
      
	/* Now look for our filename in the mapping.  */
	mapped_name = map_lookup (fontmap, name);
	return mapped_name;						/* possibly NULL */
}

/* ************************************************************** */

#endif

int main (int argc, char *argv[]) {
	int m;
	char *mapped_name;
	for (m = 1; m < argc; m++) {
		mapped_name = alias(argv[m]);
		if (mapped_name) {
			printf("%d %s => %s\n", m-1, argv[m], mapped_name);
			free(mapped_name);
		}
		else printf("%d %s => NULL\n", m-1, argv[m]);
	}
	if ((m = _heapchk ()) != _HEAPOK) {	
		fprintf(stderr, "WARNING: Heap corrupted (%d)\n", m);
	}
	return 0;
}
