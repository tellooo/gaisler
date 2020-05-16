#include <stdio.h>
#include <stdlib.h>

main() {
  int i = 0;
  while(1) {
    void *p[100];
    for (i = 0; i < 100; i++) {
      p[i] = malloc(i*100+10);
    }
    for (i = 0; i < 100; i++) {
      free(p[i]);
    }
  }
}
