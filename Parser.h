
#ifndef _PARSER_H
#define _PARSER_H

#include "Lexer.h"

typedef enum
{
	LIT_UINT,
	LIT_INT,
	LIT_DOUBLE,
	LIT_CHAR
} Literal_Type;

typedef enum
{
	ND_ERROR,
	ND_ROOT,
	ND_FN,
	ND_FN_ARG,
	ND_IF,
	ND_BINARY,
	ND_ID,
	ND_LITERAL,
	ND_STRING,
	ND_BODY,
	ND_DECL,
	ND_CALL,
} Node_Type;

typedef struct _ast_node Node;

typedef struct _ast_node
{
	Node_Type type;
	Token *token;
	union
	{
		struct
		{
			Node **expressions; // Dynamic array
		} root;
		struct
		{
			Node *condition;
			Node *then;
		} if_;
		struct
		{
			Node *left;
			Node *right;
			Token *op;
		} binary;
		struct
		{
			union
			{
				u64 _i64;
				i64 _u64;
				f64 _f64;
			};
			Literal_Type type;
		} literal;
		struct
		{
			Node **arguments; // Dynamic array
			Node *ret;
		} func;
		struct
		{
			Token *identifier;
			Token *type;
		} fn_arg;
		struct
		{
			Node **expressions; // Dynamic array
		} body;
		struct
		{
			Node *operand;
			Node *expr;
			Node *type;
		} decl;
		struct
		{
			Node *operand;
			Node **arguments;
		} fn_call;
	};
} _ast_node;

typedef struct
{
	Token *arr;
	int i;
} Token_Array;

Node *parse_tokens(Token *tokens);
Node *parse_expression(Token_Array *tokens);
Node *parse_operand(Token_Array *tokens);

#endif

