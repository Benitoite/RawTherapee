// (c) Abraham Stolk
// Licensed via GPL.

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "text.h"
#include "fntdat.h"

#if FONTH != ANNOTATION_FONT_HEIGHT
#error "Regenerate the annotation font when changing its height"
#endif

static const AnnotationGlyph* text_glyph(unsigned char c)
{
    if (c < ANNOTATION_FIRST_CHAR || c > ANNOTATION_LAST_CHAR) {
        return NULL;
    }
    return &annotation_glyphs[c - ANNOTATION_FIRST_CHAR];
}

static int text_kerning(unsigned char previous, unsigned char current)
{
    const unsigned int pair = ((unsigned int)previous << 8) | current;
    size_t first = 0;
    size_t last = sizeof(annotation_kerning) / sizeof(annotation_kerning[0]);
    while (first < last) {
        const size_t middle = first + (last - first) / 2;
        if (annotation_kerning[middle].pair < pair) {
            first = middle + 1;
        } else if (annotation_kerning[middle].pair > pair) {
            last = middle;
        } else {
            return annotation_kerning[middle].adjust;
        }
    }
    return 0;
}

int text_line_width(const char* text)
{
    int width = 0;
    unsigned char previous = 0;
    if (!text) {
        return 0;
    }
    for (const unsigned char* p = (const unsigned char*)text; *p; ++p) {
        const AnnotationGlyph* glyph = text_glyph(*p);
        if (glyph) {
            const int advance = glyph->advance + text_kerning(previous, *p);
            if (width > INT_MAX - advance) {
                return INT_MAX;
            }
            width += advance;
            previous = *p;
        }
    }
    return width;
}

static int text_write_glyph(const AnnotationGlyph* glyph, float value, float* buf,
                            int bufw, int bufh, int64_t x, int line_stride)
{
    const int64_t left = x + glyph->left;
    if (left >= bufw || left + glyph->width <= 0 || glyph->top >= bufh) {
        return 0;
    }
    const int first_col = left < 0 ? (int)-left : 0;
    const int last_col = left + glyph->width > bufw ? (int)(bufw - left) : glyph->width;
    int numpixels = 0;

    for (int row = 0; row < glyph->height && row + glyph->top < bufh; ++row) {
        float* writer = buf + (size_t)(row + glyph->top) * line_stride;
        const uint8_t* coverage = fntdat + glyph->offset + row * glyph->width;
        for (int col = first_col; col < last_col; ++col) {
            if (coverage[col]) {
                float* pixel = writer + (int)(left + col);
                if (coverage[col] == 255) {
                    *pixel = value;
                } else {
                    const float alpha = coverage[col] / 255.0f;
                    *pixel += alpha * (value - *pixel);
                }
                ++numpixels;
            }
        }
    }
    return numpixels;
}

int text_write_char(char c, float value, float* buf, int bufw, int bufh, int x, int line_stride)
{
    const AnnotationGlyph* glyph = text_glyph((unsigned char)c);
    if (!glyph || !buf || bufw <= 0 || bufh <= 0 || line_stride < bufw) {
        return 0;
    }
    return text_write_glyph(glyph, value, buf, bufw, bufh, x, line_stride);
}

int text_write_line(const char* text, float value, float* buf, int bufw, int bufh,
                    int x, int line_stride)
{
    if (!text || !buf || bufw <= 0 || bufh <= 0 || line_stride < bufw) {
        return 0;
    }

    int totalpixels = 0;
    int64_t xpos = x;
    unsigned char previous = 0;
    for (const unsigned char* p = (const unsigned char*)text; *p; ++p) {
        const AnnotationGlyph* glyph = text_glyph(*p);
        if (glyph) {
            xpos += text_kerning(previous, *p);
            // Leave room for negative bearings before clipping the rest of the line.
            if (xpos + INT8_MIN >= bufw) {
                break;
            }
            const int count = text_write_glyph(glyph, value, buf, bufw, bufh, xpos, line_stride);
            totalpixels = totalpixels > INT_MAX - count ? INT_MAX : totalpixels + count;
            xpos += glyph->advance;
            previous = *p;
        }
    }
    return totalpixels;
}
