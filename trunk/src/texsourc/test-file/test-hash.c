#include <stdio.h>
#include <time.h>

typedef struct _mapping_table mapping_table;
typedef struct _mapping_entry mapping_entry;

#define mapping_size 31627

struct _mapping_table
{
  mapping_entry * entries[mapping_size];
};

struct _mapping_entry
{
  mapping_entry * next;
  char * key;
  int  * val;
};

unsigned int font_name_hash(const char * s)
{
  const char * p;
  unsigned int h = 0, g;

  for (p = s; *p != '\0'; p++)
  {
    h = (h << 4) + *p;

    if ((g = h & 0xf0000000))
    {
      h = h ^ (g >> 24);
      h = h ^ g;
    }
  }

  return h;
}

mapping_table * font_name_hash_init (void)
{
  mapping_table * temp_table;
  int i;

  temp_table = (mapping_table *) malloc(sizeof(mapping_table));

  for (i = 0; i < mapping_size; i++)
    temp_table->entries[i] = NULL;

  return temp_table;
}

void font_name_hash_free (mapping_table * tbl)
{
  int i;
  mapping_entry *e, *next;

  for (i = 0; i < mapping_size; i++)
    for (e = tbl->entries[i]; e; e = next)
    {
      next = e->next;
      free(e->key);
      free(e);
    }

  free(tbl);
}

void font_name_hash_insert (mapping_table * tbl, const char *key, int val)
{
  int i;
  mapping_entry * e;

  i = font_name_hash(key) % mapping_size;
  e = (mapping_entry *) malloc(sizeof(mapping_entry));
  e->next = tbl->entries[i];
  e->key  = (char *) strdup(key);
  e->val  = val;
  tbl->entries[i] = e;
}

int font_name_hash_lookup (mapping_table * tbl, const char *key)
{
  int i;
  mapping_entry *e;

  i = font_name_hash(key) % mapping_size;

  for (e = tbl->entries[i]; e; e = e-> next)
    if (!strcmp(key, e->key))
      return e->val;

  return -1;
}

mapping_table * gentbl;

int main (void)
{
  gentbl = font_name_hash_init();
  font_name_hash_insert(gentbl, "cmr10", 1022);
  font_name_hash_insert(gentbl, "cmr17", 100);
  font_name_hash_insert(gentbl, "cmmi10", 99);
  printf("cmmi10: %d.\n", font_name_hash_lookup(gentbl, "cmmi10"));
  printf("cmr10: %d.\n", font_name_hash_lookup(gentbl, "cmr10"));
  printf("cmr17: %d.\n", font_name_hash_lookup(gentbl, "cmr17"));
  font_name_hash_free(gentbl);
  return 0;
}
