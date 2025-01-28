#include "chess_typedefs.h.c"
#include "chess_mask.h.c"
#include "chess_accessors.h.c"

#pragma once

void Game_MakeMove(Game* game, Move move);
#define BitboardSet_PutAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Put(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)
#define BitboardSet_Put(bitboardSet, index, side, type) \
	bitboardSet[side][type] |= MASK_INDEX(index)
#define BITBOARD_SET_PUT(bitboardSet, index, sideToken, typeToken) \
	BitboardSet_Put(bitboardSet, index, PIECE_SIDE_INDEX_##sideToken, PIECE_TYPE_INDEX_##typeToken) 

#define BitboardSet_RemoveAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Remove(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)
#define BitboardSet_Remove(bitboardSet, index, side, type) bitboardSet[side][type] &= ~MASK_INDEX(index)
#define BITBOARD_SET_REMOVE(bitboardSet, index, sideToken, typeToken) \
	BitboardSet_Remove(bitboardSet, index, PIECE_SIDE_INDEX_##sideToken, PIECE_TYPE_INDEX_##typeToken) 

void Game_MakeMove(Game* game, Move move)
{
	bool enPassantTarget_isSet_thisFrame = false;
	{
		// make move
		BitboardSet_RemoveAttrs(game->bitboardSet, move, src.index, src.side, src.type);
		BitboardSet_RemoveAttrs(game->bitboardSet, move, dst.index, dst.side, dst.type);
		BitboardSet_PutAttrs(game->bitboardSet, move, dst.index, src.side, src.type);
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
				game->index_enPassantTarget = move.src.index + OFFSET_FORWARD(move.src.side);
			}
			// en passant pawn deletion
			else if (move.dst.index == game->index_enPassantTarget)
			{
				Piece enPassantPawn = Piece_New(game->bitboardSet, move.dst.index + OFFSET_BACKWARD(move.src.side));
				BitboardSet_RemoveAttrs(game->bitboardSet, enPassantPawn, index, side, type);
			}
			// pawn promotion
			else if (MASK_INDEX(move.dst.index) & Direct_Rows(move.src.side, MASK_ROW_0))
			{
				BitboardSet_RemoveAttrs(game->bitboardSet, move, dst.index, src.side, src.type);
				BitboardSet_Put(game->bitboardSet, move.dst.index, move.src.side, move.type_promotion);
				move.type_promotion = PIECE_TYPE_INDEX_NONE;
			}
		}
		else if (move.src.type == PIECE_TYPE_INDEX_KING)
		{
			// untoggle castling rights
			// castle queen side
			if (game->castlingRights[move.src.side][CASTLING_SIDE_QUEEN]
			&& move.dst.index == move.src.index + OFFSET_LEFT * 2)
			{
				BitboardSet_Remove(game->bitboardSet, move.src.index + OFFSET_LEFT * 4, move.src.side, PIECE_TYPE_INDEX_ROOK);
				BitboardSet_Put(game->bitboardSet, move.src.index + OFFSET_LEFT, move.src.side, PIECE_TYPE_INDEX_ROOK);
			}
			// castle king side
			if (game->castlingRights[move.src.side][CASTLING_SIDE_KING]
			&& move.dst.index == move.src.index + OFFSET_RIGHT * 2)
			{
				BitboardSet_Remove(game->bitboardSet, move.src.index + OFFSET_RIGHT * 3, move.src.side, PIECE_TYPE_INDEX_ROOK);
				BitboardSet_Put(game->bitboardSet, move.src.index + OFFSET_RIGHT, move.src.side, PIECE_TYPE_INDEX_ROOK);
			}
			game->castlingRights[move.src.side][CASTLING_SIDE_QUEEN] = false;
			game->castlingRights[move.src.side][CASTLING_SIDE_KING] = false;
		}
		else if (move.src.type == PIECE_TYPE_INDEX_ROOK)
		{
			if (MASK_INDEX(move.src.index) & Direct_Rows(move.src.side, MASK_INDEX(OFFSET_DOWN * 7)))
			{
				game->castlingRights[move.src.side][CASTLING_SIDE_QUEEN] = false;
			} 
			else if (MASK_INDEX(move.src.index) & Direct_Rows(move.src.side, MASK_INDEX(OFFSET_DOWN * 7 + OFFSET_RIGHT * 7)))
			{
				game->castlingRights[move.src.side][CASTLING_SIDE_KING] = false;
			}
		}
	}
	{
		// reset en passant index after a move
		if (!enPassantTarget_isSet_thisFrame)
		{
			game->index_enPassantTarget = -1;
		}
	}
	{
		// full moves increment
		if (move.src.side == PIECE_SIDE_INDEX_BLACK)
		{
			game->counter_fullMove++;
		}
	}
	{
		// half moves
		if (move.src.type == PIECE_TYPE_INDEX_PAWN
		|| move.dst.type != PIECE_TYPE_INDEX_NONE)
		{
			game->counter_halfMove = 0;
		}
		else
		{
			game->counter_halfMove++;
		}
	}
	{
		// Switch side
		game->side_active = SIDE_NEGATE(game->side_active);
	}
}
