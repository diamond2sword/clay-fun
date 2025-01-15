#define CLAY_BOX(elementId, __VA_ARGS__2, ...) \
	CLAY( \
		elementId, \
		boxConfig = (Clay_BoxElementConfig){COLOR_NONE, COLOR_NONE, lineWidth, lineWidth}, \
		__VA_ARGS__2, \
		layoutConfig = (Clay_LayoutConfig){sizingGrow, {boxConfig.borderWidth, boxConfig.borderWidth}}, \
		rectangleConfig = (Clay_RectangleElementConfig){boxConfig.fillColor, CLAY_CORNER_RADIUS(boxConfig.cornerRadius)}, \
		##__VA_ARGS__, \
		CLAY_BORDER_OUTSIDE_RADIUS(boxConfig.borderWidth, boxConfig.borderColor, boxConfig.cornerRadius), \
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
	textConfig = (Clay_TextElementConfig){COLOR_WHITE, FONT_ID_TEXT, 0.09f * windowSmallWidth, .disablePointerEvents = true}; \
	("empty", ##__VA_ARGS__); \
	CLAY_TEXT( \
		CLAY_STRING(text), \
		CLAY_TEXT_CONFIG(textConfig) \
	); \



#define CLAY_CLICK_HANDLER(name, __VA_ARGS__1, __VA_ARGS__2, __VA_ARGS__3) \
	float clickHandler_##name##_beforeClickWaitTime; \
	ClickPhase clickHandler_##name##_clickPhase; \
	Clay_ElementId clickHandler_##name##_elementId; \
	void handle_##name##_clickInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData) \
	{ \
		if (pointerInfo.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME \
		|| pointerInfo.state == CLAY_POINTER_DATA_PRESSED) \
		{ \
			if (!isInterrupted) \
			{ \
				__VA_ARGS__2 \
				clickHandler_##name##_elementId = elementId; \
				clickHandler_##name##_clickPhase = CLICK_PHASE_PRESSED_THIS_FRAME; \
				clickHandler_##name##_beforeClickWaitTime = currentTime + MAX(0.01f, averageDeltaTime * 2); \
			} \
		} \
	} \
	void listen_##name##_click() { \
		static float mousePos2_waitTime; \
		if (isInterrupted) \
		{ \
			__VA_ARGS__3 \
			clickHandler_##name##_clickPhase = CLICK_PHASE_NONE; \
			return; \
		} \
		if (clickHandler_##name##_beforeClickWaitTime > currentTime) \
		{ \
			clickHandler_##name##_clickPhase = CLICK_PHASE_PRESSED; \
			return; \
		} \
		if (clickHandler_##name##_clickPhase == CLICK_PHASE_PRESSED) \
		{ \
			clickHandler_##name##_clickPhase = CLICK_PHASE_CHECK_POS_THIS_FRAME; \
			mousePos2_waitTime = currentTime + MAX(0.01f, averageDeltaTime * 2); \
			return; \
		} \
		if (mousePos2_waitTime > currentTime) \
		{ \
			clickHandler_##name##_clickPhase = CLICK_PHASE_CHECK_POS; \
			return; \
		} \
		if (clickHandler_##name##_clickPhase == CLICK_PHASE_CHECK_POS) \
		{ \
			__VA_ARGS__1 \
			clickHandler_##name##_clickPhase = CLICK_PHASE_NONE; \
			return; \
		} \
	}

//#define WRAP_VA_ARGS(...) __VA_ARGS__
#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? b : a)




#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;
#define CLAY_IMPLEMENTATION
#include "clay.h"

Clay_LayoutConfig layoutConfig;
Clay_RectangleElementConfig rectangleConfig;
Clay_FloatingElementConfig floatingConfig;
Clay_TextElementConfig textConfig;
// Clay Box
typedef struct Clay_BoxElementConfig {
	Clay_Color borderColor, fillColor;
	float borderWidth, cornerRadius;
} Clay_BoxElementConfig;
Clay_BoxElementConfig boxConfig;

typedef enum : uint8_t {
	CLICK_PHASE_NONE,
	CLICK_PHASE_PRESSED_THIS_FRAME,
	CLICK_PHASE_PRESSED,
	CLICK_PHASE_CHECK_POS_THIS_FRAME,
	CLICK_PHASE_CHECK_POS,
} ClickPhase;

uint32_t CLAY_LABEL_INDEX = 0;
const uint32_t FONT_ID_TEXT = 0;
double windowWidth = 1024, windowHeight = 768;

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
const Clay_Color COLOR_PLAYER_X = COLOR_BLUE;
const Clay_Color COLOR_PLAYER_O = COLOR_YELLOW;

const Clay_Sizing sizingGrow = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)};
const Clay_ChildAlignment centerXY = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER};




typedef enum : uint8_t {
	ACTIVE_PLAYER_NONE,
	ACTIVE_PLAYER_X,
	ACTIVE_PLAYER_O
} ActivePlayer;

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

ActivePlayer BOARD[9] = {0};
ActivePlayer ACTIVE_PLAYER = ACTIVE_PLAYER_X;
ActivePlayer WINNER = ACTIVE_PLAYER_NONE;
WinSlope BOARD_WIN_SLOPE_MAP[9] = {0};
uint8_t MOVE_COUNTER = 0;

uint8_t chk3(uint8_t init_index, uint8_t offset, uint8_t init_index_offset, uint8_t max_inits, ActivePlayer value)
{
	for (uint8_t i = 0; i < 9; i++) {
		BOARD_WIN_SLOPE_MAP[i] = (WinSlope) {0};
	}
	for (uint8_t inits = 0; inits < max_inits; inits++)
	{
		uint8_t cur_init_index = inits * init_index_offset;
		uint8_t is_win_line = 1;
		for (uint8_t cell_index = 0; cell_index < 3; cell_index++)
		{
			const uint8_t cur_index = init_index + cur_init_index + cell_index * offset;
			if (BOARD[cur_index] == value) continue;
			is_win_line = 0; break;
		}
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
void updateWinner(ActivePlayer value) {
	chk3(0, 1, 3, 3, value);
	chk3(2, 2, 0, 1, value);
	chk3(0, 3, 1, 3, value);
	chk3(0, 4, 0, 1, value);
}
void resetBoard() {
	for (int i = 0; i < 9; i++) BOARD[i] = 0;
	WINNER = ACTIVE_PLAYER_NONE;
	MOVE_COUNTER = 0;
	ACTIVE_PLAYER = ACTIVE_PLAYER_X;
}
void makeMove(int8_t cellIndex)
{
	if (WINNER || BOARD[cellIndex]) return;
	BOARD[cellIndex] = ACTIVE_PLAYER;
	updateWinner(ACTIVE_PLAYER);
	ACTIVE_PLAYER = ACTIVE_PLAYER == ACTIVE_PLAYER_X ? ACTIVE_PLAYER_O : ACTIVE_PLAYER_X;
	MOVE_COUNTER++;
}




float currentTime;
float deltaTime;
float averageDeltaTime;
bool isPointerCancel;
bool isPointerStart;
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


void HandleTictactoeCellInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData)
{
	handle_cellHighlight_clickInteraction(elementId, pointerInfo, userData);
	handle_cellMark_clickInteraction(elementId, pointerInfo, userData);
}

double windowSmallWidth;
float lineWidth;

void TictactoeWinner() {
	CLAY_BOX(CLAY_ID("TictactoeWinner"),
		(
			boxConfig.fillColor=WINNER == ACTIVE_PLAYER_NONE ? COLOR_BLACK : WINNER == ACTIVE_PLAYER_X ? COLOR_PLAYER_X : COLOR_PLAYER_O,
			boxConfig.borderColor=COLOR_WHITE
		),
		layoutConfig.sizing=(Clay_Sizing){CLAY_SIZING_GROW(.max=10 * lineWidth), CLAY_SIZING_GROW(.max=10 * lineWidth)},
		CLAY_FLOATING({.attachment={CLAY_ATTACH_POINT_LEFT_BOTTOM, CLAY_ATTACH_POINT_LEFT_BOTTOM}}),
		Clay_OnHover(handle_winner_clickInteraction, 0),
		listen_winner_click()
	) {
		CLAY_LABEL_OR_HOVERED("TictactoeWinner", 0, clickHandler_winner_clickPhase);
	}
}

void TictactoePlayAgain() {
	CLAY_BOX(CLAY_ID("TictactoePlayAgain"),
		(
			boxConfig.fillColor=Color_AsFaded(clickHandler_playAgain_clickPhase
				? COLOR_BLACK : COLOR_GREEN),
			boxConfig.borderColor=Color_AsFaded(clickHandler_playAgain_clickPhase
				? COLOR_WHITE : COLOR_RED)
		),
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
			(
				boxConfig.fillColor=Color_AsFaded(COLOR_BLACK),
				boxConfig.borderColor=Color_AsFaded(COLOR_WHITE)
			),
			layoutConfig.childAlignment=centerXY,
			CLAY_FLOATING(),
			Clay_OnHover(handle_end_clickInteraction, 0),
			listen_end_click()
		) {
			CLAY_LABEL_OR_HOVERED("TictactoeEnd", 0, clickHandler_end_clickPhase);
			TictactoePlayAgain();
		}
	}
}

void TicTactoeCell(uint8_t cellIndex/*, void (*onHoverFunction)(Clay_ElementId, Clay_PointerData, intptr_t)*/) {
	CLAY_BOX(CLAY_IDI("TictactoeCell", cellIndex),
		(
			boxConfig.fillColor=CELL_HOVER_INDEX == cellIndex ? COLOR_WHITE : COLOR_GREEN,
			boxConfig.borderColor=CELL_HOVER_INDEX == cellIndex ? COLOR_BLACK : COLOR_RED
		),
		Clay_OnHover(HandleTictactoeCellInteraction, cellIndex),
		cellIndex == 8 ? (listen_cellMark_click(), listen_cellHighlight_click()) : 0
	) {
		if (BOARD[cellIndex] == ACTIVE_PLAYER_X) {
			CLAY_BOX(CLAY_IDI("TictactoeMove", cellIndex), (boxConfig.fillColor=COLOR_PLAYER_X))
			{}
		} else if (BOARD[cellIndex] == ACTIVE_PLAYER_O) {
			CLAY_BOX(CLAY_IDI("TictactoeMove", cellIndex), (boxConfig.fillColor=COLOR_PLAYER_O))
			{}
		}
	}
}

void TictactoeGrid() {
	layoutConfig=(Clay_LayoutConfig){{CLAY_SIZING_GROW(0), CLAY_SIZING_PERCENT(windowWidth / windowHeight)}};
	CLAY(CLAY_LAYOUT(layoutConfig)) {
		CLAY_BOX(CLAY_ID("TictactoeGrid"),
			(boxConfig.fillColor=COLOR_WHITE, boxConfig.borderColor=COLOR_BLACK),
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
				(floatingConfig.attachment=(Clay_FloatingAttachPoints){CLAY_ATTACH_POINT_CENTER_BOTTOM, CLAY_ATTACH_POINT_CENTER_TOP}),
				clickHandler_cellHighlight_clickPhase
			);

			TictactoeEnd();
		}
	}
}

void TictactoePage() {
	windowSmallWidth = (windowWidth < windowHeight ? windowWidth : windowHeight);
	lineWidth = 0.025f * windowSmallWidth;

	CLAY_BOX(CLAY_ID("TictactoePage"),
		(boxConfig.fillColor=COLOR_GREEN, boxConfig.borderColor=COLOR_RED),
		layoutConfig.childAlignment=centerXY,
		Clay_OnHover(handle_tictactoePage_clickInteraction, 0),
		listen_tictactoePage_click()
	) {
		TictactoeGrid();
		CLAY_LABEL_OR_HOVERED("TictactoePage", 0, clickHandler_tictactoePage_clickPhase);
		TictactoeWinner();
	}
}

float animationLerpValue = -1.0f;

Clay_RenderCommandArray CreateLayout(bool mobileScreen, float lerpValue) {
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
	isPointerCancel = __isPointerCancel;
	isPointerStart = __isPointerStart;
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
