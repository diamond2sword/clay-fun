#define CLAY_BOX(elementId, __VA_ARGS__2, ...) \
	CLAY( \
		elementId, \
		boxConfig = (Clay_BoxElementConfig){{COLOR_NONE, COLOR_NONE}, {lineWidth, lineWidth}}, \
		__VA_ARGS__2, \
		layoutConfig = (Clay_LayoutConfig){sizingGrow, {boxConfig.sizing.border, boxConfig.sizing.border}}, \
		rectangleConfig = (Clay_RectangleElementConfig){boxConfig.color.fill, CLAY_CORNER_RADIUS(boxConfig.sizing.corner)}, \
		##__VA_ARGS__, \
		CLAY_BORDER_OUTSIDE_RADIUS(boxConfig.sizing.border, boxConfig.color.border, boxConfig.sizing.corner), \
		CLAY_LAYOUT(layoutConfig), \
		CLAY_RECTANGLE(rectangleConfig) \
	) 

#define CLAY_LABEL(text, ...) \
	rectangleConfig = (Clay_RectangleElementConfig){Color_AsFaded(COLOR_BLACK), CLAY_CORNER_RADIUS(lineWidth)}; \
	floatingConfig = (Clay_FloatingElementConfig){.pointerCaptureMode=CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH}; \
	CLAY( \
		CLAY_IDI("ClayLabel", CLAY_LABEL_INDEX++), \
		##__VA_ARGS__, \
		CLAY_RECTANGLE(rectangleConfig),  \
		CLAY_FLOATING(floatingConfig) \
	) { \
		NORMAL_TEXT(text); \
	}

#define CLAY_LABEL_OR_HOVERED(text, __VA_ARGS__2, ...) \
	if (Clay_Hovered(), ##__VA_ARGS__) { CLAY_LABEL("Pressed", __VA_ARGS__2) } \
	else { CLAY_LABEL(text, __VA_ARGS__2) }


#define NORMAL_TEXT(text, ...) \
	textConfig = (Clay_TextElementConfig){COLOR_WHITE, FONT_ID_TEXT, 0.09f * windowSmallSide, .disablePointerEvents = true}; \
	("empty", ##__VA_ARGS__); \
	CLAY_TEXT( \
		CLAY_STRING(text), \
		CLAY_TEXT_CONFIG(textConfig) \
	);

#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;
#define CLAY_IMPLEMENTATION
#include "clay.h"

typedef struct Clay_BoxColor {
	Clay_Color fill;
	Clay_Color border;
} Clay_BoxColor;
typedef struct Clay_BoxSizing {
	float border;
	float corner;
} Clay_BoxSizing;
typedef struct Clay_BoxElementConfig {
	Clay_BoxColor color;
	Clay_BoxSizing sizing;
} Clay_BoxElementConfig;
	
Clay_BoxElementConfig boxConfig;
Clay_LayoutConfig layoutConfig;
Clay_RectangleElementConfig rectangleConfig;
Clay_FloatingElementConfig floatingConfig;
Clay_TextElementConfig textConfig;
float lineWidth;
uint32_t CLAY_LABEL_INDEX = 0;
const uint32_t FONT_ID_TEXT = 0;
// Clay Box


Clay_Color Color_AsFaded(Clay_Color color) {
	color.a /= 2;
	return color;
}
const Clay_Color COLOR_WHITE = (Clay_Color) {255, 255, 255, 255};
const Clay_Color COLOR_BLACK = (Clay_Color) {0, 0, 0, 255};
const Clay_Color COLOR_RED = (Clay_Color) {255, 0, 0, 255};
const Clay_Color COLOR_GREEN = (Clay_Color) {0, 255, 0, 255};
const Clay_Color COLOR_YELLOW = (Clay_Color) {255, 255, 0, 255};
const Clay_Color COLOR_NONE = (Clay_Color) {0, 0, 0, 0};
const Clay_Color COLOR_BLUE = (Clay_Color) {0, 0, 255, 255};

const Clay_Sizing sizingGrow = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)};
const Clay_ChildAlignment centerXY = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER};


#define TTT_IMPLEMENTATION
#include "ttt.h"

const Clay_Color COLOR_PLAYER_X = COLOR_BLUE;
const Clay_Color COLOR_PLAYER_O = COLOR_YELLOW;





#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? b : a)

#define CLICK_IS(name, phase_token) clickHandler_##name##_clickPhase == CLICK_PHASE_##phase_token
#define CLICK_SET(name, phase_token) clickHandler_##name##_clickPhase = CLICK_PHASE_##phase_token
#define POINTER_IS(data_token) pointerInfo.state == CLAY_POINTER_DATA_##data_token
#define ON_HOVER_PARAMS Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData
#define ON_HOVER_ARGS elementId, pointerInfo, userData

#define CLAY_CLICK_HANDLER(name, __VA_ARGS__1, __VA_ARGS__2, __VA_ARGS__3) \
	float clickHandler_##name##_beforeClickWaitTime; \
	ClickPhase clickHandler_##name##_clickPhase; \
	Clay_ElementId clickHandler_##name##_elementId; \
	void handle_##name##_clickInteraction(ON_HOVER_PARAMS) \
	{ \
		if (POINTER_IS(PRESSED_THIS_FRAME) || POINTER_IS(PRESSED)) \
		{ \
			if (!isInterrupted) \
			{ \
				__VA_ARGS__2 \
				clickHandler_##name##_elementId = elementId; \
				CLICK_SET(name, PRESSED_THIS_FRAME); \
				clickHandler_##name##_beforeClickWaitTime = currentTime + MAX(0.01f, averageDeltaTime * 2); \
			} \
		} \
	} \
	void listen_##name##_click() { \
		static float mousePos2_waitTime; \
		if (isInterrupted) \
		{ \
			__VA_ARGS__3 \
			CLICK_SET(name, NONE); \
			return; \
		} \
		if (clickHandler_##name##_beforeClickWaitTime > currentTime) \
		{ \
			CLICK_SET(name, PRESSED); \
			return; \
		} \
		if (CLICK_IS(name, PRESSED)) \
		{ \
			CLICK_SET(name, CHECK_POS_THIS_FRAME); \
			mousePos2_waitTime = currentTime + MAX(0.01f, averageDeltaTime * 2); \
			return; \
		} \
		if (mousePos2_waitTime > currentTime) \
		{ \
			CLICK_SET(name, CHECK_POS); \
			return; \
		} \
		if (CLICK_IS(name, CHECK_POS)) \
		{ \
			__VA_ARGS__1 \
			CLICK_SET(name, NONE); \
			return; \
		} \
	}
//#define WRAP_VA_ARGS(...) __VA_ARGS__

typedef enum : uint8_t {
	CLICK_PHASE_NONE,
	CLICK_PHASE_PRESSED_THIS_FRAME,
	CLICK_PHASE_PRESSED,
	CLICK_PHASE_CHECK_POS_THIS_FRAME,
	CLICK_PHASE_CHECK_POS,
} ClickPhase;

double windowWidth = 1024, windowHeight = 768;
bool isPortrait;
bool isPointerCancel;
bool isPointerStart;
float deltaTime;
float currentTime;
float averageDeltaTime;
bool isInterrupted;

CLAY_CLICK_HANDLER(tictactoePage, {}, {}, {});
CLAY_CLICK_HANDLER(end, {}, {}, {});
CLAY_CLICK_HANDLER(winner, {}, {}, {});

CLAY_CLICK_HANDLER(playAgain, 
	{if (Clay_PointerOver(clickHandler_playAgain_elementId)) resetBoard();},
	{}, {}
);

int8_t CELL_MARK_INDEX = -1;
CLAY_CLICK_HANDLER(cellMark, 
	{if (Clay_PointerOver(clickHandler_cellMark_elementId)) makeMove(CELL_MARK_INDEX);},
	{CELL_MARK_INDEX = userData;},
	{if (!clickHandler_tictactoePage_clickPhase) makeMove(CELL_MARK_INDEX);}
);

int8_t CELL_HOVER_INDEX = -1;
CLAY_CLICK_HANDLER(cellHighlight,
	{CELL_HOVER_INDEX = -1;},
	{CELL_HOVER_INDEX = userData;},
	{CELL_HOVER_INDEX = -1;}
);


void HandleTictactoeCellInteraction(ON_HOVER_PARAMS)
{
	handle_cellHighlight_clickInteraction(ON_HOVER_ARGS);
	handle_cellMark_clickInteraction(ON_HOVER_ARGS);
}

double windowSmallSide;
double windowLongSide;

void TictactoeWinner() {
	CLAY_BOX(CLAY_ID("TictactoeWinner"),
		(
			boxConfig.color.fill=WINNER == ACTIVE_PLAYER_NONE ? COLOR_BLACK : WINNER == ACTIVE_PLAYER_X ? COLOR_PLAYER_X : COLOR_PLAYER_O,
			boxConfig.color.border=COLOR_WHITE
		),
		layoutConfig.sizing=(Clay_Sizing){CLAY_SIZING_GROW(.max=10 * lineWidth), CLAY_SIZING_GROW(.max=10 * lineWidth)},
		CLAY_FLOATING({.attachment={CLAY_ATTACH_POINT_LEFT_BOTTOM, CLAY_ATTACH_POINT_LEFT_BOTTOM}, .pointerCaptureMode=CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH}),
		Clay_OnHover(handle_winner_clickInteraction, 0),
		listen_winner_click()
	) {
		CLAY_LABEL_OR_HOVERED("TictactoeWinner", 0, clickHandler_winner_clickPhase);
	}
}

void TictactoePlayAgain() {
	CLAY_BOX(CLAY_ID("TictactoePlayAgain"),
		(boxConfig.color=(Clay_BoxColor){
			Color_AsFaded(clickHandler_playAgain_clickPhase
				? COLOR_BLACK : COLOR_GREEN),
			Color_AsFaded(clickHandler_playAgain_clickPhase
				? COLOR_WHITE : COLOR_RED)
		}),
		layoutConfig.sizing=(Clay_Sizing){CLAY_SIZING_FIT(), CLAY_SIZING_FIT()},
		Clay_OnHover(handle_playAgain_clickInteraction, 0),
		listen_playAgain_click()
	) {
		NORMAL_TEXT("Play Again");
		CLAY_LABEL_OR_HOVERED("TictactoePlayAgain",
			(floatingConfig.attachment=(Clay_FloatingAttachPoints){CLAY_ATTACH_POINT_CENTER_BOTTOM, CLAY_ATTACH_POINT_CENTER_TOP}),
			clickHandler_playAgain_clickPhase
		);
	}
}

void TictactoeEnd() {
	if (WINNER || MOVE_COUNTER == 9) {
		CLAY_BOX(CLAY_ID("TictactoeEnd"), 
			(boxConfig.color=(Clay_BoxColor){
			 	Color_AsFaded(COLOR_BLACK),
				Color_AsFaded(COLOR_WHITE)
			}),
			layoutConfig.childAlignment=centerXY,
			CLAY_FLOATING(),
			Clay_OnHover(handle_end_clickInteraction, 0),
			listen_end_click()
		) {
			TictactoePlayAgain();
			CLAY_LABEL_OR_HOVERED("TictactoeEnd", 0, clickHandler_end_clickPhase);
		}
	}
}

void TicTactoeCell(uint8_t cellIndex) {
	CLAY_BOX(CLAY_IDI("TictactoeCell", cellIndex),
		(boxConfig.color=CELL_HOVER_INDEX == cellIndex
		 	? (Clay_BoxColor) {COLOR_WHITE, COLOR_BLACK}
			: (Clay_BoxColor)  {COLOR_GREEN, COLOR_RED}
		),
		Clay_OnHover(HandleTictactoeCellInteraction, cellIndex),
		cellIndex == 8 ? (listen_cellMark_click(), listen_cellHighlight_click()) : 0
	) {
		if (BOARD[cellIndex] == ACTIVE_PLAYER_X) {
			CLAY_BOX(CLAY_IDI("TictactoeMove", cellIndex), (boxConfig.color.fill=COLOR_PLAYER_X))
			{}
		} else if (BOARD[cellIndex] == ACTIVE_PLAYER_O) {
			CLAY_BOX(CLAY_IDI("TictactoeMove", cellIndex), (boxConfig.color.fill=COLOR_PLAYER_O))
			{}
		}
	}
}

void TictactoeGrid() {
	layoutConfig=(Clay_LayoutConfig){{CLAY_SIZING_GROW(0), CLAY_SIZING_PERCENT(windowSmallSide/ windowLongSide)}};
	if (!isPortrait) layoutConfig.sizing = (Clay_Sizing){
		CLAY_SIZING_PERCENT(windowSmallSide / windowLongSide), CLAY_SIZING_GROW(0)
	};
	CLAY(CLAY_LAYOUT(layoutConfig)) {
		CLAY_BOX(CLAY_ID("TictactoeGrid"),
			(boxConfig.color=(Clay_BoxColor){COLOR_WHITE, COLOR_BLACK}),
			CLAY_FLOATING()
		) {
			CLAY(CLAY_LAYOUT({sizingGrow, .layoutDirection=CLAY_TOP_TO_BOTTOM})) {for (int row = 0; row < 3; row++)
			{
				CLAY(CLAY_LAYOUT({sizingGrow})) {for (int col = 0; col < 3; col++)
				{
					TicTactoeCell(row * 3 + col);
				}}
			}}

			CLAY_LABEL_OR_HOVERED("TictactoeGrid",
				(
					floatingConfig.attachment.parent=CLAY_ATTACH_POINT_CENTER_TOP,
					floatingConfig.attachment.element=isPortrait ? CLAY_ATTACH_POINT_CENTER_BOTTOM : CLAY_ATTACH_POINT_CENTER_TOP
				),
				clickHandler_cellHighlight_clickPhase
			);

			TictactoeEnd();
		}
	}
}

void TictactoePage() {
	CLAY_BOX(CLAY_ID("TictactoePage"),
		(boxConfig.color=(Clay_BoxColor){COLOR_GREEN, COLOR_RED}),
		layoutConfig.childAlignment=centerXY,
		layoutConfig.sizing=(Clay_Sizing){CLAY_SIZING_FIXED(windowWidth), CLAY_SIZING_FIXED(windowHeight)},
		Clay_OnHover(handle_tictactoePage_clickInteraction, 0),
		listen_tictactoePage_click()
	) {
		CLAY_LABEL_OR_HOVERED("TictactoePage", 0, clickHandler_tictactoePage_clickPhase);
		TictactoeGrid();
		TictactoeWinner();
	}
}

float animationLerpValue = -1.0f;

Clay_RenderCommandArray CreateLayout(bool mobileScreen, float lerpValue) {
	windowSmallSide = MIN(windowWidth, windowHeight);
	windowLongSide = MAX(windowWidth, windowHeight);
	lineWidth = 0.025f * windowSmallSide;
	isPortrait = windowWidth < windowHeight;
    Clay_BeginLayout();
	TictactoePage();
    return Clay_EndLayout();
}

bool debugModeEnabled = false;


#define N_STEPS_AVERAGE_DELTA_TIME_CYCLE 20
void updateAverageDeltaTime()
{
	static float cycleData[N_STEPS_AVERAGE_DELTA_TIME_CYCLE] = {0};
	static int cycleIndex = 0;
	static bool isPopulated = false;
	float deltaTimeSum = 0;
	if (isPopulated)
	{
		for (int i = 0; i < N_STEPS_AVERAGE_DELTA_TIME_CYCLE; i++)
		{
			deltaTimeSum += cycleData[i];
		}
	}
	else
	{
		for (int i = 0; i <= cycleIndex; i++)
		{
			deltaTimeSum += cycleData[i];
		}
		isPopulated = cycleIndex + 1 == N_STEPS_AVERAGE_DELTA_TIME_CYCLE;
	}
	averageDeltaTime = deltaTimeSum / N_STEPS_AVERAGE_DELTA_TIME_CYCLE;
	cycleData[cycleIndex] = deltaTime;
	cycleIndex = cycleIndex + 1 == N_STEPS_AVERAGE_DELTA_TIME_CYCLE ? 0 : cycleIndex + 1;
}


CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(float width, float height, float mouseWheelX, float mouseWheelY, float mousePositionX, float mousePositionY, bool isTouchDown, bool isMouseDown, bool __isPointerStart, bool __isPointerCancel, bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame, bool dKeyPressedThisFrame, float __currentTime, float __deltaTime) {
	currentTime = __currentTime;
	deltaTime = __deltaTime;
	isPointerStart = __isPointerStart;
	isPointerCancel = __isPointerCancel;
	if (isPointerCancel)
	{
		isInterrupted = true;
	} else if (isInterrupted)
	{
		isInterrupted = !isPointerStart;
	}
	updateAverageDeltaTime();
	
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
