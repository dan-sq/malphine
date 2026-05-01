#pragma once
#include <cstdint>

namespace Magics {
    uint64_t bb_from_idx(uint64_t idx, uint64_t bb);
    uint64_t sq_to_diag_move_bb(uint8_t sq);
    uint64_t sq_to_hori_move_bb(uint8_t sq);
    uint64_t generate_diagonal_moves(uint64_t bb, uint8_t sq);
    uint64_t generate_horizontal_moves(uint64_t bb, uint8_t sq);
    uint64_t generate_diag_magic_number(uint8_t sq);
    uint64_t generate_hori_magic_number(uint8_t sq);
    void find_diag_magics();
    void find_hori_magics();
}
