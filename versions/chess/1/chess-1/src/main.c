#include "clay_helpers.h"




#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL; Clay_BoundingBox subrectangle; 
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;
#define CLAY_IMPLEMENTATION
#include "clay.h"

#define BOARD_SURFACE_IMPLEMENTATION
#include "board_surface.h"

double windowWidth, windowHeight;
bool isMobileScreen;
float animationLerpValue;
bool debugModeEnabled;
const uint32_t FONT_ID_TEXT;

#define CHESS_IMPLEMENTATION
#include "chess.h"

Clay_RenderCommandArray CreateLayout() {
	ChessInit_Default();

    Clay_BeginLayout();
	BoardSurface(windowWidth);
	for (uint8_t row = 0; row < 8; row++)
	for (uint8_t col = 0; col < 8; col++)
	{
		const uint8_t index = row * 8 + col;
		if ((BOARD[index] & 0b0111) == PIECE_NONE) {
			continue;
		}
		CLAY(CLAY_LAYOUT({SIZING_FIXED(windowWidth/8, windowWidth/8)}),
			CLAY_IMAGE({
				.sourceURL=CLAY_STRING("/clay/images/pieces.svg"),
				.subrectangle={
					270.f/6 * ((BOARD[index] & 0b111) - 1),
					90.f/2 * (1 - ((BOARD[index] & 0b1000) >> 3)),
					270.f/6,
					90.f/2
				}
			}), CLAY_FLOATING({.offset={col * windowWidth / 8, row * windowWidth / 8}})
		) {};
	}
    return Clay_EndLayout();
}

double windowWidth = 1024, windowHeight = 768;
bool isMobileScreen = true;
float animationLerpValue = -1.0f;
bool debugModeEnabled = false;
const uint32_t FONT_ID_TEXT = 0;

CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(float width, float height, float mouseWheelX, float mouseWheelY, float mousePositionX, float mousePositionY, bool isTouchDown, bool isMouseDown, bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame, bool dKeyPressedThisFrame, float deltaTime)
{
    windowWidth = width;
    windowHeight = height;
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
