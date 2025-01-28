
#include <stdarg.h>


#define CLAY_HELPERS_IMPLEMENTATION
#include "clay_helpers.h.c"

#define CLAY_IMPLEMENTATION
#include "web_clay.h.c"

#define CLAY_CLICK_HANDLER_IMPLEMENTATION
#include "clay_click_handler.h.c"

#define CHESS_IMPLEMENTATION
#include "chess.h.c"

#define MY_MATH_IMPLEMENTATION
#include "math.h.c"


typedef enum PiecePromotionIndex : uint8_t {
	PIECE_PROMOTION_INDEX_QUEEN,
	PIECE_PROMOTION_INDEX_ROOK,
	PIECE_PROMOTION_INDEX_BISHOP,
	PIECE_PROMOTION_INDEX_KNIGHT,
	PIECE_PROMOTION_INDEX_COUNT,
	PIECE_PROMOTION_INDEX_NONE,
} PiecePromotionIndex;


CLAY_WASM_EXPORT("Init") void Init();

uint64_t Mask_PromotionOptions(Bitboards_All bitboardSet, uint64_t index, Move move);

void BoardClick_Phase_Reset(bool hasMadeMove);
void BoardClick_Phase_SelectSource();
void BoardClick_Phase_Reset_AutoSelect();
void BoardClick();

void listen_cell_click();
void handle_cell_clickInteraction(ON_HOVER_PARAMS);

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


uint64_t * SELECTABLES_BITBOARD;
uint64_t MOVABLES_BITBOARD;
uint64_t ATTACKS_BITBOARD;
uint64_t MOVED_BITBOARD;
int8_t INDEX_INPUT;
int8_t INDEX_SRC;
int8_t INDEX_DST;
int8_t INDEX_PROMOTION_OPTION;
Move MOVE_TEMP;
Move MOVE_RECENT;
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
	// precomputes
	Precompute_SlidingPiece_RayIndexes();

	// init game
	ChessInit_Default();
	StringIndex fen = STRING("rnbqkbn1/pppppppP/4p3/3pP3/pPp5/7p/PPPPPPPP/RNBQKBNR w KQkq d6 0 1");
	fen = STRING("rnbqkbn1/ppPppppP/4p3/4P3/8/2p3PN/PpPPpP1P/RNBQKB1R b KQq - 0 1");

	ChessInit_FromString(fen, CHESS_INIT_DEFAULT_PARAMS);
	ChessInit_Default();

	BoardClick_Phase_Reset(true);
};

uint64_t Mask_PromotionOptions(Bitboards_All bitboardSet, uint64_t index, Move move)
{
	uint64_t mask_options = Direct_Rows(move.src.side, Mask_Rows(4, 0, 1, 2, 3) & MASK_COL(Index_AsCol(move.dst.index)));
	return mask_options;
}

void BoardClick_Phase_SelectSource()
{
	if (MASK_INDEX_SIGNED(INDEX_INPUT) & MOVABLES_BITBOARD)
	{
		INDEX_SRC = INDEX_INPUT;
		ATTACKS_BITBOARD = Mask_Attacks_KingIsSafeAfter(BITBOARD_SET, Piece_New(BITBOARD_SET, INDEX_SRC), &EN_PASSANT_TARGET_INDEX, CASTLING_RIGHTS);
		SELECTABLES_BITBOARD = &ATTACKS_BITBOARD;
	}
}
void BoardClick_Phase_Reset(bool hasMadeMove)
{
	// reset click phase
	INDEX_SRC = -1;
	INDEX_DST = -1;
	INDEX_PROMOTION_OPTION = -1;
	PIECE_TYPE_PROMOTION = PIECE_TYPE_INDEX_NONE;
	MOVE_TEMP = (Move){};
	//IsCellSelectableFunc = Piece_IsMovable;
	SELECTABLES_BITBOARD = &MOVABLES_BITBOARD;
	if (hasMadeMove)
	{
		INDEX_INPUT = -1;
		MOVABLES_BITBOARD = Mask_Movables(BITBOARD_SET, ACTIVE_SIDE, &EN_PASSANT_TARGET_INDEX, CASTLING_RIGHTS, Mask_Attacks_KingIsSafeAfter);
	}
}

void BoardClick_Phase_Reset_AutoSelect()
{
	const bool selected_isOtherAlly = INDEX_INPUT != INDEX_SRC;
	// reset click phase
	BoardClick_Phase_Reset(false);
	// skip src phase if selected is movable
	if (selected_isOtherAlly)
	{
		BoardClick_Phase_SelectSource();
	}
}

void BoardClick()
{
	static bool enable_makeMove = false;
	if (INDEX_SRC == -1)
	{
		BoardClick_Phase_SelectSource();
	}
	else if (INDEX_DST == -1)
	{
		if (MASK_INDEX_SIGNED(INDEX_INPUT) & ATTACKS_BITBOARD)
		{
			enable_makeMove = true;
			// set move if valid
			INDEX_DST = INDEX_INPUT;
			MOVE_TEMP = Move_New(BITBOARD_SET, INDEX_SRC, INDEX_DST);
			{
				// to trigger pawn promotion option selection
				if (MOVE_TEMP.src.type == PIECE_TYPE_INDEX_PAWN)
				{
					if (MASK_INDEX(MOVE_TEMP.dst.index) & Direct_Rows(MOVE_TEMP.src.side, MASK_ROW_0))
					{
						enable_makeMove = false;
						INDEX_PROMOTION_OPTION = MOVE_TEMP.dst.index;
						PROMOTION_OPTS_BITBOARD = Mask_PromotionOptions(BITBOARD_SET, INDEX_PROMOTION_OPTION, MOVE_TEMP);
						//IsCellSelectableFunc = Piece_IsPromotionOpt;
						SELECTABLES_BITBOARD = &PROMOTION_OPTS_BITBOARD;
					}
				}
			}
		}
		else
		{
			BoardClick_Phase_Reset_AutoSelect();
		}
	}
	else if (INDEX_PROMOTION_OPTION != -1)
	{
		INDEX_PROMOTION_OPTION = -1;

		if (MASK_INDEX_SIGNED(INDEX_INPUT) & PROMOTION_OPTS_BITBOARD)
		{
			enable_makeMove = true;
			int row = Index_AsRow(INDEX_INPUT);
			PIECE_TYPE_PROMOTION = IF_SIDE(MOVE_TEMP.src.side,
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
		Game_MakeMove(BITBOARD_SET, MOVE_TEMP, &ACTIVE_SIDE, &EN_PASSANT_TARGET_INDEX, &PIECE_TYPE_PROMOTION, CASTLING_RIGHTS, &FULL_MOVE_COUNTER, &HALF_MOVE_COUNTER);
		MOVE_RECENT = MOVE_TEMP;
		// reset click phase
		BoardClick_Phase_Reset(true);
	}
	{
		INDEX_INPUT = -1;
	}
}

CLAY_CLICK_HANDLER(cell,
	{BoardClick();},
	{INDEX_INPUT = userData;},
	{BoardClick();},
	{INDEX_INPUT = -1;},
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
	if (INDEX_PROMOTION_OPTION == -1)
	{
		return;
	}
	for (uint8_t optIndex = 0; optIndex < 4; optIndex++)
	{
		const uint8_t index = INDEX_PROMOTION_OPTION + OFFSET_DOWN * optIndex * DIRECT_OFFSET_SIGN(MOVE_TEMP.src.side);
		CLAY(CLAY_LAYOUT(SIZING_FIXED(BOARD_CELL_WIDTH, BOARD_CELL_WIDTH)),
			CLAY_RECTANGLE((MASK_INDEX(index) & MASK_CHECKER) ? colorBlack : colorWhite),
			CLAY_FLOATING({.offset={Index_AsCol(index) * BOARD_CELL_WIDTH, Index_AsRow(index) * BOARD_CELL_WIDTH}})
		)
		BoardPieceSprite(MOVE_TEMP.src.side, MAP_PROMOTION_OPT_TO_TYPE[optIndex]);
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
		Clay_Color color = MASK_INDEX_SIGNED(index) & *SELECTABLES_BITBOARD
			? (index == INDEX_INPUT ? RGBA(0, 255, 0, 128) : RGBA(0, 0, 0, 0))
			: (index == INDEX_INPUT ? RGBA(255, 0, 0, 128) : RGBA(0, 0, 0, 128));
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
		CLAY(0)
		{
			NORMAL_TEXT("Half Moves: ", textConfig.textColor = COLOR_BLACK);
			Normal_Text(Clay__IntToString(HALF_MOVE_COUNTER), textConfig.textColor = COLOR_BLACK);
			NORMAL_TEXT("Full Moves: ", textConfig.textColor = COLOR_BLACK);
			Normal_Text(Clay__IntToString(FULL_MOVE_COUNTER), textConfig.textColor = COLOR_BLACK);
		}
	}
	{
		listen_cell_click();
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
