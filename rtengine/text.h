// (c) Abraham Stolk
// Licensed via GPL.

#ifndef RTENGINE_TEXT_H
#define RTENGINE_TEXT_H

#define FONTH 28

#ifdef __cplusplus
extern "C" {
#endif

// Pixel width using the embedded font's advances and kerning.
// Only printable ASCII is supported; other bytes are ignored.
int text_line_width(const char* text);

// bufh is the available row count starting at buf; stride is in floats.
// Blend antialiased glyphs, clip to the buffer, and return pixels touched.
int text_write_char(char c, float value, float* buf, int bufw, int bufh, int x, int line_stride);
int text_write_line(const char* text, float value, float* buf, int bufw, int bufh, int x, int line_stride);

#ifdef __cplusplus
}
#endif

#endif
