#include <stdarg.h>

#define CLAY_HELPERS_IMPLEMENTATION
#include "clay_helpers.h"

#define CLAY_IMPLEMENTATION
#include "web_clay.h"

#define CLAY_CLICK_HANDLER_IMPLEMENTATION
#include "clay_click_handler.h"

#define CHESS_IMPLEMENTATION
#define CHESS_BITBOARD_IMPLEMENTATION
#include "chess.h"

#define MY_MATH_IMPLEMENTATION
#include "math.h"




typedef struct Piece {
	uint8_t index;
	PieceSideIndex side;
	PieceTypeIndex type;
} Piece;

typedef struct Move {
	Piece src;
	Piece dst;
} Move;

typedef enum PiecePromotionIndex : uint8_t {
	PIECE_PROMOTION_INDEX_QUEEN,
	PIECE_PROMOTION_INDEX_ROOK,
	PIECE_PROMOTION_INDEX_BISHOP,
	PIECE_PROMOTION_INDEX_KNIGHT,
	PIECE_PROMOTION_INDEX_COUNT,
	PIECE_PROMOTION_INDEX_NONE,
} PiecePromotionIndex;




#define BitboardSet_PutAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Put(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)
#define BitboardSet_RemoveAttrs(bitboardSet, attrs, indexToken, sideToken, typeToken) BitboardSet_Remove(bitboardSet, attrs.indexToken, attrs.sideToken, attrs.typeToken)

#define BitboardSet_Remove(bitboardSet, index, side, type) bitboardSet[side][type] &= ~MASK_INDEX(index)

#define MASK_INDEX_SIGNED(index) (index < 0 ? MASK_EMPTY : MASK_INDEX(index))

uint64_t Mask_Rows(uint32_t count, ...);
uint64_t Mask_Cols(uint32_t count, ...);
		
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

uint64_t Direct_Rows(PieceSideIndex side, uint64_t mask);
#define DIRECT_OFFSET_SIGN(side) IF_SIDE(side, 0, 1, -1)
#define SIDE_NEGATE(side) IF_SIDE(side, side, PIECE_SIDE_INDEX_BLACK, PIECE_SIDE_INDEX_WHITE)

#define IF_SIDE(side, forNone, forWhite, forBlack) ((side) == PIECE_SIDE_INDEX_NONE ? (forNone) : (side) == PIECE_SIDE_INDEX_WHITE ? (forWhite) : (forBlack))

#define Mask_GetRow(row, mask) (MASK_ROW(row) & mask)
#define MASK_ROW(row) IF_IN_RANGE(row, 0, 7, MASK_ROW_0 << ((row) * 8), MASK_EMPTY)
#define MASK_COL(col) IF_IN_RANGE(col, 0, 7, MASK_COL_0 << (col), MASK_EMPTY)

#define IF_IN_RANGE(x, min, max, forTrue, forFalse) (IN_RANGE((x), (min), (max)) ? (forTrue) : (forFalse))

#define IN_RANGE(x, min, max) ((x) >= (min) && (x) <= (max))
	
#define Index_AsRow(index) (uint8_t)floorf((index) / 8.f)
#define Index_AsCol(index) ((index) % 8)

#define OFFSET_FORWARD(side) DIRECT_OFFSET_SIGN(side) * OFFSET_UP
#define OFFSET_BACKWARD(side) DIRECT_OFFSET_SIGN(side) * OFFSET_DOWN
#define OFFSET_UP -8
#define OFFSET_DOWN 8
#define OFFSET_LEFT -1
#define MASK_ROW_0 0xFFULL
#define MASK_COL_0 0x0101010101010101ULL
#define MASK_CHECKER 0x55AA55AA55AA55AAULL


CLAY_WASM_EXPORT("Init") void Init();
Piece Piece_New(Bitboards_All bitboardSet, uint8_t index);
Move Move_New(Bitboards_All bitboardSet, uint8_t srcIndex, uint8_t dstIndex);
uint64_t Mask_Moves(Bitboards_All bitboardSet, uint8_t index, PieceSideIndex side, PieceTypeIndex type);
uint64_t Mask_Movables(Bitboards_All bitboardSet);
bool Piece_IsMove(uint8_t index);
bool Piece_IsMovable(uint8_t index);
void Board_Side_Reset();
void Board_Side_Switch();
void BoardClick_Phase_SelectSource();
void BoardClick_Phase_Reset_AutoSelect();
void Game_MakeMove(Bitboards_All bitboards, Move move);
void BoardClick();
void listen_cell_click();
void handle_cell_clickInteraction(ON_HOVER_PARAMS);
void BoardBackground();
void BoardPieceSprites();
void BoardPromotionPopup();
void BoardClickLayer();
Clay_RenderCommandArray CreateLayout();
void Helpers_Update(float width, float height);
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
);
int main();



// clay_helpers
double windowWidth, windowHeight;
double windowSmallSide, windowLongSide;
bool isMobileScreen;
float animationLerpValue;
bool debugModeEnabled;
const uint32_t FONT_ID_TEXT;



double BOARD_WIDTH;
double BOARD_CELL_WIDTH;




bool (* IsCellSelectableFunc)(uint8_t index);
uint64_t MOVABLES_BITBOARD;
uint64_t MOVES_BITBOARD;
int8_t CELL_INDEX;
int8_t SRC_CELL_INDEX;
int8_t DST_CELL_INDEX;
int8_t OPT_CELL_INDEX;
Move RECENT_MOVE;
PieceTypeIndex PIECE_TYPE_PROMOTION;
uint64_t PROMOTION_OPTS_BITBOARD;
const PieceTypeIndex MAP_PROMOTION_OPT_TO_TYPE[PIECE_PROMOTION_INDEX_COUNT] = {
	[PIECE_PROMOTION_INDEX_QUEEN] = PIECE_TYPE_INDEX_QUEEN,
	[PIECE_PROMOTION_INDEX_ROOK] = PIECE_TYPE_INDEX_ROOK,
	[PIECE_PROMOTION_INDEX_BISHOP] = PIECE_TYPE_INDEX_BISHOP,
	[PIECE_PROMOTION_INDEX_KNIGHT] = PIECE_TYPE_INDEX_KNIGHT,
};




CLAY_WASM_EXPORT("Init") void Init()
{
	IsCellSelectableFunc = Piece_IsMovable;
	MOVABLES_BITBOARD = MASK_EMPTY;
	MOVES_BITBOARD = MASK_EMPTY;
	PROMOTION_OPTS_BITBOARD = MASK_EMPTY;
	CELL_INDEX = -1;
	SRC_CELL_INDEX = -1;
	DST_CELL_INDEX = -1;
	OPT_CELL_INDEX = -1;
	RECENT_MOVE = (Move){};
	PIECE_TYPE_PROMOTION = PIECE_TYPE_INDEX_NONE;
	ChessInit_Default();
	StringIndex fen = STRING("rnbqkbn1/pppppppP/4p3/3pP3/pPp5/7p/PPPPPPPP/RNBQKBNR w KQkq d6 0 1");
	ChessInit_FromString(fen);
	// prepare side
	Board_Side_Reset();
	// reset phase
	BoardClick_Phase_Reset_AutoSelect();
};



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

Piece Piece_New(Bitboards_All bitboardSet, uint8_t index)
{
	return (Piece){index,
		Piece_GetSideIndex(bitboardSet, index),
		Piece_GetTypeIndex(bitboardSet, index)};
}

Move Move_New(Bitboards_All bitboardSet, uint8_t srcIndex, uint8_t dstIndex)
{
	return (Move){
		Piece_New(bitboardSet, srcIndex),
		Piece_New(bitboardSet, dstIndex)};
}

MASK_INDEX_VA_ARGS(Mask_Rows, MASK_ROW);
MASK_INDEX_VA_ARGS(Mask_Cols, MASK_COL);

uint64_t Mask_Movables(Bitboards_All bitboardSet)
{
	uint64_t mask_result = MASK_EMPTY;
	for (uint8_t i = 0; i < 64; i++)
	{
		if (Piece_HasSideIndex(bitboardSet, i, ACTIVE_SIDE) && Mask_Moves(bitboardSet, i, Piece_GetSideIndex(bitboardSet, i), Piece_GetTypeIndex(bitboardSet, i)))
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

uint64_t Mask_Moves(Bitboards_All bitboardSet, uint8_t index, PieceSideIndex side, PieceTypeIndex type)
{
	uint8_t col = Index_AsCol(index);
	uint8_t row = Index_AsRow(index);
	int8_t offsetSign = DIRECT_OFFSET_SIGN(side);
	int8_t offsetForward = OFFSET_FORWARD(side);
	PieceSideIndex inactiveSide = SIDE_NEGATE(side);
	uint64_t mask_unoccupied = ~MASK_ALL(bitboardSet);
	uint64_t mask_enemy = Mask_Side(bitboardSet, inactiveSide);
	switch (type) {
	case PIECE_TYPE_INDEX_PAWN: {
		uint64_t mask_enPassantTarget = MASK_INDEX_SIGNED(EN_PASSANT_TARGET_INDEX);
		uint64_t mask_unmovePawnsRow = Direct_Rows(side, MASK_ROW(6));
		uint64_t mask_asUnmovedPawn = MASK_INDEX(index) & mask_unmovePawnsRow;
		uint64_t mask_singleForward = mask_unoccupied & MASK_INDEX(index + offsetForward);
		uint64_t mask_captures = Mask_Cols(2, col - 1, col + 1) & MASK_ROW(row - offsetSign) & (mask_enemy | mask_enPassantTarget);
		uint64_t mask_doubleForward = (mask_asUnmovedPawn && mask_singleForward) ? mask_unoccupied & MASK_INDEX(index + offsetForward * 2) : MASK_EMPTY;
		return mask_singleForward | mask_doubleForward | mask_captures;
		break;
	} default: return MASK_EMPTY;
	}
}

bool Piece_IsMove(uint8_t index)
{
	return MASK_INDEX(index) & MOVES_BITBOARD;
}

uint64_t Mask_PromotionOptions(Bitboards_All bitboardSet, uint64_t index, Move move)
{
	uint64_t mask_options = Direct_Rows(move.src.side, Mask_Rows(4, 0, 1, 2, 3) & MASK_COL(Index_AsCol(move.dst.index)));
	return mask_options;
}

bool Piece_IsPromotionOpt(uint8_t index)
{
	return MASK_INDEX(index) & PROMOTION_OPTS_BITBOARD; 
}

void Board_Side_Reset()
{
	RECENT_MOVE = (Move){};
	MOVABLES_BITBOARD = Mask_Movables(BITBOARD_SET);
}

void Board_Side_Switch()
{
	// switch side
	ACTIVE_SIDE = SIDE_NEGATE(ACTIVE_SIDE); 
	// prepare side movables
	Board_Side_Reset();
}

void BoardClick_Phase_SelectSource()
{
	if (Piece_IsMovable(CELL_INDEX))
	{
		SRC_CELL_INDEX = CELL_INDEX;
		MOVES_BITBOARD = Mask_Moves(BITBOARD_SET, SRC_CELL_INDEX, ACTIVE_SIDE, Piece_GetTypeIndex(BITBOARD_SET, SRC_CELL_INDEX));
		IsCellSelectableFunc = Piece_IsMove;
	}
}

void BoardClick_Phase_Reset_AutoSelect()
{
	// reset click phase
	SRC_CELL_INDEX = -1;
	DST_CELL_INDEX = -1;
	OPT_CELL_INDEX = -1;
	IsCellSelectableFunc = Piece_IsMovable;
	// skip src phase if selected is movable
	BoardClick_Phase_SelectSource();
}

void Game_MakeMove(Bitboards_All bitboardSet, Move move)
{
	bool enPassantTarget_isSet_thisFrame = false;
	{
		// make move
		BitboardSet_RemoveAttrs(bitboardSet, move, src.index, src.side, src.type);
		BitboardSet_PutAttrs(bitboardSet, move, dst.index, src.side, src.type);
		BitboardSet_RemoveAttrs(bitboardSet, move, dst.index, dst.side, dst.type);
	}
	{
		// make special move features
		if (move.src.type == PIECE_TYPE_INDEX_PAWN)
		{
			// pawn special features
			// en passant pawn addition
			if (move.dst.index == move.src.index + 2 * OFFSET_FORWARD(move.src.side))
			{
				enPassantTarget_isSet_thisFrame = true;
				EN_PASSANT_TARGET_INDEX = move.src.index + OFFSET_FORWARD(move.src.side);
			}
			// en passant pawn deletion
			else if (move.dst.index == EN_PASSANT_TARGET_INDEX)
			{
				Piece enPassantPawn = Piece_New(bitboardSet, move.dst.index + OFFSET_BACKWARD(move.src.side));
				BitboardSet_RemoveAttrs(bitboardSet, enPassantPawn, index, side, type);
			}
			// pawn promotion
			else if (MASK_INDEX(move.dst.index) & Direct_Rows(move.src.side, MASK_ROW_0))
			{
				BitboardSet_RemoveAttrs(bitboardSet, move, dst.index, src.side, src.type);
				BitboardSet_Put(bitboardSet, move.dst.index, move.src.side, PIECE_TYPE_PROMOTION);
				PIECE_TYPE_PROMOTION = PIECE_TYPE_INDEX_NONE;
			}
		}
	}
	{
		// Remove en passant after a move
		if (!enPassantTarget_isSet_thisFrame)
		{
			EN_PASSANT_TARGET_INDEX = -1;
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
			{
				// to trigger pawn promotion option selection
				if (RECENT_MOVE.src.type == PIECE_TYPE_INDEX_PAWN)
				{
					if (MASK_INDEX(RECENT_MOVE.dst.index) & Direct_Rows(RECENT_MOVE.src.side, MASK_ROW_0))
					{
						enable_makeMove = false;
						OPT_CELL_INDEX = RECENT_MOVE.dst.index;
						PROMOTION_OPTS_BITBOARD = Mask_PromotionOptions(BITBOARD_SET, OPT_CELL_INDEX, RECENT_MOVE);
						IsCellSelectableFunc = Piece_IsPromotionOpt;
					}
				}
			}
		}
		else
		{
			BoardClick_Phase_Reset_AutoSelect();
		}
	}
	else if (OPT_CELL_INDEX != -1)
	{
		OPT_CELL_INDEX = -1;

		if (Piece_IsPromotionOpt(CELL_INDEX))
		{
			enable_makeMove = true;
			int row = Index_AsRow(CELL_INDEX);
			PIECE_TYPE_PROMOTION = IF_SIDE(RECENT_MOVE.src.side,
				PIECE_TYPE_INDEX_NONE,
				MAP_PROMOTION_OPT_TO_TYPE[row],
				MAP_PROMOTION_OPT_TO_TYPE[7 - row]);
		}
		else
		{
			BoardClick_Phase_Reset_AutoSelect();
		}
	}
	if (enable_makeMove)
	{
		enable_makeMove = false;
		{
			Game_MakeMove(BITBOARD_SET, RECENT_MOVE);
		}
		{
			Board_Side_Switch();
		}
		{
			BoardClick_Phase_Reset_AutoSelect();
		}
	}
	CELL_INDEX = -1;
}

CLAY_CLICK_HANDLER(cell,
	{BoardClick();},
	{CELL_INDEX = userData;},
	{BoardClick();},
	{CELL_INDEX = -1;},
	{}
);
void BoardBackground()
{
	static const Clay_Color colorBlack = RGB(180, 136, 97);
	static const Clay_Color colorWhite = RGB(239, 217, 180);
	CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_WIDTH, BOARD_WIDTH)), CLAY_FLOATING(), CLAY_RECTANGLE(colorWhite))
	for (uint8_t row = 0; row < 8; row++)
	for (uint8_t col = 0; col < 8; col++)
	if (MASK_INDEX(row * 8 + col) & MASK_CHECKER)
	// fill in only black
	CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
		CLAY_FLOATING({.offset={col * BOARD_CELL_WIDTH, row * BOARD_CELL_WIDTH}}),
		CLAY_RECTANGLE(colorBlack)
	);
}

void BoardPieceSprite(uint8_t row, uint8_t col)
{
	static const float subrectangleWidth = 270.f/6;
	static const float subrectangleHeight = 90.f/2;
	static const float pieceWidthRatio = 1.f;
	const float PIECE_WIDTH = BOARD_CELL_WIDTH * pieceWidthRatio;
	CLAY(CLAY_LAYOUT(SIZING_FIXED(PIECE_WIDTH, PIECE_WIDTH)),
		CLAY_FLOATING({.attachment={CLAY_ATTACH_POINT_CENTER_CENTER, CLAY_ATTACH_POINT_CENTER_CENTER}}),
		CLAY_IMAGE({.sourceURL=CLAY_STRING("/clay/images/pieces.svg"),
			.subrectangle={
				subrectangleWidth * col,
				subrectangleHeight * row,
				subrectangleWidth,
				subrectangleHeight
			}
		})
	);
}

void BoardPieceSprites()
{
	const uint64_t occupied = MASK_ALL(BITBOARD_SET);
	for (uint8_t row = 0; row < 8; row++)
	for (uint8_t col = 0; col < 8; col++)
	{
		const uint8_t index = row * 8 + col;
		if (MASK_INDEX(index) & occupied)
		{
			Piece piece = Piece_New(BITBOARD_SET, index);
			CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
				CLAY_FLOATING({.offset={col * BOARD_CELL_WIDTH, row * BOARD_CELL_WIDTH}})
			)
			BoardPieceSprite(piece.side, piece.type);
		}
	}
}

void BoardPromotionPopup()
{
	static const Clay_Color colorBlack = RGB(180, 136, 97);
	static const Clay_Color colorWhite = RGB(239, 217, 180);
	if (OPT_CELL_INDEX == -1)
	{
		return;
	}
	for (uint8_t optIndex = 0; optIndex < 4; optIndex++)
	{
		const uint8_t index = OPT_CELL_INDEX + OFFSET_DOWN * optIndex * DIRECT_OFFSET_SIGN(RECENT_MOVE.src.side);
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
			CLAY_RECTANGLE((MASK_INDEX(index) & MASK_CHECKER) ? colorBlack : colorWhite),
			CLAY_FLOATING({.offset={Index_AsCol(index) * BOARD_CELL_WIDTH, Index_AsRow(index) * BOARD_CELL_WIDTH}})
		)
		BoardPieceSprite(RECENT_MOVE.src.side, MAP_PROMOTION_OPT_TO_TYPE[optIndex]);
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

Clay_RenderCommandArray CreateLayout()
{
	{
		BOARD_WIDTH = windowSmallSide;
		BOARD_CELL_WIDTH = BOARD_WIDTH / 8;
	}
    Clay_BeginLayout();
	{
		BoardBackground();
		BoardPieceSprites();
		BoardPromotionPopup();
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
)
{
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
