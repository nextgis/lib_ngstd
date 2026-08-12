# ngstd::widgets library

The public entry point is `<ngstd/widgets/components.h>`. Prefer focused
headers in library code.

## Theme lifecycle

`ThemeController::attach(root, options)` themes one widget tree. Nested scopes
are resolved by nearest ancestor. Calling `attach()` again for the same root
updates the existing controller. `detach()` is idempotent and restores the
recorded palette, font, style sheet, properties, delegates, attributes, and
connections.

`ThemeController::applyToApplication(application, options)` is the only global
entry point. It does not replace `QApplication::style()`.

```cpp
ngstd::widgets::ThemeOptions options;
options.setThemeMode(ngstd::widgets::ThemeMode::Dark);
options.setAnimationPolicy(
    ngstd::widgets::AnimationPolicy::System);

auto *controller = ngstd::widgets::ThemeController::attach(dialog, options);
```

Complex integrations are explicit. Use `ComboBox` for a controlled popup,
`ComboBoxAdapter` for an existing combo view/delegate, and `WizardAdapter` for
public `QWizard` pages and buttons. The adapter changes `WizardStyle` only when
explicitly requested.

## Components

Standard `QCheckBox`, `QRadioButton`, and `QTabWidget` are styled directly.
The component API contains `Button`, `SearchField`, `Tag`, `Notice`, `Toast`,
`Spinner`, `TableWidget`, `ThemeSwitch`, `PageBackground`, `Card`,
`CardButton`, `RevealWidget`, `Disclosure`, and `ComboBox`.

Container setters take ownership. Matching `take*Widget()` methods transfer
ownership back to the caller.

## Tokens

Only typed token access is public:

```cpp
const QColor surface = ngstd::widgets::DesignTokens::color(
    ngstd::widgets::ColorRole::CorporateSurface,
    ngstd::widgets::ColorScheme::Light);
```

Runtime JSON themes and arbitrary token lookup are intentionally outside the
1.0 contract.
