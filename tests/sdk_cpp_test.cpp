#include <cstdlib>
#include <iostream>

#include "torustiq/sdk.hpp"

int main() {
    const torustiq_header items[] = {{"format", "%BODY%"}};
    const torustiq_config config{items, 1};

    if (torustiq::ConfigGet(&config, "format") != "%BODY%") {
        std::cerr << "ConfigGet should return existing value\n";
        return EXIT_FAILURE;
    }
    if (torustiq::ConfigGet(&config, "nope", "dflt") != "dflt") {
        std::cerr << "ConfigGet should return the default\n";
        return EXIT_FAILURE;
    }
    if (!torustiq::ConfigGet(nullptr, "nope").empty()) {
        std::cerr << "ConfigGet(nullptr) should be empty\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
