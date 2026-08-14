"""Build Qt style resources from the canonical JSON tokens."""

# ruff: noqa: UP006, UP035

import difflib
import json
import re
import sys
from pathlib import Path
from typing import Any, Dict, List

from split_qss import QssSplitter


class StyleResourceBuilder:
    """Render and verify library and gallery style resources."""

    def __init__(self, workspace_path: Path) -> None:
        self.workspace_path = workspace_path
        self.resource_path = workspace_path / "src" / "widgets" / "resources"
        self.gallery_resource_path = (
            workspace_path / "examples" / "widgets_gallery" / "resources"
        )
        token_path = self.resource_path / "tokens" / "nextgis-tokens.json"
        self.tokens: Dict[str, Any] = json.loads(
            token_path.read_text(encoding="utf-8")
        )
        roles_path = (
            workspace_path
            / "tools"
            / "widgets"
            / "templates"
            / "gallery-roles.txt"
        )
        gallery_roles = {
            line.strip()
            for line in roles_path.read_text(encoding="utf-8").splitlines()
            if line.strip() and not line.startswith("#")
        }
        self.qss_splitter = QssSplitter(gallery_roles)

    def build(self, check_only: bool) -> int:
        """Build resources or verify that generated files are current."""
        light_core, light_gallery = self.qss_splitter.split(
            self._render_qss("light")
        )
        dark_core, dark_gallery = self.qss_splitter.split(
            self._render_qss("dark")
        )
        outputs = {
            self.resource_path / "themes" / "nextgis-light.qss": light_core,
            self.resource_path / "themes" / "nextgis-dark.qss": dark_core,
            self.gallery_resource_path
            / "themes"
            / "nextgis-gallery-light.qss": light_gallery,
            self.gallery_resource_path
            / "themes"
            / "nextgis-gallery-dark.qss": dark_gallery,
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "combobox-chevron-light.svg": self._render_combo_chevron(
                "light", expanded=False
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "combobox-chevron-light-open.svg": self._render_combo_chevron(
                "light", expanded=True
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "combobox-chevron-dark.svg": self._render_combo_chevron(
                "dark", expanded=False
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "combobox-chevron-dark-open.svg": self._render_combo_chevron(
                "dark", expanded=True
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "spinbox-chevron-up-light.svg": self._render_spin_chevron(
                "light", direction="up", disabled=False
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "spinbox-chevron-down-light.svg": self._render_spin_chevron(
                "light", direction="down", disabled=False
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "spinbox-chevron-up-light-disabled.svg": (
                self._render_spin_chevron(
                    "light", direction="up", disabled=True
                )
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "spinbox-chevron-down-light-disabled.svg": (
                self._render_spin_chevron(
                    "light", direction="down", disabled=True
                )
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "spinbox-chevron-up-dark.svg": self._render_spin_chevron(
                "dark", direction="up", disabled=False
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "spinbox-chevron-down-dark.svg": self._render_spin_chevron(
                "dark", direction="down", disabled=False
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "spinbox-chevron-up-dark-disabled.svg": (
                self._render_spin_chevron(
                    "dark", direction="up", disabled=True
                )
            ),
            self.resource_path
            / "assets"
            / "icons"
            / "generated"
            / "spinbox-chevron-down-dark-disabled.svg": (
                self._render_spin_chevron(
                    "dark", direction="down", disabled=True
                )
            ),
        }
        for theme in ("light", "dark"):
            for control in ("checkbox", "radio"):
                for state in (
                    "off",
                    "hover",
                    "checked",
                    "pressed",
                    "disabled",
                    "disabled-checked",
                ):
                    output_path = (
                        self.resource_path
                        / "assets"
                        / "icons"
                        / "generated"
                        / f"{control}-{state}-{theme}.svg"
                    )
                    outputs[output_path] = self._render_selection_indicator(
                        theme,
                        control,
                        state,
                    )
        failures: List[str] = []
        for output_path, content in outputs.items():
            if check_only:
                failures.extend(self._check_output(output_path, content))
                continue
            output_path.parent.mkdir(parents=True, exist_ok=True)
            with output_path.open(
                "w", encoding="utf-8", newline="\n"
            ) as output_file:
                output_file.write(content)

        if failures:
            sys.stderr.write("\n".join(failures) + "\n")
            return 1
        return 0

    def _render_combo_chevron(self, theme: str, expanded: bool) -> str:
        combo_box = self.tokens["desktop"]["component"]["comboBox"]
        color_token = combo_box["arrowColorToken"]
        color = self.tokens["color"][theme][color_token]
        points = "6 15 12 9 18 15" if expanded else "6 9 12 15 18 9"
        return (
            '<?xml version="1.0" encoding="UTF-8"?>\n'
            '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" '
            f'fill="none" stroke="{color}" stroke-width="2" '
            'stroke-linecap="round" stroke-linejoin="round">\n'
            f'  <polyline points="{points}"/>\n'
            "</svg>\n"
        )

    def _render_spin_chevron(
        self, theme: str, direction: str, disabled: bool
    ) -> str:
        spin_box = self.tokens["desktop"]["component"]["spinBox"]
        color_key = (
            "arrowDisabledColorToken" if disabled else "arrowColorToken"
        )
        color = self.tokens["color"][theme][spin_box[color_key]]
        points = (
            "7 14 12 9 17 14" if direction == "up" else ("7 10 12 15 17 10")
        )
        return (
            '<?xml version="1.0" encoding="UTF-8"?>\n'
            '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" '
            f'fill="none" stroke="{color}" stroke-width="2" '
            'stroke-linecap="round" stroke-linejoin="round">\n'
            f'  <polyline points="{points}"/>\n'
            "</svg>\n"
        )

    def _render_selection_indicator(
        self,
        theme: str,
        control: str,
        state: str,
    ) -> str:
        selection = self.tokens["desktop"]["component"]["selectionControl"]
        colors = self.tokens["color"][theme]
        shared_colors = self.tokens["color"]["shared"]
        checked = state in ("checked", "pressed", "disabled-checked")
        if state in ("disabled", "disabled-checked"):
            fill = colors["surfaceMuted"]
            stroke = colors["border"]
            mark = colors["textDisabled"]
        elif checked:
            color_key = "brandActive" if state == "pressed" else "brand"
            fill = (
                shared_colors["brand"]
                if color_key == "brand"
                else (colors[color_key])
            )
            stroke = fill
            mark = shared_colors["white"]
        else:
            fill = colors["surface"]
            stroke = (
                colors["brandHover"]
                if state == "hover"
                else colors["borderStrong"]
            )
            mark = shared_colors["white"]

        if control == "radio":
            shape = (
                f'<circle cx="12" cy="12" r="10.5" fill="{fill}" '
                f'stroke="{stroke}" stroke-width="1.5"/>'
            )
            mark_radius = (
                selection["radioDotSizePx"] * 12 / selection["indicatorSizePx"]
            )
            content = (
                f'<circle cx="12" cy="12" r="{mark_radius:g}" fill="{mark}"/>'
                if checked
                else ""
            )
        else:
            radius = (
                selection["indicatorRadiusPx"]
                * 24
                / selection["indicatorSizePx"]
            )
            shape = (
                f'<rect x="1" y="1" width="22" height="22" '
                f'rx="{radius:g}" fill="{fill}" stroke="{stroke}" '
                'stroke-width="1.5"/>'
            )
            content = (
                f'<polyline points="5 12 10 17 19 7" fill="none" '
                f'stroke="{mark}" stroke-width="2.5" '
                'stroke-linecap="round" stroke-linejoin="round"/>'
                if checked
                else ""
            )
        mark_line = f"  {content}\n" if content else ""
        return (
            '<?xml version="1.0" encoding="UTF-8"?>\n'
            '<svg xmlns="http://www.w3.org/2000/svg" '
            'viewBox="0 0 24 24">\n'
            f"  {shape}\n"
            f"{mark_line}"
            "</svg>\n"
        )

    def _render_qss(self, theme: str) -> str:
        template_path = (
            self.workspace_path
            / "tools"
            / "widgets"
            / "templates"
            / "nextgis.qss.in"
        )
        template = template_path.read_text(encoding="utf-8")
        theme_colors = self.tokens["color"][theme]
        shared_colors = self.tokens["color"]["shared"]
        typography = self.tokens["typography"]
        qt_scale = self.tokens["typography"]["qtScale"]
        control = self.tokens["control"]
        desktop = self.tokens["desktop"]
        desktop_components = desktop["component"]
        gallery = desktop["gallery"]
        corporate = self.tokens["product"]["corporate"]
        data = self.tokens["product"]["data"][theme]
        fieldwork = self.tokens["product"]["fieldwork"]
        hero = self.tokens["product"]["hero"]
        radius = self.tokens["radiusPx"]
        spacing = self.tokens["spacingPx"]
        trial = self.tokens["product"]["trial"]
        toolbox = self.tokens["product"]["toolbox"][theme]
        reference = self.tokens["product"]["reference"]
        asset = self.tokens["product"]["asset"]
        values: Dict[str, Any] = {
            **theme_colors,
            **shared_colors,
            "buttonRadius": radius["button"],
            "cardRadius": radius["card"],
            "controlHeightLarge": control["heightPx"]["large"],
            "controlHeightMedium": control["heightPx"]["medium"],
            "controlHeightSmall": control["heightPx"]["small"],
            "controlContentHeightLarge": control["heightPx"]["large"] - 2,
            "controlContentHeightMedium": control["heightPx"]["medium"] - 2,
            "controlContentHeightSmall": control["heightPx"]["small"] - 2,
            "controlIconSize": control["iconPx"],
            "controlPaddingX": control["paddingX"],
            "searchFieldPaddingLeft": desktop_components["field"][
                "searchPaddingLeftPx"
            ],
            "comboBoxPopup": 0
            if desktop_components["comboBox"]["popupMode"] == "below"
            else 1,
            "comboBoxContentHeight": desktop_components["comboBox"]["heightPx"]
            - 2,
            "comboBoxContentPaddingRight": desktop_components["comboBox"][
                "qtContentPaddingRightPx"
            ],
            "comboBoxDropDownWidth": desktop_components["comboBox"][
                "dropDownWidthPx"
            ],
            "comboBoxPopupItemHeight": desktop_components["comboBox"][
                "popupItemHeightPx"
            ],
            "comboBoxPopupItemPaddingHorizontal": desktop_components[
                "comboBox"
            ]["popupItemPaddingHorizontalPx"],
            "comboBoxPopupItemSpacing": desktop_components["comboBox"][
                "popupItemSpacingPx"
            ],
            "comboBoxPopupMinimumWidth": desktop_components["comboBox"][
                "popupMinimumWidthPx"
            ],
            "comboBoxPopupOffset": desktop_components["comboBox"][
                "popupOffsetPx"
            ],
            "comboBoxPopupPadding": desktop_components["comboBox"][
                "popupPaddingPx"
            ],
            "comboBoxPopupRadius": desktop_components["comboBox"][
                "popupRadiusPx"
            ],
            "comboBoxPopupBorderWidth": desktop_components["comboBox"][
                "popupBorderWidthPx"
            ],
            "comboBoxArrowSize": desktop_components["comboBox"]["arrowSizePx"],
            "comboBoxArrowAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"combobox-chevron-{theme}.svg"
            ),
            "comboBoxArrowOpenAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"combobox-chevron-{theme}-open.svg"
            ),
            "spinBoxContentHeight": desktop_components["spinBox"]["heightPx"]
            - 2,
            "spinBoxContentPaddingRight": desktop_components["spinBox"][
                "contentPaddingRightPx"
            ],
            "spinBoxButtonWidth": desktop_components["spinBox"][
                "buttonWidthPx"
            ],
            "spinBoxButtonHeight": desktop_components["spinBox"][
                "buttonHeightPx"
            ],
            "spinBoxArrowSize": desktop_components["spinBox"]["arrowSizePx"],
            "cardBorderWidth": desktop_components["card"]["borderWidthPx"],
            "focusFrameStrokeWidth": desktop_components["focusFrame"][
                "strokeWidthPx"
            ],
            "tabPaddingHorizontal": desktop_components["tab"][
                "paddingHorizontalPx"
            ],
            "tabPaddingVertical": desktop_components["tab"][
                "paddingVerticalPx"
            ],
            "tabUnderlineHeight": desktop_components["tab"][
                "underlineHeightPx"
            ],
            "themeSwitchButtonWidth": desktop_components["themeSwitch"][
                "buttonWidthPx"
            ],
            "themeSwitchButtonHeight": desktop_components["themeSwitch"][
                "buttonHeightPx"
            ],
            "themeSwitchIndicatorRadius": max(
                0,
                radius["card"]
                - desktop_components["themeSwitch"]["paddingPx"],
            ),
            "itemViewRowHeight": desktop_components["itemView"]["rowHeightPx"],
            "itemViewCellPaddingHorizontal": desktop_components["itemView"][
                "cellPaddingHorizontalPx"
            ],
            "tableHeaderHeight": desktop_components["table"]["headerHeightPx"],
            "tableRowHeight": desktop_components["table"]["rowHeightPx"],
            "tableCellPaddingHorizontal": desktop_components["table"][
                "cellPaddingHorizontalPx"
            ],
            "tableCellPaddingVertical": desktop_components["table"][
                "cellPaddingVerticalPx"
            ],
            "scrollBarExtent": desktop_components["scrollBar"]["extentPx"],
            "scrollBarMinimumThumb": desktop_components["scrollBar"][
                "minimumThumbPx"
            ],
            "scrollBarRadius": desktop_components["scrollBar"]["radiusPx"],
            "scrollBarMargin": desktop_components["scrollBar"]["marginPx"],
            "scrollBarPressed": theme_colors[
                desktop_components["scrollBar"]["pressedColorToken"]
            ]
            if desktop_components["scrollBar"]["pressedColorToken"]
            in theme_colors
            else shared_colors[
                desktop_components["scrollBar"]["pressedColorToken"]
            ],
            "progressHeight": desktop_components["progress"]["heightPx"],
            "progressRadius": desktop_components["progress"]["heightPx"] // 2,
            "spinBoxUpArrowAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"spinbox-chevron-up-{theme}.svg"
            ),
            "spinBoxDownArrowAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"spinbox-chevron-down-{theme}.svg"
            ),
            "spinBoxUpArrowDisabledAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"spinbox-chevron-up-{theme}-disabled.svg"
            ),
            "spinBoxDownArrowDisabledAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"spinbox-chevron-down-{theme}-disabled.svg"
            ),
            "corporateSurface": corporate["surface"],
            "corporateSurfaceActive": corporate["surfaceActive"],
            "corporateSurfaceHover": corporate["surfaceHover"],
            "corporateGradientStrong": corporate["gradientStrong"],
            "corporateGradientClear": corporate["gradientClear"],
            "corporateText": corporate["text"],
            "corporateActionText": corporate["actionText"],
            "corporateBorderSecondary": corporate["borderSecondary"],
            "dataActionBackground": data["actionBackground"],
            "dataActionHoverBackground": data["actionHoverBackground"],
            "dataActionPressedBackground": data["actionPressedBackground"],
            "dataActionText": data["actionText"],
            "dataOutlineHoverBackground": data["outlineHoverBackground"],
            "dataOutlinePressedBackground": data["outlinePressedBackground"],
            "dataSurface": data["surface"],
            "dataText": data["text"],
            "dataBorder": data["border"],
            "disclosureContentPadding": desktop_components["disclosure"][
                "contentPaddingPx"
            ],
            "disclosureHeaderHeight": desktop_components["disclosure"][
                "headerHeightPx"
            ],
            "fieldRadius": radius["field"],
            "fieldworkSurface": fieldwork["surface"],
            "fieldworkSurfaceActive": fieldwork["surfaceActive"],
            "fieldworkSurfaceHover": fieldwork["surfaceHover"],
            "fieldworkText": fieldwork["text"],
            "fieldworkTextSurfaceHover": fieldwork["textSurfaceHover"],
            "heroGradientActiveEnd": hero["gradientActiveEnd"],
            "heroGradientActiveStart": hero["gradientActiveStart"],
            "heroGradientEnd": hero["gradientEnd"],
            "heroGradientHoverEnd": hero["gradientHoverEnd"],
            "heroGradientHoverStart": hero["gradientHoverStart"],
            "heroGradientStart": hero["gradientStart"],
            "galleryTypeStageDisplaySize": gallery["typeStageDisplaySizePx"],
            "noticeAccentWidth": desktop_components["notice"]["accentWidthPx"],
            "noticePaddingHorizontal": desktop_components["notice"][
                "paddingHorizontalPx"
            ],
            "noticePaddingVertical": desktop_components["notice"][
                "paddingVerticalPx"
            ],
            "selectionIndicatorRadius": desktop_components["selectionControl"][
                "indicatorRadiusPx"
            ],
            "selectionIndicatorFocusSize": max(
                0,
                desktop_components["selectionControl"]["indicatorSizePx"]
                - 2
                * desktop_components["selectionControl"]["focusBorderWidthPx"],
            ),
            "selectionIndicatorFocusBorder": desktop_components[
                "selectionControl"
            ]["focusBorderWidthPx"],
            "selectionIndicatorFocusColor": theme_colors[
                desktop_components["selectionControl"]["focusColorToken"]
            ],
            "selectionIndicatorSize": max(
                0,
                desktop_components["selectionControl"]["indicatorSizePx"] - 2,
            ),
            "selectionIndicatorOuterSize": desktop_components[
                "selectionControl"
            ]["indicatorSizePx"],
            "checkBoxIndicatorOffAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"checkbox-off-{theme}.svg"
            ),
            "checkBoxIndicatorHoverAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"checkbox-hover-{theme}.svg"
            ),
            "checkBoxIndicatorCheckedAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"checkbox-checked-{theme}.svg"
            ),
            "checkBoxIndicatorPressedAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"checkbox-pressed-{theme}.svg"
            ),
            "checkBoxIndicatorDisabledAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"checkbox-disabled-{theme}.svg"
            ),
            "checkBoxIndicatorDisabledCheckedAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"checkbox-disabled-checked-{theme}.svg"
            ),
            "radioIndicatorOffAsset": (
                f":/ngstd/widgets/assets/icons/generated/radio-off-{theme}.svg"
            ),
            "radioIndicatorHoverAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"radio-hover-{theme}.svg"
            ),
            "radioIndicatorCheckedAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"radio-checked-{theme}.svg"
            ),
            "radioIndicatorPressedAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"radio-pressed-{theme}.svg"
            ),
            "radioIndicatorDisabledAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"radio-disabled-{theme}.svg"
            ),
            "radioIndicatorDisabledCheckedAsset": (
                ":/ngstd/widgets/assets/icons/generated/"
                f"radio-disabled-checked-{theme}.svg"
            ),
            "referenceSemanticActive": reference["semanticActive"],
            "referenceSemanticHover": reference["semanticHover"],
            "referenceToastSurface": reference["toastSurface"],
            "referenceToastText": reference["toastText"],
            "assetPreviewLight": asset["previewLight"],
            "assetPreviewDark": asset["previewDark"],
            "panelRadius": radius["panel"],
            "qtBodySize": qt_scale["body"]["sizePx"],
            "qtBodyWeight": qt_scale["body"]["weight"],
            "qtBodyFamily": desktop["fontFamilies"]["body"],
            "qtMonoFamily": desktop["fontFamilies"]["mono"],
            "qtMonoSize": qt_scale["mono"]["sizePx"],
            "qtMonoWeight": qt_scale["mono"]["weight"],
            "qtHeadingFamily": desktop["fontFamilies"]["heading"],
            "qtHeading1Size": qt_scale["heading1"]["sizePx"],
            "qtHeading1Weight": qt_scale["heading1"]["weight"],
            "qtHeading2Size": qt_scale["heading2"]["sizePx"],
            "qtHeading2Weight": qt_scale["heading2"]["weight"],
            "typeBodyLargeSize": typography["scale"]["bodyLarge"]["sizePx"],
            "typeBodyLargeWeight": typography["scale"]["bodyLarge"]["weight"],
            "typeBodySize": typography["scale"]["body"]["sizePx"],
            "typeBodyWeight": typography["scale"]["body"]["weight"],
            "typeBodySmallSize": typography["scale"]["bodySmall"]["sizePx"],
            "typeBodySmallWeight": typography["scale"]["bodySmall"]["weight"],
            "typeCaptionSize": typography["scale"]["caption"]["sizePx"],
            "typeCaptionWeight": typography["scale"]["caption"]["weight"],
            "typeControlSize": typography["scale"]["control"]["sizePx"],
            "typeControlWeight": typography["scale"]["control"]["weight"],
            "typeTitleSize": typography["scale"]["title"]["sizePx"],
            "typeTitleWeight": typography["scale"]["title"]["weight"],
            "typeHeading1Size": typography["scale"]["h1"]["sizePx"],
            "typeHeading1Weight": typography["scale"]["h1"]["weight"],
            "typeHeading1SubtitleSize": typography["scale"]["h1Subtitle"][
                "sizePx"
            ],
            "typeHeading1SubtitleWeight": typography["scale"]["h1Subtitle"][
                "weight"
            ],
            "typeHeading1SubtitleLineHeight": typography["scale"][
                "h1Subtitle"
            ]["lineHeightPx"],
            "typeHeading2Size": typography["scale"]["h2"]["sizePx"],
            "typeHeading2Weight": typography["scale"]["h2"]["weight"],
            "typeHeading3Size": typography["scale"]["h3"]["sizePx"],
            "typeHeading3Weight": typography["scale"]["h3"]["weight"],
            "typeHeading4Size": typography["scale"]["h4"]["sizePx"],
            "typeHeading4Weight": typography["scale"]["h4"]["weight"],
            "space2": spacing["2"],
            "space3": spacing["3"],
            "space4": spacing["4"],
            "tagHeight": desktop_components["tag"]["heightPx"],
            "tagContentHeight": desktop_components["tag"]["heightPx"] - 2,
            "tagPaddingHorizontal": desktop_components["tag"][
                "paddingHorizontalPx"
            ],
            "toggleStateContentHeight": desktop_components["toggle"][
                "stateHeightPx"
            ],
            "toggleStateContentWidth": desktop_components["toggle"][
                "stateMinimumWidthPx"
            ]
            - 2 * desktop_components["tag"]["paddingHorizontalPx"],
            "toggleStateRadius": radius["pill"],
            "toastMinimumWidth": desktop_components["toast"]["minimumWidthPx"],
            "toastPaddingHorizontal": desktop_components["toast"][
                "paddingHorizontalPx"
            ],
            "toastPaddingVertical": desktop_components["toast"][
                "paddingVerticalPx"
            ],
            "theme": theme,
            "toolboxSurface": toolbox["surface"],
            "toolboxCardSurface": toolbox["cardSurface"],
            "toolboxFocusOutline": toolbox["focusOutline"],
            "toolboxAccent": toolbox["accent"],
            "toolboxGradientClear": toolbox["gradientClear"],
            "toolboxGradientStrong": toolbox["gradientStrong"],
            "toolboxTagFeaturedBorder": toolbox["tagFeaturedBorder"],
            "toolboxTagFeaturedSurface": toolbox["tagFeaturedSurface"],
            "toolboxTagFeaturedText": toolbox["tagFeaturedText"],
            "toolboxText": toolbox["text"],
            "trialGradientEnd": trial["gradientEnd"],
            "trialGradientHoverEnd": trial["gradientHoverEnd"],
            "trialGradientHoverStart": trial["gradientHoverStart"],
            "trialGradientStart": trial["gradientStart"],
            "trialText": trial["text"],
            "wizardButtonMinimumWidth": desktop_components["wizard"][
                "buttonMinimumWidthPx"
            ],
            "wizardNavigationMargin": desktop_components["wizard"][
                "navigationMarginPx"
            ],
            "wizardPageMargin": desktop_components["wizard"]["pageMarginPx"],
        }
        placeholders = set(re.findall(r"\{\{([A-Za-z0-9]+)\}\}", template))
        missing = placeholders - values.keys()
        if missing:
            names = ", ".join(sorted(missing))
            raise KeyError(f"Missing QSS template values: {names}")
        for name in placeholders:
            template = template.replace(f"{{{{{name}}}}}", str(values[name]))
        return template

    def _check_output(self, output_path: Path, expected: str) -> List[str]:
        if not output_path.exists():
            return [f"Missing generated resource: {output_path}"]
        current = output_path.read_text(encoding="utf-8")
        if current == expected:
            return []
        difference = difflib.unified_diff(
            current.splitlines(),
            expected.splitlines(),
            fromfile=str(output_path),
            tofile=f"{output_path} (generated)",
            lineterm="",
        )
        return ["Generated resource is stale:\n" + "\n".join(difference)]


if __name__ == "__main__":
    workspace_directory = Path(__file__).resolve().parents[2]
    builder = StyleResourceBuilder(workspace_directory)
    sys.exit(builder.build(check_only="--check" in sys.argv[1:]))
