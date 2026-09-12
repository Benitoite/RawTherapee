/*
 * This file is part of RawTherapee.
 * Licensed under the GNU General Public License, version 3 or later.
 */
#include "annotation.h"

#include "rtengine.h"

#include <glib.h>

#include <iomanip>
#include <locale>
#include <memory>
#include <sstream>

namespace rtengine {
namespace {

std::string cleanName(const std::string& name)
{
    std::unique_ptr<char, decltype(&g_free)> valid(
        g_utf8_make_valid(name.data(), name.size()), g_free);
    std::string result;
    bool space = false;
    for (const unsigned char c : std::string(valid.get())) {
        if (g_ascii_isspace(c) || g_ascii_iscntrl(c)) {
            space = !result.empty();
        } else {
            if (space) {
                result += ' ';
                space = false;
            }
            result += c;
        }
    }
    return g_ascii_strcasecmp(result.c_str(), "Unknown") == 0 ? "" : result;
}

std::string number(double value, int precision)
{
    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

bool positive(double value)
{
    return std::isfinite(value) && value > 0.0;
}

} // namespace

std::string annotationFromMetadata(const FramesMetaData* metadata)
{
    if (!metadata || !metadata->hasExif()) {
        return {};
    }

    const auto make = cleanName(metadata->getMake());
    const auto model = cleanName(metadata->getModel());
    // Some metadata providers already include the make in the model.
    const bool includesMake = !make.empty() && model.size() >= make.size()
        && g_ascii_strncasecmp(model.c_str(), make.c_str(), make.size()) == 0
        && (model.size() == make.size() || model[make.size()] == ' ');
    std::string caption = includesMake || make.empty() ? model
        : (model.empty() ? make : make + " " + model);
    const auto lens = cleanName(metadata->getLens());
    if (!lens.empty()) {
        caption += (caption.empty() ? "" : " + ") + lens;
    }

    const auto append = [&caption](const std::string& field) {
        if (!caption.empty()) {
            caption += ' ';
        }
        caption += field;
    };
    const double aperture = metadata->getFNumber();
    if (positive(aperture)) {
        append("f/" + number(aperture, 1));
    }
    const double shutter = metadata->getShutterSpeed();
    if (positive(shutter)) {
        if (shutter <= 0.5 && std::isfinite(1.0 / shutter)) {
            append("1/" + number(1.0 / shutter, 0) + "s");
        } else {
            append(number(shutter, std::floor(shutter) == shutter ? 0 : 1) + "s");
        }
    }
    const int iso = metadata->getISOSpeed();
    if (iso > 0) {
        append("ISO " + std::to_string(iso));
    }
    const double focalLength = metadata->getFocalLen();
    if (positive(focalLength)) {
        append(number(focalLength, 2) + "mm");
    }
    return caption;
}

} // namespace rtengine
