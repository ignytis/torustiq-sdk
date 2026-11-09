# AGENTS.md

Guidance for AI agents and contributors working on torustiq-sdk.

## What this is
Header-only SDK for Torustiq: a pure C API (`include/torustiq/sdk.h`) used by the host
(`torustiq`) and by plugins. Optional C++ helpers live in `include/torustiq/sdk.hpp`.

## Rules
- **Keep `sdk.h` pure C (C99):** fixed-width types, `extern "C"`, no C++ features, no allocation across
  the boundary. Data passed to a callback is owned by the caller; the receiver must copy it.
- Must stay FFI-friendly (e.g. usable from Rust): plain structs, function pointers, int status codes.
- **ABI changes** (struct layouts, vtable fields, signatures) require bumping `TORUSTIQ_SDK_ABI_VERSION`
  and the project version in `CMakeLists.txt`. Prefer appending fields over reordering.
- Keep the SDK dependency-free (no third-party libraries) and portable (Linux, Windows, macOS).
- The CMake target is `torustiq::sdk` (INTERFACE). Do not rename it: the host and plugins link to it.
- Style: Google C++ style, `.clang-format` at the root. C++ code: C++20, `snake_case.hpp`,
  `PascalCase` functions, `kConstant`. C API names use the `torustiq_` / `TORUSTIQ_` prefix.

## Build and test
```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
clang-format --dry-run --Werror $(git ls-files '*.cpp' '*.hpp' '*.h' '*.c')
```
The build must stay warning-free. Tests (`tests/`) check that the header compiles as strict C99 and
C++20 and that helpers behave. Add or update a test with every API change.

## Related projects
`torustiq` (host application) consumes this SDK via `FetchContent`; change the API here first, then
adapt the host. For local development use `-DFETCHCONTENT_SOURCE_DIR_TORUSTIQ_SDK=<path to this repo>`.
