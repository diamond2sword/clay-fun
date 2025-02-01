//#define CHESS_VERBOSE

#pragma once
#include "chess_helpers.h.c"
#include "chess_mask.h.c"
// Logging
#ifdef CHESS_VERBOSE
	#include <stdio.h>
	#define err_scope(...) __VA_ARGS__
#else
	#define err_scope(...)
#endif // CHESS_VERBOSE

#define err(do_after, msg, ...) err_scope( \
		printf("\033[31m[DEBUG]\033[0m Error: %s: %s: %d: %s: ", __FILE__, __PRETTY_FUNCTION__, __LINE__, msg); \
		printf("\033[32m"); \
		__VA_ARGS__; \
		printf("\033[0m"); \
		printf("\n"); \
	) \
	do_after; \

#define err_var(fmt, name, ...) err_scope( \
	printf("%s = ", #name); \
	printf("'"); \
	printf(fmt, ##__VA_ARGS__, name); \
	printf("'"); \
	printf(": "); \
)
#define err_varn(fmt, name, ...) err_scope(err_var(fmt, name, ##__VA_ARGS__); printf("\n");)
#define err_pause(...) err_scope(do {__VA_ARGS__} while (getchar() != '\n'))



err_scope(
void Bitboard_Print(uint64_t bitboardSet);
)
#define Bitboard_PrintType(bitboardSet, typeToken) err_scope( \
	printf("%s:\n", #typeToken); \
	Bitboard_Print(MASK_TYPE(bitboardSet, typeToken)); \
)
#define Bitboard_PrintSide(bitboardSet, sideToken) err_scope( \
	printf("%s:\n", #sideToken); \
	Bitboard_Print(MASK_SIDE(bitboardSet, sideToken)); \
)
#define Bitboard_PrintPiece(bitboardSet, sideToken, typeToken) err_scope( \
	printf("%s.%s:\n", #sideToken, #typeToken); \
	Bitboard_Print(MASK_PIECE(bitboardSet, sideToken, typeToken)); \
)
#define Bitboard_PrintMask(bitboardSet) err_scope( \
	printf("%s:\n", #bitboardSet); \
	Bitboard_Print(bitboardSet); \
)


err_scope(
void Bitboard_Print(uint64_t bitboardSet)
{
		for (uint8_t row = 0; row < 8; row++)
		{
			for (uint8_t col = 0; col < 8; col++)
			{
				uint8_t index = row * 8 + col;
				StringIndex token = MASK_INDEX(index) & bitboardSet
					? STRING("x") : STRING("-");
				printf("%3.*s", token.length, token.chars);
			}
			printf("\n");
		}
}
)


