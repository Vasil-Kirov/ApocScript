
#ifndef _ANALYZER_H
#define _ANALYZER_H

#include "Basic.h"
#include "Parser.h"

typedef enum
{
	INVALID,
	T_INT,
	T_UINT,
	T_FLOAT,
	T_STRUCT,
	T_STRING,
	T_FN,
	T_BOOL,
} Type_Type;

typedef struct _Type_Info
{
	Type_Type type;
	int size;
	const char *name;
	union
	{
		struct
		{
			const struct _Type_Info **arguments; // Dynamic array
			const struct _Type_Info *ret;
		} fn;
	};
} Type_Info;

typedef struct
{
	Node **arr;
	int i;
	int length;
} Expr_Arr;

typedef struct
{
	const Type_Info *type;
	Token *id;
} Symbol;

typedef struct
{
	Symbol *symbols;
	Token *token;
} Scope;

typedef struct
{
	Scope scopes[256];
	int size;
} Scope_Array;

typedef struct
{
	char *key;
	Type_Info *value;
} Type_Table;

void analyze_ast(Node *root);
const Type_Info *analyze_expression(Node *expressions);
const Type_Info *analyze_next_expression(Expr_Arr *exprs);


#endif // _ANALYZER_H

