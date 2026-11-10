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

/* Registration function. Built-in plugins export differently named ones with this signature. */
typedef int32_t (*torustiq_plugin_register_fn)(const torustiq_registrar* registrar);

/* General information about a plugin library. */
typedef struct torustiq_plugin_info {
    uint32_t abi_version; /* TORUSTIQ_SDK_ABI_VERSION */
    const char* id;       /* unique plugin ID; also the `type` used in the pipeline YAML */
} torustiq_plugin_info;

/*
 * Entry points exported by dynamic plugin libraries (extern "C", unmangled names below).
 *
 * Library level (once per library, shared by all pipeline steps that use it):
 *   const torustiq_plugin_info* torustiq_plugin_get_info(void);
 *       Mandatory. Must have no side effects: the host calls it on every library it finds,
 *       including those the pipeline does not use. The result must stay valid while the
 *       library is loaded.
 *   int32_t torustiq_plugin_init(const torustiq_registrar* registrar);
 *       Mandatory. Called once, and only if the pipeline refers to the plugin ID. Registers
 *       vtables whose `name` equals the plugin ID.
 *   void torustiq_plugin_shutdown(void);
 *       Optional. Called once when the host unloads the library, after all steps are destroyed.
 *
 * Step level: each pipeline step gets its own `create` / `destroy` call of the registered vtable,
 * so one library used by several steps is initialised once but creates several step instances.
 */
#define TORUSTIQ_PLUGIN_GET_INFO_SYMBOL "torustiq_plugin_get_info"
#define TORUSTIQ_PLUGIN_INIT_SYMBOL "torustiq_plugin_init"
#define TORUSTIQ_PLUGIN_SHUTDOWN_SYMBOL "torustiq_plugin_shutdown"

typedef const torustiq_plugin_info* (*torustiq_plugin_get_info_fn)(void);
typedef torustiq_plugin_register_fn torustiq_plugin_init_fn;
typedef void (*torustiq_plugin_shutdown_fn)(void);

/* Visibility of exported plugin entry points. */
#if defined(_WIN32)
#define TORUSTIQ_PLUGIN_EXPORT __declspec(dllexport)
#else
#define TORUSTIQ_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TORUSTIQ_SDK_H_ */
