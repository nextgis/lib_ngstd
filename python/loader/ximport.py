# -*- coding: utf-8 -*-
# Zip file protected with password importer
#

__author__ = 'Dmitry Baryshnikov'
__date__ = 'May 2019'
__copyright__ = '(C) 2025, NextGIS'

import importlib.abc
import importlib.machinery
import importlib.util
import logging
import marshal
from pathlib import Path
from types import ModuleType
from typing import Iterator, List, Tuple, Union

import zipfile

LOG_FORMAT: str = "%(levelname)s - %(message)s"
logging.basicConfig(format=LOG_FORMAT)
logger: logging.Logger = logging.getLogger(__name__)
logger.setLevel(logging.WARNING)

__all__: List[str] = ["XImportError", "XZipImporter"]

# --------------------------------------------------------------------------- #
# Helpers                                                                     #
# --------------------------------------------------------------------------- #

_ZIP_SEARCH_ORDER: Tuple[Tuple[str, bool, bool], ...] = (
    ("/__init__.pyc", True, True),
    ("/__init__.py", False, True),
    (".pyc", True, False),
    (".py", False, False),
)


class XImportError(ImportError):
    """Raised when a module cannot be imported from the zip archive."""


def _unmarshal_code(data: bytes):
    """
    Unmarshal a .pyc byte stream into a code object.

    :param data: Raw content of a .pyc file
    :type data: bytes

    :raises XImportError: If unmarshalling fails

    :return: Code object extracted from bytecode
    :rtype: types.CodeType
    """
    for header in (8, 12):
        try:
            return marshal.loads(data[header:])
        except Exception:  # pragma: no cover
            continue
    raise XImportError("Unable to unmarshal byte‑code")


# --------------------------------------------------------------------------- #
# Import machinery                                                            #
# --------------------------------------------------------------------------- #


class XZipImporter(importlib.abc.MetaPathFinder, importlib.abc.Loader):
    """Meta path finder / loader for (optionally encrypted) zip archives.

    :param zip_path: Path to ZIP archive
    :type zip_path: str | Path
    :param password: Optional password
    :type password: str | bytes

    :example:

    >>> import sys
    >>> sys.meta_path.insert(0, XZipImporter("plugins.zip", "secret"))
    >>> import my_plugin
    """

    def __init__(self, zip_path: Union[str, Path], password: Union[str, bytes] = b""):
        self._zip_path: Path = Path(zip_path).expanduser().resolve()
        self._zip: zipfile.ZipFile = zipfile.ZipFile(self._zip_path)

        if password:
            pw = password if isinstance(password, bytes) else password.encode()
            self._zip.setpassword(pw)

        self._files: dict[str, zipfile.ZipInfo] = {
            info.filename: info for info in self._zip.infolist()
        }

    def find_spec(
        self,
        fullname: str,
        path: Union[object, None] = None,
        target: Union[ModuleType, None] = None,
    ) -> Union[importlib.machinery.ModuleSpec, None]:
        """Return a module spec if *fullname* lives inside the archive."""
        try:
            is_package, internal_path = self._locate(fullname)
        except XImportError:
            return None

        origin = f"{self._zip_path}!/{internal_path}"
        spec = importlib.machinery.ModuleSpec(
            name=fullname,
            loader=self,
            origin=origin,
            is_package=is_package,
        )
        if is_package:
            spec.submodule_search_locations = [str(Path(origin).parent)]
        return spec

    def create_module(self, spec):
        """
        Return None to indicate default module creation semantics.

        :param spec: Module specification
        :type spec: ModuleSpec
        :return: Always None
        :rtype: None
        """
        return None

    def exec_module(self, module: ModuleType):
        """
        Execute *module* inside the archive.

        :param module: Module object to execute
        :type module: ModuleType
        """
        fullname = module.__spec__.name  # type: ignore[attr-defined]
        code_obj, is_pkg, internal_path = self._get_code(fullname)

        module.__file__ = f"{self._zip_path}!/{internal_path}"
        module.__loader__ = self
        if is_pkg:
            module.__path__ = [str(Path(module.__file__).parent)]  # type: ignore[attr-defined]
            module.__package__ = fullname
        else:
            module.__package__ = fullname.rpartition(".")[0]

        exec(code_obj, module.__dict__)

    def get_data(self, internal_path: str) -> bytes:
        """
        Return *internal_path* content inside the archive.

        :param internal_path: Relative path inside archive
        :type internal_path: str
        :return: File data
        :rtype: bytes
        """
        try:
            info = self._files[internal_path]
        except KeyError as exc:
            raise FileNotFoundError(internal_path) from exc
        return self._zip.read(info)

    def get_resource_reader(self, fullname: str):
        """
        Return a :py:class:`importlib.abc.ResourceReader` implementation.

        :param fullname: Full module name
        :type fullname: str
        :return: ResourceReader or None
        :rtype: importlib.abc.ResourceReader | None
        """
        try:
            if not self._locate(fullname)[0]:
                return None
        except XImportError:
            return None
        return _ZipResourceReader(self, fullname)

    def _locate(self, fullname: str) -> Tuple[bool, str]:
        """Return (is_package, internal_path) or raise *XImportError*."""
        pkg_path = fullname.replace(".", "/")
        for suffix, _, is_pkg in _ZIP_SEARCH_ORDER:
            internal = f"{pkg_path}{suffix}"
            if internal in self._files:
                return is_pkg, internal
        raise XImportError(f"Module {fullname!r} not found in archive")

    def _get_code(self, fullname: str) -> Tuple[object, bool, str]:
        """Return (code, is_package, internal_path)."""
        pkg_path = fullname.replace(".", "/")
        for suffix, is_bytecode, is_pkg in _ZIP_SEARCH_ORDER:
            internal = f"{pkg_path}{suffix}"
            if internal not in self._files:
                continue
            data = self.get_data(internal)
            code_obj = (
                _unmarshal_code(data)
                if is_bytecode
                else compile(data.decode("utf-8"), internal, "exec")
            )
            return code_obj, is_pkg, internal
        raise XImportError(f"Module {fullname!r} not found in archive")

    def __repr__(self) -> str:
        """Return string representation of the importer."""
        return f"<XZipImporter path={self._zip_path!s}>"


ximporter = XZipImporter


class _ZipResourceReader(importlib.abc.ResourceReader):
    """``importlib.resources`` support for :class:`XZipImporter`."""

    def __init__(self, importer: XZipImporter, package: str):
        self._importer = importer
        self._package_path = package.replace(".", "/")

    def open_resource(self, resource: str):
        """
        Open the given resource from the zip archive.

        :param resource: Relative file path
        :type resource: str
        :return: Binary file object
        :rtype: io.BytesIO
        """
        from io import BytesIO

        data = self._importer.get_data(f"{self._package_path}/{resource}")
        return BytesIO(data)

    def resource_path(self, resource: str):
        """
        Raise error since zipped resources don't have real file paths.

        :param resource: Name of the resource
        :type resource: str
        :raises FileNotFoundError: Always raised
        """
        raise FileNotFoundError(
            "Resources inside zipped modules have no standalone file path"
        )

    def is_resource(self, path: str) -> bool:
        """
        Check if the name refers to a file resource.

        :param name: Resource file name
        :type name: str
        :return: True if present
        :rtype: bool
        """
        try:
            self._importer.get_data(f"{self._package_path}/{path}")
            return True
        except FileNotFoundError:
            return False

    def contents(self) -> Iterator[str]:
        """
        List direct children (files and subdirs) of the resource package.

        :return: Resource names inside package
        :rtype: Iterator[str]
        """
        seen = set()
        prefix = f"{self._package_path}/"
        for internal in self._importer._files:  # noqa: SLF001
            if internal.startswith(prefix):
                tail = internal[len(prefix) :]
                part = tail.split("/", 1)[0]
                if part not in seen:
                    seen.add(part)
                    yield part
