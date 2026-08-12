# Release procedure

## Required gates

1. Configure and test Qt 5/6 shared and static presets.
2. Run the Qt 5 sanitizer preset with ASan and UBSan.
3. Run formatting, clang-tidy, Ruff, token schema/codegen/QSS consistency,
   resource/license audit, export audit, and install-consumer tests.
4. Run deterministic Fusion screenshots for Light/Dark, LTR/RTL, and DPR 1/2;
   run native-style smoke tests on all three desktop systems.
5. Run QGIS and QtIFW source-build smoke jobs using pinned toolchains.
6. Compare the candidate ABI with the previous stable `abi-dumper` baseline.
7. Export the Conan source package, unpack it in an empty environment, and
   repeat build, test, CMake install, and `find_package()` consumption without
   network or Python.

`1.0.0-rc1` requires a clean matrix, reversible detachment, no forbidden
exports, and passing host smoke jobs. The `1.0.0` tag establishes the first ABI
baseline. Later releases compare against `abi/ngstd_widgets-1.0.0.dump`.
