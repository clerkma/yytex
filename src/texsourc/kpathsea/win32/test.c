#include <stdio.h>
#include <kpathsea/kpathsea.h>

int main(int argc, char * argv[])
{
  char **name;
  char *t[] = { "./foo", "\\\\server\\foo\\bar", "ftp://localhost/foo",
  "D:/helo.tt"};

  for (name = t; name - t < sizeof(t)/sizeof(char*); name++) {
    printf ("Path `%s' %s absolute.\n", *name,
            kpse_absolute_p(*name, false) ? "is" : "is not");
  }
  return 0;
}
