// Font and original renderer (c) Abraham Stolk. Licensed under the GPL.
// Native 24x28 glyphs with the original 32-pixel advance.
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "filmplotter.h"
#include "filmplotterfont.h"

int film_plotter_line_width(const char* text)
{
    int width = 0;
    if (!text) {
        return 0;
    }
    for (const unsigned char* p = (const unsigned char*)text; *p; ++p) {
        if (*p >= 32 && *p <= 126) {
            if (width > INT_MAX - FILM_PLOTTER_ADVANCE) {
                return INT_MAX;
            }
            width += FILM_PLOTTER_ADVANCE;
        }
    }
    return width;
}

int film_plotter_write_line(const char* text, float value, float* buffer,
                            int width, int height, int x, int stride)
{
    if (!text || !buffer || width <= 0 || height <= 0 || stride < width) {
        return 0;
    }
    int total = 0;
    int64_t xpos = x;
    const int rows = height < FILM_PLOTTER_HEIGHT ? height : FILM_PLOTTER_HEIGHT;
    for (const unsigned char* p = (const unsigned char*)text; *p; ++p) {
        if (*p < 32 || *p > 126) {
            continue;
        }
        if (xpos >= width) {
            break;
        }
        if (xpos + FILM_PLOTTER_WIDTH > 0) {
            const int first = xpos < 0 ? (int)-xpos : 0;
            const int last = xpos + FILM_PLOTTER_WIDTH > width
                ? (int)(width - xpos) : FILM_PLOTTER_WIDTH;
            const uint8_t* glyph = film_plotter_font + (*p - 32) * FILM_PLOTTER_HEIGHT * 3;
            for (int row = 0; row < rows; ++row) {
                for (int col = first; col < last; ++col) {
                    if (glyph[row * 3 + col / 8] & (0x80 >> (col % 8))) {
                        buffer[(size_t)row * stride + (int)xpos + col] = value;
                        if (total < INT_MAX) {
                            ++total;
                        }
                    }
                }
            }
        }
        xpos += FILM_PLOTTER_ADVANCE;
    }
    return total;
}
