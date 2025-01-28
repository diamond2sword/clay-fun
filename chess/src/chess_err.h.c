#ifdef CHESS_VERBOSE
	#include <stdio.h>
#endif // CHESS_VERBOSE

#pragma once

// Logging
#ifdef CHESS_VERBOSE
	#define err(do_after, msg, ...) { \
		printf("\033[31m[DEBUG]\033[0m Error: %s: %s: %d: %s: ", __FILE__, __PRETTY_FUNCTION__, __LINE__, msg); \
		printf("\033[32m"); \
		__VA_ARGS__; \
		printf("\033[0m"); \
		printf("\n"); \
		do_after; \
	} 
	#define err_var(fmt, name, ...) { \
		printf("%s = ", #name); \
		printf("'"); \
		printf(fmt, ##__VA_ARGS__, name); \
		printf("'"); \
		printf(": "); \
	}
	#define err_varn(fmt, name, ...) {err_var(fmt, name, ##__VA_ARGS__); printf("\n");}
	#define err_pause(...) do {__VA_ARGS__} while (getchar() != '\n') 
#else
	#define err(do_after, ...) do_after;
	#define err_var(...)
	#define err_varn(...)
	#define err_pause(...)
#endif // CHESS_VERBOSE


#ifdef CHESS_VERBOSE
	void Bitboard_Print(uint64_t bitboardSet);
	#define Bitboard_PrintType(bitboardSet, typeToken) { \
		printf("%s:\n", #typeToken); \
		Bitboard_Print(MASK_TYPE(bitboardSet, typeToken)); \
	}
	#define Bitboard_PrintSide(bitboardSet, sideToken) { \
		printf("%s:\n", #sideToken); \
		Bitboard_Print(MASK_SIDE(bitboardSet, sideToken)); \
	}
	#define Bitboard_PrintPiece(bitboardSet, sideToken, typeToken) { \
		printf("%s.%s:\n", #sideToken, #typeToken); \
		Bitboard_Print(MASK_PIECE(bitboardSet, sideToken, typeToken)); \
	}
#else
	#define Bitboard_PrintType(...)
	#define Bitboard_PrintSide(...) 
	#define Bitboard_PrintPiece(...)
#endif // CHESS_VERBOSE


#ifdef CHESS_VERBOSE
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
#endif // CHESS_VERBOSE



