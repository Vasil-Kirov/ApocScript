#include "Memory.h"
#include "Basic.h"
#include <assert.h>

#define TEMP_SIZE (GB(1))

typedef struct
{
	void *Start;
	void *Current;
	i64 Size;
} Memory_Arena;

Memory_Arena temporary_memory;

void init_memory()
{
	temporary_memory.Size = TEMP_SIZE;
	temporary_memory.Start = AllocateVirtualMemory(TEMP_SIZE);
	temporary_memory.Current = temporary_memory.Start;
};

void *alloc_temp_memory(int size)
{
	void *result = temporary_memory.Current;
	temporary_memory.Current = (char *)temporary_memory.Current + size;
	temporary_memory.Size -= size;
	if(temporary_memory.Size < 0)
	{
		assert(false);
	}
	return result;
}

void reset_temporary_memory()
{
	memset(temporary_memory.Start, 0, TEMP_SIZE - temporary_memory.Size);
	temporary_memory.Current = temporary_memory.Start;
	temporary_memory.Size = TEMP_SIZE;
}

