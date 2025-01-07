#define CLAY_BOX2(elementId, fillColor, borderColor, borderWidth, cornerRadius, ...) \
	CLAY( \
		elementId, \
		CLAY_LAYOUT({sizingGrow, {borderWidth, borderWidth}}), \
		CLAY_RECTANGLE({fillColor, CLAY_CORNER_RADIUS(cornerRadius)}), \
		CLAY_BORDER_OUTSIDE_RADIUS(borderWidth, borderColor, cornerRadius) \
	) CLAY ( \
		CLAY_IDI("ClayBox", CLAY_BOX_INDEX++), \
		##__VA_ARGS__ \
	)

#define CLAY_BOX(elementId, fillColor, borderColor, borderWidth, cornerRadius, ...) \
	CLAY( \
		elementId, \
		CLAY_LAYOUT({sizingGrow, {borderWidth, borderWidth}}), \
		CLAY_RECTANGLE({fillColor, CLAY_CORNER_RADIUS(cornerRadius)}), \
		CLAY_BORDER_OUTSIDE_RADIUS(borderWidth, borderColor, cornerRadius), \
		##__VA_ARGS__ \
	)

#define CLAY_LABEL(text) \
	CLAY( \
		CLAY_IDI("ClayLabel", CLAY_LABEL_INDEX++), \
		CLAY_FLOATING({.pointerCaptureMode=CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH}) \
	) { \
		CLAY_TEXT( \
			CLAY_STRING(text), \
			CLAY_TEXT_CONFIG({COLOR_WHITE, FONT_ID_TEXT, 36}) \
		); \
	}



#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;
#define CLAY_IMPLEMENTATION
#include "../clay/clay.h"

static uint64_t CLAY_BOX_INDEX = 0;
static uint64_t CLAY_LABEL_INDEX = 0;

double windowWidth = 1024, windowHeight = 768;
const uint32_t FONT_ID_TEXT = 0;
const Clay_Color COLOR_WHITE = (Clay_Color) {255, 255, 255, 255};
const Clay_Color COLOR_BLACK = (Clay_Color) {0, 0, 0, 255};
const Clay_Color COLOR_RED = (Clay_Color) {255, 0, 0, 255};
const Clay_Color COLOR_GREEN = (Clay_Color) {0, 255, 0, 255};
const Clay_Color COLOR_YELLOW = (Clay_Color) {255, 255, 0, 255};
const Clay_Color COLOR_NONE = (Clay_Color) {0, 0, 0, 0};
const Clay_Color COLOR_BLUE = (Clay_Color) {0, 0, 255, 255};
const Clay_Sizing sizingGrow = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()};

typedef enum : uint8_t {
	ACTIVE_PLAYER_NONE,
	ACTIVE_PLAYER_X,
	ACTIVE_PLAYER_O
} ActivePlayer;

ActivePlayer BOARD[9] = {0};
ActivePlayer ACTIVE_PLAYER = ACTIVE_PLAYER_X;

void HandleTictactoeCellInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) {
    if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
		if (BOARD[(uint8_t)userData] == ACTIVE_PLAYER_NONE) {
			BOARD[(uint8_t)userData] = ACTIVE_PLAYER;
			ACTIVE_PLAYER = ACTIVE_PLAYER == ACTIVE_PLAYER_X ? ACTIVE_PLAYER_O : ACTIVE_PLAYER_X;
		}
	}
}

void TictactoePage() {
	CLAY_BOX2(CLAY_ID("TictactoePage"), COLOR_GREEN, COLOR_RED, 10, 10,
//	CLAY_BOX(CLAY_ID("TictactoePage"), COLOR_GREEN, COLOR_RED, 10, 10,
		CLAY_LAYOUT({
			{CLAY_SIZING_GROW(), CLAY_SIZING_FIXED(windowHeight)},
			.childAlignment={CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER}
		})
	) {
		CLAY_LABEL("TictactoePage");
		CLAY_BOX(CLAY_ID("TictactoeGrid"),
			COLOR_WHITE, COLOR_BLACK,
			10, 10, 
			CLAY_LAYOUT({
				{CLAY_SIZING_FIXED(windowWidth - 20), CLAY_SIZING_FIXED(windowWidth - 20)},
				{10, 10}
			})
		) {
			CLAY_LABEL("TictactoeGrid");
			CLAY(CLAY_LAYOUT({sizingGrow, .layoutDirection=CLAY_TOP_TO_BOTTOM})) {for (int row = 0; row < 3; row++)
			{
				CLAY(CLAY_LAYOUT({sizingGrow})) {for (int col = 0; col < 3; col++)
				{
					CLAY_BOX(CLAY_IDI("TictactoeCell", row * 3 + col), 
						Clay_Hovered() ? COLOR_WHITE : COLOR_GREEN,
						Clay_Hovered() ? COLOR_BLACK : COLOR_RED,
						10, 10,
						Clay_OnHover(HandleTictactoeCellInteraction, row * 3 + col)
					) {
						if (BOARD[row * 3 + col] == ACTIVE_PLAYER_X) {
							CLAY_BOX(CLAY_IDI("TictactoeMove", row * 3 + col), COLOR_BLUE, COLOR_NONE, 10, 10)
							{
							}
						} else if (BOARD[row * 3 + col] == ACTIVE_PLAYER_O) {
							CLAY_BOX(CLAY_IDI("TictactoeMove", row * 3 + col), COLOR_YELLOW, COLOR_NONE, 10, 10)
							{}
						}
					}
				}}
			}}
		}
	}
}

float animationLerpValue = -1.0f;

Clay_RenderCommandArray CreateLayout(bool mobileScreen, float lerpValue) {
    Clay_BeginLayout();
	TictactoePage();
    return Clay_EndLayout();
}

bool debugModeEnabled = false;

CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(float width, float height, float mouseWheelX, float mouseWheelY, float mousePositionX, float mousePositionY, bool isTouchDown, bool isMouseDown, bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame, bool dKeyPressedThisFrame, float deltaTime) {
    windowWidth = width;
    windowHeight = height;
    Clay_SetLayoutDimensions((Clay_Dimensions) { width, height });
    if (deltaTime == deltaTime) { // NaN propagation can cause pain here
        animationLerpValue += deltaTime;
        if (animationLerpValue > 1) {
            animationLerpValue -= 2;
        }
    }

    if (dKeyPressedThisFrame) {
        debugModeEnabled = !debugModeEnabled;
        Clay_SetDebugModeEnabled(debugModeEnabled);
    }
    Clay_SetCullingEnabled(false);
    Clay_SetExternalScrollHandlingEnabled(true);

    Clay__debugViewHighlightColor = (Clay_Color) {105,210,231, 120};

    Clay_SetPointerState((Clay_Vector2) {mousePositionX, mousePositionY}, isMouseDown || isTouchDown);

    Clay_UpdateScrollContainers(isTouchDown, (Clay_Vector2) {mouseWheelX, mouseWheelY}, deltaTime);

	bool isMobileScreen = windowWidth < 750;

	if (debugModeEnabled) {
        isMobileScreen = windowWidth < 950;
    }

    return CreateLayout(isMobileScreen, animationLerpValue < 0 ? (animationLerpValue + 1) : (1 - animationLerpValue));
    //----------------------------------------------------------------------------------
}

// Dummy main() to please cmake - TODO get wasm working with cmake on this example
int main() {
    return 0;
}
