#include "Error.h"
#include <stdarg.h>
#include <stdio.h>
#include <vadefs.h>

void report_error(Token *token, const char *error_msg, ...)
{
	char print_buffer[4096] = {};

	va_list args;
	va_start(args, error_msg);
	vsnprintf(print_buffer, 4096, error_msg, args);
	va_end(args);

	printf("\n\n%s\n\n", print_buffer);
	exit(1);
}

