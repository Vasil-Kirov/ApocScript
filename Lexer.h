
#ifndef _LEXER_H
#define _LEXER_H

#include "Basic.h"

typedef enum : signed short
{
	tok_semi_colon = ';',

	tok_minus = '-',
	tok_plus = '+',
	tok_not = '!',
	tok_star = '*',
	tok_equals = '=',
	tok_left_par = '(',
	tok_right_par = ')',
	
	tok_eof = -1,
	
	// syntax commands
	tok_func = -2,
	tok_arrow = -4,
	tok_struct = -5,
	//tok_cast = -6, // #type
	tok_if = -7,   // if
	tok_for = -8,  // for
	
	// primary
	tok_identifier = -9,
	tok_const_str = -10,
	tok_number = -11,
	
	// special signs
	tok_logical_or = -12,	 // ||
	tok_logical_is = -13,	 // ==
	tok_logical_isnot = -14, // !=
	tok_logical_and = -15,	 // &&
	tok_logical_lequal = -16,
	tok_logical_gequal = -17,
	tok_logical_greater = '>',
	tok_logical_lesser = '<',
	
	// bits
	tok_bits_lshift = -18, // <<
	tok_bits_rshift = -19, // >>
	tok_bits_or = '|',
	tok_bits_xor = '^',
	tok_bits_not = '~',
	tok_bits_and = '&',
	
	//
	tok_plusplus = -20,	  // ++
	tok_minusminus = -21, // --
	
	// additional syntax (i am tired of chaning the top ones)
	tok_switch = -23, // switch statement
	tok_case = -24,	  // case in switch statement
	
	tok_plus_equals = -26,
	tok_minus_equals = -27,
	tok_mult_equals = -28,
	tok_div_equals = -29,
	tok_mod_equals = -30,
	tok_and_equals = -31,
	tok_xor_equals = -32,
	tok_or_equals = -33,
	tok_lshift_equals = -35,
	tok_rshift_equals = -36,
	
	tok_break = -37,
	tok_else = -38,
	
	tok_char = -40,
	tok_newline = -41,
	TOK_ERROR = -99
} Token_Value;

typedef struct
{
	Token_Value value;
	char *string;
	int identifier_size;
} Token;

typedef struct
{
	char *data;
	char *end;
} Parsing_Buffer;

Token lex_token(Parsing_Buffer *buf);
Token *lex_statement(Parsing_Buffer *buf);
const char *get_token_string(Token_Value token);

#endif // _LEXER_H

