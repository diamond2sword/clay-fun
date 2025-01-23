#include <stdarg.h>
#include <stdint.h>
#define CLAY_HELPERS_IMPLEMENTATION
#include "clay_helpers.h"

#define CLAY_IMPLEMENTATION
#include "web_clay.h"

#define CLAY_CLICK_HANDLER_IMPLEMENTATION
#include "clay_click_handler.h"
/*
#define BOARD_SURFACE_IMPLEMENTATION
#include "board_surface.h"
*/
#define CHESS_IMPLEMENTATION
#define CHESS_BITBOARD_IMPLEMENTATION
#include "chess.h"

#define MY_MATH_IMPLEMENTATION
#include "math.h"


double windowWidth, windowHeight;
double windowSmallSide, windowLongSide;
bool isMobileScreen;
float animationLerpValue;
bool debugModeEnabled;
const uint32_t FONT_ID_TEXT;

double BOARD_WIDTH;
double BOARD_CELL_WIDTH;


//bool (* IsCellSelectableFunc)(uint8_t index) = Piece_IsSelectable;
bool (* IsCellSelectableFunc)(uint8_t index) = Bitboard_Piece_IsSelectable;

typedef struct Piece {
	uint8_t index;
	PieceSideIndex side;
	PieceTypeIndex type;
} Piece;

Piece Piece_New(Bitboards_All bitboardSet, uint8_t index)
{
	return (Piece){index,
		Piece_GetSideIndex(bitboardSet, index),
		Piece_GetTypeIndex(bitboardSet, index)};
}

typedef struct Move {
	Piece src;
	Piece dst;
} Move;

Move Move_New(Bitboards_All bitboardSet, uint8_t srcIndex, uint8_t dstIndex)
{
	return (Move){
		Piece_New(bitboardSet, srcIndex),
		Piece_New(bitboardSet, dstIndex)};
}

typedef enum PiecePromotionIndex : uint8_t {
	PIECE_PROMOTION_INDEX_QUEEN,
	PIECE_PROMOTION_INDEX_ROOK,
	PIECE_PROMOTION_INDEX_BISHOP,
	PIECE_PROMOTION_INDEX_KNIGHT,
	PIECE_PROMOTION_INDEX_NONE,
	PIECE_PROMOTION_INDEX_COUNT,
} PiecePromotionIndex;

Move RECENT_MOVE = {0};

int8_t SRC_CELL_INDEX = -1;
uint64_t MOVES_BITBOARD = MASK_EMPTY;
uint64_t MOVABLES_BITBOARD = MASK_EMPTY;
bool Piece_IsMove(uint8_t index)
{
/*
//	uint64_t srcMask = 1ULL << SRC_CELL_INDEX;
	bool isBlack = Piece_HasSideIndex(SRC_CELL_INDEX, PIECE_SIDE_INDEX_BLACK);
//	uint64_t unmovedRowMask = Piece_RowMask(isWhite ? 6 : 1);
//	uint64_t isUnmoved = (unmovedRowMask >> SRC_CELL_INDEX) & 0b1;
//	bool isUnmoved = (unmovedMask >> index) & 0b1;
	int8_t forwardSign = isBlack ? 1 : -1;

	uint8_t forwardOnceIndex
		= SRC_CELL_INDEX + 8 * forwardSign;
	bool canForwardOnce
		= Piece_HasTypeIndex(forwardOnceIndex, PIECE_TYPE_INDEX_NONE);
	bool isForwardOnce
		= canForwardOnce && index == forwardOnceIndex;

	uint8_t forwardTwiceIndex
		= SRC_CELL_INDEX + 16 * forwardSign;
	bool canForwardTwice
		= canForwardOnce && Piece_HasTypeIndex(forwardTwiceIndex, PIECE_TYPE_INDEX_NONE);
	bool isForwardTwice
		= canForwardTwice && index == forwardTwiceIndex;
//	uint64_t forwardOneMask = 1ULL << (SRC_CELL_INDEX - 8);
//	uint64_t forwardTwoMask = isUnmoved << (SRC_CELL_INDEX - 16);
	uint64_t occupied = MASK_ALL;
	uint64_t single_step = MASK_INDEX(index) & (MASK_INDEX(SRC_CELL_INDEX + DIRECT(ACTIVE_SIDE, OFFSET_UP)));
	uint64_t double_push =  single_push & MASK_INDEX(SRC_CELL_INDEX + DIRECT(ACTIVE_SIDE, OFFSET_UP));
*/
	return MASK_INDEX(index) & MOVES_BITBOARD;
//	return (pawn_moves >> index) & 0b1;
}
bool IsNoneMove(uint8_t index)
{
	return false;
}
/*
bool (*Piece_GetMoveChecker(uint8_t index))(uint8_t)
{
	switch (Piece_GetTypeIndex(index)) {
	case PIECE_TYPE_INDEX_PAWN: return;
	default: return IsNoneMove;
	}
}
*/
#define OFFSET_FORWARD DIRECT_OFFSET_SIGN(ACTIVE_SIDE) * OFFSET_UP
#define OFFSET_UP -8
#define OFFSET_DOWN 8
#define OFFSET_LEFT -1
#define MASK_ROW_0 0xFFULL
#define MASK_COL_0 0x0101010101010101ULL

#define IN_RANGE(x, min, max) ((x) >= (min) && (x) <= (max))
#define IF_IN_RANGE(x, min, max, forTrue, forFalse) (IN_RANGE((x), (min), (max)) ? (forTrue) : (forFalse))
#define IF_SIDE(side, forNone, forWhite, forBlack) ((side) == PIECE_SIDE_INDEX_NONE ? (forNone) : (side) == PIECE_SIDE_INDEX_WHITE ? (forWhite) : (forBlack))
	
#define Index_AsRow(index) (uint8_t)floorf((index) / 8.f)
#define Index_AsCol(index) ((index) % 8)

#define MASK_ROW(row) IF_IN_RANGE(row, 0, 7, MASK_ROW_0 << ((row) * 8), MASK_EMPTY)
#define MASK_COL(col) IF_IN_RANGE(col, 0, 7, MASK_COL_0 << (col), MASK_EMPTY)

#define Mask_GetRow(row, mask) (MASK_ROW(row) & mask)

#define SIDE_NEGATE(side) IF_SIDE(side, side, PIECE_SIDE_INDEX_BLACK, PIECE_SIDE_INDEX_WHITE)
#define DIRECT_OFFSET_SIGN(side) IF_SIDE(side, 0, 1, -1)
#define MASK_INDEX_SIGNED(index) (index < 0 ? MASK_EMPTY : MASK_INDEX(index))
		

#define MASK_INDEX_VA_ARGS(funcName, maskIndexFunc) \
uint64_t funcName(uint32_t count, ...) \
{ \
	va_list args; \
	va_start(args, count); \
	uint64_t mask_result = MASK_EMPTY; \
	for (uint32_t i = 0; i < count; i++) \
	{ \
		uint32_t index = va_arg(args, uint32_t); \
		mask_result |= maskIndexFunc(index); \
	} \
	return mask_result; \
}
MASK_INDEX_VA_ARGS(Mask_Rows, MASK_ROW);
MASK_INDEX_VA_ARGS(Mask_Cols, MASK_COL);

uint64_t Direct_Rows(PieceSideIndex side, uint64_t mask)
{

	if (side != PIECE_SIDE_INDEX_BLACK) return mask;
	uint64_t mask_result = MASK_EMPTY;
	for (uint8_t i = 0; i < 8; i++)
	{
		mask_result |= (Mask_GetRow(i, mask) >> (i * 8)) << ((7 - i) * 8);
	}
	return mask_result;
}

uint64_t Mask_Moves(uint8_t index, PieceSideIndex side, PieceTypeIndex type) {
	uint8_t col = Index_AsCol(index);
	uint8_t row = Index_AsRow(index);
	int8_t offsetSign = DIRECT_OFFSET_SIGN(ACTIVE_SIDE);
	int8_t offsetForward = OFFSET_FORWARD;
	PieceSideIndex inactiveSide = SIDE_NEGATE(ACTIVE_SIDE);
	uint64_t mask_unoccupied = ~MASK_ALL(BITBOARD_SET);
	uint64_t mask_enemy = Mask_Side(BITBOARD_SET, inactiveSide);
	switch (Piece_GetTypeIndex(BITBOARD_SET, index)) {
	case PIECE_TYPE_INDEX_PAWN: {
		uint64_t mask_enPassantTarget = MASK_INDEX_SIGNED(EN_PASSANT_TARGET_INDEX);
		uint64_t mask_unmovePawnsRow = Direct_Rows(ACTIVE_SIDE, MASK_ROW(6));
		uint64_t mask_asUnmovedPawn = MASK_INDEX(index) & mask_unmovePawnsRow;
		uint64_t mask_singleForward = mask_unoccupied & MASK_INDEX(index + offsetForward);
		uint64_t mask_captures = Mask_Cols(2, col - 1, col + 1) & MASK_ROW(row - offsetSign) & (mask_enemy | mask_enPassantTarget);
		uint64_t mask_doubleForward = (mask_asUnmovedPawn && mask_singleForward) ? mask_unoccupied & MASK_INDEX(index + offsetForward * 2) : MASK_EMPTY;
		return mask_singleForward | mask_doubleForward | mask_captures;
		break;
	} default: return MASK_EMPTY;
	}
}
uint64_t Mask_Movables()
{
	uint64_t mask_result = MASK_EMPTY;
	for (uint8_t i = 0; i < 64; i++)
	{
		if (Piece_HasSideIndex(BITBOARD_SET, i, ACTIVE_SIDE) && Mask_Moves(i, Piece_GetSideIndex(BITBOARD_SET, i), Piece_GetTypeIndex(BITBOARD_SET, i)))
		{
			mask_result |= MASK_INDEX(i);
		}
	}
	return mask_result;
}
bool Piece_IsMovable(uint8_t index)
{
	return MASK_INDEX(index) & MOVABLES_BITBOARD;
}
int8_t DST_CELL_INDEX = -1;
int8_t CELL_INDEX = -1;
int8_t OPT_CELL_INDEX = -1;
PieceTypeIndex PIECE_TYPE_PROMOTION = PIECE_TYPE_INDEX_NONE;
void BoardClick_Phase_SelectSource()
{
	if (Piece_IsMovable(CELL_INDEX))
	{
		SRC_CELL_INDEX = CELL_INDEX;
		MOVES_BITBOARD = Mask_Moves(SRC_CELL_INDEX, ACTIVE_SIDE, Piece_GetTypeIndex(BITBOARD_SET, SRC_CELL_INDEX));
		IsCellSelectableFunc = Piece_IsMove;
	}
}
void BoardClick_Phase_ResetAuto()
{
	// reset click phase
	SRC_CELL_INDEX = -1;
	DST_CELL_INDEX = -1;
	IsCellSelectableFunc = Piece_IsMovable;
	// skip src phase if selected is movable
	BoardClick_Phase_SelectSource();
}
void Board_Side_Reset()
{
	RECENT_MOVE = (Move){0};
	MOVABLES_BITBOARD = Mask_Movables();
}
void Board_Side_Switch()
{
	// switch side
	ACTIVE_SIDE = SIDE_NEGATE(ACTIVE_SIDE); 
	// prepare side movables
	Board_Side_Reset();
}
#define BitboardSet_Remove(bitboardSet, index, side, type) \
	bitboardSet[side][type] &= ~MASK_INDEX(index)

#define BitboardSet_PutAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Put(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)
#define BitboardSet_RemoveAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Remove(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)
void BoardClick_Phase_MakeMove()
{
	{
		// make move
		BitboardSet_RemoveAttrs(BITBOARD_SET, RECENT_MOVE, src.index, src.side, src.type);
		BitboardSet_PutAttrs(BITBOARD_SET, RECENT_MOVE, dst.index, src.side, src.type);
		BitboardSet_RemoveAttrs(BITBOARD_SET, RECENT_MOVE, dst.index, dst.side, dst.type);
		/*
		BITBOARD_SET[RECENT_MOVE.src.side][RECENT_MOVE.src.type] &= ~MASK_INDEX(RECENT_MOVE.src.index);
		BITBOARD_SET[RECENT_MOVE.src.side][RECENT_MOVE.src.type] |= MASK_INDEX(RECENT_MOVE.dst.index);
		BITBOARD_SET[RECENT_MOVE.dst.side][RECENT_MOVE.dst.type] &= ~MASK_INDEX(RECENT_MOVE.dst.index);
		*/
	}
	int8_t index_enPassantTarget = EN_PASSANT_TARGET_INDEX;
	EN_PASSANT_TARGET_INDEX = -1;
	{
		// make special move features
		if (RECENT_MOVE.src.type == PIECE_TYPE_INDEX_PAWN)
		{
			// pawn special features
			// en passant pawn addition
			if (RECENT_MOVE.dst.index == RECENT_MOVE.src.index + OFFSET_UP * 2 * DIRECT_OFFSET_SIGN(RECENT_MOVE.src.side))
			{
				EN_PASSANT_TARGET_INDEX = RECENT_MOVE.src.index + OFFSET_UP * DIRECT_OFFSET_SIGN(RECENT_MOVE.src.side);
			}
			// en passant pawn deletion
			else if (RECENT_MOVE.dst.index == index_enPassantTarget)
			{
				Piece enPassantPawn = Piece_New(BITBOARD_SET, RECENT_MOVE.dst.index + OFFSET_UP * DIRECT_OFFSET_SIGN(SIDE_NEGATE(RECENT_MOVE.src.side)));
				BitboardSet_RemoveAttrs(BITBOARD_SET, enPassantPawn, index, side, type);
				//BITBOARD_SET[enPassantPawn.side][enPassantPawn.type] &= ~MASK_INDEX(enPassantPawn.index);
			}
			else if (MASK_INDEX(RECENT_MOVE.dst.index) & Direct_Rows(RECENT_MOVE.src.side, MASK_ROW_0))
			{
				BitboardSet_RemoveAttrs(BITBOARD_SET, RECENT_MOVE, dst.index, src.side, src.type);
				BitboardSet_Put(BITBOARD_SET, RECENT_MOVE.dst.index, RECENT_MOVE.src.side, PIECE_TYPE_PROMOTION);
				PIECE_TYPE_PROMOTION = PIECE_TYPE_INDEX_NONE;
				//BITBOARD_SET[RECENT_MOVE.src.side][RECENT_MOVE.src.type] &= ~MASK_INDEX(RECENT_MOVE.dst.index);
				//BITBOARD_SET[RECENT_MOVE.src.side][PIECE_TYPE_PROMOTION] |= MASK_INDEX(RECENT_MOVE.dst.index);
			}
		}
	}
}

void BoardClick()
{
	static bool enable_makeMove = false;
	if (SRC_CELL_INDEX == -1)
	{
		BoardClick_Phase_SelectSource();
	} 
	else if (DST_CELL_INDEX == -1)
	{
		if (Piece_IsMove(CELL_INDEX))
		{
			enable_makeMove = true;
			// set move if valid
			DST_CELL_INDEX = CELL_INDEX;
			RECENT_MOVE = Move_New(BITBOARD_SET, SRC_CELL_INDEX, DST_CELL_INDEX);
		}
		else
		{
			BoardClick_Phase_ResetAuto();
		}
		{
			// to trigger pawn promotion option selection
			if (RECENT_MOVE.src.type == PIECE_TYPE_INDEX_PAWN)
			{
				if (MASK_INDEX(RECENT_MOVE.dst.index) & Direct_Rows(RECENT_MOVE.src.side, MASK_ROW_0))
				{
					enable_makeMove = false;
					OPT_CELL_INDEX = RECENT_MOVE.dst.index;
				}
			}
		}
	}
	else if (OPT_CELL_INDEX != -1)
	{
		OPT_CELL_INDEX = -1;
		static const PieceTypeIndex map_optToType[PIECE_PROMOTION_INDEX_COUNT] = { 
			[PIECE_PROMOTION_INDEX_QUEEN] = PIECE_TYPE_INDEX_QUEEN,
			[PIECE_PROMOTION_INDEX_ROOK] = PIECE_TYPE_INDEX_ROOK,
			[PIECE_PROMOTION_INDEX_BISHOP] = PIECE_TYPE_INDEX_BISHOP,
			[PIECE_PROMOTION_INDEX_KNIGHT] = PIECE_TYPE_INDEX_KNIGHT,
			[PIECE_PROMOTION_INDEX_NONE] = PIECE_TYPE_INDEX_NONE
		};

		uint64_t mask_options = Direct_Rows(RECENT_MOVE.src.side, Mask_Rows(4, 0, 1, 2, 3)) & MASK_COL(Index_AsCol(RECENT_MOVE.dst.index));
		if (MASK_INDEX(CELL_INDEX) & mask_options)
		{
			enable_makeMove = true;
			int row = Index_AsRow(CELL_INDEX);
			PIECE_TYPE_PROMOTION = map_optToType[IF_SIDE(RECENT_MOVE.src.side, PIECE_PROMOTION_INDEX_NONE, row, 7 - row)];
		}
		else
		{
			BoardClick_Phase_ResetAuto();
		}
	}
	if (enable_makeMove)
	{
		enable_makeMove = false;
		{
			BoardClick_Phase_MakeMove();
		}
		{
			Board_Side_Switch();
		}
		{
			BoardClick_Phase_ResetAuto();
		}
	}
	CELL_INDEX = -1;
}
CLAY_WASM_EXPORT("Init") void Init()
{
	ChessInit_Default();
	StringIndex fen = STRING("rnbqkbn1/pppppppP/4p3/3pP3/pPp5/7p/PPPPPPPP/RNBQKBNR w KQkq d6 0 1");
	ChessInit_FromString(fen);
	// prepare side
	Board_Side_Reset();
	// reset phase
	BoardClick_Phase_ResetAuto();
};

CLAY_CLICK_HANDLER(cell,
	{BoardClick();},
	{CELL_INDEX = userData;},
	{BoardClick();},
	{CELL_INDEX = -1;},
	{}
);

void BoardSurface()
{
	static const Clay_Color colorBlack = RGB(180, 136, 97);
	static const Clay_Color colorWhite = RGB(239, 217, 180);
	CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_WIDTH, BOARD_WIDTH)), CLAY_FLOATING(), CLAY_RECTANGLE(colorWhite))
	for (uint8_t row = 0; row < 8; row++)
	for (uint8_t col = row % 2; col < 8; col+=2)
	CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
		CLAY_FLOATING({.offset={col * BOARD_CELL_WIDTH, row * BOARD_CELL_WIDTH}}),
		CLAY_RECTANGLE(colorBlack)
	);
}

void BoardPieces()
{
	static const float subrectangleWidth = 270.f/6;
	static const float subrectangleHeight = 90.f/2;
	static const float pieceWidthRatio = 1.f;
	const float pieceWidth = BOARD_CELL_WIDTH * pieceWidthRatio;
	for (uint8_t row = 0; row < 8; row++)
	for (uint8_t col = 0; col < 8; col++)
	{
		const uint8_t index = row * 8 + col;
		if (!Piece_IsTypeIndex(Piece_GetTypeIndex(BITBOARD_SET, index))) continue;
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
			CLAY_FLOATING({.offset={col * BOARD_CELL_WIDTH, row * BOARD_CELL_WIDTH}})
		)
		CLAY(CLAY_LAYOUT(SIZING_FIXED(pieceWidth, pieceWidth)),
			CLAY_FLOATING({.attachment={CLAY_ATTACH_POINT_CENTER_CENTER, CLAY_ATTACH_POINT_CENTER_CENTER}}),
			CLAY_IMAGE({.sourceURL=CLAY_STRING("/clay/images/pieces.svg"),
				.subrectangle={
					subrectangleWidth * (Piece_GetTypeIndex(BITBOARD_SET, index)),
					subrectangleHeight * (Piece_GetSideIndex(BITBOARD_SET, index)),
					subrectangleWidth,
					subrectangleHeight
				}
			})
		);
	}
}

void BoardHints()
{
	for (uint8_t i = 0; i < 64; i++)
	{
	}
}

void BoardClickLayer()
{
	CLAY(CLAY_LAYOUT({SIZING_FIXED(BOARD_WIDTH, BOARD_WIDTH), .layoutDirection=CLAY_TOP_TO_BOTTOM}),
		CLAY_FLOATING())
	for (uint8_t row = 0; row < 8; row++) 
	CLAY(SIZING_FIXED(BOARD_WIDTH, BOARD_CELL_WIDTH))
	for (uint8_t col = 0; col < 8; col++)
	{
		const uint8_t index = row * 8 + col;
		Clay_Color color = IsCellSelectableFunc(index)
			? (index == CELL_INDEX ? RGBA(0, 255, 0, 128) : RGBA(0, 0, 0, 0))
			: (index == CELL_INDEX ? RGBA(255, 0, 0, 128) : RGBA(0, 0, 0, 128));
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
			CLAY_RECTANGLE(color),
			Clay_OnHover(handle_cell_clickInteraction, index));
	}
}

Clay_RenderCommandArray CreateLayout() {
	{
		BOARD_WIDTH = windowSmallSide;
		BOARD_CELL_WIDTH = BOARD_WIDTH / 8;
	}
    Clay_BeginLayout();
	{
		BoardSurface();
		BoardPieces();
		BoardHints();
		BoardClickLayer();
	}
	{
		listen_cell_click();
	}
    return Clay_EndLayout();
}

float animationLerpValue = -1.0f;
bool debugModeEnabled = false;

void Helpers_Update(float width, float height)
{
	windowWidth = width;
	windowHeight = height;
	windowLongSide = MAX(windowWidth, windowHeight);
	windowSmallSide = MIN(windowWidth, windowHeight);
	isPortrait = windowWidth < windowHeight;
	lineWidth = 0.025f * windowSmallSide;
}

CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(
	float width, float height,
	float mouseWheelX, float mouseWheelY,
	float mousePositionX, float mousePositionY,
	bool isTouchDown, bool isMouseDown,
	bool isTouchStart, bool isMouseStart,
	bool isTouchCancel, bool isMouseCancel,
	bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame,
	bool dKeyPressedThisFrame,
	float currentTime, float deltaTime
) {
	Helpers_Update(width, height);
	
	ClickHandler_Update(currentTime, deltaTime, isTouchStart || isMouseStart, isTouchCancel || isMouseCancel, isTouchDown || isMouseDown);

    Clay_SetLayoutDimensions((Clay_Dimensions) { width, height });

	animationLerpValue += deltaTime;
	if (animationLerpValue > 1) {
		animationLerpValue -= 2;
	}

    if (dKeyPressedThisFrame) {
        debugModeEnabled = !debugModeEnabled;
        Clay_SetDebugModeEnabled(debugModeEnabled);
    }
    Clay_SetCullingEnabled(true);
    Clay_SetExternalScrollHandlingEnabled(false);

    Clay__debugViewHighlightColor = (Clay_Color) {105,210,231, 120};

    Clay_SetPointerState((Clay_Vector2) {mousePositionX, mousePositionY}, isMouseDown || isTouchDown);

    Clay_UpdateScrollContainers(isTouchDown, (Clay_Vector2) {mouseWheelX, mouseWheelY}, deltaTime);

	isMobileScreen = windowWidth < 750;
	if (debugModeEnabled) {
		isMobileScreen = windowWidth < 950;
	}

    return CreateLayout();
    //----------------------------------------------------------------------------------
}


// Dummy main() to please cmake - TODO get wasm working with cmake on this example
int main() {
    return 0;
}
