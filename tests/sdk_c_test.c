#include <stdio.h>
#include <string.h>

#include "torustiq/sdk.h"

#define CHECK(cond)                                           \
    do {                                                      \
        if (!(cond)) {                                        \
            fprintf(stderr, "CHECK failed: %s\n", #cond);     \
            return 1;                                         \
        }                                                     \
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
    return 0;
}
