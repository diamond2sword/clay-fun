#include <stdint.h>
#include <stdbool.h>

#pragma once

typedef enum PieceTypeIndex : uint8_t {
	PIECE_TYPE_INDEX_KING,
	PIECE_TYPE_INDEX_QUEEN,
	PIECE_TYPE_INDEX_BISHOP,
	PIECE_TYPE_INDEX_KNIGHT,
	PIECE_TYPE_INDEX_ROOK,
	PIECE_TYPE_INDEX_PAWN,
	PIECE_TYPE_INDEX_COUNT,
	PIECE_TYPE_INDEX_NONE
} PieceTypeIndex;

typedef enum PieceSideIndex : uint8_t {
	PIECE_SIDE_INDEX_WHITE,
	PIECE_SIDE_INDEX_BLACK,
	PIECE_SIDE_INDEX_COUNT,
	PIECE_SIDE_INDEX_NONE
} PieceSideIndex;

typedef uint64_t Bitboards_Side[PIECE_TYPE_INDEX_COUNT];
typedef Bitboards_Side Bitboards_All[PIECE_SIDE_INDEX_COUNT];

typedef struct Piece {
	uint8_t index;
	PieceSideIndex side;
	PieceTypeIndex type;
} Piece;

typedef struct Move {
	Piece src;
	Piece dst;
} Move;

typedef enum CastlingSide : uint8_t {
	CASTLING_SIDE_QUEEN,
	CASTLING_SIDE_KING,
	CASTLING_SIDE_COUNT,
	CASTLING_SIDE_NONE
} CastlingSide;
typedef bool CastlingRights[PIECE_SIDE_INDEX_COUNT][CASTLING_SIDE_COUNT];
