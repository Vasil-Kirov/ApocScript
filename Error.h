
#ifndef _ERRORS_H
#define _ERRORS_H

#include "Basic.h"
#include "Parser.h"


void report_error(Token *token, const char *error_msg, ...);

#endif // _ERRORS_H

