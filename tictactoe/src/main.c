#define TTT_CLAY_IMPLEMENTATION
#include "ttt_clay.h"

#define TTT_IMPLEMENTATION
#include "ttt.h"

#define CLAY_CLICK_HANDLER_IMPLEMENTATION
#include "clay_click_handler.h"

CLAY_CLICK_HANDLER(tictactoePage, {}, {}, {}, {}, {});
CLAY_CLICK_HANDLER(end, {}, {}, {}, {}, {});
CLAY_CLICK_HANDLER(winner, {}, {}, {}, {}, {});

float PLAY_AGAIN_CELL_MARKS_FADE_OUT = 0;
#define PLAY_AGAIN_CELL_MARKS_FADE_OUT_DURATION 0.5f
CLAY_CLICK_HANDLER(playAgain, 
	{
		if (Clay_PointerOver(clickHandler_playAgain_elementId))
		{
			resetBoard();
		}
	},
	{}, {}, {}, {}
);

int8_t CELL_MARK_INDEX = -1;
float CELL_MARK_FADE_IN[9] = {0};
#define CELL_MARK_FADE_IN_DURATION 0.25f
Clay_Color cellMarkColor(uint8_t cellIndex)
{
	if (BOARD[cellIndex] == ACTIVE_PLAYER_NONE) return COLOR_NONE;
	Clay_Color color = BOARD[cellIndex] == ACTIVE_PLAYER_X
		? COLOR_PLAYER_X : COLOR_PLAYER_O;
	if (CELL_MARK_FADE_IN[cellIndex] > currentTime)
	{
		const float covered = CELL_MARK_FADE_IN[cellIndex] - currentTime;
		color.a = 255 * (1 - covered / CELL_MARK_FADE_IN_DURATION);
	}
	return color;
}
CLAY_CLICK_HANDLER(cellMark, 
	{
		if (Clay_PointerOver(clickHandler_cellMark_elementId)
		&& CELL_MARK_INDEX != -1
		&& BOARD[CELL_MARK_INDEX] == ACTIVE_PLAYER_NONE)
		{
			CELL_MARK_FADE_IN[CELL_MARK_INDEX] = currentTime + CELL_MARK_FADE_IN_DURATION;
			makeMove(CELL_MARK_INDEX);
		}
	},
	{CELL_MARK_INDEX = userData;},
	{if (CLICK_IS(cellMark, NONE) && CELL_MARK_INDEX != -1) makeMove(CELL_MARK_INDEX);},
	{},
	{}
);

int8_t CELL_HOVER_INDEX = -1;
CLAY_CLICK_HANDLER(cellHighlight,
	{CELL_HOVER_INDEX = -1;},
	{CELL_HOVER_INDEX = userData;},
	{CELL_HOVER_INDEX = -1;},
	{CELL_HOVER_INDEX = -1;},
	{}
);

void listenClicks() {
	listen_tictactoePage_click();
	listen_cellHighlight_click();
	listen_cellMark_click();
	listen_end_click();
	listen_playAgain_click();
	listen_winner_click();
}

void HandleTictactoeCellInteraction(ON_HOVER_PARAMS)
{
	handle_cellHighlight_clickInteraction(ON_HOVER_ARGS);
	handle_cellMark_clickInteraction(ON_HOVER_ARGS);
}

void TictactoeWinner() {
	CLAY_BOX(CLAY_ID("TictactoeWinner"),
		(
			boxConfig.color.fill=WINNER == ACTIVE_PLAYER_NONE ? COLOR_BLACK : WINNER == ACTIVE_PLAYER_X ? COLOR_PLAYER_X : COLOR_PLAYER_O,
			boxConfig.color.border=COLOR_WHITE
		),
		layoutConfig.sizing=(Clay_Sizing){CLAY_SIZING_GROW(.max=10 * lineWidth), CLAY_SIZING_GROW(.max=10 * lineWidth)},
		CLAY_FLOATING({.attachment={CLAY_ATTACH_POINT_LEFT_BOTTOM, CLAY_ATTACH_POINT_LEFT_BOTTOM}, .pointerCaptureMode=CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH}),
		Clay_OnHover(handle_winner_clickInteraction, 0)
//		, listen_winner_click()
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
		Clay_OnHover(handle_playAgain_clickInteraction, 0)
//		, listen_playAgain_click()
	) {
		NORMAL_TEXT("Play Again");
		CLAY_LABEL_OR_HOVERED("TictactoePlayAgain",
			(floatingConfig.attachment=(Clay_FloatingAttachPoints){CLAY_ATTACH_POINT_CENTER_BOTTOM, CLAY_ATTACH_POINT_CENTER_TOP}),
			clickHandler_playAgain_clickPhase
		);
	}
}

void TictactoeEnd() {
	if (isGameDone()) {
		CLAY_BOX(CLAY_ID("TictactoeEnd"), 
			(boxConfig.color=(Clay_BoxColor){
			 	Color_AsFaded(COLOR_BLACK),
				Color_AsFaded(COLOR_WHITE)
			}),
			layoutConfig.childAlignment=centerXY,
			CLAY_FLOATING(),
			Clay_OnHover(handle_end_clickInteraction, 0)
//			, listen_end_click()
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
		Clay_OnHover(HandleTictactoeCellInteraction, cellIndex)
//		, cellIndex == 8 ? (listen_cellMark_click(), listen_cellHighlight_click()) : 0
	) {
		CLAY_BOX(CLAY_IDI("TictactoeMove", cellIndex), (boxConfig.color.fill=cellMarkColor(cellIndex)))
		{}
	}
}

void TictactoeGrid() {
	layoutConfig=(Clay_LayoutConfig){{CLAY_SIZING_GROW(0), CLAY_SIZING_PERCENT(windowSmallSide/ windowLongSide)}};
	if (!isPortrait) layoutConfig.sizing = (Clay_Sizing){
		CLAY_SIZING_PERCENT(windowSmallSide / windowLongSide), CLAY_SIZING_GROW(0),
	};
	CLAY(CLAY_LAYOUT(layoutConfig)) {
		CLAY_BOX(CLAY_ID("TictactoeGrid"),
			(boxConfig.color=(Clay_BoxColor){COLOR_WHITE, COLOR_BLACK}),
			CLAY_FLOATING(),
			Clay_OnHover(HandleTictactoeCellInteraction, -1)
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
		Clay_OnHover(handle_tictactoePage_clickInteraction, 0)
		, listen_tictactoePage_click()
	) {
		CLAY_LABEL_OR_HOVERED("TictactoePage", 0, clickHandler_tictactoePage_clickPhase);
		TictactoeGrid();
		TictactoeWinner();
	}
}

float animationLerpValue = -1.0f;

Clay_RenderCommandArray CreateLayout(bool mobileScreen, float lerpValue) {
    Clay_BeginLayout();
	TictactoePage();
	listenClicks();
    return Clay_EndLayout();
}

bool debugModeEnabled = false;



CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(float width, float height, float mouseWheelX, float mouseWheelY, float mousePositionX, float mousePositionY, bool __isTouchDown, bool __isMouseDown, bool __isPointerStart, bool __isPointerCancel, bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame, bool dKeyPressedThisFrame, float __currentTime, float __deltaTime) {
	{
		windowWidth = width, windowHeight = height;
		isPortrait = windowWidth < windowHeight;
		windowLongSide = MAX(windowWidth, windowHeight);
		windowSmallSide = MIN(windowWidth, windowHeight);
		lineWidth = 0.025f * windowSmallSide;
	}
	{
		currentTime = __currentTime;
		deltaTime = __deltaTime;
		updateAverageDeltaTime();
		isPointerStart = __isPointerStart;
		isPointerCancel = __isPointerCancel;
		isPointerDown = __isTouchDown || __isMouseDown;

		if (isPointerCancel)
		{
			isInterrupted = true;
		}
		else if (isInterrupted)
		{
			isInterrupted = !isPointerStart;
		}
		if (isPointerStart)
		{
			pointerStartLock_waitTime = currentTime + MAX(0.01f, averageDeltaTime * 2);
		}
	}

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

    Clay_SetPointerState((Clay_Vector2) {mousePositionX, mousePositionY}, isPointerDown);

    Clay_UpdateScrollContainers(__isTouchDown, (Clay_Vector2) {mouseWheelX, mouseWheelY}, deltaTime);

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
