/*
#define CHESS_MAIN
#define CHESS_IMPLEMENTATION
#define CHESS_VERBOSE
*/

#include <stdint.h>
#include <stdbool.h>


#ifndef CHESS_HEADER
#define CHESS_HEADER

#endif // CHESS_HEADER



#ifdef CHESS_IMPLEMENTATION
#undef CHESS_IMPLEMENTATION
#include "math.h.c"
#include "chess_typedefs.h.c"
#include "chess_make_move.h.c"
#include "chess_mask_attacks.h.c"
#include "chess_sliders.h.c"
#include "chess_accessors.h.c"
#include "chess_mask.h.c"
#include "chess_helpers.h.c"
#include "chess_fen.h.c"
#include "chess_err.h.c"
#include "chess_arena.h.c"

#endif // CHESS_IMPLEMENTATION




#ifdef CHESS_MAIN
#undef CHESS_MAIN
int main(int argc, char * argv[])
{
	ChessArena arena;
	ChessArena_Init(&arena);
	Game* game = (Game*)ChessArena_Allocate(&arena, sizeof(Game));
	if (argc == 1)
	{
		return Game_New(game);
	}
	else
	{
		return Game_New_FromFen(game, (StringIndex){argv[1], Stringlength(argv[1])});
	}
}
#endif // CHESS_MAIN
