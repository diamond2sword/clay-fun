#include <stdint.h>
#include <stdbool.h>

#ifdef CHESS_VERBOSE
	#include <stdio.h>
#endif // CHESS_VERBOSE

#ifndef CHESS_HEADER
#define CHESS_HEADER

typedef enum PieceType : uint8_t {
	PIECE_TYPE_NONE,
	PIECE_TYPE_KING,
	PIECE_TYPE_QUEEN,
	PIECE_TYPE_BISHOP,
	PIECE_TYPE_KNIGHT,
	PIECE_TYPE_ROOK,
	PIECE_TYPE_PAWN
} PieceType;

typedef enum PieceColor : uint8_t {
	PIECE_COLOR_WHITE = 0,
	PIECE_COLOR_BLACK = 8
} PieceColor;

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

typedef struct CastlingSides {
	bool whiteKing;
	bool whiteQueen;
	bool blackKing;
	bool blackQueen;
} CastlingSides;

typedef struct StringIndex {
	const char * chars;
	uint32_t length;
} StringIndex;

// helpers
// introduced in board
bool ContainsChar(const char c, StringIndex string);
uint32_t CountChar(const char c, StringIndex string);
uint32_t Stringlength(const char * string);
// introduced in bitboards
#define isCapitalChar(c) ((c) >= 'A' && (c) <= 'Z')
#define lowerChar(c) (isCapitalChar(c) ? (c) - ('A' - 'a') : (c))
#define ArrayPair(k, v) [k] = (v)


int ChessInit_FromString(StringIndex fen);
#define ChessInit_Default() ChessInit_FromString(STRING("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));




// Public API for BIT_BOARD
bool Bitboard_Piece_IsSelectable(uint8_t index);

#define BitboardSet_Put(bitboardSet, index, side, type) \
	bitboardSet[side][type] |= MASK_INDEX(index)
#define BITBOARD_SET_PUT(bitboardSet, index, sideToken, typeToken) \
	BitboardSet_Put(bitboardSet, index, PIECE_SIDE_INDEX_##sideToken, PIECE_TYPE_INDEX_##typeToken) 

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

// Public API for BOARD
bool Piece_IsSelectable(uint8_t index);

PieceType PieceType_FromChar(char c);
PieceColor PieceColor_FromChar(char c);
#define Piece_GetType(index) (BOARD[index] & 0b111)
#define Piece_GetColor(index) ((BOARD[index] & 0b1000) >> 3)
#define Piece_HasType(index, type) (Piece_GetType(index) == (type))
#define Piece_HasColor(index, color) (Piece_GetColor(index) == (color))


// Strings
#define __STRING_LENGTH(s) ((sizeof(s) / sizeof((s)[0])) - sizeof((s)[0]))

#define __ENSURE_STRING_LITERAL(x) ("" x "")

// Note: If an error led you here, it's because STRING can only be used with string literals, i.e. STRING("SomeString") and not STRING(yourString)
#define STRING(string) (__INIT(StringIndex) { .chars = (string), .length = __STRING_LENGTH(__ENSURE_STRING_LITERAL(string)) })

#define STRING_CONST(string) { .chars = (string), .length = __STRING_LENGTH(__ENSURE_STRING_LITERAL(string)) }

#define __INIT(type) (type)


// Logging
#ifdef CHESS_VERBOSE
	#define err(do_after, msg, ...) { \
		printf("\033[31m[DEBUG]\033[0m Error: %s: %s: %d: %s: ", __FILE__, __PRETTY_FUNCTION__, __LINE__, msg); \
		printf("\033[32m"); \
		__VA_ARGS__; \
		printf("\033[0m"); \
		printf("\n"); \
		do_after; \
	} 
	#define err_var(fmt, name, ...) { \
		printf("%s = ", #name); \
		printf("'"); \
		printf(fmt, ##__VA_ARGS__, name); \
		printf("'"); \
		printf(": "); \
	}
	#define err_varn(fmt, name, ...) {err_var(fmt, name, ##__VA_ARGS__); printf("\n");}
	#define err_pause(...) do {__VA_ARGS__} while (getchar() != '\n') 
#else
	#define err(do_after, ...) do_after;
	#define err_var(...)
	#define err_varn(...)
	#define err_pause(...)
#endif // CHESS_VERBOSE

#ifdef CHESS_VERBOSE
	void Bitboard_Print(uint64_t bitboard);
	#define Bitboard_PrintType(typeToken) { \
		printf("%s:\n", #typeToken); \
		Bitboard_Print(MASK_TYPE(BITBOARD_SET, typeToken)); \
	}
	#define Bitboard_PrintSide(sideToken) { \
		printf("%s:\n", #sideToken); \
		Bitboard_Print(MASK_SIDE(BITBOARD_SET, sideToken)); \
	}
	#define Bitboard_PrintPiece(sideToken, typeToken) { \
		printf("%s.%s:\n", #sideToken, #typeToken); \
		Bitboard_Print(MASK_PIECE(BITBOARD_SET, sideToken, typeToken)); \
	}
#else
	#define Bitboard_PrintType(...)
	#define Bitboard_PrintSide(...) 
	#define Bitboard_PrintPiece(...)
#endif // CHESS_VERBOSE


#endif // CHESS_HEADER




#ifdef CHESS_BITBOARD_IMPLEMENTATION
#undef CHESS_BITBOARD_IMPLEMENTATION

Bitboards_All BITBOARD_SET;
PieceSideIndex ACTIVE_SIDE = PIECE_SIDE_INDEX_WHITE;
uint64_t MASK_EN_PASSANT_TARGET = MASK_EMPTY;

bool Bitboard_Piece_IsSelectable(uint8_t index)
{
	return Piece_HasSideIndex(BITBOARD_SET, index, ACTIVE_SIDE);
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

#ifdef CHESS_VERBOSE
void Bitboard_Print(uint64_t bitboard)
{
	for (uint8_t row = 0; row < 8; row++)
	{
		for (uint8_t col = 0; col < 8; col++)
		{
			uint8_t index = row * 8 + col;
			StringIndex token = MASK_INDEX(index) & bitboard
				? STRING("x") : STRING("-");
			printf("%3.*s", token.length, token.chars);
		}
		printf("\n");
	}
}
#endif // CHESS_VERBOSE

#endif // CHESS_BITBOARD_IMPLEMENTATION




#ifdef CHESS_IMPLEMENTATION
#undef CHESS_IMPLEMENTATION

CastlingSides CASTLING_SIDES = {0};
uint8_t BOARD[64];
PieceColor ACTIVE_COLOR = PIECE_COLOR_WHITE;
int8_t EN_PASSANT_TARGET_INDEX = -1;
int16_t HALF_MOVE_COUNTER = 0;
int16_t FULL_MOVE_COUNTER = 1;


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
