#include "Bytecode.h"
#include "stb_ds.h"
#include <assert.h>
#include <stdbool.h>

Alloc_Table *alloc_table;
static u16 scope_allocations[1024] = {};
int current_scope = 0;

Bytecode generate_bytecode(Node *tree)
{
	Bytecode bytecode = {.bytecode = alloc_temp_memory(MB(256)), .i = 0};
	return bytecode;
}

void init_bytecode()
{
	shdefault(alloc_table, -1);
}

int find_alloc(char *name)
{
	return shget(alloc_table, name);
}

void push_byte(u8 byte, Bytecode *bytecode)
{
	bytecode->bytecode[bytecode->i++] = byte;
}

void push_word(u16 word, Bytecode *bytecode)
{
	push_byte(word >> 8, bytecode);
	push_byte(word, bytecode);
}

void push_dword(u32 dword, Bytecode *bytecode)
{
	push_word(dword >> 16, bytecode);
	push_word(dword, bytecode);
}

void push_qword(u64 qword, Bytecode *bytecode)
{
	push_dword(qword >> 32, bytecode);
	push_dword(qword, bytecode);
}

void push_binary_instruction_based_on_type(OP first_op, Bytecode *bytecode,
		const Type_Info *type_info, b32 floatable)
{
	switch(type_info->type)
	{
		case T_INT:
		{
			switch(type_info->size)
			{
				case 32:
				{
					push_byte(first_op, bytecode);
				} break;
				case 64:
				{
					push_byte(first_op + 1, bytecode);
				} break;
			}
		} break;
		case T_FLOAT:
		{
			if(!floatable)
				assert(false);

			switch(type_info->size)
			{
				case 32:
				{
					push_byte(first_op + 2, bytecode);
				} break;
				case 64:
				{
					push_byte(first_op + 3, bytecode);
				} break;
			}
		} break;
		case T_BOOL:
		{
			push_byte(first_op, bytecode);
		} break;
		default:
		{
			assert(false);
		} break;
	}
}

void push_instruction_based_on_type(OP first_op, Bytecode *bytecode,
		const Type_Info *type_info)
{
	switch(type_info->type)
	{
		case T_INT:
		{
			switch(type_info->size)
			{
				case 8:
				{
					push_byte(first_op, bytecode);
				} break;
				case 16:
				{
					push_byte(first_op + 1, bytecode);
				} break;
				case 32:
				{
					push_byte(first_op + 2, bytecode);
				} break;
				case 64:
				{
					push_byte(first_op + 3, bytecode);
				} break;
			}
		} break;
		case T_FLOAT:
		{
			switch(type_info->size)
			{
				case 32:
				{
					push_byte(first_op + 4, bytecode);
				} break;
				case 64:
				{
					push_byte(first_op + 5, bytecode);
				} break;
			}
		} break;
		case T_BOOL:
		{
			push_byte(first_op + 2, bytecode);
		} break;
		default:
		{
			assert(false);
		} break;
	}
}

void load_value(int alloc, Bytecode *bytecode, const Type_Info *type_info)
{
	push_instruction_based_on_type(LOADB, bytecode, type_info);
	push_word(alloc, bytecode);
}

void store_value(Bytecode *bytecode, char *name, const Type_Info *type_info)
{
	int alloc = scope_allocations[current_scope];

	push_instruction_based_on_type(STOREB, bytecode, type_info);
	push_word(alloc, bytecode);

	shput(alloc_table, name, scope_allocations[current_scope]++);
}

void pushop_byte(Bytecode *bytecode, u8 byte)
{
	push_byte(PUSHB, bytecode);
	push_byte(byte, bytecode);
}

void pushop_word(Bytecode *bytecode, u16 word)
{
	push_byte(PUSHW, bytecode);
	push_word(word, bytecode);
}

void pushop_dword(Bytecode *bytecode, u32 double_word)
{
	push_byte(PUSHW, bytecode);
	push_dword(double_word, bytecode);
}

void pushop_qword(Bytecode *bytecode, u64 quad_word)
{
	push_byte(PUSHW, bytecode);
	push_qword(quad_word, bytecode);
}

void generate_binary_expression(Node *binary, Bytecode *bytecode)
{
	generate_expression(binary->binary.left, bytecode);
	generate_expression(binary->binary.right, bytecode);
	switch((int)binary->binary.op->value)
	{
		case '+':
		{
			push_binary_instruction_based_on_type(ADDDW, bytecode, binary->type_info,
					true);
		} break;
		case '-':
		{
			push_binary_instruction_based_on_type(SUBDW, bytecode, binary->type_info,
					true);
		} break;
		case '*':
		{
			push_binary_instruction_based_on_type(MULDW, bytecode, binary->type_info,
					true);
		} break;
		case '/':
		{
			push_binary_instruction_based_on_type(DIVDW, bytecode, binary->type_info,
					true);
		} break;
	}
}

void generate_expression(Node *expression, Bytecode *bytecode)
{
	switch(expression->type)
	{
		case ND_ID:
		{
			int alloc = find_alloc(expression->token->string);
			assert(alloc != -1);
			load_value(alloc, bytecode, expression->type_info);
		} break;
		case ND_DECL:
		{
			generate_expression(expression->decl.expr, bytecode);
			store_value(bytecode, expression->decl.operand->token->string,
					expression->type_info);
		} break;
		case ND_LITERAL:
		{
			switch(expression->literal.type)
			{
				case LIT_CHAR:
				{
					pushop_byte(bytecode, expression->literal._i64);
				} break;
				case LIT_INT:
				{
					pushop_qword(bytecode, expression->literal._i64);
				} break;
				case LIT_DOUBLE:
				{
					pushop_qword(bytecode, expression->literal._f64);
				} break;
				default:
				assert(false);
			}
		} break;
		case ND_BINARY:
		{
			generate_binary_expression(expression, bytecode);
		} break;
	}
}

