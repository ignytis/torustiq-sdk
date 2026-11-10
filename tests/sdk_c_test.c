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
    CHECK(TORUSTIQ_SDK_ABI_VERSION == 1u);

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
