#include "chess_arena.h.c"
#include "chess_mask.h.c"
#include "chess_helpers.h.c"

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#pragma once

#define PIECE_RAY_INDEX_CARDINALS \
	PIECE_RAY_INDEX_NORTH, \
	PIECE_RAY_INDEX_SOUTH, \
	PIECE_RAY_INDEX_WEST, \
	PIECE_RAY_INDEX_EAST


#define PIECE_RAY_INDEX_INTERCARDINALS \
	PIECE_RAY_INDEX_NORTHWEST, \
	PIECE_RAY_INDEX_NORTHEAST, \
	PIECE_RAY_INDEX_SOUTHWEST, \
	PIECE_RAY_INDEX_SOUTHEAST

#define PIECE_RAY_INDEX_OCTANTS \
	PIECE_RAY_INDEX_CARDINALS, \
	PIECE_RAY_INDEX_INTERCARDINALS \

typedef enum PieceRayIndex : uint8_t {
	PIECE_RAY_INDEX_OCTANTS,
	PIECE_RAY_INDEX_COUNT,
	PIECE_RAY_INDEX_NONE,
} PieceRayIndex;

typedef uint64_t Map_DirectionToRayMask[PIECE_RAY_INDEX_COUNT][64];

void Precompute_SlidingPiece_RayIndexes(ChessArena* arena);
uint64_t Mask_Attacks_Sliding(uint64_t mask_occupied, uint8_t index, int32_t ray_count, ...);

const uint8_t MAP_RAY_TO_OFFSET[PIECE_RAY_INDEX_COUNT] = {
	ArrayPair(PIECE_RAY_INDEX_NORTH, OFFSET_UP),
	ArrayPair(PIECE_RAY_INDEX_SOUTH, OFFSET_DOWN),
	ArrayPair(PIECE_RAY_INDEX_WEST, OFFSET_LEFT),
	ArrayPair(PIECE_RAY_INDEX_EAST, OFFSET_RIGHT),
	ArrayPair(PIECE_RAY_INDEX_NORTHWEST, OFFSET_UP + OFFSET_LEFT),
	ArrayPair(PIECE_RAY_INDEX_NORTHEAST, OFFSET_UP + OFFSET_RIGHT),
	ArrayPair(PIECE_RAY_INDEX_SOUTHWEST, OFFSET_DOWN + OFFSET_LEFT),
	ArrayPair(PIECE_RAY_INDEX_SOUTHEAST, OFFSET_DOWN + OFFSET_RIGHT)
};
const bool MAP_RAY_MUST_BIT_SCAN_FORWARD[PIECE_RAY_INDEX_COUNT] = {
	ArrayPair(PIECE_RAY_INDEX_NORTH, false),
	ArrayPair(PIECE_RAY_INDEX_SOUTH, true),
	ArrayPair(PIECE_RAY_INDEX_WEST, false),
	ArrayPair(PIECE_RAY_INDEX_EAST, true),
	ArrayPair(PIECE_RAY_INDEX_NORTHWEST, false),
	ArrayPair(PIECE_RAY_INDEX_NORTHEAST, false),
	ArrayPair(PIECE_RAY_INDEX_SOUTHWEST, true),
	ArrayPair(PIECE_RAY_INDEX_SOUTHEAST, true)
};

Map_DirectionToRayMask* MAP_DIRECTION_TO_RAY_MASK;

void Precompute_SlidingPiece_RayIndexes(ChessArena* arena)
{
	MAP_DIRECTION_TO_RAY_MASK = (Map_DirectionToRayMask*)ChessArena_Allocate(arena, sizeof(Map_DirectionToRayMask));
	for (uint8_t ray = 0; ray < PIECE_RAY_INDEX_COUNT; ray++)
	{
		int8_t offset = MAP_RAY_TO_OFFSET[ray];
		int8_t offset_col = Index_AsCol(OFFSET_DOWN + OFFSET_RIGHT + offset) - 1;
		for (uint8_t i = 0; i < 64; i++)
		{
			uint64_t mask = MASK_EMPTY;
			int8_t index_this = i;
			int8_t col_this = Index_AsCol(i);
			while (IN_RANGE(index_this, 0, 63) && IN_RANGE(col_this, 0, 7))
			{
				mask |= MASK_INDEX(index_this);
				index_this += offset;
				col_this += offset_col;
			}
			{
				(*MAP_DIRECTION_TO_RAY_MASK)[ray][i] = mask & ~MASK_INDEX(i);
			}
		}
	}
}

uint64_t Mask_Attacks_Sliding(uint64_t mask_occupied, uint8_t index, int32_t ray_count, ...)
{
	va_list args;
	va_start(args, ray_count);
	uint64_t mask_attacks = MASK_EMPTY;
	for (uint8_t i = 0; i < ray_count; i++)
	{
		PieceRayIndex ray = va_arg(args, uint32_t);
		uint64_t mask_rays_this	= (*MAP_DIRECTION_TO_RAY_MASK)[ray][index];
		mask_attacks |= mask_rays_this;
		uint64_t mask_matches = mask_rays_this & mask_occupied;
		if (mask_matches)
		{
			uint8_t index_blocker = MAP_RAY_MUST_BIT_SCAN_FORWARD[ray]  ? BitScanForward(mask_matches) : BitScanReverse(mask_matches);
			mask_attacks &= ~(*MAP_DIRECTION_TO_RAY_MASK)[ray][index_blocker];
		}
	}
	return mask_attacks;
}
