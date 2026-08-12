"""Reject private implementation symbols exported by a shared library."""

import argparse
import subprocess
import sys
from pathlib import Path
from typing import List


class ExportChecker:
    """Inspect an ELF shared object using nm and c++filt."""

    FORBIDDEN_PARTS = (
        "ngstd::widgets::internal::",
        "ThemeControllerPrivate",
        "Private::",
        "qInitResources_",
        "qCleanupResources_",
    )

    def __init__(self, library_path: Path) -> None:
        self.library_path = library_path

    def run(self) -> int:
        result = subprocess.run(
            ["nm", "-D", "--defined-only", str(self.library_path)],
            check=True,
            capture_output=True,
            text=True,
        )
        demangled = subprocess.run(
            ["c++filt"],
            check=True,
            capture_output=True,
            input=result.stdout,
            text=True,
        ).stdout
        failures: List[str] = []
        for line in demangled.splitlines():
            if any(part in line for part in self.FORBIDDEN_PARTS):
                failures.append(line)
        if failures:
            sys.stderr.write("Forbidden exported symbols:\n")
            sys.stderr.write("\n".join(failures) + "\n")
            return 1
        return 0


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("library", type=Path)
    arguments = parser.parse_args()
    sys.exit(ExportChecker(arguments.library).run())
