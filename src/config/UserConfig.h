#pragma once

// TFT Configuration
#define USER_SETUP_INFO "TTGO T-Display"

#define TFT_MISO -1
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS   5  
#define TFT_DC   16
#define TFT_RST  23
#define TFT_BL   4

#define TOUCH_CS -1  // Disable touch by setting to -1
#define LOAD_GLCD   // Font 1
#define LOAD_FONT2  // Font 2
#define LOAD_FONT4  // Font 4
#define LOAD_FONT6  // Font 6
#define LOAD_FONT7  // Font 7
#define LOAD_FONT8  // Font 8
#define LOAD_GFXFF  // FreeFonts

#define SMOOTH_FONT

// Display specific settings
#define TFT_WIDTH  240
#define TFT_HEIGHT 135
