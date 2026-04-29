#pragma once
#include <vector>
#include "board/board.h"
#include "move/move.h"

enum class PAWN_FLAG { NONE = 0, PROMOTION = 1, EN_PAS = 2 };

namespace Movegen {
    constexpr int KNIGHT_OFFSETS[8] = {};
    constexpr auto FILE_A = static_cast<uint64_t>(0x0101010101010101);
    constexpr auto FILE_H = static_cast<uint64_t>(0x8080808080808080);
    constexpr auto rank2 = static_cast<uint64_t>(0x000000000000FF00);
    constexpr auto rank7 = static_cast<uint64_t>(0x00FF000000000000);
    void pawn_move(std::vector<Move>& moves, uint8_t from, uint8_t to, PAWN_FLAG flag);
    void generate_pawn_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_knight_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_bishop_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_rook_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_queen_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_king_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
    void generate_all_moves(Position& pos, PIECE_C color, std::vector<Move>& moves);
}
