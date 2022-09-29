#include <stdarg.h>
#include "common.h"
#include "../uart/uart.h"

int toupper(int c)
{
	return 'a' <= c && c <= 'z' ? c + 'A' - 'a' : c;
}
