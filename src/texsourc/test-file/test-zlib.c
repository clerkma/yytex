// cl -Izlib test-zlib.c zlib/zlib.lib
// cc test-zlib.c -lz
#include <stdio.h>
#include <zlib.h>

int main (void)
{
  FILE * fmt;
  gzFile fmtfile;

  char * demo = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
                "tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim "
                "veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex "
                "ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate "
                "velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat "
                "cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id "
                "est laborum. ";
  char * eemo = (char *) malloc(strlen(demo));

  fmt = fopen("zen.fmt", "wb");

  fmtfile = gzdopen(fileno(fmt), "wb" "R9");

  gzwrite(fmtfile, demo, strlen(demo));
  
  gzclose(fmtfile);

  fmt = fopen("zen.fmt", "rb");

  fmtfile = gzdopen(fileno(fmt), "rb" "R9");

  gzread(fmtfile, eemo, strlen(demo));
  printf("%s", eemo);

  free(eemo);

  gzclose(fmtfile);

  return 0;
}
