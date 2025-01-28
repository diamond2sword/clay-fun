#include <stdint.h>
#include "chess_err.h.c"

#pragma once

#define CHESS_ARENA_SIZE 65536
typedef struct ChessArena {
	char* begin;
	char* end;
	char* current;
} ChessArena;

void ChessArena_Init(ChessArena* arena)
{
	static char memory[CHESS_ARENA_SIZE];
	arena->begin = memory;
	arena->end = memory + CHESS_ARENA_SIZE;
	arena->current = arena->begin;
}

void* ChessArena_Allocate(ChessArena* arena, size_t size)
{
	if (arena->current + size > arena->end)
	{
		err(return NULL, "out of memory");
	}
	void* memory_allocated = arena->current;
	arena->current += size;
	return memory_allocated;
}

void ChessArena_Reset(ChessArena* arena)
{
	arena->current = arena->begin;
}
