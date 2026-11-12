#include <stdio.h>
#include <string.h>

#include "torustiq/sdk.h"

#define CHECK(cond)                                                                                \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            fprintf(stderr, "CHECK failed: %s\n", #cond);                                          \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

int main(void) {
    const torustiq_header items[] = {{"user", "bob"}, {"pass", "x"}};
    torustiq_config config;
    config.items = items;
    config.count = 2;

    CHECK(strcmp(torustiq_config_get(&config, "user"), "bob") == 0);
    CHECK(torustiq_config_get(&config, "missing") == NULL);
    CHECK(torustiq_config_get(NULL, "user") == NULL);
    CHECK(torustiq_config_get(&config, NULL) == NULL);
    CHECK(TORUSTIQ_SDK_ABI_VERSION == 2u);

    {
        /* {"user": {"name": "bob"}, "tags": ["a"]} */
        torustiq_map_entry user_entries[1];
        torustiq_map_entry root_entries[2];
        torustiq_value tag;
        torustiq_value root;
        const torustiq_value* found;
        torustiq_message message;
        const uint8_t raw[] = {1, 2, 3};

        memset(user_entries, 0, sizeof user_entries);
        memset(root_entries, 0, sizeof root_entries);
        memset(&tag, 0, sizeof tag);
        memset(&root, 0, sizeof root);

        user_entries[0].key.data = (const uint8_t*)"name";
        user_entries[0].key.len = 4;
        user_entries[0].value.kind = TORUSTIQ_VALUE_STRING;
        user_entries[0].value.as.bytes.data = (const uint8_t*)"bob";
        user_entries[0].value.as.bytes.len = 3;
        tag.kind = TORUSTIQ_VALUE_INT;
        tag.as.i = 42;
        root_entries[0].key.data = (const uint8_t*)"user";
        root_entries[0].key.len = 4;
        root_entries[0].value.kind = TORUSTIQ_VALUE_MAP;
        root_entries[0].value.as.map.entries = user_entries;
        root_entries[0].value.as.map.count = 1;
        root_entries[1].key.data = (const uint8_t*)"tags";
        root_entries[1].key.len = 4;
        root_entries[1].value.kind = TORUSTIQ_VALUE_ARRAY;
        root_entries[1].value.as.array.items = &tag;
        root_entries[1].value.as.array.count = 1;
        root.kind = TORUSTIQ_VALUE_MAP;
        root.as.map.entries = root_entries;
        root.as.map.count = 2;

        found = torustiq_value_map_get(torustiq_value_map_get(&root, "user"), "name");
        CHECK(found != NULL && found->kind == TORUSTIQ_VALUE_STRING);
        found = torustiq_value_array_at(torustiq_value_map_get(&root, "tags"), 0);
        CHECK(found != NULL && found->as.i == 42);
        CHECK(torustiq_value_array_at(torustiq_value_map_get(&root, "tags"), 1) == NULL);
        CHECK(torustiq_value_map_get(&root, "missing") == NULL);
        CHECK(torustiq_value_map_get(&tag, "user") == NULL);
        CHECK(torustiq_value_array_at(&root, 0) == NULL);
        CHECK(torustiq_value_map_get(NULL, "user") == NULL);

        message.payload = torustiq_value_from_bytes(raw, sizeof raw);
        message.headers = NULL;
        message.header_count = 0;
        CHECK(message.payload.kind == TORUSTIQ_VALUE_BYTES && message.payload.as.bytes.len == 3);

        message.payload = torustiq_value_from_text("hi", 2);
        CHECK(message.payload.kind == TORUSTIQ_VALUE_STRING && message.payload.as.bytes.len == 2);
        CHECK(TORUSTIQ_PAYLOAD_TEXT != TORUSTIQ_PAYLOAD_BYTES &&
              TORUSTIQ_PAYLOAD_TEXT != TORUSTIQ_PAYLOAD_VALUE);
    }

    {
        const torustiq_plugin_info info = {TORUSTIQ_SDK_ABI_VERSION, "fake"};
        torustiq_plugin_get_info_fn get_info = NULL;
        torustiq_plugin_init_fn init = NULL;
        torustiq_plugin_shutdown_fn shutdown = NULL;
        CHECK(get_info == NULL && init == NULL && shutdown == NULL);
        CHECK(strcmp(info.id, "fake") == 0);
        CHECK(strcmp(TORUSTIQ_PLUGIN_GET_INFO_SYMBOL, "torustiq_plugin_get_info") == 0);
        CHECK(strcmp(TORUSTIQ_PLUGIN_INIT_SYMBOL, "torustiq_plugin_init") == 0);
        CHECK(strcmp(TORUSTIQ_PLUGIN_SHUTDOWN_SYMBOL, "torustiq_plugin_shutdown") == 0);
    }
    return 0;
}
