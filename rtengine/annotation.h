/*
 * This file is part of RawTherapee.
 * Licensed under the GNU General Public License, version 3 or later.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <string>

namespace rtengine {

class FramesMetaData;

// A single line in the same order as the editor's image information overlay.
// Missing fields are omitted; no EXIF produces an empty caption.
std::string annotationFromMetadata(const FramesMetaData* metadata);

enum class AnnotationFontMode {
    ANNOTATION_SANS,
    FILM_PLOTTER,
    USER
};

inline const char* annotationFontModeName(AnnotationFontMode mode)
{
    switch (mode) {
        case AnnotationFontMode::FILM_PLOTTER: return "FilmPlotter";
        case AnnotationFontMode::USER: return "User";
        default: return "AnnotationSans";
    }
}

// Unknown values leave the current/default mode intact.
inline bool parseAnnotationFontMode(const std::string& name, AnnotationFontMode& mode)
{
    if (name == "AnnotationSans") {
        mode = AnnotationFontMode::ANNOTATION_SANS;
    } else if (name == "FilmPlotter") {
        mode = AnnotationFontMode::FILM_PLOTTER;
    } else if (name == "User") {
        mode = AnnotationFontMode::USER;
    } else {
        return false;
    }
    return true;
}

constexpr double ANNOTATION_FONT_SIZE_DEFAULT = 29.0;
constexpr double ANNOTATION_FONT_SIZE_MIN = 6.0;
constexpr double ANNOTATION_FONT_SIZE_MAX = 512.0;

inline double sanitizeAnnotationFontSize(double size)
{
    return std::isfinite(size)
        ? std::clamp(size, ANNOTATION_FONT_SIZE_MIN, ANNOTATION_FONT_SIZE_MAX)
        : ANNOTATION_FONT_SIZE_DEFAULT;
}

// channels start at the top of the bottom border; stride is measured in floats.
// Right-align to rightEdge, leaving a two-pixel bottom margin. The bitmap modes
// retain their native metrics; userFont is a Pango family/style description and
// fontSize is in exported image pixels, independent of screen DPI.
void drawAnnotation(const std::string& text, AnnotationFontMode mode,
                    const std::string& userFont, double fontSize, float value,
                    float* const channels[3], int width, int borderHeight,
                    int rightEdge, int stride);

} // namespace rtengine
