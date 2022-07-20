# -*- coding: utf-8 -*-
# Zip file protected with password importer
#

__author__ = 'Dmitry Baryshnikov'
__date__ = 'May 2019'
__copyright__ = '(C) 2019, NextGIS'

import imp
import sys
import os
import logging
import zipfile

log_FORMAT = "%(message)s"
logging.basicConfig(format=log_FORMAT)

'''
To enable debug logging set:
>>> import logging; logging.getLogger('ximport').setLevel(logging.DEBUG)
in your script.
'''
logger = logging.getLogger(__name__)
logger.setLevel(logging.WARN)

__all__ = ['XImportError', 'ximporter']

path_sep = os.path.sep

class XImportError(ImportError):
    pass

_module_type = type(sys)

class ximporter(object):

    def __init__(self, path, password=""):
        self.zf = zipfile.ZipFile(path)
        if password:
            logger.info("Password protected zip")
            self.zf.setpassword(password)
        dirname, basename = os.path.split(path)
        path = dirname
        self.archive = path
        self.prefix = basename + path_sep
        self._files = _read_directory(self, path)

    # Check whether we can satisfy the import of the module named by
    # 'fullname', or whether it could be a portion of a namespace
    # package. Return self if we can load it, a string containing the
    # full path if it's a possible namespace portion, None if we
    # can't load it.
    def find_loader(self, fullname, path=None):
        """find_loader(fullname, path=None) -> self, str or None.

        Search for a module specified by 'fullname'. 'fullname' must be the
        fully qualified (dotted) module name. It returns the ximporter
        instance itself if the module was found, a string containing the
        full path name if it's possibly a portion of a namespace package,
        or None otherwise. The optional 'path' argument is ignored -- it's
        there for compatibility with the importer protocol.
        """

        mi = _get_module_info(self, fullname)
        if mi is not None:
            # This is a module or package.
            return self, []

        # Not a module or regular package. See if this is a directory, and
        # therefore possibly a portion of a namespace package.

        # We're only interested in the last path component of fullname
        # earlier components are recorded in self.prefix.

        modpath = _get_module_path(self, fullname)
        if _is_dir(self, modpath):
            # This is possibly a portion of a namespace
            # package. Return the string representing its path,
            # without a trailing separator.
            return None, ['{}{}{}'.format(self.archive,path_sep,modpath)]

        return None, []

    # Check whether we can satisfy the import of the module named by
    # 'fullname'. Return self if we can, None if we can't.
    def find_module(self, fullname, path=None):
        """find_module(fullname, path=None) -> self or None.

        Search for a module specified by 'fullname'. 'fullname' must be the
        fully qualified (dotted) module name. It returns the ximporter
        instance itself if the module was found, or None if it wasn't.
        The optional 'path' argument is ignored -- it's there for compatibility
        with the importer protocol.
        """

        # Hack to prevent infinity loop in load/unload
        if logger is None:
            return None

        logger.debug("FINDER=================")
        logger.debug("[!] Searching %s" % fullname)
        logger.debug("[!] Path is %s" % path)
        try:
            loader = imp.find_module(fullname, path)
            if loader:
                logger.info("[-] Found locally!")
                return None
        except ImportError:
            pass

        return self.find_loader(fullname, path)[0]

    def get_code(self, fullname):
        """get_code(fullname) -> code object.

        Return the code object for the specified module. Raise XImportError
        if the module couldn't be found.
        """
        code, ispackage, modpath = _get_module_code(self, fullname)
        return code

    def get_data(self, pathname):
        """get_data(pathname) -> string with file data.

        Return the data associated with 'pathname'. Raise OSError if
        the file wasn't found.
        """

        key = pathname
        if pathname.startswith(self.archive + path_sep):
            key = pathname[len(self.archive + path_sep):]

        try:
            toc_entry = self._files[key]
        except KeyError:
            raise OSError(0, '', key)
        return _get_data(self, toc_entry)

    # Return a string matching __file__ for the named module
    def get_filename(self, fullname):
        """get_filename(fullname) -> filename string.

        Return the filename for the specified module.
        """
        # Deciding the filename requires working out where the code
        # would come from if the module was actually loaded
        code, ispackage, modpath = _get_module_code(self, fullname)
        return modpath


    def get_source(self, fullname):
        """get_source(fullname) -> source string.

        Return the source code for the specified module. Raise XImportError
        if the module couldn't be found, return None if the archive does
        contain the module, but has no source for it.
        """
        mi = _get_module_info(self, fullname)
        if mi is None:
            raise XImportError("can't find module {!r}".format(fullname), name=fullname)

        path = _get_module_path(self, fullname)
        if mi:
            fullpath = os.path.join(path, '__init__.py')
        else:
            fullpath = path + '.py'

        try:
            toc_entry = self._files[fullpath]
        except KeyError:
            # we have the module, but no source
            return None
        return _get_data(self, toc_entry) #.decode()

    # Return a bool signifying whether the module is a package or not.
    def is_package(self, fullname):
        """is_package(fullname) -> bool.

        Return True if the module specified by fullname is a package.
        Raise XImportError if the module couldn't be found.
        """
        mi = _get_module_info(self, fullname)
        if mi is None:
            raise XImportError("can't find module {!r}".format(fullname), name=fullname)
        return mi

    # Load and return the module named by 'fullname'.
    def load_module(self, fullname):
        """load_module(fullname) -> module.

        Load the module specified by 'fullname'. 'fullname' must be the
        fully qualified (dotted) module name. It returns the imported
        module, or raises XImportError if it wasn't found.
        """

        code, ispackage, modpath = _get_module_code(self, fullname)
        mod = sys.modules.get(fullname)
        if mod is None or not isinstance(mod, _module_type):
            mod = _module_type(fullname)
            sys.modules[fullname] = mod
        mod.__loader__ = self
        mod.__file__ = modpath

        try:
            if ispackage:
                # add __path__ to the module *before* the code gets
                # executed
                path = _get_module_path(self, fullname)
                fullpath = os.path.join(self.archive, path)
                mod.__path__ = [fullpath]
                # mod.__package__ = fullname.split('.')[0]
            # else:
                # mod.__package__ = fullname

            if not hasattr(mod, '__builtins__'):
                mod.__builtins__ = __builtins__
            exec(code, mod.__dict__)
        except:
            del sys.modules[fullname]
            raise

        try:
            mod = sys.modules[fullname]
        except KeyError:
            raise ImportError('Loaded module {!r} not found in sys.modules'.format(fullname))
        logger.info("[+] '{}' imported from {} succesfully!".format(fullname, modpath))
        return mod


    def get_resource_reader(self, fullname):
        """Return the ResourceReader for a package in a zip file.

        If 'fullname' is a package within the zip file, return the
        'ResourceReader' object for the package.  Otherwise return None.
        """
        try:
            if not self.is_package(fullname):
                return None
        except XImportError:
            return None
        if not _XImportResourceReader._registered:
            from importlib.abc import ResourceReader
            ResourceReader.register(_XImportResourceReader)
            _XImportResourceReader._registered = True
        return _XImportResourceReader(self, fullname)


    def __repr__(self):
        return '<ximporter object "{}{}{}">'.format(self.archive, path_sep, self.prefix)


# Directories can be recognized by the trailing path_sep in the name,
# data_size and file_offset are 0.
def _read_directory(self, archive):
    files = {}
    il = self.zf.infolist()
    for il_item in il:
        name = il_item.filename
        name = name.replace('/', path_sep)
        path = os.path.join(archive, self.prefix, name)
        t = (path, il_item)
        files[self.prefix + name] = t
    return files

# _zip_searchorder defines how we search for a module in the Zip
# archive: we first search for a package __init__, then for
# non-package .pyc, and .py entries. The .pyc entries
# are swapped by initzipimport() if we run in optimized mode. Also,
# '/' is replaced by path_sep there.
_zip_searchorder = (
    (path_sep + '__init__.pyc', True, True),
    (path_sep + '__init__.py', False, True),
    ('.pyc', True, False),
    ('.py', False, False),
)

# Given a module name, return the potential file path in the
# archive (without extension).
def _get_module_path(self, fullname):
    return self.prefix + fullname.rpartition('.')[2]

# Does this path represent a directory?
def _is_dir(self, path):
    # See if this is a "directory". If so, it's eligible to be part
    # of a namespace package. We test by seeing if the name, with an
    # appended path separator, exists.
    dirpath = path + path_sep
    # If dirpath is present in self._files, we have a directory.
    return dirpath in self._files

# Return some information about a module.
def _get_module_info(self, fullname):
    path = _get_module_path(self, fullname)
    for suffix, isbytecode, ispackage in _zip_searchorder:
        fullpath = path + suffix
        if fullpath in self._files:
            return ispackage
    return None

def _unmarshal_code(self, data):
    import marshal
    module_src = None
    try :
        module_src = marshal.loads(data[8:]) # Strip the .pyc file header of Python up to 3.3
        return module_src
    except ValueError :
        pass
    try :
        module_src = marshal.loads(data[12:])# Strip the .pyc file header of Python 3.3 and onwards (changed .pyc spec)
        return module_src
    except ValueError :
        pass
    return module_src


# Get the code object associated with the module specified by
# 'fullname'.
def _get_module_code(self, fullname):
    path = _get_module_path(self, fullname)
    for suffix, isbytecode, ispackage in _zip_searchorder:
        fullpath = path + suffix
        logger.info('trying {}{}{}', self.archive, path_sep, fullpath)
        try:
            toc_entry = self._files[fullpath]
        except KeyError:
            pass
        else:
            modpath = toc_entry[0]
            data = _get_data(self, toc_entry)
            if isbytecode:
                code = _unmarshal_code(self, data)
            else:
                code = _compile_source(modpath, data)
            if code is None:
                # bad magic number or non-matching mtime
                # in byte code, try next
                continue
            modpath = toc_entry[0]
            return code, ispackage, modpath
    else:
        raise XImportError("can't find module {!r}".format(fullname), name=fullname)

# Given a string buffer containing Python source code, compile it
# and return a code object.
def _compile_source(pathname, source):
    return compile(source, pathname, 'exec', dont_inherit=True)


# Given a path to a Zip file and a toc_entry, return the (uncompressed) data.
def _get_data(self, toc_entry):
    return self.zf.read(toc_entry[1]).decode("utf-8")

class _XImportResourceReader:
    """Private class used to support XImport.get_resource_reader().

    This class is allowed to reference all the innards and private parts of
    the ximporter.
    """
    _registered = False

    def __init__(self, ximporter, fullname):
        self.ximporter = ximporter
        self.fullname = fullname

    def open_resource(self, resource):
        fullname_as_path = self.fullname.replace('.', '/')
        path = '{}/{}'.format(fullname_as_path, resource)
        from io import BytesIO
        try:
            return BytesIO(self.ximporter.get_data(path))
        except OSError:
            raise FileNotFoundError(path)

    def resource_path(self, resource):
        # All resources are in the zip file, so there is no path to the file.
        # Raising FileNotFoundError tells the higher level API to extract the
        # binary data and create a temporary file.
        raise FileNotFoundError

    def is_resource(self, name):
        # Maybe we could do better, but if we can get the data, it's a
        # resource.  Otherwise it isn't.
        fullname_as_path = self.fullname.replace('.', '/')
        path = '{}/{}'.format(fullname_as_path, name)
        try:
            self.ximporter.get_data(path)
        except OSError:
            return False
        return True

    def contents(self):
        # This is a bit convoluted, because fullname will be a module path,
        # but _files is a list of file names relative to the top of the
        # archive's namespace.  We want to compare file paths to find all the
        # names of things inside the module represented by fullname.  So we
        # turn the module path of fullname into a file path relative to the
        # top of the archive, and then we iterate through _files looking for
        # names inside that "directory".
        from pathlib import Path
        fullname_path = Path(self.ximporter.get_filename(self.fullname))
        relative_path = fullname_path.relative_to(self.ximporter.archive)
        # Don't forget that fullname names a package, so its path will include
        # __init__.py, which we want to ignore.
        assert relative_path.name == '__init__.py'
        package_path = relative_path.parent
        subdirs_seen = set()
        for filename in self.ximporter._files:
            try:
                relative = Path(filename).relative_to(package_path)
            except ValueError:
                continue
            # If the path of the file (which is relative to the top of the zip
            # namespace), relative to the package given when the resource
            # reader was created, has a parent, then it's a name in a
            # subdirectory and thus we skip it.
            parent_name = relative.parent.name
            if len(parent_name) == 0:
                yield relative.name
            elif parent_name not in subdirs_seen:
                subdirs_seen.add(parent_name)
                yield parent_name
