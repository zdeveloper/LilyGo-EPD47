#ifndef CANVAS_HELPER_H
#define CANVAS_HELPER_H

#include <stdint.h>

// Configures canvas size, applies rotation, draws text, and returns raw buffer
uint8_t* get_rotated_text_buffer(const char* text, int text_size, int x, int y, int screen_w, int screen_h, int rotation);

#endif