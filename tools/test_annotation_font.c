// Standalone regression checks; see devnotes/annotation-font.md.
// Licensed under the GNU General Public License, version 3 or later.

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "text.h"

enum { WIDTH = 43, STRIDE = 57, GUARD = 31, REF_WIDTH = 2048, REF_ORIGIN = 512 };

static void check_metrics(void)
{
    assert(text_line_width(NULL) == 0);
    assert(text_line_width("") == 0);
    assert(text_line_width("III") < text_line_width("WWW"));
    assert(text_line_width("AV") < text_line_width("A") + text_line_width("V"));
    assert(text_line_width("To") < text_line_width("T") + text_line_width("o"));
    assert(text_line_width("  ") == 2 * text_line_width(" "));
    assert(text_line_width("I I") == 2 * text_line_width("I") + text_line_width(" "));
    assert(text_line_width("A\n\t\xc3\xa9V") == text_line_width("AV"));

    float pixels[FONTH * 64] = {0};
    for (unsigned int c = 1; c <= 255; ++c) {
        const char text[] = {(char)c, '\0'};
        const int count = text_write_char((char)c, 1.0f, pixels, 64, FONTH, 8, 64);
        if (c >= 33 && c <= 126) {
            assert(count > 0);
            assert(text_line_width(text) > 0);
        } else if (c == ' ') {
            assert(count == 0);
            assert(text_line_width(text) > 0);
        } else {
            assert(count == 0);
            assert(text_line_width(text) == 0);
        }
    }
}

static void check_antialiasing(void)
{
    float light[64 * FONTH], dark[64 * FONTH];
    for (int i = 0; i < 64 * FONTH; ++i) {
        light[i] = 1.0f;
        dark[i] = 0.0f;
    }
    assert(text_write_line("AV", 0.0f, light, 64, FONTH, 0, 64) > 0);
    assert(text_write_line("AV", 1.0f, dark, 64, FONTH, 0, 64) > 0);
    int opaque = 0, partial = 0, untouched = 0;
    for (int i = 0; i < 64 * FONTH; ++i) {
        assert(light[i] >= 0.0f && light[i] <= 1.0f);
        assert(dark[i] >= 0.0f && dark[i] <= 1.0f);
        assert(fabsf(light[i] + dark[i] - 1.0f) < 1e-6f);
        opaque += dark[i] == 1.0f;
        partial += dark[i] > 0.0f && dark[i] < 1.0f;
        untouched += dark[i] == 0.0f;
    }
    assert(opaque > 0 && partial > 0 && untouched > 0);
}

static void check_clipping(void)
{
    const char* text = "j/AV To, WWW III 2026!?";
    const int origins[] = {-120, -20, -1, 0, 22, 42, 43, 60, INT_MIN, INT_MAX};
    const int heights[] = {1, 5, 25, FONTH};
    const float sentinel = -12345.0f;
    float reference[REF_WIDTH * FONTH] = {0};
    assert(text_write_line(text, 1.0f, reference, REF_WIDTH, FONTH,
                           REF_ORIGIN, REF_WIDTH) > 0);

    for (size_t h = 0; h < sizeof(heights) / sizeof(heights[0]); ++h) {
        for (size_t o = 0; o < sizeof(origins) / sizeof(origins[0]); ++o) {
            float storage[GUARD + FONTH * STRIDE + GUARD];
            for (size_t i = 0; i < sizeof(storage) / sizeof(storage[0]); ++i) {
                storage[i] = sentinel;
            }
            float* pixels = storage + GUARD;
            for (int y = 0; y < heights[h]; ++y) {
                for (int x = 0; x < WIDTH; ++x) {
                    pixels[y * STRIDE + x] = 0.0f;
                }
            }
            text_write_line(text, 1.0f, pixels, WIDTH, heights[h], origins[o], STRIDE);
            for (int y = 0; y < FONTH; ++y) {
                for (int x = 0; x < STRIDE; ++x) {
                    float expected = sentinel;
                    if (x < WIDTH && y < heights[h]) {
                        const long long ref_x = (long long)REF_ORIGIN + x - origins[o];
                        expected = ref_x >= 0 && ref_x < REF_WIDTH ?
                            reference[y * REF_WIDTH + (int)ref_x] : 0.0f;
                    }
                    assert(pixels[y * STRIDE + x] == expected);
                }
            }
            for (int i = 0; i < GUARD; ++i) {
                assert(storage[i] == sentinel);
                assert(storage[GUARD + FONTH * STRIDE + i] == sentinel);
            }
        }
    }
}

static void check_invalid_buffers(void)
{
    float pixel = 123.0f;
    assert(text_write_line(NULL, 0.0f, &pixel, 1, 1, 0, 1) == 0);
    assert(text_write_line("A", 0.0f, NULL, 1, 1, 0, 1) == 0);
    assert(text_write_line("A", 0.0f, &pixel, 0, 1, 0, 1) == 0);
    assert(text_write_line("A", 0.0f, &pixel, 1, 0, 0, 1) == 0);
    assert(text_write_line("A", 0.0f, &pixel, 2, 1, 0, 1) == 0);
    assert(text_write_char('A', 0.0f, &pixel, 1, 1, 0, -1) == 0);
    assert(text_write_char('A', 0.0f, &pixel, 1, 1, INT_MIN, 1) == 0);
    assert(text_write_char('A', 0.0f, &pixel, 1, 1, INT_MAX, 1) == 0);
    assert(text_write_char('\0', 0.0f, &pixel, 1, 1, 0, 1) == 0);
    assert(pixel == 123.0f);
}

int main(void)
{
    check_metrics();
    check_antialiasing();
    check_clipping();
    check_invalid_buffers();
    puts("Annotation font checks passed.");
    return 0;
}
