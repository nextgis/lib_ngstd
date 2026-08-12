import re
from pathlib import Path
from typing import List, Optional, Tuple

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy

required_conan_version = ">=2.28"


class NgstdConan(ConanFile):
    name = "ngstd"
    license = "GPL-2.0-or-later"
    author = "NextGIS"
    url = "https://github.com/nextgis/ngstd"
    description = "NextGIS standard Qt library"
    topics = ("nextgis", "qt", "widgets")
    package_type = "library"

    settings = "os", "arch", "compiler", "build_type"
    exports_sources = (
        "CMakeLists.txt",
        "CMakePresets.json",
        "CHANGELOG.md",
        "LICENSE",
        "README.md",
        "THIRD_PARTY_NOTICES",
        "cmake/*",
        "docs/*",
        "examples/*",
        "licenses/*",
        "python/*",
        "src/*",
        "tests/*",
        "tools/*",
        "translations/*",
        "!**/__pycache__/*",
        "!**/*.pyc",
    )

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_qt6": [True, False],
        "with_bindings": [True, False],
        "with_sentry": [True, False],
        "with_conan_qt": [True, False],
        "qt_version": ["ANY"],
        "build_core": [True, False],
        "build_framework": [True, False],
        "build_widgets": [True, False],
        "build_widgets_gallery": [True, False],
        "build_tests": [True, False],
    }

    default_options = {
        "shared": False,
        "fPIC": True,
        "with_qt6": False,
        "with_bindings": False,
        "with_sentry": False,
        "with_conan_qt": True,
        "qt_version": "5.15.19",
        "build_core": True,
        "build_framework": True,
        "build_widgets": True,
        "build_widgets_gallery": False,
        "build_tests": False,
        "qt/*:gui": True,
        "qt/*:widgets": True,
        "qt/*:qtsvg": True,
        "qt/*:qttools": True,
        "sentry-native/*:qt": False,
    }

    def set_version(self) -> None:
        version_header = (
            Path(self.recipe_folder) / "src" / "core" / "version.h"
        )
        version_text = version_header.read_text(encoding="utf-8")
        version_parts = []
        for macro_name in (
            "NGLIB_MAJOR_VERSION",
            "NGLIB_MINOR_VERSION",
            "NGLIB_PATCH_NUMBER",
        ):
            match = re.search(rf"{macro_name}\s+([0-9]+)", version_text)
            if not match:
                raise ConanInvalidConfiguration(
                    f"Cannot read {macro_name} from {version_header}"
                )
            version_parts.append(match.group(1))
        self.version = ".".join(version_parts)

    def config_options(self) -> None:
        if self.settings.os == "Windows":
            del self.options.fPIC

    def validate(self) -> None:
        if self.options.with_sentry and not self.options.build_framework:
            raise ConanInvalidConfiguration(
                "with_sentry=True requires build_framework=True"
            )

        if not self.options.with_conan_qt:
            return

        qt_version = str(self.options.qt_version)
        if self.options.with_qt6 and not qt_version.startswith("6."):
            raise ConanInvalidConfiguration(
                "with_qt6=True requires qt_version=6.x"
            )
        if not self.options.with_qt6 and not qt_version.startswith("5."):
            raise ConanInvalidConfiguration(
                "with_qt6=False requires qt_version=5.x"
            )

    def requirements(self) -> None:
        if self.options.with_conan_qt:
            self.requires(f"qt/{self.options.qt_version}")

        if self.options.build_framework:
            self.requires("openssl/3.6.3")

        if self.options.with_sentry:
            self.requires("sentry-native/0.14.2")

    def layout(self) -> None:
        cmake_layout(self)

    def generate(self) -> None:
        deps = CMakeDeps(self)
        deps.generate()

        toolchain = CMakeToolchain(self)
        toolchain.variables["NGSTD_QT_MAJOR"] = (
            "6" if self.options.with_qt6 else "5"
        )
        toolchain.variables["WITH_BINDINGS"] = bool(self.options.with_bindings)
        toolchain.variables["WITH_SENTRY"] = bool(self.options.with_sentry)
        toolchain.variables["BUILD_CORE"] = bool(self.options.build_core)
        toolchain.variables["BUILD_FRAMEWORK"] = bool(
            self.options.build_framework
        )
        toolchain.variables["BUILD_WIDGETS"] = bool(self.options.build_widgets)
        toolchain.variables["NGSTD_WIDGETS_BUILD_GALLERY"] = bool(
            self.options.build_widgets_gallery
        )
        toolchain.variables["BUILD_TESTING"] = bool(self.options.build_tests)
        toolchain.variables["BUILD_SHARED_LIBS"] = bool(self.options.shared)
        toolchain.generate()

    def build(self) -> None:
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if self.options.build_tests:
            cmake.test()

    def package(self) -> None:
        self._copy_license()
        self._copy_headers()
        self._copy_libraries()
        self._copy_translations()
        self._copy_python_bindings()
        self._copy_widgets_notices()

    def _copy_license(self) -> None:
        copy(
            self,
            "LICENSE",
            src=self.source_folder,
            dst=str(Path(self.package_folder) / "licenses"),
        )

    def _copy_headers(self) -> None:
        headers = {
            "core": (
                "application.h",
                "core.h",
                "request.h",
                "util.h",
                "version.h",
            ),
            "framework": (
                "application.h",
                "framework.h",
                "mainwindow.h",
                "minisplitter.h",
                "navigationpane.h",
                "navigationwidget.h",
                "style.h",
                "styledbar.h",
                "updater.h",
                "access/access.h",
                "access/signbutton.h",
                "access/signdialog.h",
                "logger/baselogger.h",
            ),
            "widgets": (
                "adapters.h",
                "basic_components.h",
                "button.h",
                "card.h",
                "combo_box.h",
                "components.h",
                "design_tokens.h",
                "disclosure.h",
                "icons.h",
                "page_background.h",
                "proxy_style.h",
                "theme.h",
                "theme_options.h",
                "theme_switch.h",
                "types.h",
                "version.h",
                "view_components.h",
                "widget_style.h",
                "widgets.h",
            ),
        }
        enabled_modules = {
            "core": bool(self.options.build_core),
            "framework": bool(self.options.build_framework),
            "widgets": bool(self.options.build_widgets),
        }

        for module_name, module_headers in headers.items():
            if not enabled_modules[module_name]:
                continue

            module_source_dir = Path(self.source_folder) / "src" / module_name
            if module_name == "widgets":
                module_source_dir /= Path("include") / "ngstd" / "widgets"
            module_include_dir = (
                Path(self.package_folder) / "include" / "ngstd" / module_name
            )
            for header in module_headers:
                copy(
                    self,
                    header,
                    src=str(module_source_dir),
                    dst=str(module_include_dir),
                )

    def _copy_libraries(self) -> None:
        enabled_modules = {
            "core": bool(self.options.build_core),
            "framework": bool(self.options.build_framework),
            "widgets": bool(self.options.build_widgets),
        }

        for module_name, enabled in enabled_modules.items():
            if not enabled:
                continue

            module_build_dir = Path(self.build_folder) / "src" / module_name
            library_dir = Path(self.package_folder) / "lib"
            binary_dir = Path(self.package_folder) / "bin"
            for pattern in ("*.a", "*.lib", "*.so", "*.so.*", "*.dylib"):
                copy(
                    self,
                    pattern,
                    src=str(module_build_dir),
                    dst=str(library_dir),
                    keep_path=False,
                )
            copy(
                self,
                "*.dll",
                src=str(module_build_dir),
                dst=str(binary_dir),
                keep_path=False,
            )

    def _copy_translations(self) -> None:
        enabled_modules = {
            "core": bool(self.options.build_core),
            "framework": bool(self.options.build_framework),
        }
        translations_dir = Path(self.package_folder) / "share" / "translations"
        for module_name, enabled in enabled_modules.items():
            if not enabled:
                continue
            copy(
                self,
                "*.qm",
                src=str(Path(self.build_folder) / "src" / module_name),
                dst=str(translations_dir),
                keep_path=False,
            )

    def _copy_python_bindings(self) -> None:
        if not self.options.with_bindings:
            return

        python_build_dir = (
            Path(self.build_folder) / "python" / "package" / "ngstd"
        )
        if not python_build_dir.exists():
            raise ConanInvalidConfiguration(
                f"Python bindings were not built: {python_build_dir}"
            )

        copy(
            self,
            "*",
            src=str(python_build_dir),
            dst=str(
                Path(self.package_folder)
                / "lib"
                / "python3"
                / "dist-packages"
                / "ngstd"
            ),
            excludes=("*/__pycache__/*", "*.pyc"),
        )

    def _copy_widgets_notices(self) -> None:
        if not self.options.build_widgets:
            return

        notices_directory = Path(self.package_folder) / "licenses"
        for filename in ("CHANGELOG.md", "THIRD_PARTY_NOTICES"):
            copy(
                self,
                filename,
                src=self.source_folder,
                dst=str(notices_directory),
            )
        copy(
            self,
            "*.txt",
            src=str(Path(self.source_folder) / "licenses"),
            dst=str(notices_directory),
        )

    def package_info(self) -> None:
        self.cpp_info.set_property("cmake_file_name", "NGSTD")
        self.cpp_info.set_property("pkg_config_name", "ngstd")

        if self.options.build_core:
            self._add_component(
                name="core",
                library="ngstd_core",
                qt_components=("Core", "Network"),
            )

        if self.options.build_framework:
            requirements = ["core", "openssl::crypto"]
            requirements.extend(
                self._qt_requirements("Network", "Widgets", "Concurrent")
            )
            if self.options.with_sentry:
                requirements.append("sentry-native::sentry-native")

            self._add_component(
                name="framework",
                library="ngstd_framework",
                requirements=requirements,
            )

        if self.options.build_widgets:
            self._add_component(
                name="widgets",
                library="ngstd_widgets",
                qt_components=("Core", "Gui", "Widgets", "Svg"),
                include_directory="include",
                static_define="NGSTD_WIDGETS_STATIC",
            )

    def _add_component(
        self,
        name: str,
        library: str,
        qt_components: Tuple[str, ...] = (),
        requirements: Optional[List[str]] = None,
        include_directory: str = "include/ngstd",
        static_define: str = "NGSTD_STATIC",
    ) -> None:
        component = self.cpp_info.components[name]
        component.set_property("cmake_target_name", f"ngstd::{name}")
        component.libs = [library]
        component.includedirs = [include_directory]
        component.requires = list(requirements or ())
        component.requires.extend(self._qt_requirements(*qt_components))

        if not self.options.shared:
            component.defines = [static_define]

    def _qt_requirements(self, *components: str) -> List[str]:
        if not self.options.with_conan_qt:
            return []
        return [f"qt::qt{component}" for component in components]
