# QEFI ![Build on Linux](https://github.com/inokinoki/qefivar/actions/workflows/build-qefi-linux-release.yml/badge.svg) [![Build on Windows](https://github.com/Inokinoki/qefivar/actions/workflows/build-qefi-windows-release.yml/badge.svg)](https://github.com/Inokinoki/qefivar/actions/workflows/build-qefi-windows-release.yml)

A Qt library to read/write EFI variables on Linux (based on efivar) or Windows (using win32 API).

# Build

The library depends on Qt and efivar on *nix OSes, ensure that you have one installed. On Windows, it utilizes the WIN32 API and needs Qt.

```shell
mkdir -p build
cd build
cmake ..
```

By default, cmake will configure and build a static lib.
To build a dynamic lib, configure the project with `BUILD_SHARED_LIBS=On` and then build it.
