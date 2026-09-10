/*
 * This file is part of RawTherapee.
 * Licensed under the GNU General Public License, version 3 or later.
 */
#include "annotation.h"

#include "filmplotter.h"
#include "text.h"

#include <pango/pangocairo.h>

#include <climits>
#include <cstddef>
#include <memory>

namespace rtengine {
namespace {

struct Unref {
    void operator()(void* object) const { g_object_unref(object); }
};

template<typename T>
using Object = std::unique_ptr<T, Unref>;

void drawUserFont(const std::string& text, const std::string& font, double fontSize,
                  float value, float* const channels[3], int width, int borderHeight,
                  int rightEdge, int stride)
{
    // Each export owns its font map and layout. No GTK widgets, display, or
    // shared mutable Pango context are needed by CLI and concurrent exports.
    Object<PangoFontMap> fontMap(pango_cairo_font_map_new());
    if (!fontMap) {
        return;
    }
    Object<PangoContext> context(pango_font_map_create_context(fontMap.get()));
    pango_cairo_context_set_resolution(context.get(), 96.0);
    std::unique_ptr<cairo_font_options_t, decltype(&cairo_font_options_destroy)>
        fontOptions(cairo_font_options_create(), cairo_font_options_destroy);
    cairo_font_options_set_antialias(fontOptions.get(), CAIRO_ANTIALIAS_GRAY);
    cairo_font_options_set_hint_metrics(fontOptions.get(), CAIRO_HINT_METRICS_ON);
    pango_cairo_context_set_font_options(context.get(), fontOptions.get());

    Object<PangoLayout> layout(pango_layout_new(context.get()));
    std::unique_ptr<PangoFontDescription, decltype(&pango_font_description_free)>
        description(pango_font_description_from_string(font.empty() ? "Sans" : font.c_str()),
                    pango_font_description_free);
    pango_font_description_set_absolute_size(description.get(),
        sanitizeAnnotationFontSize(fontSize) * PANGO_SCALE);
    pango_layout_set_font_description(layout.get(), description.get());
    pango_layout_set_single_paragraph_mode(layout.get(), TRUE);

    std::unique_ptr<char, decltype(&g_free)> valid(
        g_utf8_make_valid(text.data(), text.size()), g_free);
    std::string caption(valid.get());
    for (char& c : caption) {
        if (c == '\n' || c == '\r' || c == '\t') {
            c = ' ';
        }
    }
    if (caption.size() > INT_MAX) {
        return;
    }
    pango_layout_set_text(layout.get(), caption.data(), static_cast<int>(caption.size()));
    PangoRectangle ink, logical;
    pango_layout_get_pixel_extents(layout.get(), &ink, &logical);
    const int top = std::min(ink.y, logical.y);
    const int bottom = std::max(ink.y + ink.height, logical.y + logical.height);
    const int height = bottom - top;
    if (ink.width <= 0 || ink.height <= 0 || height <= 0 || height > borderHeight - 2) {
        return;
    }

    // Allocate only the text band. Keep the photo in its float representation.
    std::unique_ptr<cairo_surface_t, decltype(&cairo_surface_destroy)> surface(
        cairo_image_surface_create(CAIRO_FORMAT_A8, width, height), cairo_surface_destroy);
    if (cairo_surface_status(surface.get()) != CAIRO_STATUS_SUCCESS) {
        return;
    }
    std::unique_ptr<cairo_t, decltype(&cairo_destroy)> cr(cairo_create(surface.get()), cairo_destroy);
    cairo_set_font_options(cr.get(), fontOptions.get());
    cairo_set_source_rgba(cr.get(), 1.0, 1.0, 1.0, 1.0);
    cairo_move_to(cr.get(), static_cast<double>(rightEdge) - logical.x - logical.width, -top);
    pango_cairo_show_layout(cr.get(), layout.get());
    if (cairo_status(cr.get()) != CAIRO_STATUS_SUCCESS) {
        return;
    }
    cairo_surface_flush(surface.get());
    const unsigned char* mask = cairo_image_surface_get_data(surface.get());
    const int maskStride = cairo_image_surface_get_stride(surface.get());
    const int firstRow = borderHeight - height - 2;
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            const unsigned char coverage = mask[static_cast<size_t>(row) * maskStride + col];
            if (!coverage) {
                continue;
            }
            const float alpha = coverage / 255.0f;
            const size_t offset = static_cast<size_t>(row + firstRow) * stride + col;
            for (int channel = 0; channel < 3; ++channel) {
                float& pixel = channels[channel][offset];
                pixel = coverage == 255 ? value : pixel + alpha * (value - pixel);
            }
        }
    }
}

} // namespace

void drawAnnotation(const std::string& text, AnnotationFontMode mode,
                    const std::string& userFont, double fontSize, float value,
                    float* const channels[3], int width, int borderHeight,
                    int rightEdge, int stride)
{
    if (text.empty() || !channels || !channels[0] || !channels[1] || !channels[2]
        || width <= 0 || borderHeight <= 2 || stride < width
        || rightEdge < 0 || rightEdge > width) {
        return;
    }
    if (mode == AnnotationFontMode::USER) {
        drawUserFont(text, userFont, fontSize, value, channels, width, borderHeight, rightEdge, stride);
        return;
    }

    const bool plotter = mode == AnnotationFontMode::FILM_PLOTTER;
    const int height = plotter ? FILM_PLOTTER_HEIGHT : FONTH;
    if (height > borderHeight - 2) {
        return;
    }
    const int textWidth = plotter ? film_plotter_line_width(text.c_str()) : text_line_width(text.c_str());
    const int xpos = rightEdge - textWidth;
    const size_t offset = static_cast<size_t>(borderHeight - height - 2) * stride;
    for (int channel = 0; channel < 3; ++channel) {
        if (plotter) {
            film_plotter_write_line(text.c_str(), value, channels[channel] + offset,
                                   width, height, xpos, stride);
        } else {
            text_write_line(text.c_str(), value, channels[channel] + offset,
                            width, height, xpos, stride);
        }
    }
}

} // namespace rtengine
