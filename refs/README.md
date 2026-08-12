# Web render references

`light.png` and `dark.png` are full-page browser renders used to compare the
overall hierarchy, density, palette, and component coverage of the Qt gallery.

They are design references, not pixel-perfect Qt baselines. Browser-specific
layout defects must not be copied into the widget implementation. In
particular, selectable-card indicators are aligned with the title row rather
than centered against the complete card content, and cards reserve an explicit
leading indicator slot.

Deterministic Qt image regression baselines live under
`examples/widgets_gallery/tests/baselines`.
