#include <stdint.h>

#ifndef TTT_HEADERS
#define TTT_HEADERS

typedef enum : uint8_t {
	ACTIVE_PLAYER_NONE,
	ACTIVE_PLAYER_X,
	ACTIVE_PLAYER_O
} ActivePlayer;

typedef enum : uint8_t {
	WIN_SLOPE_NONE,
	WIN_SLOPE_ROW,
	WIN_SLOPE_DIAG1,
	WIN_SLOPE_COL,
	WIN_SLOPE_DIAG2
} WinSlopeType;

typedef struct WinSlope {
	WinSlopeType slope1;
	WinSlopeType slope2;
} WinSlope;

uint8_t chk3(uint8_t init_index, uint8_t offset, uint8_t init_index_offset, uint8_t max_inits, ActivePlayer value);
void resetBoard();
void makeMove(int8_t cellIndex);

extern ActivePlayer BOARD[9];
extern ActivePlayer ACTIVE_PLAYER;
extern ActivePlayer WINNER;
extern WinSlope BOARD_WIN_SLOPE_MAP[9];
extern uint8_t MOVE_COUNTER;

#endif // TTT_HEADERS

#ifdef TTT_IMPLEMENTATION
#undef TTT_IMPLEMENTATION

ActivePlayer BOARD[9] = {0};
ActivePlayer ACTIVE_PLAYER = ACTIVE_PLAYER_X;
ActivePlayer WINNER = ACTIVE_PLAYER_NONE;
WinSlope BOARD_WIN_SLOPE_MAP[9] = {0};
uint8_t MOVE_COUNTER = 0;

uint8_t chk3(uint8_t init_index, uint8_t offset, uint8_t init_index_offset, uint8_t max_inits, ActivePlayer value)
{
	for (uint8_t i = 0; i < 9; i++) {
		BOARD_WIN_SLOPE_MAP[i] = (WinSlope) {0};
	}
	for (uint8_t inits = 0; inits < max_inits; inits++)
	{
		uint8_t cur_init_index = inits * init_index_offset;
		uint8_t is_win_line = 1;
		for (uint8_t cell_index = 0; cell_index < 3; cell_index++)
		{
			const uint8_t cur_index = init_index + cur_init_index + cell_index * offset;
			if (BOARD[cur_index] == value) continue;
			is_win_line = 0; break;
		}
		if (is_win_line)
		{
			WINNER = value;
			for (uint8_t cell_index = 0; cell_index < 3; cell_index++)
			{
				WinSlope * slope = BOARD_WIN_SLOPE_MAP + init_index + cell_index * offset;
				if (!slope->slope1) slope->slope1 = offset;
				else slope->slope2 = offset;
			}
		}
	}
	return 0;
}
void updateWinner(ActivePlayer value) {
	chk3(0, 1, 3, 3, value);
	chk3(2, 2, 0, 1, value);
	chk3(0, 3, 1, 3, value);
	chk3(0, 4, 0, 1, value);
}
void resetBoard() {
	for (int i = 0; i < 9; i++) BOARD[i] = 0;
	WINNER = ACTIVE_PLAYER_NONE;
	MOVE_COUNTER = 0;
	ACTIVE_PLAYER = ACTIVE_PLAYER_X;
}
void makeMove(int8_t cellIndex)
{
	if (WINNER || BOARD[cellIndex]) return;
	BOARD[cellIndex] = ACTIVE_PLAYER;
	updateWinner(ACTIVE_PLAYER);
	ACTIVE_PLAYER = ACTIVE_PLAYER == ACTIVE_PLAYER_X ? ACTIVE_PLAYER_O : ACTIVE_PLAYER_X;
	MOVE_COUNTER++;
}

#endif // TTT_IMPLEMENTATION
