# torustiq-sdk
SDK for [Torustiq](https://github.com/ignytis/torustiq) plugins and the host application.

A header-only, FFI-friendly C API (`include/torustiq/sdk.h`) with fixed-width types and no ownership
transfer across the boundary, plus optional C++ helpers (`include/torustiq/sdk.hpp`). Plugins written
in C, C++, Rust or any language with a C FFI can implement it.

## Concepts
- **Message**: a `payload` (`torustiq_value`) and string headers.
- **Payload**: either simple data (`TORUSTIQ_VALUE_BYTES`: pointer + length, may contain NUL bytes) or
  structured data (null, bool, int, double, string, array, map), so components read and modify
  attributes by key (`torustiq_value_map_get`, `torustiq_value_array_at`, or `torustiq::ValueRef` in
  C++) without parsing and re-serialising. A modified tree may point to unchanged parts of its input.
- **Payload contract**: `create` receives the payload kind of the upstream step
  (`torustiq_payload_spec`: `TORUSTIQ_PAYLOAD_BYTES`, `_VALUE` or `_TEXT`; text is a top-level `TORUSTIQ_VALUE_STRING`) and declares the kind it emits, or
  returns `TORUSTIQ_UNSUPPORTED_PAYLOAD`. The host checks the whole chain when the pipeline is built, so
  `process`/`write` never need to check the kind per message.
- **Config**: string key/value parameters passed to a component (e.g. credentials). Look up with
  `torustiq_config_get` (C) or `torustiq::ConfigGet` (C++).
- **Plugin kinds**, each described by a vtable (`abi_version`, `name`, `create`, `destroy`, ...):
  - *input*: `run` blocks and emits messages through a callback until the source ends or `stop` is called;
  - *transform*: `process` emits zero or more messages for each input message;
  - *output*: `write` consumes a message, `flush` is called once at the end.
- **Status codes**: `TORUSTIQ_OK`, `TORUSTIQ_ERROR`, `TORUSTIQ_STOPPED` (downstream wants no more data),
  `TORUSTIQ_UNSUPPORTED_PAYLOAD` (from `create`).
- **Registration**: a plugin library exports
  `int32_t torustiq_plugin_register(const torustiq_registrar*)` and registers its vtables there.
  The `name` in a vtable is the `type` used in the pipeline YAML.
- Data handed to a callback or vtable function is owned by the caller and valid only during the call;
  copy what you need to keep. Never let exceptions or panics cross the boundary.

## Writing a plugin (sketch)
```c
#include <torustiq/sdk.h>

static int32_t Create(const torustiq_config* config, const torustiq_payload_spec* input, void** out) {
    if (input->kind != TORUSTIQ_PAYLOAD_BYTES) return TORUSTIQ_UNSUPPORTED_PAYLOAD;
    *out = NULL; /* allocate state */
    return TORUSTIQ_OK;
}
static int32_t Write(void* handle, const torustiq_message* m) { /* use m->payload.as.bytes, m->headers */ return TORUSTIQ_OK; }
static void Destroy(void* handle) {}

int32_t torustiq_plugin_register(const torustiq_registrar* r) {
    torustiq_output_vtable vt = {TORUSTIQ_SDK_ABI_VERSION, "my_output", Create, Write, NULL, Destroy};
    return r->register_output(r->context, &vt);
}
```

## Using the SDK from CMake
Host applications and plugins import the SDK the same way:

```cmake
include(FetchContent)
find_package(torustiq_sdk QUIET)
if(NOT torustiq_sdk_FOUND)
    FetchContent_Declare(torustiq_sdk
        GIT_REPOSITORY git@github.com:ignytis/torustiq-sdk.git
        GIT_TAG main)
    FetchContent_MakeAvailable(torustiq_sdk)
endif()
target_link_libraries(my_plugin PRIVATE torustiq::sdk)
```

To use a local checkout instead of fetching: `-DFETCHCONTENT_SOURCE_DIR_TORUSTIQ_SDK=/path/to/torustiq-sdk`.

## Development
```sh
cmake -S . -B build -G Ninja && cmake --build build && ctest --test-dir build --output-on-failure
```
See [AGENTS.md](AGENTS.md) for contribution rules (ABI versioning, style).
