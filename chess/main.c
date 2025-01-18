#define CLAY_EXTEND_CONFIG_RECTANGLE Clay_String link; bool cursorPointer;
#define CLAY_EXTEND_CONFIG_IMAGE Clay_String sourceURL;
#define CLAY_EXTEND_CONFIG_TEXT bool disablePointerEvents;
#define CLAY_IMPLEMENTATION
#include "clay.h"

double windowWidth, windowHeight;
bool isMobileScreen;
float animationLerpValue;
bool debugModeEnabled;
const uint32_t FONT_ID_TEXT;

Clay_RenderCommandArray CreateLayout() {
    Clay_BeginLayout();
	CLAY_TEXT(CLAY_STRING("Hello World!"), CLAY_TEXT_CONFIG({{0, 0, 0, 255}, FONT_ID_TEXT, 50}));
    return Clay_EndLayout();
}

double windowWidth = 1024, windowHeight = 768;
bool isMobileScreen = true;
float animationLerpValue = -1.0f;
bool debugModeEnabled = false;
const uint32_t FONT_ID_TEXT = 0;

CLAY_WASM_EXPORT("UpdateDrawFrame") Clay_RenderCommandArray UpdateDrawFrame(float width, float height, float mouseWheelX, float mouseWheelY, float mousePositionX, float mousePositionY, bool isTouchDown, bool isMouseDown, bool arrowKeyDownPressedThisFrame, bool arrowKeyUpPressedThisFrame, bool dKeyPressedThisFrame, float deltaTime) {
    windowWidth = width;
    windowHeight = height;
    Clay_SetLayoutDimensions((Clay_Dimensions) { width, height });
    // NaN propagation can cause pain here
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
