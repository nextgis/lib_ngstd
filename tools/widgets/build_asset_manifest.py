"""Build and verify the production Qt resource manifest."""

import argparse
import hashlib
import json
import sys
from pathlib import Path
from typing import Dict, List
from xml.etree import ElementTree


class AssetManifestBuilder:
    """Describe every resource embedded into the production library."""

    def __init__(self, workspace_path: Path) -> None:
        self.workspace_path = workspace_path
        self.qrc_path = (
            workspace_path / "src" / "widgets" / "resources" / "widgets.qrc"
        )
        self.output_path = (
            workspace_path
            / "src"
            / "widgets"
            / "resources"
            / "assets-manifest.json"
        )

    @staticmethod
    def _license_for(alias: str) -> str:
        if "/fonts/qt/roboto-" in alias:
            return "OFL-1.1"
        if "/fonts/qt/ubuntu-" in alias:
            return "LicenseRef-Ubuntu-Font-1.0"
        if "/icons/ant/" in alias:
            return "MIT"
        if "/icons/lucide/" in alias:
            return "ISC AND MIT"
        if "/icons/mdi/" in alias:
            return "Apache-2.0"
        return "GPL-2.0-only"

    def build(self) -> Dict[str, object]:
        resources: List[Dict[str, object]] = []
        tree = ElementTree.parse(str(self.qrc_path))
        for element in tree.findall(".//file"):
            alias = element.attrib["alias"]
            relative_path = Path(element.text or "")
            resource_path = self.qrc_path.parent / relative_path
            if not resource_path.is_file():
                raise FileNotFoundError(resource_path)
            digest = hashlib.sha256(resource_path.read_bytes()).hexdigest()
            resources.append(
                {
                    "alias": f":/ngstd/widgets/{alias}",
                    "license": self._license_for(alias),
                    "sha256": digest,
                    "source": relative_path.as_posix(),
                }
            )
        resources.sort(key=lambda resource: str(resource["alias"]))
        return {"manifestVersion": 1, "resources": resources}

    def run(self, check_only: bool) -> int:
        content = json.dumps(self.build(), indent=2, sort_keys=True) + "\n"
        if check_only:
            if not self.output_path.is_file():
                sys.stderr.write(
                    f"Missing asset manifest: {self.output_path}\n"
                )
                return 1
            if self.output_path.read_text(encoding="utf-8") != content:
                sys.stderr.write(f"Stale asset manifest: {self.output_path}\n")
                return 1
            return 0
        self.output_path.parent.mkdir(parents=True, exist_ok=True)
        self.output_path.write_text(content, encoding="utf-8")
        return 0


class CommandLine:
    """Parse command-line arguments and run the manifest builder."""

    @staticmethod
    def run() -> int:
        parser = argparse.ArgumentParser()
        parser.add_argument("--check", action="store_true")
        arguments = parser.parse_args()
        workspace_path = Path(__file__).resolve().parents[2]
        return AssetManifestBuilder(workspace_path).run(arguments.check)


if __name__ == "__main__":
    sys.exit(CommandLine.run())
