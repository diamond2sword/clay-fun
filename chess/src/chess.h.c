#define CHESS_MAIN
#define CHESS_IMPLEMENTATION
#define CHESS_VERBOSE

#include <stdint.h>
#include <stdbool.h>


#ifndef CHESS_HEADER
#define CHESS_HEADER

#include "math.h.c"
#include "chess_typedefs.h.c"
#include "chess_game.h.c"
#include "chess_sliders.h.c"
#include "chess_accessors.h.c"
#include "chess_mask.h.c"
#include "chess_mask_attacks.h.c"
#include "chess_helpers.h.c"





// Public API for BIT_BOARD









#endif // CHESS_HEADER



#ifdef CHESS_IMPLEMENTATION
#undef CHESS_IMPLEMENTATION

Bitboards_All BITBOARD_SET;
PieceSideIndex ACTIVE_SIDE = PIECE_SIDE_INDEX_WHITE;
CastlingRights CASTLING_RIGHTS;
int8_t EN_PASSANT_TARGET_INDEX = -1;
uint16_t HALF_MOVE_COUNTER = 0;
uint16_t FULL_MOVE_COUNTER = 1;


MASK_INDEX_VA_ARGS(Mask_Rows, MASK_ROW);
MASK_INDEX_VA_ARGS(Mask_Cols, MASK_COL);

#endif // CHESS_IMPLEMENTATION




#ifdef CHESS_MAIN
#undef CHESS_MAIN
#include "chess_fen.h.c"
int main(int argc, char * argv[])
{
	return ChessInit_FromString((StringIndex){argv[1], Stringlength(argv[1])}, CHESS_INIT_DEFAULT_PARAMS);	
}
#endif // CHESS_MAIN
