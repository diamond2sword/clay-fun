#include <stdint.h>
#include <stdio.h>

typedef enum : uint8_t {
	ACTIVE_PLAYER_NONE,
	ACTIVE_PLAYER_X,
	ACTIVE_PLAYER_O
} ActivePlayer;

ActivePlayer BOARD[9] = {0};
ActivePlayer ACTIVE_PLAYER = ACTIVE_PLAYER_X;
ActivePlayer WINNER = ACTIVE_PLAYER_NONE;

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

WinSlope BOARD_WIN_SLOPE_MAP[9] = {0};

uint8_t chk3(uint8_t init_index, uint8_t offset, uint8_t init_index_offset, uint8_t max_inits, ActivePlayer value)
{
	for (uint8_t i = 0; i < 9; i++) {
		BOARD_WIN_SLOPE_MAP[i] = (WinSlope) {0};
	}
	for (uint8_t inits = 0; inits < max_inits; inits++)
	{
		uint8_t cur_init_index = inits * init_index_offset;
		uint8_t is_win_line = 1;
		printf("    info: {slope: %d, init_index: %d}:   ", offset, init_index);
		for (uint8_t cell_index = 0; cell_index < 3; cell_index++)
		{
			const uint8_t cur_index = init_index + cur_init_index + cell_index * offset;
			if (BOARD[cur_index] == value)
			{
				printf("%d", cur_index);
				continue;
			}
			is_win_line = 0; break;
		}
		printf("\n");
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

void updateWinner() {
	chk3(0, 1, 3, 3, ACTIVE_PLAYER);
	chk3(2, 2, 0, 1, ACTIVE_PLAYER);
	chk3(0, 3, 1, 3, ACTIVE_PLAYER);
	chk3(0, 4, 0, 1, ACTIVE_PLAYER);
}

void mark(uint8_t row, uint8_t col)
{
	const uint8_t index = row * 3 + col;
	char msg_token = 'S';
	if (WINNER)
	{
		printf("WINNER: %d\n", WINNER);
		return;
	}
	if (BOARD[index] == ACTIVE_PLAYER_NONE)
	{
		BOARD[index] = ACTIVE_PLAYER;
		updateWinner();
		msg_token = 'S';
	}
	else
	{
		msg_token = 'E';
	}
	printf("%c: BOARD[%d] = %d, input: {index: %d, value: %d}, winner = %d\n",
		msg_token, index, BOARD[index], index, ACTIVE_PLAYER, WINNER);
	ACTIVE_PLAYER = ACTIVE_PLAYER == ACTIVE_PLAYER_X ? ACTIVE_PLAYER_O : ACTIVE_PLAYER_X;
}


int main() {
	mark(0, 2); mark(0, 0);
	mark(1, 1); mark(2, 1);
	mark(2, 0); mark(1, 2);
	mark(1, 0); mark(2, 0);
	mark(2, 2);
	return 0;
}
