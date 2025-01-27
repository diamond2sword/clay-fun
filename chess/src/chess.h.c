#define CHESS_IMPLEMENTATION
#define CHESS_VERBOSE

#include <stdint.h>
#include <stdbool.h>

#include "math.h.c"
#include "chess_err.h.c"
#include "chess_helpers.h.c"

#ifndef CHESS_HEADER
#define CHESS_HEADER

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

typedef enum PiecePromotionIndex : uint8_t {
	PIECE_PROMOTION_INDEX_QUEEN,
	PIECE_PROMOTION_INDEX_ROOK,
	PIECE_PROMOTION_INDEX_BISHOP,
	PIECE_PROMOTION_INDEX_KNIGHT,
	PIECE_PROMOTION_INDEX_COUNT,
	PIECE_PROMOTION_INDEX_NONE,
} PiecePromotionIndex;

#define PIECE_RAY_INDEX_CARDINALS \
	PIECE_RAY_INDEX_NORTH, \
	PIECE_RAY_INDEX_SOUTH, \
	PIECE_RAY_INDEX_WEST, \
	PIECE_RAY_INDEX_EAST


#define PIECE_RAY_INDEX_INTERCARDINALS \
	PIECE_RAY_INDEX_NORTHWEST, \
	PIECE_RAY_INDEX_NORTHEAST, \
	PIECE_RAY_INDEX_SOUTHWEST, \
	PIECE_RAY_INDEX_SOUTHEAST

#define PIECE_RAY_INDEX_OCTANTS \
	PIECE_RAY_INDEX_CARDINALS, \
	PIECE_RAY_INDEX_INTERCARDINALS \

typedef enum PieceRayIndex : uint8_t {
	PIECE_RAY_INDEX_OCTANTS,
	PIECE_RAY_INDEX_COUNT,
	PIECE_RAY_INDEX_NONE,
} PieceRayIndex;

typedef enum CastlingSide : uint8_t {
	CASTLING_SIDE_QUEEN,
	CASTLING_SIDE_KING,
	CASTLING_SIDE_COUNT,
	CASTLING_SIDE_NONE
} CastlingSide;
typedef bool CastlingRights[PIECE_SIDE_INDEX_COUNT][CASTLING_SIDE_COUNT];



int ChessInit_FromString(StringIndex fen);
#define ChessInit_Default() ChessInit_FromString(STRING("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));


void Game_MakeMove(Bitboards_All bitboards, Move move, PieceSideIndex* side_active, int8_t* index_enPassantTarget, PieceTypeIndex* type_promotion, CastlingRights castlingRights, uint16_t* fullMoveCounter, uint16_t* halfMoveCounter);
#define BitboardSet_PutAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Put(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)
#define BitboardSet_Put(bitboardSet, index, side, type) \
	bitboardSet[side][type] |= MASK_INDEX(index)
#define BITBOARD_SET_PUT(bitboardSet, index, sideToken, typeToken) \
	BitboardSet_Put(bitboardSet, index, PIECE_SIDE_INDEX_##sideToken, PIECE_TYPE_INDEX_##typeToken) 

#define BitboardSet_RemoveAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Remove(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)
#define BitboardSet_Remove(bitboardSet, index, side, type) bitboardSet[side][type] &= ~MASK_INDEX(index)
#define BITBOARD_SET_REMOVE(bitboardSet, index, sideToken, typeToken) \
	BitboardSet_Remove(bitboardSet, index, PIECE_SIDE_INDEX_##sideToken, PIECE_TYPE_INDEX_##typeToken) 

uint64_t Mask_Attacks_Sliding(uint64_t mask_occupied, uint8_t index, int32_t ray_count, ...);
uint64_t Mask_Attacks(Bitboards_All bitboardSet, Piece piece);
uint64_t Mask_Movables(Bitboards_All bitboardSet, PieceSideIndex side, uint64_t (*maskMovesFunc)(Bitboards_All, Piece));
uint64_t Mask_PromotionOptions(Bitboards_All bitboardSet, uint64_t index, Move move);


Piece Piece_Get(Bitboards_All bitboardSet, uint8_t index);
Move Move_New(Bitboards_All bitboardSet, uint8_t srcIndex, uint8_t dstIndex);
void Precompute_SlidingPiece_RayIndexes();


// Public API for BIT_BOARD



#define Piece_HasSideIndex(bitboardSet, index, side) (Piece_IsSideIndex(side) \
	? Piece_GetSideIndex(bitboardSet, index) == side : false)
#define Piece_HasTypeIndex(bitboardSet, index, type) (Piece_IsTypeIndex(type) \
	? Piece_GetTypeIndex(bitboardSet, index) == type : false)
PieceSideIndex Piece_GetSideIndex(Bitboards_All bitboardSet, uint8_t index);
PieceTypeIndex Piece_GetTypeIndex(Bitboards_All bitboardSet, uint8_t index);

#define Piece_IsSideIndex(side) ((side >= 0 && side < PIECE_SIDE_INDEX_COUNT) || side == PIECE_SIDE_INDEX_NONE)
#define Piece_IsTypeIndex(type) ((type >= 0 && type < PIECE_TYPE_INDEX_COUNT) || type == PIECE_TYPE_INDEX_NONE)

PieceTypeIndex PieceTypeIndex_FromChar(char c);
#define PieceSideIndex_FromChar(c) \
	(PieceTypeIndex_FromChar(c) == PIECE_TYPE_INDEX_NONE \
	? PIECE_SIDE_INDEX_NONE : isCapitalChar(c) \
	? PIECE_SIDE_INDEX_WHITE : PIECE_SIDE_INDEX_BLACK)

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
#define Mask_Piece(bitboardSet, side, type) bitboardSet[side][type]
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
#define SIDE_NEGATE(side) IF_SIDE(side, side, PIECE_SIDE_INDEX_BLACK, PIECE_SIDE_INDEX_WHITE)

#define IF_SIDE(side, forNone, forWhite, forBlack) ((side) == PIECE_SIDE_INDEX_NONE ? (forNone) : (side) == PIECE_SIDE_INDEX_WHITE ? (forWhite) : (forBlack))
#define IF_IN_RANGE(x, min, max, forTrue, forFalse) (IN_RANGE((x), (min), (max)) ? (forTrue) : (forFalse))
#define IN_RANGE(x, min, max) ((x) >= (min) && (x) <= (max))
	
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

// Strings
#define __STRING_LENGTH(s) ((sizeof(s) / sizeof((s)[0])) - sizeof((s)[0]))

#define __ENSURE_STRING_LITERAL(x) ("" x "")

// Note: If an error led you here, it's because STRING can only be used with string literals, i.e. STRING("SomeString") and not STRING(yourString)
#define STRING(string) (__INIT(StringIndex) { .chars = (string), .length = __STRING_LENGTH(__ENSURE_STRING_LITERAL(string)) })

#define STRING_CONST(string) { .chars = (string), .length = __STRING_LENGTH(__ENSURE_STRING_LITERAL(string)) }

#define __INIT(type) (type)




#endif // CHESS_HEADER



#ifdef CHESS_IMPLEMENTATION
#undef CHESS_IMPLEMENTATION

Bitboards_All BITBOARD_SET;
PieceSideIndex ACTIVE_SIDE = PIECE_SIDE_INDEX_WHITE;
CastlingRights CASTLING_RIGHTS;
int8_t EN_PASSANT_TARGET_INDEX = -1;
uint16_t HALF_MOVE_COUNTER = 0;
uint16_t FULL_MOVE_COUNTER = 1;


uint64_t MAP_DIRECTION_TO_RAY_MASK[PIECE_RAY_INDEX_COUNT][64];
const uint8_t MAP_RAY_TO_OFFSET[PIECE_RAY_INDEX_COUNT] = {
	[PIECE_RAY_INDEX_NORTH] = OFFSET_UP,
	[PIECE_RAY_INDEX_SOUTH] = OFFSET_DOWN,
	[PIECE_RAY_INDEX_WEST] = OFFSET_LEFT,
	[PIECE_RAY_INDEX_EAST] = OFFSET_RIGHT,
	[PIECE_RAY_INDEX_NORTHWEST] = OFFSET_UP + OFFSET_LEFT,
	[PIECE_RAY_INDEX_NORTHEAST] = OFFSET_UP + OFFSET_RIGHT,
	[PIECE_RAY_INDEX_SOUTHWEST] = OFFSET_DOWN + OFFSET_LEFT,
	[PIECE_RAY_INDEX_SOUTHEAST] = OFFSET_DOWN + OFFSET_RIGHT,
};
const bool MAP_RAY_MUST_BIT_SCAN_FORWARD[PIECE_RAY_INDEX_COUNT] = {
	[PIECE_RAY_INDEX_NORTH] = false,
	[PIECE_RAY_INDEX_SOUTH] = true,
	[PIECE_RAY_INDEX_WEST] = false,
	[PIECE_RAY_INDEX_EAST] = true,
	[PIECE_RAY_INDEX_NORTHWEST] = false,
	[PIECE_RAY_INDEX_NORTHEAST] = false,
	[PIECE_RAY_INDEX_SOUTHWEST] = true,
	[PIECE_RAY_INDEX_SOUTHEAST] = true,
};

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

Piece Piece_Get(Bitboards_All bitboardSet, uint8_t index)
{
	return (Piece){index,
		Piece_GetSideIndex(bitboardSet, index),
		Piece_GetTypeIndex(bitboardSet, index)};
}

Move Move_New(Bitboards_All bitboardSet, uint8_t srcIndex, uint8_t dstIndex)
{
	return (Move){
		Piece_Get(bitboardSet, srcIndex),
		Piece_Get(bitboardSet, dstIndex)};
}

MASK_INDEX_VA_ARGS(Mask_Rows, MASK_ROW);
MASK_INDEX_VA_ARGS(Mask_Cols, MASK_COL);

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

uint64_t Mask_Attacks_OnEnemyKing(Bitboards_All bitboardSet, Piece piece)
{
	return Mask_Piece(bitboardSet, SIDE_NEGATE(piece.side), PIECE_TYPE_INDEX_KING) & Mask_Attacks(bitboardSet, piece);
}

uint64_t Mask_Movables(Bitboards_All bitboardSet, PieceSideIndex side, uint64_t (*maskMovesFunc)(Bitboards_All, Piece))
{
	uint64_t mask_result = MASK_EMPTY;
	for (uint8_t i = 0; i < 64; i++)
	{
		Piece piece = Piece_Get(bitboardSet, i);
		if (piece.side != side)
		{
			continue;
		}
		if (maskMovesFunc(bitboardSet, piece))
		{
			mask_result |= MASK_INDEX(i);
		}
	}
	return mask_result;
}

void Precompute_SlidingPiece_RayIndexes()
{
	for (uint8_t ray = 0; ray < PIECE_RAY_INDEX_COUNT; ray++)
	{
		int8_t offset = MAP_RAY_TO_OFFSET[ray];
		int8_t offset_col = Index_AsCol(OFFSET_DOWN + OFFSET_RIGHT + offset) - 1;
		for (uint8_t i = 0; i < 64; i++)
		{
			uint64_t mask = MASK_EMPTY;
			int8_t index_this = i;
			int8_t col_this = Index_AsCol(i);
			while (IN_RANGE(index_this, 0, 63) && IN_RANGE(col_this, 0, 7))
			{
				mask |= MASK_INDEX(index_this);
				index_this += offset;
				col_this += offset_col;
			}
			{
				MAP_DIRECTION_TO_RAY_MASK[ray][i] = mask & ~MASK_INDEX(i);
			}
		}
	}
}

uint64_t Mask_Attacks_Sliding(uint64_t mask_occupied, uint8_t index, int32_t ray_count, ...)
{
	va_list args;
	va_start(args, ray_count);
	uint64_t mask_attacks = MASK_EMPTY;
	for (uint8_t i = 0; i < ray_count; i++)
	{
		PieceRayIndex ray = va_arg(args, uint32_t);
		uint64_t mask_rays_this	= MAP_DIRECTION_TO_RAY_MASK[ray][index];
		mask_attacks |= mask_rays_this;
		uint64_t mask_matches = mask_rays_this & mask_occupied;
		if (mask_matches)
		{
			uint8_t index_blocker = MAP_RAY_MUST_BIT_SCAN_FORWARD[ray]  ? BitScanForward(mask_matches) : BitScanReverse(mask_matches);
			mask_attacks &= ~MAP_DIRECTION_TO_RAY_MASK[ray][index_blocker];
		}
	}
	return mask_attacks;
}

uint64_t Mask_Attacks_KingIsSafeAfter(Bitboards_All bitboardSet, Piece piece)
{
	PieceSideIndex side_inactive = SIDE_NEGATE(piece.side);
	uint64_t mask_attacks = Mask_Attacks(bitboardSet, piece);

	Bitboards_All bitboardSet_temp;
	CastlingRights castlingRights;
	for (uint8_t index_dst = 0; index_dst < 64; index_dst++)
	{
		uint64_t mask_dst = MASK_INDEX(index_dst);
		if (mask_dst & mask_attacks)
		{
			Bitboards_All_Copy(bitboardSet, bitboardSet_temp);
			PieceSideIndex side_active = ACTIVE_SIDE;
			int8_t index_enPassantTarget = EN_PASSANT_TARGET_INDEX;
			PieceTypeIndex type_promotion = PIECE_TYPE_INDEX_NONE;
			CastlingRights_Copy(CASTLING_RIGHTS, castlingRights);
			uint16_t halfMoveCounter = HALF_MOVE_COUNTER;
			uint16_t fullMoveCounter = FULL_MOVE_COUNTER;
			Game_MakeMove(bitboardSet_temp, Move_New(bitboardSet_temp, piece.index, index_dst), &side_active, &index_enPassantTarget, &type_promotion, castlingRights, &fullMoveCounter, &halfMoveCounter);
			{
				// simulate king being two if in castling
				if (piece.type == PIECE_TYPE_INDEX_KING)
				{
					if (index_dst == piece.index + OFFSET_LEFT * 2)
					{
						BitboardSet_Put(bitboardSet_temp, piece.index + OFFSET_LEFT, piece.side, piece.type);
						BitboardSet_Put(bitboardSet_temp, piece.index, piece.side, piece.type);
					} else if (index_dst == piece.index + OFFSET_RIGHT * 2)
					{
						BitboardSet_Put(bitboardSet_temp, piece.index + OFFSET_RIGHT, piece.side, piece.type);
						BitboardSet_Put(bitboardSet_temp, piece.index, piece.side, piece.type);
					}
				}
			}
			//if (Mask_Attacks_InCheck(bitboardSet_temp, side_inactive))
			if (Mask_Movables(bitboardSet_temp, side_inactive, Mask_Attacks_OnEnemyKing))
			{
				mask_attacks &= ~mask_dst;
			}
		}
	}
	return mask_attacks;
}

uint64_t Mask_Attacks(Bitboards_All bitboardSet, Piece piece)
{
	uint8_t col = Index_AsCol(piece.index);
	uint8_t row = Index_AsRow(piece.index);
	PieceSideIndex inactiveSide = SIDE_NEGATE(piece.side);
	uint64_t mask_occupied = MASK_ALL(bitboardSet);
	uint64_t mask_enemy = Mask_Side(bitboardSet, inactiveSide);
	switch (piece.type) {
	case PIECE_TYPE_INDEX_PAWN: {
		int8_t offsetSign = DIRECT_OFFSET_SIGN(piece.side);
		int8_t offsetForward = OFFSET_FORWARD(piece.side);
		uint64_t mask_enPassantTarget = MASK_INDEX_SIGNED(EN_PASSANT_TARGET_INDEX);
		uint64_t mask_unmovePawnsRow = Direct_Rows(piece.side, MASK_ROW(6));
		uint64_t mask_asUnmovedPawn = MASK_INDEX(piece.index) & mask_unmovePawnsRow;
		uint64_t mask_singleForward = MASK_INDEX_UNBOUND(piece.index + offsetForward) & ~mask_occupied;
		uint64_t mask_captures = Mask_Cols(2, col - 1, col + 1) & MASK_ROW(row - offsetSign) & (mask_enemy | mask_enPassantTarget);
		uint64_t mask_doubleForward = (mask_asUnmovedPawn && mask_singleForward) ? MASK_INDEX(piece.index + offsetForward * 2) & ~mask_occupied : MASK_EMPTY;
		return mask_singleForward | mask_doubleForward | mask_captures;
	} case PIECE_TYPE_INDEX_ROOK: {
		return Mask_Attacks_Sliding(mask_occupied, piece.index, 4, PIECE_RAY_INDEX_CARDINALS)
			& (mask_enemy | ~mask_occupied);
	} case PIECE_TYPE_INDEX_BISHOP: {
		return Mask_Attacks_Sliding(mask_occupied, piece.index, 4, PIECE_RAY_INDEX_INTERCARDINALS)
			& (mask_enemy | ~mask_occupied);
	} case PIECE_TYPE_INDEX_QUEEN: {
		return Mask_Attacks_Sliding(mask_occupied, piece.index, 8, PIECE_RAY_INDEX_OCTANTS)
			& (mask_enemy | ~mask_occupied);
	} case PIECE_TYPE_INDEX_KING: {
		uint64_t mask_singleStep = Mask_Rows(3, row - 1, row, row + 1) & Mask_Cols(3, col - 1, col, col + 1) & ~MASK_INDEX(row * 8 + col) & (mask_enemy | ~mask_occupied);
		uint64_t mask_singleLeft = MASK_INDEX(piece.index + OFFSET_LEFT) & ~mask_occupied;
		uint64_t mask_singleRight = MASK_INDEX(piece.index + OFFSET_RIGHT) & ~mask_occupied;
		uint64_t mask_singleRight_rook = MASK_INDEX(piece.index + OFFSET_LEFT * 3) & ~mask_occupied;
		uint64_t mask_castle_queenSide = (CASTLING_RIGHTS[piece.side][CASTLING_SIDE_QUEEN] && mask_singleLeft && mask_singleRight_rook) ? MASK_INDEX(piece.index + OFFSET_LEFT * 2) & ~mask_occupied : MASK_EMPTY;
		uint64_t mask_castle_kingSide = (CASTLING_RIGHTS[piece.side][CASTLING_SIDE_KING]	&& mask_singleRight) ? MASK_INDEX(piece.index + OFFSET_RIGHT * 2) & ~mask_occupied : MASK_EMPTY;
		return mask_singleStep | mask_castle_queenSide | mask_castle_kingSide;
	} case PIECE_TYPE_INDEX_KNIGHT: {
		return ((Mask_Rows(2, row - 1, row + 1) & Mask_Cols(2, col - 2 , col + 2)) | (Mask_Rows(2, row - 2, row + 2) & Mask_Cols(2, col - 1, col + 1)))
			& (mask_enemy | ~mask_occupied);
	} default: 
		return MASK_EMPTY;
	}
}

uint64_t Mask_PromotionOptions(Bitboards_All bitboardSet, uint64_t index, Move move)
{
	uint64_t mask_options = Direct_Rows(move.src.side, Mask_Rows(4, 0, 1, 2, 3) & MASK_COL(Index_AsCol(move.dst.index)));
	return mask_options;
}

void Game_MakeMove(Bitboards_All bitboardSet, Move move, PieceSideIndex* side_active, int8_t* index_enPassantTarget, PieceTypeIndex* type_promotion, CastlingRights castlingRights, uint16_t* fullMoveCounter, uint16_t* halfMoveCounter)
{
	bool enPassantTarget_isSet_thisFrame = false;
	{
		// make move
		BitboardSet_RemoveAttrs(bitboardSet, move, src.index, src.side, src.type);
		BitboardSet_RemoveAttrs(bitboardSet, move, dst.index, dst.side, dst.type);
		BitboardSet_PutAttrs(bitboardSet, move, dst.index, src.side, src.type);
	}
	{
		// make special move features
		if (move.src.type == PIECE_TYPE_INDEX_PAWN)
		{
			// pawn special features
			// en passant pawn addition
			if (move.dst.index == move.src.index + 2 * OFFSET_FORWARD(move.src.side))
			{
				enPassantTarget_isSet_thisFrame = true;
				*index_enPassantTarget = move.src.index + OFFSET_FORWARD(move.src.side);
			}
			// en passant pawn deletion
			else if (move.dst.index == *index_enPassantTarget)
			{
				Piece enPassantPawn = Piece_Get(bitboardSet, move.dst.index + OFFSET_BACKWARD(move.src.side));
				BitboardSet_RemoveAttrs(bitboardSet, enPassantPawn, index, side, type);
			}
			// pawn promotion
			else if (MASK_INDEX(move.dst.index) & Direct_Rows(move.src.side, MASK_ROW_0))
			{
				BitboardSet_RemoveAttrs(bitboardSet, move, dst.index, src.side, src.type);
				BitboardSet_Put(bitboardSet, move.dst.index, move.src.side, *type_promotion);
				*type_promotion = PIECE_TYPE_INDEX_NONE;
			}
		}
		else if (move.src.type == PIECE_TYPE_INDEX_KING)
		{
			// untoggle castling rights
			// castle queen side
			if (castlingRights[move.src.side][CASTLING_SIDE_QUEEN]
			&& move.dst.index == move.src.index + OFFSET_LEFT * 2)
			{
				BitboardSet_Remove(bitboardSet, move.src.index + OFFSET_LEFT * 4, move.src.side, PIECE_TYPE_INDEX_ROOK);
				BitboardSet_Put(bitboardSet, move.src.index + OFFSET_LEFT, move.src.side, PIECE_TYPE_INDEX_ROOK);
			}
			// castle king side
			if (castlingRights[move.src.side][CASTLING_SIDE_KING]
			&& move.dst.index == move.src.index + OFFSET_RIGHT * 2)
			{
				BitboardSet_Remove(bitboardSet, move.src.index + OFFSET_RIGHT * 3, move.src.side, PIECE_TYPE_INDEX_ROOK);
				BitboardSet_Put(bitboardSet, move.src.index + OFFSET_RIGHT, move.src.side, PIECE_TYPE_INDEX_ROOK);
			}
			castlingRights[move.src.side][CASTLING_SIDE_QUEEN] = false;
			castlingRights[move.src.side][CASTLING_SIDE_KING] = false;
		}
		else if (move.src.type == PIECE_TYPE_INDEX_ROOK)
		{
			if (MASK_INDEX(move.src.index) & Direct_Rows(move.src.side, MASK_INDEX(OFFSET_DOWN * 7)))
			{
				castlingRights[move.src.side][CASTLING_SIDE_QUEEN] = false;
			} 
			else if (MASK_INDEX(move.src.index) & Direct_Rows(move.src.side, MASK_INDEX(OFFSET_DOWN * 7 + OFFSET_RIGHT * 7)))
			{
				castlingRights[move.src.side][CASTLING_SIDE_KING] = false;
			}
		}
	}
	{
		// reset en passant index after a move
		if (!enPassantTarget_isSet_thisFrame)
		{
			*index_enPassantTarget = -1;
		}
	}
	{
		// full moves increment
		if (move.src.side == PIECE_SIDE_INDEX_BLACK)
		{
			(*fullMoveCounter)++;
		}
	}
	{
		// half moves
		if (move.src.type == PIECE_TYPE_INDEX_PAWN
		|| move.dst.type != PIECE_TYPE_INDEX_NONE)
		{
			*halfMoveCounter = 0;
		}
		else
		{
			(*halfMoveCounter)++;
		}
	}
	{
		// Switch side
		*side_active = SIDE_NEGATE(*side_active);
	}
}

PieceSideIndex Piece_GetSideIndex(Bitboards_All bitboardSet, uint8_t index)
{
	uint64_t mask = MASK_INDEX(index);
	if (mask & MASK_SIDE(bitboardSet, WHITE)) return PIECE_SIDE_INDEX_WHITE;
	if (mask & MASK_SIDE(bitboardSet, BLACK)) return PIECE_SIDE_INDEX_BLACK;
	return PIECE_SIDE_INDEX_NONE;
}


PieceTypeIndex Piece_GetTypeIndex(Bitboards_All bitboardSet, uint8_t index)
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


PieceTypeIndex PieceTypeIndex_FromChar(char c)
{
	static const char map_charToIndex[PIECE_TYPE_INDEX_COUNT] = {
		ArrayPair(PIECE_TYPE_INDEX_KING, 'k'),
		ArrayPair(PIECE_TYPE_INDEX_QUEEN, 'q'),
		ArrayPair(PIECE_TYPE_INDEX_BISHOP, 'b'),
		ArrayPair(PIECE_TYPE_INDEX_KNIGHT, 'n'),
		ArrayPair(PIECE_TYPE_INDEX_ROOK, 'r'),
		ArrayPair(PIECE_TYPE_INDEX_PAWN, 'p')
	};
	c = lowerChar(c);
	for (uint8_t i = 0; i < PIECE_TYPE_INDEX_COUNT; i++)
		if (c == map_charToIndex[i]) return i;
	return PIECE_TYPE_INDEX_NONE;
}



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

int ChessInit_FromString(StringIndex fen)
{
	
	StringIndex fieldIndexes[6] = {0};
	
	// FEN
	{
		uint8_t n_fields = 0, start_i = 0,  c = fen.chars[0], prev_c = 0;
		if (c == ' ') {
			err(return 1, "FEN can't have a leading space", {err_var("%c", c); err_var("%.*s", fen.chars, fen.length)});
		} 
		for (uint8_t i = 0; i < fen.length; i++, prev_c = c, c = fen.chars[i])
		{
			if (prev_c == ' ') start_i = i;
			if (c == ' ') {
				if (prev_c == ' ') {
					err(return 1, "double space in FEN", {err_var("%c", prev_c); err_var("%c", prev_c); err_var("%.*s", fen.chars, fen.length)});
				}
				continue;
			}
			if (i + 1 == fen.length || fen.chars[i + 1] == ' ') {
				fieldIndexes[n_fields] = (StringIndex){fen.chars + start_i, i - start_i + 1u};
				n_fields++;
				if (n_fields == 6) break;
			}
		}
		if (n_fields < 6) {
			err(return 1, "you have to provide for the 6 fields of FEN", {err_var("%d", n_fields); err_var("%.*s", fen.chars, fen.length)});
		}
	}
	
	// FIELD 0
	{
		//reset
		for (uint8_t i = 0; i < PIECE_SIDE_INDEX_COUNT; i++) 
		for (uint8_t j = 0; j < PIECE_TYPE_INDEX_COUNT; j++) 
		BITBOARD_SET[i][j] = MASK_EMPTY;

		const StringIndex field = fieldIndexes[0];

		for (uint8_t i = 0, row = 0, col = 0; i < field.length; i++)
		{
			const char c = field.chars[i];
			if (c == '/') {
				row++; col = 0;
				continue; 
			}
			if (c >= '1' && c <= '8') {
				col += c - '1' + 1;
				continue;
			}
			if (PieceTypeIndex_FromChar(c) == PIECE_TYPE_INDEX_NONE) {
				err(return 1, "not a char for piece type", err_var("%c", c));
			}
			BitboardSet_Put(BITBOARD_SET, row * 8 + col, PieceSideIndex_FromChar(c), PieceTypeIndex_FromChar(c));
			col++;

			if (i + 1 == field.length || field.chars[i + 1] == '/') {
				if (col != 8) {
					err(return 1, "cols can only be exactly 8 before next row", {err_var("%d", col); err_var("%d", row);});
				}
			}
		}
	}
	
	// FIELD 1
	{
		const StringIndex field = fieldIndexes[1];
		if (field.length > 1) {
			err(return 1, "the field must be only 1 char", err_var("%.*s", field.chars, field.length));
		}
		const char c = field.chars[0];
		if (!ContainsChar(c, STRING("wb"))) {
			err(return 1, "not a char for piece color", err_var("%c", c));
		}
		//ACTIVE_COLOR = c == 'w' ? PIECE_COLOR_WHITE : PIECE_COLOR_BLACK;
		ACTIVE_SIDE = c == 'w' ? PIECE_SIDE_INDEX_WHITE : PIECE_SIDE_INDEX_BLACK;
	}

	// FIELD 2
	{
		const StringIndex field = fieldIndexes[2];
		if (ContainsChar('-', field)) {
			if (field.length != 1) {
				err(return 1, "It must only contain '-' if it has the char '-'", err_var("%.*s", field.chars, field.length));
			}
		} else {
			const StringIndex castlingSides = STRING("KQkq");
			for (uint8_t i = 0; i < field.length; i++)
			{
				const char c = field.chars[i];
				if (!ContainsChar(c, castlingSides)) {
					err(return 1, "chars must be one of castling sides", {err_var("%.*s", field.chars, field.length); err_var("%c", c);});
				}

				switch (c) {
				case 'K': CASTLING_RIGHTS[PIECE_SIDE_INDEX_WHITE][CASTLING_SIDE_KING] = true; break;
				case 'Q': CASTLING_RIGHTS[PIECE_SIDE_INDEX_WHITE][CASTLING_SIDE_QUEEN] = true; break;
				case 'k': CASTLING_RIGHTS[PIECE_SIDE_INDEX_BLACK][CASTLING_SIDE_KING] = true; break;
				case 'q': CASTLING_RIGHTS[PIECE_SIDE_INDEX_BLACK][CASTLING_SIDE_QUEEN] = true; break;
				}
			}
			for (uint8_t i = 0; i < castlingSides.length; i++)
			{
				if (CountChar(castlingSides.chars[i], field) > 1) {
					err(return 1, "castling side chars can only appear once", err_var("%.*s", field.chars, field.length));
				}
			}
		}
	}
	
	// FIELD 3
	{
		const StringIndex field = fieldIndexes[3];
		if (ContainsChar('-', field)) {
			if (field.length != 1) {
				err(return 1, "it must only contain '-' if it has the char '-'", err_var("%.*s", field.chars, field.length));
			}
		} else {
			if (field.length != 2) {
				err(return 1, "it requires exactly two chars to represent en passant target square", err_var("%.*s", field.chars, field.length));
			}
			const char file = field.chars[0];
			const StringIndex files = STRING("abcdefgh");
			if (!ContainsChar(file, files)) {
				err(return 1, "does not represent a file", {err_var("%c", file); err_var("%.*s", files.chars, files.length)});
			}
			const char rank = field.chars[1];
			const StringIndex ranks = STRING("12345678");
			if (!ContainsChar(rank, ranks)) {
				err(return 1, "does not represent a rank", {err_var("%c", rank); err_var("%.*s", ranks.chars, ranks.length)});
			}
			EN_PASSANT_TARGET_INDEX = file - 'a' + (7 - (rank - '1')) * 8;
		}
	}
	
	// FIELD 4 and 5
	{
		uint32_t numbers[2] = {0};
		const StringIndex digits = STRING("0123456789");
		for (uint8_t j = 0; j < 2; j++)
		{
			const StringIndex field = fieldIndexes[j + 4];
			if (field.chars[0] == '0' && field.length > 1) {
				err(return 1, "there can't be leading zeroes if it's not zero", err_var("%.*s", field.chars, field.length));
			}
			uint32_t offset = 1;
			for (uint8_t i = 0; i < field.length; i++, offset *= 10)
			{
				const char c = field.chars[field.length - i - 1];
				if (!ContainsChar(c, digits)) {
					err(return 1, "a char must be a digit", {err_var("%.*s", field.chars, field.length); err_var("%c", c);});
				}
				numbers[j] += (c - '0') * offset;
			}
		}
		HALF_MOVE_COUNTER = numbers[0];
		FULL_MOVE_COUNTER = numbers[1];
		if (FULL_MOVE_COUNTER < 1) {
			err(return 1, "full move counter must be greater than zero", err_var("%d", FULL_MOVE_COUNTER));
		}
	}
	
	#ifdef CHESS_VERBOSE
	for (uint8_t i = 0; i < 6; i++)
	{
		err_varn("%.*s", fieldIndexes[i].chars, fieldIndexes[i].length);
	}
	err_varn("%d", EN_PASSANT_TARGET_INDEX);
	err_varn("%d", HALF_MOVE_COUNTER);
	err_varn("%d", FULL_MOVE_COUNTER);
	Bitboard_PrintType(KING);
	Bitboard_PrintType(QUEEN);
	Bitboard_PrintType(BISHOP);
	Bitboard_PrintType(KNIGHT);
	Bitboard_PrintType(ROOK);
	Bitboard_PrintType(PAWN);
	Bitboard_PrintSide(WHITE);
	Bitboard_PrintSide(BLACK);
	#endif // CHESS_VERBOSE
	return 0;
}
#endif // CHESS_IMPLEMENTATION

#ifdef CHESS_MAIN
#undef CHESS_MAIN
int main(int argc, char * argv[])
{
	return ChessInit_FromString((StringIndex){argv[1], Stringlength(argv[1])});	
}
#endif // CHESS_MAIN
