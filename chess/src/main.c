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
#include "chess.h"

double windowWidth, windowHeight;
double windowSmallSide, windowLongSide;
bool isMobileScreen;
float animationLerpValue;
bool debugModeEnabled;
const uint32_t FONT_ID_TEXT;

double BOARD_WIDTH;
double BOARD_CELL_WIDTH;


bool (* IsCellSelectableFunc)(uint8_t index) = Piece_IsSelectable;

typedef struct Move {
	uint8_t src;
	uint8_t dst;
} Move;

Move RECENT_MOVE = {0};

int8_t SRC_CELL_INDEX = -1;
bool IsPawnMove(uint8_t index)
{
//	uint64_t srcMask = 1ULL << SRC_CELL_INDEX;
	#define Piece_RowMask(row) 0xFFULL << (row * 8);
	bool isBlack = Piece_HasColor(SRC_CELL_INDEX, PIECE_COLOR_BLACK);
//	uint64_t unmovedRowMask = Piece_RowMask(isWhite ? 6 : 1);
//	uint64_t isUnmoved = (unmovedRowMask >> SRC_CELL_INDEX) & 0b1;
//	bool isUnmoved = (unmovedMask >> index) & 0b1;
	int8_t forwardSign = isBlack ? 1 : -1;
	uint8_t forwardOnceIndex = SRC_CELL_INDEX + 8 * forwardSign;
	bool canForwardOnce = Piece_HasType(forwardOnceIndex, PIECE_TYPE_NONE);
	bool isForwardOnce = canForwardOnce && index == forwardOnceIndex;
	uint8_t forwardTwiceIndex = SRC_CELL_INDEX + 16 * forwardSign;
	bool canForwardTwice = canForwardOnce && Piece_HasType(forwardTwiceIndex, PIECE_TYPE_NONE);
	bool isForwardTwice = canForwardTwice && index == forwardTwiceIndex;
//	uint64_t forwardOneMask = 1ULL << (SRC_CELL_INDEX - 8);
//	uint64_t forwardTwoMask = isUnmoved << (SRC_CELL_INDEX - 16);
	return isForwardOnce || isForwardTwice;
//	return (pawn_moves >> index) & 0b1;
}
bool IsNoneMove(uint8_t index)
{
	return false;
}
bool (*Piece_GetMoveChecker(uint8_t index))(uint8_t)
{
	switch (Piece_GetType(index)) {
	case PIECE_TYPE_PAWN: return IsPawnMove;
	default: return IsNoneMove;
	}
}
int8_t DST_CELL_INDEX = -1;
int8_t CELL_INDEX = -1;
void BoardClick()
{
	if (SRC_CELL_INDEX == -1)
	{
		if (IsCellSelectableFunc(CELL_INDEX))
		{
			SRC_CELL_INDEX = CELL_INDEX;
			IsCellSelectableFunc = Piece_GetMoveChecker(SRC_CELL_INDEX);
		}
	} 
	else if (DST_CELL_INDEX == -1)
	{
		if (IsCellSelectableFunc(CELL_INDEX))
		{
			// set move if valid
			DST_CELL_INDEX = CELL_INDEX;
			RECENT_MOVE = (Move){SRC_CELL_INDEX, DST_CELL_INDEX};
		}
		{
			// reset 
			SRC_CELL_INDEX = -1;
			DST_CELL_INDEX = -1;
			IsCellSelectableFunc = Piece_IsSelectable;
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
		if (Piece_HasType(index, PIECE_TYPE_NONE)) continue;
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
			CLAY_FLOATING({.offset={col * BOARD_CELL_WIDTH, row * BOARD_CELL_WIDTH}})
		)
		CLAY(CLAY_LAYOUT(SIZING_FIXED(pieceWidth, pieceWidth)),
			CLAY_FLOATING({.attachment={CLAY_ATTACH_POINT_CENTER_CENTER, CLAY_ATTACH_POINT_CENTER_CENTER}}),
			CLAY_IMAGE({.sourceURL=CLAY_STRING("/clay/images/pieces.svg"),
				.subrectangle={
					subrectangleWidth * (Piece_GetType(index) - 1),
					subrectangleHeight * (Piece_GetColor(index)),
					subrectangleWidth,
					subrectangleHeight
				}
			})
		);
	}
}

void BoardClickLayer()
{
//	static const Clay_Color colorNone = RGBA(0, 0, 0, 0);
	
	CLAY(CLAY_LAYOUT({SIZING_FIXED(BOARD_WIDTH, BOARD_WIDTH), .layoutDirection=CLAY_TOP_TO_BOTTOM}),
		CLAY_FLOATING())
	for (uint8_t row = 0; row < 8; row++) 
	CLAY(SIZING_FIXED(BOARD_WIDTH, BOARD_CELL_WIDTH))
	for (uint8_t col = 0; col < 8; col++)
	{
		const uint8_t index = row * 8 + col;
		if (IsCellSelectableFunc(index))
		{
			CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
				CLAY_RECTANGLE(index == CELL_INDEX ? RGBA(255, 0, 0, 128) : RGBA(0, 0, 0, 0))
			) {
				Clay_OnHover(handle_cell_clickInteraction, index);
			};
		}
		else
		{
			CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
				CLAY_RECTANGLE(RGBA(0, 0, 0, 64)));
		}
	}
	{
		listen_cell_click();
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
		BoardClickLayer();
	}
	{
	}
    return Clay_EndLayout();
}

CLAY_WASM_EXPORT("Init") void Init()
{
	ChessInit_Default();
};

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
