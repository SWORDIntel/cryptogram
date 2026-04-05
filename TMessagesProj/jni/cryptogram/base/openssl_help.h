#pragma once
#include <openssl/evp.h>
#include <vector>
#include <cstddef>

namespace bytes {
    using vector = std::vector<unsigned char>;
    using const_span = std::vector<unsigned char>; // Simple shim
    using span = std::vector<unsigned char>;
}
