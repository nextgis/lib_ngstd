"""Split generated QSS into production and gallery-only rule sets."""

import argparse
import re
from pathlib import Path
from typing import List, Set, Tuple


class QssSplitter:
    _RULE_PATTERN = re.compile(r"([^{}]+)\{([^{}]*)\}", re.DOTALL)
    _ROLE_PATTERN = re.compile(r'_ngstdRole="([^"]+)"')

    def __init__(self, gallery_roles: Set[str]) -> None:
        self._gallery_roles = gallery_roles

    def split(self, source: str) -> Tuple[str, str]:
        core_rules: List[str] = []
        gallery_rules: List[str] = []
        for match in self._RULE_PATTERN.finditer(source):
            selector = match.group(1).strip()
            body = match.group(2).strip()
            rule = f"{selector} {{\n{body}\n}}\n\n"
            roles = set(self._ROLE_PATTERN.findall(selector))
            if roles.intersection(self._gallery_roles):
                gallery_rules.append(rule)
            else:
                core_rules.append(rule)
        banner = (
            "/****** Generated from the canonical NextGIS QSS. ******/\n\n"
        )
        core = banner + "".join(core_rules).rstrip() + "\n"
        gallery = banner + "".join(gallery_rules).rstrip() + "\n"
        return core, gallery


class Application:
    @staticmethod
    def run() -> None:
        parser = argparse.ArgumentParser()
        parser.add_argument("source", type=Path)
        parser.add_argument("roles", type=Path)
        parser.add_argument("core_output", type=Path)
        parser.add_argument("gallery_output", type=Path)
        arguments = parser.parse_args()
        roles = {
            line.strip()
            for line in arguments.roles.read_text(
                encoding="utf-8"
            ).splitlines()
            if line.strip() and not line.startswith("#")
        }
        splitter = QssSplitter(roles)
        core, gallery = splitter.split(
            arguments.source.read_text(encoding="utf-8")
        )
        arguments.core_output.parent.mkdir(parents=True, exist_ok=True)
        arguments.gallery_output.parent.mkdir(parents=True, exist_ok=True)
        arguments.core_output.write_text(core, encoding="utf-8")
        arguments.gallery_output.write_text(gallery, encoding="utf-8")


if __name__ == "__main__":
    Application.run()
