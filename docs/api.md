# API overview

## Theme

- `ThemeOptions`: theme mode, animation policy, and feature flags.
- `ThemeController::attach()`: reversible widget-tree scope.
- `ThemeController::applyToApplication()`: explicitly global scope.
- `NextgisProxyStyle::create()`: optional style for standalone applications.
- `MotionSpec`: platform-neutral duration/easing selection resolved by the Qt
  animation adapter.
- `MotionAdapter::configure()`: applies a `MotionSpec` and the inherited
  reduced-motion policy to a `QVariantAnimation`.

Default features are standard controls, semantic components, and
`QDialogButtonBox`. Complex adapters are opt-in.

## Integration

- `ComboBox`: owned popup with placement and alignment properties.
- `ComboBoxAdapter`: reversible styling of an existing combo view.
- `WizardAdapter`: public pages/buttons only; wizard-style changes are opt-in.

## Components

`Button`, `SearchField`, `Tag`, `Notice`, `Toast`, `Spinner`, `TableWidget`,
`ThemeSwitch`, `PageBackground`, `Card`, `CardButton`, `RevealWidget`, and
`Disclosure` follow Qt ownership and property conventions. `ExpandableSection`
adds a section header, description, semantic icon, reveal motion, and a content
layout for assembling larger corporate screens. Mutable properties provide
notification signals and resets where a default exists.

Changing `PageBackground::variant` restores the decoration, grid, and gradient
defaults owned by the new product variant; the individual properties remain
overridable afterwards. `PageBackgroundCornerMode` exposes square, fully
rounded, and top-rounded composition without private dynamic properties.

`SearchField` is a compatibility convenience over `QLineEdit` with a leading
search `QAction`; it does not replace line-edit painting or geometry.

Use `WidgetStyle` for semantic roles on standard Qt widgets. The stable dynamic
property names are `ngstdButtonVariant`, `ngstdCardVariant`, `ngstdTone`,
`ngstdError`, `ngstdSelected`, and `ngstdTypographyRole`. Names beginning with
`_ngstd` are private implementation details.

## Compatibility

The public contract is C++17 with Qt 5.15 or Qt 6.2+. Source consumers select
the Qt branch with `NGSTD_QT_MAJOR=5` or `6`; a configured package
records and resolves the same major automatically.
