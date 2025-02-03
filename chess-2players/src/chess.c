#define CHESS_IMPLEMENTATION
#define CHESS_VERBOSE
#include "chess.h.c"

void click(BoardClicker* boardClicker, uint8_t x, uint8_t y)
{
	if (IN_RANGE(x, 0, 7) && IN_RANGE(y, 0, 7))
	{
		boardClicker->index_input = y * 8 + x;
		BoardClicker_Click(boardClicker);
	}
}

BoardClicker* BOARD_CLICKER;
int main(int argc, char * argv[])
{
	ChessArena arena;
	ChessArena_Init(&arena);
	Chess_Init(&arena);
	Game* game = Game_New(&arena, Game_Set_FromFen_Default);
	BOARD_CLICKER = BoardClicker_New(&arena, game);

	click(BOARD_CLICKER, 0, 6);
	Bitboard_PrintMask(BOARD_CLICKER->bitboard_attacks);
	click(BOARD_CLICKER, 0, 5);
	click(BOARD_CLICKER, 0, 1);
	click(BOARD_CLICKER, 0, 2);
	Bitboard_PrintMask(BOARD_CLICKER->bitboard_movables);
	
	ChessArena_Reset(&arena);
	return 0;
}
