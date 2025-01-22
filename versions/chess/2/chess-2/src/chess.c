#define CHESS_IMPLEMENTATION
#define CHESS_BITBOARD_IMPLEMENTATION
#define CHESS_VERBOSE
#include "chess.h"

/*
#define Piece_GetType(index) (BOARD[index] & 0b111)
#define Piece_GetColor(index) ((BOARD[index] & 0b1000) >> 3)
#define Piece_HasType(index, type) (Piece_GetType(index) == type)
#define Piece_HasColor(index, color) (Piece_GetColor(index) == color)
bool Piece_IsSelectable(uint8_t index);
*/




// new

// old
/*
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
		for (uint8_t i = 0; i < 64; i++)
		{
			BOARD[i] = PIECE_TYPE_NONE;
		}
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
			if (PieceType_FromChar(c) == PIECE_TYPE_NONE) {
				err(return 1, "not a char for piece type", err_var("%c", c));
			}
			BOARD[row * 8 + col] = PieceType_FromChar(c) | PieceColor_FromChar(c);
			col++;

			if (i + 1 == field.length || field.chars[i + 1] == '/') {
				if (col != 8) {
					err(return 1, "cols can only be exactly 8 before next row", {err_var("%d", col); err_var("%d", row);});
				}
			}
		}
	}

	// FIELD 0
	{
		//reset
		for (uint8_t i = 0; i < PIECE_SIDE_INDEX_COUNT; i++) 
		for (uint8_t j = 0; j < PIECE_TYPE_INDEX_COUNT; j++) 
		BITBOARD[i][j] = 0ULL;

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
			Bitboard_Put(row * 8 + col, PieceSideIndex_FromChar(c), PieceTypeIndex_FromChar(c));
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
		ACTIVE_COLOR = c == 'w' ? PIECE_COLOR_WHITE : PIECE_COLOR_BLACK;
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
				case 'K': CASTLING_SIDES.whiteKing = true; break;
				case 'Q': CASTLING_SIDES.whiteQueen = true; break;
				case 'k': CASTLING_SIDES.blackKing = true; break;
				case 'q': CASTLING_SIDES.blackQueen = true; break;
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
			EN_PASSANT_INDEX = file - 'a' + (7 - (rank - '1')) * 8;
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
	for (uint8_t i = 0; i < 64; i++)
	{
		if (i % 8 == 0) printf("\n");
		printf(" %2d", BOARD[i]);
		if (i == 63) printf("\n");
	}
	err_varn("%d", ACTIVE_COLOR);
	err_varn("%d", CASTLING_SIDES.blackKing);
	err_varn("%d", CASTLING_SIDES.blackQueen);
	err_varn("%d", CASTLING_SIDES.whiteKing);
	err_varn("%d", CASTLING_SIDES.whiteQueen);
	err_varn("%d", EN_PASSANT_INDEX);
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
*/
int main()
{
	ChessInit_Default();
	return 0;
}
