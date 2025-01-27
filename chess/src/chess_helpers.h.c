#include <stdint.h>
#include <stdbool.h>

#pragma once

typedef struct StringIndex {
	const char * chars;
	uint32_t length;
} StringIndex;

// helpers
// introduced in board
bool ContainsChar(const char c, StringIndex string);
uint32_t CountChar(const char c, StringIndex string);
uint32_t Stringlength(const char * string);
// introduced in bitboards
#define isCapitalChar(c) ((c) >= 'A' && (c) <= 'Z')
#define lowerChar(c) (isCapitalChar(c) ? (c) - ('A' - 'a') : (c))
#define ArrayPair(k, v) [k] = (v)
