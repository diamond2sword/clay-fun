//okay, so this is probably my ignorance with clay, having just stumbled across it the other week- but I'm trying to render a membership card, and I've implemented a small key/value component to render key/value in the style of the membership card- however it renders the value offset from the key, and I can't figure out how to get this behavior to stop-


void pdfHeadingWithText(Clay_String key, Clay_String value) {
    CLAY(CLAY_LAYOUT({ .padding = { 0, 0 }, .childGap = 0, .sizing = { CLAY_SIZING_FIT(), CLAY_SIZING_GROW() } })) {
    CLAY_TEXT(key,
        CLAY_TEXT_CONFIG({ .textColor = BLACK, .fontSize = 10, .fontFamily = CLAY_STRING("Source Sans 3") }));
    CLAY_TEXT(CLAY_STRING(":"),
        CLAY_TEXT_CONFIG({ .textColor = BLACK, .fontSize = 10, .fontFamily = CLAY_STRING("Source Sans 3") }));
    CLAY_TEXT(CLAY_STRING(" "),
        CLAY_TEXT_CONFIG({ .textColor = BLACK, .fontSize = 10, .fontFamily = CLAY_STRING("Source Sans 3") }));
    CLAY_TEXT(value,
        CLAY_TEXT_CONFIG({ .textColor = BLACK, .fontSize = 10, .fontFamily = CLAY_STRING("Source Sans 3") }));
    }
}

// and it renders the PDF with my component like this-
