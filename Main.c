
#define _CRT_SECURE_NO_WARNINGS

#include "Basic.h"
#include "Lexer.h"
#include "Parser.h"
#include "Analyzer.h"
#include "Error.h"
#include "Bytecode.h"

#include "Lexer.c"
#include "Memory.c"
#include "Parser.c"
#include "Analyzer.c"
#include "Error.c"
#include "Bytecode.c"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

void print_tokens(Token *tokens)
{
	for(int i = 0; i < ArrLen(tokens); ++i)
		printf("%s ", get_token_string(tokens[i].value));

	putc('\n', stdout);
}

int main()
{
	init_memory();
	init_lexer();
	init_analyzer();
	init_bytecode();
	char *line = VAlloc(MB(10));
	while(true)
	{
		fgets(line, MB(10), stdin);
		int line_len = strlen(line);
		Parsing_Buffer buf = {.data = line, .end = line + line_len};
		Token *tokens = lex_statement(&buf);
		// print_tokens(tokens);

		Node *tree = parse_tokens(tokens);
		analyze_ast(tree);

		free_temp_analyzer();
		reset_temporary_memory();
		ArrFree(tokens);
	}
}

const char *get_token_string(Token_Value token) {
    switch (token) {
        case tok_semi_colon: return "semicolon";
        case tok_minus: return "minus";
        case tok_plus: return "plus";
        case tok_not: return "not";
        case tok_star: return "star";
        case tok_equals: return "equals";
        case tok_eof: return "end of file";
        case tok_func: return "function";
        case tok_arrow: return "arrow";
        case tok_struct: return "structure";
        case tok_if: return "if";
        case tok_for: return "for";
        case tok_identifier: return "identifier";
        case tok_const_str: return "constant string";
        case tok_number: return "number";
        case tok_logical_or: return "logical or";
        case tok_logical_is: return "logical is";
        case tok_logical_isnot: return "logical is not";
        case tok_logical_and: return "logical and";
        case tok_logical_lequal: return "logical less than or equal";
        case tok_logical_gequal: return "logical greater than or equal";
        case tok_logical_greater: return "logical greater than";
        case tok_logical_lesser: return "logical less than";
        case tok_bits_lshift: return "left shift";
        case tok_bits_rshift: return "right shift";
        case tok_bits_or: return "bitwise or";
        case tok_bits_xor: return "bitwise xor";
        case tok_bits_not: return "bitwise not";
        case tok_bits_and: return "bitwise and";
        case tok_plusplus: return "increment";
        case tok_minusminus: return "decrement";
        case tok_switch: return "switch";
        case tok_case: return "case";
        case tok_plus_equals: return "plus equals";
        case tok_minus_equals: return "minus equals";
        case tok_mult_equals: return "multiply equals";
        case tok_div_equals: return "divide equals";
        case tok_mod_equals: return "modulo equals";
        case tok_and_equals: return "and equals";
        case tok_xor_equals: return "xor equals";
        case tok_or_equals: return "or equals";
        case tok_lshift_equals: return "left shift equals";
        case tok_rshift_equals: return "right shift equals";
        case tok_break: return "break";
        case tok_else: return "else";
        case tok_char: return "char";
        case tok_newline: return "newline";
        case tok_left_par: return "left parenthesis";
        case tok_right_par: return "right parenthesis";
        default: return "unknown token";
    }
}

