#pragma once

#include <cstdint>
#include <array>
#include <vector>

enum class PIECE_T { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE };
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
        void remove_piece(PIECE_C color, PIECE_T type, int square);
        uint64_t get_colored_pieces(PIECE_C color) const;
        uint64_t get_both_pieces() const;
        PIECE_T get_piece_on(uint8_t sq) const;
};

struct Undo {
    PIECE_C side;
    uint8_t castle;
    uint8_t en_pas;
    uint8_t half_moves;
    uint8_t ply;
    uint8_t full_moves;
    PIECE_T captured = PIECE_T::NONE;
};

class Position {
    private:
        PIECE_C side;
        uint8_t castle;
        uint8_t en_pas;
        uint8_t half_moves;
        uint8_t ply;
        uint8_t full_moves;
        std::vector<Undo> undos;
        uint64_t zobrist_hash;

    public:
        Position() : side(PIECE_C::WHITE), castle(0), en_pas(64), half_moves(0), full_moves(1), ply(0) {
            undos.reserve(64);
        }
        Pieces pieces;

        void set_piece(PIECE_C color, PIECE_T type, int square);
        void remove_piece(PIECE_C color, PIECE_T type, int square);
        void set_side(PIECE_C s);
        PIECE_C get_side();
        PIECE_C fetch_and_change_side();
        void set_castle(uint8_t cstl);
        uint8_t get_castle();
        void set_en_pas(uint8_t ep);
        uint8_t get_en_pas() const;
        void set_half_moves(uint8_t hm);
        void set_full_moves(uint8_t fm);
        void set_ply(uint8_t p);
        uint8_t get_half_moves();
        uint8_t get_full_moves();
        uint8_t get_ply();
        Undo fetch_and_pop_undos();
        void append_to_undos(Undo undo);
        uint64_t get_zobrist_hash();
        void set_zobrist_hash(uint64_t hash);
};
