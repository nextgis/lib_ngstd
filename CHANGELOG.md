# Changelog

All notable changes are documented here. The project follows Semantic
Versioning from 1.0.0 onward.

## [1.0.0] - Unreleased

### Added

- Scoped, nested, reversible theme lifecycle with Light, Dark, and System modes.
- Implicitly shared `ThemeOptions` and typed public enum/flag metatypes.
- Host-safe adapters for existing combo boxes and wizards.
- `ComboBox`, `CardButton`, and reusable `RevealWidget` components.
- Typed compiled token tables and optional `NextgisProxyStyle`.
- Install-consumer, resource, accessibility, and lifecycle test suites.

### Changed

- Public QObject and QWidget types use private implementations.
- Component properties use stable, independent semantic names.
- Gallery-only QSS and assets are isolated from the core resource bundle.

### Removed

- Runtime token JSON lookup and live token editing.
- Empty `CheckBox`, `RadioButton`, and `TabWidget` wrappers.
- `SelectableCard`, `RevealViewport`, and automatic private Qt integrations.
