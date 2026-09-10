// Standalone checks; see devnotes/annotation-font.md.
// Licensed under the GNU General Public License, version 3 or later.
#include "annotation.h"
#include "filmplotter.h"
#include "text.h"

#include <array>
#include <cassert>
#include <cmath>
#include <future>
#include <iostream>
#include <limits>
#include <vector>

using rtengine::AnnotationFontMode;

namespace {

constexpr int WIDTH = 427, HEIGHT = 80, STRIDE = 439, GUARD = 23;
constexpr float SENTINEL = -123456.0f;
using Pixels = std::array<std::vector<float>, 3>;

Pixels blank()
{
    Pixels result;
    for (int c = 0; c < 3; ++c) {
        result[c].assign(GUARD + HEIGHT * STRIDE + GUARD, SENTINEL);
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                result[c][GUARD + y * STRIDE + x] = 10000.0f * (c + 1);
            }
        }
    }
    return result;
}

Pixels render(AnnotationFontMode mode, const std::string& text,
              const std::string& font = "Sans", double size = 29, int height = HEIGHT)
{
    Pixels pixels = blank();
    float* channels[] = {pixels[0].data() + GUARD, pixels[1].data() + GUARD, pixels[2].data() + GUARD};
    rtengine::drawAnnotation(text, mode, font, size, 65535.0f,
                            channels, WIDTH, height, WIDTH - 19, STRIDE);
    // Guard regions, padded columns, and rows outside the declared border.
    for (int c = 0; c < 3; ++c) {
        for (int i = 0; i < GUARD; ++i) {
            assert(pixels[c][i] == SENTINEL);
            assert(pixels[c][GUARD + HEIGHT * STRIDE + i] == SENTINEL);
        }
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < STRIDE; ++x) {
                const float value = pixels[c][GUARD + y * STRIDE + x];
                if (x >= WIDTH) {
                    assert(value == SENTINEL);
                } else if (y >= height || y >= height - 2) {
                    assert(value == 10000.0f * (c + 1));
                }
            }
        }
    }
    return pixels;
}

void check_bitmaps()
{
    for (const auto mode : {AnnotationFontMode::ANNOTATION_SANS, AnnotationFontMode::FILM_PLOTTER}) {
        for (const auto& text : {std::string("AV To III WWW / jgy"), std::string(100, 'W')}) {
            Pixels expected = blank();
            const int logicalWidth = mode == AnnotationFontMode::ANNOTATION_SANS
                ? text_line_width(text.c_str()) : film_plotter_line_width(text.c_str());
            for (int c = 0; c < 3; ++c) {
                float* start = expected[c].data() + GUARD + (HEIGHT - 30) * STRIDE;
                if (mode == AnnotationFontMode::ANNOTATION_SANS) {
                    text_write_line(text.c_str(), 65535, start, WIDTH, 28, WIDTH - 19 - logicalWidth, STRIDE);
                } else {
                    film_plotter_write_line(text.c_str(), 65535, start, WIDTH, 28, WIDTH - 19 - logicalWidth, STRIDE);
                }
            }
            assert(render(mode, text) == expected);
            assert(render(mode, text, "Serif Bold", 512) == expected);
        }
        assert(render(mode, "Test", "Sans", 29, 29) == blank());
        assert(render(mode, "Test", "Sans", 29, 30) != blank());
        assert(render(mode, "") == blank());
    }
    assert(film_plotter_line_width("III") == 96);
    assert(film_plotter_line_width("WWW") == 96);
    assert(film_plotter_line_width("A\n\xc3\xa9V") == 64);
    // Clip the Film Plotter at each edge and reject extreme origins safely.
    float clipped[28 * 9] = {0};
    assert(film_plotter_write_line("#", 1, clipped, 7, 10, -8, 9) > 0);
    assert(film_plotter_write_line("W", 1, clipped, 7, 2, std::numeric_limits<int>::min(), 9) == 0);
    assert(film_plotter_write_line("W", 1, clipped, 7, 2, std::numeric_limits<int>::max(), 9) == 0);
}

void check_user_fonts()
{
    const auto user = AnnotationFontMode::USER;
    const auto sans = render(user, "AV To III WWW / jgy");
    assert(sans != blank());
    assert(sans != render(user, "AV To III WWW / jgy", "Serif"));
    assert(sans != render(user, "AV To III WWW / jgy", "Sans", 15));
    assert(sans == render(user, "AV To III WWW / jgy", "", 29));
    assert(sans == render(user, "AV To III WWW / jgy", "Sans", std::numeric_limits<double>::quiet_NaN()));
    assert(render(user, "Café Ω العربية 日本語") != blank());
    assert(render(user, "A\xffV") != blank());
    assert(render(user, "A\nB\tC") == render(user, "A B C"));
    assert(render(user, "Missing font", "RawTherapee-Nonexistent-Font-12345") != blank());
    assert(render(user, std::string(1000, 'W')) != blank());
    assert(render(user, "Test", "Sans", 512) == blank());
    assert(render(user, "Test", "Sans", 29, 2) == blank());
    assert(render(user, "   ") == blank());

    int antialiased = 0;
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            const int offset = GUARD + y * STRIDE + x;
            const float a = (sans[0][offset] - 10000.0f) / (65535.0f - 10000.0f);
            assert(a >= 0 && a <= 1);
            antialiased += a > 0 && a < 1;
            for (int c = 1; c < 3; ++c) {
                const float base = 10000.0f * (c + 1);
                const float other = (sans[c][offset] - base) / (65535.0f - base);
                assert(std::abs(a - other) < 1e-6f);
            }
        }
    }
    assert(antialiased > 0);

    std::vector<std::future<Pixels>> jobs;
    for (int i = 0; i < 4; ++i) {
        jobs.push_back(std::async(std::launch::async, []() {
            return render(AnnotationFontMode::USER, "AV To III WWW / jgy");
        }));
    }
    for (auto& job : jobs) {
        assert(job.get() == sans);
    }
}

} // namespace

int main()
{
    check_bitmaps();
    check_user_fonts();
    std::cout << "All three annotation renderers passed: native bitmaps, Unicode, fonts, sizes, clipping, alpha, and concurrency.\n";
}
