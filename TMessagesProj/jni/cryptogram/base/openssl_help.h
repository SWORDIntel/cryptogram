#pragma once
#include <openssl/evp.h>
#include <vector>
#include <cstddef>

namespace bytes {
    using type = unsigned char;
    using vector = std::vector<unsigned char>;
    using const_span = std::vector<unsigned char>; // Simple shim
    using span = std::vector<unsigned char>;

    template <typename Buffer>
    inline vector make_vector(const Buffer &buffer) {
        const auto *data = reinterpret_cast<const unsigned char*>(buffer.data());
        return vector(data, data + buffer.size());
    }

    template <typename Buffer>
    inline span make_span(const Buffer &buffer) {
        return make_vector(buffer);
    }

    inline void append_one(vector &output, const vector &input) {
        output.insert(output.end(), input.begin(), input.end());
    }

    template <typename... Buffers>
    inline vector concatenate(const vector &first, const Buffers&... rest) {
        vector output = first;
        (append_one(output, rest), ...);
        return output;
    }

    inline int compare(const vector &lhs, const vector &rhs) {
        if (lhs.size() != rhs.size()) {
            return (lhs.size() < rhs.size()) ? -1 : 1;
        }
        return std::memcmp(lhs.data(), rhs.data(), lhs.size());
    }
}
