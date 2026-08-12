# Migration from 0.1 to 1.0

The 0.1 API was experimental. Version 1.0 intentionally has no deprecated
compatibility layer.

- Pass `ThemeOptions` to `ThemeController::attach()` or
  `applyToApplication()` instead of a bare mode or coverage value.
- Replace `WITH_QT6` with `NGSTD_QT_MAJOR=6`.
- Replace `SelectableCard` with checkable `CardButton`; add it directly to a
  `QButtonGroup` for exclusive selection.
- Replace `RevealViewport` with `RevealWidget`.
- Replace wrapper `CheckBox`, `RadioButton`, and `TabWidget` with the standard
  Qt classes.
- Use `ComboBox` when popup placement is part of the contract, or attach a
  `ComboBoxAdapter` to an existing `QComboBox`.
- Attach `WizardAdapter` explicitly. Set `changeWizardStyle` only when the
  application permits it.
- Replace general `ngVariant`, `ngState`, and `ngRole` properties with the
  typed `WidgetStyle` API or the stable `ngstd*` properties.
- Replace arbitrary `DesignTokens::value()` access with typed token roles.
- Use `takeTopWidget()`, `takeBodyWidget()`, `takeContentWidget()`, and
  `takeCellContent()` when transferring owned content out of a component.
