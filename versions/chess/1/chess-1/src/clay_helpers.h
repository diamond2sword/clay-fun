#ifndef CLAY_HELPERS
#define CLAY_HELPERS

#define COLOR(r, g, b) (Clay_Color){r, g, b, 255}

#define SIZING_FIXED(width, height) (Clay_Sizing){CLAY_SIZING_FIXED(width), CLAY_SIZING_FIXED(height)}

#define SIZING_PERCENT(width, length) (Clay_Sizing){CLAY_SIZING_PERCENT((float)width), CLAY_SIZING_PERCENT((float)length)}

#define GET_DATA(string) Clay_GetElementData(Clay_GetElementId(CLAY_STRING(string)))

#endif // CLAY_HELPERS
