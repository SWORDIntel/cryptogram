#pragma once

#include <variant>
#include <stdexcept>
#include <utility>

namespace tl {

template<typename E>
class unexpected;

template<typename T, typename E>
class expected {
public:
    expected() = default;
    expected(const T &value) : _data(std::in_place_index<0>, value) {}
    expected(T &&value) : _data(std::in_place_index<0>, std::move(value)) {}
    
    struct unexpected_t {
        E error;
        explicit unexpected_t(E e) : error(std::move(e)) {}
    };
    
    expected(unexpected_t u) : _data(std::in_place_index<1>, std::move(u.error)) {}
    
    template<typename E2>
    expected(unexpected<E2> u) : _data(std::in_place_index<1>, std::move(u.value())) {}
    
    bool has_value() const { return _data.index() == 0; }
    explicit operator bool() const { return has_value(); }
    
    T &value() { return std::get<0>(_data); }
    const T &value() const { return std::get<0>(_data); }
    T &operator*() { return std::get<0>(_data); }
    const T &operator*() const { return std::get<0>(_data); }
    T *operator->() { return &std::get<0>(_data); }
    const T *operator->() const { return &std::get<0>(_data); }
    
    E &error() { return std::get<1>(_data); }
    const E &error() const { return std::get<1>(_data); }
    
    T value_or(T &&def) const {
        return has_value() ? std::get<0>(_data) : std::move(def);
    }
    
private:
    std::variant<T, E> _data;
};

template<typename E>
class unexpected {
public:
    explicit unexpected(E e) : _error(std::move(e)) {}
    E &&value() && { return std::move(_error); }
    const E &value() const & { return _error; }
private:
    E _error;
};

template<typename E>
unexpected<E> make_unexpected(E &&e) {
    return unexpected<E>(std::forward<E>(e));
}

} // namespace tl

namespace base {

using ::tl::expected;
using ::tl::unexpected;
using ::tl::make_unexpected;

} // namespace base
