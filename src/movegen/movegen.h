#pragma once
#include <vector>
#include "board/board.h"
#include "move/move.h"

namespace Movegen {
    struct DiagonalCache {
        std::array<uint64_t, 64> masks = {};
        std::array<uint64_t, 64> shifts = {};
        std::array<std::array<uint64_t, 512>, 64> moves = {};
    };

    struct HorizontalCache {
        std::array<uint64_t, 64> masks = {};
        std::array<uint64_t, 64> shifts = {};
        std::array<std::array<uint64_t, 4096>, 64> moves = {};
    };

    enum class GenType { ALL, CAPTURES };
    constexpr int KNIGHT_OFFSETS[8] = { 17, 10, -6, -17, -15, -10, 6, 15 };
    constexpr int KING_OFFSETS[8] = { 7, 8, 9, 1, -7, -8, -9, -1 };
    constexpr auto FILE_A = static_cast<uint64_t>(0x0101010101010101);
    constexpr auto FILE_H = static_cast<uint64_t>(0x8080808080808080);
    constexpr auto rank1 = static_cast<uint64_t>(0x00000000000000FF);
    constexpr auto rank2 = static_cast<uint64_t>(0x000000000000FF00);
    constexpr auto rank7 = static_cast<uint64_t>(0x00FF000000000000);
    constexpr auto rank8 = static_cast<uint64_t>(0xFF00000000000000);
    void init();
    template<GenType type>
    void generate_pawn_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    template<GenType type>
    void generate_knight_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    template<GenType type>
    void generate_bishop_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    template<GenType type>
    void generate_rook_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    template<GenType type>
    void generate_queen_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    template<GenType type>
    void generate_king_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    template<GenType type>
    void generate_pseudo_legal_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    bool is_sq_attacked_by_color(Position& pos, uint8_t sq, PIECE_C color);
    bool make(Position& pos, Move move);
    void unmake(Position& pos, Move move);
    uint64_t perft(Position& pos, uint64_t depth);
}
