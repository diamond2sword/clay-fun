//#define CLAY_HELPERS_IMPLEMENTATION

#include "web_clay.h.c"
#include "math.h.c"

#ifndef CLAY_HELPERS
#define CLAY_HELPERS

#define RGBA(r, g, b, a) (Clay_Color){r, g, b, a}
#define RGB(r, g, b) RGBA(r, g, b, 255)

#define COLOR_NONE RGBA(0, 0, 0, 0)
#define COLOR_BLACK RGB(0, 0, 0)
#define COLOR_WHITE RGB(255, 255, 255)


Clay_Color Color_AsFaded(Clay_Color color);

//#define DIRECTION(token) (Clay_LayoutDirection)CLAY_##token

#define SIZING_FIXED(width, height) (Clay_Sizing){CLAY_SIZING_FIXED(width), CLAY_SIZING_FIXED(height)}

#define SIZING_PERCENT(width, length) (Clay_Sizing){CLAY_SIZING_PERCENT((float)width), CLAY_SIZING_PERCENT((float)length)}

#define SIZING_GROW(...) (Clay_Sizing){CLAY_SIZING_GROW(__VA_ARGS__), CLAY_SIZING_GROW(__VA_ARGS__)}

#define SIZING_FIT(...) (Clay_Sizing){CLAY_SIZING_FIT(__VA_ARGS__), CLAY_SIZING_FIT(__VA_ARGS__)}

#define CHILD_ALIGN(x, y) (Clay_ChildAlignment){CLAY_ALIGN_X_##x, CLAY_ALIGN_Y_##y}
#define ATTACH_POINTS(element, parent) (Clay_FloatingAttachPoints){CLAY_ATTACH_POINT_##element, CLAY_ATTACH_POINT_##parent}


#define GET_DATA(string) Clay_GetElementData(Clay_GetElementId(CLAY_STRING(string)))





#define CLAY_BOX(elementId, __VA_ARGS__2, ...) \
	CLAY( \
		elementId, \
		boxConfig = (Clay_BoxElementConfig){{COLOR_NONE, COLOR_NONE}, {LINE_WIDTH, LINE_WIDTH}}, \
		__VA_ARGS__2, \
		layoutConfig = (Clay_LayoutConfig){SIZING_GROW(), CLAY_PADDING_ALL(boxConfig.sizing.border)}, \
		rectangleConfig = (Clay_RectangleElementConfig){boxConfig.color.fill, CLAY_CORNER_RADIUS(boxConfig.sizing.corner)}, \
		##__VA_ARGS__, \
		CLAY_BORDER_OUTSIDE_RADIUS(boxConfig.sizing.border, boxConfig.color.border, boxConfig.sizing.corner), \
		CLAY_LAYOUT(layoutConfig), \
		CLAY_RECTANGLE(rectangleConfig) \
	) 

#define CLAY_LABEL(text, ...) \
	rectangleConfig = (Clay_RectangleElementConfig){Color_AsFaded(COLOR_BLACK), CLAY_CORNER_RADIUS(LINE_WIDTH)}; \
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

#define Normal_Text(string, ...) \
	textConfig = (Clay_TextElementConfig){COLOR_WHITE, FONT_ID_TEXT, FONT_SIZE, .disablePointerEvents = true}; \
	("empty", ##__VA_ARGS__); \
	CLAY_TEXT( \
		string, \
		CLAY_TEXT_CONFIG(textConfig) \
	)
	

#define NORMAL_TEXT(text, ...) \
	Normal_Text(CLAY_STRING(text), ##__VA_ARGS__)

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

void Helpers_Update(float width, float height);

#endif // CLAY_HELPERS


#ifdef CLAY_HELPERS_IMPLEMENTATION
#undef CLAY_HELPERS_IMPLEMENTATION
Clay_BoxElementConfig boxConfig;
Clay_LayoutConfig layoutConfig;
Clay_RectangleElementConfig rectangleConfig;
Clay_FloatingElementConfig floatingConfig;
Clay_TextElementConfig textConfig;

float LINE_WIDTH;
float FONT_SIZE;
uint32_t CLAY_LABEL_INDEX = 0;
const uint32_t FONT_ID_TEXT = 0;
double windowWidth = 1024, windowHeight = 768;
double windowSmallSide;
double windowLongSide;
bool isPortrait;
bool isMobileScreen;

void Helpers_Update(float width, float height)
{
	windowWidth = width;
	windowHeight = height;
	windowLongSide = MAX(windowWidth, windowHeight);

	windowSmallSide = MIN(windowWidth, windowHeight);
	isPortrait = windowWidth < windowHeight;
	LINE_WIDTH = 0.025f * windowSmallSide;
	FONT_SIZE = 0.05f * windowSmallSide;
}

Clay_Color Color_AsFaded(Clay_Color color) 
{
	color.a /= 2;
	return color;
}
#endif // CLAY_HELPERS_IMPLEMENTATION
