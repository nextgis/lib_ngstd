"""Render the canonical token model as a committed C++ leaf table."""

import argparse
import json
from pathlib import Path
from typing import List

from token_model import TokenLeaf, TokenModel


class CppTokenRenderer:
    _RUNTIME_PATHS = {"name", "version"}
    _RUNTIME_PREFIXES = (
        "color.",
        "control.",
        "desktop.component.",
        "desktop.fontFamilies.",
        "desktop.fontResources.",
        "desktop.motion.",
        "effect.",
        "motion.",
        "product.",
        "radiusPx.",
        "spacingPx.",
        "typography.",
    )
    _HEADER = """/*****************************************************
 * Generated file. Do not edit manually.
 * Source: src/widgets/resources/tokens/nextgis-tokens.json
 *****************************************************************************/
#ifndef NGSTD_WIDGETS_GENERATED_TOKEN_DATA_P_H
#define NGSTD_WIDGETS_GENERATED_TOKEN_DATA_P_H

#include <cstddef>

namespace ngstd {
namespace widgets {
namespace internal {

enum class GeneratedTokenKind {
    String,
    Number,
    Boolean,
    Null
};

struct GeneratedTokenEntry {
    const char *path;
    const char *text;
    double number;
    GeneratedTokenKind kind;
    bool boolean;
};

inline constexpr GeneratedTokenEntry generatedTokenEntries[] = {
"""

    _FOOTER = """};

inline constexpr std::size_t generatedTokenEntryCount =
    sizeof(generatedTokenEntries) / sizeof(generatedTokenEntries[0]);

} // namespace internal
} // namespace widgets
} // namespace ngstd

#endif // NGSTD_WIDGETS_GENERATED_TOKEN_DATA_P_H
"""

    def render(self, model: TokenModel) -> str:
        lines: List[str] = [self._HEADER]
        for leaf in model.leaves():
            if not self._is_runtime_leaf(leaf):
                continue
            lines.append(self._render_leaf(leaf))
        lines.append(self._FOOTER)
        return "".join(lines)

    def _is_runtime_leaf(self, leaf: TokenLeaf) -> bool:
        return leaf.path in self._RUNTIME_PATHS or leaf.path.startswith(
            self._RUNTIME_PREFIXES
        )

    def _render_leaf(self, leaf: TokenLeaf) -> str:
        path = self._cpp_string(leaf.path)
        value = leaf.value
        if isinstance(value, bool):
            boolean = "true" if value else "false"
            return (
                f"    {{{path}, nullptr, 0.0, GeneratedTokenKind::Boolean, "
                f"{boolean}}},\n"
            )
        if isinstance(value, (int, float)):
            return (
                f"    {{{path}, nullptr, {float(value)!r}, "
                "GeneratedTokenKind::Number, false},\n"
            )
        if value is None:
            return (
                f"    {{{path}, nullptr, 0.0, "
                "GeneratedTokenKind::Null, false},\n"
            )
        text = self._cpp_string(str(value))
        return (
            f"    {{{path}, {text}, 0.0, "
            "GeneratedTokenKind::String, false},\n"
        )

    @staticmethod
    def _cpp_string(value: str) -> str:
        return json.dumps(value, ensure_ascii=True)


class Application:
    @staticmethod
    def run() -> None:
        parser = argparse.ArgumentParser()
        parser.add_argument("source", type=Path)
        parser.add_argument("output", type=Path)
        arguments = parser.parse_args()
        model = TokenModel.load(arguments.source)
        output = CppTokenRenderer().render(model)
        arguments.output.parent.mkdir(parents=True, exist_ok=True)
        arguments.output.write_text(output, encoding="utf-8")


if __name__ == "__main__":
    Application.run()
