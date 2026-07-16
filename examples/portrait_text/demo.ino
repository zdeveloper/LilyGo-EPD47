#include <Arduino.h>
#include "epd_driver.h"
#include "canvas_helper.h"
#include <TouchDrvGT911.hpp>
// #include <Ticker.h>

bool screenBusy = false;

uint8_t currentPage = 0;
uint8_t requestedPage = 0;

uint8_t *framebuffer;

// Ticker ticker;

// touch
TouchDrvGT911 touch;
uint32_t touch_loop_interval = 0;
bool touchOnline = false;

const char *pages[] = {
    "\"Brian Woodville!\" I sat up in bed with a start. Pitch darkness prevailed in my apartment. But, staring intently out of the alcove in the direction of the half-shuttered windows, I could just discern a faint difference in the gloom. Beyond was the balcony; beyond that the gardens. I reached up for a hanging switch and lighted my bedside lamp. The room was empty. No sound was audible except a distant rippling from the little stream at the foot of the gardens and the intimate ticking of my wrist watch.",
    "Page 1",
    "Page 2",
    "Page 3",
    "Page 4",
    "Page 5",
    "Page 6",
};

const int pagesCount = sizeof(pages) / sizeof(pages[0]);

void renderText(uint8_t page)
{
    screenBusy = true;

    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2); // Clear screen to white

    epd_poweron();
    // epd_clear();
    epd_clear_area_cycles(epd_full_screen(), 1, 5);

    // 1. Generate text on a full-screen canvas rotated to Portrait (rotation = 1)
    // Draw at Portrait coordinates X=50, Y=100
    uint8_t *canvas_buffer = get_rotated_text_buffer(pages[page], 1, 25, 30, EPD_WIDTH, EPD_HEIGHT, 3);

    // 2. Straight 1:1 Map into the LilyGo Framebuffer
    // We scan the native landscape screen layout (960 wide, 540 high)
    for (int y = 0; y < EPD_HEIGHT; y++)
    {
        for (int x = 0; x < EPD_WIDTH; x++)
        {
            // Unpack the bits from the Adafruit canvas
            int byte_index = (x + y * EPD_WIDTH) / 8;
            int bit_index = 7 - ((x + y * EPD_WIDTH) % 8);

            bool is_pixel_active = (canvas_buffer[byte_index] >> bit_index) & 0x01;

            if (is_pixel_active)
            {
                // Direct mapping! No rotation math required here anymore.
                epd_draw_pixel(x, y, 0, framebuffer);
            }
        }
    }

    // 3. Render framebuffer to screen
    epd_draw_grayscale_image(epd_full_screen(), framebuffer);
    epd_poweroff();
    screenBusy = false;
    // delay(100);
    // delay(100);
}

void handleTouch()
{
    if (touchOnline && !screenBusy)
    {
        // Limit the touch detection interval and detect the touch status every 300ms
        // https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/issues/143
        if (millis() < touch_loop_interval)
        {
            return;
        }

        int16_t x, y;

        if (!digitalRead(TOUCH_INT))
        {
            return;
        }

        uint8_t touched = touch.getPoint(&x, &y, 1);
        if (touched)
        {
            Serial.println("touched!");
            Serial.print(x);
            Serial.print(" - ");
            Serial.println(y);
            if (x > EPD_HEIGHT / 2)
            {
                Serial.println("Next page");
                if (requestedPage < pagesCount - 1)
                {
                    requestedPage++;
                }
            }
            else
            {
                Serial.println("Previous page");
                if (requestedPage > 0)
                {
                    requestedPage--;
                }
            }
        }

        if (requestedPage != currentPage && !screenBusy)
        {
            currentPage = requestedPage;
            renderText(requestedPage);
        }

        // // Set the initial touch interval value
        touch_loop_interval = millis() + 300;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Connected!");

    framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
    if (!framebuffer)
    {
        Serial.println("PSRAM Framebuffer allocation failed!");
        return;
    }

    // touch
    //  Assuming that the previous touch was in sleep state, wake it up
    pinMode(TOUCH_INT, OUTPUT);
    digitalWrite(TOUCH_INT, HIGH);
    /*
     * The touch reset pin uses hardware pull-up,
     * and the function of setting the I2C device address cannot be used.
     * Use scanning to obtain the touch device address.*/
    uint8_t touchAddress = 0x14;

    Wire.beginTransmission(0x14);
    if (Wire.endTransmission() == 0)
    {
        touchAddress = 0x14;
    }
    Wire.beginTransmission(0x5D);
    if (Wire.endTransmission() == 0)
    {
        touchAddress = 0x5D;
    }
    touch.setPins(-1, TOUCH_INT);
    if (touch.begin(Wire, touchAddress, BOARD_SDA, BOARD_SCL))
    {
        touch.setMaxCoordinates(EPD_HEIGHT, EPD_WIDTH);
        touch.setSwapXY(false);
        touch.setMirrorXY(false, false);
        touchOnline = true;
        Serial.println("Touch is online!");
    }
    else
    {
        Serial.println("Touch is offline :(");
    }

    epd_init();
    epd_poweron();
    epd_clear();
    renderText(currentPage);
    epd_poweroff();
    delay(3000);

    // // Set up a timer to wake up after 3 seconds (time in microseconds)
    // esp_sleep_enable_timer_wakeup(3 * 1000000);
    // // Ensure all serial communication finishes before sleeping
    // Serial.flush();
    // // Enter deep sleep
    // esp_light_sleep_start();

    // ticker.attach_ms(30, handleTouch);
}

void loop()
{
    handleTouch();
    // vTaskDelay(portMAX_DELAY);
}