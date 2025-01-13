#include <stdint.h>
#define CLAY_BOX(elementId, fillColor, borderColor, borderWidth, cornerRadius, ...) \
	CLAY( \
		elementId, \
		CLAY_LAYOUT({sizingGrow, {borderWidth, borderWidth}}), \
		CLAY_RECTANGLE({fillColor, CLAY_CORNER_RADIUS(cornerRadius)}), \
		CLAY_BORDER_OUTSIDE_RADIUS(borderWidth, borderColor, cornerRadius), \
		##__VA_ARGS__ \
	) 

#define CLAY_LABEL(text, ...) \
	CLAY( \
		CLAY_IDI("ClayLabel", CLAY_LABEL_INDEX++), \
		CLAY_RECTANGLE({Color_AsFaded(COLOR_BLACK), CLAY_CORNER_RADIUS(lineWidth)}),  \
		##__VA_ARGS__, \
		CLAY_FLOATING({.pointerCaptureMode=CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH}) \
	) { \
		NORMAL_TEXT(text); \
	}

#define CLAY_LABEL_OR_HOVERED(text, __VA_ARGS__2, ...) \
	if (Clay_Hovered(), ##__VA_ARGS__) { CLAY_LABEL("Pressed", __VA_ARGS__2) } \
	else { CLAY_LABEL(text, __VA_ARGS__2) }


#define NORMAL_TEXT(text) \
	CLAY_TEXT( \
		CLAY_STRING(text), \
		CLAY_TEXT_CONFIG({COLOR_WHITE, FONT_ID_TEXT, 0.09f * windowSmallWidth, .disablePointerEvents = true}) \
	)

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





float currentTime;
float deltaTime;
float averageDeltaTime;
bool isPointerCancel;
bool isPointerStart;
bool isInterrupted;

CLAY_CLICK_HANDLER(tictactoePage, {}, {}, {});
CLAY_CLICK_HANDLER(end, {}, {}, {});
CLAY_CLICK_HANDLER(winner, {}, {}, {});

uint8_t MOVE_COUNTER = 0;
CLAY_CLICK_HANDLER(playAgain, {
	if (Clay_PointerOver(clickHandler_playAgain_elementId))
	{
		for (int i = 0; i < 9; i++) BOARD[i] = 0;
		WINNER = ACTIVE_PLAYER_NONE;
		MOVE_COUNTER = 0;
		ACTIVE_PLAYER = ACTIVE_PLAYER_X;
	}
}, {}, {});

#define RESET_BOARD() \
	if (!WINNER && !BOARD[CELL_MARK_INDEX]) \
	{ \
		BOARD[CELL_MARK_INDEX] = ACTIVE_PLAYER; \
		updateWinner(ACTIVE_PLAYER); \
		ACTIVE_PLAYER = ACTIVE_PLAYER == ACTIVE_PLAYER_X ? ACTIVE_PLAYER_O : ACTIVE_PLAYER_X; \
		MOVE_COUNTER++; \
	}
int8_t CELL_MARK_INDEX = -1;
CLAY_CLICK_HANDLER(cellMark, {
	if (Clay_PointerOver(clickHandler_cellMark_elementId))
	{
		RESET_BOARD();
	}
}, {
	CELL_MARK_INDEX = userData;
}, {
	if (!clickHandler_tictactoePage_clickPhase
	&& Clay_PointerOver(clickHandler_cellMark_elementId))
	{
		RESET_BOARD();
	}
});

int8_t CELL_HOVER_INDEX = -1;
CLAY_CLICK_HANDLER(cellHighlight, {
	CELL_HOVER_INDEX = -1;
}, {
	CELL_HOVER_INDEX = userData;
}, {
	CELL_HOVER_INDEX = -1;
});


void HandleTictactoeCellInteraction(Clay_ElementId elementId, Clay_PointerData pointerInfo, intptr_t userData)
{
	handle_cellHighlight_clickInteraction(elementId, pointerInfo, userData);
	handle_cellMark_clickInteraction(elementId, pointerInfo, userData);
}

double windowSmallWidth;
float lineWidth;

void TictactoeWinner() {
	CLAY_BOX(
		CLAY_ID("TictactoeWinner"),
		WINNER == ACTIVE_PLAYER_NONE ? COLOR_BLACK : WINNER == ACTIVE_PLAYER_X ? COLOR_PLAYER_X : COLOR_PLAYER_O,
		COLOR_WHITE, lineWidth, lineWidth,
		CLAY_LAYOUT({
			{CLAY_SIZING_GROW(.max=10 * lineWidth), CLAY_SIZING_GROW(.max=10 * lineWidth)},
			{lineWidth, lineWidth}
		}),
		CLAY_FLOATING({
			.pointerCaptureMode=CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
			.attachment={.element=CLAY_ATTACH_POINT_LEFT_BOTTOM, .parent=CLAY_ATTACH_POINT_LEFT_BOTTOM}
		}),
		Clay_OnHover(handle_winner_clickInteraction, 0),
		listen_winner_click()
	) {
		CLAY_LABEL_OR_HOVERED("TictactoeWinner", 0, clickHandler_winner_clickPhase);
	}
}

void TictactoePlayAgain() {
	CLAY_BOX(CLAY_ID("TictactoePlayAgain"), 
		Color_AsFaded(clickHandler_playAgain_clickPhase
			? COLOR_BLACK : COLOR_GREEN),
		Color_AsFaded(clickHandler_playAgain_clickPhase
			? COLOR_WHITE : COLOR_RED),
		lineWidth,
		lineWidth,
		CLAY_LAYOUT({
			{CLAY_SIZING_FIT(), CLAY_SIZING_FIT()},
			{lineWidth, lineWidth}
		}),
		Clay_OnHover(handle_playAgain_clickInteraction, 0),
		listen_playAgain_click()
	) {
		NORMAL_TEXT("Play Again");
		CLAY_LABEL_OR_HOVERED("TictactoePlayAgain",
			CLAY_FLOATING({
				.attachment={CLAY_ATTACH_POINT_CENTER_BOTTOM, CLAY_ATTACH_POINT_CENTER_TOP},
				.pointerCaptureMode=CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH
			}),
			clickHandler_playAgain_clickPhase
		);
	}
}

void TictactoeEnd() {
	if (WINNER || MOVE_COUNTER == 9) {
		CLAY_BOX(CLAY_ID("TictactoeEnd"), 
			Color_AsFaded(COLOR_BLACK),
			Color_AsFaded(COLOR_WHITE),
			lineWidth,
			lineWidth,
			CLAY_LAYOUT({sizingGrow, {lineWidth, lineWidth}, .childAlignment=centerXY}),
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
		CELL_HOVER_INDEX == cellIndex ? COLOR_WHITE : COLOR_GREEN,
		CELL_HOVER_INDEX == cellIndex ? COLOR_BLACK : COLOR_RED,
		lineWidth, lineWidth,
		Clay_OnHover(HandleTictactoeCellInteraction, cellIndex),
		(cellIndex == 8 ? (
			listen_cellMark_click(),
			listen_cellHighlight_click()
		) : 0)
	) {
		if (BOARD[cellIndex] == ACTIVE_PLAYER_X) {
			CLAY_BOX(CLAY_IDI("TictactoeMove", cellIndex), COLOR_PLAYER_X, COLOR_NONE, lineWidth, lineWidth)
			{}
		} else if (BOARD[cellIndex] == ACTIVE_PLAYER_O) {
			CLAY_BOX(CLAY_IDI("TictactoeMove", cellIndex), COLOR_PLAYER_O, COLOR_NONE, lineWidth, lineWidth)
			{}
		}
	}
}

void TictactoeGrid() {
	CLAY(
		CLAY_LAYOUT({
			{CLAY_SIZING_GROW(0), CLAY_SIZING_PERCENT(windowWidth / windowHeight)}
		})
	) {
		CLAY_BOX(CLAY_ID("TictactoeGrid"),
			COLOR_WHITE, COLOR_BLACK,
			lineWidth, lineWidth, 
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
				CLAY_FLOATING({
					.attachment={CLAY_ATTACH_POINT_CENTER_BOTTOM, CLAY_ATTACH_POINT_CENTER_TOP},
					.pointerCaptureMode=CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH
				}),
				clickHandler_cellHighlight_clickPhase
			);

			TictactoeEnd();
		}
	}
}

void TictactoePage() {
	windowSmallWidth = (windowWidth < windowHeight ? windowWidth : windowHeight);
	lineWidth = 0.025f * windowSmallWidth;

	CLAY_BOX(CLAY_ID("TictactoePage"), COLOR_GREEN, COLOR_RED, lineWidth, lineWidth,
		CLAY_LAYOUT({
			sizingGrow,
			{lineWidth, lineWidth},
			.childAlignment=centerXY
		}),
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

#define N_STEPS_AVERAGE_DELTA_TIME_CYCLE 5
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
