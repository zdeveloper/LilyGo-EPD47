#include "canvas_helper.h"
#include <Adafruit_GFX.h>
#include <Fonts/FreeMonoBold18pt7b.h>

// Allocate a full-screen landscape canvas matching the LilyGo EPD dimensions
static GFXcanvas1 canvas(960, 540); 

uint8_t* get_rotated_text_buffer(const char* text, int text_size, int x, int y, int screen_w, int screen_h, int rotation) {
    // Disable character wrapping since we are manually doing word wrapping
    canvas.setTextWrap(false); 
    canvas.fillScreen(0); // Clear canvas (White background)
    
    // Set rotation: 0 = 0°, 1 = 90° (Portrait), 2 = 180°, 3 = 270°
    canvas.setRotation(rotation); 
    
    canvas.setTextColor(1); // Black text
    canvas.setTextSize(text_size); // Respecting the text_size parameter passed in
    canvas.setFont(&FreeMonoBold18pt7b);

    // Dynamic Tracking Variables
    int16_t x_start = x;
    int16_t current_x = x;
    int16_t current_y = y;

    // Determine the max boundary dynamically based on the current canvas orientation width
    int16_t max_width = canvas.width() - x_start; 

    // Create a modifiable copy of the text for string tokenization
    char buf[strlen(text) + 1];
    strcpy(buf, text);
    
    canvas.setCursor(current_x, current_y);
    
    // Split the text into individual words using spaces
    char* word = strtok(buf, " ");
    while (word != NULL) {
        int16_t x1, y1;
        uint16_t word_w, word_h;
        
        // Measure the pixel dimensions of the current word
        canvas.getTextBounds(word, current_x, current_y, &x1, &y1, &word_w, &word_h);
        
        // Check if the word pushes past our horizontal boundary constraint
        if (current_x + word_w > x_start + max_width) {
            current_x = x_start;
            // Move down by the font height plus line spacing padding (e.g., 8 pixels)
            current_y += 32; 
            canvas.setCursor(current_x, current_y);
        }
        
        // Print the word and a trailing space
        canvas.print(word);
        canvas.print(" ");
        
        // Update our coordinate trackers using the canvas's new cursor placements
        current_x = canvas.getCursorX();
        current_y = canvas.getCursorY();
        
        // Advance to the next word
        word = strtok(NULL, " ");
    }

    return canvas.getBuffer();
}