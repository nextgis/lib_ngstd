import os

from pyqtbuild import PyQtBindings, PyQtProject

class NGSTDProject(PyQtProject):
    def __init__(self):
        super().__init__()
        self.bindings_factories = [Core, Framework]

class Core(PyQtBindings):
    def __init__(self, project):
        super().__init__(project, 'core')

class Framework(PyQtBindings):
    def __init__(self, project):
        super().__init__(project, 'framework')