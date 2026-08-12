# ngstd::widgets gallery

The gallery is an independent consumer of the library's public API. It is
built only with `NGSTD_WIDGETS_BUILD_GALLERY=ON` and contains its own QSS,
cards, pictograms, and JSON tokens that are not part of the production QRC.

The components section is built from independent factories for standard
controls, cards, item views, and Wizard. Demos use the public `CardButton`,
`Disclosure`, `ExpandableSection`, `PageBackground`, `ThemeSwitch`,
`TableWidget`, and library adapters. The gallery supplies content and layout;
section interaction, background defaults, clipping, and rendering remain
library contracts.

Command-line options:

- `--theme light|dark|system` — initial theme;
- `--section colors|typography|components|backgrounds|brand|icons|motion|tokens|all`
  — section to open, or all sections;
- `--direction ltr|rtl` — layout direction;
- `--screenshot <path>` — save a deterministic Fusion screenshot and exit;
- `--viewport <width>x<height>` — capture viewport size;
- `--animation-probe <directory>` — save `Disclosure`, `Spinner`, and
  `Hero`/`Trial` button animation frames and exit;
- `--theme-cycle-probe` — cycle themes and exit; with `--screenshot`, save the
  final state for comparison with startup;
- `--adapter-probe <directory>` — save `QWizard` and `ComboBox` popup frames
  for integration testing;

Example:

```bash
QT_QPA_PLATFORM=offscreen ./ngstd_widgets_gallery \
    --theme dark \
    --section components \
    --direction rtl \
    --screenshot /tmp/widgets-gallery.png
```

The gallery always installs `NextgisProxyStyle` over Fusion before widgets are
created. This keeps reference geometry consistent on Linux, Windows, and macOS;
library tests cover host-safe scenarios that do not replace the application
style.
