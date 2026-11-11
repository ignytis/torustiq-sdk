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

    // {"user": {"name": "bob", "age": 7}, "tags": [1.5]}
    torustiq_map_entry user[2] = {};
    user[0].key = {reinterpret_cast<const uint8_t*>("name"), 4};
    user[0].value.kind = TORUSTIQ_VALUE_STRING;
    user[0].value.as.bytes = {reinterpret_cast<const uint8_t*>("bob"), 3};
    user[1].key = {reinterpret_cast<const uint8_t*>("age"), 3};
    user[1].value.kind = TORUSTIQ_VALUE_INT;
    user[1].value.as.i = 7;
    torustiq_value tag = {};
    tag.kind = TORUSTIQ_VALUE_DOUBLE;
    tag.as.d = 1.5;
    torustiq_map_entry root_entries[2] = {};
    root_entries[0].key = {reinterpret_cast<const uint8_t*>("user"), 4};
    root_entries[0].value.kind = TORUSTIQ_VALUE_MAP;
    root_entries[0].value.as.map = {user, 2};
    root_entries[1].key = {reinterpret_cast<const uint8_t*>("tags"), 4};
    root_entries[1].value.kind = TORUSTIQ_VALUE_ARRAY;
    root_entries[1].value.as.array = {&tag, 1};
    torustiq_value root = {};
    root.kind = TORUSTIQ_VALUE_MAP;
    root.as.map = {root_entries, 2};

    const torustiq::ValueRef ref(root);
    if (ref["user"]["name"].AsString() != "bob" || ref["user"]["age"].AsInt() != 7 ||
        ref["tags"][0].AsDouble() != 1.5 || ref["tags"].size() != 1 || ref.size() != 2) {
        std::cerr << "ValueRef should read nested values\n";
        return EXIT_FAILURE;
    }
    if (!ref["missing"]["deeper"].IsNull() || ref["user"]["name"].AsInt(-1) != -1 ||
        ref["tags"][5].AsString("d") != "d" || !torustiq::ValueRef().IsNull()) {
        std::cerr << "ValueRef should tolerate absent members and kind mismatches\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
