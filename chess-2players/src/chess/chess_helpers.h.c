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
#define IN_RANGE(x, min, max) ((x) >= (min) && (x) <= (max))
#define IF_IN_RANGE(x, min, max, forTrue, forFalse) (IN_RANGE((x), (min), (max)) ? (forTrue) : (forFalse))
#define IF_SIDE(side, forNone, forWhite, forBlack) ((side) == PIECE_SIDE_INDEX_NONE ? (forNone) : (side) == PIECE_SIDE_INDEX_WHITE ? (forWhite) : (forBlack))
#define SIDE_NEGATE(side) IF_SIDE(side, side, PIECE_SIDE_INDEX_BLACK, PIECE_SIDE_INDEX_WHITE)
	
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

#define T bool
#define __Def_Array_CopyTo(T) \
void __Array_##T##_Copy(T* src_first, T* src_last, T* dst_first) \
{ \
	for (; src_first != src_last; src_first++, dst_first++) \
	{ \
		*dst_first = *src_first; \
	} \
}

#define __Def_Array_Equals(T) \
bool __Array_##T##_Equals(T* src_first, T* src_last, T* dst_first) \
{ \
	for (; src_first != src_last; src_first++, dst_first++) \
	{ \
		if(*src_first != *dst_first) \
		{ \
			return false; \
		} \
	} \
	return true; \
}


#define ARRAY_2(T, cmd, src_first, dst_first) (__INIT(__Array_##T##_##cmd)((T*)(src_first), (T*)(src_first) + sizeof(src_first)/sizeof(T), (T*)(dst_first)))


__Def_Array_Equals(bool);
__Def_Array_CopyTo(bool);
__Def_Array_CopyTo(uint64_t);

#define __Def_Array_Set(T) \
void Array_##T##_Set(T* first, T* last, T value) \
{ \
	for (; first != last; first++) \
	{ \
		*first = value; \
	} \
}

__Def_Array_Set(uint64_t);
