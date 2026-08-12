"""Validate token semantics not expressible by the JSON schema alone."""

import json
import re
import sys
from pathlib import Path
from typing import Any, Dict, List


class JsonSchemaValidator:
    """Validate the JSON Schema subset used by the token contract."""

    def __init__(self, schema: Dict[str, Any]) -> None:
        self._schema = schema

    def validate(self, document: Any) -> List[str]:
        return self._validate(document, self._schema, "$")

    def _validate(
        self,
        value: Any,
        schema: Dict[str, Any],
        path: str,
    ) -> List[str]:
        if "$ref" in schema:
            return self._validate(value, self._resolve(schema["$ref"]), path)
        if "oneOf" in schema:
            matches = [
                candidate
                for candidate in schema["oneOf"]
                if not self._validate(value, candidate, path)
            ]
            if len(matches) != 1:
                return [f"{path}: value must match exactly one schema"]
            return []

        errors: List[str] = []
        expected_type = schema.get("type")
        if expected_type and not self._has_type(value, expected_type):
            return [f"{path}: expected {expected_type}"]

        if isinstance(value, dict):
            errors.extend(self._validate_object(value, schema, path))
        elif isinstance(value, list):
            item_schema = schema.get("items")
            if item_schema:
                for index, item in enumerate(value):
                    errors.extend(
                        self._validate(item, item_schema, f"{path}[{index}]")
                    )
        elif isinstance(value, str):
            minimum_length = schema.get("minLength")
            if minimum_length is not None and len(value) < minimum_length:
                errors.append(f"{path}: string is too short")
            pattern = schema.get("pattern")
            if pattern and not re.fullmatch(pattern, value):
                errors.append(f"{path}: value does not match {pattern}")
        elif isinstance(value, (int, float)) and not isinstance(value, bool):
            minimum = schema.get("minimum")
            if minimum is not None and value < minimum:
                errors.append(f"{path}: value is below {minimum}")
        return errors

    def _validate_object(
        self,
        value: Dict[str, Any],
        schema: Dict[str, Any],
        path: str,
    ) -> List[str]:
        errors: List[str] = []
        for required_name in schema.get("required", []):
            if required_name not in value:
                errors.append(f"{path}: missing required key {required_name}")
        minimum_properties = schema.get("minProperties")
        if minimum_properties is not None and len(value) < minimum_properties:
            errors.append(f"{path}: too few properties")
        property_names = schema.get("propertyNames", {})
        name_pattern = property_names.get("pattern")
        if name_pattern:
            for name in value:
                if not re.fullmatch(name_pattern, name):
                    errors.append(f"{path}: invalid property name {name}")

        properties = schema.get("properties", {})
        additional = schema.get("additionalProperties", True)
        for name, child_value in value.items():
            child_schema = properties.get(name)
            if child_schema is None:
                if additional is False:
                    errors.append(f"{path}: unexpected property {name}")
                    continue
                if isinstance(additional, dict):
                    child_schema = additional
            if child_schema:
                errors.extend(
                    self._validate(
                        child_value,
                        child_schema,
                        f"{path}.{name}",
                    )
                )
        return errors

    def _resolve(self, reference: str) -> Dict[str, Any]:
        if not reference.startswith("#/"):
            raise ValueError(f"Unsupported schema reference: {reference}")
        value: Any = self._schema
        for part in reference[2:].split("/"):
            value = value[part.replace("~1", "/").replace("~0", "~")]
        return value

    @staticmethod
    def _has_type(value: Any, expected_type: str) -> bool:
        predicates = {
            "array": lambda item: isinstance(item, list),
            "boolean": lambda item: isinstance(item, bool),
            "null": lambda item: item is None,
            "number": lambda item: (
                isinstance(item, (int, float)) and not isinstance(item, bool)
            ),
            "object": lambda item: isinstance(item, dict),
            "string": lambda item: isinstance(item, str),
        }
        return predicates[expected_type](value)


class TokenValidator:
    _COLOR_PATTERN = re.compile(
        r"^(#[0-9A-Fa-f]{3,8}|rgba?\([^)]*\)|transparent)$"
    )
    _EASING_PATTERN = re.compile(
        r"^cubic-bezier\((-?[0-9.]+, ?){3}-?[0-9.]+\)$"
    )

    def __init__(self, reference_root: Path) -> None:
        self._reference_root = reference_root
        self._errors: List[str] = []

    def validate(self, token_path: Path, schema_path: Path) -> List[str]:
        document = json.loads(token_path.read_text(encoding="utf-8"))
        schema = json.loads(schema_path.read_text(encoding="utf-8"))
        self._errors.extend(JsonSchemaValidator(schema).validate(document))
        for required_name in (
            "name",
            "version",
            "color",
            "typography",
            "spacingPx",
            "radiusPx",
            "control",
            "desktop",
            "motion",
        ):
            if required_name not in document:
                self._errors.append(f"Missing required token: {required_name}")
        self._visit(document, "")
        return self._errors

    def _visit(self, value: Any, path: str) -> None:
        if isinstance(value, dict):
            for key, child_value in value.items():
                child_path = f"{path}.{key}" if path else key
                self._visit(child_value, child_path)
            return
        if isinstance(value, list):
            for index, child_value in enumerate(value):
                self._visit(child_value, f"{path}.{index}")
            return
        leaf_name = path.rsplit(".", 1)[-1]
        if leaf_name.endswith("Ms"):
            self._check_non_negative_number(value, path)
        if leaf_name.endswith("Px") or leaf_name.endswith("Width"):
            self._check_number(value, path)
        if "opacity" in leaf_name.lower() and not leaf_name.endswith("Ms"):
            if not isinstance(value, (int, float)) or not 0 <= value <= 1:
                self._errors.append(f"Invalid opacity at {path}: {value!r}")
        if "easing" in leaf_name.lower():
            valid_easing = isinstance(
                value,
                str,
            ) and self._EASING_PATTERN.fullmatch(value)
            if not valid_easing:
                self._errors.append(f"Invalid easing at {path}: {value!r}")
        if self._looks_like_color(path, value):
            if not self._COLOR_PATTERN.fullmatch(value):
                self._errors.append(f"Invalid color at {path}: {value!r}")
        if leaf_name.lower().endswith(("asset", "resource")):
            self._check_resource(value, path)

    def _check_number(self, value: Any, path: str) -> None:
        if not isinstance(value, (int, float)) or isinstance(value, bool):
            self._errors.append(f"Expected number at {path}: {value!r}")

    def _check_non_negative_number(self, value: Any, path: str) -> None:
        self._check_number(value, path)
        if isinstance(value, (int, float)) and value < 0:
            self._errors.append(f"Expected non-negative value at {path}")

    def _check_resource(self, value: Any, path: str) -> None:
        if not isinstance(value, str):
            self._errors.append(f"Expected resource path at {path}")
            return
        if not (self._reference_root / value).is_file():
            self._errors.append(f"Missing resource at {path}: {value}")

    @staticmethod
    def _looks_like_color(path: str, value: Any) -> bool:
        if not isinstance(value, str):
            return False
        return (
            path.startswith("color.")
            or value.startswith(("#", "rgb(", "rgba("))
            or value == "transparent"
        )


class Application:
    @staticmethod
    def run() -> int:
        token_path = Path(sys.argv[1]).resolve()
        schema_path = token_path.with_suffix(".schema.json")
        reference_root = token_path.parent.parent
        errors = TokenValidator(reference_root).validate(
            token_path,
            schema_path,
        )
        if errors:
            for error in errors:
                print(error)
            return 1
        return 0


if __name__ == "__main__":
    raise SystemExit(Application.run())
