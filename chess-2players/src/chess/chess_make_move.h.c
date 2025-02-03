#pragma once
#include "chess_err.h.c"
#include "chess_helpers.h.c"
#include "chess_typedefs.h.c"
#include "chess_mask.h.c"
#include "chess_accessors.h.c"
#include "chess_arena.h.c"
#include "chess_sliders.h.c"
#define TINYMT64_IMPLEMENTATION
#include "tinymt64.h"




#define BITBOARD_SET_PUT(bitboardSet, index, sideToken, typeToken) \
	BitboardSet_Put(bitboardSet, index, PIECE_SIDE_INDEX_##sideToken, PIECE_TYPE_INDEX_##typeToken) 
#define BitboardSet_PutAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Put(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)

#define BITBOARD_SET_REMOVE(bitboardSet, index, sideToken, typeToken) \
	BitboardSet_Remove(bitboardSet, index, PIECE_SIDE_INDEX_##sideToken, PIECE_TYPE_INDEX_##typeToken) 
#define BitboardSet_RemoveAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Remove(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)
inline void BitboardSet_Put(BitboardSet* bitboardSet, uint8_t index, PieceSideIndex side, PieceTypeIndex type);
inline void BitboardSet_Remove(BitboardSet* bitboardSet, uint8_t index, PieceSideIndex side, PieceTypeIndex type);





void Game_MakeMove(Game* game, Move move, bool hash_mustUpdate);
void Chess__Game_Hash_Update(Game* game, Instruction* instruction, bool side_changed);
void Chess__Game_Hash_XorSide(Game* game, PieceSideIndex side);
void Chess__Game_Hash_XorPiece(Game* game, Piece piece);
inline void Game_Pieces_Update(Game* game, Instruction* instruction);
inline void Instruction_Append_PutPiece(Instruction* instruction, Piece piece);
inline void Instruction_Append_RemovePiece(Instruction* instruction, Piece piece);
inline void Instruction_Append_Piece(Instruction* instruction, PieceInstruction pieceInstruction);
Game* Game_New(ChessArena* arena, int (*set)(Game*));
inline void Game_Set(Game* game, int (*set)(Game*));
void Chess__Game_Hash_Compute(Game* game);
void Chess_Init(ChessArena* arena);
void Chess__Precompute_Map_GameToHashKeys(ChessArena* arena, uint64_t seed);


Map_GameToHashKeys* MAP_GAME_TO_HASH_KEYS;
bool CHESS__IS_INITIALIZED = false;


void Game_MakeMove(Game* game, Move move, bool hash_mustUpdate)
{
	bool enPassantTarget_isSet_thisFrame = false;
	bool enPassantTarget_hasAttacker = false;
	uint64_t mask_castlingRights_temp = game->mask_castlingRights;
	Instruction instruction = (Instruction){};
	{
		// make move
		Instruction_Append_RemovePiece(&instruction, move.src);
		if (move.dst.type != PIECE_TYPE_INDEX_NONE)
		{
			Instruction_Append_RemovePiece(&instruction, move.dst);
		}
		Instruction_Append_PutPiece(&instruction, (Piece){move.dst.index, move.src.side, move.src.type});
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
				{
					uint8_t col_dst = Index_AsCol(move.dst.index);
					uint8_t row_dst = Index_AsRow(move.dst.index);
					uint64_t mask_enemy_pawns = Mask_Piece(game->bitboardSet, SIDE_NEGATE(move.src.side), PIECE_TYPE_INDEX_PAWN);
					uint64_t mask_attackers = (MASK_COL(col_dst - 1) | MASK_COL(col_dst + 1)) & MASK_ROW(row_dst);
					enPassantTarget_hasAttacker = mask_enemy_pawns & mask_attackers;
				}
			}
			// en passant pawn deletion
			else if (move.dst.index == game->index_enPassantTarget)
			{
				Instruction_Append_RemovePiece(&instruction, Piece_New(game->bitboardSet, move.dst.index + OFFSET_BACKWARD(move.src.side)));
			}
			// pawn promotion
			else if (MASK_INDEX(move.dst.index) & Direct_Rows(move.src.side, MASK_ROW_0))
			{
				Instruction_Append_RemovePiece(&instruction, (Piece){move.dst.index, move.src.side, move.src.type});
				Instruction_Append_PutPiece(&instruction, (Piece){move.dst.index, move.src.side, move.type_promotion});
			}
		}
		else if (move.src.type == PIECE_TYPE_INDEX_KING)
		{
			// untoggle castling rights
			int8_t col_king = 4;
			uint8_t col_dst = Index_AsCol(move.dst.index);
			int8_t offset_x = IF_IN_RANGE(col_dst, 0, 3, -1, 1);
			int8_t col_rook = IF_IN_RANGE(col_dst, 0, 3, 0, 7);
			int8_t row = Index_AsRow(move.src.index);
			if (MASK_INDEX(move.dst.index) & MASK_COL(col_king + offset_x * 2))
			//&& (MASK_COL(col_rook) & MASK_ROW(row) & game->mask_castlingRights))
			{
				Instruction_Append_RemovePiece(&instruction, (Piece){row * OFFSET_DOWN + col_rook, move.src.side, PIECE_TYPE_INDEX_ROOK});
				Instruction_Append_PutPiece(&instruction, (Piece){move.src.index + offset_x, move.src.side, PIECE_TYPE_INDEX_ROOK});
			}
			game->mask_castlingRights &= ~Direct_Rows(move.src.side, MASK_ROW(7));

		}
		else if (move.src.type == PIECE_TYPE_INDEX_ROOK)
		{
			game->mask_castlingRights &= ~MASK_INDEX(move.src.index);
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
		bool moves_past_areRepeatable = move.src.type != PIECE_TYPE_INDEX_PAWN
			&& move.dst.type == PIECE_TYPE_INDEX_NONE
			&& game->mask_castlingRights == mask_castlingRights_temp;
		// update counter of repeatable positions
		if (moves_past_areRepeatable)
		{
			game->counter_repeatable++;
		}
		// check if there's en passant move, if so, the resulting position won't happen again, so is incomparable anymore, so 0
		else if (enPassantTarget_hasAttacker)
		{
			game->counter_repeatable = 0;
		}
		else
		{
			game->counter_repeatable = 1;
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
		Game_Pieces_Update(game, &instruction);
		// Switch side
		game->side_active = SIDE_NEGATE(game->side_active);
	}
	if (hash_mustUpdate) {
		Chess__Game_Hash_Update(game, &instruction, true);
	}
}

void Chess__Game_Hash_Update(Game* game, Instruction* instruction, bool side_changed)
{
	{
		uint64_t hash_temp = *game->list_hash->current;
		game->list_hash->current++;
		*game->list_hash->current = hash_temp;
	}
	for (uint8_t i = 0; i < instruction->length; i++)
	{
		Chess__Game_Hash_XorPiece(game, instruction->pieces[i].piece);
	}
	if (side_changed)
	{
		Chess__Game_Hash_XorSide(game, instruction->move.src.side);
	}
}

inline void Game_Pieces_Update(Game* game, Instruction* instruction)
{
	for (uint8_t i = 0; i < instruction->length; i++)
		instruction->pieces[i].type == PIECE_INSTRUCTION_TYPE_PUT
			? BitboardSet_PutAttrs(game->bitboardSet, instruction->pieces[i].piece, index, side, type)
			: BitboardSet_RemoveAttrs(game->bitboardSet, instruction->pieces[i].piece, index, side, type);
}

inline void Instruction_Append_PutPiece(Instruction* instruction, Piece piece)
{
	Instruction_Append_Piece(instruction, (PieceInstruction){piece, PIECE_INSTRUCTION_TYPE_PUT});
}

inline void Instruction_Append_RemovePiece(Instruction* instruction, Piece piece)
{
	Instruction_Append_Piece(instruction, (PieceInstruction){piece, PIECE_INSTRUCTION_TYPE_REMOVE});
}

inline void Instruction_Append_Piece(Instruction* instruction, PieceInstruction pieceInstruction)
{
	err_scope(if (instruction->length == MAX_PIECE_INSTRUCTION)
	{
		err_varn("%s", "out of memory");
	});
	instruction->pieces[instruction->length] = pieceInstruction; 
	++instruction->length;
}


inline void BitboardSet_Put(BitboardSet* bitboardSet, uint8_t index, PieceSideIndex side, PieceTypeIndex type)
{
	bitboardSet->pieces[side][type] |= MASK_INDEX(index);
}

inline void BitboardSet_Remove(BitboardSet* bitboardSet, uint8_t index, PieceSideIndex side, PieceTypeIndex type)
{
	bitboardSet->pieces[side][type] &= ~MASK_INDEX(index);
}

Game* Game_New(ChessArena* arena, int (*set)(Game*))
{
	err_scope(if (!CHESS__IS_INITIALIZED) 
	{
		err(return 0, "chess is not initialized");
	});
	Game* game = (Game*)ChessArena__Allocate(arena, sizeof(Game));
	{
		game->bitboardSet = (BitboardSet*)ChessArena__Allocate(arena, sizeof(BitboardSet));
	}
	{
		game->list_hash = (List_Hash*)ChessArena__Allocate(arena, sizeof(List_Hash));
		game->list_hash->end = game->list_hash->begin + MAX_REPEATABLE;
		Array_uint64_t_Set(game->list_hash->begin, game->list_hash->end, MASK_EMPTY);
		game->list_hash->current = game->list_hash->begin;
		Game_Set(game, set);
	}
	return game;
}

inline void Game_Set(Game* game, int (*set)(Game*))
{
	set(game);
	Chess__Game_Hash_Compute(game);
}

void Chess__Game_Hash_Compute(Game* game)
{
	*game->list_hash->current = MASK_EMPTY;
	// xor per piece at index
	for (uint8_t index = 0; index < 64; index++)
	{
		Piece piece = Piece_New(game->bitboardSet, index);
		if (piece.type == PIECE_TYPE_INDEX_NONE)
		{
			continue;
		}
		Chess__Game_Hash_XorPiece(game, piece);
	}
	{
		// xor side
		Chess__Game_Hash_XorSide(game, game->side_active);
	}
}

inline void Chess__Game_Hash_XorSide(Game* game, PieceSideIndex side)
{
	*game->list_hash->current
		^= MAP_GAME_TO_HASH_KEYS->side;
}

inline void Chess__Game_Hash_XorPiece(Game* game, Piece piece)
{
	*game->list_hash->current
		^= MAP_GAME_TO_HASH_KEYS->map_piece[piece.side][piece.type][piece.index];
}

void Chess_Init(ChessArena* arena)
{
	CHESS__IS_INITIALIZED = true;
	// precomputes
	Chess__Precompute_Map_GameToHashKeys(arena, 0x292bcd82efba82bfULL);
	Chess__Precompute_SlidingPiece_RayIndexes(arena);
}

void Chess__Precompute_Map_GameToHashKeys(ChessArena* arena, uint64_t seed)
{
	MAP_GAME_TO_HASH_KEYS = (Map_GameToHashKeys*)ChessArena__Allocate(arena, sizeof(Map_GameToHashKeys));

	tinymt64_t randomizer;
	tinymt64_init(&randomizer, seed);

	{
		for (uint8_t side = 0; side < PIECE_SIDE_INDEX_COUNT; side++)
		for (uint8_t type = 0; type < PIECE_TYPE_INDEX_COUNT; type++)
		for (uint8_t index = 0; index < 64; index++)
		{
			MAP_GAME_TO_HASH_KEYS->map_piece[side][type][index]
				= tinymt64_generate_uint64(&randomizer);
		}
	}

	MAP_GAME_TO_HASH_KEYS->side = tinymt64_generate_uint64(&randomizer);
}
