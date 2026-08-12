"""Audit production resources and bundled license texts."""

import json
import sys
from pathlib import Path
from typing import Dict, List
from xml.etree import ElementTree


class LicenseAuditor:
    """Require a local license text for every manifest license."""

    LICENSE_FILES: Dict[str, str] = {
        "Apache-2.0": "licenses/Apache-2.0.txt",
        "GPL-2.0-only": "LICENSE",
        "ISC AND MIT": "licenses/Lucide-ISC-Feather-MIT.txt",
        "LicenseRef-Ubuntu-Font-1.0": "licenses/Ubuntu-Font-1.0.txt",
        "MIT": "licenses/Ant-Design-Icons-MIT.txt",
        "OFL-1.1": "licenses/Roboto-OFL-1.1.txt",
    }
    PROHIBITED_CORE_PARTS = (
        "/cards/",
        "/pictograms/",
        "/product-logos/",
        ".woff2",
        "nextgis-tokens.json",
    )

    def __init__(self, workspace_path: Path) -> None:
        self.workspace_path = workspace_path

    def run(self) -> int:
        manifest_path = (
            self.workspace_path
            / "src"
            / "widgets"
            / "resources"
            / "assets-manifest.json"
        )
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        failures: List[str] = []
        used_licenses = {
            resource["license"] for resource in manifest["resources"]
        }
        gallery_qrc = (
            self.workspace_path
            / "examples"
            / "widgets_gallery"
            / "gallery.qrc"
        )
        gallery_tree = ElementTree.parse(str(gallery_qrc))
        gallery_resources = []
        for element in gallery_tree.findall(".//file"):
            resource_path = gallery_qrc.parent / Path(element.text or "")
            if not resource_path.is_file():
                failures.append(f"Missing gallery resource: {resource_path}")
                continue
            gallery_resources.append(resource_path)
            if resource_path.suffix == ".woff2":
                used_licenses.add(
                    "OFL-1.1"
                    if resource_path.name.startswith("roboto-")
                    else "LicenseRef-Ubuntu-Font-1.0"
                )
            else:
                used_licenses.add("GPL-2.0-only")
        for license_name in sorted(used_licenses):
            relative_path = self.LICENSE_FILES.get(license_name)
            if not relative_path:
                failures.append(f"Unknown resource license: {license_name}")
                continue
            if not (self.workspace_path / relative_path).is_file():
                failures.append(
                    f"Missing {license_name} text: {relative_path}"
                )
        for resource in manifest["resources"]:
            alias = resource["alias"]
            for prohibited_part in self.PROHIBITED_CORE_PARTS:
                if prohibited_part in alias:
                    failures.append(f"Gallery-only core resource: {alias}")
        gallery_names = {resource.name for resource in gallery_resources}
        if not any(name.endswith(".woff2") for name in gallery_names):
            failures.append("Gallery web fonts are not declared")
        if not any("card" in name for name in gallery_names):
            failures.append("Gallery cards are not declared")
        if "data.png" not in gallery_names:
            failures.append("Gallery pictograms are not declared")
        notices = self.workspace_path / "THIRD_PARTY_NOTICES"
        if not notices.is_file() or not notices.read_text(encoding="utf-8"):
            failures.append("THIRD_PARTY_NOTICES is missing or empty")
        if failures:
            sys.stderr.write("\n".join(failures) + "\n")
            return 1
        return 0


if __name__ == "__main__":
    root_path = Path(__file__).resolve().parents[2]
    sys.exit(LicenseAuditor(root_path).run())
