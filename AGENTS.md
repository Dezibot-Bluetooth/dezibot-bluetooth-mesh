# Agent Guide (dezibot-bluetooth-mesh)

This repo is an ESP-IDF project that vendors Arduino-ESP32 and other components.
Use the commands and style rules below when making changes.

## Build, Flash, Clean

- Use ESP-IDF from README: `./install.sh` then `. $HOME/esp/esp-idf/export.sh`.
- Build (project root): `idf.py -B cmake-build-idf build`
- Clean + reconfigure: `idf.py -B cmake-build-idf fullclean reconfigure`
- Flash + monitor: `idf.py -B cmake-build-idf flash monitor`

Convenience scripts (project root):
- `scripts/build`
- `scripts/clean`
- `scripts/flash_and_monitor`

## Tests

Arduino-ESP32 tests live under `components/arduino/tests` and use pytest-embedded.

Setup (Arduino tests):
- `python -m pip install -r components/arduino/tests/requirements.txt`

Run a single test file:
- `pytest -s components/arduino/tests/validation/fs/test_fs.py`

Run a single test by name/keyword:
- `pytest -s components/arduino/tests/validation/fs/test_fs.py -k test_name`

Run a single Arduino validation sketch via helper script:
- `components/arduino/.github/scripts/tests_run.sh -s wifi_ap -t esp32`

Multi-device tests (example):
- `export ESPPORT1=/dev/ttyUSB0`
- `export ESPPORT2=/dev/ttyUSB1`
- `components/arduino/.github/scripts/tests_run.sh -s wifi_ap -t esp32`

Note: pytest defaults include `--embedded-services esp,arduino,wokwi,qemu`
from `components/arduino/tests/pytest.ini`.

## Lint / Format / Style Checks

Pre-commit is the canonical lint/format pipeline for Arduino-ESP32:
- Install: `python -m pip install -r components/arduino/tools/pre-commit/requirements.txt`
- Run all hooks: `pre-commit run`
- Run a single hook: `pre-commit run codespell`

Key hooks in `components/arduino/.pre-commit-config.yaml`:
- C/C++: clang-format (v18.1.3)
- Python: black (line length 120) + flake8 (+ bugbear/comprehensions/simplify)
- YAML: prettier
- Bash: shellcheck + bashate
- Prose: vale + codespell
- General: trailing whitespace, EOF fixer, private key detection

## Code Style (Repo-Wide)

Follow existing local style files before inventing new rules.

### C/C++ / Arduino

Formatting:
- Use clang-format with `components/arduino/.clang-format` (LLVM-based).
- Indent width: 2 spaces; tabs are not used.
- Column limit: 160.
- Braces: attach to control statements; clang-format inserts braces for control flow.
- Includes: do not auto-sort (`SortIncludes: Never`). Keep local grouping.
- Pointer alignment: right (`int* p`).

Imports / Includes:
- Prefer local includes for project headers; system/SDK headers before local ones.
- Keep Arduino `.ino` files including `Arduino.h` when required by tooling (pre-commit
  hook `arduino-include-checker` enforces this).

Naming:
- Match surrounding component style: ESP-IDF uses snake_case for C APIs;
  Arduino uses CamelCase for classes and methods.
- Use clear, descriptive names for mesh roles and BLE concepts (avoid generic
  `data`, `buf`, `tmp` unless in tight scopes).

Types:
- Prefer fixed-width integer types (`uint8_t`, `uint16_t`, `int32_t`) for wire
  formats and BLE payloads.
- Use `size_t` for sizes and counts where appropriate.

Error handling:
- Use ESP-IDF error conventions (`esp_err_t`, `ESP_OK`, `ESP_FAIL`).
- Check and propagate error returns; log with `ESP_LOGE/W/I` where meaningful.
- Avoid silent failures; return early on invalid state.

Memory / concurrency:
- Avoid dynamic allocation in hot paths; prefer static buffers or stack where safe.
- Consider task context and ISR constraints; follow ESP-IDF guidance when touching
  ISR-safe APIs.

### Python (tests/tools)

Formatting:
- Black with `--line-length=120` (configured in pre-commit).
- Indent: 4 spaces.

Linting:
- Flake8 with bugbear/comprehensions/simplify; keep code clean of warnings.

Testing conventions:
- Use pytest fixtures from `components/arduino/tests/conftest.py`.
- Prefer explicit `pytest.fail(...)` for unexpected states.

### Shell scripts

- Indent: 4 spaces (per `.editorconfig`).
- Keep scripts POSIX-compatible unless the file explicitly uses `bash`.
- Pass shellcheck and bashate hooks.

### Markdown / Docs

- Line endings: LF, trim trailing whitespace, final newline.
- Vale + codespell run in CI; keep prose clean.

### EditorConfig

Arduino sub-repo uses `components/arduino/.editorconfig`:
- UTF-8, LF, trim trailing whitespace, final newline.
- Indent sizes: 2 spaces for C/C++/JSON/YAML; 4 spaces for Python and shell.

## Project Layout Notes

- Root project: ESP-IDF app; sources are under `src/` (globbed by `main/CMakeLists.txt`).
- Components: `components/` and `arduino_components/` are vendored.
- Treat `managed_components/` as vendor code; avoid edits unless required.

## Cursor/Copilot Rules

- No `.cursor/rules/`, `.cursorrules`, or `.github/copilot-instructions.md` found
  in this repository at time of writing.

## Common Pitfalls

- Do not run Arduino tests without the pytest-embedded services configured.
- `idf.py` commands require the ESP-IDF environment to be sourced.
- Avoid formatting vendored code unless your change is inside that component.
