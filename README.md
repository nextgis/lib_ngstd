# ngstd

NextGIS standard Qt library provides common C++ APIs for desktop applications.

## Components

- `ngstd_core`: application setup, version helpers, HTTP requests, JSON helpers.
- `ngstd_framework`: desktop application framework, OAuth flow, account access,
  logging and update helpers.
- `ngstd_widgets` 1.0: release-ready Qt Widgets design system with reversible
  scoped themes, typed design tokens, semantic components, and host-safe
  QGIS/QtIFW integration.
- Python bindings are available for `core` and `framework` with SIP and
  PyQt matching the selected Qt major version.

## Repository layout

- `src/<module>` contains production C++ code and module-owned resources.
- `src/resources/windows` contains the version resource shared by native
  libraries on Windows.
- `tests/<module>` contains the single canonical test suite for each module.
- `examples/widgets_gallery/resources` contains gallery-only assets.
- `tools/widgets` contains offline generators and resource audits.
- `docs`, `cmake`, `python`, `translations`, and `licenses` contain their
  conventional project-wide artifacts.

## Dependencies

Required dependencies:

- CMake 3.21 or newer
- C++17 compiler
- Qt 5 or Qt 6 with `Core`, `Gui`, `Widgets`, `Network`, `Svg`,
  `LinguistTools`
- OpenSSL when `BUILD_FRAMEWORK=ON`

Optional dependencies:

- `sentry-native` when `WITH_SENTRY=ON`
- Python, SIP 6 and PyQt5/PyQt6 when `WITH_BINDINGS=ON`

## CMake Build

CMake is used for local development builds. The complete `ngstd` package is
delivered by Conan; `ngstd_widgets` additionally installs a standalone CMake
config package for consumers that need only the widgets component.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Useful options:

- `BUILD_CORE=ON|OFF`
- `BUILD_FRAMEWORK=ON|OFF`
- `BUILD_WIDGETS=ON|OFF`
- `NGSTD_WIDGETS_BUILD_GALLERY=ON|OFF`
- `BUILD_TESTING=ON|OFF`
- `NGSTD_QT_MAJOR=5|6`
- `WITH_BINDINGS=ON|OFF`
- `WITH_SENTRY=ON|OFF`

Sentry is disabled by default. Enable it only when `sentry-native` is available
through system packages or Conan-generated CMake config files.

## Conan Build

The recipe is Conan 2 compatible. Conan is the only supported delivery path for
the `ngstd` package. Dependencies can be provided through Conan while plain
system-library CMake builds remain available for development.

```bash
conan install . --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
```

Create the package:

```bash
conan create . --build=missing -s build_type=Release
```

Enable Sentry through Conan:

```bash
conan install . --build=missing -s build_type=Release -o '&:with_sentry=True'
```

Use system Qt instead of Conan Qt:

```bash
conan install . --build=missing -s build_type=Release -o '&:with_conan_qt=False'
```

## Widgets API

The public headers use the `ngstd/widgets` prefix and the CMake target is
`ngstd::widgets`:

```cmake
find_package(ngstd_widgets 1 CONFIG REQUIRED)
target_link_libraries(my_application PRIVATE ngstd::widgets)
```

```cpp
#include <ngstd/widgets/theme.h>

ngstd::widgets::ThemeOptions options;
options.setThemeMode(ngstd::widgets::ThemeMode::System);

auto *theme = ngstd::widgets::ThemeController::attach(window, options);
```

The canonical token source is
`src/widgets/resources/tokens/nextgis-tokens.json`. Generated C++ and QSS
are committed, so normal builds require neither Python nor network access.
See [API overview](docs/api.md), [architecture](docs/architecture.md), and the
[1.0 migration guide](docs/migration-1.0.md).

## License

GNU GPL v2 or, at your option, any later version. See [LICENSE](LICENSE).
