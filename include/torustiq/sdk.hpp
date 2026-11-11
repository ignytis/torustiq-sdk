#ifndef TORUSTIQ_SDK_HPP_
#define TORUSTIQ_SDK_HPP_

// Optional C++ conveniences on top of the C API in sdk.h.

#include <string>
#include <string_view>

#include "torustiq/sdk.h"

namespace torustiq {

// Returns the config value for `key` or `default_value` if absent.
inline std::string
ConfigGet(const torustiq_config* config, const char* key, const std::string& default_value = {}) {
    const char* value = torustiq_config_get(config, key);
    return value != nullptr ? std::string(value) : default_value;
}

// Read-only view of a torustiq_value for navigating structured payloads. Missing members and kind
// mismatches yield a null ref / default values instead of failing, so chains like
// `ValueRef(msg->payload)["user"]["name"].AsString()` are safe. Valid while the message is.
class ValueRef {
  public:
    ValueRef() = default;
    explicit ValueRef(const torustiq_value& value) : value_(&value) {}
    explicit ValueRef(const torustiq_value* value) : value_(value) {}

    bool IsNull() const {
        return value_ == nullptr || value_->kind == TORUSTIQ_VALUE_NULL;
    }
    uint32_t kind() const {
        return value_ != nullptr ? value_->kind : static_cast<uint32_t>(TORUSTIQ_VALUE_NULL);
    }
    const torustiq_value* raw() const {
        return value_;
    }

    // Map member / array element; a null ref if absent or not a map / array.
    ValueRef operator[](const char* key) const {
        return ValueRef(torustiq_value_map_get(value_, key));
    }
    ValueRef operator[](size_t index) const {
        return ValueRef(torustiq_value_array_at(value_, index));
    }
    // (An int overload keeps `ref[0]` from being ambiguous with a null `const char*`.)
    ValueRef operator[](int index) const {
        return index < 0 ? ValueRef() : (*this)[static_cast<size_t>(index)];
    }

    size_t size() const {
        if (kind() == TORUSTIQ_VALUE_ARRAY) {
            return value_->as.array.count;
        }
        return kind() == TORUSTIQ_VALUE_MAP ? value_->as.map.count : 0;
    }

    // Text of a STRING (or BYTES) value; `default_value` for any other kind.
    std::string AsString(const std::string& default_value = {}) const {
        if (kind() != TORUSTIQ_VALUE_STRING && kind() != TORUSTIQ_VALUE_BYTES) {
            return default_value;
        }
        return std::string(reinterpret_cast<const char*>(value_->as.bytes.data),
                           value_->as.bytes.len);
    }
    int64_t AsInt(int64_t default_value = 0) const {
        return kind() == TORUSTIQ_VALUE_INT ? value_->as.i : default_value;
    }
    double AsDouble(double default_value = 0) const {
        if (kind() == TORUSTIQ_VALUE_DOUBLE) {
            return value_->as.d;
        }
        return kind() == TORUSTIQ_VALUE_INT ? static_cast<double>(value_->as.i) : default_value;
    }
    bool AsBool(bool default_value = false) const {
        return kind() == TORUSTIQ_VALUE_BOOL ? value_->as.b != 0 : default_value;
    }

  private:
    const torustiq_value* value_ = nullptr;
};

} // namespace torustiq

#endif // TORUSTIQ_SDK_HPP_
