#include "ssd1306.h"

void ssd1306_command(ssd1306_t* screen, uint8_t command) {
    HAL_I2C_Mem_Write(
            screen->i2c_handle,
            SSD1306_I2C_ADDR,
            0x00,
            1,
            &command,
            1,
            SSD1306_TIMEOUT
    );
}

void ssd1306_init_commands(ssd1306_t* screen) {

    ssd1306_command(screen, SSD1306_OP_TURN_OFF);

    ssd1306_command(screen, SSD1306_OP_SET_MULTIPLEX);
    ssd1306_command(screen, SSD1306_HEIGHT - 1); // set multiplex ratio to max screen height

    ssd1306_command(screen, SSD1306_OP_LOW_COLUMN_START_BASE | 0x0);
    ssd1306_command(screen, SSD1306_OP_HIGH_COLUMN_START_BASE | 0x0);
    ssd1306_set_line_offset(screen, 0x00);

    ssd1306_command(screen, SSD1306_OP_SET_MEM_MODE);
    ssd1306_command(screen, SSD1306_MEM_MODE_PAGE);

    ssd1306_command(screen, SSD1306_OP_PAGE_START_BASE | 0x0);

    ssd1306_set_vertical_mirror(screen, false);
    ssd1306_set_horizontal_mirror(screen, false);

    ssd1306_set_contrast(screen, 0xFF);

    ssd1306_command(screen, SSD1306_OP_DISPLAY_NORMAL_MODE);

    ssd1306_command(screen, SSD1306_OP_DISPLAY_RAM_MODE);

    ssd1306_command(screen, SSD1306_OP_SET_CLOCK_AND_OSCILLATOR);
    ssd1306_command(screen, (0xF) << 4 | (0x0)); // set freq to 0xF (reset) and divide ratio to 0x0 (none)

    ssd1306_command(screen, SSD1306_OP_SET_PRE_CHARGE_PERIOD);
    ssd1306_command(screen, (0x2) << 4 | (0x2)); // set phase 1 and 2 to 2

    ssd1306_command(screen, SSD1306_OP_SET_VCOMH);
    ssd1306_command(screen, SSD1306_VCOMH_0_77_VCC);

    // what does this do ??
    ssd1306_command(screen, SSD1306_OP_SET_CHARGE_PUMP);
    ssd1306_command(screen, SSD1306_CHARGE_PUMP_ON);

    ssd1306_command(screen, SSD1306_OP_TURN_ON);
}

bool ssd1306_init(ssd1306_t* screen, I2C_HandleTypeDef* handle) {
    if (screen == NULL || handle == NULL)
        return false;

    *screen = (ssd1306_t) {
            .i2c_handle = handle,
            .buffer = calloc(1, SSD1306_BUFFER_SIZE),
    };

    if (HAL_I2C_IsDeviceReady(handle, SSD1306_I2C_ADDR, 1, SSD1306_TIMEOUT) != HAL_OK)
        return false;

    ssd1306_init_commands(screen);

    ssd1306_fill(screen, BLACK);

    ssd1306_update(screen);

    return true;
}

void ssd1306_set_contrast(ssd1306_t* screen, uint8_t contrast) {
    ssd1306_command(screen, SSD1306_OP_SET_CONTRAST);
    ssd1306_command(screen, contrast);
}

void ssd1306_update(ssd1306_t* screen) {
    ssd1306_command(screen, SSD1306_OP_SET_COLUMN);
    ssd1306_command(screen, 0); // start column
    ssd1306_command(screen, 127); // end column

    ssd1306_command(screen, SSD1306_OP_SET_PAGE);
    ssd1306_command(screen, 0); // start page
    ssd1306_command(screen, 7); // end page

    HAL_I2C_Mem_Write(
            screen->i2c_handle,
            SSD1306_I2C_ADDR,
            0x40,
            1,
            screen->buffer,
            SSD1306_BUFFER_SIZE,
            SSD1306_TIMEOUT
    );
}

// draw functions
void ssd1306_fill(ssd1306_t* screen, ssd1306_color_t color) {
    memset(screen->buffer, color == WHITE ? 0xFF : 0x00, SSD1306_BUFFER_SIZE);
}

void ssd1306_fill_rect(ssd1306_t* screen, uint8_t x, uint8_t y, uint8_t width, uint8_t height, ssd1306_color_t color) {
    for (uint8_t row = 0; row < height; row++) {
        for (uint8_t col = 0; col < width; col++) {
            ssd1306_set_pixel(screen, x + col, y + row, color);
        }
    }
}

void ssd1306_set_pixel(ssd1306_t* screen, uint8_t x, uint8_t y, ssd1306_color_t color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
        return;

    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    uint16_t byte_index = x + (page * SSD1306_WIDTH);

    if (color == WHITE) {
        screen->buffer[byte_index] |= (1 << bit);
    } else {
        screen->buffer[byte_index] &= ~(1 << bit);
    }
}

void ssd1306_draw_bitmap(ssd1306_t* screen, const uint8_t* bitmap, uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
    const uint8_t pages = (height + 7) / 8;
    for (uint8_t page = 0; page < pages; page++) {
        for (uint8_t col = 0; col < width; col++) {
            const uint8_t byte = bitmap[page * width + col];

            for (uint8_t bit = 0; bit < 8; bit++) {
                const uint8_t pixel_y = y + page * 8 + bit;
                if (pixel_y >= y + height) break;

                const uint8_t pixel = (byte >> bit) & 0x01;
                ssd1306_set_pixel(screen, x + col, pixel_y, pixel ? WHITE : BLACK);
            }
        }
    }
}

// text rendering functions
const uint8_t* ssd1306_font_get_glyph(const ssd1306_font_t* font, char character) {
    // Convert lowercase to uppercase
    if (character >= 'a' && character <= 'z') {
        character = character - 'a' + 'A';
    }

    // Search for the glyph
    for (int i = 0; i < font->glyph_count; i++) {
        if (font->glyphs[i].character == character) {
            return font->glyphs[i].bitmap;
        }
    }

    // Return the first glyph (space) as default
    return font->glyphs[0].bitmap;
}

void ssd1306_draw_text_char(ssd1306_t* screen, const ssd1306_font_t* font, char character) {
    if (character == '\n') {
        screen->cursor_x = 0;
        screen->cursor_y += (uint8_t) (font->char_height * 1.5);
        return;
    }

    if (screen->cursor_x + font->char_width > SSD1306_WIDTH) {
        screen->cursor_x = 0;
        screen->cursor_y += (uint8_t) (font->char_height * 1.5);
    }

    if (screen->cursor_y + font->char_height > SSD1306_HEIGHT) {
        return;
    }

    ssd1306_draw_char(screen, font, character, screen->cursor_x, screen->cursor_y);
    screen->cursor_x += font->char_width;
}

uint8_t ssd1306_measure_text(const ssd1306_font_t* font, const char* text) {
    uint8_t width = 0;
    while (*text) {
        if (*text == '\n') {
            break; // Stop at newline
        }
        width += font->char_width;
        text++;
    }
    return width;
}

void ssd1306_draw_char(ssd1306_t* screen, const ssd1306_font_t* font, const char character, const uint8_t x, const uint8_t y) {
    const uint8_t* bitmap = ssd1306_font_get_glyph(font, character);
    ssd1306_draw_bitmap(screen, bitmap, x, y, font->char_width, font->char_height);
}

void ssd1306_draw_text_cursor(ssd1306_t* screen, const ssd1306_font_t* font, const char* text) {
    while (*text) {
        ssd1306_draw_text_char(screen, font, *text);
        text++;
    }
}

void ssd1306_set_text_cursor(ssd1306_t* screen, const ssd1306_font_t* font, const uint8_t x, const uint8_t y) {
    screen->cursor_x = x;
    screen->cursor_y = y;
}

void ssd1306_draw_text(ssd1306_t* screen, const ssd1306_font_t* font, const char* text, uint8_t x, uint8_t y) {
    const char* ptr = text;
    while (*ptr && *ptr != '\n') {
        if (x + font->char_width > SSD1306_WIDTH)
            break;

        ssd1306_draw_char(screen, font, *ptr, x, y);
        x += font->char_width;
        ptr++;
    }
}

void ssd1306_draw_text_aligned(ssd1306_t* screen, const ssd1306_font_t* font, const char* text, uint8_t y, ssd1306_text_align_t align) {
    const uint8_t text_width = ssd1306_measure_text(font, text);

    uint8_t x = 0;
    switch (align) {
        case ALIGN_LEFT:
            x = 0;
            break;
        case ALIGN_CENTER:
            x = (SSD1306_WIDTH - text_width) / 2;
            break;
        case ALIGN_RIGHT:
            x = SSD1306_WIDTH - text_width;
            break;
    }

    const char* ptr = text;
    while (*ptr && *ptr != '\n') {
        if (x + font->char_width > SSD1306_WIDTH)
            break;

        ssd1306_draw_char(screen, font, *ptr, x, y);
        x += font->char_width;
        ptr++;
    }
}


void ssd1306_set_line_offset(ssd1306_t* screen, uint8_t offset)
{
    if (offset > 0x3F)
        return;

    ssd1306_command(screen, SSD1306_OP_START_LINE_ADDR_BASE + offset);
}

void ssd1306_set_horizontal_mirror(ssd1306_t* screen, bool mirror)
{
    ssd1306_command(screen, mirror ? SSD1306_OP_SEGMENT_NORMAL_MODE : SSD1306_OP_SEGMENT_REMAP_MODE);
}

void ssd1306_set_vertical_mirror(ssd1306_t* screen, bool mirror)
{
    ssd1306_command(screen, mirror ? SSD1306_OP_COM_NORMAL_MODE : SSD1306_OP_COM_REMAP_MODE);
}

