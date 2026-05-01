#include <iostream>
#include <random>
#include <cstdint>
#include <tools/magics.h>
#include <movegen/movegen.h>

uint64_t Magics::bb_from_idx(uint64_t idx, uint64_t bb) {
    uint64_t blocker_bb = 0;
    uint64_t bit_index = 0;

    while (bb) {
        int sq = std::countr_zero(bb);
        bb &= bb - 1;

        if (idx & (static_cast<uint64_t>(1) << bit_index)) {
            blocker_bb |= static_cast<uint64_t>(1) << sq;
        }

        bit_index++;
    }

    return blocker_bb;
}

uint64_t Magics::sq_to_diag_move_bb(uint8_t sq) {
    int diagonals[4][2] = {
        { 1, 1 },
        { 1, -1 },
        {-1, 1 },
        { -1, -1 }
    };
    uint64_t blocker_mask_bb = 0;
    for(int i = 0; i < 4; i++) {
        int file = sq % 8 + diagonals[i][0];
        int rank = sq / 8 + diagonals[i][1];
        while((file > 0 && file < 7) && (rank > 0 && rank < 7)) {
            blocker_mask_bb |= static_cast<uint64_t>(1) << (8 * rank + file);
            file += diagonals[i][0];
            rank += diagonals[i][1];
        }
    }
    return blocker_mask_bb;
}

uint64_t Magics::sq_to_hori_move_bb(uint8_t sq) {
    int horizontals[4][2] = {
        { 0, 1 },
        { 1, 0 },
        { 0, -1 },
        { -1, 0 }
    };
    uint64_t blocker_mask_bb = 0;
    for(int i = 0; i < 4; i++) {
        int file = sq % 8 + horizontals[i][0];
        int rank = sq / 8 + horizontals[i][1];
        while((file > -1 && file < 8) && (rank > -1 && rank < 8)) {
            if(file + horizontals[i][0] < 0 || file + horizontals[i][0] >= 8 || rank + horizontals[i][1] < 0 || rank + horizontals[i][1] >= 8) {
                  break;
            }
            blocker_mask_bb |= static_cast<uint64_t>(1) << (8 * rank + file);
            file += horizontals[i][0];
            rank += horizontals[i][1];
        }
    }
    return blocker_mask_bb;
}

uint64_t Magics::generate_diagonal_moves(uint64_t bb, uint8_t sq) {
    int diagonals[4][2] = {
        { 1, 1 },
        { 1, -1 },
        { -1, 1 },
        { -1, -1 }
    };

    uint64_t res = 0;
    for(int i = 0; i < 4; i++) {
        int file = sq % 8 + diagonals[i][0];
        int rank = sq / 8 + diagonals[i][1];
        while((file > -1 && file < 8) && (rank > -1 && rank < 8)) {
            uint64_t to_bb = static_cast<uint64_t>(1) << (8 * rank + file);
            res |= to_bb;
            if(bb & to_bb) {
                break;
            }
            file += diagonals[i][0];
            rank += diagonals[i][1];
        }
    }

    return res;
}

uint64_t Magics::generate_horizontal_moves(uint64_t bb, uint8_t sq) {
    int horizontals[4][2] = {
        { 0, 1 },
        { 1, 0 },
        { 0, -1 },
        { -1, 0 }
    };

    uint64_t res = 0;
    for(int i = 0; i < 4; i++) {
        int file = sq % 8 + horizontals[i][0];
        int rank = sq / 8 + horizontals[i][1];
        while((file > -1 && file < 8) && (rank > -1 && rank < 8)) {
            uint64_t to_bb = static_cast<uint64_t>(1) << (8 * rank + file);
            res |= to_bb;
            if(bb & to_bb) {
                break;
            }
            file += horizontals[i][0];
            rank += horizontals[i][1];
        }
    }

    return res;
}

uint64_t Magics::generate_diag_magic_number(uint8_t sq) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<std::uint64_t>::max());
    auto moves_bb = sq_to_diag_move_bb(sq);
    auto ones_count = std::popcount(moves_bb);
    auto table_size = static_cast<uint64_t>(1) << ones_count;
    auto shift = 64 - ones_count;
    uint64_t magic = 0;
    uint64_t blocker_bb, attack_bb;

    while(1) {
        magic = dis(gen) & dis(gen) & dis(gen);
        std::array<uint64_t, 512> lookup = {};
        bool filled[512] = {};
        bool failed = 0;
        for(uint64_t idx = 0; idx < table_size; idx++) {
            auto blocker_bb = bb_from_idx(idx, moves_bb);
            auto attack_bb = generate_diagonal_moves(blocker_bb, sq);
            uint64_t magic_idx = (blocker_bb * magic) >> shift;
            if(!filled[magic_idx]) {
                filled[magic_idx] = 1;
                lookup[magic_idx] = attack_bb;
            } else if(lookup[magic_idx] != attack_bb){
                failed = 1;
                break;
            }
            if(failed) break;
        }
        if(!failed) {
            break;
        }
    }

    return magic;
}

uint64_t Magics::generate_hori_magic_number(uint8_t sq) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(0, std::numeric_limits<std::uint64_t>::max());
    auto moves_bb = sq_to_hori_move_bb(sq);
    auto ones_count = std::popcount(moves_bb);
    auto table_size = static_cast<uint64_t>(1) << ones_count;
    auto shift = 64 - ones_count;
    uint64_t magic = 0;
    uint64_t blocker_bb, attack_bb;

    while(1) {
        magic = dis(gen) & dis(gen) & dis(gen);
        std::array<uint64_t, 4096> lookup = {};
        bool filled[4096] = {};
        bool failed = 0;
        for(uint64_t idx = 0; idx < table_size; idx++) {
            blocker_bb = bb_from_idx(idx, moves_bb);
            attack_bb = generate_horizontal_moves(blocker_bb, sq);
            uint64_t magic_idx = (blocker_bb * magic) >> shift;
            if(!filled[magic_idx]) {
                filled[magic_idx] = 1;
                lookup[magic_idx] = attack_bb;
            } else if(lookup[magic_idx] != attack_bb){
                failed = 1;
                break;
            }
            if(failed) break;
        }
        if(!failed) {
            break;
        }
    }

    return magic;
}

void Magics::find_diag_magics() {
    for(int i = 0; i < 64; i++) {
        auto magic = generate_diag_magic_number(i);
        std::cout << "0x" << std::hex << magic << "ULL," << std::dec << std::endl;
    }
}
void Magics::find_hori_magics() {
    for(int i = 0; i < 64; i++) {
        auto magic = generate_hori_magic_number(i);
        std::cout << "0x" << std::hex << magic << "ULL," << std::dec << std::endl;
    }
}
