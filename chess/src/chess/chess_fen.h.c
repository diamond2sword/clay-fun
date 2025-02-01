//#define CHESS_VERBOSE
//#define CHESS_VERBOSE_IMPLEMENTATION
#pragma once
#include "chess_typedefs.h.c"
#include "chess_helpers.h.c"
#include "chess_err.h.c"
#include "chess_mask.h.c"
#include "chess_make_move.h.c"










int Game_Set_FromFen(Game* game, StringIndex fen);
#define FEN_DEFAULT STRING("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")
int Game_Set_FromFen_Default(Game* game);

PieceTypeIndex PieceTypeIndex_FromChar(char c);
#define PieceSideIndex_FromChar(c) \
	(PieceTypeIndex_FromChar(c) == PIECE_TYPE_INDEX_NONE \
	? PIECE_SIDE_INDEX_NONE : isCapitalChar(c) \
	? PIECE_SIDE_INDEX_WHITE : PIECE_SIDE_INDEX_BLACK)

int Game_Set_FromFen_Default(Game* game)
{
	return Game_Set_FromFen(game, FEN_DEFAULT);
}

int Game_Set_FromFen(Game* game, StringIndex fen)
{
	game->counter_repeatable = 1;

	StringIndex fieldIndexes[6] = {0};
	// FEN
	{
		uint8_t n_fields = 0, start_i = 0,  c = fen.chars[0], prev_c = 0;
		if (c == ' ') {
			err_scope(err(return 1, "FEN can't have a leading space", {err_var("%c", c); err_var("%.*s", fen.chars, fen.length)}));
		} 
		for (uint8_t i = 0; i < fen.length; i++, prev_c = c, c = fen.chars[i])
		{
			if (prev_c == ' ') start_i = i;
			if (c == ' ') {
				if (prev_c == ' ') {
					err_scope(err(return 1, "double space in FEN", {err_var("%c", prev_c); err_var("%c", prev_c); err_var("%.*s", fen.chars, fen.length)}));
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
			err_scope(err(return 1, "you have to provide for the 6 fields of FEN", {err_var("%d", n_fields); err_var("%.*s", fen.chars, fen.length)}));
		}
	}
	
	// FIELD 0
	{
		//reset
		for (uint8_t i = 0; i < PIECE_SIDE_INDEX_COUNT; i++) 
		for (uint8_t j = 0; j < PIECE_TYPE_INDEX_COUNT; j++) 
		game->bitboardSet->pieces[i][j] = MASK_EMPTY;

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
				err_scope(err(return 1, "not a char for piece type", err_var("%c", c)));
			}
			BitboardSet_Put(game->bitboardSet, row * 8 + col, PieceSideIndex_FromChar(c), PieceTypeIndex_FromChar(c));
			col++;

			if (i + 1 == field.length || field.chars[i + 1] == '/') {
				if (col != 8) {
					err_scope(err(return 1, "cols can only be exactly 8 before next row", {err_var("%d", col); err_var("%d", row);}));
				}
			}
		}
	}
	
	// FIELD 1
	{
		const StringIndex field = fieldIndexes[1];
		if (field.length > 1) {
			err_scope(err(return 1, "the field must be only 1 char", err_var("%.*s", field.chars, field.length)));
		}
		const char c = field.chars[0];
		if (!ContainsChar(c, STRING("wb"))) {
			err_scope(err(return 1, "not a char for piece color", err_var("%c", c)));
		}
		//ACTIVE_COLOR = c == 'w' ? PIECE_COLOR_WHITE : PIECE_COLOR_BLACK;
		game->side_active = c == 'w' ? PIECE_SIDE_INDEX_WHITE : PIECE_SIDE_INDEX_BLACK;
	}

	// FIELD 2
	{
		const StringIndex field = fieldIndexes[2];
		if (ContainsChar('-', field)) {
			if (field.length != 1) {
				err_scope(err(return 1, "It must only contain '-' if it has the char '-'", err_var("%.*s", field.chars, field.length)));
			}
		} else {
			const StringIndex castlingSides = STRING("KQkq");
			for (uint8_t i = 0; i < field.length; i++)
			{
				const char c = field.chars[i];
				if (!ContainsChar(c, castlingSides)) {
					err_scope(err(return 1, "chars must be one of castling sides", {err_var("%.*s", field.chars, field.length); err_var("%c", c);}));
				}
				
				game->mask_castlingRights |= MASK_ROW(isCapitalChar(c) ? 7 : 0)
					| MASK_COL(lowerChar(c) == 'k' ? 7 : 0);
			}
			for (uint8_t i = 0; i < castlingSides.length; i++)
			{
				if (CountChar(castlingSides.chars[i], field) > 1) {
					err_scope(err(return 1, "castling side chars can only appear once", err_var("%.*s", field.chars, field.length)));
				}
			}
		}
	}
	
	// FIELD 3
	{
		const StringIndex field = fieldIndexes[3];
		if (ContainsChar('-', field)) {
			if (field.length != 1) {
				err_scope(err(return 1, "it must only contain '-' if it has the char '-'", err_var("%.*s", field.chars, field.length)));
			}
		} else {
			if (field.length != 2) {
				err_scope(err(return 1, "it requires exactly two chars to represent en passant target square", err_var("%.*s", field.chars, field.length)));
			}
			const char file = field.chars[0];
			const StringIndex files = STRING("abcdefgh");
			if (!ContainsChar(file, files)) {
				err_scope(err(return 1, "does not represent a file", {err_var("%c", file); err_var("%.*s", files.chars, files.length)}));
			}
			const char rank = field.chars[1];
			const StringIndex ranks = STRING("12345678");
			if (!ContainsChar(rank, ranks)) {
				err_scope(err(return 1, "does not represent a rank", {err_var("%c", rank); err_var("%.*s", ranks.chars, ranks.length)}));
			}
			game->index_enPassantTarget = file - 'a' + (7 - (rank - '1')) * 8;
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
				err_scope(err(return 1, "there can't be leading zeroes if it's not zero", err_var("%.*s", field.chars, field.length)));
			}
			uint32_t offset = 1;
			for (uint8_t i = 0; i < field.length; i++, offset *= 10)
			{
				const char c = field.chars[field.length - i - 1];
				if (!ContainsChar(c, digits)) {
					err_scope(err(return 1, "a char must be a digit", {err_var("%.*s", field.chars, field.length); err_var("%c", c);}));
				}
				numbers[j] += (c - '0') * offset;
			}
		}
		game->counter_halfMove = numbers[0];
		game->counter_fullMove = numbers[1];
	}
	#ifdef CHESS_VERBOSE_IMPLEMENTATION
	err_scope(
		for (uint8_t i = 0; i < 6; i++)
		{
			err_varn("%.*s", fieldIndexes[i].chars, fieldIndexes[i].length);
		}
		err_varn("%d", game->index_enPassantTarget);
		err_varn("%d", game->counter_halfMove);
		err_varn("%d", game->counter_fullMove);
		Bitboard_PrintMask(game->mask_castlingRights);
		Bitboard_PrintType(game->bitboardSet, KING);
		Bitboard_PrintType(game->bitboardSet, QUEEN);
		Bitboard_PrintType(game->bitboardSet, BISHOP);
		Bitboard_PrintType(game->bitboardSet, KNIGHT);
		Bitboard_PrintType(game->bitboardSet, ROOK);
		Bitboard_PrintType(game->bitboardSet, PAWN);
		Bitboard_PrintSide(game->bitboardSet, WHITE);
		Bitboard_PrintSide(game->bitboardSet, BLACK);
	);
	#endif // CHESS_VERBOSE_IMPLEMENTATION
	return 0;
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
