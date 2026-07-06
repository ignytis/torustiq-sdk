#ifndef _TORUSTIQ_LIB_SDK_TYPEDEFS_H_
#define _TORUSTIQ_LIB_SDK_TYPEDEFS_H_

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TorustiqPluginInfo {
    const char* host_app;
    const unsigned int api_version;

    const char* id;
    const char* name;
} TorustiqPluginInfo;

typedef enum TorustiqMessageType {
    /**
     * Regular data message.
     */
    TORUSTIQ_MESSAGE_TYPE_DATA,
    /**
     * Send when end of data is reached.
     */
    TORUSTIQ_MESSAGE_TYPE_EOF,
} TorustiqMessageType;

typedef struct TorustiqMessageHeader {
    const char* key;
    const char* value;
} TorustiqMessageHeader;

typedef struct TorustiqMessage {
    TorustiqMessageType type;
    size_t payload_size;
    uint8_t* payload;
    size_t headers_count;
    TorustiqMessageHeader* headers;
} TorustiqMessage;

typedef enum TorustiqPluginStageKind {
    TORUSTIQ_PLUGIN_STAGE_KIND_SOURCE,
    TORUSTIQ_PLUGIN_STAGE_KIND_PROCESSOR,
    TORUSTIQ_PLUGIN_STAGE_KIND_SINK,
} TorustiqPluginStageKind;

/** A handle to a stage instance */
typedef unsigned int TorustiqPluginStageHandle;

/**
 * torustiq_plugin_get_info function
 * Returns the general information about plugin which helps the host
 * application to validate the plugin and its compatibility
 */
typedef const TorustiqPluginInfo (*TorustiqPluginGetInfoFnPtr)();

typedef struct CreateNewStageFnArgs {
    TorustiqPluginStageHandle stageHandle;
    TorustiqPluginStageKind stageKind;
} CreateNewStageFnArgs;

/**
 * torustiq_plugin_create_new_stage function
 * This function is expected to allocate resources for a new stage.
 */
typedef void (*TorustiqPluginCreateNewStageFnPtr)(CreateNewStageFnArgs args);

/**
 * torustiq_plugin_stage_start function
 * Starts the stage processing. Usually it includes some preparation like opening
 * files or sockets and then an infinite loop which processes the data
 */
typedef void (*TorustiqPluginStageStartFnPtr)(TorustiqPluginStageHandle stageHandle);

/**
 * torustiq_plugin_set_config_value function
 * Sets a value for stage confiuguration key.
 */
typedef void (*TorustiqPluginSetStageConfigValueFnPtr)(TorustiqPluginStageHandle stageHandle, const char *key, const char *value);

typedef struct TorustiqPlugin {
    TorustiqPluginCreateNewStageFnPtr       fn_stage_create_new;
    TorustiqPluginSetStageConfigValueFnPtr  fn_stage_set_config_value;
    TorustiqPluginStageStartFnPtr           fn_stage_start;
} TorustiqPlugin;

/**
 * This function is called from modules to enqueue a message to the pipeline.
 */
typedef const TorustiqMessage* (*TorustiqHostReceiveMessageFnPtr)(TorustiqPluginStageHandle);

/**
 * This function is called from modules to enqueue a message to the pipeline.
 */
typedef void (*TorustiqHostSendMessageFnPtr)(TorustiqPluginStageHandle, const TorustiqMessage*);

/**
 * Globals shared with plugin by host
 */
typedef struct TorustiqHostGlobals {
    TorustiqHostReceiveMessageFnPtr receiveMessageFnPtr;
    TorustiqHostSendMessageFnPtr sendMessageFnPtr;
} TorustiqHostGlobals;

/**
 * torustiq_plugin_init function
 * Initializes the plugin.
 * NB: this is about initializing the general plugin configuration
 * which could be shared between stages
 */
typedef const TorustiqPlugin (*TorustiqPluginInitFnPtr)(TorustiqHostGlobals globals);

#ifdef __cplusplus
}
#endif

#endif  // _TORUSTIQ_LIB_SDK_TYPEDEFS_H_
