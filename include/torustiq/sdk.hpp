#ifndef TORUSTIQ_SDK_HPP_
#define TORUSTIQ_SDK_HPP_

// Optional C++ conveniences on top of the C API in sdk.h.

#include <string>

#include "torustiq/sdk.h"

namespace torustiq {

// Returns the config value for `key` or `default_value` if absent.
inline std::string
ConfigGet(const torustiq_config* config, const char* key, const std::string& default_value = {}) {
    const char* value = torustiq_config_get(config, key);
    return value != nullptr ? std::string(value) : default_value;
}

} // namespace torustiq

#endif // TORUSTIQ_SDK_HPP_
