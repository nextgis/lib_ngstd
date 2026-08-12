# Widgets resource tools

The scripts in this directory maintain committed `ngstd_widgets` resources.
Normal CMake and Conan builds do not execute them.

- `build_style_resources.py` generates library and gallery QSS plus control
  indicator SVG files.
- `render_cpp_tokens.py` generates the private C++ token table.
- `build_asset_manifest.py` records every file embedded by `widgets.qrc`.
- `validate_tokens.py`, `check_generated_tokens.py`, and `check_licenses.py`
  provide CI consistency checks.

Run the complete resource audit from the repository root:

```bash
python3 tools/widgets/validate_tokens.py \
    src/widgets/resources/tokens/nextgis-tokens.json
python3 tools/widgets/check_generated_tokens.py \
    src/widgets/resources/tokens/nextgis-tokens.json \
    src/widgets/src/generated/token_data_p.h
python3 tools/widgets/build_style_resources.py --check
python3 tools/widgets/build_asset_manifest.py --check
python3 tools/widgets/check_licenses.py
```
