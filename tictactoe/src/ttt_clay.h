#include "web_clay.h"




#ifndef TTT_CLAY_HEADER
#define TTT_CLAY_HEADER

#define CLAY_BOX(elementId, __VA_ARGS__2, ...) \
	CLAY( \
		elementId, \
		boxConfig = (Clay_BoxElementConfig){{COLOR_NONE, COLOR_NONE}, {lineWidth, lineWidth}}, \
		__VA_ARGS__2, \
		layoutConfig = (Clay_LayoutConfig){sizingGrow, CLAY_PADDING_ALL(boxConfig.sizing.border)}, \
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

#define SET_COLOR(name, ...) const Clay_Color COLOR_##name = (Clay_Color) {__VA_ARGS__}
Clay_Color Color_AsFaded(Clay_Color color);

#endif // TTT_CLAY_HEADER




#ifdef TTT_CLAY_IMPLEMENTATION
#undef TTT_CLAY_IMPLEMENTATION

#define CLAY_IMPLEMENTATION
#include "web_clay.h"

Clay_BoxElementConfig boxConfig;
Clay_LayoutConfig layoutConfig;
Clay_RectangleElementConfig rectangleConfig;
Clay_FloatingElementConfig floatingConfig;
Clay_TextElementConfig textConfig;

double windowWidth = 1024, windowHeight = 768;
double windowSmallSide;
double windowLongSide;
bool isPortrait;

float lineWidth;
uint32_t CLAY_LABEL_INDEX = 0;
const uint32_t FONT_ID_TEXT = 0;

Clay_Color Color_AsFaded(Clay_Color color) 
{
	color.a /= 2;
	return color;
}

SET_COLOR(WHITE, 255, 255, 255, 255);
SET_COLOR(BLACK, 0, 0, 0, 255);
SET_COLOR(RED, 255, 0, 0, 255);
SET_COLOR(GREEN, 0, 255, 0, 255);
SET_COLOR(NONE, 0, 0, 0, 0);
SET_COLOR(YELLOW, 255, 255, 0, 255);
SET_COLOR(BLUE, 0, 0, 255, 255);

const Clay_Color COLOR_PLAYER_X = COLOR_BLUE;
const Clay_Color COLOR_PLAYER_O = COLOR_YELLOW;

const Clay_Sizing sizingGrow = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()};
const Clay_ChildAlignment centerXY = {CLAY_ALIGN_X_CENTER, CLAY_ALIGN_Y_CENTER};


#endif // TTT_CLAY_IMPLEMENTATION
