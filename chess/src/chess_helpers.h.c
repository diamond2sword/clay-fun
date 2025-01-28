#include <stdint.h>
#include <stdbool.h>
#include "chess_typedefs.h.c"

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
#define IN_RANGE(x, min, max) ((x) >= (min) && (x) <= (max))
#define IF_IN_RANGE(x, min, max, forTrue, forFalse) (IN_RANGE((x), (min), (max)) ? (forTrue) : (forFalse))
#define IF_SIDE(side, forNone, forWhite, forBlack) ((side) == PIECE_SIDE_INDEX_NONE ? (forNone) : (side) == PIECE_SIDE_INDEX_WHITE ? (forWhite) : (forBlack))
	
// Strings
#define __STRING_LENGTH(s) ((sizeof(s) / sizeof((s)[0])) - sizeof((s)[0]))

#define __ENSURE_STRING_LITERAL(x) ("" x "")

// Note: If an error led you here, it's because STRING can only be used with string literals, i.e. STRING("SomeString") and not STRING(yourString)
#define STRING(string) (__INIT(StringIndex) { .chars = (string), .length = __STRING_LENGTH(__ENSURE_STRING_LITERAL(string)) })

#define STRING_CONST(string) { .chars = (string), .length = __STRING_LENGTH(__ENSURE_STRING_LITERAL(string)) }

#define __INIT(type) (type)


bool ContainsChar(const char c, StringIndex string)
{
	for (uint16_t i = 0; i < string.length; i++)
	{
		if (c == string.chars[i]) return true;
	}
	return false;
}

uint32_t CountChar(const char c, StringIndex string)
{
	uint32_t count = 0;
	for (uint32_t i = 0; i < string.length; i++)
	{
		if (c == string.chars[i]) count++;
	}
	return count;
}

uint32_t Stringlength(const char * string)
{
	uint32_t length = 0;
	while (string[length] != '\0') length++;
	return length;
}

void Bitboards_All_Copy(Bitboards_All bitboardSet_src, Bitboards_All bitboardSet_dst)
{
	for (uint8_t side = 0; side < PIECE_SIDE_INDEX_COUNT; side++)
	for (uint8_t type = 0; type < PIECE_TYPE_INDEX_COUNT; type++)
	{
		bitboardSet_dst[side][type] = bitboardSet_src[side][type];
	}
}

void CastlingRights_Copy(CastlingRights castlingRights_src, CastlingRights castlingRights_dst)
{
	for (uint8_t side = 0; side < PIECE_SIDE_INDEX_COUNT; side++)
	for (uint8_t castlingSide = 0; castlingSide < CASTLING_SIDE_COUNT; castlingSide++)
	{
		castlingRights_dst[side][castlingSide] = 
			castlingRights_src[side][castlingSide];
	}
}
