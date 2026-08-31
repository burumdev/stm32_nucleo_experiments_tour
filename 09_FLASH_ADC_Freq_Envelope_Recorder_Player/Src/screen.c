#include "TM1638.h"
#include "app.h"
#include "util.h"
#include "string.h"

void update_progress_leds(TM1638* display, size_t index) {
    size_t led_count = 8 - ((index + 1) / LED_UNIT);
    for (size_t i = 1; i <= 8; i++) {
        bool is_lit = i <= led_count;
        tm1638_set_led(display, i, is_lit);
    }
}

void print_number(TM1638* display, uint16_t number, size_t offset) {
    char numbuf[6];
    uint16_to_str(number, numbuf);

    size_t leftpad = 4 - strlen(numbuf);

    for (size_t i = 0; i < 4; i++) {
        uint8_t ch = leftpad > i ? ' ' : numbuf[i - leftpad];
        tm1638_display_char(display, i + offset, ch, false);
    }
}

