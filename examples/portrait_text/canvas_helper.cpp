#include "canvas_helper.h"
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold12pt7b.h>

// Allocate a full-screen landscape canvas matching the LilyGo EPD dimensions
static GFXcanvas1 canvas(960, 540); 

uint8_t* get_rotated_text_buffer(const char* text, int text_size, int x, int y, int screen_w, int screen_h, int rotation) {
    canvas.setTextWrap(true);
    canvas.fillScreen(0); // Clear canvas (White background)
    
    // Set rotation: 0 = 0°, 1 = 90° (Portrait), 2 = 180°, 3 = 270°
    canvas.setRotation(rotation); 
    
    // Because rotation is set to 1, the cursor (x, y) is now relative to Portrait bounds
    canvas.setCursor(x, y);
    canvas.setTextColor(1); // Black text
    canvas.setTextSize(2);
    canvas.setFont(&FreeMonoBold12pt7b);
    canvas.print(text);
    

    return canvas.getBuffer();
}


