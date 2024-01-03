#include "Lexer.h"
#include "Error.h"
#include "stb_ds.h"
#include <ctype.h>

typedef struct
{
	char *key;
	Token_Value value;
} Keyword_Hash;

Keyword_Hash *keyword_table;

void advance_buffer(Parsing_Buffer *buf)
{
	buf->data++;
	if(buf->data >= buf->end)
	{
		report_error(NULL, "Unexpected end of file");
	}
}

Token *lex_statement(Parsing_Buffer *buf)
{
	Token *tokens = ArrCreate(Token);
	Token token = {};
	do {
		token = lex_token(buf);
		ArrPush(tokens, token);
	} while(token.value != ';' && token.value != tok_newline);
	Token last_token = {.value = tok_eof};
	ArrPush(tokens, last_token);
	return tokens;
}

char char_to_escaped(char c)
{
	switch(c)
    {
        case 'a':  return '\a'; break;
        case 'b':  return '\b'; break;
        case 'f':  return '\f'; break;
        case 'n':  return '\n'; break;
        case 'r':  return '\r'; break;
        case 't':  return '\t'; break;
        case 'v':  return '\v'; break;
        case '\\': return '\\'; break;
        case '\'': return '\''; break;
        case '"':  return '\"'; break;
        case '?':  return '\?'; break;
		case '0':  return '\0'; break;
		default: return 1;
    }
}

Token lex_token(Parsing_Buffer *buf)
{
	if(*buf->data == '\r')
		advance_buffer(buf);
	if(*buf->data == '\n')
	{
		Token result = {.value = tok_newline};
		return result;
	}
	while (isspace(*buf->data)) {
		advance_buffer(buf);
	}
	char *start = buf->data;
	if(isdigit(*buf->data))
	{
			b32 found_dot = false;
			do {
				advance_buffer(buf);
				if(*buf->data == '.')
				{
					if(found_dot)
					{
						// @TODO: Error handling
#if 0
						raise_token_syntax_error(f, "Number has an extra decimal point", (char *)f->path, start_line,
								start_col);
						return get_token(f);
#endif
					}
					found_dot = true;
				}
			} while (isdigit(*buf->data) || *buf->data == '.' || *buf->data == '_');
			u64 num_size = buf->data - start;
			char *number_string = VAlloc(num_size+1);
			int copy_i = 0;
			for(int i = 0; i < num_size; ++i)
			{
				if(start[i] != '_')
				{
					number_string[copy_i++] = start[i];
				}
			}
			number_string[copy_i] = '\0';

			Token result = {.value = tok_number, .string = number_string, .identifier_size = copy_i};
			return result;
	}

	if(*buf->data == '"')
	{
		advance_buffer(buf);
		while(*buf->data != '"')
		{
			if(*buf->data == '\0')
			{
				//@TODO: Error handling
				//raise_token_syntax_error(f, "Expected string literal end, got end of file", (char *)f->path, start_line, start_col);
			}
			if(*buf->data == '\\')
			{
				memmove(buf->data, buf->data + 1, VStrLen((char *)buf->data) + 1);
				*buf->data = char_to_escaped(*buf->data);
				if (*buf->data == 1)
				{
					//@TODO: Error handling
					//raise_token_syntax_error(f, "Incorrect escaped charracter", (char *)f->path, start_line, start_col);
				}
				buf->data++;
			}
			else
				advance_buffer(buf);
		}
		advance_buffer(buf);
		start++;
		u64 str_size = buf->data - start;
		char *string = VAlloc(str_size);
		memcpy(string, start, str_size);
		string[str_size-1] = '\0';

		Token result = {.value = tok_const_str, .string = string, .identifier_size = str_size - 1};
		return result;
	}
	if(*buf->data == '\'')
	{
		advance_buffer(buf);
		char c = *buf->data;
		advance_buffer(buf);
		if(*buf->data != '\'')
		{
			//@TODO: Error handling
#if 0
			raise_token_syntax_error(f, "Character literal contains more than 1 character", 
					(char *)f->path, start_line, start_col);
#endif
		}
		advance_buffer(buf);
		char *identifier = VAlloc(2);
		identifier[0] = c;
		identifier[1] = 0;
		Token result = {.value = tok_char, .string = identifier, .identifier_size = 1};
		return result;
	}
	if(ispunct(*buf->data))
	{
		if(ispunct(buf->data[1]))
		{
			char combination[3] = {};
			combination[0] = buf->data[0];
			combination[1] = buf->data[1];
			Token_Value token = shget(keyword_table, combination);
			if(token != TOK_ERROR)
			{
				advance_buffer(buf);
				advance_buffer(buf);
				Token result = {.value = token};
				return result;
			}
		}
		char c = *buf->data;
		advance_buffer(buf);
		Token result = {.value = (Token_Value)c};
		return result;
	}

	while(isalnum(*buf->data) || *buf->data == '_') 
		advance_buffer(buf);

	int identifier_size = buf->data - start;
	int name[identifier_size + 1];
	memcpy(name, start, identifier_size);
	name[identifier_size] = '\0';

	Token_Value token = shget(keyword_table, name);
	if(token == TOK_ERROR)
	{
		char *identifier = VAlloc(identifier_size + 1);
		memcpy(identifier, name, identifier_size + 1);
		Token result = {.value = tok_identifier, .string = identifier, .identifier_size = identifier_size};
		return result;
	}
	Token result = {.value = token};
	return result;
	// @TODO: Handle comments
}

void init_lexer()
{
	shdefault(keyword_table, TOK_ERROR);
	shput(keyword_table, "fn",         tok_func);
	shput(keyword_table, "struct",     tok_struct);
	shput(keyword_table, "if",         tok_if);
	shput(keyword_table, "for",        tok_for);
	shput(keyword_table, "switch",     tok_switch);
	shput(keyword_table, "case",       tok_case);
	shput(keyword_table, "break",      tok_break);
	shput(keyword_table, "else",       tok_else);

	shput(keyword_table, "->",         tok_arrow);
	shput(keyword_table, "--",         tok_minusminus);
	shput(keyword_table, "++",         tok_plusplus);
	shput(keyword_table, "||",         tok_logical_or);
	shput(keyword_table, "==",         tok_logical_is);
	shput(keyword_table, "!=",         tok_logical_isnot);
	shput(keyword_table, "&&",         tok_logical_and);
	shput(keyword_table, "<<",         tok_bits_lshift);
	shput(keyword_table, ">>",         tok_bits_rshift);
	shput(keyword_table, ">=",         tok_logical_gequal);
	shput(keyword_table, "<=",         tok_logical_lequal);
	shput(keyword_table, "+=",         tok_plus_equals);
	shput(keyword_table, "-=",         tok_minus_equals);
	shput(keyword_table, "*=",         tok_mult_equals);
	shput(keyword_table, "/=",         tok_div_equals);
	shput(keyword_table, "%=",         tok_mod_equals);
	shput(keyword_table, "&=",         tok_and_equals);
	shput(keyword_table, "^=",         tok_xor_equals);
	shput(keyword_table, "|=",         tok_or_equals);
	shput(keyword_table, "<<=",        tok_lshift_equals);
	shput(keyword_table, ">>=",        tok_rshift_equals);
}

