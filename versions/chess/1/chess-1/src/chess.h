#include <stdint.h>
#include <stdbool.h>

#ifndef CHESS_HEADER
#define CHESS_HEADER

#ifdef CHESS_VERBOSE
	#include <stdio.h>
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
#else
	#define err(do_after, ...) do_after;
#endif // CHESS_VERBOSE


#define __STRING_LENGTH(s) ((sizeof(s) / sizeof((s)[0])) - sizeof((s)[0]))

#define __ENSURE_STRING_LITERAL(x) ("" x "")

// Note: If an error led you here, it's because STRING can only be used with string literals, i.e. STRING("SomeString") and not STRING(yourString)
#define STRING(string) (__INIT(StringIndex) { .chars = (string), .length = __STRING_LENGTH(__ENSURE_STRING_LITERAL(string)) })

#define STRING_CONST(string) { .chars = (string), .length = __STRING_LENGTH(__ENSURE_STRING_LITERAL(string)) }

#define __INIT(type) (type)


typedef enum PieceType : uint8_t {
	PIECE_NONE,
	PIECE_KING,
	PIECE_QUEEN,
	PIECE_BISHOP,
	PIECE_KNIGHT,
	PIECE_ROOK,
	PIECE_PAWN
} PieceType;

typedef enum PieceColor : uint8_t {
	PIECE_WHITE = 0,
	PIECE_BLACK = 8
} PieceColor;

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

extern uint8_t BOARD[64];
extern CastlingSides CASTLING_SIDES;
extern bool IS_WHITE_TO_MOVE;
extern int8_t EN_PASSANT_INDEX;
extern int16_t HALF_MOVE_COUNTER;
extern int16_t FULL_MOVE_COUNTER;

PieceType PieceType_FromChar(char c);
PieceColor PieceColor_FromChar(char c);
bool ContainsChar(const char c, StringIndex string);
uint32_t CountChar(const char c, StringIndex string);
uint32_t Stringlength(const char * string);
int ChessInit_FromString(StringIndex fen);
#define ChessInit_Default() ChessInit_FromString(STRING("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));

#endif // CHESS_HEADER


#ifdef CHESS_IMPLEMENTATION
#undef CHESS_IMPLEMENTATION

CastlingSides CASTLING_SIDES = {0};
uint8_t BOARD[64] = {0};
bool IS_WHITE_TO_MOVE = true;
int8_t EN_PASSANT_INDEX = -1;
int16_t HALF_MOVE_COUNTER = 0;
int16_t FULL_MOVE_COUNTER = 1;

PieceType PieceType_FromChar(char c)
{
	if (c >= 'A' && c <= 'Z') {
		c += 'a' - 'A';
	}
	switch (c) {
	case 'k': return PIECE_KING;
	case 'q': return PIECE_QUEEN;
	case 'b': return PIECE_BISHOP;
	case 'n': return PIECE_KNIGHT;
	case 'r': return PIECE_ROOK;
	case 'p': return PIECE_PAWN;
	default: return PIECE_NONE;
	}
}

PieceColor PieceColor_FromChar(char c)
{
	return (PieceColor)((c >= 'A' && c <= 'Z') << 3);
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
			BOARD[i] = PIECE_NONE;
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
			if (PieceType_FromChar(c) == PIECE_NONE) {
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
		IS_WHITE_TO_MOVE = c == 'w';
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
	err_varn("%d", IS_WHITE_TO_MOVE);
	err_varn("%d", CASTLING_SIDES.blackKing);
	err_varn("%d", CASTLING_SIDES.blackQueen);
	err_varn("%d", CASTLING_SIDES.whiteKing);
	err_varn("%d", CASTLING_SIDES.whiteQueen);
	err_varn("%d", EN_PASSANT_INDEX);
	err_varn("%d", HALF_MOVE_COUNTER);
	err_varn("%d", FULL_MOVE_COUNTER);
	#endif // CHESS_VERBOSE
	
	return 0;
}


#endif

#ifdef CHESS_MAIN
#undef CHESS_MAIN
int main(int argc, char * argv[])
{
	return ChessInit_FromString((StringIndex){argv[1], Stringlength(argv[1])});	
}
#endif // CHESS_MAIN
