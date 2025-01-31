#include "chess_typedefs.h.c"
#include "chess_mask.h.c"

#include <stdint.h>

#pragma once

#define Piece_HasSideIndex(bitboardSet, index, side) (Piece_IsSideIndex(side) \
	? Piece_GetSideIndex(bitboardSet, index) == side : false)
#define Piece_HasTypeIndex(bitboardSet, index, type) (Piece_IsTypeIndex(type) \
	? Piece_GetTypeIndex(bitboardSet, index) == type : false)
PieceSideIndex Piece_GetSideIndex(Bitboards_All* bitboardSet, uint8_t index);
PieceTypeIndex Piece_GetTypeIndex(Bitboards_All* bitboardSet, uint8_t index);

#define Piece_IsSideIndex(side) ((side >= 0 && side < PIECE_SIDE_INDEX_COUNT) || side == PIECE_SIDE_INDEX_NONE)
#define Piece_IsTypeIndex(type) ((type >= 0 && type < PIECE_TYPE_INDEX_COUNT) || type == PIECE_TYPE_INDEX_NONE)

Piece Piece_New(Bitboards_All* bitboardSet, uint8_t index);
Move Move_New(Bitboards_All* bitboardSet, uint8_t srcIndex, uint8_t dstIndex);


Piece Piece_New(Bitboards_All* bitboardSet, uint8_t index)
{
	return (Piece){index,
		Piece_GetSideIndex(bitboardSet, index),
		Piece_GetTypeIndex(bitboardSet, index)};
}

Move Move_New(Bitboards_All* bitboardSet, uint8_t srcIndex, uint8_t dstIndex)
{
	return (Move){
		Piece_New(bitboardSet, srcIndex),
		Piece_New(bitboardSet, dstIndex),
		PIECE_TYPE_INDEX_NONE};
}

PieceSideIndex Piece_GetSideIndex(Bitboards_All* bitboardSet, uint8_t index)
{
	uint64_t mask = MASK_INDEX(index);
	if (mask & MASK_SIDE(bitboardSet, WHITE)) return PIECE_SIDE_INDEX_WHITE;
	if (mask & MASK_SIDE(bitboardSet, BLACK)) return PIECE_SIDE_INDEX_BLACK;
	return PIECE_SIDE_INDEX_NONE;
}


PieceTypeIndex Piece_GetTypeIndex(Bitboards_All* bitboardSet, uint8_t index)
{
	static const PieceTypeIndex types[PIECE_TYPE_INDEX_COUNT] = {
		PIECE_TYPE_INDEX_KING,
		PIECE_TYPE_INDEX_QUEEN,
		PIECE_TYPE_INDEX_BISHOP,
		PIECE_TYPE_INDEX_KNIGHT,
		PIECE_TYPE_INDEX_ROOK,
		PIECE_TYPE_INDEX_PAWN
	};
	uint64_t mask = MASK_INDEX(index);
	for (uint8_t i = 0; i < PIECE_TYPE_INDEX_COUNT; i++)
	{
		if (mask & Mask_Type(bitboardSet, types[i])) return i;
	}
	return PIECE_TYPE_INDEX_NONE;
}


