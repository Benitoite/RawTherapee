// Font and original renderer (c) Abraham Stolk. Licensed under the GPL.
#ifndef RTENGINE_FILMPLOTTER_H
#define RTENGINE_FILMPLOTTER_H

#define FILM_PLOTTER_HEIGHT 28
#define FILM_PLOTTER_WIDTH 24
#define FILM_PLOTTER_ADVANCE 32

#ifdef __cplusplus
extern "C" {
#endif

int film_plotter_line_width(const char* text);
int film_plotter_write_line(const char* text, float value, float* buffer,
                            int width, int height, int x, int stride);

#ifdef __cplusplus
}
#endif
#endif
