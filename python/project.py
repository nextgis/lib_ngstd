import os
from pathlib import Path

from pyqtbuild import PyQtBindings, PyQtProject


def linker_settings(*environment_names: str) -> list[str]:
    settings = []
    for environment_name in environment_names:
        linker_file = os.environ.get(environment_name)
        if linker_file:
            linker_path = Path(linker_file)
            if linker_path.is_absolute() or "/" in linker_file:
                linker_argument = linker_path.as_posix()
            elif linker_file.startswith("-"):
                linker_argument = linker_file
            else:
                linker_argument = f"-l{linker_file}"
            settings.append(f"LIBS += {linker_argument}")
    return settings


class NGSTDProject(PyQtProject):
    def __init__(self) -> None:
        super().__init__()
        self.bindings_factories = [Core, Framework]


class Core(PyQtBindings):
    def __init__(self, project: PyQtProject) -> None:
        super().__init__(
            project,
            "core",
            builder_settings=linker_settings("NGSTD_CORE_LINKER_FILE"),
            qmake_QT=["network"],
        )


class Framework(PyQtBindings):
    def __init__(self, project: PyQtProject) -> None:
        super().__init__(
            project,
            "framework",
            builder_settings=linker_settings(
                "NGSTD_FRAMEWORK_LINKER_FILE",
                "NGSTD_CORE_LINKER_FILE",
                "NGSTD_OPENSSL_CRYPTO_LINKER_FILE",
            ),
            qmake_QT=["network", "widgets", "concurrent"],
        )
