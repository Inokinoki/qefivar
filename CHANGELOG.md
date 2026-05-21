# Changelog

All notable changes to the `polish` branch are documented in this file.

## [Unreleased] — Polish Branch

### Fixed

- **Critical: Comma operator bugs** in device path parsers — expressions like `size -= 2, c += 2` in `while` conditions were evaluated as comma operators, causing incorrect loop control
- **Critical: Unaligned memory access** — replaced raw pointer casts with portable `qefi_read_le<T>` / `qefi_write_le<T>` templates using `memcpy` + `qFromLittleEndian`
- **Critical: NVDIMM buffer over-read** — size check used 8 bytes instead of 16 for the GUID field
- **iSCSI LUN size check** — `sizeof(quint8) * 16` corrected to `sizeof(quint8) * 8` to match UEFI spec
- **Load option description length** — `qefi_internal_description_length` used `size` (total) instead of `remaining` (bytes after header), causing out-of-bounds reads
- **`qefi_loadopt_description_length`** deprecated wrapper — now delegates to the fixed internal function
- **`qefi_format_string_to_ucs2`** — rewrote using `QString::toUtf16()` to correctly handle 4-byte UTF-8 sequences (emoji, CJK extensions)
- **`qefi_rfc4122_to_guid`** — minimum length check tightened from `< 8` to `< 16` bytes
- **`qefi_parse_dp`** — added null pointer guard
- **`qefi_format_dp`** — added null pointer guard (consistent with parse side)
- **Dummy backend `qefi_get_variable_uint16`** — removed double byte-swap (`qefi_read_le` already converts endianness)
- **WiFi SSID `ssid_len`** — added `qMax(0, ...)` guard against negative values
- **BIOS Boot device path** — implemented description field parsing (was `TODO`)
- **SCSI constructor** — fixed parameter naming
- **SATA LUN** — fixed type mismatch (`quint8` → `quint16`)
- **"Formating" → "Formatting"** typo** in all occurrences
- **Double semicolons** in UART device path parser
- **MSVC build** — added `/utf-8` flag for non-ASCII source strings
- **Windows Qt5** — pinned to `windows-2022` for MSVC compatibility
- **Windows tests** — set `QT_QPA_PLATFORM=offscreen` to run headless

### Added

- **`qefi_p.h`** internal header — centralized shared declarations, `qefi_read_le`/`qefi_write_le` templates, internal function prototypes
- **`QEFILoadOption` error reporting** — `hasError()` / `lastError()` methods
- **`QEFILoadOption` format validation** — `qefi_validate_load_option` check after formatting
- **Deprecated function wrappers** — old public functions preserved with compiler deprecation warnings
- **Qt5/Qt6 compatibility** — version-aware `Q_DECL_DEPRECATED_X` macros
- **14 test suites** — comprehensive device path tests (Hardware, ACPI, BIOS Boot, Media, Message, Chain), load option tests (parsing, formatting, helpers, mutators), boot path tests
- **Qt 6.10.3 CI** — added Windows CI for latest Qt6 version

### Changed

- **Logging** — replaced `qDebug()` with `qCDebug(QEFI_LOG)` for conditional output
- **CI workflows** — split into separate files per platform:
  - `build-qefi-linux-release.yml` (Qt5, Qt6)
  - `build-qefi-windows-release.yml` (Qt 5.15.2, 6.8.3, 6.10.3)
  - `build-qefi-freebsd-release.yml`
- **`qefi_format_string_to_ucs2` parameter** — changed from `QString` to `const QString &`

### Removed

- **Unused `QEFI_DEPRECATED_X` macro** from `qefi_p.h`
- **Duplicate forward declarations** from `qefi.cpp`
- **Build warnings** — all cleared with `-Wall -Wextra`
