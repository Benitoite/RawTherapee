# Recovered RawTherapee framing annotation patch

This is the complete annotation feature from PR #7405, rebuilt with a regular
sans serif font and integrated with upstream `RawTherapee/dev`.

- Base: `498f623784e33fd9a7077fcd8937fe0734033366`
- Recovery commit: `4daf78fb1a86670973f18e3d8d32ee3877cf2e7a`
- Local branch: `codex/framing-sans-dev`
- Patch: [rawtherapee-dev-framing-sans-serif.patch](/Users/rb/repo-rt/rawtherapee-dev-framing-sans-serif.patch)
- Integrated source: [/Users/rb/repo-rt/build-font-recovery/dev](/Users/rb/repo-rt/build-font-recovery/dev)
- Preview: [actual exported captions](/Users/rb/repo-rt/rawtherapee-dev-framing-sans-serif-preview.png)

The patch includes the Framing annotation entry, processing profile support,
batch edit tracking, rendering, font data, licensing, a font generator, and
regression checks. The font has proportional character widths, pair kerning,
and antialiased edges. The patch applies directly to `dev`; the earlier PR
patch is not required.

The main checkout at `/Users/rb/repo-rt` now also contains the complete feature.
It previously contained the annotation UI and parameter declarations but was
missing the renderer, font data, translations, and profile save/load changes.
Those missing parts have been restored, together with the remaining UI fixes.
The previous local changes are saved in
`build-font-recovery/before-annotation-repair.patch`.

The [completion patch](/Users/rb/repo-rt/rawtherapee-annotation-completion.patch)
repairs that specific partial state in another checkout. It has already been
applied here. Rebuild and reinstall the application after updating the source;
an existing app bundle still contains its previously compiled code.

The repaired main checkout was rebuilt with OpenMP and LTO enabled. Its GUI and
CLI builds, font sanitizer checks, framed exports, and profile round trips
passed. The local test binaries are in
`/Users/rb/repo-rt/build-annotation-check/Release/MacOS`; the
[repair preview](/Users/rb/repo-rt/rawtherapee-annotation-repaired-preview.png)
shows actual captions exported by that rebuilt CLI.

## Apply to a clean dev checkout

```sh
git apply --check /Users/rb/repo-rt/rawtherapee-dev-framing-sans-serif.patch
git apply /Users/rb/repo-rt/rawtherapee-dev-framing-sans-serif.patch
```

Alternatively, use `git am` to apply it as a commit. The integrated source
folder linked above already contains the change. The recovery commit is also
saved in the main repository, so the local branch can restore the source if
the separate working folder is deleted again.

## Use

Enable resizing and framing, enter text in **Framing → Annotation**, and export.
The bottom border must be at least 30 pixels high. Text is right-aligned with
the image's right edge; long text is clipped to the frame. Printable ASCII
characters are supported. Blank annotations preserve the original framing.

## Verification

- The complete patch was applied to a clean archive of upstream dev, and all
  17 changed files matched the tested source exactly.
- Both the GUI application and CLI built successfully on Apple Silicon macOS.
- Renderer checks passed with AddressSanitizer and UndefinedBehaviorSanitizer.
- White and black border exports, long captions, thin borders, and the minimum
  30-pixel border passed. Image pixels outside the annotation stayed unchanged.
- Saved processing profiles reproduced identical exports. Annotation equality,
  partial profile saves, and batch edit tracking passed separate checks.
- Regenerating the embedded font produced an identical header, and the font
  license was included in the local installation.

Build and test logs are in the integrated source folder's `build-font`
directory. The local build used Ninja, Release mode, system fmt, librsvg,
SIMDe, and `OPTION_OMP=OFF`. Other platforms were not built in this recovery.
