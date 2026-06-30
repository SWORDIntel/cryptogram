#pragma once

namespace base::flags {

template <typename T>
class flag {
public:
    constexpr explicit flag(T value) : _value(value) {
    }

    constexpr operator T() const {
        return _value;
    }

    constexpr T value() const {
        return _value;
    }

private:
    T _value;
};

} // namespace base::flags
