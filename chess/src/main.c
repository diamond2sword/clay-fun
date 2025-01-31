#include "chess/chess_fen.h.c"
#include <stdarg.h>

#define CLAY_CLICK_HANDLER_IMPLEMENTATION
#include "clay_click_handler.h.c"

#define CLAY_HELPERS_IMPLEMENTATION
#include "clay_helpers.h.c"

#define CLAY_IMPLEMENTATION
#include "web_clay.h.c"

#define CHESS_ARENA_SIZE 65536
#define CHESS_IMPLEMENTATION
#include "chess.h.c"

CLAY_WASM_EXPORT("Init") void Init();

void BoardBackground();
void BoardPieceSprite(uint8_t row, uint8_t col);
void BoardPieceSprites();
void BoardPromotionPopup();
void BoardClickLayer();

Clay_RenderCommandArray CreateLayout();

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


double BOARD_WIDTH;
double BOARD_CELL_WIDTH;


Game* GAME;
BoardClicker* BOARD_CLICKER;
ChessArena arena;

CLAY_WASM_EXPORT("Init") void Init()
{
	ChessArena_Init(&arena);
	Game_Init(&arena);
	GAME = Game_New(&arena, Game_Set_FromFen_Default);
	BOARD_CLICKER = BoardClicker_New(&arena, GAME);
};


CLAY_CLICK_HANDLER(cell,
	{BoardClicker_Click(BOARD_CLICKER);},
	{BOARD_CLICKER->index_input = userData;},
	{BoardClicker_Click(BOARD_CLICKER);},
	{BOARD_CLICKER->index_input = -1;},
	{}
);

CLAY_CLICK_HANDLER(claimDraw, {}, {}, {}, {}, {});


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
	const uint64_t occupied = MASK_ALL(GAME->bitboardSet);
	for (uint8_t row = 0; row < 8; row++)
	for (uint8_t col = 0; col < 8; col++)
	{
		const uint8_t index = row * 8 + col;
		if (MASK_INDEX(index) & occupied)
		{
			Piece piece = Piece_New(GAME->bitboardSet, index);
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
	if (BOARD_CLICKER->index_promotion_options == -1)
	{
		return;
	}
	for (uint8_t optIndex = 0; optIndex < 4; optIndex++)
	{
		const uint8_t index = BOARD_CLICKER->index_promotion_options + OFFSET_DOWN * optIndex * DIRECT_OFFSET_SIGN(BOARD_CLICKER->move_temp.src.side);
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
			CLAY_RECTANGLE((MASK_INDEX(index) & MASK_CHECKER) ? colorBlack : colorWhite),
			CLAY_FLOATING({.offset={Index_AsCol(index) * BOARD_CELL_WIDTH, Index_AsRow(index) * BOARD_CELL_WIDTH}})
		)
		BoardPieceSprite(BOARD_CLICKER->move_temp.src.side, MAP_PROMOTION_OPT_TO_TYPE[optIndex]);
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
		Clay_Color color = MASK_INDEX_SIGNED(index) & *(BOARD_CLICKER->bitboard_selectables)
			? (index == BOARD_CLICKER->index_input ? RGBA(0, 255, 0, 128) : RGBA(0, 0, 0, 0))
			: (index == BOARD_CLICKER->index_input ? RGBA(255, 0, 0, 128) : RGBA(0, 0, 0, 128));
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
	CLAY(CLAY_LAYOUT({.layoutDirection=CLAY_TOP_TO_BOTTOM}))
	{
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_WIDTH, BOARD_WIDTH)))
		{
			BoardBackground();
			BoardPieceSprites();
			BoardPromotionPopup();
			BoardClickLayer();
		}
		CLAY(CLAY_LAYOUT({.layoutDirection=CLAY_TOP_TO_BOTTOM}))
		{
			CLAY(0)
			{
				NORMAL_TEXT("Half Moves: ", textConfig.textColor = COLOR_BLACK);
				Normal_Text(Clay__IntToString(GAME->counter_halfMove), textConfig.textColor = COLOR_BLACK);
			}
			CLAY(0)
			{
				NORMAL_TEXT("Full Moves: ", textConfig.textColor = COLOR_BLACK);
				Normal_Text(Clay__IntToString(GAME->counter_fullMove), textConfig.textColor = COLOR_BLACK);
			}
			CLAY(0)
			{
				NORMAL_TEXT("Repeatable: ", textConfig.textColor = COLOR_BLACK);
				Normal_Text(Clay__IntToString(GAME->counter_repeatable), textConfig.textColor = COLOR_BLACK);
			}
			if (false) CLAY(0)
			{
				NORMAL_TEXT("Current Hash Distance: ", textConfig.textColor = COLOR_BLACK);
				Normal_Text(Clay__IntToString(GAME->table_hash->current - GAME->table_hash->begin), textConfig.textColor = COLOR_BLACK);
			}
			if (GAME->counter_repeatable > 0)
			{
				uint64_t* hash_last = GAME->table_hash->current;
				for (uint8_t i = 0; i < GAME->counter_repeatable; i++, hash_last--)
				CLAY(0)
				{
					NORMAL_TEXT("Hash ", textConfig.textColor = COLOR_BLACK);
					Normal_Text(Clay__IntToString(i), textConfig.textColor = COLOR_BLACK);
					NORMAL_TEXT(": '", textConfig.textColor = COLOR_BLACK);
					Normal_Text(Clay__IntToString(*hash_last), textConfig.textColor = COLOR_BLACK);
					NORMAL_TEXT("'", textConfig.textColor = COLOR_BLACK);
				}

			}
		}
		if (GAME->counter_halfMove >= 100)
		{
			CLAY_BOX(CLAY_STRING("ClaimDraw"),
				(boxConfig.color.fill=clickHandler_claimDraw_clickPhase
					? Color_AsFaded(COLOR_BLACK) : COLOR_BLACK),
				layoutConfig.sizing=SIZING_FIT(),
				layoutConfig.childAlignment=CHILD_ALIGN(CENTER, CENTER),
				Clay_OnHover(handle_claimDraw_clickInteraction, 0)
			) {
				NORMAL_TEXT("Claim Draw");
			}
		}
	}
	{
		listen_cell_click();
		listen_claimDraw_click();
	}
    return Clay_EndLayout();
}

float animationLerpValue = -1.0f;
bool debugModeEnabled = false;

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
