#include "chess_typedefs.h.c"
#include "chess_helpers.h.c"
#include "../math.h.c"


#include <stdint.h>
#include <stdarg.h>

#pragma once

#define MASK_INDEX_0 1ULL
#define MASK_EMPTY 0ULL
#define MASK_INDEX(index) (MASK_INDEX_0 << (index))
#define MASK_INDEX_SIGNED(index) (index < 0 ? MASK_EMPTY : MASK_INDEX(index))
#define MASK_INDEX_UNBOUND(index) (IF_IN_RANGE(index, 0, 63, MASK_INDEX(index), MASK_EMPTY))


#define MASK_ALL(bitboardSet) (MASK_SIDE(bitboardSet, BLACK) | MASK_SIDE(bitboardSet, WHITE))
#define MASK_PIECE(bitboardSet, sideToken, typeToken) \
	Mask_Piece(bitboardSet, \
		PIECE_SIDE_INDEX_##sideToken, \
		PIECE_TYPE_INDEX_##typeToken)
#define MASK_TYPE(bitboardSet, typeToken) Mask_Type(bitboardSet, PIECE_TYPE_INDEX_##typeToken)
#define MASK_SIDE(bitboardSet, sideToken) \
	Mask_Side(bitboardSet, PIECE_SIDE_INDEX_##sideToken)
#define Mask_Piece(bitboardSet, side, type) (*bitboardSet)[side][type]
#define Mask_Type(bitboardSet, type) \
	(Mask_Piece(bitboardSet, PIECE_SIDE_INDEX_WHITE, type) | \
	 Mask_Piece(bitboardSet, PIECE_SIDE_INDEX_BLACK, type))
#define Mask_Side(bitboardSet, side) \
	(Mask_Piece(bitboardSet, side, PIECE_TYPE_INDEX_KING) | Mask_Piece(bitboardSet, side, PIECE_TYPE_INDEX_QUEEN) | \
	 Mask_Piece(bitboardSet, side, PIECE_TYPE_INDEX_BISHOP) | Mask_Piece(bitboardSet, side, PIECE_TYPE_INDEX_KNIGHT) | \
	 Mask_Piece(bitboardSet, side, PIECE_TYPE_INDEX_ROOK) | Mask_Piece(bitboardSet, side, PIECE_TYPE_INDEX_PAWN))


uint64_t Mask_Rows(uint32_t count, ...);
uint64_t Mask_Cols(uint32_t count, ...);
#define Mask_GetRow(row, mask) (MASK_ROW(row) & mask)
#define MASK_ROW(row) IF_IN_RANGE(row, 0, 7, MASK_ROW_0 << ((row) * 8), MASK_EMPTY)
#define MASK_COL(col) IF_IN_RANGE(col, 0, 7, MASK_COL_0 << (col), MASK_EMPTY)
#define MASK_ROW_0 0xFFULL
#define MASK_COL_0 0x0101010101010101ULL
#define MASK_CHECKER 0x55AA55AA55AA55AAULL
		
#define MASK_INDEX_VA_ARGS(funcName, maskIndexFunc) \
uint64_t funcName(uint32_t count, ...) \
{ \
	va_list args; \
	va_start(args, count); \
	uint64_t mask_result = MASK_EMPTY; \
	for (uint32_t i = 0; i < count; i++) \
	{ \
		uint32_t index = va_arg(args, uint32_t); \
		mask_result |= maskIndexFunc(index); \
	} \
	return mask_result; \
}

uint64_t Direct_Rows(PieceSideIndex side, uint64_t mask);
#define DIRECT_OFFSET_SIGN(side) IF_SIDE(side, 0, 1, -1)
#define Index_AsRow(index) (uint8_t)floorf((index) / 8.f)
#define Index_AsCol(index) ((index) % 8)

#define BitScanForward(x) __builtin_ctzll(x)
#define BitScanReverse(x) (63 - __builtin_clzll(x))

#define OFFSET_FORWARD(side) DIRECT_OFFSET_SIGN(side) * OFFSET_UP
#define OFFSET_BACKWARD(side) DIRECT_OFFSET_SIGN(side) * OFFSET_DOWN
#define OFFSET_UP -8
#define OFFSET_DOWN 8
#define OFFSET_LEFT -1
#define OFFSET_RIGHT 1

MASK_INDEX_VA_ARGS(Mask_Rows, MASK_ROW);
MASK_INDEX_VA_ARGS(Mask_Cols, MASK_COL);

uint64_t Direct_Rows(PieceSideIndex side, uint64_t mask)
{
	if (side != PIECE_SIDE_INDEX_BLACK) return mask;
	uint64_t mask_result = MASK_EMPTY;
	for (uint8_t i = 0; i < 8; i++)
	{
		mask_result |= (Mask_GetRow(i, mask) >> (i * 8)) << ((7 - i) * 8);
	}
	return mask_result;
}
