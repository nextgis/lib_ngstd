"""Canonical design-token model used by all output renderers."""

import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, List


@dataclass(frozen=True)
class TokenLeaf:
    path: str
    value: Any


class TokenModel:
    def __init__(self, document: Any) -> None:
        self._document = document

    @classmethod
    def load(cls, source_path: Path) -> "TokenModel":
        document = json.loads(source_path.read_text(encoding="utf-8"))
        return cls(document)

    def leaves(self) -> List[TokenLeaf]:
        result: List[TokenLeaf] = []
        self._append_leaves(self._document, "", result)
        return result

    def _append_leaves(
        self,
        value: Any,
        path: str,
        result: List[TokenLeaf],
    ) -> None:
        if isinstance(value, dict):
            for key in sorted(value):
                child_path = f"{path}.{key}" if path else key
                self._append_leaves(value[key], child_path, result)
            return
        if isinstance(value, list):
            for index, child_value in enumerate(value):
                child_path = f"{path}.{index}" if path else str(index)
                self._append_leaves(child_value, child_path, result)
            return
        result.append(TokenLeaf(path=path, value=value))
