#pragma once

#include <array>
#include <cstdint>

namespace Zobrist {
    inline std::array<std::array<uint64_t, 64>, 6> white_keys;
    inline std::array<std::array<uint64_t, 64>, 6> black_keys;
    inline uint64_t side_key;
    inline std::array<uint64_t, 16> cstl_keys;
    inline std::array<uint64_t, 8> ep_keys;

    void init();
}
