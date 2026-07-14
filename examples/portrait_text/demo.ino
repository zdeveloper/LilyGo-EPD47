#include <Arduino.h>
#include "epd_driver.h"
#include "canvas_helper.h"

uint8_t *framebuffer;


void setup() {
    Serial.begin(115200);

    epd_init();
    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer) {
        Serial.println("PSRAM Framebuffer allocation failed!");
        return;
    }
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2); // Clear screen to white

    epd_poweron();
    epd_clear();

    const char* text = "\"Brian Woodville!\" I sat up in bed with a start. Pitch darkness prevailed in my apartment. But, staring intently out of the alcove in the direction of the half-shuttered windows, I could just discern a faint difference in the gloom. Beyond was the balcony; beyond that the gardens. I reached up for a hanging switch and lighted my bedside lamp. The room was empty. No sound was audible except a distant rippling from the little stream at the foot of the gardens and the intimate ticking of my wrist watch.";

    // 1. Generate text on a full-screen canvas rotated to Portrait (rotation = 1)
    // Draw at Portrait coordinates X=50, Y=100
    uint8_t* canvas_buffer = get_rotated_text_buffer(text, 1, 25, 30, EPD_WIDTH, EPD_HEIGHT, 3);

    // 2. Straight 1:1 Map into the LilyGo Framebuffer
    // We scan the native landscape screen layout (960 wide, 540 high)
    for (int y = 0; y < EPD_HEIGHT; y++) {
        for (int x = 0; x < EPD_WIDTH; x++) {
            
            // Unpack the bits from the Adafruit canvas
            int byte_index = (x + y * EPD_WIDTH) / 8;
            int bit_index = 7 - ((x + y * EPD_WIDTH) % 8);
            
            bool is_pixel_active = (canvas_buffer[byte_index] >> bit_index) & 0x01;

            if (is_pixel_active) {
                // Direct mapping! No rotation math required here anymore.
                epd_draw_pixel(x, y, 0, framebuffer); 
            }
        }
    }

    // 3. Render framebuffer to screen
    epd_draw_grayscale_image(epd_full_screen(), framebuffer);
    epd_poweroff();

    // Set up a timer to wake up after 3 seconds (time in microseconds)
    esp_sleep_enable_timer_wakeup(3 * 1000000);
    // Ensure all serial communication finishes before sleeping
    Serial.flush(); 
    // Enter deep sleep
    esp_light_sleep_start();
}

void loop() {}