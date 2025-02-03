#include "chess_helpers.h.c"
#include "chess_typedefs.h.c"
#define CHESS_VERBOSE
#include "chess_err.h.c"

int main()
{
	bool x[2][2] = {{true, false}, {false, true}};
	bool y[2 * 2] = {true, false, true, false};
	err_varn("%d", ARRAY_2(bool, Equals, x, y));
	ARRAY_2(bool, Copy, x, y);
	err_varn("%d", ARRAY_2(bool, Equals, x, y));
	return 0;
}
