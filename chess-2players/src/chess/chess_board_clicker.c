#define CHESS_VERBOSE
#define CHESS_IMPLEMENTATION
#include "../chess.h.c"


Game* GAME;
BoardClicker* BOARD_CLICKER;
Map_GameToHashKeys* MAP_GAME_TO_HASH_KEYS;

int main()
{
	ChessArena arena;
	ChessArena_Init(&arena);
	MAP_GAME_TO_HASH_KEYS = Map_GameToHashKeys_New(&arena, 0x292bcd82efba82bfULL);
	GAME = Game_New(&arena, MAP_GAME_TO_HASH_KEYS);
	BOARD_CLICKER = BoardClicker_New(&arena, GAME);
	Game_FromFen_Default(GAME);
	Game_Hash_Compute(GAME);
	BoardClicker_Phase_Reset(BOARD_CLICKER, true);
	
	Bitboard_PrintMask(BOARD_CLICKER->bitboard_movables);

	Precompute_SlidingPiece_RayIndexes();

	return 0;

}
