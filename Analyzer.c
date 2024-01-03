#include "Analyzer.h"
#include "Error.h"
#include "stb_ds.h"
#include <assert.h>

Node *get_expression(Expr_Arr *exprs)
{
	if(exprs->i >= exprs->length)
		report_error(exprs->arr[exprs->i-1]->token, "Unexpected end of expression");
	return exprs->arr[exprs->i++];
}

static Scope_Array scopes = {.scopes = {}, .size = 0};

Symbol *get_symbol(char *name)
{
	assert(scopes.size > 0);

	for(int i = 0; i < scopes.size; ++i)
	{
		Symbol *symbols = scopes.scopes[i].symbols;
		int symbol_size = ArrLen(symbols);
		for(int j = 0; j < symbol_size; ++j)
		{
			if(VStrCmp(symbols[j].id->string, name))
			{
				return symbols + j;
			}
		}
	}
	return NULL;
}

void add_symbol(Token *id, const Type_Info *type)
{
	assert(scopes.size > 0);
	
	Symbol *redecleration = get_symbol(id->string);
	if(redecleration != NULL)
	{
		// @TODO: previously declared line number
		report_error(id, "Redeclaration of symbol %s", id->string);
	}
	Symbol new_sym = {.id = id, .type = type};
	ArrPush(scopes.scopes[scopes.size - 1].symbols, new_sym);
}

void push_scope(Token *token)
{
	scopes.scopes[scopes.size].token = token;
	scopes.scopes[scopes.size].symbols = ArrCreate(Symbol);
	scopes.size++;
}

void pop_scope(Token *token)
{
	if(scopes.size <= 0)
	{
		report_error(token, "No matching scope start for scope end!");
	}

	scopes.size--;
	scopes.scopes[scopes.size].token = NULL;
	ArrFree(scopes.scopes[scopes.size].symbols);
}

Type_Table *type_table;

const Type_Info *create_fn_type(const Type_Info **args, const Type_Info *ret)
{
	Type_Info *result = (Type_Info *)VAlloc(sizeof(Type_Info *));
	result->type = T_FN;
	result->name = "fn()";
	result->fn.arguments = args;
	result->fn.ret = ret;
	return result;
}

const Type_Info *create_basic_type(const char *name, Type_Type type, int size)
{
	Type_Info *result = (Type_Info *)VAlloc(sizeof(Type_Info *));
	result->type = type;
	result->size = size;
	result->name = name;

	shput(type_table, name, result);

	return result;
}

const Type_Info *get_type(char *name)
{
	return shget(type_table, name);
}

b32 types_match(const Type_Info *a, const Type_Info *b)
{
	// @TODO: type checking
	return false;
}

void types_must_match(const Type_Info *a, const Type_Info *b, Token *token)
{
	if(!types_match(a, b))
	{
		report_error(token, "Type %s and %s don't match", a->name, b->name);
	}
}

void init_analyzer()
{
	shdefault(type_table, NULL);
	create_basic_type("i8",  T_INT,   8);
	create_basic_type("i16", T_INT,   16);
	create_basic_type("i32", T_INT,   32);
	create_basic_type("i64", T_INT,   64);
	create_basic_type("f32", T_FLOAT, 32);
	create_basic_type("f64", T_FLOAT, 64);
	create_basic_type("b32", T_BOOL,  32);
	create_basic_type("string", T_STRING, 0);
}

void free_temp_analyzer()
{
}

void analyze_ast(Node *root)
{
	int len = ArrLen(root->root.expressions);
	Expr_Arr expressions = {.arr = root->root.expressions, .i = 0, .length = len};
	push_scope(root->token);
	for (int i = 0; i < len; ++i)
	{
		analyze_next_expression(&expressions);
	}
	pop_scope(expressions.arr[expressions.i - 1]->token);
}

const Type_Info *analyze_next_expression(Expr_Arr *exprs)
{
	Node *expr = get_expression(exprs);
	return analyze_expression(expr);
}

const Type_Info *analyze_type(Node *node)
{
	// @TODO: fn type
	if(node->type == ND_ID)
	{
		return get_type(node->token->string);
	}
	else if(node->type == ND_FN)
	{
		Node **args = node->func.arguments;
		int arg_size = ArrLen(args);
		const Type_Info **arg_types = (const Type_Info **)ArrCreate(Type_Info *);
		for(int i = 0; i < arg_size; ++i)
		{
			const Type_Info *arg_type = analyze_expression(args[i]);
			ArrPush(arg_types, arg_type);
		}
		const Type_Info *ret_type = analyze_type(node->func.ret);
		return create_fn_type(arg_types, ret_type);
	}
	else
	{
		report_error(node->token, "Expected type, got %s",
				get_token_string(node->token->value));
	}

	return NULL;
}

void type_is_boolean(const Type_Info *type, Token *token)
{
	if(type->type != T_BOOL)
	{
		report_error(token, "Expected boolean expression");
	}
}

void type_is_arithmetic(const Type_Info *type, Token *token)
{
	Type_Type t = type->type;
	if(t == T_INT || t == T_FLOAT || t == T_BOOL)
		return;
	report_error(token, "Trying to perform a binary expression with non arithmetic type");
}

const Type_Info *get_literal_type(Node *literal)
{
	switch(literal->literal.type)
	{
		case LIT_CHAR:
		return get_type("i8");
		case LIT_DOUBLE:
		return get_type("f64");
		case LIT_INT:
		return get_type("i64");
		default:
		assert(false);
	}
	return NULL;
}

const Type_Info *analyze_expression(Node *expr)
{
	const Type_Info *result = NULL;
	switch(expr->type)
	{
		case ND_DECL:
		{
			// Parser already checks that it's operand is an identifier
			if (expr->decl.type)
			{
				Node *decl_type = expr->decl.type;

				result = analyze_type(decl_type);
				const Type_Info *expr_type = analyze_expression(expr->decl.expr);
				types_must_match(result, expr_type, expr->token);
			}
			else
			{
				result = analyze_expression(expr->decl.expr);
			}
			add_symbol(expr->decl.operand->token, result);
		} break;
		case ND_CALL:
		{
			const Type_Info *fn_type = analyze_expression(expr->fn_call.operand);
			if(fn_type->type != T_FN)
			{
				report_error(expr->token, "Operand of function call is not a function");
			}
			const Type_Info **args = fn_type->fn.arguments;
			int arg_size = ArrLen(args);
			int passed_size = ArrLen(expr->fn_call.arguments);
			if(passed_size != arg_size)
			{
				report_error(expr->token,
						"Incorrect number of passed arguments, wanted %d, got %d",
						arg_size, passed_size);
			}
			for(int i = 0; i < arg_size; ++i)
			{
				Node *arg = expr->fn_call.arguments[i];
				types_must_match(args[i], analyze_expression(arg), arg->token);
			}
		} break;
		case ND_BODY:
		{
			push_scope(expr->token);

			Node **body_exprs = expr->body.expressions;
			int expr_count = ArrLen(body_exprs);
			for(int i = 0; i < expr_count; ++i)
			{
				analyze_expression(body_exprs[i]);
			}

			pop_scope(expr->token);
		} break;
		case ND_STRING:
		{
			result = get_type("string");
		} break;
		case ND_LITERAL:
		{
			result = get_literal_type(expr);
		} break;
		case ND_ID:
		{
			Symbol *symbol = get_symbol(expr->token->string);
			if(symbol == NULL)
			{
				report_error(expr->token, "Undefined identifier %s", expr->token->string);
			}
			result = symbol->type;
		} break;
		case ND_BINARY:
		{
			const Type_Info *left  = analyze_expression(expr->binary.left);
			const Type_Info *right = analyze_expression(expr->binary.right);
			type_is_arithmetic(left, expr->token);
			type_is_arithmetic(right, expr->token);
			types_must_match(left, right, expr->token);
			result = left;
		} break;
		case ND_IF:
		{
			const Type_Info *condition = analyze_expression(expr->if_.condition);
			type_is_boolean(condition, expr->token);
			analyze_expression(expr->if_.then);
		} break;
		case ND_FN:
		case ND_FN_ARG:
		case ND_ROOT:
		case ND_ERROR:
		{
			report_error(expr->token, "Unexpected token");
		} break;
	}

	expr->type_info = result;
	return result;
}


