#include "Parser.h"
#include "Error.h"
#include <assert.h>
#include <stdlib.h>

void advance_token(Token_Array *tokens)
{
	if(tokens->arr[tokens->i].value == tok_eof)
	{
		report_error(&tokens->arr[tokens->i], "Unexpected end of file");
	}
	tokens->i++;
}

// @NOTE: advances token and gives the previous one
Token *get_token(Token_Array *tokens)
{
	Token *token = &tokens->arr[tokens->i];
	advance_token(tokens);
	return token;
}

Token *peek_token(Token_Array *tokens)
{
	return tokens->arr + tokens->i;
}

Token *eat_token(Token_Array *tokens, Token_Value token)
{
	Token *next = get_token(tokens);
	if(next->value != token)
	{
		report_error(next, "Unexpected token, expected %s, got %s", get_token_string(token), get_token_string(next->value));
	}
	return next;
}

Node *alloc_node()
{
	Node *result = alloc_temp_memory(sizeof(Node));
	return result;
}

Node *node_identifier(Token *token)
{
	Node *result = alloc_node();
	result->type = ND_ID;
	result->token = token;
	return result;
}

Node *node_if(Token *token, Node *condition, Node *then)
{
	Node *result = alloc_node();
	result->type = ND_IF;
	result->token = token;
	result->if_.condition = condition;
	result->if_.then = then;
	return result;
}

Node *node_decl(Token *token, Node *operand, Node *expr, Node *type)
{
	Node *result = alloc_node();
	result->type = ND_DECL;
	result->token = token;
	result->decl.operand = operand;
	result->decl.expr = expr;
	result->decl.type = type;
	return result;
}

Node *node_fn_call(Token *token, Node *operand, Node **arguments)
{
	Node *result = alloc_node();
	result->type = ND_CALL;
	result->token = token;
	result->fn_call.arguments = arguments;
	result->fn_call.operand = operand;
	return result;
}

Node *node_fn_arg(Token *token, Token *identifier, Token *type)
{
	Node *result = alloc_node();
	result->type = ND_FN_ARG;
	result->token = token;
	result->fn_arg.identifier = identifier;
	result->fn_arg.type = type;
	return result;
}

Node *node_literal(Token *token, u64 val, Literal_Type type)
{
	Node *result = alloc_node();
	result->type = ND_LITERAL;
	result->token = token;
	result->literal._u64 = val;
	result->literal.type = type;
	return result;
}

Node *parse_tokens(Token *tokens_in)
{
	Token_Array tokens = {.arr = tokens_in, .i = 0};
	Node *root = alloc_node();
	root->type = ND_ROOT;
	root->token = &tokens_in[0];
	root->root.expressions = ArrCreate(Node *);
	while(peek_token(&tokens)->value != ';' && peek_token(&tokens)->value != tok_newline)
	{
		Node *expression = parse_expression(&tokens);
		ArrPush(root->root.expressions, expression);
	}
	return root;
}

Node *parse_func_arg(Token_Array *tokens)
{
	// @TODO: default values for function arguments
	Token *identifier = eat_token(tokens, tok_identifier);
	if(peek_token(tokens)->value != ':')
		return node_fn_arg(identifier, NULL, identifier);

	get_token(tokens);
	Token *type = eat_token(tokens, tok_identifier);
	return node_fn_arg(identifier, identifier, type);
}

Node **parse_arguments(Token_Array *tokens)
{
	eat_token(tokens, tok_left_par);
	Node **result = ArrCreate(Node *);
	while(peek_token(tokens)->value != tok_right_par)
	{
		Node *arg = parse_func_arg(tokens);
		ArrPush(result, arg);
		if(peek_token(tokens)->value != ',')
		{
			eat_token(tokens, tok_right_par);
			break;
		}
		eat_token(tokens, ',');
	}
	return result;
}

Node *parse_func(Token_Array *tokens)
{
	Node *result = alloc_node();
	result->type = ND_FN;
	result->token = eat_token(tokens, tok_func);
	Token *identifier = eat_token(tokens, tok_identifier);
	result->func.arguments = parse_arguments(tokens);
	if(peek_token(tokens)->value == tok_arrow)
	{
		get_token(tokens);
		// @TODO: fn returning fn pointer
		result->func.ret = parse_expression(tokens);
	}
	return result;
}

int
get_precedence(Token_Value op, b32 on_left)
{
	switch ((int)op)
	{
	case tok_plusplus:
	case tok_minusminus:
	case '(':
	case '[':
		return on_left ? 35 : 36;

	case '*':
	case '/':
	case '%':
		return on_left ? 33 : 34;

	case '+':
	case '-':
		return on_left ? 31 : 32;

	case tok_bits_lshift:
	case tok_bits_rshift:
		return on_left ? 29 : 30;

	case '>':
	case '<':
	case tok_logical_gequal:
	case tok_logical_lequal:
		return on_left ? 27 : 28;

	case tok_logical_is:
	case tok_logical_isnot:
		return on_left ? 25 : 26;

	case tok_bits_and:
		return on_left ? 23 : 24;
	case tok_bits_xor:
		return on_left ? 21 : 22;
	case tok_bits_or:
		return on_left ? 19 : 20;
	case tok_logical_and:
		return on_left ? 17 : 18;
	case tok_logical_or:
		return on_left ? 15 : 16;

	case tok_equals:
	case tok_plus_equals:
	case tok_minus_equals:
	case tok_mult_equals:
	case tok_div_equals:
	case tok_mod_equals:
	case tok_lshift_equals:
	case tok_rshift_equals:
	case tok_and_equals:
	case tok_xor_equals:
	case tok_or_equals:
		return on_left ? 10 : 11;

	}
	return 0;
}

Node *parse_atom(Node *operand, Token_Array *tokens)
{
	b32 loop = true;
	while(loop)
	{
		Token *token = peek_token(tokens);
		switch((int)token->value)
		{
			case '(':
			{
				get_token(tokens);
				Node **arguments = ArrCreate(Node *);
				while(peek_token(tokens)->value != ')')
				{
					Node *arg = parse_expression(tokens);
					if(arg == NULL)
					{
						report_error(token, "Expected arguments for function call");
					}
					ArrPush(arguments, arg);
					if(peek_token(tokens)->value != ',')
					{
						eat_token(tokens, ')');
					}
					get_token(tokens);
				}
				operand = node_fn_call(token, operand, arguments);
			} break;
			case ':':
			{
				if(operand->type != ND_ID)
				{
					report_error(token, "Expected identifier before ':' declaration");
				}
				get_token(tokens);
				Node *type = NULL;
				if(peek_token(tokens)->value == '=')
				{
					get_token(tokens);
				}
				else
				{
					type = parse_operand(tokens);
					eat_token(tokens, '=');
				}
				Node *assign_expr = parse_expression(tokens);
				operand = node_decl(token, operand, assign_expr, type);
			} break;
			default:
			{
				loop = false;
			} break;
		}
	}
	return operand;
}

Node *parse_operand(Token_Array *tokens)
{
	Node *result = NULL;
	Token *token = peek_token(tokens);
	switch((int)token->value)
	{
		case tok_func:
		{
			result = parse_func(tokens);
		} break;
		case tok_identifier:
		{
			result = node_identifier(get_token(tokens));
		} break;
		case tok_char:
		{
			result = node_literal(get_token(tokens), (u64)token->string[0], LIT_CHAR);
		} break;
		case tok_number:
		{
			b32 is_float = false;
			for(int i = 0; i < token->identifier_size; ++i)
			{
				if(token->string[i] == '.')
				{
					is_float = true;
					break;
				}
			}
			if(is_float)
			{
				f64 number = strtod(token->string, NULL);
				result = node_literal(get_token(tokens), *(u64 *)&number, LIT_DOUBLE);
			}
			else
			{
				if(token->string[0] == '-')
				{
					i64 number = strtoll(token->string, NULL, 10);
					result = node_literal(get_token(tokens), *(u64 *)&number, LIT_INT);
				}
				else
				{
					u64 number = strtoull(token->string, NULL, 10);
					result = node_literal(get_token(tokens), number, LIT_UINT);
				}
			}
		} break;
		case tok_const_str:
		{
			result = alloc_node();
			result->type = ND_STRING;
			result->token = get_token(tokens);
		} break;
		case '(':
		{
			get_token(tokens);
			result = parse_expression(tokens);
			eat_token(tokens, tok_right_par);
		} break;
		case '{':
		{
			get_token(tokens);
			result = alloc_node();
			result->type = ND_BODY;
			result->token = token;
			result->body.expressions = ArrCreate(Node *);
			while(peek_token(tokens)->value != '}')
			{
				Node *expr = parse_expression(tokens);
				ArrPush(result->body.expressions, expr);
			};
		} break;
		case '[':
		{
			// array list
			get_token(tokens);
			assert(false);
		} break;
	}
	return result;
}

Node *parse_unary_expression(Token_Array *tokens)
{
	Token *token = peek_token(tokens);
	switch((int)token->value)
	{
		case tok_if:
		{
			get_token(tokens);
			Node *condition = parse_expression(tokens);
			Node *then = parse_expression(tokens);
			return node_if(token, condition, then);
		} break;
	}
	Node *result = parse_atom(parse_operand(tokens), tokens);
	if(result == NULL)
	{
		report_error(token, "Expected operand for expression");
	}
	return result;
}

Node *parse_binary_expression(Token_Array *tokens, int min_bp)
{
	Node *result = parse_unary_expression(tokens);
	while(true)
	{
		Token *token = peek_token(tokens);
		int l_bp = get_precedence(token->value, true);
		int r_bp = get_precedence(token->value, false);
		if(l_bp < min_bp)
			break;
		get_token(tokens);

		Node *right = parse_binary_expression(tokens, r_bp);
		Node *binary = alloc_node();
		binary->type = ND_BINARY;
		binary->binary.left = result;
		binary->binary.right = right;
		binary->binary.op = token;
		binary->token = token;
		result = binary;
	}
	return result;
}

Node *parse_expression(Token_Array *tokens)
{
	return parse_binary_expression(tokens, 1);
}

