#define CHESS_VERBOSE
#define CHESS_ARENA_SIZE (1 << 20)
#include "chess_fen.h.c"
#include "chess_make_move.h.c"
#include "chess_err.h.c"


Map_GameToHashKeys* Map_GameToHashKeys_Init(ChessArena* arena, uint64_t seed);
Game* Game_New(ChessArena* arena, Map_GameToHashKeys* map_gameToHashKeys);
void Game_Hash_XorPiece(Game* game, Piece piece);
void Game_Hash_XorSide(Game* game, PieceSideIndex side);
void Game_Hash_Compute(Game* game);
void Game_Hash_CopyToBegin(Game* game);
void Game_Hash_CopyToNext(Game* game);

Map_GameToHashKeys* MAP_GAME_TO_HASH_KEYS;
Game* GAME;
int main()
{
	ChessArena arena;
	ChessArena_Init(&arena);
	
	MAP_GAME_TO_HASH_KEYS = Map_GameToHashKeys_Init(&arena, 0x292bcd82efba82bfULL);

	GAME = Game_New(&arena, MAP_GAME_TO_HASH_KEYS);

	Game_FromFen_Default(GAME);
	Game_Hash_Compute(GAME);
	err_varn("%lu", *GAME->table_hash->current);
	Game_FromFen(GAME, STRING("6K1/Rp1Q4/7P/2r2Np1/p5P1/1pp4k/3P3b/n7 w - - 0 1"));
	Game_Hash_Compute(GAME);
	Game_Hash_CopyToNext(GAME);
	err_varn("%lu", *(-1 + GAME->table_hash->current));
	err_varn("%lu", *GAME->table_hash->current);
	Game_FromFen(GAME, STRING("6k1/8/3q2PR/1KP3Pp/P7/2Npp1p1/2n5/1N5b w - - 0 1"));
	Game_Hash_Compute(GAME);
	err_varn("%lu", *GAME->table_hash->current);
	Game_Hash_CopyToBegin(GAME);
	err_varn("%lu", *GAME->table_hash->begin);

	/*
	Game_FromFen(GAME, STRING("6K1/Rp1Q4/7P/2r2Np1/p5P1/1pp4k/3P3b/n7 w - - 0 1"));
	Game_Hash_Compute(GAME, MAP_GAME_TO_HASH_KEYS);
	err_varn("%lu", *GAME->table_hash->current);

	Game_FromFen(GAME, STRING("6k1/8/3q2PR/1KP3Pp/P7/2Npp1p1/2n5/1N5b w - - 0 1"));
	Game_Hash_Compute(GAME, MAP_GAME_TO_HASH_KEYS);
	err_varn("%lu", *GAME->table_hash->current);

	Game_FromFen_Default(GAME);
	Game_Hash_Compute(GAME, MAP_GAME_TO_HASH_KEYS);
	err_varn("%lu", *GAME->table_hash->current);

	Game_FromFen(GAME, STRING("6k1/8/3q2PR/1KP3Pp/P7/2Npp1p1/2n5/1N5b w - - 0 1"));
	Game_Hash_Compute(GAME, MAP_GAME_TO_HASH_KEYS);
	err_varn("%lu", *GAME->table_hash->current);

	Game_FromFen(GAME, STRING("6K1/Rp1Q4/7P/2r2Np1/p5P1/1pp4k/3P3b/n7 w - - 0 1"));
	Game_Hash_Compute(GAME, MAP_GAME_TO_HASH_KEYS);
	err_varn("%lu", *GAME->table_hash->current);
	*/

	ChessArena_Reset(&arena);
	return 0;
}

void Game_Hash_Compute(Game* game)
{
	*game->table_hash->current = MASK_EMPTY;
	// xor per piece at index
	for (uint8_t index = 0; index < 64; index++)
	{
		Piece piece = Piece_New(game->bitboardSet, index);
		if (piece.type == PIECE_TYPE_INDEX_NONE)
		{
			continue;
		}
		Game_Hash_XorPiece(game, piece);
	}
	{
		// xor side
		Game_Hash_XorSide(game, game->side_active);
	}
}

inline void Game_Hash_CopyToBegin(Game* game)
{
	uint64_t hash_temp = *game->table_hash->current;
	game->table_hash->current = game->table_hash->begin;
	*game->table_hash->current = hash_temp;
}

inline void Game_Hash_CopyToNext(Game* game)
{
	uint64_t hash_temp = *game->table_hash->current;
	++game->table_hash->current;
	*game->table_hash->current = hash_temp;
}

inline void Game_Hash_XorSide(Game* game, PieceSideIndex side)
{
	*game->table_hash->current
		^= game->map_gameToHashKeys->map_side[side];
}

inline void Game_Hash_XorPiece(Game* game, Piece piece)
{
	*game->table_hash->current
		^= game->map_gameToHashKeys->map_piece[piece.side][piece.type][piece.index];
}

Game* Game_New(ChessArena* arena, Map_GameToHashKeys* map_gameToHashKeys)
{
	Game* game;
	game = (Game*)ChessArena_Allocate(arena, sizeof(Game));
	{
		game->table_hash = (Table_Hash*)ChessArena_Allocate(arena, sizeof(Table_Hash));
		game->table_hash->end = game->table_hash->begin + MAX_REPEATABLE;
		game->table_hash->current = game->table_hash->begin;
	}
	{
		game->map_gameToHashKeys = map_gameToHashKeys;
	}
	return game;
}

Map_GameToHashKeys* Map_GameToHashKeys_Init(ChessArena* arena, uint64_t seed)
{
	static Map_GameToHashKeys* map_gameToHashKeys;
	map_gameToHashKeys = (Map_GameToHashKeys*)ChessArena_Allocate(arena, sizeof(Map_GameToHashKeys));

	tinymt64_t randomizer;
	tinymt64_init(&randomizer, seed);

	for (uint8_t side = 0; side < PIECE_SIDE_INDEX_COUNT; side++)
	for (uint8_t type = 0; type < PIECE_TYPE_INDEX_COUNT; type++)
	for (uint8_t index = 0; index < 64; index++)
	{
		map_gameToHashKeys->map_piece[side][type][index]
			= tinymt64_generate_uint64(&randomizer);
	}

	for (uint8_t side = 0; side < PIECE_SIDE_INDEX_COUNT; side++)
	{
		map_gameToHashKeys->map_side[side]
			= tinymt64_generate_uint64(&randomizer);
	}
	return map_gameToHashKeys;
}

