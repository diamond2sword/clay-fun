#include "chess_arena.h.c"
#include "chess_err.h.c"
#include "chess_mask_attacks.h.c"

#include <stdint.h>

#pragma once

typedef enum PiecePromotionIndex : uint8_t {
	PIECE_PROMOTION_INDEX_QUEEN,
	PIECE_PROMOTION_INDEX_ROOK,
	PIECE_PROMOTION_INDEX_BISHOP,
	PIECE_PROMOTION_INDEX_KNIGHT,
	PIECE_PROMOTION_INDEX_COUNT,
	PIECE_PROMOTION_INDEX_NONE,
} PiecePromotionIndex;

typedef struct BoardClicker {
	Game* game;
	uint64_t bitboard_movables;
	uint64_t bitboard_attacks;
	uint64_t bitboard_promotion_options;
	uint64_t* bitboard_selectables;
	Move move_temp;
	int8_t index_input;
	int8_t index_src;
	int8_t index_dst;
	int8_t index_promotion_options;
} BoardClicker;

BoardClicker* BoardClicker_New(ChessArena* arena, Game* game);
void BoardClicker_Phase_Reset(BoardClicker* boardClicker, bool hasMadeMove);
void BoardClicker_Phase_SelectSource(BoardClicker* boardClicker);
void BoardClicker_Phase_Reset_AutoSelect(BoardClicker* boardClicker);
void BoardClicker_Click(BoardClicker* boardClicker);

const PieceTypeIndex MAP_PROMOTION_OPT_TO_TYPE[PIECE_PROMOTION_INDEX_COUNT] = {
	ArrayPair(PIECE_PROMOTION_INDEX_QUEEN, PIECE_TYPE_INDEX_QUEEN),
	ArrayPair(PIECE_PROMOTION_INDEX_ROOK, PIECE_TYPE_INDEX_ROOK),
	ArrayPair(PIECE_PROMOTION_INDEX_BISHOP, PIECE_TYPE_INDEX_BISHOP),
	ArrayPair(PIECE_PROMOTION_INDEX_KNIGHT, PIECE_TYPE_INDEX_KNIGHT)
};

BoardClicker* BoardClicker_New(ChessArena* arena, Game* game)
{
	BoardClicker* boardClicker = (BoardClicker*)ChessArena__Allocate(arena, sizeof(BoardClicker));
	boardClicker->game = game;
	BoardClicker_Phase_Reset(boardClicker, true);
	return boardClicker;
}

void BoardClicker_Phase_SelectSource(BoardClicker* boardClicker)
{
	if (MASK_INDEX_SIGNED(boardClicker->index_input) & boardClicker->bitboard_movables)
	{
		boardClicker->index_src = boardClicker->index_input;
		boardClicker->bitboard_attacks = Mask_Attacks_KingIsSafeAfter(boardClicker->game, Piece_New(boardClicker->game->bitboardSet, boardClicker->index_src));
		boardClicker->bitboard_selectables = &boardClicker->bitboard_attacks;
	}
}
void BoardClicker_Phase_Reset(BoardClicker* boardClicker, bool hasMadeMove)
{
	// reset click phase
	boardClicker->index_src = -1;
	boardClicker->index_dst = -1;
	boardClicker->index_promotion_options = -1;
	boardClicker->move_temp = (Move){};
	//IsCellSelectableFunc = Piece_IsMovable;
	boardClicker->bitboard_selectables = &boardClicker->bitboard_movables;
	if (hasMadeMove)
	{
		boardClicker->index_input = -1;
		boardClicker->bitboard_movables = Mask_Movables(boardClicker->game, boardClicker->game->side_active, Mask_Attacks_KingIsSafeAfter);
	}
}

void BoardClicker_Phase_Reset_AutoSelect(BoardClicker* boardClicker)
{
	const bool selected_isOtherAlly = boardClicker->index_input != boardClicker->index_src;
	// reset click phase
	BoardClicker_Phase_Reset(boardClicker, false);
	// skip src phase if selected is movable
	if (selected_isOtherAlly)
	{
		BoardClicker_Phase_SelectSource(boardClicker);
	}
}

void BoardClicker_Click(BoardClicker* boardClicker)
{
	static bool enable_makeMove = false;
	if (boardClicker->index_src == -1)
	{
		BoardClicker_Phase_SelectSource(boardClicker);
	}
	else if (boardClicker->index_dst == -1)
	{
		if (MASK_INDEX_SIGNED(boardClicker->index_input) & boardClicker->bitboard_attacks)
		{
			enable_makeMove = true;
			// set move if valid
			boardClicker->index_dst = boardClicker->index_input;
			boardClicker->move_temp = Move_New(boardClicker->game->bitboardSet, boardClicker->index_src, boardClicker->index_dst);
			{
				// to trigger pawn promotion option selection
				if (boardClicker->move_temp.src.type == PIECE_TYPE_INDEX_PAWN)
				{
					if (MASK_INDEX(boardClicker->move_temp.dst.index) & Direct_Rows(boardClicker->move_temp.src.side, MASK_ROW_0))
					{
						enable_makeMove = false;
						boardClicker->index_promotion_options = boardClicker->move_temp.dst.index;
						boardClicker->bitboard_promotion_options = Direct_Rows(boardClicker->move_temp.src.side, Mask_Rows(4, 0, 1, 2, 3) & MASK_COL(Index_AsCol(boardClicker->index_promotion_options))); 
						//IsCellSelectableFunc = Piece_IsPromotionOpt;
						boardClicker->bitboard_selectables = &boardClicker->bitboard_promotion_options;
					}
				}
			}
		}
		else
		{
			BoardClicker_Phase_Reset_AutoSelect(boardClicker);
		}
	}
	else if (boardClicker->index_promotion_options != -1)
	{
		boardClicker->index_promotion_options = -1;

		if (MASK_INDEX_SIGNED(boardClicker->index_input) & boardClicker->bitboard_promotion_options)
		{
			enable_makeMove = true;
			int row = Index_AsRow(boardClicker->index_input);
			boardClicker->move_temp.type_promotion = IF_SIDE(boardClicker->move_temp.src.side,
				PIECE_TYPE_INDEX_NONE,
				MAP_PROMOTION_OPT_TO_TYPE[row],
				MAP_PROMOTION_OPT_TO_TYPE[7 - row]);
		}
		else
		{
			BoardClicker_Phase_Reset_AutoSelect(boardClicker);
		}
	}
	if (enable_makeMove)
	{
		enable_makeMove = false;
		Game_MakeMove(boardClicker->game, boardClicker->move_temp, true);
		// reset click phase
		BoardClicker_Phase_Reset(boardClicker, true);
	}
	{
		boardClicker->index_input = -1;
	}
}
