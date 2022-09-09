#include "common.h"

int toupper(int c) {
  return 'a' <= c && c <= 'z' ? c + 'A' - 'a' : c;
}
