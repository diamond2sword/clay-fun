#include "chess_typedefs.h.c"
#include "chess_mask.h.c"
#include "chess_accessors.h.c"
#include "chess_make_move.h.c"
#include "chess_helpers.h.c"
#include "chess_sliders.h.c"

#include <stdint.h>

#pragma once


uint64_t Mask_Attacks(Game* game, Piece piece);
uint64_t Mask_Movables(Game* game, PieceSideIndex side, uint64_t (*maskMovesFunc)(Game*, Piece));
uint64_t Mask_Attacks_OnEnemyKing(Game* game, Piece piece);
uint64_t Mask_Attacks_KingIsSafeAfter(Game* game, Piece piece);

uint64_t Mask_Attacks_OnEnemyKing(Game* game, Piece piece)
{
	return Mask_Piece(game->bitboardSet, SIDE_NEGATE(piece.side), PIECE_TYPE_INDEX_KING) & Mask_Attacks(game, piece);
}

uint64_t Mask_Movables(Game* game, PieceSideIndex side, uint64_t (*maskMovesFunc)(Game*, Piece))
{
	uint64_t mask_result = MASK_EMPTY;
	for (uint8_t i = 0; i < 64; i++)
	{
		Piece piece = Piece_New(game->bitboardSet, i);
		if (piece.side != side)
		{
			continue;
		}
		if (maskMovesFunc(game, piece))
		{
			mask_result |= MASK_INDEX(i);
		}
	}
	return mask_result;
}

uint64_t Mask_Attacks_KingIsSafeAfter(Game* game, Piece piece)
{
	PieceSideIndex side_inactive = SIDE_NEGATE(piece.side);
	uint64_t mask_attacks = Mask_Attacks(game, piece);

	Bitboards_All bitboardSet_temp;
	Game game_temp;
	game->bitboardSet = &bitboardSet_temp;

	for (uint8_t index_dst = 0; index_dst < 64; index_dst++)
	{
		uint64_t mask_dst = MASK_INDEX(index_dst);
		if (mask_dst & mask_attacks)
		{
			ARRAY_2(uint64_t, Copy, *game->bitboardSet, game_temp.bitboardSet);
			Move move_mock = Move_New(game_temp.bitboardSet, piece.index, index_dst);
			Game_MakeMove(&game_temp, move_mock, false);
			{
				// simulate king being two if in castling
				if (piece.type == PIECE_TYPE_INDEX_KING)
				{
					if (index_dst == piece.index + OFFSET_LEFT * 2)
					{
						Bitboards_All_Put(game_temp.bitboardSet, piece.index + OFFSET_LEFT, piece.side, piece.type);
						Bitboards_All_Put(game_temp.bitboardSet, piece.index, piece.side, piece.type);
					} else if (index_dst == piece.index + OFFSET_RIGHT * 2)
					{
						Bitboards_All_Put(game_temp.bitboardSet, piece.index + OFFSET_RIGHT, piece.side, piece.type);
						Bitboards_All_Put(game_temp.bitboardSet, piece.index, piece.side, piece.type);
					}
				}
			}
			//if (Mask_Attacks_InCheck(bitboardSet_temp, side_inactive))
			if (Mask_Movables(&game_temp, side_inactive, Mask_Attacks_OnEnemyKing))
			{
				mask_attacks &= ~mask_dst;
			}
		}
	}
	return mask_attacks;
}

uint64_t Mask_Attacks(Game* game, Piece piece)
{
	uint8_t col = Index_AsCol(piece.index);
	uint8_t row = Index_AsRow(piece.index);
	PieceSideIndex inactiveSide = SIDE_NEGATE(piece.side);
	uint64_t mask_occupied = MASK_ALL(game->bitboardSet);
	uint64_t mask_enemy = Mask_Side(game->bitboardSet, inactiveSide);
	switch (piece.type) {
	case PIECE_TYPE_INDEX_PAWN: {
		int8_t offsetSign = DIRECT_OFFSET_SIGN(piece.side);
		int8_t offsetForward = OFFSET_FORWARD(piece.side);
		uint64_t mask_enPassantTarget = MASK_INDEX_SIGNED(game->index_enPassantTarget);
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
		uint64_t mask_canCastle_left = (MASK_ROW(row) & MASK_COL(0)) & game->mask_castlingRights;
		uint64_t mask_canCastle_right = (MASK_ROW(row) & MASK_COL(7)) & game->mask_castlingRights;
		uint64_t mask_castle_queenSide = (mask_canCastle_left && mask_singleLeft && mask_singleRight_rook) ? MASK_INDEX(piece.index + OFFSET_LEFT * 2) & ~mask_occupied : MASK_EMPTY;
		uint64_t mask_castle_kingSide = (mask_canCastle_right && mask_singleRight) ? MASK_INDEX(piece.index + OFFSET_RIGHT * 2) & ~mask_occupied : MASK_EMPTY;
		return mask_singleStep | mask_castle_queenSide | mask_castle_kingSide;
	} case PIECE_TYPE_INDEX_KNIGHT: {
		return ((Mask_Rows(2, row - 1, row + 1) & Mask_Cols(2, col - 2 , col + 2)) | (Mask_Rows(2, row - 2, row + 2) & Mask_Cols(2, col - 1, col + 1)))
			& (mask_enemy | ~mask_occupied);
	} default: 
		return MASK_EMPTY;
	}
}

