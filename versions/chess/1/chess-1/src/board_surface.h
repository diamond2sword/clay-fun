#include "clay.h"
#include "clay_helpers.h"




#ifndef BOARD_SURFACE_HEADER
#define BOARD_SURFACE_HEADER

#define BoardSurface_ColorBlack COLOR(180, 136, 97)
#define boardSurface_ColorWhite COLOR(239, 217, 180)
void BoardSurface(double width);

#endif // BOARD_SURFACE_HEADER




#ifdef BOARD_SURFACE_IMPLEMENTATION
#undef BOARD_SURFACE_IMPLEMENTATION

void BoardSurface(double width)
{
	const double cellWidth = width / 8;
	CLAY(CLAY_LAYOUT(SIZING_FIXED(width, width)), CLAY_RECTANGLE(boardSurface_ColorWhite), CLAY_FLOATING())
	for (uint8_t i = 0; i < 8; i++)
	for (uint8_t j = i % 2; j < 8; j+=2)
	CLAY(CLAY_LAYOUT(SIZING_FIXED(cellWidth, cellWidth)), CLAY_RECTANGLE(BoardSurface_ColorBlack), CLAY_FLOATING({.offset = {.x = i * cellWidth, .y = j * cellWidth}}));
}

#endif // BOARD_SURFACE_IMPLEMENTATION
