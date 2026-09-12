# Framing annotations

The Framing tool can print a single line of text in the bottom border on export.
Enable both Resize and Framing to include the border in the export. Text is
right-aligned with the image's right edge and clipped to the frame. Its color is
black or white, chosen to contrast with the border. A two-pixel bottom margin
is reserved; if the text line is taller than the available border, it is omitted.
Long captions require a wider export or, for user fonts, a smaller font size.

## Font selection

Preferences → General → Appearance offers three default annotation fonts:

- **Annotation Sans**: proportional, antialiased embedded bitmap font.
- **Film Plotter**: the original monochrome bitmap font, with 24 × 28 pixel
  glyphs and a fixed 32-pixel advance.
- **User font**: an installed font family/style selected with the font chooser.
  Its size is measured in exported image pixels (6–512), independently of the
  display's DPI. Pango supplies shaping, Unicode support, and fallback for missing
  fonts or glyphs. Exact output depends on the installed fonts.

Both bitmap fonts retain their native 28-pixel line height and require a bottom
border of at least 30 pixels. The user font must fit its full line metrics within
the border, which may require more height than its nominal size.

Preferences supply defaults for new images and profiles without saved font
fields. Each image's profile saves `Framing/AnnotationFontMode` (`AnnotationSans`,
`FilmPlotter`, or `User`), `AnnotationFont` (family/style), and `AnnotationFontSize`.
Use **Use font from Preferences** in Framing to apply changed defaults to an
already-open image. Unknown mode names leave the current/default mode intact.

## Automatic EXIF annotation

The **ⓘ Use EXIF information** switch fills the annotation from the source image
in this order: camera + lens, aperture, shutter speed, ISO, and focal length.
For example:

```text
NIKON D7500 + Sigma 150-600mm F5-6.3 DG OS HSMI C f/7.1 1/6000s ISO 1200 600.00mm
```

Unavailable fields are omitted; an image without EXIF produces no automatic
caption. Camera names are not duplicated when the model already includes the
manufacturer. The text entry previews the generated caption and is read-only
while the switch is on. Turn the switch off to restore the custom annotation.

The profile saves the switch as `Framing/AnnotationFromExif`, defaulting to false
for old profiles, and keeps the custom text in `Framing/BorderAnnotation`.
Automatic text is generated separately for each image at export time, including
batch and command-line exports. It is never frozen to the first image when a
profile is copied. The switch has an independent partial-profile edited flag.

## Rendering

The embedded **Annotation Sans Regular** font supports printable ASCII
(U+0020–U+007E). Other bytes are ignored. It uses proportional advances, pair
kerning, and 8-bit antialiased coverage. Measuring and rendering use the same
metrics, including spaces. Neither bitmap mode needs an installed font.

The engine links Pango/Cairo (`pangocairo >= 1.40`) for user fonts. Each render
owns its font map, context, and layout, without GTK widgets or a display. It
renders an A8 coverage mask for just the text band, then blends it into the
existing float RGB channels. The photo is not converted to an 8-bit surface.

## Font source and regeneration

Film Plotter's original 8,064 bitmap bytes are preserved in
`rtengine/filmplotterfont.h`, from Abraham Stolk's framing annotation
[PR #7405](https://github.com/RawTherapee/RawTherapee/pull/7405), commit
`8b59f18d4edefd9406efc7b4f83aa14402e05e80`, with its GPL attribution.

The bitmap data in `rtengine/fntdat.h` is derived from Liberation Sans Regular
2.1.5, rasterized at 29 pixels with a 28-pixel line height and a baseline at row
22. The derivative is named Annotation Sans Regular to respect the upstream
reserved font names. Its copyright notice and SIL Open Font License 1.1 are in
`licenses/AnnotationSans_LICENSE`, which is included in installations.

Download the upstream
[Liberation Fonts 2.1.5 archive](https://github.com/liberationfonts/liberation-fonts/files/7261482/liberation-fonts-ttf-2.1.5.tar.gz)
and extract `LiberationSans-Regular.ttf`. Its SHA-256 is:

```text
76d04c18ea243f426b7de1f3ad208e927008f961dc5945e5aad352d0dfde8ee8
```

Regeneration requires Python 3, Pillow, and a shared FreeType library. The
checked-in data was generated using Pillow 12.3.0 and FreeType 2.14.3. Different
rasterizer versions can change the pixels or hinting, so review regenerated
data visually. The generator checks the source font hash and extracts
grid-fitted kerning directly from FreeType.

```sh
python3 tools/generate_annotation_font.py /path/to/LiberationSans-Regular.ttf
```

If FreeType cannot be found automatically, pass `--freetype /path/to/library`.
Use `--output /path/to/fntdat.h` to generate a comparison without overwriting the
checked-in data.

## Renderer checks

The standalone test covers proportional spacing, kerning, spaces, unsupported
bytes, antialiasing, clipping, short buffers, padded rows, and extreme origins.
Run it from the repository root with Clang or GCC:

```sh
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
   -Irtengine tools/test_annotation_font.c rtengine/text.c \
   -o /tmp/test-annotation-font
/tmp/test-annotation-font
```

The three-mode test checks bitmap parity, Unicode, font selection, sizes,
missing-font fallback, alpha blending, clipping, padded rows, and concurrent
renders. It requires the Pango/Cairo development package:

```sh
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
   -Irtengine -c rtengine/text.c -o /tmp/annotation-text.o
cc -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
   -Irtengine -c rtengine/filmplotter.c -o /tmp/annotation-plotter.o
c++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -Irtengine tools/test_annotation_renderer.cc rtengine/annotation.cc \
    /tmp/annotation-text.o /tmp/annotation-plotter.o \
    $(pkg-config --cflags --libs pangocairo) -o /tmp/test-annotation-renderers
/tmp/test-annotation-renderers
```

The export test requires Python 3 and Pillow. It creates isolated settings and
synthetic images with EXIF, verifies all three fonts against literal captions,
checks profile reuse on a second camera, missing metadata, manual-text
restoration, font defaults, partial profiles, and border bounds:

```sh
python3 tools/test_annotation_exports.py --cli /path/to/rawtherapee-cli \
    --output-dir /tmp/annotation-exports
```
