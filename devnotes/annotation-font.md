# Framing annotations

The Framing tool can print a single line of text in the bottom border on export.
The annotation is saved in the processing profile as `Framing/BorderAnnotation`.
An empty annotation leaves the frame unchanged. The bottom border must be at
least 30 pixels high: a 28-pixel text line and a 2-pixel bottom margin. Text is
right-aligned with the image's right edge and clipped to the frame. Its color is
black or white, chosen to contrast with the border.

The embedded **Annotation Sans Regular** font supports printable ASCII
(U+0020–U+007E). Other bytes are ignored. It uses proportional advances, pair
kerning, and 8-bit antialiased coverage. Measuring and rendering use the same
metrics, including spaces. No installed font or font-rendering library is
required at build time or runtime.

## Font source and regeneration

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

For an export check, use a processing profile with resizing and framing enabled,
a bottom border of at least 30 pixels, and `BorderAnnotation=AV To III WWW`.
Check both a white and a black border. A blank annotation or a bottom border
below 30 pixels should leave the exported frame unchanged.
