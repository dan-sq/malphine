#pragma once
#include <vector>
#include "board/board.h"
#include "move/move.h"

namespace Movegen {
    inline std::array<uint64_t, 64> bishop_magics = {};
    inline std::array<uint64_t, 64> bishop_masks = {};
    inline std::array<uint64_t, 64> bishop_shifts = {};
    inline std::array<std::array<uint64_t, 512>, 64> bishop_attacks = {};
    constexpr int KNIGHT_OFFSETS[8] = { 17, 10, -6, -17, -15, -10, 6, 15 };
    constexpr auto FILE_A = static_cast<uint64_t>(0x0101010101010101);
    constexpr auto FILE_B = static_cast<uint64_t>(0x0404040404040404);
    constexpr auto FILE_G = static_cast<uint64_t>(0x7070707070707070);
    constexpr auto FILE_H = static_cast<uint64_t>(0x8080808080808080);
    constexpr auto rank1 = static_cast<uint64_t>(0x00000000000000FF);
    constexpr auto rank2 = static_cast<uint64_t>(0x000000000000FF00);
    constexpr auto rank7 = static_cast<uint64_t>(0x00FF000000000000);
    constexpr auto rank8 = static_cast<uint64_t>(0xFF00000000000000);
    uint64_t blocked_bishop_attacks(uint64_t bb, uint8_t sq);
    uint64_t bb_from_idx(uint64_t idx, uint64_t bb);
    void init_bishop_magics();
    void generate_pawn_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_knight_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_bishop_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_rook_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_queen_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_king_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_all_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
}
