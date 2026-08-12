# Architecture

## Theme ownership

Each `QApplication` owns one private `ThemeManager`. It observes application
palette changes and late child widgets. Public `ThemeController` objects are
scope facades registered with that manager. The nearest attached ancestor wins
for nested scopes.

Application-wide palette changes made by the theme itself are not interpreted
as operating-system theme changes. On supported Qt 6 versions, system color
scheme notifications come from `QStyleHints`; palette observation remains the
fallback for scoped and older-Qt integrations. Switching between an explicit
mode and an equivalent system mode does not repolish the widget tree.

Each scope owns a `StateJournal`. Before a decorator changes a widget, the
journal records the original state. Reapplication restores the previous
snapshot first; detachment restores it permanently. The contract includes
style sheets, palettes, fonts, dynamic properties, attributes, delegates, and
connections.

`WidgetDecorator` is the extension seam for standard controls. Expensive or
structurally sensitive integrations are not implicit decorators: they are
public adapters with explicit lifetimes.

## Host-safe rendering

Application-wide theming never replaces `QApplication::style()`. It uses the
application palette and applies minimal QSS to top-level widget scopes.
`NextgisProxyStyle` is an opt-in factory for standalone applications and must
be installed before constructing UI.

Rendering has one owner per concern:

- `QPalette` supplies standard system color roles;
- core QSS supplies colors, borders, and simple states for semantic widgets;
- `NextgisProxyStyle` owns metrics, subcontrol rectangles, primitives, and
  complex standard controls;
- corporate widgets exist only where an additional semantic or API contract is
  required.

`ExpandableSection` is the reusable owner of large section header interaction,
hover/pressed rendering, reveal motion, borders, and content padding. Consumers
only populate its public content layout; they do not compose local clickable
frames or border overlays. `PageBackground` similarly owns variant defaults and
corner clipping through typed properties.

The proxy style stores transient animation state in the style adapter. It does
not create overlay widgets or mutate control structure during polish. Hosts
that cannot install the proxy style retain native standard-control geometry
with the NextGIS palette.

## Motion

Public motion tokens are two orthogonal scales: `MotionDuration`
(`Instant`, `Fast`, `Normal`, `Slow`) and `MotionEasing` (`Standard`, `Enter`,
`Exit`). A `MotionSpec` combines them, and the Qt motion adapter resolves that
specification into `QVariantAnimation` settings while applying reduced-motion
policy. Component transitions map to a specification inside their owner; they
do not add component names to the public duration scale.

Selection feedback uses one shared inset geometry contract. Pulse painters may
specialize shape and color, but never expand outside widget bounds. Keyboard
focus is represented by Qt `QFocusFrame`; components do not paint a second
focus outline into their own border.

## Public ABI

Exported QObject and QWidget classes are non-copyable, have out-of-line
destructors, and store state behind d-pointers. Public enums live in an
exported Qt namespace. Internal classes and generated resource entry points
must not appear in the dynamic symbol table.

## Tokens and resources

The canonical JSON is validated against a schema. Generators produce committed
C++ leaf tables, core QSS, gallery QSS, and an asset manifest. Runtime library
code does not parse JSON. The core QRC is an explicit allowlist; demo content
is linked only into the optional gallery.

Runtime assets live beside their owner: library assets under
`src/widgets/resources`, framework assets under `src/framework/resources`, and
demo-only assets under `examples/widgets_gallery/resources`. Offline generators
live under `tools/widgets`; they are never required by an installed package.

## Visual test ownership

Canonical rendering contracts for public widgets live in `tests/widgets` and
use an isolated harness linked only to `ngstd::widgets` and Qt. Their Qt 6
baselines are stored in `tests/widgets/baselines/qt6`, so they remain available
when the optional gallery is not built. Gallery visual tests cover only the
gallery application's layout, theme cycling, startup, and adapter integration.
The library harness fixes animation time explicitly for hover, pressed, pulse,
ripple, popup, focus, and expanded-state frames instead of relying on wall-clock
delays.
