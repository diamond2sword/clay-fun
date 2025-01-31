#include <stdint.h>
#include <stdbool.h>

#pragma once

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

typedef struct Piece {
	uint8_t index;
	PieceSideIndex side;
	PieceTypeIndex type;
} Piece;

typedef struct Move {
	Piece src;
	Piece dst;
	PieceTypeIndex type_promotion;
} Move;

#define MAX_REPEATABLE 128

typedef uint64_t Map_PieceToHashKey[PIECE_SIDE_INDEX_COUNT][PIECE_TYPE_INDEX_COUNT][64];

typedef struct Map_GameToHashKeys {
	Map_PieceToHashKey map_piece;
	uint64_t side;
} Map_GameToHashKeys;

typedef struct Table_Hash {
	uint64_t begin[MAX_REPEATABLE];
	uint64_t* end;
	uint64_t* current;
} Table_Hash;

typedef uint64_t Bitboards_Side[PIECE_TYPE_INDEX_COUNT];
typedef Bitboards_Side Bitboards_All[PIECE_SIDE_INDEX_COUNT];

typedef struct Game {
	Bitboards_All* bitboardSet;
	Table_Hash* table_hash;
	Map_GameToHashKeys* map_gameToHashKeys;
	uint64_t mask_castlingRights;
	uint16_t counter_fullMove;
	uint16_t counter_halfMove;
	uint16_t counter_repeatable;
	int8_t index_enPassantTarget;
	PieceSideIndex side_active;
} Game;


typedef enum PieceInstructionType : uint8_t {
	PIECE_INSTRUCTION_TYPE_REMOVE,
	PIECE_INSTRUCTION_TYPE_PUT
} PieceInstructionType;

typedef struct PieceInstruction {
	Piece piece;
	PieceInstructionType type;
} PieceInstruction;

typedef enum InstructionType : uint8_t {
	INSTRUCTION_TYPE_NONE,
	INSTRUCTION_TYPE_PAWN_DOUBLE_FORWARD,
	INSTRUCTION_TYPE_PAWN_EN_PASSANT_CAPTURE,
	INSTRUCTION_TYPE_PAWN_PROMOTION,
	INSTRUCTION_TYPE_KING_STEP,
	INSTRUCTION_TYPE_KING_CASTLE,
	INSTRUCTION_TYPE_ROOK_FIRST_MOVE
} InstructionType;

#define MAX_PIECE_INSTRUCTION 5

typedef struct Instruction { 
	PieceInstruction pieces[MAX_PIECE_INSTRUCTION];
	Move move;
	uint8_t length;
	InstructionType type;
} Instruction;
