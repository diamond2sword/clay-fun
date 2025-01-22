#include <stdio.h>
#define CHESS_VERBOSE
#include "chess.h"

typedef enum PieceTypeIndex : uint8_t {
	PIECE_TYPE_INDEX_NONE,
	PIECE_TYPE_INDEX_KING,
	PIECE_TYPE_INDEX_QUEEN,
	PIECE_TYPE_INDEX_BISHOP,
	PIECE_TYPE_INDEX_KNIGHT,
	PIECE_TYPE_INDEX_ROOK,
	PIECE_TYPE_INDEX_PAWN,
	PIECE_TYPE_INDEX_COUNT
} PieceTypeIndex;

typedef enum PieceSideIndex : uint8_t {
	PIECE_SIDE_INDEX_NONE,
	PIECE_SIDE_INDEX_WHITE,
	PIECE_SIDE_INDEX_BLACK,
	PIECE_SIDE_INDEX_COUNT,
} PieceSideIndex;

typedef uint64_t Bitboards_Side[PIECE_TYPE_INDEX_COUNT];
typedef Bitboards_Side Bitboards_All[PIECE_SIDE_INDEX_COUNT];
Bitboards_All BITBOARD;

#define __MASK_PIECE(side, type) BITBOARD[side][type]
#define MASK_PIECE(sideToken, typeToken) __MASK_PIECE(PIECE_SIDE_INDEX_##sideToken, PIECE_TYPE_INDEX_##typeToken)
#define MASK_SIDE(sideToken) (MASK_PIECE(sideToken, KING) | MASK_PIECE(sideToken, QUEEN) | MASK_PIECE(sideToken, BISHOP) | MASK_PIECE(sideToken, KNIGHT) | MASK_PIECE(sideToken, ROOK) | MASK_PIECE(sideToken, PAWN))
#define MASK_TYPE(typeToken) (MASK_PIECE(WHITE, typeToken) | MASK_PIECE(BLACK, typeToken))
#define MASK_ALL (MASK_SIDE(BLACK) | MASK_SIDE(WHITE))
/*
#define Piece_GetType(index) (BOARD[index] & 0b111)
#define Piece_GetColor(index) ((BOARD[index] & 0b1000) >> 3)
#define Piece_HasType(index, type) (Piece_GetType(index) == type)
#define Piece_HasColor(index, color) (Piece_GetColor(index) == color)
bool Piece_IsSelectable(uint8_t index);
*/
#define lowerChar(c) ( \
	(c >= 'A' && c <= 'Z') \
	? c - ('A' - 'a') : c \
)

#define ArrayPair(k, v) [k] = v

PieceTypeIndex PieceTypeIndex_FromChar(char c)
{
	#define ArrayPair_PieceTypeIndexToChar(k, v) ArrayPair(PIECE_TYPE_INDEX_##k, v)
	static const char map_charToIndex[PIECE_TYPE_INDEX_COUNT] = {
		ArrayPair_PieceTypeIndexToChar(NONE, -1),
		ArrayPair_PieceTypeIndexToChar(KING, 'k'),
		ArrayPair_PieceTypeIndexToChar(QUEEN, 'q'),
		ArrayPair_PieceTypeIndexToChar(BISHOP, 'b'),
		ArrayPair_PieceTypeIndexToChar(KNIGHT, 'n'),
		ArrayPair_PieceTypeIndexToChar(ROOK, 'r'),
		ArrayPair_PieceTypeIndexToChar(PAWN, 'p')
	};
	c = lowerChar(c);
	for (uint8_t i = 0; i < PIECE_TYPE_INDEX_COUNT; i++)
	{
		if (c == map_charToIndex[i]) return i;
	}
	return PIECE_TYPE_INDEX_NONE;
			  /*
	case 'q': return PIECE_TYPE_QUEEN;
	case 'b': return PIECE_TYPE_BISHOP;
	case 'n': return PIECE_TYPE_KNIGHT;
	case 'r': return PIECE_TYPE_ROOK;
	case 'p': return PIECE_TYPE_PAWN;
	default: return PIECE_TYPE_NONE;
	}
	*/
}

PieceSideIndex PieceSideIndex_FromChar(char c)
{
	#define ArrayPair_PieceSideIndexToChar(k, v) ArrayPair(PIECE_SIDE_INDEX_##k, v)
	static const char map_charToIndex[PIECE_SIDE_INDEX_COUNT] = {
		ArrayPair_PieceSideIndexToChar(NONE, -1),
		ArrayPair_PieceSideIndexToChar(WHITE, 'w'),
		ArrayPair_PieceSideIndexToChar(BLACK, 'b'),
	};
	c = lowerChar(c);
	for (uint8_t i = 0; i < PIECE_SIDE_INDEX_COUNT; i++)
	{
		if (c == map_charToIndex[i]) return i;
	}
	return PIECE_SIDE_INDEX_NONE;
			  /*
	case 'q': return PIECE_TYPE_QUEEN;
	case 'b': return PIECE_TYPE_BISHOP;
	case 'n': return PIECE_TYPE_KNIGHT;
	case 'r': return PIECE_TYPE_ROOK;
	case 'p': return PIECE_TYPE_PAWN;
	default: return PIECE_TYPE_NONE;
	}
	*/
}

void Bitboard_Put(uint8_t index, PieceSideIndex side, PieceTypeIndex type)
{
	uint64_t emptyMask = 0ULL;
	uint64_t positionMask = (1ULL << index);
	BITBOARD[side][type] |= positionMask;
	/*
	Bitboards_Side * side = color == PIECE_COLOR_WHITE
		? &BITBOARD.white : &BITBOARD.black;
	uint64_t * pieces
		= type == PIECE_TYPE_KING ? &side->kings
		: type == PIECE_TYPE_QUEEN ? &side->queens
		: type == PIECE_TYPE_BISHOP ? &side->bishops
		: type == PIECE_TYPE_KNIGHT ? &side->knights
		: type == PIECE_TYPE_ROOK ? &side->rooks
		: type == PIECE_TYPE_PAWN ? &side->pawns
		: &emptyMask;
	*pieces |= positionMask;
	*/
}
#define BITBOARD_PUT(index, sideToken, typeToken) Bitboard_Put(index, PIECE_SIDE_INDEX_##sideToken, PIECE_TYPE_INDEX_##typeToken) 

#ifdef CHESS_VERBOSE
void Bitboard_Print(uint64_t bitboard)
{
	for (uint8_t row = 0; row < 8; row++)
	{
		for (uint8_t col = 0; col < 8; col++)
		{
			uint8_t index = row * 8 + col;
			StringIndex token = ((bitboard >> index) & 0b1)
				? STRING("x") : STRING("-");
			printf("%3.*s", token.length, token.chars);
		}
		printf("\n");
	}
}
#define Bitboard_PrintType(typeToken) { \
	printf("%s:\n", #typeToken); \
	Bitboard_Print(MASK_TYPE(typeToken)); \
}
#define Bitboard_PrintSide(sideToken) { \
	printf("%s:\n", #sideToken); \
	Bitboard_Print(MASK_SIDE(sideToken)); \
}
#define Bitboard_PrintPiece(sideToken, typeToken) { \
	printf("%s.%s:\n", #sideToken, #typeToken); \
	Bitboard_Print(MASK_PIECE(sideToken, typeToken)); \
}
#endif // CHESS_VERBOSE

// new

// old
CastlingSides CASTLING_SIDES = {0};
uint8_t BOARD[64] = {0};
PieceColor ACTIVE_COLOR = PIECE_COLOR_WHITE;
int8_t EN_PASSANT_INDEX = -1;
int16_t HALF_MOVE_COUNTER = 0;
int16_t FULL_MOVE_COUNTER = 1;


#define Piece_GetType(index) (BOARD[index] & 0b111)
#define Piece_GetColor(index) ((BOARD[index] & 0b1000) >> 3)
#define Piece_HasType(index, type) (Piece_GetType(index) == type)
#define Piece_HasColor(index, color) (Piece_GetColor(index) == color)
bool Piece_IsSelectable(uint8_t index);

bool Piece_IsSelectable(uint8_t index)
{
	return !Piece_HasType(index, PIECE_TYPE_NONE)
	&& Piece_HasColor(index, ACTIVE_COLOR);
}

PieceType PieceType_FromChar(char c)
{
	if (c >= 'A' && c <= 'Z') {
		c += 'a' - 'A';
	}
	switch (c) {
	case 'k': return PIECE_TYPE_KING;
	case 'q': return PIECE_TYPE_QUEEN;
	case 'b': return PIECE_TYPE_BISHOP;
	case 'n': return PIECE_TYPE_KNIGHT;
	case 'r': return PIECE_TYPE_ROOK;
	case 'p': return PIECE_TYPE_PAWN;
	default: return PIECE_TYPE_NONE;
	}
}


PieceColor PieceColor_FromChar(char c)
{
	return (c >= 'A' && c <= 'Z') ? PIECE_COLOR_WHITE : PIECE_COLOR_BLACK;
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
			err_pause(
				err_var("%c", c);
				err_var("%d", PieceSideIndex_FromChar(c));
				err_varn("%d", PieceTypeIndex_FromChar(c));
			);
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
	#define Bidboard_Print(mask) {\
		Bitboard_Print(mask); \
	}
	#define Bitboard_PrintByType(type) {\
		printf("%s:\n", #type); \
		Bitboard_Print(MASK_TYPE(type)); \
	}
	/*
	Bitboard_PrintType(KING);
	Bitboard_PrintType(QUEEN);
	Bitboard_PrintType(BISHOP);
	Bitboard_PrintType(KNIGHT);
	Bitboard_PrintType(ROOK);
	Bitboard_PrintType(PAWN);
	Bitboard_PrintSide(WHITE);
	Bitboard_PrintSide(BLACK);
	*/
	#endif // CHESS_VERBOSE
	BITBOARD_PUT(0, WHITE, PAWN);
	Bitboard_PrintPiece(WHITE, PAWN);
	return 0;
}



int main()
{
	ChessInit_Default();
	return 0;
}
