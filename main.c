#include <stdio.h>
#include "api.h"

int main(void)
{
  printf("Start\n");
  Api* api = Api_create("https://...");
  if (! api) {
    fprintf(stderr, "Error in creation of Api object\n");
    return 1;
  }

  test(api);
  
  Api_destroy(api);
  printf("End\n");
  return 0;
}
