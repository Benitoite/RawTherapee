#!/usr/bin/env python3
"""Exercise annotation fonts using a built RawTherapee CLI. Requires Pillow.

Example: python3 tools/test_annotation_exports.py --cli /path/to/rawtherapee-cli
Use --output-dir to retain profiles, exported PNGs, and logs for inspection.
"""
import argparse
import configparser
import os
from pathlib import Path
import subprocess
import tempfile

from PIL import Image, ImageChops, TiffImagePlugin


def run(cli, output):
    output.mkdir(parents=True, exist_ok=True)
    settings = output / "settings"
    settings.mkdir(exist_ok=True)
    env = dict(os.environ, RT_SETTINGS=str(settings), RT_CACHE=str(output / "cache"),
               XDG_CACHE_HOME=str(output / "font-cache"), LC_ALL="C")
    source = output / "input.png"
    Image.new("RGB", (901, 601), (116, 136, 153)).save(source)
    caption = "RawTherapee / Yosemite 2026"

    def preferences(mode="AnnotationSans", font="Sans", size=29):
        (settings / "options").write_text(
            f"[GUI]\nAnnotationFontMode={mode}\nAnnotationFont={font}\nAnnotationFontSize={size}\n",
            encoding="utf-8")

    def export(name, mode="AnnotationSans", font="Sans", size=29, text=caption,
               border=80, color=255, profiles=None, from_exif=False, input_path=None):
        if profiles is None:
            profile = output / (name + ".pp3")
            profile.write_text(
                "[Version]\nVersion=353\n[Resize]\nEnabled=true\nScale=1\nDataSpecified=0\n"
                "[Framing]\nEnabled=true\nFramingMethod=Standard\nAspectRatio=0\n"
                f"BorderSizingMethod=FixedSize\nAbsWidth={border}\nAbsHeight={border}\n"
                f"BorderRed={color}\nBorderGreen={color}\nBorderBlue={color}\nBorderAnnotation={text}\n"
                + (f"AnnotationFromExif={'true' if from_exif else 'false'}\n" if from_exif is not None else "")
                + (f"AnnotationFontMode={mode}\nAnnotationFont={font}\nAnnotationFontSize={size}\n"
                   if mode is not None else ""), encoding="utf-8")
            profiles = [profile]
        destination = output / (name + ".png")
        args = [str(cli), "-q", "-n", "-Y"]
        for profile in profiles:
            args += ["-p", str(profile)]
        input_path = input_path or source
        args += ["-O", str(destination), "-c", str(input_path)]
        with (output / (name + ".log")).open("w") as log:
            subprocess.run(args, check=True, env=env, stdout=log, stderr=subprocess.STDOUT)
        image = Image.open(destination).convert("RGB")
        with Image.open(input_path) as original:
            assert image.size == (original.width + 2 * border, original.height + 2 * border)
        saved = configparser.ConfigParser(interpolation=None)
        saved.read(str(destination) + ".pp3", encoding="utf-8")
        if mode is not None:
            assert saved["Framing"]["AnnotationFontMode"] == mode
            assert saved["Framing"]["AnnotationFont"] == font
            assert float(saved["Framing"]["AnnotationFontSize"]) == size
        assert saved["Framing"]["BorderAnnotation"] == text.strip()
        if from_exif is not None:
            assert saved["Framing"].getboolean("AnnotationFromExif") == from_exif
        print("Passed export:", name, flush=True)
        return image

    def equal(first, second):
        return ImageChops.difference(first, second).getbbox() is None

    def annotation_only(image, blank):
        bounds = ImageChops.difference(image, blank).getbbox()
        assert bounds is not None
        assert bounds[1] >= image.height - 80  # All changes stay in the bottom border.
        assert bounds[3] <= image.height - 2
        assert equal(image.crop((80, 80, 981, 681)), blank.crop((80, 80, 981, 681)))

    preferences()
    blank = export("blank", text="")
    sans = export("annotation-sans")
    plotter = export("film-plotter", mode="FilmPlotter")
    user = export("user-serif", mode="User", font="Serif")
    for image in (sans, plotter, user):
        annotation_only(image, blank)
    assert not equal(sans, plotter) and not equal(sans, user) and not equal(plotter, user)

    dark_blank = export("dark-blank", text="", color=0)
    dark_user = export("user-dark", mode="User", font="Serif", color=0)
    annotation_only(dark_user, dark_blank)
    unicode_text = "Café — Ωμέγα — العربية"
    annotation_only(export("user-unicode", mode="User", font="Sans", text=unicode_text), blank)
    annotation_only(export("user-long", mode="User", text="AV To III WWW " * 100), blank)
    smaller = export("user-small", mode="User", font="Serif", size=15)
    assert not equal(smaller, user)
    assert equal(export("thin", text=caption, border=29), export("thin-blank", text="", border=29))

    # Defaults affect profiles without font fields, then saved font fields win.
    preferences("FilmPlotter", "Serif", 40)
    assert equal(export("default-plotter", mode=None), plotter)
    for name, mode, font, image in [("annotation-sans", "AnnotationSans", "Sans", sans),
                                    ("film-plotter", "FilmPlotter", "Sans", plotter),
                                    ("user-serif", "User", "Serif", user)]:
        assert equal(export(name + "-roundtrip", mode=mode, font=font,
                            profiles=[output / (name + ".png.pp3")]), image)

    # A partial profile changes only its marked font field.
    partial = output / "size-only.pp3"
    partial.write_text("[Version]\nVersion=353\n[Framing]\nAnnotationFontSize=15\n")
    result = export("partial-size", mode="User", font="Serif", size=15,
                    profiles=[output / "user-serif.png.pp3", partial])
    assert equal(result, smaller)
    partial.write_text("[Version]\nVersion=353\n[Framing]\nAnnotationFontMode=Unrecognized\n")
    result = export("unknown-mode", mode="User", font="Serif",
                    profiles=[output / "user-serif.png.pp3", partial])
    assert equal(result, user)

    # Actual EXIF files exercise metadata decoding as well as caption generation.
    def exif_source(name, make, model, lens, aperture, shutter, iso, focal):
        exif = Image.Exif()
        if make:
            exif[271] = make
        if model:
            exif[272] = model
        rational = TiffImagePlugin.IFDRational
        photo = {}
        if lens:
            photo[42036] = lens
        if aperture:
            photo[33437] = rational(aperture)
        if shutter:
            photo[33434] = rational(shutter)
        if iso:
            photo[34855] = iso
        if focal:
            photo[37386] = rational(focal)
        if photo:
            exif[34665] = photo
        path = output / (name + ".jpg")
        Image.new("RGB", (3601, 601), (116, 136, 153)).save(path, exif=exif)
        return path

    nikon = exif_source("nikon", "NIKON CORPORATION", "NIKON D7500",
                        "Sigma 150-600mm F5-6.3 DG OS HSMI C", 7.1, 1 / 6000, 1200, 600)
    expected = "NIKON D7500 + Sigma 150-600mm F5-6.3 DG OS HSMI C f/7.1 1/6000s ISO 1200 600.00mm"
    for mode in ("AnnotationSans", "FilmPlotter", "User"):
        automatic = export("exif-" + mode, mode=mode, from_exif=True, input_path=nikon)
        manual = export("exif-expected-" + mode, mode=mode, text=expected, input_path=nikon)
        assert equal(automatic, manual), mode
        assert not equal(manual, export("exif-blank-" + mode, mode=mode, text="", input_path=nikon))

    # Reusing a saved profile reads the next image's EXIF, not the first caption.
    canon = exif_source("canon", "Canon", "Canon EOS R5", "RF50mm F1.8 STM", 2.8, 2, 400, 50)
    saved_auto = output / "exif-User.png.pp3"
    automatic = export("exif-next-image", mode="User", profiles=[saved_auto], from_exif=True, input_path=canon)
    expected = "Canon EOS R5 + RF50mm F1.8 STM f/2.8 2s ISO 400 50.00mm"
    assert equal(automatic, export("exif-next-expected", mode="User", text=expected, input_path=canon))
    partial.write_text("[Version]\nVersion=353\n[Framing]\nAnnotationFromExif=false\n")
    manual = export("exif-off", mode="User", profiles=[saved_auto, partial], input_path=canon)
    assert equal(manual, export("exif-restored-manual", mode="User", input_path=canon))

    # Missing information stays absent; a camera-only image has no fake exposure fields.
    camera_only = exif_source("camera-only", "NIKON", "D7500", None, None, None, None, None)
    automatic = export("exif-camera-only", from_exif=True, input_path=camera_only)
    assert equal(automatic, export("exif-camera-only-expected", text="NIKON D7500", input_path=camera_only))
    assert equal(export("exif-missing", from_exif=True), blank)
    # Old profiles without the toggle retain manual annotation behavior.
    assert equal(export("legacy-manual", from_exif=None), sans)
    print("All fonts, EXIF captions, preferences, profile round trips, partial profiles, and export bounds passed.", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cli", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path)
    args = parser.parse_args()
    if args.output_dir:
        run(args.cli.resolve(), args.output_dir.resolve())
    else:
        with tempfile.TemporaryDirectory(prefix="rt-annotation-fonts-") as directory:
            run(args.cli.resolve(), Path(directory))


if __name__ == "__main__":
    main()
