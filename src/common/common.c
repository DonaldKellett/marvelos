#include <stdarg.h>
#include "common.h"
#include "../uart/uart.h"

int toupper(int c) {
  return 'a' <= c && c <= 'z' ? c + 'A' - 'a' : c;
}

char *strcpy(char *destination, const char *source) {
  if (!destination || !source)
    return NULL;
  char *tmp = destination;
  while (*source)
    *tmp++ = *source++;
  *tmp = '\0';
  return destination;
}
