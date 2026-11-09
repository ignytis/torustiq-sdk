#ifndef TORUSTIQ_SDK_H_
#define TORUSTIQ_SDK_H_

/*
 * Torustiq plugin SDK: a plain C API with fixed-width types only, so plugins
 * can be written in C, C++, Rust or any language with a C FFI.
 *
 * Conventions:
 *  - All strings are NUL-terminated UTF-8.
 *  - Data passed to a callback or vtable function is owned by the caller and
 *    valid only for the duration of that call. The receiver must copy it.
 *  - Functions must not let exceptions / panics cross the boundary.
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TORUSTIQ_SDK_ABI_VERSION 1u

/* Status codes. */
#define TORUSTIQ_OK 0
#define TORUSTIQ_ERROR (-1)
/* Returned by an emit callback: downstream accepts no more data; stop producing. */
#define TORUSTIQ_STOPPED 2

/* A byte array with explicit length (may contain NUL bytes). */
typedef struct torustiq_bytes {
    const uint8_t* data;
    size_t len;
} torustiq_bytes;

/* A string key-value pair: message header or configuration parameter. */
typedef struct torustiq_header {
    const char* key;
    const char* value;
} torustiq_header;

typedef struct torustiq_message {
    torustiq_bytes body;
    const torustiq_header* headers;
    size_t header_count;
} torustiq_message;

/* Component parameters (e.g. credentials), from the "config" section of the pipeline YAML. */
typedef struct torustiq_config {
    const torustiq_header* items;
    size_t count;
} torustiq_config;

/* Returns the value for `key`, or NULL if absent. */
static inline const char* torustiq_config_get(const torustiq_config* config, const char* key) {
    size_t i;
    if (config == NULL || key == NULL) {
        return NULL;
    }
    for (i = 0; i < config->count; ++i) {
        if (strcmp(config->items[i].key, key) == 0) {
            return config->items[i].value;
        }
    }
    return NULL;
}

/* Delivers a message downstream. Returns TORUSTIQ_OK, TORUSTIQ_STOPPED or TORUSTIQ_ERROR. */
typedef int32_t (*torustiq_emit_fn)(void* user_data, const torustiq_message* message);

/*
 * Input: produces messages. `run` blocks, emitting messages until the source is exhausted
 * (finite inputs), `stop` is called (continuous inputs), or emit returns non-OK.
 * `stop` may be called from another thread, before or during `run`, and more than once.
 */
typedef struct torustiq_input_vtable {
    uint32_t abi_version; /* TORUSTIQ_SDK_ABI_VERSION */
    const char* name;     /* type name used in the pipeline YAML */
    int32_t (*create)(const torustiq_config* config, void** out_handle);
    int32_t (*run)(void* handle, torustiq_emit_fn emit, void* user_data);
    void (*stop)(void* handle);
    void (*destroy)(void* handle);
} torustiq_input_vtable;

/* Transform: emits zero or more messages for each input message. */
typedef struct torustiq_transform_vtable {
    uint32_t abi_version;
    const char* name;
    int32_t (*create)(const torustiq_config* config, void** out_handle);
    int32_t (*process)(void* handle,
                       const torustiq_message* message,
                       torustiq_emit_fn emit,
                       void* user_data);
    void (*destroy)(void* handle);
} torustiq_transform_vtable;

/* Output: consumes messages. `flush` is called once when the stream ends. */
typedef struct torustiq_output_vtable {
    uint32_t abi_version;
    const char* name;
    int32_t (*create)(const torustiq_config* config, void** out_handle);
    int32_t (*write)(void* handle, const torustiq_message* message);
    int32_t (*flush)(void* handle);
    void (*destroy)(void* handle);
} torustiq_output_vtable;

/* Passed to a plugin's registration function. The host copies the vtables. */
typedef struct torustiq_registrar {
    void* context;
    int32_t (*register_input)(void* context, const torustiq_input_vtable* vtable);
    int32_t (*register_transform)(void* context, const torustiq_transform_vtable* vtable);
    int32_t (*register_output)(void* context, const torustiq_output_vtable* vtable);
} torustiq_registrar;

/*
 * Entry point exported by dynamic plugin libraries:
 *   int32_t torustiq_plugin_register(const torustiq_registrar* registrar);
 * Built-in plugins use differently named registration functions with the same signature.
 */
typedef int32_t (*torustiq_plugin_register_fn)(const torustiq_registrar* registrar);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TORUSTIQ_SDK_H_ */
