#pragma once

#include <cstdint>
#include <array>

enum class PIECE_T { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum class PIECE_C { WHITE, BLACK, BOTH };
enum class FILE_IDX { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };
enum class RANK_IDX { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };
enum class CASTLE_PERM { WKC = 1 << 0, WQC = 1 << 1, BKC = 1 << 2, BQC = 1 << 3 };

class Pieces {
    private:
        static constexpr int NUM_COLORS = 2;
        static constexpr int NUM_PIECES = 6;
        std::array<std::array<uint64_t, NUM_PIECES>, NUM_COLORS> piece_board;
        std::array<uint64_t, NUM_COLORS + 1> occupancy_board;

    public:
        Pieces() : piece_board{}, occupancy_board{} {}

        uint64_t get_pieces(PIECE_C color, PIECE_T type) const;
        void set_piece(PIECE_C color, PIECE_T type, int square);
        uint64_t get_colored_pieces(PIECE_C color) const;
        uint64_t get_both_pieces() const;
};

class Position {
    private:
        PIECE_C side;
        uint8_t castle;
        uint8_t en_pas;
        uint8_t half_ply;
        uint8_t ply;

    public:
        Position() : side(PIECE_C::WHITE), castle(0), en_pas(64), half_ply(0), ply(0) {}
        Pieces pieces;

        void set_piece(PIECE_C color, PIECE_T type, int square);
        void set_side(PIECE_C s);
        void set_castle(uint8_t cstl);
        void set_en_pas(uint8_t ep);
        uint8_t get_en_pas() const;
        void set_half_ply(uint8_t hp);
        void set_ply(uint8_t p);
};
