#include <stdarg.h>

#define CLAY_HELPERS_IMPLEMENTATION
#include "clay_helpers.h"

#define CLAY_IMPLEMENTATION
#include "web_clay.h"

#define CLAY_CLICK_HANDLER_IMPLEMENTATION
#include "clay_click_handler.h"

#define CHESS_IMPLEMENTATION
#define CHESS_BITBOARD_IMPLEMENTATION
#include "chess.h"

#define MY_MATH_IMPLEMENTATION
#include "math.h"




#define BitboardSet_PutAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Put(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)
#define BitboardSet_RemoveAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Remove(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)

#define BitboardSet_Remove(bitboardSet, index, side, type) bitboardSet[side][type] &= ~MASK_INDEX(index)

#define MASK_INDEX_SIGNED(index) (index < 0 ? MASK_EMPTY : MASK_INDEX(index))
#define MASK_INDEX_UNBOUND(index) (IF_IN_RANGE(index, 0, 63, MASK_INDEX(index), MASK_EMPTY))

uint64_t Mask_Rows(uint32_t count, ...);
uint64_t Mask_Cols(uint32_t count, ...);
		
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

#define Mask_GetRow(row, mask) (MASK_ROW(row) & mask)
#define MASK_ROW(row) IF_IN_RANGE(row, 0, 7, MASK_ROW_0 << ((row) * 8), MASK_EMPTY)
#define MASK_COL(col) IF_IN_RANGE(col, 0, 7, MASK_COL_0 << (col), MASK_EMPTY)

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
#define MASK_ROW_0 0xFFULL
#define MASK_COL_0 0x0101010101010101ULL
#define MASK_CHECKER 0x55AA55AA55AA55AAULL


CLAY_WASM_EXPORT("Init") void Init();
Piece Piece_Get(Bitboards_All bitboardSet, uint8_t index);
Move Move_New(Bitboards_All bitboardSet, uint8_t srcIndex, uint8_t dstIndex);
void Precompute_SlidingPiece_RayIndexes();
uint64_t Mask_Attacks_Sliding(uint64_t mask_occupied, uint8_t index, int32_t ray_count, ...);
uint64_t Mask_Attacks(Bitboards_All bitboardSet, Piece piece);
uint64_t Mask_Movables(Bitboards_All bitboardSet, PieceSideIndex side, uint64_t (*maskMovesFunc)(Bitboards_All, Piece));
uint64_t Mask_PromotionOptions(Bitboards_All bitboardSet, uint64_t index, Move move);
/*
bool Piece_IsAttack(int8_t index);
bool Piece_IsMovable(int8_t index);
bool Piece_IsPromotionOpt(int8_t index);
*/
void BoardClick_Phase_Reset(bool hasMadeMove);
void BoardClick_Phase_SelectSource();
void BoardClick_Phase_Reset_AutoSelect();
void Game_MakeMove(Bitboards_All bitboards, Move move, PieceSideIndex* side_active, int8_t* index_enPassantTarget, PieceTypeIndex* type_promotion, CastlingRights castlingRights, uint16_t* fullMoveCounter, uint16_t* halfMoveCounter);
void BoardClick();
void listen_cell_click();
void handle_cell_clickInteraction(ON_HOVER_PARAMS);
void BoardBackground();
void BoardPieceSprites();
void BoardPromotionPopup();
void BoardClickLayer();
Clay_RenderCommandArray CreateLayout();
void Helpers_Update(float width, float height);
CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(
	float width, float height,
	float mouseWheelX, float mouseWheelY,
	float mousePositionX, float mousePositionY,
	bool isTouchDown, bool isMouseDown,
	bool isTouchStart, bool isMouseStart,
	bool isTouchCancel, bool isMouseCancel,
	bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame,
	bool dKeyPressedThisFrame,
	float currentTime, float deltaTime
);
int main();



// clay_helpers
double windowWidth, windowHeight;
double windowSmallSide, windowLongSide;
bool isMobileScreen;
float animationLerpValue;
bool debugModeEnabled;
const uint32_t FONT_ID_TEXT;



double BOARD_WIDTH;
double BOARD_CELL_WIDTH;



bool (* IsCellSelectableFunc)(int8_t index);
uint64_t * SELECTABLES_BITBOARD;
uint64_t MOVABLES_BITBOARD;
uint64_t ATTACKS_BITBOARD;
uint64_t MOVED_BITBOARD;
int8_t INDEX_INPUT;
int8_t INDEX_SRC;
int8_t INDEX_DST;
int8_t INDEX_PROMOTION_OPTION;
Move MOVE_TEMP;
Move MOVE_RECENT;
PieceTypeIndex PIECE_TYPE_PROMOTION;
uint64_t PROMOTION_OPTS_BITBOARD;
const PieceTypeIndex MAP_PROMOTION_OPT_TO_TYPE[PIECE_PROMOTION_INDEX_COUNT] = {
	[PIECE_PROMOTION_INDEX_QUEEN] = PIECE_TYPE_INDEX_QUEEN,
	[PIECE_PROMOTION_INDEX_ROOK] = PIECE_TYPE_INDEX_ROOK,
	[PIECE_PROMOTION_INDEX_BISHOP] = PIECE_TYPE_INDEX_BISHOP,
	[PIECE_PROMOTION_INDEX_KNIGHT] = PIECE_TYPE_INDEX_KNIGHT,
};

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

typedef struct BoardClicker {
	Bitboards_All bitboardSet;
	uint64_t * mask_selectables;
	uint64_t mask_movables;
	uint64_t mask_attacks;
	uint64_t mask_options;
	PieceTypeIndex* promotionType;
	Move recentMove;
	CastlingRights castlingRights;
} BoardClicker;

CLAY_WASM_EXPORT("Init") void Init()
{
	// precomputes
	Precompute_SlidingPiece_RayIndexes();

	// init game
	ChessInit_Default();
	StringIndex fen = STRING("rnbqkbn1/pppppppP/4p3/3pP3/pPp5/7p/PPPPPPPP/RNBQKBNR w KQkq d6 0 1");
	fen = STRING("rnbqkbn1/ppPppppP/4p3/4P3/8/2p3PN/PpPPpP1P/RNBQKB1R b KQq - 0 1");

	ChessInit_FromString(fen);
	ChessInit_Default();

	BoardClick_Phase_Reset(true);
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
			PieceTypeIndex type_promotion = PIECE_TYPE_PROMOTION;
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
/*
bool Piece_IsAttack(int8_t index)
{
	return MASK_INDEX_SIGNED(index) & ATTACKS_BITBOARD;
}

bool Piece_IsMovable(int8_t index)
{
	return MASK_INDEX_SIGNED(index) & MOVABLES_BITBOARD;
}

bool Piece_IsPromotionOpt(int8_t index)
{
	return MASK_INDEX_SIGNED(index) & PROMOTION_OPTS_BITBOARD; 
}
*/

void BoardClick_Phase_SelectSource()
{
	if (MASK_INDEX_SIGNED(INDEX_INPUT) & MOVABLES_BITBOARD)
	{
		INDEX_SRC = INDEX_INPUT;
		ATTACKS_BITBOARD = Mask_Attacks_KingIsSafeAfter(BITBOARD_SET, Piece_Get(BITBOARD_SET, INDEX_SRC));
		SELECTABLES_BITBOARD = &ATTACKS_BITBOARD;
	}
}
void BoardClick_Phase_Reset(bool hasMadeMove)
{
	// reset click phase
	INDEX_SRC = -1;
	INDEX_DST = -1;
	INDEX_PROMOTION_OPTION = -1;
	PIECE_TYPE_PROMOTION = PIECE_TYPE_INDEX_NONE;
	MOVE_TEMP = (Move){};
	//IsCellSelectableFunc = Piece_IsMovable;
	SELECTABLES_BITBOARD = &MOVABLES_BITBOARD;
	if (hasMadeMove)
	{
		INDEX_INPUT = -1;
		MOVABLES_BITBOARD = Mask_Movables(BITBOARD_SET, ACTIVE_SIDE, Mask_Attacks_KingIsSafeAfter);
	}
}

void BoardClick_Phase_Reset_AutoSelect()
{
	const bool selected_isOtherAlly = INDEX_INPUT != INDEX_SRC;
	// reset click phase
	BoardClick_Phase_Reset(false);
	// skip src phase if selected is movable
	if (selected_isOtherAlly)
	{
		BoardClick_Phase_SelectSource();
	}
}

void BoardClick()
{
	static bool enable_makeMove = false;
	if (INDEX_SRC == -1)
	{
		BoardClick_Phase_SelectSource();
	}
	else if (INDEX_DST == -1)
	{
		if (MASK_INDEX_SIGNED(INDEX_INPUT) & ATTACKS_BITBOARD)
		{
			enable_makeMove = true;
			// set move if valid
			INDEX_DST = INDEX_INPUT;
			MOVE_TEMP = Move_New(BITBOARD_SET, INDEX_SRC, INDEX_DST);
			{
				// to trigger pawn promotion option selection
				if (MOVE_TEMP.src.type == PIECE_TYPE_INDEX_PAWN)
				{
					if (MASK_INDEX(MOVE_TEMP.dst.index) & Direct_Rows(MOVE_TEMP.src.side, MASK_ROW_0))
					{
						enable_makeMove = false;
						INDEX_PROMOTION_OPTION = MOVE_TEMP.dst.index;
						PROMOTION_OPTS_BITBOARD = Mask_PromotionOptions(BITBOARD_SET, INDEX_PROMOTION_OPTION, MOVE_TEMP);
						//IsCellSelectableFunc = Piece_IsPromotionOpt;
						SELECTABLES_BITBOARD = &PROMOTION_OPTS_BITBOARD;
					}
				}
			}
		}
		else
		{
			BoardClick_Phase_Reset_AutoSelect();
		}
	}
	else if (INDEX_PROMOTION_OPTION != -1)
	{
		INDEX_PROMOTION_OPTION = -1;

		if (MASK_INDEX_SIGNED(INDEX_INPUT) & PROMOTION_OPTS_BITBOARD)
		{
			enable_makeMove = true;
			int row = Index_AsRow(INDEX_INPUT);
			PIECE_TYPE_PROMOTION = IF_SIDE(MOVE_TEMP.src.side,
				PIECE_TYPE_INDEX_NONE,
				MAP_PROMOTION_OPT_TO_TYPE[row],
				MAP_PROMOTION_OPT_TO_TYPE[7 - row]);
		}
		else
		{
			BoardClick_Phase_Reset_AutoSelect();
		}
	}
	if (enable_makeMove)
	{
		enable_makeMove = false;
		Game_MakeMove(BITBOARD_SET, MOVE_TEMP, &ACTIVE_SIDE, &EN_PASSANT_TARGET_INDEX, &PIECE_TYPE_PROMOTION, CASTLING_RIGHTS, &FULL_MOVE_COUNTER, &HALF_MOVE_COUNTER);
		MOVE_RECENT = MOVE_TEMP;
		// reset click phase
		BoardClick_Phase_Reset(true);
	}
	{
		INDEX_INPUT = -1;
	}
}

CLAY_CLICK_HANDLER(cell,
	{BoardClick();},
	{INDEX_INPUT = userData;},
	{BoardClick();},
	{INDEX_INPUT = -1;},
	{}
);
void BoardBackground()
{
	static const Clay_Color colorBlack = RGB(180, 136, 97);
	static const Clay_Color colorWhite = RGB(239, 217, 180);
	CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_WIDTH, BOARD_WIDTH)), CLAY_FLOATING(), CLAY_RECTANGLE(colorWhite))
	for (uint8_t row = 0; row < 8; row++)
	for (uint8_t col = 0; col < 8; col++)
	if (MASK_INDEX(row * 8 + col) & MASK_CHECKER)
	// fill in only black
	CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
		CLAY_FLOATING({.offset={col * BOARD_CELL_WIDTH, row * BOARD_CELL_WIDTH}}),
		CLAY_RECTANGLE(colorBlack)
	);
}

void BoardPieceSprite(uint8_t row, uint8_t col)
{
	static const float subrectangleWidth = 270.f/6;
	static const float subrectangleHeight = 90.f/2;
	static const float pieceWidthRatio = 1.f;
	const float PIECE_WIDTH = BOARD_CELL_WIDTH * pieceWidthRatio;
	CLAY(CLAY_LAYOUT(SIZING_FIXED(PIECE_WIDTH, PIECE_WIDTH)),
		CLAY_FLOATING({.attachment={CLAY_ATTACH_POINT_CENTER_CENTER, CLAY_ATTACH_POINT_CENTER_CENTER}}),
		CLAY_IMAGE({.sourceURL=CLAY_STRING("/clay/images/pieces.svg"),
			.subrectangle={
				subrectangleWidth * col,
				subrectangleHeight * row,
				subrectangleWidth,
				subrectangleHeight
			}
		})
	);
}

void BoardPieceSprites()
{
	const uint64_t occupied = MASK_ALL(BITBOARD_SET);
	for (uint8_t row = 0; row < 8; row++)
	for (uint8_t col = 0; col < 8; col++)
	{
		const uint8_t index = row * 8 + col;
		if (MASK_INDEX(index) & occupied)
		{
			Piece piece = Piece_Get(BITBOARD_SET, index);
			CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
				CLAY_FLOATING({.offset={col * BOARD_CELL_WIDTH, row * BOARD_CELL_WIDTH}})
			)
			BoardPieceSprite(piece.side, piece.type);
		}
	}
}

void BoardPromotionPopup()
{
	static const Clay_Color colorBlack = RGB(180, 136, 97);
	static const Clay_Color colorWhite = RGB(239, 217, 180);
	if (INDEX_PROMOTION_OPTION == -1)
	{
		return;
	}
	for (uint8_t optIndex = 0; optIndex < 4; optIndex++)
	{
		const uint8_t index = INDEX_PROMOTION_OPTION + OFFSET_DOWN * optIndex * DIRECT_OFFSET_SIGN(MOVE_TEMP.src.side);
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
			CLAY_RECTANGLE((MASK_INDEX(index) & MASK_CHECKER) ? colorBlack : colorWhite),
			CLAY_FLOATING({.offset={Index_AsCol(index) * BOARD_CELL_WIDTH, Index_AsRow(index) * BOARD_CELL_WIDTH}})
		)
		BoardPieceSprite(MOVE_TEMP.src.side, MAP_PROMOTION_OPT_TO_TYPE[optIndex]);
	}
}

void BoardClickLayer()
{
	CLAY(CLAY_LAYOUT({SIZING_FIXED(BOARD_WIDTH, BOARD_WIDTH), .layoutDirection=CLAY_TOP_TO_BOTTOM}),
		CLAY_FLOATING())
	for (uint8_t row = 0; row < 8; row++) 
	CLAY(SIZING_FIXED(BOARD_WIDTH, BOARD_CELL_WIDTH))
	for (uint8_t col = 0; col < 8; col++)
	{
		const uint8_t index = row * 8 + col;
		Clay_Color color = MASK_INDEX_SIGNED(index) & *SELECTABLES_BITBOARD
			? (index == INDEX_INPUT ? RGBA(0, 255, 0, 128) : RGBA(0, 0, 0, 0))
			: (index == INDEX_INPUT ? RGBA(255, 0, 0, 128) : RGBA(0, 0, 0, 128));
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
			CLAY_RECTANGLE(color),
			Clay_OnHover(handle_cell_clickInteraction, index));
	}
}

Clay_RenderCommandArray CreateLayout()
{
	{
		BOARD_WIDTH = windowSmallSide;
		BOARD_CELL_WIDTH = BOARD_WIDTH / 8;
	}
    Clay_BeginLayout();
	CLAY(CLAY_LAYOUT({.layoutDirection=CLAY_TOP_TO_BOTTOM}))
	{
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_WIDTH, BOARD_WIDTH)))
		{
			BoardBackground();
			BoardPieceSprites();
			BoardPromotionPopup();
			BoardClickLayer();
		}
		CLAY(0)
		{
			NORMAL_TEXT("Half Moves: ", textConfig.textColor = COLOR_BLACK);
			Normal_Text(Clay__IntToString(HALF_MOVE_COUNTER), textConfig.textColor = COLOR_BLACK);
			NORMAL_TEXT("Full Moves: ", textConfig.textColor = COLOR_BLACK);
			Normal_Text(Clay__IntToString(FULL_MOVE_COUNTER), textConfig.textColor = COLOR_BLACK);
		}
	}
	{
		listen_cell_click();
	}
    return Clay_EndLayout();
}

float animationLerpValue = -1.0f;
bool debugModeEnabled = false;

void Helpers_Update(float width, float height)
{
	windowWidth = width;
	windowHeight = height;
	windowLongSide = MAX(windowWidth, windowHeight);
	windowSmallSide = MIN(windowWidth, windowHeight);
	isPortrait = windowWidth < windowHeight;
	lineWidth = 0.025f * windowSmallSide;
}

CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(
	float width, float height,
	float mouseWheelX, float mouseWheelY,
	float mousePositionX, float mousePositionY,
	bool isTouchDown, bool isMouseDown,
	bool isTouchStart, bool isMouseStart,
	bool isTouchCancel, bool isMouseCancel,
	bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame,
	bool dKeyPressedThisFrame,
	float currentTime, float deltaTime
)
{
	Helpers_Update(width, height);
	
	ClickHandler_Update(currentTime, deltaTime, isTouchStart || isMouseStart, isTouchCancel || isMouseCancel, isTouchDown || isMouseDown);

    Clay_SetLayoutDimensions((Clay_Dimensions) { width, height });

	animationLerpValue += deltaTime;
	if (animationLerpValue > 1) {
		animationLerpValue -= 2;
	}

    if (dKeyPressedThisFrame) {
        debugModeEnabled = !debugModeEnabled;
        Clay_SetDebugModeEnabled(debugModeEnabled);
    }
    Clay_SetCullingEnabled(true);
    Clay_SetExternalScrollHandlingEnabled(false);

    Clay__debugViewHighlightColor = (Clay_Color) {105,210,231, 120};

    Clay_SetPointerState((Clay_Vector2) {mousePositionX, mousePositionY}, isMouseDown || isTouchDown);

    Clay_UpdateScrollContainers(isTouchDown, (Clay_Vector2) {mouseWheelX, mouseWheelY}, deltaTime);

	isMobileScreen = windowWidth < 750;
	if (debugModeEnabled) {
		isMobileScreen = windowWidth < 950;
	}

    return CreateLayout();
    //----------------------------------------------------------------------------------
}


// Dummy main() to please cmake - TODO get wasm working with cmake on this example
int main() {
    return 0;
}
