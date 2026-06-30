#pragma once

namespace base {

template <typename T = void>
class weak_ptr {
public:
    weak_ptr() = default;
    T *get() const { return nullptr; }
    explicit operator bool() const { return false; }
};

class has_weak_ptr {
public:
    virtual ~has_weak_ptr() = default;
};

} // namespace base
