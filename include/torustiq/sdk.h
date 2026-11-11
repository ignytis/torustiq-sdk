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
/* Returned by a step's `create`: it cannot handle the payload kind of the upstream step. */
#define TORUSTIQ_UNSUPPORTED_PAYLOAD 3

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

/* Kinds of a payload value. */
typedef enum torustiq_value_kind {
    TORUSTIQ_VALUE_NULL = 0,
    TORUSTIQ_VALUE_BOOL = 1,   /* as.b: 0 or 1 */
    TORUSTIQ_VALUE_INT = 2,    /* as.i */
    TORUSTIQ_VALUE_DOUBLE = 3, /* as.d */
    TORUSTIQ_VALUE_STRING = 4, /* as.bytes: UTF-8, not necessarily NUL-terminated */
    TORUSTIQ_VALUE_BYTES = 5,  /* as.bytes: opaque data */
    TORUSTIQ_VALUE_ARRAY = 6,  /* as.array */
    TORUSTIQ_VALUE_MAP = 7     /* as.map: string keys, in order */
} torustiq_value_kind;

struct torustiq_map_entry;

/*
 * A payload value: either simple data (BYTES) or structured data (any other kind), so structured
 * components can read and modify attributes without parsing and re-serialising. Children are
 * referenced by pointer, which lets a plugin build a modified tree that shares the unchanged
 * parts of its input. Copying a torustiq_value is a shallow copy.
 */
typedef struct torustiq_value {
    uint32_t kind; /* torustiq_value_kind */
    union {
        int32_t b;
        int64_t i;
        double d;
        torustiq_bytes bytes;
        struct {
            const struct torustiq_value* items;
            size_t count;
        } array;
        struct {
            const struct torustiq_map_entry* entries;
            size_t count;
        } map;
    } as;
} torustiq_value;

typedef struct torustiq_map_entry {
    torustiq_bytes key; /* UTF-8, not necessarily NUL-terminated */
    torustiq_value value;
} torustiq_map_entry;

/* Returns the value for the NUL-terminated `key` of a MAP value, or NULL if absent / not a map. */
static inline const torustiq_value* torustiq_value_map_get(const torustiq_value* value,
                                                           const char* key) {
    size_t i;
    size_t key_len;
    if (value == NULL || key == NULL || value->kind != TORUSTIQ_VALUE_MAP) {
        return NULL;
    }
    key_len = strlen(key);
    for (i = 0; i < value->as.map.count; ++i) {
        const torustiq_map_entry* entry = &value->as.map.entries[i];
        if (entry->key.len == key_len && memcmp(entry->key.data, key, key_len) == 0) {
            return &entry->value;
        }
    }
    return NULL;
}

/* Returns element `index` of an ARRAY value, or NULL if out of range / not an array. */
static inline const torustiq_value* torustiq_value_array_at(const torustiq_value* value,
                                                            size_t index) {
    if (value == NULL || value->kind != TORUSTIQ_VALUE_ARRAY || index >= value->as.array.count) {
        return NULL;
    }
    return &value->as.array.items[index];
}

/* Builds a BYTES value. */
static inline torustiq_value torustiq_value_from_bytes(const uint8_t* data, size_t len) {
    torustiq_value value;
    memset(&value, 0, sizeof value);
    value.kind = TORUSTIQ_VALUE_BYTES;
    value.as.bytes.data = data;
    value.as.bytes.len = len;
    return value;
}

/*
 * A message. `payload` is simple data (kind BYTES) or structured data (any other kind). The kind
 * is agreed once when the pipeline is built (see torustiq_payload_spec), so a step does not need
 * to check it for every message.
 */
typedef struct torustiq_message {
    torustiq_value payload;
    const torustiq_header* headers;
    size_t header_count;
} torustiq_message;

/* What a step receives or emits: simple data (BYTES) or structured data (VALUE). */
#define TORUSTIQ_PAYLOAD_BYTES 0u
#define TORUSTIQ_PAYLOAD_VALUE 1u

/*
 * Payload contract between neighbouring steps, negotiated while the pipeline is built, before
 * any message flows. The host passes the upstream step's output spec to the `create` of the
 * next step; a step that cannot handle it returns TORUSTIQ_UNSUPPORTED_PAYLOAD. At run time the
 * host guarantees that every message matches the agreed spec.
 */
typedef struct torustiq_payload_spec {
    uint32_t kind; /* TORUSTIQ_PAYLOAD_BYTES or TORUSTIQ_PAYLOAD_VALUE */
} torustiq_payload_spec;

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
    /* `out_output` is the payload kind this input will emit. */
    int32_t (*create)(const torustiq_config* config,
                      torustiq_payload_spec* out_output,
                      void** out_handle);
    int32_t (*run)(void* handle, torustiq_emit_fn emit, void* user_data);
    void (*stop)(void* handle);
    void (*destroy)(void* handle);
} torustiq_input_vtable;

/* Transform: emits zero or more messages for each input message. */
typedef struct torustiq_transform_vtable {
    uint32_t abi_version;
    const char* name;
    /* `input` is what the step will receive; it fills `out_output` with what it will emit. */
    int32_t (*create)(const torustiq_config* config,
                      const torustiq_payload_spec* input,
                      torustiq_payload_spec* out_output,
                      void** out_handle);
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
    /* `input` is what the step will receive. */
    int32_t (*create)(const torustiq_config* config,
                      const torustiq_payload_spec* input,
                      void** out_handle);
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
